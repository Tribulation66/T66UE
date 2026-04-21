// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66FrontendGameMode.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66CompanionPreviewStage.h"
#include "Gameplay/T66CombatComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI/T66UIManager.h"
#include "UI/T66ScreenBase.h"
#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/Screens/T66HeroGridScreen.h"
#include "UI/Screens/T66CompanionGridScreen.h"
#include "UI/Screens/T66SaveSlotsScreen.h"
#include "UI/Screens/T66AchievementsScreen.h"
#include "UI/Screens/T66PauseMenuScreen.h"
#include "UI/Screens/T66ReportBugScreen.h"
#include "UI/Screens/T66SettingsScreen.h"
#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/Screens/T66PlayerSummaryPickerScreen.h"
#include "UI/Screens/T66SavePreviewScreen.h"
#include "UI/Screens/T66ShopScreen.h"
#include "UI/Screens/T66TemporaryBuffSelectionScreen.h"
#include "UI/Screens/T66TemporaryBuffShopScreen.h"
#include "UI/Screens/T66UnlocksScreen.h"
#include "UI/Screens/T66SnakeGameModal.h"
#include "UI/Screens/T66LeaderboardScreen.h"
#include "UI/Screens/T66AccountStatusScreen.h"
#include "UI/Screens/T66ChallengesScreen.h"
#include "UI/Screens/T66DailyClimbScreen.h"
#include "UI/Screens/T66PartyInviteModal.h"
#include "UI/T66GameplayHUDWidget.h"
#include "UI/T66LabOverlayWidget.h"
#include "UI/T66CircusOverlayWidget.h"
#include "UI/T66GamblerOverlayWidget.h"
#include "UI/T66CowardicePromptWidget.h"
#include "UI/T66IdolAltarOverlayWidget.h"
#include "UI/T66VendorOverlayWidget.h"
#include "UI/T66CollectorOverlayWidget.h"
#include "UI/T66CrateOverlayWidget.h"
#include "Core/T66SessionSubsystem.h"
#include "Gameplay/T66FountainOfLifeInteractable.h"
#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66WheelSpinInteractable.h"
#include "Gameplay/T66CrateInteractable.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Frontend, Log, All);
#include "Gameplay/T66CasinoInteractable.h"
#include "Gameplay/T66PilotableTractor.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "Gameplay/T66StageCatchUpGate.h"
#include "Gameplay/T66StageCatchUpGoldInteractable.h"
#include "Gameplay/T66StageCatchUpLootInteractable.h"
#include "Gameplay/T66TutorialPortal.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66MediaViewerSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "UI/T66LoadingScreenWidget.h"
#include "UI/Style/T66Style.h"
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
#include "HAL/FileManager.h"
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
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include "HAL/PlatformMisc.h"
#include "Camera/CameraActor.h"
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

namespace
{
	bool TryResolveFrontendScreenName(const FString& ScreenName, ET66ScreenType& OutScreenType)
	{
		const FString Normalized = ScreenName.TrimStartAndEnd();
		if (Normalized.Equals(TEXT("MainMenu"), ESearchCase::IgnoreCase))
		{
			OutScreenType = ET66ScreenType::MainMenu;
			return true;
		}
		if (Normalized.Equals(TEXT("Unlocks"), ESearchCase::IgnoreCase) || Normalized.Equals(TEXT("Minigames"), ESearchCase::IgnoreCase))
		{
			OutScreenType = ET66ScreenType::Unlocks;
			return true;
		}
		if (Normalized.Equals(TEXT("MiniMainMenu"), ESearchCase::IgnoreCase))
		{
			OutScreenType = ET66ScreenType::MiniMainMenu;
			return true;
		}
		if (Normalized.Equals(TEXT("MiniCharacterSelect"), ESearchCase::IgnoreCase))
		{
			OutScreenType = ET66ScreenType::MiniCharacterSelect;
			return true;
		}
		if (Normalized.Equals(TEXT("MiniCompanionSelect"), ESearchCase::IgnoreCase))
		{
			OutScreenType = ET66ScreenType::MiniCompanionSelect;
			return true;
		}
		if (Normalized.Equals(TEXT("MiniDifficultySelect"), ESearchCase::IgnoreCase))
		{
			OutScreenType = ET66ScreenType::MiniDifficultySelect;
			return true;
		}
		if (Normalized.Equals(TEXT("MiniIdolSelect"), ESearchCase::IgnoreCase))
		{
			OutScreenType = ET66ScreenType::MiniIdolSelect;
			return true;
		}
		if (Normalized.Equals(TEXT("MiniSaveSlots"), ESearchCase::IgnoreCase))
		{
			OutScreenType = ET66ScreenType::MiniSaveSlots;
			return true;
		}
		if (Normalized.Equals(TEXT("TDMainMenu"), ESearchCase::IgnoreCase))
		{
			OutScreenType = ET66ScreenType::TDMainMenu;
			return true;
		}
		if (Normalized.Equals(TEXT("TDDifficultySelect"), ESearchCase::IgnoreCase))
		{
			OutScreenType = ET66ScreenType::TDDifficultySelect;
			return true;
		}
		if (Normalized.Equals(TEXT("TDBattle"), ESearchCase::IgnoreCase))
		{
			OutScreenType = ET66ScreenType::TDBattle;
			return true;
		}
		if (Normalized.Equals(TEXT("Challenges"), ESearchCase::IgnoreCase))
		{
			OutScreenType = ET66ScreenType::Challenges;
			return true;
		}
		if (Normalized.Equals(TEXT("DailyClimb"), ESearchCase::IgnoreCase))
		{
			OutScreenType = ET66ScreenType::DailyClimb;
			return true;
		}

		return false;
	}
}

