// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66VendorOverlayWidget.h"
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
#include "UI/Style/T66OverlayChromeStyle.h"
#include "UI/Style/T66Style.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
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

static FString MakeInventoryStackKey(const FT66InventorySlot& Slot)
{
	return FString::Printf(TEXT("%s|%d"), *Slot.ItemTemplateID.ToString(), static_cast<int32>(Slot.Rarity));
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

void UT66VendorOverlayWidget::NativeDestruct()
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
				RunState->GoldChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleGoldOrDebtChanged);
				RunState->DebtChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleGoldOrDebtChanged);
				RunState->InventoryChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleInventoryChanged);
				RunState->VendorChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleVendorChanged);
				RunState->BuybackChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleBuybackChanged);
				RunState->BossChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleBossChanged);
			}
		}
	}

	// Safety: if destroyed unexpectedly, restore gameplay input.
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->IsGameplayLevel() && !PC->IsPaused())
		{
			PC->RestoreGameplayInputMode();
		}
	}

	ReleaseCachedSlateResources();
	Super::NativeDestruct();
}

void UT66VendorOverlayWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	ReleaseCachedSlateResources();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void UT66VendorOverlayWidget::ReleaseCachedSlateResources()
{
	PageSwitcher.Reset();
	NetWorthText.Reset();
	GoldText.Reset();
	DebtText.Reset();
	StatusText.Reset();
	AngerCircleImage.Reset();
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
	InventorySlotCountTexts.Reset();
	InventorySlotIconImages.Reset();
	InventorySlotIconBrushes.Reset();
}

void UT66VendorOverlayWidget::CloseOverlay()
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

