// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66UITypes.h"

enum class ET66ShelvedFeature : uint8
{
	DailyDescent,
	VehicleInteractables
};

struct T66_API FT66ShelvedFeatureGate
{
	static bool IsFeatureEnabled(ET66ShelvedFeature Feature);
	static bool IsScreenAllowed(ET66ScreenType ScreenType);
	static bool IsDailyDescentEnabled();
	static bool IsVehicleInteractablesEnabled();
	static const TCHAR* GetShelvedReason(ET66ShelvedFeature Feature);
};
