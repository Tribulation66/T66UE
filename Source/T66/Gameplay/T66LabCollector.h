// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "T66LabCollector.generated.h"

/**
 * Lab-only NPC "The Collector". Interacting opens a full-screen UI to add items, spawn NPCs, enemies, and interactables.
 * For now represented as a cylinder (uses HouseNPCBase visual).
 */
UCLASS()
class T66_API AT66LabCollector : public AT66HouseNPCBase
{
	GENERATED_BODY()

public:
	AT66LabCollector();

	/** Opens the Collector full-screen overlay on the given player controller. */
	virtual bool Interact(APlayerController* PC) override;
};