TSharedRef<SWidget> UT66VendorOverlayWidget::RebuildWidget()
{
	bBoughtSomethingThisVisit = false;
	BorrowAmount = FMath::Max(0, BorrowAmount);
	PaybackAmount = FMath::Max(0, PaybackAmount);
	SelectedInventoryIndex = -1;

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
		RunState->EnsureVendorStockForCurrentStage();
		bBoughtSomethingThisVisit = RunState->HasBoughtFromVendorThisStage();
		bCachedBossActive = RunState->GetBossActive();

		// Bind live updates (event-driven UI).
		RunState->GoldChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleGoldOrDebtChanged);
		RunState->DebtChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleGoldOrDebtChanged);
		RunState->InventoryChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleInventoryChanged);
		RunState->VendorChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleVendorChanged);
		RunState->BuybackChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleBuybackChanged);
		RunState->BossChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleBossChanged);

		RunState->GoldChanged.AddDynamic(this, &UT66VendorOverlayWidget::HandleGoldOrDebtChanged);
		RunState->DebtChanged.AddDynamic(this, &UT66VendorOverlayWidget::HandleGoldOrDebtChanged);
		RunState->InventoryChanged.AddDynamic(this, &UT66VendorOverlayWidget::HandleInventoryChanged);
		RunState->VendorChanged.AddDynamic(this, &UT66VendorOverlayWidget::HandleVendorChanged);
		RunState->BuybackChanged.AddDynamic(this, &UT66VendorOverlayWidget::HandleBuybackChanged);
		RunState->BossChanged.AddDynamic(this, &UT66VendorOverlayWidget::HandleBossChanged);
	}

	const ISlateStyle& Style = FT66Style::Get();

	const FTextBlockStyle& TextTitle = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Title");
	const FTextBlockStyle& TextHeading = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading");
	const FTextBlockStyle& TextBody = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Body");
	const FTextBlockStyle& TextChip = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip");
	const bool bCompactCasinoLayout = bEmbeddedInCasinoShell;
	const float OverlayPadding = bCompactCasinoLayout ? 12.f : FT66Style::Tokens::NPCOverlayPadding;
	const float AngerFaceSize = bCompactCasinoLayout ? 85.f : FT66Style::Tokens::NPCAngerCircleSize;
	const float StatsPanelWidth = bCompactCasinoLayout ? 150.f : FT66Style::Tokens::NPCVendorStatsPanelWidth;
	const float RightPanelWidth = bCompactCasinoLayout ? 200.f : FT66Style::Tokens::NPCRightPanelWidth;
	const float MainRowHeight = bCompactCasinoLayout ? 320.f : FT66Style::Tokens::NPCMainRowHeight;
	const float ShopCardSize = bCompactCasinoLayout ? FT66Style::Tokens::NPCCompactShopCardWidth : FT66Style::Tokens::NPCShopCardWidth;
	const float ShopCardHeight = bCompactCasinoLayout ? FT66Style::Tokens::NPCCompactShopCardHeight : FT66Style::Tokens::NPCShopCardHeight;
	const float ShopCardGap = bCompactCasinoLayout ? FT66Style::Tokens::Space3 : FT66Style::Tokens::Space4;
	const float ShopCardPadding = bCompactCasinoLayout ? 5.f : FT66Style::Tokens::Space4;
	const float ShopNameBoxHeight = bCompactCasinoLayout ? 28.f : 60.f;
	const float ShopIconSize = ShopCardSize - ShopCardPadding * 2.f;
	const float InventorySlotSize = bCompactCasinoLayout ? 80.f : FT66Style::Tokens::InventorySlotSize;
	const float AngerImageSize = bCompactCasinoLayout ? 136.f : 260.f;
	const float SellPanelSize = bCompactCasinoLayout ? 128.f : 160.f;
	const float BankSpinBoxWidth = bCompactCasinoLayout ? 68.f : FT66Style::Tokens::NPCBankSpinBoxWidth;
	const float BankSpinBoxHeight = bCompactCasinoLayout ? 28.f : FT66Style::Tokens::NPCBankSpinBoxHeight;
	const float CardButtonMinWidth = bCompactCasinoLayout ? 0.f : 100.f;
	const FMargin ShopButtonPadding = bCompactCasinoLayout ? FMargin(5.f, 3.f) : FMargin(8.f, 6.f);
	const FMargin ActionButtonPadding = bCompactCasinoLayout ? FMargin(8.f, 5.f) : FMargin(16.f, 10.f);
	const int32 StatsPanelFontAdjustment = bCompactCasinoLayout ? -5 : 0;
	const int32 CardHeadingFontSize = bCompactCasinoLayout ? 9 : 16;
	const int32 CardBodyFontSize = bCompactCasinoLayout ? 7 : 12;
	const int32 CardButtonFontSize = bCompactCasinoLayout ? 9 : 14;
	const int32 InventoryCountFontSize = bCompactCasinoLayout ? 8 : 14;
	const int32 InventoryDashFontSize = bCompactCasinoLayout ? 10 : 16;
	const int32 SectionHeadingFontSize = bCompactCasinoLayout ? 10 : 16;
	const int32 PageTitleFontSize = bCompactCasinoLayout ? 32 : 64;
	const int32 StatusFontSize = bCompactCasinoLayout ? 8 : 12;
	const int32 SpinBoxFontSize = bCompactCasinoLayout ? 10 : 16;

	// --- NPC anger face sprites ---
	auto InitFaceBrush = [AngerFaceSize](FSlateBrush& B) {
		B = FSlateBrush();
		B.ImageSize = FVector2D(AngerFaceSize, AngerFaceSize);
		B.DrawAs = ESlateBrushDrawType::Image;
	};
	InitFaceBrush(AngerFace_Happy);
	InitFaceBrush(AngerFace_Neutral);
	InitFaceBrush(AngerFace_Angry);

	if (TexPool)
	{
		const TSoftObjectPtr<UTexture2D> HappySoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/NPCs/Vendor/Happy.Happy")));
		const TSoftObjectPtr<UTexture2D> NeutralSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/NPCs/Vendor/Neutral.Neutral")));
		const TSoftObjectPtr<UTexture2D> AngrySoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/NPCs/Vendor/Angry.Angry")));
		T66SlateTexture::BindBrushAsync(TexPool, HappySoft, this, AngerFace_Happy, FName(TEXT("VendorFaceHappy")), /*bClearWhileLoading*/ true);
		T66SlateTexture::BindBrushAsync(TexPool, NeutralSoft, this, AngerFace_Neutral, FName(TEXT("VendorFaceNeutral")), /*bClearWhileLoading*/ true);
		T66SlateTexture::BindBrushAsync(TexPool, AngrySoft, this, AngerFace_Angry, FName(TEXT("VendorFaceAngry")), /*bClearWhileLoading*/ true);
	}

	const FText VendorTitle = Loc ? Loc->GetText_Vendor() : NSLOCTEXT("T66.Vendor", "VendorTitle", "VENDOR");
	const FText ShopTitle = Loc ? Loc->GetText_Shop() : NSLOCTEXT("T66.Vendor", "ShopTitle", "SHOP");
	const FText BankTitle = Loc ? Loc->GetText_Bank() : NSLOCTEXT("T66.Vendor", "BankTitle", "BANK");
	const FText CloseText = NSLOCTEXT("T66.Common", "Close", "CLOSE");
	const FText InventoryTitle = Loc ? Loc->GetText_YourItems() : NSLOCTEXT("T66.Vendor", "InventoryTitle", "INVENTORY");
	const FText RerollText = Loc ? Loc->GetText_Reroll() : NSLOCTEXT("T66.Vendor", "Reroll", "REROLL");
	LiveStatsPanel = MakeShared<T66StatsPanelSlate::FT66LiveStatsPanel>();

	const TArray<FName> EmptyStock;
	const TArray<FName>& Stock = RunState ? RunState->GetVendorStockItemIDs() : EmptyStock;

	static constexpr int32 ShopSlotCount = UT66RunStateSubsystem::VendorDisplaySlotCount;
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

	InventorySlotBorders.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotButtons.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotTexts.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotCountTexts.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotIconImages.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotIconBrushes.SetNum(UT66RunStateSubsystem::MaxInventorySlots);

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
		InventorySlotIconBrushes[i]->ImageSize = FVector2D(InventorySlotSize, InventorySlotSize);
	}

	TSharedRef<SHorizontalBox> ShopRow = SNew(SHorizontalBox);

	for (int32 i = 0; i < ShopSlotCount; ++i)
	{
		TSharedRef<SWidget> BuyBtnWidget = T66OverlayChromeStyle::MakeButton(
			T66OverlayChromeStyle::MakeButtonParams(
				Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY"),
				FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnBuySlot, i),
				ET66OverlayChromeButtonFamily::Primary)
			.SetMinWidth(CardButtonMinWidth)
			.SetMinHeight(36.f)
			.SetPadding(ShopButtonPadding)
			.SetFontSize(CardButtonFontSize)
			.SetContent(
				SAssignNew(BuyButtonTexts[i], STextBlock)
				.Text(Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY"))
				.Font(FT66Style::Tokens::FontBold(CardButtonFontSize))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			)
		);
		BuyButtons[i] = BuyBtnWidget;

		TSharedRef<SWidget> StealBtnWidget = T66OverlayChromeStyle::MakeButton(
			T66OverlayChromeStyle::MakeButtonParams(
				Loc ? Loc->GetText_Steal() : NSLOCTEXT("T66.Vendor", "Steal", "STEAL"),
				FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnStealSlot, i),
				ET66OverlayChromeButtonFamily::Danger)
			.SetMinWidth(CardButtonMinWidth)
			.SetMinHeight(36.f)
			.SetPadding(ShopButtonPadding)
			.SetFontSize(CardButtonFontSize)
		);
		StealButtons[i] = StealBtnWidget;

		ShopRow->AddSlot()
			.AutoWidth()
			.Padding(i > 0 ? FMargin(ShopCardGap, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			SNew(SBox)
			.WidthOverride(ShopCardSize)
			.HeightOverride(ShopCardHeight)
			[
				T66OverlayChromeStyle::MakePanel(
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
							.Font(FT66Style::Tokens::FontBold(CardHeadingFontSize))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.AutoWrapText(true)
							.WrapTextAt(ShopCardSize - ShopCardPadding * 2.f)
						]
					]
					// 2. Large image below name (centered)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)
						[
							T66OverlayChromeStyle::MakePanel(
								SNew(SBox)
								.WidthOverride(ShopIconSize)
								.HeightOverride(ShopIconSize)
								[
									SAssignNew(ItemIconImages[i], SImage)
									.Image(ItemIconBrushes[i].Get())
									.ColorAndOpacity(FLinearColor::White)
								],
								ET66OverlayChromeBrush::SlotNormal,
								FMargin(0.f),
								&ItemIconBorders[i])
						]
					]
					// 3. Two stat lines stacked
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f)
					[
						SAssignNew(ItemDescTexts[i], STextBlock)
						.Text(FText::GetEmpty())
						.TextStyle(&TextBody)
						.Font(FT66Style::Tokens::FontRegular(CardBodyFontSize))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
						.WrapTextAt(ShopCardSize - ShopCardPadding * 2.f)
					]
					// 4. Buy and Steal side by side
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
							.FillWidth(1.f)
							.Padding(0.f, 0.f, FT66Style::Tokens::Space2, 0.f)
						[ BuyBtnWidget ]
						+ SHorizontalBox::Slot()
							.FillWidth(1.f)
						[ StealBtnWidget ]
					]
				,
					ET66OverlayChromeBrush::OfferCardNormal,
					FMargin(ShopCardPadding),
					&ItemTileBorders[i])
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
	const FText BuybackTitle = NSLOCTEXT("T66.Vendor", "Buyback", "BUYBACK");
	TSharedRef<SHorizontalBox> BuybackRow = SNew(SHorizontalBox);
	for (int32 i = 0; i < BuybackSlotCount; ++i)
	{
		TSharedRef<SWidget> BuybackBtnWidget = T66OverlayChromeStyle::MakeButton(
			T66OverlayChromeStyle::MakeButtonParams(
				Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY"),
				FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnBuybackSlot, i),
				ET66OverlayChromeButtonFamily::Primary)
			.SetMinWidth(CardButtonMinWidth)
			.SetMinHeight(36.f)
			.SetPadding(ShopButtonPadding)
			.SetFontSize(CardButtonFontSize)
			.SetContent(
				SAssignNew(BuybackPriceTexts[i], STextBlock)
				.Text(Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY"))
				.Font(FT66Style::Tokens::FontBold(CardButtonFontSize))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			)
		);
		BuybackBuyButtons[i] = BuybackBtnWidget;
		BuybackRow->AddSlot()
			.AutoWidth()
			.Padding(i > 0 ? FMargin(ShopCardGap, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			SNew(SBox)
			.WidthOverride(ShopCardSize)
			.HeightOverride(ShopCardHeight)
			[
				T66OverlayChromeStyle::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBox)
						.HeightOverride(ShopNameBoxHeight)
						[
							SAssignNew(BuybackNameTexts[i], STextBlock)
							.Text(FText::GetEmpty())
							.TextStyle(&TextHeading)
							.Font(FT66Style::Tokens::FontBold(CardHeadingFontSize))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.AutoWrapText(true)
							.WrapTextAt(ShopCardSize - ShopCardPadding * 2.f)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)
						[
							T66OverlayChromeStyle::MakePanel(
								SNew(SBox)
								.WidthOverride(ShopIconSize)
								.HeightOverride(ShopIconSize)
								[
									SAssignNew(BuybackIconImages[i], SImage)
									.Image(BuybackIconBrushes[i].Get())
									.ColorAndOpacity(FLinearColor::White)
								],
								ET66OverlayChromeBrush::SlotNormal,
								FMargin(0.f),
								&BuybackIconBorders[i])
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f)
					[
						SAssignNew(BuybackDescTexts[i], STextBlock)
						.Text(FText::GetEmpty())
						.TextStyle(&TextBody)
						.Font(FT66Style::Tokens::FontRegular(CardBodyFontSize))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
						.WrapTextAt(ShopCardSize - ShopCardPadding * 2.f)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f)
					[
						BuybackBtnWidget
					]
				,
					ET66OverlayChromeBrush::OfferCardNormal,
					FMargin(ShopCardPadding),
					&BuybackTileBorders[i])
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
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Vendor", "StealTimingTitle", "STEAL (TIMING)"))
					.TextStyle(&TextHeading)
					.Font(FT66Style::Tokens::FontBold(CardHeadingFontSize))
					.ColorAndOpacity(FT66Style::Tokens::Text)
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
							FT66Style::MakePanel(SNullWidget::NullWidget, FT66PanelParams(ET66PanelType::Panel2))
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
									FT66Style::MakePanel(
										SNullWidget::NullWidget,
										FT66PanelParams(ET66PanelType::Panel2).SetColor(FT66Style::Tokens::Success))
								]
							]
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Vendor", "StealTimingInstructions", "Press STOP near the center."))
					.TextStyle(&TextBody)
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					FT66Style::MakeButton(
						FT66Style::MakeInRunButtonParams(
							NSLOCTEXT("T66.Vendor", "Stop", "STOP"),
							FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnStealStop),
							ET66ButtonType::Primary)
						.SetMinWidth(0.f)
						.SetPadding(FMargin(22.f, 12.f))
					)
				]
			,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(18.f).SetColor(FT66Style::Tokens::Panel2))
		];

	auto MakeTitle = [&](const FText& Text) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(Text)
			.TextStyle(&TextTitle)
			.ColorAndOpacity(FT66Style::Tokens::Text);
	};

	// Pre-create inventory grid (buttons use centralized MakeButton).
	TSharedRef<SUniformGridPanel> InventoryGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(FT66Style::Tokens::Space2, 0.f));

	for (int32 Inv = 0; Inv < UT66RunStateSubsystem::MaxInventorySlots; ++Inv)
	{
		TSharedRef<SWidget> SlotBtn = FT66Style::MakeButton(
			FT66ButtonParams(
				FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnSelectInventorySlot, Inv),
				ET66ButtonType::Neutral)
			.SetMinWidth(InventorySlotSize).SetHeight(InventorySlotSize)
			.SetPadding(FMargin(0.f))
			.SetContent(
				T66OverlayChromeStyle::MakePanel(
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SAssignNew(InventorySlotIconImages[Inv], SImage)
						.Image(InventorySlotIconBrushes[Inv].Get())
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 6.f, 8.f, 0.f)
					[
						SAssignNew(InventorySlotCountTexts[Inv], STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(InventoryCountFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.ShadowOffset(FVector2D(1.f, 1.f))
						.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f))
						.Visibility(EVisibility::Hidden)
					]
					+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						SAssignNew(InventorySlotTexts[Inv], STextBlock)
						.Text(NSLOCTEXT("T66.Common", "Dash", "-"))
						.TextStyle(&TextChip)
						.Font(FT66Style::Tokens::FontBold(InventoryDashFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				,
					ET66OverlayChromeBrush::SlotNormal,
					FMargin(0.f),
					&InventorySlotBorders[Inv])
			)
		);
		InventorySlotButtons[Inv] = SlotBtn;
		InventoryGrid->AddSlot(Inv, 0)
		[
			SNew(SBox)
			.WidthOverride(InventorySlotSize)
			.HeightOverride(InventorySlotSize)
			[
				SlotBtn
			]
		];
	}

	// Pre-create sell button (needs member reference for later SetEnabled).
	TSharedRef<SWidget> SellBtnWidget = T66OverlayChromeStyle::MakeButton(
		T66OverlayChromeStyle::MakeButtonParams(
			Loc ? Loc->GetText_Sell() : NSLOCTEXT("T66.Common", "Sell", "SELL"),
			FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnSellSelectedClicked),
			ET66OverlayChromeButtonFamily::Primary)
		.SetMinWidth(0.f)
		.SetPadding(ActionButtonPadding)
		.SetFontSize(CardButtonFontSize)
	);
	SellItemButton = SellBtnWidget;

	TSharedRef<SWidget> ShopCardsScroller =
		SNew(SScrollBox)
		.Orientation(Orient_Horizontal)
		.ScrollBarVisibility(EVisibility::Visible)
		+ SScrollBox::Slot()
		[
			ShopRow
		];

	TSharedRef<SWidget> BuybackCardsScroller =
		SNew(SScrollBox)
		.Orientation(Orient_Horizontal)
		.ScrollBarVisibility(EVisibility::Visible)
		+ SScrollBox::Slot()
		[
			BuybackRow
		];

	TSharedRef<SWidget> ShopModeToggleButton = T66OverlayChromeStyle::MakeButton(
		T66OverlayChromeStyle::MakeButtonParams(
			BuybackTitle,
			FOnClicked::CreateLambda([this]()
			{
				const bool bShowingBuyback = ShopBuybackSwitcher.IsValid() && ShopBuybackSwitcher->GetActiveWidgetIndex() == 1;
				if (ShopBuybackSwitcher.IsValid())
				{
					ShopBuybackSwitcher->SetActiveWidgetIndex(bShowingBuyback ? 0 : 1);
				}

				if (bShowingBuyback)
				{
					RefreshStock();
				}
				else
				{
					if (UT66RunStateSubsystem* RS = GetRunStateFromWorld(GetWorld()))
					{
						RS->GenerateBuybackDisplay();
					}
					RefreshBuyback();
				}
				RefreshShopChrome();
				return FReply::Handled();
			}),
			ET66OverlayChromeButtonFamily::Tab)
		.SetMinWidth(0.f)
		.SetPadding(bCompactCasinoLayout ? FMargin(8.f, 5.f) : FMargin(12.f, 8.f))
		.SetFontSize(bCompactCasinoLayout ? 11 : 16)
		.SetContent(
			SAssignNew(ShopModeToggleButtonText, STextBlock)
			.Text(BuybackTitle)
			.Font(FT66Style::Tokens::FontBold(bCompactCasinoLayout ? 11 : 16))
			.ColorAndOpacity(FT66Style::Tokens::Text))
	);

	TSharedRef<SWidget> ContextRerollButton = T66OverlayChromeStyle::MakeButton(
		T66OverlayChromeStyle::MakeButtonParams(
			RerollText,
			FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnReroll),
			ET66OverlayChromeButtonFamily::Neutral)
		.SetMinWidth(0.f)
		.SetPadding(ActionButtonPadding)
		.SetFontSize(bCompactCasinoLayout ? 11 : 16)
	);
	ContextRerollButtonWidget = ContextRerollButton;

	// Build main 3-column row (Stats | Shop | Bank) as a separate widget to avoid Slate parser issues with SBox::FArguments.
	TSharedRef<SWidget> MainRowContent = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, bCompactCasinoLayout ? FT66Style::Tokens::Space3 : FT66Style::Tokens::Space6, 0.f)
		[
			SAssignNew(StatsPanelBox, SBox)
			.WidthOverride(StatsPanelWidth)
			.HeightOverride(MainRowHeight)
			[
				T66StatsPanelSlate::MakeLiveEssentialStatsPanel(RunState, Loc, LiveStatsPanel.ToSharedRef(), StatsPanelWidth, true, StatsPanelFontAdjustment)
			]
		]
		+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, bCompactCasinoLayout ? FT66Style::Tokens::Space3 : FT66Style::Tokens::Space6, 0.f)
		[
			SNew(SBox)
			.MinDesiredHeight(MainRowHeight)
			[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66Style::Tokens::Space4, 0.f)
						[
							ShopModeToggleButton
						]
						+ SHorizontalBox::Slot().AutoWidth()
						[
							ContextRerollButton
						]
					]
					+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, FT66Style::Tokens::Space4, 0.f, 0.f)
					[
						T66OverlayChromeStyle::MakePanel(
							SAssignNew(ShopBuybackSwitcher, SWidgetSwitcher)
							+ SWidgetSwitcher::Slot()
							[
								ShopCardsScroller
							]
							+ SWidgetSwitcher::Slot()
							[
								BuybackCardsScroller
							],
							ET66OverlayChromeBrush::ContentPanelWide,
							FMargin(FT66Style::Tokens::Space6))
					]
					]
				]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 0.f, 0.f)
		[
			SNew(SBox)
			.WidthOverride(RightPanelWidth)
			.MinDesiredHeight(MainRowHeight)
			[
				T66OverlayChromeStyle::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 14.f)
					[
						SNew(SBox)
						.WidthOverride(AngerImageSize)
						.HeightOverride(AngerImageSize)
						[
							SAssignNew(AngerCircleImage, SImage)
							.Image(&AngerFace_Happy)
						]
					]
					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						SNew(SSpacer)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 0.f)
					[
						T66OverlayChromeStyle::MakePanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, FT66Style::Tokens::Space4)
							[
								SNew(STextBlock)
								.Text(BankTitle)
								.TextStyle(&TextHeading)
								.Font(FT66Style::Tokens::FontBold(SectionHeadingFontSize))
								.ColorAndOpacity(FT66Style::Tokens::Text)
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
										.Font(FT66Style::Tokens::FontBold(SpinBoxFontSize))
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
									T66OverlayChromeStyle::MakeButton(
										T66OverlayChromeStyle::MakeButtonParams(
											Loc ? Loc->GetText_Borrow() : NSLOCTEXT("T66.Vendor", "Borrow_Button", "BORROW"),
											FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnBorrowClicked),
											ET66OverlayChromeButtonFamily::Neutral)
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
										.Font(FT66Style::Tokens::FontBold(SpinBoxFontSize))
										.Value(PaybackAmount)
										.OnValueChanged_Lambda([this](int32 V) { PaybackAmount = FMath::Max(0, V); })
									]
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									T66OverlayChromeStyle::MakeButton(
										T66OverlayChromeStyle::MakeButtonParams(
											Loc ? Loc->GetText_Payback() : NSLOCTEXT("T66.Vendor", "Payback_Button", "PAYBACK"),
											FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnPaybackClicked),
											ET66OverlayChromeButtonFamily::Neutral)
										.SetMinWidth(0.f)
										.SetPadding(ActionButtonPadding)
										.SetFontSize(CardButtonFontSize)
									)
								]
							]
						,
							ET66OverlayChromeBrush::InnerPanel,
							FMargin(bCompactCasinoLayout ? FT66Style::Tokens::Space3 : FT66Style::Tokens::Space5))
					]
				,
					ET66OverlayChromeBrush::ContentPanelTall,
					FMargin(bCompactCasinoLayout ? FT66Style::Tokens::Space3 : FT66Style::Tokens::Space6))
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
				.Font(FT66Style::Tokens::FontBold(PageTitleFontSize))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Visibility(bCompactCasinoLayout ? EVisibility::Collapsed : EVisibility::Visible)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, bCompactCasinoLayout ? 0.f : 12.f, 0.f, 0.f)
		[
			SAssignNew(StatusText, STextBlock)
			.Text(FText::GetEmpty())
			.TextStyle(&TextBody)
			.Font(FT66Style::Tokens::FontRegular(StatusFontSize))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, bCompactCasinoLayout ? FT66Style::Tokens::Space4 : FT66Style::Tokens::Space6, 0.f, 0.f)
		[
			MainRowContent
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, bCompactCasinoLayout ? FT66Style::Tokens::Space4 : FT66Style::Tokens::Space6, 0.f, 0.f)
		[
			T66OverlayChromeStyle::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(InventoryTitle)
								.TextStyle(&TextHeading)
								.Font(FT66Style::Tokens::FontBold(SectionHeadingFontSize))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(bCompactCasinoLayout ? 10.f : 18.f, 0.f, bCompactCasinoLayout ? 10.f : 16.f, 0.f)
					[
						SAssignNew(NetWorthText, STextBlock)
						.Text(FText::Format(
							Loc ? Loc->GetText_NetWorthFormat() : NSLOCTEXT("T66.GameplayHUD", "NetWorthFormat", "Net Worth: {0}"),
							FText::AsNumber(0)))
						.TextStyle(&TextHeading)
						.Font(FT66Style::Tokens::FontBold(SectionHeadingFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, bCompactCasinoLayout ? 10.f : 16.f, 0.f)
					[
						SAssignNew(GoldText, STextBlock)
						.Text(FText::Format(
							Loc ? Loc->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}"),
							FText::AsNumber(0)))
						.TextStyle(&TextHeading)
						.Font(FT66Style::Tokens::FontBold(SectionHeadingFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SAssignNew(DebtText, STextBlock)
						.Text(FText::Format(
							Loc ? Loc->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Debt: {0}"),
							FText::AsNumber(0)))
						.TextStyle(&TextHeading)
						.Font(FT66Style::Tokens::FontBold(SectionHeadingFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Danger)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(SSpacer)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f)
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
					+ SHorizontalBox::Slot().AutoWidth().Padding(bCompactCasinoLayout ? FT66Style::Tokens::Space3 : FT66Style::Tokens::Space6, 0.f, 0.f, 0.f)
					[
						// Sell details for selected item (sized to match inventory slot: 160x160)
						SAssignNew(SellPanelContainer, SBox)
						.WidthOverride(SellPanelSize)
						.HeightOverride(SellPanelSize)
						.Visibility(EVisibility::Visible)
						[
							T66OverlayChromeStyle::MakePanel(
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									SAssignNew(SellItemNameText, STextBlock)
									.Text(FText::GetEmpty())
									.TextStyle(&TextHeading)
									.Font(FT66Style::Tokens::FontBold(CardHeadingFontSize))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
								[
									SAssignNew(SellItemDescText, STextBlock)
									.Text(FText::GetEmpty())
									.TextStyle(&TextBody)
									.Font(FT66Style::Tokens::FontRegular(CardBodyFontSize))
									.ColorAndOpacity(FT66Style::Tokens::TextMuted)
									.AutoWrapText(true)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
								[
									SAssignNew(SellItemPriceText, STextBlock)
									.Text(FText::GetEmpty())
									.TextStyle(&TextChip)
									.Font(FT66Style::Tokens::FontBold(CardButtonFontSize))
									.ColorAndOpacity(FT66Style::Tokens::Accent2)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
								[ SellBtnWidget ]
							,
								ET66OverlayChromeBrush::InnerPanel,
								FMargin(bCompactCasinoLayout ? FT66Style::Tokens::Space3 : FT66Style::Tokens::Space4))
						]
					]
				]
			,
				ET66OverlayChromeBrush::ContentPanelWide,
				FMargin(bCompactCasinoLayout ? FT66Style::Tokens::Space3 : FT66Style::Tokens::Space4))
		];

	const TAttribute<FOptionalSize> ShopPageWidthAttr = TAttribute<FOptionalSize>::CreateLambda([this, OverlayPadding]() -> FOptionalSize
	{
		const FVector2D Bounds = bEmbeddedInCasinoShell ? FT66Style::GetViewportLogicalSize() : FT66Style::GetSafeFrameSize();
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
		return bEmbeddedInCasinoShell ? FMargin(0.f) : FT66Style::GetSafeFrameInsets();
	});

	const TAttribute<FMargin> SafeClosePadding = TAttribute<FMargin>::CreateLambda([this, OverlayPadding]() -> FMargin
	{
		const FMargin LocalPadding(
			OverlayPadding,
			OverlayPadding,
			OverlayPadding,
			0.f);
		return bEmbeddedInCasinoShell ? LocalPadding : FT66Style::GetSafePadding(LocalPadding);
	});

	TSharedRef<SWidget> Root =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			T66OverlayChromeStyle::MakePanel(
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
				ET66OverlayChromeBrush::ContentPanelWide,
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
				T66OverlayChromeStyle::MakeButton(
					T66OverlayChromeStyle::MakeButtonParams(CloseText,
						FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnBack),
						ET66OverlayChromeButtonFamily::Danger)
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

	SetPage(EVendorPage::Shop);
	RefreshAll();
	RefreshShopChrome();
	return bEmbeddedInCasinoShell ? Root : FT66Style::MakeResponsiveRoot(Root);
}

void UT66VendorOverlayWidget::HandleGoldOrDebtChanged()
{
	RefreshTopBar();
}

void UT66VendorOverlayWidget::HandleInventoryChanged()
{
	PrimeVisibleItemIconTextures();
	RefreshTopBar();
	RefreshInventory();
	RefreshSellPanel();
	RefreshStatsPanel();
}

void UT66VendorOverlayWidget::HandleVendorChanged()
{
	PrimeVisibleItemIconTextures();
	RefreshShopChrome();
	RefreshTopBar();
	RefreshStock();
}

void UT66VendorOverlayWidget::HandleBuybackChanged()
{
	PrimeVisibleItemIconTextures();
	RefreshShopChrome();
	RefreshTopBar();
	RefreshBuyback();
}

void UT66VendorOverlayWidget::RefreshAll()
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

void UT66VendorOverlayWidget::RefreshShopChrome()
{
	UWorld* World = GetWorld();
	UT66LocalizationSubsystem* Loc = World && World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>()
		: nullptr;
	const FText ShopTitle = Loc ? Loc->GetText_Shop() : NSLOCTEXT("T66.Vendor", "Shop", "SHOP");
	const FText BuybackTitle = NSLOCTEXT("T66.Vendor", "Buyback", "BUYBACK");
	const bool bShowingBuyback = ShopBuybackSwitcher.IsValid() && ShopBuybackSwitcher->GetActiveWidgetIndex() == 1;

	if (ShopPageTitleText.IsValid())
	{
		ShopPageTitleText->SetText(bShowingBuyback ? BuybackTitle : ShopTitle);
	}

	if (ShopModeToggleButtonText.IsValid())
	{
		ShopModeToggleButtonText->SetText(bShowingBuyback ? ShopTitle : BuybackTitle);
	}

	if (ContextRerollButtonWidget.IsValid())
	{
		UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
		const bool bEnabled = RunState
			&& (bShowingBuyback
				? (RunState->GetBuybackPoolSize() > UT66RunStateSubsystem::BuybackDisplaySlotCount)
				: !bCachedBossActive);
		ContextRerollButtonWidget->SetEnabled(bEnabled);
	}

	if (CloseButtonBox.IsValid())
	{
		CloseButtonBox->SetVisibility(bEmbeddedInCasinoShell ? EVisibility::Collapsed : EVisibility::Visible);
	}
}

void UT66VendorOverlayWidget::PrimeVisibleItemIconTextures()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	if (!RunState || !GI || !TexPool)
	{
		return;
	}

	TArray<FSoftObjectPath> IconPaths;
	IconPaths.Reserve(
		UT66RunStateSubsystem::VendorDisplaySlotCount +
		UT66RunStateSubsystem::BuybackDisplaySlotCount +
		UT66RunStateSubsystem::MaxInventorySlots);

	const TArray<FT66InventorySlot>& StockSlots = RunState->GetVendorStockSlots();
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

	const TArray<FT66InventorySlot>& InventorySlots = RunState->GetInventorySlots();
	for (const FT66InventorySlot& InventorySlot : InventorySlots)
	{
		if (!InventorySlot.IsValid())
		{
			continue;
		}

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

void UT66VendorOverlayWidget::RefreshStatsPanel()
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

void UT66VendorOverlayWidget::RefreshTopBar()
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
			? FT66Style::Tokens::Success
			: (NetWorth < 0 ? FT66Style::Tokens::Danger : FT66Style::Tokens::Text);
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

	// Anger face sprites: Happy (0-33%), Neutral (34-66%), Angry (67-99%), boss at 100%
	if (AngerCircleImage.IsValid())
	{
		const int32 Anger = FMath::Clamp(RunState->GetVendorAngerGold(), 0, UT66RunStateSubsystem::VendorAngerThresholdGold);
		const int32 Pct = FMath::RoundToInt((static_cast<float>(Anger) / static_cast<float>(UT66RunStateSubsystem::VendorAngerThresholdGold)) * 100.f);
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
}

FReply UT66VendorOverlayWidget::OnReroll()
{
	if (UWorld* World = GetWorld())
	{
		if (UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World))
		{
			const bool bShowingBuyback = ShopBuybackSwitcher.IsValid() && ShopBuybackSwitcher->GetActiveWidgetIndex() == 1;
			if (bShowingBuyback)
			{
				RunState->RerollBuybackDisplay();
				RefreshBuyback();
				return FReply::Handled();
			}

			if (IsBossActive())
			{
				return FReply::Handled();
			}

			RunState->RerollVendorStockForCurrentStage();
		}
	}
	RefreshAll();
	return FReply::Handled();
}

void UT66VendorOverlayWidget::RefreshStock()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return;

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	const TArray<FName>& Stock = RunState->GetVendorStockItemIDs();
	const TArray<FT66InventorySlot>& StockSlots = RunState->GetVendorStockSlots();
	const int32 SlotCount = ItemNameTexts.Num();
	for (int32 i = 0; i < SlotCount; ++i)
	{
		const bool bHasItem = Stock.IsValidIndex(i) && !Stock[i].IsNone();
		const bool bSold = bHasItem ? RunState->IsVendorStockSlotSold(i) : true;
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
			ItemIconBorders[i]->SetBorderBackgroundColor(bHasData ? FItemData::GetItemRarityColor(SlotRarity) : FT66Style::Tokens::Panel2);
		}
		if (ItemIconBrushes.IsValidIndex(i) && ItemIconBrushes[i].IsValid())
		{
			const TSoftObjectPtr<UTexture2D> SlotIconSoft = bHasData ? D.GetIconForRarity(SlotRarity) : TSoftObjectPtr<UTexture2D>();
			if (!SlotIconSoft.IsNull() && TexPool)
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, SlotIconSoft, this, ItemIconBrushes[i], FName(TEXT("VendorStock"), i + 1), /*bClearWhileLoading*/ false);
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
			ItemTileBorders[i]->SetBorderBackgroundColor(bHasItem ? FT66Style::Tokens::Panel2 : FT66Style::Tokens::Panel);
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
				BuyButtonTexts[i]->SetText(NSLOCTEXT("T66.Vendor", "Sold", "SOLD"));
			}
			else
			{
				const int32 Price = bHasData ? D.GetBuyGoldForRarity(SlotRarity) : 0;
				BuyButtonTexts[i]->SetText(FText::Format(
					NSLOCTEXT("T66.Vendor", "BuyPriceFormat", "BUY ({0}g)"),
					FText::AsNumber(Price)));
			}
		}
		if (StealButtons.IsValidIndex(i) && StealButtons[i].IsValid())
		{
			StealButtons[i]->SetVisibility(bVendorAllowsSteal ? EVisibility::Visible : EVisibility::Collapsed);
			StealButtons[i]->SetEnabled(bVendorAllowsSteal && bHasItem && !bSold && bBoughtSomethingThisVisit && !IsBossActive());
		}
	}
}

