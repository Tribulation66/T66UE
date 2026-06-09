// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "T66WeaponAltarOverlayWidget.generated.h"

class AT66WeaponAltar;
class SBorder;
class SBox;
class SImage;
class STextBlock;
class SWidget;
struct FSlateBrush;

UCLASS(Blueprintable)
class T66_API UT66WeaponAltarOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

	void SetSourceAltar(AT66WeaponAltar* InSourceAltar) { SourceAltar = InSourceAltar; }

private:
	static constexpr int32 OfferSlotCount = 5;

	TArray<TSharedPtr<SBox>> OfferCardBoxes;
	TArray<TSharedPtr<STextBlock>> OfferNameTexts;
	TArray<TSharedPtr<STextBlock>> OfferDescriptionTexts;
	TArray<TSharedPtr<SImage>> OfferIconImages;
	TArray<TSharedPtr<FSlateBrush>> OfferIconBrushes;
	TArray<TSharedPtr<SBorder>> OfferTileBorders;
	TArray<TSharedPtr<SBorder>> OfferIconBorders;
	TArray<TSharedPtr<SWidget>> OfferButtons;
	TArray<TSharedPtr<SBorder>> OfferButtonBorders;
	TArray<TSharedPtr<STextBlock>> OfferButtonTexts;

	TSharedPtr<STextBlock> StatusText;
	TWeakObjectPtr<AT66WeaponAltar> SourceAltar;

	FReply OnChooseSlot(int32 SlotIndex);
	FReply OnBack();

	void RefreshOffers();
	bool HasSelectionsRemaining() const;
	void ConsumeSelectionBudget();
	void CloseAndReturnToGameplay();
	void HandleWeaponsChanged();
};
