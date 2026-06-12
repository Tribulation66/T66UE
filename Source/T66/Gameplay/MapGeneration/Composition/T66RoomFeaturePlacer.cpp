// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/MapGeneration/Composition/T66RoomFeaturePlacer.h"

#include "Gameplay/MapGeneration/Libraries/T66HazardLibrary.h"
#include "Gameplay/MapGeneration/Libraries/T66StructureLibrary.h"

namespace T66MapGeneration
{
	namespace
	{
		bool IsNearCell(const FIntPoint& A, const FIntPoint& B)
		{
			return FMath::Abs(A.X - B.X) <= 1 && FMath::Abs(A.Y - B.Y) <= 1;
		}

		bool IsCourseCellFree(const FRoomPlacementContext& Context, const T66TowerMapTerrain::FRoom& Room, const FIntPoint& Cell)
		{
			if (!Context.IsValid()
				|| !Context.IsWalkableBounceCell(Cell)
				|| Context.GetCellTier(Cell) != 0
				|| Context.IsCellInsideAnyMesa(Cell)
				|| Context.OccupiedCells->Contains(Context.GetGridCellIndex(Cell)))
			{
				return false;
			}

			const T66TowerMapTerrain::FFloor& Floor = *Context.Floor;
			return !IsNearCell(Cell, Floor.ExitCell) && !IsNearCell(Cell, Floor.ArrivalCell);
		}

		void AddPlatformAndOccupy(
			FRoomPlacementContext& Context,
			const FIntPoint& Cell,
			const FVector2D& Center,
			const float Footprint,
			const int32 Tier,
			const float TopZ,
			const bool bSafeChain,
			const T66TowerMapTerrain::ET66BouncePlatformShape Shape,
			const uint8 YawSteps)
		{
			Context.AddBouncePlatform(Cell, Center, Footprint, Tier, TopZ, bSafeChain, Shape, YawSteps);
			Context.OccupiedCells->Add(Context.GetGridCellIndex(Cell));
		}
	}

	bool FRoomPlacementContext::IsValid() const
	{
		return Layout
			&& Floor
			&& Rng
			&& OccupiedCells
			&& RampCandidates
			&& IsWalkableBounceCell
			&& GetCellTier
			&& IsCellInsideAnyMesa
			&& GetGridCellIndex
			&& GetCellCenter
			&& AddBouncePlatform
			&& PickShape;
	}

	void ResetFloorPlacementOutputs(T66TowerMapTerrain::FFloor& Floor)
	{
		Floor.HazardAnchors.Reset();
		Floor.HazardAnchorTypes.Reset();
		Floor.BouncePadSpots.Reset();
	}

	void AddRewardSlot(T66TowerMapTerrain::FRoom& Room, const FVector& Location)
	{
		Room.RewardSlots.Add(Location);
	}

	void AddHazardAnchor(T66TowerMapTerrain::FFloor& Floor, const FVector& Location, const FName HazardID)
	{
		Floor.HazardAnchors.Add(Location);
		Floor.HazardAnchorTypes.Add(IsHazardKnown(HazardID) ? HazardID : NAME_None);
	}

	bool IsMesaStructure(const T66TowerMapTerrain::FRoom& Room)
	{
		return Room.StructureIDs.Contains(StructureCentralMesa) || Room.StructureIDs.Contains(StructureRingMesa);
	}

