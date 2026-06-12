// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/MapGeneration/Types/T66RoomCompositionTypes.h"

namespace T66MapGeneration
{
	const FRoomCompositionProfile& GetCompositionProfile(FName ProfileID);
	const FStructureDefinition* FindStructureDefinition(FName StructureID);
	bool IsStructureKnown(FName StructureID);
}
