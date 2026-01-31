// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66Rarity.generated.h"

/** Shared rarity tier used by World Interactables (and later loot). */
UENUM(BlueprintType)
enum class ET66Rarity : uint8
{
	Black UMETA(DisplayName = "Black"),
	Red UMETA(DisplayName = "Red"),
	Yellow UMETA(DisplayName = "Yellow"),
	White UMETA(DisplayName = "White"),
};

/** Shared rarity/tier helpers (kept tiny and allocation-free). */
struct FT66RarityUtil
{
	/** Default rarity roll (left-weighted): Black common → White extremely rare. Tunable later. */
	static ET66Rarity RollDefaultRarity(FRandomStream& Rng);

	/** Luck-influenced rarity roll (used for world interactables + loot bags). LuckStat>=1. */
	static ET66Rarity RollRarityWithLuck(FRandomStream& Rng, int32 LuckStat);

	/** UI/world tint for a rarity. */
	static FLinearColor GetRarityColor(ET66Rarity R);

	/**
	 * Tier color for "5-slot compression" displays (Hearts and Skulls):
	 * tier 0 = Red, tier 1 = Blue, tier 2 = Green, tier 3 = Purple, tier 4 = Gold, tier 5+ = Cyan.
	 */
	static FLinearColor GetTierColor(int32 TierIndex);

	/** Convert a total into (tier, count within [1..5]). Total<=0 returns (0,0). */
	static void ComputeTierAndCount5(int32 Total, int32& OutTierIndex, int32& OutCount1To5);
};

