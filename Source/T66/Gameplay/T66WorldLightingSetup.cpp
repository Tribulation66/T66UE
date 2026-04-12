// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66WorldLightingSetup.h"

#include "Core/T66DungeonVisionSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66PosterizeSubsystem.h"
#include "Core/T66RetroFXSubsystem.h"
#include "Core/T66RetroFXSettings.h"
#include "Gameplay/T66EclipseActor.h"
#include "Gameplay/T66MainMapTerrain.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66WorldLightingSetup, Log, All);

namespace
{
	static bool T66UsesMainMapTerrainStage(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		return !MapName.Contains(TEXT("Coliseum"))
			&& !MapName.Contains(TEXT("Tutorial"))
			&& !MapName.Contains(TEXT("Lab"));
	}

	static bool T66IsDungeonLightingPreset(const UWorld* World)
	{
		return UT66GameInstance::GetEffectiveLightingPreset(World) == ET66LightingPreset::Dungeon;
	}

	static int32 T66DestroyEclipseActors(UWorld* World)
	{
		if (!World)
		{
			return 0;
		}

		TArray<AT66EclipseActor*> EclipseActorsToDestroy;
		for (TActorIterator<AT66EclipseActor> It(World); It; ++It)
		{
			EclipseActorsToDestroy.Add(*It);
		}

		for (AT66EclipseActor* EclipseActor : EclipseActorsToDestroy)
		{
			if (EclipseActor)
			{
				EclipseActor->Destroy();
			}
		}

		return EclipseActorsToDestroy.Num();
	}
}

