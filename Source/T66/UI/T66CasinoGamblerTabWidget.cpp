// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CasinoGamblerTabWidget.h"

#include "Core/T66AudioSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/Gambler/T66CoinFlipGameWidget.h"
#include "UI/Gambler/T66FindJokerGameWidget.h"
#include "UI/Gambler/T66GuessCupGameWidget.h"
#include "UI/Gambler/T66StickPickGameWidget.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/T66DemoModeUIUtils.h"
#include "UI/WidgetGames/T66WidgetGameHostContext.h"
#include "UI/WidgetGames/T66WidgetGameRegistry.h"
#include "UI/WidgetGames/T66WidgetGameResult.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	static UT66RunStateSubsystem* ResolveCasinoRunState(const UUserWidget* Widget)
	{
		if (!Widget)
		{
			return nullptr;
		}

		const UWorld* World = Widget->GetWorld();
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		return GameInstance ? GameInstance->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	}

	static UT66RngSubsystem* ResolveCasinoRng(const UUserWidget* Widget)
	{
		if (!Widget)
		{
			return nullptr;
		}

		const UWorld* World = Widget->GetWorld();
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		return GameInstance ? GameInstance->GetSubsystem<UT66RngSubsystem>() : nullptr;
	}

	static float ResolveGamblingLuckRescueChance01(const UUserWidget* Widget)
	{
		const UT66RunStateSubsystem* RunState = ResolveCasinoRunState(Widget);
		return RunState ? RunState->GetGamblingLuckRescueRerollChance01() : 0.f;
	}

	static float ComputeCasinoExpectedChanceWithRescue(const float BaseExpectedChance01, const float RescueChance01)
	{
		const float Base = FMath::Clamp(BaseExpectedChance01, 0.f, 1.f);
		const float Rescue = FMath::Clamp(RescueChance01, 0.f, 1.f);
		return FMath::Clamp(Base + ((1.f - Base) * Rescue * Base), 0.f, 1.f);
	}

	static FText BuildCasinoWagerText(const int32 WagerAmount)
	{
		return WagerAmount > 0
			? FText::Format(NSLOCTEXT("T66.Gambler", "WagerFormat", "Wager: {0}"), FText::AsNumber(WagerAmount))
			: FText::GetEmpty();
	}

	static FText BuildCasinoGoldText(const int32 Gold)
	{
		return FText::Format(NSLOCTEXT("T66.Gambler", "GoldFormat", "Gold: {0}"), FText::AsNumber(Gold));
	}

	static TSharedRef<SWidget> MakeCasinoButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		const ET66ButtonType Type = ET66ButtonType::Primary)
	{
		return FT66FlatStyle::MakeButton(
			FT66FlatStyle::MakeInRunButtonParams(Label, OnClicked, Type)
			.SetMinWidth(0.f)
			.SetPadding(FMargin(16.f, 10.f)));
	}

	static TSharedRef<SWidget> MakeCasinoGameCard(
		const FText& Title,
		const FText& Description,
		const FText& PayoutText,
		const FOnClicked& OnClicked)
	{
		const FTextBlockStyle& TextTitle = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Title"));
		const FTextBlockStyle& TextBody = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body"));

		return FT66FlatStyle::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(Title)
				.TextStyle(&TextTitle)
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 12.f)
			[
				SNew(STextBlock)
				.Text(Description)
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)
			[
				MakeCasinoButton(PayoutText, OnClicked)
			],
			FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66FlatStyle::Tokens::Space5));
	}

	static TSharedRef<SWidget> MakeCasinoFallbackGameWidget(const FText& Label)
	{
		const FTextBlockStyle& TextBody = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body"));
		return SNew(SBorder)
			.BorderBackgroundColor(FT66FlatStyle::Tokens::Panel2)
			.Padding(FT66FlatStyle::Tokens::Space6)
			[
				SNew(STextBlock)
				.Text(Label)
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
			];
	}

	static const TCHAR* BoolText(const bool bValue)
	{
		return bValue ? TEXT("true") : TEXT("false");
	}
}

