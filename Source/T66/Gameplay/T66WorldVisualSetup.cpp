// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66WorldVisualSetup.h"

#include "Core/T66PlayerSettingsSubsystem.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66PerActorLightDirection.h"
#include "Gameplay/T66ThemeAtmosphereData.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Engine/TextureCube.h"
#include "Engine/World.h"
#include "Components/SkyAtmosphereComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66WorldVisualSetup, Log, All);

namespace
{
	static const FName QuakeSkyTag(TEXT("T66QuakeSky"));
	static const FName T66AtmosphereSparedTag(TEXT("T66_AtmosphereSpared"));
	static const FName T66NeutralPostProcessTag(TEXT("T66_NeutralPostProcess"));
	static const FName T66ThemePostProcessTag(TEXT("T66_ThemeAtmospherePostProcess"));
	struct FRegisteredToonMaterial
	{
		TWeakObjectPtr<UMeshComponent> Component;
		TWeakObjectPtr<UMaterialInstanceDynamic> MID;
		ET66ToonMaterialKind Kind = ET66ToonMaterialKind::Character;
		int32 MaterialIndex = 0;
	};

	static TArray<FRegisteredToonMaterial> GRegisteredToonMaterials;

	static FLinearColor T66LightDirectionToColor(const FVector& Direction)
	{
		const FVector Normalized = Direction.GetSafeNormal(UE_SMALL_NUMBER, FVector(-0.4f, 0.6f, -0.7f));
		return FLinearColor(Normalized.X, Normalized.Y, Normalized.Z, 0.0f);
	}

	static FVector T66ResolveCharacterLightDirection(UMeshComponent* Component, const FT66ThemeCelAtmosphere& Cel)
	{
		FVector LightDirection = Cel.LightDirection;
		if (Component)
		{
			if (const AActor* Owner = Component->GetOwner())
			{
				if (const UT66PerActorLightDirection* OverrideComponent = Owner->FindComponentByClass<UT66PerActorLightDirection>())
				{
					OverrideComponent->GetEffectiveLightDirection(LightDirection);
				}
			}
		}
		return LightDirection;
	}

	static void T66ApplyCelParametersToMID(UMeshComponent* Component, UMaterialInstanceDynamic* MID, const ET66ToonMaterialKind Kind, const FT66ThemeAtmosphereSpec& Spec)
	{
		if (!MID)
		{
			return;
		}

		const FT66ThemeCelAtmosphere& Cel = Spec.CelAtmosphere;
		if (Kind == ET66ToonMaterialKind::Outline)
		{
			MID->SetVectorParameterValue(TEXT("OutlineColor"), Cel.OutlineColor);
			MID->SetScalarParameterValue(TEXT("OutlineWidth"), Cel.OutlineWidth);
			MID->SetScalarParameterValue(TEXT("OutlineBaseWidth"), Cel.OutlineWidth);
			MID->SetScalarParameterValue(TEXT("OutlineReferenceDistance"), Cel.OutlineReferenceDistance);
			MID->SetScalarParameterValue(TEXT("OutlineReferenceFOVTanHalf"), Cel.OutlineReferenceFOVTanHalf);
			MID->SetScalarParameterValue(TEXT("OutlineDepthOffsetScalar"), Cel.OutlineDepthOffsetScalar);
			return;
		}

		MID->SetVectorParameterValue(TEXT("LightDirection"), T66LightDirectionToColor(T66ResolveCharacterLightDirection(Component, Cel)));
		MID->SetScalarParameterValue(TEXT("RampStep1"), Cel.RampStep1);
		MID->SetScalarParameterValue(TEXT("RampStep2"), Cel.RampStep2);

		if (Kind == ET66ToonMaterialKind::Environment)
		{
			MID->SetVectorParameterValue(TEXT("EnvShadeColor"), Cel.EnvShadeColor);
			MID->SetVectorParameterValue(TEXT("EnvMidtoneColor"), Cel.EnvMidtoneColor);
			MID->SetVectorParameterValue(TEXT("EnvLitColor"), Cel.EnvLitColor);
			return;
		}

		MID->SetVectorParameterValue(TEXT("ShadeColor"), Cel.ShadeColor);
		MID->SetVectorParameterValue(TEXT("MidtoneColor"), Cel.MidtoneColor);
		MID->SetVectorParameterValue(TEXT("LitColor"), Cel.LitColor);
		MID->SetVectorParameterValue(TEXT("RimColor"), Cel.RimColor);
		MID->SetScalarParameterValue(TEXT("RimPower"), Cel.RimPower);
		MID->SetScalarParameterValue(TEXT("RimStrength"), Cel.RimStrength);
	}

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

