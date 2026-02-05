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
	const float Delta = FMath::Clamp(DeltaTime, 0.f, 0.5f);
	if (bHasMovementInput)
	{
		TimeWalkingSeconds += Delta;
		const float MinSpeedWhenMoving = MaxSpeed * BaseSpeedFraction;
		if (CurrentSpeed < MinSpeedWhenMoving)
			CurrentSpeed = MinSpeedWhenMoving;
		CurrentSpeed = FMath::Min(CurrentSpeed + AccelerationPerSecond * Delta, MaxSpeed);
	}
	else
	{
		TimeWalkingSeconds = 0.f;
		CurrentSpeed = FMath::Max(0.f, CurrentSpeed - DecelerationPerSecond * Delta);
	}
}

int32 UT66HeroSpeedSubsystem::GetMovementAnimState() const
{
	if (CurrentSpeed <= 0.f)
		return 0; // Idle (alert)
	if (TimeWalkingSeconds >= SecondsWalkingBeforeRun)
		return 2; // Run
	return 1; // Walk
}
