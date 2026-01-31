// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66EnemyBase.h"
#include "Data/T66DataTypes.h"
#include "T66UniqueDebuffEnemy.generated.h"

/** Floating unique enemy that fires fast straight debuff projectiles. */
UCLASS(Blueprintable)
class T66_API AT66UniqueDebuffEnemy : public AT66EnemyBase
{
	GENERATED_BODY()

public:
	AT66UniqueDebuffEnemy();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unique")
	float HoverHeight = 260.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unique")
	float FireIntervalSeconds = 1.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unique")
	float LifetimeSeconds = 10.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void FireAtPlayer();

private:
	FTimerHandle FireTimer;
	float LifeRemaining = 0.f;
};

