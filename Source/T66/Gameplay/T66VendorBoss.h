// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66BossBase.h"
#include "T66VendorBoss.generated.h"

/** Vendor hidden boss, spawned when a shop steal attempt fails. Uses AT66BossBase (armor, debuff, Taunt). */
UCLASS(Blueprintable)
class T66_API AT66VendorBoss : public AT66BossBase
{
	GENERATED_BODY()

public:
	AT66VendorBoss();

protected:
	virtual void BeginPlay() override;
	virtual void Die() override;
};
