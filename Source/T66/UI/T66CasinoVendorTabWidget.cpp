// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CasinoVendorTabWidget.h"
#include "UI/T66StatsPanelSlate.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Data/T66DataTypes.h"
#include "UI/T66ItemCardTextUtils.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66FlatStyle.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SNullWidget.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Engine/Texture2D.h"

static UT66RunStateSubsystem* GetRunStateFromWorld(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}

static FString MakeShopInventoryStackKey(const FT66InventorySlot& Slot)
{
	return FString::Printf(TEXT("%s|%d"), *Slot.ItemTemplateID.ToString(), static_cast<int32>(Slot.Rarity));
}

static constexpr int32 T66MobLootSellSelectionIndex = -2;
static const FName T66MobLootItemID(TEXT("Item_MobLoot"));

static int32 GetVisibleSellSlotCount()
{
	return UT66CasinoVendorTabWidget::SellVisibleSlotCount;
}

static int32 GetSellEntryCount(const TArray<FName>& Inventory, const bool bHasMobLootStack)
{
	int32 Count = bHasMobLootStack ? 1 : 0;
	for (const FName ItemID : Inventory)
	{
		if (!ItemID.IsNone())
		{
			++Count;
		}
	}
	return Count;
}

static int32 ResolveSellEntryToInventoryIndex(const TArray<FName>& Inventory, const bool bHasMobLootStack, int32 EntryIndex)
{
	if (EntryIndex < 0)
	{
		return INDEX_NONE;
	}

	if (bHasMobLootStack)
	{
		if (EntryIndex == 0)
		{
			return T66MobLootSellSelectionIndex;
		}
		--EntryIndex;
	}

	int32 Seen = 0;
	for (int32 InventoryIndex = 0; InventoryIndex < Inventory.Num(); ++InventoryIndex)
	{
		if (Inventory[InventoryIndex].IsNone())
		{
			continue;
		}
		if (Seen == EntryIndex)
		{
			return InventoryIndex;
		}
		++Seen;
	}

	return INDEX_NONE;
}

static int32 FindSellEntryIndexForSelection(const TArray<FName>& Inventory, const bool bHasMobLootStack, const int32 SelectedInventoryIndex)
{
	if (SelectedInventoryIndex == T66MobLootSellSelectionIndex)
	{
		return bHasMobLootStack ? 0 : INDEX_NONE;
	}

	if (!Inventory.IsValidIndex(SelectedInventoryIndex) || Inventory[SelectedInventoryIndex].IsNone())
	{
		return INDEX_NONE;
	}

	int32 EntryIndex = bHasMobLootStack ? 1 : 0;
	for (int32 InventoryIndex = 0; InventoryIndex < Inventory.Num(); ++InventoryIndex)
	{
		if (Inventory[InventoryIndex].IsNone())
		{
			continue;
		}
		if (InventoryIndex == SelectedInventoryIndex)
		{
			return EntryIndex;
		}
		++EntryIndex;
	}

	return INDEX_NONE;
}

static void AddItemIconPath(
	UT66GameInstance* GI,
	FName ItemID,
	ET66ItemRarity Rarity,
	TArray<FSoftObjectPath>& OutPaths)
{
	if (!GI || ItemID.IsNone())
	{
		return;
	}

	FItemData ItemData;
	if (!GI->GetItemData(ItemID, ItemData))
	{
		return;
	}

	const TSoftObjectPtr<UTexture2D> IconSoft = ItemData.GetIconForRarity(Rarity);
	if (!IconSoft.IsNull())
	{
		OutPaths.AddUnique(IconSoft.ToSoftObjectPath());
	}
}

namespace
{
	void AddVendorShopCanvasSlot(
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

	TSharedRef<SWidget> MakeVendorFlatButton(
		const ET66FlatState State,
		const FText& Label,
		FOnClicked OnClicked,
		const FMargin& Padding,
		const int32 FontSize,
		const FName Tag,
		const FName ToggleGroup = NAME_None)
	{
		return FT66FlatStyle::MakeFlatButton(
			State,
			Label,
			MoveTemp(OnClicked),
			nullptr,
			nullptr,
			Padding,
			0.f,
			0.f,
			true,
			FontSize,
			Tag,
			ToggleGroup);
	}

	TSharedRef<SWidget> MakeVendorLabel(
		const FText& Text,
		const int32 FontSize,
		const FName Tag,
		const FLinearColor Color = FT66FlatStyle::PrimaryText())
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(STextBlock)
				.Text(Text)
				.Font(FT66FlatStyle::MakeBoldFont(FontSize))
				.ColorAndOpacity(Color),
			Tag,
			TEXT("Label.Body"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	}

	FText MakeVendorStatValue(const int32 Value)
	{
		return FText::FromString(FString::Printf(TEXT("%d/99"), FMath::Clamp(Value, 0, UT66RunStateSubsystem::MaxHeroStatValue)));
	}

	FText MakeVendorSecondaryStatValue(const UT66RunStateSubsystem* RunState, const ET66SecondaryStatType StatType)
	{
		const int32 Value = RunState ? FMath::RoundToInt(RunState->GetSecondaryStatValue(StatType)) : 0;
		return MakeVendorStatValue(Value);
	}

	void AddVendorStatsTextLine(
		const TSharedRef<SVerticalBox>& Box,
		const FText& Text,
		const int32 FontSize,
		const FLinearColor Color = FT66FlatStyle::PrimaryText())
	{
		Box->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)
		[
			SNew(STextBlock)
			.Text(Text)
			.Font(FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(Color)
		];
	}

	void AddVendorStatsSection(
		const TSharedRef<SVerticalBox>& Box,
		const FText& Header,
		const int32 HeaderSize)
	{
		Box->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 4.f)
		[
			SNew(STextBlock)
			.Text(Header)
			.Font(FT66FlatStyle::MakeBoldFont(HeaderSize))
			.ColorAndOpacity(FT66FlatStyle::PrimaryText())
		];
	}

	TSharedRef<SWidget> MakeVendorReferenceStatsPanel(UT66RunStateSubsystem* RunState)
	{
		const int32 BodyFontSize = 14;
		const int32 HeaderFontSize = 18;
		const FText StatLineFormat = NSLOCTEXT("T66.Vendor", "VendorStatLineFormat", "{0} {1}");

		TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);
		Content->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.StatsPanel", "Header", "STATS"))
			.Font(FT66FlatStyle::MakeBoldFont(26))
			.ColorAndOpacity(FT66FlatStyle::PrimaryText())
		];

		AddVendorStatsSection(Content, NSLOCTEXT("T66.StatsPanel", "VendorDamageHeader", "DAMAGE:"), HeaderFontSize);
		AddVendorStatsTextLine(Content, FText::Format(StatLineFormat, NSLOCTEXT("T66.StatsPanel", "VendorDamage", "Damage"), MakeVendorStatValue(RunState ? RunState->GetDamageStat() : 0)), BodyFontSize);
		AddVendorStatsTextLine(Content, FText::Format(StatLineFormat, NSLOCTEXT("T66.StatsPanel", "VendorAoeDamage", "AOE Damage"), MakeVendorSecondaryStatValue(RunState, ET66SecondaryStatType::AoeDamage)), BodyFontSize);
		AddVendorStatsTextLine(Content, FText::Format(StatLineFormat, NSLOCTEXT("T66.StatsPanel", "VendorBounceDamage", "Bounce Damage"), MakeVendorSecondaryStatValue(RunState, ET66SecondaryStatType::BounceDamage)), BodyFontSize);
		AddVendorStatsTextLine(Content, FText::Format(StatLineFormat, NSLOCTEXT("T66.StatsPanel", "VendorPierceDamage", "Pierce Damage"), MakeVendorSecondaryStatValue(RunState, ET66SecondaryStatType::PierceDamage)), BodyFontSize);

		AddVendorStatsSection(Content, NSLOCTEXT("T66.StatsPanel", "VendorAttackSpeedHeader", "ATTACK SPEED:"), HeaderFontSize);
		AddVendorStatsTextLine(Content, FText::Format(StatLineFormat, NSLOCTEXT("T66.StatsPanel", "VendorAttackSpeed", "Attack Speed"), MakeVendorStatValue(RunState ? RunState->GetAttackSpeedStat() : 0)), BodyFontSize);
		AddVendorStatsTextLine(Content, FText::Format(StatLineFormat, NSLOCTEXT("T66.StatsPanel", "VendorAttackScale", "Attack Scale"), MakeVendorStatValue(RunState ? RunState->GetScaleStat() : 0)), BodyFontSize);

		AddVendorStatsSection(Content, NSLOCTEXT("T66.StatsPanel", "VendorDefenseHeader", "DEFENSE:"), HeaderFontSize);
		AddVendorStatsTextLine(Content, FText::Format(StatLineFormat, NSLOCTEXT("T66.StatsPanel", "VendorAccuracy", "Accuracy"), MakeVendorStatValue(RunState ? RunState->GetAccuracyStat() : 0)), BodyFontSize);
		AddVendorStatsTextLine(Content, FText::Format(StatLineFormat, NSLOCTEXT("T66.StatsPanel", "VendorArmor", "Armor"), MakeVendorStatValue(RunState ? RunState->GetArmorStat() : 0)), BodyFontSize);
		AddVendorStatsTextLine(Content, FText::Format(StatLineFormat, NSLOCTEXT("T66.StatsPanel", "VendorEvasion", "Evasion"), MakeVendorStatValue(RunState ? RunState->GetEvasionStat() : 0)), BodyFontSize);

		AddVendorStatsSection(Content, NSLOCTEXT("T66.StatsPanel", "VendorUtilityHeader", "UTILITY:"), HeaderFontSize);
		AddVendorStatsTextLine(Content, FText::Format(StatLineFormat, NSLOCTEXT("T66.StatsPanel", "VendorLuck", "Luck"), MakeVendorStatValue(RunState ? RunState->GetLuckStat() : 0)), BodyFontSize);
		AddVendorStatsTextLine(Content, FText::Format(StatLineFormat, NSLOCTEXT("T66.StatsPanel", "VendorSpeed", "Speed"), MakeVendorStatValue(RunState ? RunState->GetSpeedStat() : 0)), BodyFontSize);

		return FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(16.f, 12.f),
			Content,
			nullptr,
			FName(TEXT("Vendor.StatsPanel")));
	}
}

void UT66CasinoVendorTabWidget::NativeDestruct()
{
	// Stop any steal prompt ticking.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StealTickTimerHandle);
	}

	// Unbind runstate delegates (safe to call even if not bound).
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->GoldChanged.RemoveDynamic(this, &UT66CasinoVendorTabWidget::HandleGoldOrDebtChanged);
				RunState->DebtChanged.RemoveDynamic(this, &UT66CasinoVendorTabWidget::HandleGoldOrDebtChanged);
				RunState->InventoryChanged.RemoveDynamic(this, &UT66CasinoVendorTabWidget::HandleInventoryChanged);
				RunState->ShopChanged.RemoveDynamic(this, &UT66CasinoVendorTabWidget::HandleShopChanged);
				RunState->BuybackChanged.RemoveDynamic(this, &UT66CasinoVendorTabWidget::HandleBuybackChanged);
				RunState->BossChanged.RemoveDynamic(this, &UT66CasinoVendorTabWidget::HandleBossChanged);
			}
		}
	}

	// Safety: if destroyed unexpectedly as a standalone overlay, restore gameplay input.
	// Embedded casino-shell tabs are rebuilt through the host overlay and must not steal
	// input back from the visible shell.
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (!bEmbeddedInCasinoShell && PC->IsGameplayLevel() && !PC->IsPaused())
		{
			PC->RestoreGameplayInputMode();
		}
	}

	ReleaseCachedSlateResources();
	Super::NativeDestruct();
}

void UT66CasinoVendorTabWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	ReleaseCachedSlateResources();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void UT66CasinoVendorTabWidget::ReleaseCachedSlateResources()
{
	PageSwitcher.Reset();
	NetWorthText.Reset();
	GoldText.Reset();
	DebtText.Reset();
	StatusText.Reset();
	StatsPanelBox.Reset();
	LiveStatsPanel.Reset();
	ShopBuybackSwitcher.Reset();
	SellPanelContainer.Reset();
	SellItemNameText.Reset();
	SellItemDescText.Reset();
	SellItemPriceText.Reset();
	SellItemButton.Reset();
	BorrowAmountSpin.Reset();
	PaybackAmountSpin.Reset();
	StealPromptContainer.Reset();
	StealMarkerSpacerBox.Reset();

	ItemNameTexts.Reset();
	ItemDescTexts.Reset();
	ItemPriceTexts.Reset();
	ItemTileBorders.Reset();
	ItemIconBorders.Reset();
	ItemIconImages.Reset();
	ItemIconBrushes.Reset();
	BuyButtons.Reset();
	StealButtons.Reset();
	BuyButtonTexts.Reset();

	BuybackNameTexts.Reset();
	BuybackDescTexts.Reset();
	BuybackPriceTexts.Reset();
	BuybackTileBorders.Reset();
	BuybackIconBorders.Reset();
	BuybackIconImages.Reset();
	BuybackIconBrushes.Reset();
	BuybackBuyButtons.Reset();

	InventorySlotBorders.Reset();
	InventorySlotButtons.Reset();
	InventorySlotTexts.Reset();
	InventorySlotDescTexts.Reset();
	InventorySlotCountTexts.Reset();
	InventorySlotActionTexts.Reset();
	InventorySlotIconImages.Reset();
	InventorySlotIconBrushes.Reset();
	InventoryPageText.Reset();
	InventoryRotateButtonWidget.Reset();
}

