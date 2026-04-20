// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66HeroMovementTypes.generated.h"

UENUM(BlueprintType)
enum class ET66DashDirection : uint8
{
	None UMETA(DisplayName = "None"),
	North UMETA(DisplayName = "North"),
	NorthEast UMETA(DisplayName = "North East"),
	East UMETA(DisplayName = "East"),
	SouthEast UMETA(DisplayName = "South East"),
	South UMETA(DisplayName = "South"),
	SouthWest UMETA(DisplayName = "South West"),
	West UMETA(DisplayName = "West"),
	NorthWest UMETA(DisplayName = "North West")
};

USTRUCT(BlueprintType)
struct T66_API FT66HeroMovementTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "100.0"))
	float DefaultWalkSpeed = 1800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float MaxAcceleration = 9000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float BrakingDecelerationWalking = 12000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float GroundFriction = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (InlineEditConditionToggle))
	bool bUseSeparateBrakingFriction = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", EditCondition = "bUseSeparateBrakingFriction"))
	float BrakingFriction = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float BrakingFrictionFactor = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float JumpZVelocity = 1600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float JumpMaxHoldTime = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float AirControl = 0.40f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.1"))
	float GravityScale = 4.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float FallingLateralFriction = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float BrakingDecelerationFalling = 4096.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.05"))
	float DashCooldownSeconds = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "100.0"))
	float DashStrength = 3200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "1.0"))
	float DashSpeedMultiplierOverWalkSpeed = 1.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float RotationRateYaw = 1440.f;
};