void FT66WorldLightingSetup::EnsureSharedLightingForWorld(UWorld* World)
{
	if (!World)
	{
		return;
	}

	static const FName MoonTag(TEXT("T66Moon"));

	ASkyAtmosphere* Atmosphere = nullptr;
	for (TActorIterator<ASkyAtmosphere> It(World); It; ++It)
	{
		Atmosphere = *It;
		break;
	}

	ADirectionalLight* SunForAtmosphere = nullptr;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		SunForAtmosphere = *It;
		break;
	}

	ASkyLight* SkyForCapture = nullptr;
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		SkyForCapture = *It;
		break;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (!Atmosphere)
	{
		UE_LOG(LogT66WorldLightingSetup, Verbose, TEXT("WorldLighting: no SkyAtmosphere found; spawning runtime sky atmosphere"));
		ASkyAtmosphere* SpawnedAtmosphere = World->SpawnActor<ASkyAtmosphere>(
			ASkyAtmosphere::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (SpawnedAtmosphere)
		{
#if WITH_EDITOR
			SpawnedAtmosphere->SetActorLabel(TEXT("T66_AutoSkyAtmosphere"));
#endif
			Atmosphere = SpawnedAtmosphere;
		}
	}

	if (!SunForAtmosphere)
	{
		UE_LOG(LogT66WorldLightingSetup, Verbose, TEXT("WorldLighting: no directional light found; spawning runtime sun"));

		ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(
			ADirectionalLight::StaticClass(),
			FVector(0.f, 0.f, 1000.f),
			FRotator(-50.f, -45.f, 0.f),
			SpawnParams
		);

		if (Sun)
		{
			if (UDirectionalLightComponent* LightComp = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
			{
				LightComp->SetMobility(EComponentMobility::Movable);
				LightComp->SetIntensity(3.f);
				LightComp->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f));
				LightComp->CastShadows = false;
				LightComp->bAtmosphereSunLight = true;
				LightComp->AtmosphereSunLightIndex = 0;
				LightComp->SetForwardShadingPriority(1);
			}
#if WITH_EDITOR
			Sun->SetActorLabel(TEXT("T66_AutoSun"));
#endif
			SunForAtmosphere = Sun;
			UE_LOG(LogT66WorldLightingSetup, Verbose, TEXT("WorldLighting: spawned runtime directional light"));
		}
	}

	int32 DirLightCount = 0;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		++DirLightCount;
	}

	if (DirLightCount < 2)
	{
		ADirectionalLight* Moon = World->SpawnActor<ADirectionalLight>(
			ADirectionalLight::StaticClass(),
			FVector(0.f, 0.f, 1000.f),
			FRotator(50.f, 135.f, 0.f),
			SpawnParams
		);
		if (Moon)
		{
			Moon->Tags.Add(MoonTag);
			if (UDirectionalLightComponent* LC = Cast<UDirectionalLightComponent>(Moon->GetLightComponent()))
			{
				LC->SetMobility(EComponentMobility::Movable);
				LC->SetIntensity(0.f);
				LC->SetLightColor(FLinearColor(0.72f, 0.8f, 1.f));
				LC->CastShadows = false;
				LC->bAtmosphereSunLight = true;
				LC->AtmosphereSunLightIndex = 1;
				LC->SetForwardShadingPriority(0);
			}
#if WITH_EDITOR
			Moon->SetActorLabel(TEXT("T66_AutoMoon"));
#endif
			UE_LOG(LogT66WorldLightingSetup, Verbose, TEXT("WorldLighting: spawned runtime moon light"));
		}
	}

	if (!SkyForCapture)
	{
		UE_LOG(LogT66WorldLightingSetup, Verbose, TEXT("WorldLighting: no sky light found; spawning runtime ambient light"));

		ASkyLight* Sky = World->SpawnActor<ASkyLight>(
			ASkyLight::StaticClass(),
			FVector(0.f, 0.f, 500.f),
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Sky)
		{
#if WITH_EDITOR
			Sky->SetActorLabel(TEXT("T66_AutoSkyLight"));
#endif
			SkyForCapture = Sky;
			UE_LOG(LogT66WorldLightingSetup, Verbose, TEXT("WorldLighting: spawned runtime sky light"));
		}
	}

	AExponentialHeightFog* HeightFog = nullptr;
	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		HeightFog = *It;
		break;
	}

	if (!HeightFog)
	{
		UE_LOG(LogT66WorldLightingSetup, Verbose, TEXT("WorldLighting: no ExponentialHeightFog found; spawning runtime fog"));
		AExponentialHeightFog* SpawnedFog = World->SpawnActor<AExponentialHeightFog>(
			AExponentialHeightFog::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);
		if (SpawnedFog)
		{
#if WITH_EDITOR
			SpawnedFog->SetActorLabel(TEXT("T66_AutoExponentialHeightFog"));
#endif
			HeightFog = SpawnedFog;
		}
	}

	if (HeightFog)
	{
		ConfigureGameplayFogForWorld(World);
	}

	APostProcessVolume* PPVolume = nullptr;
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		APostProcessVolume* P = *It;
		if (P && P->bUnbound)
		{
			PPVolume = P;
			break;
		}
	}

	if (!PPVolume)
	{
		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			PPVolume = *It;
			break;
		}
	}

	if (!PPVolume)
	{
		UE_LOG(LogT66WorldLightingSetup, Verbose, TEXT("WorldLighting: no PostProcessVolume found; spawning runtime post process"));
		APostProcessVolume* SpawnedPP = World->SpawnActor<APostProcessVolume>(
			APostProcessVolume::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);
		if (SpawnedPP)
		{
			SpawnedPP->bUnbound = true;
			FPostProcessSettings& PPS = SpawnedPP->Settings;
			PPS.bOverride_AutoExposureMinBrightness = true;
			PPS.AutoExposureMinBrightness = 1.0f;
			PPS.bOverride_AutoExposureMaxBrightness = true;
			PPS.AutoExposureMaxBrightness = 1.0f;
			PPS.bOverride_AmbientOcclusionIntensity = true;
			PPS.AmbientOcclusionIntensity = 0.0f;
			PPS.bOverride_ColorSaturation = true;
			PPS.ColorSaturation = FVector4(0.95f, 0.95f, 0.95f, 1.f);
#if WITH_EDITOR
			SpawnedPP->SetActorLabel(TEXT("T66_AutoPostProcessVolume"));
#endif
		}
	}
	else
	{
		PPVolume->bUnbound = true;
		FPostProcessSettings& PPS = PPVolume->Settings;
		PPS.bOverride_AutoExposureMinBrightness = true;
		PPS.AutoExposureMinBrightness = 1.0f;
		PPS.bOverride_AutoExposureMaxBrightness = true;
		PPS.AutoExposureMaxBrightness = 1.0f;
		PPS.bOverride_AmbientOcclusionIntensity = true;
		PPS.AmbientOcclusionIntensity = 0.0f;
		PPS.bOverride_ColorSaturation = true;
		PPS.ColorSaturation = FVector4(0.95f, 0.95f, 0.95f, 1.f);
	}

	for (TActorIterator<ASkyAtmosphere> It(World); It; ++It)
	{
		It->SetActorHiddenInGame(false);
		if (USkyAtmosphereComponent* AtmosphereComponent = It->FindComponentByClass<USkyAtmosphereComponent>())
		{
			AtmosphereComponent->SetVisibility(true);
		}
	}

	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		ADirectionalLight* DirLight = *It;
		DirLight->SetActorHiddenInGame(false);
		if (UDirectionalLightComponent* LC = Cast<UDirectionalLightComponent>(DirLight->GetLightComponent()))
		{
			LC->SetMobility(EComponentMobility::Movable);
			LC->CastShadows = false;
			LC->bAtmosphereSunLight = true;
			LC->AtmosphereSunLightIndex = DirLight->Tags.Contains(MoonTag) ? 1 : 0;
			LC->SetForwardShadingPriority(DirLight->Tags.Contains(MoonTag) ? 0 : 1);
			LC->SetVisibility(true);
			if (DirLight->Tags.Contains(MoonTag))
			{
				LC->SetLightColor(FLinearColor(0.72f, 0.8f, 1.f));
			}
			else
			{
				LC->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f));
				LC->SetIntensity(3.f);
			}
		}
		if (USceneComponent* Root = DirLight->GetRootComponent())
		{
			Root->SetMobility(EComponentMobility::Movable);
		}
	}

	ApplyThemeToDirectionalLightsForWorld(World);

	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		It->SetActorHiddenInGame(false);
		if (USkyLightComponent* SC = Cast<USkyLightComponent>(It->GetLightComponent()))
		{
			SC->SetMobility(EComponentMobility::Movable);
			SC->SetRealTimeCapture(false);
			SC->SourceType = ESkyLightSourceType::SLS_CapturedScene;
			SC->Cubemap = nullptr;
			SC->SetIntensity(8.0f);
			SC->SetVisibility(true);
			SC->bLowerHemisphereIsBlack = false;
			SC->SetLowerHemisphereColor(FLinearColor(0.95f, 0.95f, 0.95f));
			SC->SetLightColor(FLinearColor::White);
			SC->RecaptureSky();
			UE_LOG(LogT66WorldLightingSetup, Log, TEXT("[LIGHT] SkyLight using captured-scene ambient (intensity 8.0)"));
		}
		if (USceneComponent* Root = It->GetRootComponent())
		{
			Root->SetMobility(EComponentMobility::Movable);
		}
	}

	ApplyThemeToAtmosphereAndLightingForWorld(World);
}