void UT66VendorOverlayWidget::RefreshBuyback()
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
			BuybackIconBorders[i]->SetBorderBackgroundColor(bHasData ? FItemData::GetItemRarityColor(SlotRarity) : FT66Style::Tokens::Panel2);
		}
		if (BuybackIconBrushes.IsValidIndex(i) && BuybackIconBrushes[i].IsValid())
		{
			const TSoftObjectPtr<UTexture2D> SlotIconSoft = bHasData ? D.GetIconForRarity(SlotRarity) : TSoftObjectPtr<UTexture2D>();
			if (!SlotIconSoft.IsNull() && TexPool)
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, SlotIconSoft, this, BuybackIconBrushes[i], FName(TEXT("VendorBuyback"), i + 1), /*bClearWhileLoading*/ false);
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
			BuybackTileBorders[i]->SetBorderBackgroundColor(FT66Style::Tokens::Panel2);
		}
		if (BuybackPriceTexts.IsValidIndex(i) && BuybackPriceTexts[i].IsValid())
		{
			BuybackPriceTexts[i]->SetText(bHasSlot
				? FText::Format(NSLOCTEXT("T66.Vendor", "BuyPriceFormat", "BUY ({0}g)"), FText::AsNumber(SellPrice > 0 ? SellPrice : 1))
				: (Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY")));
		}
		if (BuybackBuyButtons.IsValidIndex(i) && BuybackBuyButtons[i].IsValid())
		{
			BuybackBuyButtons[i]->SetEnabled(bHasSlot && RunState->GetCurrentGold() >= (SellPrice > 0 ? SellPrice : 1) && RunState->HasInventorySpace());
		}
	}
}

