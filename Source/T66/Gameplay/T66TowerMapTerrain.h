// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/MapGeneration/Types/T66RoomCompositionTypes.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"

class UWorld;
enum class ET66Difficulty : uint8;
struct FActorSpawnParameters;

namespace T66TowerMapTerrain
{
	enum class ET66TowerFloorRole : uint8
	{
		Start,
		Mob,
		Boss,
	};

	enum class ET66TowerStartGalleryCategory : uint8
	{
		Heroes,
		Enemies,
		Bosses,
		World,
	};

	enum class ET66TowerGameplayLevelTheme : uint8
	{
		Dungeon,
		Forest,
		Ocean,
		Martian,
		Hell,
	};

	enum class ET66TowerMazeMode : uint8
	{
		LegacyLanes,
		GridGraph,
	};

	enum class ET66TowerGridCellSemantic : uint8
	{
		Unused,
		MainPath,
		OptionalLoop,
		Arrival,
		Exit,
		ArrivalExit,
	};

	enum class ET66TowerGridTemplate : uint8
	{
		Solid,
		Arena,
		Straight,
		Corner,
		TJunction,
		Cross,
		OptionalPocket,
	};

	enum ET66TowerGridConnection : uint8
	{
		GridNorth = 1 << 0,
		GridEast = 1 << 1,
		GridSouth = 1 << 2,
		GridWest = 1 << 3,
	};

	struct FGridCell
	{
		FIntPoint Coord = FIntPoint(INDEX_NONE, INDEX_NONE);
		FBox2D Bounds;
		FVector WorldCenter = FVector::ZeroVector;
		uint8 ConnectionMask = 0;
		ET66TowerGridCellSemantic Semantic = ET66TowerGridCellSemantic::Unused;
		ET66TowerGridTemplate Template = ET66TowerGridTemplate::Solid;
		bool bContainsArrival = false;
		bool bContainsExit = false;
		int32 MainPathIndex = INDEX_NONE;
		int32 LoopId = INDEX_NONE;
		TArray<FBox2D> EmittedWallBoxes;
		TArray<FVector> CachedSpawnSlots;
	};

	struct FRoom
	{
		int32 RoomId = INDEX_NONE;
		int32 FloorNumber = 0;
		FName RoomRuleID = NAME_None;
		FName RoomRoleID = NAME_None;
		FIntPoint MinCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		FIntPoint MaxCellExclusive = FIntPoint(INDEX_NONE, INDEX_NONE);
		FIntPoint CenterCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		FVector WorldCenter = FVector::ZeroVector;
		FBox2D Bounds;
		int32 WidthTiles = 0;
		int32 HeightTiles = 0;
		bool bContainsArrival = false;
		bool bContainsExit = false;
		/** Composition profile chosen by the procedural room composer. */
		FName CompositionProfileID = NAME_None;
		/** Reusable room structures selected by the composer: platforms, ramps, mesas, bridges, lifts, and bounce toys. */
		TArray<FName> StructureIDs;
		/**
		 * Designed payoff points (deck tops, pit floors, bridge ends). The
		 * interactable/NPC population fills these FIRST; each gets a beacon visual.
		 */
		TArray<FVector> RewardSlots;
	};

	/**
	 * Raised bouncy obstacle-course platform on a gameplay floor.
	 * Collision is a hidden box proxy from floor surface to TopZ; visuals are baffle tubes.
	 * Tier 1 tops sit one jump above the base floor; Tier 2 tops stay above the lava-rise cap.
	 */
	/**
	 * Course platform shape (design ref section 1.6). Every kit shape has an EXACT
	 * square AABB (corners/flats land on the AABB faces), so Bounds stays the truth
	 * for the box-gap traversal proof and content exclusion. Non-square shapes carry
	 * their mesh's own exact simple collision (cylinder primitive / 1-hull convex prism).
	 */
	enum class ET66BouncePlatformShape : uint8
	{
		Square,
		Round,
		Hex,
		/** Scatter-only: pointy sides never carry the chain. */
		Triangle,
	};

