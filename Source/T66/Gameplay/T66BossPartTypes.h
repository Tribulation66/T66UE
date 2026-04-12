// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66CombatTargetTypes.h"
#include "T66BossPartTypes.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66BossPartDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Part")
	FName PartID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Part")
	ET66HitZoneType HitZoneType = ET66HitZoneType::Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Part", meta = (ClampMin = "0.01"))
	float HPWeight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Part", meta = (ClampMin = "0.1"))
	float DamageMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Part")
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Part", meta = (ClampMin = "1.0"))
	float Radius = 48.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Part")
	bool bTargetable = true;
};

USTRUCT(BlueprintType)
struct T66_API FT66BossPartSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss Part")
	FName PartID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss Part")
	ET66HitZoneType HitZoneType = ET66HitZoneType::Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss Part")
	int32 MaxHP = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss Part")
	int32 CurrentHP = 0;

	bool IsAlive() const
	{
		return CurrentHP > 0;
	}
};
