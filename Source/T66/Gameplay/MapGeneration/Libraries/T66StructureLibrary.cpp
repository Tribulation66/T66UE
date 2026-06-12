// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/MapGeneration/Libraries/T66StructureLibrary.h"

namespace T66MapGeneration
{
	const FRoomCompositionProfile& GetCompositionProfile(const FName ProfileID)
	{
		static const FRoomCompositionProfile CombatPlayhouse = []()
		{
			FRoomCompositionProfile Profile;
			Profile.ProfileID = ProfileCombatPlayhouse;
			Profile.MinStructures = 1;
			Profile.MaxStructures = 2;
			Profile.MinHazards = 0;
			Profile.MaxHazards = 2;
			Profile.MinRewards = 1;
			Profile.MaxRewards = 2;
			Profile.MinCombatOpenAreaRatio = 0.60f;
			Profile.MaxStructureDensity = 0.35f;
			Profile.bRequiresLavaSurvivalPath = true;
			Profile.bAllowElevatedStructures = true;
			Profile.bAllowMovingStructures = true;
			return Profile;
		}();

		static const FRoomCompositionProfile FlatCombat = []()
		{
			FRoomCompositionProfile Profile;
			Profile.ProfileID = ProfileFlatCombat;
			Profile.MinStructures = 1;
			Profile.MaxStructures = 1;
			Profile.MinHazards = 0;
			Profile.MaxHazards = 1;
			Profile.MinRewards = 1;
			Profile.MaxRewards = 1;
			Profile.MinCombatOpenAreaRatio = 0.70f;
			Profile.MaxStructureDensity = 0.20f;
			Profile.bRequiresLavaSurvivalPath = false;
			Profile.bAllowElevatedStructures = true;
			Profile.bAllowMovingStructures = false;
			return Profile;
		}();

		return ProfileID == ProfileFlatCombat ? FlatCombat : CombatPlayhouse;
	}

	const FStructureDefinition* FindStructureDefinition(const FName StructureID)
	{
		static const FStructureDefinition CentralMesa = {
			StructureCentralMesa,
			FName(TEXT("Mesa")),
			FVector2D(5.0f, 5.0f),
			4,
			true,
			true,
			true,
			true
		};
		static const FStructureDefinition RingMesa = {
			StructureRingMesa,
			FName(TEXT("Mesa")),
			FVector2D(6.0f, 6.0f),
			4,
			true,
			true,
			true,
			false
		};
		static const FStructureDefinition SteppingStones = {
			StructureSteppingStones,
			FName(TEXT("StonePath")),
			FVector2D(4.0f, 4.0f),
			2,
			false,
			true,
			true,
			false
		};
		static const FStructureDefinition BridgeDeck = {
			StructureBridgeDeck,
			FName(TEXT("Bridge")),
			FVector2D(4.0f, 3.0f),
			3,
			true,
			true,
			true,
			true
		};
		static const FStructureDefinition ScatterStones = {
			StructureScatterStones,
			FName(TEXT("Scatter")),
			FVector2D(3.0f, 3.0f),
			1,
			false,
			false,
			true,
			false
		};

		if (StructureID == StructureCentralMesa) return &CentralMesa;
		if (StructureID == StructureRingMesa) return &RingMesa;
		if (StructureID == StructureSteppingStones) return &SteppingStones;
		if (StructureID == StructureBridgeDeck) return &BridgeDeck;
		if (StructureID == StructureScatterStones) return &ScatterStones;
		return nullptr;
	}

	bool IsStructureKnown(const FName StructureID)
	{
		return FindStructureDefinition(StructureID) != nullptr;
	}
}
