// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66FrontendGameMode.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66CompanionPreviewStage.h"
#include "Gameplay/T66PreviewStageEnvironment.h"
#include "Gameplay/T66WorldVisualSetup.h"
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

namespace
{
	template <typename TStage>
	TStage* T66ResolveOrSpawnPreviewStage(
		UWorld* World,
		TWeakObjectPtr<TStage>& CachedStage,
		const FVector& StageLocation,
		const FActorSpawnParameters& SpawnParams,
		bool& bOutSpawned)
	{
		bOutSpawned = false;

		if (TStage* Stage = CachedStage.Get())
		{
			return Stage;
		}

		if (!World)
		{
			return nullptr;
		}

		for (TActorIterator<TStage> It(World); It; ++It)
		{
			if (TStage* ExistingStage = *It)
			{
				CachedStage = ExistingStage;
				return ExistingStage;
			}
		}

		if (TStage* SpawnedStage = World->SpawnActor<TStage>(
			TStage::StaticClass(),
			StageLocation,
			FRotator::ZeroRotator,
			SpawnParams))
		{
			CachedStage = SpawnedStage;
			bOutSpawned = true;
			return SpawnedStage;
		}

		return nullptr;
	}
}

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
	// Match gameplay's neutral visual setup so previews are not dependent on a separate sky/light stack.
	FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(World);

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
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Both preview stages share the same platform location; we toggle visibility
		// instead of moving the camera between two positions.
		const FVector PreviewOrigin(100000.f, 0.f, 0.f);

		bool bSpawnedHeroStage = false;
		if (AT66HeroPreviewStage* HeroStage = T66ResolveOrSpawnPreviewStage(World, HeroPreviewStage, PreviewOrigin, SpawnParams, bSpawnedHeroStage))
		{
			HeroStage->SetActorLocation(PreviewOrigin);
			HeroStage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
			HeroStage->ResetFramingCache();
		}
		if (bSpawnedHeroStage)
		{
			UE_LOG(LogT66FrontendGameMode, Log, TEXT("T66FrontendGameMode: Spawned HeroPreviewStage for in-world preview"));
		}

		bool bSpawnedCompanionStage = false;
		if (AT66CompanionPreviewStage* CompStage = T66ResolveOrSpawnPreviewStage(World, CompanionPreviewStage, PreviewOrigin, SpawnParams, bSpawnedCompanionStage))
		{
			CompStage->SetActorLocation(PreviewOrigin);
			CompStage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
			CompStage->ResetFramingCache();
			if (bSpawnedCompanionStage)
			{
				CompStage->SetStageVisible(false);
			}
		}
		if (bSpawnedCompanionStage)
		{
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

	HeroPreviewStage.Reset();
	CompanionPreviewStage.Reset();
	PreviewCamera = nullptr;
	Super::EndPlay(EndPlayReason);
}

void AT66FrontendGameMode::HandleSettingsChanged()
{
	UWorld* World = GetWorld();
	FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(World);

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

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	bool bSpawnedHeroStage = false;
	AT66HeroPreviewStage* HeroStage = T66ResolveOrSpawnPreviewStage(World, HeroPreviewStage, FVector(100000.f, 0.f, 0.f), SpawnParams, bSpawnedHeroStage);
	bool bSpawnedCompanionStage = false;
	AT66CompanionPreviewStage* CompanionStage = T66ResolveOrSpawnPreviewStage(World, CompanionPreviewStage, FVector(100000.f, 0.f, 0.f), SpawnParams, bSpawnedCompanionStage);
	static_cast<void>(bSpawnedHeroStage);
	static_cast<void>(bSpawnedCompanionStage);

	// Show hero, hide companion (they share the same platform location).
	if (HeroStage)
	{
		HeroStage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		HeroStage->SetStageVisible(true);
	}
	if (CompanionStage)
	{
		CompanionStage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		CompanionStage->SetStageVisible(false);
	}

	// Position camera using the hero stage's ideal transform.
	if (HeroStage)
	{
		const FVector CamLoc = HeroStage->GetIdealCameraLocation();
		const FRotator CamRot = HeroStage->GetIdealCameraRotation();
		if (!CamLoc.IsNearlyZero())
		{
			PreviewCamera->SetActorLocation(CamLoc);
			PreviewCamera->SetActorRotation(CamRot);
			UE_LOG(LogT66FrontendGameMode, Verbose, TEXT("T66FrontendGameMode: Camera positioned for hero preview at (%.1f,%.1f,%.1f)"),
				CamLoc.X, CamLoc.Y, CamLoc.Z);
		}
	}
}

void AT66FrontendGameMode::PositionCameraForCompanionPreview()
{
	if (!PreviewCamera) return;
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	bool bSpawnedCompanionStage = false;
	AT66CompanionPreviewStage* CompanionStage = T66ResolveOrSpawnPreviewStage(World, CompanionPreviewStage, FVector(100000.f, 0.f, 0.f), SpawnParams, bSpawnedCompanionStage);
	bool bSpawnedHeroStage = false;
	AT66HeroPreviewStage* HeroStage = T66ResolveOrSpawnPreviewStage(World, HeroPreviewStage, FVector(100000.f, 0.f, 0.f), SpawnParams, bSpawnedHeroStage);
	static_cast<void>(bSpawnedCompanionStage);
	static_cast<void>(bSpawnedHeroStage);

	// Show companion, hide hero (they share the same platform location).
	if (CompanionStage)
	{
		CompanionStage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		CompanionStage->SetStageVisible(true);
	}
	if (HeroStage)
	{
		HeroStage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		HeroStage->SetStageVisible(false);
	}

	// Position camera using the companion stage's ideal transform.
	if (CompanionStage)
	{
		const FVector CamLoc = CompanionStage->GetIdealCameraLocation();
		const FRotator CamRot = CompanionStage->GetIdealCameraRotation();
		if (!CamLoc.IsNearlyZero())
		{
			PreviewCamera->SetActorLocation(CamLoc);
			PreviewCamera->SetActorRotation(CamRot);
			UE_LOG(LogT66FrontendGameMode, Log, TEXT("T66FrontendGameMode: Camera positioned for companion preview at (%.1f,%.1f,%.1f)"),
				CamLoc.X, CamLoc.Y, CamLoc.Z);
		}
	}
}


