// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66EnemyBase.h"
#include "T66CashTruckMimicEnemy.generated.h"

class UStaticMeshComponent;

/**
 * Chest Mimic (formerly "Cash Truck Mimic"): chases the player for 5 seconds, deals heavy touch damage, then disappears.
 * Canonical in-game name: "Chest Mimic". Class alias: AT66ChestMimicEnemy.
 */
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

/** Canonical alias: use AT66ChestMimicEnemy in new code. */
using AT66ChestMimicEnemy = AT66CashTruckMimicEnemy;

