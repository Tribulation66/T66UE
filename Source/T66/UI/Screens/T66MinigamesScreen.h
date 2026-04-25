// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/T66ScreenBase.h"
#include "T66MinigamesScreen.generated.h"

class UT66LocalizationSubsystem;

UCLASS(Blueprintable)
class T66_API UT66MinigamesScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66MinigamesScreen(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Minigames")
	void OnBackClicked();

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	UT66LocalizationSubsystem* GetLocSubsystem() const;

	FReply HandleBackClicked();
	FReply HandleOpenMiniChadpocalypseClicked();
	FReply HandleOpenChadpocalypseTDClicked();

	UFUNCTION()
	void HandleLanguageChanged(ET66Language NewLanguage);
};