void FT66WorldLightingSetup::ConfigureGameplayFogForWorld(UWorld* World)
{
	if (!World)
	{
		return;
	}

	const bool bUseDungeonLighting = T66IsDungeonLightingPreset(World);
	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	const float FogIntensityPercent = PS ? PS->GetFogIntensityPercent() : 55.0f;
	const bool bFogEnabled = !PS || (PS->GetFogEnabled() && FogIntensityPercent > KINDA_SMALL_NUMBER);
	const float FogDensityScale = FMath::Clamp(FogIntensityPercent / 55.0f, 0.0f, 100.0f / 55.0f);

	struct FT66FogTuning
	{
		float BaseFogDensity = 0.0225f;
		float FogStartDistance = 1400.f;
		float FogHeightFalloff = 0.085f;
		float FogMaxOpacity = 0.96f;
		float DirectionalExponent = 10.0f;
		float DirectionalStartDistance = 2600.0f;
		float FogCutoffDistance = 0.0f;
		FLinearColor FogColor = FLinearColor(0.05f, 0.06f, 0.08f);
		FLinearColor DirectionalFogColor = FLinearColor(0.14f, 0.18f, 0.24f);
	};

	FT66FogTuning FogTuning;
	FogTuning.BaseFogDensity = 0.0105f;
	FogTuning.FogStartDistance = 6200.f;
	FogTuning.FogHeightFalloff = 0.008f;
	FogTuning.FogMaxOpacity = 1.0f;
	FogTuning.DirectionalExponent = 1.6f;
	FogTuning.DirectionalStartDistance = 6800.0f;
	FogTuning.FogColor = FLinearColor(0.0f, 0.53f, 0.60f);
	FogTuning.DirectionalFogColor = FLinearColor(0.10f, 0.64f, 0.74f);

	if (T66UsesMainMapTerrainStage(World) && T66GI)
	{
		const ET66MainMapLayoutVariant LayoutVariant = UT66GameInstance::ResolveMainMapLayoutVariant(T66GI);
		const FT66MapPreset MainMapPreset = T66MainMapTerrain::BuildPresetForDifficulty(T66GI->SelectedDifficulty, T66GI->RunSeed, LayoutVariant);
		const T66MainMapTerrain::FSettings MainMapSettings = T66MainMapTerrain::MakeSettings(MainMapPreset);
		const float ClearVisibilityDistance = MainMapSettings.CellSize * 3.5f;

		FogTuning.BaseFogDensity = 0.140f;
		FogTuning.FogStartDistance = ClearVisibilityDistance;
		FogTuning.FogHeightFalloff = 0.0012f;
		FogTuning.FogMaxOpacity = 1.0f;
		FogTuning.DirectionalExponent = 5.5f;
		FogTuning.DirectionalStartDistance = ClearVisibilityDistance + MainMapSettings.CellSize * 0.125f;
		FogTuning.FogColor = FLinearColor(0.02f, 0.48f, 0.54f);
		FogTuning.DirectionalFogColor = FLinearColor(0.03f, 0.50f, 0.56f);
	}

	const float FogDensity = FogTuning.BaseFogDensity * FogDensityScale;

	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		UExponentialHeightFogComponent* FogComp = It->FindComponentByClass<UExponentialHeightFogComponent>();
		if (!FogComp)
		{
			FogComp = Cast<UExponentialHeightFogComponent>(It->GetRootComponent());
		}
		if (!FogComp)
		{
			continue;
		}

		if (bUseDungeonLighting)
		{
			FogComp->SetStartDistance(0.0f);
			FogComp->SetFogDensity(0.0f);
			FogComp->SetFogHeightFalloff(0.0f);
			FogComp->SetFogMaxOpacity(0.0f);
			FogComp->SetFogCutoffDistance(0.0f);
			FogComp->SetFogInscatteringColor(FLinearColor::Black);
			FogComp->SetDirectionalInscatteringColor(FLinearColor::Black);
			FogComp->SetDirectionalInscatteringExponent(0.0f);
			FogComp->SetDirectionalInscatteringStartDistance(0.0f);
			FogComp->SetVisibility(false);
		}
		else
		{
			FogComp->SetStartDistance(FogTuning.FogStartDistance);
			FogComp->SetFogDensity(FogDensity);
			FogComp->SetFogHeightFalloff(FogTuning.FogHeightFalloff);
			FogComp->SetFogMaxOpacity(FogTuning.FogMaxOpacity);
			FogComp->SetFogCutoffDistance(FogTuning.FogCutoffDistance);
			FogComp->SetFogInscatteringColor(FogTuning.FogColor);
			FogComp->SetDirectionalInscatteringColor(FogTuning.DirectionalFogColor);
			FogComp->SetDirectionalInscatteringExponent(FogTuning.DirectionalExponent);
			FogComp->SetDirectionalInscatteringStartDistance(FogTuning.DirectionalStartDistance);
			FogComp->SetVisibility(bFogEnabled);
		}
		FogComp->SetSecondFogDensity(0.0f);
		FogComp->SetSecondFogHeightFalloff(0.0f);
		FogComp->SetSecondFogHeightOffset(0.0f);
		FogComp->SetVolumetricFog(false);
		break;
	}

	if (GI)
	{
		if (UT66RetroFXSubsystem* RetroFX = GI->GetSubsystem<UT66RetroFXSubsystem>())
		{
			FT66RetroFXSettings RetroSettings = PS ? PS->GetRetroFXSettings() : FT66RetroFXSettings();
			if (T66UsesMainMapTerrainStage(World))
			{
				RetroSettings.bEnableWorldGeometry = false;
				RetroSettings.WorldVertexSnapPercent = 0.0f;
				RetroSettings.WorldVertexSnapResolutionPercent = 0.0f;
				RetroSettings.WorldVertexNoisePercent = 0.0f;
				RetroSettings.WorldAffineBlendPercent = 0.0f;
				RetroSettings.WorldAffineDistance1Percent = 0.0f;
				RetroSettings.WorldAffineDistance2Percent = 0.0f;
				RetroSettings.WorldAffineDistance3Percent = 0.0f;
				RetroSettings.PS1FogPercent = 0.0f;
				RetroSettings.PS1SceneDepthFogPercent = 0.0f;
				RetroSettings.PS1FogDensityPercent = 0.0f;
			}
			else if (bUseDungeonLighting)
			{
				RetroSettings.PS1FogPercent = 0.0f;
				RetroSettings.PS1SceneDepthFogPercent = 0.0f;
				RetroSettings.PS1FogDensityPercent = 0.0f;
			}
			RetroFX->ApplySettings(RetroSettings, World);
		}
	}
}

