// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66WheelSpinInteractable.generated.h"

class UStaticMeshComponent;
class APlayerController;

/** Wheel Spin: opens a non-pausing wheel popup and awards gold. */
UCLASS(Blueprintable)
class T66_API AT66WheelSpinInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66WheelSpinInteractable();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wheel")
	TObjectPtr<UStaticMeshComponent> SphereBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wheel")
	TObjectPtr<UStaticMeshComponent> WheelMesh;

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
};

