// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66GameplayInteractable.generated.h"

/**
 * Base class for gameplay interactables that use bespoke interaction UI or
 * stage-flow logic instead of the generic world-interactable prompt path.
 */
UCLASS(Abstract, Blueprintable)
class T66_API AT66GameplayInteractable : public AActor
{
	GENERATED_BODY()

public:
	AT66GameplayInteractable();
};
