// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66StageBoostGoldInteractable.generated.h"

/** Stage Boost interactable: grants gold once. */
UCLASS(Blueprintable)
class T66_API AT66StageBoostGoldInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66StageBoostGoldInteractable();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	int32 GoldAmount = 0;

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
};

