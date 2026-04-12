// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66WorldVisualSetup.h"

#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "Components/SkyAtmosphereComponent.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66WorldVisualSetup, Log, All);

namespace
{
	static const FName QuakeSkyTag(TEXT("T66QuakeSky"));

	template <typename TActor>
	static int32 T66DestroyActorsOfType(UWorld* World)
	{
		if (!World)
		{
			return 0;
		}

		TArray<TActor*> ActorsToDestroy;
		for (TActorIterator<TActor> It(World); It; ++It)
		{
			ActorsToDestroy.Add(*It);
		}

		for (TActor* Actor : ActorsToDestroy)
		{
			if (Actor)
			{
				Actor->Destroy();
			}
		}

		return ActorsToDestroy.Num();
	}

	static int32 T66DestroyActorsWithTag(UWorld* World, const FName Tag)
	{
		if (!World || Tag.IsNone())
		{
			return 0;
		}

		TArray<AActor*> ActorsToDestroy;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (AActor* Actor = *It; Actor && Actor->ActorHasTag(Tag))
			{
				ActorsToDestroy.Add(Actor);
			}
		}

		for (AActor* Actor : ActorsToDestroy)
		{
			if (Actor)
			{
				Actor->Destroy();
			}
		}

		return ActorsToDestroy.Num();
	}

	static int32 T66DestroyActorsWithClassName(UWorld* World, const TCHAR* ClassName)
	{
		if (!World || !ClassName || !ClassName[0])
		{
			return 0;
		}

		TArray<AActor*> ActorsToDestroy;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			const UClass* ActorClass = Actor->GetClass();
			if (ActorClass && ActorClass->GetName().Equals(ClassName))
			{
				ActorsToDestroy.Add(Actor);
			}
		}

		for (AActor* Actor : ActorsToDestroy)
		{
			if (Actor)
			{
				Actor->Destroy();
			}
		}

		return ActorsToDestroy.Num();
	}

	static APostProcessVolume* T66FindOrCreateUnboundPostProcessVolume(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			if (APostProcessVolume* Volume = *It; Volume && Volume->bUnbound)
			{
				return Volume;
			}
		}

		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			if (APostProcessVolume* Volume = *It)
			{
				Volume->bUnbound = true;
				return Volume;
			}
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APostProcessVolume* Volume = World->SpawnActor<APostProcessVolume>(
			APostProcessVolume::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams);
		if (Volume)
		{
			Volume->bUnbound = true;
		}
		return Volume;
	}

	static void T66ApplyNeutralPostProcess(APostProcessVolume* Volume)
	{
		if (!Volume)
		{
			return;
		}

		Volume->bUnbound = true;
		FPostProcessSettings& PPS = Volume->Settings;
		// The project already disables adaptive exposure globally. Forcing AEM_Manual here
		// collapsed tower gameplay to black after the lighting purge, so keep the runtime
		// volume on the project's fixed exposure baseline instead of overriding the method.
		PPS.bOverride_AutoExposureMethod = false;
		PPS.bOverride_AutoExposureBias = false;
		PPS.bOverride_AutoExposureMinBrightness = true;
		PPS.AutoExposureMinBrightness = 1.0f;
		PPS.bOverride_AutoExposureMaxBrightness = true;
		PPS.AutoExposureMaxBrightness = 1.0f;
		PPS.bOverride_AmbientOcclusionIntensity = true;
		PPS.AmbientOcclusionIntensity = 0.0f;
		PPS.bOverride_BloomIntensity = true;
		PPS.BloomIntensity = 0.0f;
		PPS.bOverride_BloomThreshold = true;
		PPS.BloomThreshold = 10.0f;
		PPS.bOverride_ColorSaturation = true;
		PPS.ColorSaturation = FVector4(0.95f, 0.95f, 0.95f, 1.0f);
	}
}

void FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(UWorld* World)
{
	if (!World)
	{
		return;
	}

	const int32 RemovedAtmospheres = T66DestroyActorsOfType<ASkyAtmosphere>(World);
	const int32 RemovedDirectionalLights = T66DestroyActorsOfType<ADirectionalLight>(World);
	const int32 RemovedSkyLights = T66DestroyActorsOfType<ASkyLight>(World);
	const int32 RemovedFogActors = T66DestroyActorsOfType<AExponentialHeightFog>(World);
	const int32 RemovedTaggedQuakeSkyActors = T66DestroyActorsWithTag(World, QuakeSkyTag);
	const int32 RemovedLegacyQuakeSkyActors = T66DestroyActorsWithClassName(World, TEXT("T66QuakeSkyActor"));
	const int32 RemovedLegacyEclipseActors = T66DestroyActorsWithClassName(World, TEXT("T66EclipseActor"));

	if (APostProcessVolume* PPVolume = T66FindOrCreateUnboundPostProcessVolume(World))
	{
		T66ApplyNeutralPostProcess(PPVolume);
	}

	const int32 RemovedActorCount = RemovedAtmospheres
		+ RemovedDirectionalLights
		+ RemovedSkyLights
		+ RemovedFogActors
		+ RemovedTaggedQuakeSkyActors
		+ RemovedLegacyQuakeSkyActors
		+ RemovedLegacyEclipseActors;

	if (RemovedActorCount > 0)
	{
		UE_LOG(
			LogT66WorldVisualSetup,
			Log,
			TEXT("[VISUAL] Removed %d legacy lighting actor(s): atmosphere=%d directional=%d skylight=%d fog=%d quakeSky=%d eclipse=%d"),
			RemovedActorCount,
			RemovedAtmospheres,
			RemovedDirectionalLights,
			RemovedSkyLights,
			RemovedFogActors,
			RemovedTaggedQuakeSkyActors + RemovedLegacyQuakeSkyActors,
			RemovedLegacyEclipseActors);
	}
}
