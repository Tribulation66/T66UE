// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/MapGeneration/Validation/T66LavaSurvivalGraph.h"

#include "Gameplay/MapGeneration/Libraries/T66StructureLibrary.h"

namespace T66MapGeneration
{
	bool RoomHasLavaSurvivalSupport(const T66TowerMapTerrain::FFloor& Floor, const T66TowerMapTerrain::FRoom& Room)
	{
		for (const FName StructureID : Room.StructureIDs)
		{
			const FStructureDefinition* Structure = FindStructureDefinition(StructureID);
			if (Structure && Structure->bSupportsLavaSurvival)
			{
				return true;
			}
		}

		for (const FIntPoint& ChainCell : Floor.SafeChainCells)
		{
			if (ChainCell.X >= Room.MinCell.X && ChainCell.X < Room.MaxCellExclusive.X
				&& ChainCell.Y >= Room.MinCell.Y && ChainCell.Y < Room.MaxCellExclusive.Y)
			{
				return true;
			}
		}

		return false;
	}
}
