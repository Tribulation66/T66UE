// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Components/T66MiniPickupMagnetComponent.h"

UT66MiniPickupMagnetComponent::UT66MiniPickupMagnetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UT66MiniPickupMagnetComponent::SetMagnetProfile(const float InMagnetRadius, const float InPullSpeed)
{
	MagnetRadius = FMath::Max(0.f, InMagnetRadius);
	PullSpeed = FMath::Max(0.f, InPullSpeed);
}