	bool PlaceSteppingStoneStructure(FRoomPlacementContext& Context, T66TowerMapTerrain::FRoom& Room)
	{
		if (!Context.IsValid() || !Room.StructureIDs.Contains(StructureSteppingStones))
		{
			return false;
		}

		FIntPoint Cursor(Room.MinCell.X + 1, Room.MinCell.Y + 1);
		const FIntPoint Target(Room.MaxCellExclusive.X - 2, Room.MaxCellExclusive.Y - 2);
		TArray<FIntPoint> CourtCells;
		CourtCells.Add(Cursor);

		bool bStepX = true;
		int32 Guard = 64;
		while (Cursor != Target && Guard-- > 0)
		{
			if (bStepX && Cursor.X != Target.X)
			{
				Cursor.X += (Target.X > Cursor.X) ? 1 : -1;
			}
			else if (Cursor.Y != Target.Y)
			{
				Cursor.Y += (Target.Y > Cursor.Y) ? 1 : -1;
			}
			else if (Cursor.X != Target.X)
			{
				Cursor.X += (Target.X > Cursor.X) ? 1 : -1;
			}
			bStepX = !bStepX;
			CourtCells.Add(Cursor);
		}

		int32 CourtPlaced = 0;
		bool bPreviousPlaced = false;
		for (int32 CourtIndex = 0; CourtIndex < CourtCells.Num(); ++CourtIndex)
		{
			const FIntPoint& CourtCell = CourtCells[CourtIndex];
			if (!IsCourseCellFree(Context, Room, CourtCell))
			{
				bPreviousPlaced = false;
				continue;
			}

			const bool bLast = CourtIndex == CourtCells.Num() - 1;
			const FVector2D Center = Context.GetCellCenter(CourtCell);
			const uint8 YawSteps = static_cast<uint8>(Context.Rng->RandRange(0, 3));
			if (bLast && bPreviousPlaced)
			{
				AddPlatformAndOccupy(
					Context,
					CourtCell,
					Center,
					720.0f,
					2,
					Context.Tier2TopZ,
					false,
					Context.PickShape(true),
					YawSteps);
				AddRewardSlot(Room, FVector(Center.X, Center.Y, Context.Tier2TopZ));
			}
			else
			{
				const float Footprint = (CourtIndex % 2 == 0) ? 660.0f : 580.0f;
				AddPlatformAndOccupy(
					Context,
					CourtCell,
					Center,
					Footprint,
					1,
					Context.Tier1TopZ,
					false,
					Context.PickShape(false),
					YawSteps);
				if (bLast)
				{
					AddRewardSlot(Room, FVector(Center.X, Center.Y, Context.Tier1TopZ));
				}
			}

			bPreviousPlaced = true;
			++CourtPlaced;
		}

		const bool bComposed = CourtPlaced >= 2;
		if (bComposed && Room.RewardSlots.Num() <= 0)
		{
			AddRewardSlot(Room, FVector(Room.WorldCenter.X, Room.WorldCenter.Y, Context.SurfaceZ));
		}
		return bComposed;
	}

	bool PlaceBridgeDeckStructure(FRoomPlacementContext& Context, T66TowerMapTerrain::FRoom& Room, const FName HazardID)
	{
		if (!Context.IsValid() || !Room.StructureIDs.Contains(StructureBridgeDeck))
		{
			return false;
		}

		const bool bAlongX = Room.WidthTiles >= Room.HeightTiles;
		const int32 FixedCoord = bAlongX
			? (Room.MinCell.Y + Room.MaxCellExclusive.Y) / 2
			: (Room.MinCell.X + Room.MaxCellExclusive.X) / 2;
		const int32 SpanFrom = (bAlongX ? Room.MinCell.X : Room.MinCell.Y) + 1;
		const int32 SpanToInclusive = (bAlongX ? Room.MaxCellExclusive.X : Room.MaxCellExclusive.Y) - 2;

		TArray<FIntPoint> BridgeCells;
		for (int32 Along = SpanFrom; Along <= SpanToInclusive; ++Along)
		{
			const FIntPoint BridgeCell = bAlongX ? FIntPoint(Along, FixedCoord) : FIntPoint(FixedCoord, Along);
			if (IsCourseCellFree(Context, Room, BridgeCell))
			{
				BridgeCells.Add(BridgeCell);
			}
		}

		if (BridgeCells.Num() < 3)
		{
			return false;
		}

		for (const FIntPoint& BridgeCell : BridgeCells)
		{
			AddPlatformAndOccupy(
				Context,
				BridgeCell,
				Context.GetCellCenter(BridgeCell),
				960.0f,
				2,
				Context.Tier2TopZ,
				false,
				T66TowerMapTerrain::ET66BouncePlatformShape::Square,
				0);
		}

		const FIntPoint AlongSign = bAlongX ? FIntPoint(1, 0) : FIntPoint(0, 1);
		TArray<FIntPoint> MountCells;
		MountCells.Add(BridgeCells[0] - AlongSign);
		MountCells.Add(BridgeCells.Last() + AlongSign);
		for (const FIntPoint& MountCell : MountCells)
		{
			if (IsCourseCellFree(Context, Room, MountCell))
			{
				AddPlatformAndOccupy(
					Context,
					MountCell,
					Context.GetCellCenter(MountCell),
					640.0f,
					1,
					Context.Tier1TopZ,
					false,
					Context.PickShape(false),
					static_cast<uint8>(Context.Rng->RandRange(0, 3)));
			}
		}

		const FIntPoint& MidCell = BridgeCells[BridgeCells.Num() / 2];
		const FVector2D MidCenter = Context.GetCellCenter(MidCell);
		AddRewardSlot(Room, FVector(MidCenter.X, MidCenter.Y, Context.Tier2TopZ));
		AddHazardAnchor(*Context.Floor, FVector(MidCenter.X, MidCenter.Y, Context.SurfaceZ), HazardID);
		return true;
	}

