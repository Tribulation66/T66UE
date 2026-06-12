// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66WorldVisualSetup.h"

#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/World.h"
#include "Components/SkyAtmosphereComponent.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66WorldVisualSetup, Log, All);

namespace
{
	static const FName QuakeSkyTag(TEXT("T66QuakeSky"));
	static const FName T66AtmosphereSparedTag(TEXT("T66_AtmosphereSpared"));
	static const FName T66NeutralPostProcessTag(TEXT("T66_NeutralPostProcess"));
	static const FName T66ThemePostProcessTag(TEXT("T66_ThemeAtmospherePostProcess"));

	template <typename TActor>
	static int32 T66DestroyActorsOfType(UWorld* World)
	{
		if (!World)
		{
			return 0;
		}

		// Setup-only classification: this intentionally scans the world during
		// visual bootstrap/cleanup, not during per-frame gameplay.
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

	template <typename TActor>
	static int32 T66DestroyActorsOfTypeExceptTagged(UWorld* World, const FName TagToSpare)
	{
		if (!World)
		{
			return 0;
		}

		TArray<TActor*> ActorsToDestroy;
		for (TActorIterator<TActor> It(World); It; ++It)
		{
			TActor* Actor = *It;
			if (Actor && !Actor->ActorHasTag(TagToSpare))
			{
				ActorsToDestroy.Add(Actor);
			}
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

	template <typename TActor>
	static TActor* T66FindActorWithTag(UWorld* World, const FName Tag)
	{
		if (!World || Tag.IsNone())
		{
			return nullptr;
		}

		for (TActorIterator<TActor> It(World); It; ++It)
		{
			if (TActor* Actor = *It; Actor && Actor->ActorHasTag(Tag))
			{
				return Actor;
			}
		}
		return nullptr;
	}

	static int32 T66DestroyActorsWithTag(UWorld* World, const FName Tag)
	{
		if (!World || Tag.IsNone())
		{
			return 0;
		}

		// Setup-only classification: tag cleanup is bounded to world bootstrap.
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

		// Setup-only classification: legacy class cleanup is bounded to world bootstrap.
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
			if (APostProcessVolume* Volume = *It; Volume && Volume->ActorHasTag(T66NeutralPostProcessTag))
			{
				Volume->bUnbound = true;
				return Volume;
			}
		}

		// Setup/helper lookup: callers cache their returned volume when used from
		// runtime settings paths, so this scan should not sit on a tick path.
		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			if (APostProcessVolume* Volume = *It; Volume && Volume->bUnbound && !Volume->ActorHasTag(T66ThemePostProcessTag))
			{
				Volume->Tags.AddUnique(T66NeutralPostProcessTag);
				return Volume;
			}
		}

		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			if (APostProcessVolume* Volume = *It; Volume && !Volume->ActorHasTag(T66ThemePostProcessTag))
			{
				Volume->bUnbound = true;
				Volume->Tags.AddUnique(T66NeutralPostProcessTag);
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
			Volume->Tags.AddUnique(T66NeutralPostProcessTag);
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

	// Hosts the single-rig WINNING post-process volume (the tag predates the per-theme purge;
	// kept so existing tagged volumes in live worlds are reused rather than duplicated).
	static APostProcessVolume* T66FindOrCreateThemePostProcessVolume(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		if (APostProcessVolume* Existing = T66FindActorWithTag<APostProcessVolume>(World, T66ThemePostProcessTag))
		{
			Existing->Tags.AddUnique(T66AtmosphereSparedTag);
			Existing->bUnbound = true;
			return Existing;
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
			Volume->Tags.AddUnique(T66AtmosphereSparedTag);
			Volume->Tags.AddUnique(T66ThemePostProcessTag);
			Volume->bUnbound = true;
		}
		return Volume;
	}

	// TRUE single rig: ONE soft directional KEY + ONE neutral SkyLight fill. The skylight replaces
	// the two former fill directionals (one directional also kills the engine's "multiple directional
	// lights are competing for forward shading" banner). Real-time capture keeps the fill tracking the
	// key-lit scene under the fixed manual exposure. Both tagged spared so neutral re-setup keeps them.
	static constexpr float T66_SingleRigKeyIntensity = 10.0f;     // soft key (was 9.0 with 2 fills)
	static constexpr float T66_SingleRigSkyLightIntensity = 1.6f; // the fill knob
	static void T66EnsureSingleRigDirectionalLight(UWorld* World)
	{
		if (!World)
		{
			return;
		}
		static const FName KeyTag(TEXT("T66_SingleRigKey"));
		const FRotator KeyRot(-50.f, -40.f, 0.f);
		ADirectionalLight* Key = T66FindActorWithTag<ADirectionalLight>(World, KeyTag);
		if (!Key)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Key = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector(0.f, 0.f, 2000.f), KeyRot, SpawnParams);
		}
		if (Key)
		{
			Key->Tags.AddUnique(T66AtmosphereSparedTag);
			Key->Tags.AddUnique(KeyTag);
			Key->SetMobility(EComponentMobility::Movable);
			Key->SetActorRotation(KeyRot);
			if (UDirectionalLightComponent* DLC = Cast<UDirectionalLightComponent>(Key->GetLightComponent()))
			{
				DLC->SetIntensity(T66_SingleRigKeyIntensity);
				DLC->SetLightColor(FLinearColor::White);
				DLC->LightSourceAngle = 3.0f; // soft shadows
				DLC->SetCastShadows(true);
				DLC->MarkRenderStateDirty();
			}
		}

		// Retire any old fill directionals left in reused worlds from the 3-directional era.
		for (const TCHAR* OldFill : { TEXT("T66_SingleRigFillA"), TEXT("T66_SingleRigFillB") })
		{
			if (ADirectionalLight* Fill = T66FindActorWithTag<ADirectionalLight>(World, FName(OldFill)))
			{
				Fill->Destroy();
			}
		}

		static const FName SkyTag(TEXT("T66_SingleRigSkyLight"));
		ASkyLight* Sky = T66FindActorWithTag<ASkyLight>(World, SkyTag);
		if (!Sky)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Sky = World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FVector(0.f, 0.f, 2000.f), FRotator::ZeroRotator, SpawnParams);
		}
		if (Sky)
		{
			Sky->Tags.AddUnique(T66AtmosphereSparedTag);
			Sky->Tags.AddUnique(SkyTag);
			if (USkyLightComponent* SkyComp = Sky->GetLightComponent())
			{
				SkyComp->SetMobility(EComponentMobility::Movable);
				SkyComp->SetIntensity(T66_SingleRigSkyLightIntensity);
				SkyComp->SetLightColor(FLinearColor::White);
				SkyComp->SetLowerHemisphereColor(FLinearColor::White); // even fill from below too — no harsh CG dark
				// One-shot scene captures (real-time capture wants a SkyAtmosphere and banners without
				// one): capture now, then once more after procedural geometry has built so the fill
				// matches the key-lit level rather than the half-loaded world.
				SkyComp->bRealTimeCapture = false;
				SkyComp->MarkRenderStateDirty();
				SkyComp->RecaptureSky();
				FTimerHandle RecaptureHandle;
				TWeakObjectPtr<USkyLightComponent> WeakSky(SkyComp);
				World->GetTimerManager().SetTimer(RecaptureHandle, FTimerDelegate::CreateLambda([WeakSky]()
				{
					if (USkyLightComponent* Comp = WeakSky.Get())
					{
						Comp->RecaptureSky();
					}
				}), 1.0f, false);
			}
		}

