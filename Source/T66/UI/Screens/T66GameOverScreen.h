// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66GameOverScreen.generated.h"

class SWidget;

UCLASS(Blueprintable)
class T66_API UT66GameOverScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66GameOverScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleContinueClicked();
	void GatherRunRewards(int32& OutCoupons, int32& OutAchievements, int32& OutSecretAchievements) const;
};