void UT66CasinoVendorTabWidget::CloseOverlay()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StealTickTimerHandle);
	}
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

TSharedRef<SWidget> UT66CasinoVendorTabWidget::RebuildWidget()
{
	bBoughtSomethingThisVisit = false;
	BorrowAmount = FMath::Max(0, BorrowAmount);
	PaybackAmount = FMath::Max(0, PaybackAmount);
	SelectedInventoryIndex = -1;
	SellInventoryPageIndex = 0;

	UWorld* World = GetWorld();
	UT66LocalizationSubsystem* Loc = nullptr;
	UT66UITexturePoolSubsystem* TexPool = nullptr;
	if (World)
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
			TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>();
		}
	}

	// Force stock generation now so we can build tiles.
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (RunState)
	{
		RunState->EnsureShopStockForCurrentStage();
		bBoughtSomethingThisVisit = RunState->HasBoughtFromShopThisStage();
		bCachedBossActive = RunState->GetBossActive();

		// Bind live updates (event-driven UI).
		RunState->GoldChanged.RemoveDynamic(this, &UT66CasinoVendorTabWidget::HandleGoldOrDebtChanged);
		RunState->DebtChanged.RemoveDynamic(this, &UT66CasinoVendorTabWidget::HandleGoldOrDebtChanged);
		RunState->InventoryChanged.RemoveDynamic(this, &UT66CasinoVendorTabWidget::HandleInventoryChanged);
		RunState->ShopChanged.RemoveDynamic(this, &UT66CasinoVendorTabWidget::HandleShopChanged);
		RunState->BuybackChanged.RemoveDynamic(this, &UT66CasinoVendorTabWidget::HandleBuybackChanged);
		RunState->BossChanged.RemoveDynamic(this, &UT66CasinoVendorTabWidget::HandleBossChanged);

		RunState->GoldChanged.AddDynamic(this, &UT66CasinoVendorTabWidget::HandleGoldOrDebtChanged);
		RunState->DebtChanged.AddDynamic(this, &UT66CasinoVendorTabWidget::HandleGoldOrDebtChanged);
		RunState->InventoryChanged.AddDynamic(this, &UT66CasinoVendorTabWidget::HandleInventoryChanged);
		RunState->ShopChanged.AddDynamic(this, &UT66CasinoVendorTabWidget::HandleShopChanged);
		RunState->BuybackChanged.AddDynamic(this, &UT66CasinoVendorTabWidget::HandleBuybackChanged);
		RunState->BossChanged.AddDynamic(this, &UT66CasinoVendorTabWidget::HandleBossChanged);
	}

	const FTextBlockStyle& TextTitle = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Title"));
	const FTextBlockStyle& TextHeading = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading"));
	const FTextBlockStyle& TextBody = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body"));
	const FTextBlockStyle& TextChip = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Chip"));
	const bool bCompactCasinoLayout = bEmbeddedInCasinoShell;
	const float CompactUiScale = bCompactCasinoLayout ? FMath::Max(0.1f, FT66FlatStyle::GetGlobalUIScale()) : 1.f;
	auto CompactPx = [bCompactCasinoLayout, CompactUiScale](const float Value)
	{
		return bCompactCasinoLayout ? (Value / CompactUiScale) : Value;
	};
	const float OverlayPadding = bCompactCasinoLayout ? 0.f : FT66FlatStyle::Tokens::NPCOverlayPadding;
	const float StatsPanelWidth = bCompactCasinoLayout ? 270.f : FT66FlatStyle::Tokens::NPCShopStatsPanelWidth;
	const float RightPanelWidth = bCompactCasinoLayout ? 355.f : FT66FlatStyle::Tokens::NPCRightPanelWidth;
	const float MainRowHeight = bCompactCasinoLayout ? 651.f : FT66FlatStyle::Tokens::NPCMainRowHeight;
	const float ShopCardSize = bCompactCasinoLayout ? CompactPx(226.f) : FT66FlatStyle::Tokens::NPCShopCardWidth;
	const float ShopCardHeight = bCompactCasinoLayout ? CompactPx(527.f) : FT66FlatStyle::Tokens::NPCShopCardHeight;
	const float ShopCardGap = bCompactCasinoLayout ? CompactPx(13.f) : FT66FlatStyle::Tokens::Space4;
	const float ShopCardPadding = bCompactCasinoLayout ? CompactPx(12.f) : FT66FlatStyle::Tokens::Space4;
	const float ShopNameBoxHeight = bCompactCasinoLayout ? CompactPx(50.f) : 60.f;
	const float ShopIconSize = ShopCardSize - ShopCardPadding * 2.f;
	const float InventorySlotSize = bCompactCasinoLayout ? CompactPx(75.f) : FT66FlatStyle::Tokens::InventorySlotSize;
	const float SellPanelSize = bCompactCasinoLayout ? CompactPx(92.f) : 160.f;
	const float BankSpinBoxWidth = bCompactCasinoLayout ? CompactPx(140.f) : FT66FlatStyle::Tokens::NPCBankSpinBoxWidth;
	const float BankSpinBoxHeight = bCompactCasinoLayout ? CompactPx(48.f) : FT66FlatStyle::Tokens::NPCBankSpinBoxHeight;
	const float CardButtonMinWidth = bCompactCasinoLayout ? 0.f : 100.f;
	const FMargin ShopButtonPadding = bCompactCasinoLayout ? FMargin(CompactPx(10.f), CompactPx(7.f)) : FMargin(8.f, 6.f);
	const FMargin ActionButtonPadding = bCompactCasinoLayout ? FMargin(CompactPx(14.f), CompactPx(10.f)) : FMargin(16.f, 10.f);
	const int32 StatsPanelFontAdjustment = bCompactCasinoLayout ? 0 : 0;
	const int32 CardHeadingFontSize = bCompactCasinoLayout ? 17 : 16;
	const int32 CardBodyFontSize = bCompactCasinoLayout ? 13 : 12;
	const int32 CardButtonFontSize = bCompactCasinoLayout ? 15 : 14;
	const int32 InventoryCountFontSize = bCompactCasinoLayout ? 11 : 14;
	const int32 InventoryDashFontSize = bCompactCasinoLayout ? 15 : 16;
	const int32 SectionHeadingFontSize = bCompactCasinoLayout ? 24 : 16;
	const int32 PageTitleFontSize = bCompactCasinoLayout ? 32 : 64;
	const int32 StatusFontSize = bCompactCasinoLayout ? 18 : 12;
	const int32 SpinBoxFontSize = bCompactCasinoLayout ? 16 : 16;

	const FText ShopTitle = Loc ? Loc->GetText_Shop() : NSLOCTEXT("T66.Shop", "ShopTitle", "SHOP");
	const FText BankTitle = Loc ? Loc->GetText_Bank() : NSLOCTEXT("T66.Shop", "BankTitle", "BANK");
	const FText CloseText = NSLOCTEXT("T66.Common", "Close", "CLOSE");
	const FText InventoryTitle = Loc ? Loc->GetText_YourItems() : NSLOCTEXT("T66.Shop", "InventoryTitle", "INVENTORY");
	const FText RerollText = Loc ? Loc->GetText_Reroll() : NSLOCTEXT("T66.Shop", "Reroll", "REROLL");
	LiveStatsPanel = MakeShared<T66StatsPanelSlate::FT66LiveStatsPanel>();

	const TArray<FName> EmptyStock;
	const TArray<FName>& Stock = RunState ? RunState->GetShopStockItemIDs() : EmptyStock;

	static constexpr int32 ShopSlotCount = UT66RunStateSubsystem::ShopDisplaySlotCount;
	ItemNameTexts.SetNum(ShopSlotCount);
	ItemDescTexts.SetNum(ShopSlotCount);
	ItemPriceTexts.SetNum(ShopSlotCount);
	ItemTileBorders.SetNum(ShopSlotCount);
	ItemIconBorders.SetNum(ShopSlotCount);
	ItemIconImages.SetNum(ShopSlotCount);
	ItemIconBrushes.SetNum(ShopSlotCount);
	BuyButtons.SetNum(ShopSlotCount);
	StealButtons.SetNum(ShopSlotCount);
	BuyButtonTexts.SetNum(ShopSlotCount);

	InventorySlotBorders.SetNum(InventorySlotCount);
	InventorySlotButtons.SetNum(InventorySlotCount);
	InventorySlotTexts.SetNum(InventorySlotCount);
	InventorySlotDescTexts.SetNum(InventorySlotCount);
	InventorySlotCountTexts.SetNum(InventorySlotCount);
	InventorySlotActionTexts.SetNum(InventorySlotCount);
	InventorySlotIconImages.SetNum(InventorySlotCount);
	InventorySlotIconBrushes.SetNum(InventorySlotCount);

	for (int32 i = 0; i < ShopSlotCount; ++i)
	{
		ItemIconBrushes[i] = MakeShared<FSlateBrush>();
		ItemIconBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		ItemIconBrushes[i]->ImageSize = FVector2D(ShopIconSize, ShopIconSize);
	}
	for (int32 i = 0; i < InventorySlotIconBrushes.Num(); ++i)
	{
		InventorySlotIconBrushes[i] = MakeShared<FSlateBrush>();
		InventorySlotIconBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		InventorySlotIconBrushes[i]->ImageSize = FVector2D(ShopIconSize, ShopIconSize);
	}

	TSharedRef<SHorizontalBox> ShopRow = SNew(SHorizontalBox);

	for (int32 i = 0; i < ShopSlotCount; ++i)
	{
		TSharedRef<SWidget> BuyBtnWidget = FT66FlatStyle::MakeFlatToggleGroupButton(
			ET66FlatState::Selected,
				SAssignNew(BuyButtonTexts[i], STextBlock)
				.Text(Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY"))
				.Font(FT66FlatStyle::MakeBoldFont(CardButtonFontSize))
				.ColorAndOpacity(FT66FlatStyle::SelectedText()),
			FOnClicked::CreateUObject(this, &UT66CasinoVendorTabWidget::OnBuySlot, i),
			ShopButtonPadding,
			CardButtonMinWidth,
			CompactPx(36.f),
			true,
			FName(*FString::Printf(TEXT("Vendor.ShopCard.%02d.BuyButton"), i + 1)));
		BuyButtons[i] = BuyBtnWidget;

		TSharedRef<SWidget> StealBtnWidget = FT66FlatStyle::MakeFlatButton(
			ET66FlatState::Default,
			Loc ? Loc->GetText_Steal() : NSLOCTEXT("T66.Shop", "Steal", "STEAL"),
			FOnClicked::CreateUObject(this, &UT66CasinoVendorTabWidget::OnStealSlot, i),
			nullptr,
			nullptr,
			ShopButtonPadding,
			CardButtonMinWidth,
			CompactPx(36.f),
			true,
			CardButtonFontSize,
			FName(*FString::Printf(TEXT("Vendor.ShopCard.%02d.StealButton"), i + 1)));
		StealButtons[i] = StealBtnWidget;

		ShopRow->AddSlot()
			.AutoWidth()
			.Padding(i > 0 ? FMargin(ShopCardGap, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			SNew(SBox)
			.WidthOverride(ShopCardSize)
			.HeightOverride(ShopCardHeight)
			[
				FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Default,
					FMargin(ShopCardPadding),
					SNew(SVerticalBox)
					// 1. Name at top
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBox)
						.HeightOverride(ShopNameBoxHeight)
						[
							SAssignNew(ItemNameTexts[i], STextBlock)
							.Text(FText::GetEmpty())
							.TextStyle(&TextHeading)
							.Font(FT66FlatStyle::Tokens::FontBold(CardHeadingFontSize))
							.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
							.AutoWrapText(true)
							.WrapTextAt(ShopCardSize - ShopCardPadding * 2.f)
						]
					]
					// 2. Large image below name (centered)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66FlatStyle::Tokens::Space2, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)
						[
							FT66FlatStyle::MakeFlatPanel(
								ET66FlatState::Default,
								FMargin(0.f),
								SNew(SBox)
								.WidthOverride(ShopIconSize)
								.HeightOverride(ShopIconSize)
								[
									FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(
										SAssignNew(ItemIconImages[i], SImage)
										.Image(ItemIconBrushes[i].Get())
										.ColorAndOpacity(FLinearColor::White)),
										FName(*FString::Printf(TEXT("Vendor.ShopCard.%02d.Icon"), i + 1)),
										TEXT("Icon"))
								],
								&ItemIconBorders[i],
								FName(*FString::Printf(TEXT("Vendor.ShopCard.%02d.IconPanel"), i + 1)))
						]
					]
					// 3. Two stat lines stacked
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66FlatStyle::Tokens::Space2, 0.f, 0.f)
					[
						SAssignNew(ItemDescTexts[i], STextBlock)
						.Text(FText::GetEmpty())
						.TextStyle(&TextBody)
						.Font(FT66FlatStyle::Tokens::FontRegular(CardBodyFontSize))
						.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
						.AutoWrapText(true)
						.WrapTextAt(ShopCardSize - ShopCardPadding * 2.f)
					]
					// 4. Buy and Steal side by side
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66FlatStyle::Tokens::Space3, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
							.FillWidth(1.f)
							.Padding(0.f, 0.f, FT66FlatStyle::Tokens::Space2, 0.f)
						[ BuyBtnWidget ]
						+ SHorizontalBox::Slot()
							.FillWidth(1.f)
						[ StealBtnWidget ]
					]
					,
					&ItemTileBorders[i],
					FName(*FString::Printf(TEXT("Vendor.ShopCard.%02d.Panel"), i + 1)))
			]
		];
	}

	// Buyback row (shared slot count; BUY only, price = sell price)
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
		BuybackIconBrushes[i]->ImageSize = FVector2D(ShopIconSize, ShopIconSize);
	}
	const FText BuybackTitle = NSLOCTEXT("T66.Shop", "Buyback", "BUYBACK");
	TSharedRef<SHorizontalBox> BuybackRow = SNew(SHorizontalBox);
	for (int32 i = 0; i < BuybackSlotCount; ++i)
	{
		TSharedRef<SWidget> BuybackBtnWidget = FT66FlatStyle::MakeFlatToggleGroupButton(
			ET66FlatState::Selected,
				SAssignNew(BuybackPriceTexts[i], STextBlock)
				.Text(Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY"))
				.Font(FT66FlatStyle::MakeBoldFont(CardButtonFontSize))
				.ColorAndOpacity(FT66FlatStyle::SelectedText()),
			FOnClicked::CreateUObject(this, &UT66CasinoVendorTabWidget::OnBuybackSlot, i),
			ShopButtonPadding,
			CardButtonMinWidth,
			CompactPx(36.f),
			true,
			FName(*FString::Printf(TEXT("Vendor.BuybackCard.%02d.BuyButton"), i + 1)));
		BuybackBuyButtons[i] = BuybackBtnWidget;
		BuybackRow->AddSlot()
			.AutoWidth()
			.Padding(i > 0 ? FMargin(ShopCardGap, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			SNew(SBox)
			.WidthOverride(ShopCardSize)
			.HeightOverride(ShopCardHeight)
			[
				FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Default,
					FMargin(ShopCardPadding),
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBox)
						.HeightOverride(ShopNameBoxHeight)
						[
							SAssignNew(BuybackNameTexts[i], STextBlock)
							.Text(FText::GetEmpty())
							.TextStyle(&TextHeading)
							.Font(FT66FlatStyle::Tokens::FontBold(CardHeadingFontSize))
							.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
							.AutoWrapText(true)
							.WrapTextAt(ShopCardSize - ShopCardPadding * 2.f)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66FlatStyle::Tokens::Space2, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)
						[
							FT66FlatStyle::MakeFlatPanel(
								ET66FlatState::Default,
								FMargin(0.f),
								SNew(SBox)
								.WidthOverride(ShopIconSize)
								.HeightOverride(ShopIconSize)
								[
									FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(
										SAssignNew(BuybackIconImages[i], SImage)
										.Image(BuybackIconBrushes[i].Get())
										.ColorAndOpacity(FLinearColor::White)),
										FName(*FString::Printf(TEXT("Vendor.BuybackCard.%02d.Icon"), i + 1)),
										TEXT("Icon"))
								],
								&BuybackIconBorders[i],
								FName(*FString::Printf(TEXT("Vendor.BuybackCard.%02d.IconPanel"), i + 1)))
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
						.WrapTextAt(ShopCardSize - ShopCardPadding * 2.f)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66FlatStyle::Tokens::Space3, 0.f, 0.f)
					[
						BuybackBtnWidget
					]
					,
					&BuybackTileBorders[i],
					FName(*FString::Printf(TEXT("Vendor.BuybackCard.%02d.Panel"), i + 1)))
			]
		];
	}

	// Steal prompt (created once, overlaid and toggled visible when needed).
	TSharedRef<SWidget> StealPromptWidget =
		SAssignNew(StealPromptContainer, SBox)
		.WidthOverride(bCompactCasinoLayout ? 360.f : 560.f)
		.HeightOverride(bCompactCasinoLayout ? 160.f : 220.f)
		.Visibility(EVisibility::Collapsed)
		[
			FT66FlatStyle::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Shop", "StealTimingTitle", "STEAL (TIMING)"))
					.TextStyle(&TextHeading)
					.Font(FT66FlatStyle::Tokens::FontBold(CardHeadingFontSize))
					.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(SBox)
					.WidthOverride(bCompactCasinoLayout ? 220.f : 360.f)
					.HeightOverride(28.f)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							FT66FlatStyle::MakePanel(SNullWidget::NullWidget, FT66PanelParams(ET66PanelType::Panel2))
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Fill)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SAssignNew(StealMarkerSpacerBox, SBox)
								.WidthOverride(0.f)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SBox)
								.WidthOverride(10.f)
								[
									FT66FlatStyle::MakePanel(
										SNullWidget::NullWidget,
										FT66PanelParams(ET66PanelType::Panel2).SetColor(FT66FlatStyle::Tokens::Success))
								]
							]
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Shop", "StealTimingInstructions", "Press STOP near the center."))
					.TextStyle(&TextBody)
					.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					FT66FlatStyle::MakeButton(
						FT66FlatStyle::MakeInRunButtonParams(
							NSLOCTEXT("T66.Shop", "Stop", "STOP"),
							FOnClicked::CreateUObject(this, &UT66CasinoVendorTabWidget::OnStealStop),
							ET66ButtonType::Primary)
						.SetMinWidth(0.f)
						.SetPadding(FMargin(22.f, 12.f))
					)
				]
			,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(18.f).SetColor(FT66FlatStyle::Tokens::Panel2))
		];

	auto MakeTitle = [&](const FText& Text) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(Text)
			.TextStyle(&TextTitle)
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text);
	};

	TSharedRef<SHorizontalBox> SellRow = SNew(SHorizontalBox);
	for (int32 Inv = 0; Inv < InventorySlotCount; ++Inv)
	{
		TSharedRef<SWidget> SellBtnWidget = FT66FlatStyle::MakeFlatToggleGroupButton(
			ET66FlatState::Selected,
				SAssignNew(InventorySlotActionTexts[Inv], STextBlock)
				.Text(Loc ? Loc->GetText_Sell() : NSLOCTEXT("T66.Common", "Sell", "SELL"))
				.Font(FT66FlatStyle::MakeBoldFont(CardButtonFontSize))
				.ColorAndOpacity(FT66FlatStyle::SelectedText()),
			FOnClicked::CreateUObject(this, &UT66CasinoVendorTabWidget::OnSellSlotClicked, Inv),
			ShopButtonPadding,
			CardButtonMinWidth,
			CompactPx(36.f),
			true,
			FName(*FString::Printf(TEXT("Vendor.SellCard.%02d.SellButton"), Inv + 1)));
		InventorySlotButtons[Inv] = SellBtnWidget;

		SellRow->AddSlot()
			.AutoWidth()
			.Padding(Inv > 0 ? FMargin(ShopCardGap, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			SNew(SBox)
			.WidthOverride(ShopCardSize)
			.HeightOverride(ShopCardHeight)
			[
				FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Default,
					FMargin(ShopCardPadding),
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBox)
						.HeightOverride(ShopNameBoxHeight)
						[
							SAssignNew(InventorySlotTexts[Inv], STextBlock)
							.Text(NSLOCTEXT("T66.Common", "Empty", "EMPTY"))
							.TextStyle(&TextHeading)
							.Font(FT66FlatStyle::Tokens::FontBold(CardHeadingFontSize))
							.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
							.AutoWrapText(true)
							.WrapTextAt(ShopCardSize - ShopCardPadding * 2.f)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66FlatStyle::Tokens::Space2, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)
						[
							FT66FlatStyle::MakeFlatPanel(
								ET66FlatState::Default,
								FMargin(0.f),
								SNew(SBox)
								.WidthOverride(ShopIconSize)
								.HeightOverride(ShopIconSize)
								[
									SNew(SOverlay)
									+ SOverlay::Slot()
									[
										FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(
											SAssignNew(InventorySlotIconImages[Inv], SImage)
											.Image(InventorySlotIconBrushes[Inv].Get())
											.ColorAndOpacity(FLinearColor::White)),
											FName(*FString::Printf(TEXT("Vendor.SellCard.%02d.Icon"), Inv + 1)),
											TEXT("Icon"))
									]
									+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 8.f, 10.f, 0.f)
									[
										SAssignNew(InventorySlotCountTexts[Inv], STextBlock)
										.Text(FText::GetEmpty())
										.Font(FT66FlatStyle::Tokens::FontBold(InventoryCountFontSize))
										.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
										.ShadowOffset(FVector2D(1.f, 1.f))
										.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f))
										.Visibility(EVisibility::Hidden)
									]
								],
								nullptr,
								FName(*FString::Printf(TEXT("Vendor.SellCard.%02d.IconPanel"), Inv + 1)))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66FlatStyle::Tokens::Space2, 0.f, 0.f)
					[
						SAssignNew(InventorySlotDescTexts[Inv], STextBlock)
						.Text(FText::GetEmpty())
						.TextStyle(&TextBody)
						.Font(FT66FlatStyle::Tokens::FontRegular(CardBodyFontSize))
						.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
						.AutoWrapText(true)
						.WrapTextAt(ShopCardSize - ShopCardPadding * 2.f)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66FlatStyle::Tokens::Space3, 0.f, 0.f)
					[
						SellBtnWidget
					]
					,
					&InventorySlotBorders[Inv],
					FName(*FString::Printf(TEXT("Vendor.SellCard.%02d.Panel"), Inv + 1)))
			]
		];
	}

	TSharedRef<SWidget> ShopCardsScroller =
		SNew(SScrollBox)
		.Orientation(Orient_Horizontal)
		.ScrollBarVisibility(bCompactCasinoLayout ? EVisibility::Collapsed : EVisibility::Visible)
		+ SScrollBox::Slot()
		[
			ShopRow
		];

	TSharedRef<SWidget> BuybackCardsScroller =
		SNew(SScrollBox)
		.Orientation(Orient_Horizontal)
		.ScrollBarVisibility(bCompactCasinoLayout ? EVisibility::Collapsed : EVisibility::Visible)
		+ SScrollBox::Slot()
		[
			BuybackRow
		];

	TSharedRef<SWidget> SellCardsScroller =
		SNew(SScrollBox)
		.Orientation(Orient_Horizontal)
		.ScrollBarVisibility(bCompactCasinoLayout ? EVisibility::Collapsed : EVisibility::Visible)
		+ SScrollBox::Slot()
		[
			SellRow
		];

	TSharedRef<SUniformGridPanel> InventoryGrid = SNew(SUniformGridPanel);
	TSharedRef<SWidget> SellBtnWidget = SNullWidget::NullWidget;
	SellItemButton = SellBtnWidget;

	const FText BuyTitle = Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY");
	const FText SellTitle = Loc ? Loc->GetText_Sell() : NSLOCTEXT("T66.Common", "Sell", "SELL");
	const FName ShopModeToggleGroup(TEXT("Vendor.ShopMode"));
	auto MakeShopModeButton = [&](const EShopMode Mode, const FText& Label, const FName WidgetTag) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatToggleGroupButton(
			Mode == ActiveShopMode ? ET66FlatState::Selected : ET66FlatState::Default,
			SNew(STextBlock)
				.Text(Label)
				.Font(FT66FlatStyle::MakeBoldFont(bCompactCasinoLayout ? 22 : 16))
				.ColorAndOpacity(Mode == ActiveShopMode ? FT66FlatStyle::SelectedText() : FT66FlatStyle::DefaultText()),
			FOnClicked::CreateLambda([this, Mode]()
			{
				SetShopMode(Mode);
				return FReply::Handled();
			}),
			bCompactCasinoLayout ? FMargin(12.f, 8.f) : FMargin(12.f, 8.f),
			0.f,
			0.f,
			true,
			WidgetTag,
			ShopModeToggleGroup);
	};

	TSharedRef<SWidget> BuyModeButton = MakeShopModeButton(EShopMode::Buy, BuyTitle, FName(TEXT("Vendor.ShopMode.BuyButton")));
	TSharedRef<SWidget> SellModeButton = MakeShopModeButton(EShopMode::Sell, SellTitle, FName(TEXT("Vendor.ShopMode.SellButton")));
	TSharedRef<SWidget> BuybackModeButton = MakeShopModeButton(EShopMode::Buyback, BuybackTitle, FName(TEXT("Vendor.ShopMode.BuybackButton")));

	TSharedRef<SWidget> ContextRerollButton = FT66FlatStyle::MakeFlatButton(
			ET66FlatState::Default,
			RerollText,
			FOnClicked::CreateUObject(this, &UT66CasinoVendorTabWidget::OnReroll),
			nullptr,
			nullptr,
			ActionButtonPadding,
			0.f,
			0.f,
			true,
			bCompactCasinoLayout ? 22 : 16,
			FName(TEXT("Vendor.ShopMode.RerollButton")));
	ContextRerollButtonWidget = ContextRerollButton;

	auto MakeInventoryRotationControls = [&](const int32 FontSize, const FName NamePrefix) -> TSharedRef<SWidget>
	{
		TSharedRef<SWidget> RotateButton = FT66FlatStyle::MakeFlatButton(
			ET66FlatState::Default,
			NSLOCTEXT("T66.Shop", "NextSellSlots", "NEXT"),
			FOnClicked::CreateUObject(this, &UT66CasinoVendorTabWidget::OnRotateInventorySlotsClicked),
			nullptr,
			nullptr,
			FMargin(10.f, 6.f),
			0.f,
			0.f,
			true,
			FontSize,
			FName(*FString::Printf(TEXT("%s.NextButton"), *NamePrefix.ToString())));
		InventoryRotateButtonWidget = RotateButton;

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
			[
				SAssignNew(InventoryPageText, STextBlock)
				.Text(NSLOCTEXT("T66.Shop", "SellSlotPageInitial", "1/1"))
				.TextStyle(&TextBody)
				.Font(FT66FlatStyle::Tokens::FontBold(FontSize))
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				RotateButton
			];
	};

	if (bCompactCasinoLayout)
	{
		TSharedRef<SWidget> ShopButtonWidget = MakeVendorFlatButton(
			ET66FlatState::Selected,
			ShopTitle,
			FOnClicked::CreateLambda([this]()
			{
				if (ShopBuybackSwitcher.IsValid())
				{
					ShopBuybackSwitcher->SetActiveWidgetIndex(0);
				}
				RefreshStock();
				RefreshShopChrome();
				return FReply::Handled();
			}),
			FMargin(12.f, 8.f),
			22,
			FName(TEXT("Vendor.ShopMode.ShopButton")),
			ShopModeToggleGroup);

		TSharedRef<SWidget> BankPanel =
			FT66FlatStyle::MakeFlatPanel(
				ET66FlatState::Default,
				FMargin(CompactPx(22.f), CompactPx(18.f)),
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, CompactPx(18.f))
				[
					MakeVendorLabel(BankTitle, 24, FName(TEXT("Vendor.Bank.Title")))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, CompactPx(12.f))
				[
					MakeVendorLabel(NSLOCTEXT("T66.Shop", "BorrowAmountLabel", "Borrow amount"), 16, FName(TEXT("Vendor.Bank.BorrowLabel")), FT66FlatStyle::SecondaryText())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, CompactPx(22.f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, CompactPx(22.f), 0.f)
					[
						SNew(SBox)
						.WidthOverride(BankSpinBoxWidth)
						.HeightOverride(BankSpinBoxHeight)
						[
							SAssignNew(BorrowAmountSpin, SSpinBox<int32>)
							.MinValue(0).MaxValue(999999).Delta(10)
							.Font(FT66FlatStyle::MakeBoldFont(SpinBoxFontSize))
							.Value(BorrowAmount)
							.OnValueChanged_Lambda([this](int32 V)
							{
								int32 MaxBorrow = TNumericLimits<int32>::Max();
								if (UT66RunStateSubsystem* RunState = GetRunStateFromWorld(GetWorld()))
								{
									MaxBorrow = RunState->GetRemainingBorrowCapacity();
								}
								BorrowAmount = FMath::Clamp(V, 0, FMath::Max(0, MaxBorrow));
							})
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						FT66FlatStyle::MakeFlatButton(
							ET66FlatState::Default,
							Loc ? Loc->GetText_Borrow() : NSLOCTEXT("T66.Shop", "Borrow_Button", "BORROW"),
							FOnClicked::CreateUObject(this, &UT66CasinoVendorTabWidget::OnBorrowClicked),
							nullptr,
							nullptr,
							FMargin(12.f, 8.f),
							0.f,
							CompactPx(52.f),
							true,
							16,
							FName(TEXT("Vendor.Bank.BorrowButton")))
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, CompactPx(12.f))
				[
					MakeVendorLabel(NSLOCTEXT("T66.Shop", "PaybackAmountLabel", "Payback amount"), 16, FName(TEXT("Vendor.Bank.PaybackLabel")), FT66FlatStyle::SecondaryText())
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, CompactPx(22.f), 0.f)
					[
						SNew(SBox)
						.WidthOverride(BankSpinBoxWidth)
						.HeightOverride(BankSpinBoxHeight)
						[
							SAssignNew(PaybackAmountSpin, SSpinBox<int32>)
							.MinValue(0).MaxValue(999999).Delta(10)
							.Font(FT66FlatStyle::MakeBoldFont(SpinBoxFontSize))
							.Value(PaybackAmount)
							.OnValueChanged_Lambda([this](int32 V) { PaybackAmount = FMath::Max(0, V); })
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						FT66FlatStyle::MakeFlatButton(
							ET66FlatState::Default,
							Loc ? Loc->GetText_Payback() : NSLOCTEXT("T66.Shop", "Payback_Button", "PAYBACK"),
							FOnClicked::CreateUObject(this, &UT66CasinoVendorTabWidget::OnPaybackClicked),
							nullptr,
							nullptr,
							FMargin(12.f, 8.f),
							0.f,
							CompactPx(52.f),
							true,
							16,
							FName(TEXT("Vendor.Bank.PaybackButton")))
					]
				],
				nullptr,
				FName(TEXT("Vendor.BankPanel")));

		TSharedRef<SWidget> InventoryPanel =
			FT66FlatStyle::MakeFlatPanel(
				ET66FlatState::Default,
				FMargin(18.f, 18.f),
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						MakeVendorLabel(InventoryTitle, 28, FName(TEXT("Vendor.Inventory.Title")))
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(34.f, 0.f, 32.f, 0.f)
					[
						FT66FlatStyle::AttachMetadata(
							SAssignNew(NetWorthText, STextBlock)
								.Text(FText::Format(
									Loc ? Loc->GetText_NetWorthFormat() : NSLOCTEXT("T66.GameplayHUD", "NetWorthFormat", "Net Worth: {0}"),
									FText::AsNumber(0)))
								.Font(FT66FlatStyle::MakeBoldFont(18))
								.ColorAndOpacity(FT66FlatStyle::GoodStandingGreen()),
							FName(TEXT("Vendor.Inventory.NetWorth")),
							TEXT("Label.Body"),
							ET66FlatState::Default,
							TOptional<FLinearColor>(),
							false,
							NAME_None,
							true)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 32.f, 0.f)
					[
						FT66FlatStyle::AttachMetadata(
							SAssignNew(GoldText, STextBlock)
								.Text(FText::Format(
									Loc ? Loc->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}"),
									FText::AsNumber(0)))
								.Font(FT66FlatStyle::MakeBoldFont(18))
								.ColorAndOpacity(FT66FlatStyle::PrimaryText()),
							FName(TEXT("Vendor.Inventory.Gold")),
							TEXT("Label.Body"),
							ET66FlatState::Default,
							TOptional<FLinearColor>(),
							false,
							NAME_None,
							true)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						FT66FlatStyle::AttachMetadata(
							SAssignNew(DebtText, STextBlock)
								.Text(FText::Format(
									Loc ? Loc->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Debt: {0}"),
									FText::AsNumber(0)))
								.Font(FT66FlatStyle::MakeBoldFont(18))
								.ColorAndOpacity(FT66FlatStyle::SelectedText()),
							FName(TEXT("Vendor.Inventory.Debt")),
							TEXT("Label.Body"),
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
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						MakeInventoryRotationControls(16, FName(TEXT("Vendor.Inventory.CompactRotate")))
					]
				]
				+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 22.f, 0.f, 0.f)
				[
					SNew(SScrollBox)
					.Orientation(Orient_Horizontal)
					.ScrollBarVisibility(EVisibility::Collapsed)
					+ SScrollBox::Slot()
					[
						InventoryGrid
					]
				],
				nullptr,
				FName(TEXT("Vendor.InventoryPanel")));

		TSharedRef<SConstraintCanvas> VendorCanvas = SNew(SConstraintCanvas);
		AddVendorShopCanvasSlot(VendorCanvas, 17.f, 182.f, 270.f, 651.f,
			MakeVendorReferenceStatsPanel(RunState));
		AddVendorShopCanvasSlot(VendorCanvas, 312.f, 185.f, 318.f, 57.f, BuyModeButton);
		AddVendorShopCanvasSlot(VendorCanvas, 653.f, 185.f, 318.f, 57.f, SellModeButton);
		AddVendorShopCanvasSlot(VendorCanvas, 986.f, 185.f, 306.f, 57.f, BuybackModeButton);
		AddVendorShopCanvasSlot(VendorCanvas, 310.f, 262.f, 1226.f, 568.f,
			FT66FlatStyle::MakeFlatPanel(
				ET66FlatState::Default,
				FMargin(18.f),
				SAssignNew(ShopBuybackSwitcher, SWidgetSwitcher)
				+ SWidgetSwitcher::Slot()
				[
					ShopCardsScroller
				]
				+ SWidgetSwitcher::Slot()
				[
					SellCardsScroller
				]
				+ SWidgetSwitcher::Slot()
				[
					BuybackCardsScroller
				],
				nullptr,
				FName(TEXT("Vendor.ShopCardsPanel"))));
		AddVendorShopCanvasSlot(VendorCanvas, 1564.f, 526.f, 355.f, 309.f, BankPanel);
		AddVendorShopCanvasSlot(VendorCanvas, 310.f, 842.f, 306.f, 57.f, ContextRerollButton);

		SetPage(EShopPage::Shop);
		SetShopMode(EShopMode::Buy);
		RefreshAll();
		RefreshShopChrome();
		return FT66FlatStyle::AttachMetadata(VendorCanvas, FName(TEXT("Vendor.Root")), TEXT("Overlay"), ET66FlatState::Default);
	}

	// Build main 3-column row (Stats | Shop | Bank) as a separate widget to avoid Slate parser issues with SBox::FArguments.
	TSharedRef<SWidget> MainRowContent = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space6, 0.f)
		[
			SAssignNew(StatsPanelBox, SBox)
			.WidthOverride(StatsPanelWidth)
			.HeightOverride(MainRowHeight)
			[
				T66StatsPanelSlate::MakeLiveEssentialStatsPanel(RunState, Loc, LiveStatsPanel.ToSharedRef(), StatsPanelWidth, true, StatsPanelFontAdjustment)
			]
		]
		+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space6, 0.f)
		[
			SNew(SBox)
			.MinDesiredHeight(MainRowHeight)
			[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66FlatStyle::Tokens::Space4, 0.f)
						[
							BuyModeButton
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66FlatStyle::Tokens::Space4, 0.f)
						[
							SellModeButton
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66FlatStyle::Tokens::Space4, 0.f)
						[
							BuybackModeButton
						]
						+ SHorizontalBox::Slot().AutoWidth()
						[
							ContextRerollButton
						]
					]
					+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, FT66FlatStyle::Tokens::Space4, 0.f, 0.f)
					[
						FT66FlatStyle::MakeFlatOverlayPanel(
							SAssignNew(ShopBuybackSwitcher, SWidgetSwitcher)
							+ SWidgetSwitcher::Slot()
							[
								ShopCardsScroller
							]
							+ SWidgetSwitcher::Slot()
							[
								SellCardsScroller
							]
							+ SWidgetSwitcher::Slot()
							[
								BuybackCardsScroller
							],
							ET66FlatOverlayChromeBrush::ContentPanelWide,
							FMargin(FT66FlatStyle::Tokens::Space6))
					]
					]
				]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 0.f, 0.f)
		[
			SNew(SBox)
			.WidthOverride(RightPanelWidth)
			.MinDesiredHeight(MainRowHeight)
			[
				FT66FlatStyle::MakeFlatOverlayPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						SNew(SSpacer)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 0.f)
					[
						FT66FlatStyle::MakeFlatOverlayPanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, FT66FlatStyle::Tokens::Space4)
							[
								SNew(STextBlock)
								.Text(BankTitle)
								.TextStyle(&TextHeading)
								.Font(FT66FlatStyle::Tokens::FontBold(SectionHeadingFontSize))
								.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
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
											if (UT66RunStateSubsystem* RunState = GetRunStateFromWorld(GetWorld()))
											{
												MaxBorrow = RunState->GetRemainingBorrowCapacity();
											}
											BorrowAmount = FMath::Clamp(V, 0, FMath::Max(0, MaxBorrow));
										})
									]
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									FT66FlatStyle::MakeFlatOverlayButton(
										FT66FlatStyle::MakeFlatOverlayButtonParams(
											Loc ? Loc->GetText_Borrow() : NSLOCTEXT("T66.Shop", "Borrow_Button", "BORROW"),
											FOnClicked::CreateUObject(this, &UT66CasinoVendorTabWidget::OnBorrowClicked),
											ET66FlatOverlayChromeButtonFamily::Neutral)
										.SetMinWidth(0.f)
										.SetPadding(ActionButtonPadding)
										.SetFontSize(CardButtonFontSize)
									)
								]
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
									FT66FlatStyle::MakeFlatOverlayButton(
										FT66FlatStyle::MakeFlatOverlayButtonParams(
											Loc ? Loc->GetText_Payback() : NSLOCTEXT("T66.Shop", "Payback_Button", "PAYBACK"),
											FOnClicked::CreateUObject(this, &UT66CasinoVendorTabWidget::OnPaybackClicked),
											ET66FlatOverlayChromeButtonFamily::Neutral)
										.SetMinWidth(0.f)
										.SetPadding(ActionButtonPadding)
										.SetFontSize(CardButtonFontSize)
									)
								]
							]
						,
							ET66FlatOverlayChromeBrush::InnerPanel,
							FMargin(bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space5))
					]
				,
					ET66FlatOverlayChromeBrush::ContentPanelTall,
					FMargin(bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space6))
			]
		];

	TSharedRef<SWidget> ShopPageBody =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SAssignNew(ShopPageTitleText, STextBlock)
				.Text(ShopTitle)
				.TextStyle(&TextTitle)
				.Font(FT66FlatStyle::Tokens::FontBold(PageTitleFontSize))
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
				.Visibility(bCompactCasinoLayout ? EVisibility::Collapsed : EVisibility::Visible)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, bCompactCasinoLayout ? 0.f : 12.f, 0.f, 0.f)
		[
			SAssignNew(StatusText, STextBlock)
			.Text(FText::GetEmpty())
			.TextStyle(&TextBody)
			.Font(FT66FlatStyle::Tokens::FontRegular(StatusFontSize))
			.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space4 : FT66FlatStyle::Tokens::Space6, 0.f, 0.f)
		[
			MainRowContent
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space4 : FT66FlatStyle::Tokens::Space6, 0.f, 0.f)
		[
			FT66FlatStyle::MakeFlatOverlayPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(InventoryTitle)
								.TextStyle(&TextHeading)
								.Font(FT66FlatStyle::Tokens::FontBold(SectionHeadingFontSize))
								.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
							]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(bCompactCasinoLayout ? 10.f : 18.f, 0.f, bCompactCasinoLayout ? 10.f : 16.f, 0.f)
					[
						SAssignNew(NetWorthText, STextBlock)
						.Text(FText::Format(
							Loc ? Loc->GetText_NetWorthFormat() : NSLOCTEXT("T66.GameplayHUD", "NetWorthFormat", "Net Worth: {0}"),
							FText::AsNumber(0)))
						.TextStyle(&TextHeading)
						.Font(FT66FlatStyle::Tokens::FontBold(SectionHeadingFontSize))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, bCompactCasinoLayout ? 10.f : 16.f, 0.f)
					[
						SAssignNew(GoldText, STextBlock)
						.Text(FText::Format(
							Loc ? Loc->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}"),
							FText::AsNumber(0)))
						.TextStyle(&TextHeading)
						.Font(FT66FlatStyle::Tokens::FontBold(SectionHeadingFontSize))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SAssignNew(DebtText, STextBlock)
						.Text(FText::Format(
							Loc ? Loc->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Debt: {0}"),
							FText::AsNumber(0)))
						.TextStyle(&TextHeading)
						.Font(FT66FlatStyle::Tokens::FontBold(SectionHeadingFontSize))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Danger)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(SSpacer)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						MakeInventoryRotationControls(SectionHeadingFontSize, FName(TEXT("Vendor.Inventory.Rotate")))
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
							InventoryGrid
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space6, 0.f, 0.f, 0.f)
					[
						// Sell details for selected item (sized to match inventory slot: 160x160)
						SAssignNew(SellPanelContainer, SBox)
						.WidthOverride(SellPanelSize)
						.HeightOverride(SellPanelSize)
						.Visibility(EVisibility::Visible)
						[
							FT66FlatStyle::MakeFlatOverlayPanel(
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
								[ SellBtnWidget ]
							,
								ET66FlatOverlayChromeBrush::InnerPanel,
								FMargin(bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space4))
						]
					]
				]
			,
				ET66FlatOverlayChromeBrush::ContentPanelWide,
				FMargin(bCompactCasinoLayout ? FT66FlatStyle::Tokens::Space3 : FT66FlatStyle::Tokens::Space4))
		];

	const TAttribute<FOptionalSize> ShopPageWidthAttr = TAttribute<FOptionalSize>::CreateLambda([this, OverlayPadding]() -> FOptionalSize
	{
		const FVector2D Bounds = bEmbeddedInCasinoShell ? FT66FlatStyle::GetViewportLogicalSize() : FT66FlatStyle::GetSafeFrameSize();
		const float HorizontalMargins = bEmbeddedInCasinoShell
			? (OverlayPadding * 4.f)
			: (OverlayPadding * 2.f);
		return FOptionalSize(FMath::Max(1.f, Bounds.X - HorizontalMargins));
	});

	TSharedRef<SWidget> ShopPage =
		SNew(SScrollBox)
		.Orientation(Orient_Vertical)
		+ SScrollBox::Slot()
		[
			SNew(SBox)
			.WidthOverride(ShopPageWidthAttr)
			[
				ShopPageBody
			]
		];

	const TAttribute<FMargin> SafeContentInsets = TAttribute<FMargin>::CreateLambda([this]() -> FMargin
	{
		return bEmbeddedInCasinoShell ? FMargin(0.f) : FT66FlatStyle::GetSafeFrameInsets();
	});

	const TAttribute<FMargin> SafeClosePadding = TAttribute<FMargin>::CreateLambda([this, OverlayPadding]() -> FMargin
	{
		const FMargin LocalPadding(
			OverlayPadding,
			OverlayPadding,
			OverlayPadding,
			0.f);
		return bEmbeddedInCasinoShell ? LocalPadding : FT66FlatStyle::GetSafePadding(LocalPadding);
	});

	TSharedRef<SWidget> Root =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			FT66FlatStyle::MakeFlatOverlayPanel(
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.Padding(SafeContentInsets)
				[
					SAssignNew(PageSwitcher, SWidgetSwitcher)
					+ SWidgetSwitcher::Slot()
					[
						ShopPage
					]
				]
			,
				ET66FlatOverlayChromeBrush::ContentPanelWide,
				FMargin(OverlayPadding))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(SafeClosePadding)
		[
			SAssignNew(CloseButtonBox, SBox)
			.Visibility(EVisibility::Visible)
			[
				FT66FlatStyle::MakeFlatOverlayButton(
					FT66FlatStyle::MakeFlatOverlayButtonParams(CloseText,
						FOnClicked::CreateUObject(this, &UT66CasinoVendorTabWidget::OnBack),
						ET66FlatOverlayChromeButtonFamily::Danger)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(20.f, 12.f))
				)
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			StealPromptWidget
		];

	SetPage(EShopPage::Shop);
	RefreshAll();
	RefreshShopChrome();
	return bEmbeddedInCasinoShell ? Root : FT66FlatStyle::MakeResponsiveRoot(Root);
}

