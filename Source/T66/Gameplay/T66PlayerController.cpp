// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66CombatComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI/T66UIManager.h"
#include "UI/T66ScreenBase.h"
#include "UI/Screens/T66HeroGridScreen.h"
#include "UI/Screens/T66CompanionGridScreen.h"
#include "UI/Screens/T66SaveSlotsScreen.h"
#include "UI/Screens/T66AchievementsScreen.h"
#include "UI/Screens/T66PartyInviteModal.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66PlayerController, Log, All);
#include "UI/Screens/T66PauseMenuScreen.h"
#include "UI/Screens/T66ReportBugScreen.h"
#include "UI/Screens/T66SettingsScreen.h"
#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/Screens/T66PlayerSummaryPickerScreen.h"
#include "UI/Screens/T66ShopScreen.h"
#include "UI/Screens/T66AccountStatusScreen.h"
#include "UI/T66GameplayHUDWidget.h"
#include "UI/T66LabOverlayWidget.h"
#include "UI/T66GamblerOverlayWidget.h"
#include "UI/T66CowardicePromptWidget.h"
#include "UI/T66IdolAltarOverlayWidget.h"
#include "UI/T66LoadingScreenWidget.h"
#include "UI/T66VendorOverlayWidget.h"
#include "UI/T66CollectorOverlayWidget.h"
#include "UI/T66CrateOverlayWidget.h"
#include "Gameplay/T66FountainOfLifeInteractable.h"
#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66WheelSpinInteractable.h"
#include "Gameplay/T66CrateInteractable.h"
#include "Gameplay/T66CasinoInteractable.h"
#include "Gameplay/T66PilotableTractor.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "Gameplay/T66StageCatchUpGate.h"
#include "Gameplay/T66StageCatchUpGoldInteractable.h"
#include "Gameplay/T66StageCatchUpLootInteractable.h"
#include "Gameplay/T66TutorialPortal.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66MediaViewerSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Gameplay/T66IdolAltar.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66RecruitableCompanion.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "EngineUtils.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"
#include "Camera/CameraComponent.h"

#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66ItemPickup.h"
#include "Gameplay/T66MainMapTerrain.h"
#include "Gameplay/T66TowerMapTerrain.h"
#include "Gameplay/T66WorldVisualSetup.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66StageGate.h"
#include "Gameplay/T66CowardiceGate.h"
#include "Gameplay/T66DifficultyTotem.h"
#include "Gameplay/T66BossGroundAOE.h"
#include "Gameplay/T66HeroPlagueCloud.h"
#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Styling/CoreStyle.h"

namespace
{
	constexpr int32 T66GameplayViewTargetRetryBudget = 20;
	constexpr float T66GameplayViewTargetRetryDelaySeconds = 0.05f;
	constexpr int32 T66ClientGameplayWorldSetupRetryBudget = 60;
	constexpr float T66ClientGameplayWorldSetupRetryDelaySeconds = 0.10f;
	const FName T66MainMapTerrainVisualTag(TEXT("T66_MainMapTerrain_Visual"));
}


AT66PlayerController::AT66PlayerController()
{
	// Default to showing mouse cursor (will be hidden in gameplay mode)
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	JumpVFXNiagara = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1")));
	PixelVFXNiagara = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle")));
}


