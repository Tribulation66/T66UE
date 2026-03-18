// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiasmaBoundary.generated.h"

class UProceduralMeshComponent;
class USceneComponent;

/**
 * Procedural cliff ring at the playable area boundary.
 * The cliffs block movement, and if the player somehow gets outside the safe rectangle they still take ticking damage.
 */
UCLASS(Blueprintable)
class T66_API AT66MiasmaBoundary : public AActor
{
	GENERATED_BODY()

public:
	AT66MiasmaBoundary();

	/** Half-extent of the safe playable rectangle (centered at 0,0). Outside this = miasma damage. 100k map = 50000. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miasma Boundary")
	float SafeHalfExtent = 50000.f;

	/** Damage interval while outside the boundary (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary")
	float DamageIntervalSeconds = 1.0f;

	/** Minimum ridge height of the generated cliff ring. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary|Cliffs")
	float WallHeight = 3600.f;

	/** Maximum ridge height of the generated cliff ring. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary|Cliffs")
	float MaxWallHeight = 7600.f;

	/** Minimum outward depth of the generated cliff ring. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary|Cliffs")
	float WallThickness = 1800.f;

	/** Maximum outward depth of the generated cliff ring. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary|Cliffs")
	float MaxWallThickness = 5200.f;

	/** Number of faceted cliff segments generated per side of the map. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary|Cliffs")
	int32 SegmentsPerSide = 24;

	/** Slight inward offset so the blocking cliff face sits right on the playable edge. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary|Cliffs")
	float InnerInset = 180.f;

	/** How far the cliff base is buried into the ground to hide seams at the terrain edge. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary|Cliffs")
	float BaseSink = 1000.f;

	/** Height of the straight vertical contact face before the cliff breaks into angled planes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary|Cliffs")
	float VerticalFaceHeight = 1400.f;

	/** Extra overlap past each corner so adjacent sides interlock instead of leaving corner gaps. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary|Cliffs")
	float CornerOverlap = 1800.f;

	/** Seed controlling deterministic cliff faceting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary|Cliffs")
	int32 BoundarySeed = 660066;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void BuildBoundaryCliffs();
	void ApplyBoundaryDamageTick();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Miasma Boundary")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Miasma Boundary")
	TObjectPtr<UProceduralMeshComponent> BoundaryCliffMesh;

	FTimerHandle DamageTimerHandle;
};
