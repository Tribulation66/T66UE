// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66StageCatchUpGoldInteractable.generated.h"

/** Stage Catch Up interactable: grants gold once. */
UCLASS(Blueprintable)
class T66_API AT66StageCatchUpGoldInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66StageCatchUpGoldInteractable();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CatchUp")
	int32 GoldAmount = 0;

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
};