void UT66VendorOverlayWidget::SetPage(EVendorPage Page)
{
	if (!PageSwitcher.IsValid()) return;
	PageSwitcher->SetActiveWidgetIndex(static_cast<int32>(Page));
	RefreshShopChrome();
}

void UT66VendorOverlayWidget::OpenShopPage()
{
	SetPage(EVendorPage::Shop);
	if (ShopBuybackSwitcher.IsValid())
	{
		ShopBuybackSwitcher->SetActiveWidgetIndex(0);
	}
	RefreshAll();
}

void UT66VendorOverlayWidget::RefreshInventory()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return;

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	const TArray<FName>& Inv = RunState->GetInventory();
	const TArray<FT66InventorySlot>& InvSlots = RunState->GetInventorySlots();
	TMap<FString, int32> StackCounts;
	for (const FT66InventorySlot& InventorySlotData : InvSlots)
	{
		if (InventorySlotData.IsValid())
		{
			StackCounts.FindOrAdd(MakeInventoryStackKey(InventorySlotData))++;
		}
	}

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
		const int32 StackCount = (bHasItem && InvSlots.IsValidIndex(i) && InvSlots[i].IsValid())
			? StackCounts.FindRef(MakeInventoryStackKey(InvSlots[i]))
			: 0;
		if (InventorySlotCountTexts.IsValidIndex(i) && InventorySlotCountTexts[i].IsValid())
		{
			InventorySlotCountTexts[i]->SetText(
				StackCount > 1
					? FText::Format(NSLOCTEXT("T66.Inventory", "StackCountFormat", "{0}x"), FText::AsNumber(StackCount))
					: FText::GetEmpty());
			InventorySlotCountTexts[i]->SetVisibility(StackCount > 1 ? EVisibility::Visible : EVisibility::Hidden);
		}
		if (InventorySlotBorders.IsValidIndex(i) && InventorySlotBorders[i].IsValid())
		{
			FLinearColor Fill = FT66Style::Tokens::Panel2;
			FItemData D;
			const bool bHasData = bHasItem && GI && GI->GetItemData(Inv[i], D);

			if (i == SelectedInventoryIndex)
			{
				Fill = (Fill * 0.45f + FT66Style::Tokens::Accent * 0.55f);
			}
			InventorySlotBorders[i]->SetBorderBackgroundColor(Fill);

			if (InventorySlotIconBrushes.IsValidIndex(i) && InventorySlotIconBrushes[i].IsValid())
			{
				const ET66ItemRarity SlotRarity = (bHasData && InvSlots.IsValidIndex(i)) ? InvSlots[i].Rarity : ET66ItemRarity::Black;
				const TSoftObjectPtr<UTexture2D> SlotIconSoft = bHasData ? D.GetIconForRarity(SlotRarity) : TSoftObjectPtr<UTexture2D>();
				if (!SlotIconSoft.IsNull() && TexPool)
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, SlotIconSoft, this, InventorySlotIconBrushes[i], FName(TEXT("VendorInv"), i + 1), /*bClearWhileLoading*/ false);
				}
				else
				{
					InventorySlotIconBrushes[i]->SetResourceObject(nullptr);
				}
			}
			if (InventorySlotIconImages.IsValidIndex(i) && InventorySlotIconImages[i].IsValid())
			{
				const ET66ItemRarity SlotRarity = (bHasData && InvSlots.IsValidIndex(i)) ? InvSlots[i].Rarity : ET66ItemRarity::Black;
				const bool bHasIcon = bHasData && !D.GetIconForRarity(SlotRarity).IsNull();
				InventorySlotIconImages[i]->SetVisibility(bHasIcon ? EVisibility::Visible : EVisibility::Hidden);
			}
		}
	}
}

