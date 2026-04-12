// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66CombatTargetTypes.generated.h"

class AActor;
class UPrimitiveComponent;

UENUM(BlueprintType)
enum class ET66HitZoneType : uint8
{
	None UMETA(DisplayName = "None"),
	Body UMETA(DisplayName = "Body"),
	Head UMETA(DisplayName = "Head"),
	Core UMETA(DisplayName = "Core"),
	WeakPoint UMETA(DisplayName = "Weak Point"),
	LeftArm UMETA(DisplayName = "Left Arm"),
	RightArm UMETA(DisplayName = "Right Arm"),
	LeftLeg UMETA(DisplayName = "Left Leg"),
	RightLeg UMETA(DisplayName = "Right Leg"),
};

USTRUCT(BlueprintType)
struct T66_API FT66CombatTargetHandle
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> Actor;

	UPROPERTY()
	TWeakObjectPtr<UPrimitiveComponent> HitComponent;

	UPROPERTY()
	ET66HitZoneType HitZoneType = ET66HitZoneType::None;

	UPROPERTY()
	FName HitZoneName = NAME_None;

	UPROPERTY()
	FVector AimPoint = FVector::ZeroVector;

	bool IsValid() const
	{
		return Actor.IsValid();
	}

	bool IsActorAlive() const
	{
		return Actor.IsValid();
	}

	void Reset()
	{
		Actor.Reset();
		HitComponent.Reset();
		HitZoneType = ET66HitZoneType::None;
		HitZoneName = NAME_None;
		AimPoint = FVector::ZeroVector;
	}
};
