// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66AccountStatusScreen.generated.h"

/** Simple account-status modal opened from the frontend top bar. */
UCLASS(Blueprintable)
class T66_API UT66AccountStatusScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66AccountStatusScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void RefreshScreen_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleBackClicked();
};

