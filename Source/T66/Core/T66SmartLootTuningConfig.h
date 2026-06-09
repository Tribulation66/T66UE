// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Config-backed smart loot tuning.
 *
 * Kept as a plain C++ object to match the RNG tuning pattern and avoid
 * UObject CDO/live-coding churn for data-only runtime weights.
 */
class T66_API UT66SmartLootTuningConfig
{
public:
	bool bEnableSmartLoot = true;

	float BaseCandidateWeight = 1.0f;
	float InventoryBaseStatWeight = 0.25f;
	float InventoryStatWeight = 1.25f;
	float InventoryAttackCategoryWeight = 0.50f;
	float IdolElementWeight = 1.50f;
	float IdolAttackCategoryWeight = 1.75f;
	float MaxCandidateWeight = 8.0f;

	float ShopRerollSeenDecayFactor = 2.0f;
	float ShopRerollSeenWeightFloor = 0.05f;

	void LoadFromConfig();
};
