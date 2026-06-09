// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66RngTuningConfig.h"
#include "T66TrapTuningConfig.generated.h"

class UNiagaraSystem;
class UStaticMesh;

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
struct T66_API FT66TrapFloorSpawnTuning
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Floor", meta = (ClampMin = "0"))
	FT66IntRange TotalTrapCount = { 3, 4 };
};

USTRUCT(BlueprintType)
struct T66_API FT66TrapTowerFloorSpawnTuning
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Floor")
	int32 FloorNumber = 0;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Floor")
	FT66TrapFloorSpawnTuning Tuning;
};

USTRUCT(BlueprintType)
struct T66_API FT66TrapVisualAssetConfig
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Assets")
	FString WallArrowMesh;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Assets")
	FString ArrowProjectileMesh;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Assets")
	FString ArrowProjectileTrailNiagara;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Assets")
	FString ArrowProjectileFallbackTrailNiagara;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Assets")
	FString FloorFlameNiagara;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Assets")
	FString FloorSpikeMesh;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Assets")
	FString FloorSpikeClusterMesh;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Assets")
	FString FloorSpikeRiseBurstNiagara;
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

USTRUCT(BlueprintType)
struct T66_API FT66ObstacleTrapTuning
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Spawn")
	FT66TrapSpawnWindow Spawn;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Spawn", meta = (ClampMin = "0.0"))
	float EdgePadding = 1900.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Spawn", meta = (ClampMin = "0.0"))
	float HolePadding = 2100.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Spawn", meta = (ClampMin = "0.0"))
	float FootprintRadius = 650.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Obstacle", meta = (ClampMin = "0.0"))
	float LaunchXY = 9500.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Obstacle", meta = (ClampMin = "0.0"))
	float LaunchZ = 850.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Obstacle", meta = (ClampMin = "0.0"))
	float CooldownSeconds = 0.70f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Obstacle", meta = (ClampMin = "0.0"))
	float PrimarySize = 900.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Obstacle", meta = (ClampMin = "0.0"))
	float SecondarySize = 260.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Obstacle", meta = (ClampMin = "0.0"))
	float Height = 180.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Obstacle")
	float SpeedOrPeriod = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Trap|Obstacle")
	FT66FloatRange InitialPhaseSeconds = { 0.0f, 1.0f };
};

class T66_API UT66TrapTuningConfig
{
public:
	UT66TrapTuningConfig();

	void LoadFromConfig();

	const FT66TrapFloorSpawnTuning* FindFloorSpawnTuning(int32 TowerFloorNumber) const;
	const FT66WallProjectileTrapTuning* FindWallProjectileTuning(FName RegistryKey) const;
	const FT66FloorBurstTrapTuning* FindFloorBurstTuning(FName RegistryKey) const;
	const FT66AreaControlTrapTuning* FindAreaControlTuning(FName RegistryKey) const;
	const FT66ObstacleTrapTuning* FindObstacleTuning(FName RegistryKey) const;

	static const FT66TrapVisualAssetConfig& GetRuntimeTrapAssets();
	static UStaticMesh* LoadConfiguredTrapStaticMesh(const FString& ObjectPathString, const TCHAR* ConfigKey);
	static UNiagaraSystem* LoadConfiguredTrapNiagaraSystem(const FString& ObjectPathString, const TCHAR* ConfigKey);

	FT66TrapFloorSpawnTuning TowerFloor2;
	FT66TrapFloorSpawnTuning TowerFloor3;
	FT66TrapFloorSpawnTuning TowerFloor4;
	TArray<FT66TrapTowerFloorSpawnTuning> TowerFloorSpawnTunings;

	FT66TrapVisualAssetConfig TrapAssets;

	FT66WallProjectileTrapTuning DungeonWallArrow;
	FT66FloorBurstTrapTuning DungeonFloorFlame;
	FT66AreaControlTrapTuning DungeonFloorSpikePatch;
	FT66ObstacleTrapTuning ObstacleSweeperArm;
	FT66ObstacleTrapTuning ObstacleFloorBumper;
	FT66ObstacleTrapTuning ObstacleWallBumper;
	FT66ObstacleTrapTuning ObstacleCeilingHammer;

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
