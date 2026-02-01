// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66WheelSpinInteractable.generated.h"

class UStaticMeshComponent;
class UStaticMesh;
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

	/** Optional rarity meshes (when imported). If null/unloaded, we fall back to primitive shapes. */
	UPROPERTY(EditDefaultsOnly, Category = "Wheel|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshBlack;

	UPROPERTY(EditDefaultsOnly, Category = "Wheel|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshRed;

	UPROPERTY(EditDefaultsOnly, Category = "Wheel|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshYellow;

	UPROPERTY(EditDefaultsOnly, Category = "Wheel|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshWhite;

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
};

