// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "T66HeroGridScreen.generated.h"

class UT66LocalizationSubsystem;
class UTexture2D;

/**
 * Hero Grid Modal - overlay showing a grid of all heroes by their placeholder/sprite color.
 * Clicking a hero selects that hero on the underlying Hero Selection screen and closes the modal.
 */
UCLASS(Blueprintable)
class T66_API UT66HeroGridScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66HeroGridScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	TArray<FName> AllHeroIDs;
	TArray<TSharedPtr<struct FSlateBrush>> HeroPortraitBrushes;

	// GC safety: Slate brushes do not keep UTexture2D resources alive.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTexture2D>> HeroPortraitTextures;

	UT66LocalizationSubsystem* GetLocSubsystem() const;
	FReply HandleHeroClicked(FName HeroID);
	FReply HandleCloseClicked();
};
