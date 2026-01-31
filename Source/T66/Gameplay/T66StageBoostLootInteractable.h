// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66StageBoostLootInteractable.generated.h"

/** Stage Boost interactable: spawns loot bags once. */
UCLASS(Blueprintable)
class T66_API AT66StageBoostLootInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66StageBoostLootInteractable();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	int32 LootBagCount = 0;

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
};

