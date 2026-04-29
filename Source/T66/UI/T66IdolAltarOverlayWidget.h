// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "T66IdolAltarOverlayWidget.generated.h"

class SBorder;
class SBox;
class SImage;
class SWidget;
class SRichTextBlock;
class STextBlock;
class SWidget;
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

	TArray<TSharedPtr<SBox>> OfferCardBoxes;
	TArray<TSharedPtr<STextBlock>> OfferNameTexts;
	TArray<TSharedPtr<SRichTextBlock>> OfferDescriptionTexts;
	TArray<TSharedPtr<SImage>> OfferIconImages;
	TArray<TSharedPtr<FSlateBrush>> OfferIconBrushes;
	TArray<TSharedPtr<SBorder>> OfferTileBorders;
	TArray<TSharedPtr<SBorder>> OfferIconBorders;
	TArray<TSharedPtr<SWidget>> OfferButtons;
	TArray<TSharedPtr<SBorder>> OfferButtonBorders;
	TArray<TSharedPtr<STextBlock>> OfferButtonTexts;

	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<SWidget> RerollButton;
	TSharedPtr<SBorder> RerollButtonBorder;
	TSharedPtr<STextBlock> RerollButtonText;
	TWeakObjectPtr<AT66IdolAltar> SourceAltar;
	int32 ActiveOfferCategoryIndex = 0;

	FReply OnReroll();
	FReply OnToggleSlot(int32 SlotIndex);
	FReply OnBack();

	void RefreshStock();
	bool HasSelectionsRemaining() const;
	void ConsumeSelectionBudget(int32 SlotIndex);
	void RefundSelectionBudget(int32 SlotIndex);
	int32 GetOfferStockIndexForVisibleSlot(int32 VisibleSlotIndex) const;
	bool IsTutorialSingleOfferMode() const;
	bool IsWeaponOfferMode() const;
	FName GetTutorialOfferedIdolID() const;

	UFUNCTION()
	void HandleIdolsChanged();
	void HandleWeaponsChanged();
};
