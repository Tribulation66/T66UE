// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66ShelvedFeatureGate.h"

namespace
{
	constexpr bool bT66DailyDescentEnabled = false;
	constexpr bool bT66VehicleInteractablesEnabled = false;
}

bool FT66ShelvedFeatureGate::IsFeatureEnabled(const ET66ShelvedFeature Feature)
{
	switch (Feature)
	{
	case ET66ShelvedFeature::DailyDescent:
		return bT66DailyDescentEnabled;
	case ET66ShelvedFeature::VehicleInteractables:
		return bT66VehicleInteractablesEnabled;
	default:
		return false;
	}
}

bool FT66ShelvedFeatureGate::IsScreenAllowed(const ET66ScreenType ScreenType)
{
	if (ScreenType == ET66ScreenType::DailyDescent)
	{
		return IsDailyDescentEnabled();
	}

	return true;
}

bool FT66ShelvedFeatureGate::IsDailyDescentEnabled()
{
	return IsFeatureEnabled(ET66ShelvedFeature::DailyDescent);
}

bool FT66ShelvedFeatureGate::IsVehicleInteractablesEnabled()
{
	return IsFeatureEnabled(ET66ShelvedFeature::VehicleInteractables);
}

const TCHAR* FT66ShelvedFeatureGate::GetShelvedReason(const ET66ShelvedFeature Feature)
{
	switch (Feature)
	{
	case ET66ShelvedFeature::DailyDescent:
		return TEXT("Daily Descent is shelved in this build.");
	case ET66ShelvedFeature::VehicleInteractables:
		return TEXT("Vehicle interactables are shelved in this build.");
	default:
		return TEXT("Feature is shelved in this build.");
	}
}
