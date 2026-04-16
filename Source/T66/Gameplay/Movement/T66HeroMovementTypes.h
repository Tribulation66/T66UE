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
	float MaxAcceleration = 99999.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float BrakingDecelerationWalking = 99999.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float GroundFriction = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float BrakingFrictionFactor = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float JumpZVelocity = 2200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float AirControl = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.1"))
	float GravityScale = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.05"))
	float DashCooldownSeconds = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "100.0"))
	float DashStrength = 1600.f;
};
