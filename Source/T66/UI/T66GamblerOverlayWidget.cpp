// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66GamblerOverlayWidget.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Styling/CoreStyle.h"
#include "EngineUtils.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Gameplay/T66PlayerController.h"

void UT66GamblerOverlayWidget::SetWinGoldAmount(int32 InAmount)
{
	WinGoldAmount = FMath::Max(0, InAmount);
	// Use DT value as a default suggested bet amount (player can change it in the UI).
	GambleAmount = FMath::Max(0, WinGoldAmount);
	if (GambleAmountSpin.IsValid())
	{
		GambleAmountSpin->SetValue(GambleAmount);
	}
	RefreshTopBar();
}

TSharedRef<SWidget> UT66GamblerOverlayWidget::RebuildWidget()
{
	bInputLocked = false;
	GambleAmount = FMath::Max(0, GambleAmount);
	BorrowAmount = FMath::Max(0, BorrowAmount);
	PaybackAmount = FMath::Max(0, PaybackAmount);

	UT66LocalizationSubsystem* Loc = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
		}
	}

	auto MakeTitle = [](const FText& Text) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
			.ColorAndOpacity(FLinearColor::White);
	};

	auto MakeButton = [](const FText& Text, const FOnClicked& OnClicked, const FLinearColor& Bg = FLinearColor(0.2f, 0.2f, 0.28f, 1.f)) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(420.f)
			.HeightOverride(62.f)
			.Padding(FMargin(0.f, 8.f))
			[
				SNew(SButton)
				.OnClicked(OnClicked)
				.ButtonColorAndOpacity(Bg)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Text)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(FLinearColor::White)
				]
			];
	};

	TSharedRef<SWidget> DialoguePage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 20.f)
		[ MakeTitle(Loc ? Loc->GetText_Gambler() : FText::FromString(TEXT("GAMBLER"))) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_GamblerDialoguePrompt() : FText::FromString(TEXT("What do you want?")))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
			.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.f))
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[ MakeButton(Loc ? Loc->GetText_LetMeGamble() : FText::FromString(TEXT("LET ME GAMBLE")), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnDialogueGamble), FLinearColor(0.7f, 0.55f, 0.1f, 1.f)) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(420.f)
			.HeightOverride(62.f)
			.Padding(FMargin(0.f, 8.f))
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnDialogueTeleport))
				.IsEnabled_Lambda([this]() { return !IsBossActive(); })
				.ButtonColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.35f, 1.f))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_TeleportMeToYourBrother() : FText::FromString(TEXT("TELEPORT ME TO YOUR BROTHER")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
		];

	TSharedRef<SWidget> CasinoPage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f, 0.f, 14.f)
		[ MakeTitle(Loc ? Loc->GetText_Casino() : FText::FromString(TEXT("CASINO"))) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 24.f, 0.f)
			[
				// Bank panel
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.10f, 1.f))
				.Padding(16.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_Bank() : FText::FromString(TEXT("BANK")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
						[ SNew(STextBlock).Text(Loc ? Loc->GetText_Bet() : FText::FromString(TEXT("Bet"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)).ColorAndOpacity(FLinearColor::White) ]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
						[
							SAssignNew(GambleAmountSpin, SSpinBox<int32>)
							.MinValue(0).MaxValue(999999).Delta(10)
							.Value_Lambda([this]() { return GambleAmount; })
							.OnValueChanged_Lambda([this](int32 V) { GambleAmount = FMath::Max(0, V); })
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SButton)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnGambleMax))
							[ SNew(STextBlock).Text(Loc ? Loc->GetText_Max() : FText::FromString(TEXT("MAX"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)).ColorAndOpacity(FLinearColor::White) ]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
						[ SNew(STextBlock).Text(Loc ? Loc->GetText_Borrow() : FText::FromString(TEXT("Borrow"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)).ColorAndOpacity(FLinearColor::White) ]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
						[
							SAssignNew(BorrowAmountSpin, SSpinBox<int32>)
							.MinValue(0).MaxValue(999999).Delta(10)
							.Value_Lambda([this]() { return BorrowAmount; })
							.OnValueChanged_Lambda([this](int32 V) { BorrowAmount = FMath::Max(0, V); })
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SButton)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBorrowClicked))
							.ButtonColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.35f, 1.f))
							[ SNew(STextBlock).Text(Loc ? Loc->GetText_Borrow() : FText::FromString(TEXT("BORROW"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)).ColorAndOpacity(FLinearColor::White) ]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
						[ SNew(STextBlock).Text(Loc ? Loc->GetText_Payback() : FText::FromString(TEXT("Payback"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)).ColorAndOpacity(FLinearColor::White) ]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
						[
							SAssignNew(PaybackAmountSpin, SSpinBox<int32>)
							.MinValue(0).MaxValue(999999).Delta(10)
							.Value_Lambda([this]() { return PaybackAmount; })
							.OnValueChanged_Lambda([this](int32 V) { PaybackAmount = FMath::Max(0, V); })
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
						[
							SNew(SButton)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnPaybackMax))
							[ SNew(STextBlock).Text(Loc ? Loc->GetText_Max() : FText::FromString(TEXT("MAX"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)).ColorAndOpacity(FLinearColor::White) ]
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SButton)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnPaybackClicked))
							.ButtonColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.35f, 1.f))
							[ SNew(STextBlock).Text(Loc ? Loc->GetText_Payback() : FText::FromString(TEXT("PAYBACK"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)).ColorAndOpacity(FLinearColor::White) ]
						]
					]
				]
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				// Games panel
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.10f, 1.f))
				.Padding(16.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_Games() : FText::FromString(TEXT("GAMES")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[ MakeButton(Loc ? Loc->GetText_CoinFlip() : FText::FromString(TEXT("COIN FLIP")), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenCoinFlip), FLinearColor(0.55f, 0.50f, 0.12f, 1.f)) ]
					+ SVerticalBox::Slot().AutoHeight()
					[ MakeButton(Loc ? Loc->GetText_RockPaperScissors() : FText::FromString(TEXT("ROCK PAPER SCISSORS")), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenRps), FLinearColor(0.55f, 0.50f, 0.12f, 1.f)) ]
					+ SVerticalBox::Slot().AutoHeight()
					[ MakeButton(Loc ? Loc->GetText_FindTheBall() : FText::FromString(TEXT("FIND THE BALL")), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenFindBall), FLinearColor(0.55f, 0.50f, 0.12f, 1.f)) ]
				]
			]
		];

	TSharedRef<SWidget> CoinFlipPage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f)
		[ MakeTitle(Loc ? Loc->GetText_CoinFlip() : FText::FromString(TEXT("COIN FLIP"))) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_ChooseHeadsOrTails() : FText::FromString(TEXT("Choose Heads or Tails.")))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
			.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.f))
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 16.f)
		[
			SAssignNew(CoinFlipResultText, STextBlock)
			.Text(Loc ? Loc->GetText_ResultDash() : FText::FromString(TEXT("Result: -")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			.ColorAndOpacity(FLinearColor::White)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(CoinFlipHeadsButton, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCoinFlipHeads))
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_Heads() : FText::FromString(TEXT("HEADS")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(CoinFlipTailsButton, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCoinFlipTails))
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_Tails() : FText::FromString(TEXT("TAILS")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
		];

	TSharedRef<SWidget> RpsPage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f)
		[ MakeTitle(Loc ? Loc->GetText_RockPaperScissors() : FText::FromString(TEXT("ROCK PAPER SCISSORS"))) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 16.f)
		[
			SAssignNew(RpsResultText, STextBlock)
			.Text(Loc ? Loc->GetText_PickOne() : FText::FromString(TEXT("Pick one.")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			.ColorAndOpacity(FLinearColor::White)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(RpsRockButton, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnRpsRock))
				[ SNew(STextBlock).Text(Loc ? Loc->GetText_Rock() : FText::FromString(TEXT("ROCK"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 18)).ColorAndOpacity(FLinearColor::White) ]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(RpsPaperButton, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnRpsPaper))
				[ SNew(STextBlock).Text(Loc ? Loc->GetText_Paper() : FText::FromString(TEXT("PAPER"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 18)).ColorAndOpacity(FLinearColor::White) ]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(RpsScissorsButton, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnRpsScissors))
				[ SNew(STextBlock).Text(Loc ? Loc->GetText_Scissors() : FText::FromString(TEXT("SCISSORS"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 18)).ColorAndOpacity(FLinearColor::White) ]
			]
		];

	TSharedRef<SWidget> FindBallPage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f)
		[ MakeTitle(Loc ? Loc->GetText_FindTheBall() : FText::FromString(TEXT("FIND THE BALL"))) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_PickACup() : FText::FromString(TEXT("Pick a cup.")))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
			.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.f))
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 16.f)
		[
			SAssignNew(BallResultText, STextBlock)
			.Text(Loc ? Loc->GetText_ResultDash() : FText::FromString(TEXT("Result: -")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			.ColorAndOpacity(FLinearColor::White)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(BallCup1Button, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBallCup1))
				[ SNew(STextBlock).Text(Loc ? Loc->GetText_Cup(1) : FText::FromString(TEXT("CUP 1"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 18)).ColorAndOpacity(FLinearColor::White) ]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(BallCup2Button, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBallCup2))
				[ SNew(STextBlock).Text(Loc ? Loc->GetText_Cup(2) : FText::FromString(TEXT("CUP 2"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 18)).ColorAndOpacity(FLinearColor::White) ]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(BallCup3Button, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBallCup3))
				[ SNew(STextBlock).Text(Loc ? Loc->GetText_Cup(3) : FText::FromString(TEXT("CUP 3"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 18)).ColorAndOpacity(FLinearColor::White) ]
			]
		];

	TSharedRef<SWidget> Root =
		SNew(SBorder)
		// Full-screen, opaque (no transparency)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 1.f))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(18.f, 14.f, 18.f, 6.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 14.f, 0.f)
					[
						SNew(SButton)
						.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBack))
					[ SNew(STextBlock).Text(Loc ? Loc->GetText_Back() : FText::FromString(TEXT("BACK"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)).ColorAndOpacity(FLinearColor::White) ]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SAssignNew(StatusText, STextBlock)
						.Text(FText::FromString(TEXT("")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SAssignNew(GoldText, STextBlock)
					.Text(FText::GetEmpty())
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(14.f, 0.f, 0.f, 0.f)
					[
						SAssignNew(DebtText, STextBlock)
					.Text(FText::GetEmpty())
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
						.ColorAndOpacity(FLinearColor(0.95f, 0.15f, 0.15f, 1.f))
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(18.f, 0.f, 18.f, 10.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
					[
						SNew(STextBlock)
					.Text(Loc ? Loc->GetText_Anger() : FText::FromString(TEXT("ANGER")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(FLinearColor(1.f, 0.4f, 0.35f, 1.f))
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 2.f, 10.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(240.f)
						.HeightOverride(14.f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.18f, 1.f))
							]
							+ SOverlay::Slot().HAlign(HAlign_Left)
							[
								SAssignNew(AngerFillBox, SBox)
								.WidthOverride(0.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.95f, 0.2f, 0.15f, 1.f))
								]
							]
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SAssignNew(AngerText, STextBlock)
						.Text(FText::FromString(TEXT("0%")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(FLinearColor::White)
					]
				]
				+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Fill).VAlign(VAlign_Fill).Padding(40.f)
				[
					SAssignNew(PageSwitcher, SWidgetSwitcher)
					+ SWidgetSwitcher::Slot()
					[ DialoguePage ]
					+ SWidgetSwitcher::Slot()
					[ CasinoPage ]
					+ SWidgetSwitcher::Slot()
					[ CoinFlipPage ]
					+ SWidgetSwitcher::Slot()
					[ RpsPage ]
					+ SWidgetSwitcher::Slot()
					[ FindBallPage ]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(CheatPromptContainer, SBox)
				.Visibility(EVisibility::Collapsed)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.1f, 0.95f))
					.Padding(18.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
						[
							SNew(STextBlock)
					.Text(Loc ? Loc->GetText_CheatPromptTitle() : FText::FromString(TEXT("Cheat?")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
							.ColorAndOpacity(FLinearColor::White)
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 14.f)
						[
							SNew(STextBlock)
					.Text(Loc ? Loc->GetText_CheatPromptBody() : FText::FromString(TEXT("Cheating increases Anger.")))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
							.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.f))
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(140.f)
								.HeightOverride(44.f)
								[
									SNew(SButton)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCheatYes))
									.ButtonColorAndOpacity(FLinearColor(0.85f, 0.2f, 0.15f, 1.f))
									.HAlign(HAlign_Center).VAlign(VAlign_Center)
									[ SNew(STextBlock).Text(Loc ? Loc->GetText_Yes() : FText::FromString(TEXT("YES"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)).ColorAndOpacity(FLinearColor::White) ]
								]
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(140.f)
								.HeightOverride(44.f)
								[
									SNew(SButton)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCheatNo))
									.ButtonColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.35f, 1.f))
									.HAlign(HAlign_Center).VAlign(VAlign_Center)
									[ SNew(STextBlock).Text(Loc ? Loc->GetText_No() : FText::FromString(TEXT("NO"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)).ColorAndOpacity(FLinearColor::White) ]
								]
							]
						]
					]
				]
			]
		];

	// Ensure initial page + gold display are correct even when re-opening the same widget instance.
	SetPage(EGamblerPage::Dialogue);
	RefreshTopBar();
	return Root;
}

