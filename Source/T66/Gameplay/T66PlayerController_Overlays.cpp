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
#include "UI/Screens/T66LobbyScreen.h"
#include "UI/Screens/T66LobbyReadyCheckModal.h"
#include "UI/Screens/T66LobbyBackConfirmModal.h"
#include "UI/Screens/T66AchievementsScreen.h"
#include "UI/Screens/T66PauseMenuScreen.h"
#include "UI/Screens/T66ReportBugScreen.h"
#include "UI/Screens/T66SettingsScreen.h"
#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/Screens/T66PlayerSummaryPickerScreen.h"
#include "UI/Screens/T66PowerUpScreen.h"
#include "UI/Screens/T66LeaderboardScreen.h"
#include "UI/Screens/T66AccountStatusScreen.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "UI/T66GameplayHUDWidget.h"
#include "UI/Style/T66Style.h"
#include "UI/T66LabOverlayWidget.h"
#include "UI/T66CircusOverlayWidget.h"
#include "UI/T66GamblerOverlayWidget.h"
#include "UI/T66CowardicePromptWidget.h"
#include "UI/T66LoadPreviewOverlayWidget.h"
#include "UI/T66IdolAltarOverlayWidget.h"
#include "UI/T66VendorOverlayWidget.h"
#include "UI/T66CollectorOverlayWidget.h"
#include "UI/T66CrateOverlayWidget.h"
#include "Gameplay/T66FountainOfLifeInteractable.h"
#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66WheelSpinInteractable.h"
#include "Gameplay/T66CrateInteractable.h"
#include "Gameplay/T66CircusInteractable.h"
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
#include "Core/T66PowerUpSubsystem.h"
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


void AT66PlayerController::SetupGameplayHUD()
{
	if (!IsGameplayLevel()) return;
	UClass* HUDClass = ResolveGameplayHUDClass();
	if (!HUDClass) return;
	GameplayHUDWidget = CreateWidget<UT66GameplayHUDWidget>(this, HUDClass);
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->AddToViewport(0);
	}
	// The Lab: no floating panel; player interacts with The Collector NPC to open full-screen Collector UI.
	// (LabOverlayWidget not created when in Lab.)
}

void AT66PlayerController::RebuildThemeAwareUI()
{
	if (UIManager)
	{
		UIManager->RebuildAllVisibleUI();
	}

	if (GameplayHUDWidget && GameplayHUDWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(GameplayHUDWidget, 0);
	}
	if (GamblerOverlayWidget && GamblerOverlayWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(GamblerOverlayWidget, 100);
	}
	if (CircusOverlayWidget && CircusOverlayWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(CircusOverlayWidget, 100);
	}
	if (VendorOverlayWidget && VendorOverlayWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(VendorOverlayWidget, 100);
	}
	if (CollectorOverlayWidget && CollectorOverlayWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(CollectorOverlayWidget, 100);
	}
	if (CowardicePromptWidget && CowardicePromptWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(CowardicePromptWidget, 200);
	}
	if (IdolAltarOverlayWidget && IdolAltarOverlayWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(IdolAltarOverlayWidget, 150);
	}
}


