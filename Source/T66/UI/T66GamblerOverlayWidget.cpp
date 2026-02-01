// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66GamblerOverlayWidget.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Data/T66DataTypes.h"
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
#include "Widgets/Images/SImage.h"
#include "Styling/CoreStyle.h"
#include "UI/Style/T66Style.h"
#include "EngineUtils.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Gameplay/T66PlayerController.h"

void UT66GamblerOverlayWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RevealTimerHandle);
	}
	Super::NativeDestruct();
}

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
	SelectedInventoryIndex = -1;

	InventorySlotBorders.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotButtons.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotTexts.SetNum(UT66RunStateSubsystem::MaxInventorySlots);

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

	// --- Casino layout: left content switcher + right panel + bottom inventory ---
	const FText CasinoTitle = Loc ? Loc->GetText_Casino() : NSLOCTEXT("T66.Gambler", "Casino", "CASINO");
	const FText BankTitle = Loc ? Loc->GetText_Bank() : NSLOCTEXT("T66.Vendor", "BankTitle", "BANK");
	const FText InventoryTitle = Loc ? Loc->GetText_YourItems() : NSLOCTEXT("T66.Vendor", "InventoryTitle", "INVENTORY");

	TSharedRef<SWidget> RightPanel =
		SNew(SBorder)
		.BorderImage(Style.GetBrush("T66.Brush.Panel"))
		.BorderBackgroundColor(FT66Style::Tokens::Panel)
		.Padding(FT66Style::Tokens::Space6)
		[
			SNew(SVerticalBox)
			// Anger circle (top)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 14.f)
			[
				SNew(SBox)
				.WidthOverride(220.f)
				.HeightOverride(220.f)
				[
					SAssignNew(AngerCircleImage, SImage)
					.Image(Style.GetBrush("T66.Brush.Circle"))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
			// Bank (bottom)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(BankTitle)
					.TextStyle(&TextHeading)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(110.f)
						.HeightOverride(44.f)
						[
							SAssignNew(BorrowAmountSpin, SSpinBox<int32>)
							.MinValue(0).MaxValue(999999).Delta(10)
							.Font(FT66Style::Tokens::FontBold(16))
							.Value_Lambda([this]() { return BorrowAmount; })
							.OnValueChanged_Lambda([this](int32 V) { BorrowAmount = FMath::Max(0, V); })
						]
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
					[
						SNew(SBox)
						.WidthOverride(110.f)
						.HeightOverride(44.f)
						[
							SAssignNew(PaybackAmountSpin, SSpinBox<int32>)
							.MinValue(0).MaxValue(999999).Delta(10)
							.Font(FT66Style::Tokens::FontBold(16))
							.Value_Lambda([this]() { return PaybackAmount; })
							.OnValueChanged_Lambda([this](int32 V) { PaybackAmount = FMath::Max(0, V); })
						]
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
		];

	auto MakeGameCard = [&](const FText& TitleText, const FOnClicked& OnClicked) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(260.f)
			.HeightOverride(160.f)
			.Padding(FMargin(10.f, 0.f))
			[
				SNew(SButton)
				.OnClicked(OnClicked)
				.ButtonStyle(&BtnNeutral)
				.ContentPadding(FMargin(0.f))
				[
					SNew(SBorder)
					.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
					.BorderBackgroundColor(FT66Style::Tokens::Panel2)
					.Padding(FT66Style::Tokens::Space6)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SNew(SSpacer)
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
						[
							SNew(STextBlock)
							.Text(TitleText)
							.TextStyle(&TextHeading)
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.AutoWrapText(true)
						]
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SNew(SSpacer)
						]
					]
				]
			];
	};

	TSharedRef<SWidget> CardsView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
		[
			SNew(STextBlock)
			.Text(CasinoTitle)
			.TextStyle(&TextTitle)
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			// Reserve left side for TikTok by pushing cards to the right.
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(SSpacer)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SUniformGridPanel)
				.SlotPadding(FMargin(0.f, 0.f, 0.f, 0.f))
				// Layout:
				// [Game 2] [Game 3]
				// [Game 1] [   -  ]
				+ SUniformGridPanel::Slot(0, 0)
				[
					MakeGameCard(Loc ? Loc->GetText_RockPaperScissors() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenRps))
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					MakeGameCard(Loc ? Loc->GetText_FindTheBall() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenFindBall))
				]
				+ SUniformGridPanel::Slot(0, 1)
				[
					MakeGameCard(Loc ? Loc->GetText_CoinFlip() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenCoinFlip))
				]
			]
		];

	TSharedRef<SWidget> CoinFlipView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_CoinFlip() : FText::GetEmpty())
			.TextStyle(&TextHeading)
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_ChooseHeadsOrTails() : FText::GetEmpty())
			.TextStyle(&TextBody)
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(0.f, 0.f, 0.f, 16.f)
		[
			SNew(SBox)
			.HeightOverride(220.f)
			[
				SNew(SBorder)
				.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
				.BorderBackgroundColor(FT66Style::Tokens::Panel2)
				.Padding(FT66Style::Tokens::Space6)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SAssignNew(CoinFlipResultText, STextBlock)
					.Text(Loc ? Loc->GetText_ResultDash() : FText::GetEmpty())
					.TextStyle(&TextHeading)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
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

	TSharedRef<SWidget> RpsView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_RockPaperScissors() : FText::GetEmpty())
			.TextStyle(&TextHeading)
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(0.f, 0.f, 0.f, 16.f)
		[
			SNew(SBox)
			.HeightOverride(220.f)
			[
				SNew(SBorder)
				.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
				.BorderBackgroundColor(FT66Style::Tokens::Panel2)
				.Padding(FT66Style::Tokens::Space6)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SAssignNew(RpsResultText, STextBlock)
					.Text(Loc ? Loc->GetText_PickOne() : FText::GetEmpty())
					.TextStyle(&TextHeading)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
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

	TSharedRef<SWidget> FindBallView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_FindTheBall() : FText::GetEmpty())
			.TextStyle(&TextHeading)
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_PickACup() : FText::GetEmpty())
			.TextStyle(&TextBody)
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(0.f, 0.f, 0.f, 16.f)
		[
			SNew(SBox)
			.HeightOverride(220.f)
			[
				SNew(SBorder)
				.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
				.BorderBackgroundColor(FT66Style::Tokens::Panel2)
				.Padding(FT66Style::Tokens::Space6)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SAssignNew(BallResultText, STextBlock)
					.Text(Loc ? Loc->GetText_ResultDash() : FText::GetEmpty())
					.TextStyle(&TextHeading)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
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

	TSharedRef<SWidget> BetRow =
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
		];

	TSharedRef<SWidget> CenterPanel =
		SNew(SBorder)
		.BorderImage(Style.GetBrush("T66.Brush.Panel"))
		.BorderBackgroundColor(FT66Style::Tokens::Panel)
		.Padding(FT66Style::Tokens::Space6)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SAssignNew(CasinoSwitcher, SWidgetSwitcher)
				+ SWidgetSwitcher::Slot() [ CardsView ]
				+ SWidgetSwitcher::Slot() [ CoinFlipView ]
				+ SWidgetSwitcher::Slot() [ RpsView ]
				+ SWidgetSwitcher::Slot() [ FindBallView ]
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f, 0.f, 0.f)
			[
				SNew(SBox)
				.Visibility_Lambda([this]() -> EVisibility
				{
					return (CasinoSwitcher.IsValid() && CasinoSwitcher->GetActiveWidgetIndex() != 0) ? EVisibility::Visible : EVisibility::Collapsed;
				})
				[
					BetRow
				]
			]
		];

	TSharedRef<SWidget> InventoryPanel =
		SNew(SBorder)
		.BorderImage(Style.GetBrush("T66.Brush.Panel"))
		.BorderBackgroundColor(FT66Style::Tokens::Panel)
		.Padding(FT66Style::Tokens::Space6)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(InventoryTitle)
					.TextStyle(&TextHeading)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(18.f, 0.f, 16.f, 0.f)
				[
					SAssignNew(GoldText, STextBlock)
					.Text(FText::GetEmpty())
					.TextStyle(&TextHeading)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SAssignNew(DebtText, STextBlock)
					.Text(FText::GetEmpty())
					.TextStyle(&TextHeading)
					.ColorAndOpacity(FT66Style::Tokens::Danger)
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SSpacer)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space4, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SUniformGridPanel)
					.SlotPadding(FMargin(FT66Style::Tokens::Space2, 0.f))
					+ SUniformGridPanel::Slot(0, 0)
					[
						SAssignNew(InventorySlotButtons[0], SButton)
						.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnSelectInventorySlot, 0))
						.ButtonStyle(&BtnNeutral)
						.ContentPadding(FMargin(0.f))
						[
							SNew(SBox)
							.WidthOverride(160.f)
							.HeightOverride(160.f)
							[
								SAssignNew(InventorySlotBorders[0], SBorder)
								.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
								.Padding(FMargin(12.f, 10.f))
								[
									SAssignNew(InventorySlotTexts[0], STextBlock)
									.Text(NSLOCTEXT("T66.Common", "Dash", "-"))
									.TextStyle(&TextChip)
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
						]
					]
					+ SUniformGridPanel::Slot(1, 0)
					[
						SAssignNew(InventorySlotButtons[1], SButton)
						.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnSelectInventorySlot, 1))
						.ButtonStyle(&BtnNeutral)
						.ContentPadding(FMargin(0.f))
						[
							SNew(SBox)
							.WidthOverride(160.f)
							.HeightOverride(160.f)
							[
								SAssignNew(InventorySlotBorders[1], SBorder)
								.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
								.Padding(FMargin(12.f, 10.f))
								[
									SAssignNew(InventorySlotTexts[1], STextBlock)
									.Text(NSLOCTEXT("T66.Common", "Dash", "-"))
									.TextStyle(&TextChip)
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
						]
					]
					+ SUniformGridPanel::Slot(2, 0)
					[
						SAssignNew(InventorySlotButtons[2], SButton)
						.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnSelectInventorySlot, 2))
						.ButtonStyle(&BtnNeutral)
						.ContentPadding(FMargin(0.f))
						[
							SNew(SBox)
							.WidthOverride(160.f)
							.HeightOverride(160.f)
							[
								SAssignNew(InventorySlotBorders[2], SBorder)
								.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
								.Padding(FMargin(12.f, 10.f))
								[
									SAssignNew(InventorySlotTexts[2], STextBlock)
									.Text(NSLOCTEXT("T66.Common", "Dash", "-"))
									.TextStyle(&TextChip)
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
						]
					]
					+ SUniformGridPanel::Slot(3, 0)
					[
						SAssignNew(InventorySlotButtons[3], SButton)
						.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnSelectInventorySlot, 3))
						.ButtonStyle(&BtnNeutral)
						.ContentPadding(FMargin(0.f))
						[
							SNew(SBox)
							.WidthOverride(160.f)
							.HeightOverride(160.f)
							[
								SAssignNew(InventorySlotBorders[3], SBorder)
								.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
								.Padding(FMargin(12.f, 10.f))
								[
									SAssignNew(InventorySlotTexts[3], STextBlock)
									.Text(NSLOCTEXT("T66.Common", "Dash", "-"))
									.TextStyle(&TextChip)
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
						]
					]
					+ SUniformGridPanel::Slot(4, 0)
					[
						SAssignNew(InventorySlotButtons[4], SButton)
						.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnSelectInventorySlot, 4))
						.ButtonStyle(&BtnNeutral)
						.ContentPadding(FMargin(0.f))
						[
							SNew(SBox)
							.WidthOverride(160.f)
							.HeightOverride(160.f)
							[
								SAssignNew(InventorySlotBorders[4], SBorder)
								.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
								.Padding(FMargin(12.f, 10.f))
								[
									SAssignNew(InventorySlotTexts[4], STextBlock)
									.Text(NSLOCTEXT("T66.Common", "Dash", "-"))
									.TextStyle(&TextChip)
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
						]
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(FT66Style::Tokens::Space6, 0.f, 0.f, 0.f)
				[
					SAssignNew(SellPanelContainer, SBox)
					.Visibility(EVisibility::Visible)
					[
						SNew(SBorder)
						.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
						.BorderBackgroundColor(FT66Style::Tokens::Panel2)
						.Padding(FT66Style::Tokens::Space4)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SAssignNew(SellItemNameText, STextBlock)
								.Text(FText::GetEmpty())
								.TextStyle(&TextHeading)
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
							[
								SAssignNew(SellItemDescText, STextBlock)
								.Text(FText::GetEmpty())
								.TextStyle(&TextBody)
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
								.AutoWrapText(true)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
							[
								SAssignNew(SellItemPriceText, STextBlock)
								.Text(FText::GetEmpty())
								.TextStyle(&TextChip)
								.ColorAndOpacity(FT66Style::Tokens::Accent2)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
							[
								SAssignNew(SellItemButton, SButton)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnSellSelectedClicked))
								.ButtonStyle(&BtnPrimary)
								.ContentPadding(FMargin(18.f, 10.f))
								[
									SNew(STextBlock)
									.Text(Loc ? Loc->GetText_Sell() : NSLOCTEXT("T66.Common", "Sell", "SELL"))
									.TextStyle(&TextButton)
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
						]
					]
				]
			]
		];

	TSharedRef<SWidget> CasinoPage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, FT66Style::Tokens::Space6, 0.f)
			[
				CenterPanel
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				RightPanel
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space6, 0.f, 0.f)
		[
			InventoryPanel
		];

	TSharedRef<SWidget> Root =
		SNew(SBorder)
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
				]
				+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Fill).VAlign(VAlign_Fill).Padding(40.f)
				[
					SAssignNew(PageSwitcher, SWidgetSwitcher)
					+ SWidgetSwitcher::Slot()
					[ DialoguePage ]
					+ SWidgetSwitcher::Slot()
					[ CasinoPage ]
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
	RefreshInventory();
	RefreshSellPanel();
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
	if (Index == 0) // Dialogue
	{
		CloseOverlay();
	}
	else
	{
		// If a game is open, go back to the game cards first.
		if (CasinoSwitcher.IsValid() && CasinoSwitcher->GetActiveWidgetIndex() != 0)
		{
			CasinoSwitcher->SetActiveWidgetIndex(0);
			SetStatus(FText::GetEmpty());
			RefreshTopBar();
			RefreshInventory();
			RefreshSellPanel();
		}
		else
		{
			SetPage(EGamblerPage::Dialogue);
		}
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
	// PageSwitcher now has 2 pages: Dialogue (0) and Casino (1).
	// CasinoSwitcher contains the sub-views: Cards (0), CoinFlip (1), RPS (2), FindBall (3).
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
		case EGamblerPage::RockPaperScissors:
			PageIndex = 1;
			CasinoIndex = 2;
			break;
		case EGamblerPage::FindTheBall:
			PageIndex = 1;
			CasinoIndex = 3;
			break;
		default:
			PageIndex = 0;
			CasinoIndex = 0;
			break;
	}

	PageSwitcher->SetActiveWidgetIndex(PageIndex);
	if (CasinoSwitcher.IsValid())
	{
		CasinoSwitcher->SetActiveWidgetIndex(CasinoIndex);
	}
	bInputLocked = false;
	SetStatus(FText::GetEmpty());
	RefreshTopBar();
	RefreshInventory();
	RefreshSellPanel();

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

void UT66GamblerOverlayWidget::OpenCasinoPage()
{
	SetPage(EGamblerPage::Casino);
}

void UT66GamblerOverlayWidget::RefreshTopBar()
{
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
	if (GoldText.IsValid())
	{
		GoldText->SetText(FText::Format(GoldFmt, FText::AsNumber(Gold)));
	}
	if (DebtText.IsValid())
	{
		DebtText->SetText(FText::Format(OweFmt, FText::AsNumber(Debt)));
	}
	if (AngerCircleImage.IsValid())
	{
		// Anger circle color rules (discrete):
		// 0 => white
		// 1-50 => pink
		// 51-99 => purple
		// 100 => red (boss triggers)
		const int32 Pct = FMath::Clamp(FMath::RoundToInt(Anger01 * 100.f), 0, 100);
		FLinearColor C = FLinearColor::White;
		if (Pct >= 100)
		{
			C = FT66Style::Tokens::Danger;
		}
		else if (Pct >= 51)
		{
			C = FLinearColor(0.60f, 0.25f, 0.90f, 1.f); // purple
		}
		else if (Pct >= 1)
		{
			C = FLinearColor(0.95f, 0.35f, 0.65f, 1.f); // pink
		}
		AngerCircleImage->SetColorAndOpacity(FSlateColor(C));
	}
	if (AngerText.IsValid())
	{
		const int32 Pct = FMath::RoundToInt(Anger01 * 100.f);
		AngerText->SetText(FText::Format(NSLOCTEXT("T66.Common", "PercentInt", "{0}%"), FText::AsNumber(Pct)));
	}
}

FReply UT66GamblerOverlayWidget::OnSelectInventorySlot(int32 InventoryIndex)
{
	SelectedInventoryIndex = InventoryIndex;
	RefreshInventory();
	RefreshSellPanel();
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnSellSelectedClicked()
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return FReply::Handled();

	if (SelectedInventoryIndex < 0)
	{
		SetStatus(NSLOCTEXT("T66.Vendor", "SelectItemToSell", "Select an item to sell."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return FReply::Handled();
	}

	const bool bSold = RunState->SellInventoryItemAt(SelectedInventoryIndex);
	if (bSold)
	{
		SetStatus(NSLOCTEXT("T66.Vendor", "SoldStatus", "Sold."), FLinearColor(0.3f, 1.f, 0.4f, 1.f));
	}
	else
	{
		SetStatus(NSLOCTEXT("T66.Vendor", "CouldNotSell", "Could not sell."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
	}

	RefreshTopBar();
	RefreshInventory();
	RefreshSellPanel();
	return FReply::Handled();
}

void UT66GamblerOverlayWidget::RefreshInventory()
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	const TArray<FName>& Inv = RunState->GetInventory();

	// Auto-select the first valid item so the details panel is "big" immediately.
	if (SelectedInventoryIndex < 0)
	{
		for (int32 i = 0; i < Inv.Num(); ++i)
		{
			if (!Inv[i].IsNone())
			{
				SelectedInventoryIndex = i;
				break;
			}
		}
	}
	if (SelectedInventoryIndex >= Inv.Num())
	{
		SelectedInventoryIndex = (Inv.Num() > 0) ? (Inv.Num() - 1) : -1;
	}

	for (int32 i = 0; i < InventorySlotTexts.Num(); ++i)
	{
		const bool bHasItem = Inv.IsValidIndex(i) && !Inv[i].IsNone();
		if (InventorySlotTexts[i].IsValid())
		{
			InventorySlotTexts[i]->SetText(bHasItem ? FText::FromName(Inv[i]) : NSLOCTEXT("T66.Common", "Dash", "-"));
		}
		if (InventorySlotButtons.IsValidIndex(i) && InventorySlotButtons[i].IsValid())
		{
			InventorySlotButtons[i]->SetEnabled(bHasItem && !IsBossActive());
		}
		if (InventorySlotBorders.IsValidIndex(i) && InventorySlotBorders[i].IsValid())
		{
			FLinearColor Fill = FT66Style::Tokens::Panel2;
			if (bHasItem && T66GI)
			{
				FItemData D;
				if (T66GI->GetItemData(Inv[i], D))
				{
					Fill = D.PlaceholderColor;
				}
			}

			if (i == SelectedInventoryIndex)
			{
				Fill = Fill * 0.45f + FT66Style::Tokens::Accent * 0.55f;
			}
			InventorySlotBorders[i]->SetBorderBackgroundColor(Fill);
		}
	}
}

void UT66GamblerOverlayWidget::RefreshSellPanel()
{
	UWorld* World = GetWorld();
	UGameInstance* GI0 = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI0 ? GI0->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	const TArray<FName>& Inv = RunState->GetInventory();
	const bool bValidSelection = Inv.IsValidIndex(SelectedInventoryIndex) && !Inv[SelectedInventoryIndex].IsNone();

	if (SellPanelContainer.IsValid())
	{
		// Keep visible so the inventory layout doesn't "pop" when selecting an item.
		SellPanelContainer->SetVisibility(EVisibility::Visible);
	}
	if (!bValidSelection)
	{
		if (SellItemNameText.IsValid()) SellItemNameText->SetText(FText::GetEmpty());
		if (SellItemDescText.IsValid()) SellItemDescText->SetText(FText::GetEmpty());
		if (SellItemPriceText.IsValid()) SellItemPriceText->SetText(FText::GetEmpty());
		if (SellItemButton.IsValid()) SellItemButton->SetEnabled(false);
		return;
	}

	UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	FItemData D;
	const bool bHasData = T66GI && T66GI->GetItemData(Inv[SelectedInventoryIndex], D);

	if (SellItemNameText.IsValid())
	{
		SellItemNameText->SetText(FText::FromName(Inv[SelectedInventoryIndex]));
	}

	UT66LocalizationSubsystem* Loc = GI0 ? GI0->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	auto StatLabel = [&](ET66HeroStatType Type) -> FText
	{
		if (Loc)
		{
			switch (Type)
			{
				case ET66HeroStatType::Damage: return Loc->GetText_Stat_Damage();
				case ET66HeroStatType::AttackSpeed: return Loc->GetText_Stat_AttackSpeed();
				case ET66HeroStatType::AttackSize: return Loc->GetText_Stat_AttackSize();
				case ET66HeroStatType::Armor: return Loc->GetText_Stat_Armor();
				case ET66HeroStatType::Evasion: return Loc->GetText_Stat_Evasion();
				case ET66HeroStatType::Luck: return Loc->GetText_Stat_Luck();
				default: break;
			}
		}
		switch (Type)
		{
			case ET66HeroStatType::Damage: return NSLOCTEXT("T66.Stats", "Damage", "Damage");
			case ET66HeroStatType::AttackSpeed: return NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed");
			case ET66HeroStatType::AttackSize: return NSLOCTEXT("T66.Stats", "AttackSize", "Attack Size");
			case ET66HeroStatType::Armor: return NSLOCTEXT("T66.Stats", "Armor", "Armor");
			case ET66HeroStatType::Evasion: return NSLOCTEXT("T66.Stats", "Evasion", "Evasion");
			case ET66HeroStatType::Luck: return NSLOCTEXT("T66.Stats", "Luck", "Luck");
			default: return FText::GetEmpty();
		}
	};

	if (SellItemDescText.IsValid())
	{
		if (!bHasData)
		{
			SellItemDescText->SetText(FText::GetEmpty());
		}
		else
		{
			ET66HeroStatType MainType = D.MainStatType;
			int32 MainValue = D.MainStatValue;
			if (MainValue == 0)
			{
				switch (D.EffectType)
				{
					case ET66ItemEffectType::BonusDamagePct: MainType = ET66HeroStatType::Damage; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f); break;
					case ET66ItemEffectType::BonusAttackSpeedPct: MainType = ET66HeroStatType::AttackSpeed; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f); break;
					case ET66ItemEffectType::BonusArmorPctPoints: MainType = ET66HeroStatType::Armor; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 4.f); break;
					case ET66ItemEffectType::BonusEvasionPctPoints: MainType = ET66HeroStatType::Evasion; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 4.f); break;
					case ET66ItemEffectType::BonusLuckFlat: MainType = ET66HeroStatType::Luck; MainValue = FMath::RoundToInt(FMath::Max(0.f, D.EffectMagnitude)); break;
					case ET66ItemEffectType::BonusMoveSpeedPct:
					case ET66ItemEffectType::DashCooldownReductionPct:
						MainType = ET66HeroStatType::Evasion;
						MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f);
						break;
					default: break;
				}
			}

			SellItemDescText->SetText((MainValue > 0)
				? FText::Format(NSLOCTEXT("T66.ItemTooltip", "MainStatLineFormat", "{0}: +{1}"), StatLabel(MainType), FText::AsNumber(MainValue))
				: FText::GetEmpty());
		}
	}

	if (SellItemPriceText.IsValid())
	{
		const int32 SellValue = bHasData ? D.SellValueGold : 0;
		SellItemPriceText->SetText(FText::Format(
			NSLOCTEXT("T66.Vendor", "SellForFormat", "SELL FOR: {0}g"),
			FText::AsNumber(SellValue)));
	}

	if (SellItemButton.IsValid())
	{
		SellItemButton->SetEnabled(!IsBossActive());
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

