// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Gameplay/T66CombatTargetTypes.h"
#include "T66CombatHitZoneComponent.generated.h"

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class T66_API UT66CombatHitZoneComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UT66CombatHitZoneComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	ET66HitZoneType HitZoneType = ET66HitZoneType::Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName HitZoneName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.1"))
	float DamageMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bTargetable = true;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	FVector GetAimPoint() const { return GetComponentLocation(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	FT66CombatTargetHandle MakeTargetHandle() const;
};
