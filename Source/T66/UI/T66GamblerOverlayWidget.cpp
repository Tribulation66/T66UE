// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66GamblerOverlayWidget.h"
#include "UI/T66StatsPanelSlate.h"
#include "UI/T66SlateTextureHelpers.h"
#include "Core/T66AchievementsSubsystem.h"
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
#include "Widgets/Layout/SScrollBox.h"
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
		World->GetTimerManager().ClearTimer(CoinSpinTimerHandle);
		World->GetTimerManager().ClearTimer(LotteryDrawTimerHandle);
		World->GetTimerManager().ClearTimer(PlinkoTimerHandle);
		World->GetTimerManager().ClearTimer(BoxOpeningTimerHandle);
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

	LotteryNumberBorders.SetNum(10);

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

	// --- NPC anger face sprites ---
	const float AngerFaceSize = FT66Style::Tokens::NPCAngerCircleSize;
	auto InitFaceBrush = [](FSlateBrush& B, float Size) {
		B = FSlateBrush();
		B.ImageSize = FVector2D(Size, Size);
		B.DrawAs = ESlateBrushDrawType::Image;
	};
	InitFaceBrush(AngerFace_Happy, AngerFaceSize);
	InitFaceBrush(AngerFace_Neutral, AngerFaceSize);
	InitFaceBrush(AngerFace_Angry, AngerFaceSize);

	if (TexPool)
	{
		const TSoftObjectPtr<UTexture2D> HappySoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/NPCs/Gambler/Happy.Happy")));
		const TSoftObjectPtr<UTexture2D> NeutralSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/NPCs/Gambler/Neutral.Neutral")));
		const TSoftObjectPtr<UTexture2D> AngrySoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/NPCs/Gambler/Angry.Angry")));
		T66SlateTexture::BindBrushAsync(TexPool, HappySoft, this, AngerFace_Happy, FName(TEXT("GamblerFaceHappy")), /*bClearWhileLoading*/ true);
		T66SlateTexture::BindBrushAsync(TexPool, NeutralSoft, this, AngerFace_Neutral, FName(TEXT("GamblerFaceNeutral")), /*bClearWhileLoading*/ true);
		T66SlateTexture::BindBrushAsync(TexPool, AngrySoft, this, AngerFace_Angry, FName(TEXT("GamblerFaceAngry")), /*bClearWhileLoading*/ true);
	}

	// --- Game icons (Sprites/Games) ---
	// Avoid sync loads; request via the central UI texture pool.
	// Game cards are ~30% bigger than base ItemPanelIconSize (200 -> 260).
	static constexpr float GameCardIconSize = 260.f;
	GameIcon_CoinFlip = FSlateBrush();
	GameIcon_CoinFlip.ImageSize = FVector2D(GameCardIconSize, GameCardIconSize);
	GameIcon_CoinFlip.DrawAs = ESlateBrushDrawType::Image;

	GameIcon_Rps = FSlateBrush();
	GameIcon_Rps.ImageSize = FVector2D(GameCardIconSize, GameCardIconSize);
	GameIcon_Rps.DrawAs = ESlateBrushDrawType::Image;

	GameIcon_BlackJack = FSlateBrush();
	GameIcon_BlackJack.ImageSize = FVector2D(GameCardIconSize, GameCardIconSize);
	GameIcon_BlackJack.DrawAs = ESlateBrushDrawType::Image;

	BlackJackCardBackBrush = FSlateBrush();
	BlackJackCardBackBrush.ImageSize = FVector2D(80.f, 112.f);
	BlackJackCardBackBrush.DrawAs = ESlateBrushDrawType::Image;

	if (TexPool)
	{
		const TSoftObjectPtr<UTexture2D> CoinSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/T_Game_Coin.T_Game_Coin")));
		const TSoftObjectPtr<UTexture2D> RpsSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/T_Game_RPS.T_Game_RPS")));
		const TSoftObjectPtr<UTexture2D> BJSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/T_Game_BJ.T_Game_BJ")));
		const TSoftObjectPtr<UTexture2D> CardBackSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/T_Game_CardBack.T_Game_CardBack")));
		T66SlateTexture::BindBrushAsync(TexPool, CoinSoft, this, GameIcon_CoinFlip, FName(TEXT("GamblerGameIcon"), 1), /*bClearWhileLoading*/ true);
		T66SlateTexture::BindBrushAsync(TexPool, RpsSoft, this, GameIcon_Rps, FName(TEXT("GamblerGameIcon"), 2), /*bClearWhileLoading*/ true);
		T66SlateTexture::BindBrushAsync(TexPool, BJSoft, this, GameIcon_BlackJack, FName(TEXT("GamblerGameIcon"), 3), /*bClearWhileLoading*/ true);
		T66SlateTexture::BindBrushAsync(TexPool, CardBackSoft, this, BlackJackCardBackBrush, FName(TEXT("GamblerCardBack"), 1), /*bClearWhileLoading*/ true);
	}

	// More Games icons (Lottery, Plinko, Box Opening / CRATE)
	GameIcon_Lottery = FSlateBrush();
	GameIcon_Lottery.ImageSize = FVector2D(GameCardIconSize, GameCardIconSize);
	GameIcon_Lottery.DrawAs = ESlateBrushDrawType::Image;
	GameIcon_Plinko = FSlateBrush();
	GameIcon_Plinko.ImageSize = FVector2D(GameCardIconSize, GameCardIconSize);
	GameIcon_Plinko.DrawAs = ESlateBrushDrawType::Image;
	GameIcon_BoxOpening = FSlateBrush();
	GameIcon_BoxOpening.ImageSize = FVector2D(GameCardIconSize, GameCardIconSize);
	GameIcon_BoxOpening.DrawAs = ESlateBrushDrawType::Image;
	if (TexPool)
	{
		const TSoftObjectPtr<UTexture2D> LotterySoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/LOTTERY.LOTTERY")));
		const TSoftObjectPtr<UTexture2D> PlinkoSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/PLINKO.PLINKO")));
		const TSoftObjectPtr<UTexture2D> CrateSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/CRATE.CRATE")));
		T66SlateTexture::BindBrushAsync(TexPool, LotterySoft, this, GameIcon_Lottery, FName(TEXT("GamblerGameIconLottery")), /*bClearWhileLoading*/ true);
		T66SlateTexture::BindBrushAsync(TexPool, PlinkoSoft, this, GameIcon_Plinko, FName(TEXT("GamblerGameIconPlinko")), /*bClearWhileLoading*/ true);
		T66SlateTexture::BindBrushAsync(TexPool, CrateSoft, this, GameIcon_BoxOpening, FName(TEXT("GamblerGameIconCrate")), /*bClearWhileLoading*/ true);
	}

	// --- Coin flip sprites (Heads / Tails / Side) ---
	static constexpr float CoinSpriteSize = 180.f;
	auto InitCoinBrush = [](FSlateBrush& B, float Sz) {
		B = FSlateBrush();
		B.ImageSize = FVector2D(Sz, Sz);
		B.DrawAs = ESlateBrushDrawType::Image;
	};
	InitCoinBrush(CoinBrush_Heads, CoinSpriteSize);
	InitCoinBrush(CoinBrush_Tails, CoinSpriteSize);
	InitCoinBrush(CoinBrush_Side, CoinSpriteSize);
	if (TexPool)
	{
		const TSoftObjectPtr<UTexture2D> HeadsSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/Coin/Heads.Heads")));
		const TSoftObjectPtr<UTexture2D> TailsSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/Coin/TAILS.TAILS")));
		const TSoftObjectPtr<UTexture2D> SideSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/Coin/SIDE.SIDE")));
		T66SlateTexture::BindBrushAsync(TexPool, HeadsSoft, this, CoinBrush_Heads, FName(TEXT("CoinHeads")), true);
		T66SlateTexture::BindBrushAsync(TexPool, TailsSoft, this, CoinBrush_Tails, FName(TEXT("CoinTails")), true);
		T66SlateTexture::BindBrushAsync(TexPool, SideSoft, this, CoinBrush_Side, FName(TEXT("CoinSide")), true);
	}

	// --- RPS hand sprites (Human + Demon × Rock/Paper/Scissors) ---
	static constexpr float RpsHandSize = 160.f;
	auto InitRpsBrush = [](FSlateBrush& B, float Sz) {
		B = FSlateBrush();
		B.ImageSize = FVector2D(Sz, Sz);
		B.DrawAs = ESlateBrushDrawType::Image;
	};
	InitRpsBrush(RpsBrush_HumanRock, RpsHandSize);
	InitRpsBrush(RpsBrush_HumanPaper, RpsHandSize);
	InitRpsBrush(RpsBrush_HumanScissors, RpsHandSize);
	InitRpsBrush(RpsBrush_DemonRock, RpsHandSize);
	InitRpsBrush(RpsBrush_DemonPaper, RpsHandSize);
	InitRpsBrush(RpsBrush_DemonScissors, RpsHandSize);
	if (TexPool)
	{
		T66SlateTexture::BindBrushAsync(TexPool, TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/RPS/HumanRock.HumanRock"))), this, RpsBrush_HumanRock, FName(TEXT("RpsHumanRock")), true);
		T66SlateTexture::BindBrushAsync(TexPool, TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/RPS/HumanPaper.HumanPaper"))), this, RpsBrush_HumanPaper, FName(TEXT("RpsHumanPaper")), true);
		T66SlateTexture::BindBrushAsync(TexPool, TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/RPS/HumanScissors.HumanScissors"))), this, RpsBrush_HumanScissors, FName(TEXT("RpsHumanScissors")), true);
		T66SlateTexture::BindBrushAsync(TexPool, TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/RPS/DemonRock.DemonRock"))), this, RpsBrush_DemonRock, FName(TEXT("RpsDemonRock")), true);
		T66SlateTexture::BindBrushAsync(TexPool, TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/RPS/DemonPaper.DemonPaper"))), this, RpsBrush_DemonPaper, FName(TEXT("RpsDemonPaper")), true);
		T66SlateTexture::BindBrushAsync(TexPool, TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/RPS/DemonScissors.DemonScissors"))), this, RpsBrush_DemonScissors, FName(TEXT("RpsDemonScissors")), true);
	}

	// --- BlackJack card sprites (52 face cards + 1 back = 53 brushes) ---
	InitBJCardBrushes();
	if (TexPool)
	{
		// Suits in card index order: spades(0), hearts(1), diamonds(2), clubs(3)
		static const TCHAR* SuitNames[] = { TEXT("spades"), TEXT("hearts"), TEXT("diamonds"), TEXT("clubs") };
		static const TCHAR* RankNames[] = { TEXT("A"), TEXT("02"), TEXT("03"), TEXT("04"), TEXT("05"), TEXT("06"), TEXT("07"), TEXT("08"), TEXT("09"), TEXT("10"), TEXT("J"), TEXT("Q"), TEXT("K") };
		for (int32 i = 0; i < 52; ++i)
		{
			const int32 Suit = i / 13;
			const int32 Rank = i % 13;
			const FString AssetName = FString::Printf(TEXT("card_%s_%s"), SuitNames[Suit], RankNames[Rank]);
			const FString PathStr = FString::Printf(TEXT("/Game/UI/Sprites/Games/BJ/%s.%s"), *AssetName, *AssetName);
			FSoftObjectPath CardPath{PathStr};
			TSoftObjectPtr<UTexture2D> CardSoft{MoveTemp(CardPath)};
			T66SlateTexture::BindBrushAsync(TexPool, CardSoft, this, BJCardBrushes[i], FName(*FString::Printf(TEXT("BJCard_%d"), i)), true);
		}
		// Card back = index 52
		{
			FSoftObjectPath BackPath{TEXT("/Game/UI/Sprites/Games/BJ/card_back.card_back")};
			TSoftObjectPtr<UTexture2D> BackSoft{MoveTemp(BackPath)};
			T66SlateTexture::BindBrushAsync(TexPool, BackSoft, this, BJCardBrushes[52], FName(TEXT("BJCardBack")), true);
		}
	}

	// Inventory slot icon brushes — same as Vendor (160×160)
	const float InvSlotSz = FT66Style::Tokens::InventorySlotSize;
	for (int32 i = 0; i < InventorySlotIconBrushes.Num(); ++i)
	{
		InventorySlotIconBrushes[i] = MakeShared<FSlateBrush>();
		InventorySlotIconBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		InventorySlotIconBrushes[i]->ImageSize = FVector2D(InvSlotSz, InvSlotSz);
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

	// Right panel: gambler portrait larger than Vendor (260×260), Bank at bottom
	static constexpr float GamblerAngerCircleSize = 260.f;
	TSharedRef<SWidget> RightPanel =
		FT66Style::MakePanel(
			SNew(SVerticalBox)
			// Gambler portrait (top) — larger than Vendor for prominence
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 14.f)
			[
				SNew(SBox)
				.WidthOverride(GamblerAngerCircleSize)
				.HeightOverride(GamblerAngerCircleSize)
				[
					SAssignNew(AngerCircleImage, SImage)
					.Image(&AngerFace_Happy)
				]
			]
			// Spacer to push Bank to bottom of panel
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SSpacer)
			]
			// Bank (bottom, separate panel) — same wrapper as Vendor
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 0.f)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, FT66Style::Tokens::Space4)
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
							.WidthOverride(FT66Style::Tokens::NPCBankSpinBoxWidth)
							.HeightOverride(FT66Style::Tokens::NPCBankSpinBoxHeight)
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
								Loc ? Loc->GetText_Borrow() : NSLOCTEXT("T66.Vendor", "Borrow_Button", "BORROW"),
								FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBorrowClicked),
								ET66ButtonType::Neutral)
								.SetMinWidth(0.f)
								.SetPadding(FMargin(16.f, 10.f)))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(FT66Style::Tokens::NPCBankSpinBoxWidth)
							.HeightOverride(FT66Style::Tokens::NPCBankSpinBoxHeight)
							[
								SAssignNew(PaybackAmountSpin, SSpinBox<int32>)
								.MinValue(0).MaxValue(999999).Delta(10)
								.Font(FT66Style::Tokens::FontBold(16))
								.Value_Lambda([this]() { return PaybackAmount; })
								.OnValueChanged_Lambda([this](int32 V) { PaybackAmount = FMath::Max(0, V); })
							]
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							FT66Style::MakeButton(FT66ButtonParams(
								Loc ? Loc->GetText_Payback() : NSLOCTEXT("T66.Vendor", "Payback_Button", "PAYBACK"),
								FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnPaybackClicked),
								ET66ButtonType::Neutral)
								.SetMinWidth(0.f)
								.SetPadding(FMargin(16.f, 10.f)))
						]
					]
				,
					FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66Style::Tokens::Space5))
			]
		,
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel));

	// Game card: icon + game name above Play button; tall enough so Play button fits inside panel
	static constexpr float GameCardTotalHeight = 420.f;
	auto MakeGameCard = [&](const FText& TitleText, const FOnClicked& OnClicked, const FSlateBrush* IconBrush) -> TSharedRef<SWidget>
	{
		const FText PlayText = NSLOCTEXT("T66.Gambler", "Play", "Play");
		TSharedRef<SWidget> PlayBtn = FT66Style::MakeButton(
			FT66ButtonParams(PlayText, OnClicked, ET66ButtonType::Primary)
			.SetMinWidth(100.f)
			.SetPadding(FMargin(8.f, 6.f)));
		return SNew(SBox)
			.WidthOverride(GameCardIconSize)
			.HeightOverride(GameCardTotalHeight)
			.Padding(FMargin(4.f, 0.f))
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					// 1. Large image (texture size)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)
						[
							FT66Style::MakePanel(
								SNew(SBox)
								.WidthOverride(GameCardIconSize)
								.HeightOverride(GameCardIconSize)
								[
									SNew(SImage)
									.Image(IconBrush)
									.ColorAndOpacity(FLinearColor::White)
								],
								FT66PanelParams(ET66PanelType::Panel2).SetPadding(0.f))
						]
					]
					// 2. Game name above Play button
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.TextStyle(&TextHeading)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					// 3. Play button
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f)
					[
						PlayBtn
					]
				,
					FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4))
			];
	};

	const FText MoreGamesText = NSLOCTEXT("T66.Gambler", "MoreGames", "More Games");
	const FText BackToGamesText = NSLOCTEXT("T66.Gambler", "BackToGames", "Back to Games");
	const FText LotteryText = NSLOCTEXT("T66.Gambler", "Lottery", "Lottery");
	const FText PlinkoText = NSLOCTEXT("T66.Gambler", "Plinko", "Plinko");
	const FText BoxOpeningText = NSLOCTEXT("T66.Gambler", "BoxOpening", "Box Opening");

	TSharedRef<SWidget> MainGamesView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f) [ SNew(SSpacer) ]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SUniformGridPanel)
				.SlotPadding(FMargin(16.f, 8.f))
				+ SUniformGridPanel::Slot(0, 0) [ MakeGameCard(Loc ? Loc->GetText_RockPaperScissors() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenRps), &GameIcon_Rps) ]
				+ SUniformGridPanel::Slot(1, 0) [ MakeGameCard(Loc ? Loc->GetText_BlackJack() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenBlackJack), &GameIcon_BlackJack) ]
				+ SUniformGridPanel::Slot(2, 0) [ MakeGameCard(Loc ? Loc->GetText_CoinFlip() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenCoinFlip), &GameIcon_CoinFlip) ]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 28.f, 0.f, 0.f)
		[
			FT66Style::MakeButton(
				FT66ButtonParams(MoreGamesText, FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnMoreGamesClicked), ET66ButtonType::Neutral)
				.SetMinWidth(0.f)
				.SetPadding(FMargin(20.f, 12.f)))
		];

	TSharedRef<SWidget> MoreGamesView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f) [ SNew(SSpacer) ]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SUniformGridPanel)
				.SlotPadding(FMargin(16.f, 8.f))
				+ SUniformGridPanel::Slot(0, 0) [ MakeGameCard(LotteryText, FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenLottery), &GameIcon_Lottery) ]
				+ SUniformGridPanel::Slot(1, 0) [ MakeGameCard(PlinkoText, FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenPlinko), &GameIcon_Plinko) ]
				+ SUniformGridPanel::Slot(2, 0) [ MakeGameCard(BoxOpeningText, FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenBoxOpening), &GameIcon_BoxOpening) ]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, FT66Style::Tokens::Space6, 0.f, 0.f)
		[
			FT66Style::MakeButton(
				FT66ButtonParams(BackToGamesText, FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBackToMainGames), ET66ButtonType::Neutral)
				.SetMinWidth(0.f)
				.SetPadding(FMargin(20.f, 12.f)))
		];

	TSharedRef<SWidget> CardsView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SAssignNew(GameSelectionSwitcher, SWidgetSwitcher)
			+ SWidgetSwitcher::Slot() [ MainGamesView ]
			+ SWidgetSwitcher::Slot() [ MoreGamesView ]
		];

	TSharedRef<SWidget> CoinFlipView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Gambler", "Back", "BACK"),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnGameBackToSelection),
					ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f)))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SAssignNew(CoinFlipStatusText, STextBlock)
				.Text(FText::GetEmpty())
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() {
					const int32 N = (LockedBetAmount > 0) ? LockedBetAmount : PendingBetAmount;
					return N > 0 ? FText::Format(NSLOCTEXT("T66.Gambler", "WagerFormat", "Wager: {0}"), FText::AsNumber(N)) : FText::GetEmpty();
				})
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
		]
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
					SNew(SOverlay)
					// Coin sprite (center)
					+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						SNew(SBox).WidthOverride(CoinSpriteSize).HeightOverride(CoinSpriteSize)
						[
							SAssignNew(CoinFlipImage, SImage)
							.Image(&CoinBrush_Heads)
						]
					]
					// Result text overlay (bottom)
					+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 6.f)
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
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Gambler", "Back", "BACK"),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnGameBackToSelection),
					ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f)))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SAssignNew(RpsStatusText, STextBlock)
				.Text(FText::GetEmpty())
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() {
					const int32 N = (LockedBetAmount > 0) ? LockedBetAmount : PendingBetAmount;
					return N > 0 ? FText::Format(NSLOCTEXT("T66.Gambler", "WagerFormat", "Wager: {0}"), FText::AsNumber(N)) : FText::GetEmpty();
				})
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
		]
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
					SNew(SVerticalBox)
					// Hand images (hidden until reveal)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 6.f)
					[
						SAssignNew(RpsHandsContainer, SBox)
						.Visibility(EVisibility::Collapsed)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
							[
								SNew(SBox).WidthOverride(RpsHandSize).HeightOverride(RpsHandSize)
								[
									SAssignNew(RpsHumanHandImage, SImage)
									.Image(&RpsBrush_HumanRock)
								]
							]
							+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(16.f, 0.f)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.Gambler", "VS", "VS"))
								.TextStyle(&TextHeading)
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
							[
								SNew(SBox).WidthOverride(RpsHandSize).HeightOverride(RpsHandSize)
								[
									SAssignNew(RpsDemonHandImage, SImage)
									.Image(&RpsBrush_DemonRock)
								]
							]
						]
					]
					// Result text (centered)
					+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
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

	// Black Jack: card slots (image-based). Dealer: hole = card back / face when revealed. Player: up to 11 card images.
	static constexpr int32 BJMaxCards = 11;
	static constexpr float BJCardW = 80.f;
	static constexpr float BJCardH = 112.f;
	BlackJackDealerCardImages.SetNum(BJMaxCards);
	BlackJackPlayerCardImages.SetNum(BJMaxCards);

	auto MakeCardImageBox = [&](TSharedPtr<SImage>& OutImage) -> TSharedRef<SWidget>
	{
		return SNew(SBox).WidthOverride(BJCardW).HeightOverride(BJCardH)
		[
			SAssignNew(OutImage, SImage)
			.Image(GetBJCardBackBrush())
			.Visibility(EVisibility::Collapsed)
		];
	};
	TSharedRef<SWidget> BlackJackDealerRow =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() {
				if (BJDealerHand.Num() < 2) return FText::FromString(TEXT("-"));
				if (!BJDealerHoleRevealed)
				{
					const int32 c = BJDealerHand[1];
					const int32 r = c % 13;
					const int32 v = (r == 0) ? 11 : (r >= 9) ? 10 : (r + 1);
					return FText::AsNumber(v);
				}
				return FText::AsNumber(BJHandValue(BJDealerHand));
			})
			.TextStyle(&TextBody)
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
			[ SNew(STextBlock).Text(Loc ? Loc->GetText_Dealer() : FText::GetEmpty()).TextStyle(&TextBody).ColorAndOpacity(FT66Style::Tokens::Text) ]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBox).WidthOverride(BJCardW).HeightOverride(BJCardH)
					.Visibility_Lambda([this](){ return (BJDeck.Num() > 0 && !BJDealerHoleRevealed) ? EVisibility::Visible : EVisibility::Collapsed; })
					[
						SAssignNew(BlackJackDealerHoleCardBackImage, SImage)
						.Image(GetBJCardBackBrush())
					]
				]
				+ SOverlay::Slot()
				[
					MakeCardImageBox(BlackJackDealerCardImages[0])
				]
			]
		];
	TSharedRef<SHorizontalBox> BlackJackDealerCardsRow = SNew(SHorizontalBox);
	for (int32 i = 1; i < BJMaxCards; ++i)
	{
		BlackJackDealerCardsRow->AddSlot().AutoWidth().Padding(4.f, 0.f)[ MakeCardImageBox(BlackJackDealerCardImages[i]) ];
	}
	TSharedRef<SHorizontalBox> BlackJackPlayerCardsRow = SNew(SHorizontalBox);
	for (int32 i = 0; i < BJMaxCards; ++i)
	{
		BlackJackPlayerCardsRow->AddSlot().AutoWidth().Padding(4.f, 0.f)[ MakeCardImageBox(BlackJackPlayerCardImages[i]) ];
	}
	TSharedRef<SWidget> BlackJackView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Gambler", "Back", "BACK"),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnGameBackToSelection),
					ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f)))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SAssignNew(BlackJackStatusText, STextBlock)
				.Text(FText::GetEmpty())
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() {
					const int32 N = (LockedBetAmount > 0) ? LockedBetAmount : (BJBetAmount > 0 ? BJBetAmount : PendingBetAmount);
					return N > 0 ? FText::Format(NSLOCTEXT("T66.Gambler", "WagerFormat", "Wager: {0}"), FText::AsNumber(N)) : FText::GetEmpty();
				})
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_BlackJack() : FText::GetEmpty())
			.TextStyle(&TextHeading)
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()[ BlackJackDealerRow ]
			+ SHorizontalBox::Slot().AutoWidth()[ BlackJackDealerCardsRow ]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() {
					if (BJPlayerHands.Num() == 0 || BJCurrentHandIndex >= BJPlayerHands.Num())
						return FText::FromString(TEXT("-"));
					return FText::AsNumber(BJHandValue(BJPlayerHands[BJCurrentHandIndex]));
				})
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
				[ SNew(STextBlock).Text(Loc ? Loc->GetText_You() : FText::GetEmpty()).TextStyle(&TextBody).ColorAndOpacity(FT66Style::Tokens::Text) ]
				+ SHorizontalBox::Slot().AutoWidth()[ BlackJackPlayerCardsRow ]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 16.f, 0.f, 12.f)
		[
			SNew(SBox).HeightOverride(FT66Style::Tokens::NPCBankSpinBoxHeight)
			[
				SAssignNew(BlackJackResultText, STextBlock)
				.Text(Loc ? Loc->GetText_ResultDash() : FText::GetEmpty())
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f)
			[
				SNew(SBox)
				.Visibility_Lambda([this]() { return (BJDeck.Num() == 0) ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					FT66Style::MakeButton(FT66ButtonParams(
						NSLOCTEXT("T66.Gambler", "Deal", "Deal"),
						FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBJDealClicked),
						ET66ButtonType::Primary).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f)))
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f)
			[
				SNew(SBox)
				.Visibility_Lambda([this]() { return (BJDeck.Num() > 0) ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					FT66Style::MakeButton(FT66ButtonParams(
						Loc ? Loc->GetText_Hit() : FText::GetEmpty(),
						FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBJHit),
						ET66ButtonType::Primary).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f)))
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f)
			[
				SNew(SBox)
				.Visibility_Lambda([this]() { return (BJDeck.Num() > 0) ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					FT66Style::MakeButton(FT66ButtonParams(
						Loc ? Loc->GetText_Stand() : FText::GetEmpty(),
						FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBJStand),
						ET66ButtonType::Primary).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f)))
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f)
			[
				SNew(SBox)
				.Visibility_Lambda([this]() { return (BJDeck.Num() > 0) ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					FT66Style::MakeButton(FT66ButtonParams(
						Loc ? Loc->GetText_Double() : FText::GetEmpty(),
						FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBJDouble),
						ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f)))
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f)
			[
				SNew(SBox)
				.Visibility_Lambda([this]() { return (BJDeck.Num() > 0) ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					FT66Style::MakeButton(FT66ButtonParams(
						Loc ? Loc->GetText_Split() : FText::GetEmpty(),
						FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBJSplit),
						ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f)))
				]
			]
		];

	// Back button row: top-left, not wide (used by Lottery, Plinko, Box Opening)
	auto MakeGameBackRow = [&]() -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Gambler", "Back", "BACK"),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnGameBackToSelection),
					ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f)))
			];
	};

	const FText LotteryDesc = NSLOCTEXT("T66.Gambler", "LotteryDesc", "Pick 5 numbers from 1–10. Five numbers are drawn at random. Payout depends on how many match.");
	const FText YourPicksText = NSLOCTEXT("T66.Gambler", "YourPicks", "Your picks:");
	const FText DrawnText = NSLOCTEXT("T66.Gambler", "Drawn", "Drawn:");

	TSharedRef<SHorizontalBox> LotteryNumberRow = SNew(SHorizontalBox);
	for (int32 i = 0; i < 10; ++i)
	{
		const int32 Num = i + 1;
		LotteryNumberRow->AddSlot()
			.AutoWidth()
			.Padding(i > 0 ? FMargin(FT66Style::Tokens::Space2, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			FT66Style::MakePanel(
				FT66Style::MakeButton(
					FT66ButtonParams(FText::AsNumber(Num), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnLotteryNumberClicked, Num), ET66ButtonType::Neutral)
					.SetMinWidth(0.f).SetPadding(FMargin(10.f, 8.f))),
				FT66PanelParams(ET66PanelType::Panel2).SetPadding(0.f),
				&LotteryNumberBorders[i])
		];
	}

	TSharedRef<SWidget> LotteryView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) [ MakeGameBackRow() ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f)
			[ SNew(STextBlock).Text(LotteryText).TextStyle(&TextTitle).ColorAndOpacity(FT66Style::Tokens::Text) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 8.f)
			[ SNew(STextBlock).Text(LotteryDesc).TextStyle(&TextBody).ColorAndOpacity(FT66Style::Tokens::TextMuted).AutoWrapText(true) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f) [ LotteryNumberRow ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 2.f)
			[ SAssignNew(LotteryPicksText, STextBlock).Text(YourPicksText).TextStyle(&TextBody).ColorAndOpacity(FT66Style::Tokens::Text) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 2.f)
			[ SAssignNew(LotteryDrawnText, STextBlock).Text(DrawnText).TextStyle(&TextBody).ColorAndOpacity(FT66Style::Tokens::Text) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f)
			[ SAssignNew(LotteryResultText, STextBlock).Text(FText::GetEmpty()).TextStyle(&TextHeading).ColorAndOpacity(FT66Style::Tokens::Text) ];

	// Plinko: visual board with pin grid + ball
	const FText PlinkoSlotsDesc = NSLOCTEXT("T66.Gambler", "PlinkoSlotsDesc", "Ball drops through pins. Payout by slot below.");
	static constexpr float PlinkoBoardW = 280.f;
	static constexpr float PlinkoBoardH = 220.f;
	static constexpr float PlinkoPinSize = 8.f;
	static constexpr float PlinkoPinSpacing = 12.f;
	static constexpr float PlinkoRowH = 20.f;
	static constexpr float PlinkoBallSize = 14.f;
	static constexpr float PlinkoSlotPayouts[PlinkoSlotCount] = { 30.f, 5.f, 1.f, 0.5f, 0.25f, 0.5f, 1.f, 5.f, 30.f };

	auto MakePinDot = []() -> TSharedRef<SWidget> {
		return SNew(SBox).WidthOverride(PlinkoPinSize).HeightOverride(PlinkoPinSize)
			[ SNew(SBorder).BorderBackgroundColor(FLinearColor(0.5f, 0.55f, 0.65f, 1.f)).Padding(0.f)[ SNew(SSpacer) ] ];
	};

	TSharedRef<SVerticalBox> PinGrid = SNew(SVerticalBox);
	for (int32 r = 0; r < 8; ++r)
	{
		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
		Row->AddSlot().FillWidth(1.f);
		for (int32 j = 0; j <= r; ++j)
		{
			Row->AddSlot().AutoWidth().Padding(j > 0 ? FMargin(PlinkoPinSpacing * 0.5f, 0.f, PlinkoPinSpacing * 0.5f, 0.f) : FMargin(0.f))[ MakePinDot() ];
		}
		Row->AddSlot().FillWidth(1.f);
		PinGrid->AddSlot().AutoHeight().HAlign(HAlign_Center)[ SNew(SBox).HeightOverride(PlinkoRowH)[ Row ] ];
	}
	TSharedRef<SHorizontalBox> SlotRow = SNew(SHorizontalBox);
	for (int32 s = 0; s < PlinkoSlotCount; ++s)
	{
		SlotRow->AddSlot()
			.FillWidth(1.f)
			.Padding(2.f, 4.f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.25f, 0.3f, 0.4f, 1.f))
				.Padding(4.f)
				[
					SNew(STextBlock)
					.Text(FText::AsNumber(FMath::RoundToInt(PlinkoSlotPayouts[s] * 100) / 100.0))
					.TextStyle(&TextChip)
					.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 1.f))
					.Justification(ETextJustify::Center)
				]
			];
	}
	PinGrid->AddSlot().AutoHeight()[ SNew(SBox).HeightOverride(36.f)[ SlotRow ] ];

	TSharedRef<SWidget> PlinkoBall = SNew(SBox).WidthOverride(PlinkoBallSize).HeightOverride(PlinkoBallSize)
		[
			SNew(SBorder).BorderBackgroundColor(FLinearColor(1.f, 0.85f, 0.2f, 1.f)).Padding(0.f)[ SNew(SSpacer) ]
		];

	TSharedRef<SWidget> PlinkoView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) [ MakeGameBackRow() ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f)
			[ SNew(STextBlock).Text(PlinkoText).TextStyle(&TextTitle).ColorAndOpacity(FT66Style::Tokens::Text) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 4.f)
			[ SNew(STextBlock).Text(PlinkoSlotsDesc).TextStyle(&TextBody).ColorAndOpacity(FT66Style::Tokens::TextMuted).AutoWrapText(true) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f)
			[
				SAssignNew(PlinkoBoardContainer, SBox)
				.WidthOverride(PlinkoBoardW)
				.HeightOverride(PlinkoBoardH)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()[ FT66Style::MakePanel(PinGrid, FT66PanelParams(ET66PanelType::Panel2).SetPadding(4.f)) ]
					+ SOverlay::Slot()
						.HAlign(HAlign_Left).VAlign(VAlign_Top)
						.Padding(TAttribute<FMargin>::CreateLambda([this]() {
							const float ColW = PlinkoBoardW / static_cast<float>(PlinkoSlotCount);
							return FMargin(ColW * PlinkoBallSlot + (ColW - PlinkoBallSize) * 0.5f, PlinkoRow * PlinkoRowH + (PlinkoRowH - PlinkoBallSize) * 0.5f, 0.f, 0.f);
						}))
						[ PlinkoBall ]
				]
			]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f)
			[ SAssignNew(PlinkoResultText, STextBlock).Text(FText::GetEmpty()).TextStyle(&TextHeading).ColorAndOpacity(FT66Style::Tokens::Text) ];

	// Box Opening: 5 colored squares; middle one is the result when spin stops
	const FText BoxOpeningDesc = NSLOCTEXT("T66.Gambler", "BoxOpeningDesc", "Bet then spin. Colors scroll; the one in the middle when it stops is your result.");

	static constexpr float BoxSquareSize = 48.f;
	static constexpr float BoxSquareGap = 4.f;
	TSharedRef<SHorizontalBox> BoxStripRow = SNew(SHorizontalBox);
	for (int32 i = 0; i < 5; ++i)
	{
		const int32 Idx = i;
		BoxStripRow->AddSlot()
			.AutoWidth()
			.Padding(Idx > 0 ? FMargin(BoxSquareGap, 0.f, 0.f, 0.f) : FMargin(0.f))
			[
				SNew(SBorder)
				.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([this, Idx]() {
					return FSlateColor(GetBoxOpeningColor((BoxOpeningStripOffset + Idx) % BoxColorCount));
				}))
				.Padding(0.f)
				[
					SNew(SBox).WidthOverride(BoxSquareSize).HeightOverride(BoxSquareSize)[ SNew(SSpacer) ]
				]
			];
	}

	TSharedRef<SWidget> BoxOpeningView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) [ MakeGameBackRow() ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f)
			[ SNew(STextBlock).Text(BoxOpeningText).TextStyle(&TextTitle).ColorAndOpacity(FT66Style::Tokens::Text) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 8.f)
			[ SNew(STextBlock).Text(BoxOpeningDesc).TextStyle(&TextBody).ColorAndOpacity(FT66Style::Tokens::TextMuted).AutoWrapText(true) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 12.f, 0.f, 4.f)
			[
				SNew(SBox)
				.WidthOverride(5 * BoxSquareSize + 4 * BoxSquareGap)
				.HeightOverride(BoxSquareSize + 8.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center)
						[
							SAssignNew(BoxOpeningStripContainer, SBox)[ BoxStripRow ]
						]
					+ SOverlay::Slot()
						.HAlign(HAlign_Left).VAlign(VAlign_Center)
						.Padding(2.f * (BoxSquareSize + BoxSquareGap), 0.f, 0.f, 0.f)
						[
							SNew(SBorder)
							.BorderBackgroundColor(FLinearColor(1.f, 1.f, 1.f, 0.9f))
							.Padding(2.f)
							.Visibility(EVisibility::HitTestInvisible)
							[
								SNew(SBox).WidthOverride(BoxSquareSize + 2.f).HeightOverride(BoxSquareSize + 2.f)[ SNew(SSpacer) ]
							]
						]
				]
			]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f)
			[ SAssignNew(BoxOpeningResultText, STextBlock).Text(FText::GetEmpty()).TextStyle(&TextHeading).ColorAndOpacity(FT66Style::Tokens::Text) ];

	TSharedRef<SWidget> BetRow =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
		[ SNew(STextBlock).Text(Loc ? Loc->GetText_BetAmount() : NSLOCTEXT("T66.Gambler", "BetAmount", "Bet Amount")).TextStyle(&TextBody).ColorAndOpacity(FT66Style::Tokens::Text) ]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
		[
			SAssignNew(GambleAmountSpin, SSpinBox<int32>)
			.MinValue(0).MaxValue(999999).Delta(10)
			.Font(FT66Style::Tokens::FontBold(26))
			.Value_Lambda([this]() { return GambleAmount; })
			.OnValueChanged_Lambda([this](int32 V) { GambleAmount = FMath::Max(0, V); })
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			FT66Style::MakeButton(FT66ButtonParams(
				Loc ? Loc->GetText_Bet() : NSLOCTEXT("T66.Gambler", "Bet", "Bet"),
				FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBetClicked),
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
				+ SWidgetSwitcher::Slot() [ BlackJackView ]
				+ SWidgetSwitcher::Slot() [ LotteryView ]
				+ SWidgetSwitcher::Slot() [ PlinkoView ]
				+ SWidgetSwitcher::Slot() [ BoxOpeningView ]
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f, 0.f, 0.f)
			[
				SNew(SBox)
				.Visibility_Lambda([this]() -> EVisibility
				{
					if (!CasinoSwitcher.IsValid()) return EVisibility::Collapsed;
					const int32 Idx = CasinoSwitcher->GetActiveWidgetIndex();
					if (Idx == 0) return EVisibility::Collapsed;
					// During a Black Jack round, hide bet row until round is resolved
					if (Idx == 3 && BJDeck.Num() > 0) return EVisibility::Collapsed;
					// Show bet row for Lottery/Plinko/BoxOpening (4,5,6) and other game views
					return EVisibility::Visible;
				})
				[
					BetRow
				]
			]
		,
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel));

	// Inventory slot buttons: same structure as vendor (single panel, overlay image + dash, no inner box)
	for (int32 i = 0; i < UT66RunStateSubsystem::MaxInventorySlots; ++i)
	{
		InventorySlotButtons[i] = FT66Style::MakeButton(
			FT66ButtonParams(FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnSelectInventorySlot, i),
				ET66ButtonType::Neutral)
				.SetMinWidth(FT66Style::Tokens::InventorySlotSize)
				.SetHeight(FT66Style::Tokens::InventorySlotSize)
				.SetPadding(FMargin(0.f))
				.SetContent(
					FT66Style::MakePanel(
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SAssignNew(InventorySlotIconImages[i], SImage)
							.Image(InventorySlotIconBrushes[i].Get())
							.ColorAndOpacity(FLinearColor::White)
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

	// Inventory grid: same as Vendor — 20 slots in horizontal scroll, each 160×160
	TSharedRef<SUniformGridPanel> GamblerInventoryGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(FT66Style::Tokens::Space2, 0.f));
	for (int32 Inv = 0; Inv < UT66RunStateSubsystem::MaxInventorySlots; ++Inv)
	{
		GamblerInventoryGrid->AddSlot(Inv, 0)
		[
			SNew(SBox)
			.WidthOverride(FT66Style::Tokens::InventorySlotSize)
			.HeightOverride(FT66Style::Tokens::InventorySlotSize)
			[
				InventorySlotButtons[Inv].ToSharedRef()
			]
		];
	}

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
					SNew(SScrollBox)
					.Orientation(Orient_Horizontal)
					.ScrollBarVisibility(EVisibility::Visible)
					+ SScrollBox::Slot()
					[
						GamblerInventoryGrid
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(FT66Style::Tokens::Space6, 0.f, 0.f, 0.f)
				[
					SAssignNew(SellPanelContainer, SBox)
					.WidthOverride(FT66Style::Tokens::InventorySlotSize)
					.HeightOverride(FT66Style::Tokens::InventorySlotSize)
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
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, FT66Style::Tokens::Space4)
		[
			SNew(STextBlock)
			.Text(CasinoTitle)
			.TextStyle(&TextTitle)
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, FT66Style::Tokens::Space6)
		[
			SNew(SBox)
			.HeightOverride(FT66Style::Tokens::NPCMainRowHeight)
			[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66Style::Tokens::Space6, 0.f)
			[
				SAssignNew(StatsPanelBox, SBox)
				.WidthOverride(FT66Style::Tokens::NPCGamblerStatsPanelWidth)
				.HeightOverride(FT66Style::Tokens::NPCMainRowHeight)
				[
					T66StatsPanelSlate::MakeEssentialStatsPanel(RunState, Loc, FT66Style::Tokens::NPCGamblerStatsPanelWidth, true)
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66Style::Tokens::Space6, 0.f)
			[
				SNew(SBox)
				.WidthOverride(FT66Style::Tokens::NPCCenterPanelTotalWidth)
				.HeightOverride(FT66Style::Tokens::NPCMainRowHeight)
				[
					CenterPanel
				]
			]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(SSpacer)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(FT66Style::Tokens::NPCRightPanelWidth)
				[
					RightPanel
				]
			]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 0.f)
		[
			SNew(SBox)
			.HeightOverride(FT66Style::Tokens::NPCGamblerInventoryPanelHeight)
			[
				InventoryPanel
			]
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
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SAssignNew(StatusText, STextBlock)
						.Text(FText::GetEmpty())
						.TextStyle(&TextBody)
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					]
				]
				+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Fill).VAlign(VAlign_Top).Padding(40.f, 16.f, 40.f, 0.f)
				[
					SAssignNew(PageSwitcher, SWidgetSwitcher)
					+ SWidgetSwitcher::Slot()
					[ DialoguePage ]
					+ SWidgetSwitcher::Slot()
					[ CasinoPage ]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FT66Style::Tokens::Space6, FT66Style::Tokens::Space4, 0.f, 0.f)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					Loc ? Loc->GetText_Back() : FText::GetEmpty(),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBack),
					ET66ButtonType::Neutral)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(18.f, 10.f)))
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

