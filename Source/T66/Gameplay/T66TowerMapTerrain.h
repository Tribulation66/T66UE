// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"

class UWorld;
enum class ET66Difficulty : uint8;
struct FActorSpawnParameters;

namespace T66TowerMapTerrain
{
	enum class ET66TowerFloorRole : uint8
	{
		Start,
		Gameplay,
		Boss,
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

	struct FFloor
	{
		int32 FloorNumber = 0;
		ET66TowerFloorRole FloorRole = ET66TowerFloorRole::Gameplay;
		int32 GameplayLevelNumber = INDEX_NONE;
		ET66TowerGameplayLevelTheme Theme = ET66TowerGameplayLevelTheme::Dungeon;
		bool bGameplayFloor = false;
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
		TArray<FBox2D> MazeWallBoxes;
		TArray<FBox2D> TrapEligibleWallBoxes;
		TArray<FVector> CachedWalkableSpawnSlots;
		TArray<FVector> CachedMainPathSpawnSlots;
		TArray<FVector> CachedOptionalSpawnSlots;
		TArray<FVector> CachedContentSpawnSlots;
		FName FloorTag = NAME_None;
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
		int32 FirstGameplayFloorNumber = 2;
		int32 LastGameplayFloorNumber = 6;
		int32 BossFloorNumber = 7;
		FVector SpawnSurfaceLocation = FVector::ZeroVector;
		FVector StartAnchorSurfaceLocation = FVector::ZeroVector;
		FVector StartPathSurfaceLocation = FVector::ZeroVector;
		FVector StartAreaCenterSurfaceLocation = FVector::ZeroVector;
		FVector BossAnchorSurfaceLocation = FVector::ZeroVector;
		FVector BossSpawnSurfaceLocation = FVector::ZeroVector;
		FVector BossBeaconSurfaceLocation = FVector::ZeroVector;
		FVector BossAreaCenterSurfaceLocation = FVector::ZeroVector;
		TArray<FVector> RescueAnchorLocations;
	};

	int32 ResolveGameplayLevelNumberForDifficulty(ET66Difficulty Difficulty);
	ET66TowerGameplayLevelTheme ResolveGameplayLevelTheme(int32 GameplayLevelNumber);
	FText GetFloorDisplayName(const FFloor& Floor);
	FText GetFloorDisplayName(const FLayout& Layout, int32 FloorNumber);

	bool BuildLayout(const FT66MapPreset& Preset, FLayout& OutLayout);
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
	bool TryGetMazeWallSpawnLocation(UWorld* World, const FLayout& Layout, int32 FloorNumber, FRandomStream& Rng, FVector& OutLocation, FVector& OutWallNormal, float EndPadding = 500.0f);
	bool TryGetWallSpawnLocation(UWorld* World, const FLayout& Layout, const FVector& PlayerLocation, float MinDistance, float MaxDistance, FRandomStream& Rng, FVector& OutLocation);
	bool TryGetWallSpawnLocation(UWorld* World, const FLayout& Layout, const FVector& PlayerLocation, float MinDistance, float MaxDistance, FRandomStream& Rng, FVector& OutLocation, FVector& OutWallNormal);
	bool Spawn(UWorld* World, const FLayout& Layout, ET66Difficulty Difficulty, const FActorSpawnParameters& SpawnParams, bool& bOutCollisionReady);
}
