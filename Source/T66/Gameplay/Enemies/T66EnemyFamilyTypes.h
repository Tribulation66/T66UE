// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66EnemyFamilyTypes.generated.h"

UENUM(BlueprintType)
enum class ET66EnemyFamily : uint8
{
	Melee UMETA(DisplayName = "Melee"),
	Flying UMETA(DisplayName = "Flying"),
	Ranged UMETA(DisplayName = "Ranged"),
	Rush UMETA(DisplayName = "Rush"),
	Special UMETA(DisplayName = "Special")
};