TSharedRef<SWidget> UT66CasinoGamblerTabWidget::RebuildWidget()
{
	const FTextBlockStyle& TextHeading = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading"));
	const FTextBlockStyle& TextTitle = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Title"));
	const FTextBlockStyle& TextBody = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body"));

	if (!CoinFlipGameWidget)
	{
		CoinFlipGameWidget = CreateWidget<UT66CoinFlipGameWidget>(GetOwningPlayer(), UT66CoinFlipGameWidget::StaticClass());
	}
	if (!GuessCupGameWidget)
	{
		GuessCupGameWidget = CreateWidget<UT66GuessCupGameWidget>(GetOwningPlayer(), UT66GuessCupGameWidget::StaticClass());
	}
	if (!StickPickGameWidget)
	{
		StickPickGameWidget = CreateWidget<UT66StickPickGameWidget>(GetOwningPlayer(), UT66StickPickGameWidget::StaticClass());
	}
	if (!FindJokerGameWidget)
	{
		FindJokerGameWidget = CreateWidget<UT66FindJokerGameWidget>(GetOwningPlayer(), UT66FindJokerGameWidget::StaticClass());
	}

	if (CoinFlipGameWidget)
	{
		CoinFlipGameWidget->SetReturnCallback([this]() { ReturnToGameSelection(); });
		CoinFlipGameWidget->SetChoiceCallback([this](const bool bHeads) { ResolveCoinFlip(bHeads); });
	}
	if (GuessCupGameWidget)
	{
		GuessCupGameWidget->SetReturnCallback([this]() { ReturnToGameSelection(); });
		GuessCupGameWidget->SetChoiceCallback([this](const int32 CupIndex) { ResolveGuessCup(CupIndex); });
	}
	if (StickPickGameWidget)
	{
		StickPickGameWidget->SetReturnCallback([this]() { ReturnToGameSelection(); });
		StickPickGameWidget->SetChoiceCallback([this](const int32 StickIndex) { ResolveStickPick(StickIndex); });
	}
	if (FindJokerGameWidget)
	{
		FindJokerGameWidget->SetReturnCallback([this]() { ReturnToGameSelection(); });
		FindJokerGameWidget->SetChoiceCallback([this](const int32 CardIndex) { ResolveFindJoker(CardIndex); });
	}

	TSharedRef<SWidget> CoinFlipView = CoinFlipGameWidget
		? CoinFlipGameWidget->TakeWidget()
		: MakeCasinoFallbackGameWidget(NSLOCTEXT("T66.Gambler", "CoinFlipUnavailable", "Coin Flip unavailable."));
	TSharedRef<SWidget> GuessCupView = GuessCupGameWidget
		? GuessCupGameWidget->TakeWidget()
		: MakeCasinoFallbackGameWidget(NSLOCTEXT("T66.Gambler", "GuessCupUnavailable", "Guess the Cup unavailable."));
	TSharedRef<SWidget> StickPickView = StickPickGameWidget
		? StickPickGameWidget->TakeWidget()
		: MakeCasinoFallbackGameWidget(NSLOCTEXT("T66.Gambler", "StickPickUnavailable", "Pick the Stick unavailable."));
	TSharedRef<SWidget> FindJokerView = FindJokerGameWidget
		? FindJokerGameWidget->TakeWidget()
		: MakeCasinoFallbackGameWidget(NSLOCTEXT("T66.Gambler", "FindJokerUnavailable", "Find the Joker unavailable."));

	TSharedRef<SUniformGridPanel> GameGrid = SNew(SUniformGridPanel).SlotPadding(FMargin(8.f));
	GameGrid->AddSlot(0, 0)
	[
		MakeCasinoGameCard(
			NSLOCTEXT("T66.Gambler", "CoinFlipTitle", "COIN FLIP"),
			NSLOCTEXT("T66.Gambler", "CoinFlipDesc", "Pick heads or tails. Correct side pays 2x."),
			NSLOCTEXT("T66.Gambler", "Play2x", "PLAY 2X"),
			FOnClicked::CreateUObject(this, &UT66CasinoGamblerTabWidget::OnOpenCoinFlip))
	];
	GameGrid->AddSlot(1, 0)
	[
		MakeCasinoGameCard(
			NSLOCTEXT("T66.Gambler", "GuessCupTitle", "GUESS THE CUP"),
			NSLOCTEXT("T66.Gambler", "GuessCupDesc", "Pick the cup hiding the token. Correct cup pays 3x."),
			NSLOCTEXT("T66.Gambler", "Play3x", "PLAY 3X"),
			FOnClicked::CreateUObject(this, &UT66CasinoGamblerTabWidget::OnOpenGuessCup))
	];
	GameGrid->AddSlot(0, 1)
	[
		MakeCasinoGameCard(
			NSLOCTEXT("T66.Gambler", "StickPickTitle", "PICK THE STICK"),
			NSLOCTEXT("T66.Gambler", "StickPickDesc", "Pick the called longest or shortest stick. Correct stick pays 5x."),
			NSLOCTEXT("T66.Gambler", "Play5x", "PLAY 5X"),
			FOnClicked::CreateUObject(this, &UT66CasinoGamblerTabWidget::OnOpenStickPick))
	];
	GameGrid->AddSlot(1, 1)
	[
		MakeCasinoGameCard(
			NSLOCTEXT("T66.Gambler", "FindJokerTitle", "FIND THE JOKER"),
			NSLOCTEXT("T66.Gambler", "FindJokerDesc", "Pick the card hiding the Joker. Correct card pays 10x."),
			NSLOCTEXT("T66.Gambler", "Play10x", "PLAY 10X"),
			FOnClicked::CreateUObject(this, &UT66CasinoGamblerTabWidget::OnOpenFindJoker))
	];

	TSharedRef<SWidget> DialoguePage =
		FT66FlatStyle::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Gambler", "GamblerTitle", "THE GAMBLER"))
				.TextStyle(&TextHeading)
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Gambler", "GamblerPrompt", "Choose one of the live casino games. A round consumes the wager and pays only on a win."))
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				MakeCasinoButton(
					NSLOCTEXT("T66.Gambler", "OpenCasino", "OPEN CASINO"),
					FOnClicked::CreateUObject(this, &UT66CasinoGamblerTabWidget::OnDialogueGamble))
			],
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66FlatStyle::Tokens::Space6));

	TSharedRef<SWidget> CasinoPage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeCasinoButton(
					NSLOCTEXT("T66.Gambler", "Back", "BACK"),
					FOnClicked::CreateUObject(this, &UT66CasinoGamblerTabWidget::OnBack),
					ET66ButtonType::Neutral)
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SAssignNew(StatusText, STextBlock)
				.Text(FText::GetEmpty())
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SAssignNew(GoldText, STextBlock)
				.Text(BuildCasinoGoldText(0))
				.TextStyle(&TextTitle)
				.ColorAndOpacity(FT66FlatStyle::Tokens::Accent2)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
		[
			FT66FlatStyle::MakePanel(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Gambler", "Wager", "Wager"))
					.TextStyle(&TextBody)
					.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
				[
					SAssignNew(GambleAmountSpin, SSpinBox<int32>)
					.MinValue(1)
					.MaxValue(99999)
					.Value(GambleAmount)
					.IsEnabled_Lambda([this]()
					{
						return RoundState == ECasinoRoundState::NoGame || RoundState == ECasinoRoundState::ReadyForBet;
					})
					.OnValueChanged_Lambda([this](const int32 NewValue)
					{
						GambleAmount = FMath::Max(1, NewValue);
					})
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(MainActionButtonBox, SBox)
						[
							BuildMainActionButton()
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
					[
						SAssignNew(WinCloseButtonBox, SBox)
						.Visibility(EVisibility::Collapsed)
						[
							BuildWinCloseButton()
						]
					]
				],
				FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66FlatStyle::Tokens::Space4))
		]
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SAssignNew(CasinoSwitcher, SWidgetSwitcher)
			+ SWidgetSwitcher::Slot()
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					GameGrid
				]
			]
			+ SWidgetSwitcher::Slot()[CoinFlipView]
			+ SWidgetSwitcher::Slot()[GuessCupView]
			+ SWidgetSwitcher::Slot()[StickPickView]
			+ SWidgetSwitcher::Slot()[FindJokerView]
		];

	TSharedRef<SWidget> Root =
		SNew(SBorder)
		.BorderBackgroundColor(FT66FlatStyle::Tokens::Panel)
		.Padding(FT66FlatStyle::Tokens::Space6)
		[
			SAssignNew(PageSwitcher, SWidgetSwitcher)
			+ SWidgetSwitcher::Slot()
			[
				DialoguePage
			]
			+ SWidgetSwitcher::Slot()
			[
				CasinoPage
			]
		];

	if (PageSwitcher.IsValid())
	{
		PageSwitcher->SetActiveWidgetIndex(bGamblingOnlyKiosk ? 1 : 0);
	}
	if (CasinoSwitcher.IsValid())
	{
		CasinoSwitcher->SetActiveWidgetIndex(0);
	}

	RefreshTopBar();
	RefreshActionControls();
	return Root;
}

void UT66CasinoGamblerTabWidget::NativeDestruct()
{
	if (CoinFlipGameWidget)
	{
		CoinFlipGameWidget->DeactivateWidgetGame();
	}
	if (GuessCupGameWidget)
	{
		GuessCupGameWidget->DeactivateWidgetGame();
	}
	if (StickPickGameWidget)
	{
		StickPickGameWidget->DeactivateWidgetGame();
	}
	if (FindJokerGameWidget)
	{
		FindJokerGameWidget->DeactivateWidgetGame();
	}

	Super::NativeDestruct();
}

