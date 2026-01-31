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

/** Full-screen, non-pausing Vendor shop UI (buy + steal + loans). */
UCLASS(Blueprintable)
class T66_API UT66VendorOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

	void CloseOverlay();

private:
	// Top bar
	TSharedPtr<STextBlock> GoldText;
	TSharedPtr<STextBlock> DebtText;
	TSharedPtr<STextBlock> StatusText;

	// Anger meter (0..1)
	TSharedPtr<SBox> AngerFillBox;
	TSharedPtr<STextBlock> AngerText;

	// Vendor stock widgets
	TArray<TSharedPtr<STextBlock>> ItemNameTexts;
	TArray<TSharedPtr<STextBlock>> ItemDescTexts;
	TArray<TSharedPtr<STextBlock>> ItemPriceTexts;
	TArray<TSharedPtr<SBorder>> ItemTileBorders;
	TArray<TSharedPtr<SButton>> BuyButtons;
	TArray<TSharedPtr<SButton>> StealButtons;

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

	FReply OnBack();
	FReply OnBorrowClicked();
	FReply OnPaybackClicked();
	FReply OnSellFirstClicked();

	FReply OnBuySlot(int32 SlotIndex);
	FReply OnStealSlot(int32 SlotIndex);

	void ShowStealPrompt(int32 SlotIndex);
	void HideStealPrompt();
	void TickStealBar();
	FReply OnStealStop();

	void TriggerVendorBossIfAngry();
	bool IsBossActive() const;
};

