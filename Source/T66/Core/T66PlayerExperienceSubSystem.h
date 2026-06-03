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

struct FStreamableHandle;

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
struct T66_API FT66LootWheelRewardWeights
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|LootWheel", meta = (ClampMin = "0.0"))
	float Gold = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|LootWheel", meta = (ClampMin = "0.0"))
	float Item = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|LootWheel", meta = (ClampMin = "0.0"))
	float Boost = 0.10f;
};

USTRUCT(BlueprintType)
struct T66_API FT66LootWheelRewardWeightsByRarity
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|LootWheel")
	FT66LootWheelRewardWeights Black;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|LootWheel")
	FT66LootWheelRewardWeights Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|LootWheel")
	FT66LootWheelRewardWeights Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|LootWheel")
	FT66LootWheelRewardWeights White;
};

USTRUCT(BlueprintType)
struct T66_API FT66PlayerExperienceDifficultyTuning : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EnemyLootBagDropChanceBase = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Loot")
	FT66IntRange EnemyLootBagCountOnDrop;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Loot")
	FT66RarityWeights EnemyLootBagRarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66IntRange ChestsPerStage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66RarityWeights ChestRarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66RarityIntRanges ChestGoldRangeByRarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChestMimicChance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66IntRange CratesPerStage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66RarityWeights CrateRarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66IntRange LootWheelsPerStage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66RarityWeights LootWheelRarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66LootWheelRewardWeightsByRarity LootWheelRewardWeightsByRarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World")
	FT66RarityIntRanges LootWheelGoldRangeByRarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GamblerCheatSuccessChanceBase = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShopStealSuccessChanceOnTimingHitBase = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Totems")
	FT66TotemRules TotemRules;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	FT66EnemyScoreTuning EnemyScoreTuning;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	FT66BossScoreTuning BossScoreTuning;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Spawning")
	FT66SpawnBudget TowerSpawnBudgetBase;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|LevelUp", meta = (ClampMin = "1"))
	int32 LevelUpXPThreshold = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|LevelUp", meta = (ClampMin = "0.0"))
	float LevelUpWaveRadiusUU = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Headshot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HeadshotChancePerBonusPoint = 0.005f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Headshot", meta = (ClampMin = "0.0"))
	float HeadshotStunDurationSeconds = 0.75f;
};

class T66_API FT66PlayerExperienceTuningTable
{
public:
	FT66PlayerExperienceDifficultyTuning Easy;
	FT66PlayerExperienceDifficultyTuning Medium;
	FT66PlayerExperienceDifficultyTuning Hard;
	FT66PlayerExperienceDifficultyTuning VeryHard;
	FT66PlayerExperienceDifficultyTuning Impossible;

	bool LoadFromDataTable(const UDataTable* DataTable);
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

	const FT66PlayerExperienceDifficultyTuning& GetDifficultyTuning(ET66Difficulty Difficulty) const;
	float GetDifficultyEnemyLootBagDropChanceBase(ET66Difficulty Difficulty) const;
	FT66IntRange GetDifficultyEnemyLootBagCountOnDrop(ET66Difficulty Difficulty) const;
	FT66RarityWeights GetDifficultyEnemyLootBagRarityWeights(ET66Difficulty Difficulty) const;
	FT66IntRange GetDifficultyChestCountRange(ET66Difficulty Difficulty) const;
	FT66RarityWeights GetDifficultyChestRarityWeights(ET66Difficulty Difficulty) const;
	FT66IntRange GetDifficultyChestGoldRange(ET66Difficulty Difficulty, ET66Rarity Rarity) const;
	float GetDifficultyChestMimicChance(ET66Difficulty Difficulty) const;
	FT66IntRange GetDifficultyCrateCountRange(ET66Difficulty Difficulty) const;
	FT66RarityWeights GetDifficultyCrateRarityWeights(ET66Difficulty Difficulty) const;
	FT66IntRange GetDifficultyLootWheelCountRange(ET66Difficulty Difficulty) const;
	FT66RarityWeights GetDifficultyLootWheelRarityWeights(ET66Difficulty Difficulty) const;
	FT66LootWheelRewardWeights GetDifficultyLootWheelRewardWeights(ET66Difficulty Difficulty, ET66Rarity WheelRarity) const;
	FT66IntRange GetDifficultyLootWheelGoldRange(ET66Difficulty Difficulty, ET66Rarity WheelRarity) const;
	float GetDifficultyGamblerCheatSuccessChanceBase(ET66Difficulty Difficulty) const;
	float GetDifficultyShopStealSuccessChanceOnTimingHitBase(ET66Difficulty Difficulty) const;
	FT66TotemRules GetDifficultyTotemRules(ET66Difficulty Difficulty) const;
	int32 GetDifficultyTotemUsesPerTotem(ET66Difficulty Difficulty) const;
	int32 GetDifficultySkullColorBandSize(ET66Difficulty Difficulty) const;
	bool ShouldSpawnDifficultyTotemOnTowerFloor(ET66Difficulty Difficulty, bool bBossRushFinaleStage, int32 FloorNumber, int32 FirstMobFloorNumber, int32 LastMobFloorNumber) const;
	FT66SpawnBudget BuildTowerSpawnBudget(
		ET66Difficulty Difficulty,
		float DifficultyScalar,
		float InitialPopulationScalar,
		float RuntimeTrickleCountScalar,
		float RuntimeTrickleIntervalScalar,
		float StageTimerSeconds) const;
	int32 ResolveEnemyScoreAtSpawn(ET66Difficulty Difficulty, int32 BasePointValue, float DifficultyScalar) const;
	int32 ResolveBossScore(ET66Difficulty Difficulty, int32 BasePointValue, float DifficultyScalar) const;
	int32 GetDifficultyLevelUpXPThreshold(ET66Difficulty Difficulty) const;
	float GetDifficultyLevelUpWaveRadiusUU(ET66Difficulty Difficulty) const;
	float GetDifficultyHeadshotChancePerBonusPoint(ET66Difficulty Difficulty) const;
	float GetDifficultyHeadshotStunDurationSeconds(ET66Difficulty Difficulty) const;

private:
	void QueueTuningDataTableLoad();
	void HandleTuningDataTableLoaded();
	bool IsTuningReady(const TCHAR* Caller) const;

	FT66PlayerExperienceTuningTable CachedTuning;
	bool bTuningLoaded = false;
	mutable bool bWarnedTuningUnavailable = false;
	TSharedPtr<FStreamableHandle> TuningDataTableLoadHandle;
};
