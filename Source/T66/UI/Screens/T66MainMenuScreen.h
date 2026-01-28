// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66MainMenuScreen.generated.h"

/**
 * Main Menu Screen
 * Left side: Navigation buttons (New Game, Load Game, Settings, Achievements)
 * Right side: Leaderboard display
 * Corners: Language button (bottom-left), Quit button (top-right)
 */
UCLASS(Blueprintable)
class T66_API UT66MainMenuScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66MainMenuScreen(const FObjectInitializer& ObjectInitializer);

	// Button handlers

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void OnNewGameClicked();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void OnLoadGameClicked();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void OnSettingsClicked();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void OnAchievementsClicked();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void OnLanguageClicked();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void OnQuitClicked();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void OnAccountStatusClicked();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Main Menu")
	bool ShouldShowAccountStatus() const;

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	// Button click handlers for Slate (return FReply)
	FReply HandleNewGameClicked();
	FReply HandleLoadGameClicked();
	FReply HandleSettingsClicked();
	FReply HandleAchievementsClicked();
	FReply HandleLanguageClicked();
	FReply HandleQuitClicked();
};