	struct FBouncePlatform
	{
		FBox2D Bounds;
		float TopZ = 0.0f;
		int32 Tier = 1;
		FIntPoint Cell = FIntPoint(INDEX_NONE, INDEX_NONE);
		/** True when the platform belongs to the guaranteed arrival->exit dry chain. */
		bool bSafeChain = false;
		ET66BouncePlatformShape Shape = ET66BouncePlatformShape::Square;
		/** Seeded 90-degree yaw steps (hex/triangle variety; square AABB keeps Bounds yaw-invariant). */
		uint8 YawSteps = 0;
	};

	/** Walkable wedge connecting the base floor to a Tier 1 platform top. */
	struct FBounceRamp
	{
		/** Axis-aligned footprint; ascent runs along the long axis toward AscentSign. */
		FBox2D Bounds;
		float BaseZ = 0.0f;
		float TopZ = 0.0f;
		/** Unit grid direction of ascent (+X, -X, +Y, or -Y). */
		FIntPoint AscentSign = FIntPoint(1, 0);
	};

	/**
	 * Raised terrain-tier mesa inside a room (Tail Tag central-platform pattern).
	 * The mesa is a solid block TierHeight above the floor surface, always inset so a
	 * walkable ground ring remains, and always connected through 2+ ramp cells.
	 */
	struct FTierMesa
	{
		int32 RoomId = INDEX_NONE;
		FIntPoint MinCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		FIntPoint MaxCellExclusive = FIntPoint(INDEX_NONE, INDEX_NONE);
		FBox2D Bounds;
		/**
		 * Ring-mesa center hole (design ref section 1.6): hole cells revert to tier 0,
		 * the deck spawns as 4 frame slabs, and falling through is a legal drop into
		 * the open under-deck ground. INDEX_NONE min cell = solid deck (no hole).
		 */
		FIntPoint HoleMinCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		FIntPoint HoleMaxCellExclusive = FIntPoint(INDEX_NONE, INDEX_NONE);
		FBox2D HoleBounds;

		bool HasHole() const { return HoleMinCell.X != INDEX_NONE; }
	};

	/** Ground-cell ramp rising one tier step into an adjacent mesa edge (direction-locked). */
	struct FTierRamp
	{
		FIntPoint Cell = FIntPoint(INDEX_NONE, INDEX_NONE);
		/** Unit grid direction of ascent (into the mesa). */
		FIntPoint AscentSign = FIntPoint(1, 0);
		FBox2D Bounds;
	};

	/**
	 * Moving lift platform cycling between the ground tier and an adjacent mesa top
	 * (Fall Guys elevator, non-trap). Generated as an alternative to one of a mesa's
	 * constructive ramps; counts as an up-edge in the no-softlock BFS but is NEVER a
	 * dry safe-chain anchor (the parked slab is submerged at full lava flood).
	 * Collision at runtime is a moving hidden box proxy owned by AT66TowerLiftPlatform.
	 */
	struct FTierLift
	{
		FIntPoint Cell = FIntPoint(INDEX_NONE, INDEX_NONE);
		/** Unit grid direction of ascent (into the mesa). */
		FIntPoint AscentSign = FIntPoint(1, 0);
		/** Slab footprint, pushed toward the mesa face (20uu clearance). */
		FBox2D Bounds;
		/** Floor surface height (deck rests 30uu above this at the bottom of the cycle). */
		float BaseZ = 0.0f;
		/** Mesa surface height (deck top lands flush here at the top of the cycle). */
		float TopZ = 0.0f;
		/** Seeded cycle phase fraction [0..1) so co-located lifts desynchronize. */
		float PhaseFraction = 0.0f;
	};

