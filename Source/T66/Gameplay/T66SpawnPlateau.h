// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66SpawnPlateau.generated.h"

class UStaticMeshComponent;

/**
 * Flat disc placed under NPCs and world interactables so they sit on level ground
 * even on hills. Creates a small "plateau" at the spawn location.
 */
UCLASS()
class T66_API AT66SpawnPlateau : public AActor
{
	GENERATED_BODY()

public:
	AT66SpawnPlateau();

	/** Radius of the flat disc in UU. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Plateau")
	float PlateauRadius = 500.f;

	/** Total height (thickness) of the disc in UU. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Plateau")
	float PlateauHeight = 20.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plateau")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