void AT66PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [SessionSubsystem]()
				{
					SessionSubsystem->SyncLocalLobbyProfile();
				}));
			}
		}
	}

	BindPartyInviteEvents();

	if (IsGameplayLevel())
	{
		if (!IsLocalController())
		{
			UE_LOG(LogT66PlayerController, Verbose, TEXT("PlayerController: Skipping gameplay UI init for non-local controller"));
			return;
		}

		SetupGameplayMode();
		GameplayViewTargetRetriesRemaining = T66GameplayViewTargetRetryBudget;
		RefreshGameplayViewTarget(true);
		bClientGameplayWorldSetupComplete = false;
		bReceivedGameplayRunSettingsFromServer = false;
		ClientGameplayWorldSetupRetriesRemaining = T66ClientGameplayWorldSetupRetryBudget;
		EnsureClientGameplayWorldSetup(true);
		SetupGameplayHUD();
		CachedJumpVFXNiagara = JumpVFXNiagara.Get();
		if (!CachedJumpVFXNiagara)
		{
			CachedJumpVFXNiagara = JumpVFXNiagara.LoadSynchronous();
		}
		CachedPixelVFXNiagara = PixelVFXNiagara.Get();
		if (!CachedPixelVFXNiagara)
		{
			CachedPixelVFXNiagara = PixelVFXNiagara.LoadSynchronous();
		}
		UWorld* World = GetWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;

		UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		if (RunState)
		{
			RunState->OnPlayerDied.AddDynamic(this, &AT66PlayerController::OnPlayerDied);
			RunState->QuickReviveChanged.AddDynamic(this, &AT66PlayerController::HandleQuickReviveStateChanged);
		}

		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI); T66GI && T66GI->bSaveSlotPreviewMode)
		{
			SetPause(true);
			EnsureGameplayUIManager();
			if (UIManager)
			{
				UIManager->ShowModal(ET66ScreenType::SavePreview);
			}

			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
			bEnableClickEvents = true;
			bEnableMouseOverEvents = true;
			if (GameplayHUDWidget)
			{
				GameplayHUDWidget->SetInteractive(true);
				GameplayHUDWidget->MarkHUDDirty();
			}
		}

		UE_LOG(LogT66PlayerController, Log, TEXT("PlayerController: Gameplay mode initialized"));
	}
	else
	{
		if (!IsLocalController())
		{
			UE_LOG(LogT66PlayerController, Verbose, TEXT("PlayerController: Skipping frontend UI init for non-local controller"));
			return;
		}

		EndHeroOneScopedUlt();

		// Frontend mode: UI input, show cursor, initialize UI
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;

		if (bAutoShowInitialScreen)
		{
			// Defer UI init by a short delay so the PIE viewport can stabilize its resolution.
			// Without this, the first frame renders at a transient viewport size, causing a brief
			// "zoomed-in" flash before snapping to the correct resolution. The delay also gives
			// GameInstance async texture preloads more time to complete, reducing sync load fallbacks.
			FTimerHandle DeferHandle;
			GetWorld()->GetTimerManager().SetTimer(DeferHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				InitializeUI();
			}), 0.1f, false);
		}
		UE_LOG(LogT66PlayerController, Log, TEXT("PlayerController: Frontend mode initialized (UI deferred)"));
	}
}

void AT66PlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();

	if (!IsGameplayLevel())
	{
		return;
	}

	GameplayViewTargetRetriesRemaining = T66GameplayViewTargetRetryBudget;
	RefreshGameplayViewTarget(true);
}

void AT66PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn || !IsGameplayLevel())
	{
		return;
	}

	FRotator DesiredRotation = InPawn->GetActorRotation();
	DesiredRotation.Roll = 0.f;
	if (DesiredRotation.Pitch > 45.f || DesiredRotation.Pitch < -45.f)
	{
		DesiredRotation.Pitch = 0.f;
	}

	SetControlRotation(DesiredRotation);
	ClientSetRotation(DesiredRotation, true);

	GameplayViewTargetRetriesRemaining = T66GameplayViewTargetRetryBudget;
	RefreshGameplayViewTarget(true);
}

void AT66PlayerController::ClientRestart_Implementation(APawn* NewPawn)
{
	Super::ClientRestart_Implementation(NewPawn);

	if (!IsGameplayLevel())
	{
		return;
	}

	GameplayViewTargetRetriesRemaining = T66GameplayViewTargetRetryBudget;
	RefreshGameplayViewTarget(true);
}

void AT66PlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (!IsGameplayLevel())
	{
		return;
	}

	GameplayViewTargetRetriesRemaining = T66GameplayViewTargetRetryBudget;
	RefreshGameplayViewTarget(true);
}


void AT66PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeathVFXTimerHandle);
		World->GetTimerManager().ClearTimer(GameplayViewTargetRetryTimerHandle);
		World->GetTimerManager().ClearTimer(ClientGameplayWorldSetupRetryTimerHandle);
	}

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		RunState->OnPlayerDied.RemoveDynamic(this, &AT66PlayerController::OnPlayerDied);
		RunState->QuickReviveChanged.RemoveDynamic(this, &AT66PlayerController::HandleQuickReviveStateChanged);
	}

	UnbindPartyInviteEvents();

	if (FrontendLaunchPolicyResponseHandle.IsValid())
	{
		if (UGameInstance* CurrentGameInstance = GetGameInstance())
		{
			if (UT66BackendSubsystem* Backend = CurrentGameInstance->GetSubsystem<UT66BackendSubsystem>())
			{
				Backend->OnClientLaunchPolicyResponse().Remove(FrontendLaunchPolicyResponseHandle);
			}
		}

		FrontendLaunchPolicyResponseHandle.Reset();
	}

	GetWorldTimerManager().ClearTimer(FrontendLaunchPolicyTimeoutTimerHandle);
	HideFrontendStartupOverlay();
	bClientGameplayWorldSetupComplete = false;
	bReceivedGameplayRunSettingsFromServer = false;

	Super::EndPlay(EndPlayReason);
}

