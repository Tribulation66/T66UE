// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "T66IdolAltarOverlayWidget.generated.h"

class SBorder;
class STextBlock;
class ST66IdolDropTarget;
class UTexture2D;

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
	virtual void NativeDestruct() override;

private:
	FName PendingSelectedIdolID = NAME_None;

	TSharedPtr<ST66IdolDropTarget> CenterPadBorder;
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> HoverTooltipText;

	/** Returns a stable brush pointer for an idol's icon (or nullptr). */
	const FSlateBrush* GetOrCreateIdolIconBrush(FName IdolID);

	/**
	 * Slate icon brush lifetime:
	 * - Our idol tiles pass a raw FSlateBrush* into SImage.
	 * - If the brush storage is local to RebuildWidget(), Slate can later dereference a dangling pointer and crash.
	 * Keep brushes (and their textures) alive for the lifetime of the overlay widget instance.
	 */
	TMap<FName, TSharedPtr<FSlateBrush>> IdolIconBrushes;

	/** Strong refs to keep icon textures from being GC'd while the overlay is open. */
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UTexture2D>> IdolIconTextureRefs;

	FReply OnConfirm();
	FReply OnBack();

	void RefreshCenterPad();
};

