// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "T66IdolAltarOverlayWidget.generated.h"

class SBorder;
class STextBlock;
class ST66IdolDropTarget;

/**
 * Idol Altar overlay (Stage 1):
 * - 12 idol tiles around a center drop pad (4x4 outer ring, center spans 2x2)
 * - Drag idol -> drop on center pad -> press Confirm to equip into slot 1
 */
UCLASS(Blueprintable)
class T66_API UT66IdolAltarOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	FName PendingSelectedIdolID = NAME_None;

	TSharedPtr<ST66IdolDropTarget> CenterPadBorder;
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> HoverTooltipText;

	FReply OnConfirm();
	FReply OnBack();

	void RefreshCenterPad();
};