void UT66CasinoGamblerTabWidget::SetWinGoldAmount(const int32 InAmount)
{
	WinGoldAmount = FMath::Max(1, InAmount);
	GambleAmount = WinGoldAmount;
	if (GambleAmountSpin.IsValid())
	{
		GambleAmountSpin->SetValue(GambleAmount);
	}
	RefreshTopBar();
}

bool UT66CasinoGamblerTabWidget::IsCasinoGameAllowed(const FName CasinoGameID) const
{
	const FT66WidgetGameDescriptor* Descriptor = T66WidgetGames::Registry::FindDescriptor(CasinoGameID);
	if (!Descriptor)
	{
		Descriptor = T66WidgetGames::Registry::FindByLegacyID(CasinoGameID);
	}
	return Descriptor && T66WidgetGames::Registry::IsAvailable(this, *Descriptor);
}

FReply UT66CasinoGamblerTabWidget::HandleBlockedCasinoGame()
{
	SetStatus(T66DemoModeUI::GetUnavailableContentText(this), FT66FlatStyle::Tokens::TextMuted);
	return FReply::Handled();
}

FReply UT66CasinoGamblerTabWidget::OnBack()
{
	CloseOverlay();
	return FReply::Handled();
}

FReply UT66CasinoGamblerTabWidget::OnDialogueGamble()
{
	SetPage(EGamblerPage::Casino);
	return FReply::Handled();
}

FReply UT66CasinoGamblerTabWidget::OnBetClicked()
{
	if (RoundState == ECasinoRoundState::LostCloseOnly)
	{
		FinalizeCasinoSessionIfResolved();
		CloseOverlay();
		return FReply::Handled();
	}

	if (RoundState == ECasinoRoundState::WonCanDoubleDown)
	{
		if (BeginCasinoRound(FMath::Max(1, InitialBetAmount * 2), true))
		{
			ResolveLockedCasinoGameAutomatically();
		}
		return FReply::Handled();
	}

	if (RoundState == ECasinoRoundState::WaitingForChoice)
	{
		ResolveLockedCasinoGameAutomatically();
		return FReply::Handled();
	}

	if (RoundState != ECasinoRoundState::ReadyForBet)
	{
		SetStatus(NSLOCTEXT("T66.Gambler", "ChooseGameFirst", "Choose a game first."), FT66FlatStyle::Tokens::TextMuted);
		return FReply::Handled();
	}

	if (BeginCasinoRound(FMath::Max(1, GambleAmount), false))
	{
		ResolveLockedCasinoGameAutomatically();
	}
	return FReply::Handled();
}

FReply UT66CasinoGamblerTabWidget::OnWinCloseClicked()
{
	FinalizeCasinoSessionIfResolved();
	CloseOverlay();
	return FReply::Handled();
}

FReply UT66CasinoGamblerTabWidget::OnOpenCoinFlip()
{
	if (!IsCasinoGameAllowed(FName(TEXT("Casino_CoinFlip"))))
	{
		return HandleBlockedCasinoGame();
	}
	if (!LockCasinoGame(EGamblerPage::CoinFlip))
	{
		return FReply::Handled();
	}
	SetPage(EGamblerPage::CoinFlip);
	ActivateCoinFlipPage();
	return FReply::Handled();
}

FReply UT66CasinoGamblerTabWidget::OnOpenGuessCup()
{
	if (!IsCasinoGameAllowed(FName(TEXT("Casino_GuessTheCup"))))
	{
		return HandleBlockedCasinoGame();
	}
	if (!LockCasinoGame(EGamblerPage::GuessCup))
	{
		return FReply::Handled();
	}
	SetPage(EGamblerPage::GuessCup);
	ActivateGuessCupPage();
	return FReply::Handled();
}

FReply UT66CasinoGamblerTabWidget::OnOpenStickPick()
{
	if (!IsCasinoGameAllowed(FName(TEXT("Casino_PickLongestShortestStick"))))
	{
		return HandleBlockedCasinoGame();
	}
	if (!LockCasinoGame(EGamblerPage::StickPick))
	{
		return FReply::Handled();
	}
	SetPage(EGamblerPage::StickPick);
	ActivateStickPickPage();
	return FReply::Handled();
}

FReply UT66CasinoGamblerTabWidget::OnOpenFindJoker()
{
	if (!IsCasinoGameAllowed(FName(TEXT("Casino_FindJoker"))))
	{
		return HandleBlockedCasinoGame();
	}
	if (!LockCasinoGame(EGamblerPage::FindJoker))
	{
		return FReply::Handled();
	}
	SetPage(EGamblerPage::FindJoker);
	ActivateFindJokerPage();
	return FReply::Handled();
}

FReply UT66CasinoGamblerTabWidget::OnGameBackToSelection()
{
	ReturnToGameSelection();
	return FReply::Handled();
}

void UT66CasinoGamblerTabWidget::SetPage(const EGamblerPage Page)
{
	if (!PageSwitcher.IsValid())
	{
		return;
	}

	int32 PageIndex = 0;
	int32 CasinoIndex = 0;
	switch (Page)
	{
	case EGamblerPage::Dialogue:
		PageIndex = 0;
		CasinoIndex = 0;
		break;
	case EGamblerPage::Casino:
		PageIndex = 1;
		CasinoIndex = 0;
		break;
	case EGamblerPage::CoinFlip:
		PageIndex = 1;
		CasinoIndex = 1;
		break;
	case EGamblerPage::GuessCup:
		PageIndex = 1;
		CasinoIndex = 2;
		break;
	case EGamblerPage::StickPick:
		PageIndex = 1;
		CasinoIndex = 3;
		break;
	case EGamblerPage::FindJoker:
		PageIndex = 1;
		CasinoIndex = 4;
		break;
	default:
		break;
	}

	PageSwitcher->SetActiveWidgetIndex(PageIndex);
	if (CasinoSwitcher.IsValid())
	{
		CasinoSwitcher->SetActiveWidgetIndex(CasinoIndex);
	}
	if (RoundState != ECasinoRoundState::WaitingForChoice)
	{
		bInputLocked = false;
	}
	if (RoundState == ECasinoRoundState::NoGame)
	{
		SetStatus(FText::GetEmpty());
	}
	RefreshTopBar();
	RefreshActionControls();
}