void UT66CasinoVendorTabWidget::HandleGoldOrDebtChanged()
{
	RefreshTopBar();
}

void UT66CasinoVendorTabWidget::HandleInventoryChanged()
{
	PrimeVisibleItemIconTextures();
	RefreshTopBar();
	RefreshInventory();
	RefreshSellPanel();
	RefreshStatsPanel();
}

void UT66CasinoVendorTabWidget::HandleShopChanged()
{
	PrimeVisibleItemIconTextures();
	RefreshShopChrome();
	RefreshTopBar();
	RefreshStock();
}

void UT66CasinoVendorTabWidget::HandleBuybackChanged()
{
	PrimeVisibleItemIconTextures();
	RefreshShopChrome();
	RefreshTopBar();
	RefreshBuyback();
}

void UT66CasinoVendorTabWidget::RefreshAll()
{
	PrimeVisibleItemIconTextures();
	RefreshShopChrome();
	RefreshTopBar();
	RefreshStock();
	RefreshBuyback();
	RefreshInventory();
	RefreshSellPanel();
	RefreshStatsPanel();
}

void UT66CasinoVendorTabWidget::RefreshShopChrome()
{
	UWorld* World = GetWorld();
	UT66LocalizationSubsystem* Loc = World && World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>()
		: nullptr;
	const FText BuyTitle = Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY");
	const FText SellTitle = Loc ? Loc->GetText_Sell() : NSLOCTEXT("T66.Common", "Sell", "SELL");
	const FText BuybackTitle = NSLOCTEXT("T66.Shop", "Buyback", "BUYBACK");
	if (ShopBuybackSwitcher.IsValid())
	{
		const int32 ActiveIndex = ShopBuybackSwitcher->GetActiveWidgetIndex();
		if (ActiveIndex >= 0 && ActiveIndex <= static_cast<int32>(EShopMode::Buyback))
		{
			ActiveShopMode = static_cast<EShopMode>(ActiveIndex);
		}
	}

	if (ShopPageTitleText.IsValid())
	{
		switch (ActiveShopMode)
		{
		case EShopMode::Sell:
			ShopPageTitleText->SetText(SellTitle);
			break;
		case EShopMode::Buyback:
			ShopPageTitleText->SetText(BuybackTitle);
			break;
		case EShopMode::Buy:
		default:
			ShopPageTitleText->SetText(BuyTitle);
			break;
		}
	}

	if (ContextRerollButtonWidget.IsValid())
	{
		UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
		const bool bShowingBuyback = ActiveShopMode == EShopMode::Buyback;
		const bool bShowingSell = ActiveShopMode == EShopMode::Sell;
		const bool bEnabled = RunState
			&& !bShowingSell
			&& (bShowingBuyback
				? (RunState->GetBuybackPoolSize() > UT66RunStateSubsystem::BuybackDisplaySlotCount)
				: !bCachedBossActive);
		ContextRerollButtonWidget->SetVisibility(bShowingSell ? EVisibility::Collapsed : EVisibility::Visible);
		ContextRerollButtonWidget->SetEnabled(bEnabled);
	}

	if (CloseButtonBox.IsValid())
	{
		CloseButtonBox->SetVisibility(bEmbeddedInCasinoShell ? EVisibility::Collapsed : EVisibility::Visible);
	}
}

