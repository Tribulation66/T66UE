// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66DifficultyTotem.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class APlayerController;

/** Difficulty Totem: interact to increase difficulty tier (enemy HP/damage multiplier). */
UCLASS(Blueprintable)
class T66_API AT66DifficultyTotem : public AActor
{
	GENERATED_BODY()

public:
	AT66DifficultyTotem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	bool Interact(APlayerController* PC);
};

