// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66FrontendGameMode.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66CompanionPreviewStage.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66RetroFXSubsystem.h"
#include "UI/Style/T66Style.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66FrontendGameMode, Log, All);

AT66FrontendGameMode::AT66FrontendGameMode()
{
	// No default pawn in frontend
	DefaultPawnClass = nullptr;
	PlayerStateClass = AT66SessionPlayerState::StaticClass();
	bUseSeamlessTravel = true;
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
			if (UT66DamageLogSubsystem* DamageLog = GI->GetSubsystem<UT66DamageLogSubsystem>())
			{
				DamageLog->ResetForNewRun();
			}
			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
			{
				T66GI->bIsStageTransition = false;
				T66GI->bForceColiseumMode = false;
				T66GI->ColiseumFlowMode = ET66ColiseumFlowMode::None;
			}
		}
	}

	UWorld* World = GetWorld();
	// Match gameplay lighting so frontend previews render under the same ambient setup.
	AT66GameMode::EnsureSharedLightingForWorld(World);

	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.AddDynamic(this, &AT66FrontendGameMode::HandleSettingsChanged);
		}
	}
	HandleSettingsChanged();

	// Spawn preview stages at visible positions (the main camera will look at them directly).
	if (World)
	{
		UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance());
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Both preview stages share the same platform location; we toggle visibility
		// instead of moving the camera between two positions.
		const FVector PreviewOrigin(100000.f, 0.f, 0.f);

		bool bHasPreviewStage = false;
		for (TActorIterator<AT66HeroPreviewStage> It(World); It; ++It)
		{
			bHasPreviewStage = true;
			It->SetActorLocation(PreviewOrigin);
			It->ResetFramingCache();
			break;
		}
		if (!bHasPreviewStage)
		{
			World->SpawnActor<AT66HeroPreviewStage>(
				AT66HeroPreviewStage::StaticClass(),
				PreviewOrigin,
				FRotator::ZeroRotator,
				SpawnParams
			);
			UE_LOG(LogT66FrontendGameMode, Log, TEXT("T66FrontendGameMode: Spawned HeroPreviewStage for in-world preview"));
		}

		bool bHasCompanionPreview = false;
		for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It)
		{
			bHasCompanionPreview = true;
			It->SetActorLocation(PreviewOrigin);
			It->ResetFramingCache();
			break;
		}
		if (!bHasCompanionPreview)
		{
			AT66CompanionPreviewStage* CompStage = World->SpawnActor<AT66CompanionPreviewStage>(
				AT66CompanionPreviewStage::StaticClass(),
				PreviewOrigin,
				FRotator::ZeroRotator,
				SpawnParams
			);
			// Start hidden; the hero preview is shown first.
			if (CompStage) CompStage->SetStageVisible(false);
			UE_LOG(LogT66FrontendGameMode, Log, TEXT("T66FrontendGameMode: Spawned CompanionPreviewStage (hidden) for in-world preview"));
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
					CamComp->bConstrainAspectRatio = false; // Game view fills entire screen (no letterbox bars)
				}
				UE_LOG(LogT66FrontendGameMode, Log, TEXT("T66FrontendGameMode: Spawned PreviewCamera for in-world character viewing"));
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

	UE_LOG(LogT66FrontendGameMode, Log, TEXT("T66FrontendGameMode BeginPlay - Menu level initialized (in-world preview)"));
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
	AT66GameMode::ApplyThemeToAtmosphereAndLightingForWorld(World);

	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		if (UT66RetroFXSubsystem* RetroFX = GI->GetSubsystem<UT66RetroFXSubsystem>())
		{
			RetroFX->ApplyCurrentSettings(World);
		}
	}
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
			UE_LOG(LogT66FrontendGameMode, Log, TEXT("T66FrontendGameMode: Camera positioned for hero preview at (%.1f,%.1f,%.1f)"),
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
			UE_LOG(LogT66FrontendGameMode, Log, TEXT("T66FrontendGameMode: Camera positioned for companion preview at (%.1f,%.1f,%.1f)"),
				CamLoc.X, CamLoc.Y, CamLoc.Z);
		}
		break;
	}
}


