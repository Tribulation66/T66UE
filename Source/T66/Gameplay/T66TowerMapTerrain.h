// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"

class UWorld;
enum class ET66Difficulty : uint8;
struct FActorSpawnParameters;

namespace T66TowerMapTerrain
{
	struct FFloor
	{
		int32 FloorNumber = 0;
		bool bGameplayFloor = false;
		bool bHasDropHole = false;
		FVector Center = FVector::ZeroVector;
		float SurfaceZ = 0.0f;
		float WalkableHalfExtent = 9000.0f;
		float PolygonApothem = 9000.0f;
		float BoundsHalfExtent = 9000.0f;
		FVector HoleCenter = FVector::ZeroVector;
		FVector2D HoleHalfExtent = FVector2D::ZeroVector;
		TArray<FBox2D> MazeWallBoxes;
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
		float TraceStartZ = 16000.0f;
		float TraceEndZ = -32000.0f;
		int32 StartFloorNumber = 1;
		int32 FirstGameplayFloorNumber = 2;
		int32 LastGameplayFloorNumber = 5;
		int32 BossFloorNumber = 6;
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
	bool TryGetWallSpawnLocation(UWorld* World, const FLayout& Layout, const FVector& PlayerLocation, float MinDistance, float MaxDistance, FRandomStream& Rng, FVector& OutLocation);
	bool TryGetWallSpawnLocation(UWorld* World, const FLayout& Layout, const FVector& PlayerLocation, float MinDistance, float MaxDistance, FRandomStream& Rng, FVector& OutLocation, FVector& OutWallNormal);
	bool Spawn(UWorld* World, const FLayout& Layout, ET66Difficulty Difficulty, const FActorSpawnParameters& SpawnParams, bool& bOutCollisionReady);
}
