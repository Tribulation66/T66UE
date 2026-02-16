// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66BossBase.h"
#include "T66GamblerBoss.generated.h"

/** Gambler Boss spawned when Anger reaches 100%. Uses AT66BossBase (armor, debuff, Taunt). */
UCLASS(Blueprintable)
class T66_API AT66GamblerBoss : public AT66BossBase
{
	GENERATED_BODY()

public:
	AT66GamblerBoss();

protected:
	virtual void BeginPlay() override;
	virtual void Die() override;
};
