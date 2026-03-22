// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PixelVFXSubsystem.h"

#include "HAL/IConsoleManager.h"
#include "Misc/CoreMisc.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"

namespace
{
	static const FName UserTintParam(TEXT("User.Tint"));
	static const FName UserColorParam(TEXT("User.Color"));
	static const FName UserSpriteSizeParam(TEXT("User.SpriteSize"));

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

	CachedPixelSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle"));
	if (!CachedPixelSystem)
	{
		CachedLegacyPixelSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1"));
	}

	return CachedPixelSystem ? CachedPixelSystem : CachedLegacyPixelSystem;
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
		BudgetLimit = CVarPixelVFXLowBudget.GetValueOnGameThread();
		break;
	case ET66PixelVFXPriority::Medium:
		BudgetLimit = CVarPixelVFXMediumBudget.GetValueOnGameThread();
		break;
	case ET66PixelVFXPriority::High:
		BudgetLimit = CVarPixelVFXHighBudget.GetValueOnGameThread();
		break;
	default:
		BudgetLimit = CVarPixelVFXMediumBudget.GetValueOnGameThread();
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
