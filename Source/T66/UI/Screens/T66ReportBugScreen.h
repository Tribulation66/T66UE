// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66ReportBugScreen.generated.h"

UCLASS(Blueprintable)
class T66_API UT66ReportBugScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66ReportBugScreen(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Report Bug")
	void OnSubmitClicked();

	UFUNCTION(BlueprintCallable, Category = "Report Bug")
	void OnCancelClicked();

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleSubmitClicked();
	FReply HandleCancelClicked();

	FString BugReportText;
};