void UT66CasinoVendorTabWidget::PrimeVisibleItemIconTextures()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	if (!RunState || !GI || !TexPool)
	{
		return;
	}

	TArray<FSoftObjectPath> IconPaths;
	IconPaths.Reserve(
		UT66RunStateSubsystem::ShopDisplaySlotCount +
		UT66RunStateSubsystem::BuybackDisplaySlotCount +
		GetVisibleSellSlotCount());

	const TArray<FT66InventorySlot>& StockSlots = RunState->GetShopStockSlots();
	for (const FT66InventorySlot& StockSlot : StockSlots)
	{
		if (!StockSlot.IsValid())
		{
			continue;
		}

		AddItemIconPath(GI, StockSlot.ItemTemplateID, StockSlot.Rarity, IconPaths);
	}

	const TArray<FT66InventorySlot>& BuybackSlots = RunState->GetBuybackDisplaySlots();
	for (const FT66InventorySlot& BuybackSlot : BuybackSlots)
	{
		if (!BuybackSlot.IsValid())
		{
			continue;
		}

		AddItemIconPath(GI, BuybackSlot.ItemTemplateID, BuybackSlot.Rarity, IconPaths);
	}

	const TArray<FName>& Inventory = RunState->GetInventory();
	const TArray<FT66InventorySlot>& InventorySlots = RunState->GetInventorySlots();
	const bool bHasMobLootStack = RunState->GetCollectedMobLootStack() > 0;
	if (bHasMobLootStack)
	{
		AddItemIconPath(GI, T66MobLootItemID, ET66ItemRarity::Black, IconPaths);
	}
	const int32 EntryCount = GetSellEntryCount(Inventory, bHasMobLootStack);
	const int32 MaxPage = EntryCount > 0 ? (EntryCount - 1) / FMath::Max(1, GetVisibleSellSlotCount()) : 0;
	SellInventoryPageIndex = FMath::Clamp(SellInventoryPageIndex, 0, MaxPage);
	for (int32 DisplaySlot = 0; DisplaySlot < GetVisibleSellSlotCount(); ++DisplaySlot)
	{
		const int32 EntryIndex = SellInventoryPageIndex * GetVisibleSellSlotCount() + DisplaySlot;
		const int32 InventoryIndex = ResolveSellEntryToInventoryIndex(Inventory, bHasMobLootStack, EntryIndex);
		if (InventoryIndex < 0 || !InventorySlots.IsValidIndex(InventoryIndex) || !InventorySlots[InventoryIndex].IsValid())
		{
			continue;
		}

		const FT66InventorySlot& InventorySlot = InventorySlots[InventoryIndex];
		AddItemIconPath(GI, InventorySlot.ItemTemplateID, InventorySlot.Rarity, IconPaths);
	}

	if (IconPaths.Num() > 0)
	{
		for (const FSoftObjectPath& IconPath : IconPaths)
		{
			if (!IconPath.IsValid())
			{
				continue;
			}

			TexPool->RequestTexture(TSoftObjectPtr<UTexture2D>(IconPath), this, [](UTexture2D*) {});
		}
	}
}

