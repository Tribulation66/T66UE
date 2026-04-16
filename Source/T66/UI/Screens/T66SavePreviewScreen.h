// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66SavePreviewScreen.generated.h"

class UT66LocalizationSubsystem;

UCLASS(Blueprintable)
class T66_API UT66SavePreviewScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66SavePreviewScreen(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Save Preview")
	void OnBackClicked();

	UFUNCTION(BlueprintCallable, Category = "Save Preview")
	void OnLoadClicked();

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	UT66LocalizationSubsystem* GetLocSubsystem() const;

	FReply HandleBackClicked();
	FReply HandleLoadClicked();
};
