// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66TowerTuningConfig.h"

#include "Misc/ConfigCacheIni.h"

namespace
{
	static constexpr const TCHAR* T66TowerConfigSection = TEXT("/Script/T66.T66TowerTuningConfig");

	static FString GetT66TowerConfigFilename()
	{
		FString ConfigFilename;
		FConfigCacheIni::LoadGlobalIniFile(ConfigFilename, TEXT("T66TowerTuning"));
		return ConfigFilename;
	}

	static FT66IntRange SanitizeIntRange(FT66IntRange Range, const int32 MinAllowed)
	{
		Range.Min = FMath::Max(MinAllowed, Range.Min);
		Range.Max = FMath::Max(Range.Min, Range.Max);
		return Range;
	}

	static void LoadTowerIntValue(const FString& ConfigFilename, const TCHAR* Key, int32& Value)
	{
		if (GConfig)
		{
			GConfig->GetInt(T66TowerConfigSection, Key, Value, ConfigFilename);
		}
	}

	static void LoadTowerFloatValue(const FString& ConfigFilename, const TCHAR* Key, float& Value)
	{
		if (GConfig)
		{
			GConfig->GetFloat(T66TowerConfigSection, Key, Value, ConfigFilename);
		}
	}

	static void LoadTowerNameValue(const FString& ConfigFilename, const TCHAR* Key, FName& Value)
	{
		if (!GConfig)
		{
			return;
		}

		FString RawValue;
		if (GConfig->GetString(T66TowerConfigSection, Key, RawValue, ConfigFilename))
		{
			Value = FName(*RawValue.TrimStartAndEnd());
		}
	}

	template <typename StructType>
	static void LoadTowerStructValue(const FString& ConfigFilename, const TCHAR* Key, StructType& Value)
	{
		if (!GConfig)
		{
			return;
		}

		FString RawValue;
		if (!GConfig->GetString(T66TowerConfigSection, Key, RawValue, ConfigFilename) || RawValue.IsEmpty())
		{
			return;
		}

		UScriptStruct* StructTypeInfo = StructType::StaticStruct();
		if (!StructTypeInfo)
		{
			return;
		}

		StructTypeInfo->ImportText(*RawValue, &Value, nullptr, PPF_None, GLog, StructTypeInfo->GetName());
	}

	template <typename StructType>
	static void LoadTowerStructArray(const FString& ConfigFilename, const TCHAR* Key, TArray<StructType>& Values)
	{
		if (!GConfig)
		{
			return;
		}

		TArray<FString> RawValues;
		if (GConfig->GetArray(T66TowerConfigSection, Key, RawValues, ConfigFilename) <= 0)
		{
			return;
		}

		UScriptStruct* StructTypeInfo = StructType::StaticStruct();
		if (!StructTypeInfo)
		{
			return;
		}

		Values.Reset();
		for (const FString& RawValue : RawValues)
		{
			if (RawValue.IsEmpty())
			{
				continue;
			}

			StructType ParsedValue;
			StructTypeInfo->ImportText(*RawValue, &ParsedValue, nullptr, PPF_None, GLog, StructTypeInfo->GetName());
			Values.Add(ParsedValue);
		}
	}
}