void UT66CasinoVendorTabWidget::RefreshStatsPanel()
{
	if (!LiveStatsPanel.IsValid()) return;
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	LiveStatsPanel->Update(RunState, Loc);
}

void UT66CasinoVendorTabWidget::RefreshTopBar()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return;

	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	const int32 BorrowCapacity = RunState->GetRemainingBorrowCapacity();
	BorrowAmount = FMath::Clamp(BorrowAmount, 0, BorrowCapacity);
	PaybackAmount = FMath::Max(0, PaybackAmount);

	if (BorrowAmountSpin.IsValid())
	{
		BorrowAmountSpin->SetValue(BorrowAmount);
	}
	if (PaybackAmountSpin.IsValid())
	{
		PaybackAmountSpin->SetValue(PaybackAmount);
	}

	if (NetWorthText.IsValid())
	{
		const int32 NetWorth = RunState->GetNetWorth();
		const FText Fmt = Loc ? Loc->GetText_NetWorthFormat() : NSLOCTEXT("T66.GameplayHUD", "NetWorthFormat", "Net Worth: {0}");
		NetWorthText->SetText(FText::Format(Fmt, FText::AsNumber(NetWorth)));
		const FLinearColor NetWorthColor = NetWorth > 0
			? FT66FlatStyle::Tokens::Success
			: (NetWorth < 0 ? FT66FlatStyle::Tokens::Danger : FT66FlatStyle::Tokens::Text);
		NetWorthText->SetColorAndOpacity(NetWorthColor);
	}
	if (GoldText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}");
		GoldText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentGold())));
	}
	if (DebtText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Debt: {0}");
		DebtText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentDebt())));
	}

}

