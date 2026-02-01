// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66DifficultyTotem.generated.h"

class APlayerController;
class UStaticMeshComponent;
class UStaticMesh;

/** Difficulty Totem: interact to add skulls and scale the run difficulty. */
UCLASS(Blueprintable)
class T66_API AT66DifficultyTotem : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66DifficultyTotem();

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void BeginPlay() override;

private:
	void ApplyGrowthFromInteractionCount(int32 InteractionCount);

	/** Extra stacked visual segments (do not affect collision/interaction). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> StackedVisualSegments;

	/** Optional imported totem mesh (if null/unloaded, uses cube). */
	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
	TSoftObjectPtr<UStaticMesh> TotemMeshOverride;
};

