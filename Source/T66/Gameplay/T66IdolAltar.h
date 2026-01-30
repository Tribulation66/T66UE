// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66IdolAltar.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/**
 * Stage 1 Start Area Idol Altar.
 * Visual: 3 stacked rectangles (pyramid feel).
 * Interaction: Press Interact near it to open the Idol Altar overlay.
 */
UCLASS(Blueprintable)
class T66_API AT66IdolAltar : public AActor
{
	GENERATED_BODY()

public:
	AT66IdolAltar();

	/** Interact radius/trigger. We use PlayerController sphere overlap, so this just needs to overlap Pawn. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IdolAltar")
	TObjectPtr<UBoxComponent> InteractTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IdolAltar")
	TObjectPtr<UStaticMeshComponent> BaseRect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IdolAltar")
	TObjectPtr<UStaticMeshComponent> MidRect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IdolAltar")
	TObjectPtr<UStaticMeshComponent> TopRect;

	/** Apply simple placeholder visuals (color/material). */
	void ApplyVisuals();

protected:
	virtual void BeginPlay() override;
};

