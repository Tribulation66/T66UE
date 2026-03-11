// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66ChestInteractable.generated.h"

class UStaticMeshComponent;
class UStaticMesh;

/**
 * Treasure Chest: grants gold (rarity-scaled) or spawns a Mimic.
 */
UCLASS(Blueprintable)
class T66_API AT66ChestInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66ChestInteractable();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
	bool bIsMimic = false;

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
};
