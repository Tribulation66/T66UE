// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UT66RunStateSubsystem;
class UT66LocalizationSubsystem;
class UT66LeaderboardRunSummarySaveGame;

/**
 * Shared Slate builder for the essential stats panel (Level + 8 stats).
 * Used by Pause menu, Vendor shop, Gambler casino, and gameplay HUD.
 */
namespace T66StatsPanelSlate
{
	/** Build the standard stats panel: header + Level + 8 primary stats; if bExtended, also all secondary stats in a scrollable list. Returns a fixed-width panel; if RunState is null, returns an empty placeholder. */
	TSharedRef<class SWidget> MakeEssentialStatsPanel(
		UT66RunStateSubsystem* RunState,
		UT66LocalizationSubsystem* Loc,
		float WidthOverride = 320.f,
		bool bExtended = false);

	/** Build the same stats panel from a saved/fake run snapshot (primary + secondary if SecondaryStatValues is populated). Used by Run Summary when viewing leaderboard/saved run. */
	TSharedRef<class SWidget> MakeEssentialStatsPanelFromSnapshot(
		UT66LeaderboardRunSummarySaveGame* Snapshot,
		UT66LocalizationSubsystem* Loc,
		float WidthOverride = 320.f);
}
