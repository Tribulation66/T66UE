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

DEFINE_LOG_CATEGORY_STATIC(LogT66PlayerInput, Log, All);
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
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66GameInstance.h"
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
	static void EnsureGameplayActionHasDefaultKeyboardBinding(
		APlayerController* PlayerController,
		const FName ActionName,
		const FKey DesiredKey)
	{
		UInputSettings* Settings = UInputSettings::GetInputSettings();
		if (!Settings || !DesiredKey.IsValid())
		{
			return;
		}

		bool bActionAlreadyHasKeyboardBinding = false;
		bool bDesiredKeyClaimedByAnotherKeyboardAction = false;

		for (const FInputActionKeyMapping& Mapping : Settings->GetActionMappings())
		{
			if (!Mapping.Key.IsValid() || Mapping.Key.IsGamepadKey() || Mapping.Key.IsMouseButton())
			{
				continue;
			}

			if (Mapping.ActionName == ActionName)
			{
				bActionAlreadyHasKeyboardBinding = true;
			}

			if (Mapping.Key == DesiredKey && Mapping.ActionName != ActionName)
			{
				bDesiredKeyClaimedByAnotherKeyboardAction = true;
			}
		}

		if (bActionAlreadyHasKeyboardBinding || bDesiredKeyClaimedByAnotherKeyboardAction)
		{
			return;
		}

		Settings->AddActionMapping(FInputActionKeyMapping(ActionName, DesiredKey));
		Settings->SaveKeyMappings();
		Settings->SaveConfig();

		if (PlayerController && PlayerController->PlayerInput)
		{
			PlayerController->PlayerInput->ForceRebuildingKeyMaps(true);
		}
	}

	static void MigrateLegacyMediaViewerTabBinding(APlayerController* PlayerController)
	{
		UInputSettings* Settings = UInputSettings::GetInputSettings();
		if (!Settings)
		{
			return;
		}

		static const FName ToggleTikTokAction(TEXT("ToggleTikTok"));
		static const FName ToggleMediaViewerAction(TEXT("ToggleMediaViewer"));
		const FKey DesiredKey = EKeys::Tab;
		const FKey LegacyKey = EKeys::CapsLock;

		bool bToggleTikTokAlreadyUsesTab = false;
		bool bLegacyCapsLockMappingPresent = false;
		bool bTabAlreadyClaimedByAnotherKeyboardAction = false;

		for (const FInputActionKeyMapping& Mapping : Settings->GetActionMappings())
		{
			if (!Mapping.Key.IsValid() || Mapping.Key.IsGamepadKey() || Mapping.Key.IsMouseButton())
			{
				continue;
			}

			if (Mapping.ActionName == ToggleTikTokAction && Mapping.Key == DesiredKey)
			{
				bToggleTikTokAlreadyUsesTab = true;
			}

			if (Mapping.ActionName == ToggleTikTokAction && Mapping.Key == LegacyKey)
			{
				bLegacyCapsLockMappingPresent = true;
			}

			if (Mapping.Key == DesiredKey
				&& Mapping.ActionName != ToggleTikTokAction
				&& Mapping.ActionName != ToggleMediaViewerAction)
			{
				bTabAlreadyClaimedByAnotherKeyboardAction = true;
			}
		}

		if (bToggleTikTokAlreadyUsesTab || !bLegacyCapsLockMappingPresent || bTabAlreadyClaimedByAnotherKeyboardAction)
		{
			return;
		}

		Settings->RemoveActionMapping(FInputActionKeyMapping(ToggleTikTokAction, LegacyKey));
		Settings->AddActionMapping(FInputActionKeyMapping(ToggleTikTokAction, DesiredKey));
		Settings->SaveKeyMappings();
		Settings->SaveConfig();

		if (PlayerController && PlayerController->PlayerInput)
		{
			PlayerController->PlayerInput->ForceRebuildingKeyMaps(true);
		}
	}

	static bool CanUsePausedMediaViewer(const AT66PlayerController* PlayerController)
	{
		if (!PlayerController)
		{
			return false;
		}

		if (!PlayerController->IsPaused())
		{
			return true;
		}

		const UT66UIManager* LocalUIManager = PlayerController->GetUIManager();
		return LocalUIManager
			&& LocalUIManager->IsModalActive()
			&& LocalUIManager->GetCurrentModalType() == ET66ScreenType::PauseMenu;
	}
}


