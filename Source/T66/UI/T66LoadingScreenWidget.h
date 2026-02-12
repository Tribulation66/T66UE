// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "T66LoadingScreenWidget.generated.h"

class STextBlock;

/**
 * Full-screen loading overlay shown during level transitions (e.g. "Enter the Tribulation").
 * Covers the viewport with a solid background and localized loading text so the player
 * doesn't see a frozen frame while the level loads.
 */
UCLASS(Blueprintable)
class T66_API UT66LoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
