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
	FT66IntRange TrapSlots = { 0, 0 };

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Room", meta = (ClampMin = "0"))
	FT66IntRange NonTrapContentSlots = { 0, 0 };

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Room", meta = (ClampMin = "0.0"))
	float Weight = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Room")
	FName LootBiasTag = NAME_None;
};

USTRUCT(BlueprintType)
struct T66_API FT66TowerFloorTrapPoolTuning
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Trap")
	int32 FloorNumber = 0;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Trap")
	FName FloorRole = NAME_None;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tower|Trap")
	TArray<FName> TrapPool;
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
	const FT66TowerFloorTrapPoolTuning* FindTrapPoolForFloor(int32 FloorNumber, FName FloorRole) const;

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

	FName DefaultRoomRuleID = FName(TEXT("DefaultCombat"));
	FName StartRoomRuleID = FName(TEXT("Start"));
	FName BossRoomRuleID = FName(TEXT("Boss"));
	TArray<FT66TowerRoomRuleTuning> RoomRules;
	TArray<FT66TowerFloorTrapPoolTuning> TrapPools;

private:
	void Sanitize();
};