FT66WidgetGameHostContext UT66CasinoGamblerTabWidget::BuildChildHostContext()
{
	FT66WidgetGameHostContext HostContext;
	HostContext.WorldContextObject = this;
	HostContext.OwningPlayer = GetOwningPlayer();
	HostContext.StatusTextCallback = [WeakThis = TWeakObjectPtr<UT66CasinoGamblerTabWidget>(this)](const FText& Message)
	{
		if (UT66CasinoGamblerTabWidget* This = WeakThis.Get())
		{
			This->SetStatus(Message);
		}
	};
	HostContext.ReturnNavigationCallback = [WeakThis = TWeakObjectPtr<UT66CasinoGamblerTabWidget>(this)](ET66WidgetGameExitReason)
	{
		if (UT66CasinoGamblerTabWidget* This = WeakThis.Get())
		{
			This->ReturnToGameSelection();
		}
	};
	HostContext.ResultCallback = [WeakThis = TWeakObjectPtr<UT66CasinoGamblerTabWidget>(this)](const FT66WidgetGameResult& Result)
	{
		if (UT66CasinoGamblerTabWidget* This = WeakThis.Get())
		{
			This->HandleCasinoWidgetGameResult(Result);
		}
	};
	HostContext.AvailabilityQueryCallback = [WeakThis = TWeakObjectPtr<UT66CasinoGamblerTabWidget>(this)](const FName GameID)
	{
		const UT66CasinoGamblerTabWidget* This = WeakThis.Get();
		return This ? This->IsCasinoGameAllowed(GameID) : false;
	};
	HostContext.WagerCallback = [WeakThis = TWeakObjectPtr<UT66CasinoGamblerTabWidget>(this)]()
	{
		const UT66CasinoGamblerTabWidget* This = WeakThis.Get();
		return This ? This->GetCurrentCasinoWager() : 0;
	};
	HostContext.PayoutCallback = [WeakThis = TWeakObjectPtr<UT66CasinoGamblerTabWidget>(this)](const int32 PayoutGold)
	{
		if (UT66CasinoGamblerTabWidget* This = WeakThis.Get())
		{
			This->AwardPayoutGoldAmount(PayoutGold);
		}
	};
	return HostContext;
}

void UT66CasinoGamblerTabWidget::ReturnToGameSelection()
{
	if (LockedGamePage != EGamblerPage::Casino || RoundState != ECasinoRoundState::NoGame)
	{
		SetStatus(NSLOCTEXT("T66.Gambler", "GameLocked", "Game locked. Resolve the wager or close the gambler."), FT66FlatStyle::Tokens::TextMuted);
		return;
	}

	if (CoinFlipGameWidget)
	{
		CoinFlipGameWidget->DeactivateWidgetGame();
	}
	if (GuessCupGameWidget)
	{
		GuessCupGameWidget->DeactivateWidgetGame();
	}
	if (StickPickGameWidget)
	{
		StickPickGameWidget->DeactivateWidgetGame();
	}
	if (FindJokerGameWidget)
	{
		FindJokerGameWidget->DeactivateWidgetGame();
	}
	SetPage(EGamblerPage::Casino);
}

void UT66CasinoGamblerTabWidget::RefreshTopBar()
{
	const UT66RunStateSubsystem* RunState = ResolveCasinoRunState(this);
	if (GoldText.IsValid())
	{
		GoldText->SetText(BuildCasinoGoldText(RunState ? RunState->GetCurrentGold() : 0));
	}
	if (CoinFlipGameWidget)
	{
		CoinFlipGameWidget->SetWagerAmount(GetCurrentCasinoWager());
	}
	if (GuessCupGameWidget)
	{
		GuessCupGameWidget->SetWagerAmount(GetCurrentCasinoWager());
	}
	if (StickPickGameWidget)
	{
		StickPickGameWidget->SetWagerAmount(GetCurrentCasinoWager());
	}
	if (FindJokerGameWidget)
	{
		FindJokerGameWidget->SetWagerAmount(GetCurrentCasinoWager());
	}
	RefreshActionControls();
}

void UT66CasinoGamblerTabWidget::RefreshActionControls()
{
	if (MainActionButtonBox.IsValid())
	{
		MainActionButtonBox->SetContent(BuildMainActionButton());
	}

	if (WinCloseButtonBox.IsValid())
	{
		WinCloseButtonBox->SetVisibility(RoundState == ECasinoRoundState::WonCanDoubleDown ? EVisibility::Visible : EVisibility::Collapsed);
		WinCloseButtonBox->SetContent(BuildWinCloseButton());
	}
}

FText UT66CasinoGamblerTabWidget::GetMainActionLabel() const
{
	switch (RoundState)
	{
	case ECasinoRoundState::ReadyForBet:
		return NSLOCTEXT("T66.Gambler", "Bet", "BET");
	case ECasinoRoundState::WaitingForChoice:
		return NSLOCTEXT("T66.Gambler", "Resolving", "RESOLVING");
	case ECasinoRoundState::WonCanDoubleDown:
		return NSLOCTEXT("T66.Gambler", "DoubleDown", "DOUBLE DOWN");
	case ECasinoRoundState::LostCloseOnly:
		return NSLOCTEXT("T66.Gambler", "Close", "CLOSE");
	case ECasinoRoundState::NoGame:
	default:
		return NSLOCTEXT("T66.Gambler", "ChooseGame", "CHOOSE GAME");
	}
}

TSharedRef<SWidget> UT66CasinoGamblerTabWidget::BuildMainActionButton()
{
	const ET66ButtonType ButtonType = RoundState == ECasinoRoundState::LostCloseOnly
		? ET66ButtonType::Neutral
		: ET66ButtonType::Primary;
	return MakeCasinoButton(
		GetMainActionLabel(),
		FOnClicked::CreateUObject(this, &UT66CasinoGamblerTabWidget::OnBetClicked),
		ButtonType);
}

TSharedRef<SWidget> UT66CasinoGamblerTabWidget::BuildWinCloseButton()
{
	return MakeCasinoButton(
		NSLOCTEXT("T66.Gambler", "CashOutClose", "CLOSE"),
		FOnClicked::CreateUObject(this, &UT66CasinoGamblerTabWidget::OnWinCloseClicked),
		ET66ButtonType::Neutral);
}

bool UT66CasinoGamblerTabWidget::LockCasinoGame(const EGamblerPage Page)
{
	if (LockedGamePage != EGamblerPage::Casino && LockedGamePage != Page)
	{
		SetStatus(NSLOCTEXT("T66.Gambler", "AlreadyLockedDifferentGame", "You are locked into your chosen game."), FT66FlatStyle::Tokens::TextMuted);
		return false;
	}

	if (RoundState == ECasinoRoundState::LostCloseOnly)
	{
		SetStatus(NSLOCTEXT("T66.Gambler", "LostCloseOnly", "The wager is lost. Close the gambler."), FLinearColor::Red);
		return false;
	}

	LockedGamePage = Page;
	if (RoundState == ECasinoRoundState::NoGame)
	{
		RoundState = ECasinoRoundState::ReadyForBet;
		SetStatus(NSLOCTEXT("T66.Gambler", "GameLockedReady", "Game locked. Enter a wager and press BET."), FT66FlatStyle::Tokens::Accent2);
	}
	RefreshTopBar();
	return true;
}

