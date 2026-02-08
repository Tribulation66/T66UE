// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PixelationSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/PostProcessVolume.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

static const TCHAR* PixelationMaterialPath = TEXT("/Game/UI/M_PixelationPostProcess.M_PixelationPostProcess");
static const FName ParamNamePixelGridSize(TEXT("PixelGridSize"));

// Level 1 = subtle (high grid), 10 = strong (low grid). Linear: 320 down to 32.
int32 UT66PixelationSubsystem::LevelToPixelGridSize(int32 Level)
{
	if (Level <= 0) return 320;
	Level = FMath::Clamp(Level, 1, 10);
	// 320, 288, 256, 224, 192, 160, 128, 96, 64, 32
	const int32 GridSizes[] = { 320, 288, 256, 224, 192, 160, 128, 96, 64, 32 };
	return GridSizes[Level - 1];
}

UMaterialInterface* UT66PixelationSubsystem::GetOrCreatePixelationMaterial()
{
	UMaterialInterface* Loaded = LoadObject<UMaterialInterface>(nullptr, PixelationMaterialPath);
	if (!Loaded)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pixelation] Failed to load material at %s — pixelation disabled."), PixelationMaterialPath);
	}
	return Loaded;
}

void UT66PixelationSubsystem::SetPixelationLevel(int32 Level)
{
	CurrentLevel = FMath::Clamp(Level, 0, 10);
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	EnsureBlendableInWorld(World);
	ApplyLevelToBlendable();
}

void UT66PixelationSubsystem::EnsureBlendableInWorld(UWorld* World)
{
	if (!World)
	{
		return;
	}
	// Reuse our volume if it's in this world
	if (PixelationVolume && PixelationVolume->GetWorld() == World)
	{
		return;
	}
	// Clear stale reference if we switched worlds
	if (PixelationVolume && PixelationVolume->GetWorld() != World)
	{
		PixelationVolume = nullptr;
		PixelationDMI = nullptr;
	}

	// Find existing unbound volume to add our blendable to (avoids extra volume)
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
			SpawnParams
		);
		if (UseVolume)
		{
			UseVolume->bUnbound = true;
		}
	}
	if (!UseVolume)
	{
		return;
	}

	PixelationVolume = UseVolume;

	UMaterialInterface* BaseMat = GetOrCreatePixelationMaterial();
	if (!BaseMat)
	{
		return;
	}

	if (!PixelationDMI)
	{
		PixelationDMI = UMaterialInstanceDynamic::Create(BaseMat, this);
	}
	if (!PixelationDMI)
	{
		return;
	}

	FPostProcessSettings& PPS = UseVolume->Settings;
	// Remove any previous entry for our DMI (by object)
	for (int32 i = PPS.WeightedBlendables.Array.Num() - 1; i >= 0; --i)
	{
		if (PPS.WeightedBlendables.Array[i].Object == PixelationDMI)
		{
			PPS.WeightedBlendables.Array.RemoveAt(i);
			break;
		}
	}
	PPS.WeightedBlendables.Array.Add(FWeightedBlendable(0.0f, PixelationDMI)); // weight set in ApplyLevelToBlendable
}

void UT66PixelationSubsystem::ApplyLevelToBlendable()
{
	if (!PixelationDMI || !PixelationVolume)
	{
		return;
	}
	const int32 GridSize = LevelToPixelGridSize(CurrentLevel);
	const float Weight = (CurrentLevel > 0) ? 1.0f : 0.0f;

	PixelationDMI->SetScalarParameterValue(ParamNamePixelGridSize, static_cast<float>(GridSize));

	// Update blendable weight in the volume
	FPostProcessSettings& PPS = PixelationVolume->Settings;
	for (FWeightedBlendable& Blend : PPS.WeightedBlendables.Array)
	{
		if (Blend.Object == PixelationDMI)
		{
			Blend.Weight = Weight;
			break;
		}
	}
}
