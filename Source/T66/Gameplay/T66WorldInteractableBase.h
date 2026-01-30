// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/T66Rarity.h"
#include "T66WorldInteractableBase.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class APlayerController;

/** Base class for simple world interactables (press F near it). */
UCLASS(Abstract, Blueprintable)
class T66_API AT66WorldInteractableBase : public AActor
{
	GENERATED_BODY()

public:
	AT66WorldInteractableBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
	TObjectPtr<UBoxComponent> TriggerBox;

	/** Primary visual mesh (subclasses may add more). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable")
	ET66Rarity Rarity = ET66Rarity::Black;

	/** If true, this interactable is already consumed (single-player v0). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
	bool bConsumed = false;

	/** Assign rarity and refresh visuals. */
	void SetRarity(ET66Rarity InRarity);

	/** Press-F interaction. Returns true if consumed. */
	virtual bool Interact(APlayerController* PC) PURE_VIRTUAL(AT66WorldInteractableBase::Interact, return false;);

protected:
	virtual void ApplyRarityVisuals();
};

