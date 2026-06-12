// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66TowerLiftPlatform.generated.h"

class UBoxComponent;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Fall Guys-style moving lift platform (non-trap elevator) cycling between the
 * ground tier and a mesa top inside one tower floor. Collision is a MOVING hidden
 * box proxy slab the hero rides through the standard character based-movement
 * path; the visual is a separate no-collision candy slab (FallGuysKit).
 *
 * Stateful floor rule: a lift cycling between tiers WITHIN a floor never touches
 * floor membership. If a future lift ever crosses FLOORS, that arrival is an
 * explicit transition and must call AT66GameMode::SetHeroTowerFloorNumber.
 */
UCLASS()
class T66_API AT66TowerLiftPlatform : public AActor
{
	GENERATED_BODY()

public:
	AT66TowerLiftPlatform();

	/**
	 * Configures geometry and cycle timing. DeckBottomZ/DeckTopZ are the deck-top
	 * heights at the cycle extremes; PhaseFraction [0..1) offsets the cycle start.
	 */
	void InitLift(
		UStaticMesh* SlabMesh,
		UMaterialInterface* SlabMaterial,
		const FVector2D& DeckCenter,
		const FVector2D& DeckHalfExtents,
		float InDeckBottomZ,
		float InDeckTopZ,
		float InTravelSeconds,
		float InDwellSeconds,
		float PhaseFraction);

	virtual void Tick(float DeltaSeconds) override;

	/** Deck-top Z for a given point in the cycle (exposed for automation proof). */
	float ResolveDeckTopZ(float InCycleSeconds) const;
	float GetCyclePeriod() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> LiftCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> LiftVisual;

	/** Collision slab thickness (deck top to slab underside). */
	static constexpr float SlabThickness = 60.0f;

private:
	void ApplyDeckTopZ(float DeckTopZ);

	float DeckBottomZ = 0.0f;
	float DeckTopZ = 0.0f;
	float TravelSeconds = 3.0f;
	float DwellSeconds = 2.0f;
	float CycleSeconds = 0.0f;
	FVector2D CachedDeckCenter = FVector2D::ZeroVector;
};
