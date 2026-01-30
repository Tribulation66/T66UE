// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66EnemyBase.h"
#include "Core/T66Rarity.h"
#include "T66GoblinThiefEnemy.generated.h"

/**
 * Goblin Thief:
 * - Chases the hero
 * - On touch: steals gold instead of dealing heart damage
 */
UCLASS(Blueprintable)
class T66_API AT66GoblinThiefEnemy : public AT66EnemyBase
{
	GENERATED_BODY()

public:
	AT66GoblinThiefEnemy();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity")
	ET66Rarity Rarity = ET66Rarity::Black;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Thief")
	int32 GoldStolenPerHit = 50;

	void SetRarity(ET66Rarity InRarity);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnCapsuleBeginOverlapThief(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void ApplyRarityVisuals();
	void RecomputeGoldFromRarity();
};

