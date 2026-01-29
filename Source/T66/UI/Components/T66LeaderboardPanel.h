// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"

class UT66LocalizationSubsystem;

/**
 * Leaderboard Panel - Slate widget for displaying leaderboard
 * Used in Main Menu right panel
 */
class T66_API ST66LeaderboardPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(ST66LeaderboardPanel)
		: _LocalizationSubsystem(nullptr)
	{}
		SLATE_ARGUMENT(UT66LocalizationSubsystem*, LocalizationSubsystem)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

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
	ET66LeaderboardType CurrentType = ET66LeaderboardType::HighScore;

	TArray<FLeaderboardEntry> LeaderboardEntries;
	UT66LocalizationSubsystem* LocSubsystem = nullptr;

	TSharedPtr<SVerticalBox> EntryListBox;

	// Dropdown options
	TArray<TSharedPtr<FString>> PartySizeOptions;
	TArray<TSharedPtr<FString>> DifficultyOptions;
	TArray<TSharedPtr<FString>> TypeOptions;

	TSharedPtr<FString> SelectedPartySizeOption;
	TSharedPtr<FString> SelectedDifficultyOption;
	TSharedPtr<FString> SelectedTypeOption;

	// Generate placeholder data
	void GeneratePlaceholderData();
	void RebuildEntryList();

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

	TSharedRef<SWidget> MakePartySizeWidget(TSharedPtr<FString> InOption);
	TSharedRef<SWidget> MakeDifficultyWidget(TSharedPtr<FString> InOption);
	TSharedRef<SWidget> MakeTypeWidget(TSharedPtr<FString> InOption);

	// Get localized text
	FText GetFilterText(ET66LeaderboardFilter Filter) const;
	FText GetPartySizeText(ET66PartySize Size) const;
	FText GetDifficultyText(ET66Difficulty Diff) const;
	FText GetTimeText(ET66LeaderboardTime Time) const;
	FText GetTypeText(ET66LeaderboardType Type) const;

	// Helper to format time
	FString FormatTime(float Seconds) const;
};
