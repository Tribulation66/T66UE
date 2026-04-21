// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66CircusInteractable.h"
#include "Gameplay/Enemies/T66EnemyFamilyResolver.h"
#include "Gameplay/Enemies/T66MeleeEnemy.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66GoblinThiefEnemy.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66TowerMapTerrain.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Core/T66GameplayLayout.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66StageProgressionSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
// [GOLD] EngineUtils.h removed — TActorIterator replaced by ActorRegistry.
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66EnemyPoolSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66EnemyDirector, Log, All);

namespace
{
	static constexpr bool T66EnableTowerEnemySpawns = true;
	static constexpr int32 T66TowerTargetInitialEnemiesPerGameplayFloor = 4;
	static constexpr float T66TowerTargetRuntimeSpawnIntervalSeconds = 9.0f;
	static constexpr int32 T66TowerTargetEnemiesPerWave = 1;
	static constexpr int32 T66TowerTargetMaxAliveEnemies = 12;

	static void T66ResolveStageMobIDs(UGameInstance* GI, const int32 StageNum, TArray<FName>& OutMobIDs)
	{
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

		OutMobIDs = { MobA, MobB, MobC };
	}
}

AT66EnemyDirector::AT66EnemyDirector()
{
	PrimaryActorTick.bCanEverTick = false;
	EnemyClass = AT66MeleeEnemy::StaticClass();
	GoblinThiefClass = AT66GoblinThiefEnemy::StaticClass();
}