void AT66PlayerController::OpenGamblerOverlay(int32 WinGoldAmount)
{
	if (!IsGameplayLevel()) return;

	if (!GamblerOverlayWidget)
	{
		GamblerOverlayWidget = CreateWidget<UT66GamblerOverlayWidget>(this, ResolveGamblerOverlayClass());
	}

	if (GamblerOverlayWidget && !GamblerOverlayWidget->IsInViewport())
	{
		GamblerOverlayWidget->SetWinGoldAmount(WinGoldAmount);
		GamblerOverlayWidget->AddToViewport(100); // above HUD
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}


void AT66PlayerController::OpenCircusOverlay()
{
	if (!IsGameplayLevel()) return;
	ActiveVendorNPC.Reset();

	if (!CircusOverlayWidget)
	{
		CircusOverlayWidget = CreateWidget<UT66CircusOverlayWidget>(this, ResolveCircusOverlayClass());
	}

	if (CircusOverlayWidget)
	{
		CircusOverlayWidget->OpenVendorTab();
		if (!CircusOverlayWidget->IsInViewport())
		{
			CircusOverlayWidget->AddToViewport(100);
		}

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AT66PlayerController::CloseCircusOverlay()
{
	if (CircusOverlayWidget && CircusOverlayWidget->IsInViewport())
	{
		CircusOverlayWidget->RemoveFromParent();
	}
	RestoreGameplayInputMode();
}

void AT66PlayerController::SwitchCircusOverlayToGambling()
{
	if (CircusOverlayWidget)
	{
		CircusOverlayWidget->OpenGamblingTab();
	}
}

void AT66PlayerController::SwitchCircusOverlayToVendor()
{
	if (CircusOverlayWidget)
	{
		CircusOverlayWidget->OpenVendorTab();
	}
}

void AT66PlayerController::SwitchCircusOverlayToAlchemy()
{
	if (CircusOverlayWidget)
	{
		CircusOverlayWidget->OpenAlchemyTab();
	}
}

bool AT66PlayerController::IsCircusOverlayOpen() const
{
	return CircusOverlayWidget && CircusOverlayWidget->IsInViewport();
}

bool AT66PlayerController::TriggerCircusBossIfAngry()
{
	if (!IsGameplayLevel()) return false;

	UWorld* World = GetWorld();
	if (!World) return false;

	UT66RunStateSubsystem* RunState = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState || RunState->GetCasinoAnger01() < 1.f || RunState->GetBossActive())
	{
		return false;
	}

	for (TActorIterator<AT66GamblerBoss> It(World); It; ++It)
	{
		return false;
	}

	FVector SpawnLoc = FVector::ZeroVector;
	bool bHasSpawnLoc = false;
	for (TActorIterator<AT66CircusInteractable> It(World); It; ++It)
	{
		if (AT66CircusInteractable* Circus = *It)
		{
			SpawnLoc = Circus->GetActorLocation();
			bHasSpawnLoc = true;
			break;
		}
	}

	if (!bHasSpawnLoc)
	{
		for (TActorIterator<AT66GamblerNPC> It(World); It; ++It)
		{
			if (AT66GamblerNPC* Gambler = *It)
			{
				SpawnLoc = Gambler->GetActorLocation();
				bHasSpawnLoc = true;
				Gambler->Destroy();
				break;
			}
		}
	}

	if (!bHasSpawnLoc)
	{
		if (APawn* FallbackPawn = GetPawn())
		{
			SpawnLoc = FallbackPawn->GetActorLocation() + FVector(600.f, 0.f, 0.f);
			bHasSpawnLoc = true;
		}
	}
	if (!bHasSpawnLoc)
	{
		return false;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AT66GamblerBoss>(AT66GamblerBoss::StaticClass(), SpawnLoc + FVector(0.f, 0.f, 200.f), FRotator::ZeroRotator, Params);

	if (CircusOverlayWidget && CircusOverlayWidget->IsInViewport())
	{
		CircusOverlayWidget->RemoveFromParent();
	}
	if (GamblerOverlayWidget && GamblerOverlayWidget->IsInViewport())
	{
		GamblerOverlayWidget->RemoveFromParent();
	}
	if (VendorOverlayWidget && VendorOverlayWidget->IsInViewport())
	{
		VendorOverlayWidget->RemoveFromParent();
	}
	RestoreGameplayInputMode();
	return true;
}

void AT66PlayerController::OpenVendorOverlay()
{
	if (!IsGameplayLevel()) return;

	if (!VendorOverlayWidget)
	{
		VendorOverlayWidget = CreateWidget<UT66VendorOverlayWidget>(this, ResolveVendorOverlayClass());
	}

	if (VendorOverlayWidget)
	{
		VendorOverlayWidget->SetEmbeddedInCircusShell(false);
		VendorOverlayWidget->SetVendorAllowsSteal(ActiveVendorNPC.IsValid() ? ActiveVendorNPC->DoesAllowSteal() : true);
		if (!VendorOverlayWidget->IsInViewport())
		{
			VendorOverlayWidget->AddToViewport(100); // above HUD
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
	}
}


void AT66PlayerController::OpenCollectorOverlay()
{
	if (!IsGameplayLevel()) return;

	if (!CollectorOverlayWidget)
	{
		CollectorOverlayWidget = CreateWidget<UT66CollectorOverlayWidget>(this, ResolveCollectorOverlayClass());
	}

	if (CollectorOverlayWidget && !CollectorOverlayWidget->IsInViewport())
	{
		CollectorOverlayWidget->AddToViewport(100);
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}


void AT66PlayerController::OpenCowardicePrompt(AT66CowardiceGate* Gate)
{
	if (!IsGameplayLevel() || !Gate) return;

	if (!CowardicePromptWidget)
	{
		CowardicePromptWidget = CreateWidget<UT66CowardicePromptWidget>(this, ResolveCowardicePromptClass());
	}

	if (CowardicePromptWidget && !CowardicePromptWidget->IsInViewport())
	{
		CowardicePromptWidget->SetGate(Gate);
		CowardicePromptWidget->AddToViewport(200); // above gambler overlay
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}


void AT66PlayerController::ShowLoadPreviewOverlay()
{
	if (!IsGameplayLevel()) return;

	UT66LoadPreviewOverlayWidget* W = CreateWidget<UT66LoadPreviewOverlayWidget>(this, ResolveLoadPreviewOverlayClass());
	if (W)
	{
		W->AddToViewport(500); // above HUD and other overlays
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
	}
}


void AT66PlayerController::StartWheelSpinHUD(ET66Rarity Rarity)
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->StartWheelSpin(Rarity);
	}
}


void AT66PlayerController::StartCrateOpenHUD()
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->StartCrateOpen();
	}
}


void AT66PlayerController::OnPlayerDied()
{
	EndHeroOneScopedUlt();

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				MV->SetMediaViewerOpen(false);
			}
		}
	}

	// Spawn death VFX (red pixel burst) at hero location before pausing.
	if (APawn* HeroPawn = GetPawn())
	{
		const FVector DeathLoc = HeroPawn->GetActorLocation();
		if (UT66CombatComponent* Combat = HeroPawn->FindComponentByClass<UT66CombatComponent>())
		{
			Combat->SpawnDeathVFX(DeathLoc);
		}
		HeroPawn->SetActorHiddenInGame(true);
	}

	// Brief delay so the death particles are visible before the game pauses.
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(DeathVFXTimerHandle, [this]()
		{
			SetPause(true);
			EnsureGameplayUIManager();
			if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
			{
				UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
				UT66PowerUpSubsystem* PowerUpSub = GI->GetSubsystem<UT66PowerUpSubsystem>();
				if (RunState)
				{
					RunState->MarkRunEnded(false);
				}
				if (RunState && PowerUpSub)
				{
					const int32 Earned = RunState->GetPowerCrystalsEarnedThisRun();
					if (Earned > 0)
					{
						PowerUpSub->AddPowerCrystals(Earned);
					}
				}
				if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
				{
					Achieve->NotifyRunCompleted(RunState);
				}
			}
			if (UIManager)
			{
				UIManager->ShowModal(ET66ScreenType::RunSummary);
			}
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}, 0.5f, false);
	}
	else
	{
		SetPause(true);
		EnsureGameplayUIManager();
		if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->MarkRunEnded(false);
			}
		}
		if (UIManager)
		{
			UIManager->ShowModal(ET66ScreenType::RunSummary);
		}
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AT66PlayerController::ShowVictoryRunSummary()
{
	EndHeroOneScopedUlt();

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				MV->SetMediaViewerOpen(false);
			}
		}

		UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		UT66PowerUpSubsystem* PowerUpSub = GI->GetSubsystem<UT66PowerUpSubsystem>();
		if (RunState)
		{
			RunState->MarkRunEnded(true);
		}
		if (RunState && PowerUpSub)
		{
			const int32 Earned = RunState->GetPowerCrystalsEarnedThisRun();
			if (Earned > 0)
			{
				PowerUpSub->AddPowerCrystals(Earned);
			}
		}
		if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achieve->NotifyRunCompleted(RunState);
		}
	}

	SetPause(true);
	EnsureGameplayUIManager();
	if (UIManager)
	{
		UIManager->ShowModal(ET66ScreenType::RunSummary);
	}
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AT66PlayerController::ShowDifficultyClearSummary()
{
	EndHeroOneScopedUlt();

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				MV->SetMediaViewerOpen(false);
			}
		}
	}

	SetPause(true);
	EnsureGameplayUIManager();
	if (UIManager)
	{
		UIManager->ShowModal(ET66ScreenType::RunSummary);
	}
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}