FReply UT66GamblerOverlayWidget::OnBack()
{
	// Back navigates up; if at dialogue, closes the overlay (return to gameplay).
	if (!PageSwitcher.IsValid())
	{
		CloseOverlay();
		return FReply::Handled();
	}

	const int32 Index = PageSwitcher->GetActiveWidgetIndex();
	if (Index == static_cast<int32>(EGamblerPage::Dialogue))
	{
		CloseOverlay();
	}
	else if (Index == static_cast<int32>(EGamblerPage::Casino))
	{
		SetPage(EGamblerPage::Dialogue);
	}
	else
	{
		SetPage(EGamblerPage::Casino);
	}
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnDialogueGamble()
{
	SetPage(EGamblerPage::Casino);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnDialogueTeleport()
{
	if (IsBossActive())
	{
		UT66LocalizationSubsystem* Loc2 = nullptr;
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
			}
		}
		SetStatus(Loc2 ? Loc2->GetText_TeleportDisabledBossActive() : FText::FromString(TEXT("Teleport disabled (boss encounter started).")), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return FReply::Handled();
	}
	TeleportToVendor();
	CloseOverlay();
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnGambleMax()
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				GambleAmount = FMath::Max(0, RunState->GetCurrentGold());
				if (GambleAmountSpin.IsValid())
				{
					GambleAmountSpin->SetValue(GambleAmount);
				}
				SetStatus(FText::GetEmpty());
				RefreshTopBar();
			}
		}
	}
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnBorrowClicked()
{
	if (BorrowAmount <= 0)
	{
		UT66LocalizationSubsystem* Loc2 = nullptr;
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
			}
		}
		SetStatus(Loc2 ? Loc2->GetText_BorrowAmountMustBePositive() : FText::FromString(TEXT("Borrow amount must be > 0.")), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return FReply::Handled();
	}
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->BorrowGold(BorrowAmount);
				UT66LocalizationSubsystem* Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
				const FText Fmt = Loc2 ? Loc2->GetText_BorrowedAmountFormat() : FText::FromString(TEXT("Borrowed {0}."));
				SetStatus(FText::Format(Fmt, FText::AsNumber(BorrowAmount)), FLinearColor(0.3f, 1.f, 0.4f, 1.f));
				RefreshTopBar();
			}
		}
	}
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnPaybackMax()
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				PaybackAmount = FMath::Max(0, RunState->GetCurrentDebt());
				if (PaybackAmountSpin.IsValid())
				{
					PaybackAmountSpin->SetValue(PaybackAmount);
				}
				SetStatus(FText::GetEmpty());
				RefreshTopBar();
			}
		}
	}
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnPaybackClicked()
{
	if (PaybackAmount <= 0)
	{
		UT66LocalizationSubsystem* Loc2 = nullptr;
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
			}
		}
		SetStatus(Loc2 ? Loc2->GetText_PaybackAmountMustBePositive() : FText::FromString(TEXT("Payback amount must be > 0.")), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return FReply::Handled();
	}
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				const int32 Paid = RunState->PayDebt(PaybackAmount);
				if (Paid <= 0)
				{
					UT66LocalizationSubsystem* Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
					SetStatus(Loc2 ? Loc2->GetText_NotEnoughGoldOrNoDebt() : FText::FromString(TEXT("Not enough gold (or no debt).")), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
				}
				else
				{
					UT66LocalizationSubsystem* Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
					const FText Fmt = Loc2 ? Loc2->GetText_PaidBackAmountFormat() : FText::FromString(TEXT("Paid back {0}."));
					SetStatus(FText::Format(Fmt, FText::AsNumber(Paid)), FLinearColor(0.3f, 1.f, 0.4f, 1.f));
				}
				RefreshTopBar();
			}
		}
	}
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnOpenCoinFlip()
{
	SetPage(EGamblerPage::CoinFlip);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnOpenRps()
{
	SetPage(EGamblerPage::RockPaperScissors);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnOpenFindBall()
{
	SetPage(EGamblerPage::FindTheBall);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnCoinFlipHeads() { ResolveCoinFlip(true); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnCoinFlipTails() { ResolveCoinFlip(false); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnRpsRock() { ResolveRps(0); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnRpsPaper() { ResolveRps(1); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnRpsScissors() { ResolveRps(2); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnBallCup1() { ResolveFindBall(0); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnBallCup2() { ResolveFindBall(1); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnBallCup3() { ResolveFindBall(2); return FReply::Handled(); }

void UT66GamblerOverlayWidget::SetPage(EGamblerPage Page)
{
	if (!PageSwitcher.IsValid()) return;
	PageSwitcher->SetActiveWidgetIndex(static_cast<int32>(Page));
	bInputLocked = false;
	SetStatus(FText::GetEmpty());
	RefreshTopBar();

	// Clear per-game result text when opening pages
	UT66LocalizationSubsystem* Loc2 = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
		}
	}
	if (Page == EGamblerPage::CoinFlip && CoinFlipResultText.IsValid()) CoinFlipResultText->SetText(Loc2 ? Loc2->GetText_ResultDash() : FText::FromString(TEXT("Result: -")));
	if (Page == EGamblerPage::RockPaperScissors && RpsResultText.IsValid()) RpsResultText->SetText(Loc2 ? Loc2->GetText_PickOne() : FText::FromString(TEXT("Pick one.")));
	if (Page == EGamblerPage::FindTheBall && BallResultText.IsValid()) BallResultText->SetText(Loc2 ? Loc2->GetText_ResultDash() : FText::FromString(TEXT("Result: -")));
}

void UT66GamblerOverlayWidget::RefreshTopBar()
{
	if (!GoldText.IsValid()) return;
	int32 Gold = 0;
	int32 Debt = 0;
	float Anger01 = 0.f;
	UT66LocalizationSubsystem* Loc2 = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				Gold = RunState->GetCurrentGold();
				Debt = RunState->GetCurrentDebt();
				Anger01 = RunState->GetGamblerAnger01();
			}
		}
	}
	const FText GoldFmt = Loc2 ? Loc2->GetText_GoldFormat() : FText::FromString(TEXT("Gold: {0}"));
	const FText OweFmt = Loc2 ? Loc2->GetText_OweFormat() : FText::FromString(TEXT("Owe: {0}"));
	GoldText->SetText(FText::Format(GoldFmt, FText::AsNumber(Gold)));
	if (DebtText.IsValid())
	{
		DebtText->SetText(FText::Format(OweFmt, FText::AsNumber(Debt)));
	}
	if (AngerFillBox.IsValid())
	{
		const float Full = 240.f;
		AngerFillBox->SetWidthOverride(FMath::Clamp(Full * Anger01, 0.f, Full));
	}
	if (AngerText.IsValid())
	{
		const int32 Pct = FMath::RoundToInt(Anger01 * 100.f);
		AngerText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Pct)));
	}
}

void UT66GamblerOverlayWidget::ShowCheatPrompt()
{
	bCheatPromptVisible = true;
	if (CheatPromptContainer.IsValid())
	{
		CheatPromptContainer->SetVisibility(EVisibility::Visible);
	}
}

void UT66GamblerOverlayWidget::HideCheatPrompt()
{
	bCheatPromptVisible = false;
	if (CheatPromptContainer.IsValid())
	{
		CheatPromptContainer->SetVisibility(EVisibility::Collapsed);
	}
}

void UT66GamblerOverlayWidget::TriggerGamblerBossIfAngry()
{
	UWorld* World = GetWorld();
	if (!World) return;
	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	if (RunState->GetGamblerAnger01() < 1.f)
	{
		return;
	}

	// Spawn GamblerBoss at gambler NPC location (and remove the NPC).
	FVector SpawnLoc = FVector::ZeroVector;
	AT66GamblerNPC* Gambler = nullptr;
	for (TActorIterator<AT66GamblerNPC> It(World); It; ++It)
	{
		Gambler = *It;
		break;
	}
	if (Gambler)
	{
		SpawnLoc = Gambler->GetActorLocation();
		Gambler->Destroy();
	}
	else
	{
		if (APawn* P = GetOwningPlayerPawn())
		{
			SpawnLoc = P->GetActorLocation() + FVector(600.f, 0.f, 0.f);
		}
	}

	// Avoid duplicates
	for (TActorIterator<AT66GamblerBoss> It(World); It; ++It)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AT66GamblerBoss>(AT66GamblerBoss::StaticClass(), SpawnLoc + FVector(0.f, 0.f, 200.f), FRotator::ZeroRotator, Params);
}

bool UT66GamblerOverlayWidget::IsBossActive() const
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				return RunState->GetBossActive();
			}
		}
	}
	return false;
}

