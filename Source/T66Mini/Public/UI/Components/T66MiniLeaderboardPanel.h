// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66MiniLeaderboardSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SCompoundWidget.h"

class UT66MiniDataSubsystem;
class UT66PartySubsystem;
class STextBlock;
class SVerticalBox;

class T66MINI_API ST66MiniLeaderboardPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(ST66MiniLeaderboardPanel) {}
		SLATE_ARGUMENT(UT66MiniDataSubsystem*, DataSubsystem)
		SLATE_ARGUMENT(UT66MiniLeaderboardSubsystem*, LeaderboardSubsystem)
		SLATE_ARGUMENT(UT66PartySubsystem*, PartySubsystem)
	SLATE_END_ARGS()

	~ST66MiniLeaderboardPanel();

	void Construct(const FArguments& InArgs);

private:
	void BuildDifficultyOptions();
	void RefreshLeaderboard();
	void RebuildEntryList();
	void OnMiniLeaderboardUpdated(const FString& CacheKey);

	FReply HandleGlobalClicked();
	FReply HandleFriendsClicked();
	void OnPartySizeChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	void OnDifficultyChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

	static FString PartySizeLabel(ET66PartySize PartySize);
	static ET66PartySize PartySizeFromLabel(const FString& Label);

	UT66MiniDataSubsystem* DataSubsystem = nullptr;
	UT66MiniLeaderboardSubsystem* LeaderboardSubsystem = nullptr;
	UT66PartySubsystem* PartySubsystem = nullptr;

	ET66MiniLeaderboardFilter CurrentFilter = ET66MiniLeaderboardFilter::Global;
	ET66PartySize CurrentPartySize = ET66PartySize::Solo;
	FName CurrentDifficultyId = NAME_None;
	FString CurrentCacheKey;
	TArray<FT66MiniLeaderboardEntry> CachedEntries;
	FString CachedStatusMessage;

	TArray<TSharedPtr<FString>> PartySizeOptions;
	TArray<TSharedPtr<FString>> DifficultyOptions;
	TMap<FString, FName> DifficultyIdsByDisplayName;
	TSharedPtr<FString> SelectedPartySizeOption;
	TSharedPtr<FString> SelectedDifficultyOption;

	TSharedPtr<SVerticalBox> EntryListBox;
	TSharedPtr<STextBlock> StatusTextBlock;
	FDelegateHandle LeaderboardUpdatedHandle;
};
