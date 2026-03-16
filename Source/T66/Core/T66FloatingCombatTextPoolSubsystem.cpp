// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66FloatingCombatTextPoolSubsystem.h"

#include "Gameplay/T66FloatingCombatTextActor.h"
#include "Engine/World.h"
#include "TimerManager.h"

AT66FloatingCombatTextActor* UT66FloatingCombatTextPoolSubsystem::AcquireActor(AActor* AttachTarget, const FVector& RelativeLocation, float LifetimeSeconds)
{
	if (!IsValid(AttachTarget))
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	++TotalRequested;
	CompactPools();

	AT66FloatingCombatTextActor* Actor = nullptr;
	while (InactiveActors.Num() > 0 && !Actor)
	{
		Actor = InactiveActors.Pop(EAllowShrinking::No);
		if (!IsValid(Actor))
		{
			Actor = nullptr;
		}
	}

	if (Actor)
	{
		++TotalReused;
	}
	else if ((ActiveActors.Num() + InactiveActors.Num()) < MaxTrackedActors)
	{
		Actor = SpawnPooledActor();
		if (Actor)
		{
			++TotalSpawned;
		}
	}
	else
	{
		Actor = RecycleOldestActiveActor();
		if (Actor)
		{
			++TotalRecycled;
		}
	}

	if (!Actor)
	{
		return nullptr;
	}

	ClearReleaseTimer(Actor);
	Actor->ActivateForTarget(AttachTarget, RelativeLocation);
	ActiveActors.AddUnique(Actor);
	ActiveOrder.Add(Actor);
	ScheduleRelease(Actor, LifetimeSeconds);
	return Actor;
}

void UT66FloatingCombatTextPoolSubsystem::ReleaseActor(AT66FloatingCombatTextActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	ClearReleaseTimer(Actor);
	ActiveActors.Remove(Actor);
	ActiveOrder.Remove(Actor);
	Actor->DeactivateForPool();
	InactiveActors.AddUnique(Actor);
}

int32 UT66FloatingCombatTextPoolSubsystem::GetActiveCount() const
{
	int32 Count = 0;
	for (AT66FloatingCombatTextActor* Actor : ActiveActors)
	{
		if (IsValid(Actor))
		{
			++Count;
		}
	}
	return Count;
}

int32 UT66FloatingCombatTextPoolSubsystem::GetInactiveCount() const
{
	int32 Count = 0;
	for (AT66FloatingCombatTextActor* Actor : InactiveActors)
	{
		if (IsValid(Actor))
		{
			++Count;
		}
	}
	return Count;
}

void UT66FloatingCombatTextPoolSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		for (TPair<TWeakObjectPtr<AT66FloatingCombatTextActor>, FTimerHandle>& Pair : ReleaseTimers)
		{
			World->GetTimerManager().ClearTimer(Pair.Value);
		}
	}
	ReleaseTimers.Reset();

	for (AT66FloatingCombatTextActor* Actor : ActiveActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	for (AT66FloatingCombatTextActor* Actor : InactiveActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}

	ActiveActors.Reset();
	InactiveActors.Reset();
	ActiveOrder.Reset();

	Super::Deinitialize();
}

void UT66FloatingCombatTextPoolSubsystem::CompactPools()
{
	InactiveActors.RemoveAll([](const TObjectPtr<AT66FloatingCombatTextActor>& Actor)
	{
		return !IsValid(Actor);
	});
	ActiveActors.RemoveAll([](const TObjectPtr<AT66FloatingCombatTextActor>& Actor)
	{
		return !IsValid(Actor);
	});
	ActiveOrder.RemoveAll([](const TWeakObjectPtr<AT66FloatingCombatTextActor>& Actor)
	{
		return !Actor.IsValid();
	});

	for (auto It = ReleaseTimers.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
		}
	}
}

AT66FloatingCombatTextActor* UT66FloatingCombatTextPoolSubsystem::SpawnPooledActor()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AT66FloatingCombatTextActor* Actor = World->SpawnActor<AT66FloatingCombatTextActor>(FVector(0.f, 0.f, -50000.f), FRotator::ZeroRotator, SpawnParams);
	if (Actor)
	{
		Actor->DeactivateForPool();
	}
	return Actor;
}

AT66FloatingCombatTextActor* UT66FloatingCombatTextPoolSubsystem::RecycleOldestActiveActor()
{
	while (ActiveOrder.Num() > 0)
	{
		TWeakObjectPtr<AT66FloatingCombatTextActor> WeakActor = ActiveOrder[0];
		ActiveOrder.RemoveAt(0, 1, EAllowShrinking::No);

		AT66FloatingCombatTextActor* Actor = WeakActor.Get();
		if (!IsValid(Actor))
		{
			continue;
		}

		ReleaseActor(Actor);
		InactiveActors.Remove(Actor);
		return Actor;
	}

	return nullptr;
}

void UT66FloatingCombatTextPoolSubsystem::ScheduleRelease(AT66FloatingCombatTextActor* Actor, float LifetimeSeconds)
{
	UWorld* World = GetWorld();
	if (!World || !IsValid(Actor))
	{
		return;
	}

	FTimerHandle& Handle = ReleaseTimers.FindOrAdd(Actor);
	const TWeakObjectPtr<AT66FloatingCombatTextActor> WeakActor = Actor;
	World->GetTimerManager().SetTimer(
		Handle,
		FTimerDelegate::CreateWeakLambda(this, [this, WeakActor]()
		{
			if (AT66FloatingCombatTextActor* ResolvedActor = WeakActor.Get())
			{
				ReleaseActor(ResolvedActor);
			}
		}),
		FMath::Max(0.01f, LifetimeSeconds),
		false);
}

void UT66FloatingCombatTextPoolSubsystem::ClearReleaseTimer(AT66FloatingCombatTextActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (FTimerHandle* Handle = ReleaseTimers.Find(Actor))
		{
			World->GetTimerManager().ClearTimer(*Handle);
		}
	}
	ReleaseTimers.Remove(Actor);
}
