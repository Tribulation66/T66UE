// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66TreeOfLifeInteractable.generated.h"

class UStaticMeshComponent;
class UStaticMesh;

/**
 * Fountain of Life (formerly "Tree of Life"): fully heals, grants +1 max heart, and consumes itself.
 * Canonical in-game name: "Fountain of Life". Class alias: AT66FountainOfLifeInteractable.
 */
UCLASS(Blueprintable)
class T66_API AT66TreeOfLifeInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66TreeOfLifeInteractable();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fountain")
	TObjectPtr<UStaticMeshComponent> CrownMesh;

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
};

/** Canonical alias: use AT66FountainOfLifeInteractable in new code. */
using AT66FountainOfLifeInteractable = AT66TreeOfLifeInteractable;