	struct FFloor
	{
		int32 FloorNumber = 0;
		ET66TowerFloorRole FloorRole = ET66TowerFloorRole::Mob;
		int32 GameplayLevelNumber = INDEX_NONE;
		ET66TowerGameplayLevelTheme Theme = ET66TowerGameplayLevelTheme::Dungeon;
		bool bMobFloor = false;
		bool bHasDropHole = false;
		FVector Center = FVector::ZeroVector;
		float SurfaceZ = 0.0f;
		float WalkableHalfExtent = 9000.0f;
		float PolygonApothem = 9000.0f;
		float BoundsHalfExtent = 9000.0f;
		FVector HoleCenter = FVector::ZeroVector;
		FVector2D HoleHalfExtent = FVector2D::ZeroVector;
		FVector ArrivalPoint = FVector::ZeroVector;
		FVector ExitPoint = FVector::ZeroVector;
		FIntPoint ArrivalCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		FIntPoint ExitCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		TArray<FGridCell> GridCells;
		TArray<FIntPoint> MainPathCells;
		TArray<FIntPoint> OptionalCells;
		TArray<FBox2D> WalkableFloorBoxes;
		TArray<FBox2D> MazeWallBoxes;
		TArray<FBox2D> DoorwayHeaderBoxes;
		TArray<FBox2D> TrapEligibleWallBoxes;
		TArray<FBox2D> OuterShellWallBoxes;
		TArray<FVector> CachedWalkableSpawnSlots;
		TArray<FVector> CachedMainPathSpawnSlots;
		TArray<FVector> CachedOptionalSpawnSlots;
		TArray<FVector> CachedContentSpawnSlots;
		TArray<FRoom> Rooms;
		TArray<FBouncePlatform> BouncePlatforms;
		TArray<FBounceRamp> BounceRamps;
		/** Ordered BFS cell path from arrival to exit used by the guaranteed dry platform chain. */
		TArray<FIntPoint> SafeChainCells;
		/** Per grid cell terrain tier (0 = ground, 1 = mesa). Empty when tiers are disabled. */
		TArray<uint8> CellTiers;
		TArray<FTierMesa> TierMesas;
		TArray<FTierRamp> TierRamps;
		TArray<FTierLift> TierLifts;
		/**
		 * Composer-owned hazard anchors: signature hazards spawn here instead of at
		 * random floor locations. Parallel arrays: anchor world position (Z may sit
		 * on a deck top) and the preferred hazard type (NAME_None = any obstacle).
		 */
		TArray<FVector> HazardAnchors;
		TArray<FName> HazardAnchorTypes;
		/** Bounce pad spots (analysis C3): deliberate trampolines up to decks/rims. */
		TArray<FVector> BouncePadSpots;
		FName FloorTag = NAME_None;
	};

	struct FStartGalleryWing
	{
		ET66TowerStartGalleryCategory Category = ET66TowerStartGalleryCategory::Heroes;
		FName CategoryID = NAME_None;
		FVector Center = FVector::ZeroVector;
		FVector Direction = FVector::ForwardVector;
		FVector SideDirection = FVector::RightVector;
		float SurfaceZ = 0.0f;
		float AcrossHalfExtent = 4800.0f;
		float DepthHalfExtent = 4400.0f;
		FBox2D WalkableBox;
		int32 FloorNumber = 0;
	};