UT66TowerTuningConfig::UT66TowerTuningConfig()
{
	FT66TowerRoomRuleTuning& StartRule = RoomRules.AddDefaulted_GetRef();
	StartRule.RuleID = StartRoomRuleID;
	StartRule.RoomRole = TEXT("Start");
	StartRule.WidthTiles = { 5, 5 };
	StartRule.HeightTiles = { 5, 5 };
	StartRule.TrapSlots = { 0, 0 };
	StartRule.NonTrapContentSlots = { 0, 0 };
	StartRule.Weight = 1.0f;

	FT66TowerRoomRuleTuning& CombatRule = RoomRules.AddDefaulted_GetRef();
	CombatRule.RuleID = DefaultRoomRuleID;
	CombatRule.RoomRole = TEXT("Combat");
	CombatRule.WidthTiles = { DungeonMinRoomTiles, DungeonMaxRoomTiles };
	CombatRule.HeightTiles = { DungeonMinRoomTiles, DungeonMaxRoomTiles };
	CombatRule.TrapSlots = { 1, 2 };
	CombatRule.NonTrapContentSlots = { 1, 1 };
	CombatRule.Weight = 1.0f;

	FT66TowerRoomRuleTuning& BossRule = RoomRules.AddDefaulted_GetRef();
	BossRule.RuleID = BossRoomRuleID;
	BossRule.RoomRole = TEXT("Boss");
	BossRule.WidthTiles = { 5, 5 };
	BossRule.HeightTiles = { 5, 5 };
	BossRule.TrapSlots = { 0, 0 };
	BossRule.NonTrapContentSlots = { 0, 0 };
	BossRule.Weight = 1.0f;

	FT66TowerFloorTrapPoolTuning& Floor2Pool = TrapPools.AddDefaulted_GetRef();
	Floor2Pool.FloorNumber = 2;
	Floor2Pool.FloorRole = TEXT("Mob");
	Floor2Pool.TrapPool = { TEXT("ObstacleSweeperArm"), TEXT("ObstacleFloorBumper"), TEXT("ObstacleWallBumper"), TEXT("ObstacleCeilingHammer") };

	FT66TowerFloorTrapPoolTuning& Floor3Pool = TrapPools.AddDefaulted_GetRef();
	Floor3Pool.FloorNumber = 3;
	Floor3Pool.FloorRole = TEXT("Mob");
	Floor3Pool.TrapPool = Floor2Pool.TrapPool;

	FT66TowerFloorTrapPoolTuning& Floor4Pool = TrapPools.AddDefaulted_GetRef();
	Floor4Pool.FloorNumber = 4;
	Floor4Pool.FloorRole = TEXT("Mob");
	Floor4Pool.TrapPool.Reset();
}

void UT66TowerTuningConfig::LoadFromConfig()
{
	const FString ConfigFilename = GetT66TowerConfigFilename();

	LoadTowerFloatValue(ConfigFilename, TEXT("RoofSkinThickness"), RoofSkinThickness);
	LoadTowerFloatValue(ConfigFilename, TEXT("StartFloorHeadroom"), StartFloorHeadroom);
	LoadTowerFloatValue(ConfigFilename, TEXT("StandardFloorHeadroom"), StandardFloorHeadroom);
	LoadTowerFloatValue(ConfigFilename, TEXT("PlacementCellSize"), PlacementCellSize);
	LoadTowerFloatValue(ConfigFilename, TEXT("DungeonKitWallDepth"), DungeonKitWallDepth);
	LoadTowerFloatValue(ConfigFilename, TEXT("GeneratedDungeonKitWallHeight"), GeneratedDungeonKitWallHeight);
	LoadTowerFloatValue(ConfigFilename, TEXT("GeneratedDungeonKitFloorThickness"), GeneratedDungeonKitFloorThickness);
	LoadTowerIntValue(ConfigFilename, TEXT("GeneratedDungeonKitCullDistance"), GeneratedDungeonKitCullDistance);
	LoadTowerFloatValue(ConfigFilename, TEXT("ShellRadius"), ShellRadius);
	LoadTowerFloatValue(ConfigFilename, TEXT("StartRoomSquareSize"), StartRoomSquareSize);

	LoadTowerIntValue(ConfigFilename, TEXT("GridColumns"), GridColumns);
	LoadTowerIntValue(ConfigFilename, TEXT("GridRows"), GridRows);
	LoadTowerFloatValue(ConfigFilename, TEXT("GridCellSize"), GridCellSize);
	LoadTowerFloatValue(ConfigFilename, TEXT("GridDoorWidth"), GridDoorWidth);
	LoadTowerIntValue(ConfigFilename, TEXT("DungeonMinRooms"), DungeonMinRooms);
	LoadTowerIntValue(ConfigFilename, TEXT("DungeonMaxRooms"), DungeonMaxRooms);
	LoadTowerIntValue(ConfigFilename, TEXT("DungeonMinRoomTiles"), DungeonMinRoomTiles);
	LoadTowerIntValue(ConfigFilename, TEXT("DungeonMaxRoomTiles"), DungeonMaxRoomTiles);
	LoadTowerIntValue(ConfigFilename, TEXT("StartRoomMinTiles"), StartRoomMinTiles);
	LoadTowerIntValue(ConfigFilename, TEXT("StartRoomMaxTiles"), StartRoomMaxTiles);
	LoadTowerFloatValue(ConfigFilename, TEXT("GridBranchChance"), GridBranchChance);
	LoadTowerIntValue(ConfigFilename, TEXT("GridMaxBranchCells"), GridMaxBranchCells);

	LoadTowerIntValue(ConfigFilename, TEXT("StartFloorNumber"), StartFloorNumber);
	LoadTowerIntValue(ConfigFilename, TEXT("FirstMobFloorNumber"), FirstMobFloorNumber);
	LoadTowerIntValue(ConfigFilename, TEXT("LastMobFloorNumber"), LastMobFloorNumber);
	LoadTowerIntValue(ConfigFilename, TEXT("BossFloorNumber"), BossFloorNumber);
	LoadTowerIntValue(ConfigFilename, TEXT("BossRushBossFloorNumber"), BossRushBossFloorNumber);

	LoadTowerStructValue(ConfigFilename, TEXT("TowerChestCountPerFloor"), TowerChestCountPerFloor);
	LoadTowerStructValue(ConfigFilename, TEXT("TowerCrateCountPerFloor"), TowerCrateCountPerFloor);
	LoadTowerFloatValue(ConfigFilename, TEXT("TowerFountainChancePerFloor"), TowerFountainChancePerFloor);

	LoadTowerNameValue(ConfigFilename, TEXT("DefaultRoomRuleID"), DefaultRoomRuleID);
	LoadTowerNameValue(ConfigFilename, TEXT("StartRoomRuleID"), StartRoomRuleID);
	LoadTowerNameValue(ConfigFilename, TEXT("BossRoomRuleID"), BossRoomRuleID);
	LoadTowerStructArray(ConfigFilename, TEXT("RoomRules"), RoomRules);
	LoadTowerStructArray(ConfigFilename, TEXT("TrapPools"), TrapPools);

	Sanitize();
}