void FT66WorldLightingSetup::ApplyThemeToDirectionalLightsForWorld(UWorld* World)
{
	if (!World)
	{
		return;
	}

	if (T66IsDungeonLightingPreset(World))
	{
		for (TActorIterator<ADirectionalLight> It(World); It; ++It)
		{
			ADirectionalLight* Light = *It;
			if (!Light)
			{
				continue;
			}

			Light->SetActorHiddenInGame(true);
			if (UDirectionalLightComponent* LightComp = Cast<UDirectionalLightComponent>(Light->GetLightComponent()))
			{
				LightComp->SetIntensity(0.0f);
				LightComp->bAtmosphereSunLight = false;
				LightComp->SetCastShadows(false);
				LightComp->SetVisibility(false);
			}
		}
		return;
	}

	static const FName MoonTag(TEXT("T66Moon"));
	ADirectionalLight* SunLight = nullptr;
	ADirectionalLight* MoonLight = nullptr;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		ADirectionalLight* Light = *It;
		if (Light->Tags.Contains(MoonTag))
		{
			MoonLight = Light;
		}
		else
		{
			SunLight = Light;
		}
	}
	if (!SunLight)
	{
		return;
	}

	UDirectionalLightComponent* SunComp = Cast<UDirectionalLightComponent>(SunLight->GetLightComponent());
	if (!SunComp)
	{
		return;
	}

	SunLight->SetActorHiddenInGame(false);
	SunComp->SetVisibility(true);
	SunComp->SetIntensity(0.f);
	SunComp->bAtmosphereSunLight = false;
	if (MoonLight)
	{
		UDirectionalLightComponent* MoonComp = Cast<UDirectionalLightComponent>(MoonLight->GetLightComponent());
		if (MoonComp)
		{
			MoonLight->SetActorHiddenInGame(false);
			MoonComp->SetVisibility(true);
			MoonComp->SetIntensity(5.0f);
			MoonComp->SetLightColor(FLinearColor(1.0f, 0.95f, 0.9f));
			MoonComp->bAtmosphereSunLight = true;
			MoonComp->AtmosphereSunLightIndex = 0;
			MoonComp->SetAtmosphereSunDiskColorScale(FLinearColor(0.f, 0.f, 0.f));
			MoonComp->SetCastShadows(false);
			MoonLight->SetActorRotation(FRotator(-50.f, 135.f, 0.f));
		}
	}
}

