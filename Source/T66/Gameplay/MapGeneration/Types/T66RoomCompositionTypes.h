// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace T66MapGeneration
{
	enum class ET66RoomFeatureCategory : uint8
	{
		Structure,
		Hazard,
		Reward,
	};

	struct FRoomCompositionProfile
	{
		FName ProfileID = NAME_None;
		int32 MinStructures = 0;
		int32 MaxStructures = 0;
		int32 MinHazards = 0;
		int32 MaxHazards = 0;
		int32 MinRewards = 0;
		int32 MaxRewards = 0;
		float MinCombatOpenAreaRatio = 0.65f;
		float MaxStructureDensity = 0.35f;
		bool bRequiresLavaSurvivalPath = true;
		bool bAllowElevatedStructures = true;
		bool bAllowMovingStructures = true;
	};

	struct FStructureDefinition
	{
		FName StructureID = NAME_None;
		FName FamilyID = NAME_None;
		FVector2D FootprintTiles = FVector2D::ZeroVector;
		int32 DensityCost = 1;
		bool bBlocksMovement = false;
		bool bSupportsLavaSurvival = false;
		bool bCanHoldReward = false;
		bool bCanCreateHazardAnchor = false;
	};

	struct FHazardDefinition
	{
		FName HazardID = NAME_None;
		FName FamilyID = NAME_None;
		FVector2D RequiredClearanceTiles = FVector2D::ZeroVector;
		float PressureCost = 1.0f;
		bool bCanAttachToStructure = true;
		bool bCanGuardReward = true;
	};

	inline const FName ProfileCombatPlayhouse(TEXT("CombatPlayhouse"));
	inline const FName ProfileFlatCombat(TEXT("FlatCombat"));

	inline const FName StructureCentralMesa(TEXT("Structure.CentralMesa"));
	inline const FName StructureRingMesa(TEXT("Structure.RingMesa"));
	inline const FName StructureSteppingStones(TEXT("Structure.SteppingStones"));
	inline const FName StructureBridgeDeck(TEXT("Structure.BridgeDeck"));
	inline const FName StructureScatterStones(TEXT("Structure.ScatterStones"));

	inline const FName HazardSweeperArm(TEXT("ObstacleSweeperArm"));
	inline const FName HazardCeilingHammer(TEXT("ObstacleCeilingHammer"));
}
