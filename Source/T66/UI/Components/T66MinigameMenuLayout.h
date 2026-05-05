// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"

class SHorizontalBox;
class SVerticalBox;
class UT66BackendSubsystem;

struct T66_API FT66MinigameDifficultyOption
{
	FName DifficultyID = NAME_None;
	FText DisplayName;
};

struct T66_API FT66MinigameLeaderboardEntry
{
	int32 Rank = 0;
	FString DisplayName;
	int64 Score = 0;
	bool bIsLocalPlayer = false;
};

enum class ET66MinigameLeaderboardScope : uint8
{
	Daily,
	AllTime
};

DECLARE_DELEGATE_RetVal_OneParam(TArray<FT66MinigameLeaderboardEntry>, FT66OnBuildMinigameLeaderboardEntries, const FName /*DifficultyID*/);
DECLARE_DELEGATE_RetVal_OneParam(FText, FT66OnGetMinigameLeaderboardStatus, const FName /*DifficultyID*/);

class T66_API ST66MinigameMenuLayout : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(ST66MinigameMenuLayout)
		: _AccentColor(FLinearColor(0.86f, 0.62f, 0.28f, 1.0f))
		, _BackgroundColor(FLinearColor(0.018f, 0.018f, 0.024f, 1.0f))
		, _LoadGameEnabled(true)
	{}
		SLATE_ARGUMENT(FName, GameID)
		SLATE_ARGUMENT(FText, Title)
		SLATE_ARGUMENT(FText, Subtitle)
		SLATE_ARGUMENT(FText, DailyTitle)
		SLATE_ARGUMENT(FText, DailyBody)
		SLATE_ARGUMENT(FText, DailyRules)
		SLATE_ARGUMENT(FLinearColor, AccentColor)
		SLATE_ARGUMENT(FLinearColor, BackgroundColor)
		SLATE_ARGUMENT(TArray<FT66MinigameDifficultyOption>, DifficultyOptions)
		SLATE_ARGUMENT(bool, LoadGameEnabled)
		SLATE_ARGUMENT(UT66BackendSubsystem*, BackendSubsystem)
		SLATE_EVENT(FOnClicked, OnNewGameClicked)
		SLATE_EVENT(FOnClicked, OnLoadGameClicked)
		SLATE_EVENT(FOnClicked, OnDailyClicked)
		SLATE_EVENT(FOnClicked, OnBackClicked)
		SLATE_EVENT(FT66OnBuildMinigameLeaderboardEntries, OnBuildDailyEntries)
		SLATE_EVENT(FT66OnBuildMinigameLeaderboardEntries, OnBuildAllTimeEntries)
		SLATE_EVENT(FT66OnGetMinigameLeaderboardStatus, OnGetDailyStatus)
		SLATE_EVENT(FT66OnGetMinigameLeaderboardStatus, OnGetAllTimeStatus)
	SLATE_END_ARGS()

	~ST66MinigameMenuLayout();

	void Construct(const FArguments& InArgs);

	FName GetSelectedDifficultyID() const;
	ET66MinigameLeaderboardScope GetSelectedScope() const { return SelectedScope; }

private:
	TSharedRef<SWidget> BuildDailyPanel() const;
	TSharedRef<SWidget> BuildCenterPanel() const;
	TSharedRef<SWidget> BuildLeaderboardPanel();
	TSharedRef<SWidget> MakePanel(const TSharedRef<SWidget>& Content, const FMargin& Padding) const;
	TSharedRef<SWidget> MakeMenuButton(const FText& Text, const FOnClicked& Handler, bool bEnabled, float Height) const;
	TSharedRef<SWidget> MakeScopeButton(const FText& Text, ET66MinigameLeaderboardScope Scope);
	TSharedRef<SWidget> MakeDifficultyDropdown();
	TSharedRef<SWidget> MakeLeaderboardRows() const;
	void RefreshLeaderboardWidgets();
	void OnDifficultyChanged(TSharedPtr<FT66MinigameDifficultyOption> NewSelection, ESelectInfo::Type SelectInfo);
	FReply HandleScopeClicked(ET66MinigameLeaderboardScope Scope);
	TArray<FT66MinigameLeaderboardEntry> GetCurrentEntries() const;
	FText GetCurrentStatus() const;
	void RequestCurrentBackendData();
	void OnBackendLeaderboardReady(const FString& CacheKey);
	void OnBackendDailyChallengeReady(const FString& CacheKey);
	FString GetBackendGameToken() const;
	FString GetBackendScopeToken() const;
	FString GetBackendDifficultyToken() const;
	FString GetBackendLeaderboardCacheKey() const;
	FString GetBackendDailyChallengeCacheKey() const;

	FName GameID = NAME_None;
	FText Title;
	FText Subtitle;
	FText DailyTitle;
	FText DailyBody;
	FText DailyRules;
	FLinearColor AccentColor;
	FLinearColor BackgroundColor;
	bool bLoadGameEnabled = true;

	FOnClicked OnNewGameClicked;
	FOnClicked OnLoadGameClicked;
	FOnClicked OnDailyClicked;
	FOnClicked OnBackClicked;
	FT66OnBuildMinigameLeaderboardEntries OnBuildDailyEntries;
	FT66OnBuildMinigameLeaderboardEntries OnBuildAllTimeEntries;
	FT66OnGetMinigameLeaderboardStatus OnGetDailyStatus;
	FT66OnGetMinigameLeaderboardStatus OnGetAllTimeStatus;
	TWeakObjectPtr<UT66BackendSubsystem> BackendSubsystem;
	TSet<FString> RequestedBackendLeaderboardKeys;
	TSet<FString> RequestedBackendDailyChallengeKeys;

	TArray<TSharedPtr<FT66MinigameDifficultyOption>> DifficultyOptions;
	TSharedPtr<FT66MinigameDifficultyOption> SelectedDifficulty;
	ET66MinigameLeaderboardScope SelectedScope = ET66MinigameLeaderboardScope::Daily;
	TSharedPtr<SHorizontalBox> ScopeButtonsBox;
	TSharedPtr<SVerticalBox> LeaderboardRowsBox;
};
