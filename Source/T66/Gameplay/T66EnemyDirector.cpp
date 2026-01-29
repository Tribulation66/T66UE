// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

AT66EnemyDirector::AT66EnemyDirector()
{
	PrimaryActorTick.bCanEverTick = false;
	EnemyClass = AT66EnemyBase::StaticClass();
}

void AT66EnemyDirector::BeginPlay()
{
	Super::BeginPlay();
	// First wave after 2s, then every SpawnIntervalSeconds
	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AT66EnemyDirector::SpawnWave, SpawnIntervalSeconds, true, 2.f);
}

void AT66EnemyDirector::NotifyEnemyDied(AT66EnemyBase* Enemy)
{
	if (Enemy && AliveCount > 0)
	{
		AliveCount--;
	}
}

void AT66EnemyDirector::SpawnWave()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn || !EnemyClass) return;

	int32 ToSpawn = FMath::Min(EnemiesPerWave, MaxAliveEnemies - AliveCount);
	if (ToSpawn <= 0) return;

	UWorld* World = GetWorld();
	for (int32 i = 0; i < ToSpawn; ++i)
	{
		FVector PlayerLoc = PlayerPawn->GetActorLocation();
		float Angle = FMath::RandRange(0.f, 2.f * PI);
		float Dist = FMath::RandRange(SpawnMinDistance, SpawnMaxDistance);
		FVector Offset(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
		FVector SpawnLoc = PlayerLoc + Offset;

		// Trace down for ground
		FHitResult Hit;
		FVector Start = SpawnLoc + FVector(0.f, 0.f, 500.f);
		FVector End = SpawnLoc - FVector(0.f, 0.f, 1000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			SpawnLoc = Hit.ImpactPoint + FVector(0.f, 0.f, 90.f);
		}
		else
		{
			SpawnLoc.Z = PlayerLoc.Z;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(EnemyClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (Enemy)
		{
			Enemy->OwningDirector = this;
			AliveCount++;
			UE_LOG(LogTemp, Log, TEXT("EnemyDirector: spawned enemy %d at (%.0f, %.0f, %.0f)"), AliveCount, SpawnLoc.X, SpawnLoc.Y, SpawnLoc.Z);
		}
	}
}