void UT66VendorOverlayWidget::RefreshSellPanel()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
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
			NSLOCTEXT("T66.Vendor", "SellForFormat", "SELL FOR: {0}g"),
			FText::AsNumber(SellValue)));
	}
	if (SellItemButton.IsValid())
	{
		SellItemButton->SetEnabled(!IsBossActive());
	}
}

FReply UT66VendorOverlayWidget::OnBack()
{
	CloseOverlay();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnBorrowClicked()
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
			StatusText->SetText(Loc ? Loc->GetText_BorrowAmountMustBePositive() : NSLOCTEXT("T66.Vendor", "BorrowMustBePositive", "Borrow amount must be > 0."));
		}
		return FReply::Handled();
	}
	if (!RunState->BorrowGold(BorrowAmount))
	{
		BorrowAmount = FMath::Clamp(BorrowAmount, 0, RunState->GetRemainingBorrowCapacity());
		if (StatusText.IsValid())
		{
			const FText Fmt = Loc ? Loc->GetText_BorrowExceedsNetWorthFormat() : NSLOCTEXT("T66.Vendor", "BorrowExceedsNetWorthFormat", "Borrow amount exceeds remaining Net Worth ({0}).");
			StatusText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetRemainingBorrowCapacity())));
		}
		RefreshTopBar();
		return FReply::Handled();
	}
	if (StatusText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_BorrowedAmountFormat() : NSLOCTEXT("T66.Vendor", "BorrowedAmountFormat", "Borrowed {0}.");
		StatusText->SetText(FText::Format(Fmt, FText::AsNumber(BorrowAmount)));
	}
	RefreshTopBar();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnPaybackClicked()
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
			StatusText->SetText(Loc ? Loc->GetText_PaybackAmountMustBePositive() : NSLOCTEXT("T66.Vendor", "PaybackMustBePositive", "Payback amount must be > 0."));
		}
		return FReply::Handled();
	}
	const int32 Paid = RunState->PayDebt(PaybackAmount);
	if (StatusText.IsValid()) StatusText->SetText(FText::GetEmpty());
	RefreshTopBar();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnSelectInventorySlot(int32 InventoryIndex)
{
	SelectedInventoryIndex = InventoryIndex;
	RefreshInventory();
	RefreshSellPanel();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnSellSelectedClicked()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return FReply::Handled();

	if (SelectedInventoryIndex < 0)
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "SelectItemToSell", "Select an item to sell."));
		return FReply::Handled();
	}

	const bool bSold = RunState->SellInventoryItemAt(SelectedInventoryIndex);
	if (bSold)
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "SoldStatus", "Sold."));
	}
	else
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "CouldNotSell", "Could not sell."));
	}
	RefreshAll();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnBuySlot(int32 SlotIndex)
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return FReply::Handled();

	if (IsBossActive())
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "BossIsActive", "Boss is active."));
		return FReply::Handled();
	}

	if (!RunState->HasInventorySpace())
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "InventoryFull", "Inventory full."));
		return FReply::Handled();
	}

	const bool bBought = RunState->TryBuyVendorStockSlot(SlotIndex);
	if (bBought)
	{
		bBoughtSomethingThisVisit = true;
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
			{
				Achieve->NotifyVendorPurchase();
			}
		}
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "Purchased", "Purchased."));
	}
	else
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "CouldNotPurchase", "Could not purchase."));
	}
	RefreshAll();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnBuybackSlot(int32 SlotIndex)
{
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(GetWorld());
	if (!RunState) return FReply::Handled();

	const bool bBought = RunState->TryBuybackSlot(SlotIndex);
	if (StatusText.IsValid())
	{
		StatusText->SetText(bBought
			? NSLOCTEXT("T66.Vendor", "Purchased", "Purchased.")
			: NSLOCTEXT("T66.Vendor", "CouldNotPurchase", "Could not purchase."));
	}
	RefreshAll();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnStealSlot(int32 SlotIndex)
{
	if (!bVendorAllowsSteal)
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "StealDisabled", "Stealing is disabled here."));
		return FReply::Handled();
	}
	if (!bBoughtSomethingThisVisit)
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "BuyOneBeforeStealing", "Buy one item before stealing."));
		return FReply::Handled();
	}
	if (IsBossActive())
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "BossIsActive", "Boss is active."));
		return FReply::Handled();
	}
	ShowStealPrompt(SlotIndex);
	return FReply::Handled();
}

