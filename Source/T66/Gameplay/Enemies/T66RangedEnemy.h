// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66EnemyBase.h"
#include "T66RangedEnemy.generated.h"

class AT66EnemyProjectileBase;

UCLASS(Blueprintable)
class T66_API AT66RangedEnemy : public AT66EnemyBase
{
	GENERATED_BODY()

public:
	AT66RangedEnemy();

protected:
	virtual void ResetFamilyState() override;
	virtual void TickFamilyBehavior(APawn* PlayerPawn, float DeltaSeconds, float Dist2DToPlayer, bool bShouldRunAwayFromPlayer) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged")
	float DesiredMinRange = 800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged")
	float DesiredMaxRange = 1400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged")
	float FireIntervalSeconds = 1.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged")
	float ProjectileSpawnHeight = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged")
	TSubclassOf<AT66EnemyProjectileBase> ProjectileClass;

private:
	void FireProjectileAtPlayer(APawn* PlayerPawn);

	float FireCooldownRemaining = 0.6f;
};
