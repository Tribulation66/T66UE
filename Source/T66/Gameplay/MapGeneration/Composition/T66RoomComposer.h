// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66TowerMapTerrain.h"

namespace T66MapGeneration
{
	void ResetRoomComposition(T66TowerMapTerrain::FRoom& Room);
	void DowngradeRoomToFlatCombat(T66TowerMapTerrain::FRoom& Room);
	void ComposeRoomStructures(const T66TowerMapTerrain::FLayout& Layout, T66TowerMapTerrain::FRoom& Room, FRandomStream& Rng);
	bool RoomHasStructure(const T66TowerMapTerrain::FRoom& Room, FName StructureID);
}
