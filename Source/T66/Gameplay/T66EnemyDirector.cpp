// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66LeprechaunEnemy.h"
#include "Gameplay/T66GoblinThiefEnemy.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Core/T66Rarity.h"
#include "Core/T66RunStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

AT66EnemyDirector::AT66EnemyDirector()
{
	PrimaryActorTick.bCanEverTick = false;
	EnemyClass = AT66EnemyBase::StaticClass();
	LeprechaunClass = AT66LeprechaunEnemy::StaticClass();
	GoblinThiefClass = AT66GoblinThiefEnemy::StaticClass();
}

void AT66EnemyDirector::BeginPlay()
{
	Super::BeginPlay();

	// Only begin spawning once the Stage Timer becomes active (after Start Gate).
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->StageTimerChanged.AddDynamic(this, &AT66EnemyDirector::HandleStageTimerChanged);
			HandleStageTimerChanged(); // initial state
		}
	}
}

void AT66EnemyDirector::NotifyEnemyDied(AT66EnemyBase* Enemy)
{
	if (Enemy && AliveCount > 0)
	{
		AliveCount--;
	}
}

void AT66EnemyDirector::HandleStageTimerChanged()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	if (RunState->GetStageTimerActive())
	{
		if (!bSpawningArmed)
		{
			bSpawningArmed = true;
			// Spawn immediately so players don't think spawns are broken, then continue on interval.
			SpawnWave();
			World->GetTimerManager().ClearTimer(SpawnTimerHandle);
			World->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AT66EnemyDirector::SpawnWave, SpawnIntervalSeconds, true, 2.f);
		}
	}
	else
	{
		// Timer frozen: don't spawn waves.
		bSpawningArmed = false;
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

void AT66EnemyDirector::SpawnWave()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn || !EnemyClass) return;

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	// Only start spawning once the stage timer is active (i.e. after start gate / start pillars).
	if (!RunState || !RunState->GetStageTimerActive())
	{
		return;
	}

	int32 ToSpawn = FMath::Min(EnemiesPerWave, MaxAliveEnemies - AliveCount);
	if (ToSpawn <= 0) return;

	UWorld* World = GetWorld();
	FRandomStream Rng(static_cast<int32>(FPlatformTime::Cycles()));
	for (int32 i = 0; i < ToSpawn; ++i)
	{
		FVector PlayerLoc = PlayerPawn->GetActorLocation();
		FVector SpawnLoc = PlayerLoc;
		// Try a few times to avoid spawning inside safe zones
		for (int32 Try = 0; Try < 6; ++Try)
		{
			float Angle = FMath::RandRange(0.f, 2.f * PI);
			float Dist = FMath::RandRange(SpawnMinDistance, SpawnMaxDistance);
			FVector Offset(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
			SpawnLoc = PlayerLoc + Offset;

			bool bInSafe = false;
			for (TActorIterator<AT66HouseNPCBase> It(World); It; ++It)
			{
				AT66HouseNPCBase* NPC = *It;
				if (!NPC) continue;
				const float R = NPC->GetSafeZoneRadius();
				if (FVector::DistSquared2D(SpawnLoc, NPC->GetActorLocation()) < (R * R))
				{
					bInSafe = true;
					break;
				}
			}
			if (!bInSafe)
			{
				break;
			}
		}

		// Trace down for ground
		FHitResult Hit;
		FVector Start = SpawnLoc + FVector(0.f, 0.f, 500.f);
		FVector End = SpawnLoc - FVector(0.f, 0.f, 1000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			// Spawn at ground + capsule half-height (so actor is grounded).
			static constexpr float EnemyCapsuleHalfHeight = 88.f;
			SpawnLoc = Hit.ImpactPoint + FVector(0.f, 0.f, EnemyCapsuleHalfHeight);
		}
		else
		{
			SpawnLoc.Z = PlayerLoc.Z;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		// Pick which enemy type to spawn.
		TSubclassOf<AT66EnemyBase> ClassToSpawn = EnemyClass;
		const float Roll = Rng.FRand();
		if (LeprechaunClass && Roll < LeprechaunChance)
		{
			ClassToSpawn = LeprechaunClass;
		}
		else if (GoblinThiefClass && Roll < (LeprechaunChance + GoblinThiefChance))
		{
			ClassToSpawn = GoblinThiefClass;
		}

		AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(ClassToSpawn, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (Enemy)
		{
			// Special enemies roll rarity and apply visuals/effects.
			const ET66Rarity R = FT66RarityUtil::RollDefaultRarity(Rng);
			if (AT66LeprechaunEnemy* Lep = Cast<AT66LeprechaunEnemy>(Enemy))
			{
				Lep->SetRarity(R);
			}
			else if (AT66GoblinThiefEnemy* Gob = Cast<AT66GoblinThiefEnemy>(Enemy))
			{
				Gob->SetRarity(R);
			}

			Enemy->OwningDirector = this;
			if (RunState)
			{
				Enemy->ApplyDifficultyTier(RunState->GetDifficultyTier());
			}
			AliveCount++;
			UE_LOG(LogTemp, Log, TEXT("EnemyDirector: spawned enemy %d at (%.0f, %.0f, %.0f)"), AliveCount, SpawnLoc.X, SpawnLoc.Y, SpawnLoc.Z);
		}
	}
}
