// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "Styling/SlateBrush.h"
#include "T66CasinoVendorTabWidget.generated.h"

class STextBlock;
class SButton;
template<typename NumericType> class SSpinBox;
class SBox;
class SBorder;
class SImage;
class SUniformGridPanel;
class SWidgetSwitcher;
struct FSlateBrush;
namespace T66StatsPanelSlate { struct FT66LiveStatsPanel; }

/** Full-screen, non-pausing Shop UI (buy + steal + loans). */
UCLASS(Blueprintable)
class T66_API UT66CasinoVendorTabWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	static constexpr int32 SellVisibleSlotCount = 4;

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	void CloseOverlay();
	void SetEmbeddedInCasinoShell(bool bEmbedded) { bEmbeddedInCasinoShell = bEmbedded; }
	void SetShopAllowsSteal(bool bEnabled) { bShopAllowsSteal = bEnabled; }

	/** Skip dialogue and open shop page (used by in-world dialogue). */
	void OpenShopPage();

private:
	enum class EShopPage : uint8
	{
		Shop = 0,
	};

	enum class EShopMode : uint8
	{
		Buy = 0,
		Sell = 1,
		Buyback = 2,
	};

	TSharedPtr<SWidgetSwitcher> PageSwitcher;
	TSharedPtr<STextBlock> ShopPageTitleText;
	TSharedPtr<SWidget> ContextRerollButtonWidget;
	TSharedPtr<SBox> CloseButtonBox;
	EShopMode ActiveShopMode = EShopMode::Buy;

	// Bottom bar (next to inventory title)
	TSharedPtr<STextBlock> NetWorthText;
	TSharedPtr<STextBlock> GoldText;
	TSharedPtr<STextBlock> DebtText;
	TSharedPtr<STextBlock> StatusText;

	// Stats panel (refreshable when inventory changes)
	TSharedPtr<SBox> StatsPanelBox;
	TSharedPtr<T66StatsPanelSlate::FT66LiveStatsPanel> LiveStatsPanel;

	// Shop stock widgets
	TArray<TSharedPtr<STextBlock>> ItemNameTexts;
	TArray<TSharedPtr<STextBlock>> ItemDescTexts;
	TArray<TSharedPtr<STextBlock>> ItemPriceTexts;
	TArray<TSharedPtr<SBorder>> ItemTileBorders;
	TArray<TSharedPtr<SBorder>> ItemIconBorders;
	TArray<TSharedPtr<SImage>> ItemIconImages;
	TArray<TSharedPtr<FSlateBrush>> ItemIconBrushes;
	TArray<TSharedPtr<SWidget>> BuyButtons;
	TArray<TSharedPtr<SWidget>> StealButtons;
	TArray<TSharedPtr<STextBlock>> BuyButtonTexts;

	// Shop tab: 0 = Buy, 1 = Sell, 2 = Buyback
	TSharedPtr<SWidgetSwitcher> ShopBuybackSwitcher;
	TArray<TSharedPtr<STextBlock>> BuybackNameTexts;
	TArray<TSharedPtr<STextBlock>> BuybackDescTexts;
	TArray<TSharedPtr<STextBlock>> BuybackPriceTexts;
	TArray<TSharedPtr<SBorder>> BuybackTileBorders;
	TArray<TSharedPtr<SBorder>> BuybackIconBorders;
	TArray<TSharedPtr<SImage>> BuybackIconImages;
	TArray<TSharedPtr<FSlateBrush>> BuybackIconBrushes;
	TArray<TSharedPtr<SWidget>> BuybackBuyButtons;

	// Sell strip: visible rotating window over the full backing inventory.
	static constexpr int32 InventorySlotCount = SellVisibleSlotCount;
	TArray<TSharedPtr<SBorder>> InventorySlotBorders;
	TArray<TSharedPtr<SWidget>> InventorySlotButtons;
	TArray<TSharedPtr<STextBlock>> InventorySlotTexts;
	TArray<TSharedPtr<STextBlock>> InventorySlotDescTexts;
	TArray<TSharedPtr<STextBlock>> InventorySlotCountTexts;
	TArray<TSharedPtr<STextBlock>> InventorySlotActionTexts;
	TArray<TSharedPtr<SImage>> InventorySlotIconImages;
	TArray<TSharedPtr<FSlateBrush>> InventorySlotIconBrushes;
	TSharedPtr<STextBlock> InventoryPageText;
	TSharedPtr<SWidget> InventoryRotateButtonWidget;
	int32 SelectedInventoryIndex = -1;
	int32 SellInventoryPageIndex = 0;

	// Loans/sell controls
	TSharedPtr<SSpinBox<int32>> BorrowAmountSpin;
	TSharedPtr<SSpinBox<int32>> PaybackAmountSpin;
	int32 BorrowAmount = 0;
	int32 PaybackAmount = 0;

	// Steal prompt
	TSharedPtr<SBox> StealPromptContainer;
	TSharedPtr<SBox> StealMarkerSpacerBox;
	bool bStealPromptVisible = false;
	int32 PendingStealIndex = -1;
	float StealMarker01 = 0.f;
	bool bStealForward = true;
	float StealLastTickTimeSeconds = 0.f;
	FTimerHandle StealTickTimerHandle;

	// Per-visit gating: must buy once before any stealing.
	bool bBoughtSomethingThisVisit = false;
	bool bCachedBossActive = false;

	void RefreshAll();
	void RefreshShopChrome();
	void RefreshTopBar();
	void RefreshStock();
	void RefreshBuyback();
	void RefreshInventory();
	void RefreshSellPanel();
	void RefreshStatsPanel();
	void PrimeVisibleItemIconTextures();
	void ReleaseCachedSlateResources();

	FReply OnReroll();
	void SetPage(EShopPage Page);
	void SetShopMode(EShopMode Mode);

	FReply OnBack();
	FReply OnBorrowClicked();
	FReply OnPaybackClicked();
	FReply OnSelectInventorySlot(int32 InventoryIndex);
	FReply OnRotateInventorySlotsClicked();
	FReply OnSellSelectedClicked();
	FReply OnSellSlotClicked(int32 DisplaySlotIndex);

	FReply OnBuySlot(int32 SlotIndex);
	FReply OnBuybackSlot(int32 SlotIndex);
	FReply OnStealSlot(int32 SlotIndex);

	void ShowStealPrompt(int32 SlotIndex);
	void HideStealPrompt();
	void TickStealBar();
	FReply OnStealStop();

	void SpawnVendorBoss();
	bool IsBossActive() const;

	UFUNCTION()
	void HandleGoldOrDebtChanged();

	UFUNCTION()
	void HandleInventoryChanged();

	UFUNCTION()
	void HandleShopChanged();

	UFUNCTION()
	void HandleBuybackChanged();

	UFUNCTION()
	void HandleBossChanged();

	bool bEmbeddedInCasinoShell = false;
	bool bShopAllowsSteal = true;
};
