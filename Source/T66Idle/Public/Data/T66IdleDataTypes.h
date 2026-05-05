// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "T66IdleDataTypes.generated.h"

USTRUCT(BlueprintType)
struct T66IDLE_API FT66IdleHeroDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FName HeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString RoleLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FLinearColor AccentColor = FLinearColor(0.86f, 0.62f, 0.22f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double HireCostGold = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double TapDamage = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double PassiveDamagePerSecond = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double TapDamagePerLevel = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double PassiveDamagePerLevel = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	int32 MaxLevel = 1;
};

USTRUCT(BlueprintType)
struct T66IDLE_API FT66IdleCompanionDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FName CompanionID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString RoleLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FLinearColor AccentColor = FLinearColor(0.42f, 0.72f, 0.86f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double HireCostGold = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double PassiveDamagePerSecond = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double PassiveDamagePerLevel = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	int32 MaxLevel = 1;
};

USTRUCT(BlueprintType)
struct T66IDLE_API FT66IdleItemDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString Category;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FLinearColor AccentColor = FLinearColor(0.70f, 0.54f, 0.22f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double BaseCostGold = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double TapDamageBonus = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double PassiveDamageBonus = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double GoldRewardMultiplier = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	int32 MaxLevel = 1;
};

USTRUCT(BlueprintType)
struct T66IDLE_API FT66IdleIdolDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FName IdolID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString Category;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FLinearColor AccentColor = FLinearColor(0.62f, 0.34f, 0.88f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double BaseCostGold = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double TapDamageMultiplier = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double PassiveDamageMultiplier = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double GoldRewardMultiplier = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	int32 MaxLevel = 1;
};

USTRUCT(BlueprintType)
struct T66IDLE_API FT66IdleEnemyDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FName EnemyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString ArchetypeLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FLinearColor AccentColor = FLinearColor(0.82f, 0.24f, 0.18f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double BaseHealth = 10.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double HealthPerStage = 4.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double BaseGoldReward = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double GoldPerStage = 0.65;
};

USTRUCT(BlueprintType)
struct T66IDLE_API FT66IdleZoneDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FName ZoneID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString ThemeLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	int32 FirstStage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	int32 StageCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double BaseEnemyHealth = 10.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double BaseGoldReward = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	TArray<FName> EnemyIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double EnemyHealthScalar = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double GoldRewardScalar = 1.0;
};

USTRUCT(BlueprintType)
struct T66IDLE_API FT66IdleStageDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FName StageID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	int32 StageIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FName ZoneID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FName EnemyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FName BossEnemyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	bool bBossStage = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double ClearGoldReward = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FName NextStageID = NAME_None;
};

USTRUCT(BlueprintType)
struct T66IDLE_API FT66IdleTuningDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FName TuningID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	FName StartingHeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	int32 StartingStage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double StartingGold = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double StartingTapDamage = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double StartingPassiveDamagePerSecond = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double OfflineProgressCapHours = 12.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double AutosaveIntervalSeconds = 10.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double EngineProgressPerPassiveDamageSecond = 0.035;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double EngineGoldPayoutMultiplier = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double TapUpgradeBaseCost = 12.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double TapUpgradeCostPerDamage = 12.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double TapUpgradeDamageStep = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double EngineUpgradeBaseCost = 18.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double EngineUpgradeCostPerDps = 18.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle")
	double EngineUpgradeDpsStep = 0.5;
};

USTRUCT(BlueprintType)
struct T66IDLE_API FT66IdleProfileSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	int32 HighestStageReached = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	int32 CurrentStage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	FName CurrentStageID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	FName CurrentEnemyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	bool bCurrentEnemyIsStageBoss = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	double EnemyHealth = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	double EnemyMaxHealth = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	double EngineProgress = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	double UncollectedProgress = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	double Gold = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	double LifetimeGold = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	int32 BossStagesCleared = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	double TapDamage = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	double PassiveDamagePerSecond = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	FString LastSavedUtc;
};
