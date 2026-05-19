// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PerActorLightDirection.h"

UT66PerActorLightDirection::UT66PerActorLightDirection()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UT66PerActorLightDirection::SetLightDirectionOverride(FVector InDirection)
{
	if (InDirection.IsNearlyZero())
	{
		ClearLightDirectionOverride();
		return;
	}

	LightDirectionOverride = InDirection.GetSafeNormal();
	bUseLightDirectionOverride = true;
}

void UT66PerActorLightDirection::ClearLightDirectionOverride()
{
	LightDirectionOverride = FVector::ZeroVector;
	bUseLightDirectionOverride = false;
}

bool UT66PerActorLightDirection::GetEffectiveLightDirection(FVector& OutDirection) const
{
	if (!bUseLightDirectionOverride || LightDirectionOverride.IsNearlyZero())
	{
		return false;
	}

	OutDirection = LightDirectionOverride.GetSafeNormal();
	return true;
}
