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
#include "UI/Screens/T66PauseMenuScreen.h"
#include "UI/Screens/T66ReportBugScreen.h"
#include "UI/Screens/T66SettingsScreen.h"
#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/Screens/T66PlayerSummaryPickerScreen.h"
#include "UI/Screens/T66PowerUpScreen.h"
#include "UI/Screens/T66AccountStatusScreen.h"
#include "UI/T66GameplayHUDWidget.h"
#include "UI/T66LabOverlayWidget.h"
#include "UI/T66GamblerOverlayWidget.h"
#include "UI/T66CowardicePromptWidget.h"
#include "UI/T66IdolAltarOverlayWidget.h"
#include "UI/T66VendorOverlayWidget.h"
#include "UI/T66CollectorOverlayWidget.h"
#include "Gameplay/T66TreeOfLifeInteractable.h"
#include "Gameplay/T66CashTruckInteractable.h"
#include "Gameplay/T66WheelSpinInteractable.h"
#include "Gameplay/T66StageBoostGate.h"
#include "Gameplay/T66StageBoostGoldInteractable.h"
#include "Gameplay/T66StageBoostLootInteractable.h"
#include "Gameplay/T66TutorialPortal.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66PowerUpSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66MediaViewerSubsystem.h"
#include "Gameplay/T66IdolAltar.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66RecruitableCompanion.h"
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
#include "Gameplay/T66EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
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
#include "EngineUtils.h"

#if !UE_BUILD_SHIPPING
namespace
{
	class ST66DevConsole final : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66DevConsole) {}
			SLATE_ARGUMENT(TArray<FString>, Commands)
			SLATE_EVENT(FSimpleDelegate, OnRequestClose)
			SLATE_EVENT(FSimpleDelegate, OnRequestRunOptionHelp)
			SLATE_EVENT(FOnTextCommitted, OnExecuteCommand)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			AllCommands.Reserve(InArgs._Commands.Num());
			for (const FString& S : InArgs._Commands)
			{
				AllCommands.Add(MakeShared<FString>(S));
			}

			OnRequestClose = InArgs._OnRequestClose;
			OnExecuteCommand = InArgs._OnExecuteCommand;

			RebuildFiltered(TEXT(""));

			ChildSlot
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.60f))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.Padding(FMargin(24.f, 60.f, 24.f, 24.f))
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.06f, 0.95f))
					.Padding(12.f)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("DEV CONSOLE")))
							.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.9f))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 6.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Enter = run   Esc = close   Up/Down = history")))
							.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.6f))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 10.f, 0.f, 0.f)
						[
							SAssignNew(InputBox, SEditableTextBox)
							.HintText(FText::FromString(TEXT("Type a command (e.g. Option1, Option4B, clearlead)")))
							.OnTextChanged(this, &ST66DevConsole::HandleTextChanged)
							.OnTextCommitted(this, &ST66DevConsole::HandleTextCommitted)
							.OnKeyDownHandler(this, &ST66DevConsole::HandleInputKeyDown)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 10.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Commands (filtered):")))
							.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.7f))
						]

						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						.Padding(0.f, 6.f, 0.f, 0.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.35f))
							.Padding(6.f)
							[
								SAssignNew(CommandListView, SListView<TSharedPtr<FString>>)
								.ListItemsSource(&FilteredCommands)
								.OnGenerateRow(this, &ST66DevConsole::GenerateRow)
								.OnSelectionChanged(this, &ST66DevConsole::OnSelected)
							]
						]
					]
				]
			];
		}

		void FocusInput()
		{
			if (InputBox.IsValid())
			{
				FSlateApplication::Get().SetKeyboardFocus(InputBox, EFocusCause::SetDirectly);
			}
		}

		void PushHistory(const FString& Cmd)
		{
			if (Cmd.IsEmpty()) return;
			History.Add(Cmd);
			HistoryIndex = History.Num(); // one past end
		}

	private:
		TSharedRef<ITableRow> GenerateRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
		{
			return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
			[
				SNew(STextBlock)
				.Text(Item.IsValid() ? FText::FromString(*Item) : FText::GetEmpty())
				.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.85f))
			];
		}

		void OnSelected(TSharedPtr<FString> Item, ESelectInfo::Type)
		{
			if (!Item.IsValid() || !InputBox.IsValid()) return;
			InputBox->SetText(FText::FromString(*Item));
			FocusInput();
		}

		void HandleTextChanged(const FText& NewText)
		{
			RebuildFiltered(*NewText.ToString());
			if (CommandListView.IsValid())
			{
				CommandListView->RequestListRefresh();
			}
		}

		void HandleTextCommitted(const FText& Text, ETextCommit::Type CommitType)
		{
			if (CommitType != ETextCommit::OnEnter) return;
			const FString Cmd = Text.ToString().TrimStartAndEnd();
			if (Cmd.IsEmpty()) return;

			PushHistory(Cmd);

			if (OnExecuteCommand.IsBound())
			{
				OnExecuteCommand.Execute(Text, CommitType);
			}

			if (InputBox.IsValid())
			{
				InputBox->SetText(FText::GetEmpty());
			}
			RebuildFiltered(TEXT(""));
			if (CommandListView.IsValid())
			{
				CommandListView->RequestListRefresh();
			}
			FocusInput();
		}

		FReply HandleInputKeyDown(const FGeometry&, const FKeyEvent& E)
		{
			const FKey Key = E.GetKey();
			if (Key == EKeys::Escape)
			{
				if (OnRequestClose.IsBound())
				{
					OnRequestClose.Execute();
				}
				return FReply::Handled();
			}

			if (Key == EKeys::Up)
			{
				if (History.Num() > 0)
				{
					HistoryIndex = FMath::Clamp(HistoryIndex - 1, 0, History.Num() - 1);
					if (InputBox.IsValid())
					{
						InputBox->SetText(FText::FromString(History[HistoryIndex]));
					}
				}
				return FReply::Handled();
			}

			if (Key == EKeys::Down)
			{
				if (History.Num() > 0)
				{
					HistoryIndex = FMath::Clamp(HistoryIndex + 1, 0, History.Num());
					if (InputBox.IsValid())
					{
						if (HistoryIndex >= History.Num())
						{
							InputBox->SetText(FText::GetEmpty());
						}
						else
						{
							InputBox->SetText(FText::FromString(History[HistoryIndex]));
						}
					}
				}
				return FReply::Handled();
			}

			return FReply::Unhandled();
		}

		void RebuildFiltered(const FString& Prefix)
		{
			const FString P = Prefix.TrimStartAndEnd();
			FilteredCommands.Reset();

			for (const TSharedPtr<FString>& Cmd : AllCommands)
			{
				if (!Cmd.IsValid()) continue;
				if (P.IsEmpty() || Cmd->StartsWith(P, ESearchCase::IgnoreCase))
				{
					FilteredCommands.Add(Cmd);
				}
			}
		}

	private:
		TArray<TSharedPtr<FString>> AllCommands;
		TArray<TSharedPtr<FString>> FilteredCommands;

		TSharedPtr<SEditableTextBox> InputBox;
		TSharedPtr<SListView<TSharedPtr<FString>>> CommandListView;

		TArray<FString> History;
		int32 HistoryIndex = 0;

		FSimpleDelegate OnRequestClose;
		FOnTextCommitted OnExecuteCommand;
	};
}
#endif // !UE_BUILD_SHIPPING