	static ASkyLight* T66FindOrCreateAtmosphereSkyLight(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		if (ASkyLight* Existing = T66FindActorWithTag<ASkyLight>(World, T66AtmosphereSparedTag))
		{
			return Existing;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASkyLight* SkyLight = World->SpawnActor<ASkyLight>(
			ASkyLight::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams);
		if (SkyLight)
		{
			SkyLight->Tags.AddUnique(T66AtmosphereSparedTag);
		}
		return SkyLight;
	}

	static void T66ApplyAtmosphereSkyLight(ASkyLight* SkyLight, const FT66ThemeAtmosphereSpec& Spec)
	{
		if (!SkyLight)
		{
			return;
		}

		SkyLight->Tags.AddUnique(T66AtmosphereSparedTag);
		if (USkyLightComponent* SkyComponent = SkyLight->GetLightComponent())
		{
			SkyComponent->SetMobility(EComponentMobility::Movable);
			SkyComponent->SourceType = SLS_SpecifiedCubemap;
			SkyComponent->Cubemap = nullptr;
			SkyComponent->SetIntensity(Spec.SkyLightIntensity);
			SkyComponent->SetLightColor(Spec.SkyLightColor);
			SkyComponent->SetLowerHemisphereColor(Spec.SkyLightColor);
			SkyComponent->SetCastShadows(false);
			SkyComponent->bRealTimeCapture = false;
		}

		UE_LOG(
			LogT66WorldVisualSetup,
			Display,
			TEXT("[ATMOSPHERE] SkyLight setup: intensity=%.3f color=(%.3f, %.3f, %.3f) %s"),
			Spec.SkyLightIntensity,
			Spec.SkyLightColor.R,
			Spec.SkyLightColor.G,
			Spec.SkyLightColor.B,
			Spec.SkyLightIntensity <= KINDA_SMALL_NUMBER ? TEXT("(decommissioned as ambient source)") : TEXT(""));
	}

	static float T66ResolveFogDensity(UWorld* World, const FT66ThemeAtmosphereSpec& Spec)
	{
		if (!World)
		{
			return Spec.FogDensity;
		}

		const UGameInstance* GameInstance = World->GetGameInstance();
		const UT66PlayerSettingsSubsystem* PlayerSettings = GameInstance
			? GameInstance->GetSubsystem<UT66PlayerSettingsSubsystem>()
			: nullptr;
		if (!PlayerSettings)
		{
			return Spec.FogDensity;
		}

		if (!PlayerSettings->GetFogEnabled())
		{
			return 0.0f;
		}

		return Spec.FogDensity * (PlayerSettings->GetFogIntensityPercent() * 0.01f);
	}

	static AExponentialHeightFog* T66FindOrCreateAtmosphereFog(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		if (AExponentialHeightFog* Existing = T66FindActorWithTag<AExponentialHeightFog>(World, T66AtmosphereSparedTag))
		{
			return Existing;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(
			AExponentialHeightFog::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams);
		if (Fog)
		{
			Fog->Tags.AddUnique(T66AtmosphereSparedTag);
		}
		return Fog;
	}

	static void T66ApplyAtmosphereFog(UWorld* World, AExponentialHeightFog* Fog, const FT66ThemeAtmosphereSpec& Spec)
	{
		if (!Fog)
		{
			return;
		}

		Fog->Tags.AddUnique(T66AtmosphereSparedTag);
		if (UExponentialHeightFogComponent* FogComponent = Fog->GetComponent())
		{
			FogComponent->SetMobility(EComponentMobility::Movable);
			FogComponent->SetFogDensity(T66ResolveFogDensity(World, Spec));
			FogComponent->SetFogHeightFalloff(Spec.FogHeightFalloff);
			FogComponent->SetFogInscatteringColor(Spec.FogInscatteringColor);
			FogComponent->SetStartDistance(Spec.FogStartDistance);
			FogComponent->SetFogCutoffDistance(Spec.FogCutoffDistance);
			FogComponent->SetVolumetricFog(false);
		}
	}

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

	static void T66ApplyThemePostProcess(APostProcessVolume* Volume, const FT66ThemeAtmosphereSpec& Spec)
	{
		if (!Volume)
		{
			return;
		}

		Volume->Tags.AddUnique(T66AtmosphereSparedTag);
		Volume->Tags.AddUnique(T66ThemePostProcessTag);
		Volume->bUnbound = true;
		Volume->Priority = 1000.0f;
		Volume->BlendWeight = 1.0f;

		FPostProcessSettings& PPS = Volume->Settings;
		PPS.bOverride_ColorGainShadows = true;
		PPS.ColorGainShadows = Spec.ColorGradeShadowsTint;
		PPS.bOverride_ColorGainMidtones = true;
		PPS.ColorGainMidtones = Spec.ColorGradeMidtonesTint;
		PPS.bOverride_ColorGainHighlights = true;
		PPS.ColorGainHighlights = Spec.ColorGradeHighlightsTint;
		PPS.bOverride_ColorSaturation = true;
		PPS.ColorSaturation = Spec.ColorGradeSaturation;
		PPS.bOverride_ColorContrast = true;
		PPS.ColorContrast = Spec.ColorGradeContrast;
		PPS.bOverride_ColorGain = true;
		PPS.ColorGain = Spec.ColorGradeGain;

		// Ambient cubemap: primary ambient source for stylized indoor scenes.
		UTextureCube* LoadedCubemap = Spec.AmbientCubemap.LoadSynchronous();
		PPS.AmbientCubemap = LoadedCubemap;
		PPS.AmbientCubemapIntensity = Spec.AmbientCubemapIntensity;
		PPS.bOverride_AmbientCubemapIntensity = true;
		PPS.AmbientCubemapTint = Spec.AmbientCubemapTint;
		PPS.bOverride_AmbientCubemapTint = true;

		const FString AppliedCubemapPath = PPS.AmbientCubemap
			? PPS.AmbientCubemap->GetPathName()
			: FString(TEXT("None"));
		UE_LOG(
			LogT66WorldVisualSetup,
			Display,
			TEXT("[ATMOSPHERE] Ambient cubemap setup: path=%s loaded=%s intensity=%.2f tint=(%.3f, %.3f, %.3f) volume=%s applied=%s"),
			*Spec.AmbientCubemap.ToSoftObjectPath().ToString(),
			LoadedCubemap ? TEXT("yes") : TEXT("no"),
			Spec.AmbientCubemapIntensity,
			Spec.AmbientCubemapTint.R,
			Spec.AmbientCubemapTint.G,
			Spec.AmbientCubemapTint.B,
			*Volume->GetName(),
			*AppliedCubemapPath);
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
	const int32 RemovedSkyLights = T66DestroyActorsOfTypeExceptTagged<ASkyLight>(World, T66AtmosphereSparedTag);
	const int32 RemovedFogActors = T66DestroyActorsOfTypeExceptTagged<AExponentialHeightFog>(World, T66AtmosphereSparedTag);
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

void FT66WorldVisualSetup::EnsureAtmosphereSkyLightForWorld(UWorld* World)
{
	if (!World)
	{
		return;
	}

	const FT66ThemeAtmosphereSpec& Spec = T66ThemeAtmosphereData::GetSpecForTheme(T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon);
	T66ApplyAtmosphereSkyLight(T66FindOrCreateAtmosphereSkyLight(World), Spec);
}

void FT66WorldVisualSetup::EnsureAtmosphereForWorld(UWorld* World, const T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme)
{
	if (!World)
	{
		return;
	}

	const FT66ThemeAtmosphereSpec& Spec = T66ThemeAtmosphereData::GetSpecForTheme(Theme);
	T66ApplyAtmosphereSkyLight(T66FindOrCreateAtmosphereSkyLight(World), Spec);
	T66ApplyAtmosphereFog(World, T66FindOrCreateAtmosphereFog(World), Spec);
	T66ApplyThemePostProcess(T66FindOrCreateThemePostProcessVolume(World), Spec);
	ApplyAtmosphereToHeroCarryLights(World, Spec);
	ApplyToonCelAtmosphereToRegisteredMaterials(Theme);
}

void FT66WorldVisualSetup::ApplyAtmosphereToHeroCarryLights(UWorld* World, const FT66ThemeAtmosphereSpec& Spec)
{
	if (!World)
	{
		return;
	}

	int32 HeroCount = 0;
	int32 AppliedCount = 0;
	for (TActorIterator<AT66HeroBase> It(World); It; ++It)
	{
		AT66HeroBase* Hero = *It;
		if (!Hero)
		{
			continue;
		}

		++HeroCount;
		if (UPointLightComponent* Carry = Hero->CarryLight)
		{
			Carry->SetIntensity(Spec.CarryLightIntensity);
			Carry->SetAttenuationRadius(Spec.CarryLightAttenuationRadius);
			Carry->SetLightColor(Spec.CarryLightColor);
			Carry->SetLightFalloffExponent(Spec.CarryLightFalloffExponent);
			Carry->SetUseInverseSquaredFalloff(false);
			Carry->SetCastShadows(false);
			Carry->SetRelativeLocation(FVector(0.0f, 0.0f, Spec.CarryLightVerticalOffset));
			++AppliedCount;
		}
	}

	UE_LOG(
		LogT66WorldVisualSetup,
		Log,
		TEXT("[ATMOSPHERE] Applied carry-light spec to %d/%d hero(es) (intensity=%.1f radius=%.1f color=%s falloff=%.2f z=%.1f)."),
		AppliedCount,
		HeroCount,
		Spec.CarryLightIntensity,
		Spec.CarryLightAttenuationRadius,
		*Spec.CarryLightColor.ToString(),
		Spec.CarryLightFalloffExponent,
		Spec.CarryLightVerticalOffset);
}

UMaterialInstanceDynamic* FT66WorldVisualSetup::RegisterToonMaterial(UMeshComponent* Component, const ET66ToonMaterialKind Kind, const int32 MaterialIndex)
{
	if (!Component || MaterialIndex < 0)
	{
		return nullptr;
	}

	UMaterialInstanceDynamic* MID = Component->CreateAndSetMaterialInstanceDynamic(MaterialIndex);
	if (!MID)
	{
		return nullptr;
	}

	GRegisteredToonMaterials.RemoveAll([](const FRegisteredToonMaterial& Entry)
	{
		return !Entry.Component.IsValid() || Entry.Component.Get() == nullptr;
	});

	for (FRegisteredToonMaterial& Entry : GRegisteredToonMaterials)
	{
		if (Entry.Component.Get() == Component && Entry.MaterialIndex == MaterialIndex)
		{
			Entry.MID = MID;
			Entry.Kind = Kind;
			T66ApplyCelParametersToMID(Component, MID, Kind, T66ThemeAtmosphereData::GetSpecForTheme(T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon));
			return MID;
		}
	}

	FRegisteredToonMaterial Entry;
	Entry.Component = Component;
	Entry.MID = MID;
	Entry.Kind = Kind;
	Entry.MaterialIndex = MaterialIndex;
	GRegisteredToonMaterials.Add(Entry);
	T66ApplyCelParametersToMID(Component, MID, Kind, T66ThemeAtmosphereData::GetSpecForTheme(T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon));
	return MID;
}

void FT66WorldVisualSetup::UnregisterToonMaterial(UMeshComponent* Component)
{
	if (!Component)
	{
		return;
	}

	GRegisteredToonMaterials.RemoveAll([Component](const FRegisteredToonMaterial& Entry)
	{
		return !Entry.Component.IsValid() || Entry.Component.Get() == Component;
	});
}

int32 FT66WorldVisualSetup::ApplyToonCelAtmosphereToRegisteredMaterials(const T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme)
{
	const FT66ThemeAtmosphereSpec& Spec = T66ThemeAtmosphereData::GetSpecForTheme(Theme);
	GRegisteredToonMaterials.RemoveAll([](const FRegisteredToonMaterial& Entry)
	{
		return !Entry.Component.IsValid() || !Entry.MID.IsValid();
	});

	int32 AppliedCount = 0;
	for (const FRegisteredToonMaterial& Entry : GRegisteredToonMaterials)
	{
		if (UMaterialInstanceDynamic* MID = Entry.MID.Get())
		{
			T66ApplyCelParametersToMID(Entry.Component.Get(), MID, Entry.Kind, Spec);
			++AppliedCount;
		}
	}

	UE_LOG(
		LogT66WorldVisualSetup,
		Log,
		TEXT("[TOON] Applied cel atmosphere theme=%d to %d registered toon material(s)."),
		static_cast<int32>(Theme),
		AppliedCount);
	return AppliedCount;
}

int32 FT66WorldVisualSetup::UpdateToonOutlineViewParameters(UWorld* World)
{
	GRegisteredToonMaterials.RemoveAll([](const FRegisteredToonMaterial& Entry)
	{
		return !Entry.Component.IsValid() || !Entry.MID.IsValid();
	});

	float FOVDegrees = 90.0f;
	bool bResolvedCameraFOV = false;
	if (World)
	{
		if (const APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			if (const APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
			{
				FOVDegrees = CameraManager->GetFOVAngle();
				bResolvedCameraFOV = true;
			}
		}
	}

	static bool bLoggedFallbackFOV = false;
	if (!bResolvedCameraFOV && !bLoggedFallbackFOV)
	{
		bLoggedFallbackFOV = true;
		UE_LOG(LogT66WorldVisualSetup, Warning, TEXT("[TOON] Could not resolve active camera FOV for outline material parameters; using 90 degree fallback."));
	}

	const float ClampedFOV = FMath::Clamp(FOVDegrees, 5.0f, 170.0f);
	const float OutlineFOVTanHalf = FMath::Tan(FMath::DegreesToRadians(ClampedFOV) * 0.5f);

	int32 AppliedCount = 0;
	for (const FRegisteredToonMaterial& Entry : GRegisteredToonMaterials)
	{
		if (Entry.Kind != ET66ToonMaterialKind::Outline)
		{
			continue;
		}

		if (UMaterialInstanceDynamic* MID = Entry.MID.Get())
		{
			MID->SetScalarParameterValue(TEXT("OutlineFOVTanHalf"), OutlineFOVTanHalf);
			++AppliedCount;
		}
	}

	return AppliedCount;
}
APostProcessVolume* FT66WorldVisualSetup::FindOrCreateRuntimePostProcessVolume(UWorld* World)
{
	return T66FindOrCreateUnboundPostProcessVolume(World);
}
