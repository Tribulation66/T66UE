// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66LeprechaunEnemy.h"
#include "Gameplay/T66GoblinThiefEnemy.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66UniqueDebuffEnemy.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngSubsystem.h"
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

	// Cache base counts (used for difficulty scaling).
	BaseEnemiesPerWave = FMath::Max(1, EnemiesPerWave);
	BaseMaxAliveEnemies = FMath::Max(1, MaxAliveEnemies);

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

void AT66EnemyDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind from long-lived RunState delegates and stop timers.
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->StageTimerChanged.RemoveDynamic(this, &AT66EnemyDirector::HandleStageTimerChanged);
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
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

	if (!bSpawningPaused && RunState->GetStageTimerActive())
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
	if (bSpawningPaused) return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	// Only start spawning once the stage timer is active (i.e. after start gate / start pillars).
	if (!RunState || !RunState->GetStageTimerActive())
	{
		return;
	}

	// Difficulty scaling affects enemy count (waves + max alive).
	const float Scalar = RunState->GetDifficultyScalar();
	int32 EffectivePerWave = FMath::Clamp(FMath::RoundToInt(static_cast<float>(FMath::Max(1, BaseEnemiesPerWave)) * Scalar), 1, 200);
	int32 EffectiveMaxAlive = FMath::Clamp(FMath::RoundToInt(static_cast<float>(FMath::Max(1, BaseMaxAliveEnemies)) * Scalar), 1, 500);
	EffectiveMaxAlive = FMath::Max(EffectiveMaxAlive, EffectivePerWave);

	int32 ToSpawn = FMath::Min(EffectivePerWave, EffectiveMaxAlive - AliveCount);
	if (ToSpawn <= 0) return;

	UWorld* World = GetWorld();
	FRandomStream Rng(static_cast<int32>(FPlatformTime::Cycles()));

	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	if (RngSub && RunState)
	{
		RngSub->UpdateLuckStat(RunState->GetLuckStat());
	}
	const UT66RngTuningConfig* Tuning = RngSub ? RngSub->GetTuning() : nullptr;

	// Cache safe-zone NPCs per-world so we don't do repeated full actor iterators during spawn waves.
	struct FSafeNPCache
	{
		TWeakObjectPtr<UWorld> World;
		double LastRefreshSeconds = -1.0;
		TArray<TWeakObjectPtr<AT66HouseNPCBase>> NPCs;
	};
	static FSafeNPCache SafeCache;

	auto RefreshSafeCacheIfNeeded = [&]()
	{
		if (!World) return;
		const double NowSeconds = FPlatformTime::Seconds();
		const bool bWorldChanged = (SafeCache.World.Get() != World);
		const bool bNeedsRefresh = bWorldChanged || (SafeCache.LastRefreshSeconds < 0.0) || ((NowSeconds - SafeCache.LastRefreshSeconds) > 2.0);
		if (!bNeedsRefresh) return;

		SafeCache.World = World;
		SafeCache.LastRefreshSeconds = NowSeconds;
		SafeCache.NPCs.Reset();
		for (TActorIterator<AT66HouseNPCBase> It(World); It; ++It)
		{
			if (AT66HouseNPCBase* NPC = *It)
			{
				SafeCache.NPCs.Add(NPC);
			}
		}
	};

	auto IsInAnySafeZone2D = [&](const FVector& Loc) -> bool
	{
		RefreshSafeCacheIfNeeded();
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : SafeCache.NPCs)
		{
			AT66HouseNPCBase* NPC = WeakNPC.Get();
			if (!NPC) continue;
			const float R = NPC->GetSafeZoneRadius();
			if (FVector::DistSquared2D(Loc, NPC->GetActorLocation()) < (R * R))
			{
				return true;
			}
		}
		return false;
	};

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

	// ================================
	// Special mobs (Goblin Thief / Leprechaun) — per-wave counts (Luck-affected)
	// ================================
	int32 GobToSpawn = 0;
	int32 LepToSpawn = 0;
	if (Tuning)
	{
		// Each special has its own "wave appears" chance, then a count roll.
		const float GobChance = RngSub ? RngSub->BiasChance01(Tuning->GoblinWaveChanceBase) : FMath::Clamp(Tuning->GoblinWaveChanceBase, 0.f, 1.f);
		const float LepChance = RngSub ? RngSub->BiasChance01(Tuning->LeprechaunWaveChanceBase) : FMath::Clamp(Tuning->LeprechaunWaveChanceBase, 0.f, 1.f);

		if (GoblinThiefClass && (Rng.GetFraction() < GobChance))
		{
			GobToSpawn = RngSub ? RngSub->RollIntRangeBiased(Tuning->GoblinCountPerWave, Rng) : Rng.RandRange(Tuning->GoblinCountPerWave.Min, Tuning->GoblinCountPerWave.Max);
		}
		if (LeprechaunClass && (Rng.GetFraction() < LepChance))
		{
			LepToSpawn = RngSub ? RngSub->RollIntRangeBiased(Tuning->LeprechaunCountPerWave, Rng) : Rng.RandRange(Tuning->LeprechaunCountPerWave.Min, Tuning->LeprechaunCountPerWave.Max);
		}

		GobToSpawn = FMath::Max(0, GobToSpawn);
		LepToSpawn = FMath::Max(0, LepToSpawn);

		// Fit specials into the wave budget (they replace regular spawns, not add extra).
		GobToSpawn = FMath::Clamp(GobToSpawn, 0, ToSpawn);
		LepToSpawn = FMath::Clamp(LepToSpawn, 0, ToSpawn - GobToSpawn);
	}

	// Luck Rating tracking (quantity): per-wave special mob counts (including 0 when not present).
	if (RunState && Tuning)
	{
		RunState->RecordLuckQuantityRoll(FName(TEXT("GoblinCountPerWave")), GobToSpawn, 0, Tuning->GoblinCountPerWave.Max);
		RunState->RecordLuckQuantityRoll(FName(TEXT("LeprechaunCountPerWave")), LepToSpawn, 0, Tuning->LeprechaunCountPerWave.Max);
	}

	// Build the exact spawn plan for this wave.
	const int32 MobToSpawn = FMath::Max(0, ToSpawn - GobToSpawn - LepToSpawn);
	TArray<TSubclassOf<AT66EnemyBase>> SpawnPlan;
	SpawnPlan.Reserve(ToSpawn);
	for (int32 i = 0; i < MobToSpawn; ++i) SpawnPlan.Add(RegularClass);
	for (int32 i = 0; i < LepToSpawn; ++i) SpawnPlan.Add(LeprechaunClass);
	for (int32 i = 0; i < GobToSpawn; ++i) SpawnPlan.Add(GoblinThiefClass);

	// Shuffle plan so specials aren't always in the same slots (this shuffle is not luck-affected; it uses the wave RNG only).
	for (int32 i = SpawnPlan.Num() - 1; i > 0; --i)
	{
		const int32 j = Rng.RandRange(0, i);
		SpawnPlan.Swap(i, j);
	}

	// Mini-boss: choose one of the *mob* spawns, not specials.
	const bool bCanSpawnMiniBoss = !ActiveMiniBoss.IsValid();
	int32 MiniBossIndex = INDEX_NONE;
	if (bCanSpawnMiniBoss && (Rng.FRand() < MiniBossChancePerWave) && MobToSpawn > 0)
	{
		TArray<int32> MobIndices;
		MobIndices.Reserve(MobToSpawn);
		for (int32 i = 0; i < SpawnPlan.Num(); ++i)
		{
			if (SpawnPlan[i] == RegularClass)
			{
				MobIndices.Add(i);
			}
		}
		if (MobIndices.Num() > 0)
		{
			MiniBossIndex = MobIndices[Rng.RandRange(0, MobIndices.Num() - 1)];
		}
	}

	for (int32 i = 0; i < SpawnPlan.Num(); ++i)
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

			if (!IsInAnySafeZone2D(SpawnLoc))
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
		TSubclassOf<AT66EnemyBase> ClassToSpawn = SpawnPlan[i];
		if (!ClassToSpawn)
		{
			ClassToSpawn = RegularClass;
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
			const FT66RarityWeights Weights = Tuning ? Tuning->SpecialEnemyRarityBase : FT66RarityWeights{};
			const ET66Rarity R = (RngSub && Tuning) ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
			if (AT66LeprechaunEnemy* Lep = Cast<AT66LeprechaunEnemy>(Enemy))
			{
				Lep->SetRarity(R);
				if (RunState)
				{
					RunState->RecordLuckQualityRarity(FName(TEXT("LeprechaunRarity")), R);
				}
			}
			else if (AT66GoblinThiefEnemy* Gob = Cast<AT66GoblinThiefEnemy>(Enemy))
			{
				Gob->SetRarity(R);
				if (RunState)
				{
					RunState->RecordLuckQualityRarity(FName(TEXT("GoblinRarity")), R);
				}
			}

			if (RunState)
			{
				Enemy->ApplyStageScaling(RunState->GetCurrentStage());
				Enemy->ApplyDifficultyScalar(Scalar);
			}
			if (bIsMiniBoss)
			{
				Enemy->ApplyMiniBossMultipliers(MiniBossHPScalar, MiniBossDamageScalar, MiniBossScale);
				ActiveMiniBoss = Enemy;
			}
			AliveCount++;
#if !UE_BUILD_SHIPPING
			// Avoid log spam in wave loops (especially with many spawns).
			static int32 SpawnLogsRemaining = 12;
			if (SpawnLogsRemaining > 0)
			{
				--SpawnLogsRemaining;
				UE_LOG(LogTemp, Log, TEXT("EnemyDirector: spawned enemy %d Mob=%s MiniBoss=%d at (%.0f, %.0f, %.0f)"),
					AliveCount,
					*MobID.ToString(),
					bIsMiniBoss ? 1 : 0,
					SpawnLoc.X, SpawnLoc.Y, SpawnLoc.Z);
			}
#endif
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

			if (!IsInAnySafeZone2D(SpawnLoc)) break;
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
				Unique->ApplyStageScaling(RunState->GetCurrentStage());
				Unique->ApplyDifficultyScalar(Scalar);
			}
			ActiveUniqueEnemy = Unique;
			bSpawnedUniqueThisStage = true;
		}
	}
}