void FT66WorldLightingSetup::ApplyThemeToAtmosphereAndLightingForWorld(UWorld* World)
{
	if (!World)
	{
		return;
	}

	const bool bUseDungeonLighting = T66IsDungeonLightingPreset(World);

	if (bUseDungeonLighting)
	{
		for (TActorIterator<ASkyAtmosphere> It(World); It; ++It)
		{
			It->SetActorHiddenInGame(true);
			if (USkyAtmosphereComponent* Atmos = It->FindComponentByClass<USkyAtmosphereComponent>())
			{
				Atmos->SetVisibility(false);
				Atmos->RayleighScatteringScale = 0.0f;
				Atmos->MieScatteringScale = 0.0f;
				Atmos->MultiScatteringFactor = 0.0f;
				Atmos->MarkRenderStateDirty();
			}
		}

		for (TActorIterator<ASkyLight> It(World); It; ++It)
		{
			It->SetActorHiddenInGame(true);
			USkyLightComponent* SC = Cast<USkyLightComponent>(It->GetLightComponent());
			if (!SC)
			{
				continue;
			}

			SC->SetVisibility(false);
			SC->SetIntensity(0.0f);
			SC->SetLightColor(FLinearColor::Black);
			SC->bLowerHemisphereIsBlack = true;
			SC->SetLowerHemisphereColor(FLinearColor::Black);
			SC->RecaptureSky();
		}

		ConfigureGameplayFogForWorld(World);

		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			if (!It->bUnbound)
			{
				continue;
			}
			FPostProcessSettings& PPS = It->Settings;

			PPS.bOverride_ColorSaturation = true;
			PPS.ColorSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.f);
			PPS.bOverride_ColorGamma = false;
			PPS.bOverride_ColorGain = false;
			PPS.bOverride_BloomIntensity = true;
			PPS.BloomIntensity = 0.0f;
			PPS.bOverride_BloomThreshold = true;
			PPS.BloomThreshold = 10.0f;
			break;
		}

		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66PosterizeSubsystem* Post = GI->GetSubsystem<UT66PosterizeSubsystem>())
			{
				Post->SetEnabled(false);
			}
		}

		UT66DungeonVisionSubsystem::SetEnabledForWorld(World, true);

		const int32 RemovedEclipseActors = T66DestroyEclipseActors(World);
		if (RemovedEclipseActors > 0)
		{
			UE_LOG(LogT66WorldLightingSetup, Log, TEXT("[LIGHT] Removed %d eclipse actor(s) for dungeon preset."), RemovedEclipseActors);
		}
		return;
	}

	for (TActorIterator<ASkyAtmosphere> It(World); It; ++It)
	{
		It->SetActorHiddenInGame(false);
		USkyAtmosphereComponent* Atmos = It->FindComponentByClass<USkyAtmosphereComponent>();
		if (!Atmos)
		{
			continue;
		}

		Atmos->SetVisibility(true);
		Atmos->RayleighScattering = FLinearColor(0.028f, 0.005f, 0.004f);
		Atmos->RayleighScatteringScale = 0.8f;
		Atmos->MieScatteringScale = 0.01f;
		Atmos->MultiScatteringFactor = 0.5f;
		Atmos->MarkRenderStateDirty();
		break;
	}

	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		It->SetActorHiddenInGame(false);
		USkyLightComponent* SC = Cast<USkyLightComponent>(It->GetLightComponent());
		if (!SC)
		{
			continue;
		}

		SC->SetVisibility(true);
		SC->SetRealTimeCapture(false);
		SC->SourceType = ESkyLightSourceType::SLS_CapturedScene;
		SC->Cubemap = nullptr;
		SC->SetIntensity(4.0f);
		SC->SetLightColor(FLinearColor(0.25f, 0.2f, 0.25f));
		SC->bLowerHemisphereIsBlack = false;
		SC->SetLowerHemisphereColor(FLinearColor(0.12f, 0.1f, 0.12f));
		SC->RecaptureSky();
		break;
	}

	ConfigureGameplayFogForWorld(World);

	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		if (!It->bUnbound)
		{
			continue;
		}
		FPostProcessSettings& PPS = It->Settings;

		PPS.bOverride_ColorSaturation = true;
		PPS.ColorSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.f);
		PPS.bOverride_ColorGamma = false;
		PPS.bOverride_ColorGain = false;
		PPS.bOverride_BloomIntensity = true;
		PPS.BloomIntensity = 0.0f;
		PPS.bOverride_BloomThreshold = true;
		PPS.BloomThreshold = 10.0f;
		break;
	}

	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UT66PosterizeSubsystem* Post = GI->GetSubsystem<UT66PosterizeSubsystem>())
		{
			Post->SetEnabled(true);
		}
	}

	if (UT66DungeonVisionSubsystem* DungeonVision = UT66DungeonVisionSubsystem::Get(World))
	{
		DungeonVision->SetEnabled(false);
	}

	AT66EclipseActor* ExistingEclipse = nullptr;
	for (TActorIterator<AT66EclipseActor> It(World); It; ++It)
	{
		ExistingEclipse = *It;
		break;
	}

	if (!ExistingEclipse)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AT66EclipseActor>(AT66EclipseActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}
}
