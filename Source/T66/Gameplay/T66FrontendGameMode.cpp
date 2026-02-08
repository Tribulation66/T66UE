// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66FrontendGameMode.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66CompanionPreviewStage.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "UI/Style/T66Style.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/PlayerController.h"

static const FName T66MoonTag(TEXT("T66Moon"));

static void SpawnFrontendLightingIfNeeded(UWorld* World)
{
	if (!World) return;

	ASkyAtmosphere* Atmosphere = nullptr;
	for (TActorIterator<ASkyAtmosphere> It(World); It; ++It) { Atmosphere = *It; break; }
	ADirectionalLight* Sun = nullptr;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It) { Sun = *It; break; }
	ASkyLight* SkyLight = nullptr;
	for (TActorIterator<ASkyLight> It(World); It; ++It) { SkyLight = *It; break; }

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (!Atmosphere)
	{
		Atmosphere = World->SpawnActor<ASkyAtmosphere>(ASkyAtmosphere::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (Atmosphere) UE_LOG(LogTemp, Log, TEXT("Frontend: Spawned SkyAtmosphere for preview (same as gameplay)"));
	}
	if (!Sun)
	{
		Sun = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector(0.f, 0.f, 1000.f), FRotator(-50.f, -45.f, 0.f), SpawnParams);
		if (Sun)
		{
			if (UDirectionalLightComponent* LC = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
			{
				LC->SetMobility(EComponentMobility::Movable);
				LC->SetIntensity(4.f);
				LC->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f));
				LC->bAtmosphereSunLight = true;
				LC->AtmosphereSunLightIndex = 0;
			}
			UE_LOG(LogTemp, Log, TEXT("Frontend: Spawned DirectionalLight for preview (same as gameplay)"));
		}
	}

	// Ensure two directional lights (sun + moon) for theme-driven day/night, same as gameplay.
	int32 DirLightCount = 0;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It) { ++DirLightCount; }
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
			Moon->Tags.Add(T66MoonTag);
			if (UDirectionalLightComponent* LC = Cast<UDirectionalLightComponent>(Moon->GetLightComponent()))
			{
				LC->SetMobility(EComponentMobility::Movable);
				LC->SetIntensity(0.f);
				LC->SetLightColor(FLinearColor(0.72f, 0.8f, 1.f));
				LC->bAtmosphereSunLight = true;
				LC->AtmosphereSunLightIndex = 1;
			}
			UE_LOG(LogTemp, Log, TEXT("Frontend: Spawned moon light for theme (same as gameplay)"));
		}
	}

	// Assign atmosphere indices and apply current theme.
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		ADirectionalLight* DirLight = *It;
		if (UDirectionalLightComponent* LC = Cast<UDirectionalLightComponent>(DirLight->GetLightComponent()))
		{
			LC->SetMobility(EComponentMobility::Movable);
			LC->bAtmosphereSunLight = true;
			LC->AtmosphereSunLightIndex = DirLight->Tags.Contains(T66MoonTag) ? 1 : 0;
		}
	}
	AT66GameMode::ApplyThemeToDirectionalLightsForWorld(World);

	if (!SkyLight)
	{
		SkyLight = World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FVector(0.f, 0.f, 500.f), FRotator::ZeroRotator, SpawnParams);
		if (SkyLight)
		{
			if (USkyLightComponent* SC = SkyLight->GetLightComponent())
			{
				SC->SetMobility(EComponentMobility::Movable);
				SC->SetIntensity(0.8f);
				SC->SetLightColor(FLinearColor::White);
				SC->RecaptureSky();
			}
			UE_LOG(LogTemp, Log, TEXT("Frontend: Spawned SkyLight for preview (same as gameplay)"));
		}
	}
	else if (SkyLight && SkyLight->GetLightComponent())
	{
		if (USkyLightComponent* SC = Cast<USkyLightComponent>(SkyLight->GetLightComponent()))
			SC->RecaptureSky();
	}

	// Exponential Height Fog (same as gameplay) for atmospheric depth.
	AExponentialHeightFog* HeightFog = nullptr;
	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It) { HeightFog = *It; break; }
	if (!HeightFog)
	{
		HeightFog = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (HeightFog)
		{
			UExponentialHeightFogComponent* FogComp = HeightFog->FindComponentByClass<UExponentialHeightFogComponent>();
			if (!FogComp) FogComp = Cast<UExponentialHeightFogComponent>(HeightFog->GetRootComponent());
			if (FogComp)
			{
				FogComp->SetFogDensity(0.015f);
				FogComp->SetFogHeightFalloff(0.2f);
				FogComp->SetFogMaxOpacity(0.6f);
				FogComp->SetStartDistance(0.f);
				FogComp->SetFogInscatteringColor(FLinearColor(0.7f, 0.75f, 0.85f));
			}
			UE_LOG(LogTemp, Log, TEXT("Frontend: Spawned ExponentialHeightFog (same as gameplay)"));
		}
	}

	// PostProcessVolume (unbound) with same exposure as gameplay so frontend matches brightness.
	APostProcessVolume* PPVolume = nullptr;
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		APostProcessVolume* P = *It;
		if (P && P->bUnbound) { PPVolume = P; break; }
	}
	if (!PPVolume)
	{
		for (TActorIterator<APostProcessVolume> It(World); It; ++It) { PPVolume = *It; break; }
	}
	if (!PPVolume)
	{
		PPVolume = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (PPVolume)
		{
			PPVolume->bUnbound = true;
			FPostProcessSettings& PPS = PPVolume->Settings;
			PPS.bOverride_AutoExposureMinBrightness = true;
			PPS.AutoExposureMinBrightness = 0.4f;
			PPS.bOverride_AutoExposureMaxBrightness = true;
			PPS.AutoExposureMaxBrightness = 1.2f;
			PPS.bOverride_ColorSaturation = true;
			PPS.ColorSaturation = FVector4(0.95f, 0.95f, 0.95f, 1.f);
			UE_LOG(LogTemp, Log, TEXT("Frontend: Spawned PostProcessVolume (same exposure as gameplay)"));
		}
	}

	// Second pass: all SkyLights to 0.8f and RecaptureSky (match gameplay).
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		ASkyLight* SL = *It;
		if (USkyLightComponent* SC = Cast<USkyLightComponent>(SL->GetLightComponent()))
		{
			SC->SetMobility(EComponentMobility::Movable);
			SC->SetIntensity(0.8f);
			SC->RecaptureSky();
		}
		if (USceneComponent* Root = SL->GetRootComponent())
			Root->SetMobility(EComponentMobility::Movable);
	}
}

