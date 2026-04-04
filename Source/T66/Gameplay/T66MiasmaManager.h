// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiasmaManager.generated.h"

class AT66MiasmaTile;
class UInstancedStaticMeshComponent;

/**
 * Spawns thin black miasma tiles as soon as the stage timer starts.
 * When timer reaches 0, spawns full coverage. Can be cleared (e.g. on boss death).
 */
UCLASS(Blueprintable)
class T66_API AT66MiasmaManager : public AActor
{
	GENERATED_BODY()

public:
	AT66MiasmaManager();

	/** Half-extent of square area to cover (centered at world origin). (4x map = 2x linear) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma")
	float CoverageHalfExtent = 50000.f;

	/** Tile size (square). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma")
	float TileSize = 750.f;

	/** Miasma spawns slightly above ground. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma")
	float TileZ = 5.f;

	/** Damage interval while the hero is standing on active lava/miasma tiles. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma")
	float DamageIntervalSeconds = 1.0f;

	/** Spawn order random seed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma")
	int32 Seed = 1337;

	/** When true, no ground miasma tiles are spawned or updated. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miasma")
	bool bSpawningPaused = false;

	/** Clears all spawned miasma tiles. */
	void ClearAllMiasma();

	/** Called when stage timer changes / starts; spawns additional tiles based on progress. */
	void UpdateFromRunState();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> TileInstances;

	UPROPERTY()
	TArray<FVector> TileCenters;

	int32 SpawnedTileCount = 0;
	float DamageTickAccumulator = 0.f;

	void BuildGrid();
	void EnsureSpawnedCount(int32 DesiredCount);
	void BuildMainMapSubcellGrid();
	void TickDamageOverActiveTiles(float DeltaTime);
};