FReply UT66CasinoVendorTabWidget::OnReroll()
{
	if (UWorld* World = GetWorld())
	{
		if (UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World))
		{
			if (ActiveShopMode == EShopMode::Sell)
			{
				return FReply::Handled();
			}
			if (ActiveShopMode == EShopMode::Buyback)
			{
				RunState->RerollBuybackDisplay();
				RefreshBuyback();
				return FReply::Handled();
			}

			if (IsBossActive())
			{
				return FReply::Handled();
			}

			RunState->RerollShopStockForCurrentStage();
		}
	}
	RefreshAll();
	return FReply::Handled();
}

void UT66CasinoVendorTabWidget::RefreshStock()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return;

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	const TArray<FName>& Stock = RunState->GetShopStockItemIDs();
	const TArray<FT66InventorySlot>& StockSlots = RunState->GetShopStockSlots();
	const int32 SlotCount = ItemNameTexts.Num();
	for (int32 i = 0; i < SlotCount; ++i)
	{
		const bool bHasItem = Stock.IsValidIndex(i) && !Stock[i].IsNone();
		const bool bSold = bHasItem ? RunState->IsShopStockSlotSold(i) : true;
		FItemData D;
		const bool bHasData = bHasItem && GI && GI->GetItemData(Stock[i], D);
		const ET66ItemRarity SlotRarity = StockSlots.IsValidIndex(i) ? StockSlots[i].Rarity : ET66ItemRarity::Black;

		if (ItemNameTexts.IsValidIndex(i) && ItemNameTexts[i].IsValid())
		{
			ItemNameTexts[i]->SetText(bHasItem
				? (Loc ? Loc->GetText_ItemDisplayNameForRarity(Stock[i], SlotRarity) : FText::FromName(Stock[i]))
				: NSLOCTEXT("T66.Common", "Empty", "EMPTY"));
		}
		if (ItemDescTexts.IsValidIndex(i) && ItemDescTexts[i].IsValid())
		{
			if (!bHasData)
			{
				ItemDescTexts[i]->SetText(FText::GetEmpty());
			}
			else
			{
				const int32 MainValue = StockSlots.IsValidIndex(i) ? StockSlots[i].Line1RolledValue : 0;
				const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f;
				ItemDescTexts[i]->SetText(T66ItemCardTextUtils::BuildItemCardDescription(Loc, D, SlotRarity, MainValue, ScaleMult, StockSlots.IsValidIndex(i) ? StockSlots[i].GetLine2Multiplier() : 0.f));
			}
		}
		if (ItemIconBorders.IsValidIndex(i) && ItemIconBorders[i].IsValid())
		{
			ItemIconBorders[i]->SetBorderBackgroundColor(bHasData ? FItemData::GetItemRarityColor(SlotRarity) : FT66FlatStyle::Tokens::Panel2);
		}
		if (ItemIconBrushes.IsValidIndex(i) && ItemIconBrushes[i].IsValid())
		{
			const TSoftObjectPtr<UTexture2D> SlotIconSoft = bHasData ? D.GetIconForRarity(SlotRarity) : TSoftObjectPtr<UTexture2D>();
			if (!SlotIconSoft.IsNull() && TexPool)
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, SlotIconSoft, this, ItemIconBrushes[i], FName(TEXT("ShopStock"), i + 1), /*bClearWhileLoading*/ false);
			}
			else
			{
				ItemIconBrushes[i]->SetResourceObject(nullptr);
			}
		}
		if (ItemIconImages.IsValidIndex(i) && ItemIconImages[i].IsValid())
		{
			const bool bHasIcon = bHasData && !D.GetIconForRarity(SlotRarity).IsNull();
			ItemIconImages[i]->SetVisibility(bHasIcon ? EVisibility::Visible : EVisibility::Hidden);
		}
		if (ItemTileBorders.IsValidIndex(i) && ItemTileBorders[i].IsValid())
		{
			ItemTileBorders[i]->SetBorderBackgroundColor(bHasItem ? FT66FlatStyle::Tokens::Panel2 : FT66FlatStyle::Tokens::Panel);
		}
		if (BuyButtons.IsValidIndex(i) && BuyButtons[i].IsValid())
		{
			BuyButtons[i]->SetEnabled(bHasItem && !bSold && !IsBossActive());
		}
		if (BuyButtonTexts.IsValidIndex(i) && BuyButtonTexts[i].IsValid())
		{
			if (!bHasItem)
			{
				BuyButtonTexts[i]->SetText(Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY"));
			}
			else if (bSold)
			{
				BuyButtonTexts[i]->SetText(NSLOCTEXT("T66.Shop", "Sold", "SOLD"));
			}
			else
			{
				const int32 Price = RunState->GetBuyGoldForShopStockSlot(i);
				BuyButtonTexts[i]->SetText(FText::Format(
					NSLOCTEXT("T66.Shop", "BuyPriceFormat", "BUY ({0}g)"),
					FText::AsNumber(Price)));
			}
		}
		if (StealButtons.IsValidIndex(i) && StealButtons[i].IsValid())
		{
			StealButtons[i]->SetVisibility(bShopAllowsSteal ? EVisibility::Visible : EVisibility::Collapsed);
			StealButtons[i]->SetEnabled(bShopAllowsSteal && bHasItem && !bSold && bBoughtSomethingThisVisit && !IsBossActive());
		}
	}
}

void UT66CasinoVendorTabWidget::RefreshBuyback()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return;

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	const TArray<FT66InventorySlot>& Slots = RunState->GetBuybackDisplaySlots();
	const int32 SlotCount = BuybackNameTexts.Num();
	for (int32 i = 0; i < SlotCount; ++i)
	{
		const bool bHasSlot = Slots.IsValidIndex(i) && Slots[i].IsValid();
		FItemData D;
		const bool bHasData = bHasSlot && GI && GI->GetItemData(Slots[i].ItemTemplateID, D);
		const ET66ItemRarity SlotRarity = bHasSlot ? Slots[i].Rarity : ET66ItemRarity::Black;
		const int32 SellPrice = (bHasSlot && RunState) ? RunState->GetSellGoldForInventorySlot(Slots[i]) : 0;

		if (BuybackNameTexts.IsValidIndex(i) && BuybackNameTexts[i].IsValid())
		{
			BuybackNameTexts[i]->SetText(bHasSlot
				? (Loc ? Loc->GetText_ItemDisplayNameForRarity(Slots[i].ItemTemplateID, SlotRarity) : FText::FromName(Slots[i].ItemTemplateID))
				: NSLOCTEXT("T66.Common", "Empty", "EMPTY"));
		}
		if (BuybackDescTexts.IsValidIndex(i) && BuybackDescTexts[i].IsValid())
		{
			if (!bHasData)
			{
				BuybackDescTexts[i]->SetText(FText::GetEmpty());
			}
			else
			{
				const int32 MainValue = Slots[i].Line1RolledValue;
				const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f;
				BuybackDescTexts[i]->SetText(T66ItemCardTextUtils::BuildItemCardDescription(Loc, D, SlotRarity, MainValue, ScaleMult, Slots[i].GetLine2Multiplier()));
			}
		}
		if (BuybackIconBorders.IsValidIndex(i) && BuybackIconBorders[i].IsValid())
		{
			BuybackIconBorders[i]->SetBorderBackgroundColor(bHasData ? FItemData::GetItemRarityColor(SlotRarity) : FT66FlatStyle::Tokens::Panel2);
		}
		if (BuybackIconBrushes.IsValidIndex(i) && BuybackIconBrushes[i].IsValid())
		{
			const TSoftObjectPtr<UTexture2D> SlotIconSoft = bHasData ? D.GetIconForRarity(SlotRarity) : TSoftObjectPtr<UTexture2D>();
			if (!SlotIconSoft.IsNull() && TexPool)
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, SlotIconSoft, this, BuybackIconBrushes[i], FName(TEXT("ShopBuyback"), i + 1), /*bClearWhileLoading*/ false);
			}
			else
			{
				BuybackIconBrushes[i]->SetResourceObject(nullptr);
			}
		}
		if (BuybackIconImages.IsValidIndex(i) && BuybackIconImages[i].IsValid())
		{
			const bool bHasIcon = bHasData && !D.GetIconForRarity(SlotRarity).IsNull();
			BuybackIconImages[i]->SetVisibility(bHasIcon ? EVisibility::Visible : EVisibility::Hidden);
		}
		if (BuybackTileBorders.IsValidIndex(i) && BuybackTileBorders[i].IsValid())
		{
			BuybackTileBorders[i]->SetBorderBackgroundColor(FT66FlatStyle::Tokens::Panel2);
		}
		if (BuybackPriceTexts.IsValidIndex(i) && BuybackPriceTexts[i].IsValid())
		{
			BuybackPriceTexts[i]->SetText(bHasSlot
				? FText::Format(NSLOCTEXT("T66.Shop", "BuyPriceFormat", "BUY ({0}g)"), FText::AsNumber(SellPrice > 0 ? SellPrice : 1))
				: (Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY")));
		}
		if (BuybackBuyButtons.IsValidIndex(i) && BuybackBuyButtons[i].IsValid())
		{
			BuybackBuyButtons[i]->SetEnabled(bHasSlot && RunState->GetCurrentGold() >= (SellPrice > 0 ? SellPrice : 1) && RunState->HasInventorySpace());
		}
	}
}

