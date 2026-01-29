// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66PauseMenuScreen.generated.h"

class AT66PlayerController;

UCLASS(Blueprintable)
class T66_API UT66PauseMenuScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66PauseMenuScreen(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void OnResumeClicked();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void OnSaveAndQuitClicked();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void OnRestartClicked();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void OnSettingsClicked();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void OnReportBugClicked();

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleResumeClicked();
	FReply HandleSaveAndQuitClicked();
	FReply HandleRestartClicked();
	FReply HandleSettingsClicked();
	FReply HandleReportBugClicked();

	AT66PlayerController* GetT66PlayerController() const;
};