void UT66VendorOverlayWidget::ShowStealPrompt(int32 SlotIndex)
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
		// 30Hz is sufficient for this UI and reduces overhead on low-end CPUs.
		World->GetTimerManager().SetTimer(StealTickTimerHandle, this, &UT66VendorOverlayWidget::TickStealBar, 0.033f, true);
	}
}

void UT66VendorOverlayWidget::HideStealPrompt()
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

void UT66VendorOverlayWidget::TickStealBar()
{
	// Bounce marker back and forth.
	const float Speed = 1.6f; // cycles per second-ish
	UWorld* World = GetWorld();
	if (!World) return;
	const float Now = static_cast<float>(World->GetTimeSeconds());
	float Delta = Now - StealLastTickTimeSeconds;
	StealLastTickTimeSeconds = Now;
	Delta = FMath::Clamp(Delta, 0.f, 0.05f);
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

FReply UT66VendorOverlayWidget::OnStealStop()
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
	const bool bGranted = RunState->ResolveVendorStealAttempt(PendingStealIndex, bTimingHit, false);
	HideStealPrompt();

	if (StatusText.IsValid())
	{
		const ET66VendorStealOutcome Outcome = RunState->GetLastVendorStealOutcome();
		if (Outcome == ET66VendorStealOutcome::Miss) StatusText->SetText(NSLOCTEXT("T66.Vendor", "StealFailedMiss", "Steal failed (miss). Anger increased."));
		else if (Outcome == ET66VendorStealOutcome::Failed) StatusText->SetText(NSLOCTEXT("T66.Vendor", "StealFailed", "Steal failed. Anger increased."));
		else if (Outcome == ET66VendorStealOutcome::InventoryFull) StatusText->SetText(NSLOCTEXT("T66.Vendor", "StealSucceededButInventoryFull", "Steal succeeded, but inventory is full."));
		else if (Outcome == ET66VendorStealOutcome::Success && bGranted) StatusText->SetText(NSLOCTEXT("T66.Vendor", "Stolen", "Stolen."));
		else StatusText->SetText(NSLOCTEXT("T66.Vendor", "StealFailed", "Steal failed. Anger increased."));
	}

	RefreshAll();
	TriggerVendorBossIfAngry();
	return FReply::Handled();
}

bool UT66VendorOverlayWidget::IsBossActive() const
{
	return bCachedBossActive;
}

void UT66VendorOverlayWidget::HandleBossChanged()
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

void UT66VendorOverlayWidget::TriggerVendorBossIfAngry()
{
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->TriggerCasinoBossIfAngry();
	}
}

