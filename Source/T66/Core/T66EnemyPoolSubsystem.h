// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "T66EnemyPoolSubsystem.generated.h"

class AT66EnemyBase;

/**
 * [GOLD] Enemy Object Pool: pre-spawns and reuses enemy actors instead of
 * SpawnActor/Destroy every wave. Reduces GC pressure and spawn hitches.
 *
 * Usage:
 *   Acquire(ClassToSpawn, Location) — returns an active enemy (spawns new if pool empty).
 *   Release(Enemy)                 — deactivates and returns enemy to pool for reuse.
 */
UCLASS()
class T66_API UT66EnemyPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Try to acquire a pooled (inactive) enemy of the given class.
	 * Returns nullptr if no pooled instance is available — caller should spawn fresh.
	 *
	 * @param EnemyClass   The class to look for in the pool.
	 * @param Location     Where to place the reused enemy.
	 * @return             A reused enemy, or nullptr if pool is empty for this class.
	 */
	AT66EnemyBase* TryAcquire(TSubclassOf<AT66EnemyBase> EnemyClass, const FVector& Location);

	/**
	 * Return an enemy to the pool for later reuse.
	 * The enemy is hidden, collision disabled, and tick paused.
	 */
	void Release(AT66EnemyBase* Enemy);

	/** Total currently-pooled (inactive) enemies. */
	int32 GetPooledCount() const;

	/** Total enemies ever acquired (stats). */
	int32 GetTotalAcquired() const { return TotalAcquired; }

	/** Total times a pooled enemy was reused (stats). */
	int32 GetTotalReused() const { return TotalReused; }

protected:
	virtual void Deinitialize() override;

private:
	/** Pooled (inactive) enemies keyed by their UClass pointer. */
	TMap<UClass*, TArray<TWeakObjectPtr<AT66EnemyBase>>> Pool;

	int32 TotalAcquired = 0;
	int32 TotalReused = 0;
};
