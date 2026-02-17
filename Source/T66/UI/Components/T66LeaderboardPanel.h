// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Styling/SlateBrush.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"

class UT66LocalizationSubsystem;
class UT66LeaderboardSubsystem;
class UT66BackendSubsystem;
class UT66UIManager;

/**
 * Leaderboard Panel - Slate widget for displaying leaderboard
 * Used in Main Menu right panel
 */
class T66_API ST66LeaderboardPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(ST66LeaderboardPanel)
		: _LocalizationSubsystem(nullptr)
		, _LeaderboardSubsystem(nullptr)
		, _UIManager(nullptr)
	{}
		SLATE_ARGUMENT(UT66LocalizationSubsystem*, LocalizationSubsystem)
		SLATE_ARGUMENT(UT66LeaderboardSubsystem*, LeaderboardSubsystem)
		SLATE_ARGUMENT(UT66UIManager*, UIManager)
	SLATE_END_ARGS()

	~ST66LeaderboardPanel();

	void Construct(const FArguments& InArgs);

	/** Must be set after the owning screen has a valid UIManager. */
	void SetUIManager(UT66UIManager* InUIManager);

	void SetFilter(ET66LeaderboardFilter NewFilter);
	void SetTimeFilter(ET66LeaderboardTime NewTime);
	void SetPartySize(ET66PartySize NewPartySize);
	void SetDifficulty(ET66Difficulty NewDifficulty);
	void SetLeaderboardType(ET66LeaderboardType NewType);
	void RefreshLeaderboard();

private:
	// Current filter states
	ET66LeaderboardFilter CurrentFilter = ET66LeaderboardFilter::Global;
	ET66LeaderboardTime CurrentTimeFilter = ET66LeaderboardTime::Current;
	ET66PartySize CurrentPartySize = ET66PartySize::Solo;
	ET66Difficulty CurrentDifficulty = ET66Difficulty::Easy;
	ET66LeaderboardType CurrentType = ET66LeaderboardType::Score;

	TArray<FLeaderboardEntry> LeaderboardEntries;
	UT66LocalizationSubsystem* LocSubsystem = nullptr;
	UT66LeaderboardSubsystem* LeaderboardSubsystem = nullptr;
	UT66UIManager* UIManager = nullptr;

	TSharedPtr<SVerticalBox> EntryListBox;

	// Dropdown options
	TArray<TSharedPtr<FString>> PartySizeOptions;
	TArray<TSharedPtr<FString>> DifficultyOptions;
	TArray<TSharedPtr<FString>> TypeOptions;
	TArray<TSharedPtr<FString>> StageOptions;

	TSharedPtr<FString> SelectedPartySizeOption;
	TSharedPtr<FString> SelectedDifficultyOption;
	TSharedPtr<FString> SelectedTypeOption;
	TSharedPtr<FString> SelectedStageOption;

	// Only used for Speed Run leaderboard (stage 1..10).
	int32 CurrentSpeedRunStage = 1;

	// Generate placeholder data
	void GeneratePlaceholderData();
	void RebuildEntryList();
	FReply HandleEntryClicked(const FLeaderboardEntry& Entry);
	FReply HandleLocalEntryClicked(const FLeaderboardEntry& Entry);

	// Backend async refresh
	bool bBoundToBackendDelegate = false;
	void OnBackendLeaderboardReady(const FString& Key);

	// Backend run summary fetch
	bool bBoundToRunSummaryDelegate = false;
	FString PendingRunSummaryEntryId;
	void OnBackendRunSummaryReady(const FString& EntryId);

	// Button handlers
	FReply HandleGlobalClicked();
	FReply HandleFriendsClicked();
	FReply HandleStreamersClicked();
	FReply HandleCurrentClicked();
	FReply HandleAllTimeClicked();

	// Dropdown handlers
	void OnPartySizeChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	void OnDifficultyChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	void OnTypeChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	void OnStageChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

	// Get localized text
	FText GetFilterText(ET66LeaderboardFilter Filter) const;
	FText GetPartySizeText(ET66PartySize Size) const;
	FText GetDifficultyText(ET66Difficulty Diff) const;
	FText GetTimeText(ET66LeaderboardTime Time) const;
	FText GetTypeText(ET66LeaderboardType Type) const;

	// Helper to format time
	FString FormatTime(float Seconds) const;

	// Slate brushes for filter icon buttons (kept alive for SImage)
	TSharedPtr<FSlateBrush> FilterBrushGlobal;
	TSharedPtr<FSlateBrush> FilterBrushFriends;
	TSharedPtr<FSlateBrush> FilterBrushStreamers;

	// Avatar brushes (keyed by URL, kept alive for SImage)
	TMap<FString, TSharedPtr<FSlateBrush>> AvatarBrushes;

	// Default avatar brush (used when no avatar URL or download pending)
	TSharedPtr<FSlateBrush> DefaultAvatarBrush;

	/** Create or retrieve a cached FSlateBrush for an avatar URL. */
	const FSlateBrush* GetOrCreateAvatarBrush(const FString& AvatarUrl);
};
