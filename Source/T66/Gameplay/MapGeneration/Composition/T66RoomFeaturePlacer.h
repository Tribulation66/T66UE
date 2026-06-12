// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66TowerMapTerrain.h"

namespace T66MapGeneration
{
	struct FScatterRampCandidate
	{
		FIntPoint Cell = FIntPoint(INDEX_NONE, INDEX_NONE);
		int32 Tier = 1;
	};

	struct FRoomPlacementContext
	{
		const T66TowerMapTerrain::FLayout* Layout = nullptr;
		T66TowerMapTerrain::FFloor* Floor = nullptr;
		FRandomStream* Rng = nullptr;
		TSet<int32>* OccupiedCells = nullptr;
		TArray<FScatterRampCandidate>* RampCandidates = nullptr;
		TArray<FIntPoint> NeighborDeltas;
		float CellSize = 0.0f;
		float Tier1TopZ = 0.0f;
		float Tier2TopZ = 0.0f;
		float SurfaceZ = 0.0f;

		TFunction<bool(const FIntPoint&)> IsWalkableBounceCell;
		TFunction<uint8(const FIntPoint&)> GetCellTier;
		TFunction<bool(const FIntPoint&)> IsCellInsideAnyMesa;
		TFunction<int32(const FIntPoint&)> GetGridCellIndex;
		TFunction<FVector2D(const FIntPoint&)> GetCellCenter;
		TFunction<void(
			const FIntPoint&,
			const FVector2D&,
			float,
			int32,
			float,
			bool,
			T66TowerMapTerrain::ET66BouncePlatformShape,
			uint8)> AddBouncePlatform;
		TFunction<T66TowerMapTerrain::ET66BouncePlatformShape(bool)> PickShape;

		bool IsValid() const;
	};

	void ResetFloorPlacementOutputs(T66TowerMapTerrain::FFloor& Floor);
	void AddRewardSlot(T66TowerMapTerrain::FRoom& Room, const FVector& Location);
	void AddHazardAnchor(T66TowerMapTerrain::FFloor& Floor, const FVector& Location, FName HazardID);
	bool IsMesaStructure(const T66TowerMapTerrain::FRoom& Room);
	bool PlaceSteppingStoneStructure(FRoomPlacementContext& Context, T66TowerMapTerrain::FRoom& Room);
	bool PlaceBridgeDeckStructure(FRoomPlacementContext& Context, T66TowerMapTerrain::FRoom& Room, FName HazardID);
	int32 PlaceScatterStoneStructure(FRoomPlacementContext& Context, T66TowerMapTerrain::FRoom& Room, int32 TargetCount);
}