UNiagaraSystem* AT66PlayerController::GetActiveJumpVFXSystem() const
{
	return CachedPixelVFXNiagara ? CachedPixelVFXNiagara : CachedJumpVFXNiagara;
}


void AT66PlayerController::RestoreGameplayInputMode()
{
	// Classic gameplay: mouse rotates camera, cursor hidden.
	bInventoryInspectOpen = false;
	bInventoryInspectRestoreFreeCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->SetInventoryInspectMode(false);
		GameplayHUDWidget->SetInteractive(false);
	}
}

void AT66PlayerController::SetInventoryInspectOpen(bool bOpen)
{
	if (!GameplayHUDWidget)
	{
		bInventoryInspectOpen = false;
		bInventoryInspectRestoreFreeCursor = false;
		return;
	}

	if (bOpen)
	{
		if (bInventoryInspectOpen)
		{
			return;
		}

		bInventoryInspectRestoreFreeCursor = bShowMouseCursor;
		bInventoryInspectOpen = true;
		GameplayHUDWidget->SetInventoryInspectMode(true);

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
		GameplayHUDWidget->SetInteractive(true);
		return;
	}

	if (!bInventoryInspectOpen)
	{
		return;
	}

	bInventoryInspectOpen = false;
	GameplayHUDWidget->SetInventoryInspectMode(false);

	if (bInventoryInspectRestoreFreeCursor)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
		GameplayHUDWidget->SetInteractive(true);
	}
	else
	{
		RestoreGameplayInputMode();
	}

	bInventoryInspectRestoreFreeCursor = false;
}