bool UT66GamblerOverlayWidget::TryPayToPlay()
{
	if (bInputLocked) return false;
	if (GambleAmount <= 0)
	{
		UT66LocalizationSubsystem* Loc2 = nullptr;
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
			}
		}
		SetStatus(Loc2 ? Loc2->GetText_GambleAmountMustBePositive() : FText::FromString(TEXT("Gamble amount must be > 0.")), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return false;
	}
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				if (!RunState->TrySpendGold(GambleAmount))
				{
					UT66LocalizationSubsystem* Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
					SetStatus(Loc2 ? Loc2->GetText_NotEnoughGold() : FText::FromString(TEXT("Not enough gold.")), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
					RefreshTopBar();
					return false;
				}
				RefreshTopBar();
				return true;
			}
		}
	}
	return false;
}

void UT66GamblerOverlayWidget::AwardWin()
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				// "Double up": after spending Bet, winning returns 2*Bet (net +Bet).
				const int64 Payout = static_cast<int64>(GambleAmount) * 2;
				RunState->AddGold(static_cast<int32>(FMath::Clamp<int64>(Payout, 0, INT32_MAX)));
				RefreshTopBar();
			}
		}
	}
}

void UT66GamblerOverlayWidget::ResolveCoinFlip(bool bChoseHeads)
{
	if (!TryPayToPlay()) return;
	bInputLocked = true;
	PendingRevealType = ERevealType::CoinFlip;
	bPendingCoinFlipChoseHeads = bChoseHeads;
	ShowCheatPrompt();
}

