// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Core/T66LocalizationSubsystem.h"
#include "T66MainMenuScreen.generated.h"

struct FSlateBrush;
class ST66LeaderboardPanel;
class SImage;
enum class ET66UITheme : uint8;

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
	virtual void RefreshScreen_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	TSharedPtr<ST66LeaderboardPanel> LeaderboardPanel;
	TSharedPtr<FSlateBrush> MainMenuBackgroundBrush;
	TSharedPtr<SImage> MainMenuBackgroundImage;

	// Theme toggle: deferred + locked to prevent crashes from mid-event rebuild
	ET66UITheme PendingTheme;
	bool bThemeChangeInProgress = false;

	// Get localization subsystem
	UT66LocalizationSubsystem* GetLocSubsystem() const;

	// Button click handlers for Slate (return FReply)
	FReply HandleNewGameClicked();
	FReply HandleLoadGameClicked();
	FReply HandleSettingsClicked();
	FReply HandleAchievementsClicked();
	FReply HandleLanguageClicked();
	FReply HandleQuitClicked();
	FReply HandleDarkThemeClicked();
	FReply HandleLightThemeClicked();
	void ApplyPendingTheme();

	// Handle language change to rebuild UI
	UFUNCTION()
	void OnLanguageChanged(ET66Language NewLanguage);
};
