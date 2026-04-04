// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66IdolAltar.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UPrimitiveComponent;

/**
 * Stage-entry Idol Altar.
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

	/** Optional imported altar mesh (if set, we hide the 3-rect placeholder stack). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IdolAltar")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditDefaultsOnly, Category = "IdolAltar")
	TSoftObjectPtr<UStaticMesh> AltarMeshOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IdolAltar")
	float VisualScaleMultiplier = 4.0f;

	/** Remaining selections granted by this altar visit. The altar destroys itself when this hits zero. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IdolAltar")
	int32 RemainingSelections = 1;

	/** Extra selections granted to catch up with skipped starting stages on higher difficulties. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IdolAltar")
	int32 CatchUpSelectionsRemaining = 0;

	/** Apply simple placeholder visuals (color/material). */
	void ApplyVisuals();

protected:
	virtual void BeginPlay() override;

private:
	void UpdateInteractionBounds();
	void ConfigureVisualCollision(UPrimitiveComponent* Primitive, bool bEnableCollision) const;
};