bool UT66CasinoGamblerTabWidget::BeginCasinoRound(const int32 BetAmount, const bool bDoubleDown)
{
	if (LockedGamePage == EGamblerPage::Casino)
	{
		SetStatus(NSLOCTEXT("T66.Gambler", "ChooseGameBeforeBet", "Choose a game before betting."), FT66FlatStyle::Tokens::TextMuted);
		return false;
	}

	const int32 ClampedBet = FMath::Max(1, BetAmount);
	UT66RunStateSubsystem* RunState = ResolveCasinoRunState(this);
	if (!RunState || !RunState->TrySpendGold(ClampedBet))
	{
		UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("UI.Deny")));
		SetStatus(NSLOCTEXT("T66.Gambler", "NotEnoughGold", "Not enough gold."), FLinearColor::Red);
		RefreshTopBar();
		return false;
	}

	UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Casino.Bet")));

	if (!bDoubleDown || InitialBetAmount <= 0)
	{
		InitialBetAmount = ClampedBet;
	}

	LockedBetAmount = ClampedBet;
	CurrentRoundBetAmount = ClampedBet;
	RoundState = ECasinoRoundState::WaitingForChoice;
	bInputLocked = false;
	bCasinoSessionShouldConsumeOnClose = true;
	LastResolvedCasinoGameID = NAME_None;
	LastResolvedCasinoPayoutGold = 0;
	bLastResolvedCasinoWin = false;

	if (bDoubleDown)
	{
		ResetActiveGameForNextRound();
	}

	SetStatus(FText::Format(
		bDoubleDown
			? NSLOCTEXT("T66.Gambler", "DoubleDownLockedFmt", "Double down wager: {0}. Resolving.")
			: NSLOCTEXT("T66.Gambler", "BetLockedFmt", "Wager: {0}. Resolving."),
		FText::AsNumber(ClampedBet)),
		FT66FlatStyle::Tokens::Accent2);
	RefreshTopBar();
	return true;
}

void UT66CasinoGamblerTabWidget::ResolveLockedCasinoGameAutomatically()
{
	if (!CanResolveCasinoChoice())
	{
		SetStatus(NSLOCTEXT("T66.Gambler", "PressBetFirst", "Press BET before resolving."), FLinearColor::Red);
		return;
	}

	int32 ChoiceSeed = 0;
	int32 ChoiceDrawIndex = INDEX_NONE;
	switch (LockedGamePage)
	{
	case EGamblerPage::CoinFlip:
		ResolveCoinFlip(DrawCasinoIndex(2, ChoiceSeed, ChoiceDrawIndex) == 0);
		break;
	case EGamblerPage::GuessCup:
		ResolveGuessCup(DrawCasinoIndex(3, ChoiceSeed, ChoiceDrawIndex));
		break;
	case EGamblerPage::StickPick:
		ResolveStickPick(DrawCasinoIndex(5, ChoiceSeed, ChoiceDrawIndex));
		break;
	case EGamblerPage::FindJoker:
		ResolveFindJoker(DrawCasinoIndex(10, ChoiceSeed, ChoiceDrawIndex));
		break;
	default:
		SetStatus(NSLOCTEXT("T66.Gambler", "ChooseGameBeforeResolve", "Choose a game before resolving."), FT66FlatStyle::Tokens::TextMuted);
		break;
	}
}

bool UT66CasinoGamblerTabWidget::CanResolveCasinoChoice() const
{
	return RoundState == ECasinoRoundState::WaitingForChoice && CurrentRoundBetAmount > 0;
}

void UT66CasinoGamblerTabWidget::HandleCasinoRoundCompleted(const FName GameID, const bool bSuccessful, const int32 PayoutGold)
{
	LastResolvedCasinoGameID = GameID;
	LastResolvedCasinoPayoutGold = FMath::Max(0, PayoutGold);
	bLastResolvedCasinoWin = bSuccessful;
	CurrentRoundBetAmount = 0;
	LockedBetAmount = 0;
	RoundState = bSuccessful ? ECasinoRoundState::WonCanDoubleDown : ECasinoRoundState::LostCloseOnly;
	bInputLocked = false;
	bCasinoSessionShouldConsumeOnClose = true;

	UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(bSuccessful ? TEXT("Casino.Win") : TEXT("Casino.Lose")));

	SetStatus(
		bSuccessful
			? FText::Format(
				NSLOCTEXT("T66.Gambler", "WinDoubleDownPrompt", "Win. Double down for {0} or close."),
				FText::AsNumber(FMath::Max(1, InitialBetAmount * 2)))
			: NSLOCTEXT("T66.Gambler", "LoseClosePrompt", "Lose. Close the gambler."),
		bSuccessful ? FT66FlatStyle::Tokens::Accent2 : FLinearColor::Red);
	RefreshTopBar();
}

void UT66CasinoGamblerTabWidget::ResetActiveGameForNextRound()
{
	switch (LockedGamePage)
	{
	case EGamblerPage::CoinFlip:
		if (CoinFlipGameWidget)
		{
			CoinFlipGameWidget->ResetForOpen();
			CoinFlipGameWidget->SetWagerAmount(GetCurrentCasinoWager());
		}
		break;
	case EGamblerPage::GuessCup:
		if (GuessCupGameWidget)
		{
			GuessCupGameWidget->ResetForOpen();
			GuessCupGameWidget->SetWagerAmount(GetCurrentCasinoWager());
		}
		break;
	case EGamblerPage::StickPick:
		ActivateStickPickPage();
		break;
	case EGamblerPage::FindJoker:
		if (FindJokerGameWidget)
		{
			FindJokerGameWidget->ResetForOpen();
			FindJokerGameWidget->SetWagerAmount(GetCurrentCasinoWager());
		}
		break;
	default:
		break;
	}
}

void UT66CasinoGamblerTabWidget::ResetCasinoSessionState(const bool bClearLockedGame)
{
	bInputLocked = false;
	LockedBetAmount = 0;
	CurrentRoundBetAmount = 0;
	InitialBetAmount = 0;
	LastResolvedCasinoGameID = NAME_None;
	LastResolvedCasinoPayoutGold = 0;
	bLastResolvedCasinoWin = false;
	bCasinoSessionShouldConsumeOnClose = false;
	RoundState = ECasinoRoundState::NoGame;
	if (bClearLockedGame)
	{
		LockedGamePage = EGamblerPage::Casino;
	}
	RefreshTopBar();
}

bool UT66CasinoGamblerTabWidget::TryPayWithLockedBet(int32& OutBetAmount)
{
	if (!CanResolveCasinoChoice())
	{
		SetStatus(NSLOCTEXT("T66.Gambler", "PressBetFirst", "Press BET before choosing."), FLinearColor::Red);
		OutBetAmount = 0;
		return false;
	}

	OutBetAmount = CurrentRoundBetAmount;
	if (OutBetAmount <= 0)
	{
		SetStatus(NSLOCTEXT("T66.Gambler", "InvalidWager", "Choose a wager first."), FLinearColor::Red);
		return false;
	}

	CurrentRoundBetAmount = 0;
	RefreshTopBar();
	return true;
}