	int32 PlaceScatterStoneStructure(FRoomPlacementContext& Context, T66TowerMapTerrain::FRoom& Room, const int32 TargetCount)
	{
		if (!Context.IsValid() || TargetCount <= 0)
		{
			return 0;
		}

		int32 Placed = 0;
		for (int32 Attempt = 0; Attempt < TargetCount * 6 && Placed < TargetCount; ++Attempt)
		{
			const FIntPoint Candidate(
				Context.Rng->RandRange(Room.MinCell.X + 1, Room.MaxCellExclusive.X - 2),
				Context.Rng->RandRange(Room.MinCell.Y + 1, Room.MaxCellExclusive.Y - 2));
			if (!Context.IsWalkableBounceCell(Candidate))
			{
				continue;
			}

			const int32 CandidateIndex = Context.GetGridCellIndex(Candidate);
			if (Context.OccupiedCells->Contains(CandidateIndex)
				|| IsNearCell(Candidate, Context.Floor->ExitCell)
				|| IsNearCell(Candidate, Context.Floor->ArrivalCell))
			{
				continue;
			}

			const int32 Tier = (Context.Rng->FRand() < 0.6f) ? 1 : 2;
			const float Footprint = Context.Rng->FRandRange(Context.Layout->RoomPlatformFootprintMin, Context.Layout->RoomPlatformFootprintMax);
			const float JitterRange = FMath::Max((Context.CellSize - Footprint) * 0.5f - 60.0f, 0.0f);
			const FVector2D Center = Context.GetCellCenter(Candidate)
				+ FVector2D(Context.Rng->FRandRange(-JitterRange, JitterRange), Context.Rng->FRandRange(-JitterRange, JitterRange));

			if (Tier == 2)
			{
				FIntPoint BuddyCell(INDEX_NONE, INDEX_NONE);
				for (const FIntPoint& Delta : Context.NeighborDeltas)
				{
					const FIntPoint Buddy = Candidate + Delta;
					const int32 BuddyIndex = Context.IsWalkableBounceCell(Buddy)
						? Context.GetGridCellIndex(Buddy)
						: INDEX_NONE;
					if (BuddyIndex != INDEX_NONE
						&& !Context.OccupiedCells->Contains(BuddyIndex)
						&& !IsNearCell(Buddy, Context.Floor->ExitCell)
						&& !IsNearCell(Buddy, Context.Floor->ArrivalCell))
					{
						BuddyCell = Buddy;
						break;
					}
				}

				if (BuddyCell.X != INDEX_NONE)
				{
					AddPlatformAndOccupy(
						Context,
						Candidate,
						Center,
						Footprint,
						2,
						Context.Tier2TopZ,
						false,
						Context.PickShape(false),
						static_cast<uint8>(Context.Rng->RandRange(0, 3)));
					const FVector2D TowardTier2(
						static_cast<float>(Candidate.X - BuddyCell.X),
						static_cast<float>(Candidate.Y - BuddyCell.Y));
					const FVector2D BuddyCenter = Context.GetCellCenter(BuddyCell) + (TowardTier2 * 150.0f);
					AddPlatformAndOccupy(
						Context,
						BuddyCell,
						BuddyCenter,
						600.0f,
						1,
						Context.Tier1TopZ,
						false,
						Context.PickShape(false),
						static_cast<uint8>(Context.Rng->RandRange(0, 3)));
					++Placed;
					continue;
				}
			}

			const T66TowerMapTerrain::ET66BouncePlatformShape StoneShape = Context.PickShape(false);
			AddPlatformAndOccupy(
				Context,
				Candidate,
				Center,
				Footprint,
				1,
				Context.Tier1TopZ,
				false,
				StoneShape,
				static_cast<uint8>(Context.Rng->RandRange(0, 3)));
			if (StoneShape == T66TowerMapTerrain::ET66BouncePlatformShape::Square)
			{
				Context.RampCandidates->Add({ Candidate, 1 });
			}
			++Placed;
		}

		return Placed;
	}
}