TSubclassOf<UT66ScreenBase> AT66PlayerController::ResolveScreenClass(ET66ScreenType ScreenType) const
{
	if (FT66Style::IsDotaTheme())
	{
		if (const TSubclassOf<UT66ScreenBase>* DotaClass = DotaScreenClasses.Find(ScreenType))
		{
			if (*DotaClass)
			{
				return *DotaClass;
			}
		}
	}

	if (ScreenType == ET66ScreenType::HeroSelection)
	{
		return UT66HeroSelectionScreen::StaticClass();
	}

	if (const TSubclassOf<UT66ScreenBase>* ScreenClass = ScreenClasses.Find(ScreenType))
	{
		return *ScreenClass;
	}

	switch (ScreenType)
	{
	case ET66ScreenType::HeroGrid:
		return UT66HeroGridScreen::StaticClass();
	case ET66ScreenType::CompanionGrid:
		return UT66CompanionGridScreen::StaticClass();
	case ET66ScreenType::SaveSlots:
		return UT66SaveSlotsScreen::StaticClass();
	case ET66ScreenType::Lobby:
	case ET66ScreenType::LobbyReadyCheck:
	case ET66ScreenType::LobbyBackConfirm:
		if (const TSubclassOf<UT66ScreenBase>* MainMenuClass = ScreenClasses.Find(ET66ScreenType::MainMenu))
		{
			return *MainMenuClass;
		}
		return nullptr;
	case ET66ScreenType::PauseMenu:
		return UT66PauseMenuScreen::StaticClass();
	case ET66ScreenType::Achievements:
		return UT66AchievementsScreen::StaticClass();
	case ET66ScreenType::Unlocks:
		return UT66UnlocksScreen::StaticClass();
	case ET66ScreenType::SnakeGame:
		return UT66SnakeGameModal::StaticClass();
	case ET66ScreenType::MiniMainMenu:
		return LoadClass<UT66ScreenBase>(nullptr, TEXT("/Script/T66Mini.T66MiniMainMenuScreen"));
	case ET66ScreenType::MiniSaveSlots:
		return LoadClass<UT66ScreenBase>(nullptr, TEXT("/Script/T66Mini.T66MiniSaveSlotsScreen"));
	case ET66ScreenType::MiniCharacterSelect:
		return LoadClass<UT66ScreenBase>(nullptr, TEXT("/Script/T66Mini.T66MiniCharacterSelectScreen"));
	case ET66ScreenType::MiniCompanionSelect:
		return LoadClass<UT66ScreenBase>(nullptr, TEXT("/Script/T66Mini.T66MiniCompanionSelectScreen"));
	case ET66ScreenType::MiniDifficultySelect:
		return LoadClass<UT66ScreenBase>(nullptr, TEXT("/Script/T66Mini.T66MiniDifficultySelectScreen"));
	case ET66ScreenType::MiniIdolSelect:
		return LoadClass<UT66ScreenBase>(nullptr, TEXT("/Script/T66Mini.T66MiniIdolSelectScreen"));
	case ET66ScreenType::MiniShop:
		return LoadClass<UT66ScreenBase>(nullptr, TEXT("/Script/T66Mini.T66MiniShopScreen"));
	case ET66ScreenType::MiniRunSummary:
		return LoadClass<UT66ScreenBase>(nullptr, TEXT("/Script/T66Mini.T66MiniRunSummaryScreen"));
	case ET66ScreenType::TDMainMenu:
		return LoadClass<UT66ScreenBase>(nullptr, TEXT("/Script/T66TD.T66TDMainMenuScreen"));
	case ET66ScreenType::TDDifficultySelect:
		return LoadClass<UT66ScreenBase>(nullptr, TEXT("/Script/T66TD.T66TDDifficultySelectScreen"));
	case ET66ScreenType::TDBattle:
		return LoadClass<UT66ScreenBase>(nullptr, TEXT("/Script/T66TD.T66TDBattleScreen"));
	case ET66ScreenType::ReportBug:
		return UT66ReportBugScreen::StaticClass();
	case ET66ScreenType::Settings:
		return UT66SettingsScreen::StaticClass();
	case ET66ScreenType::RunSummary:
		return UT66RunSummaryScreen::StaticClass();
	case ET66ScreenType::PlayerSummaryPicker:
		return UT66PlayerSummaryPickerScreen::StaticClass();
	case ET66ScreenType::SavePreview:
		return UT66SavePreviewScreen::StaticClass();
	case ET66ScreenType::PowerUp:
		return UT66ShopScreen::StaticClass();
	case ET66ScreenType::TemporaryBuffSelection:
		return UT66TemporaryBuffSelectionScreen::StaticClass();
	case ET66ScreenType::TemporaryBuffShop:
		return UT66TemporaryBuffShopScreen::StaticClass();
	case ET66ScreenType::Leaderboard:
		return UT66LeaderboardScreen::StaticClass();
	case ET66ScreenType::AccountStatus:
		return UT66AccountStatusScreen::StaticClass();
	case ET66ScreenType::Challenges:
		return UT66ChallengesScreen::StaticClass();
	case ET66ScreenType::DailyClimb:
		return UT66DailyClimbScreen::StaticClass();
	case ET66ScreenType::PartyInvite:
		return UT66PartyInviteModal::StaticClass();
	default:
		return nullptr;
	}
}

