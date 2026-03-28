// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "T66PowerUpScreen.generated.h"

class UT66LocalizationSubsystem;
class UT66PowerUpSubsystem;
struct FSlateBrush;

/**
 * Power Up screen: 6 stats (Damage, Attack Speed, Attack Scale, Evasion, Armor, Luck),
 * each represented by a statue that fills from bottom to top as the stat is unlocked.
 * Page 0: 2x3 stat grid. Page 1: Power Coupon to Achievement Coin converter.
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

	// Page switching (0 = stats grid, 1 = converter)
	int32 CurrentPage = 0;
	TSharedPtr<SWidgetSwitcher> PageSwitcher;
	TMap<FString, TSharedPtr<FSlateBrush>> StatueBrushes;
	static constexpr int32 AchievementCoinsPerPowerCoupon = 100;

	FReply HandleBackClicked();
	FReply HandleOpenConverterClicked();
	FReply HandleUnlockClicked(ET66HeroStatType StatType);
	FReply HandleConvertOneClicked();
	FReply HandleConvertFiveClicked();
	FReply HandleConvertAllClicked();
	bool TryConvertPowerCoupons(int32 RequestedCoupons);
};