AT66PlayerController::AT66PlayerController()
{
	// Default to showing mouse cursor (will be hidden in gameplay mode)
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	// Same Niagara asset as slash (VFX_Attack1) for jump puffs.
	JumpVFXNiagara = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1")));
}

void AT66PlayerController::ToggleDevConsole()
{
#if !UE_BUILD_SHIPPING
	if (!IsGameplayLevel()) return;
	// Don't allow dev console while paused (prevents Esc competition with PauseMenu).
	if (IsPaused()) return;
	if (bDevConsoleOpen)
	{
		return; // close via Esc
	}
	OpenDevConsole();
#endif
}

void AT66PlayerController::OpenDevConsole()
{
#if !UE_BUILD_SHIPPING
	if (bDevConsoleOpen) return;
	if (!GEngine || !GEngine->GameViewport) return;

	// Stop gameplay input while console is open.
	SetIgnoreLookInput(true);
	SetIgnoreMoveInput(true);

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	const TArray<FString> Commands = {
		TEXT("clearlead"),
		TEXT("clear.lead"),
		TEXT("sus1"),
		TEXT("sus2"),
		TEXT("t66.AccountStatus.Force 1"),
		TEXT("t66.AccountStatus.Force 2"),
	};

	TSharedPtr<ST66DevConsole> ConsoleWidget;
	SAssignNew(ConsoleWidget, ST66DevConsole)
		.Commands(Commands)
		.OnRequestClose(FSimpleDelegate::CreateUObject(this, &AT66PlayerController::CloseDevConsole))
		.OnExecuteCommand(FOnTextCommitted::CreateWeakLambda(this, [this](const FText& Text, ETextCommit::Type)
		{
			const FString Cmd = Text.ToString().TrimStartAndEnd();
			if (!Cmd.IsEmpty())
			{
				// Route through PlayerController so Exec UFUNCTIONs are reachable.
				ConsoleCommand(Cmd, /*bWriteToLog*/ true);
			}
		}));

	DevConsoleWidget = ConsoleWidget;
	DevConsoleWeakWidget = SNew(SWeakWidget).PossiblyNullContent(DevConsoleWidget.ToSharedRef());

	GEngine->GameViewport->AddViewportWidgetContent(DevConsoleWeakWidget.ToSharedRef(), 20000);
	bDevConsoleOpen = true;

	ConsoleWidget->FocusInput();
	UE_LOG(LogTemp, Log, TEXT("DevConsole: opened"));
#endif
}

void AT66PlayerController::CloseDevConsole()
{
#if !UE_BUILD_SHIPPING
	if (!bDevConsoleOpen) return;
	if (GEngine && GEngine->GameViewport && DevConsoleWeakWidget.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(DevConsoleWeakWidget.ToSharedRef());
	}

	DevConsoleWeakWidget.Reset();
	DevConsoleWidget.Reset();
	bDevConsoleOpen = false;

	// Safety: if PauseMenu got opened while console was up, close it and unpause.
	if (IsPaused())
	{
		if (UIManager && UIManager->IsModalActive() && UIManager->GetCurrentModalType() == ET66ScreenType::PauseMenu)
		{
			UIManager->CloseModal();
		}
		SetPause(false);
	}

	// Restore normal gameplay input.
	SetIgnoreLookInput(false);
	SetIgnoreMoveInput(false);
	RestoreGameplayInputMode();
	UE_LOG(LogTemp, Log, TEXT("DevConsole: closed"));
#endif
}

void AT66PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsGameplayLevel())
	{
		SetupGameplayMode();
		SetupGameplayHUD();
		CachedJumpVFXNiagara = JumpVFXNiagara.LoadSynchronous();

		// Prewarm TikTok/WebView2 so login + CSS formatting are done before first toggle.
		if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
			{
				MV->PrewarmTikTok();
			}
		}

		UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		if (RunState)
		{
			RunState->OnPlayerDied.AddDynamic(this, &AT66PlayerController::OnPlayerDied);
			OnPlayerDiedHandle.Reset(); // dynamic delegate has no handle; unbind via RemoveDynamic in EndPlay if needed
		}
		UE_LOG(LogTemp, Log, TEXT("PlayerController: Gameplay mode initialized"));
	}
	else
	{
		// Frontend mode: UI input, show cursor, initialize UI
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;

		if (bAutoShowInitialScreen)
		{
			// Immediately cover the viewport with a black curtain so the 3D frontend level
			// (platform, hero) is never visible before the main menu UI is ready.
			TSharedPtr<SBorder> BlackCurtain;
			if (GEngine && GEngine->GameViewport)
			{
				SAssignNew(BlackCurtain, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor::Black);
				GEngine->GameViewport->AddViewportWidgetContent(
					SNew(SWeakWidget).PossiblyNullContent(BlackCurtain), 9999);
			}

			// Defer UI init by a short delay so the PIE viewport can stabilize its resolution.
			// Without this, the first frame renders at a transient viewport size, causing a brief
			// "zoomed-in" flash before snapping to the correct resolution. The delay also gives
			// GameInstance async texture preloads more time to complete, reducing sync load fallbacks.
			FTimerHandle DeferHandle;
			GetWorld()->GetTimerManager().SetTimer(DeferHandle, FTimerDelegate::CreateWeakLambda(this, [this, BlackCurtain]()
			{
				InitializeUI();

				// Remove the black curtain now that the main menu is fully painted.
				if (GEngine && GEngine->GameViewport && BlackCurtain.IsValid())
				{
					GEngine->GameViewport->RemoveViewportWidgetContent(BlackCurtain.ToSharedRef());
				}
			}), 0.1f, false);
		}
		UE_LOG(LogTemp, Log, TEXT("PlayerController: Frontend mode initialized (UI deferred)"));
	}
}

void AT66PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind from long-lived RunState delegates.
	if (UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		RunState->OnPlayerDied.RemoveDynamic(this, &AT66PlayerController::OnPlayerDied);
	}

	Super::EndPlay(EndPlayReason);
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
	return MapName.Contains(TEXT("Gameplay"));
}

void AT66PlayerController::SetupGameplayMode()
{
	RestoreGameplayInputMode();
	UE_LOG(LogTemp, Log, TEXT("PlayerController: Gameplay input mode set, cursor hidden"));
}