void AT66EnemyDirector::BeginPlay()
{
	Super::BeginPlay();

	// Cache base counts (used for difficulty scaling).
	BaseEnemiesPerWave = FMath::Max(1, EnemiesPerWave);
	BaseMaxAliveEnemies = FMath::Max(1, MaxAliveEnemies);
	BaseSpawnIntervalSeconds = FMath::Max(0.10f, SpawnIntervalSeconds);

	if (AT66GameMode* GameMode = GetWorld() ? Cast<AT66GameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
	{
		if (T66EnableTowerEnemySpawns && GameMode->IsUsingTowerMainMapLayout())
		{
			SpawnInitialPopulationForStage();
		}
	}

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

void AT66EnemyDirector::SpawnInitialPopulationForStage()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AT66GameMode* GameMode = Cast<AT66GameMode>(World->GetAuthGameMode());
	if (!GameMode || !GameMode->IsUsingTowerMainMapLayout())
	{
		return;
	}

	T66TowerMapTerrain::FLayout TowerLayout;
	if (!GameMode->GetTowerMainMapLayout(TowerLayout))
	{
		return;
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66StageProgressionSubsystem* StageProgression = GI ? GI->GetSubsystem<UT66StageProgressionSubsystem>() : nullptr;
	const int32 StageNum = RunState ? RunState->GetCurrentStage() : 1;
	if (StageProgression)
	{
		StageProgression->RefreshSnapshot(false);
	}

	const FT66StageProgressionSnapshot Snapshot = StageProgression
		? StageProgression->GetCurrentSnapshot()
		: FT66StageProgressionSnapshot{};

	TSubclassOf<AT66EnemyBase> RegularClass = EnemyClass;
	if (!RegularClass)
	{
		RegularClass = AT66MeleeEnemy::StaticClass();
	}

	TArray<FName> MobIDs;
	T66ResolveStageMobIDs(GI, StageNum, MobIDs);
	if (MobIDs.Num() <= 0)
	{
		return;
	}

	FRandomStream Rng((StageNum * 4051) + 177);
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
	{
		if (T66GI->RunSeed != 0)
		{
			Rng.Initialize(T66GI->RunSeed + StageNum * 4051 + 177);
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
	{
		if (!Floor.bGameplayFloor)
		{
			continue;
		}

		const int32 InitialPopulationCount = FMath::Clamp(
			T66TowerTargetInitialEnemiesPerGameplayFloor,
			0,
			64);
		for (int32 SpawnIndex = 0; SpawnIndex < InitialPopulationCount; ++SpawnIndex)
		{
			FVector SpawnLoc = FVector::ZeroVector;
			bool bFoundLocation = false;
			for (int32 Attempt = 0; Attempt < 24; ++Attempt)
			{
				if (T66TowerMapTerrain::TryGetRandomSurfaceLocationOnFloor(
					World,
					TowerLayout,
					Floor.FloorNumber,
					Rng,
					SpawnLoc,
					InitialTowerSpawnEdgePadding,
					InitialTowerSpawnHolePadding))
				{
					bFoundLocation = true;
					break;
				}
			}

			if (!bFoundLocation)
			{
				continue;
			}

			const FName MobID = MobIDs[Rng.RandRange(0, MobIDs.Num() - 1)];
			const TSubclassOf<AT66EnemyBase> MobClass = FT66EnemyFamilyResolver::ResolveEnemyClass(MobID, RegularClass);
			const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLoc);
			AT66EnemyBase* Enemy = World->SpawnActorDeferred<AT66EnemyBase>(
				MobClass,
				SpawnTransform,
				this,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			if (!Enemy)
			{
				continue;
			}

			Enemy->OwningDirector = this;
			Enemy->ConfigureAsMob(MobID);
			UGameplayStatics::FinishSpawningActor(Enemy, SpawnTransform);
			if (RunState)
			{
				Enemy->ApplyStageScaling(StageNum);
				Enemy->ApplyDifficultyScalar(RunState->GetDifficultyScalar());
				Enemy->ApplyProgressionEnemyScalar(Snapshot.EnemyStatScalar);
				Enemy->ApplyFinaleScaling(RunState->GetFinalSurvivalEnemyScalar());
			}

			++AliveCount;
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

void AT66EnemyDirector::RefreshSpawningFromProgression()
{
	UWorld* World = GetWorld();
	if (!World || bSpawningPaused || !bSpawningArmed)
	{
		return;
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66StageProgressionSubsystem* StageProgression = GI ? GI->GetSubsystem<UT66StageProgressionSubsystem>() : nullptr;
	if (!RunState || !RunState->GetStageTimerActive())
	{
		return;
	}

	if (StageProgression)
	{
		StageProgression->RefreshSnapshot(false);
	}

	const FT66StageProgressionSnapshot Snapshot = StageProgression
		? StageProgression->GetCurrentSnapshot()
		: FT66StageProgressionSnapshot{};
	const float RuntimeSpawnInterval = FMath::Max(
		0.15f,
		(BaseSpawnIntervalSeconds * Snapshot.RuntimeTrickleIntervalScalar) / FMath::Max(1.0f, Snapshot.DifficultyScalar));

	World->GetTimerManager().ClearTimer(SpawnTimerHandle);
	World->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AT66EnemyDirector::SpawnRuntimeTrickleWave,
		RuntimeSpawnInterval,
		true);
}

void AT66EnemyDirector::HandleStageTimerChanged()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;
	AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	if (!T66EnableTowerEnemySpawns && GameMode && GameMode->IsUsingTowerMainMapLayout())
	{
		bSpawningArmed = false;
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
		World->GetTimerManager().ClearTimer(StaggeredSpawnTimerHandle);
		PendingSpawns.Empty();
		return;
	}

	if (!bSpawningPaused && RunState->GetStageTimerActive())
	{
		if (!bSpawningArmed)
		{
			UT66StageProgressionSubsystem* StageProgression = GI ? GI->GetSubsystem<UT66StageProgressionSubsystem>() : nullptr;
			if (StageProgression)
			{
				StageProgression->RefreshSnapshot(false);
			}

			const FT66StageProgressionSnapshot Snapshot = StageProgression
				? StageProgression->GetCurrentSnapshot()
				: FT66StageProgressionSnapshot{};
			const AT66GameMode* ActiveGameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
			const bool bTowerLayout = ActiveGameMode && ActiveGameMode->IsUsingTowerMainMapLayout();
			const float RuntimeSpawnInterval = FMath::Max(
				0.15f,
				bTowerLayout
					? T66TowerTargetRuntimeSpawnIntervalSeconds
					: (BaseSpawnIntervalSeconds * Snapshot.RuntimeTrickleIntervalScalar) / FMath::Max(1.0f, Snapshot.DifficultyScalar));

			bSpawningArmed = true;
			SpawnRuntimeTrickleWave();
			World->GetTimerManager().ClearTimer(SpawnTimerHandle);
			World->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AT66EnemyDirector::SpawnRuntimeTrickleWave, RuntimeSpawnInterval, true);
		}
	}
	else
	{
		// Timer frozen: don't spawn waves.
		bSpawningArmed = false;
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
		World->GetTimerManager().ClearTimer(StaggeredSpawnTimerHandle);
		PendingSpawns.Empty();
	}
}

void AT66EnemyDirector::SpawnRuntimeTrickleWave()
{
	if (bSpawningPaused) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// Let the current staggered wave finish before planning another one.
	if (PendingSpawns.Num() > 0 || World->GetTimerManager().IsTimerActive(StaggeredSpawnTimerHandle))
	{
		return;
	}

	FLagScopedScope LagScope(GetWorld(), TEXT("EnemyDirector::SpawnRuntimeTrickleWave"));

	TArray<APawn*> PlayerPawns;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APawn* PlayerPawn = It->Get() ? It->Get()->GetPawn() : nullptr)
		{
			PlayerPawns.Add(PlayerPawn);
		}
	}
	if (PlayerPawns.Num() <= 0) return;

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	FRandomStream LocalRng(static_cast<int32>(FPlatformTime::Cycles()));
	FRandomStream& Rng = RngSub ? RngSub->GetRunStream() : LocalRng;
	APawn* PlayerPawn = PlayerPawns[RngSub ? RngSub->RunRandRange(0, PlayerPawns.Num() - 1) : Rng.RandRange(0, PlayerPawns.Num() - 1)];
	if (!PlayerPawn) return;

	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66StageProgressionSubsystem* StageProgression = GI ? GI->GetSubsystem<UT66StageProgressionSubsystem>() : nullptr;
	// Only start spawning once the stage timer is active (i.e. after the main-area entrance trigger).
	if (!RunState || !RunState->GetStageTimerActive())
	{
		return;
	}

	if (StageProgression)
	{
		StageProgression->RefreshSnapshot(false);
	}

	const FT66StageProgressionSnapshot Snapshot = StageProgression
		? StageProgression->GetCurrentSnapshot()
		: FT66StageProgressionSnapshot{};

	AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	const bool bTowerLayout = GameMode && GameMode->IsUsingTowerMainMapLayout();
	if (bTowerLayout && !T66EnableTowerEnemySpawns)
	{
		return;
	}

	// Difficulty scaling affects enemy count (waves + max alive) outside the tower progression curve.
	const float Scalar = RunState->GetDifficultyScalar();
	const float FinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
	const float SpawnScalar = Scalar * FinaleScalar * Snapshot.RuntimeTrickleCountScalar;
	int32 EffectivePerWave = 0;
	int32 EffectiveMaxAlive = 0;
	if (bTowerLayout)
	{
		EffectivePerWave = T66TowerTargetEnemiesPerWave;
		EffectiveMaxAlive = T66TowerTargetMaxAliveEnemies;
	}
	else
	{
		static constexpr float SpawnDensityMultiplier = 3.0f;
		EffectivePerWave = FMath::Clamp(
			FMath::RoundToInt(static_cast<float>(FMath::Max(1, BaseEnemiesPerWave)) * SpawnScalar * SpawnDensityMultiplier),
			1,
			600);
		EffectiveMaxAlive = FMath::Clamp(
			FMath::RoundToInt(static_cast<float>(FMath::Max(1, BaseMaxAliveEnemies)) * SpawnScalar * SpawnDensityMultiplier),
			1,
			1500);
	}
	EffectiveMaxAlive = FMath::Max(EffectiveMaxAlive, EffectivePerWave);

	int32 ToSpawn = FMath::Min(EffectivePerWave, EffectiveMaxAlive - AliveCount);
	if (ToSpawn <= 0) return;

	if (RngSub && RunState)
	{
		RngSub->UpdateLuckStat(RunState->GetEffectiveLuckBiasStat());
	}
	const UT66RngTuningConfig* Tuning = RngSub ? RngSub->GetTuning() : nullptr;

	// [GOLD] Use the actor registry for safe-zone NPC lookup (replaces TActorIterator cache).
	UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;

	auto IsInAnySafeZone2D = [&](const FVector& Loc) -> bool
	{
		if (!Registry) return false;
		const int32 CandidateFloorNumber = (bTowerLayout && GameMode) ? GameMode->GetTowerFloorIndexForLocation(Loc) : INDEX_NONE;
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Registry->GetNPCs())
		{
			AT66HouseNPCBase* NPC = WeakNPC.Get();
			if (!NPC) continue;
			if (bTowerLayout && GameMode && CandidateFloorNumber != INDEX_NONE
				&& GameMode->GetTowerFloorIndexForLocation(NPC->GetActorLocation()) != CandidateFloorNumber)
			{
				continue;
			}
			const float R = NPC->GetSafeZoneRadius();
			if (FVector::DistSquared2D(Loc, NPC->GetActorLocation()) < (R * R))
			{
				return true;
			}
		}
		for (const TWeakObjectPtr<AT66CircusInteractable>& WeakCircus : Registry->GetCircuses())
		{
			AT66CircusInteractable* Circus = WeakCircus.Get();
			if (!Circus) continue;
			if (bTowerLayout && GameMode && CandidateFloorNumber != INDEX_NONE
				&& GameMode->GetTowerFloorIndexForLocation(Circus->GetActorLocation()) != CandidateFloorNumber)
			{
				continue;
			}
			const float R = Circus->GetSafeZoneRadius();
			if (FVector::DistSquared2D(Loc, Circus->GetActorLocation()) < (R * R))
			{
				return true;
			}
		}
		return false;
	};

	auto IsInBlockedTraversalZone2D = [bTowerLayout](const FVector& Loc) -> bool
	{
		return !bTowerLayout && T66GameplayLayout::IsInsideReservedTraversalZone2D(Loc, 120.f);
	};

	auto IsFarEnoughFromPlayers2D = [&](const FVector& Loc, const float MinDistance) -> bool
	{
		const float MinDistanceSq = FMath::Square(FMath::Max(MinDistance, 0.f));
		const int32 CandidateFloorNumber = (bTowerLayout && GameMode) ? GameMode->GetTowerFloorIndexForLocation(Loc) : INDEX_NONE;
		for (const APawn* OtherPlayerPawn : PlayerPawns)
		{
			if (!OtherPlayerPawn)
			{
				continue;
			}

			if (bTowerLayout && GameMode && CandidateFloorNumber != INDEX_NONE)
			{
				const int32 OtherFloorNumber = GameMode->GetTowerFloorIndexForLocation(OtherPlayerPawn->GetActorLocation());
				if (OtherFloorNumber != CandidateFloorNumber)
				{
					continue;
				}
			}

			if (FVector::DistSquared2D(Loc, OtherPlayerPawn->GetActorLocation()) < MinDistanceSq)
			{
				return false;
			}
		}

		return true;
	};

	// Robust fallback: if EnemyClass is unset or misconfigured to a special enemy, use base enemy for the "regular" slot.
	TSubclassOf<AT66EnemyBase> RegularClass = EnemyClass;
	if (!RegularClass)
	{
		RegularClass = AT66MeleeEnemy::StaticClass();
	}

	// Stage mobs: pull exact roster from DT_Stages (EnemyA/B/C). Fallback is deterministic IDs.
	const int32 StageNum = RunState->GetCurrentStage();
	TArray<FName> MobIDs;
	T66ResolveStageMobIDs(GI, StageNum, MobIDs);
	const FName MobA = MobIDs.IsValidIndex(0) ? MobIDs[0] : NAME_None;
	const FName MobB = MobIDs.IsValidIndex(1) ? MobIDs[1] : NAME_None;
	const FName MobC = MobIDs.IsValidIndex(2) ? MobIDs[2] : NAME_None;

#if !UE_BUILD_SHIPPING
	static int32 LoggedMobWaves = 0;
	if (LoggedMobWaves < 3)
	{
		++LoggedMobWaves;
		UE_LOG(LogT66EnemyDirector, Verbose, TEXT("[SPAWN] SpawnWave Stage=%d MobIDs: A=%s  B=%s  C=%s (generic fallback would be Mob_StageXX_X — if you see that, reimport DT_Stages)"),
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
		const bool bGoblinWaveSpawned = GoblinThiefClass && (RngSub ? RngSub->RollChance01(GobChance) : (Rng.GetFraction() < GobChance));
		const int32 GoblinWaveDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		const int32 GoblinWavePreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
		if (RunState)
		{
			RunState->RecordLuckQuantityBool(
				FName(TEXT("GoblinWaveSpawned")),
				bGoblinWaveSpawned,
				GobChance,
				GoblinWaveDrawIndex,
				GoblinWavePreDrawSeed);
		}

		if (bGoblinWaveSpawned)
		{
			GobToSpawn = RngSub ? RngSub->RollIntRangeBiased(Tuning->GoblinCountPerWave, Rng) : Rng.RandRange(Tuning->GoblinCountPerWave.Min, Tuning->GoblinCountPerWave.Max);
		}

		GobToSpawn = FMath::Max(0, GobToSpawn);
		GobToSpawn = FMath::Clamp(GobToSpawn, 0, ToSpawn);
	}

	// Luck Rating tracking (quantity): per-wave special mob counts.
	if (RunState && Tuning)
	{
		RunState->RecordLuckQuantityRoll(
			FName(TEXT("GoblinCountPerWave")),
			GobToSpawn,
			Tuning->GoblinCountPerWave.Min,
			Tuning->GoblinCountPerWave.Max,
			(RngSub && GobToSpawn > 0) ? RngSub->GetLastRunDrawIndex() : INDEX_NONE,
			(RngSub && GobToSpawn > 0) ? RngSub->GetLastRunPreDrawSeed() : 0);
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
	if (bCanSpawnMiniBoss && MobToSpawn > 0)
	{
		const float MiniBossChance = FMath::Clamp(MiniBossChancePerWave, 0.f, 1.f);
		const int32 MiniBossWaveFallbackPreDrawSeed = RngSub ? 0 : Rng.GetCurrentSeed();
		const bool bMiniBossWaveSpawned = RngSub
			? RngSub->RollChance01(MiniBossChance)
			: (Rng.GetFraction() < MiniBossChance);
		const int32 MiniBossWaveDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		const int32 MiniBossWavePreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : MiniBossWaveFallbackPreDrawSeed;
		if (RunState)
		{
			RunState->RecordLuckQuantityBool(
				FName(TEXT("MiniBossWaveSpawned")),
				bMiniBossWaveSpawned,
				MiniBossChance,
				MiniBossWaveDrawIndex,
				MiniBossWavePreDrawSeed);
		}

		if (bMiniBossWaveSpawned)
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
	EffectiveSpawnMin = FMath::Max(EffectiveSpawnMin, FMath::Max(MinimumPlayerSpawnClearance, 0.f));
	EffectiveSpawnMax = FMath::Max(EffectiveSpawnMax, EffectiveSpawnMin + 400.f);

	const FVector PlayerLoc = PlayerPawn->GetActorLocation();
	auto TraceGroundZAtXY = [&](const FVector& XYLoc, float& OutGroundZ) -> bool
	{
		if (bTowerLayout)
		{
			return false;
		}

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
	if (bTowerLayout)
	{
		PlayerGroundZ = PlayerLoc.Z;
		if (const AT66HeroBase* Hero = Cast<AT66HeroBase>(PlayerPawn))
		{
			if (const UCapsuleComponent* Capsule = Hero->GetCapsuleComponent())
			{
				PlayerGroundZ -= Capsule->GetScaledCapsuleHalfHeight();
			}
		}
	}
	else if (!TraceGroundZAtXY(PlayerLoc, PlayerGroundZ))
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
		if (bTowerLayout)
		{
			return FVector(XYLoc.X, XYLoc.Y, XYLoc.Z + EnemyCapsuleHalfHeight);
		}

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
		FVector SpawnWallNormal = FVector::ZeroVector;
		bool bFoundSpawnLoc = false;
		if (bTowerLayout && GameMode)
		{
			for (int32 Try = 0; Try < 12; ++Try)
			{
				FVector WallSpawnSurface = FVector::ZeroVector;
				FVector WallNormal = FVector::ZeroVector;
				if (!GameMode->TryGetTowerEnemySpawnLocation(PlayerLoc, EffectiveSpawnMin, EffectiveSpawnMax, Rng, WallSpawnSurface, WallNormal))
				{
					continue;
				}

				SpawnLoc = ResolveGroundedSpawnLocation(WallSpawnSurface);
				SpawnWallNormal = WallNormal;
				if (!IsInAnySafeZone2D(SpawnLoc)
					&& !IsInBlockedTraversalZone2D(SpawnLoc)
					&& IsFarEnoughFromPlayers2D(SpawnLoc, EffectiveSpawnMin))
				{
					bFoundSpawnLoc = true;
					break;
				}
			}
		}
		else
		{
			// Try a few times to avoid spawning inside safe zones or reserved traversal spaces.
			for (int32 Try = 0; Try < 12; ++Try)
			{
				const float Angle = Rng.FRandRange(0.f, 2.f * PI);
				const float Dist = Rng.FRandRange(EffectiveSpawnMin, EffectiveSpawnMax);
				FVector Offset(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
				SpawnLoc = FVector(PlayerLoc.X + Offset.X, PlayerLoc.Y + Offset.Y, PlayerGroundZ);

				if (!IsInAnySafeZone2D(SpawnLoc)
					&& !IsInBlockedTraversalZone2D(SpawnLoc)
					&& IsFarEnoughFromPlayers2D(SpawnLoc, EffectiveSpawnMin))
				{
					bFoundSpawnLoc = true;
					break;
				}
			}
		}
		// Guarantee: if still inside a safe zone, push outward past the owning safe-zone radius.
		if (!bTowerLayout && Registry && IsInAnySafeZone2D(SpawnLoc))
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
					FVector Dir(Rng.FRandRange(-1.f, 1.f), Rng.FRandRange(-1.f, 1.f), 0.f);
					Dir.Z = 0.f;
					if (Dir.Normalize())
					{
						SpawnLoc = NPC->GetActorLocation() + Dir * (R + SafeZonePushMargin);
						SpawnLoc.Z = PlayerGroundZ;
					}
					break;
				}
			}
			for (const TWeakObjectPtr<AT66CircusInteractable>& WeakCircus : Registry->GetCircuses())
			{
				AT66CircusInteractable* Circus = WeakCircus.Get();
				if (!Circus) continue;
				const float R = Circus->GetSafeZoneRadius();
				FVector ToSpawnPt = SpawnLoc - Circus->GetActorLocation();
				ToSpawnPt.Z = 0.f;
				const float Dist2D = ToSpawnPt.Size();
				if (Dist2D < R && Dist2D > 1.f)
				{
					FVector Dir = ToSpawnPt / Dist2D;
					SpawnLoc = Circus->GetActorLocation() + FVector(Dir.X, Dir.Y, 0.f) * (R + SafeZonePushMargin);
					SpawnLoc.Z = PlayerGroundZ;
					break;
				}
				else if (Dist2D <= 1.f)
				{
					FVector Dir(Rng.FRandRange(-1.f, 1.f), Rng.FRandRange(-1.f, 1.f), 0.f);
					Dir.Z = 0.f;
					if (Dir.Normalize())
					{
						SpawnLoc = Circus->GetActorLocation() + Dir * (R + SafeZonePushMargin);
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

		if (!bTowerLayout)
		{
			EnforceMinSpawnDistance2D(SpawnLoc);
			SpawnLoc = ResolveGroundedSpawnLocation(SpawnLoc);
		}

		if (!IsFarEnoughFromPlayers2D(SpawnLoc, EffectiveSpawnMin))
		{
			if (bTowerLayout)
			{
				continue;
			}

			bool bResolvedPlayerClearance = false;
			for (int32 Retry = 0; Retry < 8; ++Retry)
			{
				const float Angle = Rng.FRandRange(0.f, 2.f * PI);
				const float Dist = Rng.FRandRange(EffectiveSpawnMin, EffectiveSpawnMax + 400.f);
				const FVector Offset(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
				FVector RetryLoc = ResolveGroundedSpawnLocation(FVector(PlayerLoc.X + Offset.X, PlayerLoc.Y + Offset.Y, PlayerGroundZ));
				if (IsInAnySafeZone2D(RetryLoc)
					|| IsInBlockedTraversalZone2D(RetryLoc)
					|| !IsFarEnoughFromPlayers2D(RetryLoc, EffectiveSpawnMin))
				{
					continue;
				}

				SpawnLoc = RetryLoc;
				bResolvedPlayerClearance = true;
				break;
			}

			if (!bResolvedPlayerClearance)
			{
				continue;
			}
		}

		if (IsInBlockedTraversalZone2D(SpawnLoc))
		{
			if (bTowerLayout)
			{
				continue;
			}

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

		const bool bIsMob = (ClassToSpawn != GoblinThiefClass);
		const bool bIsMiniBossSlot = bIsMob && (MiniBossIndex == i);
		const FName MobID = bIsMob ? MobIDs[RngSub ? RngSub->RunRandRange(0, MobIDs.Num() - 1) : Rng.RandRange(0, MobIDs.Num() - 1)] : NAME_None;
		if (bIsMob)
		{
			ClassToSpawn = FT66EnemyFamilyResolver::ResolveEnemyClass(MobID, RegularClass);
		}

		FPendingEnemySpawn Slot;
		Slot.GroundLocation = SpawnLoc;
		Slot.ClassToSpawn = ClassToSpawn;
		Slot.MobID = MobID;
		Slot.bIsMiniBoss = bIsMiniBossSlot;
		Slot.bSpawnFromWall = bTowerLayout;
		Slot.DifficultyScalar = Scalar;
		Slot.FinaleScalar = FinaleScalar;
		Slot.EnemyProgressionScalar = Snapshot.EnemyStatScalar;
		Slot.StageNum = RunState ? RunState->GetCurrentStage() : 1;
		Slot.WallNormal = SpawnWallNormal;
		Slot.Channel = ET66EnemySpawnChannel::RuntimeTrickle;
		PendingSpawns.Add(Slot);
	}

	World->GetTimerManager().ClearTimer(StaggeredSpawnTimerHandle);
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
	FRandomStream LocalRng(static_cast<int32>(FPlatformTime::Cycles()));
	FRandomStream& Rng = RngSub ? RngSub->GetRunStream() : LocalRng;
	AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	const bool bTowerLayout = GameMode && GameMode->IsUsingTowerMainMapLayout();

	const int32 BatchSize = FMath::Max(1, MaxSpawnsPerStaggeredBatch);
	int32 ProcessedCount = 0;
	while (ProcessedCount < PendingSpawns.Num() && ProcessedCount < BatchSize)
	{
		const FPendingEnemySpawn Slot = PendingSpawns[ProcessedCount];
		++ProcessedCount;

		const bool bIsMob = !Slot.MobID.IsNone();
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
			const bool bGoblinRarityReplayable = (RngSub && Tuning);
			const ET66Rarity R = bGoblinRarityReplayable ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
			if (AT66GoblinThiefEnemy* Gob = Cast<AT66GoblinThiefEnemy>(Enemy))
			{
				Gob->SetRarity(R);
				if (RunState)
				{
					RunState->RecordLuckQualityRarity(
						FName(TEXT("GoblinRarity")),
						R,
						bGoblinRarityReplayable ? RngSub->GetLastRunDrawIndex() : INDEX_NONE,
						bGoblinRarityReplayable ? RngSub->GetLastRunPreDrawSeed() : 0,
						bGoblinRarityReplayable ? &Weights : nullptr);
				}
			}

			if (RunState)
			{
				Enemy->ApplyStageScaling(Slot.StageNum);
				Enemy->ApplyDifficultyScalar(Slot.DifficultyScalar);
				Enemy->ApplyProgressionEnemyScalar(Slot.EnemyProgressionScalar);
			}
			if (Slot.bIsMiniBoss)
			{
				Enemy->ApplyMiniBossMultipliers(MiniBossHPScalar, MiniBossDamageScalar, MiniBossScale);
				ActiveMiniBoss = Enemy;
			}
			Enemy->ApplyFinaleScaling(Slot.FinaleScalar);
			AliveCount++;
			if (Slot.bSpawnFromWall && bTowerLayout)
			{
				Enemy->StartEmergeFromWall(Slot.GroundLocation, Slot.WallNormal);
			}
			else if (!bTowerLayout)
			{
				Enemy->StartRiseFromGround(Slot.GroundLocation.Z);
			}
		}
	}

	if (ProcessedCount > 0)
	{
		PendingSpawns.RemoveAt(0, ProcessedCount, EAllowShrinking::No);
	}

	if (PendingSpawns.Num() > 0)
	{
		World->GetTimerManager().SetTimer(
			StaggeredSpawnTimerHandle,
			this,
			&AT66EnemyDirector::SpawnNextStaggeredBatch,
			FMath::Max(0.0f, StaggeredSpawnIntervalSeconds),
			false);
	}
	else
	{
		World->GetTimerManager().ClearTimer(StaggeredSpawnTimerHandle);
	}
}