void AT66PlayerController::RefreshGameplayViewTarget(bool bAllowRetry)
{
	if (!IsLocalController() || !IsGameplayLevel())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		if (bAllowRetry && GameplayViewTargetRetriesRemaining > 0)
		{
			--GameplayViewTargetRetriesRemaining;
			World->GetTimerManager().SetTimer(
				GameplayViewTargetRetryTimerHandle,
				FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					RefreshGameplayViewTarget(true);
				}),
				T66GameplayViewTargetRetryDelaySeconds,
				false);
		}
		return;
	}

	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(ControlledPawn))
	{
		Hero->SetPreviewMode(false);
		if (Hero->FollowCamera && !Hero->FollowCamera->IsActive())
		{
			Hero->FollowCamera->SetActive(true);
		}
	}

	World->GetTimerManager().ClearTimer(GameplayViewTargetRetryTimerHandle);
	GameplayViewTargetRetriesRemaining = 0;
	AutoManageActiveCameraTarget(ControlledPawn);

	if (GetViewTarget() != ControlledPawn)
	{
		SetViewTarget(ControlledPawn);
		UE_LOG(LogT66PlayerController, Log, TEXT("PlayerController: restored gameplay view target to %s"), *GetNameSafe(ControlledPawn));
	}

	if (World->GetNetMode() == NM_Client)
	{
		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
		{
			T66GI->HidePersistentGameplayTransitionCurtain();
		}
	}
}

bool AT66PlayerController::ApplyHostPartyRunSettingsToGameInstance() const
{
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!T66GI)
	{
		return false;
	}

	UT66SessionSubsystem* SessionSubsystem = T66GI->GetSubsystem<UT66SessionSubsystem>();
	if (!SessionSubsystem)
	{
		return false;
	}

	FT66LobbyPlayerInfo HostLobbyInfo;
	if (!SessionSubsystem->GetHostLobbyProfile(HostLobbyInfo))
	{
		return false;
	}

	if (HostLobbyInfo.RunSeed == 0)
	{
		return false;
	}

	T66GI->RunSeed = HostLobbyInfo.RunSeed;
	T66GI->SelectedDifficulty = HostLobbyInfo.LobbyDifficulty;
	T66GI->CurrentMainMapLayoutVariant = HostLobbyInfo.MainMapLayoutVariant;
	return true;
}