void AT66PlayerController::RestoreGameplayInputMode()
{
	// Classic gameplay: mouse rotates camera, cursor hidden.
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
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

void AT66PlayerController::HandleUltimatePressed()
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	if (!RunState->TryActivateUltimate())
	{
		return;
	}

	// v0 effect: deal flat damage to all active enemies.
	const FName UltimateSourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
	for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
	{
		if (AT66EnemyBase* E = *It)
		{
			E->TakeDamageFromHero(UT66RunStateSubsystem::UltimateDamage, UltimateSourceID, NAME_None);
		}
	}
}

void AT66PlayerController::HandleToggleMediaViewerPressed()
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (UT66MediaViewerSubsystem* MV = GI ? GI->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
	{
		MV->ToggleMediaViewer();
		const bool bNowOpen = MV->IsMediaViewerOpen();
		UE_LOG(LogTemp, Log, TEXT("MediaViewer toggled: %s"), bNowOpen ? TEXT("OPEN") : TEXT("CLOSED"));
	}
}

void AT66PlayerController::HandleOpenFullMapPressed()
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	// Full map is a HUD overlay (non-pausing). Toggle with the OpenFullMap binding (default: M).
	if (!GameplayHUDWidget) return;

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

void AT66PlayerController::HandleToggleGamerModePressed()
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	// TODO: Implement hitbox + projectile hitbox visualization overlay (Stage 50 unlock in Bible).
	UE_LOG(LogTemp, Log, TEXT("ToggleGamerMode pressed (not implemented yet)."));
}

void AT66PlayerController::HandleRestartRunPressed()
{
	if (!IsGameplayLevel()) return;
	// Bible: host-only in co-op. v0: solo only (no co-op yet).
	if (IsPaused()) SetPause(false);
	UGameplayStatics::OpenLevel(this, FName(TEXT("GameplayLevel")));
}

bool AT66PlayerController::CanUseCombatMouseInput() const
{
	// Suppress combat mouse inputs if a modal overlay is active (cursor is visible for UI).
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	return IsGameplayLevel()
		&& !(GameplayHUDWidget && GameplayHUDWidget->IsFullMapOpen())
		&& !(GamblerOverlayWidget && GamblerOverlayWidget->IsInViewport())
		&& !(CowardicePromptWidget && CowardicePromptWidget->IsInViewport())
		&& !(IdolAltarOverlayWidget && IdolAltarOverlayWidget->IsInViewport())
		&& !(VendorOverlayWidget && VendorOverlayWidget->IsInViewport());
}

void AT66PlayerController::HandleAttackLockPressed()
{
	if (!CanUseCombatMouseInput()) return;

	APawn* P = GetPawn();
	if (!P) return;
	AT66HeroBase* Hero = Cast<AT66HeroBase>(P);
	if (!Hero || !Hero->CombatComponent) return;

	int32 SizeX = 0, SizeY = 0;
	GetViewportSize(SizeX, SizeY);
	if (SizeX <= 0 || SizeY <= 0) return;

	// Crosshair is slightly above screen-center (matches HUD crosshair placement).
	static constexpr float CrosshairYOffset = -140.f;
	const FVector2D Center(static_cast<float>(SizeX) * 0.5f, (static_cast<float>(SizeY) * 0.5f) + CrosshairYOffset);
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AimLock), false);
	Params.AddIgnoredActor(Hero);
	const bool bHit = GetHitResultAtScreenPosition(Center, ECC_Visibility, Params, Hit);

	AT66EnemyBase* Enemy = bHit ? Cast<AT66EnemyBase>(Hit.GetActor()) : nullptr;
	if (!Enemy || Enemy->CurrentHP <= 0)
	{
		// Clicking empty space does nothing.
		return;
	}

	// Toggle if clicking the same enemy.
	if (LockedEnemy.IsValid() && LockedEnemy.Get() == Enemy)
	{
		HandleAttackUnlockPressed();
		return;
	}

	// Clear previous indicator.
	if (LockedEnemy.IsValid())
	{
		LockedEnemy->SetLockedIndicator(false);
	}

	LockedEnemy = Enemy;
	Enemy->SetLockedIndicator(true);
	Hero->CombatComponent->SetLockedTarget(Enemy);
}

void AT66PlayerController::HandleAttackUnlockPressed()
{
	// If we're standing on a loot bag, RMB discards it (so loot accept/reject works without showing cursor).
	if (AT66LootBagPickup* Bag = NearbyLootBag.Get())
	{
		if (APawn* P = GetPawn())
		{
			const float DistSq = FVector::DistSquared(P->GetActorLocation(), Bag->GetActorLocation());
			if (DistSq <= (250.f * 250.f))
			{
				Bag->ConsumeAndDestroy();
				NearbyLootBag.Reset();
				return;
			}
		}
	}

	APawn* P = GetPawn();
	AT66HeroBase* Hero = Cast<AT66HeroBase>(P);
	if (Hero && Hero->CombatComponent)
	{
		Hero->CombatComponent->ClearLockedTarget();
	}

	if (LockedEnemy.IsValid())
	{
		LockedEnemy->SetLockedIndicator(false);
	}
	LockedEnemy.Reset();
}

void AT66PlayerController::HandleToggleMouseLockPressed()
{
	if (!IsGameplayLevel()) return;
	if (!CanUseCombatMouseInput()) return;

	if (bShowMouseCursor)
	{
		RestoreGameplayInputMode();
	}
	else
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AT66PlayerController::SetupGameplayEnhancedInputMappings()
{
	if (IA_AttackLock && IA_AttackUnlock && IA_ToggleMouseLock && IMC_GameplayMouse) return;

	UObject* Outer = GetTransientPackage();
	IA_AttackLock = NewObject<UInputAction>(Outer, FName(TEXT("IA_AttackLock_T66")));
	if (IA_AttackLock) IA_AttackLock->ValueType = EInputActionValueType::Boolean;
	IA_AttackUnlock = NewObject<UInputAction>(Outer, FName(TEXT("IA_AttackUnlock_T66")));
	if (IA_AttackUnlock) IA_AttackUnlock->ValueType = EInputActionValueType::Boolean;
	IA_ToggleMouseLock = NewObject<UInputAction>(Outer, FName(TEXT("IA_ToggleMouseLock_T66")));
	if (IA_ToggleMouseLock) IA_ToggleMouseLock->ValueType = EInputActionValueType::Boolean;

	IMC_GameplayMouse = NewObject<UInputMappingContext>(Outer, FName(TEXT("IMC_GameplayMouse_T66")));
	if (IMC_GameplayMouse)
	{
		if (IA_AttackLock) IMC_GameplayMouse->MapKey(IA_AttackLock, EKeys::LeftMouseButton);
		if (IA_AttackUnlock) IMC_GameplayMouse->MapKey(IA_AttackUnlock, EKeys::RightMouseButton);
		if (IA_ToggleMouseLock) IMC_GameplayMouse->MapKey(IA_ToggleMouseLock, EKeys::RightMouseButton);
	}

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (IMC_GameplayMouse) Sub->AddMappingContext(IMC_GameplayMouse, 0);
		}
	}
}

