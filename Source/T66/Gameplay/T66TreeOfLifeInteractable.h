// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66TreeOfLifeInteractable.generated.h"

class UStaticMeshComponent;
class UStaticMesh;

/**
 * Fountain of Life (formerly "Tree of Life"): grants max hearts (rarity-scaled) and consumes itself.
 * Canonical in-game name: "Fountain of Life". Class alias: AT66FountainOfLifeInteractable.
 */
UCLASS(Blueprintable)
class T66_API AT66TreeOfLifeInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66TreeOfLifeInteractable();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tree")
	TObjectPtr<UStaticMeshComponent> CrownMesh;

	/** Optional rarity meshes (when imported). If null/unloaded, we fall back to primitive shapes. */
	UPROPERTY(EditDefaultsOnly, Category = "Tree|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshBlack;

	UPROPERTY(EditDefaultsOnly, Category = "Tree|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshRed;

	UPROPERTY(EditDefaultsOnly, Category = "Tree|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshYellow;

	UPROPERTY(EditDefaultsOnly, Category = "Tree|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshWhite;

	virtual bool Interact(APlayerController* PC) override;

	/** Luck multiplier from items (multiplies spawn quantity/rarity for fountains). */
	float GetLuckFountainMultiplier() const;

protected:
	virtual void ApplyRarityVisuals() override;
};

/** Canonical alias: use AT66FountainOfLifeInteractable in new code. */
using AT66FountainOfLifeInteractable = AT66TreeOfLifeInteractable;

