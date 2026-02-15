// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66EnemyPoolSubsystem.h"
#include "Gameplay/T66EnemyBase.h"
#include "GameFramework/CharacterMovementComponent.h"

// ---------------------------------------------------------------------------
AT66EnemyBase* UT66EnemyPoolSubsystem::TryAcquire(TSubclassOf<AT66EnemyBase> EnemyClass, const FVector& Location)
{
	if (!EnemyClass) return nullptr;

	++TotalAcquired;

	// Try to reuse a pooled instance of the exact class.
	TArray<TWeakObjectPtr<AT66EnemyBase>>* ClassPool = Pool.Find(EnemyClass.Get());
	if (!ClassPool) return nullptr;

	while (ClassPool->Num() > 0)
	{
		TWeakObjectPtr<AT66EnemyBase> Weak = ClassPool->Pop(EAllowShrinking::No);
		AT66EnemyBase* Enemy = Weak.Get();
		if (!IsValid(Enemy)) continue; // was garbage-collected

		++TotalReused;
		UE_LOG(LogTemp, Log, TEXT("[GOLD] EnemyPool: reused %s (pool=%d, reuse-rate=%.0f%%)"),
			*Enemy->GetClass()->GetName(), ClassPool->Num(),
			TotalAcquired > 0 ? (100.f * static_cast<float>(TotalReused) / static_cast<float>(TotalAcquired)) : 0.f);
		return Enemy;
	}

	// Pool empty for this class.
	UE_LOG(LogTemp, Verbose, TEXT("[GOLD] EnemyPool: no pooled %s available, caller should spawn fresh (total-acquired=%d)"),
		*EnemyClass->GetName(), TotalAcquired);
	return nullptr;
}

// ---------------------------------------------------------------------------
void UT66EnemyPoolSubsystem::Release(AT66EnemyBase* Enemy)
{
	if (!IsValid(Enemy)) return;

	// Deactivate: hide, disable collision, pause ticking.
	Enemy->SetActorHiddenInGame(true);
	Enemy->SetActorEnableCollision(false);
	Enemy->SetActorTickEnabled(false);
	if (UCharacterMovementComponent* Move = Enemy->GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->SetComponentTickEnabled(false);
	}

	// Move far away to avoid any residual overlap queries.
	Enemy->SetActorLocation(FVector(0.f, 0.f, -50000.f));

	TArray<TWeakObjectPtr<AT66EnemyBase>>& ClassPool = Pool.FindOrAdd(Enemy->GetClass());
	ClassPool.Add(Enemy);

	UE_LOG(LogTemp, Verbose, TEXT("[GOLD] EnemyPool: released %s back to pool (pool=%d)"),
		*Enemy->GetClass()->GetName(), ClassPool.Num());
}

// ---------------------------------------------------------------------------
int32 UT66EnemyPoolSubsystem::GetPooledCount() const
{
	int32 Total = 0;
	for (const auto& Pair : Pool)
	{
		Total += Pair.Value.Num();
	}
	return Total;
}

// ---------------------------------------------------------------------------
void UT66EnemyPoolSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("[GOLD] EnemyPool: shutting down. Total acquired=%d, reused=%d (%.0f%%)"),
		TotalAcquired, TotalReused,
		TotalAcquired > 0 ? (100.f * static_cast<float>(TotalReused) / static_cast<float>(TotalAcquired)) : 0.f);
	Pool.Empty();
	Super::Deinitialize();
}
