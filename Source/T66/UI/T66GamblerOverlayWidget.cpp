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
#include "UI/Style/T66Style.h"
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

	const ISlateStyle& Style = FT66Style::Get();
	const FButtonStyle& BtnPrimary = Style.GetWidgetStyle<FButtonStyle>("T66.Button.Primary");
	const FButtonStyle& BtnNeutral = Style.GetWidgetStyle<FButtonStyle>("T66.Button.Neutral");
	const FButtonStyle& BtnDanger = Style.GetWidgetStyle<FButtonStyle>("T66.Button.Danger");

	const FTextBlockStyle& TextTitle = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Title");
	const FTextBlockStyle& TextHeading = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading");
	const FTextBlockStyle& TextBody = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Body");
	const FTextBlockStyle& TextChip = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip");
	const FTextBlockStyle& TextButton = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Button");

	auto MakeTitle = [&](const FText& Text) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(Text)
			.TextStyle(&TextTitle)
			.ColorAndOpacity(FT66Style::Tokens::Text);
	};

	auto MakeButton = [&](const FText& Text, const FOnClicked& OnClicked, const FButtonStyle* BtnStyle = nullptr) -> TSharedRef<SWidget>
	{
		const FButtonStyle* Use = BtnStyle ? BtnStyle : &BtnNeutral;
		return SNew(SBox)
			.MinDesiredWidth(420.f)
			.HeightOverride(56.f)
			.Padding(FMargin(0.f, 8.f))
			[
				SNew(SButton)
				.OnClicked(OnClicked)
				.ButtonStyle(Use)
				.ContentPadding(FMargin(18.f, 10.f))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Text)
					.TextStyle(&TextButton)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			];
	};

	TSharedRef<SWidget> DialoguePage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 20.f)
		[ MakeTitle(Loc ? Loc->GetText_Gambler() : FText::GetEmpty()) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_GamblerDialoguePrompt() : FText::GetEmpty())
			.TextStyle(&TextBody)
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[ MakeButton(Loc ? Loc->GetText_LetMeGamble() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnDialogueGamble), &BtnPrimary) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SBox)
			.MinDesiredWidth(420.f)
			.HeightOverride(56.f)
			.Padding(FMargin(0.f, 8.f))
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnDialogueTeleport))
				.IsEnabled_Lambda([this]() { return !IsBossActive(); })
				.ButtonStyle(&BtnNeutral)
				.ContentPadding(FMargin(18.f, 10.f))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_TeleportMeToYourBrother() : FText::GetEmpty())
					.TextStyle(&TextButton)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
		];

	TSharedRef<SWidget> CasinoPage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f, 0.f, 14.f)
		[ MakeTitle(Loc ? Loc->GetText_Casino() : FText::GetEmpty()) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 24.f, 0.f)
			[
				// Bank panel
				SNew(SBorder)
				.BorderImage(Style.GetBrush("T66.Brush.Panel"))
				.BorderBackgroundColor(FT66Style::Tokens::Panel)
				.Padding(FT66Style::Tokens::Space6)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_Bank() : FText::GetEmpty())
						.TextStyle(&TextHeading)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
						[ SNew(STextBlock).Text(Loc ? Loc->GetText_Bet() : FText::GetEmpty()).TextStyle(&TextBody).ColorAndOpacity(FT66Style::Tokens::Text) ]
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
							.ButtonStyle(&BtnNeutral)
							.ContentPadding(FMargin(14.f, 8.f))
							[
								SNew(STextBlock)
								.Text(Loc ? Loc->GetText_Max() : FText::GetEmpty())
								.TextStyle(&TextButton)
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
						[ SNew(STextBlock).Text(Loc ? Loc->GetText_Borrow() : FText::GetEmpty()).TextStyle(&TextBody).ColorAndOpacity(FT66Style::Tokens::Text) ]
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
							.ButtonStyle(&BtnNeutral)
							.ContentPadding(FMargin(14.f, 8.f))
							[
								SNew(STextBlock)
								.Text(Loc ? Loc->GetText_Borrow() : FText::GetEmpty())
								.TextStyle(&TextButton)
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
						[ SNew(STextBlock).Text(Loc ? Loc->GetText_Payback() : FText::GetEmpty()).TextStyle(&TextBody).ColorAndOpacity(FT66Style::Tokens::Text) ]
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
							.ButtonStyle(&BtnNeutral)
							.ContentPadding(FMargin(14.f, 8.f))
							[
								SNew(STextBlock)
								.Text(Loc ? Loc->GetText_Max() : FText::GetEmpty())
								.TextStyle(&TextButton)
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SButton)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnPaybackClicked))
							.ButtonStyle(&BtnNeutral)
							.ContentPadding(FMargin(14.f, 8.f))
							[
								SNew(STextBlock)
								.Text(Loc ? Loc->GetText_Payback() : FText::GetEmpty())
								.TextStyle(&TextButton)
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
						]
					]
				]
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				// Games panel
				SNew(SBorder)
				.BorderImage(Style.GetBrush("T66.Brush.Panel"))
				.BorderBackgroundColor(FT66Style::Tokens::Panel)
				.Padding(FT66Style::Tokens::Space6)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_Games() : FText::GetEmpty())
						.TextStyle(&TextHeading)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[ MakeButton(Loc ? Loc->GetText_CoinFlip() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenCoinFlip), &BtnPrimary) ]
					+ SVerticalBox::Slot().AutoHeight()
					[ MakeButton(Loc ? Loc->GetText_RockPaperScissors() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenRps), &BtnPrimary) ]
					+ SVerticalBox::Slot().AutoHeight()
					[ MakeButton(Loc ? Loc->GetText_FindTheBall() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenFindBall), &BtnPrimary) ]
				]
			]
		];

	TSharedRef<SWidget> CoinFlipPage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f)
		[ MakeTitle(Loc ? Loc->GetText_CoinFlip() : FText::GetEmpty()) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_ChooseHeadsOrTails() : FText::GetEmpty())
			.TextStyle(&TextBody)
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 16.f)
		[
			SAssignNew(CoinFlipResultText, STextBlock)
			.Text(Loc ? Loc->GetText_ResultDash() : FText::GetEmpty())
			.TextStyle(&TextHeading)
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(CoinFlipHeadsButton, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCoinFlipHeads))
				.ButtonStyle(&BtnPrimary)
				.ContentPadding(FMargin(18.f, 10.f))
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_Heads() : FText::GetEmpty())
					.TextStyle(&TextButton)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(CoinFlipTailsButton, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCoinFlipTails))
				.ButtonStyle(&BtnPrimary)
				.ContentPadding(FMargin(18.f, 10.f))
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_Tails() : FText::GetEmpty())
					.TextStyle(&TextButton)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
		];

	TSharedRef<SWidget> RpsPage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f)
		[ MakeTitle(Loc ? Loc->GetText_RockPaperScissors() : FText::GetEmpty()) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 16.f)
		[
			SAssignNew(RpsResultText, STextBlock)
			.Text(Loc ? Loc->GetText_PickOne() : FText::GetEmpty())
			.TextStyle(&TextHeading)
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(RpsRockButton, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnRpsRock))
				.ButtonStyle(&BtnPrimary)
				.ContentPadding(FMargin(18.f, 10.f))
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_Rock() : FText::GetEmpty())
					.TextStyle(&TextButton)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(RpsPaperButton, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnRpsPaper))
				.ButtonStyle(&BtnPrimary)
				.ContentPadding(FMargin(18.f, 10.f))
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_Paper() : FText::GetEmpty())
					.TextStyle(&TextButton)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(RpsScissorsButton, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnRpsScissors))
				.ButtonStyle(&BtnPrimary)
				.ContentPadding(FMargin(18.f, 10.f))
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_Scissors() : FText::GetEmpty())
					.TextStyle(&TextButton)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
		];

	TSharedRef<SWidget> FindBallPage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f)
		[ MakeTitle(Loc ? Loc->GetText_FindTheBall() : FText::GetEmpty()) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_PickACup() : FText::GetEmpty())
			.TextStyle(&TextBody)
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 16.f)
		[
			SAssignNew(BallResultText, STextBlock)
			.Text(Loc ? Loc->GetText_ResultDash() : FText::GetEmpty())
			.TextStyle(&TextHeading)
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(BallCup1Button, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBallCup1))
				.ButtonStyle(&BtnPrimary)
				.ContentPadding(FMargin(18.f, 10.f))
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_Cup(1) : FText::GetEmpty())
					.TextStyle(&TextButton)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(BallCup2Button, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBallCup2))
				.ButtonStyle(&BtnPrimary)
				.ContentPadding(FMargin(18.f, 10.f))
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_Cup(2) : FText::GetEmpty())
					.TextStyle(&TextButton)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SAssignNew(BallCup3Button, SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBallCup3))
				.ButtonStyle(&BtnPrimary)
				.ContentPadding(FMargin(18.f, 10.f))
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_Cup(3) : FText::GetEmpty())
					.TextStyle(&TextButton)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
		];

	TSharedRef<SWidget> Root =
		SNew(SBorder)
		// Full-screen, opaque (no transparency)
		.BorderImage(Style.GetBrush("T66.Brush.Bg"))
		.BorderBackgroundColor(FT66Style::Tokens::Bg)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(FT66Style::Tokens::Space6, FT66Style::Tokens::Space4, FT66Style::Tokens::Space6, FT66Style::Tokens::Space2)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66Style::Tokens::Space4, 0.f)
					[
						SNew(SButton)
						.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBack))
						.ButtonStyle(&BtnNeutral)
						.ContentPadding(FMargin(18.f, 10.f))
						[
							SNew(STextBlock)
							.Text(Loc ? Loc->GetText_Back() : FText::GetEmpty())
							.TextStyle(&TextButton)
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SAssignNew(StatusText, STextBlock)
						.Text(FText::GetEmpty())
						.TextStyle(&TextBody)
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SAssignNew(GoldText, STextBlock)
					.Text(FText::GetEmpty())
						.TextStyle(&TextChip)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FT66Style::Tokens::Space4, 0.f, 0.f, 0.f)
					[
						SAssignNew(DebtText, STextBlock)
					.Text(FText::GetEmpty())
						.TextStyle(&TextChip)
						.ColorAndOpacity(FT66Style::Tokens::Danger)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(FT66Style::Tokens::Space6, 0.f, FT66Style::Tokens::Space6, FT66Style::Tokens::Space3)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, FT66Style::Tokens::Space3, 0.f)
					[
						SNew(STextBlock)
					.Text(Loc ? Loc->GetText_Anger() : FText::GetEmpty())
						.TextStyle(&TextChip)
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 2.f, FT66Style::Tokens::Space3, 0.f)
					[
						SNew(SBox)
						.WidthOverride(240.f)
						.HeightOverride(14.f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SBorder)
								.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
								.BorderBackgroundColor(FT66Style::Tokens::Panel2)
							]
							+ SOverlay::Slot().HAlign(HAlign_Left)
							[
								SAssignNew(AngerFillBox, SBox)
								.WidthOverride(0.f)
								[
									SNew(SBorder)
									.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
									.BorderBackgroundColor(FT66Style::Tokens::Danger)
								]
							]
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SAssignNew(AngerText, STextBlock)
						.Text(FText::AsPercent(0.0))
						.TextStyle(&TextChip)
						.ColorAndOpacity(FT66Style::Tokens::Text)
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
					.BorderImage(Style.GetBrush("T66.Brush.Panel"))
					.BorderBackgroundColor(FT66Style::Tokens::Panel)
					.Padding(FT66Style::Tokens::Space6)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
						[
							SNew(STextBlock)
					.Text(Loc ? Loc->GetText_CheatPromptTitle() : FText::GetEmpty())
							.TextStyle(&TextHeading)
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 14.f)
						[
							SNew(STextBlock)
					.Text(Loc ? Loc->GetText_CheatPromptBody() : FText::GetEmpty())
							.TextStyle(&TextBody)
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
							[
								SNew(SBox)
								.MinDesiredWidth(140.f)
								.HeightOverride(48.f)
								[
									SNew(SButton)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCheatYes))
									.ButtonStyle(&BtnDanger)
									.ContentPadding(FMargin(18.f, 10.f))
									.HAlign(HAlign_Center).VAlign(VAlign_Center)
									[
										SNew(STextBlock)
										.Text(Loc ? Loc->GetText_Yes() : FText::GetEmpty())
										.TextStyle(&TextButton)
										.ColorAndOpacity(FT66Style::Tokens::Text)
									]
								]
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
							[
								SNew(SBox)
								.MinDesiredWidth(140.f)
								.HeightOverride(48.f)
								[
									SNew(SButton)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCheatNo))
									.ButtonStyle(&BtnNeutral)
									.ContentPadding(FMargin(18.f, 10.f))
									.HAlign(HAlign_Center).VAlign(VAlign_Center)
									[
										SNew(STextBlock)
										.Text(Loc ? Loc->GetText_No() : FText::GetEmpty())
										.TextStyle(&TextButton)
										.ColorAndOpacity(FT66Style::Tokens::Text)
									]
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
		SetStatus(Loc2 ? Loc2->GetText_TeleportDisabledBossActive() : NSLOCTEXT("T66.Gambler", "TeleportDisabledBossActive", "Teleport disabled (boss encounter started)."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
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
		SetStatus(Loc2 ? Loc2->GetText_BorrowAmountMustBePositive() : NSLOCTEXT("T66.Gambler", "BorrowAmountMustBePositive", "Borrow amount must be > 0."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
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
				const FText Fmt = Loc2 ? Loc2->GetText_BorrowedAmountFormat() : NSLOCTEXT("T66.Gambler", "BorrowedAmountFormat", "Borrowed {0}.");
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
		SetStatus(Loc2 ? Loc2->GetText_PaybackAmountMustBePositive() : NSLOCTEXT("T66.Gambler", "PaybackAmountMustBePositive", "Payback amount must be > 0."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
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
					SetStatus(Loc2 ? Loc2->GetText_NotEnoughGoldOrNoDebt() : NSLOCTEXT("T66.Gambler", "NotEnoughGoldOrNoDebt", "Not enough gold (or no debt)."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
				}
				else
				{
					UT66LocalizationSubsystem* Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
					const FText Fmt = Loc2 ? Loc2->GetText_PaidBackAmountFormat() : NSLOCTEXT("T66.Gambler", "PaidBackAmountFormat", "Paid back {0}.");
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
	if (Page == EGamblerPage::CoinFlip && CoinFlipResultText.IsValid()) CoinFlipResultText->SetText(Loc2 ? Loc2->GetText_ResultDash() : NSLOCTEXT("T66.Gambler", "ResultDash", "Result: -"));
	if (Page == EGamblerPage::RockPaperScissors && RpsResultText.IsValid()) RpsResultText->SetText(Loc2 ? Loc2->GetText_PickOne() : NSLOCTEXT("T66.Gambler", "PickOne", "Pick one."));
	if (Page == EGamblerPage::FindTheBall && BallResultText.IsValid()) BallResultText->SetText(Loc2 ? Loc2->GetText_ResultDash() : NSLOCTEXT("T66.Gambler", "ResultDash", "Result: -"));
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
	const FText GoldFmt = Loc2 ? Loc2->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}");
	const FText OweFmt = Loc2 ? Loc2->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Owe: {0}");
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
		AngerText->SetText(FText::Format(NSLOCTEXT("T66.Common", "PercentInt", "{0}%"), FText::AsNumber(Pct)));
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
		SetStatus(Loc2 ? Loc2->GetText_GambleAmountMustBePositive() : NSLOCTEXT("T66.Gambler", "GambleAmountMustBePositive", "Gamble amount must be > 0."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
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
					SetStatus(Loc2 ? Loc2->GetText_NotEnoughGold() : NSLOCTEXT("T66.Gambler", "NotEnoughGold", "Not enough gold."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
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
			if (CoinFlipResultText.IsValid()) CoinFlipResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : NSLOCTEXT("T66.Gambler", "Rolling", "Rolling..."));
			break;
		case ERevealType::RockPaperScissors:
			// Opponent choice that loses to player
			PendingRpsOppChoice = (PendingRpsPlayerChoice + 2) % 3;
			if (RpsResultText.IsValid()) RpsResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : NSLOCTEXT("T66.Gambler", "Rolling", "Rolling..."));
			break;
		case ERevealType::FindTheBall:
			PendingFindBallCorrectCup = PendingFindBallChosenCup;
			if (BallResultText.IsValid()) BallResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : NSLOCTEXT("T66.Gambler", "Rolling", "Rolling..."));
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
			if (CoinFlipResultText.IsValid()) CoinFlipResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : NSLOCTEXT("T66.Gambler", "Rolling", "Rolling..."));
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
			if (RpsResultText.IsValid()) RpsResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : NSLOCTEXT("T66.Gambler", "Rolling", "Rolling..."));
			break;
		}
		case ERevealType::FindTheBall:
		{
			PendingFindBallCorrectCup = FMath::RandRange(0, 2);
			bPendingWin = (PendingFindBallChosenCup == PendingFindBallCorrectCup);
			if (BallResultText.IsValid()) BallResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : NSLOCTEXT("T66.Gambler", "Rolling", "Rolling..."));
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
				? (Loc2 ? Loc2->GetText_Heads() : NSLOCTEXT("T66.Gambler", "Heads", "HEADS"))
				: (Loc2 ? Loc2->GetText_Tails() : NSLOCTEXT("T66.Gambler", "Tails", "TAILS"));
			if (bPendingWin) AwardWin();
			if (CoinFlipResultText.IsValid())
			{
				const FText Outcome = bPendingWin
					? (Loc2 ? Loc2->GetText_Win() : NSLOCTEXT("T66.Gambler", "Win", "WIN"))
					: (Loc2 ? Loc2->GetText_Lose() : NSLOCTEXT("T66.Gambler", "Lose", "LOSE"));
				const FText Fmt = Loc2 ? Loc2->GetText_CoinFlipResultFormat() : NSLOCTEXT("T66.Gambler", "CoinFlipResultFormat", "Result: {0} — {1}");
				CoinFlipResultText->SetText(FText::Format(Fmt, ResultStr, Outcome));
			}
			const FText WinFmt = Loc2 ? Loc2->GetText_WinPlusAmountFormat() : NSLOCTEXT("T66.Gambler", "WinPlusAmountFormat", "WIN (+{0})");
			const FText Status = bPendingWin
				? FText::Format(WinFmt, FText::AsNumber(GambleAmount))
				: (Loc2 ? Loc2->GetText_Lose() : NSLOCTEXT("T66.Gambler", "Lose", "LOSE"));
			SetStatus(Status,
				bPendingWin ? FLinearColor(0.3f, 1.f, 0.4f, 1.f) : FLinearColor(1.f, 0.3f, 0.3f, 1.f));
			break;
		}
		case ERevealType::RockPaperScissors:
		{
			if (bPendingWin) AwardWin();
			if (RpsResultText.IsValid())
			{
				const FText YouChoice = (PendingRpsPlayerChoice == 0) ? (Loc2 ? Loc2->GetText_Rock() : NSLOCTEXT("T66.Gambler", "Rock", "Rock"))
					: (PendingRpsPlayerChoice == 1) ? (Loc2 ? Loc2->GetText_Paper() : NSLOCTEXT("T66.Gambler", "Paper", "Paper"))
					: (Loc2 ? Loc2->GetText_Scissors() : NSLOCTEXT("T66.Gambler", "Scissors", "Scissors"));
				const FText OppChoice = (PendingRpsOppChoice == 0) ? (Loc2 ? Loc2->GetText_Rock() : NSLOCTEXT("T66.Gambler", "Rock", "Rock"))
					: (PendingRpsOppChoice == 1) ? (Loc2 ? Loc2->GetText_Paper() : NSLOCTEXT("T66.Gambler", "Paper", "Paper"))
					: (Loc2 ? Loc2->GetText_Scissors() : NSLOCTEXT("T66.Gambler", "Scissors", "Scissors"));
				const FText Outcome = bPendingWin
					? (Loc2 ? Loc2->GetText_Win() : NSLOCTEXT("T66.Gambler", "Win", "WIN"))
					: (Loc2 ? Loc2->GetText_Lose() : NSLOCTEXT("T66.Gambler", "Lose", "LOSE"));
				const FText Fmt = Loc2 ? Loc2->GetText_RpsResultFormat() : NSLOCTEXT("T66.Gambler", "RpsResultFormat", "You: {0}  |  Gambler: {1}  —  {2}");
				RpsResultText->SetText(FText::Format(Fmt, YouChoice, OppChoice, Outcome));
			}
			const FText WinFmt = Loc2 ? Loc2->GetText_WinPlusAmountFormat() : NSLOCTEXT("T66.Gambler", "WinPlusAmountFormat", "WIN (+{0})");
			const FText Status = bPendingWin
				? FText::Format(WinFmt, FText::AsNumber(GambleAmount))
				: (Loc2 ? Loc2->GetText_Lose() : NSLOCTEXT("T66.Gambler", "Lose", "LOSE"));
			SetStatus(Status,
				bPendingWin ? FLinearColor(0.3f, 1.f, 0.4f, 1.f) : FLinearColor(1.f, 0.3f, 0.3f, 1.f));
			break;
		}
		case ERevealType::FindTheBall:
		{
			if (bPendingWin) AwardWin();
			if (BallResultText.IsValid())
			{
				const int32 CupIndex1Based = PendingFindBallCorrectCup + 1;
				const FText Cup = Loc2
					? Loc2->GetText_Cup(CupIndex1Based)
					: FText::Format(NSLOCTEXT("T66.Gambler", "CupIndexFormat", "CUP {0}"), FText::AsNumber(CupIndex1Based));
				const FText Outcome = bPendingWin
					? (Loc2 ? Loc2->GetText_Win() : NSLOCTEXT("T66.Gambler", "Win", "WIN"))
					: (Loc2 ? Loc2->GetText_Lose() : NSLOCTEXT("T66.Gambler", "Lose", "LOSE"));
				const FText Fmt = Loc2 ? Loc2->GetText_FindBallResultFormat() : NSLOCTEXT("T66.Gambler", "FindBallResultFormat", "Ball was under {0} — {1}");
				BallResultText->SetText(FText::Format(Fmt, Cup, Outcome));
			}
			const FText WinFmt = Loc2 ? Loc2->GetText_WinPlusAmountFormat() : NSLOCTEXT("T66.Gambler", "WinPlusAmountFormat", "WIN (+{0})");
			const FText Status = bPendingWin
				? FText::Format(WinFmt, FText::AsNumber(GambleAmount))
				: (Loc2 ? Loc2->GetText_Lose() : NSLOCTEXT("T66.Gambler", "Lose", "LOSE"));
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