		int32 DirectionalCount = 0;
		for (TActorIterator<ADirectionalLight> It(World); It; ++It)
		{
			++DirectionalCount;
		}
		int32 SkyCount = 0;
		for (TActorIterator<ASkyLight> It(World); It; ++It)
		{
			++SkyCount;
		}
		UE_LOG(LogT66WorldVisualSetup, Display, TEXT("[SingleRig] Lights: directional=%d (expected 1) skylight=%d (expected 1) key=%.1f sky=%.1f"),
			DirectionalCount, SkyCount, T66_SingleRigKeyIntensity, T66_SingleRigSkyLightIntensity);
	}

	// THE lighting + post state, applied to every world (gameplay, hub, menus, boss floors):
	// the 3-directional soft rig plus the single winning post-process volume — fixed manual
	// exposure (AEM_Manual, the proven recipe; histogram clamps are adaptive and blow bright
	// subjects on dark scenes), neutral white/saturation, clamped bloom, no vignette. Priority
	// 3000 outranks any leftover volume (TestRoom legacy=2000, theme default=1000, neutral=0).
	static void T66ApplySingleRigToWorld(UWorld* World, APostProcessVolume* WinningVolume)
	{
		T66EnsureSingleRigDirectionalLight(World);
		if (!WinningVolume)
		{
			return;
		}
		WinningVolume->bUnbound = true;
		WinningVolume->Priority = 3000.f;
		FPostProcessSettings& PPS = WinningVolume->Settings;
		PPS.bOverride_AutoExposureMethod = true; PPS.AutoExposureMethod = AEM_Manual;
		PPS.bOverride_AutoExposureApplyPhysicalCameraExposure = true; PPS.AutoExposureApplyPhysicalCameraExposure = false;
		PPS.bOverride_AutoExposureBias = true; PPS.AutoExposureBias = 0.0f;
		PPS.bOverride_AutoExposureMinBrightness = false;
		PPS.bOverride_AutoExposureMaxBrightness = false;
		PPS.bOverride_WhiteTemp = true; PPS.WhiteTemp = 6500.f;
		PPS.bOverride_ColorSaturation = true; PPS.ColorSaturation = FVector4(1.f, 1.f, 1.f, 1.f);
		PPS.bOverride_BloomIntensity = true; PPS.BloomIntensity = 0.10f;
		PPS.bOverride_VignetteIntensity = true; PPS.VignetteIntensity = 0.0f;
		UE_LOG(LogT66WorldVisualSetup, Display,
			TEXT("[SingleRig] Winning PPV=%s priority=%.0f exposure(method=Manual bias=%.2f) bloom=%.2f whiteTemp=%.0f"),
			*WinningVolume->GetName(), WinningVolume->Priority,
			PPS.AutoExposureBias, PPS.BloomIntensity, PPS.WhiteTemp);
	}
}

void FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(UWorld* World)
{
	if (!World)
	{
		return;
	}

	const int32 RemovedAtmospheres = T66DestroyActorsOfType<ASkyAtmosphere>(World);
	const int32 RemovedDirectionalLights = T66DestroyActorsOfTypeExceptTagged<ADirectionalLight>(World, T66AtmosphereSparedTag);
	const int32 RemovedSkyLights = T66DestroyActorsOfTypeExceptTagged<ASkyLight>(World, T66AtmosphereSparedTag);
	const int32 RemovedFogActors = T66DestroyActorsOfTypeExceptTagged<AExponentialHeightFog>(World, T66AtmosphereSparedTag);
	const int32 RemovedTaggedQuakeSkyActors = T66DestroyActorsWithTag(World, QuakeSkyTag);
	const int32 RemovedLegacyQuakeSkyActors = T66DestroyActorsWithClassName(World, TEXT("T66QuakeSkyActor"));
	const int32 RemovedLegacyEclipseActors = T66DestroyActorsWithClassName(World, TEXT("T66EclipseActor"));

	if (APostProcessVolume* PPVolume = T66FindOrCreateUnboundPostProcessVolume(World))
	{
		T66ApplyNeutralPostProcess(PPVolume);
	}

	// The single rig is THE lighting path everywhere — hub, menus, and bootstrap worlds included.
	T66ApplySingleRigToWorld(World, T66FindOrCreateThemePostProcessVolume(World));

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

void FT66WorldVisualSetup::EnsureAtmosphereForWorld(UWorld* World)
{
	if (!World)
	{
		return;
	}

	// One shared bright soft rig everywhere: real soft directional key + fills, one winning PPV.
	// The per-theme atmosphere/fog/skylight/cel/torch/carry systems are deleted.
	T66ApplySingleRigToWorld(World, T66FindOrCreateThemePostProcessVolume(World));
}

APostProcessVolume* FT66WorldVisualSetup::FindOrCreateRuntimePostProcessVolume(UWorld* World)
{
	return T66FindOrCreateUnboundPostProcessVolume(World);
}
