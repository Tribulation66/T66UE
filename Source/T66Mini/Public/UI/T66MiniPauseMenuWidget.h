// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "T66MiniPauseMenuWidget.generated.h"

UCLASS(Blueprintable)
class T66MINI_API UT66MiniPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	FReply HandleResumeClicked();
	FReply HandleMainMenuClicked();
};