const UT66TowerTuningConfig& UT66TowerTuningConfig::GetRuntimeConfig()
{
	static UT66TowerTuningConfig RuntimeConfig;
	static bool bLoaded = false;
	if (!bLoaded)
	{
		RuntimeConfig.LoadFromConfig();
		bLoaded = true;
	}
	return RuntimeConfig;
}

int32 UT66TowerTuningConfig::GetNormalTotalFloorCount() const
{
	return FMath::Max(1, BossFloorNumber - StartFloorNumber + 1);
}

int32 UT66TowerTuningConfig::GetBossRushTotalFloorCount() const
{
	return FMath::Max(1, BossRushBossFloorNumber - StartFloorNumber + 1);
}

FT66IntRange UT66TowerTuningConfig::GetTowerChestCountRange() const
{
	return SanitizeIntRange(TowerChestCountPerFloor, 0);
}

FT66IntRange UT66TowerTuningConfig::GetTowerCrateCountRange() const
{
	return SanitizeIntRange(TowerCrateCountPerFloor, 0);
}

const FT66TowerRoomRuleTuning* UT66TowerTuningConfig::FindRoomRule(const FName RuleID) const
{
	for (const FT66TowerRoomRuleTuning& Rule : RoomRules)
	{
		if (Rule.RuleID == RuleID)
		{
			return &Rule;
		}
	}
	return nullptr;
}

const FT66TowerFloorTrapPoolTuning* UT66TowerTuningConfig::FindTrapPoolForFloor(const int32 FloorNumber, const FName FloorRole) const
{
	const FT66TowerFloorTrapPoolTuning* RoleFallback = nullptr;
	for (const FT66TowerFloorTrapPoolTuning& Pool : TrapPools)
	{
		if (Pool.FloorNumber == FloorNumber)
		{
			return &Pool;
		}
		if (Pool.FloorNumber <= 0 && !FloorRole.IsNone() && Pool.FloorRole == FloorRole)
		{
			RoleFallback = &Pool;
		}
	}
	return RoleFallback;
}