void AT66PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind gameplay inputs (these work when in gameplay mode)
	if (InputComponent)
	{
		// Movement (WASD)
		InputComponent->BindAxis(TEXT("MoveForward"), this, &AT66PlayerController::HandleMoveForward);
		InputComponent->BindAxis(TEXT("MoveRight"), this, &AT66PlayerController::HandleMoveRight);
		
		// Look (Mouse)
		InputComponent->BindAxis(TEXT("LookUp"), this, &AT66PlayerController::HandleLookUp);
		InputComponent->BindAxis(TEXT("Turn"), this, &AT66PlayerController::HandleTurn);
		
		// Zoom (scroll wheel, game world only)
		InputComponent->BindAxis(TEXT("Zoom"), this, &AT66PlayerController::HandleZoom);

		// Dash (default: LeftShift; configured in DefaultInput.ini)
		InputComponent->BindAction(TEXT("Dash"), IE_Pressed, this, &AT66PlayerController::HandleDashPressed);
		
		// Jump (Space)
		InputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AT66PlayerController::HandleJumpPressed);
		InputComponent->BindAction(TEXT("Jump"), IE_Released, this, &AT66PlayerController::HandleJumpReleased);

		// Escape and P (pause menu in gameplay; P works in PIE without stealing focus)
		InputComponent->BindAction(TEXT("Escape"), IE_Pressed, this, &AT66PlayerController::HandleEscapePressed);
		InputComponent->BindAction(TEXT("Pause"), IE_Pressed, this, &AT66PlayerController::HandleEscapePressed);

		// T = toggle HUD panels (inventory + minimap), F = interact (vendor / pickup)
		InputComponent->BindAction(TEXT("ToggleHUD"), IE_Pressed, this, &AT66PlayerController::HandleToggleHUDPressed);
		InputComponent->BindAction(TEXT("ToggleImmortality"), IE_Pressed, this, &AT66PlayerController::HandleToggleImmortalityPressed);
		InputComponent->BindAction(TEXT("TogglePower"), IE_Pressed, this, &AT66PlayerController::HandleTogglePowerPressed);
		InputComponent->BindAction(TEXT("ToggleTikTok"), IE_Pressed, this, &AT66PlayerController::HandleToggleTikTokPressed);
		InputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AT66PlayerController::HandleInteractPressed);
		InputComponent->BindAction(TEXT("Ultimate"), IE_Pressed, this, &AT66PlayerController::HandleUltimatePressed);
		InputComponent->BindAction(TEXT("ToggleMediaViewer"), IE_Pressed, this, &AT66PlayerController::HandleToggleMediaViewerPressed);
		InputComponent->BindAction(TEXT("OpenFullMap"), IE_Pressed, this, &AT66PlayerController::HandleOpenFullMapPressed);
		InputComponent->BindAction(TEXT("InspectInventory"), IE_Pressed, this, &AT66PlayerController::HandleInspectInventoryPressed);
		InputComponent->BindAction(TEXT("ToggleGamerMode"), IE_Pressed, this, &AT66PlayerController::HandleToggleGamerModePressed);
		InputComponent->BindAction(TEXT("RestartRun"), IE_Pressed, this, &AT66PlayerController::HandleRestartRunPressed);

		// Manual attack lock / unlock / toggle mouse lock (Enhanced Input: LMB, RMB)
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
		{
			SetupGameplayEnhancedInputMappings();
			if (IA_AttackLock) EIC->BindAction(IA_AttackLock, ETriggerEvent::Started, this, &AT66PlayerController::HandleAttackLockPressed);
			if (IA_AttackUnlock) EIC->BindAction(IA_AttackUnlock, ETriggerEvent::Started, this, &AT66PlayerController::HandleAttackUnlockPressed);
			if (IA_ToggleMouseLock) EIC->BindAction(IA_ToggleMouseLock, ETriggerEvent::Started, this, &AT66PlayerController::HandleToggleMouseLockPressed);
		}

		// TikTok next/prev (hard-bound; no mouse required). Only acts while viewer is open.
		// User preference: Q = Next, E = Previous.
		InputComponent->BindKey(EKeys::Q, IE_Pressed, this, &AT66PlayerController::HandleTikTokNextPressed);
		InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AT66PlayerController::HandleTikTokPrevPressed);

		// Old builds used CapsLock here; migrate the legacy default to Tab without touching custom rebinds.
		MigrateLegacyMediaViewerTabBinding(this);
		EnsureGameplayActionHasDefaultKeyboardBinding(this, FName(TEXT("InspectInventory")), EKeys::I);

#if !UE_BUILD_SHIPPING
		// Dev console overlay: Enter to open (close via Esc).
		InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &AT66PlayerController::ToggleDevConsole);
#endif
	}
}


void AT66PlayerController::HandleToggleImmortalityPressed()
{
	if (!IsGameplayLevel()) return;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->ToggleDevImmortality();
		}
	}
}


void AT66PlayerController::HandleTogglePowerPressed()
{
	if (!IsGameplayLevel()) return;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->ToggleDevPower();
		}
	}
}


void AT66PlayerController::HandleToggleMediaViewerPressed()
{
	if (!IsGameplayLevel()) return;
	if (!CanUsePausedMediaViewer(this)) return;
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr)
	{
		if (!PS->GetMediaViewerEnabled()) return;
	}
	if (UT66MediaViewerSubsystem* MV = GI ? GI->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
	{
		MV->ToggleMediaViewer();
		const bool bNowOpen = MV->IsMediaViewerOpen();
		UE_LOG(LogT66PlayerInput, Log, TEXT("MediaViewer toggled: %s"), bNowOpen ? TEXT("OPEN") : TEXT("CLOSED"));
	}
}