void AT66PlayerController::EnsureClientGameplayWorldSetup(bool bAllowRetry)
{
	if (!IsLocalController() || !IsGameplayLevel())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() != NM_Client)
	{
		return;
	}

	if (bClientGameplayWorldSetupComplete)
	{
		return;
	}

	int32 ExistingTerrainVisualCount = 0;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (AActor* Actor = *It; Actor && Actor->ActorHasTag(T66MainMapTerrainVisualTag))
		{
			++ExistingTerrainVisualCount;
		}
	}

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	const bool bHasRunSettings = T66GI && (bReceivedGameplayRunSettingsFromServer || ApplyHostPartyRunSettingsToGameInstance());
	if (!T66GI || !bHasRunSettings)
	{
		if (bAllowRetry && ClientGameplayWorldSetupRetriesRemaining > 0)
		{
			--ClientGameplayWorldSetupRetriesRemaining;
			World->GetTimerManager().SetTimer(
				ClientGameplayWorldSetupRetryTimerHandle,
				FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					EnsureClientGameplayWorldSetup(true);
				}),
				T66ClientGameplayWorldSetupRetryDelaySeconds,
				false);
		}
		else if (ClientGameplayWorldSetupRetriesRemaining <= 0)
		{
			UE_LOG(LogT66PlayerController, Warning, TEXT("PlayerController: client gameplay setup timed out waiting for host run settings."));
		}
		return;
	}

	FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(World);

	if (T66GI->CurrentMainMapLayoutVariant != ET66MainMapLayoutVariant::Tower)
	{
		World->GetTimerManager().ClearTimer(ClientGameplayWorldSetupRetryTimerHandle);
		ClientGameplayWorldSetupRetriesRemaining = 0;
		bClientGameplayWorldSetupComplete = true;
		return;
	}

	if (ExistingTerrainVisualCount >= 10)
	{
		World->GetTimerManager().ClearTimer(ClientGameplayWorldSetupRetryTimerHandle);
		ClientGameplayWorldSetupRetriesRemaining = 0;
		bClientGameplayWorldSetupComplete = true;
		RefreshGameplayViewTarget(true);
		return;
	}

	if (ExistingTerrainVisualCount > 0)
	{
		TArray<AActor*> TerrainActorsToDestroy;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (AActor* Actor = *It; Actor && Actor->ActorHasTag(T66MainMapTerrainVisualTag))
			{
				TerrainActorsToDestroy.Add(Actor);
			}
		}

		for (AActor* Actor : TerrainActorsToDestroy)
		{
			if (Actor)
			{
				Actor->Destroy();
			}
		}
	}

	const FT66MapPreset Preset = T66MainMapTerrain::BuildPresetForDifficulty(T66GI->SelectedDifficulty, T66GI->RunSeed);
	T66TowerMapTerrain::FLayout Layout;
	if (!T66TowerMapTerrain::BuildLayout(Preset, Layout))
	{
		UE_LOG(LogT66PlayerController, Warning, TEXT("PlayerController: failed to build client tower layout for run seed %d"), T66GI->RunSeed);
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	bool bCollisionReady = false;
	if (!T66TowerMapTerrain::Spawn(World, Layout, T66GI->SelectedDifficulty, SpawnParams, bCollisionReady))
	{
		UE_LOG(LogT66PlayerController, Warning, TEXT("PlayerController: failed to spawn client tower terrain for run seed %d"), T66GI->RunSeed);
		return;
	}

	UE_LOG(
		LogT66PlayerController,
		Log,
		TEXT("PlayerController: spawned client-side gameplay tower terrain (seed=%d, difficulty=%d, collisionReady=%d)"),
		T66GI->RunSeed,
		static_cast<int32>(T66GI->SelectedDifficulty),
		bCollisionReady ? 1 : 0);

	World->GetTimerManager().ClearTimer(ClientGameplayWorldSetupRetryTimerHandle);
	ClientGameplayWorldSetupRetriesRemaining = 0;
	bClientGameplayWorldSetupComplete = true;
	RefreshGameplayViewTarget(true);
}

void AT66PlayerController::ClientApplyGameplayRunSettings_Implementation(int32 InRunSeed, ET66Difficulty InDifficulty, ET66MainMapLayoutVariant InLayoutVariant)
{
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		T66GI->RunSeed = InRunSeed;
		T66GI->SelectedDifficulty = InDifficulty;
		T66GI->CurrentMainMapLayoutVariant = InLayoutVariant;
	}

	bReceivedGameplayRunSettingsFromServer = true;

	if (IsGameplayLevel())
	{
		bClientGameplayWorldSetupComplete = false;
		ClientGameplayWorldSetupRetriesRemaining = T66ClientGameplayWorldSetupRetryBudget;
		EnsureClientGameplayWorldSetup(true);
	}
}

void AT66PlayerController::PushLobbyProfileToServer(const FT66LobbyPlayerInfo& LobbyInfo)
{
	if (AT66SessionPlayerState* SessionPlayerState = GetPlayerState<AT66SessionPlayerState>())
	{
		if (HasAuthority())
		{
			SessionPlayerState->ApplyLobbyInfo(LobbyInfo);
		}
		else
		{
			ServerSubmitLobbyProfile(LobbyInfo);
		}
	}
}

void AT66PlayerController::PushPartyRunSummaryToServer(const FString& RequestKey, const FString& RunSummaryJson)
{
	if (RequestKey.IsEmpty() || RunSummaryJson.IsEmpty())
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (AT66SessionPlayerState* SessionPlayerState = GetPlayerState<AT66SessionPlayerState>())
			{
				if (HasAuthority())
				{
					SessionSubsystem->StorePartyRunSummaryForSteamId(RequestKey, SessionPlayerState->GetSteamId(), RunSummaryJson);
				}
				else
				{
					ServerSubmitPartyRunSummary(RequestKey, RunSummaryJson);
				}
			}
		}
	}
}

void AT66PlayerController::ServerSubmitLobbyProfile_Implementation(const FT66LobbyPlayerInfo& LobbyInfo)
{
	if (AT66SessionPlayerState* SessionPlayerState = GetPlayerState<AT66SessionPlayerState>())
	{
		FT66LobbyPlayerInfo SanitizedInfo = LobbyInfo;
		const FT66LobbyPlayerInfo& ExistingInfo = SessionPlayerState->GetLobbyInfo();
		if (!ExistingInfo.SteamId.IsEmpty())
		{
			SanitizedInfo.SteamId = ExistingInfo.SteamId;
		}
		if (SanitizedInfo.DisplayName.IsEmpty())
		{
			SanitizedInfo.DisplayName = ExistingInfo.DisplayName.IsEmpty() ? SessionPlayerState->GetPlayerName() : ExistingInfo.DisplayName;
		}

		// Remote clients never get to self-assign host authority.
		SanitizedInfo.bPartyHost = false;
		SessionPlayerState->ApplyLobbyInfo(SanitizedInfo);
	}
}

