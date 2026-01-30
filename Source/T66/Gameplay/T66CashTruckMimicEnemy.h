// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66EnemyBase.h"
#include "T66CashTruckMimicEnemy.generated.h"

class UStaticMeshComponent;

/** Cash Truck Mimic: chases the player for 5 seconds, deals heavy touch damage, then disappears. */
UCLASS(Blueprintable)
class T66_API AT66CashTruckMimicEnemy : public AT66EnemyBase
{
	GENERATED_BODY()

public:
	AT66CashTruckMimicEnemy();

protected:
	virtual void OnDeath() override;

private:
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> WheelMeshes;
};