void AT66PlayerController::HandleOpenFullMapPressed()
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	// Full map is a HUD overlay (non-pausing). Toggle with the OpenFullMap binding (default: M).
	if (!GameplayHUDWidget) return;

	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
	}

	// Allow closing even though CanUseCombatMouseInput() is false while the map is open.
	if (GameplayHUDWidget->IsFullMapOpen())
	{
		GameplayHUDWidget->SetFullMapOpen(false);
		RestoreGameplayInputMode();
		return;
	}

	// If another gameplay overlay is active, ignore to avoid cursor/input-mode fights.
	if (!CanUseCombatMouseInput()) return;

	GameplayHUDWidget->ToggleFullMap();
	if (GameplayHUDWidget->IsFullMapOpen())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
	else
	{
		RestoreGameplayInputMode();
	}
}

void AT66PlayerController::HandleInspectInventoryPressed()
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	if (!GameplayHUDWidget) return;

	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
		return;
	}

	const bool bHasBlockingOverlay =
		(GameplayHUDWidget && GameplayHUDWidget->IsFullMapOpen())
		|| (GamblerOverlayWidget && GamblerOverlayWidget->IsInViewport())
		|| IsCircusOverlayOpen()
		|| (CowardicePromptWidget && CowardicePromptWidget->IsInViewport())
		|| (IdolAltarOverlayWidget && IdolAltarOverlayWidget->IsInViewport())
		|| (VendorOverlayWidget && VendorOverlayWidget->IsInViewport())
		|| (CollectorOverlayWidget && CollectorOverlayWidget->IsInViewport())
		|| bWorldDialogueOpen;
	if (bHasBlockingOverlay)
	{
		return;
	}

	SetInventoryInspectOpen(true);
}


void AT66PlayerController::HandleToggleGamerModePressed()
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	// TODO: Implement hitbox + projectile hitbox visualization overlay (Stage 50 unlock in Bible).
	UE_LOG(LogT66PlayerInput, Log, TEXT("ToggleGamerMode pressed (not implemented yet)."));
}


void AT66PlayerController::HandleRestartRunPressed()
{
	if (!IsGameplayLevel()) return;
	// Bible: host-only in co-op. v0: solo only (no co-op yet).
	if (IsPaused()) SetPause(false);
	const FName LevelToOpen = UT66GameInstance::GetTribulationEntryLevelName();
	UGameplayStatics::OpenLevel(this, LevelToOpen);
}


void AT66PlayerController::HandleDashPressed()
{
	if (!IsGameplayLevel()) return;
	if (AActor* A = GetPawn())
	{
		if (AT66HeroBase* Hero = Cast<AT66HeroBase>(A))
		{
			if (Hero->IsVehicleMounted()) return;
			Hero->DashForward();
		}
	}
}


void AT66PlayerController::HandleToggleHUDPressed()
{
	if (!IsGameplayLevel()) return;
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState)
	{
		RunState->ToggleHUDPanels();
	}
}


void AT66PlayerController::HandleToggleTikTokPressed()
{
	if (!IsGameplayLevel()) return;
	if (!CanUsePausedMediaViewer(this)) return;
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr)
	{
		if (!PS->GetMediaViewerEnabled()) return;
	}
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->ToggleTikTokPlaceholder();
	}
}