AT66FrontendGameMode::AT66FrontendGameMode()
{
	// No default pawn in frontend
	DefaultPawnClass = nullptr;
}

void AT66FrontendGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Returning to frontend should reset run state (fresh menu flow).
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->ResetForNewRun();
			}
			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
			{
				T66GI->bIsStageTransition = false;
				T66GI->bForceColiseumMode = false;
			}
		}
	}

	UWorld* World = GetWorld();
	// Same sky and lights as gameplay level — the main viewport camera renders with full Lumen GI.
	SpawnFrontendLightingIfNeeded(World);

	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.AddDynamic(this, &AT66FrontendGameMode::HandleSettingsChanged);
		}
	}

	// Spawn preview stages at visible positions (the main camera will look at them directly).
	if (World)
	{
		UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance());
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Both preview stages share the same platform location — we toggle visibility
		// instead of moving the camera between two positions.
		const FVector PreviewOrigin(100000.f, 0.f, 200.f);

		bool bHasPreviewStage = false;
		for (TActorIterator<AT66HeroPreviewStage> It(World); It; ++It) { bHasPreviewStage = true; break; }
		if (!bHasPreviewStage)
		{
			World->SpawnActor<AT66HeroPreviewStage>(
				AT66HeroPreviewStage::StaticClass(),
				PreviewOrigin,
				FRotator::ZeroRotator,
				SpawnParams
			);
			UE_LOG(LogTemp, Log, TEXT("T66FrontendGameMode: Spawned HeroPreviewStage for in-world preview"));
		}

		bool bHasCompanionPreview = false;
		for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It) { bHasCompanionPreview = true; break; }
		if (!bHasCompanionPreview)
		{
			AT66CompanionPreviewStage* CompStage = World->SpawnActor<AT66CompanionPreviewStage>(
				AT66CompanionPreviewStage::StaticClass(),
				PreviewOrigin,
				FRotator::ZeroRotator,
				SpawnParams
			);
			// Start hidden — hero is shown first; companion becomes visible when its screen activates.
			if (CompStage) CompStage->SetStageVisible(false);
			UE_LOG(LogTemp, Log, TEXT("T66FrontendGameMode: Spawned CompanionPreviewStage (hidden) for in-world preview"));
		}

		// Spawn the world camera that views the preview characters.
		if (!PreviewCamera)
		{
			PreviewCamera = World->SpawnActor<ACameraActor>(
				ACameraActor::StaticClass(),
				FVector(-400.f, 0.f, 350.f), // Default: looking at hero preview area
				FRotator(-15.f, 0.f, 0.f),
				SpawnParams
			);
			if (PreviewCamera)
			{
				if (UCameraComponent* CamComp = PreviewCamera->GetCameraComponent())
				{
					CamComp->SetFieldOfView(90.f); // Match gameplay FollowCamera FOV
				}
				UE_LOG(LogTemp, Log, TEXT("T66FrontendGameMode: Spawned PreviewCamera for in-world character viewing"));
			}
		}

		// Set the player controller to look through our camera.
		if (PreviewCamera)
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				PC->SetViewTarget(PreviewCamera);
			}
		}

		// Pre-warm preview so the first time the UI opens it's already "styled" (no pop-in).
		if (T66GI)
		{
			for (TActorIterator<AT66HeroPreviewStage> It(World); It; ++It)
			{
				const FName HeroID = !T66GI->SelectedHeroID.IsNone()
					? T66GI->SelectedHeroID
					: (T66GI->GetAllHeroIDs().Num() > 0 ? T66GI->GetAllHeroIDs()[0] : NAME_None);
				if (!HeroID.IsNone())
				{
					FName SkinID = T66GI->SelectedHeroSkinID.IsNone() ? FName(TEXT("Default")) : T66GI->SelectedHeroSkinID;
					It->SetPreviewHero(HeroID, T66GI->SelectedHeroBodyType, SkinID);
				}
				break;
			}
			for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It)
			{
				It->SetPreviewCompanion(T66GI->SelectedCompanionID);
				break;
			}
		}

		// Position camera at the hero preview by default.
		PositionCameraForHeroPreview();
	}

	UE_LOG(LogTemp, Log, TEXT("T66FrontendGameMode BeginPlay - Menu level initialized (in-world preview)"));
}