void AT66PlayerController::HandleQuickReviveStateChanged()
{
	if (!IsGameplayLevel())
	{
		return;
	}

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const bool bDowned = RunState && RunState->IsInQuickReviveDownedState();

	if (bDowned)
	{
		EndHeroOneScopedUlt();
	}

	SetIgnoreMoveInput(bDowned);
	SetIgnoreLookInput(bDowned);

	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn()))
	{
		Hero->SetQuickReviveDowned(bDowned);

		if (UT66CombatComponent* Combat = Hero->FindComponentByClass<UT66CombatComponent>())
		{
			if (bDowned)
			{
				Combat->SetAutoAttackSuppressed(true);
				Combat->ClearLockedTarget();
			}
			else if (!bHeroOneScopedUltActive && !Hero->IsVehicleMounted())
			{
				Combat->SetAutoAttackSuppressed(false);
			}
		}
	}

	SetLockedEnemy(nullptr, false);
}


void AT66PlayerController::EnsureGameplayUIManager()
{
	if (UIManager || !IsGameplayLevel()) return;

	UIManager = NewObject<UT66UIManager>(this, UT66UIManager::StaticClass());
	if (!UIManager) return;

	UIManager->Initialize(this);
	if (TSubclassOf<UT66ScreenBase> PauseClass = ResolveScreenClass(ET66ScreenType::PauseMenu))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::PauseMenu, PauseClass);
	}
	if (TSubclassOf<UT66ScreenBase> AchievementsClass = ResolveScreenClass(ET66ScreenType::Achievements))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::Achievements, AchievementsClass);
	}
	if (TSubclassOf<UT66ScreenBase> ReportBugClass = ResolveScreenClass(ET66ScreenType::ReportBug))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::ReportBug, ReportBugClass);
	}
	if (TSubclassOf<UT66ScreenBase> SettingsClass = ResolveScreenClass(ET66ScreenType::Settings))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::Settings, SettingsClass);
	}
	if (TSubclassOf<UT66ScreenBase> RunSummaryClass = ResolveScreenClass(ET66ScreenType::RunSummary))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::RunSummary, RunSummaryClass);
	}
	if (TSubclassOf<UT66ScreenBase> SummaryPickerClass = ResolveScreenClass(ET66ScreenType::PlayerSummaryPicker))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::PlayerSummaryPicker, SummaryPickerClass);
	}
	if (TSubclassOf<UT66ScreenBase> PowerUpClass = ResolveScreenClass(ET66ScreenType::PowerUp))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::PowerUp, PowerUpClass);
	}
	if (TSubclassOf<UT66ScreenBase> LeaderboardClass = ResolveScreenClass(ET66ScreenType::Leaderboard))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::Leaderboard, LeaderboardClass);
	}
	if (TSubclassOf<UT66ScreenBase> AccountStatusClass = ResolveScreenClass(ET66ScreenType::AccountStatus))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::AccountStatus, AccountStatusClass);
	}
}


