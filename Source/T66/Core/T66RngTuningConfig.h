// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66RngTuningConfig.generated.h"

/**
 * Central RNG tuning values (config-backed).
 *
 * This is intentionally config-only (no .uasset) so it can be edited safely while multiple agents work.
 * Later we can migrate to a DataAsset without changing call sites (subsystem remains the API).
 */
USTRUCT(BlueprintType)
struct T66_API FT66RarityWeights
{
	GENERATED_BODY()

	/** Probability mass for Black. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "RNG|Rarity", meta = (ClampMin = "0.0"))
	float Black = 0.80f;

	/** Probability mass for Red. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "RNG|Rarity", meta = (ClampMin = "0.0"))
	float Red = 0.15f;

	/** Probability mass for Yellow. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "RNG|Rarity", meta = (ClampMin = "0.0"))
	float Yellow = 0.045f;

	/** Probability mass for White. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "RNG|Rarity", meta = (ClampMin = "0.0"))
	float White = 0.005f;
};

USTRUCT(BlueprintType)
struct T66_API FT66IntRange
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "RNG", meta = (ClampMin = "0"))
	int32 Min = 0;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "RNG", meta = (ClampMin = "0"))
	int32 Max = 0;
};

USTRUCT(BlueprintType)
struct T66_API FT66FloatRange
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "RNG")
	float Min = 0.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "RNG")
	float Max = 0.f;
};

/**
 * Plain config-backed RNG tuning data.
 *
 * This intentionally does not derive from UObject. The previous UObject-based
 * config class created a CDO at /Script/T66.Default__T66RngTuningConfig, which
 * was crashing during hot reload / live coding replacement.
 */
class T66_API UT66RngTuningConfig
{
public:
	// ================================
	// Luck → bias model (global)
	// ================================

	/** How quickly LuckStat shifts outcome weights (per point above 1). */
	float LuckPerPoint = 0.03f;

	/** Max normalized luck factor (0..1). */
	float LuckMax01 = 0.95f;

	/** Strength of rarity reweighting at LuckMax01. Higher = shifts more mass upward. */
	float RarityBiasStrength = 1.35f;

	/** Strength of high-end bias for continuous ranges at LuckMax01 (power transform). */
	float RangeHighBiasStrength = 1.75f;

	/** Strength of "better outcome" bias for Bernoulli events (e.g. loot bag drop) at LuckMax01. */
	float BernoulliBiasStrength = 1.25f;

	// ================================
	// Loot bags (enemies)
	// ================================

	/** Baseline chance an eligible enemy drops a loot bag (per kill). */
	float LootBagDropChanceBase = 0.10f;

	/** Baseline rarity distribution for loot bags. */
	FT66RarityWeights LootBagRarityBase;

	// ================================
	// Specials (Goblin Thief) per-wave
	// ================================

	/** Chance a wave includes Goblin Thief spawns (before count roll). */
	float GoblinWaveChanceBase = 0.10f;

	/** Count range for Goblins when they spawn in a wave. */
	FT66IntRange GoblinCountPerWave = { 1, 3 };

	/** Baseline rarity distribution for special enemies (their variant rarity). */
	FT66RarityWeights SpecialEnemyRarityBase;

	// ================================
	// Vendor / Gambler success odds
	// ================================

	/** Vendor steal: base success chance if timing window is hit (Luck can bias this upward). */
	float VendorStealSuccessChanceOnTimingHitBase = 0.65f;

	/** Gambler cheat: base success chance (Luck can bias this upward). */
	float GamblerCheatSuccessChanceBase = 0.40f;

	// ================================
	// World interactables (count + rarity)
	// ================================

	FT66IntRange TreesPerStage = { 2, 5 };

	FT66IntRange ChestsPerStage = { 2, 5 };

	FT66IntRange WheelsPerStage = { 2, 5 };

	FT66RarityWeights InteractableRarityBase;

	/** Chest mimic chance is explicitly NOT luck-affected. */
	float ChestMimicChance = 0.20f;

	FT66IntRange CratesPerStage = { 1, 3 };

	// ================================
	// Wheel payouts (rarity + payout range)
	// ================================

	FT66RarityWeights WheelRarityBase;

	FT66FloatRange WheelGoldRange_Black = { 25.f, 100.f };

	FT66FloatRange WheelGoldRange_Red = { 100.f, 300.f };

	FT66FloatRange WheelGoldRange_Yellow = { 200.f, 600.f };

	FT66FloatRange WheelGoldRange_White = { 500.f, 1500.f };

	void LoadFromConfig();
};
