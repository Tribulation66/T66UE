// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngTuningConfig.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "T66PlayerExperienceSubSystem.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66RarityIntValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience", meta = (ClampMin = "0"))
	int32 Black = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience", meta = (ClampMin = "0"))
	int32 Red = 150;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience", meta = (ClampMin = "0"))
	int32 Yellow = 300;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience", meta = (ClampMin = "0"))
	int32 White = 600;
};

USTRUCT(BlueprintType)
struct T66_API FT66PlayerExperienceDifficultyTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|RunStart", meta = (ClampMin = "1", ClampMax = "33"))
	int32 StartStage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|RunStart", meta = (ClampMin = "0"))
	int32 StartGoldBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|RunStart", meta = (ClampMin = "0"))
	int32 StartLootBags = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|RunStart", meta = (ClampMin = "0"))
	int32 StartHeroBonusLevels = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|RunStart")
	bool bSpawnSupportVendorAtRunStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|RunStart")
	bool bSupportVendorAllowSteal = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EnemyLootBagDropChanceBase = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Loot")
	FT66IntRange EnemyLootBagCountOnDrop = { 1, 1 };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Loot")
	FT66RarityWeights EnemyLootBagRarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Loot")
	FT66RarityWeights CatchUpLootBagRarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66IntRange ChestsPerStage = { 4, 10 };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66RarityWeights ChestRarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66RarityIntValues ChestGoldByRarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChestMimicChance = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66IntRange WheelsPerStage = { 5, 11 };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66RarityWeights WheelRarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66FloatRange WheelGoldRangeBlack = { 25.f, 100.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66FloatRange WheelGoldRangeRed = { 100.f, 300.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66FloatRange WheelGoldRangeYellow = { 200.f, 600.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66FloatRange WheelGoldRangeWhite = { 500.f, 1500.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66IntRange CratesPerStage = { 3, 6 };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66RarityWeights CrateRarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GamblerCheatSuccessChanceBase = 0.40f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VendorStealSuccessChanceOnTimingHitBase = 0.65f;
};

class T66_API FT66PlayerExperienceTuningTable
{
public:
	FT66PlayerExperienceDifficultyTuning Easy;
	FT66PlayerExperienceDifficultyTuning Medium;
	FT66PlayerExperienceDifficultyTuning Hard;
	FT66PlayerExperienceDifficultyTuning VeryHard;
	FT66PlayerExperienceDifficultyTuning Impossible;
	FT66PlayerExperienceDifficultyTuning Perdition;
	FT66PlayerExperienceDifficultyTuning Final;

	FT66PlayerExperienceTuningTable();

	void LoadFromConfig();
	const FT66PlayerExperienceDifficultyTuning& Get(ET66Difficulty Difficulty) const;
};

/**
 * Central tuning surface for progression, rewards, and expected player outcomes.
 *
 * Luck bias and RNG execution still live in UT66RngSubsystem, but the gameplay values that
 * define what can spawn, how valuable it is, and how generous each difficulty feels should
 * come through here.
 */
UCLASS()
class T66_API UT66PlayerExperienceSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerExperience")
	int32 GetDifficultyIndex(ET66Difficulty Difficulty) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerExperience")
	int32 GetDifficultyStartStage(ET66Difficulty Difficulty) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerExperience")
	int32 GetDifficultyStartGoldBonus(ET66Difficulty Difficulty) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerExperience")
	int32 GetDifficultyStartLootBags(ET66Difficulty Difficulty) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerExperience")
	int32 GetDifficultyStartHeroBonusLevels(ET66Difficulty Difficulty) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerExperience")
	bool ShouldSpawnSupportVendorAtRunStart(ET66Difficulty Difficulty) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerExperience")
	bool ShouldSupportVendorAllowSteal(ET66Difficulty Difficulty) const;

	const FT66PlayerExperienceDifficultyTuning& GetDifficultyTuning(ET66Difficulty Difficulty) const;
	float GetDifficultyEnemyLootBagDropChanceBase(ET66Difficulty Difficulty) const;
	FT66IntRange GetDifficultyEnemyLootBagCountOnDrop(ET66Difficulty Difficulty) const;
	FT66RarityWeights GetDifficultyEnemyLootBagRarityWeights(ET66Difficulty Difficulty) const;
	FT66RarityWeights GetDifficultyCatchUpLootBagRarityWeights(ET66Difficulty Difficulty) const;
	FT66IntRange GetDifficultyChestCountRange(ET66Difficulty Difficulty) const;
	FT66RarityWeights GetDifficultyChestRarityWeights(ET66Difficulty Difficulty) const;
	int32 GetDifficultyChestGoldForRarity(ET66Difficulty Difficulty, ET66Rarity Rarity) const;
	float GetDifficultyChestMimicChance(ET66Difficulty Difficulty) const;
	FT66IntRange GetDifficultyWheelCountRange(ET66Difficulty Difficulty) const;
	FT66RarityWeights GetDifficultyWheelRarityWeights(ET66Difficulty Difficulty) const;
	FT66FloatRange GetDifficultyWheelGoldRange(ET66Difficulty Difficulty, ET66Rarity Rarity) const;
	FT66IntRange GetDifficultyCrateCountRange(ET66Difficulty Difficulty) const;
	FT66RarityWeights GetDifficultyCrateRarityWeights(ET66Difficulty Difficulty) const;
	float GetDifficultyGamblerCheatSuccessChanceBase(ET66Difficulty Difficulty) const;
	float GetDifficultyVendorStealSuccessChanceOnTimingHitBase(ET66Difficulty Difficulty) const;

private:
	FT66PlayerExperienceTuningTable CachedTuning;
};
