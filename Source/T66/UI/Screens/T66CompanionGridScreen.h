// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "T66CompanionGridScreen.generated.h"

class UT66LocalizationSubsystem;

/**
 * Companion Grid Modal - overlay showing a grid of all companions (and NO COMPANION) by color.
 * Clicking a companion selects it on the underlying Companion Selection screen and closes the modal.
 */
UCLASS(Blueprintable)
class T66_API UT66CompanionGridScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66CompanionGridScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	TArray<FName> AllCompanionIDs;
	/** Portrait brushes for grid cells (1:1 ratio, 256x256). */
	TArray<TSharedPtr<struct FSlateBrush>> CompanionPortraitBrushes;

	UT66LocalizationSubsystem* GetLocSubsystem() const;
	FReply HandleCompanionClicked(FName CompanionID);
	FReply HandleCloseClicked();
};
