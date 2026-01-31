// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66LeprechaunEnemy.h"
#include "Gameplay/T66GoblinThiefEnemy.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66UniqueDebuffEnemy.h"
#include "Core/T66Rarity.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

AT66EnemyDirector::AT66EnemyDirector()
{
	PrimaryActorTick.bCanEverTick = false;
	EnemyClass = AT66EnemyBase::StaticClass();
	LeprechaunClass = AT66LeprechaunEnemy::StaticClass();
	GoblinThiefClass = AT66GoblinThiefEnemy::StaticClass();
	UniqueEnemyClass = AT66UniqueDebuffEnemy::StaticClass();
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
	if (Enemy && ActiveMiniBoss.IsValid() && ActiveMiniBoss.Get() == Enemy)
	{
		ActiveMiniBoss = nullptr;
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
			bSpawnedUniqueThisStage = false;
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
		bSpawnedUniqueThisStage = false;
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

void AT66EnemyDirector::SpawnWave()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

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

	// Robust fallback: if EnemyClass is unset or misconfigured to a special enemy, use base enemy for the "regular" slot.
	TSubclassOf<AT66EnemyBase> RegularClass = EnemyClass;
	if (!RegularClass
		|| RegularClass->IsChildOf(AT66GoblinThiefEnemy::StaticClass())
		|| RegularClass->IsChildOf(AT66LeprechaunEnemy::StaticClass())
		|| RegularClass->IsChildOf(AT66UniqueDebuffEnemy::StaticClass()))
	{
		static bool bWarnedEnemyClass = false;
		if (!bWarnedEnemyClass)
		{
			bWarnedEnemyClass = true;
			const FString BadName = RegularClass ? RegularClass->GetName() : FString(TEXT("None"));
			UE_LOG(LogTemp, Warning, TEXT("EnemyDirector: EnemyClass is '%s' (invalid for regular). Falling back to AT66EnemyBase for regular spawns."), *BadName);
		}
		RegularClass = AT66EnemyBase::StaticClass();
	}

	// Stage mobs: pull exact roster from DT_Stages (EnemyA/B/C). Fallback is deterministic IDs.
	const int32 StageNum = RunState->GetCurrentStage();
	FName MobA = FName(*FString::Printf(TEXT("Mob_Stage%02d_A"), StageNum));
	FName MobB = FName(*FString::Printf(TEXT("Mob_Stage%02d_B"), StageNum));
	FName MobC = FName(*FString::Printf(TEXT("Mob_Stage%02d_C"), StageNum));
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
	{
		FStageData StageData;
		if (T66GI->GetStageData(StageNum, StageData))
		{
			if (!StageData.EnemyA.IsNone()) MobA = StageData.EnemyA;
			if (!StageData.EnemyB.IsNone()) MobB = StageData.EnemyB;
			if (!StageData.EnemyC.IsNone()) MobC = StageData.EnemyC;
		}
	}
	const TArray<FName> MobIDs = { MobA, MobB, MobC };

	const bool bCanSpawnMiniBoss = !ActiveMiniBoss.IsValid();
	const int32 MiniBossIndex = (bCanSpawnMiniBoss && (Rng.FRand() < MiniBossChancePerWave))
		? Rng.RandRange(0, FMath::Max(0, ToSpawn - 1))
		: INDEX_NONE;

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
		TSubclassOf<AT66EnemyBase> ClassToSpawn = RegularClass;
		const float Roll = Rng.FRand();
		if (LeprechaunClass && Roll < LeprechaunChance)
		{
			ClassToSpawn = LeprechaunClass;
		}
		else if (GoblinThiefClass && Roll < (LeprechaunChance + GoblinThiefChance))
		{
			ClassToSpawn = GoblinThiefClass;
		}

		const bool bIsMob = (ClassToSpawn == RegularClass);
		const bool bIsMiniBoss = bIsMob && (MiniBossIndex == i);
		const FName MobID = bIsMob ? MobIDs[Rng.RandRange(0, MobIDs.Num() - 1)] : NAME_None;

		AT66EnemyBase* Enemy = nullptr;
		if (bIsMob)
		{
			// Use deferred spawn so we can set mob visuals before BeginPlay.
			const FTransform Xform(FRotator::ZeroRotator, SpawnLoc);
			Enemy = World->SpawnActorDeferred<AT66EnemyBase>(ClassToSpawn, Xform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			if (Enemy)
			{
				Enemy->OwningDirector = this;
				Enemy->ConfigureAsMob(MobID);
				UGameplayStatics::FinishSpawningActor(Enemy, Xform);
			}
		}
		else
		{
			Enemy = World->SpawnActor<AT66EnemyBase>(ClassToSpawn, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		}
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

			if (RunState)
			{
				Enemy->ApplyDifficultyTier(RunState->GetDifficultyTier());
			}
			if (bIsMiniBoss)
			{
				Enemy->ApplyMiniBossMultipliers(MiniBossHPScalar, MiniBossDamageScalar, MiniBossScale);
				ActiveMiniBoss = Enemy;
			}
			AliveCount++;
			UE_LOG(LogTemp, Log, TEXT("EnemyDirector: spawned enemy %d Mob=%s MiniBoss=%d at (%.0f, %.0f, %.0f)"),
				AliveCount,
				*MobID.ToString(),
				bIsMiniBoss ? 1 : 0,
				SpawnLoc.X, SpawnLoc.Y, SpawnLoc.Z);
		}
	}

	// Unique enemy: one at a time, spawned as a pressure spike.
	if (UniqueEnemyClass && !ActiveUniqueEnemy.IsValid()
		&& (!bSpawnedUniqueThisStage || (Rng.FRand() < UniqueEnemyChancePerWave)))
	{
		FVector PlayerLoc = PlayerPawn->GetActorLocation();
		FVector SpawnLoc = PlayerLoc;

		// Try a few times to avoid spawning inside safe zones.
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
			if (!bInSafe) break;
		}

		// Trace down for ground.
		FHitResult Hit;
		FVector Start = SpawnLoc + FVector(0.f, 0.f, 500.f);
		FVector End = SpawnLoc - FVector(0.f, 0.f, 2000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			static constexpr float EnemyCapsuleHalfHeight = 88.f;
			SpawnLoc = Hit.ImpactPoint + FVector(0.f, 0.f, EnemyCapsuleHalfHeight);
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AT66EnemyBase* Unique = World->SpawnActor<AT66EnemyBase>(UniqueEnemyClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (Unique)
		{
			// Unique enemies shouldn't influence the director's AliveCount budget.
			Unique->OwningDirector = nullptr;
			if (RunState)
			{
				Unique->ApplyDifficultyTier(RunState->GetDifficultyTier());
			}
			ActiveUniqueEnemy = Unique;
			bSpawnedUniqueThisStage = true;
		}
	}
}
