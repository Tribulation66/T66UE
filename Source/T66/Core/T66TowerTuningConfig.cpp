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
	StartRule.HazardSlots = { 0, 0 };
	StartRule.RewardContentSlots = { 0, 0 };
	StartRule.Weight = 1.0f;

	FT66TowerRoomRuleTuning& CombatRule = RoomRules.AddDefaulted_GetRef();
	CombatRule.RuleID = DefaultRoomRuleID;
	CombatRule.RoomRole = TEXT("Combat");
	CombatRule.WidthTiles = { DungeonMinRoomTiles, DungeonMaxRoomTiles };
	CombatRule.HeightTiles = { DungeonMinRoomTiles, DungeonMaxRoomTiles };
	CombatRule.HazardSlots = { 1, 2 };
	CombatRule.RewardContentSlots = { 1, 1 };
	CombatRule.Weight = 1.0f;

	FT66TowerRoomRuleTuning& BossRule = RoomRules.AddDefaulted_GetRef();
	BossRule.RuleID = BossRoomRuleID;
	BossRule.RoomRole = TEXT("Boss");
	BossRule.WidthTiles = { 5, 5 };
	BossRule.HeightTiles = { 5, 5 };
	BossRule.HazardSlots = { 0, 0 };
	BossRule.RewardContentSlots = { 0, 0 };
	BossRule.Weight = 1.0f;

	FT66TowerFloorHazardPoolTuning& Floor2Pool = HazardPools.AddDefaulted_GetRef();
	Floor2Pool.FloorNumber = 2;
	Floor2Pool.FloorRole = TEXT("Mob");
	Floor2Pool.HazardPool = { TEXT("ObstacleSweeperArm"), TEXT("ObstacleFloorBumper"), TEXT("ObstacleWallBumper"), TEXT("ObstacleCeilingHammer") };

	FT66TowerFloorHazardPoolTuning& Floor3Pool = HazardPools.AddDefaulted_GetRef();
	Floor3Pool.FloorNumber = 3;
	Floor3Pool.FloorRole = TEXT("Mob");
	Floor3Pool.HazardPool = Floor2Pool.HazardPool;

	FT66TowerFloorHazardPoolTuning& Floor4Pool = HazardPools.AddDefaulted_GetRef();
	Floor4Pool.FloorNumber = 4;
	Floor4Pool.FloorRole = TEXT("Mob");
	Floor4Pool.HazardPool.Reset();
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
	LoadTowerIntValue(ConfigFilename, TEXT("RoomMaxGapCells"), RoomMaxGapCells);
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

	LoadTowerIntValue(ConfigFilename, TEXT("BounceCoursePlatforms"), BounceCoursePlatforms);
	LoadTowerFloatValue(ConfigFilename, TEXT("PlatformTier1Height"), PlatformTier1Height);
	LoadTowerFloatValue(ConfigFilename, TEXT("PlatformTier2Height"), PlatformTier2Height);
	LoadTowerFloatValue(ConfigFilename, TEXT("ChainPlatformFootprint"), ChainPlatformFootprint);
	LoadTowerFloatValue(ConfigFilename, TEXT("RoomPlatformFootprintMin"), RoomPlatformFootprintMin);
	LoadTowerFloatValue(ConfigFilename, TEXT("RoomPlatformFootprintMax"), RoomPlatformFootprintMax);
	LoadTowerIntValue(ConfigFilename, TEXT("RoomPlatformDensityTiles"), RoomPlatformDensityTiles);
	LoadTowerFloatValue(ConfigFilename, TEXT("PlatformChainMaxGap"), PlatformChainMaxGap);
	LoadTowerFloatValue(ConfigFilename, TEXT("RoundPlatformChance"), RoundPlatformChance);
	LoadTowerFloatValue(ConfigFilename, TEXT("RampWidth"), RampWidth);
	LoadTowerFloatValue(ConfigFilename, TEXT("RampLength"), RampLength);

	LoadTowerIntValue(ConfigFilename, TEXT("TierTerrain"), TierTerrain);
	LoadTowerFloatValue(ConfigFilename, TEXT("TierHeight"), TierHeight);
	LoadTowerIntValue(ConfigFilename, TEXT("MesaInsetCells"), MesaInsetCells);
	LoadTowerIntValue(ConfigFilename, TEXT("MesaMinSpanCells"), MesaMinSpanCells);
	LoadTowerIntValue(ConfigFilename, TEXT("MesaRampsMin"), MesaRampsMin);
	LoadTowerIntValue(ConfigFilename, TEXT("MesaRampsMax"), MesaRampsMax);
	LoadTowerFloatValue(ConfigFilename, TEXT("MesaTopBafflePitch"), MesaTopBafflePitch);
	LoadTowerFloatValue(ConfigFilename, TEXT("MesaTopBaffleDiameter"), MesaTopBaffleDiameter);
	LoadTowerFloatValue(ConfigFilename, TEXT("RampRollerDiameter"), RampRollerDiameter);
	LoadTowerFloatValue(ConfigFilename, TEXT("RingMesaChance"), RingMesaChance);

	LoadTowerIntValue(ConfigFilename, TEXT("TierLifts"), TierLifts);
	LoadTowerFloatValue(ConfigFilename, TEXT("LiftChance"), LiftChance);
	LoadTowerFloatValue(ConfigFilename, TEXT("LiftFootprint"), LiftFootprint);
	LoadTowerFloatValue(ConfigFilename, TEXT("LiftTravelSeconds"), LiftTravelSeconds);
	LoadTowerFloatValue(ConfigFilename, TEXT("LiftDwellSeconds"), LiftDwellSeconds);

	LoadTowerIntValue(ConfigFilename, TEXT("TowerLavaRise"), TowerLavaRise);
	LoadTowerFloatValue(ConfigFilename, TEXT("LavaGraceSeconds"), LavaGraceSeconds);
	LoadTowerFloatValue(ConfigFilename, TEXT("LavaRiseSeconds"), LavaRiseSeconds);
	LoadTowerFloatValue(ConfigFilename, TEXT("LavaMaxHeight"), LavaMaxHeight);
	LoadTowerIntValue(ConfigFilename, TEXT("LavaDamagePerTick"), LavaDamagePerTick);

	LoadTowerIntValue(ConfigFilename, TEXT("DoorwayArches"), DoorwayArches);
	LoadTowerIntValue(ConfigFilename, TEXT("ArchSegments"), ArchSegments);
	LoadTowerFloatValue(ConfigFilename, TEXT("ArchTubeDiameter"), ArchTubeDiameter);

	LoadTowerNameValue(ConfigFilename, TEXT("DefaultRoomRuleID"), DefaultRoomRuleID);
	LoadTowerNameValue(ConfigFilename, TEXT("StartRoomRuleID"), StartRoomRuleID);
	LoadTowerNameValue(ConfigFilename, TEXT("BossRoomRuleID"), BossRoomRuleID);
	LoadTowerStructArray(ConfigFilename, TEXT("RoomRules"), RoomRules);
	LoadTowerStructArray(ConfigFilename, TEXT("HazardPools"), HazardPools);

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

