// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66GamblerOverlayWidget.h"
#include "UI/T66StatsPanelSlate.h"
#include "UI/T66SlateTextureHelpers.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "UI/T66ItemCardTextUtils.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Images/SImage.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "UI/Style/T66AnimatedStyle.h"
#include "UI/Style/T66FlatStyle.h"
#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Gameplay/T66PlayerController.h"
#include "Engine/Texture2D.h"

static FString MakeGamblerBuildInventoryStackKey(const FT66InventorySlot& Slot)
{
	return FString::Printf(TEXT("%s|%d"), *Slot.ItemTemplateID.ToString(), static_cast<int32>(Slot.Rarity));
}

static FText BuildGamblerBuildWagerText(const int32 WagerAmount)
{
	return WagerAmount > 0
		? FText::Format(NSLOCTEXT("T66.Gambler", "WagerFormat", "Wager: {0}"), FText::AsNumber(WagerAmount))
		: FText::GetEmpty();
}

template <typename TNpcType>
static TNpcType* GetRegisteredGamblerBuildNpc(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
	{
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNpc : Registry->GetNPCs())
		{
			if (TNpcType* Npc = Cast<TNpcType>(WeakNpc.Get()))
			{
				return Npc;
			}
		}
	}

	return nullptr;
}

static bool HasRegisteredGamblerBuildBoss(UWorld* World)
{
	if (!World)
	{
		return false;
	}

	if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
	{
		for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
		{
			if (Cast<AT66GamblerBoss>(WeakBoss.Get()) != nullptr)
			{
				return true;
			}
		}
	}

	return false;
}

