// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66GamblerOverlayWidget.h"
#include "UI/T66StatsPanelSlate.h"
#include "UI/T66SlateTextureHelpers.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
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
#include "Styling/SlateBrush.h"
#include "UI/Style/T66Style.h"
#include "EngineUtils.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Gameplay/T66PlayerController.h"
#include "Engine/Texture2D.h"

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
	InventorySlotIconImages.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotIconBrushes.SetNum(UT66RunStateSubsystem::MaxInventorySlots);

	UT66LocalizationSubsystem* Loc = nullptr;
	UT66UITexturePoolSubsystem* TexPool = nullptr;
	UT66RunStateSubsystem* RunState = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
			TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>();
			RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		}
	}

	const ISlateStyle& Style = FT66Style::Get();

	const FTextBlockStyle& TextTitle = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Title");
	const FTextBlockStyle& TextHeading = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading");
	const FTextBlockStyle& TextBody = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Body");
	const FTextBlockStyle& TextChip = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip");

	// --- Game icons (Sprites/Games) ---
	// Avoid sync loads; request via the central UI texture pool.
	GameIcon_CoinFlip = FSlateBrush();
	GameIcon_CoinFlip.ImageSize = FVector2D(72.f, 72.f);
	GameIcon_CoinFlip.DrawAs = ESlateBrushDrawType::Image;

	GameIcon_Rps = FSlateBrush();
	GameIcon_Rps.ImageSize = FVector2D(72.f, 72.f);
	GameIcon_Rps.DrawAs = ESlateBrushDrawType::Image;

	GameIcon_FindBall = FSlateBrush();
	GameIcon_FindBall.ImageSize = FVector2D(72.f, 72.f);
	GameIcon_FindBall.DrawAs = ESlateBrushDrawType::Image;

	if (TexPool)
	{
		const TSoftObjectPtr<UTexture2D> CoinSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/T_Game_Coin.T_Game_Coin")));
		const TSoftObjectPtr<UTexture2D> RpsSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/T_Game_RPS.T_Game_RPS")));
		const TSoftObjectPtr<UTexture2D> BallSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/T_Game_Ball_game.T_Game_Ball_game")));
		T66SlateTexture::BindBrushAsync(TexPool, CoinSoft, this, GameIcon_CoinFlip, FName(TEXT("GamblerGameIcon"), 1), /*bClearWhileLoading*/ true);
		T66SlateTexture::BindBrushAsync(TexPool, RpsSoft, this, GameIcon_Rps, FName(TEXT("GamblerGameIcon"), 2), /*bClearWhileLoading*/ true);
		T66SlateTexture::BindBrushAsync(TexPool, BallSoft, this, GameIcon_FindBall, FName(TEXT("GamblerGameIcon"), 3), /*bClearWhileLoading*/ true);
	}

	// Inventory slot icon brushes (same sizing as vendor for consistency)
	for (int32 i = 0; i < InventorySlotIconBrushes.Num(); ++i)
	{
		InventorySlotIconBrushes[i] = MakeShared<FSlateBrush>();
		InventorySlotIconBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		InventorySlotIconBrushes[i]->ImageSize = FVector2D(148.f, 148.f);
	}

	auto MakeTitle = [&](const FText& Text) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(Text)
			.TextStyle(&TextTitle)
			.ColorAndOpacity(FT66Style::Tokens::Text);
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
		[
			FT66Style::MakeButton(FT66ButtonParams(
				Loc ? Loc->GetText_LetMeGamble() : FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnDialogueGamble),
				ET66ButtonType::Primary)
				.SetMinWidth(420.f)
				.SetPadding(FMargin(18.f, 10.f)))
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			FT66Style::MakeButton(FT66ButtonParams(
				Loc ? Loc->GetText_TeleportMeToYourBrother() : FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnDialogueTeleport),
				ET66ButtonType::Neutral)
				.SetMinWidth(420.f)
				.SetPadding(FMargin(18.f, 10.f))
				.SetEnabled(TAttribute<bool>::CreateLambda([this]() { return !IsBossActive(); })))
		];

	// --- Casino layout: left content switcher + right panel + bottom inventory ---
	const FText CasinoTitle = Loc ? Loc->GetText_Casino() : NSLOCTEXT("T66.Gambler", "Casino", "CASINO");
	const FText BankTitle = Loc ? Loc->GetText_Bank() : NSLOCTEXT("T66.Vendor", "BankTitle", "BANK");
	const FText InventoryTitle = Loc ? Loc->GetText_YourItems() : NSLOCTEXT("T66.Vendor", "InventoryTitle", "INVENTORY");

	TSharedRef<SWidget> RightPanel =
		FT66Style::MakePanel(
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
					FT66Style::MakeButton(FT66ButtonParams(
						Loc ? Loc->GetText_Borrow() : FText::GetEmpty(),
						FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBorrowClicked),
						ET66ButtonType::Neutral)
						.SetMinWidth(0.f)
						.SetPadding(FMargin(14.f, 8.f)))
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
					FT66Style::MakeButton(FT66ButtonParams(
						Loc ? Loc->GetText_Max() : FText::GetEmpty(),
						FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnPaybackMax),
						ET66ButtonType::Neutral)
						.SetMinWidth(0.f)
						.SetPadding(FMargin(14.f, 8.f)))
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					FT66Style::MakeButton(FT66ButtonParams(
						Loc ? Loc->GetText_Payback() : FText::GetEmpty(),
						FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnPaybackClicked),
						ET66ButtonType::Neutral)
						.SetMinWidth(0.f)
						.SetPadding(FMargin(14.f, 8.f)))
				]
				]
			]
		,
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel));

	auto MakeGameCard = [&](const FText& TitleText, const FOnClicked& OnClicked, const FSlateBrush* IconBrush) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.Padding(FMargin(10.f, 0.f))
			[
				FT66Style::MakeButton(FT66ButtonParams(FText::GetEmpty(), OnClicked, ET66ButtonType::Neutral)
					.SetMinWidth(260.f).SetHeight(160.f)
					.SetPadding(FMargin(0.f))
					.SetContent(
						FT66Style::MakePanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
							[
								SNew(SBox)
								.WidthOverride(84.f)
								.HeightOverride(84.f)
								[
									SNew(SImage)
									.Image(IconBrush)
									.ColorAndOpacity(FLinearColor::White)
								]
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[
								SNew(STextBlock)
								.Text(TitleText)
								.TextStyle(&TextHeading)
								.ColorAndOpacity(FT66Style::Tokens::Text)
								.AutoWrapText(true)
							]
							+ SVerticalBox::Slot().FillHeight(1.f) [ SNew(SSpacer) ]
						,
							FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66Style::Tokens::Space6))))
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
					MakeGameCard(Loc ? Loc->GetText_RockPaperScissors() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenRps), &GameIcon_Rps)
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					MakeGameCard(Loc ? Loc->GetText_FindTheBall() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenFindBall), &GameIcon_FindBall)
				]
				+ SUniformGridPanel::Slot(0, 1)
				[
					MakeGameCard(Loc ? Loc->GetText_CoinFlip() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenCoinFlip), &GameIcon_CoinFlip)
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
				FT66Style::MakePanel(
					SNew(SBox)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(CoinFlipResultText, STextBlock)
						.Text(Loc ? Loc->GetText_ResultDash() : FText::GetEmpty())
						.TextStyle(&TextHeading)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					],
					FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66Style::Tokens::Space6))
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					Loc ? Loc->GetText_Heads() : FText::GetEmpty(),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCoinFlipHeads),
					ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(18.f, 10.f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					Loc ? Loc->GetText_Tails() : FText::GetEmpty(),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCoinFlipTails),
					ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(18.f, 10.f)))
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
				FT66Style::MakePanel(
					SNew(SBox)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(RpsResultText, STextBlock)
						.Text(Loc ? Loc->GetText_PickOne() : FText::GetEmpty())
						.TextStyle(&TextHeading)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					],
					FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66Style::Tokens::Space6))
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					Loc ? Loc->GetText_Rock() : FText::GetEmpty(),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnRpsRock),
					ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(18.f, 10.f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					Loc ? Loc->GetText_Paper() : FText::GetEmpty(),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnRpsPaper),
					ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(18.f, 10.f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					Loc ? Loc->GetText_Scissors() : FText::GetEmpty(),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnRpsScissors),
					ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(18.f, 10.f)))
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
				FT66Style::MakePanel(
					SNew(SBox)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(BallResultText, STextBlock)
						.Text(Loc ? Loc->GetText_ResultDash() : FText::GetEmpty())
						.TextStyle(&TextHeading)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					],
					FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66Style::Tokens::Space6))
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					Loc ? Loc->GetText_Cup(1) : FText::GetEmpty(),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBallCup1),
					ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(18.f, 10.f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					Loc ? Loc->GetText_Cup(2) : FText::GetEmpty(),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBallCup2),
					ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(18.f, 10.f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					Loc ? Loc->GetText_Cup(3) : FText::GetEmpty(),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBallCup3),
					ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(18.f, 10.f)))
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
			FT66Style::MakeButton(FT66ButtonParams(
				Loc ? Loc->GetText_Max() : FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnGambleMax),
				ET66ButtonType::Neutral)
				.SetMinWidth(0.f)
				.SetPadding(FMargin(14.f, 8.f)))
		];

	TSharedRef<SWidget> CenterPanel =
		FT66Style::MakePanel(
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
		,
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel));

	// Pre-create inventory slot buttons for centralized styling
	for (int32 i = 0; i < UT66RunStateSubsystem::MaxInventorySlots; ++i)
	{
		InventorySlotButtons[i] = FT66Style::MakeButton(
			FT66ButtonParams(FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnSelectInventorySlot, i),
				ET66ButtonType::Neutral)
				.SetMinWidth(0.f)
				.SetPadding(FMargin(0.f))
				.SetContent(
					SNew(SBox)
					.WidthOverride(160.f)
					.HeightOverride(160.f)
					[
						FT66Style::MakePanel(
							SNew(SOverlay)
							+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(148.f)
								.HeightOverride(148.f)
								[
									SAssignNew(InventorySlotIconImages[i], SImage)
									.Image(InventorySlotIconBrushes[i].Get())
									.ColorAndOpacity(FLinearColor::White)
								]
							]
							+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
							[
								SAssignNew(InventorySlotTexts[i], STextBlock)
								.Text(NSLOCTEXT("T66.Common", "Dash", "-"))
								.TextStyle(&TextChip)
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
						,
							FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(0.f)),
							&InventorySlotBorders[i])
					]
				));
	}

	// Pre-create sell button for centralized styling
	SellItemButton = FT66Style::MakeButton(
		FT66ButtonParams(
			Loc ? Loc->GetText_Sell() : NSLOCTEXT("T66.Common", "Sell", "SELL"),
			FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnSellSelectedClicked),
			ET66ButtonType::Primary)
			.SetMinWidth(0.f)
			.SetPadding(FMargin(18.f, 10.f)));

	TSharedRef<SWidget> InventoryPanel =
		FT66Style::MakePanel(
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
				+ SUniformGridPanel::Slot(0, 0) [ InventorySlotButtons[0].ToSharedRef() ]
				+ SUniformGridPanel::Slot(1, 0) [ InventorySlotButtons[1].ToSharedRef() ]
				+ SUniformGridPanel::Slot(2, 0) [ InventorySlotButtons[2].ToSharedRef() ]
				+ SUniformGridPanel::Slot(3, 0) [ InventorySlotButtons[3].ToSharedRef() ]
				+ SUniformGridPanel::Slot(4, 0) [ InventorySlotButtons[4].ToSharedRef() ]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(FT66Style::Tokens::Space6, 0.f, 0.f, 0.f)
				[
					SAssignNew(SellPanelContainer, SBox)
					.Visibility(EVisibility::Visible)
					[
						FT66Style::MakePanel(
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
							SellItemButton.ToSharedRef()
						]
						,
							FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66Style::Tokens::Space4))
					]
				]
			]
		,
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel));

	TSharedRef<SWidget> CasinoPage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66Style::Tokens::Space6, 0.f)
			[
				SAssignNew(StatsPanelBox, SBox)
				[
					T66StatsPanelSlate::MakeEssentialStatsPanel(RunState, Loc)
				]
			]
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
		FT66Style::MakePanel(
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(FT66Style::Tokens::Space6, FT66Style::Tokens::Space4, FT66Style::Tokens::Space6, FT66Style::Tokens::Space2)
				[
					SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66Style::Tokens::Space4, 0.f)
				[
					FT66Style::MakeButton(FT66ButtonParams(
						Loc ? Loc->GetText_Back() : FText::GetEmpty(),
						FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBack),
						ET66ButtonType::Neutral)
						.SetMinWidth(0.f)
						.SetPadding(FMargin(18.f, 10.f)))
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
					FT66Style::MakePanel(
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
							FT66Style::MakeButton(FT66ButtonParams(
								Loc ? Loc->GetText_Yes() : FText::GetEmpty(),
								FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCheatYes),
								ET66ButtonType::Danger)
								.SetMinWidth(140.f)
								.SetPadding(FMargin(18.f, 10.f)))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
						[
							FT66Style::MakeButton(FT66ButtonParams(
								Loc ? Loc->GetText_No() : FText::GetEmpty(),
								FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCheatNo),
								ET66ButtonType::Neutral)
								.SetMinWidth(140.f)
								.SetPadding(FMargin(18.f, 10.f)))
						]
					]
					,
						FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel))
				]
			]
		,
			FT66PanelParams(ET66PanelType::Bg).SetPadding(0.f).SetColor(FT66Style::Tokens::Bg));

	// Ensure initial page + gold display are correct even when re-opening the same widget instance.
	SetPage(EGamblerPage::Dialogue);
	RefreshTopBar();
	RefreshInventory();
	RefreshSellPanel();
	return Root;
}

