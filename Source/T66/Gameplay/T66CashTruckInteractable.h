// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66CashTruckInteractable.generated.h"

class UStaticMeshComponent;
class UStaticMesh;

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

	/** Optional rarity meshes (when imported). If null/unloaded, we fall back to primitive shapes. */
	UPROPERTY(EditDefaultsOnly, Category = "Truck|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshBlack;

	UPROPERTY(EditDefaultsOnly, Category = "Truck|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshRed;

	UPROPERTY(EditDefaultsOnly, Category = "Truck|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshYellow;

	UPROPERTY(EditDefaultsOnly, Category = "Truck|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshWhite;

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
};

