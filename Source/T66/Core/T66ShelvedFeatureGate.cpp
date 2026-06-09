// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66ShelvedFeatureGate.h"

namespace
{
	constexpr bool bT66DailyDescentEnabled = false;
	constexpr bool bT66VehicleInteractablesEnabled = false;
	constexpr bool bT66PetsEnabled = false;
	constexpr bool bT66MobLootEnabled = false;
}

bool FT66ShelvedFeatureGate::IsFeatureEnabled(const ET66ShelvedFeature Feature)
{
	switch (Feature)
	{
	case ET66ShelvedFeature::DailyDescent:
		return bT66DailyDescentEnabled;
	case ET66ShelvedFeature::VehicleInteractables:
		return bT66VehicleInteractablesEnabled;
	case ET66ShelvedFeature::Pets:
		return bT66PetsEnabled;
	case ET66ShelvedFeature::MobLoot:
		return bT66MobLootEnabled;
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
	if (ScreenType == ET66ScreenType::PetSelection)
	{
		return IsPetsEnabled();
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

bool FT66ShelvedFeatureGate::IsPetsEnabled()
{
	return IsFeatureEnabled(ET66ShelvedFeature::Pets);
}

bool FT66ShelvedFeatureGate::IsMobLootEnabled()
{
	return IsFeatureEnabled(ET66ShelvedFeature::MobLoot);
}

const TCHAR* FT66ShelvedFeatureGate::GetShelvedReason(const ET66ShelvedFeature Feature)
{
	switch (Feature)
	{
	case ET66ShelvedFeature::DailyDescent:
		return TEXT("Daily Descent is shelved in this build.");
	case ET66ShelvedFeature::VehicleInteractables:
		return TEXT("Vehicle interactables are shelved in this build.");
	case ET66ShelvedFeature::Pets:
		return TEXT("Pets are shelved in this build.");
	case ET66ShelvedFeature::MobLoot:
		return TEXT("Mob Loot is shelved in this build.");
	default:
		return TEXT("Feature is shelved in this build.");
	}
}
