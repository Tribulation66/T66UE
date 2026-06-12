// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66TowerMapTerrain.h"

namespace T66MapGeneration
{
	struct FRoomValidationResult
	{
		bool bValid = true;
		int32 StructureCount = 0;
		int32 RewardSlotCount = 0;
		int32 HazardAnchorCount = 0;
		float StructureDensity = 0.0f;
		float EstimatedCombatOpenAreaRatio = 1.0f;
		FString FailureReason;
	};

	FRoomValidationResult ValidateRoomComposition(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const T66TowerMapTerrain::FRoom& Room);
}
