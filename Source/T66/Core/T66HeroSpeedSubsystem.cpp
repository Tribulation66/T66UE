// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66HeroSpeedSubsystem.h"

void UT66HeroSpeedSubsystem::SetParams(float InMovementSpeed, float AccelerationPercentPerSecond)
{
	MaxSpeed = FMath::Max(1.f, InMovementSpeed);
}

void UT66HeroSpeedSubsystem::Update(float DeltaTime, bool bHasMovementInput)
{
	CurrentSpeed = bHasMovementInput ? MaxSpeed : 0.f;
}

int32 UT66HeroSpeedSubsystem::GetMovementAnimState() const
{
	return MovementAnimState;
}

void UT66HeroSpeedSubsystem::SetMovementAnimState(const int32 InState)
{
	MovementAnimState = FMath::Clamp(InState, 0, 3);
}
