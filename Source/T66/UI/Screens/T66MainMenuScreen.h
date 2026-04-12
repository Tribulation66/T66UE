// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "UI/T66ScreenBase.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UObject/StrongObjectPtr.h"
#include "T66MainMenuScreen.generated.h"

struct FSlateBrush;
class ST66LeaderboardPanel;
class SVerticalBox;
class UTexture2D;
/**
 * Main Menu Screen
 * Left side: social / profile panel
 * Center: primary actions
 * Right side: leaderboard display
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
	void OnPowerUpClicked();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void OnMinigamesClicked();

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
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void RefreshScreen_Implementation() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	TSharedPtr<ST66LeaderboardPanel> LeaderboardPanel;
	TSharedPtr<FSlateBrush> SkyBackgroundBrush;
	TStrongObjectPtr<UTexture2D> SkyBackgroundTexture;
	TSharedPtr<FSlateBrush> FireMoonBrush;
	TStrongObjectPtr<UTexture2D> FireMoonTexture;
	TSharedPtr<FSlateBrush> PyramidChadBrush;
	TStrongObjectPtr<UTexture2D> PyramidChadTexture;
	TSharedPtr<FSlateBrush> ProfileAvatarBrush;
	TSharedPtr<FSlateBrush> PrimaryCTAFillBrush;
	TStrongObjectPtr<UTexture2D> PrimaryCTAFillTexture;
	TSharedPtr<FSlateBrush> SettingsIconBrush;
	TSharedPtr<FSlateBrush> LanguageIconBrush;
	TArray<TSharedPtr<FSlateBrush>> FriendPortraitBrushes;
	TArray<TSharedPtr<FSlateBrush>> PartyPortraitBrushes;
	TSharedPtr<SVerticalBox> FriendsListContainer;
	bool bShowOnlineFriends = true;
	bool bShowOfflineFriends = true;
	FString FriendSearchQuery;
	FVector2D CachedViewportSize = FVector2D::ZeroVector;
	bool bViewportResponsiveRebuildQueued = false;

	// Get localization subsystem
	UT66LocalizationSubsystem* GetLocSubsystem() const;

	// Button click handlers for Slate (return FReply)
	FReply HandleNewGameClicked();
	FReply HandleLoadGameClicked();
	FReply HandlePowerUpClicked();
	FReply HandleMinigamesClicked();
	FReply HandleAchievementsClicked();
	FReply HandleSettingsClicked();
	FReply HandleLanguageClicked();
	FReply HandleQuitClicked();
	FReply HandleLeavePartyClicked();

	/** Load or bind the main menu animated background layers. */
	void RequestBackgroundTexture();
	void RequestUtilityButtonIcons();
	void HandlePartyStateChanged();
	void HandleSessionStateChanged();
	void SyncToSharedPartyScreen();
	void HandleFriendSearchTextChanged(const FText& NewText);

	// Handle language change to rebuild UI
	UFUNCTION()
	void OnLanguageChanged(ET66Language NewLanguage);

	FDelegateHandle PartyStateChangedHandle;
	FDelegateHandle SessionStateChangedHandle;
};