TSubclassOf<UT66GameplayHUDWidget> AT66PlayerController::ResolveGameplayHUDClass() const
{
	if (FT66Style::IsDotaTheme() && DotaGameplayHUDClass)
	{
		return DotaGameplayHUDClass;
	}

	return UT66GameplayHUDWidget::StaticClass();
}

TSubclassOf<UT66GamblerOverlayWidget> AT66PlayerController::ResolveGamblerOverlayClass() const
{
	if (FT66Style::IsDotaTheme() && DotaGamblerOverlayClass)
	{
		return DotaGamblerOverlayClass;
	}

	return UT66GamblerOverlayWidget::StaticClass();
}

TSubclassOf<UT66CircusOverlayWidget> AT66PlayerController::ResolveCircusOverlayClass() const
{
	if (FT66Style::IsDotaTheme() && DotaCircusOverlayClass)
	{
		return DotaCircusOverlayClass;
	}

	return UT66CircusOverlayWidget::StaticClass();
}

TSubclassOf<UT66VendorOverlayWidget> AT66PlayerController::ResolveVendorOverlayClass() const
{
	if (FT66Style::IsDotaTheme() && DotaVendorOverlayClass)
	{
		return DotaVendorOverlayClass;
	}

	return UT66VendorOverlayWidget::StaticClass();
}

TSubclassOf<UT66CollectorOverlayWidget> AT66PlayerController::ResolveCollectorOverlayClass() const
{
	if (FT66Style::IsDotaTheme() && DotaCollectorOverlayClass)
	{
		return DotaCollectorOverlayClass;
	}

	return UT66CollectorOverlayWidget::StaticClass();
}

TSubclassOf<UT66CowardicePromptWidget> AT66PlayerController::ResolveCowardicePromptClass() const
{
	if (FT66Style::IsDotaTheme() && DotaCowardicePromptClass)
	{
		return DotaCowardicePromptClass;
	}

	return UT66CowardicePromptWidget::StaticClass();
}

