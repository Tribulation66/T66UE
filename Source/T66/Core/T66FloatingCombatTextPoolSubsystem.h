// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "T66FloatingCombatTextPoolSubsystem.generated.h"

class AActor;
class AT66FloatingCombatTextActor;

/**
 * World-scoped pool for floating combat text actors.
 * Reuses widget actors instead of spawning/destroying one per damage event.
 */
UCLASS()
class T66_API UT66FloatingCombatTextPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	AT66FloatingCombatTextActor* AcquireActor(AActor* AttachTarget, const FVector& RelativeLocation, float LifetimeSeconds);
	void ReleaseActor(AT66FloatingCombatTextActor* Actor);

	int32 GetActiveCount() const;
	int32 GetInactiveCount() const;
	int32 GetTotalRequested() const { return TotalRequested; }
	int32 GetTotalSpawned() const { return TotalSpawned; }
	int32 GetTotalReused() const { return TotalReused; }
	int32 GetTotalRecycled() const { return TotalRecycled; }

protected:
	virtual void Deinitialize() override;

private:
	static constexpr int32 MaxTrackedActors = 48;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AT66FloatingCombatTextActor>> InactiveActors;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AT66FloatingCombatTextActor>> ActiveActors;

	TArray<TWeakObjectPtr<AT66FloatingCombatTextActor>> ActiveOrder;
	TMap<TWeakObjectPtr<AT66FloatingCombatTextActor>, FTimerHandle> ReleaseTimers;

	int32 TotalRequested = 0;
	int32 TotalSpawned = 0;
	int32 TotalReused = 0;
	int32 TotalRecycled = 0;
	int32 AcquireRequestsSinceLastCompact = 0;

	static constexpr int32 CompactIntervalRequests = 16;

	void CompactPools();
	AT66FloatingCombatTextActor* SpawnPooledActor();
	AT66FloatingCombatTextActor* RecycleOldestActiveActor();
	void ScheduleRelease(AT66FloatingCombatTextActor* Actor, float LifetimeSeconds);
	void ClearReleaseTimer(AT66FloatingCombatTextActor* Actor);
};
