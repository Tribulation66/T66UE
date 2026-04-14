// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66MiniRunSummaryScreen.generated.h"

UCLASS()
class T66MINI_API UT66MiniRunSummaryScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66MiniRunSummaryScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandlePlayAgainClicked();
	FReply HandleMiniMenuClicked();
	FReply HandleMainMenuClicked();
};