namespace
{
	void AddGamblerCanvasSlot(
		const TSharedRef<SConstraintCanvas>& Canvas,
		const float X,
		const float Y,
		const float W,
		const float H,
		const TSharedRef<SWidget>& Widget)
	{
		const float UiScale = FMath::Max(0.1f, FT66FlatStyle::GetGlobalUIScale());
		Canvas->AddSlot()
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X / UiScale, Y / UiScale, W / UiScale, H / UiScale))
		[
			Widget
		];
	}

	enum class EGamblerFlatPanel : uint8
	{
		OverlayModalPanel,
		CasinoShellPanel,
		ContentPanelWide,
		ContentPanelTall,
		InnerPanel,
		HeaderSummaryBar,
		CrateStripFrame,
		SlotNormal,
		SlotHover,
		SlotSelected,
		SlotDisabled,
		OfferCardNormal,
		OfferCardHover,
		OfferCardSelected,
		OfferCardDisabled,
		CrateWinnerMarker,
	};

	enum class EGamblerFlatButton : uint8
	{
		Neutral,
		Primary,
		Danger,
		Tab,
		Select,
		DuoLeft,
		DuoRight,
		Central,
		DropdownOption,
		BorderlessIcon,
	};

	struct FGamblerFlatButtonParams
	{
		FText Label;
		FOnClicked OnClicked;
		EGamblerFlatButton Family = EGamblerFlatButton::Neutral;
		float MinWidth = 120.f;
		float MinHeight = 44.f;
		int32 FontSize = 14;
		FMargin Padding = FMargin(12.f, 5.f);
		TAttribute<bool> IsEnabled = true;
		TAttribute<bool> IsSelected = false;
		TSharedPtr<SWidget> CustomContent;
		FName Tag = NAME_None;
		FName ToggleGroup = NAME_None;

		FGamblerFlatButtonParams() = default;
		FGamblerFlatButtonParams(const FText& InLabel, FOnClicked InOnClicked, const EGamblerFlatButton InFamily = EGamblerFlatButton::Neutral)
			: Label(InLabel)
			, OnClicked(MoveTemp(InOnClicked))
			, Family(InFamily)
		{
		}

		FGamblerFlatButtonParams& SetMinWidth(const float InMinWidth) { MinWidth = InMinWidth; return *this; }
		FGamblerFlatButtonParams& SetMinHeight(const float InMinHeight) { MinHeight = InMinHeight; return *this; }
		FGamblerFlatButtonParams& SetFontSize(const int32 InFontSize) { FontSize = InFontSize; return *this; }
		FGamblerFlatButtonParams& SetPadding(const FMargin& InPadding) { Padding = InPadding; return *this; }
		FGamblerFlatButtonParams& SetEnabled(const TAttribute<bool>& InIsEnabled) { IsEnabled = InIsEnabled; return *this; }
		FGamblerFlatButtonParams& SetSelected(const TAttribute<bool>& InIsSelected) { IsSelected = InIsSelected; return *this; }
		FGamblerFlatButtonParams& SetContent(const TSharedRef<SWidget>& InContent) { CustomContent = InContent; return *this; }
		FGamblerFlatButtonParams& SetTag(const FName InTag) { Tag = InTag; return *this; }
		FGamblerFlatButtonParams& SetToggleGroup(const FName InToggleGroup) { ToggleGroup = InToggleGroup; return *this; }
	};

	static ET66FlatState StateForGamblerPanel(const EGamblerFlatPanel Panel)
	{
		switch (Panel)
		{
		case EGamblerFlatPanel::SlotSelected:
		case EGamblerFlatPanel::OfferCardSelected:
		case EGamblerFlatPanel::CrateWinnerMarker:
			return ET66FlatState::Selected;
		case EGamblerFlatPanel::SlotDisabled:
		case EGamblerFlatPanel::OfferCardDisabled:
			return ET66FlatState::Disabled;
		case EGamblerFlatPanel::SlotHover:
		case EGamblerFlatPanel::OfferCardHover:
			return ET66FlatState::Ready;
		default:
			return ET66FlatState::Default;
		}
	}

	static ET66FlatState StateForGamblerButton(const EGamblerFlatButton Family, const bool bSelected, const bool bEnabled)
	{
		if (!bEnabled)
		{
			return ET66FlatState::Disabled;
		}
		if (bSelected || Family == EGamblerFlatButton::Danger)
		{
			return ET66FlatState::Selected;
		}
		if (Family == EGamblerFlatButton::Primary
			|| Family == EGamblerFlatButton::Central
			|| Family == EGamblerFlatButton::Select)
		{
			return ET66FlatState::Ready;
		}
		return ET66FlatState::Default;
	}

	static FGamblerFlatButtonParams MakeGamblerFlatButtonParams(
		const FText& Label,
		FOnClicked OnClicked,
		const EGamblerFlatButton Family = EGamblerFlatButton::Neutral)
	{
		return FGamblerFlatButtonParams(Label, MoveTemp(OnClicked), Family);
	}

	static TSharedRef<SWidget> MakeGamblerFlatPanel(
		const TSharedRef<SWidget>& Content,
		const EGamblerFlatPanel Panel = EGamblerFlatPanel::ContentPanelWide,
		const FMargin& Padding = FMargin(16.f),
		TSharedPtr<SBorder>* OutBorder = nullptr,
		const FName Tag = NAME_None)
	{
		return FT66FlatStyle::MakeFlatPanel(StateForGamblerPanel(Panel), Padding, Content, OutBorder, Tag);
	}

	static TSharedRef<SWidget> MakeGamblerFlatButton(const FGamblerFlatButtonParams& Params)
	{
		const ET66FlatState State = StateForGamblerButton(
			Params.Family,
			Params.IsSelected.Get(false),
			Params.IsEnabled.Get(true));

		if (Params.CustomContent.IsValid())
		{
			return FT66FlatStyle::MakeFlatToggleGroupButton(
				State,
				Params.CustomContent.ToSharedRef(),
				Params.OnClicked,
				Params.Padding,
				Params.MinWidth,
				Params.MinHeight,
				Params.IsEnabled,
				Params.Tag,
				Params.ToggleGroup);
		}

		return FT66FlatStyle::MakeFlatButton(
			State,
			Params.Label,
			Params.OnClicked,
			nullptr,
			nullptr,
			Params.Padding,
			Params.MinWidth,
			Params.MinHeight,
			Params.IsEnabled,
			Params.FontSize,
			Params.Tag,
			Params.ToggleGroup);
	}

	static int32 T66BuildGamblerBuildNumberMask(const TSet<int32>& Numbers)
	{
		int32 Mask = 0;
		for (const int32 Number : Numbers)
		{
			if (Number >= 1 && Number <= 10)
			{
				Mask |= (1 << (Number - 1));
			}
		}
		return Mask;
	}

	static int32 T66BuildGamblerBuildNumberMask(const TArray<int32>& Numbers)
	{
		int32 Mask = 0;
		for (const int32 Number : Numbers)
		{
			if (Number >= 1 && Number <= 10)
			{
				Mask |= (1 << (Number - 1));
			}
		}
		return Mask;
	}

	static int32 T66GetGamblerBuildPlinkoPayoutTierFromSlot(const int32 SlotIndex)
	{
		static const int32 Tiers[9] = { 4, 3, 2, 1, 0, 1, 2, 3, 4 };
		return Tiers[FMath::Clamp(SlotIndex, 0, 8)];
	}

	static ET66Rarity T66GamblerBuildBoxOpeningIndexToRarity(const int32 ColorIndex)
	{
		switch (ColorIndex)
		{
		case 1: return ET66Rarity::Red;
		case 2: return ET66Rarity::Yellow;
		case 3: return ET66Rarity::White;
		default: return ET66Rarity::Black;
		}
	}
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
	InventorySlotCountTexts.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
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
			if (RunState)
			{
				RunState->BuybackChanged.RemoveDynamic(this, &UT66GamblerOverlayWidget::HandleBuybackChanged);
				RunState->BossChanged.RemoveDynamic(this, &UT66GamblerOverlayWidget::HandleBossChanged);
				RunState->BuybackChanged.AddDynamic(this, &UT66GamblerOverlayWidget::HandleBuybackChanged);
				RunState->BossChanged.AddDynamic(this, &UT66GamblerOverlayWidget::HandleBossChanged);
				bCachedBossActive = RunState->GetBossActive();
			}
		}
	}
	LiveStatsPanel = MakeShared<T66StatsPanelSlate::FT66LiveStatsPanel>();

	const FTextBlockStyle& TextTitle = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Title"));
	const FTextBlockStyle& TextHeading = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading"));
	const FTextBlockStyle& TextBody = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body"));
	const FTextBlockStyle& TextChip = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Chip"));
	const bool bCompactCasinoLayout = bEmbeddedInCasinoShell;
	const float StatsPanelWidth = bCompactCasinoLayout ? 240.f : FT66FlatStyle::Tokens::NPCGamblerStatsPanelWidth;
	const float RightPanelWidth = bCompactCasinoLayout ? 275.f : FT66FlatStyle::Tokens::NPCRightPanelWidth;
	const float MainRowHeight = bCompactCasinoLayout ? 380.f : FT66FlatStyle::Tokens::NPCMainRowHeight;
	const float CenterPanelWidth = bCompactCasinoLayout ? 812.f : 0.f;
	const float InventorySlotSize = bCompactCasinoLayout ? 80.f : FT66FlatStyle::Tokens::InventorySlotSize;
	const float GameCardSize = bCompactCasinoLayout ? FT66FlatStyle::Tokens::NPCCompactShopCardWidth : FT66FlatStyle::Tokens::NPCShopCardWidth;
	const float GameCardPadding = bCompactCasinoLayout ? 5.f : FT66FlatStyle::Tokens::Space4;
	const float GameCardIconSize = bCompactCasinoLayout ? GameCardSize - GameCardPadding * 2.f : 260.f;
	const float GameCardTotalHeight = bCompactCasinoLayout ? FT66FlatStyle::Tokens::NPCCompactShopCardHeight : 420.f;
	const float BuybackCardSize = bCompactCasinoLayout ? GameCardSize : FT66FlatStyle::Tokens::NPCShopCardWidth;
	const float BuybackCardHeight = bCompactCasinoLayout ? 264.f : FT66FlatStyle::Tokens::NPCShopCardHeight;
	const float BuybackIconSize = bCompactCasinoLayout ? BuybackCardSize - GameCardPadding * 2.f : BuybackCardSize - FT66FlatStyle::Tokens::Space4 * 2.f;
	const float GamblerAngerCircleSize = bCompactCasinoLayout ? 136.f : 260.f;
	const float BankSpinBoxWidth = bCompactCasinoLayout ? 68.f : FT66FlatStyle::Tokens::NPCBankSpinBoxWidth;
	const float BankSpinBoxHeight = bCompactCasinoLayout ? 28.f : FT66FlatStyle::Tokens::NPCBankSpinBoxHeight;
	const int32 StatsPanelFontAdjustment = bCompactCasinoLayout ? -5 : 0;
	const int32 SectionHeadingFontSize = bCompactCasinoLayout ? 10 : 16;
	const int32 CardHeadingFontSize = bCompactCasinoLayout ? 9 : 16;
	const int32 CardBodyFontSize = bCompactCasinoLayout ? 8 : 12;
	const int32 CardButtonFontSize = bCompactCasinoLayout ? 9 : 14;
	const int32 InventoryCountFontSize = bCompactCasinoLayout ? 8 : 14;
	const int32 InventoryDashFontSize = bCompactCasinoLayout ? 10 : 16;
	const int32 StatusFontSize = bCompactCasinoLayout ? 8 : 12;
	const int32 SpinBoxFontSize = bCompactCasinoLayout ? 10 : 16;
	const FMargin CardButtonPadding = bCompactCasinoLayout ? FMargin(5.f, 3.f) : FMargin(8.f, 6.f);
	const FMargin ActionButtonPadding = bCompactCasinoLayout ? FMargin(8.f, 5.f) : FMargin(16.f, 10.f);

	// --- NPC anger face sprites ---
	const float AngerFaceSize = bCompactCasinoLayout ? 85.f : FT66FlatStyle::Tokens::NPCAngerCircleSize;
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
		const TSoftObjectPtr<UTexture2D> CardBackSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/BJ/card_back.card_back")));
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

	// Inventory slot icon brushes — same as Shop (160×160)
	const float InvSlotSz = InventorySlotSize;
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
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text);
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
			.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			MakeGamblerFlatButton(MakeGamblerFlatButtonParams(
				Loc ? Loc->GetText_LetMeGamble() : FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnDialogueGamble),
				EGamblerFlatButton::Primary)
				.SetMinWidth(420.f)
				.SetPadding(FMargin(18.f, 10.f)))
		];

	// --- Casino layout: left content switcher + right panel + bottom inventory ---
	const FText CasinoTitle = Loc ? Loc->GetText_Casino() : NSLOCTEXT("T66.Gambler", "Casino", "CASINO");
	const FText BuybackTitle = NSLOCTEXT("T66.Shop", "Buyback", "BUYBACK");
	const FText RerollText = Loc ? Loc->GetText_Reroll() : NSLOCTEXT("T66.Shop", "Reroll", "REROLL");
	const FText BankTitle = Loc ? Loc->GetText_Bank() : NSLOCTEXT("T66.Shop", "BankTitle", "BANK");
	const FText InventoryTitle = Loc ? Loc->GetText_YourItems() : NSLOCTEXT("T66.Shop", "InventoryTitle", "INVENTORY");
	const FText GamesTitle = NSLOCTEXT("T66.Gambler", "Games", "GAMES");

	const FName GamblerModeToggleGroup(TEXT("Gambler.ModeSelection"));

	// Right panel: gambler portrait + bank. Embedded casino layout uses the smaller shop footprint.
	TSharedRef<SWidget> RightPanel =
		MakeGamblerFlatPanel(
			SNew(SVerticalBox)
			// Gambler portrait (top) — larger than Shop for prominence
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 14.f)
			[
				MakeGamblerFlatPanel(
					SNew(SBox)
					.WidthOverride(GamblerAngerCircleSize)
					.HeightOverride(GamblerAngerCircleSize)
					[
						SAssignNew(AngerCircleImage, SImage)
						.Image(&AngerFace_Happy)
					],
					EGamblerFlatPanel::InnerPanel,
					FMargin(0.f),
					nullptr,
					FName(TEXT("Gambler.PortraitPanel")))
			]
			// Spacer to push Bank to bottom of panel
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SSpacer)
			]
			// Bank (bottom, separate panel) - same wrapper as Shop
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 0.f)
			[
				MakeGamblerFlatPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, FT66FlatStyle::Tokens::Space4)
					[
						FT66FlatStyle::AttachMetadata(SNew(STextBlock)
						.Text(BankTitle)
						.TextStyle(&TextHeading)
						.Font(FT66FlatStyle::Tokens::FontBold(SectionHeadingFontSize))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text),
						FName(TEXT("Gambler.Bank.Title")),
						TEXT("Label.Section"),
						ET66FlatState::Default,
						TOptional<FLinearColor>(),
						false,
						NAME_None,
						true)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						FT66FlatStyle::AttachMetadata(SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Gambler", "BorrowAmountLabel", "BORROW AMOUNT"))
						.TextStyle(&TextChip)
						.Font(FT66FlatStyle::Tokens::FontBold(FMath::Max(8, CardButtonFontSize - 1)))
						.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted),
						FName(TEXT("Gambler.Bank.BorrowLabel")),
						TEXT("Label.Caption"),
						ET66FlatState::Default,
						TOptional<FLinearColor>(),
						false,
						NAME_None,
						true)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(BankSpinBoxWidth)
							.HeightOverride(BankSpinBoxHeight)
							[
								SAssignNew(BorrowAmountSpin, SSpinBox<int32>)
								.MinValue(0).MaxValue(999999).Delta(10)
								.Font(FT66FlatStyle::Tokens::FontBold(SpinBoxFontSize))
								.Value(BorrowAmount)
								.OnValueChanged_Lambda([this](int32 V)
								{
									int32 MaxBorrow = TNumericLimits<int32>::Max();
									if (UWorld* World = GetWorld())
									{
										if (UGameInstance* GI = World->GetGameInstance())
										{
											if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
											{
												MaxBorrow = RunState->GetRemainingBorrowCapacity();
											}
										}
									}
									BorrowAmount = FMath::Clamp(V, 0, FMath::Max(0, MaxBorrow));
								})
							]
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							MakeGamblerFlatButton(MakeGamblerFlatButtonParams(
								NSLOCTEXT("T66.Gambler", "Borrow_Button_Upper", "BORROW"),
								FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBorrowClicked),
								EGamblerFlatButton::Neutral)
								.SetMinWidth(0.f)
								.SetPadding(ActionButtonPadding)
								.SetFontSize(CardButtonFontSize)
								.SetTag(FName(TEXT("Gambler.Bank.BorrowButton"))))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						FT66FlatStyle::AttachMetadata(SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Gambler", "PaybackAmountLabel", "PAYBACK AMOUNT"))
						.TextStyle(&TextChip)
						.Font(FT66FlatStyle::Tokens::FontBold(FMath::Max(8, CardButtonFontSize - 1)))
						.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted),
						FName(TEXT("Gambler.Bank.PaybackLabel")),
						TEXT("Label.Caption"),
						ET66FlatState::Default,
						TOptional<FLinearColor>(),
						false,
						NAME_None,
						true)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(BankSpinBoxWidth)
							.HeightOverride(BankSpinBoxHeight)
							[
								SAssignNew(PaybackAmountSpin, SSpinBox<int32>)
								.MinValue(0).MaxValue(999999).Delta(10)
								.Font(FT66FlatStyle::Tokens::FontBold(SpinBoxFontSize))
								.Value(PaybackAmount)
								.OnValueChanged_Lambda([this](int32 V) { PaybackAmount = FMath::Max(0, V); })
							]
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							MakeGamblerFlatButton(MakeGamblerFlatButtonParams(
								NSLOCTEXT("T66.Gambler", "Payback_Button_Upper", "PAYBACK"),
								FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnPaybackClicked),
								EGamblerFlatButton::Neutral)
								.SetMinWidth(0.f)
								.SetPadding(ActionButtonPadding)
								.SetFontSize(CardButtonFontSize)
								.SetTag(FName(TEXT("Gambler.Bank.PaybackButton"))))
						]
					]
				,
					EGamblerFlatPanel::InnerPanel,
					FMargin(bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space5),
					nullptr,
					FName(TEXT("Gambler.BankPanel")))
			]
		,
			EGamblerFlatPanel::ContentPanelTall,
			FMargin(bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space6),
			nullptr,
			FName(TEXT("Gambler.RightPanel")));

	// Game card: title, content well, and red Play action; compact embedded layout mirrors the reference.
	auto MakeGameCard = [&](const int32 CardIndex, const FText& TitleText, const FOnClicked& OnClicked, const FSlateBrush* IconBrush) -> TSharedRef<SWidget>
	{
		const FString CardPrefix = FString::Printf(TEXT("Gambler.GameCard.%02d"), CardIndex);
		const FName CardPanelTag(*FString::Printf(TEXT("%s.Panel"), *CardPrefix));
		const FName CardTitleTag(*FString::Printf(TEXT("%s.Title"), *CardPrefix));
		const FName CardIconTag(*FString::Printf(TEXT("%s.IconWell"), *CardPrefix));
		const FName CardPlayTag(*FString::Printf(TEXT("%s.PlayButton"), *CardPrefix));
		const FText PlayText = NSLOCTEXT("T66.Gambler", "Play", "PLAY");
		TSharedRef<SWidget> PlayBtn = MakeGamblerFlatButton(
			MakeGamblerFlatButtonParams(PlayText, OnClicked, EGamblerFlatButton::Danger)
			.SetMinWidth(bCompactCasinoLayout ? 0.f : 100.f)
			.SetPadding(CardButtonPadding)
			.SetFontSize(CardButtonFontSize)
			.SetTag(CardPlayTag));
		return SNew(SBox)
			.WidthOverride(GameCardSize)
			.HeightOverride(GameCardTotalHeight)
			.Padding(FMargin(GameCardPadding, 0.f))
			[
				MakeGamblerFlatPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, FT66FlatStyle::Tokens::Space3)
					[
						FT66FlatStyle::AttachMetadata(SNew(STextBlock)
						.Text(TitleText)
						.TextStyle(&TextHeading)
						.Font(FT66FlatStyle::Tokens::FontBold(CardHeadingFontSize))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
						.AutoWrapText(true)
						.WrapTextAt(GameCardSize - GameCardPadding * 2.f),
						CardTitleTag,
						TEXT("Label.CardTitle"),
						ET66FlatState::Default,
						TOptional<FLinearColor>(),
						false,
						NAME_None,
						true)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)
						[
							MakeGamblerFlatPanel(
								SNew(SBox)
								.WidthOverride(GameCardIconSize)
								.HeightOverride(GameCardIconSize)
								[
									FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(
										SNew(SImage)
										.Image(IconBrush)
										.ColorAndOpacity(FLinearColor::White)),
										CardIconTag,
										TEXT("Icon"))
								],
								EGamblerFlatPanel::SlotNormal,
								FMargin(0.f),
								nullptr,
								CardIconTag)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66FlatStyle::Tokens::Space3, 0.f, 0.f)
					[
						PlayBtn
					]
				,
					EGamblerFlatPanel::OfferCardNormal,
					FMargin(GameCardPadding),
					nullptr,
					CardPanelTag)
			];
	};

	const FText MoreGamesText = NSLOCTEXT("T66.Gambler", "MoreGames", "MORE GAMES");
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
				.SlotPadding(bCompactCasinoLayout ? FMargin(8.f, 6.f) : FMargin(16.f, 8.f))
				+ SUniformGridPanel::Slot(0, 0) [ MakeGameCard(1, Loc ? Loc->GetText_RockPaperScissors() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenRps), &GameIcon_Rps) ]
				+ SUniformGridPanel::Slot(1, 0) [ MakeGameCard(2, Loc ? Loc->GetText_BlackJack() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenBlackJack), &GameIcon_BlackJack) ]
				+ SUniformGridPanel::Slot(2, 0) [ MakeGameCard(3, Loc ? Loc->GetText_CoinFlip() : FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenCoinFlip), &GameIcon_CoinFlip) ]
				+ SUniformGridPanel::Slot(3, 0) [ bCompactCasinoLayout ? MakeGameCard(4, LotteryText, FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenLottery), &GameIcon_Lottery) : SNullWidget::NullWidget ]
				+ SUniformGridPanel::Slot(4, 0) [ bCompactCasinoLayout ? MakeGameCard(5, PlinkoText, FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenPlinko), &GameIcon_Plinko) : SNullWidget::NullWidget ]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, bCompactCasinoLayout ? 10.f : 28.f, 0.f, 0.f)
		[
				MakeGamblerFlatButton(
				MakeGamblerFlatButtonParams(MoreGamesText, FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnMoreGamesClicked), EGamblerFlatButton::Neutral)
				.SetMinWidth(0.f)
				.SetPadding(bCompactCasinoLayout ? FMargin(10.f, 6.f) : FMargin(20.f, 12.f))
				.SetFontSize(bCompactCasinoLayout ? CardButtonFontSize : 16)
				.SetTag(FName(TEXT("Gambler.MoreGamesButton"))))
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
				.SlotPadding(bCompactCasinoLayout ? FMargin(8.f, 6.f) : FMargin(16.f, 8.f))
				+ SUniformGridPanel::Slot(0, 0) [ MakeGameCard(6, BoxOpeningText, FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenBoxOpening), &GameIcon_BoxOpening) ]
				+ SUniformGridPanel::Slot(1, 0) [ bCompactCasinoLayout ? SNullWidget::NullWidget : MakeGameCard(7, LotteryText, FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenLottery), &GameIcon_Lottery) ]
				+ SUniformGridPanel::Slot(2, 0) [ bCompactCasinoLayout ? SNullWidget::NullWidget : MakeGameCard(8, PlinkoText, FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnOpenPlinko), &GameIcon_Plinko) ]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space6, 0.f, 0.f)
		[
			MakeGamblerFlatButton(
				MakeGamblerFlatButtonParams(BackToGamesText, FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBackToMainGames), EGamblerFlatButton::Neutral)
				.SetMinWidth(0.f)
				.SetPadding(bCompactCasinoLayout ? FMargin(10.f, 6.f) : FMargin(20.f, 12.f))
				.SetFontSize(bCompactCasinoLayout ? CardButtonFontSize : 16))
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
				FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
					Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Gambler", "Back", "BACK"),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnGameBackToSelection),
					ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f)))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SAssignNew(CoinFlipStatusText, STextBlock)
				.Text(FText::GetEmpty())
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SAssignNew(CoinFlipWagerText, STextBlock)
				.Text(FText::GetEmpty())
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_CoinFlip() : FText::GetEmpty())
			.TextStyle(&TextHeading)
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_ChooseHeadsOrTails() : FText::GetEmpty())
			.TextStyle(&TextBody)
			.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(0.f, 0.f, 0.f, 16.f)
		[
			SNew(SBox)
			.HeightOverride(220.f)
			[
				FT66FlatStyle::MakePanel(
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
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
					],
					FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66FlatStyle::Tokens::Space6))
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
					Loc ? Loc->GetText_Heads() : FText::GetEmpty(),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCoinFlipHeads),
					ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(18.f, 10.f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
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
				FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
					Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Gambler", "Back", "BACK"),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnGameBackToSelection),
					ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f)))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SAssignNew(RpsStatusText, STextBlock)
				.Text(FText::GetEmpty())
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SAssignNew(RpsWagerText, STextBlock)
				.Text(FText::GetEmpty())
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_RockPaperScissors() : FText::GetEmpty())
			.TextStyle(&TextHeading)
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(0.f, 0.f, 0.f, 16.f)
		[
			SNew(SBox)
			.HeightOverride(220.f)
			[
				FT66FlatStyle::MakePanel(
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
								.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
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
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
					],
					FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66FlatStyle::Tokens::Space6))
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
					Loc ? Loc->GetText_Rock() : FText::GetEmpty(),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnRpsRock),
					ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(18.f, 10.f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
					Loc ? Loc->GetText_Paper() : FText::GetEmpty(),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnRpsPaper),
					ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(18.f, 10.f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
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
				SAssignNew(BlackJackDealerValueText, STextBlock)
				.Text(FText::FromString(TEXT("-")))
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
			[ SNew(STextBlock).Text(Loc ? Loc->GetText_Dealer() : FText::GetEmpty()).TextStyle(&TextBody).ColorAndOpacity(FT66FlatStyle::Tokens::Text) ]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SAssignNew(BlackJackDealerHoleCardBackBox, SBox)
					.WidthOverride(BJCardW)
					.HeightOverride(BJCardH)
					.Visibility(EVisibility::Collapsed)
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
				FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
					Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Gambler", "Back", "BACK"),
					FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnGameBackToSelection),
					ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f)))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SAssignNew(BlackJackStatusText, STextBlock)
				.Text(FText::GetEmpty())
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SAssignNew(BlackJackWagerText, STextBlock)
				.Text(FText::GetEmpty())
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_BlackJack() : FText::GetEmpty())
			.TextStyle(&TextHeading)
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
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
				SAssignNew(BlackJackPlayerValueText, STextBlock)
				.Text(FText::FromString(TEXT("-")))
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
				[ SNew(STextBlock).Text(Loc ? Loc->GetText_You() : FText::GetEmpty()).TextStyle(&TextBody).ColorAndOpacity(FT66FlatStyle::Tokens::Text) ]
				+ SHorizontalBox::Slot().AutoWidth()[ BlackJackPlayerCardsRow ]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 16.f, 0.f, 12.f)
		[
			SNew(SBox).HeightOverride(FT66FlatStyle::Tokens::NPCBankSpinBoxHeight)
			[
				SAssignNew(BlackJackResultText, STextBlock)
				.Text(Loc ? Loc->GetText_ResultDash() : FText::GetEmpty())
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f)
			[
				SAssignNew(BlackJackDealButtonBox, SBox)
				.Visibility(EVisibility::Visible)
				[
					FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
						NSLOCTEXT("T66.Gambler", "Deal", "Deal"),
						FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBJDealClicked),
						ET66ButtonType::Primary).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f)))
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f)
			[
				SAssignNew(BlackJackHitButtonBox, SBox)
				.Visibility(EVisibility::Collapsed)
				[
					FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
						Loc ? Loc->GetText_Hit() : FText::GetEmpty(),
						FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBJHit),
						ET66ButtonType::Primary).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f)))
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f)
			[
				SAssignNew(BlackJackStandButtonBox, SBox)
				.Visibility(EVisibility::Collapsed)
				[
					FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
						Loc ? Loc->GetText_Stand() : FText::GetEmpty(),
						FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBJStand),
						ET66ButtonType::Primary).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f)))
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f)
			[
				SAssignNew(BlackJackDoubleButtonBox, SBox)
				.Visibility(EVisibility::Collapsed)
				[
					FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
						Loc ? Loc->GetText_Double() : FText::GetEmpty(),
						FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBJDouble),
						ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f)))
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f)
			[
				SAssignNew(BlackJackSplitButtonBox, SBox)
				.Visibility(EVisibility::Collapsed)
				[
					FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
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
				FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
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
			.Padding(i > 0 ? FMargin(FT66FlatStyle::Tokens::Space2, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			FT66FlatStyle::MakePanel(
				FT66FlatStyle::MakeButton(
					FT66FlatStyle::MakeInRunButtonParams(FText::AsNumber(Num), FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnLotteryNumberClicked, Num), ET66ButtonType::Neutral)
					.SetMinWidth(0.f).SetPadding(FMargin(10.f, 8.f))),
				FT66PanelParams(ET66PanelType::Panel2).SetPadding(0.f),
				&LotteryNumberBorders[i])
		];
	}

	TSharedRef<SWidget> LotteryView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) [ MakeGameBackRow() ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f)
			[ SNew(STextBlock).Text(LotteryText).TextStyle(&TextTitle).ColorAndOpacity(FT66FlatStyle::Tokens::Text) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 8.f)
			[ SNew(STextBlock).Text(LotteryDesc).TextStyle(&TextBody).ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted).AutoWrapText(true) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f) [ LotteryNumberRow ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 2.f)
			[ SAssignNew(LotteryPicksText, STextBlock).Text(YourPicksText).TextStyle(&TextBody).ColorAndOpacity(FT66FlatStyle::Tokens::Text) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 2.f)
			[ SAssignNew(LotteryDrawnText, STextBlock).Text(DrawnText).TextStyle(&TextBody).ColorAndOpacity(FT66FlatStyle::Tokens::Text) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f)
			[ SAssignNew(LotteryResultText, STextBlock).Text(FText::GetEmpty()).TextStyle(&TextHeading).ColorAndOpacity(FT66FlatStyle::Tokens::Text) ];

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

	TSharedRef<SWidget> PlinkoBall =
		SAssignNew(PlinkoBallWidget, SBorder)
		.BorderBackgroundColor(FLinearColor(1.f, 0.85f, 0.2f, 1.f))
		.Padding(0.f)
		[
			SNew(SBox).WidthOverride(PlinkoBallSize).HeightOverride(PlinkoBallSize)[ SNew(SSpacer) ]
		];

	TSharedRef<SWidget> PlinkoView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) [ MakeGameBackRow() ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f)
			[ SNew(STextBlock).Text(PlinkoText).TextStyle(&TextTitle).ColorAndOpacity(FT66FlatStyle::Tokens::Text) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 4.f)
			[ SNew(STextBlock).Text(PlinkoSlotsDesc).TextStyle(&TextBody).ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted).AutoWrapText(true) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f)
			[
				SAssignNew(PlinkoBoardContainer, SBox)
				.WidthOverride(PlinkoBoardW)
				.HeightOverride(PlinkoBoardH)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()[ FT66FlatStyle::MakePanel(PinGrid, FT66PanelParams(ET66PanelType::Panel2).SetPadding(4.f)) ]
					+ SOverlay::Slot()
						.HAlign(HAlign_Left).VAlign(VAlign_Top)
						[ PlinkoBall ]
				]
			]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f)
			[ SAssignNew(PlinkoResultText, STextBlock).Text(FText::GetEmpty()).TextStyle(&TextHeading).ColorAndOpacity(FT66FlatStyle::Tokens::Text) ];

	// Box Opening: 5 colored squares; middle one is the result when spin stops
	const FText BoxOpeningDesc = NSLOCTEXT("T66.Gambler", "BoxOpeningDesc", "Bet then spin. Colors scroll; the one in the middle when it stops is your result.");

	static constexpr float BoxSquareSize = 48.f;
	static constexpr float BoxSquareGap = 4.f;
	TSharedRef<SHorizontalBox> BoxStripRow = SNew(SHorizontalBox);
	BoxOpeningColorBorders.SetNum(5);
	for (int32 i = 0; i < 5; ++i)
	{
		const int32 Idx = i;
		BoxStripRow->AddSlot()
			.AutoWidth()
			.Padding(Idx > 0 ? FMargin(BoxSquareGap, 0.f, 0.f, 0.f) : FMargin(0.f))
			[
				SAssignNew(BoxOpeningColorBorders[Idx], SBorder)
				.BorderBackgroundColor(FSlateColor(GetBoxOpeningColor((BoxOpeningStripOffset + Idx) % BoxColorCount)))
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
			[ SNew(STextBlock).Text(BoxOpeningText).TextStyle(&TextTitle).ColorAndOpacity(FT66FlatStyle::Tokens::Text) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 8.f)
			[ SNew(STextBlock).Text(BoxOpeningDesc).TextStyle(&TextBody).ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted).AutoWrapText(true) ]
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
			[ SAssignNew(BoxOpeningResultText, STextBlock).Text(FText::GetEmpty()).TextStyle(&TextHeading).ColorAndOpacity(FT66FlatStyle::Tokens::Text) ];

	TSharedRef<SWidget> BetRow =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
		[ SNew(STextBlock).Text(Loc ? Loc->GetText_BetAmount() : NSLOCTEXT("T66.Gambler", "BetAmount", "Bet Amount")).TextStyle(&TextBody).ColorAndOpacity(FT66FlatStyle::Tokens::Text) ]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
		[
			SAssignNew(GambleAmountSpin, SSpinBox<int32>)
			.MinValue(0).MaxValue(999999).Delta(10)
			.Font(FT66FlatStyle::Tokens::FontBold(26))
			.Value(GambleAmount)
			.OnValueChanged_Lambda([this](int32 V) { GambleAmount = FMath::Max(0, V); })
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			MakeGamblerFlatButton(MakeGamblerFlatButtonParams(
				Loc ? Loc->GetText_Bet() : NSLOCTEXT("T66.Gambler", "Bet", "Bet"),
				FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBetClicked),
				EGamblerFlatButton::Neutral)
				.SetMinWidth(0.f)
				.SetPadding(FMargin(14.f, 8.f)))
		];

	TSharedRef<SWidget> CasinoCenterContent =
		MakeGamblerFlatPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SAssignNew(CasinoSwitcher, SWidgetSwitcher)
				+ SWidgetSwitcher::Slot() [ CardsView ]
				+ SWidgetSwitcher::Slot() [ FT66AnimatedStyle::AttachMetadata(CoinFlipView, TEXT("Casino.Gambler.CoinFlip.PlayArea"), TEXT("GamblingGame")) ]
				+ SWidgetSwitcher::Slot() [ FT66AnimatedStyle::AttachMetadata(RpsView, TEXT("Casino.Gambler.RPS.PlayArea"), TEXT("GamblingGame")) ]
				+ SWidgetSwitcher::Slot() [ FT66AnimatedStyle::AttachMetadata(BlackJackView, TEXT("Casino.Gambler.BlackJack.PlayArea"), TEXT("GamblingGame")) ]
				+ SWidgetSwitcher::Slot() [ FT66AnimatedStyle::AttachMetadata(LotteryView, TEXT("Casino.Gambler.Lottery.PlayArea"), TEXT("GamblingGame")) ]
				+ SWidgetSwitcher::Slot() [ FT66AnimatedStyle::AttachMetadata(PlinkoView, TEXT("Casino.Gambler.Plinko.PlayArea"), TEXT("GamblingGame")) ]
				+ SWidgetSwitcher::Slot() [ FT66AnimatedStyle::AttachMetadata(BoxOpeningView, TEXT("Casino.Gambler.BoxOpening.PlayArea"), TEXT("GamblingGame")) ]
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f, 0.f, 0.f)
			[
				SAssignNew(CasinoBetRowBox, SBox)
				.Visibility(EVisibility::Collapsed)
				[
					BetRow
				]
			]
		,
			EGamblerFlatPanel::ContentPanelWide,
			FMargin(FT66FlatStyle::Tokens::Space6));

	// Buyback row (shared slot count with shop buyback)
	static constexpr int32 BuybackSlotCount = UT66RunStateSubsystem::BuybackDisplaySlotCount;
	BuybackNameTexts.SetNum(BuybackSlotCount);
	BuybackDescTexts.SetNum(BuybackSlotCount);
	BuybackPriceTexts.SetNum(BuybackSlotCount);
	BuybackTileBorders.SetNum(BuybackSlotCount);
	BuybackIconBorders.SetNum(BuybackSlotCount);
	BuybackIconImages.SetNum(BuybackSlotCount);
	BuybackIconBrushes.SetNum(BuybackSlotCount);
	BuybackBuyButtons.SetNum(BuybackSlotCount);
	for (int32 i = 0; i < BuybackSlotCount; ++i)
	{
		BuybackIconBrushes[i] = MakeShared<FSlateBrush>();
		BuybackIconBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		BuybackIconBrushes[i]->ImageSize = FVector2D(BuybackIconSize, BuybackIconSize);
	}
	TSharedRef<SHorizontalBox> BuybackRow = SNew(SHorizontalBox);
	for (int32 i = 0; i < BuybackSlotCount; ++i)
	{
		TSharedRef<SWidget> BuybackBtnWidget = MakeGamblerFlatButton(
			MakeGamblerFlatButtonParams(
				Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY"),
				FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBuybackSlot, i),
				EGamblerFlatButton::Primary)
			.SetMinWidth(bCompactCasinoLayout ? 0.f : 100.f)
			.SetPadding(CardButtonPadding)
			.SetFontSize(CardButtonFontSize)
			.SetContent(
				SAssignNew(BuybackPriceTexts[i], STextBlock)
				.Text(Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY"))
				.Font(FT66FlatStyle::Tokens::FontBold(CardButtonFontSize))
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			)
		);
		BuybackBuyButtons[i] = BuybackBtnWidget;
		BuybackRow->AddSlot()
			.AutoWidth()
			.Padding(i > 0 ? FMargin(bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space4, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			SNew(SBox)
			.WidthOverride(BuybackCardSize)
			.HeightOverride(BuybackCardHeight)
			[
				MakeGamblerFlatPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBox)
						.HeightOverride(60.f)
						[
							SAssignNew(BuybackNameTexts[i], STextBlock)
							.Text(FText::GetEmpty())
							.TextStyle(&TextHeading)
							.Font(FT66FlatStyle::Tokens::FontBold(CardHeadingFontSize))
							.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
							.AutoWrapText(true)
							.WrapTextAt(BuybackCardSize - GameCardPadding * 2.f)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66FlatStyle::Tokens::Space2, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)
						[
							MakeGamblerFlatPanel(
								SNew(SBox)
								.WidthOverride(BuybackIconSize)
								.HeightOverride(BuybackIconSize)
								[
									FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(
										SAssignNew(BuybackIconImages[i], SImage)
										.Image(BuybackIconBrushes[i].Get())
										.ColorAndOpacity(FLinearColor::White)),
										FName(*FString::Printf(TEXT("Gambler.BuybackCard.%02d.Icon"), i + 1)),
										TEXT("Icon"))
								],
								EGamblerFlatPanel::SlotNormal,
								FMargin(0.f),
								&BuybackIconBorders[i])
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66FlatStyle::Tokens::Space2, 0.f, 0.f)
					[
						SAssignNew(BuybackDescTexts[i], STextBlock)
						.Text(FText::GetEmpty())
						.TextStyle(&TextBody)
						.Font(FT66FlatStyle::Tokens::FontRegular(CardBodyFontSize))
						.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
						.AutoWrapText(true)
						.WrapTextAt(BuybackCardSize - GameCardPadding * 2.f)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66FlatStyle::Tokens::Space3, 0.f, 0.f)
					[
						BuybackBtnWidget
					]
				,
					EGamblerFlatPanel::OfferCardNormal,
					FMargin(GameCardPadding),
					&BuybackTileBorders[i])
			]
		];
	}

	TSharedRef<SWidget> CenterControls = bCompactCasinoLayout
		? StaticCastSharedRef<SWidget>(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66FlatStyle::Tokens::Space4, 0.f)
			[
				MakeGamblerFlatButton(
					MakeGamblerFlatButtonParams(
						BuybackTitle,
						FOnClicked::CreateLambda([this]()
						{
							if (CasinoBuybackSwitcher.IsValid())
							{
								CasinoBuybackSwitcher->SetActiveWidgetIndex(1);
							}
							if (UT66RunStateSubsystem* RS = GetWorld() && GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
							{
								RS->GenerateBuybackDisplay();
							}
							RefreshBuyback();
							RefreshCasinoGameChrome();
							return FReply::Handled();
						}),
						EGamblerFlatButton::Tab)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(12.f, 7.f))
					.SetFontSize(11)
					.SetTag(FName(TEXT("Gambler.Mode.BuybackButton")))
					.SetToggleGroup(GamblerModeToggleGroup))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66FlatStyle::Tokens::Space4, 0.f)
			[
				MakeGamblerFlatButton(
					MakeGamblerFlatButtonParams(
						GamesTitle,
						FOnClicked::CreateLambda([this]()
						{
							if (CasinoBuybackSwitcher.IsValid())
							{
								CasinoBuybackSwitcher->SetActiveWidgetIndex(0);
							}
							if (GameSelectionSwitcher.IsValid())
							{
								GameSelectionSwitcher->SetActiveWidgetIndex(0);
							}
							if (CasinoSwitcher.IsValid())
							{
								CasinoSwitcher->SetActiveWidgetIndex(0);
							}
							RefreshCasinoGameChrome();
							return FReply::Handled();
						}),
						EGamblerFlatButton::Tab)
					.SetSelected(true)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(12.f, 7.f))
					.SetFontSize(11)
					.SetTag(FName(TEXT("Gambler.Mode.GamesButton")))
					.SetToggleGroup(GamblerModeToggleGroup))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(SSpacer)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(CasinoRerollButtonWidget, SBox)
				[
					MakeGamblerFlatButton(
					MakeGamblerFlatButtonParams(
						RerollText,
						FOnClicked::CreateLambda([this]()
						{
							if (UT66RunStateSubsystem* RS = GetWorld() && GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
							{
								RS->RerollBuybackDisplay();
							}
							RefreshBuyback();
							RefreshCasinoGameChrome();
							return FReply::Handled();
						}),
						EGamblerFlatButton::Neutral)
					.SetMinWidth(0.f)
					.SetPadding(ActionButtonPadding)
					.SetFontSize(11)
					.SetTag(FName(TEXT("Gambler.Mode.RerollButton"))))
				]
			])
		: StaticCastSharedRef<SWidget>(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66FlatStyle::Tokens::Space4, 0.f)
			[
				MakeGamblerFlatButton(
					MakeGamblerFlatButtonParams(CasinoTitle, FOnClicked::CreateLambda([this]() {
						if (CasinoBuybackSwitcher.IsValid()) { CasinoBuybackSwitcher->SetActiveWidgetIndex(0); }
						RefreshCasinoGameChrome();
						return FReply::Handled();
					}), EGamblerFlatButton::Tab)
					.SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f))
				)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeGamblerFlatButton(
					MakeGamblerFlatButtonParams(BuybackTitle, FOnClicked::CreateLambda([this]() {
						if (CasinoBuybackSwitcher.IsValid()) { CasinoBuybackSwitcher->SetActiveWidgetIndex(1); }
						if (UT66RunStateSubsystem* RS = GetWorld() && GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr) { if (RS) RS->GenerateBuybackDisplay(); }
						RefreshBuyback();
						RefreshCasinoGameChrome();
						return FReply::Handled();
					}), EGamblerFlatButton::Tab)
					.SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f))
				)
			]);

	TSharedRef<SWidget> CenterPanel =
		MakeGamblerFlatPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, FT66FlatStyle::Tokens::Space4)
			[
				CenterControls
			]
			+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, FT66FlatStyle::Tokens::Space4, 0.f, 0.f)
			[
				SAssignNew(CasinoBuybackSwitcher, SWidgetSwitcher)
				+ SWidgetSwitcher::Slot()
				[
					SNew(SScrollBox)
					.Orientation(Orient_Horizontal)
					.ScrollBarVisibility(EVisibility::Visible)
					+ SScrollBox::Slot()
					[
						CasinoCenterContent
					]
				]
				+ SWidgetSwitcher::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(0.f, 0.f, 0.f, 0.f)
					[
						SNew(SScrollBox)
						.Orientation(Orient_Horizontal)
						.ScrollBarVisibility(EVisibility::Visible)
						+ SScrollBox::Slot()
						[
							BuybackRow
						]
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, FT66FlatStyle::Tokens::Space6, 0.f, 0.f)
					[
						SAssignNew(CasinoRerollButtonWidget, SBox)
						[
							MakeGamblerFlatButton(
							MakeGamblerFlatButtonParams(RerollText,
								FOnClicked::CreateLambda([this]() {
									if (UT66RunStateSubsystem* RS = GetWorld() && GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr) { if (RS) RS->RerollBuybackDisplay(); }
									RefreshBuyback();
									RefreshCasinoGameChrome();
									return FReply::Handled();
								}),
								EGamblerFlatButton::Neutral)
							.SetMinWidth(0.f)
							.SetPadding(FMargin(16.f, 10.f)))
						]
					]
				]
			]
		,
			EGamblerFlatPanel::ContentPanelWide,
			FMargin(bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space4 : FT66FlatStyle::Tokens::Space6),
			nullptr,
			FName(TEXT("Gambler.GamesPanel")));

	// Inventory slot buttons: same structure as shop (single panel, overlay image + dash, no inner box)
	const FName GamblerInventoryToggleGroup(TEXT("Gambler.InventorySelection"));
	for (int32 i = 0; i < UT66RunStateSubsystem::MaxInventorySlots; ++i)
	{
		const FName SlotTag(*FString::Printf(TEXT("Gambler.InventorySlot.%02d"), i + 1));
		InventorySlotButtons[i] = MakeGamblerFlatButton(
			MakeGamblerFlatButtonParams(FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnSelectInventorySlot, i),
				EGamblerFlatButton::Neutral)
				.SetMinWidth(InventorySlotSize)
				.SetMinHeight(InventorySlotSize)
				.SetPadding(FMargin(0.f))
				.SetTag(SlotTag)
				.SetToggleGroup(GamblerInventoryToggleGroup)
				.SetContent(
					MakeGamblerFlatPanel(
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(
								SAssignNew(InventorySlotIconImages[i], SImage)
								.Image(InventorySlotIconBrushes[i].Get())
								.ColorAndOpacity(FLinearColor::White)),
								FName(*FString::Printf(TEXT("Gambler.InventorySlot.%02d.Icon"), i + 1)),
								TEXT("Icon"))
						]
						+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 6.f, 8.f, 0.f)
						[
							SAssignNew(InventorySlotCountTexts[i], STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66FlatStyle::Tokens::FontBold(InventoryCountFontSize))
							.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
							.ShadowOffset(FVector2D(1.f, 1.f))
							.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f))
							.Visibility(EVisibility::Hidden)
						]
						+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
						[
							SAssignNew(InventorySlotTexts[i], STextBlock)
							.Text(NSLOCTEXT("T66.Common", "Dash", "-"))
							.TextStyle(&TextChip)
							.Font(FT66FlatStyle::Tokens::FontBold(InventoryDashFontSize))
							.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
						]
					,
						EGamblerFlatPanel::SlotNormal,
						FMargin(0.f),
						&InventorySlotBorders[i])
				));
	}

	// Pre-create sell button for centralized styling
	SellItemButton = MakeGamblerFlatButton(
		MakeGamblerFlatButtonParams(
			Loc ? Loc->GetText_Sell() : NSLOCTEXT("T66.Common", "Sell", "SELL"),
			FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnSellSelectedClicked),
			EGamblerFlatButton::Danger)
			.SetMinWidth(0.f)
			.SetPadding(ActionButtonPadding)
			.SetFontSize(CardButtonFontSize)
			.SetTag(FName(TEXT("Gambler.SellButton"))));

	// Inventory grid: same as Shop — 20 slots in horizontal scroll, each 160×160
	TSharedRef<SUniformGridPanel> GamblerInventoryGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(FT66FlatStyle::Tokens::Space2, 0.f));
	for (int32 Inv = 0; Inv < UT66RunStateSubsystem::MaxInventorySlots; ++Inv)
	{
		GamblerInventoryGrid->AddSlot(Inv, 0)
		[
			SNew(SBox)
			.WidthOverride(InventorySlotSize)
			.HeightOverride(InventorySlotSize)
			[
				InventorySlotButtons[Inv].ToSharedRef()
			]
		];
	}

	TSharedRef<SWidget> InventoryPanel =
		MakeGamblerFlatPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					FT66FlatStyle::AttachMetadata(SNew(STextBlock)
					.Text(InventoryTitle)
					.TextStyle(&TextHeading)
					.Font(FT66FlatStyle::Tokens::FontBold(SectionHeadingFontSize))
					.ColorAndOpacity(FT66FlatStyle::Tokens::Text),
					FName(TEXT("Gambler.Inventory.Title")),
					TEXT("Label.Section"),
					ET66FlatState::Default,
					TOptional<FLinearColor>(),
					false,
					NAME_None,
					true)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(18.f, 0.f, 16.f, 0.f)
				[
					FT66FlatStyle::AttachMetadata(SAssignNew(NetWorthText, STextBlock)
					.Text(FText::GetEmpty())
					.TextStyle(&TextHeading)
					.Font(FT66FlatStyle::Tokens::FontBold(SectionHeadingFontSize))
					.ColorAndOpacity(FT66FlatStyle::Tokens::Text),
					FName(TEXT("Gambler.Inventory.NetWorth")),
					TEXT("Label.Stat"),
					ET66FlatState::Default,
					TOptional<FLinearColor>(),
					false,
					NAME_None,
					true)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 16.f, 0.f)
				[
					FT66FlatStyle::AttachMetadata(SAssignNew(GoldText, STextBlock)
					.Text(FText::GetEmpty())
					.TextStyle(&TextHeading)
					.Font(FT66FlatStyle::Tokens::FontBold(SectionHeadingFontSize))
					.ColorAndOpacity(FT66FlatStyle::Tokens::Text),
					FName(TEXT("Gambler.Inventory.Gold")),
					TEXT("Label.Stat"),
					ET66FlatState::Default,
					TOptional<FLinearColor>(),
					false,
					NAME_None,
					true)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					FT66FlatStyle::AttachMetadata(SAssignNew(DebtText, STextBlock)
					.Text(FText::GetEmpty())
					.TextStyle(&TextHeading)
					.Font(FT66FlatStyle::Tokens::FontBold(SectionHeadingFontSize))
					.ColorAndOpacity(FT66FlatStyle::Tokens::Danger),
					FName(TEXT("Gambler.Inventory.Debt")),
					TEXT("Label.Stat"),
					ET66FlatState::Default,
					TOptional<FLinearColor>(),
					false,
					NAME_None,
					true)
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SSpacer)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66FlatStyle::Tokens::Space3, 0.f, 0.f)
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
				+ SHorizontalBox::Slot().AutoWidth().Padding(FT66FlatStyle::Tokens::Space6, 0.f, 0.f, 0.f)
				[
					SAssignNew(SellPanelContainer, SBox)
					.WidthOverride(InventorySlotSize)
					.HeightOverride(InventorySlotSize)
					.Visibility(EVisibility::Visible)
					[
						MakeGamblerFlatPanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SAssignNew(SellItemNameText, STextBlock)
								.Text(FText::GetEmpty())
								.TextStyle(&TextHeading)
								.Font(FT66FlatStyle::Tokens::FontBold(CardHeadingFontSize))
								.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
							[
								SAssignNew(SellItemDescText, STextBlock)
								.Text(FText::GetEmpty())
								.TextStyle(&TextBody)
								.Font(FT66FlatStyle::Tokens::FontRegular(CardBodyFontSize))
								.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
								.AutoWrapText(true)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
							[
								SAssignNew(SellItemPriceText, STextBlock)
								.Text(FText::GetEmpty())
								.TextStyle(&TextChip)
								.Font(FT66FlatStyle::Tokens::FontBold(CardButtonFontSize))
								.ColorAndOpacity(FT66FlatStyle::Tokens::Accent2)
							]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
						[
							SellItemButton.ToSharedRef()
						]
						,
							EGamblerFlatPanel::InnerPanel,
							FMargin(bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space4),
							nullptr,
							FName(TEXT("Gambler.SellPanel")))
					]
				]
			]
		,
			EGamblerFlatPanel::ContentPanelWide,
			FMargin(bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space4),
			nullptr,
			FName(TEXT("Gambler.InventoryPanel")));

	TSharedRef<SWidget> CasinoPageBody =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, FT66FlatStyle::Tokens::Space4)
		[
			SNew(STextBlock)
			.Text(CasinoTitle)
			.TextStyle(&TextTitle)
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			.Visibility(bCompactCasinoLayout ? EVisibility::Collapsed : EVisibility::Visible)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, bCompactCasinoLayout ? 0.f : 0.f, 0.f, bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space4 : FT66FlatStyle::Tokens::Space6)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space6, 0.f)
			[
				SAssignNew(StatsPanelBox, SBox)
				.WidthOverride(StatsPanelWidth)
				.HeightOverride(MainRowHeight)
				[
					FT66FlatStyle::AttachMetadata(
						T66StatsPanelSlate::MakeLiveEssentialStatsPanel(RunState, Loc, LiveStatsPanel.ToSharedRef(), StatsPanelWidth, true, StatsPanelFontAdjustment),
						FName(TEXT("Gambler.StatsPanel")),
						TEXT("Panel"),
						ET66FlatState::Default)
				]
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space6, 0.f)
			[
				SNew(SBox)
				.WidthOverride(CenterPanelWidth > 0.f ? FOptionalSize(CenterPanelWidth) : FOptionalSize())
				.MinDesiredHeight(MainRowHeight)
				[
					CenterPanel
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(RightPanelWidth)
				.MinDesiredHeight(MainRowHeight)
				[
					RightPanel
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 0.f)
		[
			InventoryPanel
		];

	if (bEmbeddedInCasinoShell)
	{
		TSharedRef<SConstraintCanvas> GamblerCanvas = SNew(SConstraintCanvas);
		AddGamblerCanvasSlot(GamblerCanvas, 17.f, 182.f, 1902.f, 887.f, CasinoPageBody);

		RefreshTopBar();
		RefreshInventory();
		RefreshSellPanel();
		RefreshCasinoGameChrome();
		return FT66FlatStyle::AttachMetadata(GamblerCanvas, FName(TEXT("Gambler.Root")), TEXT("Overlay"), ET66FlatState::Default);
	}

	TSharedRef<SWidget> CasinoPage =
		SNew(SScrollBox)
		.Orientation(Orient_Vertical)
		+ SScrollBox::Slot()
		[
			SNew(SBox)
			.WidthOverride(TAttribute<FOptionalSize>::CreateLambda([this]() -> FOptionalSize
			{
				const FVector2D Bounds = bEmbeddedInCasinoShell ? FT66FlatStyle::GetViewportLogicalSize() : FT66FlatStyle::GetSafeFrameSize();
				const float HorizontalMargins = bEmbeddedInCasinoShell ? 128.f : 80.f;
				return FOptionalSize(FMath::Max(1.f, Bounds.X - HorizontalMargins));
			}))
			[
				CasinoPageBody
			]
		];

	const TAttribute<FMargin> SafeContentInsets = TAttribute<FMargin>::CreateLambda([this]() -> FMargin
	{
		return bEmbeddedInCasinoShell ? FMargin(0.f) : FT66FlatStyle::GetSafeFrameInsets();
	});

	const TAttribute<FMargin> SafeClosePadding = TAttribute<FMargin>::CreateLambda([this]() -> FMargin
	{
		const FMargin LocalPadding(FT66FlatStyle::Tokens::Space6, FT66FlatStyle::Tokens::Space4, FT66FlatStyle::Tokens::Space6, 0.f);
		return bEmbeddedInCasinoShell ? LocalPadding : FT66FlatStyle::GetSafePadding(LocalPadding);
	});

	TSharedRef<SWidget> Root =
		MakeGamblerFlatPanel(
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.Padding(SafeContentInsets)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(FT66FlatStyle::Tokens::Space6, FT66FlatStyle::Tokens::Space4, FT66FlatStyle::Tokens::Space6, FT66FlatStyle::Tokens::Space2)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
						[
							SAssignNew(StatusText, STextBlock)
							.Text(FText::GetEmpty())
							.TextStyle(&TextBody)
							.Font(FT66FlatStyle::Tokens::FontRegular(StatusFontSize))
							.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
						]
					]
					+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Fill).VAlign(VAlign_Top).Padding(bCompactCasinoLayout ? 20.f : 40.f, bCompactCasinoLayout ? 8.f : 16.f, bCompactCasinoLayout ? 20.f : 40.f, 0.f)
					[
						SAssignNew(PageSwitcher, SWidgetSwitcher)
						+ SWidgetSwitcher::Slot()
						[ DialoguePage ]
						+ SWidgetSwitcher::Slot()
						[ CasinoPage ]
					]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.Padding(SafeClosePadding)
			[
				SAssignNew(CloseButtonBox, SBox)
				[
					MakeGamblerFlatButton(MakeGamblerFlatButtonParams(
						NSLOCTEXT("T66.Common", "Close", "CLOSE"),
						FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnBack),
						EGamblerFlatButton::Danger)
						.SetMinWidth(0.f)
						.SetPadding(FMargin(18.f, 10.f)))
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(CheatPromptContainer, SBox)
				.Visibility(EVisibility::Collapsed)
				[
					MakeGamblerFlatPanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
						[
							SNew(STextBlock)
							.Text(Loc ? Loc->GetText_CheatPromptTitle() : FText::GetEmpty())
							.TextStyle(&TextHeading)
							.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 14.f)
						[
							SNew(STextBlock)
							.Text(Loc ? Loc->GetText_CheatPromptBody() : FText::GetEmpty())
							.TextStyle(&TextBody)
							.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
						]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
						[
							MakeGamblerFlatButton(MakeGamblerFlatButtonParams(
								Loc ? Loc->GetText_Yes() : FText::GetEmpty(),
								FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCheatYes),
								EGamblerFlatButton::Danger)
								.SetMinWidth(140.f)
								.SetPadding(FMargin(18.f, 10.f)))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
						[
							MakeGamblerFlatButton(MakeGamblerFlatButtonParams(
								Loc ? Loc->GetText_No() : FText::GetEmpty(),
								FOnClicked::CreateUObject(this, &UT66GamblerOverlayWidget::OnCheatNo),
								EGamblerFlatButton::Neutral)
								.SetMinWidth(140.f)
								.SetPadding(FMargin(18.f, 10.f)))
						]
					]
					,
						EGamblerFlatPanel::OverlayModalPanel,
						FMargin(FT66FlatStyle::Tokens::Space6))
				]
			]
		,
			EGamblerFlatPanel::ContentPanelWide,
			FMargin(0.f));

	// Embedded casino tabs enter directly into the casino page; standalone use keeps the dialogue gate.
	SetPage(bEmbeddedInCasinoShell ? EGamblerPage::Casino : EGamblerPage::Dialogue);
	RefreshTopBar();
	RefreshInventory();
	RefreshSellPanel();
	return bEmbeddedInCasinoShell ? Root : FT66FlatStyle::MakeResponsiveRoot(Root);
}

