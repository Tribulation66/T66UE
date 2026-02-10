// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "T66PowerUpScreen.generated.h"

class UT66LocalizationSubsystem;
class UT66PowerUpSubsystem;

/**
 * Power Up screen: 6 stats (Damage, Attack Speed, Attack Size, Evasion, Armor, Luck),
 * each with 10 unlockable slices. Cost 10 Power Crystals per slice.
 * Page 0: 2x3 stat grid.  Page 1: Random panel (full-width).
 */
UCLASS(Blueprintable)
class T66_API UT66PowerUpScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66PowerUpScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual void RefreshScreen_Implementation() override;

private:
	UT66LocalizationSubsystem* GetLocSubsystem() const;
	UT66PowerUpSubsystem* GetPowerUpSubsystem() const;

	// Page switching (0 = stats grid, 1 = random)
	int32 CurrentPage = 0;
	TSharedPtr<SWidgetSwitcher> PageSwitcher;

	FReply HandleBackClicked();
	FReply HandleNextClicked();
	FReply HandleUnlockClicked(ET66HeroStatType StatType);
	FReply HandleUnlockRandomClicked();
};
