// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66RunTypes.generated.h"

UENUM(BlueprintType)
enum class ET66RunMode : uint8
{
	Regular UMETA(DisplayName = "Regular"),
	DailyClimb UMETA(DisplayName = "Daily Climb"),
	Offline UMETA(DisplayName = "Offline")
};

UENUM(BlueprintType)
enum class ET66RunCategory : uint8
{
	Tower UMETA(DisplayName = "Tower"),
	Lab UMETA(DisplayName = "Lab"),
	Tutorial UMETA(DisplayName = "Tutorial"),
	TestRoom UMETA(DisplayName = "Test Room")
};
