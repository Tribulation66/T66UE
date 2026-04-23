// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngTuningConfig.h"
#include "Core/PlayerExperience/T66PlayerExperienceTypes.h"
#include "Engine/DataTable.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "T66PlayerExperienceSubSystem.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66RarityIntRanges
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience")
	FT66IntRange Black;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience")
	FT66IntRange Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience")
	FT66IntRange Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience")
	FT66IntRange White;
};

USTRUCT(BlueprintType)
struct T66_API FT66PlayerExperienceDifficultyTuning : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|RunStart", meta = (ClampMin = "1", ClampMax = "23"))
	int32 StartStage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|RunShape", meta = (ClampMin = "1", ClampMax = "23"))
	int32 EndStage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|RunStart", meta = (ClampMin = "0"))
	int32 StartGoldBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|RunStart", meta = (ClampMin = "0"))
	int32 StartLootBags = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|RunStart", meta = (ClampMin = "0"))
	int32 StartHeroBonusLevels = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|RunStart")
	bool bSpawnSupportVendorAtRunStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|RunStart")
	bool bSupportVendorAllowSteal = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EnemyLootBagDropChanceBase = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Loot")
	FT66IntRange EnemyLootBagCountOnDrop;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Loot")
	FT66RarityWeights EnemyLootBagRarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Loot")
	FT66RarityWeights CatchUpLootBagRarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66IntRange ChestsPerStage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66RarityWeights ChestRarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66RarityIntRanges ChestGoldRangeByRarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChestMimicChance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66IntRange WheelsPerStage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66RarityWeights WheelRarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66FloatRange WheelGoldRangeBlack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66FloatRange WheelGoldRangeRed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66FloatRange WheelGoldRangeYellow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66FloatRange WheelGoldRangeWhite;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66IntRange CratesPerStage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66RarityWeights CrateRarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GamblerCheatSuccessChanceBase = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VendorStealSuccessChanceOnTimingHitBase = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Totems")
	FT66TotemRules TotemRules;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	FT66EnemyScoreTuning EnemyScoreTuning;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	FT66BossScoreTuning BossScoreTuning;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Spawning")
	FT66SpawnBudget TowerSpawnBudgetBase;
};

class T66_API FT66PlayerExperienceTuningTable
{
public:
	FT66PlayerExperienceDifficultyTuning Easy;
	FT66PlayerExperienceDifficultyTuning Medium;
	FT66PlayerExperienceDifficultyTuning Hard;
	FT66PlayerExperienceDifficultyTuning VeryHard;
	FT66PlayerExperienceDifficultyTuning Impossible;

	void LoadFromDataTable(const UDataTable* DataTable);
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
	int32 GetDifficultyEndStage(ET66Difficulty Difficulty) const;

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
	FT66IntRange GetDifficultyChestGoldRange(ET66Difficulty Difficulty, ET66Rarity Rarity) const;
	float GetDifficultyChestMimicChance(ET66Difficulty Difficulty) const;
	FT66IntRange GetDifficultyWheelCountRange(ET66Difficulty Difficulty) const;
	FT66RarityWeights GetDifficultyWheelRarityWeights(ET66Difficulty Difficulty) const;
	FT66FloatRange GetDifficultyWheelGoldRange(ET66Difficulty Difficulty, ET66Rarity Rarity) const;
	FT66IntRange GetDifficultyCrateCountRange(ET66Difficulty Difficulty) const;
	FT66RarityWeights GetDifficultyCrateRarityWeights(ET66Difficulty Difficulty) const;
	float GetDifficultyGamblerCheatSuccessChanceBase(ET66Difficulty Difficulty) const;
	float GetDifficultyVendorStealSuccessChanceOnTimingHitBase(ET66Difficulty Difficulty) const;
	FT66TotemRules GetDifficultyTotemRules(ET66Difficulty Difficulty) const;
	int32 GetDifficultyTotemUsesPerTotem(ET66Difficulty Difficulty) const;
	int32 GetDifficultySkullColorBandSize(ET66Difficulty Difficulty) const;
	bool ShouldSpawnDifficultyTotemOnTowerFloor(ET66Difficulty Difficulty, bool bBossRushFinaleStage, int32 FloorNumber, int32 FirstGameplayFloorNumber, int32 LastGameplayFloorNumber) const;
	FT66SpawnBudget BuildTowerSpawnBudget(
		ET66Difficulty Difficulty,
		float DifficultyScalar,
		float InitialPopulationScalar,
		float RuntimeTrickleCountScalar,
		float RuntimeTrickleIntervalScalar,
		float StageTimerSeconds) const;
	int32 ResolveEnemyScoreAtSpawn(ET66Difficulty Difficulty, int32 BasePointValue, float DifficultyScalar) const;
	int32 ResolveBossScore(ET66Difficulty Difficulty, int32 BasePointValue, float DifficultyScalar) const;

private:
	FT66PlayerExperienceTuningTable CachedTuning;
};