void AT66PlayerController::HandleDashPressed()
{
	if (!IsGameplayLevel()) return;
	if (AActor* A = GetPawn())
	{
		if (AT66HeroBase* Hero = Cast<AT66HeroBase>(A))
		{
			Hero->DashForward();
		}
	}
}

void AT66PlayerController::SetupGameplayHUD()
{
	if (!IsGameplayLevel()) return;
	UClass* HUDClass = UT66GameplayHUDWidget::StaticClass();
	if (!HUDClass) return;
	GameplayHUDWidget = CreateWidget<UT66GameplayHUDWidget>(this, HUDClass);
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->AddToViewport(0);
	}
	// The Lab: no floating panel; player interacts with The Collector NPC to open full-screen Collector UI.
	// (LabOverlayWidget not created when in Lab.)
}

void AT66PlayerController::HandleToggleHUDPressed()
{
	if (!IsGameplayLevel()) return;
	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState)
	{
		RunState->ToggleHUDPanels();
	}
}

void AT66PlayerController::HandleToggleTikTokPressed()
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->ToggleTikTokPlaceholder();
	}
}

void AT66PlayerController::HandleTikTokPrevPressed()
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
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
	if (IsPaused()) return;
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (UT66MediaViewerSubsystem* MV = GI ? GI->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
	{
		if (MV->IsMediaViewerOpen())
		{
			MV->TikTokNext();
		}
	}
}

