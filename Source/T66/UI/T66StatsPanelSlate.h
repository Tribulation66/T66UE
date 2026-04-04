// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"

class UT66RunStateSubsystem;
class UT66LocalizationSubsystem;
class UT66LeaderboardRunSummarySaveGame;

/**
 * Shared Slate builder for the essential stats panel (Level + 8 stats).
 * Used by Pause menu, Vendor shop, Gambler casino, and gameplay HUD.
 */
namespace T66StatsPanelSlate
{
	struct FT66LiveStatsPanel
	{
		TArray<TSharedPtr<class STextBlock>> PrimaryLines;
		TMap<ET66SecondaryStatType, TSharedPtr<class STextBlock>> SecondaryLines;
		TSharedPtr<class STextBlock> ArmorReductionLine;
		TSharedPtr<class STextBlock> EvasionChanceLine;

		void Reset();
		void Update(UT66RunStateSubsystem* RunState, UT66LocalizationSubsystem* Loc) const;
	};

	/** Build the standard stats panel: header + Level + 8 primary stats; if bExtended, also all secondary stats in a scrollable list. Returns a fixed-width panel; if RunState is null, returns an empty placeholder. */
	TSharedRef<class SWidget> MakeEssentialStatsPanel(
		UT66RunStateSubsystem* RunState,
		UT66LocalizationSubsystem* Loc,
		float WidthOverride = 320.f,
		bool bExtended = false);

	/** Build the standard stats panel with stable text widgets that can be updated in place. */
	TSharedRef<class SWidget> MakeLiveEssentialStatsPanel(
		UT66RunStateSubsystem* RunState,
		UT66LocalizationSubsystem* Loc,
		const TSharedRef<FT66LiveStatsPanel>& LivePanel,
		float WidthOverride = 320.f,
		bool bExtended = false);

	/** Build the same stats panel from a saved/fake run snapshot (primary + secondary if SecondaryStatValues is populated). Used by Run Summary when viewing leaderboard/saved run. */
	TSharedRef<class SWidget> MakeEssentialStatsPanelFromSnapshot(
		UT66LeaderboardRunSummarySaveGame* Snapshot,
		UT66LocalizationSubsystem* Loc,
		float WidthOverride = 320.f);
}
