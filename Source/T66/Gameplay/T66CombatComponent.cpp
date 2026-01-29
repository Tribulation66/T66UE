// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66HeroProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

UT66CombatComponent::UT66CombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UT66CombatComponent::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &UT66CombatComponent::TryFire, FireIntervalSeconds, true, FireIntervalSeconds);
}

void UT66CombatComponent::TryFire()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FVector MyLoc = OwnerActor->GetActorLocation();
	AT66EnemyBase* ClosestEnemy = nullptr;
	float ClosestDistSq = AttackRange * AttackRange;

	for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
	{
		AT66EnemyBase* Enemy = *It;
		if (!Enemy || Enemy->CurrentHP <= 0) continue;
		float DistSq = FVector::DistSquared(MyLoc, Enemy->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestEnemy = Enemy;
		}
	}

	if (ClosestEnemy)
	{
		FVector SpawnLoc = MyLoc + FVector(0.f, 0.f, 50.f);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerActor;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66HeroProjectile* Proj = World->SpawnActor<AT66HeroProjectile>(AT66HeroProjectile::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (Proj)
		{
			Proj->Damage = DamagePerShot; // 999 = insta kill
			Proj->SetTargetLocation(ClosestEnemy->GetActorLocation());
		}
	}
}