void UT66TowerTuningConfig::Sanitize()
{
	RoofSkinThickness = FMath::Max(0.0f, RoofSkinThickness);
	StartFloorHeadroom = FMath::Max(600.0f, StartFloorHeadroom);
	StandardFloorHeadroom = FMath::Max(600.0f, StandardFloorHeadroom);
	PlacementCellSize = FMath::Max(600.0f, PlacementCellSize);
	DungeonKitWallDepth = FMath::Max(20.0f, DungeonKitWallDepth);
	GeneratedDungeonKitWallHeight = FMath::Max(600.0f, GeneratedDungeonKitWallHeight);
	GeneratedDungeonKitFloorThickness = FMath::Max(1.0f, GeneratedDungeonKitFloorThickness);
	GeneratedDungeonKitCullDistance = FMath::Max(0, GeneratedDungeonKitCullDistance);
	ShellRadius = FMath::Max(PlacementCellSize * 4.0f, ShellRadius);
	StartRoomSquareSize = FMath::Max(PlacementCellSize, StartRoomSquareSize);

	GridColumns = FMath::Max(3, GridColumns);
	GridRows = FMath::Max(3, GridRows);
	GridCellSize = FMath::Max(600.0f, GridCellSize);
	GridDoorWidth = FMath::Max(200.0f, GridDoorWidth);
	DungeonMinRooms = FMath::Max(1, DungeonMinRooms);
	DungeonMaxRooms = FMath::Max(DungeonMinRooms, DungeonMaxRooms);
	DungeonMinRoomTiles = FMath::Max(1, DungeonMinRoomTiles);
	DungeonMaxRoomTiles = FMath::Max(DungeonMinRoomTiles, DungeonMaxRoomTiles);
	StartRoomMinTiles = FMath::Max(1, StartRoomMinTiles);
	StartRoomMaxTiles = FMath::Max(StartRoomMinTiles, StartRoomMaxTiles);
	GridBranchChance = FMath::Clamp(GridBranchChance, 0.0f, 1.0f);
	GridMaxBranchCells = FMath::Max(1, GridMaxBranchCells);

	StartFloorNumber = FMath::Max(1, StartFloorNumber);
	BossFloorNumber = FMath::Max(StartFloorNumber, BossFloorNumber);
	FirstMobFloorNumber = FMath::Clamp(FirstMobFloorNumber, StartFloorNumber, BossFloorNumber);
	LastMobFloorNumber = FMath::Clamp(LastMobFloorNumber, StartFloorNumber, BossFloorNumber);
	BossRushBossFloorNumber = FMath::Max(StartFloorNumber + 1, BossRushBossFloorNumber);

	TowerChestCountPerFloor = SanitizeIntRange(TowerChestCountPerFloor, 0);
	TowerCrateCountPerFloor = SanitizeIntRange(TowerCrateCountPerFloor, 0);
	TowerFountainChancePerFloor = FMath::Clamp(TowerFountainChancePerFloor, 0.0f, 1.0f);

	for (FT66TowerRoomRuleTuning& Rule : RoomRules)
	{
		Rule.WidthTiles = SanitizeIntRange(Rule.WidthTiles, 1);
		Rule.HeightTiles = SanitizeIntRange(Rule.HeightTiles, 1);
		Rule.TrapSlots = SanitizeIntRange(Rule.TrapSlots, 0);
		Rule.NonTrapContentSlots = SanitizeIntRange(Rule.NonTrapContentSlots, 0);
		Rule.Weight = FMath::Max(0.0f, Rule.Weight);
		if (Rule.RuleID.IsNone())
		{
			Rule.RuleID = DefaultRoomRuleID;
		}
	}

	if (RoomRules.Num() <= 0)
	{
		FT66TowerRoomRuleTuning& Rule = RoomRules.AddDefaulted_GetRef();
		Rule.RuleID = DefaultRoomRuleID;
		Rule.RoomRole = TEXT("Combat");
		Rule.WidthTiles = { DungeonMinRoomTiles, DungeonMaxRoomTiles };
		Rule.HeightTiles = { DungeonMinRoomTiles, DungeonMaxRoomTiles };
	}
}