TSubclassOf<UT66IdolAltarOverlayWidget> AT66PlayerController::ResolveIdolAltarOverlayClass() const
{
	if (FT66Style::IsDotaTheme() && DotaIdolAltarOverlayClass)
	{
		return DotaIdolAltarOverlayClass;
	}

	return UT66IdolAltarOverlayWidget::StaticClass();
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
	UE_LOG(LogT66Frontend, Log, TEXT("DevConsole: opened"));
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
	UE_LOG(LogT66Frontend, Log, TEXT("DevConsole: closed"));
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
			UE_LOG(LogT66Frontend, Log, TEXT("Auto-loaded screen class: %s"), Mapping.AssetPath);
		}
		else
		{
			if (Mapping.Type == ET66ScreenType::HeroGrid)
			{
				ScreenClasses.Add(ET66ScreenType::HeroGrid, UT66HeroGridScreen::StaticClass());
				UE_LOG(LogT66Frontend, Log, TEXT("Using C++ HeroGrid screen (WBP not found)"));
			}
			else if (Mapping.Type == ET66ScreenType::CompanionGrid)
			{
				ScreenClasses.Add(ET66ScreenType::CompanionGrid, UT66CompanionGridScreen::StaticClass());
				UE_LOG(LogT66Frontend, Log, TEXT("Using C++ CompanionGrid screen (WBP not found)"));
			}
			else if (Mapping.Type == ET66ScreenType::SaveSlots)
			{
				ScreenClasses.Add(ET66ScreenType::SaveSlots, UT66SaveSlotsScreen::StaticClass());
				UE_LOG(LogT66Frontend, Log, TEXT("Using C++ SaveSlots screen (WBP not found)"));
			}
			else
			{
				UE_LOG(LogT66Frontend, Warning, TEXT("Failed to auto-load screen class: %s"), Mapping.AssetPath);
			}
		}
	}

	// HeroGrid/CompanionGrid are C++ screens by design (no optional WBP overrides).
	if (!ScreenClasses.Contains(ET66ScreenType::HeroSelection) || ScreenClasses[ET66ScreenType::HeroSelection] == nullptr)
	{
		ScreenClasses.Add(ET66ScreenType::HeroSelection, UT66HeroSelectionScreen::StaticClass());
	}
	if (!ScreenClasses.Contains(ET66ScreenType::HeroGrid) || ScreenClasses[ET66ScreenType::HeroGrid] == nullptr)
	{
		ScreenClasses.Add(ET66ScreenType::HeroGrid, UT66HeroGridScreen::StaticClass());
	}
	if (!ScreenClasses.Contains(ET66ScreenType::CompanionGrid) || ScreenClasses[ET66ScreenType::CompanionGrid] == nullptr)
	{
		ScreenClasses.Add(ET66ScreenType::CompanionGrid, UT66CompanionGridScreen::StaticClass());
	}
	ScreenClasses.Remove(ET66ScreenType::Lobby);
	ScreenClasses.Remove(ET66ScreenType::LobbyReadyCheck);
	ScreenClasses.Remove(ET66ScreenType::LobbyBackConfirm);
}

void AT66PlayerController::ApplyFrontendCommandLineOverrides(ET66ScreenType& ScreenToShow)
{
	FrontendAutomationScreenshotPath.Reset();
	FrontendAutomationScreenshotDelaySeconds = 0.f;
	bFrontendAutomationKeepAliveAfterScreenshot = false;

	FString RequestedScreenName;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66FrontendScreen="), RequestedScreenName))
	{
		ET66ScreenType RequestedScreenType = ET66ScreenType::None;
		if (TryResolveFrontendScreenName(RequestedScreenName, RequestedScreenType))
		{
			ScreenToShow = RequestedScreenType;
			UE_LOG(LogT66Frontend, Log, TEXT("Frontend automation: overriding startup screen to '%s'"), *RequestedScreenName);
		}
		else
		{
			UE_LOG(LogT66Frontend, Warning, TEXT("Frontend automation: unknown screen override '%s'"), *RequestedScreenName);
		}
	}

	FString RequestedScreenshotPath;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66AutoScreenshot="), RequestedScreenshotPath))
	{
		FrontendAutomationScreenshotPath = FPaths::ConvertRelativePathToFull(RequestedScreenshotPath);
		FrontendAutomationScreenshotDelaySeconds = 2.0f;
		FParse::Value(FCommandLine::Get(), TEXT("T66AutoScreenshotDelay="), FrontendAutomationScreenshotDelaySeconds);
		bFrontendAutomationKeepAliveAfterScreenshot = FParse::Param(FCommandLine::Get(), TEXT("T66KeepAliveAfterScreenshot"));
	}
}

void AT66PlayerController::QueueFrontendAutomationScreenshotIfRequested()
{
	if (!GetWorld() || FrontendAutomationScreenshotPath.IsEmpty())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(FrontendAutomationScreenshotTimerHandle);
	GetWorldTimerManager().SetTimer(
		FrontendAutomationScreenshotTimerHandle,
		this,
		&AT66PlayerController::HandleFrontendAutomationScreenshot,
		FMath::Max(0.1f, FrontendAutomationScreenshotDelaySeconds),
		false);

	UE_LOG(
		LogT66Frontend,
		Log,
		TEXT("Frontend automation: queued screenshot '%s' in %.2f seconds"),
		*FrontendAutomationScreenshotPath,
		FrontendAutomationScreenshotDelaySeconds);
}

