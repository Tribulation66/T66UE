// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66FountainInteractable.generated.h"

class UStaticMeshComponent;
class UStaticMesh;

/** Fountain: resets hero damage percent to 0% and consumes itself. */
UCLASS(Blueprintable)
class T66_API AT66FountainInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66FountainInteractable();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fountain")
	TObjectPtr<UStaticMeshComponent> WaterMesh;

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
};
