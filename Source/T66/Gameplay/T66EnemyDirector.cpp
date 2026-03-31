// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66GoblinThiefEnemy.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Core/T66GameplayLayout.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
// [GOLD] EngineUtils.h removed — TActorIterator replaced by ActorRegistry.
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66EnemyPoolSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66EnemyDirector, Log, All);

AT66EnemyDirector::AT66EnemyDirector()
{
	PrimaryActorTick.bCanEverTick = false;
	EnemyClass = AT66EnemyBase::StaticClass();
	GoblinThiefClass = AT66GoblinThiefEnemy::StaticClass();
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
		World->GetTimerManager().ClearTimer(StaggeredSpawnTimerHandle);
	}
	PendingSpawns.Empty();

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

void AT66EnemyDirector::SetSpawningPaused(bool bPaused)
{
	if (bSpawningPaused == bPaused)
	{
		return;
	}

	bSpawningPaused = bPaused;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
		World->GetTimerManager().ClearTimer(StaggeredSpawnTimerHandle);
	}

	bSpawningArmed = false;
	if (bSpawningPaused)
	{
		PendingSpawns.Empty();
		return;
	}

	HandleStageTimerChanged();
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
			SpawnWave();
			World->GetTimerManager().ClearTimer(SpawnTimerHandle);
			World->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AT66EnemyDirector::SpawnWave, SpawnIntervalSeconds, true);
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
	if (bSpawningPaused) return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	// Only start spawning once the stage timer is active (i.e. after the main-area entrance trigger).
	if (!RunState || !RunState->GetStageTimerActive())
	{
		return;
	}

	// Global density pass: triple combat spawns on top of the existing difficulty scalar.
	static constexpr float SpawnDensityMultiplier = 3.0f;

	// Difficulty scaling affects enemy count (waves + max alive).
	const float Scalar = RunState->GetDifficultyScalar();
	int32 EffectivePerWave = FMath::Clamp(
		FMath::RoundToInt(static_cast<float>(FMath::Max(1, BaseEnemiesPerWave)) * Scalar * SpawnDensityMultiplier),
		1,
		600);
	int32 EffectiveMaxAlive = FMath::Clamp(
		FMath::RoundToInt(static_cast<float>(FMath::Max(1, BaseMaxAliveEnemies)) * Scalar * SpawnDensityMultiplier),
		1,
		1500);
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

	// [GOLD] Use the actor registry for safe-zone NPC lookup (replaces TActorIterator cache).
	UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;

	auto IsInAnySafeZone2D = [&](const FVector& Loc) -> bool
	{
		if (!Registry) return false;
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Registry->GetNPCs())
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

	auto IsInBlockedTraversalZone2D = [](const FVector& Loc) -> bool
	{
		return T66GameplayLayout::IsInsideReservedTraversalZone2D(Loc, 120.f);
	};

	// Robust fallback: if EnemyClass is unset or misconfigured to a special enemy, use base enemy for the "regular" slot.
	TSubclassOf<AT66EnemyBase> RegularClass = EnemyClass;
	if (!RegularClass
		|| RegularClass->IsChildOf(AT66GoblinThiefEnemy::StaticClass()))
	{
		static bool bWarnedEnemyClass = false;
		if (!bWarnedEnemyClass)
		{
			bWarnedEnemyClass = true;
			const FString BadName = RegularClass ? RegularClass->GetName() : FString(TEXT("None"));
			UE_LOG(LogT66EnemyDirector, Warning, TEXT("EnemyDirector: EnemyClass is '%s' (invalid for regular). Falling back to AT66EnemyBase for regular spawns."), *BadName);
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

#if !UE_BUILD_SHIPPING
	static int32 LoggedMobWaves = 0;
	if (LoggedMobWaves < 3)
	{
		++LoggedMobWaves;
		UE_LOG(LogT66EnemyDirector, Warning, TEXT("[SPAWN] SpawnWave Stage=%d MobIDs: A=%s  B=%s  C=%s (generic fallback would be Mob_StageXX_X — if you see that, reimport DT_Stages)"),
			StageNum, *MobA.ToString(), *MobB.ToString(), *MobC.ToString());
	}
#endif

	// ================================
	// Special mobs (Goblin Thief) — per-wave counts (Luck-affected)
	// ================================
	int32 GobToSpawn = 0;
	if (Tuning)
	{
		const float GobChance = RngSub ? RngSub->BiasChance01(Tuning->GoblinWaveChanceBase) : FMath::Clamp(Tuning->GoblinWaveChanceBase, 0.f, 1.f);

		if (GoblinThiefClass && (Rng.GetFraction() < GobChance))
		{
			GobToSpawn = RngSub ? RngSub->RollIntRangeBiased(Tuning->GoblinCountPerWave, Rng) : Rng.RandRange(Tuning->GoblinCountPerWave.Min, Tuning->GoblinCountPerWave.Max);
		}

		GobToSpawn = FMath::Max(0, GobToSpawn);
		GobToSpawn = FMath::Clamp(GobToSpawn, 0, ToSpawn);
	}

	// Luck Rating tracking (quantity): per-wave special mob counts.
	if (RunState && Tuning)
	{
		RunState->RecordLuckQuantityRoll(FName(TEXT("GoblinCountPerWave")), GobToSpawn, 0, Tuning->GoblinCountPerWave.Max);
	}

	// Build the exact spawn plan for this wave.
	const int32 MobToSpawn = FMath::Max(0, ToSpawn - GobToSpawn);
	TArray<TSubclassOf<AT66EnemyBase>> SpawnPlan;
	SpawnPlan.Reserve(ToSpawn);
	for (int32 i = 0; i < MobToSpawn; ++i) SpawnPlan.Add(RegularClass);
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

	// Spawn always outside hero attack range (use scaled range so scale stat is respected).
	float EffectiveSpawnMin = SpawnMinDistance;
	float EffectiveSpawnMax = SpawnMaxDistance;
	if (RunState)
	{
		const float ActualAttackRange = RunState->GetSecondaryStatValue(ET66SecondaryStatType::AttackRange);
		const float AttackRange = FMath::Max(400.f, ActualAttackRange);
		EffectiveSpawnMin = AttackRange * 1.25f;  // always outside range
		EffectiveSpawnMax = EffectiveSpawnMin + 400.f;
	}

	const FVector PlayerLoc = PlayerPawn->GetActorLocation();
	auto TraceGroundZAtXY = [&](const FVector& XYLoc, float& OutGroundZ) -> bool
	{
		FHitResult Hit;
		const FVector TraceStart(XYLoc.X, XYLoc.Y, XYLoc.Z + 6000.f);
		const FVector TraceEnd(XYLoc.X, XYLoc.Y, XYLoc.Z - 20000.f);
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			OutGroundZ = Hit.ImpactPoint.Z;
			return true;
		}
		return false;
	};

	float PlayerGroundZ = GetActorLocation().Z;
	if (!TraceGroundZAtXY(PlayerLoc, PlayerGroundZ))
	{
		PlayerGroundZ = GetActorLocation().Z;
	}

	auto EnforceMinSpawnDistance2D = [&](FVector& InOutLoc)
	{
		FVector FromPlayer = InOutLoc - PlayerLoc;
		FromPlayer.Z = 0.f;
		const float Dist = FromPlayer.Size();
		if (Dist >= EffectiveSpawnMin)
		{
			return;
		}

		FVector Dir = (Dist > 1.f) ? (FromPlayer / Dist) : FVector::ZeroVector;
		if (Dir.IsNearlyZero())
		{
			Dir = FVector(Rng.FRandRange(-1.f, 1.f), Rng.FRandRange(-1.f, 1.f), 0.f);
			if (!Dir.Normalize())
			{
				Dir = FVector(1.f, 0.f, 0.f);
			}
		}

		InOutLoc = PlayerLoc + (Dir * EffectiveSpawnMin);
		InOutLoc.Z = PlayerGroundZ;
	};

	auto ResolveGroundedSpawnLocation = [&](const FVector& XYLoc) -> FVector
	{
		static constexpr float EnemyCapsuleHalfHeight = 88.f;
		float GroundZ = PlayerGroundZ;
		if (TraceGroundZAtXY(XYLoc, GroundZ))
		{
			return FVector(XYLoc.X, XYLoc.Y, GroundZ + EnemyCapsuleHalfHeight);
		}
		return FVector(XYLoc.X, XYLoc.Y, PlayerGroundZ + EnemyCapsuleHalfHeight);
	};

	PendingSpawns.Empty();
	for (int32 i = 0; i < SpawnPlan.Num(); ++i)
	{
		FVector SpawnLoc(PlayerLoc.X, PlayerLoc.Y, PlayerGroundZ);
		bool bFoundSpawnLoc = false;
		// Try a few times to avoid spawning inside safe zones or reserved traversal spaces.
		for (int32 Try = 0; Try < 12; ++Try)
		{
			const float Angle = Rng.FRandRange(0.f, 2.f * PI);
			const float Dist = Rng.FRandRange(EffectiveSpawnMin, EffectiveSpawnMax);
			FVector Offset(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
			SpawnLoc = FVector(PlayerLoc.X + Offset.X, PlayerLoc.Y + Offset.Y, PlayerGroundZ);

			if (!IsInAnySafeZone2D(SpawnLoc) && !IsInBlockedTraversalZone2D(SpawnLoc))
			{
				bFoundSpawnLoc = true;
				break;
			}
		}
		// Guarantee: if still inside a safe zone, push outward past nearest NPC radius
		if (Registry && IsInAnySafeZone2D(SpawnLoc))
		{
			static constexpr float SafeZonePushMargin = 50.f;
			for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Registry->GetNPCs())
			{
				AT66HouseNPCBase* NPC = WeakNPC.Get();
				if (!NPC) continue;
				const float R = NPC->GetSafeZoneRadius();
				FVector ToSpawnPt = SpawnLoc - NPC->GetActorLocation();
				ToSpawnPt.Z = 0.f;
				const float Dist2D = ToSpawnPt.Size();
				if (Dist2D < R && Dist2D > 1.f)
				{
					FVector Dir = ToSpawnPt / Dist2D;
					SpawnLoc = NPC->GetActorLocation() + FVector(Dir.X, Dir.Y, 0.f) * (R + SafeZonePushMargin);
					SpawnLoc.Z = PlayerGroundZ;
					break;
				}
				else if (Dist2D <= 1.f)
				{
					FVector Dir(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), 0.f);
					Dir.Z = 0.f;
					if (Dir.Normalize())
					{
						SpawnLoc = NPC->GetActorLocation() + Dir * (R + SafeZonePushMargin);
						SpawnLoc.Z = PlayerGroundZ;
					}
					break;
				}
			}
		}

		TSubclassOf<AT66EnemyBase> ClassToSpawn = SpawnPlan[i];
		if (!ClassToSpawn)
		{
			ClassToSpawn = RegularClass;
		}

		EnforceMinSpawnDistance2D(SpawnLoc);
		SpawnLoc = ResolveGroundedSpawnLocation(SpawnLoc);
		if (IsInBlockedTraversalZone2D(SpawnLoc))
		{
			if (!bFoundSpawnLoc)
			{
				continue;
			}

			bool bResolvedBlockedZone = false;
			for (int32 Retry = 0; Retry < 8; ++Retry)
			{
				const float Angle = Rng.FRandRange(0.f, 2.f * PI);
				const float Dist = Rng.FRandRange(EffectiveSpawnMin, EffectiveSpawnMax + 400.f);
				const FVector Offset(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
				FVector RetryLoc = ResolveGroundedSpawnLocation(FVector(PlayerLoc.X + Offset.X, PlayerLoc.Y + Offset.Y, PlayerGroundZ));
				if (IsInAnySafeZone2D(RetryLoc) || IsInBlockedTraversalZone2D(RetryLoc))
				{
					continue;
				}

				SpawnLoc = RetryLoc;
				bResolvedBlockedZone = true;
				break;
			}

			if (!bResolvedBlockedZone)
			{
				continue;
			}
		}

		const bool bIsMob = (ClassToSpawn == RegularClass);
		const bool bIsMiniBossSlot = bIsMob && (MiniBossIndex == i);
		const FName MobID = bIsMob ? MobIDs[Rng.RandRange(0, MobIDs.Num() - 1)] : NAME_None;

		FPendingEnemySpawn Slot;
		Slot.GroundLocation = SpawnLoc;
		Slot.ClassToSpawn = ClassToSpawn;
		Slot.MobID = MobID;
		Slot.bIsMiniBoss = bIsMiniBossSlot;
		Slot.DifficultyScalar = Scalar;
		Slot.StageNum = RunState ? RunState->GetCurrentStage() : 1;
		PendingSpawns.Add(Slot);
	}

	SpawnNextStaggeredBatch();
}

