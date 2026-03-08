// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66HeroSpeedSubsystem.h"

void UT66HeroSpeedSubsystem::SetParams(float InMaxSpeed, float AccelerationPercentPerSecond)
{
	MaxSpeed = FMath::Max(1.f, InMaxSpeed);
}

void UT66HeroSpeedSubsystem::Update(float DeltaTime, bool bHasMovementInput)
{
	bLastHasMovementInput = bHasMovementInput;
	CurrentSpeed = bHasMovementInput ? MaxSpeed : 0.f;
}

int32 UT66HeroSpeedSubsystem::GetMovementAnimState() const
{
	if (!bLastHasMovementInput)
		return 0; // Idle
	return 1; // Moving
}