void AT66PlayerController::HandleInteractPressed()
{
	if (!IsGameplayLevel()) return;

	// If we're in an in-world NPC dialogue, Interact confirms the selection.
	if (bWorldDialogueOpen)
	{
		ConfirmWorldDialogue();
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const float InteractRadius = 200.f;
	FVector Loc = ControlledPawn->GetActorLocation();
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ControlledPawn);
	World->OverlapMultiByChannel(Overlaps, Loc, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(InteractRadius), Params);

	AT66StageGate* ClosestStageGate = nullptr;
	AT66CowardiceGate* ClosestCowardiceGate = nullptr;
	AT66DifficultyTotem* ClosestTotem = nullptr;
	AT66HouseNPCBase* ClosestNPC = nullptr;
	AT66RecruitableCompanion* ClosestRecruitableCompanion = nullptr;
	AT66ItemPickup* ClosestPickup = nullptr;
	AT66LootBagPickup* ClosestLootBag = nullptr;
	AT66TutorialPortal* ClosestTutorialPortal = nullptr;
	AT66IdolAltar* ClosestIdolAltar = nullptr;
	AT66TreeOfLifeInteractable* ClosestTree = nullptr;
	AT66CashTruckInteractable* ClosestTruck = nullptr;
	AT66WheelSpinInteractable* ClosestWheel = nullptr;
	AT66StageBoostGate* ClosestBoostGate = nullptr;
	AT66StageBoostGoldInteractable* ClosestBoostGold = nullptr;
	AT66StageBoostLootInteractable* ClosestBoostLoot = nullptr;
	float ClosestStageGateDistSq = InteractRadius * InteractRadius;
	float ClosestCowardiceGateDistSq = InteractRadius * InteractRadius;
	float ClosestTotemDistSq = InteractRadius * InteractRadius;
	float ClosestNPCDistSq = InteractRadius * InteractRadius;
	float ClosestRecruitableCompanionDistSq = InteractRadius * InteractRadius;
	float ClosestPickupDistSq = InteractRadius * InteractRadius;
	float ClosestLootBagDistSq = InteractRadius * InteractRadius;
	float ClosestTutorialPortalDistSq = InteractRadius * InteractRadius;
	float ClosestIdolAltarDistSq = InteractRadius * InteractRadius;
	float ClosestTreeDistSq = InteractRadius * InteractRadius;
	float ClosestTruckDistSq = InteractRadius * InteractRadius;
	float ClosestWheelDistSq = InteractRadius * InteractRadius;
	float ClosestBoostGateDistSq = InteractRadius * InteractRadius;
	float ClosestBoostGoldDistSq = InteractRadius * InteractRadius;
	float ClosestBoostLootDistSq = InteractRadius * InteractRadius;

	for (const FOverlapResult& R : Overlaps)
	{
		AActor* A = R.GetActor();
		if (!A) continue;
		float DistSq = FVector::DistSquared(Loc, A->GetActorLocation());
		if (AT66StageGate* G = Cast<AT66StageGate>(A))
		{
			if (DistSq < ClosestStageGateDistSq) { ClosestStageGateDistSq = DistSq; ClosestStageGate = G; }
		}
		else if (AT66CowardiceGate* CG = Cast<AT66CowardiceGate>(A))
		{
			if (DistSq < ClosestCowardiceGateDistSq) { ClosestCowardiceGateDistSq = DistSq; ClosestCowardiceGate = CG; }
		}
		else if (AT66DifficultyTotem* DT = Cast<AT66DifficultyTotem>(A))
		{
			if (DistSq < ClosestTotemDistSq) { ClosestTotemDistSq = DistSq; ClosestTotem = DT; }
		}
		else if (AT66HouseNPCBase* N = Cast<AT66HouseNPCBase>(A))
		{
			if (DistSq < ClosestNPCDistSq) { ClosestNPCDistSq = DistSq; ClosestNPC = N; }
		}
		else if (AT66RecruitableCompanion* RC = Cast<AT66RecruitableCompanion>(A))
		{
			if (DistSq < ClosestRecruitableCompanionDistSq) { ClosestRecruitableCompanionDistSq = DistSq; ClosestRecruitableCompanion = RC; }
		}
		else if (AT66ItemPickup* P = Cast<AT66ItemPickup>(A))
		{
			if (DistSq < ClosestPickupDistSq) { ClosestPickupDistSq = DistSq; ClosestPickup = P; }
		}
		else if (AT66LootBagPickup* Bag = Cast<AT66LootBagPickup>(A))
		{
			if (DistSq < ClosestLootBagDistSq) { ClosestLootBagDistSq = DistSq; ClosestLootBag = Bag; }
		}
		else if (AT66TutorialPortal* Portal = Cast<AT66TutorialPortal>(A))
		{
			if (DistSq < ClosestTutorialPortalDistSq) { ClosestTutorialPortalDistSq = DistSq; ClosestTutorialPortal = Portal; }
		}
		else if (AT66IdolAltar* Altar = Cast<AT66IdolAltar>(A))
		{
			if (DistSq < ClosestIdolAltarDistSq) { ClosestIdolAltarDistSq = DistSq; ClosestIdolAltar = Altar; }
		}
		else if (AT66TreeOfLifeInteractable* Tree = Cast<AT66TreeOfLifeInteractable>(A))
		{
			if (DistSq < ClosestTreeDistSq) { ClosestTreeDistSq = DistSq; ClosestTree = Tree; }
		}
		else if (AT66CashTruckInteractable* Truck = Cast<AT66CashTruckInteractable>(A))
		{
			if (DistSq < ClosestTruckDistSq) { ClosestTruckDistSq = DistSq; ClosestTruck = Truck; }
		}
		else if (AT66WheelSpinInteractable* W = Cast<AT66WheelSpinInteractable>(A))
		{
			if (DistSq < ClosestWheelDistSq) { ClosestWheelDistSq = DistSq; ClosestWheel = W; }
		}
		else if (AT66StageBoostGate* BG = Cast<AT66StageBoostGate>(A))
		{
			if (DistSq < ClosestBoostGateDistSq) { ClosestBoostGateDistSq = DistSq; ClosestBoostGate = BG; }
		}
		else if (AT66StageBoostGoldInteractable* BoostGold = Cast<AT66StageBoostGoldInteractable>(A))
		{
			if (DistSq < ClosestBoostGoldDistSq) { ClosestBoostGoldDistSq = DistSq; ClosestBoostGold = BoostGold; }
		}
		else if (AT66StageBoostLootInteractable* L = Cast<AT66StageBoostLootInteractable>(A))
		{
			if (DistSq < ClosestBoostLootDistSq) { ClosestBoostLootDistSq = DistSq; ClosestBoostLoot = L; }
		}
	}

	// Recruitable companion should take priority over stage gate so you can recruit right after boss death.
	if (ClosestRecruitableCompanion && ClosestRecruitableCompanion->Interact(this))
	{
		return;
	}

	// Interact with Stage Gate (F) advances to next stage and reloads level
	if (ClosestStageGate && ClosestStageGate->AdvanceToNextStage())
	{
		return;
	}

	// Tutorial portal (teleport within the same map; no load)
	if (ClosestTutorialPortal && ClosestTutorialPortal->Interact(this))
	{
		return;
	}

	// Stage Boost gate (F) enters the chosen difficulty start stage
	if (ClosestBoostGate && ClosestBoostGate->EnterChosenStage())
	{
		return;
	}

	// Difficulty totem (F) increases difficulty tier
	if (ClosestTotem && ClosestTotem->Interact(this))
	{
		return;
	}

	// Cowardice Gate (F) opens yes/no prompt
	if (ClosestCowardiceGate && ClosestCowardiceGate->Interact(this))
	{
		return;
	}

	// NPCs (Vendor/Gambler/Saint/Ouroboros)
	if (ClosestNPC && ClosestNPC->Interact(this))
	{
		return;
	}
	// Idol Altar (Stage 1)
	if (ClosestIdolAltar)
	{
		UT66IdolAltarOverlayWidget* W = CreateWidget<UT66IdolAltarOverlayWidget>(this, UT66IdolAltarOverlayWidget::StaticClass());
		if (W)
		{
			IdolAltarOverlayWidget = W;
			W->AddToViewport(150); // above gambler overlay
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		return;
	}
	// World interactables (Tree/Truck/Wheel)
	if (ClosestTree && ClosestTree->Interact(this))
	{
		return;
	}
	if (ClosestTruck && ClosestTruck->Interact(this))
	{
		return;
	}
	if (ClosestWheel && ClosestWheel->Interact(this))
	{
		return;
	}

	// Stage Boost interactables (Gold/Loot)
	if (ClosestBoostGold && ClosestBoostGold->Interact(this))
	{
		return;
	}
	if (ClosestBoostLoot && ClosestBoostLoot->Interact(this))
	{
		return;
	}
	if (ClosestLootBag)
	{
		UT66RunStateSubsystem* RunState = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		if (RunState && RunState->HasInventorySpace())
		{
			RunState->AddItem(ClosestLootBag->GetItemID());
			ClosestLootBag->ConsumeAndDestroy();
			ClearNearbyLootBag(ClosestLootBag);
		}
		return;
	}
	// Backward-compat: if any old pickups exist, collect instantly (no popup).
	if (ClosestPickup)
	{
		if (UT66RunStateSubsystem* RunState = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			if (RunState->HasInventorySpace())
			{
				RunState->AddItem(ClosestPickup->GetItemID());
				ClosestPickup->Destroy();
			}
			return;
		}
	}
}

void AT66PlayerController::OpenGamblerOverlay(int32 WinGoldAmount)
{
	if (!IsGameplayLevel()) return;

	if (!GamblerOverlayWidget)
	{
		GamblerOverlayWidget = CreateWidget<UT66GamblerOverlayWidget>(this, UT66GamblerOverlayWidget::StaticClass());
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

void AT66PlayerController::OpenVendorOverlay()
{
	if (!IsGameplayLevel()) return;

	if (!VendorOverlayWidget)
	{
		VendorOverlayWidget = CreateWidget<UT66VendorOverlayWidget>(this, UT66VendorOverlayWidget::StaticClass());
	}

	if (VendorOverlayWidget && !VendorOverlayWidget->IsInViewport())
	{
		VendorOverlayWidget->AddToViewport(100); // above HUD
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AT66PlayerController::OpenCollectorOverlay()
{
	if (!IsGameplayLevel()) return;

	if (!CollectorOverlayWidget)
	{
		CollectorOverlayWidget = CreateWidget<UT66CollectorOverlayWidget>(this, UT66CollectorOverlayWidget::StaticClass());
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

void AT66PlayerController::OpenWorldDialogueVendor(AT66VendorNPC* Vendor)
{
	if (!IsGameplayLevel()) return;
	if (!GameplayHUDWidget) return;
	if (!Vendor) return;

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	const TArray<FText> Options =
	{
		Loc ? Loc->GetText_IWantToShop() : NSLOCTEXT("T66.Vendor", "IWantToShop", "I want to shop"),
		Loc ? Loc->GetText_TeleportMeToYourBrother() : NSLOCTEXT("T66.Gambler", "TeleportMeToYourBrother", "Teleport me to your brother"),
		Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "Back"),
	};

	WorldDialogueKind = ET66WorldDialogueKind::Vendor;
	WorldDialogueTargetNPC = Vendor;
	WorldDialogueTargetCompanion.Reset();
	WorldDialogueSelectedIndex = 0;
	WorldDialogueNumOptions = 3;
	bWorldDialogueOpen = true;
	LastWorldDialogueNavTimeSeconds = -1000.f;

	GameplayHUDWidget->ShowWorldDialogue(Options, WorldDialogueSelectedIndex);
	UpdateWorldDialoguePosition();
	GetWorldTimerManager().SetTimer(WorldDialoguePositionTimerHandle, this, &AT66PlayerController::UpdateWorldDialoguePosition, 0.05f, true);
}

void AT66PlayerController::OpenWorldDialogueGambler(AT66GamblerNPC* Gambler)
{
	if (!IsGameplayLevel()) return;
	if (!GameplayHUDWidget) return;
	if (!Gambler) return;

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	const TArray<FText> Options =
	{
		Loc ? Loc->GetText_LetMeGamble() : NSLOCTEXT("T66.Gambler", "LetMeGamble", "Let me gamble"),
		Loc ? Loc->GetText_TeleportMeToYourBrother() : NSLOCTEXT("T66.Gambler", "TeleportMeToYourBrother", "Teleport me to your brother"),
		Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "Back"),
	};

	WorldDialogueKind = ET66WorldDialogueKind::Gambler;
	WorldDialogueTargetNPC = Gambler;
	WorldDialogueTargetCompanion.Reset();
	WorldDialogueSelectedIndex = 0;
	WorldDialogueNumOptions = 3;
	bWorldDialogueOpen = true;
	LastWorldDialogueNavTimeSeconds = -1000.f;

	GameplayHUDWidget->ShowWorldDialogue(Options, WorldDialogueSelectedIndex);
	UpdateWorldDialoguePosition();
	GetWorldTimerManager().SetTimer(WorldDialoguePositionTimerHandle, this, &AT66PlayerController::UpdateWorldDialoguePosition, 0.05f, true);
}

void AT66PlayerController::OpenWorldDialogueCompanion(AT66RecruitableCompanion* Companion)
{
	if (!IsGameplayLevel()) return;
	if (!GameplayHUDWidget) return;
	if (!Companion) return;

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	const TArray<FText> Options =
	{
		Loc ? Loc->GetText_FollowMe() : NSLOCTEXT("T66.CompanionRecruit", "FollowMe", "Follow me"),
		Loc ? Loc->GetText_Leave() : NSLOCTEXT("T66.CompanionRecruit", "Leave", "Leave"),
	};

	WorldDialogueKind = ET66WorldDialogueKind::Companion;
	WorldDialogueTargetNPC.Reset();
	WorldDialogueTargetCompanion = Companion;
	WorldDialogueSelectedIndex = 0;
	WorldDialogueNumOptions = 2;
	bWorldDialogueOpen = true;
	LastWorldDialogueNavTimeSeconds = -1000.f;

	GameplayHUDWidget->ShowWorldDialogue(Options, WorldDialogueSelectedIndex);
	UpdateWorldDialoguePosition();
	GetWorldTimerManager().SetTimer(WorldDialoguePositionTimerHandle, this, &AT66PlayerController::UpdateWorldDialoguePosition, 0.05f, true);
}

void AT66PlayerController::CloseWorldDialogue()
{
	bWorldDialogueOpen = false;
	WorldDialogueKind = ET66WorldDialogueKind::None;
	WorldDialogueSelectedIndex = 0;
	WorldDialogueNumOptions = 3;
	WorldDialogueTargetNPC.Reset();
	WorldDialogueTargetCompanion.Reset();
	GetWorldTimerManager().ClearTimer(WorldDialoguePositionTimerHandle);

	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->HideWorldDialogue();
	}
}

void AT66PlayerController::NavigateWorldDialogue(int32 Delta)
{
	if (!bWorldDialogueOpen) return;
	const int32 MaxIndex = FMath::Max(0, WorldDialogueNumOptions - 1);
	WorldDialogueSelectedIndex = FMath::Clamp(WorldDialogueSelectedIndex + Delta, 0, MaxIndex);
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->SetWorldDialogueSelection(WorldDialogueSelectedIndex);
	}
}

void AT66PlayerController::ConfirmWorldDialogue()
{
	if (!bWorldDialogueOpen) return;

	// Capture before we close (closing clears state).
	const ET66WorldDialogueKind Kind = WorldDialogueKind;
	const int32 Choice = WorldDialogueSelectedIndex;
	AT66HouseNPCBase* TargetNPC = WorldDialogueTargetNPC.Get();
	AT66RecruitableCompanion* TargetCompanion = WorldDialogueTargetCompanion.Get();
	const int32 GamblerWinGoldAmount = (Kind == ET66WorldDialogueKind::Gambler)
		? (Cast<AT66GamblerNPC>(TargetNPC) ? Cast<AT66GamblerNPC>(TargetNPC)->GetWinGoldAmount() : 0)
		: 0;

	CloseWorldDialogue();

	if (Kind == ET66WorldDialogueKind::Companion)
	{
		// 0: follow me, 1: leave
		if (Choice != 0)
		{
			return;
		}

		if (!TargetCompanion || TargetCompanion->CompanionID.IsNone())
		{
			return;
		}

		if (AT66GameMode* GM = GetWorld() ? Cast<AT66GameMode>(UGameplayStatics::GetGameMode(this)) : nullptr)
		{
			const FName NewCompanionID = TargetCompanion->CompanionID;
			if (GM->SwapCompanionForPlayer(this, NewCompanionID))
			{
				TargetCompanion->Destroy();
			}
		}
		return;
	}

	// Vendor/Gambler: 0: main action, 1: teleport, 2: back
	if (Choice == 2)
	{
		return;
	}

	if (Choice == 1)
	{
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
				{
					// Mirror overlay behavior: no teleport once boss encounter starts.
					if (RunState->GetBossActive())
					{
						return;
					}
				}
			}
		}

		TeleportToNPC((Kind == ET66WorldDialogueKind::Vendor) ? FName(TEXT("Gambler")) : FName(TEXT("Vendor")));
		return;
	}

	// Choice == 0
	if (Kind == ET66WorldDialogueKind::Vendor)
	{
		OpenVendorOverlay();
		if (VendorOverlayWidget)
		{
			// Skip overlay dialogue page; go straight to shop.
			VendorOverlayWidget->OpenShopPage();
		}
	}
	else if (Kind == ET66WorldDialogueKind::Gambler)
	{
		OpenGamblerOverlay(GamblerWinGoldAmount);
		if (GamblerOverlayWidget)
		{
			// Skip overlay dialogue page; go straight to casino.
			GamblerOverlayWidget->OpenCasinoPage();
		}
	}
}

void AT66PlayerController::UpdateWorldDialoguePosition()
{
	if (!bWorldDialogueOpen) return;
	if (!GameplayHUDWidget) return;

	AActor* AnchorActor = nullptr;
	if (WorldDialogueKind == ET66WorldDialogueKind::Companion)
	{
		AnchorActor = WorldDialogueTargetCompanion.Get();
	}
	else
	{
		AnchorActor = WorldDialogueTargetNPC.Get();
	}

	if (!AnchorActor)
	{
		CloseWorldDialogue();
		return;
	}

	FVector2D ScreenPos(0.f, 0.f);
	const FVector WorldAnchor = AnchorActor->GetActorLocation() + FVector(0.f, 0.f, 180.f);
	if (!ProjectWorldLocationToScreen(WorldAnchor, ScreenPos, true))
	{
		return;
	}

	// Nudge to the right of the NPC (like the sketch), slightly upward.
	ScreenPos += FVector2D(140.f, -40.f);

	int32 VX = 0, VY = 0;
	GetViewportSize(VX, VY);
	ScreenPos.X = FMath::Clamp(ScreenPos.X, 12.f, FMath::Max(12.f, static_cast<float>(VX) - 340.f));
	ScreenPos.Y = FMath::Clamp(ScreenPos.Y, 12.f, FMath::Max(12.f, static_cast<float>(VY) - 180.f));

	GameplayHUDWidget->SetWorldDialogueScreenPosition(ScreenPos);
}

void AT66PlayerController::TeleportToNPC(FName NPCID)
{
	UWorld* World = GetWorld();
	if (!World) return;
	APawn* PlayerPawn = GetPawn();
	if (!PlayerPawn) return;

	AT66HouseNPCBase* Target = nullptr;
	for (TActorIterator<AT66HouseNPCBase> It(World); It; ++It)
	{
		AT66HouseNPCBase* N = *It;
		if (N && N->NPCID == NPCID)
		{
			Target = N;
			break;
		}
	}
	if (!Target) return;

	PlayerPawn->SetActorLocation(Target->GetActorLocation() + FVector(250.f, 0.f, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
}

void AT66PlayerController::OpenCowardicePrompt(AT66CowardiceGate* Gate)
{
	if (!IsGameplayLevel() || !Gate) return;

	if (!CowardicePromptWidget)
	{
		CowardicePromptWidget = CreateWidget<UT66CowardicePromptWidget>(this, UT66CowardicePromptWidget::StaticClass());
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

void AT66PlayerController::StartWheelSpinHUD(ET66Rarity Rarity)
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->StartWheelSpin(Rarity);
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

void AT66PlayerController::OnPlayerDied()
{
	// Hard rule: if TikTok/media viewer is open, force-close it on death.
	// This ensures the overlay is hidden and audio cannot continue during the run summary.
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
	// Persist Power Crystals earned this run before showing Run Summary.
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		UT66PowerUpSubsystem* PowerUpSub = GI->GetSubsystem<UT66PowerUpSubsystem>();
		if (RunState && PowerUpSub)
		{
			const int32 Earned = RunState->GetPowerCrystalsEarnedThisRun();
			if (Earned > 0)
			{
				PowerUpSub->AddPowerCrystals(Earned);
			}
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

void AT66PlayerController::EnsureGameplayUIManager()
{
	if (UIManager || !IsGameplayLevel()) return;

	UIManager = NewObject<UT66UIManager>(this, UT66UIManager::StaticClass());
	if (!UIManager) return;

	UIManager->Initialize(this);
	UIManager->RegisterScreenClass(ET66ScreenType::PauseMenu, UT66PauseMenuScreen::StaticClass());
	UIManager->RegisterScreenClass(ET66ScreenType::ReportBug, UT66ReportBugScreen::StaticClass());
	UIManager->RegisterScreenClass(ET66ScreenType::Settings, UT66SettingsScreen::StaticClass());
	UIManager->RegisterScreenClass(ET66ScreenType::RunSummary, UT66RunSummaryScreen::StaticClass());
	UIManager->RegisterScreenClass(ET66ScreenType::PlayerSummaryPicker, UT66PlayerSummaryPickerScreen::StaticClass());
	UIManager->RegisterScreenClass(ET66ScreenType::PowerUp, UT66PowerUpScreen::StaticClass());
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

	// Debounce to avoid key repeat / rapid presses freezing or flickering
	const float Now = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds()) : 0.f;
	if (Now - LastPauseToggleTime < PauseToggleDebounceSeconds)
	{
		return;
	}
	LastPauseToggleTime = Now;

	const bool bPaused = IsPaused();
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
			const ET66ScreenType ClosingModal = UIManager->GetCurrentModalType();
			UIManager->CloseModal();
			// Esc as back: Settings or Report Bug -> return to Pause Menu; Pause Menu -> unpause
			if (ClosingModal == ET66ScreenType::Settings || ClosingModal == ET66ScreenType::ReportBug)
			{
				UIManager->ShowModal(ET66ScreenType::PauseMenu);
			}
			else
			{
				SetPause(false);
				RestoreGameplayInputMode();
			}
		}
		else
		{
			SetPause(false);
			RestoreGameplayInputMode();
		}
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
	AddPitchInput(Value);
}

void AT66PlayerController::HandleTurn(float Value)
{
	if (!IsGameplayLevel() || FMath::IsNearlyZero(Value)) return;
	// When the gameplay cursor is visible, mouse movement should not rotate the camera.
	if (bShowMouseCursor) return;
	AddYawInput(Value);
}

void AT66PlayerController::HandleJumpPressed()
{
	if (!IsGameplayLevel()) return;

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
		MyCharacter->Jump();

		// Jump VFX: spawn a few puffs at the character's feet (same Niagara as slash).
		UWorld* World = GetWorld();
		if (World && CachedJumpVFXNiagara)
		{
			const FVector FeetLoc = MyCharacter->GetActorLocation() - FVector(0.f, 0.f, 50.f);
			const FVector4 TintWhite(1.f, 1.f, 1.f, 1.f);
			for (int32 i = 0; i < 3; ++i)
			{
				UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					World, CachedJumpVFXNiagara, FeetLoc, FRotator::ZeroRotator,
					FVector(1.f), true, true, ENCPoolMethod::None);
				if (NC)
				{
					NC->SetVariableVec4(FName(TEXT("User.Tint")), TintWhite);
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
	
	// Adjust camera boom length on the possessed hero (third-person zoom)
	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;
	
	AT66HeroBase* Hero = Cast<AT66HeroBase>(MyPawn);
	if (Hero && Hero->CameraBoom)
	{
		const float ZoomSpeed = 25.f;
		const float MinLength = 200.f;
		const float MaxLength = 2400.f;  // Max zoom-out (scroll wheel limit; doubled from 1200)
		float NewLength = Hero->CameraBoom->TargetArmLength - Value * ZoomSpeed;
		NewLength = FMath::Clamp(NewLength, MinLength, MaxLength);
		Hero->CameraBoom->TargetArmLength = NewLength;
	}
}

void AT66PlayerController::AutoLoadScreenClasses()
{
	// Screen type to asset path mapping
	struct FScreenPathMapping
	{
		ET66ScreenType Type;
		const TCHAR* AssetPath;
	};

	static const FScreenPathMapping ScreenPaths[] = {
		{ ET66ScreenType::MainMenu, TEXT("/Game/Blueprints/UI/WBP_MainMenu.WBP_MainMenu_C") },
		{ ET66ScreenType::PartySizePicker, TEXT("/Game/Blueprints/UI/WBP_PartySizePicker.WBP_PartySizePicker_C") },
		{ ET66ScreenType::HeroSelection, TEXT("/Game/Blueprints/UI/WBP_HeroSelection.WBP_HeroSelection_C") },
		{ ET66ScreenType::CompanionSelection, TEXT("/Game/Blueprints/UI/WBP_CompanionSelection.WBP_CompanionSelection_C") },
		{ ET66ScreenType::SaveSlots, TEXT("/Game/Blueprints/UI/WBP_SaveSlots.WBP_SaveSlots_C") },
		{ ET66ScreenType::Settings, TEXT("/Game/Blueprints/UI/WBP_Settings.WBP_Settings_C") },
		{ ET66ScreenType::QuitConfirmation, TEXT("/Game/Blueprints/UI/WBP_QuitConfirmation.WBP_QuitConfirmation_C") },
	};

	for (const auto& Mapping : ScreenPaths)
	{
		// Skip if already registered
		if (ScreenClasses.Contains(Mapping.Type) && ScreenClasses[Mapping.Type] != nullptr)
		{
			continue;
		}

		// Try to load the class
		UClass* LoadedClass = LoadClass<UT66ScreenBase>(nullptr, Mapping.AssetPath);
		if (LoadedClass)
		{
			ScreenClasses.Add(Mapping.Type, LoadedClass);
			UE_LOG(LogTemp, Log, TEXT("Auto-loaded screen class: %s"), Mapping.AssetPath);
		}
		else
		{
			if (Mapping.Type == ET66ScreenType::HeroGrid)
			{
				ScreenClasses.Add(ET66ScreenType::HeroGrid, UT66HeroGridScreen::StaticClass());
				UE_LOG(LogTemp, Log, TEXT("Using C++ HeroGrid screen (WBP not found)"));
			}
			else if (Mapping.Type == ET66ScreenType::CompanionGrid)
			{
				ScreenClasses.Add(ET66ScreenType::CompanionGrid, UT66CompanionGridScreen::StaticClass());
				UE_LOG(LogTemp, Log, TEXT("Using C++ CompanionGrid screen (WBP not found)"));
			}
			else if (Mapping.Type == ET66ScreenType::SaveSlots)
			{
				ScreenClasses.Add(ET66ScreenType::SaveSlots, UT66SaveSlotsScreen::StaticClass());
				UE_LOG(LogTemp, Log, TEXT("Using C++ SaveSlots screen (WBP not found)"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to auto-load screen class: %s"), Mapping.AssetPath);
			}
		}
	}

	// HeroGrid/CompanionGrid are C++ screens by design (no optional WBP overrides).
	if (!ScreenClasses.Contains(ET66ScreenType::HeroGrid) || ScreenClasses[ET66ScreenType::HeroGrid] == nullptr)
	{
		ScreenClasses.Add(ET66ScreenType::HeroGrid, UT66HeroGridScreen::StaticClass());
	}
	if (!ScreenClasses.Contains(ET66ScreenType::CompanionGrid) || ScreenClasses[ET66ScreenType::CompanionGrid] == nullptr)
	{
		ScreenClasses.Add(ET66ScreenType::CompanionGrid, UT66CompanionGridScreen::StaticClass());
	}
	if (!ScreenClasses.Contains(ET66ScreenType::Lobby) || ScreenClasses[ET66ScreenType::Lobby] == nullptr)
	{
		ScreenClasses.Add(ET66ScreenType::Lobby, UT66LobbyScreen::StaticClass());
	}
	if (!ScreenClasses.Contains(ET66ScreenType::LobbyReadyCheck) || ScreenClasses[ET66ScreenType::LobbyReadyCheck] == nullptr)
	{
		ScreenClasses.Add(ET66ScreenType::LobbyReadyCheck, UT66LobbyReadyCheckModal::StaticClass());
	}
	if (!ScreenClasses.Contains(ET66ScreenType::LobbyBackConfirm) || ScreenClasses[ET66ScreenType::LobbyBackConfirm] == nullptr)
	{
		ScreenClasses.Add(ET66ScreenType::LobbyBackConfirm, UT66LobbyBackConfirmModal::StaticClass());
	}
}

void AT66PlayerController::InitializeUI()
{
	if (bUIInitialized)
	{
		return;
	}

	// Auto-load screen classes if enabled
	if (bAutoLoadScreenClasses)
	{
		AutoLoadScreenClasses();
	}

	// Create the UI Manager
	UIManager = NewObject<UT66UIManager>(this, UT66UIManager::StaticClass());
	if (!UIManager)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create UI Manager!"));
		return;
	}

	UIManager->Initialize(this);

	// Register all screen classes
	for (const auto& Pair : ScreenClasses)
	{
		if (Pair.Value)
		{
			UIManager->RegisterScreenClass(Pair.Key, Pair.Value);
			UE_LOG(LogTemp, Log, TEXT("Registered screen class for type %d"), static_cast<int32>(Pair.Key));
		}
	}

	// Ensure core modal screens are available in frontend too (leaderboard row opens Run Summary / Pick the Player).
	UIManager->RegisterScreenClass(ET66ScreenType::RunSummary, UT66RunSummaryScreen::StaticClass());
	UIManager->RegisterScreenClass(ET66ScreenType::PlayerSummaryPicker, UT66PlayerSummaryPickerScreen::StaticClass());
	UIManager->RegisterScreenClass(ET66ScreenType::PowerUp, UT66PowerUpScreen::StaticClass());
	// Account Status is a C++ modal by default (no WBP required). If a WBP is registered, do not override it.
	if (!ScreenClasses.Contains(ET66ScreenType::AccountStatus) || ScreenClasses[ET66ScreenType::AccountStatus] == nullptr)
	{
		UIManager->RegisterScreenClass(ET66ScreenType::AccountStatus, UT66AccountStatusScreen::StaticClass());
	}

	bUIInitialized = true;

	// Show the initial screen
	if (InitialScreen != ET66ScreenType::None)
	{
		UIManager->ShowScreen(InitialScreen);
	}
}

void AT66PlayerController::ShowScreen(ET66ScreenType ScreenType)
{
	if (!bUIInitialized)
	{
		InitializeUI();
	}

	if (UIManager)
	{
		UIManager->ShowScreen(ScreenType);
	}
}