void AT66EnemyDirector::SpawnNextStaggeredBatch()
{
	if (PendingSpawns.Num() <= 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66EnemyPoolSubsystem* EnemyPool = World->GetSubsystem<UT66EnemyPoolSubsystem>();
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	const UT66RngTuningConfig* Tuning = RngSub ? RngSub->GetTuning() : nullptr;
	FRandomStream Rng(static_cast<int32>(FPlatformTime::Cycles()));

	int32 ProcessedCount = 0;
	while (ProcessedCount < PendingSpawns.Num())
	{
		const FPendingEnemySpawn Slot = PendingSpawns[ProcessedCount];
		++ProcessedCount;

		const bool bIsMob = (Slot.ClassToSpawn == EnemyClass);
		AT66EnemyBase* Enemy = nullptr;

		if (bIsMob)
		{
			if (EnemyPool)
			{
				Enemy = EnemyPool->TryAcquire(Slot.ClassToSpawn, Slot.GroundLocation);
			}
			if (Enemy)
			{
				Enemy->ResetForReuse(Slot.GroundLocation, this);
				Enemy->ConfigureAsMob(Slot.MobID);
			}
			else
			{
				const FTransform Xform(FRotator::ZeroRotator, Slot.GroundLocation);
				Enemy = World->SpawnActorDeferred<AT66EnemyBase>(Slot.ClassToSpawn, Xform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
				if (Enemy)
				{
					Enemy->OwningDirector = this;
					Enemy->ConfigureAsMob(Slot.MobID);
					UGameplayStatics::FinishSpawningActor(Enemy, Xform);
				}
			}
		}
		else
		{
			if (EnemyPool)
			{
				Enemy = EnemyPool->TryAcquire(Slot.ClassToSpawn, Slot.GroundLocation);
				if (Enemy)
				{
					Enemy->ResetForReuse(Slot.GroundLocation, this);
				}
			}
			if (!Enemy)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
				Enemy = World->SpawnActor<AT66EnemyBase>(Slot.ClassToSpawn, Slot.GroundLocation, FRotator::ZeroRotator, SpawnParams);
			}
		}

		if (Enemy)
		{
			const FT66RarityWeights Weights = Tuning ? Tuning->SpecialEnemyRarityBase : FT66RarityWeights{};
			const ET66Rarity R = (RngSub && Tuning) ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
			if (AT66GoblinThiefEnemy* Gob = Cast<AT66GoblinThiefEnemy>(Enemy))
			{
				Gob->SetRarity(R);
				if (RunState)
				{
					RunState->RecordLuckQualityRarity(FName(TEXT("GoblinRarity")), R);
				}
			}

			if (RunState)
			{
				Enemy->ApplyStageScaling(Slot.StageNum);
				Enemy->ApplyDifficultyScalar(Slot.DifficultyScalar);
			}
			if (Slot.bIsMiniBoss)
			{
				Enemy->ApplyMiniBossMultipliers(MiniBossHPScalar, MiniBossDamageScalar, MiniBossScale);
				ActiveMiniBoss = Enemy;
			}
			AliveCount++;
			Enemy->StartRiseFromGround(Slot.GroundLocation.Z);
		}
	}

	if (ProcessedCount > 0)
	{
		PendingSpawns.RemoveAt(0, ProcessedCount, EAllowShrinking::No);
	}
}
