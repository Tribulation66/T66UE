// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66HeroSpeedSubsystem.h"

void UT66HeroSpeedSubsystem::SetParams(float InMaxSpeed, float AccelerationPercentPerSecond)
{
	MaxSpeed = FMath::Max(1.f, InMaxSpeed);
	const float Percent = FMath::Clamp(AccelerationPercentPerSecond, 1.f, 100.f) / 100.f;
	AccelerationPerSecond = MaxSpeed * Percent;
	DecelerationPerSecond = AccelerationPerSecond;
}

void UT66HeroSpeedSubsystem::Update(float DeltaTime, bool bHasMovementInput)
{
	bLastHasMovementInput = bHasMovementInput;
	const float Delta = FMath::Clamp(DeltaTime, 0.f, 0.5f);
	if (bHasMovementInput)
	{
		const float MinSpeedWhenMoving = MaxSpeed * BaseSpeedFraction;
		if (CurrentSpeed < MinSpeedWhenMoving)
			CurrentSpeed = MinSpeedWhenMoving;
		CurrentSpeed = FMath::Min(CurrentSpeed + AccelerationPerSecond * Delta, MaxSpeed);
	}
	else
	{
		CurrentSpeed = FMath::Max(0.f, CurrentSpeed - DecelerationPerSecond * Delta);
	}
}

int32 UT66HeroSpeedSubsystem::GetMovementAnimState() const
{
	// Visual state switches to idle instantly when input stops (no waiting for deceleration).
	// Speed still decelerates smoothly for gameplay feel.
	if (!bLastHasMovementInput)
		return 0; // Idle
	return 1; // Moving
}
