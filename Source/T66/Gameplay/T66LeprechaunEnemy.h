// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66EnemyBase.h"
#include "Core/T66Rarity.h"
#include "T66LeprechaunEnemy.generated.h"

class UStaticMeshComponent;

/**
 * Leprechaun:
 * - Flees from the hero
 * - Grants gold when hit
 */
UCLASS(Blueprintable)
class T66_API AT66LeprechaunEnemy : public AT66EnemyBase
{
	GENERATED_BODY()

public:
	AT66LeprechaunEnemy();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity")
	ET66Rarity Rarity = ET66Rarity::Black;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Leprechaun")
	int32 GoldPerHit = 25;

	void SetRarity(ET66Rarity InRarity);

	virtual bool TakeDamageFromHero(int32 Damage) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> HeadSphere;

	void ApplyRarityVisuals();
	void RecomputeGoldFromRarity();
};

