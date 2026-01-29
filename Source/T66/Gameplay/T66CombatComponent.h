// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "T66CombatComponent.generated.h"

class AT66EnemyBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class T66_API UT66CombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66CombatComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackRange = 4000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float FireIntervalSeconds = 10.f;

	/** Damage per shot (use high value for insta kill) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 DamagePerShot = 999;

protected:
	virtual void BeginPlay() override;

	void TryFire();

	FTimerHandle FireTimerHandle;
};