void UT66GamblerOverlayWidget::ResolveRps(int32 PlayerChoice)
{
	if (!TryPayToPlay()) return;
	bInputLocked = true;
	PendingRevealType = ERevealType::RockPaperScissors;
	PendingRpsPlayerChoice = FMath::Clamp(PlayerChoice, 0, 2);
	ShowCheatPrompt();
}

void UT66GamblerOverlayWidget::ResolveFindBall(int32 ChosenCup)
{
	if (!TryPayToPlay()) return;
	bInputLocked = true;
	PendingRevealType = ERevealType::FindTheBall;
	PendingFindBallChosenCup = FMath::Clamp(ChosenCup, 0, 2);
	ShowCheatPrompt();
}

FReply UT66GamblerOverlayWidget::OnCheatYes()
{
	if (!bInputLocked || PendingRevealType == ERevealType::None) return FReply::Handled();
	HideCheatPrompt();
	bPendingCheat = true;

	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
	}
	if (RunState)
	{
		RunState->AddGamblerAngerFromBet(GambleAmount);
		RefreshTopBar();
	}

	// If anger hit 100%, close overlay and spawn Gambler boss immediately.
	if (RunState && RunState->GetGamblerAnger01() >= 1.f)
	{
		CloseOverlay();
		TriggerGamblerBossIfAngry();
		return FReply::Handled();
	}

	bPendingWin = true;
	bPendingDraw = false;

	UT66LocalizationSubsystem* Loc2 = RunState ? RunState->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	// Force a win by forcing the "result" to match the player's choice.
	switch (PendingRevealType)
	{
		case ERevealType::CoinFlip:
			bPendingCoinFlipResultHeads = bPendingCoinFlipChoseHeads;
			if (CoinFlipResultText.IsValid()) CoinFlipResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : FText::FromString(TEXT("Rolling...")));
			break;
		case ERevealType::RockPaperScissors:
			// Opponent choice that loses to player
			PendingRpsOppChoice = (PendingRpsPlayerChoice + 2) % 3;
			if (RpsResultText.IsValid()) RpsResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : FText::FromString(TEXT("Rolling...")));
			break;
		case ERevealType::FindTheBall:
			PendingFindBallCorrectCup = PendingFindBallChosenCup;
			if (BallResultText.IsValid()) BallResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : FText::FromString(TEXT("Rolling...")));
			break;
		default:
			break;
	}

	if (World)
	{
		World->GetTimerManager().ClearTimer(RevealTimerHandle);
		World->GetTimerManager().SetTimer(RevealTimerHandle, this, &UT66GamblerOverlayWidget::RevealPendingOutcome, 0.35f, false);
	}
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnCheatNo()
{
	if (!bInputLocked || PendingRevealType == ERevealType::None) return FReply::Handled();
	HideCheatPrompt();
	bPendingCheat = false;

	UWorld* World = GetWorld();
	UT66LocalizationSubsystem* Loc2 = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	bPendingDraw = false;

	switch (PendingRevealType)
	{
		case ERevealType::CoinFlip:
		{
			bPendingCoinFlipResultHeads = FMath::RandBool();
			bPendingWin = (bPendingCoinFlipChoseHeads == bPendingCoinFlipResultHeads);
			if (CoinFlipResultText.IsValid()) CoinFlipResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : FText::FromString(TEXT("Rolling...")));
			break;
		}
		case ERevealType::RockPaperScissors:
		{
			do
			{
				PendingRpsOppChoice = FMath::RandRange(0, 2);
			} while (PendingRpsOppChoice == PendingRpsPlayerChoice);
			bPendingWin = (PendingRpsPlayerChoice == 0 && PendingRpsOppChoice == 2)
				|| (PendingRpsPlayerChoice == 1 && PendingRpsOppChoice == 0)
				|| (PendingRpsPlayerChoice == 2 && PendingRpsOppChoice == 1);
			if (RpsResultText.IsValid()) RpsResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : FText::FromString(TEXT("Rolling...")));
			break;
		}
		case ERevealType::FindTheBall:
		{
			PendingFindBallCorrectCup = FMath::RandRange(0, 2);
			bPendingWin = (PendingFindBallChosenCup == PendingFindBallCorrectCup);
			if (BallResultText.IsValid()) BallResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : FText::FromString(TEXT("Rolling...")));
			break;
		}
		default:
			bPendingWin = false;
			break;
	}

	if (World)
	{
		World->GetTimerManager().ClearTimer(RevealTimerHandle);
		World->GetTimerManager().SetTimer(RevealTimerHandle, this, &UT66GamblerOverlayWidget::RevealPendingOutcome, 0.35f, false);
	}
	return FReply::Handled();
}