void UT66CasinoVendorTabWidget::SetPage(EShopPage Page)
{
	if (!PageSwitcher.IsValid()) return;
	PageSwitcher->SetActiveWidgetIndex(static_cast<int32>(Page));
	RefreshShopChrome();
}

void UT66CasinoVendorTabWidget::SetShopMode(const EShopMode Mode)
{
	ActiveShopMode = Mode;
	if (ShopBuybackSwitcher.IsValid())
	{
		ShopBuybackSwitcher->SetActiveWidgetIndex(static_cast<int32>(Mode));
	}

	if (Mode == EShopMode::Buy)
	{
		RefreshStock();
	}
	else if (Mode == EShopMode::Sell)
	{
		RefreshInventory();
		RefreshSellPanel();
	}
	else
	{
		if (UT66RunStateSubsystem* RunState = GetRunStateFromWorld(GetWorld()))
		{
			RunState->GenerateBuybackDisplay();
		}
		RefreshBuyback();
	}
	RefreshShopChrome();
}

void UT66CasinoVendorTabWidget::OpenShopPage()
{
	SetPage(EShopPage::Shop);
	SetShopMode(EShopMode::Buy);
	RefreshAll();
}

void UT66CasinoVendorTabWidget::RefreshInventory()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return;

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	const TArray<FName>& Inv = RunState->GetInventory();
	const TArray<FT66InventorySlot>& InvSlots = RunState->GetInventorySlots();
	const int32 MobLootStack = RunState->GetCollectedMobLootStack();
	const bool bHasMobLootStack = MobLootStack > 0;
	TMap<FString, int32> StackCounts;
	for (const FT66InventorySlot& InventorySlotData : InvSlots)
	{
		if (InventorySlotData.IsValid())
		{
			StackCounts.FindOrAdd(MakeShopInventoryStackKey(InventorySlotData))++;
		}
	}

	const int32 VisibleSlotCount = FMath::Max(1, GetVisibleSellSlotCount());
	const int32 EntryCount = GetSellEntryCount(Inv, bHasMobLootStack);
	const int32 MaxPage = EntryCount > 0 ? (EntryCount - 1) / VisibleSlotCount : 0;
	SellInventoryPageIndex = FMath::Clamp(SellInventoryPageIndex, 0, MaxPage);

	if (FindSellEntryIndexForSelection(Inv, bHasMobLootStack, SelectedInventoryIndex) == INDEX_NONE)
	{
		SelectedInventoryIndex = EntryCount > 0
			? ResolveSellEntryToInventoryIndex(Inv, bHasMobLootStack, SellInventoryPageIndex * VisibleSlotCount)
			: INDEX_NONE;
	}

	const int32 SelectedEntryIndex = FindSellEntryIndexForSelection(Inv, bHasMobLootStack, SelectedInventoryIndex);
	if (SelectedEntryIndex != INDEX_NONE)
	{
		SellInventoryPageIndex = FMath::Clamp(SelectedEntryIndex / VisibleSlotCount, 0, MaxPage);
	}

	if (InventoryPageText.IsValid())
	{
		const int32 PageDisplay = EntryCount > 0 ? SellInventoryPageIndex + 1 : 0;
		const int32 PageCount = EntryCount > 0 ? MaxPage + 1 : 0;
		InventoryPageText->SetText(FText::Format(
			NSLOCTEXT("T66.Shop", "SellSlotPageFormat", "{0}/{1}"),
			FText::AsNumber(PageDisplay),
			FText::AsNumber(PageCount)));
	}
	if (InventoryRotateButtonWidget.IsValid())
	{
		InventoryRotateButtonWidget->SetEnabled(EntryCount > VisibleSlotCount && !IsBossActive());
	}

	for (int32 i = 0; i < InventorySlotTexts.Num(); ++i)
	{
		const int32 EntryIndex = SellInventoryPageIndex * VisibleSlotCount + i;
		const int32 InventoryIndex = ResolveSellEntryToInventoryIndex(Inv, bHasMobLootStack, EntryIndex);
		const bool bMobLootSlot = InventoryIndex == T66MobLootSellSelectionIndex;
		const bool bHasItem = bMobLootSlot || (Inv.IsValidIndex(InventoryIndex) && !Inv[InventoryIndex].IsNone());
		const bool bHasInventorySlotData = !bMobLootSlot && bHasItem && InvSlots.IsValidIndex(InventoryIndex) && InvSlots[InventoryIndex].IsValid();
		const ET66ItemRarity SlotRarity = bHasInventorySlotData ? InvSlots[InventoryIndex].Rarity : ET66ItemRarity::Black;
		FItemData D;
		const FName CardItemID = bMobLootSlot ? T66MobLootItemID : (bHasItem && Inv.IsValidIndex(InventoryIndex) ? Inv[InventoryIndex] : NAME_None);
		const bool bHasData = bHasItem && GI && !CardItemID.IsNone() && GI->GetItemData(CardItemID, D);
		if (InventorySlotTexts[i].IsValid())
		{
			InventorySlotTexts[i]->SetText(bHasItem
				? (bMobLootSlot
					? NSLOCTEXT("T66.Shop", "MobLootSellCardName", "Mob Loot")
					: (Loc ? Loc->GetText_ItemDisplayNameForRarity(CardItemID, SlotRarity) : FText::FromName(CardItemID)))
				: NSLOCTEXT("T66.Common", "Empty", "EMPTY"));
		}
		if (InventorySlotDescTexts.IsValidIndex(i) && InventorySlotDescTexts[i].IsValid())
		{
			if (bMobLootSlot)
			{
				InventorySlotDescTexts[i]->SetText(NSLOCTEXT("T66.Shop", "MobLootSellDesc", "Collected monster loot. Sells for 1g each."));
			}
			else if (bHasData && bHasInventorySlotData)
			{
				const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f;
				InventorySlotDescTexts[i]->SetText(T66ItemCardTextUtils::BuildItemCardDescription(Loc, D, SlotRarity, InvSlots[InventoryIndex].Line1RolledValue, ScaleMult, InvSlots[InventoryIndex].GetLine2Multiplier()));
			}
			else
			{
				InventorySlotDescTexts[i]->SetText(FText::GetEmpty());
			}
		}
		if (InventorySlotButtons.IsValidIndex(i) && InventorySlotButtons[i].IsValid())
		{
			InventorySlotButtons[i]->SetEnabled(bHasItem && !IsBossActive());
		}
		if (InventorySlotActionTexts.IsValidIndex(i) && InventorySlotActionTexts[i].IsValid())
		{
			int32 SellValue = 0;
			if (bMobLootSlot)
			{
				SellValue = RunState->GetCollectedMobLootSellValue();
			}
			else if (bHasInventorySlotData)
			{
				SellValue = RunState->GetSellGoldForInventorySlot(InvSlots[InventoryIndex]);
			}

			InventorySlotActionTexts[i]->SetText(bHasItem
				? FText::Format(NSLOCTEXT("T66.Shop", "SellPriceFormat", "SELL ({0}g)"), FText::AsNumber(FMath::Max(0, SellValue)))
				: (Loc ? Loc->GetText_Sell() : NSLOCTEXT("T66.Common", "Sell", "SELL")));
		}
		const int32 StackCount = bMobLootSlot
			? MobLootStack
			: (bHasInventorySlotData
				? StackCounts.FindRef(MakeShopInventoryStackKey(InvSlots[InventoryIndex]))
				: 0);
		if (InventorySlotCountTexts.IsValidIndex(i) && InventorySlotCountTexts[i].IsValid())
		{
			InventorySlotCountTexts[i]->SetText(
				StackCount > 1 || bMobLootSlot
					? FText::Format(NSLOCTEXT("T66.Inventory", "StackCountFormat", "{0}x"), FText::AsNumber(StackCount))
					: FText::GetEmpty());
			InventorySlotCountTexts[i]->SetVisibility((StackCount > 1 || bMobLootSlot) ? EVisibility::Visible : EVisibility::Hidden);
		}
		if (InventorySlotBorders.IsValidIndex(i) && InventorySlotBorders[i].IsValid())
		{
			FLinearColor Fill = FT66FlatStyle::Tokens::Panel2;

			if ((bMobLootSlot && SelectedInventoryIndex == T66MobLootSellSelectionIndex)
				|| (!bMobLootSlot && InventoryIndex == SelectedInventoryIndex))
			{
				Fill = (Fill * 0.45f + FT66FlatStyle::Tokens::Accent * 0.55f);
			}
			else if (bMobLootSlot)
			{
				Fill = (Fill * 0.55f + FLinearColor(0.95f, 0.67f, 0.18f, 1.0f) * 0.45f);
			}
			InventorySlotBorders[i]->SetBorderBackgroundColor(Fill);

			if (InventorySlotIconBrushes.IsValidIndex(i) && InventorySlotIconBrushes[i].IsValid())
			{
				const TSoftObjectPtr<UTexture2D> SlotIconSoft = bHasData ? D.GetIconForRarity(SlotRarity) : TSoftObjectPtr<UTexture2D>();
				if (!SlotIconSoft.IsNull() && TexPool)
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, SlotIconSoft, this, InventorySlotIconBrushes[i], FName(TEXT("ShopInv"), i + 1), /*bClearWhileLoading*/ false);
				}
				else
				{
					InventorySlotIconBrushes[i]->SetResourceObject(nullptr);
				}
			}
			if (InventorySlotIconImages.IsValidIndex(i) && InventorySlotIconImages[i].IsValid())
			{
				const bool bHasIcon = bHasData && !D.GetIconForRarity(SlotRarity).IsNull();
				InventorySlotIconImages[i]->SetVisibility(bHasIcon ? EVisibility::Visible : EVisibility::Hidden);
			}
		}
	}
}

void UT66CasinoVendorTabWidget::RefreshSellPanel()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return;

	const TArray<FName>& Inv = RunState->GetInventory();
	const int32 MobLootStack = RunState->GetCollectedMobLootStack();
	const bool bMobLootSelection = SelectedInventoryIndex == T66MobLootSellSelectionIndex && MobLootStack > 0;
	const bool bValidSelection = Inv.IsValidIndex(SelectedInventoryIndex) && !Inv[SelectedInventoryIndex].IsNone();

	if (SellPanelContainer.IsValid())
	{
		// Keep visible so the inventory layout doesn't "pop" when selecting an item.
		SellPanelContainer->SetVisibility(EVisibility::Visible);
	}
	if (!bMobLootSelection && !bValidSelection)
	{
		if (SellItemNameText.IsValid()) SellItemNameText->SetText(FText::GetEmpty());
		if (SellItemDescText.IsValid()) SellItemDescText->SetText(FText::GetEmpty());
		if (SellItemPriceText.IsValid()) SellItemPriceText->SetText(FText::GetEmpty());
		if (SellItemButton.IsValid()) SellItemButton->SetEnabled(false);
		return;
	}

	if (bMobLootSelection)
	{
		if (SellItemNameText.IsValid())
		{
			SellItemNameText->SetText(NSLOCTEXT("T66.Shop", "MobLootSellName", "Mob Loot"));
		}
		if (SellItemDescText.IsValid())
		{
			SellItemDescText->SetText(NSLOCTEXT("T66.Shop", "MobLootSellDesc", "Collected monster loot. Sells for 1g each."));
		}
		if (SellItemPriceText.IsValid())
		{
			SellItemPriceText->SetText(FText::Format(
				NSLOCTEXT("T66.Shop", "SellForFormat", "SELL FOR: {0}g"),
				FText::AsNumber(RunState->GetCollectedMobLootSellValue())));
		}
		if (SellItemButton.IsValid())
		{
			SellItemButton->SetEnabled(!IsBossActive());
		}
		return;
	}

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	FItemData D;
	const bool bHasData = GI && GI->GetItemData(Inv[SelectedInventoryIndex], D);
	ET66ItemRarity SelectedRarity = ET66ItemRarity::Black;
	int32 MainValue = 0;
	float Line2Multiplier = 0.f;
	if (RunState)
	{
		const TArray<FT66InventorySlot>& Slots = RunState->GetInventorySlots();
		if (SelectedInventoryIndex >= 0 && SelectedInventoryIndex < Slots.Num())
		{
			MainValue = Slots[SelectedInventoryIndex].Line1RolledValue;
			SelectedRarity = Slots[SelectedInventoryIndex].Rarity;
			Line2Multiplier = Slots[SelectedInventoryIndex].GetLine2Multiplier();
		}
	}
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI2 = World ? World->GetGameInstance() : nullptr)
	{
		Loc = GI2->GetSubsystem<UT66LocalizationSubsystem>();
	}

	if (SellItemNameText.IsValid())
	{
		SellItemNameText->SetText(Loc ? Loc->GetText_ItemDisplayNameForRarity(Inv[SelectedInventoryIndex], SelectedRarity) : FText::FromName(Inv[SelectedInventoryIndex]));
	}
	if (SellItemDescText.IsValid())
	{
		if (!bHasData)
		{
			SellItemDescText->SetText(FText::GetEmpty());
		}
		else
		{
			const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f;
			SellItemDescText->SetText(T66ItemCardTextUtils::BuildItemCardDescription(Loc, D, SelectedRarity, MainValue, ScaleMult, Line2Multiplier));
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
				SellValue = RunState->GetSellGoldForInventorySlot(Slots[SelectedInventoryIndex]);
			}
		}
		SellItemPriceText->SetText(FText::Format(
			NSLOCTEXT("T66.Shop", "SellForFormat", "SELL FOR: {0}g"),
			FText::AsNumber(SellValue)));
	}
	if (SellItemButton.IsValid())
	{
		SellItemButton->SetEnabled(!IsBossActive());
	}
}