int32 UT66CasinoGamblerTabWidget::GetCurrentCasinoWager() const
{
	if (CurrentRoundBetAmount > 0)
	{
		return CurrentRoundBetAmount;
	}
	if (LockedBetAmount > 0)
	{
		return LockedBetAmount;
	}
	if (RoundState == ECasinoRoundState::WonCanDoubleDown && InitialBetAmount > 0)
	{
		return FMath::Max(1, InitialBetAmount * 2);
	}
	return FMath::Max(1, GambleAmount);
}

void UT66CasinoGamblerTabWidget::AwardPayoutGoldAmount(const int32 PayoutGold)
{
	if (PayoutGold <= 0)
	{
		RefreshTopBar();
		return;
	}

	if (UT66RunStateSubsystem* RunState = ResolveCasinoRunState(this))
	{
		RunState->AddGold(PayoutGold, ET66GoldTransactionSource::Gambler);
	}
	RefreshTopBar();
}

void UT66CasinoGamblerTabWidget::SetStatus(const FText& Msg, const FLinearColor& Color)
{
	if (CasinoSwitcher.IsValid())
	{
		switch (CasinoSwitcher->GetActiveWidgetIndex())
		{
		case 1:
			if (CoinFlipGameWidget)
			{
				CoinFlipGameWidget->SetStatus(Msg, Color);
				if (StatusText.IsValid())
				{
					StatusText->SetText(FText::GetEmpty());
				}
				return;
			}
			break;
		case 2:
			if (GuessCupGameWidget)
			{
				GuessCupGameWidget->SetStatus(Msg, Color);
				if (StatusText.IsValid())
				{
					StatusText->SetText(FText::GetEmpty());
				}
				return;
			}
			break;
		case 3:
			if (StickPickGameWidget)
			{
				StickPickGameWidget->SetStatus(Msg, Color);
				if (StatusText.IsValid())
				{
					StatusText->SetText(FText::GetEmpty());
				}
				return;
			}
			break;
		case 4:
			if (FindJokerGameWidget)
			{
				FindJokerGameWidget->SetStatus(Msg, Color);
				if (StatusText.IsValid())
				{
					StatusText->SetText(FText::GetEmpty());
				}
				return;
			}
			break;
		default:
			break;
		}
	}

	if (StatusText.IsValid())
	{
		StatusText->SetText(Msg);
		StatusText->SetColorAndOpacity(FSlateColor(Color));
	}
}

void UT66CasinoGamblerTabWidget::HandleCasinoWidgetGameResult(const FT66WidgetGameResult& Result)
{
	if (Result.ExitReason != ET66WidgetGameExitReason::Completed)
	{
		return;
	}

	HandleCasinoRoundCompleted(Result.GameID, Result.bSuccessful, Result.bHasPayout ? Result.Payout : 0);
}

void UT66CasinoGamblerTabWidget::ReportCasinoResult(const FName GameID, const bool bSuccessful, const int32 PayoutGold)
{
	FT66WidgetGameResult Result;
	Result.GameID = GameID;
	Result.ExitReason = ET66WidgetGameExitReason::Completed;
	Result.Payout = FMath::Max(0, PayoutGold);
	Result.bHasPayout = true;
	Result.bSuccessful = bSuccessful;
	HandleCasinoWidgetGameResult(Result);
}

void UT66CasinoGamblerTabWidget::RecordCasinoRound(
	const ET66AntiCheatGamblerGameType GameType,
	const int32 BetGold,
	const int32 PayoutGold,
	const bool bWin,
	const int32 PlayerChoice,
	const int32 OpponentChoice,
	const int32 OutcomeValue,
	const int32 OutcomeSecondaryValue,
	const int32 OutcomePreDrawSeed,
	const int32 OutcomeDrawIndex,
	const float OutcomeExpectedChance01,
	const FString& ActionSequence)
{
	if (UT66RunStateSubsystem* RunState = ResolveCasinoRunState(this))
	{
		RunState->RecordAntiCheatGamblerRound(
			GameType,
			BetGold,
			PayoutGold,
			false,
			false,
			bWin,
			false,
			PlayerChoice,
			OpponentChoice,
			OutcomeValue,
			OutcomeSecondaryValue,
			0,
			0,
			0,
			0,
			INDEX_NONE,
			OutcomePreDrawSeed,
			OutcomeDrawIndex,
			OutcomeExpectedChance01,
			ActionSequence);
	}
}

void UT66CasinoGamblerTabWidget::ActivateCoinFlipPage()
{
	if (CoinFlipGameWidget)
	{
		CoinFlipGameWidget->ActivateWidgetGame(BuildChildHostContext());
		CoinFlipGameWidget->SetWagerAmount(GetCurrentCasinoWager());
	}
}

void UT66CasinoGamblerTabWidget::ActivateGuessCupPage()
{
	if (GuessCupGameWidget)
	{
		GuessCupGameWidget->ActivateWidgetGame(BuildChildHostContext());
		GuessCupGameWidget->SetWagerAmount(GetCurrentCasinoWager());
	}
}

void UT66CasinoGamblerTabWidget::ActivateStickPickPage()
{
	int32 ModeSeed = 0;
	int32 ModeDrawIndex = INDEX_NONE;
	bPendingStickTargetShortest = DrawCasinoIndex(2, ModeSeed, ModeDrawIndex) == 0;

	if (StickPickGameWidget)
	{
		StickPickGameWidget->ActivateWidgetGame(BuildChildHostContext());
		StickPickGameWidget->SetWagerAmount(GetCurrentCasinoWager());
		StickPickGameWidget->SetTargetShortest(bPendingStickTargetShortest);
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[T66Proof][CasinoStickMode] TargetShortest=%s Seed=%d DrawIndex=%d"),
		BoolText(bPendingStickTargetShortest),
		ModeSeed,
		ModeDrawIndex);
}

void UT66CasinoGamblerTabWidget::ActivateFindJokerPage()
{
	if (FindJokerGameWidget)
	{
		FindJokerGameWidget->ActivateWidgetGame(BuildChildHostContext());
		FindJokerGameWidget->SetWagerAmount(GetCurrentCasinoWager());
	}
}

