// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66CrateInteractable.generated.h"

class APlayerController;

/** Crate: opens a CS:GO-style item-reveal animation and grants an item. */
UCLASS(Blueprintable)
class T66_API AT66CrateInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66CrateInteractable();

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
};
