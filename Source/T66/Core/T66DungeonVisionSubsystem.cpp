// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66DungeonVisionSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66DungeonVision, Log, All);

namespace
{
	static const TCHAR* DungeonVisionMaterialPath = TEXT("/Game/UI/M_DungeonVisionPostProcess.M_DungeonVisionPostProcess");
	static const FName ParamNamePlayerWorldPosition(TEXT("PlayerWorldPosition"));
	static const FName ParamNameVisionRadius(TEXT("VisionRadius"));
	static const FName ParamNameFalloffDistance(TEXT("FalloffDistance"));
	static const FName ParamNameAmbientVisibility(TEXT("AmbientVisibility"));

	static float GT66DungeonVisionRadius = 2200.0f;
	static FAutoConsoleVariableRef CVarT66DungeonVisionRadius(
		TEXT("t66.DungeonVisionRadius"),
		GT66DungeonVisionRadius,
		TEXT("World-space XY reveal radius for the dungeon lighting preset."),
		ECVF_Default);

	static float GT66DungeonVisionFalloff = 650.0f;
	static FAutoConsoleVariableRef CVarT66DungeonVisionFalloff(
		TEXT("t66.DungeonVisionFalloff"),
		GT66DungeonVisionFalloff,
		TEXT("World-space falloff distance outside the reveal radius for the dungeon lighting preset."),
		ECVF_Default);

	static float GT66DungeonVisionAmbientVisibility = 0.0f;
	static FAutoConsoleVariableRef CVarT66DungeonVisionAmbientVisibility(
		TEXT("t66.DungeonVisionAmbientVisibility"),
		GT66DungeonVisionAmbientVisibility,
		TEXT("Minimum visibility outside the reveal radius for the dungeon lighting preset (0..1)."),
		ECVF_Default);
}

UT66DungeonVisionSubsystem* UT66DungeonVisionSubsystem::Get(UWorld* World)
{
	return World ? World->GetSubsystem<UT66DungeonVisionSubsystem>() : nullptr;
}

void UT66DungeonVisionSubsystem::SetEnabledForWorld(UWorld* World, bool bInEnabled)
{
	if (UT66DungeonVisionSubsystem* VisionSubsystem = Get(World))
	{
		VisionSubsystem->SetEnabled(bInEnabled);
	}
}

bool UT66DungeonVisionSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UT66DungeonVisionSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bEnabled)
	{
		return;
	}

	(void)DeltaTime;

	VisionRadius = FMath::Max(0.0f, GT66DungeonVisionRadius);
	FalloffDistance = FMath::Max(1.0f, GT66DungeonVisionFalloff);
	AmbientVisibility = FMath::Clamp(GT66DungeonVisionAmbientVisibility, 0.0f, 1.0f);

	EnsureBlendableInWorld();
	UpdateTrackedOrigin();
	ApplyParametersToBlendable();
}

TStatId UT66DungeonVisionSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UT66DungeonVisionSubsystem, STATGROUP_Tickables);
}

void UT66DungeonVisionSubsystem::SetEnabled(bool bInEnabled)
{
	bEnabled = bInEnabled;
	VisionRadius = FMath::Max(0.0f, GT66DungeonVisionRadius);
	FalloffDistance = FMath::Max(1.0f, GT66DungeonVisionFalloff);
	AmbientVisibility = FMath::Clamp(GT66DungeonVisionAmbientVisibility, 0.0f, 1.0f);

	if (!bEnabled && !DungeonVisionDMI && !DungeonVisionVolume)
	{
		return;
	}

	EnsureBlendableInWorld();
	UpdateTrackedOrigin();
	ApplyParametersToBlendable();
}

UMaterialInterface* UT66DungeonVisionSubsystem::GetOrLoadDungeonVisionMaterial()
{
	UMaterialInterface* Loaded = LoadObject<UMaterialInterface>(nullptr, DungeonVisionMaterialPath);
	if (!Loaded)
	{
		UE_LOG(LogT66DungeonVision, Warning, TEXT("[DungeonVision] Failed to load material at %s - dungeon lighting mask disabled."), DungeonVisionMaterialPath);
	}
	return Loaded;
}

void UT66DungeonVisionSubsystem::EnsureBlendableInWorld()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (DungeonVisionVolume && DungeonVisionVolume->GetWorld() == World)
	{
		return;
	}

	if (DungeonVisionVolume && DungeonVisionVolume->GetWorld() != World)
	{
		DungeonVisionVolume = nullptr;
		DungeonVisionDMI = nullptr;
	}

	APostProcessVolume* UseVolume = nullptr;
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		if (It->bUnbound)
		{
			UseVolume = *It;
			break;
		}
	}

	if (!UseVolume)
	{
		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			UseVolume = *It;
			break;
		}
	}

	if (!UseVolume)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		UseVolume = World->SpawnActor<APostProcessVolume>(
			APostProcessVolume::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams);
		if (UseVolume)
		{
			UseVolume->bUnbound = true;
		}
	}

	if (!UseVolume)
	{
		return;
	}

	DungeonVisionVolume = UseVolume;

	UMaterialInterface* BaseMat = GetOrLoadDungeonVisionMaterial();
	if (!BaseMat)
	{
		return;
	}

	if (!DungeonVisionDMI)
	{
		DungeonVisionDMI = UMaterialInstanceDynamic::Create(BaseMat, this);
	}

	if (!DungeonVisionDMI)
	{
		return;
	}

	FPostProcessSettings& PPS = UseVolume->Settings;
	for (int32 Index = PPS.WeightedBlendables.Array.Num() - 1; Index >= 0; --Index)
	{
		if (PPS.WeightedBlendables.Array[Index].Object == DungeonVisionDMI)
		{
			PPS.WeightedBlendables.Array.RemoveAt(Index);
			break;
		}
	}

	PPS.WeightedBlendables.Array.Insert(FWeightedBlendable(0.0f, DungeonVisionDMI), 0);
}

void UT66DungeonVisionSubsystem::ApplyParametersToBlendable()
{
	if (!DungeonVisionDMI || !DungeonVisionVolume)
	{
		return;
	}

	const float Weight = bEnabled ? 1.0f : 0.0f;

	DungeonVisionDMI->SetVectorParameterValue(ParamNamePlayerWorldPosition, FLinearColor(TrackedOrigin.X, TrackedOrigin.Y, TrackedOrigin.Z, 0.0f));
	DungeonVisionDMI->SetScalarParameterValue(ParamNameVisionRadius, VisionRadius);
	DungeonVisionDMI->SetScalarParameterValue(ParamNameFalloffDistance, FalloffDistance);
	DungeonVisionDMI->SetScalarParameterValue(ParamNameAmbientVisibility, AmbientVisibility);

	FPostProcessSettings& PPS = DungeonVisionVolume->Settings;
	for (FWeightedBlendable& Blend : PPS.WeightedBlendables.Array)
	{
		if (Blend.Object == DungeonVisionDMI)
		{
			Blend.Weight = Weight;
			break;
		}
	}
}

void UT66DungeonVisionSubsystem::UpdateTrackedOrigin()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		TrackedOrigin = FVector::ZeroVector;
		return;
	}

	if (APawn* Pawn = PlayerController->GetPawn())
	{
		TrackedOrigin = Pawn->GetActorLocation();
		return;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	TrackedOrigin = ViewLocation;
}
