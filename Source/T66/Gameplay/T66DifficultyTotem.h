// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66DifficultyTotem.generated.h"

class APlayerController;

/** Difficulty Totem: interact to increase difficulty tier (enemy HP/damage multiplier). */
UCLASS(Blueprintable)
class T66_API AT66DifficultyTotem : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66DifficultyTotem();

	virtual bool Interact(APlayerController* PC) override;
};

