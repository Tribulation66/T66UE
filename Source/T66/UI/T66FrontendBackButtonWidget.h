// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66FrontendBackButtonWidget.generated.h"

class UT66LocalizationSubsystem;

UCLASS()
class T66_API UT66FrontendBackButtonWidget : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66FrontendBackButtonWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual void RefreshScreen_Implementation() override;

private:
	UT66LocalizationSubsystem* GetLocSubsystem() const;
	FReply HandleBackClicked();
};
