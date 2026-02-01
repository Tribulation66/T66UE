// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "T66VendorOverlayWidget.generated.h"

class STextBlock;
class SButton;
template<typename NumericType> class SSpinBox;
class SBox;
class SBorder;
class SImage;
class SUniformGridPanel;
class SWidgetSwitcher;

/** Full-screen, non-pausing Vendor shop UI (buy + steal + loans). */
UCLASS(Blueprintable)
class T66_API UT66VendorOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

	void CloseOverlay();

	/** Skip dialogue and open shop page (used by in-world dialogue). */
	void OpenShopPage();

private:
	enum class EVendorPage : uint8
	{
		Dialogue = 0,
		Shop = 1,
	};

	TSharedPtr<SWidgetSwitcher> PageSwitcher;

	// Bottom bar (next to inventory title)
	TSharedPtr<STextBlock> GoldText;
	TSharedPtr<STextBlock> DebtText;
	TSharedPtr<STextBlock> StatusText;

	// Anger indicator (0..1) - circle color + optional text
	TSharedPtr<SImage> AngerCircleImage;

	// Vendor stock widgets
	TArray<TSharedPtr<STextBlock>> ItemNameTexts;
	TArray<TSharedPtr<STextBlock>> ItemDescTexts;
	TArray<TSharedPtr<STextBlock>> ItemPriceTexts;
	TArray<TSharedPtr<SBorder>> ItemTileBorders;
	TArray<TSharedPtr<SBorder>> ItemIconBorders;
	TArray<TSharedPtr<SButton>> BuyButtons;
	TArray<TSharedPtr<SButton>> StealButtons;
	TArray<TSharedPtr<STextBlock>> BuyButtonTexts;

	// Inventory strip
	static constexpr int32 InventorySlotCount = 5;
	TArray<TSharedPtr<SBorder>> InventorySlotBorders;
	TArray<TSharedPtr<SButton>> InventorySlotButtons;
	TArray<TSharedPtr<STextBlock>> InventorySlotTexts;
	int32 SelectedInventoryIndex = -1;

	// Sell panel
	TSharedPtr<SBox> SellPanelContainer;
	TSharedPtr<STextBlock> SellItemNameText;
	TSharedPtr<STextBlock> SellItemDescText;
	TSharedPtr<STextBlock> SellItemPriceText;
	TSharedPtr<SButton> SellItemButton;

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
	FTimerHandle StealTickTimerHandle;

	// Per-visit gating: must buy once before any stealing.
	bool bBoughtSomethingThisVisit = false;

	void RefreshAll();
	void RefreshTopBar();
	void RefreshStock();
	void RefreshInventory();
	void RefreshSellPanel();

	FReply OnReroll();
	FReply OnDialogueShop();
	FReply OnDialogueTeleport();
	void SetPage(EVendorPage Page);
	void TeleportToGambler();

	FReply OnBack();
	FReply OnBorrowClicked();
	FReply OnPaybackMax();
	FReply OnPaybackClicked();
	FReply OnSelectInventorySlot(int32 InventoryIndex);
	FReply OnSellSelectedClicked();

	FReply OnBuySlot(int32 SlotIndex);
	FReply OnStealSlot(int32 SlotIndex);

	void ShowStealPrompt(int32 SlotIndex);
	void HideStealPrompt();
	void TickStealBar();
	FReply OnStealStop();

	void TriggerVendorBossIfAngry();
	bool IsBossActive() const;

	UFUNCTION()
	void HandleGoldOrDebtChanged();

	UFUNCTION()
	void HandleInventoryChanged();

	UFUNCTION()
	void HandleVendorChanged();
};

