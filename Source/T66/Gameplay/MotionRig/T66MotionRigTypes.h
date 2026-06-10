// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66MotionRigTypes.generated.h"

// MotionRig lane: see MOTION_RIG.md at repo root for the authority doc.
// States are motor profiles, not modes — there is no kinematic/simulated switch.
UENUM(BlueprintType)
enum class ET66MotionRigState : uint8
{
	Idle,
	Walk,
	Jump,
	Dive,
	Knockdown,
	GetUp
};

inline const TCHAR* T66MotionRigStateName(const ET66MotionRigState State)
{
	switch (State)
	{
	case ET66MotionRigState::Idle:      return TEXT("Idle");
	case ET66MotionRigState::Walk:      return TEXT("Walk");
	case ET66MotionRigState::Jump:      return TEXT("Jump");
	case ET66MotionRigState::Dive:      return TEXT("Dive");
	case ET66MotionRigState::Knockdown: return TEXT("Knockdown");
	case ET66MotionRigState::GetUp:     return TEXT("GetUp");
	}
	return TEXT("Unknown");
}

// Per-body-set motor gains. DampingRatio < 1 is deliberately allowed: that is
// the wobble. MaxTorque 0 = unlimited (plugin convention).
USTRUCT()
struct FT66MotionRigMotorGains
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "MotionRig")
	float LinearStrength = 0.f;

	UPROPERTY(EditAnywhere, Category = "MotionRig")
	float LinearDampingRatio = 1.f;

	UPROPERTY(EditAnywhere, Category = "MotionRig")
	float AngularStrength = 0.f;

	UPROPERTY(EditAnywhere, Category = "MotionRig")
	float AngularDampingRatio = 1.f;
};

// Multipliers applied on top of the base set gains while a state is active.
// Knockdown drops everything near zero; GetUp ramps back in over RampSeconds.
USTRUCT()
struct FT66MotionRigStateMotorScale
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "MotionRig")
	float AllScale = 1.f;

	UPROPERTY(EditAnywhere, Category = "MotionRig")
	float ArmScale = 1.f;

	UPROPERTY(EditAnywhere, Category = "MotionRig")
	float PelvisWorldScale = 1.f;

	UPROPERTY(EditAnywhere, Category = "MotionRig")
	float RampSeconds = 0.f;
};
