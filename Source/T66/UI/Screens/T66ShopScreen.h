// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "T66ShopScreen.generated.h"

class UT66LocalizationSubsystem;
class UT66BuffSubsystem;
struct FSlateBrush;

/**
 * Shop screen: permanent buffs on one tab and next-run secondary-stat buffs on the other.
 */
UCLASS(Blueprintable)
class T66_API UT66ShopScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66ShopScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual void RefreshScreen_Implementation() override;

private:
	UT66LocalizationSubsystem* GetLocSubsystem() const;
	UT66BuffSubsystem* GetBuffSubsystem() const;
	void SetShowingSingleUse(bool bInShowingSingleUse);

	bool bShowingSingleUse = false;
	TSharedPtr<SWidgetSwitcher> PageSwitcher;
	TMap<FString, TSharedPtr<FSlateBrush>> OwnedBrushes;

	FReply HandleBackClicked();
	FReply HandleShowPermanentClicked();
	FReply HandleShowSingleUseClicked();
	FReply HandleUnlockClicked(ET66HeroStatType StatType);
	FReply HandlePurchaseSingleUseClicked(ET66SecondaryStatType StatType);
};
