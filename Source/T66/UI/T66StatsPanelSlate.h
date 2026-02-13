// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UT66RunStateSubsystem;
class UT66LocalizationSubsystem;

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
}
