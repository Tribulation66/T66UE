// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/MapGeneration/Composition/T66RoomComposer.h"

#include "Gameplay/MapGeneration/Libraries/T66StructureLibrary.h"

namespace T66MapGeneration
{
	namespace
	{
		void SetRoomStructures(T66TowerMapTerrain::FRoom& Room, const TArray<FName>& StructureIDs)
		{
			Room.StructureIDs.Reset();
			for (const FName StructureID : StructureIDs)
			{
				if (!StructureID.IsNone() && IsStructureKnown(StructureID))
				{
					Room.StructureIDs.AddUnique(StructureID);
				}
			}
		}

		void SetSingleStructure(T66TowerMapTerrain::FRoom& Room, const FName StructureID)
		{
			SetRoomStructures(Room, { StructureID });
		}

		void AddRoomStructure(T66TowerMapTerrain::FRoom& Room, const FName StructureID)
		{
			if (!StructureID.IsNone() && IsStructureKnown(StructureID))
			{
				Room.StructureIDs.AddUnique(StructureID);
			}
		}

		bool ProfileAllowsAnotherStructure(const FRoomCompositionProfile& Profile, const T66TowerMapTerrain::FRoom& Room)
		{
			return Room.StructureIDs.Num() < FMath::Max(0, Profile.MaxStructures);
		}

		bool IsPrimaryStructureEligibleForScatterSupport(const FName StructureID)
		{
			return StructureID == StructureSteppingStones || StructureID == StructureBridgeDeck;
		}
	}

	void ResetRoomComposition(T66TowerMapTerrain::FRoom& Room)
	{
		Room.CompositionProfileID = NAME_None;
		Room.StructureIDs.Reset();
		Room.RewardSlots.Reset();
	}

	void DowngradeRoomToFlatCombat(T66TowerMapTerrain::FRoom& Room)
	{
		Room.CompositionProfileID = ProfileFlatCombat;
		SetSingleStructure(Room, StructureScatterStones);
	}

	void ComposeRoomStructures(
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FRoom& Room,
		FRandomStream& Rng)
	{
		ResetRoomComposition(Room);
		if (Room.bContainsArrival || Room.bContainsExit)
		{
			return;
		}

		const int32 StructureAvailX = (Room.MaxCellExclusive.X - Room.MinCell.X) - (Layout.MesaInsetCells * 2);
		const int32 StructureAvailY = (Room.MaxCellExclusive.Y - Room.MinCell.Y) - (Layout.MesaInsetCells * 2);
		const bool bMesaCapable = StructureAvailX >= Layout.MesaMinSpanCells && StructureAvailY >= Layout.MesaMinSpanCells;
		const bool bRingCapable = FMath::Min(StructureAvailX, 8) >= 5 && FMath::Min(StructureAvailY, 8) >= 5;
		const bool bCourtCapable = Room.WidthTiles >= 4 && Room.HeightTiles >= 4;

		Room.CompositionProfileID = bMesaCapable || bCourtCapable ? ProfileCombatPlayhouse : ProfileFlatCombat;
		const FRoomCompositionProfile& Profile = GetCompositionProfile(Room.CompositionProfileID);
		if (!ProfileAllowsAnotherStructure(Profile, Room))
		{
			return;
		}

		const float Roll = Rng.FRand();
		FName PrimaryStructure = NAME_None;
		if (bMesaCapable)
		{
			if (Roll < 0.35f)
			{
				PrimaryStructure = StructureCentralMesa;
			}
			else if (Roll < 0.60f)
			{
				PrimaryStructure = bRingCapable ? StructureRingMesa : StructureCentralMesa;
			}
			else if (Roll < 0.75f)
			{
				PrimaryStructure = StructureSteppingStones;
			}
			else if (Roll < 0.90f)
			{
				PrimaryStructure = StructureBridgeDeck;
			}
			else
			{
				PrimaryStructure = StructureScatterStones;
			}
		}
		else if (bCourtCapable)
		{
			PrimaryStructure = Roll < 0.35f
				? StructureSteppingStones
				: (Roll < 0.65f ? StructureBridgeDeck : StructureScatterStones);
		}
		else
		{
			DowngradeRoomToFlatCombat(Room);
		}

		if (!PrimaryStructure.IsNone())
		{
			SetSingleStructure(Room, PrimaryStructure);
			if (ProfileAllowsAnotherStructure(Profile, Room)
				&& IsPrimaryStructureEligibleForScatterSupport(PrimaryStructure)
				&& Rng.FRand() < 0.35f)
			{
				AddRoomStructure(Room, StructureScatterStones);
			}
		}

		if (Room.StructureIDs.Num() < Profile.MinStructures)
		{
			DowngradeRoomToFlatCombat(Room);
		}
	}

	bool RoomHasStructure(const T66TowerMapTerrain::FRoom& Room, const FName StructureID)
	{
		return Room.StructureIDs.Contains(StructureID);
	}
}
