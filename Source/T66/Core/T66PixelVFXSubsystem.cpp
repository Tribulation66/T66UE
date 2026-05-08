// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PixelVFXSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CoreMisc.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	static const FName UserTintParam(TEXT("User.Tint"));
	static const FName UserColorParam(TEXT("User.Color"));
	static const FName UserSpriteSizeParam(TEXT("User.SpriteSize"));
	static const FSoftObjectPath PixelSystemPath(TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle"));
	static const FSoftObjectPath LegacyPixelSystemPath(TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1"));

	static ENCPoolMethod T66_GetPixelPoolingMethod(bool bAutoDestroy)
	{
		return bAutoDestroy ? ENCPoolMethod::AutoRelease : ENCPoolMethod::None;
	}

	static TAutoConsoleVariable<int32> CVarPixelVFXLowBudget(
		TEXT("T66.PixelVFX.LowBudget"),
		72,
		TEXT("Max low-priority pixel VFX spawns per frame."),
		ECVF_Default);

	static TAutoConsoleVariable<int32> CVarPixelVFXMediumBudget(
		TEXT("T66.PixelVFX.MediumBudget"),
		112,
		TEXT("Max medium-priority pixel VFX spawns per frame."),
		ECVF_Default);

	static TAutoConsoleVariable<int32> CVarPixelVFXHighBudget(
		TEXT("T66.PixelVFX.HighBudget"),
		144,
		TEXT("Max high-priority pixel VFX spawns per frame."),
		ECVF_Default);

	static TAutoConsoleVariable<int32> CVarPixelVFXUseEffectsScalability(
		TEXT("T66.PixelVFX.UseEffectsScalability"),
		1,
		TEXT("Scale pixel VFX frame budgets by sg.EffectsQuality."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarPixelVFXBudgetScale(
		TEXT("T66.PixelVFX.BudgetScale"),
		1.0f,
		TEXT("Global multiplier applied after EffectsQuality scaling."),
		ECVF_Default);

	static float T66_GetEffectsQualityScale()
	{
		if (CVarPixelVFXUseEffectsScalability.GetValueOnGameThread() == 0)
		{
			return 1.0f;
		}

		IConsoleVariable* EffectsQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.EffectsQuality"));
		const int32 EffectsQuality = EffectsQualityCVar ? FMath::Clamp(EffectsQualityCVar->GetInt(), 0, 4) : 3;
		switch (EffectsQuality)
		{
		case 0:
			return 0.35f;
		case 1:
			return 0.55f;
		case 2:
			return 0.75f;
		case 4:
			return 1.15f;
		case 3:
		default:
			return 1.0f;
		}
	}

	static int32 T66_ScalePixelBudget(const int32 BaseBudget, const ET66PixelVFXPriority Priority)
	{
		if (BaseBudget <= 0)
		{
			return BaseBudget;
		}

		const float Scale = FMath::Max(0.05f, T66_GetEffectsQualityScale() * CVarPixelVFXBudgetScale.GetValueOnGameThread());
		const int32 ScaledBudget = FMath::RoundToInt(static_cast<float>(BaseBudget) * Scale);
		const int32 MinimumBudget = (Priority == ET66PixelVFXPriority::High) ? 16 : (Priority == ET66PixelVFXPriority::Medium ? 12 : 8);
		return FMath::Max(MinimumBudget, ScaledBudget);
	}

	static UNiagaraSystem* T66_ResolveNiagaraSystem(const FSoftObjectPath& Path)
	{
		return Cast<UNiagaraSystem>(Path.ResolveObject());
	}
}

void UT66PixelVFXSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RequestDefaultPixelSystemsAsync();
}

void UT66PixelVFXSubsystem::Deinitialize()
{
	DefaultPixelSystemsLoadHandle.Reset();
	CachedPixelSystem = nullptr;
	CachedLegacyPixelSystem = nullptr;
	Super::Deinitialize();
}

bool UT66PixelVFXSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

UNiagaraComponent* UT66PixelVFXSubsystem::SpawnPixelAtLocation(
	const FVector& Location,
	const FLinearColor& Tint,
	const FVector2D& SpriteSize,
	ET66PixelVFXPriority Priority,
	const FRotator& Rotation,
	const FVector& Scale,
	UNiagaraSystem* SystemOverride,
	bool bAutoDestroy,
	bool bAutoActivate)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UNiagaraSystem* System = SystemOverride ? SystemOverride : GetDefaultPixelSystem();
	if (!System)
	{
		ResetFrameBudgetIfNeeded();
		++RequestedThisFrame;
		++DroppedThisFrame;
		++TotalRequested;
		++TotalDropped;
		return nullptr;
	}

	if (!ConsumeBudget(Priority))
	{
		return nullptr;
	}

	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		System,
		Location,
		Rotation,
		Scale,
		bAutoDestroy,
		bAutoActivate,
		T66_GetPixelPoolingMethod(bAutoDestroy));

	if (!NiagaraComponent)
	{
		++DroppedThisFrame;
		++TotalDropped;
		return nullptr;
	}

	NiagaraComponent->SetVariableLinearColor(UserTintParam, Tint);
	NiagaraComponent->SetVariableLinearColor(UserColorParam, Tint);
	NiagaraComponent->SetVariableVec2(UserSpriteSizeParam, SpriteSize);

	++EmittedThisFrame;
	++TotalEmitted;
	return NiagaraComponent;
}