const FT66TowerFloorHazardPoolTuning* UT66TowerTuningConfig::FindHazardPoolForFloor(const int32 FloorNumber, const FName FloorRole) const
{
	const FT66TowerFloorHazardPoolTuning* RoleFallback = nullptr;
	for (const FT66TowerFloorHazardPoolTuning& Pool : HazardPools)
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
	// Bounds the corridor a room-graph edge can carve: halls stay short by construction.
	RoomMaxGapCells = FMath::Clamp(RoomMaxGapCells, 2, 20);
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

	// Bounce-course invariants derive from hero jump reach (JumpZ 1600 / gravity 4.5 -> ~290uu
	// max step, ~420uu safe flat gap at minimum walk speed). Each tier step and the chain gap
	// must stay single-jumpable, and the lava cap must stay below Tier 2 tops.
	PlatformTier1Height = FMath::Clamp(PlatformTier1Height, 120.0f, 260.0f);
	PlatformTier2Height = FMath::Clamp(PlatformTier2Height, PlatformTier1Height + 120.0f, PlatformTier1Height + 260.0f);
	PlatformChainMaxGap = FMath::Clamp(PlatformChainMaxGap, 200.0f, 420.0f);
	const float MaxChainFootprint = FMath::Max(GridCellSize - 240.0f, 400.0f);
	const float MinChainFootprint = FMath::Clamp(GridCellSize - PlatformChainMaxGap, 400.0f, MaxChainFootprint);
	ChainPlatformFootprint = FMath::Clamp(ChainPlatformFootprint, MinChainFootprint, MaxChainFootprint);
	RoomPlatformFootprintMin = FMath::Clamp(RoomPlatformFootprintMin, 400.0f, MaxChainFootprint);
	RoomPlatformFootprintMax = FMath::Clamp(RoomPlatformFootprintMax, RoomPlatformFootprintMin, MaxChainFootprint);
	RoomPlatformDensityTiles = FMath::Max(2, RoomPlatformDensityTiles);
	RoundPlatformChance = FMath::Clamp(RoundPlatformChance, 0.0f, 1.0f);
	RampWidth = FMath::Clamp(RampWidth, 300.0f, GridCellSize);
	RampLength = FMath::Clamp(RampLength, PlatformTier1Height * 1.6f, GridCellSize);

	LavaGraceSeconds = FMath::Clamp(LavaGraceSeconds, 0.0f, 180.0f);
	LavaRiseSeconds = FMath::Clamp(LavaRiseSeconds, 20.0f, 600.0f);
	LavaMaxHeight = FMath::Clamp(LavaMaxHeight, 60.0f, PlatformTier2Height - 60.0f);
	LavaDamagePerTick = FMath::Clamp(LavaDamagePerTick, 1, 200);

	// Tier terrain invariants: the tier step must clear both jump reach (so ramps
	// matter) and the lava cap (so mesas stay dry); ramps occupy one grid cell, so
	// the slope stays walkable for any TierHeight <= GridCellSize (<= 45 degrees).
	TierHeight = FMath::Clamp(TierHeight, FMath::Max(360.0f, LavaMaxHeight + 120.0f), GridCellSize);
	MesaInsetCells = FMath::Clamp(MesaInsetCells, 1, 6);
	MesaMinSpanCells = FMath::Clamp(MesaMinSpanCells, 2, 12);
	MesaRampsMin = FMath::Clamp(MesaRampsMin, 2, 4);
	MesaRampsMax = FMath::Clamp(MesaRampsMax, MesaRampsMin, 6);
	MesaTopBafflePitch = FMath::Clamp(MesaTopBafflePitch, 120.0f, 400.0f);
	MesaTopBaffleDiameter = FMath::Clamp(MesaTopBaffleDiameter, 100.0f, MesaTopBafflePitch * 1.25f);
	RampRollerDiameter = FMath::Clamp(RampRollerDiameter, 50.0f, 200.0f);
	RingMesaChance = FMath::Clamp(RingMesaChance, 0.0f, 1.0f);

	// Lift invariants: the slab must fit its grid cell with clearance to the mesa
	// face, the rise speed must stay rideable (<= ~400uu/s keeps based movement
	// comfortable), and the dwell must leave time to board a parked slab.
	LiftChance = FMath::Clamp(LiftChance, 0.0f, 1.0f);
	LiftFootprint = FMath::Clamp(LiftFootprint, 400.0f, FMath::Max(GridCellSize - 100.0f, 400.0f));
	LiftTravelSeconds = FMath::Clamp(LiftTravelSeconds, FMath::Max(1.0f, TierHeight / 400.0f), 15.0f);
	LiftDwellSeconds = FMath::Clamp(LiftDwellSeconds, 0.75f, 10.0f);

	ArchSegments = FMath::Clamp(ArchSegments, 4, 24);
	ArchTubeDiameter = FMath::Clamp(ArchTubeDiameter, 60.0f, 200.0f);

	for (FT66TowerRoomRuleTuning& Rule : RoomRules)
	{
		Rule.WidthTiles = SanitizeIntRange(Rule.WidthTiles, 1);
		Rule.HeightTiles = SanitizeIntRange(Rule.HeightTiles, 1);
		Rule.HazardSlots = SanitizeIntRange(Rule.HazardSlots, 0);
		Rule.RewardContentSlots = SanitizeIntRange(Rule.RewardContentSlots, 0);
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
