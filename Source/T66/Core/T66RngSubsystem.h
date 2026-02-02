// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/T66RngTuningConfig.h"
#include "Core/T66Rarity.h"
#include "T66RngSubsystem.generated.h"

/**
 * Central RNG subsystem (Option A).
 *
 * - Owns run-seeded streams
 * - Applies Luck bias consistently across systems
 * - Provides a single API surface for all gameplay RNG that should contribute to Luck Rating
 *
 * NOTE: This is foundation only. Call sites will be migrated in small batches.
 */
UCLASS()
class T66_API UT66RngSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Start a new run RNG stream. Call when a run begins (or on ResetForNewRun). */
	UFUNCTION(BlueprintCallable, Category = "RNG")
	void BeginRun(int32 LuckStat);

	/** Update the effective Luck stat without reseeding the run stream. */
	UFUNCTION(BlueprintCallable, Category = "RNG")
	void UpdateLuckStat(int32 InLuckStat);

	/** Current run seed (debug/provenance). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RNG")
	int32 GetRunSeed() const { return RunSeed; }

	/** Effective Luck stat used for biasing (>=1). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RNG")
	int32 GetLuckStat() const { return LuckStat; }

	/** Normalized Luck factor (0..1) derived from LuckStat and tuning. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RNG")
	float GetLuck01() const { return Luck01; }

	// ================================
	// Core helpers (used by roll APIs)
	// ================================

	/** Apply high-end bias to a uniform [0,1) sample based on Luck01. */
	float BiasHigh01(float U01) const;

	/** Apply luck to a Bernoulli chance (better outcome = true). */
	float BiasChance01(float BaseChance01) const;

	/** Roll a rarity using baseline weights and Luck bias. */
	ET66Rarity RollRarityWeighted(const FT66RarityWeights& BaseWeights, FRandomStream& Stream) const;

	/** Roll an int in [Min,Max] with Luck bias toward higher values (if range non-empty). */
	int32 RollIntRangeBiased(const FT66IntRange& Range, FRandomStream& Stream) const;

	/** Roll a float in [Min,Max] with Luck bias toward higher values (if range non-empty). */
	float RollFloatRangeBiased(const FT66FloatRange& Range, FRandomStream& Stream) const;

	/** Access tuning (config-backed). */
	const UT66RngTuningConfig* GetTuning() const;

	/** Access the primary run stream. */
	FRandomStream& GetRunStream() { return RunStream; }
	const FRandomStream& GetRunStream() const { return RunStream; }

private:
	void RecomputeLuck01();

	/** Seed used for RunStream. */
	int32 RunSeed = 0;

	/** Luck stat points (>=1). */
	int32 LuckStat = 1;

	/** Cached normalized luck factor (0..1). */
	float Luck01 = 0.f;

	/** Run-persistent RNG stream (seeded once per run). */
	FRandomStream RunStream;
};