FReply UT66GamblerOverlayWidget::OnBack()
{
	// Back should always exit the interaction (close the overlay).
	CloseOverlay();
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
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
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
			// Keep the strip clean: icons for items, "-" for empty slots.
			InventorySlotTexts[i]->SetText(bHasItem ? FText::GetEmpty() : NSLOCTEXT("T66.Common", "Dash", "-"));
		}
		if (InventorySlotButtons.IsValidIndex(i) && InventorySlotButtons[i].IsValid())
		{
			InventorySlotButtons[i]->SetEnabled(bHasItem && !IsBossActive());
		}
		if (InventorySlotBorders.IsValidIndex(i) && InventorySlotBorders[i].IsValid())
		{
			FLinearColor Fill = FT66Style::Tokens::Panel2;
			FItemData D;
			const bool bHasData = bHasItem && T66GI && T66GI->GetItemData(Inv[i], D);
			if (bHasData)
			{
				Fill = FT66Style::Tokens::Panel2;
			}

			if (i == SelectedInventoryIndex)
			{
				Fill = Fill * 0.45f + FT66Style::Tokens::Accent * 0.55f;
			}
			InventorySlotBorders[i]->SetBorderBackgroundColor(Fill);

			if (InventorySlotIconBrushes.IsValidIndex(i) && InventorySlotIconBrushes[i].IsValid())
			{
				if (bHasData && !D.Icon.IsNull() && TexPool)
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, D.Icon, this, InventorySlotIconBrushes[i], FName(TEXT("GamblerInv"), i + 1), /*bClearWhileLoading*/ true);
				}
				else
				{
					InventorySlotIconBrushes[i]->SetResourceObject(nullptr);
				}
			}
			if (InventorySlotIconImages.IsValidIndex(i) && InventorySlotIconImages[i].IsValid())
			{
				const bool bHasIcon = bHasData && !D.Icon.IsNull();
				InventorySlotIconImages[i]->SetVisibility(bHasIcon ? EVisibility::Visible : EVisibility::Hidden);
			}
		}
	}
	RefreshStatsPanel();
}