void AT66PlayerController::HandleTikTokPrevPressed()
{
	if (!IsGameplayLevel()) return;
	if (!CanUsePausedMediaViewer(this)) return;
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (UT66MediaViewerSubsystem* MV = GI ? GI->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
	{
		if (MV->IsMediaViewerOpen())
		{
			MV->TikTokPrev();
		}
	}
}


void AT66PlayerController::HandleTikTokNextPressed()
{
	if (!IsGameplayLevel()) return;
	if (!CanUsePausedMediaViewer(this)) return;
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (UT66MediaViewerSubsystem* MV = GI ? GI->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
	{
		if (MV->IsMediaViewerOpen())
		{
			MV->TikTokNext();
		}
	}
}


void AT66PlayerController::SetNearbyLootBag(AT66LootBagPickup* LootBag)
{
	if (NearbyLootBag.Get() != LootBag)
	{
		NearbyLootBag = LootBag;
		NearbyLootBagChanged.Broadcast();
	}
}


void AT66PlayerController::ClearNearbyLootBag(AT66LootBagPickup* LootBag)
{
	if (NearbyLootBag.Get() == LootBag)
	{
		NearbyLootBag.Reset();
		NearbyLootBagChanged.Broadcast();
	}
}


void AT66PlayerController::HandleMoveForward(float Value)
{
	if (!IsGameplayLevel()) return;

	// While an in-world NPC dialogue is open, use forward/back as UI navigation and block movement.
	if (bWorldDialogueOpen)
	{
		if (UWorld* World = GetWorld())
		{
			const float Now = World->GetTimeSeconds();
			if (Now - LastWorldDialogueNavTimeSeconds >= WorldDialogueNavDebounceSeconds)
			{
				// Down (S / stick down) goes to next option.
				if (Value <= -0.6f)
				{
					NavigateWorldDialogue(+1);
					LastWorldDialogueNavTimeSeconds = Now;
				}
				// Up (W / stick up) goes to previous option.
				else if (Value >= 0.6f)
				{
					NavigateWorldDialogue(-1);
					LastWorldDialogueNavTimeSeconds = Now;
				}
			}
		}
		return;
	}

	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn()))
	{
		if (Hero->IsVehicleMounted())
		{
			if (AT66PilotableTractor* Tractor = Hero->GetMountedTractor())
			{
				Tractor->SetDriveForwardInput(Value);
			}
			return;
		}
	}

	if (FMath::IsNearlyZero(Value)) return;

	// Tutorial progress: first time the player moves.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->NotifyTutorialMoveInput();
		}
	}
	
	if (APawn* MyPawn = GetPawn())
	{
		// Get the control rotation (where we're looking)
		FRotator ControlRot = GetControlRotation();
		ControlRot.Pitch = 0.f; // Don't include pitch for movement
		ControlRot.Roll = 0.f;
		
		// Get forward vector based on control rotation
		FVector ForwardDir = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::X);
		MyPawn->AddMovementInput(ForwardDir, Value);
	}
}


void AT66PlayerController::HandleMoveRight(float Value)
{
	if (!IsGameplayLevel()) return;

	// Block strafing while an in-world NPC dialogue is open (so WASD doesn't move you).
	if (bWorldDialogueOpen) return;

	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn()))
	{
		if (Hero->IsVehicleMounted())
		{
			if (AT66PilotableTractor* Tractor = Hero->GetMountedTractor())
			{
				Tractor->SetDriveRightInput(Value);
			}
			return;
		}
	}

	if (FMath::IsNearlyZero(Value)) return;

	// Tutorial progress: first time the player moves.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->NotifyTutorialMoveInput();
		}
	}
	
	if (APawn* MyPawn = GetPawn())
	{
		// Get the control rotation (where we're looking)
		FRotator ControlRot = GetControlRotation();
		ControlRot.Pitch = 0.f;
		ControlRot.Roll = 0.f;
		
		// Get right vector based on control rotation
		FVector RightDir = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::Y);
		MyPawn->AddMovementInput(RightDir, Value);
	}
}


void AT66PlayerController::HandleLookUp(float Value)
{
	if (!IsGameplayLevel() || FMath::IsNearlyZero(Value)) return;
	// When the gameplay cursor is visible, mouse movement should not rotate the camera.
	if (bShowMouseCursor) return;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->NotifyTutorialLookInput();
		}
	}
	AddPitchInput(Value);
}


void AT66PlayerController::HandleTurn(float Value)
{
	if (!IsGameplayLevel() || FMath::IsNearlyZero(Value)) return;
	// When the gameplay cursor is visible, mouse movement should not rotate the camera.
	if (bShowMouseCursor) return;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->NotifyTutorialLookInput();
		}
	}
	AddYawInput(Value);
}