void AT66PlayerController::HandleFrontendAutomationScreenshot()
{
	if (FrontendAutomationScreenshotPath.IsEmpty())
	{
		return;
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(FrontendAutomationScreenshotPath), true);
	FScreenshotRequest::RequestScreenshot(FrontendAutomationScreenshotPath, true, false, false);
	UE_LOG(LogT66Frontend, Log, TEXT("Frontend automation: requested screenshot '%s'"), *FrontendAutomationScreenshotPath);

	if (!bFrontendAutomationKeepAliveAfterScreenshot && GetWorld())
	{
		GetWorldTimerManager().ClearTimer(FrontendAutomationQuitTimerHandle);
		GetWorldTimerManager().SetTimer(
			FrontendAutomationQuitTimerHandle,
			this,
			&AT66PlayerController::HandleFrontendAutomationQuit,
			1.5f,
			false);
	}
}

void AT66PlayerController::HandleFrontendAutomationQuit()
{
	ConsoleCommand(TEXT("quit"));
}

void AT66PlayerController::EnsureFrontendStartupOverlay(const FText& LoadingText)
{
	if (FrontendStartupOverlayWidget)
	{
		FrontendStartupOverlayWidget->SetLoadingText(LoadingText);
		return;
	}

	if (UT66LoadingScreenWidget* Overlay = CreateWidget<UT66LoadingScreenWidget>(this, UT66LoadingScreenWidget::StaticClass()))
	{
		Overlay->SetLoadingText(LoadingText);
		Overlay->AddToViewport(10000);
		FrontendStartupOverlayWidget = Overlay;
	}
}

void AT66PlayerController::HideFrontendStartupOverlay()
{
	if (FrontendStartupOverlayWidget)
	{
		FrontendStartupOverlayWidget->RemoveFromParent();
		FrontendStartupOverlayWidget = nullptr;
	}
}

void AT66PlayerController::StartFrontendLaunchPolicyCheck()
{
	if (bFrontendLaunchPolicyCheckComplete)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FrontendLaunchPolicyTimeoutTimerHandle);
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			if (FrontendLaunchPolicyResponseHandle.IsValid())
			{
				Backend->OnClientLaunchPolicyResponse().Remove(FrontendLaunchPolicyResponseHandle);
				FrontendLaunchPolicyResponseHandle.Reset();
			}
		}
	}

	bFrontendLaunchPolicyCheckStarted = false;
	bFrontendLaunchPolicyCheckComplete = true;
	bFrontendLaunchPolicyUpdateBlocked = false;
	HideFrontendStartupOverlay();
	UE_LOG(LogT66Frontend, Log, TEXT("Frontend update gate disabled; continuing startup without client-config validation."));
	InitializeUI();
}

void AT66PlayerController::HandleFrontendLaunchPolicyResponse(bool bSuccess, bool bUpdateRequired, int32 RequiredBuildId, int32 LatestBuildId, const FString& Message)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			if (FrontendLaunchPolicyResponseHandle.IsValid())
			{
				Backend->OnClientLaunchPolicyResponse().Remove(FrontendLaunchPolicyResponseHandle);
				FrontendLaunchPolicyResponseHandle.Reset();
			}
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FrontendLaunchPolicyTimeoutTimerHandle);
	}

	bFrontendLaunchPolicyCheckStarted = false;
	bFrontendLaunchPolicyCheckComplete = true;

	if (bSuccess && bUpdateRequired)
	{
		UE_LOG(
			LogT66Frontend,
			Warning,
			TEXT("Frontend update gate: blocking outdated client local=%d required=%d latest=%d."),
			FrontendLocalSteamBuildId,
			RequiredBuildId,
			LatestBuildId);
		bFrontendLaunchPolicyUpdateBlocked = true;
		HideFrontendStartupOverlay();
		ShowFrontendUpdateRequiredAndQuit(Message);
		return;
	}

	if (!bSuccess)
	{
		UE_LOG(LogT66Frontend, Warning, TEXT("Frontend update gate: backend unavailable, allowing startup."));
	}

	InitializeUI();
}

void AT66PlayerController::HandleFrontendLaunchPolicyTimeout()
{
	if (!bFrontendLaunchPolicyCheckStarted || bFrontendLaunchPolicyCheckComplete)
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			if (FrontendLaunchPolicyResponseHandle.IsValid())
			{
				Backend->OnClientLaunchPolicyResponse().Remove(FrontendLaunchPolicyResponseHandle);
				FrontendLaunchPolicyResponseHandle.Reset();
			}
		}
	}

	bFrontendLaunchPolicyCheckStarted = false;
	bFrontendLaunchPolicyCheckComplete = true;
	UE_LOG(LogT66Frontend, Warning, TEXT("Frontend update gate: timed out, allowing startup."));
	InitializeUI();
}

