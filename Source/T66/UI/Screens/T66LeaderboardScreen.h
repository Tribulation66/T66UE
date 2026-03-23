// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66LeaderboardScreen.generated.h"

class ST66LeaderboardPanel;

UCLASS(Blueprintable)
class T66_API UT66LeaderboardScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66LeaderboardScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleBackClicked();

	TSharedPtr<ST66LeaderboardPanel> LeaderboardPanel;
};