void UT66GamblerOverlayWidget::RefreshStatsPanel()
{
	if (!StatsPanelBox.IsValid()) return;
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = nullptr;
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	StatsPanelBox->SetContent(T66StatsPanelSlate::MakeEssentialStatsPanel(RunState, Loc));
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

	UT66LocalizationSubsystem* Loc = GI0 ? GI0->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	if (SellItemNameText.IsValid())
	{
		SellItemNameText->SetText(Loc ? Loc->GetText_ItemDisplayName(Inv[SelectedInventoryIndex]) : FText::FromName(Inv[SelectedInventoryIndex]));
	}

	auto StatLabel = [&](ET66HeroStatType Type) -> FText
	{
		if (Loc)
		{
			switch (Type)
			{
				case ET66HeroStatType::Damage: return Loc->GetText_Stat_Damage();
				case ET66HeroStatType::AttackSpeed: return Loc->GetText_Stat_AttackSpeed();
				case ET66HeroStatType::AttackScale: return Loc->GetText_Stat_AttackScale();
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
			case ET66HeroStatType::AttackScale: return NSLOCTEXT("T66.Stats", "AttackScale", "Attack Scale");
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
			const ET66HeroStatType MainType = D.PrimaryStatType;
			int32 MainValue = 0;
			if (RunState)
			{
				const TArray<FT66InventorySlot>& Slots = RunState->GetInventorySlots();
				if (SelectedInventoryIndex >= 0 && SelectedInventoryIndex < Slots.Num())
				{
					MainValue = Slots[SelectedInventoryIndex].Line1RolledValue;
				}
			}

			SellItemDescText->SetText((MainValue > 0)
				? FText::Format(NSLOCTEXT("T66.ItemTooltip", "MainStatLineFormat", "{0}: +{1}"), StatLabel(MainType), FText::AsNumber(MainValue))
				: FText::GetEmpty());
		}
	}

	if (SellItemPriceText.IsValid())
	{
		int32 SellValue = 0;
		if (bHasData && RunState)
		{
			const TArray<FT66InventorySlot>& Slots = RunState->GetInventorySlots();
			if (SelectedInventoryIndex >= 0 && SelectedInventoryIndex < Slots.Num())
			{
				SellValue = D.GetSellGoldForRarity(Slots[SelectedInventoryIndex].Rarity);
			}
		}
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
	UT66RngSubsystem* RngSub = nullptr;
	const UT66RngTuningConfig* Tuning = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		RngSub = GI->GetSubsystem<UT66RngSubsystem>();
		if (RngSub)
		{
			Tuning = RngSub->GetTuning();
		}
	}

	bPendingWin = true;
	bPendingDraw = false;

	UT66LocalizationSubsystem* Loc2 = RunState ? RunState->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	// Roll cheat success: success => force win with no anger increase. failure => anger increases and normal RNG plays out.
	bool bCheatSuccess = false;
	{
		const float Base = Tuning ? FMath::Clamp(Tuning->GamblerCheatSuccessChanceBase, 0.f, 1.f) : 0.40f;
		if (RunState && RngSub && Base > 0.f)
		{
			RngSub->UpdateLuckStat(RunState->GetLuckStat());
			const float Chance = RngSub->BiasChance01(Base);
			bCheatSuccess = (RngSub->GetRunStream().GetFraction() < Chance);
		}
		else
		{
			bCheatSuccess = (FMath::FRand() < Base);
		}
	}

	if (RunState)
	{
		RunState->RecordLuckQuantityBool(FName(TEXT("GamblerCheatSuccess")), bCheatSuccess);
	}

	if (!bCheatSuccess && RunState)
	{
		RunState->AddGamblerAngerFromBet(GambleAmount);
		RefreshTopBar();

		// If anger hit 100%, close overlay and spawn Gambler boss immediately.
		if (RunState->GetGamblerAnger01() >= 1.f)
		{
			CloseOverlay();
			TriggerGamblerBossIfAngry();
			return FReply::Handled();
		}
	}

	// Determine pending outcome.
	switch (PendingRevealType)
	{
		case ERevealType::CoinFlip:
		{
			if (bCheatSuccess)
			{
				bPendingCoinFlipResultHeads = bPendingCoinFlipChoseHeads;
				bPendingWin = true;
			}
			else
			{
				bPendingCoinFlipResultHeads = FMath::RandBool();
				bPendingWin = (bPendingCoinFlipChoseHeads == bPendingCoinFlipResultHeads);
			}
			if (CoinFlipResultText.IsValid()) CoinFlipResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : NSLOCTEXT("T66.Gambler", "Rolling", "Rolling..."));
			break;
		}
		case ERevealType::RockPaperScissors:
		{
			if (bCheatSuccess)
			{
				// Opponent choice that loses to player
				PendingRpsOppChoice = (PendingRpsPlayerChoice + 2) % 3;
				bPendingWin = true;
			}
			else
			{
				do
				{
					PendingRpsOppChoice = FMath::RandRange(0, 2);
				} while (PendingRpsOppChoice == PendingRpsPlayerChoice);
				bPendingWin = (PendingRpsPlayerChoice == 0 && PendingRpsOppChoice == 2)
					|| (PendingRpsPlayerChoice == 1 && PendingRpsOppChoice == 0)
					|| (PendingRpsPlayerChoice == 2 && PendingRpsOppChoice == 1);
			}
			if (RpsResultText.IsValid()) RpsResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : NSLOCTEXT("T66.Gambler", "Rolling", "Rolling..."));
			break;
		}
		case ERevealType::FindTheBall:
		{
			if (bCheatSuccess)
			{
				PendingFindBallCorrectCup = PendingFindBallChosenCup;
				bPendingWin = true;
			}
			else
			{
				PendingFindBallCorrectCup = FMath::RandRange(0, 2);
				bPendingWin = (PendingFindBallChosenCup == PendingFindBallCorrectCup);
			}
			if (BallResultText.IsValid()) BallResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : NSLOCTEXT("T66.Gambler", "Rolling", "Rolling..."));
			break;
		}
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