void AT66PlayerController::ShowFrontendUpdateRequiredAndQuit(const FString& Message) const
{
	const FString ResolvedMessage = Message.IsEmpty()
		? FString::Printf(
			TEXT("A new build is available. Please restart Steam to update CHADPOCALYPSE.\n\nInstalled build: %d"),
			FrontendLocalSteamBuildId)
		: Message;

	FPlatformMisc::MessageBoxExt(EAppMsgType::Ok, *ResolvedMessage, TEXT("Update Required"));
	const_cast<AT66PlayerController*>(this)->ConsoleCommand(TEXT("quit"));
}


void AT66PlayerController::InitializeUI()
{
	if (bUIInitialized)
	{
		return;
	}

	if (!IsLocalController())
	{
		return;
	}

	if (IsFrontendLevel() && !bFrontendLaunchPolicyCheckComplete)
	{
		StartFrontendLaunchPolicyCheck();
		return;
	}

	if (bFrontendLaunchPolicyUpdateBlocked)
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
		UE_LOG(LogT66Frontend, Error, TEXT("Failed to create UI Manager!"));
		return;
	}

	UIManager->Initialize(this);

	// Register all screen classes
	for (const auto& Pair : ScreenClasses)
	{
		if (TSubclassOf<UT66ScreenBase> ResolvedClass = ResolveScreenClass(Pair.Key))
		{
			UIManager->RegisterScreenClass(Pair.Key, ResolvedClass);
			UE_LOG(LogT66Frontend, Log, TEXT("Registered screen class for type %d"), static_cast<int32>(Pair.Key));
		}
	}

	// Ensure core modal screens are available in frontend too (leaderboard row opens Run Summary / Pick the Player).
	if (TSubclassOf<UT66ScreenBase> RunSummaryClass = ResolveScreenClass(ET66ScreenType::RunSummary))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::RunSummary, RunSummaryClass);
	}
	if (TSubclassOf<UT66ScreenBase> SummaryPickerClass = ResolveScreenClass(ET66ScreenType::PlayerSummaryPicker))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::PlayerSummaryPicker, SummaryPickerClass);
	}
	if (TSubclassOf<UT66ScreenBase> ShopClass = ResolveScreenClass(ET66ScreenType::PowerUp))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::PowerUp, ShopClass);
	}
	if (TSubclassOf<UT66ScreenBase> AchievementsClass = ResolveScreenClass(ET66ScreenType::Achievements))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::Achievements, AchievementsClass);
	}
	if (TSubclassOf<UT66ScreenBase> TemporaryBuffSelectionClass = ResolveScreenClass(ET66ScreenType::TemporaryBuffSelection))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::TemporaryBuffSelection, TemporaryBuffSelectionClass);
	}
	if (TSubclassOf<UT66ScreenBase> TemporaryBuffShopClass = ResolveScreenClass(ET66ScreenType::TemporaryBuffShop))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::TemporaryBuffShop, TemporaryBuffShopClass);
	}
	if (TSubclassOf<UT66ScreenBase> UnlocksClass = ResolveScreenClass(ET66ScreenType::Unlocks))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::Unlocks, UnlocksClass);
	}
	if (TSubclassOf<UT66ScreenBase> SnakeGameClass = ResolveScreenClass(ET66ScreenType::SnakeGame))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::SnakeGame, SnakeGameClass);
	}
	if (TSubclassOf<UT66ScreenBase> MiniMainMenuClass = ResolveScreenClass(ET66ScreenType::MiniMainMenu))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::MiniMainMenu, MiniMainMenuClass);
	}
	if (TSubclassOf<UT66ScreenBase> MiniSaveSlotsClass = ResolveScreenClass(ET66ScreenType::MiniSaveSlots))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::MiniSaveSlots, MiniSaveSlotsClass);
	}
	if (TSubclassOf<UT66ScreenBase> MiniCharacterSelectClass = ResolveScreenClass(ET66ScreenType::MiniCharacterSelect))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::MiniCharacterSelect, MiniCharacterSelectClass);
	}
	if (TSubclassOf<UT66ScreenBase> MiniCompanionSelectClass = ResolveScreenClass(ET66ScreenType::MiniCompanionSelect))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::MiniCompanionSelect, MiniCompanionSelectClass);
	}
	if (TSubclassOf<UT66ScreenBase> MiniDifficultySelectClass = ResolveScreenClass(ET66ScreenType::MiniDifficultySelect))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::MiniDifficultySelect, MiniDifficultySelectClass);
	}
	if (TSubclassOf<UT66ScreenBase> MiniIdolSelectClass = ResolveScreenClass(ET66ScreenType::MiniIdolSelect))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::MiniIdolSelect, MiniIdolSelectClass);
	}
	if (TSubclassOf<UT66ScreenBase> MiniShopClass = ResolveScreenClass(ET66ScreenType::MiniShop))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::MiniShop, MiniShopClass);
	}
	if (TSubclassOf<UT66ScreenBase> MiniRunSummaryClass = ResolveScreenClass(ET66ScreenType::MiniRunSummary))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::MiniRunSummary, MiniRunSummaryClass);
	}
	if (TSubclassOf<UT66ScreenBase> TDMainMenuClass = ResolveScreenClass(ET66ScreenType::TDMainMenu))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::TDMainMenu, TDMainMenuClass);
	}
	if (TSubclassOf<UT66ScreenBase> TDDifficultySelectClass = ResolveScreenClass(ET66ScreenType::TDDifficultySelect))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::TDDifficultySelect, TDDifficultySelectClass);
	}
	if (TSubclassOf<UT66ScreenBase> TDBattleClass = ResolveScreenClass(ET66ScreenType::TDBattle))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::TDBattle, TDBattleClass);
	}
	if (TSubclassOf<UT66ScreenBase> LeaderboardClass = ResolveScreenClass(ET66ScreenType::Leaderboard))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::Leaderboard, LeaderboardClass);
	}
	// Account Status is a C++ modal by default (no WBP required). If a WBP is registered, do not override it.
	if (TSubclassOf<UT66ScreenBase> AccountStatusClass = ResolveScreenClass(ET66ScreenType::AccountStatus))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::AccountStatus, AccountStatusClass);
	}
	if (TSubclassOf<UT66ScreenBase> ChallengesClass = ResolveScreenClass(ET66ScreenType::Challenges))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::Challenges, ChallengesClass);
	}
	if (TSubclassOf<UT66ScreenBase> DailyClimbClass = ResolveScreenClass(ET66ScreenType::DailyClimb))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::DailyClimb, DailyClimbClass);
	}
	if (TSubclassOf<UT66ScreenBase> PartyInviteClass = ResolveScreenClass(ET66ScreenType::PartyInvite))
	{
		UIManager->RegisterScreenClass(ET66ScreenType::PartyInvite, PartyInviteClass);
	}

	bUIInitialized = true;

	if (IsFrontendLevel())
	{
		EnsureLocalFrontendPreviewScene();
	}

	// Show the initial screen
	ET66ScreenType ScreenToShow = InitialScreen;
	const bool bClosedConnectionReturn = GetWorld() && GetWorld()->URL.HasOption(TEXT("closed"));
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		if (bClosedConnectionReturn)
		{
			ScreenToShow = ET66ScreenType::MainMenu;
			GI->PendingFrontendScreen = ET66ScreenType::None;
			if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
			{
				SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::MainMenu, true);
				GI->PendingFrontendScreen = ET66ScreenType::None;
			}
			UE_LOG(LogT66Frontend, Log, TEXT("Frontend disconnect return detected; forcing MainMenu startup screen."));
		}
		else if (GI->PendingFrontendScreen != ET66ScreenType::None)
		{
			ScreenToShow = GI->PendingFrontendScreen;
			GI->PendingFrontendScreen = ET66ScreenType::None;
		}

		if (!bClosedConnectionReturn)
		{
			if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
			{
				const ET66ScreenType DesiredPartyScreen = SessionSubsystem->GetDesiredPartyFrontendScreen();
				if (SessionSubsystem->IsPartyLobbyContextActive()
					&& !SessionSubsystem->IsLocalPlayerPartyHost()
					&& DesiredPartyScreen != ET66ScreenType::None)
				{
					ScreenToShow = DesiredPartyScreen;
				}
			}
		}
	}

	ApplyFrontendCommandLineOverrides(ScreenToShow);

	if (ScreenToShow != ET66ScreenType::None)
	{
		UIManager->ShowScreen(ScreenToShow);
		RefreshPartyInviteModal();
		QueueFrontendAutomationScreenshotIfRequested();
	}

	HideFrontendStartupOverlay();
}

