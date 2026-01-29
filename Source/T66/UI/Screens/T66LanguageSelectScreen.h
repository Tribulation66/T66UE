// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Core/T66LocalizationSubsystem.h"
#include "T66LanguageSelectScreen.generated.h"

/**
 * Language Select Modal Screen
 * Shows a vertical list of available languages
 * Selecting a language previews it, Confirm applies, Back cancels
 */
UCLASS(Blueprintable)
class T66_API UT66LanguageSelectScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66LanguageSelectScreen(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Language Select")
	void SelectLanguage(ET66Language Language);

	UFUNCTION(BlueprintCallable, Category = "Language Select")
	void OnConfirmClicked();

	UFUNCTION(BlueprintCallable, Category = "Language Select")
	void OnBackClicked();

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	ET66Language PreviewedLanguage = ET66Language::English;
	ET66Language OriginalLanguage = ET66Language::English;

	TSharedPtr<SVerticalBox> LanguageListBox;

	FReply HandleLanguageClicked(ET66Language Language);
	FReply HandleConfirmClicked();
	FReply HandleBackClicked();

	void RebuildLanguageList();
	UT66LocalizationSubsystem* GetLocSubsystem() const;
};
