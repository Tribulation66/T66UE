// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66EnemyBase.h"
#include "T66RushEnemy.generated.h"

UCLASS(Blueprintable)
class T66_API AT66RushEnemy : public AT66EnemyBase
{
	GENERATED_BODY()

public:
	AT66RushEnemy();

protected:
	virtual void ResetFamilyState() override;
	virtual void TickFamilyBehavior(APawn* PlayerPawn, float DeltaSeconds, float Dist2DToPlayer, bool bShouldRunAwayFromPlayer) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rush")
	float RushIntervalSeconds = 2.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rush")
	float RushDurationSeconds = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rush")
	float RushSpeedMultiplier = 3.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rush")
	float RushTriggerDistance = 1700.f;

private:
	float RushCooldownRemaining = 0.8f;
	float RushSecondsRemaining = 0.f;
	FVector RushDirection = FVector::ZeroVector;
};