UNiagaraSystem* UT66PixelVFXSubsystem::GetDefaultPixelSystem()
{
	if (CachedPixelSystem)
	{
		return CachedPixelSystem;
	}
	if (CachedLegacyPixelSystem)
	{
		return CachedLegacyPixelSystem;
	}

	CachedPixelSystem = T66_ResolveNiagaraSystem(PixelSystemPath);
	CachedLegacyPixelSystem = T66_ResolveNiagaraSystem(LegacyPixelSystemPath);
	if (!CachedPixelSystem && !CachedLegacyPixelSystem)
	{
		RequestDefaultPixelSystemsAsync();
	}

	return CachedPixelSystem ? CachedPixelSystem : CachedLegacyPixelSystem;
}

void UT66PixelVFXSubsystem::RequestDefaultPixelSystemsAsync()
{
	if (!CachedPixelSystem)
	{
		CachedPixelSystem = T66_ResolveNiagaraSystem(PixelSystemPath);
	}
	if (!CachedLegacyPixelSystem)
	{
		CachedLegacyPixelSystem = T66_ResolveNiagaraSystem(LegacyPixelSystemPath);
	}

	if ((CachedPixelSystem && CachedLegacyPixelSystem) || DefaultPixelSystemsLoadHandle.IsValid())
	{
		return;
	}

	TArray<FSoftObjectPath> AssetPaths;
	if (!CachedPixelSystem)
	{
		AssetPaths.Add(PixelSystemPath);
	}
	if (!CachedLegacyPixelSystem)
	{
		AssetPaths.Add(LegacyPixelSystemPath);
	}
	if (AssetPaths.Num() <= 0)
	{
		return;
	}

	DefaultPixelSystemsLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		AssetPaths,
		FStreamableDelegate::CreateUObject(this, &UT66PixelVFXSubsystem::HandleDefaultPixelSystemsLoaded));
	if (!DefaultPixelSystemsLoadHandle.IsValid())
	{
		HandleDefaultPixelSystemsLoaded();
	}
}

void UT66PixelVFXSubsystem::HandleDefaultPixelSystemsLoaded()
{
	DefaultPixelSystemsLoadHandle.Reset();
	CachedPixelSystem = T66_ResolveNiagaraSystem(PixelSystemPath);
	CachedLegacyPixelSystem = T66_ResolveNiagaraSystem(LegacyPixelSystemPath);
}

void UT66PixelVFXSubsystem::ResetFrameBudgetIfNeeded()
{
	if (LastBudgetFrame != GFrameCounter)
	{
		LastBudgetFrame = GFrameCounter;
		RequestedThisFrame = 0;
		EmittedThisFrame = 0;
		DroppedThisFrame = 0;
	}
}

bool UT66PixelVFXSubsystem::ConsumeBudget(ET66PixelVFXPriority Priority)
{
	ResetFrameBudgetIfNeeded();

	++RequestedThisFrame;
	++TotalRequested;

	int32 BudgetLimit = 0;
	switch (Priority)
	{
	case ET66PixelVFXPriority::Low:
		BudgetLimit = T66_ScalePixelBudget(CVarPixelVFXLowBudget.GetValueOnGameThread(), Priority);
		break;
	case ET66PixelVFXPriority::Medium:
		BudgetLimit = T66_ScalePixelBudget(CVarPixelVFXMediumBudget.GetValueOnGameThread(), Priority);
		break;
	case ET66PixelVFXPriority::High:
		BudgetLimit = T66_ScalePixelBudget(CVarPixelVFXHighBudget.GetValueOnGameThread(), Priority);
		break;
	default:
		BudgetLimit = T66_ScalePixelBudget(CVarPixelVFXMediumBudget.GetValueOnGameThread(), ET66PixelVFXPriority::Medium);
		break;
	}

	if (BudgetLimit > 0 && EmittedThisFrame >= BudgetLimit)
	{
		++DroppedThisFrame;
		++TotalDropped;
		return false;
	}

	return true;
}
