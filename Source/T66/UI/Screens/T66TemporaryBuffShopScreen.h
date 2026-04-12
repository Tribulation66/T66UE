// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "T66TemporaryBuffShopScreen.generated.h"

class UT66BuffSubsystem;
class UT66LocalizationSubsystem;

UCLASS(Blueprintable)
class T66_API UT66TemporaryBuffShopScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66TemporaryBuffShopScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual void RefreshScreen_Implementation() override;

private:
	UT66LocalizationSubsystem* GetLocSubsystem() const;
	UT66BuffSubsystem* GetBuffSubsystem() const;

	FReply HandlePurchaseClicked(ET66SecondaryStatType StatType);
	FReply HandleBackToBuffsClicked();

	TArray<TSharedPtr<struct FSlateBrush>> BuffIconBrushes;
};
