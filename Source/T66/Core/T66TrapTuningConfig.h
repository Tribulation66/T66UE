// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66RngTuningConfig.h"
#include "T66TrapTuningConfig.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66TrapSpawnWindow
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Spawn", meta = (ClampMin = "0"))
	FT66IntRange SpawnCount = { 1, 1 };

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Spawn", meta = (ClampMin = "1"))
	int32 SpawnAttempts = 18;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Spawn", meta = (ClampMin = "0.0"))
	float MinTrapSpacing = 1300.f;
};

USTRUCT(BlueprintType)
struct T66_API FT66TrapLevelSpawnTuning
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Level", meta = (ClampMin = "0"))
	FT66IntRange TotalTrapCount = { 3, 4 };
};

USTRUCT(BlueprintType)
struct T66_API FT66WallProjectileTrapTuning
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Spawn")
	FT66TrapSpawnWindow Spawn;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Wall", meta = (ClampMin = "0.0"))
	float HeightOffset = 115.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Wall", meta = (ClampMin = "0.05"))
	float FireIntervalSeconds = 2.6f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Wall", meta = (ClampMin = "0.0"))
	float WindupDurationSeconds = 0.4f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Wall")
	FT66FloatRange InitialFireDelaySeconds = { 0.35f, 1.15f };

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Wall", meta = (ClampMin = "0.0"))
	float DetectionRange = 5200.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Wall", meta = (ClampMin = "0.0"))
	float ProjectileSpeed = 2400.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Wall", meta = (ClampMin = "1"))
	int32 DamageHP = 12;
};

USTRUCT(BlueprintType)
struct T66_API FT66FloorBurstTrapTuning
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Spawn")
	FT66TrapSpawnWindow Spawn;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Spawn", meta = (ClampMin = "0.0"))
	float EdgePadding = 1800.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Spawn", meta = (ClampMin = "0.0"))
	float HolePadding = 1900.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Burst", meta = (ClampMin = "0.0"))
	float Radius = 260.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Burst", meta = (ClampMin = "0.0"))
	float WarningDurationSeconds = 0.8f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Burst", meta = (ClampMin = "0.0"))
	float ActiveDurationSeconds = 1.15f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Burst", meta = (ClampMin = "0.05"))
	float CooldownDurationSeconds = 3.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Burst", meta = (ClampMin = "0.05"))
	float DamageIntervalSeconds = 0.35f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Burst")
	FT66FloatRange InitialCycleDelaySeconds = { 0.80f, 2.10f };

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Burst", meta = (ClampMin = "1"))
	int32 DamageHP = 10;
};

USTRUCT(BlueprintType)
struct T66_API FT66AreaControlTrapTuning
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Spawn")
	FT66TrapSpawnWindow Spawn;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Spawn", meta = (ClampMin = "0.0"))
	float EdgePadding = 1500.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Spawn", meta = (ClampMin = "0.0"))
	float HolePadding = 1650.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Area", meta = (ClampMin = "0.0"))
	float Radius = 280.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Area", meta = (ClampMin = "0.0"))
	float WarningDurationSeconds = 0.85f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Area", meta = (ClampMin = "0.0"))
	float RiseDurationSeconds = 0.25f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Area", meta = (ClampMin = "0.0"))
	float RaisedDurationSeconds = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Area", meta = (ClampMin = "0.0"))
	float RetractDurationSeconds = 0.25f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Area", meta = (ClampMin = "0.05"))
	float CooldownDurationSeconds = 2.8f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Area", meta = (ClampMin = "0.05"))
	float DamageIntervalSeconds = 0.35f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Area")
	FT66FloatRange InitialCycleDelaySeconds = { 0.95f, 2.10f };

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Area", meta = (ClampMin = "30.0"))
	float SpikeHeight = 160.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Area", meta = (ClampMin = "3"))
	int32 SpikeCount = 10;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Area", meta = (ClampMin = "1"))
	int32 DamageHP = 11;
};

class T66_API UT66TrapTuningConfig
{
public:
	UT66TrapTuningConfig();

	void LoadFromConfig();

	const FT66TrapLevelSpawnTuning* FindLevelSpawnTuning(int32 GameplayLevelNumber) const;
	const FT66WallProjectileTrapTuning* FindWallProjectileTuning(FName RegistryKey) const;
	const FT66FloorBurstTrapTuning* FindFloorBurstTuning(FName RegistryKey) const;
	const FT66AreaControlTrapTuning* FindAreaControlTuning(FName RegistryKey) const;

	FT66TrapLevelSpawnTuning GameplayLevel1;
	FT66TrapLevelSpawnTuning GameplayLevel2;
	FT66TrapLevelSpawnTuning GameplayLevel3;
	FT66TrapLevelSpawnTuning GameplayLevel4;
	FT66TrapLevelSpawnTuning GameplayLevel5;

	FT66WallProjectileTrapTuning DungeonWallArrow;
	FT66FloorBurstTrapTuning DungeonFloorFlame;
	FT66AreaControlTrapTuning DungeonFloorSpikePatch;

	FT66WallProjectileTrapTuning ForestThornVolley;
	FT66FloorBurstTrapTuning ForestSporeBurst;
	FT66AreaControlTrapTuning ForestBramblePatch;

	FT66WallProjectileTrapTuning OceanHarpoonVolley;
	FT66FloorBurstTrapTuning OceanSteamBurst;
	FT66AreaControlTrapTuning OceanUrchinPatch;

	FT66WallProjectileTrapTuning MartianShardVolley;
	FT66FloorBurstTrapTuning MartianPlasmaBurst;
	FT66AreaControlTrapTuning MartianCrystalPatch;

	FT66WallProjectileTrapTuning HellSoulBoltVolley;
	FT66FloorBurstTrapTuning HellEmberBurst;
	FT66AreaControlTrapTuning HellBrimstonePatch;
};