FReply UT66CasinoVendorTabWidget::OnBack()
{
	CloseOverlay();
	return FReply::Handled();
}

FReply UT66CasinoVendorTabWidget::OnBorrowClicked()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return FReply::Handled();
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	if (BorrowAmount <= 0)
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(Loc ? Loc->GetText_BorrowAmountMustBePositive() : NSLOCTEXT("T66.Shop", "BorrowMustBePositive", "Borrow amount must be > 0."));
		}
		return FReply::Handled();
	}
	if (!RunState->BorrowGold(BorrowAmount))
	{
		BorrowAmount = FMath::Clamp(BorrowAmount, 0, RunState->GetRemainingBorrowCapacity());
		if (StatusText.IsValid())
		{
			const FText Fmt = Loc ? Loc->GetText_BorrowExceedsNetWorthFormat() : NSLOCTEXT("T66.Shop", "BorrowExceedsNetWorthFormat", "Borrow amount exceeds remaining Net Worth ({0}).");
			StatusText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetRemainingBorrowCapacity())));
		}
		RefreshTopBar();
		return FReply::Handled();
	}
	if (StatusText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_BorrowedAmountFormat() : NSLOCTEXT("T66.Shop", "BorrowedAmountFormat", "Borrowed {0}.");
		StatusText->SetText(FText::Format(Fmt, FText::AsNumber(BorrowAmount)));
	}
	RefreshTopBar();
	return FReply::Handled();
}

FReply UT66CasinoVendorTabWidget::OnPaybackClicked()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return FReply::Handled();
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	if (PaybackAmount <= 0)
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(Loc ? Loc->GetText_PaybackAmountMustBePositive() : NSLOCTEXT("T66.Shop", "PaybackMustBePositive", "Payback amount must be > 0."));
		}
		return FReply::Handled();
	}
	const int32 Paid = RunState->PayDebt(PaybackAmount);
	if (StatusText.IsValid()) StatusText->SetText(FText::GetEmpty());
	RefreshTopBar();
	return FReply::Handled();
}

FReply UT66CasinoVendorTabWidget::OnSelectInventorySlot(int32 InventoryIndex)
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState)
	{
		return FReply::Handled();
	}

	const TArray<FName>& Inv = RunState->GetInventory();
	const bool bHasMobLootStack = RunState->GetCollectedMobLootStack() > 0;
	const int32 EntryIndex = SellInventoryPageIndex * FMath::Max(1, GetVisibleSellSlotCount()) + InventoryIndex;
	SelectedInventoryIndex = ResolveSellEntryToInventoryIndex(Inv, bHasMobLootStack, EntryIndex);
	RefreshInventory();
	RefreshSellPanel();
	return FReply::Handled();
}

FReply UT66CasinoVendorTabWidget::OnRotateInventorySlotsClicked()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState)
	{
		return FReply::Handled();
	}

	const TArray<FName>& Inv = RunState->GetInventory();
	const bool bHasMobLootStack = RunState->GetCollectedMobLootStack() > 0;
	const int32 VisibleSlotCount = FMath::Max(1, GetVisibleSellSlotCount());
	const int32 EntryCount = GetSellEntryCount(Inv, bHasMobLootStack);
	const int32 MaxPage = EntryCount > 0 ? (EntryCount - 1) / VisibleSlotCount : 0;
	SellInventoryPageIndex = MaxPage > 0 ? ((SellInventoryPageIndex + 1) % (MaxPage + 1)) : 0;
	SelectedInventoryIndex = EntryCount > 0
		? ResolveSellEntryToInventoryIndex(Inv, bHasMobLootStack, SellInventoryPageIndex * VisibleSlotCount)
		: INDEX_NONE;

	RefreshInventory();
	RefreshSellPanel();
	return FReply::Handled();
}

FReply UT66CasinoVendorTabWidget::OnSellSlotClicked(const int32 DisplaySlotIndex)
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState)
	{
		return FReply::Handled();
	}

	const TArray<FName>& Inv = RunState->GetInventory();
	const bool bHasMobLootStack = RunState->GetCollectedMobLootStack() > 0;
	const int32 EntryIndex = SellInventoryPageIndex * FMath::Max(1, GetVisibleSellSlotCount()) + DisplaySlotIndex;
	SelectedInventoryIndex = ResolveSellEntryToInventoryIndex(Inv, bHasMobLootStack, EntryIndex);
	return OnSellSelectedClicked();
}

FReply UT66CasinoVendorTabWidget::OnSellSelectedClicked()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return FReply::Handled();

	if (SelectedInventoryIndex == T66MobLootSellSelectionIndex)
	{
		const bool bSoldMobLoot = RunState->SellCollectedMobLoot();
		if (StatusText.IsValid())
		{
			StatusText->SetText(bSoldMobLoot
				? NSLOCTEXT("T66.Shop", "SoldMobLootStatus", "Sold Mob Loot.")
				: NSLOCTEXT("T66.Shop", "CouldNotSell", "Could not sell."));
		}
		RefreshAll();
		return FReply::Handled();
	}

	if (SelectedInventoryIndex < 0)
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Shop", "SelectItemToSell", "Select an item to sell."));
		return FReply::Handled();
	}

	const bool bSold = RunState->SellInventoryItemAt(SelectedInventoryIndex);
	if (bSold)
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Shop", "SoldStatus", "Sold."));
	}
	else
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Shop", "CouldNotSell", "Could not sell."));
	}
	RefreshAll();
	return FReply::Handled();
}

FReply UT66CasinoVendorTabWidget::OnBuySlot(int32 SlotIndex)
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return FReply::Handled();

	if (IsBossActive())
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Shop", "BossIsActive", "Boss is active."));
		return FReply::Handled();
	}

	if (!RunState->HasInventorySpace())
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Shop", "InventoryFull", "Inventory full."));
		return FReply::Handled();
	}

	const bool bBought = RunState->TryBuyShopStockSlot(SlotIndex);
	if (bBought)
	{
		bBoughtSomethingThisVisit = true;
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
			{
				Achieve->NotifyShopPurchase();
			}
		}
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Shop", "Purchased", "Purchased."));
	}
	else
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Shop", "CouldNotPurchase", "Could not purchase."));
	}
	RefreshAll();
	return FReply::Handled();
}

FReply UT66CasinoVendorTabWidget::OnBuybackSlot(int32 SlotIndex)
{
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(GetWorld());
	if (!RunState) return FReply::Handled();

	const bool bBought = RunState->TryBuybackSlot(SlotIndex);
	if (StatusText.IsValid())
	{
		StatusText->SetText(bBought
			? NSLOCTEXT("T66.Shop", "Purchased", "Purchased.")
			: NSLOCTEXT("T66.Shop", "CouldNotPurchase", "Could not purchase."));
	}
	RefreshAll();
	return FReply::Handled();
}

FReply UT66CasinoVendorTabWidget::OnStealSlot(int32 SlotIndex)
{
	if (!bShopAllowsSteal)
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Shop", "StealDisabled", "Stealing is disabled here."));
		return FReply::Handled();
	}
	if (!bBoughtSomethingThisVisit)
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Shop", "BuyOneBeforeStealing", "Buy one item before stealing."));
		return FReply::Handled();
	}
	if (IsBossActive())
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Shop", "BossIsActive", "Boss is active."));
		return FReply::Handled();
	}
	ShowStealPrompt(SlotIndex);
	return FReply::Handled();
}

void UT66CasinoVendorTabWidget::ShowStealPrompt(int32 SlotIndex)
{
	PendingStealIndex = SlotIndex;
	bStealPromptVisible = true;
	StealMarker01 = 0.f;
	bStealForward = true;
	StealLastTickTimeSeconds = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds()) : 0.f;

	if (StealMarkerSpacerBox.IsValid())
	{
		StealMarkerSpacerBox->SetWidthOverride(0.f);
	}
	if (StealPromptContainer.IsValid())
	{
		StealPromptContainer->SetVisibility(EVisibility::Visible);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StealTickTimerHandle);
		// 15Hz keeps the timing read clear without forcing a layout update every frame.
		World->GetTimerManager().SetTimer(StealTickTimerHandle, this, &UT66CasinoVendorTabWidget::TickStealBar, 1.f / 15.f, true);
	}
}

void UT66CasinoVendorTabWidget::HideStealPrompt()
{
	bStealPromptVisible = false;
	PendingStealIndex = -1;
	if (StealPromptContainer.IsValid())
	{
		StealPromptContainer->SetVisibility(EVisibility::Collapsed);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StealTickTimerHandle);
	}
}

void UT66CasinoVendorTabWidget::TickStealBar()
{
	// Bounce marker back and forth.
	const float Speed = 1.6f; // cycles per second-ish
	UWorld* World = GetWorld();
	if (!World) return;
	const float Now = static_cast<float>(World->GetTimeSeconds());
	float Delta = Now - StealLastTickTimeSeconds;
	StealLastTickTimeSeconds = Now;
	Delta = FMath::Clamp(Delta, 0.f, 0.10f);
	StealMarker01 += (bStealForward ? 1.f : -1.f) * Speed * Delta;
	if (StealMarker01 >= 1.f) { StealMarker01 = 1.f; bStealForward = false; }
	if (StealMarker01 <= 0.f) { StealMarker01 = 0.f; bStealForward = true; }

	// Bar width is 360, marker is 10, so travel range is 350.
	const float Travel = 350.f;
	if (StealMarkerSpacerBox.IsValid())
	{
		StealMarkerSpacerBox->SetWidthOverride(Travel * StealMarker01);
	}
}

FReply UT66CasinoVendorTabWidget::OnStealStop()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState)
	{
		HideStealPrompt();
		return FReply::Handled();
	}

	const float DistFromCenter = FMath::Abs(StealMarker01 - 0.5f);
	const bool bTimingHit = (DistFromCenter <= 0.12f);

	// Backend owns RNG: timing improves odds, Luck can bias outcomes.
	const bool bGranted = RunState->ResolveShopStealAttempt(PendingStealIndex, bTimingHit, false);
	HideStealPrompt();

	const ET66ShopStealOutcome Outcome = RunState->GetLastShopStealOutcome();

	if (StatusText.IsValid())
	{
		if (Outcome == ET66ShopStealOutcome::Miss) StatusText->SetText(NSLOCTEXT("T66.Shop", "StealFailedMiss", "Steal failed (miss)."));
		else if (Outcome == ET66ShopStealOutcome::Failed) StatusText->SetText(NSLOCTEXT("T66.Shop", "StealFailed", "Steal failed."));
		else if (Outcome == ET66ShopStealOutcome::InventoryFull) StatusText->SetText(NSLOCTEXT("T66.Shop", "StealSucceededButInventoryFull", "Steal succeeded, but inventory is full."));
		else if (Outcome == ET66ShopStealOutcome::Success && bGranted) StatusText->SetText(NSLOCTEXT("T66.Shop", "Stolen", "Stolen."));
		else StatusText->SetText(NSLOCTEXT("T66.Shop", "StealFailed", "Steal failed."));
	}

	RefreshAll();
	if (RunState->DidLastShopStealAttemptTriggerVendorBoss())
	{
		SpawnVendorBoss();
	}
	return FReply::Handled();
}

bool UT66CasinoVendorTabWidget::IsBossActive() const
{
	return bCachedBossActive;
}

void UT66CasinoVendorTabWidget::HandleBossChanged()
{
	bCachedBossActive = false;
	if (UWorld* World = GetWorld())
	{
		if (UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World))
		{
			bCachedBossActive = RunState->GetBossActive();
		}
	}

	RefreshStock();
	RefreshInventory();
	RefreshSellPanel();
	RefreshShopChrome();
}

void UT66CasinoVendorTabWidget::SpawnVendorBoss()
{
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->SpawnVendorBoss();
	}
}