void AT66PlayerController::EnsureLocalFrontendPreviewScene()
{
	if (!IsLocalController() || !IsFrontendLevel())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (Cast<AT66FrontendGameMode>(World->GetAuthGameMode()))
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector PreviewOrigin(100000.f, 0.f, 0.f);

	if (!LocalFrontendHeroPreviewStage.IsValid())
	{
		LocalFrontendHeroPreviewStage = World->SpawnActor<AT66HeroPreviewStage>(
			AT66HeroPreviewStage::StaticClass(),
			PreviewOrigin,
			FRotator::ZeroRotator,
			SpawnParams);
	}
	if (LocalFrontendHeroPreviewStage.IsValid())
	{
		LocalFrontendHeroPreviewStage.Get()->SetActorLocation(PreviewOrigin);
		LocalFrontendHeroPreviewStage.Get()->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		LocalFrontendHeroPreviewStage.Get()->SetStageVisible(true);
		LocalFrontendHeroPreviewStage.Get()->ResetFramingCache();
	}

	if (!LocalFrontendCompanionPreviewStage.IsValid())
	{
		LocalFrontendCompanionPreviewStage = World->SpawnActor<AT66CompanionPreviewStage>(
			AT66CompanionPreviewStage::StaticClass(),
			PreviewOrigin,
			FRotator::ZeroRotator,
			SpawnParams);
	}
	if (LocalFrontendCompanionPreviewStage.IsValid())
	{
		LocalFrontendCompanionPreviewStage.Get()->SetActorLocation(PreviewOrigin);
		LocalFrontendCompanionPreviewStage.Get()->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		LocalFrontendCompanionPreviewStage.Get()->SetStageVisible(false);
		LocalFrontendCompanionPreviewStage.Get()->ResetFramingCache();
	}

	if (!LocalFrontendPreviewCamera.IsValid())
	{
		LocalFrontendPreviewCamera = World->SpawnActor<ACameraActor>(
			ACameraActor::StaticClass(),
			FVector(-400.f, 0.f, 350.f),
			FRotator(-15.f, 0.f, 0.f),
			SpawnParams);
		if (LocalFrontendPreviewCamera.IsValid())
		{
			if (UCameraComponent* CameraComponent = LocalFrontendPreviewCamera.Get()->GetCameraComponent())
			{
				CameraComponent->SetFieldOfView(90.f);
				CameraComponent->bConstrainAspectRatio = false;
			}
		}
	}
}

