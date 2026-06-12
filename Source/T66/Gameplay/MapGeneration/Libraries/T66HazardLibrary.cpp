// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/MapGeneration/Libraries/T66HazardLibrary.h"

namespace T66MapGeneration
{
	const FHazardDefinition* FindHazardDefinition(const FName HazardID)
	{
		static const FHazardDefinition Sweeper = {
			HazardSweeperArm,
			FName(TEXT("Sweeper")),
			FVector2D(3.0f, 3.0f),
			1.0f,
			true,
			true
		};
		static const FHazardDefinition CeilingHammer = {
			HazardCeilingHammer,
			FName(TEXT("Hammer")),
			FVector2D(3.0f, 2.0f),
			1.0f,
			true,
			true
		};

		if (HazardID == HazardSweeperArm) return &Sweeper;
		if (HazardID == HazardCeilingHammer) return &CeilingHammer;
		return nullptr;
	}

	bool IsHazardKnown(const FName HazardID)
	{
		return FindHazardDefinition(HazardID) != nullptr;
	}
}
