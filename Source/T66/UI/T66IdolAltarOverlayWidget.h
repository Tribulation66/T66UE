// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "T66IdolAltarOverlayWidget.generated.h"

class SBorder;
class STextBlock;
class SImage;
class SWidget;
struct FSlateBrush;

/**
 * Idol Altar overlay — shop-style panel showing 3 idol cards with SELECT + REROLL.
 */
UCLASS(Blueprintable)
class T66_API UT66IdolAltarOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

private:
	static constexpr int32 SlotCount = 3;

	// Per-slot UI references
	TArray<TSharedPtr<STextBlock>> IdolNameTexts;
	TArray<TSharedPtr<STextBlock>> IdolDescTexts;
	TArray<TSharedPtr<SImage>> IdolIconImages;
	TArray<TSharedPtr<FSlateBrush>> IdolIconBrushes;
	TArray<TSharedPtr<SBorder>> IdolTileBorders;
	TArray<TSharedPtr<SBorder>> IdolIconBorders;
	TArray<TSharedPtr<SWidget>> SelectButtons;
	TArray<TSharedPtr<STextBlock>> SelectButtonTexts;

	TSharedPtr<STextBlock> StatusText;

	// Actions
	FReply OnSelectSlot(int32 SlotIndex);
	FReply OnReroll();
	FReply OnBack();

	// Refresh
	void RefreshStock();

	// Delegate binding
	UFUNCTION()
	void HandleIdolsChanged();
};