void UT66CasinoGamblerTabWidget::ResolveCoinFlip(const bool bChoseHeads)
{
	if (bInputLocked)
	{
		return;
	}

	int32 BetAmount = 0;
	if (!TryPayWithLockedBet(BetAmount))
	{
		return;
	}

	bInputLocked = true;
	int32 PreDrawSeed = 0;
	int32 DrawIndex = INDEX_NONE;
	int32 ResultIndex = DrawCasinoIndex(2, PreDrawSeed, DrawIndex);
	const int32 PlayerChoice = bChoseHeads ? 0 : 1;
	FString ActionSequence = FString::Printf(TEXT("CoinFlip.%s"), bChoseHeads ? TEXT("Heads") : TEXT("Tails"));
	bool bWin = PlayerChoice == ResultIndex;
	if (!bWin)
	{
		bWin = TryApplyGamblingLuckRescueReroll(2, PlayerChoice, ResultIndex, PreDrawSeed, DrawIndex, ActionSequence);
	}
	const bool bResultHeads = ResultIndex == 0;
	const int32 PayoutGold = bWin ? BetAmount * 2 : 0;

	if (CoinFlipGameWidget)
	{
		CoinFlipGameWidget->StartSpin(bResultHeads, 1.2f);
		CoinFlipGameWidget->SetResultText(FText::Format(
			bWin
				? NSLOCTEXT("T66.Gambler", "CoinFlipWinFmt", "{0}. WIN (+{1})")
				: NSLOCTEXT("T66.Gambler", "CoinFlipLoseFmt", "{0}. LOSE"),
			bResultHeads ? NSLOCTEXT("T66.Gambler", "Heads", "Heads") : NSLOCTEXT("T66.Gambler", "Tails", "Tails"),
			FText::AsNumber(PayoutGold)));
	}

	AwardPayoutGoldAmount(PayoutGold);
	RecordCasinoRound(
		ET66AntiCheatGamblerGameType::CoinFlip,
		BetAmount,
		PayoutGold,
		bWin,
		PlayerChoice,
		ResultIndex,
		ResultIndex,
		0,
		PreDrawSeed,
		DrawIndex,
		ComputeCasinoExpectedChanceWithRescue(0.5f, ResolveGamblingLuckRescueChance01(this)),
		ActionSequence);
	ReportCasinoResult(FName(TEXT("Casino_CoinFlip")), bWin, PayoutGold);
}

void UT66CasinoGamblerTabWidget::ResolveGuessCup(const int32 CupIndex)
{
	if (bInputLocked)
	{
		return;
	}

	int32 BetAmount = 0;
	if (!TryPayWithLockedBet(BetAmount))
	{
		return;
	}

	bInputLocked = true;
	int32 PreDrawSeed = 0;
	int32 DrawIndex = INDEX_NONE;
	int32 WinningCup = DrawCasinoIndex(3, PreDrawSeed, DrawIndex);
	FString ActionSequence = FString::Printf(TEXT("GuessTheCup.Cup=%d"), CupIndex);
	bool bWin = CupIndex == WinningCup;
	if (!bWin)
	{
		bWin = TryApplyGamblingLuckRescueReroll(3, CupIndex, WinningCup, PreDrawSeed, DrawIndex, ActionSequence);
	}
	const int32 PayoutGold = bWin ? BetAmount * 3 : 0;

	if (GuessCupGameWidget)
	{
		GuessCupGameWidget->RevealResult(CupIndex, WinningCup, PayoutGold);
	}
	AwardPayoutGoldAmount(PayoutGold);
	RecordCasinoRound(
		ET66AntiCheatGamblerGameType::GuessTheCup,
		BetAmount,
		PayoutGold,
		bWin,
		CupIndex,
		WinningCup,
		WinningCup,
		0,
		PreDrawSeed,
		DrawIndex,
		ComputeCasinoExpectedChanceWithRescue(1.f / 3.f, ResolveGamblingLuckRescueChance01(this)),
		ActionSequence);
	ReportCasinoResult(FName(TEXT("Casino_GuessTheCup")), bWin, PayoutGold);
}

void UT66CasinoGamblerTabWidget::ResolveStickPick(const int32 StickIndex)
{
	if (bInputLocked)
	{
		return;
	}

	int32 BetAmount = 0;
	if (!TryPayWithLockedBet(BetAmount))
	{
		return;
	}

	bInputLocked = true;
	int32 PreDrawSeed = 0;
	int32 DrawIndex = INDEX_NONE;
	int32 TargetStick = DrawCasinoIndex(5, PreDrawSeed, DrawIndex);
	FString ActionSequence = FString::Printf(TEXT("StickPick.Stick=%d;TargetShortest=%s"), StickIndex, BoolText(bPendingStickTargetShortest));
	bool bWin = StickIndex == TargetStick;
	if (!bWin)
	{
		bWin = TryApplyGamblingLuckRescueReroll(5, StickIndex, TargetStick, PreDrawSeed, DrawIndex, ActionSequence);
	}
	const int32 PayoutGold = bWin ? BetAmount * 5 : 0;

	if (StickPickGameWidget)
	{
		StickPickGameWidget->RevealResult(StickIndex, TargetStick, bPendingStickTargetShortest, PayoutGold);
	}
	AwardPayoutGoldAmount(PayoutGold);
	RecordCasinoRound(
		ET66AntiCheatGamblerGameType::PickLongestShortestStick,
		BetAmount,
		PayoutGold,
		bWin,
		StickIndex,
		TargetStick,
		TargetStick,
		bPendingStickTargetShortest ? 1 : 0,
		PreDrawSeed,
		DrawIndex,
		ComputeCasinoExpectedChanceWithRescue(0.2f, ResolveGamblingLuckRescueChance01(this)),
		ActionSequence);
	ReportCasinoResult(FName(TEXT("Casino_PickLongestShortestStick")), bWin, PayoutGold);
}

void UT66CasinoGamblerTabWidget::ResolveFindJoker(const int32 CardIndex)
{
	if (bInputLocked)
	{
		return;
	}

	int32 BetAmount = 0;
	if (!TryPayWithLockedBet(BetAmount))
	{
		return;
	}

	bInputLocked = true;
	int32 PreDrawSeed = 0;
	int32 DrawIndex = INDEX_NONE;
	int32 JokerCard = DrawCasinoIndex(10, PreDrawSeed, DrawIndex);
	FString ActionSequence = FString::Printf(TEXT("FindJoker.Card=%d"), CardIndex);
	bool bWin = CardIndex == JokerCard;
	if (!bWin)
	{
		bWin = TryApplyGamblingLuckRescueReroll(10, CardIndex, JokerCard, PreDrawSeed, DrawIndex, ActionSequence);
	}
	const int32 PayoutGold = bWin ? BetAmount * 10 : 0;

	if (FindJokerGameWidget)
	{
		FindJokerGameWidget->RevealResult(CardIndex, JokerCard, PayoutGold);
	}
	AwardPayoutGoldAmount(PayoutGold);
	RecordCasinoRound(
		ET66AntiCheatGamblerGameType::FindJoker,
		BetAmount,
		PayoutGold,
		bWin,
		CardIndex,
		JokerCard,
		JokerCard,
		0,
		PreDrawSeed,
		DrawIndex,
		ComputeCasinoExpectedChanceWithRescue(0.1f, ResolveGamblingLuckRescueChance01(this)),
		ActionSequence);
	ReportCasinoResult(FName(TEXT("Casino_FindJoker")), bWin, PayoutGold);
}

int32 UT66CasinoGamblerTabWidget::DrawCasinoIndex(const int32 ExclusiveMax, int32& OutPreDrawSeed, int32& OutDrawIndex) const
{
	OutPreDrawSeed = 0;
	OutDrawIndex = INDEX_NONE;
	if (ExclusiveMax <= 1)
	{
		return 0;
	}

	if (UT66RngSubsystem* RngSub = ResolveCasinoRng(this))
	{
		const int32 Value = RngSub->RunRandRange(0, ExclusiveMax - 1);
		OutPreDrawSeed = RngSub->GetLastRunPreDrawSeed();
		OutDrawIndex = RngSub->GetLastRunDrawIndex();
		return Value;
	}

	return FMath::RandRange(0, ExclusiveMax - 1);
}