void UT66GamblerOverlayWidget::RevealPendingOutcome()
{
	UT66LocalizationSubsystem* Loc2 = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
		}
	}

	switch (PendingRevealType)
	{
		case ERevealType::CoinFlip:
		{
			const FText ResultStr = bPendingCoinFlipResultHeads
				? (Loc2 ? Loc2->GetText_Heads() : FText::FromString(TEXT("Heads")))
				: (Loc2 ? Loc2->GetText_Tails() : FText::FromString(TEXT("Tails")));
			if (bPendingWin) AwardWin();
			if (CoinFlipResultText.IsValid())
			{
				const FText Outcome = bPendingWin ? (Loc2 ? Loc2->GetText_Win() : FText::FromString(TEXT("WIN"))) : (Loc2 ? Loc2->GetText_Lose() : FText::FromString(TEXT("LOSE")));
				const FText Fmt = Loc2 ? Loc2->GetText_CoinFlipResultFormat() : FText::FromString(TEXT("Result: {0} — {1}"));
				CoinFlipResultText->SetText(FText::Format(Fmt, ResultStr, Outcome));
			}
			const FText WinFmt = Loc2 ? Loc2->GetText_WinPlusAmountFormat() : FText::FromString(TEXT("WIN (+{0})"));
			const FText Status = bPendingWin ? FText::Format(WinFmt, FText::AsNumber(GambleAmount)) : (Loc2 ? Loc2->GetText_Lose() : FText::FromString(TEXT("LOSE")));
			SetStatus(Status,
				bPendingWin ? FLinearColor(0.3f, 1.f, 0.4f, 1.f) : FLinearColor(1.f, 0.3f, 0.3f, 1.f));
			break;
		}
		case ERevealType::RockPaperScissors:
		{
			if (bPendingWin) AwardWin();
			if (RpsResultText.IsValid())
			{
				const FText YouChoice = (PendingRpsPlayerChoice == 0) ? (Loc2 ? Loc2->GetText_Rock() : FText::FromString(TEXT("Rock")))
					: (PendingRpsPlayerChoice == 1) ? (Loc2 ? Loc2->GetText_Paper() : FText::FromString(TEXT("Paper")))
					: (Loc2 ? Loc2->GetText_Scissors() : FText::FromString(TEXT("Scissors")));
				const FText OppChoice = (PendingRpsOppChoice == 0) ? (Loc2 ? Loc2->GetText_Rock() : FText::FromString(TEXT("Rock")))
					: (PendingRpsOppChoice == 1) ? (Loc2 ? Loc2->GetText_Paper() : FText::FromString(TEXT("Paper")))
					: (Loc2 ? Loc2->GetText_Scissors() : FText::FromString(TEXT("Scissors")));
				const FText Outcome = bPendingWin ? (Loc2 ? Loc2->GetText_Win() : FText::FromString(TEXT("WIN"))) : (Loc2 ? Loc2->GetText_Lose() : FText::FromString(TEXT("LOSE")));
				const FText Fmt = Loc2 ? Loc2->GetText_RpsResultFormat() : FText::FromString(TEXT("You: {0}  |  Gambler: {1}  —  {2}"));
				RpsResultText->SetText(FText::Format(Fmt, YouChoice, OppChoice, Outcome));
			}
			const FText WinFmt = Loc2 ? Loc2->GetText_WinPlusAmountFormat() : FText::FromString(TEXT("WIN (+{0})"));
			const FText Status = bPendingWin ? FText::Format(WinFmt, FText::AsNumber(GambleAmount)) : (Loc2 ? Loc2->GetText_Lose() : FText::FromString(TEXT("LOSE")));
			SetStatus(Status,
				bPendingWin ? FLinearColor(0.3f, 1.f, 0.4f, 1.f) : FLinearColor(1.f, 0.3f, 0.3f, 1.f));
			break;
		}
		case ERevealType::FindTheBall:
		{
			if (bPendingWin) AwardWin();
			if (BallResultText.IsValid())
			{
				const FText Cup = Loc2 ? Loc2->GetText_Cup(PendingFindBallCorrectCup + 1) : FText::FromString(FString::Printf(TEXT("CUP %d"), PendingFindBallCorrectCup + 1));
				const FText Outcome = bPendingWin ? (Loc2 ? Loc2->GetText_Win() : FText::FromString(TEXT("WIN"))) : (Loc2 ? Loc2->GetText_Lose() : FText::FromString(TEXT("LOSE")));
				const FText Fmt = Loc2 ? Loc2->GetText_FindBallResultFormat() : FText::FromString(TEXT("Ball was under {0} — {1}"));
				BallResultText->SetText(FText::Format(Fmt, Cup, Outcome));
			}
			const FText WinFmt = Loc2 ? Loc2->GetText_WinPlusAmountFormat() : FText::FromString(TEXT("WIN (+{0})"));
			const FText Status = bPendingWin ? FText::Format(WinFmt, FText::AsNumber(GambleAmount)) : (Loc2 ? Loc2->GetText_Lose() : FText::FromString(TEXT("LOSE")));
			SetStatus(Status,
				bPendingWin ? FLinearColor(0.3f, 1.f, 0.4f, 1.f) : FLinearColor(1.f, 0.3f, 0.3f, 1.f));
			break;
		}
		default:
			break;
	}

	PendingRevealType = ERevealType::None;
	bInputLocked = false;
}

void UT66GamblerOverlayWidget::TeleportToVendor()
{
	UWorld* World = GetWorld();
	if (!World) return;

	APawn* Pawn = GetOwningPlayerPawn();
	if (!Pawn) return;

	AT66VendorNPC* Vendor = nullptr;
	for (TActorIterator<AT66VendorNPC> It(World); It; ++It)
	{
		Vendor = *It;
		break;
	}
	if (!Vendor) return;

	const FVector VendorLoc = Vendor->GetActorLocation();
	Pawn->SetActorLocation(VendorLoc + FVector(250.f, 0.f, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
}

void UT66GamblerOverlayWidget::CloseOverlay()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RevealTimerHandle);
	}
	PendingRevealType = ERevealType::None;
	bInputLocked = false;
	HideCheatPrompt();

	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
}

void UT66GamblerOverlayWidget::SetStatus(const FText& Msg, const FLinearColor& Color)
{
	if (StatusText.IsValid())
	{
		StatusText->SetText(Msg);
		StatusText->SetColorAndOpacity(FSlateColor(Color));
	}
}

