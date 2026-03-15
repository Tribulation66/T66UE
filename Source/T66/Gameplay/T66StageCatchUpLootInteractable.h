// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66StageCatchUpLootInteractable.generated.h"

/** Stage Catch Up interactable: spawns loot bags once. */
UCLASS(Blueprintable)
class T66_API AT66StageCatchUpLootInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66StageCatchUpLootInteractable();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CatchUp")
	int32 LootBagCount = 0;

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
};
