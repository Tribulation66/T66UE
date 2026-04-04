// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "T66IdolAltarOverlayWidget.generated.h"

class SBorder;
class SBox;
class SButton;
class SImage;
class STextBlock;
class SWidget;
class SWidgetSwitcher;
struct FSlateBrush;
class AT66IdolAltar;

UCLASS(Blueprintable)
class T66_API UT66IdolAltarOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;
	void SetSourceAltar(AT66IdolAltar* InSourceAltar) { SourceAltar = InSourceAltar; }

private:
	static constexpr int32 OfferSlotCount = 16;
	static constexpr int32 OfferSlotsPerCategory = 4;
	static constexpr int32 OfferCategoryCount = 4;
	static constexpr int32 TradeSlotCount = 4;

	TSharedPtr<STextBlock> CategoryTitleText;
	TArray<TSharedPtr<SBox>> OfferCardBoxes;
	TArray<TSharedPtr<STextBlock>> OfferNameTexts;
	TArray<TSharedPtr<SImage>> OfferIconImages;
	TArray<TSharedPtr<FSlateBrush>> OfferIconBrushes;
	TArray<TSharedPtr<SBorder>> OfferTileBorders;
	TArray<TSharedPtr<SBorder>> OfferIconBorders;
	TArray<TSharedPtr<SButton>> OfferButtons;
	TArray<TSharedPtr<SBorder>> OfferButtonBorders;
	TArray<TSharedPtr<STextBlock>> OfferButtonTexts;

	TArray<TSharedPtr<SBox>> TradeCardBoxes;
	TArray<TSharedPtr<STextBlock>> TradeNameTexts;
	TArray<TSharedPtr<SImage>> TradeIconImages;
	TArray<TSharedPtr<FSlateBrush>> TradeIconBrushes;
	TArray<TSharedPtr<SBorder>> TradeTileBorders;
	TArray<TSharedPtr<SBorder>> TradeIconBorders;
	TArray<TSharedPtr<SButton>> TradeButtons;
	TArray<TSharedPtr<SBorder>> TradeButtonBorders;
	TArray<TSharedPtr<STextBlock>> TradeButtonTexts;

	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> OffersTabText;
	TSharedPtr<STextBlock> TradeTabText;
	TSharedPtr<SBorder> OffersTabBorder;
	TSharedPtr<SBorder> TradeTabBorder;
	TSharedPtr<SWidgetSwitcher> ViewSwitcher;
	TSharedPtr<SButton> RerollButton;
	TSharedPtr<SBorder> RerollButtonBorder;
	TSharedPtr<STextBlock> RerollButtonText;
	TWeakObjectPtr<AT66IdolAltar> SourceAltar;
	int32 ActiveViewIndex = 0;
	int32 ActiveOfferCategoryIndex = 0;
	FText OffersButtonLabel;
	FText TradeButtonLabel;

	FReply OnSelectSlot(int32 SlotIndex);
	FReply OnSellSlot(int32 SlotIndex);
	FReply OnReroll();
	FReply OnShowOffers();
	FReply OnShowTrade();
	FReply OnBack();

	void RefreshStock();
	void RefreshTrade();
	void RefreshViewState();
	bool HasSelectionsRemaining() const;
	bool ConsumeSelectionBudget();
	int32 GetOfferStockIndexForVisibleSlot(int32 VisibleSlotIndex) const;

	UFUNCTION()
	void HandleIdolsChanged();
};