void AT66PlayerController::HandleEscapePressed()
{
	if (!IsGameplayLevel()) return;

#if !UE_BUILD_SHIPPING
	// Highest priority: if dev console is open, Esc should close it and never open PauseMenu.
	if (bDevConsoleOpen)
	{
		CloseDevConsole();

		// Debounce to avoid the same keypress immediately toggling PauseMenu.
		const float Now = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds()) : 0.f;
		LastPauseToggleTime = Now;
		return;
	}
#endif

	// Esc as back: close Media Viewer before other UI.
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				MV->SetMediaViewerOpen(false);
				SetIgnoreMoveInput(false);
				SetIgnoreLookInput(false);
				RestoreGameplayInputMode();
				return;
			}
		}
	}

	// Esc as back: close the full map before opening pause menu.
	if (GameplayHUDWidget && GameplayHUDWidget->IsFullMapOpen())
	{
		GameplayHUDWidget->SetFullMapOpen(false);
		RestoreGameplayInputMode();
		return;
	}

	const float Now = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds()) : 0.f;
	const bool bPaused = IsPaused();

	// Closing a sub-modal (Settings / Report Bug / Achievements) returns to Pause menu without debounce so the next Esc can unpause.
	if (bPaused && UIManager && UIManager->IsModalActive())
	{
		const ET66ScreenType ClosingModal = UIManager->GetCurrentModalType();
		if (ClosingModal == ET66ScreenType::Settings
			|| ClosingModal == ET66ScreenType::ReportBug
			|| ClosingModal == ET66ScreenType::Achievements
			|| ClosingModal == ET66ScreenType::Leaderboard
			|| ClosingModal == ET66ScreenType::PlayerSummaryPicker
			|| ClosingModal == ET66ScreenType::AccountStatus
			|| ClosingModal == ET66ScreenType::RunSummary)
		{
			UIManager->CloseModal();

			if (ClosingModal == ET66ScreenType::PlayerSummaryPicker || ClosingModal == ET66ScreenType::AccountStatus)
			{
				UIManager->ShowModal(ET66ScreenType::Leaderboard);
				return;
			}

			if (ClosingModal == ET66ScreenType::RunSummary)
			{
				if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
				{
					if (UT66LeaderboardSubsystem* LB = GI->GetSubsystem<UT66LeaderboardSubsystem>())
					{
						const ET66ScreenType ReturnModal = LB->ConsumePendingReturnModalAfterViewerRunSummary();
						if (ReturnModal != ET66ScreenType::None)
						{
							UIManager->ShowModal(ReturnModal);
							return;
						}
					}
				}
			}

			UIManager->ShowModal(ET66ScreenType::PauseMenu);
			return;
		}
	}

	// Debounce only when actually toggling pause (open or full unpause)
	if (Now - LastPauseToggleTime < PauseToggleDebounceSeconds)
	{
		return;
	}
	LastPauseToggleTime = Now;

	if (!bPaused)
	{
		SetPause(true);
		EnsureGameplayUIManager();
		if (UIManager)
		{
			UIManager->ShowModal(ET66ScreenType::PauseMenu);
		}
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
	else
	{
		if (UIManager && UIManager->IsModalActive())
		{
			UIManager->CloseModal();
			SetPause(false);
			RestoreGameplayInputMode();
		}
		else
		{
			SetPause(false);
			RestoreGameplayInputMode();
		}
	}
}
