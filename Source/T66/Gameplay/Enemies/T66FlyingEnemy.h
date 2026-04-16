// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66EnemyBase.h"
#include "T66FlyingEnemy.generated.h"

UCLASS(Blueprintable)
class T66_API AT66FlyingEnemy : public AT66EnemyBase
{
	GENERATED_BODY()

public:
	AT66FlyingEnemy();

protected:
	virtual void ResetFamilyState() override;
	virtual void TickFamilyBehavior(APawn* PlayerPawn, float DeltaSeconds, float Dist2DToPlayer, bool bShouldRunAwayFromPlayer) override;
	virtual EMovementMode GetDefaultMovementMode() const override { return MOVE_Flying; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flying")
	float HoverHeight = 180.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flying")
	float HoverBobAmplitude = 35.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flying")
	float HoverBobFrequency = 2.2f;

private:
	float HoverAnchorZ = 0.f;
	float HoverBobTime = 0.f;
};