	struct FLayout
	{
		FT66MapPreset Preset;
		TArray<FFloor> Floors;
		float PlacementCellSize = 1800.0f;
		float FloorThickness = 320.0f;
		float FloorSpacing = 2600.0f;
		float WallThickness = 320.0f;
		float ShellRadius = 12000.0f;
		ET66TowerMazeMode MazeMode = ET66TowerMazeMode::LegacyLanes;
		int32 GridColumns = 6;
		int32 GridRows = 6;
		float GridCellSize = 6500.0f;
		float GridDoorWidth = 2600.0f;
		float TraceStartZ = 16000.0f;
		float TraceEndZ = -32000.0f;
		int32 StartFloorNumber = 1;
		int32 FirstMobFloorNumber = 2;
		int32 LastMobFloorNumber = 3;
		int32 BossFloorNumber = 4;
		int32 DungeonMinRooms = 15;
		int32 DungeonMaxRooms = 20;
		int32 DungeonMinRoomTiles = 2;
		int32 DungeonMaxRoomTiles = 5;
		/** Max edge gap (cells) a new room may have to the cluster; bounds hall length. */
		int32 RoomMaxGapCells = 6;
		int32 StartRoomMinTiles = 3;
		int32 StartRoomMaxTiles = 4;
		float GridBranchChance = 0.35f;
		int32 GridMaxBranchCells = 3;
		float RoofSkinThickness = 12.0f;
		float StartFloorHeadroom = 2000.0f;
		int32 GeneratedDungeonKitCullDistance = 30000;
		bool bBounceCoursePlatforms = true;
		float PlatformTier1Height = 200.0f;
		float PlatformTier2Height = 400.0f;
		float ChainPlatformFootprint = 700.0f;
		float RoomPlatformFootprintMin = 550.0f;
		float RoomPlatformFootprintMax = 750.0f;
		int32 RoomPlatformDensityTiles = 8;
		float PlatformChainMaxGap = 350.0f;
		/** Seeded share of non-chain platforms spawned as cylinders instead of cubes. */
		float RoundPlatformChance = 0.45f;
		float RampWidth = 600.0f;
		float RampLength = 520.0f;
		float LavaMaxHeight = 320.0f;
		bool bTierTerrain = true;
		float TierHeight = 500.0f;
		int32 MesaInsetCells = 2;
		int32 MesaMinSpanCells = 3;
		int32 MesaRampsMin = 2;
		int32 MesaRampsMax = 4;
		float MesaTopBafflePitch = 200.0f;
		float MesaTopBaffleDiameter = 180.0f;
		float RampRollerDiameter = 90.0f;
		/** Chance a >=5x5 mesa becomes a RING deck with a center drop hole. */
		float RingMesaChance = 0.5f;
		bool bTierLifts = true;
		float LiftChance = 0.5f;
		float LiftFootprint = 600.0f;
		float LiftTravelSeconds = 3.0f;
		float LiftDwellSeconds = 2.0f;
		bool bDoorwayArches = true;
		int32 ArchSegments = 10;
		float ArchTubeDiameter = 110.0f;
		FName DefaultRoomRuleID = FName(TEXT("DefaultCombat"));
		FName StartRoomRuleID = FName(TEXT("Start"));
		FName BossRoomRuleID = FName(TEXT("Boss"));
		FVector SpawnSurfaceLocation = FVector::ZeroVector;
		FVector StartAnchorSurfaceLocation = FVector::ZeroVector;
		FVector StartPathSurfaceLocation = FVector::ZeroVector;
		FVector StartAreaCenterSurfaceLocation = FVector::ZeroVector;
		FVector BossAnchorSurfaceLocation = FVector::ZeroVector;
		FVector BossSpawnSurfaceLocation = FVector::ZeroVector;
		FVector BossAreaCenterSurfaceLocation = FVector::ZeroVector;
		TArray<FVector> RescueAnchorLocations;
		TArray<FStartGalleryWing> StartGalleryWings;
	};

	int32 ResolveGameplayLevelNumberForDifficulty(ET66Difficulty Difficulty);
	ET66TowerGameplayLevelTheme ResolveGameplayLevelTheme(int32 GameplayLevelNumber);
	ET66TowerGameplayLevelTheme ResolveGameplayLevelThemeForDifficulty(ET66Difficulty Difficulty);
	FText GetFloorDisplayName(const FFloor& Floor);
	FText GetFloorDisplayName(const FLayout& Layout, int32 FloorNumber);