void AT66PlayerController::ServerSubmitPartyRunSummary_Implementation(const FString& RequestKey, const FString& RunSummaryJson)
{
	if (RequestKey.IsEmpty() || RunSummaryJson.IsEmpty())
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (AT66SessionPlayerState* SessionPlayerState = GetPlayerState<AT66SessionPlayerState>())
			{
				SessionSubsystem->StorePartyRunSummaryForSteamId(RequestKey, SessionPlayerState->GetSteamId(), RunSummaryJson);
			}
		}
	}
}

void AT66PlayerController::ServerRequestPartySaveAndReturnToFrontend_Implementation()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->SaveCurrentRunAndReturnToFrontend();
		}
	}
}

void AT66PlayerController::BindPartyInviteEvents()
{
	UnbindPartyInviteEvents();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			PendingPartyInvitesChangedHandle = Backend->OnPendingPartyInvitesChanged().AddUObject(this, &AT66PlayerController::HandlePendingPartyInvitesChanged);
			Backend->PollPendingPartyInvites(true);
		}
	}
}

void AT66PlayerController::UnbindPartyInviteEvents()
{
	if (!PendingPartyInvitesChangedHandle.IsValid())
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnPendingPartyInvitesChanged().Remove(PendingPartyInvitesChangedHandle);
		}
	}

	PendingPartyInvitesChangedHandle.Reset();
}

void AT66PlayerController::HandlePendingPartyInvitesChanged()
{
	RefreshPartyInviteModal();
}

void AT66PlayerController::RefreshPartyInviteModal()
{
	if (!UIManager)
	{
		if (IsGameplayLevel())
		{
			EnsureGameplayUIManager();
		}
		else
		{
			return;
		}
	}

	if (!UIManager)
	{
		return;
	}

	UT66BackendSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend)
	{
		return;
	}

	const bool bHasPendingInvites = Backend->HasPendingPartyInvites();
	const ET66ScreenType CurrentModalType = UIManager->GetCurrentModalType();
	if (!bHasPendingInvites)
	{
		if (CurrentModalType == ET66ScreenType::PartyInvite)
		{
			UIManager->CloseModal();
			if (IsGameplayLevel() && !IsPaused())
			{
				RestoreGameplayInputMode();
			}
		}
		return;
	}

	if (CurrentModalType == ET66ScreenType::PartyInvite)
	{
		if (UT66ScreenBase* CurrentModal = UIManager->GetCurrentModal())
		{
			CurrentModal->ForceRebuildSlate();
		}
		return;
	}

	if (CurrentModalType != ET66ScreenType::None)
	{
		UIManager->CloseModal();
	}

	if (IsGameplayLevel())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}

	UIManager->ShowModal(ET66ScreenType::PartyInvite);
}


bool AT66PlayerController::IsFrontendLevel() const
{
	if (!GetWorld()) return true; // Default to frontend if no world
	
	FString MapName = GetWorld()->GetMapName();
	// Remove any prefix like "UEDPIE_0_" from PIE sessions
	MapName = UWorld::RemovePIEPrefix(MapName);
	
	// Check if this is a frontend level (contains "Frontend" or "Menu")
	return MapName.Contains(TEXT("Frontend")) || MapName.Contains(TEXT("Menu"));
}


bool AT66PlayerController::IsGameplayLevel() const
{
	if (!GetWorld()) return false;
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
		{
			if (T66GI->bIsLabLevel) return true;
		}
	}
	FString MapName = GetWorld()->GetMapName();
	MapName = UWorld::RemovePIEPrefix(MapName);
	if (MapName.Contains(TEXT("Gameplay"))) return true;
	return false;
}


void AT66PlayerController::SetupGameplayMode()
{
	const bool bHadScopedState = bHeroOneScopedUltActive || bHeroOneScopeViewEnabled;
	EndHeroOneScopedUlt();
	RestoreGameplayInputMode();
	UE_LOG(LogT66PlayerController, Log, TEXT("PlayerController: Gameplay input mode set, cursor hidden (scoped state reset=%d)"), bHadScopedState ? 1 : 0);
}