void AT66FrontendGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.RemoveDynamic(this, &AT66FrontendGameMode::HandleSettingsChanged);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void AT66FrontendGameMode::HandleSettingsChanged()
{
	UWorld* World = GetWorld();
	AT66GameMode::ApplyThemeToDirectionalLightsForWorld(World);
	// No SceneCapture to refresh — the main viewport camera gets theme changes automatically via Lumen.
}

void AT66FrontendGameMode::PositionCameraForHeroPreview()
{
	if (!PreviewCamera) return;
	UWorld* World = GetWorld();
	if (!World) return;

	// Show hero, hide companion (they share the same platform location).
	for (TActorIterator<AT66HeroPreviewStage> It(World); It; ++It) { (*It)->SetStageVisible(true); break; }
	for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It) { (*It)->SetStageVisible(false); break; }

	// Position camera using the hero stage's ideal transform.
	for (TActorIterator<AT66HeroPreviewStage> It(World); It; ++It)
	{
		AT66HeroPreviewStage* Stage = *It;
		const FVector CamLoc = Stage->GetIdealCameraLocation();
		const FRotator CamRot = Stage->GetIdealCameraRotation();
		if (!CamLoc.IsNearlyZero())
		{
			PreviewCamera->SetActorLocation(CamLoc);
			PreviewCamera->SetActorRotation(CamRot);
			UE_LOG(LogTemp, Log, TEXT("T66FrontendGameMode: Camera positioned for hero preview at (%.1f,%.1f,%.1f)"),
				CamLoc.X, CamLoc.Y, CamLoc.Z);
		}
		break;
	}
}

void AT66FrontendGameMode::PositionCameraForCompanionPreview()
{
	if (!PreviewCamera) return;
	UWorld* World = GetWorld();
	if (!World) return;

	// Show companion, hide hero (they share the same platform location).
	for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It) { (*It)->SetStageVisible(true); break; }
	for (TActorIterator<AT66HeroPreviewStage> It(World); It; ++It) { (*It)->SetStageVisible(false); break; }

	// Position camera using the companion stage's ideal transform.
	for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It)
	{
		AT66CompanionPreviewStage* Stage = *It;
		const FVector CamLoc = Stage->GetIdealCameraLocation();
		const FRotator CamRot = Stage->GetIdealCameraRotation();
		if (!CamLoc.IsNearlyZero())
		{
			PreviewCamera->SetActorLocation(CamLoc);
			PreviewCamera->SetActorRotation(CamRot);
			UE_LOG(LogTemp, Log, TEXT("T66FrontendGameMode: Camera positioned for companion preview at (%.1f,%.1f,%.1f)"),
				CamLoc.X, CamLoc.Y, CamLoc.Z);
		}
		break;
	}
}
