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
#include "UI/T66GameplayHUDWidget.h"
#include "UI/T66LabOverlayWidget.h"
#include "UI/T66CasinoOverlayWidget.h"
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
	UIManager->RegisterScreenClass(ET66ScreenType::Leaderboard, UT66LeaderboardScreen::StaticClass());
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