void AT66PlayerController::PositionLocalFrontendCameraForHeroPreview()
{
	EnsureLocalFrontendPreviewScene();
	if (!LocalFrontendPreviewCamera.IsValid())
	{
		return;
	}

	if (LocalFrontendHeroPreviewStage.IsValid())
	{
		LocalFrontendHeroPreviewStage.Get()->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		LocalFrontendHeroPreviewStage.Get()->SetStageVisible(true);
	}
	if (LocalFrontendCompanionPreviewStage.IsValid())
	{
		LocalFrontendCompanionPreviewStage.Get()->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		LocalFrontendCompanionPreviewStage.Get()->SetStageVisible(false);
	}

	if (LocalFrontendHeroPreviewStage.IsValid())
	{
		const FVector CameraLocation = LocalFrontendHeroPreviewStage.Get()->GetIdealCameraLocation();
		const FRotator CameraRotation = LocalFrontendHeroPreviewStage.Get()->GetIdealCameraRotation();
		if (!CameraLocation.IsNearlyZero())
		{
			LocalFrontendPreviewCamera.Get()->SetActorLocation(CameraLocation);
			LocalFrontendPreviewCamera.Get()->SetActorRotation(CameraRotation);
		}
	}

	if (GetViewTarget() != LocalFrontendPreviewCamera.Get())
	{
		SetViewTarget(LocalFrontendPreviewCamera.Get());
	}
}

void AT66PlayerController::PositionLocalFrontendCameraForCompanionPreview()
{
	EnsureLocalFrontendPreviewScene();
	if (!LocalFrontendPreviewCamera.IsValid())
	{
		return;
	}

	if (LocalFrontendCompanionPreviewStage.IsValid())
	{
		LocalFrontendCompanionPreviewStage.Get()->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		LocalFrontendCompanionPreviewStage.Get()->SetStageVisible(true);
	}
	if (LocalFrontendHeroPreviewStage.IsValid())
	{
		LocalFrontendHeroPreviewStage.Get()->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		LocalFrontendHeroPreviewStage.Get()->SetStageVisible(false);
	}

	if (LocalFrontendCompanionPreviewStage.IsValid())
	{
		const FVector CameraLocation = LocalFrontendCompanionPreviewStage.Get()->GetIdealCameraLocation();
		const FRotator CameraRotation = LocalFrontendCompanionPreviewStage.Get()->GetIdealCameraRotation();
		if (!CameraLocation.IsNearlyZero())
		{
			LocalFrontendPreviewCamera.Get()->SetActorLocation(CameraLocation);
			LocalFrontendPreviewCamera.Get()->SetActorRotation(CameraRotation);
		}
	}

	if (GetViewTarget() != LocalFrontendPreviewCamera.Get())
	{
		SetViewTarget(LocalFrontendPreviewCamera.Get());
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