bool UT66CasinoGamblerTabWidget::TryApplyGamblingLuckRescueReroll(
	const int32 ExclusiveMax,
	const int32 PlayerChoice,
	int32& InOutOutcomeValue,
	int32& InOutPreDrawSeed,
	int32& InOutDrawIndex,
	FString& InOutActionSequence) const
{
	const float RescueChance = ResolveGamblingLuckRescueChance01(this);
	if (ExclusiveMax <= 1 || RescueChance <= 0.f)
	{
		return false;
	}

	bool bRescueTriggered = false;
	int32 RescuePreDrawSeed = 0;
	int32 RescueDrawIndex = INDEX_NONE;
	if (UT66RngSubsystem* RngSub = ResolveCasinoRng(this))
	{
		bRescueTriggered = RngSub->RollChance01(RescueChance);
		RescuePreDrawSeed = RngSub->GetLastRunPreDrawSeed();
		RescueDrawIndex = RngSub->GetLastRunDrawIndex();
	}
	else
	{
		bRescueTriggered = FMath::FRand() < RescueChance;
	}

	if (!bRescueTriggered)
	{
		InOutActionSequence += FString::Printf(
			TEXT(";GamblingLuckRescue=No;RescueChance=%.3f;RescueSeed=%d;RescueDraw=%d"),
			RescueChance,
			RescuePreDrawSeed,
			RescueDrawIndex);
		return false;
	}

	int32 RerollPreDrawSeed = 0;
	int32 RerollDrawIndex = INDEX_NONE;
	const int32 RerollOutcome = DrawCasinoIndex(ExclusiveMax, RerollPreDrawSeed, RerollDrawIndex);
	InOutOutcomeValue = RerollOutcome;
	InOutPreDrawSeed = RerollPreDrawSeed;
	InOutDrawIndex = RerollDrawIndex;
	const bool bRescueWin = RerollOutcome == PlayerChoice;
	InOutActionSequence += FString::Printf(
		TEXT(";GamblingLuckRescue=Yes;RescueChance=%.3f;RescueSeed=%d;RescueDraw=%d;RerollOutcome=%d;RerollWin=%d"),
		RescueChance,
		RescuePreDrawSeed,
		RescueDrawIndex,
		RerollOutcome,
		bRescueWin ? 1 : 0);
	return bRescueWin;
}

void UT66CasinoGamblerTabWidget::OpenCasinoPage()
{
	SetPage(EGamblerPage::Casino);
}

void UT66CasinoGamblerTabWidget::FinalizeCasinoSessionIfResolved()
{
	if (!bCasinoSessionShouldConsumeOnClose)
	{
		return;
	}

	bCasinoSessionShouldConsumeOnClose = false;

	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->HandleCasinoGambleResolved(
			LastResolvedCasinoGameID,
			bLastResolvedCasinoWin,
			LastResolvedCasinoPayoutGold);
	}
}

void UT66CasinoGamblerTabWidget::CloseOverlay()
{
	FinalizeCasinoSessionIfResolved();
	ResetCasinoSessionState(true);

	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (bEmbeddedInCasinoShell)
		{
			PC->CloseCasinoOverlay();
			return;
		}
	}

	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
}

#if !UE_BUILD_SHIPPING
bool UT66CasinoGamblerTabWidget::RunCasinoDoubleDownAutomationProof(FString& OutDetail)
{
	OutDetail.Reset();
	UT66RunStateSubsystem* RunState = ResolveCasinoRunState(this);
	if (!RunState)
	{
		OutDetail = TEXT("RunStateMissing");
		return false;
	}

	const int32 GoldBefore = RunState->GetCurrentGold();
	RunState->AddGold(10000, ET66GoldTransactionSource::Gambler);

	ResetCasinoSessionState(true);
	GambleAmount = 10;
	if (!LockCasinoGame(EGamblerPage::CoinFlip))
	{
		OutDetail = TEXT("LockFailed");
		return false;
	}

	const bool bInitialBet = BeginCasinoRound(10, false);
	const int32 InitialRoundAmount = CurrentRoundBetAmount;
	const bool bInitialWaiting = RoundState == ECasinoRoundState::WaitingForChoice && CurrentRoundBetAmount == 10 && InitialBetAmount == 10;
	AwardPayoutGoldAmount(20);
	ReportCasinoResult(FName(TEXT("Casino_CoinFlip")), true, 20);
	const bool bWinState = RoundState == ECasinoRoundState::WonCanDoubleDown && GetCurrentCasinoWager() == 20;

	const bool bDoubleDownBet = BeginCasinoRound(20, true);
	const int32 DoubleDownRoundAmount = CurrentRoundBetAmount;
	const bool bDoubleDownWaiting = RoundState == ECasinoRoundState::WaitingForChoice && CurrentRoundBetAmount == 20 && InitialBetAmount == 10;
	ReportCasinoResult(FName(TEXT("Casino_CoinFlip")), false, 0);
	const bool bLossState = RoundState == ECasinoRoundState::LostCloseOnly && bCasinoSessionShouldConsumeOnClose;
	const bool bRejectAfterLoss = !LockCasinoGame(EGamblerPage::CoinFlip);

	const int32 GoldAfter = RunState->GetCurrentGold();
	const int32 GoldDelta = GoldAfter - GoldBefore;
	const int32 ExpectedGoldDelta = 10000 - 10 + 20 - 20;
	ResetCasinoSessionState(true);
	OutDetail = FString::Printf(
		TEXT("GoldBefore=%d GoldAfter=%d GoldDelta=%d ExpectedGoldDelta=%d InitialBet=%d InitialRoundAmount=%d InitialWaiting=%d WinState=%d DoubleDownBet=%d DoubleDownRoundAmount=%d DoubleDownWaiting=%d LossState=%d RejectAfterLoss=%d"),
		GoldBefore,
		GoldAfter,
		GoldDelta,
		ExpectedGoldDelta,
		bInitialBet ? 1 : 0,
		InitialRoundAmount,
		bInitialWaiting ? 1 : 0,
		bWinState ? 1 : 0,
		bDoubleDownBet ? 1 : 0,
		DoubleDownRoundAmount,
		bDoubleDownWaiting ? 1 : 0,
		bLossState ? 1 : 0,
		bRejectAfterLoss ? 1 : 0);
	return bInitialBet
		&& InitialRoundAmount == 10
		&& bInitialWaiting
		&& bWinState
		&& bDoubleDownBet
		&& DoubleDownRoundAmount == 20
		&& bDoubleDownWaiting
		&& bLossState
		&& bRejectAfterLoss
		&& GoldDelta == ExpectedGoldDelta;
}
#endif
