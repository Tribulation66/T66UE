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
class SBorder;
class SBox;
class SButton;
class SHorizontalBox;
class SImage;
class STextBlock;
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
	void OnDailyClimbClicked();

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
	virtual void NativeDestruct() override;
	virtual void RefreshScreen_Implementation() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	struct FFriendGroupWidgetRefs
	{
		bool bOnlineGroup = false;
		TSharedPtr<SBox> RootBox;
		TSharedPtr<STextBlock> CountText;
		TSharedPtr<SImage> ExpandArrowImage;
	};

	struct FFriendRowWidgetRefs
	{
		FString PlayerId;
		FString FriendName;
		FString BaseStatus;
		bool bOnline = false;
		TSharedPtr<SBox> RootBox;
		TSharedPtr<SBorder> RowBorder;
		TSharedPtr<STextBlock> StatusText;
		TSharedPtr<SButton> FavoriteButton;
		TSharedPtr<STextBlock> FavoriteGlyphText;
		TSharedPtr<SButton> ActionButton;
		TSharedPtr<SBorder> ActionFillBorder;
		TSharedPtr<STextBlock> ActionText;
	};

	uint32 CaptureMenuStateHash() const;
	bool ShouldRebuildRetainedSlate() const;

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
	TArray<FFriendGroupWidgetRefs> FriendGroupWidgetRefs;
	TArray<FFriendRowWidgetRefs> FriendRowWidgetRefs;
	TSharedPtr<SBox> FriendGroupsDividerBox;
	TSharedPtr<SBox> NoMatchingFriendsBox;
	bool bShowOnlineFriends = true;
	bool bShowOfflineFriends = true;
	FString FriendSearchQuery;
	FVector2D CachedViewportSize = FVector2D::ZeroVector;
	FVector2D LastBuiltViewportSize = FVector2D::ZeroVector;
	FVector2D PendingViewportSize = FVector2D::ZeroVector;
	float PendingViewportStableTime = 0.f;
	bool bViewportResponsiveRebuildQueued = false;
	ET66Language LastBuiltLanguage = ET66Language::English;
	uint32 LastBuiltMenuStateHash = 0;

	// Get localization subsystem
	UT66LocalizationSubsystem* GetLocSubsystem() const;

	// Button click handlers for Slate (return FReply)
	FReply HandleNewGameClicked();
	FReply HandleLoadGameClicked();
	FReply HandleDailyClimbClicked();
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
	void RefreshFriendListVisualState();
	void ReleaseRetainedSlateState();

	// Handle language change to rebuild UI
	UFUNCTION()
	void OnLanguageChanged(ET66Language NewLanguage);

	FDelegateHandle PartyStateChangedHandle;
	FDelegateHandle SessionStateChangedHandle;
};
