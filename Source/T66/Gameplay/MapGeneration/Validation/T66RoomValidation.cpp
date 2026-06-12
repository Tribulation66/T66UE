// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/MapGeneration/Validation/T66RoomValidation.h"

#include "Gameplay/MapGeneration/Libraries/T66StructureLibrary.h"
#include "Gameplay/MapGeneration/Validation/T66LavaSurvivalGraph.h"

namespace T66MapGeneration
{
	namespace
	{
		bool AnchorInsideRoom(const T66TowerMapTerrain::FRoom& Room, const FVector& Location)
		{
			return Room.Bounds.IsInside(FVector2D(Location.X, Location.Y));
		}
	}

	FRoomValidationResult ValidateRoomComposition(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const T66TowerMapTerrain::FRoom& Room)
	{
		FRoomValidationResult Result;
		if (Room.bContainsArrival || Room.bContainsExit || Room.CompositionProfileID.IsNone())
		{
			return Result;
		}

		const FRoomCompositionProfile& Profile = GetCompositionProfile(Room.CompositionProfileID);
		Result.StructureCount = Room.StructureIDs.Num();
		Result.RewardSlotCount = Room.RewardSlots.Num();

		int32 StructureDensityCost = 0;
		for (const FName StructureID : Room.StructureIDs)
		{
			const FStructureDefinition* Structure = FindStructureDefinition(StructureID);
			if (!Structure)
			{
				Result.bValid = false;
				Result.FailureReason = FString::Printf(TEXT("UnknownStructure:%s"), *StructureID.ToString());
				return Result;
			}
			StructureDensityCost += FMath::Max(0, Structure->DensityCost);
		}

		for (const FVector& Anchor : Floor.HazardAnchors)
		{
			if (AnchorInsideRoom(Room, Anchor))
			{
				++Result.HazardAnchorCount;
			}
		}

		const int32 RoomAreaTiles = FMath::Max(1, Room.WidthTiles * Room.HeightTiles);
		Result.StructureDensity = static_cast<float>(StructureDensityCost) / static_cast<float>(RoomAreaTiles);
		Result.EstimatedCombatOpenAreaRatio = FMath::Clamp(1.0f - Result.StructureDensity, 0.0f, 1.0f);

		if (Result.StructureCount < Profile.MinStructures || Result.StructureCount > Profile.MaxStructures)
		{
			Result.bValid = false;
			Result.FailureReason = TEXT("StructureBudget");
		}
		else if (Result.RewardSlotCount < Profile.MinRewards || Result.RewardSlotCount > Profile.MaxRewards)
		{
			Result.bValid = false;
			Result.FailureReason = TEXT("RewardBudget");
		}
		else if (Result.HazardAnchorCount > Profile.MaxHazards)
		{
			Result.bValid = false;
			Result.FailureReason = TEXT("HazardBudget");
		}
		else if (Result.StructureDensity > Profile.MaxStructureDensity)
		{
			Result.bValid = false;
			Result.FailureReason = TEXT("StructureDensity");
		}
		else if (Result.EstimatedCombatOpenAreaRatio < Profile.MinCombatOpenAreaRatio)
		{
			Result.bValid = false;
			Result.FailureReason = TEXT("CombatOpenArea");
		}
		else if (Profile.bRequiresLavaSurvivalPath && Layout.bBounceCoursePlatforms && !RoomHasLavaSurvivalSupport(Floor, Room))
		{
			Result.bValid = false;
			Result.FailureReason = TEXT("LavaSurvival");
		}

		return Result;
	}
}