void AT66PlayerController::HandleJumpPressed()
{
	if (!IsGameplayLevel()) return;

	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn()))
	{
		if (Hero->IsVehicleMounted()) return;
	}

	// Tutorial progress: first time the player jumps.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->NotifyTutorialJumpInput();
		}
	}
	
	if (ACharacter* MyCharacter = Cast<ACharacter>(GetPawn()))
	{
		UCharacterMovementComponent* CMC = MyCharacter->GetCharacterMovement();
		const int32 JumpCurrent = MyCharacter->JumpCurrentCount;
		const int32 JumpMax = MyCharacter->JumpMaxCount;
		const bool bOnGround = CMC ? CMC->IsMovingOnGround() : false;
		const bool bFalling = CMC ? CMC->IsFalling() : false;
		const FString MoveMode = CMC ? StaticEnum<EMovementMode>()->GetNameStringByValue(static_cast<int64>(CMC->MovementMode)) : TEXT("N/A");
		UE_LOG(LogT66PlayerInput, Verbose, TEXT("[JUMP] Space pressed: JumpCount=%d/%d OnGround=%d Falling=%d MoveMode=%s Z=%.1f"),
			JumpCurrent, JumpMax, bOnGround ? 1 : 0, bFalling ? 1 : 0, *MoveMode, MyCharacter->GetActorLocation().Z);

		MyCharacter->Jump();

		UWorld* World = GetWorld();
		UNiagaraSystem* JumpVFX = GetActiveJumpVFXSystem();
 		if (World && JumpVFX)
 		{
 			const FVector FeetLoc = MyCharacter->GetActorLocation() - FVector(0.f, 0.f, 50.f);
 			const FVector4 TintWhite(1.f, 1.f, 1.f, 0.8f);
 			UT66PixelVFXSubsystem* PixelVFX = World->GetSubsystem<UT66PixelVFXSubsystem>();
 			for (int32 i = 0; i < 6; ++i)
 			{
 				const FVector Offset(FMath::FRandRange(-30.f, 30.f), FMath::FRandRange(-30.f, 30.f), 0.f);
 				if (PixelVFX)
 				{
 					PixelVFX->SpawnPixelAtLocation(
 						FeetLoc + Offset,
 						FLinearColor(TintWhite.X, TintWhite.Y, TintWhite.Z, TintWhite.W),
 						FVector2D(2.0f, 2.0f),
 						ET66PixelVFXPriority::Low,
 						FRotator::ZeroRotator,
 						FVector(1.f),
 						JumpVFX);
 				}
 			}
		}
	}
}


void AT66PlayerController::HandleJumpReleased()
{
	if (!IsGameplayLevel()) return;
	
	if (ACharacter* MyCharacter = Cast<ACharacter>(GetPawn()))
	{
		MyCharacter->StopJumping();
	}
}


void AT66PlayerController::HandleZoom(float Value)
{
	if (!IsGameplayLevel() || FMath::IsNearlyZero(Value)) return;

	if (bHeroOneScopedUltActive && bHeroOneScopeViewEnabled)
	{
		if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn()))
		{
			if (Hero->FollowCamera)
			{
				const float NewFOV = FMath::Clamp(
					Hero->FollowCamera->FieldOfView - (Value * HeroOneScopedZoomStep),
					HeroOneScopedMinFOV,
					HeroOneScopedMaxFOV);
				Hero->FollowCamera->SetFieldOfView(NewFOV);
			}
		}
		return;
	}
	
	// Adjust camera boom length on the possessed hero (third-person zoom)
	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;
	
	AT66HeroBase* Hero = Cast<AT66HeroBase>(MyPawn);
	if (Hero && Hero->CameraBoom)
	{
		const float ZoomSpeed = 25.f;
		const float MinLength = 1200.f;  // Keep the current default gameplay view as the closest zoom.
		const float MaxLength = 2400.f;  // Start at the fully zoomed-out view and do not allow any farther zoom.
		float NewLength = Hero->CameraBoom->TargetArmLength - Value * ZoomSpeed;
		NewLength = FMath::Clamp(NewLength, MinLength, MaxLength);
		Hero->CameraBoom->TargetArmLength = NewLength;
	}
}
