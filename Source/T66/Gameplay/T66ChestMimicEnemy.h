// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66EnemyBase.h"
#include "T66ChestMimicEnemy.generated.h"

class UStaticMeshComponent;

/**
 * Chest Mimic: chases the player for 5 seconds, deals heavy touch damage, then disappears.
 */
UCLASS(Blueprintable)
class T66_API AT66ChestMimicEnemy : public AT66EnemyBase
{
	GENERATED_BODY()

public:
	AT66ChestMimicEnemy();

protected:
	virtual void OnDeath() override;
};
