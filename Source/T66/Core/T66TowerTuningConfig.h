// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66RngTuningConfig.h"
#include "T66TowerTuningConfig.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66TowerRoomRuleTuning
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Room")
	FName RuleID = FName(TEXT("DefaultCombat"));

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Room")
	FName RoomRole = FName(TEXT("Combat"));

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Room", meta = (ClampMin = "1"))
	FT66IntRange WidthTiles = { 2, 5 };

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Room", meta = (ClampMin = "1"))
	FT66IntRange HeightTiles = { 2, 5 };

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Room", meta = (ClampMin = "0"))
	FT66IntRange HazardSlots = { 0, 0 };

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Room", meta = (ClampMin = "0"))
	FT66IntRange RewardContentSlots = { 0, 0 };

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Room", meta = (ClampMin = "0.0"))
	float Weight = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Room")
	FName LootBiasTag = NAME_None;
};

USTRUCT(BlueprintType)
struct T66_API FT66TowerFloorHazardPoolTuning
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Hazard")
	int32 FloorNumber = 0;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Hazard")
	FName FloorRole = NAME_None;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Hazard")
	TArray<FName> HazardPool;
};

class T66_API UT66TowerTuningConfig
{
public:
	UT66TowerTuningConfig();

	void LoadFromConfig();

	static const UT66TowerTuningConfig& GetRuntimeConfig();

	int32 GetNormalTotalFloorCount() const;
	int32 GetBossRushTotalFloorCount() const;
	FT66IntRange GetTowerChestCountRange() const;
	FT66IntRange GetTowerCrateCountRange() const;
	const FT66TowerRoomRuleTuning* FindRoomRule(FName RuleID) const;
	const FT66TowerFloorHazardPoolTuning* FindHazardPoolForFloor(int32 FloorNumber, FName FloorRole) const;

	float RoofSkinThickness = 12.0f;
	float StartFloorHeadroom = 2000.0f;
	float StandardFloorHeadroom = 1200.0f;
	float PlacementCellSize = 1000.0f;
	float DungeonKitWallDepth = 120.0f;
	float GeneratedDungeonKitWallHeight = 1200.0f;
	float GeneratedDungeonKitFloorThickness = 24.0f;
	int32 GeneratedDungeonKitCullDistance = 30000;
	float ShellRadius = 20000.0f;
	float StartRoomSquareSize = 6500.0f;

	int32 GridColumns = 70;
	int32 GridRows = 70;
	float GridCellSize = 1000.0f;
	float GridDoorWidth = 1000.0f;
	int32 DungeonMinRooms = 10;
	int32 DungeonMaxRooms = 10;
	int32 DungeonMinRoomTiles = 10;
	int32 DungeonMaxRoomTiles = 20;
	int32 RoomMaxGapCells = 6;
	int32 StartRoomMinTiles = 3;
	int32 StartRoomMaxTiles = 4;
	float GridBranchChance = 0.35f;
	int32 GridMaxBranchCells = 3;

	int32 StartFloorNumber = 1;
	int32 FirstMobFloorNumber = 2;
	int32 LastMobFloorNumber = 3;
	int32 BossFloorNumber = 4;
	int32 BossRushBossFloorNumber = 2;

	FT66IntRange TowerChestCountPerFloor = { 1, 3 };
	FT66IntRange TowerCrateCountPerFloor = { 1, 3 };
	float TowerFountainChancePerFloor = 0.40f;

	// Bouncy obstacle-course platforms (gameplay floors only).
	int32 BounceCoursePlatforms = 1;
	float PlatformTier1Height = 200.0f;
	float PlatformTier2Height = 400.0f;
	float ChainPlatformFootprint = 700.0f;
	float RoomPlatformFootprintMin = 550.0f;
	float RoomPlatformFootprintMax = 750.0f;
	int32 RoomPlatformDensityTiles = 8;
	float PlatformChainMaxGap = 350.0f;
	float RoundPlatformChance = 0.45f;
	float RampWidth = 600.0f;
	float RampLength = 520.0f;

	// Tier terrain: per-room raised mesas with constructive ramp access (Tail Tag pattern).
	int32 TierTerrain = 1;
	float TierHeight = 500.0f;
	int32 MesaInsetCells = 2;
	int32 MesaMinSpanCells = 3;
	int32 MesaRampsMin = 2;
	int32 MesaRampsMax = 4;
	float MesaTopBafflePitch = 200.0f;
	float MesaTopBaffleDiameter = 180.0f;
	float RampRollerDiameter = 90.0f;
	// Ring mesas: chance a >=5x5 mesa deck gets a center drop hole (section 1.6).
	float RingMesaChance = 0.5f;

	// Moving lift platforms (Fall Guys elevators): a mesa with a surplus ramp
	// candidate converts one into a cycling lift; static ramps stay >= MesaRampsMin.
	int32 TierLifts = 1;
	float LiftChance = 0.5f;
	float LiftFootprint = 600.0f;
	float LiftTravelSeconds = 3.0f;
	float LiftDwellSeconds = 2.0f;

	// Rising-lava floor hazard (replaces the spread coverage on tower gameplay floors when enabled).
	int32 TowerLavaRise = 1;
	float LavaGraceSeconds = 25.0f;
	float LavaRiseSeconds = 150.0f;
	float LavaMaxHeight = 320.0f;
	int32 LavaDamagePerTick = 20;

	// Inflatable doorway arches (visual replacement for flat doorway lintels in baffle mode).
	int32 DoorwayArches = 1;
	int32 ArchSegments = 10;
	float ArchTubeDiameter = 110.0f;

	FName DefaultRoomRuleID = FName(TEXT("DefaultCombat"));
	FName StartRoomRuleID = FName(TEXT("Start"));
	FName BossRoomRuleID = FName(TEXT("Boss"));
	TArray<FT66TowerRoomRuleTuning> RoomRules;
	TArray<FT66TowerFloorHazardPoolTuning> HazardPools;

private:
	void Sanitize();
};
