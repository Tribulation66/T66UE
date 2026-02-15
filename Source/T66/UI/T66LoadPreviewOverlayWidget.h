// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "T66LoadPreviewOverlayWidget.generated.h"

/**
 * Overlay shown when entering gameplay in "preview" mode (load save frozen).
 * Shows a LOAD button at bottom-left; clicking it unfreezes the game and removes the overlay.
 */
UCLASS(Blueprintable)
class T66_API UT66LoadPreviewOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	FReply OnLoadClicked();
};