FReply UT66GamblerOverlayWidget::OnBetClicked()
{
	if (GambleAmount <= 0)
	{
		UT66LocalizationSubsystem* Loc2 = nullptr;
		if (UWorld* World = GetWorld())
		{
			if (World->GetGameInstance())
			{
				Loc2 = World->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>();
			}
		}
		SetStatus(Loc2 ? Loc2->GetText_GambleAmountMustBePositive() : NSLOCTEXT("T66.Gambler", "GambleAmountMustBePositive", "Gamble amount must be > 0."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return FReply::Handled();
	}
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState || RunState->GetCurrentGold() < GambleAmount)
	{
		UT66LocalizationSubsystem* Loc2 = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
		SetStatus(Loc2 ? Loc2->GetText_NotEnoughGold() : NSLOCTEXT("T66.Gambler", "NotEnoughGold", "Not enough gold."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		RefreshTopBar();
		return FReply::Handled();
	}

	// Lottery / Plinko / Box Opening: Bet starts the game (deduct and run)
	if (CasinoSwitcher.IsValid())
	{
		const int32 Idx = CasinoSwitcher->GetActiveWidgetIndex();
		if (Idx == 4)
		{
			if (LotterySelected.Num() != 5)
			{
				SetStatus(NSLOCTEXT("T66.Gambler", "Pick5Numbers", "Pick exactly 5 numbers."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
				return FReply::Handled();
			}
			if (!RunState->TrySpendGold(GambleAmount)) { RefreshTopBar(); return FReply::Handled(); }
			PendingBetAmount = GambleAmount;
			LockedBetAmount = 0;
			RefreshTopBar();
			StartLotteryDraw();
			return FReply::Handled();
		}
		if (Idx == 5)
		{
			if (!RunState->TrySpendGold(GambleAmount)) { RefreshTopBar(); return FReply::Handled(); }
			PendingBetAmount = GambleAmount;
			LockedBetAmount = 0;
			RefreshTopBar();
			StartPlinkoDrop();
			return FReply::Handled();
		}
		if (Idx == 6)
		{
			if (!RunState->TrySpendGold(GambleAmount)) { RefreshTopBar(); return FReply::Handled(); }
			PendingBetAmount = GambleAmount;
			LockedBetAmount = 0;
			RefreshTopBar();
			StartBoxOpeningSpin();
			return FReply::Handled();
		}
	}

	LockedBetAmount = GambleAmount;
	SetStatus(FText::GetEmpty());
	RefreshTopBar();
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
	// Reset coin to Heads face
	if (CoinFlipImage.IsValid()) CoinFlipImage->SetImage(&CoinBrush_Heads);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnOpenRps()
{
	SetPage(EGamblerPage::RockPaperScissors);
	// Hide hand images until a round is revealed
	if (RpsHandsContainer.IsValid()) RpsHandsContainer->SetVisibility(EVisibility::Collapsed);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnOpenBlackJack()
{
	SetPage(EGamblerPage::BlackJack);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnGameBackToSelection()
{
	SetPage(EGamblerPage::Casino);
	if (GameSelectionSwitcher.IsValid()) GameSelectionSwitcher->SetActiveWidgetIndex(0);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnMoreGamesClicked()
{
	if (GameSelectionSwitcher.IsValid()) GameSelectionSwitcher->SetActiveWidgetIndex(1);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnBackToMainGames()
{
	if (GameSelectionSwitcher.IsValid()) GameSelectionSwitcher->SetActiveWidgetIndex(0);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnOpenLottery()
{
	SetPage(EGamblerPage::Lottery);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnOpenPlinko()
{
	SetPage(EGamblerPage::Plinko);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnOpenBoxOpening()
{
	SetPage(EGamblerPage::BoxOpening);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnLotteryNumberClicked(int32 Num)
{
	if (Num < 1 || Num > 10) return FReply::Handled();
	if (LotterySelected.Contains(Num))
		LotterySelected.Remove(Num);
	else if (LotterySelected.Num() < 5)
		LotterySelected.Add(Num);
	for (int32 i = 0; i < 10 && i < LotteryNumberBorders.Num(); ++i)
	{
		if (LotteryNumberBorders[i].IsValid())
			LotteryNumberBorders[i]->SetBorderBackgroundColor(LotterySelected.Contains(i + 1) ? FT66Style::Tokens::Accent2 : FT66Style::Tokens::Panel2);
	}
	if (LotteryPicksText.IsValid())
	{
		TArray<int32> Sorted(LotterySelected.Array());
		Sorted.Sort();
		FString Str;
		for (int32 i = 0; i < Sorted.Num(); ++i) { if (i) Str += TEXT(", "); Str += FString::FromInt(Sorted[i]); }
		LotteryPicksText->SetText(FText::FromString(FString::Printf(TEXT("Your picks: %s"), *Str)));
	}
	if (CasinoSwitcher.IsValid()) CasinoSwitcher->Invalidate(EInvalidateWidget::Layout);
	return FReply::Handled();
}

void UT66GamblerOverlayWidget::StartLotteryDraw()
{
	LotteryDrawn.Reset();
	UT66RunStateSubsystem* RunState = nullptr;
	UT66RngSubsystem* RngSub = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
			RngSub = GI->GetSubsystem<UT66RngSubsystem>();
		}
	}
	TArray<int32> Pool;
	for (int32 i = 1; i <= 10; ++i) Pool.Add(i);
	if (RngSub)
	{
		for (int32 i = Pool.Num() - 1; i > 0; --i)
			Pool.Swap(i, RngSub->GetRunStream().RandRange(0, i));
	}
	else
	{
		for (int32 i = Pool.Num() - 1; i > 0; --i)
			Pool.Swap(i, FMath::RandRange(0, i));
	}
	for (int32 i = 0; i < 5; ++i) LotteryDrawn.Add(Pool[i]);
	LotteryRevealedCount = 0;
	if (LotteryDrawnText.IsValid()) LotteryDrawnText->SetText(NSLOCTEXT("T66.Gambler", "Drawn", "Drawn:"));
	if (LotteryResultText.IsValid()) LotteryResultText->SetText(FText::GetEmpty());
	UWorld* World = GetWorld();
	if (World) World->GetTimerManager().SetTimer(LotteryDrawTimerHandle, this, &UT66GamblerOverlayWidget::TickLotteryRevealNext, 0.6f, true, 0.1f);
}

void UT66GamblerOverlayWidget::TickLotteryRevealNext()
{
	UWorld* World = GetWorld();
	if (!World || LotteryRevealedCount >= 5)
	{
		if (World) World->GetTimerManager().ClearTimer(LotteryDrawTimerHandle);
		if (LotteryRevealedCount >= 5)
		{
			int32 Matches = 0;
			for (int32 D : LotteryDrawn)
				if (LotterySelected.Contains(D)) ++Matches;
			static const float PayoutMultiplier[] = { 0.f, 0.25f, 0.5f, 1.f, 5.f, 30.f };
			float Mult = (Matches >= 0 && Matches <= 5) ? PayoutMultiplier[Matches] : 0.f;
			AwardPayout(PendingBetAmount, Mult);
			if (LotteryResultText.IsValid())
				LotteryResultText->SetText(FText::Format(
					NSLOCTEXT("T66.Gambler", "LotteryResultFmt", "Matches: {0} — Payout: {1}x"),
					FText::AsNumber(Matches), FText::AsNumber(FMath::RoundToInt(Mult * 100) / 100.0)));
		}
		return;
	}
	LotteryRevealedCount++;
	FString DrawnStr = TEXT("Drawn: ");
	for (int32 i = 0; i < LotteryRevealedCount; ++i)
	{
		if (i) DrawnStr += TEXT(", ");
		DrawnStr += FString::FromInt(LotteryDrawn[i]);
	}
	if (LotteryDrawnText.IsValid()) LotteryDrawnText->SetText(FText::FromString(DrawnStr));
}

FLinearColor UT66GamblerOverlayWidget::GetBoxOpeningColor(int32 ColorIndex) const
{
	switch (ColorIndex % BoxColorCount)
	{
		case 0: return FLinearColor(0.15f, 0.15f, 0.15f, 1.f);   // Black
		case 1: return FLinearColor(0.9f, 0.2f, 0.2f, 1.f);       // Red
		case 2: return FLinearColor(0.95f, 0.85f, 0.2f, 1.f);     // Yellow
		case 3: return FLinearColor(0.95f, 0.95f, 0.95f, 1.f);    // White
		default: return FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
	}
}

void UT66GamblerOverlayWidget::StartPlinkoDrop()
{
	// 9 slots: classic plinko payouts (middle = 0.25x). Slots 0..8 left to right.
	PlinkoBallSlot = 4;
	PlinkoRow = 0;
	if (PlinkoResultText.IsValid()) PlinkoResultText->SetText(FText::GetEmpty());
	if (PlinkoBoardContainer.IsValid()) PlinkoBoardContainer->Invalidate(EInvalidateWidget::Layout);
	UWorld* World = GetWorld();
	if (World) World->GetTimerManager().SetTimer(PlinkoTimerHandle, this, &UT66GamblerOverlayWidget::TickPlinkoDrop, 0.12f, true, 0.05f);
}

void UT66GamblerOverlayWidget::TickPlinkoDrop()
{
	UWorld* World = GetWorld();
	// 8 rows of pins; each step 50% left (-1 slot) or right (+1 slot), clamped to 0..8
	static const float SlotPayouts[PlinkoSlotCount] = { 30.f, 5.f, 1.f, 0.5f, 0.25f, 0.5f, 1.f, 5.f, 30.f };
	if (!World) return;
	if (PlinkoRow >= 8)
	{
		World->GetTimerManager().ClearTimer(PlinkoTimerHandle);
		float Mult = SlotPayouts[PlinkoBallSlot];
		AwardPayout(PendingBetAmount, Mult);
		if (PlinkoResultText.IsValid())
			PlinkoResultText->SetText(FText::Format(
				NSLOCTEXT("T66.Gambler", "PlinkoWinFmt", "Landed in slot {0} — {1}x — You won {2}!"),
				FText::AsNumber(PlinkoBallSlot + 1), FText::AsNumber(Mult),
				FText::AsNumber(FMath::RoundToInt(static_cast<float>(PendingBetAmount) * Mult))));
		return;
	}
	UT66RngSubsystem* RngSub = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UT66RngSubsystem>() : nullptr;
	bool bRight = RngSub ? (RngSub->GetRunStream().GetFraction() >= 0.5f) : (FMath::FRand() >= 0.5f);
	if (bRight && PlinkoBallSlot < 8) PlinkoBallSlot++;
	else if (!bRight && PlinkoBallSlot > 0) PlinkoBallSlot--;
	PlinkoRow++;
	if (PlinkoBoardContainer.IsValid()) PlinkoBoardContainer->Invalidate(EInvalidateWidget::Layout);
}

void UT66GamblerOverlayWidget::StartBoxOpeningSpin()
{
	BoxOpeningCurrentIndex = 0;
	BoxOpeningSpinSteps = 0;
	BoxOpeningStripOffset = 0;
	// Weighted result: Black (0) most likely, then Red (1), Yellow (2), White (3) rarest. 0.25x, 1x, 5x, 30x
	UT66RngSubsystem* RngSub = nullptr;
	UWorld* World = GetWorld();
	if (World && World->GetGameInstance())
		RngSub = World->GetGameInstance()->GetSubsystem<UT66RngSubsystem>();
	float R = RngSub ? RngSub->GetRunStream().GetFraction() : FMath::FRand();
	if (R < 0.55f) BoxOpeningResultIndex = 0;
	else if (R < 0.85f) BoxOpeningResultIndex = 1;
	else if (R < 0.95f) BoxOpeningResultIndex = 2;
	else BoxOpeningResultIndex = 3;
	if (BoxOpeningResultText.IsValid()) BoxOpeningResultText->SetText(FText::GetEmpty());
	if (BoxOpeningStripContainer.IsValid()) BoxOpeningStripContainer->Invalidate(EInvalidateWidget::Layout);
	if (World) World->GetTimerManager().SetTimer(BoxOpeningTimerHandle, this, &UT66GamblerOverlayWidget::TickBoxOpeningSpin, 0.08f, true, 0.05f);
}

void UT66GamblerOverlayWidget::TickBoxOpeningSpin()
{
	UWorld* World = GetWorld();
	static const TCHAR* ColorNames[] = { TEXT("Black"), TEXT("Red"), TEXT("Yellow"), TEXT("White") };
	static const float Payouts[] = { 0.25f, 1.f, 5.f, 30.f };
	if (!World) return;
	BoxOpeningSpinSteps++;
	// Fast spin then slow; middle of 5 boxes shows (StripOffset+2)%4. When done, set StripOffset so middle = result.
	const int32 FastSteps = 25;
	const int32 SlowSteps = 8;
	if (BoxOpeningSpinSteps <= FastSteps)
		BoxOpeningStripOffset++;
	else if (BoxOpeningSpinSteps <= FastSteps + SlowSteps)
		BoxOpeningStripOffset++;
	else
		BoxOpeningStripOffset = (BoxOpeningResultIndex + 2) % BoxColorCount;  // so (StripOffset+2)%4 == ResultIndex

	if (BoxOpeningStripContainer.IsValid()) BoxOpeningStripContainer->Invalidate(EInvalidateWidget::Layout);

	if (BoxOpeningSpinSteps == FastSteps + SlowSteps + 1)
	{
		World->GetTimerManager().ClearTimer(BoxOpeningTimerHandle);
		float Mult = Payouts[BoxOpeningResultIndex];
		AwardPayout(PendingBetAmount, Mult);
		if (BoxOpeningResultText.IsValid())
			BoxOpeningResultText->SetText(FText::Format(
				NSLOCTEXT("T66.Gambler", "BoxResultFmt", "{0} — {1}x — Won {2}!"),
				FText::FromString(ColorNames[BoxOpeningResultIndex]),
				FText::AsNumber(Mult),
				FText::AsNumber(FMath::RoundToInt(static_cast<float>(PendingBetAmount) * Mult))));
	}
}

FReply UT66GamblerOverlayWidget::OnBJDealClicked()
{
	if (LockedBetAmount <= 0)
	{
		UT66LocalizationSubsystem* Loc2 = nullptr;
		if (UWorld* World = GetWorld())
		{
			if (World->GetGameInstance())
			{
				Loc2 = World->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>();
			}
		}
		SetStatus(Loc2 ? Loc2->GetText_BetFirst() : NSLOCTEXT("T66.Gambler", "BetFirst", "Bet First"), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return FReply::Handled();
	}
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState || !RunState->TrySpendGold(LockedBetAmount))
	{
		UT66LocalizationSubsystem* Loc2 = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
		SetStatus(Loc2 ? Loc2->GetText_NotEnoughGold() : NSLOCTEXT("T66.Gambler", "NotEnoughGold", "Not enough gold."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		RefreshTopBar();
		return FReply::Handled();
	}
	BJBetAmount = LockedBetAmount;
	LockedBetAmount = 0;
	RefreshTopBar();
	StartBlackJackRound();
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnCoinFlipHeads() { ResolveCoinFlip(true); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnCoinFlipTails() { ResolveCoinFlip(false); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnRpsRock() { ResolveRps(0); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnRpsPaper() { ResolveRps(1); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnRpsScissors() { ResolveRps(2); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnBJHit() { HandleBJHit(); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnBJStand() { HandleBJStand(); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnBJDouble() { HandleBJDouble(); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnBJSplit() { HandleBJSplit(); return FReply::Handled(); }

void UT66GamblerOverlayWidget::SetPage(EGamblerPage Page)
{
	if (!PageSwitcher.IsValid()) return;
	LockedBetAmount = 0; // changing game requires locking a new bet
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
		case EGamblerPage::BlackJack:
			PageIndex = 1;
			CasinoIndex = 3;
			// Reset BJ table so Deal button shows and table is empty
			BJDeck.Reset();
			BJDealerHand.Reset();
			BJPlayerHands.Reset();
			BJCurrentHandIndex = 0;
			BJBetAmount = 0;
			BJDealerHoleRevealed = false;
			BJPhase = EBlackJackPhase::WaitingForAction;
			bBJPlayerBusted = false;
			BJWinAmount = 0;
			RefreshBlackJackCards();
			if (CasinoSwitcher.IsValid()) CasinoSwitcher->Invalidate(EInvalidateWidget::Layout);
			break;
		case EGamblerPage::Lottery:
			PageIndex = 1;
			CasinoIndex = 4;
			if (UWorld* W = GetWorld()) W->GetTimerManager().ClearTimer(LotteryDrawTimerHandle);
			LotterySelected.Reset();
			LotteryDrawn.Reset();
			LotteryRevealedCount = 0;
			break;
		case EGamblerPage::Plinko:
			PageIndex = 1;
			CasinoIndex = 5;
			if (UWorld* W = GetWorld()) W->GetTimerManager().ClearTimer(PlinkoTimerHandle);
			PlinkoBallSlot = 4;
			PlinkoRow = 0;
			break;
		case EGamblerPage::BoxOpening:
			PageIndex = 1;
			CasinoIndex = 6;
			if (UWorld* W = GetWorld()) W->GetTimerManager().ClearTimer(BoxOpeningTimerHandle);
			BoxOpeningCurrentIndex = 0;
			BoxOpeningResultIndex = 0;
			BoxOpeningSpinSteps = 0;
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
	if (CoinFlipStatusText.IsValid()) CoinFlipStatusText->SetText(FText::GetEmpty());
	if (RpsStatusText.IsValid()) RpsStatusText->SetText(FText::GetEmpty());
	if (BlackJackStatusText.IsValid()) BlackJackStatusText->SetText(FText::GetEmpty());
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
	if (Page == EGamblerPage::BlackJack && BlackJackResultText.IsValid()) BlackJackResultText->SetText(Loc2 ? Loc2->GetText_ResultDash() : NSLOCTEXT("T66.Gambler", "ResultDash", "Result: -"));
	if (Page == EGamblerPage::Lottery)
	{
		for (int32 i = 0; i < 10 && i < LotteryNumberBorders.Num(); ++i)
			if (LotteryNumberBorders[i].IsValid()) LotteryNumberBorders[i]->SetBorderBackgroundColor(FT66Style::Tokens::Panel2);
		if (LotteryPicksText.IsValid()) LotteryPicksText->SetText(NSLOCTEXT("T66.Gambler", "YourPicks", "Your picks:"));
		if (LotteryDrawnText.IsValid()) LotteryDrawnText->SetText(NSLOCTEXT("T66.Gambler", "Drawn", "Drawn:"));
		if (LotteryResultText.IsValid()) LotteryResultText->SetText(FText::GetEmpty());
	}
	if (Page == EGamblerPage::Plinko)
	{
		if (PlinkoResultText.IsValid()) PlinkoResultText->SetText(FText::GetEmpty());
		if (PlinkoBoardContainer.IsValid()) PlinkoBoardContainer->Invalidate(EInvalidateWidget::Layout);
	}
	if (Page == EGamblerPage::BoxOpening)
	{
		if (BoxOpeningResultText.IsValid()) BoxOpeningResultText->SetText(FText::GetEmpty());
		if (BoxOpeningStripContainer.IsValid()) BoxOpeningStripContainer->Invalidate(EInvalidateWidget::Layout);
	}
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
	// Anger face sprites: Happy (0-33%), Neutral (34-66%), Angry (67-99%), boss at 100%
	if (AngerCircleImage.IsValid())
	{
		const int32 Pct = FMath::Clamp(FMath::RoundToInt(Anger01 * 100.f), 0, 100);
		if (Pct >= 67)
		{
			AngerCircleImage->SetImage(&AngerFace_Angry);
		}
		else if (Pct >= 34)
		{
			AngerCircleImage->SetImage(&AngerFace_Neutral);
		}
		else
		{
			AngerCircleImage->SetImage(&AngerFace_Happy);
		}
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
	const TArray<FT66InventorySlot>& InvSlots = RunState->GetInventorySlots();

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
			if (bHasData && InvSlots.IsValidIndex(i))
			{
				Fill = FItemData::GetItemRarityColor(InvSlots[i].Rarity);
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
	StatsPanelBox->SetContent(T66StatsPanelSlate::MakeEssentialStatsPanel(RunState, Loc, FT66Style::Tokens::NPCGamblerStatsPanelWidth, true));
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

			FText DescLine;
			if (MainValue > 0)
			{
				if (MainType == ET66HeroStatType::AttackScale && RunState)
				{
					const float ScaleMult = RunState->GetHeroScaleMultiplier();
					DescLine = FText::Format(
						NSLOCTEXT("T66.ItemTooltip", "AttackScaleLineFormat", "{0}: +{1} ({2})"),
						StatLabel(MainType), FText::AsNumber(MainValue), FText::FromString(FString::Printf(TEXT("%.1f"), ScaleMult)));
				}
				else
				{
					DescLine = FText::Format(NSLOCTEXT("T66.ItemTooltip", "MainStatLineFormat", "{0}: +{1}"), StatLabel(MainType), FText::AsNumber(MainValue));
				}
			}
			else
			{
				DescLine = FText::GetEmpty();
			}
			SellItemDescText->SetText(DescLine);
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

bool UT66GamblerOverlayWidget::TryPayWithLockedBet()
{
	if (bInputLocked) return false;
	if (LockedBetAmount <= 0)
	{
		UT66LocalizationSubsystem* Loc2 = nullptr;
		if (UWorld* World = GetWorld())
		{
			if (World->GetGameInstance())
			{
				Loc2 = World->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>();
			}
		}
		SetStatus(Loc2 ? Loc2->GetText_BetFirst() : NSLOCTEXT("T66.Gambler", "BetFirst", "Bet First"), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return false;
	}
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState || !RunState->TrySpendGold(LockedBetAmount))
	{
		UT66LocalizationSubsystem* Loc2 = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
		SetStatus(Loc2 ? Loc2->GetText_NotEnoughGold() : NSLOCTEXT("T66.Gambler", "NotEnoughGold", "Not enough gold."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		RefreshTopBar();
		return false;
	}
	PendingBetAmount = LockedBetAmount;
	LockedBetAmount = 0;
	RefreshTopBar();
	return true;
}

void UT66GamblerOverlayWidget::AwardWin()
{
	AwardWin(PendingBetAmount);
}

void UT66GamblerOverlayWidget::AwardWin(int32 BetAmount)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				const int64 Payout = static_cast<int64>(BetAmount) * 2;
				RunState->AddGold(static_cast<int32>(FMath::Clamp<int64>(Payout, 0, INT32_MAX)));
				RefreshTopBar();
			}
			if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
			{
				Achieve->NotifyGamblerWin();
			}
		}
	}
}

void UT66GamblerOverlayWidget::AwardPayout(int32 BetAmount, float Multiplier)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				const int64 Payout = FMath::RoundToInt64(static_cast<double>(BetAmount) * static_cast<double>(Multiplier));
				RunState->AddGold(static_cast<int32>(FMath::Clamp<int64>(Payout, 0, INT32_MAX)));
				RefreshTopBar();
			}
		}
	}
}

void UT66GamblerOverlayWidget::ResolveCoinFlip(bool bChoseHeads)
{
	if (!TryPayWithLockedBet()) return;
	bInputLocked = true;
	PendingRevealType = ERevealType::CoinFlip;
	bPendingCoinFlipChoseHeads = bChoseHeads;
	ShowCheatPrompt();
}

void UT66GamblerOverlayWidget::ResolveRps(int32 PlayerChoice)
{
	if (!TryPayWithLockedBet()) return;
	bInputLocked = true;
	PendingRevealType = ERevealType::RockPaperScissors;
	PendingRpsPlayerChoice = FMath::Clamp(PlayerChoice, 0, 2);
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
		RunState->AddGamblerAngerFromBet(PendingBetAmount);
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
	float RevealDelay = 0.35f;
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
			// Start coin spin animation
			bCoinSpinResultHeads = bPendingCoinFlipResultHeads;
			bCoinSpinActive = true;
			CoinSpinElapsed = 0.f;
			if (World)
			{
				World->GetTimerManager().ClearTimer(CoinSpinTimerHandle);
				World->GetTimerManager().SetTimer(CoinSpinTimerHandle, this, &UT66GamblerOverlayWidget::TickCoinSpin, 0.033f, true);
			}
			RevealDelay = CoinSpinDuration + 0.15f; // reveal shortly after spin ends
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
		default:
			break;
	}

	if (World)
	{
		World->GetTimerManager().ClearTimer(RevealTimerHandle);
		World->GetTimerManager().SetTimer(RevealTimerHandle, this, &UT66GamblerOverlayWidget::RevealPendingOutcome, RevealDelay, false);
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

	float RevealDelay = 0.35f;
	switch (PendingRevealType)
	{
		case ERevealType::CoinFlip:
		{
			bPendingCoinFlipResultHeads = FMath::RandBool();
			bPendingWin = (bPendingCoinFlipChoseHeads == bPendingCoinFlipResultHeads);
			if (CoinFlipResultText.IsValid()) CoinFlipResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : NSLOCTEXT("T66.Gambler", "Rolling", "Rolling..."));
			// Start coin spin animation
			bCoinSpinResultHeads = bPendingCoinFlipResultHeads;
			bCoinSpinActive = true;
			CoinSpinElapsed = 0.f;
			if (World)
			{
				World->GetTimerManager().ClearTimer(CoinSpinTimerHandle);
				World->GetTimerManager().SetTimer(CoinSpinTimerHandle, this, &UT66GamblerOverlayWidget::TickCoinSpin, 0.033f, true);
			}
			RevealDelay = CoinSpinDuration + 0.15f;
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
		default:
			bPendingWin = false;
			break;
	}

	if (World)
	{
		World->GetTimerManager().ClearTimer(RevealTimerHandle);
		World->GetTimerManager().SetTimer(RevealTimerHandle, this, &UT66GamblerOverlayWidget::RevealPendingOutcome, RevealDelay, false);
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
				? FText::Format(WinFmt, FText::AsNumber(PendingBetAmount))
				: (Loc2 ? Loc2->GetText_Lose() : NSLOCTEXT("T66.Gambler", "Lose", "LOSE"));
			SetStatus(Status,
				bPendingWin ? FLinearColor(0.3f, 1.f, 0.4f, 1.f) : FLinearColor(1.f, 0.3f, 0.3f, 1.f));
			break;
		}
		case ERevealType::RockPaperScissors:
		{
			if (bPendingWin) AwardWin();
			// Show hand images
			if (RpsHandsContainer.IsValid()) RpsHandsContainer->SetVisibility(EVisibility::Visible);
			if (RpsHumanHandImage.IsValid())
			{
				FSlateBrush* HumanBrush = (PendingRpsPlayerChoice == 0) ? &RpsBrush_HumanRock
					: (PendingRpsPlayerChoice == 1) ? &RpsBrush_HumanPaper : &RpsBrush_HumanScissors;
				RpsHumanHandImage->SetImage(HumanBrush);
			}
			if (RpsDemonHandImage.IsValid())
			{
				FSlateBrush* DemonBrush = (PendingRpsOppChoice == 0) ? &RpsBrush_DemonRock
					: (PendingRpsOppChoice == 1) ? &RpsBrush_DemonPaper : &RpsBrush_DemonScissors;
				RpsDemonHandImage->SetImage(DemonBrush);
			}
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
				? FText::Format(WinFmt, FText::AsNumber(PendingBetAmount))
				: (Loc2 ? Loc2->GetText_Lose() : NSLOCTEXT("T66.Gambler", "Lose", "LOSE"));
			SetStatus(Status,
				bPendingWin ? FLinearColor(0.3f, 1.f, 0.4f, 1.f) : FLinearColor(1.f, 0.3f, 0.3f, 1.f));
			break;
		}
		case ERevealType::BlackJack:
			RevealBlackJackOutcome();
			break;
		default:
			break;
	}

	PendingRevealType = ERevealType::None;
	bInputLocked = false;
}

// ── Coin flip spin animation ──────────────────────────────────────────────
void UT66GamblerOverlayWidget::TickCoinSpin()
{
	if (!bCoinSpinActive || !CoinFlipImage.IsValid()) return;
	UWorld* World = GetWorld();
	if (!World) return;

	CoinSpinElapsed += 0.033f; // ~30 Hz
	const float Alpha = FMath::Clamp(CoinSpinElapsed / CoinSpinDuration, 0.f, 1.f);

	// Cycle: Heads → Side → Tails → Side → repeat
	// Speed slows down via ease-out (fast at start, slow at end)
	const float Ease = FMath::InterpEaseOut(0.f, 1.f, Alpha, 3.f);
	// Total half-flips: start with many, decelerate. We do ~12 half-flips total.
	const float TotalHalfFlips = 12.f;
	const float CurrentHalfFlip = TotalHalfFlips * Ease;
	const int32 Phase = FMath::FloorToInt(CurrentHalfFlip * 2.f); // 0..N where each unit = quarter flip

	if (Alpha >= 1.f)
	{
		// Land on final result
		FinishCoinSpin();
		return;
	}

	// Cycling pattern within each half-flip: Heads/Tails → Side → Tails/Heads → Side...
	const int32 Step = Phase % 4;
	switch (Step)
	{
	case 0: CoinFlipImage->SetImage(&CoinBrush_Heads); break;
	case 1: CoinFlipImage->SetImage(&CoinBrush_Side); break;
	case 2: CoinFlipImage->SetImage(&CoinBrush_Tails); break;
	case 3: CoinFlipImage->SetImage(&CoinBrush_Side); break;
	}
}

void UT66GamblerOverlayWidget::FinishCoinSpin()
{
	bCoinSpinActive = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CoinSpinTimerHandle);
	}
	// Show final face
	if (CoinFlipImage.IsValid())
	{
		CoinFlipImage->SetImage(bCoinSpinResultHeads ? &CoinBrush_Heads : &CoinBrush_Tails);
	}
}

// ── BlackJack card helpers ────────────────────────────────────────────────
void UT66GamblerOverlayWidget::InitBJCardBrushes()
{
	BJCardBrushes.SetNum(53); // 0..51 face cards, 52 = back
	static constexpr float CardW = 80.f;
	static constexpr float CardH = 112.f;
	for (int32 i = 0; i < 53; ++i)
	{
		BJCardBrushes[i] = FSlateBrush();
		BJCardBrushes[i].ImageSize = FVector2D(CardW, CardH);
		BJCardBrushes[i].DrawAs = ESlateBrushDrawType::Image;
	}
}

FSlateBrush* UT66GamblerOverlayWidget::GetBJCardBrush(int32 CardIndex)
{
	if (CardIndex >= 0 && CardIndex < 52 && BJCardBrushes.Num() > CardIndex)
	{
		return &BJCardBrushes[CardIndex];
	}
	return GetBJCardBackBrush();
}

FSlateBrush* UT66GamblerOverlayWidget::GetBJCardBackBrush()
{
	if (BJCardBrushes.Num() > 52)
	{
		return &BJCardBrushes[52];
	}
	return &BlackJackCardBackBrush; // legacy fallback
}

void UT66GamblerOverlayWidget::StartBlackJackRound()
{
	BJDeck.SetNum(52);
	for (int32 i = 0; i < 52; ++i) BJDeck[i] = i;
	// Fisher-Yates shuffle
	for (int32 i = 51; i >= 1; --i)
	{
		const int32 j = FMath::RandRange(0, i);
		Swap(BJDeck[i], BJDeck[j]);
	}
	BJDealerHand.Reset();
	BJPlayerHands.Reset();
	BJPlayerHands.AddDefaulted(1);
	BJCurrentHandIndex = 0;
	// BJBetAmount set by OnOpenBlackJack before calling StartBlackJackRound
	BJDealerHoleRevealed = false;
	bBJPlayerBusted = false;
	BJPhase = EBlackJackPhase::WaitingForAction;

	// Deal 2 to player, 2 to dealer (one dealer card hidden)
	auto Draw = [this]() -> int32 { if (BJDeck.Num() == 0) return -1; const int32 c = BJDeck.Pop(); return c; };
	BJPlayerHands[0].Add(Draw());
	BJPlayerHands[0].Add(Draw());
	BJDealerHand.Add(Draw());
	BJDealerHand.Add(Draw());

	RefreshBlackJackCards();
	if (CasinoSwitcher.IsValid()) CasinoSwitcher->Invalidate(EInvalidateWidget::Layout);
}

void UT66GamblerOverlayWidget::RefreshBlackJackCards()
{
	const int32 MaxCards = FMath::Min(11, BlackJackDealerCardImages.Num());
	for (int32 i = 0; i < MaxCards; ++i)
	{
		if (BlackJackDealerCardImages[i].IsValid())
		{
			if (i == 0 && !BJDealerHoleRevealed)
			{
				// Hole card hidden — handled by the overlay visibility lambda
				BlackJackDealerCardImages[i]->SetVisibility(EVisibility::Collapsed);
			}
			else if (i < BJDealerHand.Num())
			{
				BlackJackDealerCardImages[i]->SetImage(GetBJCardBrush(BJDealerHand[i]));
				BlackJackDealerCardImages[i]->SetVisibility(EVisibility::Visible);
			}
			else
			{
				BlackJackDealerCardImages[i]->SetVisibility(EVisibility::Collapsed);
			}
		}
	}
	const TArray<int32>& PlayerHand = (BJPlayerHands.Num() > 0 && BJCurrentHandIndex < BJPlayerHands.Num())
		? BJPlayerHands[BJCurrentHandIndex] : (BJPlayerHands.Num() > 0 ? BJPlayerHands[0] : TArray<int32>());
	const int32 MaxPlayer = FMath::Min(11, BlackJackPlayerCardImages.Num());
	for (int32 i = 0; i < MaxPlayer; ++i)
	{
		if (BlackJackPlayerCardImages[i].IsValid())
		{
			if (i < PlayerHand.Num())
			{
				BlackJackPlayerCardImages[i]->SetImage(GetBJCardBrush(PlayerHand[i]));
				BlackJackPlayerCardImages[i]->SetVisibility(EVisibility::Visible);
			}
			else
			{
				BlackJackPlayerCardImages[i]->SetVisibility(EVisibility::Collapsed);
			}
		}
	}
}

int32 UT66GamblerOverlayWidget::BJHandValue(const TArray<int32>& Hand) const
{
	int32 value = 0;
	int32 aces = 0;
	for (int32 c : Hand)
	{
		if (c < 0) continue;
		const int32 r = c % 13;
		if (r == 0) { aces++; value += 11; }
		else if (r >= 9) value += 10; // 10, J, Q, K
		else value += r + 1;
	}
	while (value > 21 && aces > 0) { value -= 10; aces--; }
	return value;
}

FText UT66GamblerOverlayWidget::BJCardToText(int32 CardIndex) const
{
	if (CardIndex < 0 || CardIndex > 51) return FText::FromString(TEXT("?"));
	const int32 r = CardIndex % 13;
	const int32 s = CardIndex / 13;
	static const TCHAR* Ranks[] = { TEXT("A"), TEXT("2"), TEXT("3"), TEXT("4"), TEXT("5"), TEXT("6"), TEXT("7"), TEXT("8"), TEXT("9"), TEXT("10"), TEXT("J"), TEXT("Q"), TEXT("K") };
	static const TCHAR* Suits[] = { TEXT("\u2660"), TEXT("\u2665"), TEXT("\u2666"), TEXT("\u2663") }; // ♠♥♦♣
	FString str = FString::Printf(TEXT("%s%s"), Ranks[r], Suits[s]);
	return FText::FromString(str);
}

void UT66GamblerOverlayWidget::BJDealerPlay()
{
	BJDealerHoleRevealed = true;
	RefreshBlackJackCards();
	if (CasinoSwitcher.IsValid()) CasinoSwitcher->Invalidate(EInvalidateWidget::Layout);

	while (BJHandValue(BJDealerHand) < 17 && BJDeck.Num() > 0)
	{
		BJDealerHand.Add(BJDeck.Pop());
		RefreshBlackJackCards();
	}
	BJEndRound();
}

void UT66GamblerOverlayWidget::BJEndRound()
{
	BJPhase = EBlackJackPhase::RoundOver;
	const int32 playerVal = BJHandValue(BJPlayerHands[0]);
	const int32 dealerVal = BJHandValue(BJDealerHand);
	const bool playerBust = (playerVal > 21);
	bBJPlayerBusted = playerBust;
	const bool dealerBust = (dealerVal > 21);

	if (playerBust)
	{
		bPendingWin = false;
		bPendingDraw = false;
		BJWinAmount = 0;
	}
	else if (dealerBust)
	{
		bPendingWin = true;
		bPendingDraw = false;
		BJWinAmount = BJBetAmount;
	}
	else if (playerVal > dealerVal)
	{
		bPendingWin = true;
		bPendingDraw = false;
		BJWinAmount = BJBetAmount;
	}
	else if (playerVal < dealerVal)
	{
		bPendingWin = false;
		bPendingDraw = false;
		BJWinAmount = 0;
	}
	else
	{
		bPendingWin = false;
		bPendingDraw = true;
		BJWinAmount = 0;
	}

	PendingRevealType = ERevealType::BlackJack;
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(RevealTimerHandle);
		World->GetTimerManager().SetTimer(RevealTimerHandle, this, &UT66GamblerOverlayWidget::RevealPendingOutcome, 0.5f, false);
	}
}

void UT66GamblerOverlayWidget::RevealBlackJackOutcome()
{
	UT66LocalizationSubsystem* Loc2 = nullptr;
	UT66RunStateSubsystem* RunState = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
			RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		}
	}
	if (bPendingWin)
	{
		AwardWin(BJBetAmount);
		if (BlackJackResultText.IsValid())
			BlackJackResultText->SetText(Loc2 ? Loc2->GetText_Win() : NSLOCTEXT("T66.Gambler", "Win", "WIN"));
		const FText WinFmt = Loc2 ? Loc2->GetText_WinPlusAmountFormat() : NSLOCTEXT("T66.Gambler", "WinPlusAmountFormat", "WIN (+{0})");
		SetStatus(FText::Format(WinFmt, FText::AsNumber(BJBetAmount)), FLinearColor(0.3f, 1.f, 0.4f, 1.f));
	}
	else if (bPendingDraw)
	{
		if (RunState) RunState->AddGold(BJBetAmount);
		RefreshTopBar();
		if (BlackJackResultText.IsValid())
			BlackJackResultText->SetText(Loc2 ? Loc2->GetText_Push() : NSLOCTEXT("T66.Gambler", "Push", "PUSH"));
		SetStatus(Loc2 ? Loc2->GetText_Push() : NSLOCTEXT("T66.Gambler", "Push", "PUSH"), FLinearColor(1.f, 1.f, 0.6f, 1.f));
	}
	else
	{
		const FText LossText = bBJPlayerBusted
			? (Loc2 ? Loc2->GetText_Bust() : NSLOCTEXT("T66.Gambler", "Bust", "BUST"))
			: (Loc2 ? Loc2->GetText_Lose() : NSLOCTEXT("T66.Gambler", "Lose", "LOSE"));
		if (BlackJackResultText.IsValid()) BlackJackResultText->SetText(LossText);
		SetStatus(LossText, FLinearColor(1.f, 0.3f, 0.3f, 1.f));
	}

	// After outcome is shown, wait then animate cards being taken and reset for another round
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(BJCardsTakenTimerHandle);
		World->GetTimerManager().SetTimer(BJCardsTakenTimerHandle, this, &UT66GamblerOverlayWidget::StartBJCardsTakenAnimation, 2.0f, false);
	}
}

void UT66GamblerOverlayWidget::StartBJCardsTakenAnimation()
{
	UWorld* World = GetWorld();
	if (!World) return;
	World->GetTimerManager().ClearTimer(BJCardsTakenTimerHandle);
	BJCardsTakenStep = 0;
	World->GetTimerManager().SetTimer(BJCardsTakenTimerHandle, this, &UT66GamblerOverlayWidget::TickBJCardsTaken, 0.08f, true);
}

void UT66GamblerOverlayWidget::TickBJCardsTaken()
{
	constexpr int32 MaxCards = 11;
	// Steps 0..10: hide dealer cards; steps 11..21: hide player cards
	if (BJCardsTakenStep < MaxCards)
	{
		if (BlackJackDealerCardImages.IsValidIndex(BJCardsTakenStep) && BlackJackDealerCardImages[BJCardsTakenStep].IsValid())
		{
			BlackJackDealerCardImages[BJCardsTakenStep]->SetVisibility(EVisibility::Collapsed);
		}
		// Also hide dealer hole card back on step 0 if still visible
		if (BJCardsTakenStep == 0 && BlackJackDealerHoleCardBackImage.IsValid())
		{
			BlackJackDealerHoleCardBackImage->SetVisibility(EVisibility::Collapsed);
		}
	}
	else if (BJCardsTakenStep < MaxCards + MaxCards)
	{
		const int32 PlayerIdx = BJCardsTakenStep - MaxCards;
		if (BlackJackPlayerCardImages.IsValidIndex(PlayerIdx) && BlackJackPlayerCardImages[PlayerIdx].IsValid())
		{
			BlackJackPlayerCardImages[PlayerIdx]->SetVisibility(EVisibility::Collapsed);
		}
	}

	++BJCardsTakenStep;
	if (BJCardsTakenStep >= MaxCards + MaxCards)
	{
		UWorld* World = GetWorld();
		if (World) World->GetTimerManager().ClearTimer(BJCardsTakenTimerHandle);
		ResetBlackJackForNewRound();
	}
	else if (CasinoSwitcher.IsValid())
	{
		CasinoSwitcher->Invalidate(EInvalidateWidget::Layout);
	}
}

void UT66GamblerOverlayWidget::ResetBlackJackForNewRound()
{
	BJDeck.Reset();
	BJDealerHand.Reset();
	BJPlayerHands.Reset();
	BJCurrentHandIndex = 0;
	BJBetAmount = 0;
	BJDealerHoleRevealed = false;
	BJPhase = EBlackJackPhase::WaitingForAction;
	bBJPlayerBusted = false;
	BJWinAmount = 0;

	UT66LocalizationSubsystem* Loc2 = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (World->GetGameInstance())
		{
			Loc2 = World->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>();
		}
	}
	if (BlackJackResultText.IsValid())
		BlackJackResultText->SetText(Loc2 ? Loc2->GetText_ResultDash() : NSLOCTEXT("T66.Gambler", "ResultDash", "Result: -"));
	if (BlackJackStatusText.IsValid())
		BlackJackStatusText->SetText(FText::GetEmpty());

	RefreshBlackJackCards();
	if (CasinoSwitcher.IsValid()) CasinoSwitcher->Invalidate(EInvalidateWidget::Layout);
}

void UT66GamblerOverlayWidget::HandleBJHit()
{
	if (BJPhase != EBlackJackPhase::WaitingForAction || BJPlayerHands.Num() == 0) return;
	if (BJDeck.Num() == 0) return;
	BJPlayerHands[BJCurrentHandIndex].Add(BJDeck.Pop());
	RefreshBlackJackCards();
	if (CasinoSwitcher.IsValid()) CasinoSwitcher->Invalidate(EInvalidateWidget::Layout);
	if (BJHandValue(BJPlayerHands[BJCurrentHandIndex]) > 21)
	{
		BJPhase = EBlackJackPhase::PlayerBusted;
		bBJPlayerBusted = true;
		bPendingWin = false;
		bPendingDraw = false;
		BJWinAmount = 0;
		PendingRevealType = ERevealType::BlackJack;
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(RevealTimerHandle);
			World->GetTimerManager().SetTimer(RevealTimerHandle, this, &UT66GamblerOverlayWidget::RevealPendingOutcome, 0.5f, false);
		}
	}
}

void UT66GamblerOverlayWidget::HandleBJStand()
{
	if (BJPhase != EBlackJackPhase::WaitingForAction) return;
	BJDealerPlay();
}

void UT66GamblerOverlayWidget::HandleBJDouble()
{
	if (BJPhase != EBlackJackPhase::WaitingForAction || BJPlayerHands.Num() == 0) return;
	if (BJPlayerHands[BJCurrentHandIndex].Num() != 2) return; // only on first two cards
	UT66RunStateSubsystem* RunState = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
			RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
	}
	if (!RunState || !RunState->TrySpendGold(BJBetAmount))
	{
		UWorld* World = GetWorld();
		UT66LocalizationSubsystem* Loc2 = (World && World->GetGameInstance()) ? World->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
		SetStatus(Loc2 ? Loc2->GetText_NotEnoughGold() : NSLOCTEXT("T66.Gambler", "NotEnoughGold", "Not enough gold."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		RefreshTopBar();
		return;
	}
	BJBetAmount += BJBetAmount;
	RefreshTopBar();
	if (BJDeck.Num() > 0) BJPlayerHands[BJCurrentHandIndex].Add(BJDeck.Pop());
	RefreshBlackJackCards();
	if (CasinoSwitcher.IsValid()) CasinoSwitcher->Invalidate(EInvalidateWidget::Layout);
	if (BJHandValue(BJPlayerHands[BJCurrentHandIndex]) > 21)
	{
		BJPhase = EBlackJackPhase::PlayerBusted;
		bBJPlayerBusted = true;
		bPendingWin = false;
		bPendingDraw = false;
		BJWinAmount = 0;
		PendingRevealType = ERevealType::BlackJack;
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(RevealTimerHandle);
			World->GetTimerManager().SetTimer(RevealTimerHandle, this, &UT66GamblerOverlayWidget::RevealPendingOutcome, 0.5f, false);
		}
	}
	else
	{
		BJDealerPlay();
	}
}

void UT66GamblerOverlayWidget::HandleBJSplit()
{
	if (BJPhase != EBlackJackPhase::WaitingForAction || BJPlayerHands.Num() != 1) return;
	if (BJPlayerHands[0].Num() != 2) return;
	const int32 r0 = BJPlayerHands[0][0] % 13;
	const int32 r1 = BJPlayerHands[0][1] % 13;
	if (r0 != r1) return; // same rank only
	if (BJDeck.Num() < 2) return;
	TArray<int32> hand1;
	hand1.Add(BJPlayerHands[0][0]);
	hand1.Add(BJDeck.Pop());
	BJPlayerHands[0].SetNum(1);
	BJPlayerHands[0].Add(BJDeck.Pop());
	BJPlayerHands.Add(hand1);
	BJCurrentHandIndex = 0;
	RefreshBlackJackCards();
	if (CasinoSwitcher.IsValid()) CasinoSwitcher->Invalidate(EInvalidateWidget::Layout);
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
	// When a game is active, show status in that game panel instead of the top bar
	if (CasinoSwitcher.IsValid())
	{
		const int32 Idx = CasinoSwitcher->GetActiveWidgetIndex();
		if (Idx == 1 && CoinFlipStatusText.IsValid())
		{
			CoinFlipStatusText->SetText(Msg);
			CoinFlipStatusText->SetColorAndOpacity(FSlateColor(Color));
			if (StatusText.IsValid()) { StatusText->SetText(FText::GetEmpty()); }
			return;
		}
		if (Idx == 2 && RpsStatusText.IsValid())
		{
			RpsStatusText->SetText(Msg);
			RpsStatusText->SetColorAndOpacity(FSlateColor(Color));
			if (StatusText.IsValid()) { StatusText->SetText(FText::GetEmpty()); }
			return;
		}
		if (Idx == 3 && BlackJackStatusText.IsValid())
		{
			BlackJackStatusText->SetText(Msg);
			BlackJackStatusText->SetColorAndOpacity(FSlateColor(Color));
			if (StatusText.IsValid()) { StatusText->SetText(FText::GetEmpty()); }
			return;
		}
	}
	if (StatusText.IsValid())
	{
		StatusText->SetText(Msg);
		StatusText->SetColorAndOpacity(FSlateColor(Color));
	}
}

