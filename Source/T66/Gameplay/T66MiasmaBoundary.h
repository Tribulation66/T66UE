// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiasmaBoundary.generated.h"

class UInstancedStaticMeshComponent;
class USceneComponent;

namespace T66MainMapTerrain
{
	struct FBoard;
}

/**
 * Runtime lava perimeter around the playable terrain footprint.
 * The outer lava apron visualizes the non-playable edge, while the actor still damages pawns that leave the playable cells.
 */
UCLASS(Blueprintable)
class T66_API AT66MiasmaBoundary : public AActor
{
	GENERATED_BODY()

public:
	AT66MiasmaBoundary();

	/** Fallback half-extent used by non-main-map stages. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miasma Boundary")
	float SafeHalfExtent = 40000.f;

	/** Damage interval while outside the boundary (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary")
	float DamageIntervalSeconds = 1.0f;

	/** Total square footprint, in cells, covered by the main map plus its lava perimeter. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary|Lava", meta = (ClampMin = "81", ClampMax = "160"))
	int32 TotalFootprintSizeCells = 100;

	/** Perimeter lava renders slightly above the sampled ground height. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary|Lava")
	float BoundaryLavaTileZ = 4.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void BuildBoundaryWalls();
	void CachePlayableCells(const T66MainMapTerrain::FBoard& Board);
	bool IsLocationInsidePlayableSpace(const FVector& Location) const;
	void ApplyBoundaryDamageTick();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Miasma Boundary")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Miasma Boundary")
	TObjectPtr<UInstancedStaticMeshComponent> BoundaryWallInstances;

	FTimerHandle DamageTimerHandle;
	TSet<FIntPoint> PlayableCellCoordinates;
	float PlayableCellSize = 2000.f;
	float PlayableHalfExtent = 40000.f;
};