	bool BuildLayout(const FT66MapPreset& Preset, FLayout& OutLayout, bool bBossRushFinaleStage = false);
	FVector GetPreferredSpawnLocation(const FLayout& Layout, float HeightOffset);
	int32 FindFloorIndexForLocation(const FLayout& Layout, const FVector& Location, float VerticalTolerance = 900.0f);
	bool TryGetFloorBounds(const FLayout& Layout, int32 FloorNumber, FVector2D& OutCenter, FVector2D& OutHalfExtents);
	bool TryGetFloorPolygon(const FLayout& Layout, int32 FloorNumber, TArray<FVector2D>& OutVertices);
	bool TryGetPolygonBandXRange(const TArray<FVector2D>& PolygonVertices, float BandMinY, float BandMaxY, float& OutMinX, float& OutMaxX);
	bool TryGetFloorHoleLocation(const FLayout& Layout, int32 FloorNumber, FVector& OutLocation);
	bool TryGetStartPlacementLocation(UWorld* World, const FLayout& Layout, float SideCells, float InwardCells, FVector& OutLocation);
	bool TryGetRandomSurfaceLocationOnFloor(UWorld* World, const FLayout& Layout, int32 FloorNumber, FRandomStream& Rng, FVector& OutLocation, float EdgePadding = 900.0f, float HolePadding = 1000.0f);
	bool TryGetRandomGameplaySurfaceLocation(UWorld* World, const FLayout& Layout, FRandomStream& Rng, FVector& OutLocation);
	bool TryGetFloorTileCenterSpawnLocation(UWorld* World, const FLayout& Layout, int32 FloorNumber, FRandomStream& Rng, FVector& OutLocation, float EdgePadding = 900.0f, float HolePadding = 1000.0f, float WallPadding = 700.0f);
	bool TryGetRoomSurfaceLocation(UWorld* World, const FLayout& Layout, const FFloor& Floor, const FRoom& Room, FRandomStream& Rng, FVector& OutLocation, float EdgePadding = 700.0f, float HolePadding = 900.0f, float WallPadding = 500.0f);
	bool TryGetObstacleTrapSpawnLocation(UWorld* World, const FLayout& Layout, int32 FloorNumber, FRandomStream& Rng, FVector& OutLocation, float FootprintRadius = 650.0f, float EdgePadding = 1400.0f, float HolePadding = 1600.0f);
	bool TryGetMazeWallSpawnLocation(UWorld* World, const FLayout& Layout, int32 FloorNumber, FRandomStream& Rng, FVector& OutLocation, FVector& OutWallNormal, float EndPadding = 500.0f);

	/** True when the 2D point sits inside (or within Padding of) any bounce platform or ramp footprint on the floor. */
	bool IsPointInsideBounceObstacle(const FFloor& Floor, const FVector& Location, float Padding = 0.0f);
	bool TryGetWallSpawnLocation(UWorld* World, const FLayout& Layout, const FVector& PlayerLocation, float MinDistance, float MaxDistance, FRandomStream& Rng, FVector& OutLocation);
	bool TryGetWallSpawnLocation(UWorld* World, const FLayout& Layout, const FVector& PlayerLocation, float MinDistance, float MaxDistance, FRandomStream& Rng, FVector& OutLocation, FVector& OutWallNormal);
	bool Spawn(UWorld* World, const FLayout& Layout, ET66Difficulty Difficulty, const FActorSpawnParameters& SpawnParams, bool& bOutCollisionReady);

	// Themed-surface parity for the Test Room: spawn the SAME visuals the live
	// maze renders (Dungeon-theme baffle tubes, themed-cube fallback, honoring
	// the t66.Tower.*Baffles CVars) over arbitrary boxes. Collision is the
	// caller's responsibility — the maze itself pairs these with hidden box
	// proxies. Returns false only when no visual could be spawned at all.
	bool SpawnThemedFloorVisual(UWorld* World, const FBox2D& Box, float SurfaceZ, const TArray<FName>& Tags);
	bool SpawnThemedCeilingVisual(UWorld* World, const FBox2D& Box, float CeilingBottomZ, const TArray<FName>& Tags);
	bool SpawnThemedWallVisual(UWorld* World, const FBox2D& WallBox, float BaseZ, float Height, const TArray<FName>& Tags);
}
