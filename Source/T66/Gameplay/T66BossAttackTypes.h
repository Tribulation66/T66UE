// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66BossAttackTypes.generated.h"

UENUM(BlueprintType)
enum class ET66BossAttackProfile : uint8
{
	Balanced UMETA(DisplayName = "Balanced"),
	Sharpshooter UMETA(DisplayName = "Sharpshooter"),
	Juggernaut UMETA(DisplayName = "Juggernaut"),
	Duelist UMETA(DisplayName = "Duelist"),
	Vendor UMETA(DisplayName = "Vendor"),
	Gambler UMETA(DisplayName = "Gambler"),
};
