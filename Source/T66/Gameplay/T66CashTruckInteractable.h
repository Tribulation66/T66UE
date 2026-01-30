// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66CashTruckInteractable.generated.h"

class UStaticMeshComponent;

/** Cash Truck: grants gold (rarity-scaled) or spawns a Mimic truck enemy. */
UCLASS(Blueprintable)
class T66_API AT66CashTruckInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66CashTruckInteractable();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Truck")
	TArray<TObjectPtr<UStaticMeshComponent>> WheelMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Truck")
	bool bIsMimic = false;

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
};

