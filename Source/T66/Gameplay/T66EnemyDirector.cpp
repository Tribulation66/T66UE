// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/Enemies/T66EnemyFamilyResolver.h"
#include "Gameplay/Enemies/T66FlyingEnemy.h"
#include "Gameplay/Enemies/T66MeleeEnemy.h"
#include "Gameplay/Enemies/T66RangedEnemy.h"
#include "Gameplay/Enemies/T66RushEnemy.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66MobBase.h"
#include "Gameplay/T66MobManagerSubsystem.h"
#include "Gameplay/T66TowerMapTerrain.h"
#include "Gameplay/T66NPCBase.h"
#include "Gameplay/T66SafeZoneComponent.h"
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
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66EnemyDirector, Log, All);

namespace
{
	static constexpr bool T66EnableTowerEnemySpawns = true;
	static constexpr int32 T66TowerTargetInitialEnemiesPerMobFloor = 4;

	static TAutoConsoleVariable<int32> CVarT66EnemyDirectorMaxAliveOverride(
		TEXT("T66.EnemyDirector.MaxAliveOverride"),
		0,
		TEXT("Non-shipping diagnostics: overrides runtime max alive enemy cap when greater than 0. Default 0 uses data/default tuning."),
#if UE_BUILD_SHIPPING
		ECVF_ReadOnly
#else
		ECVF_Default
#endif
	);

	static ET66RouteAttributionReason T66ResolveRouteAttributionReason(
		const FName MobID,
		const ET66EnemyFamily Family,
		const bool bIsMiniBoss,
		const bool bIsSpecialSpawn)
	{
		if (bIsMiniBoss)
		{
			return ET66RouteAttributionReason::RoutedRich_MiniBossPromotion;
		}
		if (MobID.IsNone() || bIsSpecialSpawn)
		{
			return ET66RouteAttributionReason::RoutedRich_SpecialOrMiniBoss;
		}
		if (Family == ET66EnemyFamily::Special)
		{
			return ET66RouteAttributionReason::RoutedRich_FamilyLookupFailed;
		}

		return ET66RouteAttributionReason::RoutedRich_FallbackBranch;
	}

	static int32 T66ResolveRuntimeMaxAliveEnemies(const int32 DataMaxAliveEnemies, const int32 FallbackMaxAliveEnemies)
	{
#if !UE_BUILD_SHIPPING
		const int32 OverrideMaxAliveEnemies = CVarT66EnemyDirectorMaxAliveOverride.GetValueOnGameThread();
		if (OverrideMaxAliveEnemies > 0)
		{
			return OverrideMaxAliveEnemies;
		}
#endif
		return DataMaxAliveEnemies > 0 ? DataMaxAliveEnemies : FallbackMaxAliveEnemies;
	}

	static void T66ResolveStageMobIDs(UGameInstance* GI, const int32 StageNum, TArray<FName>& OutMobIDs)
	{
		TArray<FName> StageMobIDs;
		StageMobIDs.Reserve(10);

		auto AddStageMob = [&StageMobIDs](const FName MobID)
		{
			if (!MobID.IsNone())
			{
				StageMobIDs.Add(MobID);
			}
		};

		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
		{
			FStageData StageData;
			if (T66GI->GetStageData(StageNum, StageData))
			{
				AddStageMob(StageData.EnemyA);
				AddStageMob(StageData.EnemyB);
				AddStageMob(StageData.EnemyC);
				AddStageMob(StageData.EnemyD);
				AddStageMob(StageData.EnemyE);
				AddStageMob(StageData.EnemyF);
				AddStageMob(StageData.EnemyG);
				AddStageMob(StageData.EnemyH);
				AddStageMob(StageData.EnemyI);
				AddStageMob(StageData.EnemyJ);
				AddStageMob(StageData.EnemyK);
				AddStageMob(StageData.EnemyL);
			}
		}

		if (StageMobIDs.Num() > 0)
		{
			OutMobIDs = MoveTemp(StageMobIDs);
			return;
		}

		OutMobIDs = {
			FName(*FString::Printf(TEXT("Mob_Stage%02d_A"), StageNum)),
			FName(*FString::Printf(TEXT("Mob_Stage%02d_B"), StageNum)),
			FName(*FString::Printf(TEXT("Mob_Stage%02d_C"), StageNum)),
			FName(*FString::Printf(TEXT("Mob_Stage%02d_D"), StageNum)),
			FName(*FString::Printf(TEXT("Mob_Stage%02d_E"), StageNum))
		};
	}

	static TSubclassOf<AT66EnemyBase> T66ResolveEnemyClassFromFamilyID(const FName FamilyID, TSubclassOf<AT66EnemyBase> FallbackClass)
	{
		if (FamilyID == FName(TEXT("Flying")))
		{
			return AT66FlyingEnemy::StaticClass();
		}
		if (FamilyID == FName(TEXT("Ranged")))
		{
			return AT66RangedEnemy::StaticClass();
		}
		if (FamilyID == FName(TEXT("Rush")))
		{
			return AT66RushEnemy::StaticClass();
		}
		if (FamilyID == FName(TEXT("Melee")))
		{
			return AT66MeleeEnemy::StaticClass();
		}

		if (FallbackClass)
		{
			return FallbackClass;
		}
		return AT66MeleeEnemy::StaticClass();
	}

	static ET66EnemyFamily T66ResolveEnemyFamilyFromFamilyID(const FName FamilyID, const ET66EnemyFamily FallbackFamily)
	{
		if (FamilyID == FName(TEXT("Flying")))
		{
			return ET66EnemyFamily::Flying;
		}
		if (FamilyID == FName(TEXT("Ranged")))
		{
			return ET66EnemyFamily::Ranged;
		}
		if (FamilyID == FName(TEXT("Rush")))
		{
			return ET66EnemyFamily::Rush;
		}
		if (FamilyID == FName(TEXT("Melee")))
		{
			return ET66EnemyFamily::Melee;
		}

		return FallbackFamily;
	}

	static TSubclassOf<AT66EnemyBase> T66ResolveStageEnemyClass(UT66GameInstance* T66GI, const FName MobID, TSubclassOf<AT66EnemyBase> FallbackClass)
	{
		if (T66GI)
		{
			FT66EnemyData EnemyData;
			if (T66GI->GetEnemyData(MobID, EnemyData))
			{
				return T66ResolveEnemyClassFromFamilyID(EnemyData.FamilyID, FallbackClass);
			}
		}

		return FT66EnemyFamilyResolver::ResolveEnemyClass(MobID, FallbackClass);
	}

	static ET66EnemyFamily T66ResolveStageEnemyFamily(UT66GameInstance* T66GI, const FName MobID)
	{
		if (T66GI)
		{
			FT66EnemyData EnemyData;
			if (T66GI->GetEnemyData(MobID, EnemyData))
			{
				return T66ResolveEnemyFamilyFromFamilyID(EnemyData.FamilyID, FT66EnemyFamilyResolver::ResolveFamily(MobID));
			}
		}

		return FT66EnemyFamilyResolver::ResolveFamily(MobID);
	}

	static FName T66ResolveStageEnemyArchetype(UT66GameInstance* T66GI, const FName MobID)
	{
		if (T66GI)
		{
			FT66EnemyData EnemyData;
			if (T66GI->GetEnemyData(MobID, EnemyData))
			{
				return EnemyData.Archetype;
			}
		}

		return NAME_None;
	}

	static void T66IncrementLightweightFamilyCounter(const ET66EnemyFamily Family, int32& MeleeCount, int32& RushCount, int32& FlyingCount, int32& RangedCount)
	{
		if (Family == ET66EnemyFamily::Melee)
		{
			++MeleeCount;
		}
		else if (Family == ET66EnemyFamily::Rush)
		{
			++RushCount;
		}
		else if (Family == ET66EnemyFamily::Flying)
		{
			++FlyingCount;
		}
		else if (Family == ET66EnemyFamily::Ranged)
		{
			++RangedCount;
		}
	}

	static void T66DecrementLightweightFamilyCounter(const ET66EnemyFamily Family, int32& MeleeCount, int32& RushCount, int32& FlyingCount, int32& RangedCount)
	{
		if (Family == ET66EnemyFamily::Melee && MeleeCount > 0)
		{
			--MeleeCount;
		}
		else if (Family == ET66EnemyFamily::Rush && RushCount > 0)
		{
			--RushCount;
		}
		else if (Family == ET66EnemyFamily::Flying && FlyingCount > 0)
		{
			--FlyingCount;
		}
		else if (Family == ET66EnemyFamily::Ranged && RangedCount > 0)
		{
			--RangedCount;
		}
	}
}

AT66EnemyDirector::AT66EnemyDirector()
{
	PrimaryActorTick.bCanEverTick = false;
	EnemyClass = AT66MeleeEnemy::StaticClass();
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
	ActiveStaggeredSpawnIntervalSeconds = FMath::Max(0.0f, StaggeredSpawnIntervalSeconds);
	ActiveRuntimeWaveCooldownSeconds = 0.05f;
	ActiveMaxSpawnsPerStaggeredBatch = FMath::Max(1, MaxSpawnsPerStaggeredBatch);

	Super::EndPlay(EndPlayReason);
}

void AT66EnemyDirector::SetPandemoniumMode(
	const bool bEnabled,
	const float RuntimeSpawnIntervalSeconds,
	const int32 RuntimeEnemiesPerWave,
	const int32 RuntimeMaxAliveEnemies,
	const int32 RuntimeMaxSpawnsPerBatch)
{
	bPandemoniumMode = bEnabled;
	PandemoniumRuntimeSpawnIntervalSeconds = FMath::Clamp(RuntimeSpawnIntervalSeconds, 0.05f, 5.f);
	PandemoniumRuntimeEnemiesPerWave = FMath::Clamp(RuntimeEnemiesPerWave, 1, 600);
	PandemoniumRuntimeMaxAliveEnemies = FMath::Clamp(RuntimeMaxAliveEnemies, PandemoniumRuntimeEnemiesPerWave, 1500);
	PandemoniumMaxSpawnsPerBatch = FMath::Clamp(RuntimeMaxSpawnsPerBatch, 1, 64);
	RefreshSpawningFromProgression();
}

void AT66EnemyDirector::SpawnInitialPopulationForStage()
{
	UWorld* World = GetWorld();
	AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	if (!GameMode || !GameMode->IsUsingTowerMainMapLayout())
	{
		return;
	}

	SpawnInitialPopulationForTowerFloor(GameMode->GetCurrentTowerFloorIndex());
}

int32 AT66EnemyDirector::SpawnInitialPopulationForTowerFloor(const int32 RequestedFloorNumber)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	AT66GameMode* GameMode = Cast<AT66GameMode>(World->GetAuthGameMode());
	if (!GameMode || !GameMode->IsUsingTowerMainMapLayout())
	{
		return 0;
	}

	T66TowerMapTerrain::FLayout TowerLayout;
	if (!GameMode->GetTowerMainMapLayout(TowerLayout))
	{
		return 0;
	}
	const int32 ActiveMobFloorNumber = RequestedFloorNumber;
	if (ActiveMobFloorNumber == INDEX_NONE || TowerFloorsWithInitialPopulation.Contains(ActiveMobFloorNumber))
	{
		return 0;
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
	const FT66SpawnBudget SpawnBudget = Snapshot.SpawnBudget;

	TSubclassOf<AT66EnemyBase> RegularClass = EnemyClass;
	if (!RegularClass)
	{
		RegularClass = AT66MeleeEnemy::StaticClass();
	}

	TArray<FName> MobIDs;
	T66ResolveStageMobIDs(GI, StageNum, MobIDs);
	if (MobIDs.Num() <= 0)
	{
		return 0;
	}

	FRandomStream Rng((StageNum * 4051) + 177);
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	if (T66GI)
	{
		if (T66GI->RunSeed != 0)
		{
			Rng.Initialize(T66GI->RunSeed + StageNum * 4051 + 177);
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>();

	int32 SpawnedCount = 0;
	for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
	{
		if (!Floor.bMobFloor)
		{
			continue;
		}
		if (ActiveMobFloorNumber != INDEX_NONE && Floor.FloorNumber != ActiveMobFloorNumber)
		{
			continue;
		}

		const int32 InitialPopulationCount = FMath::Clamp(
			SpawnBudget.InitialEnemiesPerMobFloor > 0 ? SpawnBudget.InitialEnemiesPerMobFloor : T66TowerTargetInitialEnemiesPerMobFloor,
			0,
			128);
		for (int32 SpawnIndex = 0; SpawnIndex < InitialPopulationCount; ++SpawnIndex)
		{
			FVector SpawnLoc = FVector::ZeroVector;
			auto TryFindInitialTowerSpawnLocation = [&](const float EdgePadding, const float HolePadding, const int32 Attempts) -> bool
			{
				for (int32 Attempt = 0; Attempt < Attempts; ++Attempt)
				{
					if (T66TowerMapTerrain::TryGetRandomSurfaceLocationOnFloor(
						World,
						TowerLayout,
						Floor.FloorNumber,
						Rng,
						SpawnLoc,
						EdgePadding,
						HolePadding))
					{
						return true;
					}
				}
				return false;
			};

			if (!TryFindInitialTowerSpawnLocation(InitialTowerSpawnEdgePadding, InitialTowerSpawnHolePadding, 24)
				&& !TryFindInitialTowerSpawnLocation(600.0f, 850.0f, 24)
				&& !TryFindInitialTowerSpawnLocation(100.0f, 100.0f, 24))
			{
				continue;
			}

			static constexpr float EnemyCapsuleHalfHeight = 88.f;
			SpawnLoc.Z += EnemyCapsuleHalfHeight;

			const FName MobID = MobIDs[Rng.RandRange(0, MobIDs.Num() - 1)];
			const ET66EnemyFamily MobFamily = T66ResolveStageEnemyFamily(T66GI, MobID);
			const FName MobArchetype = T66ResolveStageEnemyArchetype(T66GI, MobID);
			const TSubclassOf<AT66EnemyBase> MobClass = T66ResolveStageEnemyClass(T66GI, MobID, RegularClass);
			const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLoc);
			const bool bShouldRouteLightweight = ShouldRouteSpawnToLightweightMob(MobID, MobFamily, false, false);
			if (bShouldRouteLightweight)
			{
				bool bRequiresFinishSpawning = false;
				AT66MobBase* Mob = MobManager ? MobManager->AcquireMob(AT66MobBase::StaticClass(), SpawnTransform, &bRequiresFinishSpawning) : nullptr;
				if (!Mob)
				{
					if (MobManager)
					{
						MobManager->RecordLightweightAcquireFailed(MobFamily, ET66RouteAttributionChannel::InitialPopulation);
					}
					continue;
				}

				Mob->OwningDirector = this;
				Mob->MobID = MobID;
				Mob->CharacterVisualID = MobID;
				Mob->ConfigureAsMob(
					MobID,
					MobFamily,
					MobArchetype,
					StageNum,
					RunState ? RunState->GetDifficultyScalar() : 1.f,
					Snapshot.EnemyStatScalar,
					RunState ? RunState->GetFinalSurvivalEnemyScalar() : 1.f,
					false);
				if (bRequiresFinishSpawning)
				{
					UGameplayStatics::FinishSpawningActor(Mob, SpawnTransform);
				}
				++LightweightAliveCount;
				T66IncrementLightweightFamilyCounter(Mob->GetEnemyFamily(), LightweightMeleeAliveCount, LightweightRushAliveCount, LightweightFlyingAliveCount, LightweightRangedAliveCount);
				if (MobManager && Mob->GetEnemyFamily() == ET66EnemyFamily::Ranged)
				{
					MobManager->RecordRangedMobSpawn(true, Mob->MobID);
				}
				if (MobManager)
				{
					MobManager->RecordRouteAttribution(Mob->GetEnemyFamily(), ET66RouteAttributionReason::RoutedLightweight_BasicFamily, ET66RouteAttributionChannel::InitialPopulation);
				}
				++SpawnedCount;
				continue;
			}

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
				Enemy->FreezeScoreAwardAtSpawn(RunState->GetDifficultyScalar());
				RunState->RegisterSpawnedEnemyScoreBudget(Enemy->GetResolvedScoreAward(), StageNum);
			}

			++AliveCount;
			if (MobManager && Enemy->EnemyFamily == ET66EnemyFamily::Ranged)
			{
				MobManager->RecordRangedMobSpawn(false, Enemy->MobID);
			}
			if (MobManager)
			{
				MobManager->RecordRouteAttribution(
					MobFamily,
					T66ResolveRouteAttributionReason(MobID, MobFamily, false, false),
					ET66RouteAttributionChannel::InitialPopulation);
			}
			++SpawnedCount;
		}
	}

	if (SpawnedCount > 0)
	{
		TowerFloorsWithInitialPopulation.Add(ActiveMobFloorNumber);
		UE_LOG(LogT66EnemyDirector, Log, TEXT("[SPAWN] Tower initial enemies floor=%d spawned=%d alive=%d richAlive=%d lightweightAlive=%d."),
			ActiveMobFloorNumber,
			SpawnedCount,
			GetAliveEnemyCount(),
			AliveCount,
			LightweightAliveCount);
	}
	else
	{
		UE_LOG(LogT66EnemyDirector, Warning, TEXT("[SPAWN] Tower initial enemies floor=%d spawned=0; will retry on next floor activation."),
			ActiveMobFloorNumber);
	}
	return SpawnedCount;
}

void AT66EnemyDirector::NotifyEnemyDied(AT66EnemyBase* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	if (AliveCount > 0)
	{
		AliveCount--;
	}

	UWorld* World = GetWorld();
	if (!World || bSpawningPaused || !bSpawningArmed)
	{
		return;
	}

	if (PendingSpawns.Num() > 0 || World->GetTimerManager().IsTimerActive(StaggeredSpawnTimerHandle))
	{
		return;
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	const UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	if (RunState && RunState->GetStageTimerActive() && GameMode && GameMode->IsUsingTowerMainMapLayout())
	{
		ScheduleNextTowerRuntimeWave(0.05f);
	}
}

void AT66EnemyDirector::NotifyMobDied(AT66MobBase* Mob)
{
	if (!Mob)
	{
		return;
	}

	if (LightweightAliveCount > 0)
	{
		--LightweightAliveCount;
	}
	T66DecrementLightweightFamilyCounter(Mob->GetEnemyFamily(), LightweightMeleeAliveCount, LightweightRushAliveCount, LightweightFlyingAliveCount, LightweightRangedAliveCount);

	UE_LOG(
		LogT66EnemyDirector,
		Verbose,
		TEXT("[LightweightMob] Mob died MobID=%s family=%d richAlive=%d lightweightAlive=%d lightweightMelee=%d lightweightRush=%d lightweightFlying=%d lightweightRanged=%d totalAlive=%d"),
		Mob->MobID.IsNone() ? TEXT("unset") : *Mob->MobID.ToString(),
		static_cast<int32>(Mob->GetEnemyFamily()),
		AliveCount,
		LightweightAliveCount,
		LightweightMeleeAliveCount,
		LightweightRushAliveCount,
		LightweightFlyingAliveCount,
		LightweightRangedAliveCount,
		GetAliveEnemyCount());

	UWorld* World = GetWorld();
	if (!World || bSpawningPaused || !bSpawningArmed)
	{
		return;
	}

	if (PendingSpawns.Num() > 0 || World->GetTimerManager().IsTimerActive(StaggeredSpawnTimerHandle))
	{
		return;
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	const UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	if (RunState && RunState->GetStageTimerActive() && GameMode && GameMode->IsUsingTowerMainMapLayout())
	{
		ScheduleNextTowerRuntimeWave(0.05f);
	}
}

bool AT66EnemyDirector::ShouldRouteSpawnToLightweightMob(
	const FName MobID,
	const ET66EnemyFamily Family,
	const bool bIsMiniBoss,
	const bool bIsSpecialSpawn) const
{
	if (MobID.IsNone() || bIsMiniBoss || bIsSpecialSpawn)
	{
		return false;
	}

	return Family == ET66EnemyFamily::Melee
		|| Family == ET66EnemyFamily::Rush
		|| Family == ET66EnemyFamily::Flying
		|| Family == ET66EnemyFamily::Ranged;
}

int32 AT66EnemyDirector::GetAliveEnemyCountForSpawnBudget()
{
	const int32 CombinedAliveCount = GetAliveEnemyCount();
	if (!bLoggedLightweightCountWidening && LightweightAliveCount > 0)
	{
		UE_LOG(
			LogT66EnemyDirector,
			Warning,
			TEXT("[LightweightMob] Wave progression/live-count widened: richAlive=%d lightweightAlive=%d combinedAlive=%d."),
			AliveCount,
			LightweightAliveCount,
			CombinedAliveCount);
		bLoggedLightweightCountWidening = true;
	}

	return CombinedAliveCount;
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
		ActiveStaggeredSpawnIntervalSeconds = FMath::Max(0.0f, StaggeredSpawnIntervalSeconds);
		ActiveRuntimeWaveCooldownSeconds = 0.05f;
		ActiveMaxSpawnsPerStaggeredBatch = FMath::Max(1, MaxSpawnsPerStaggeredBatch);
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
	const AT66GameMode* ActiveGameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	const bool bTowerLayout = ActiveGameMode && ActiveGameMode->IsUsingTowerMainMapLayout();
	if (bTowerLayout)
	{
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
		if (PendingSpawns.Num() <= 0 && !World->GetTimerManager().IsTimerActive(StaggeredSpawnTimerHandle))
		{
			ScheduleNextTowerRuntimeWave(0.05f);
		}
		return;
	}

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
				(BaseSpawnIntervalSeconds * Snapshot.RuntimeTrickleIntervalScalar) / FMath::Max(1.0f, Snapshot.DifficultyScalar));

			bSpawningArmed = true;
			World->GetTimerManager().ClearTimer(SpawnTimerHandle);
			SpawnRuntimeTrickleWave();
			if (!bTowerLayout)
			{
				World->GetTimerManager().SetTimer(
					SpawnTimerHandle,
					this,
					&AT66EnemyDirector::SpawnRuntimeTrickleWave,
					RuntimeSpawnInterval,
					true);
			}
		}
	}
	else
	{
		// Timer frozen: don't spawn waves.
		bSpawningArmed = false;
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
		World->GetTimerManager().ClearTimer(StaggeredSpawnTimerHandle);
		PendingSpawns.Empty();
		ActiveStaggeredSpawnIntervalSeconds = FMath::Max(0.0f, StaggeredSpawnIntervalSeconds);
		ActiveRuntimeWaveCooldownSeconds = 0.05f;
		ActiveMaxSpawnsPerStaggeredBatch = FMath::Max(1, MaxSpawnsPerStaggeredBatch);
	}
}

void AT66EnemyDirector::ScheduleNextTowerRuntimeWave(const float DelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(SpawnTimerHandle);
	World->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AT66EnemyDirector::SpawnRuntimeTrickleWave,
		FMath::Max(0.05f, DelaySeconds),
		false);
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
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	FRandomStream LocalRng(static_cast<int32>(FPlatformTime::Cycles()));
	FRandomStream& Rng = RngSub ? RngSub->GetRunStream() : LocalRng;
	APawn* PlayerPawn = PlayerPawns[RngSub ? RngSub->RunRandRange(0, PlayerPawns.Num() - 1) : Rng.RandRange(0, PlayerPawns.Num() - 1)];
	if (!PlayerPawn) return;
	const FVector PlayerLoc = PlayerPawn->GetActorLocation();

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
	T66TowerMapTerrain::FLayout TowerLayout;
	const bool bHasTowerLayout = bTowerLayout && GameMode && GameMode->GetTowerMainMapLayout(TowerLayout);
	const int32 PlayerTowerFloorNumber = bHasTowerLayout ? GameMode->GetTowerFloorIndexForLocation(PlayerLoc) : INDEX_NONE;
	if (bTowerLayout && !T66EnableTowerEnemySpawns)
	{
		return;
	}
	if (bTowerLayout)
	{
		const float RuntimeSpawnInterval = bPandemoniumMode
			? FMath::Max(0.05f, PandemoniumRuntimeSpawnIntervalSeconds)
			: FMath::Max(0.10f, Snapshot.SpawnBudget.RuntimeSpawnIntervalSeconds);
		const float RuntimeWaveDuration = FMath::Max(0.0f, Snapshot.SpawnBudget.RuntimeWaveStaggerDurationSeconds);
		ActiveRuntimeWaveCooldownSeconds = FMath::Max(0.0f, RuntimeSpawnInterval - RuntimeWaveDuration);
	}

	// Difficulty scaling affects enemy count (waves + max alive) outside the tower progression curve.
	const float Scalar = RunState->GetDifficultyScalar();
	const float FinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
	const float SpawnScalar = Scalar * FinaleScalar * Snapshot.RuntimeTrickleCountScalar;
	int32 EffectivePerWave = 0;
	int32 EffectiveMaxAlive = 0;
	if (bTowerLayout)
	{
		EffectivePerWave = bPandemoniumMode
			? FMath::Max(1, PandemoniumRuntimeEnemiesPerWave)
			: FMath::Max(1, Snapshot.SpawnBudget.RuntimeEnemiesPerWave);
		EffectiveMaxAlive = bPandemoniumMode
			? FMath::Max(EffectivePerWave, PandemoniumRuntimeMaxAliveEnemies)
			: FMath::Max(
				EffectivePerWave,
				T66ResolveRuntimeMaxAliveEnemies(Snapshot.SpawnBudget.RuntimeMaxAliveEnemies, MaxAliveEnemies));
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

	const int32 AliveCountForSpawnBudget = GetAliveEnemyCountForSpawnBudget();
	int32 ToSpawn = FMath::Min(EffectivePerWave, EffectiveMaxAlive - AliveCountForSpawnBudget);
	if (ToSpawn <= 0)
	{
		if (bTowerLayout)
		{
			ScheduleNextTowerRuntimeWave(0.25f);
		}
		return;
	}

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
		for (const TWeakObjectPtr<AT66NPCBase>& WeakNPC : Registry->GetNPCs())
		{
			AT66NPCBase* NPC = WeakNPC.Get();
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
		for (const TWeakObjectPtr<UT66SafeZoneComponent>& WeakSafeZone : Registry->GetSafeZones())
		{
			const UT66SafeZoneComponent* SafeZone = WeakSafeZone.Get();
			if (!SafeZone) continue;
			const FVector SafeZoneLocation = SafeZone->GetComponentLocation();
			if (bTowerLayout && GameMode && CandidateFloorNumber != INDEX_NONE
				&& GameMode->GetTowerFloorIndexForLocation(SafeZoneLocation) != CandidateFloorNumber)
			{
				continue;
			}
			const float R = SafeZone->GetSafeZoneRadius();
			if (FVector::DistSquared2D(Loc, SafeZoneLocation) < (R * R))
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

	// Stage mobs: pull exact non-empty roster from DT_Stages (EnemyA..EnemyL). Fallback is deterministic IDs.
	const int32 StageNum = RunState->GetCurrentStage();
	TArray<FName> MobIDs;
	T66ResolveStageMobIDs(GI, StageNum, MobIDs);

#if !UE_BUILD_SHIPPING
	static int32 LoggedMobWaves = 0;
	if (LoggedMobWaves < 3)
	{
		++LoggedMobWaves;
		TArray<FString> MobDebugParts;
		MobDebugParts.Reserve(MobIDs.Num());
		for (int32 MobIndex = 0; MobIndex < MobIDs.Num(); ++MobIndex)
		{
			MobDebugParts.Add(FString::Printf(TEXT("%d=%s"), MobIndex + 1, *MobIDs[MobIndex].ToString()));
		}
		UE_LOG(LogT66EnemyDirector, Verbose, TEXT("[SPAWN] SpawnWave Stage=%d MobIDs: %s (generic fallback would be Mob_StageXX_X - if you see that, reimport DT_Stages)"),
			StageNum, *FString::Join(MobDebugParts, TEXT("  ")));
	}
#endif

	// Build the exact spawn plan for this wave. All wave enemies are stage mobs;
	// the Goblin Thief special was removed in the roster restructure.
	const int32 MobToSpawn = FMath::Max(0, ToSpawn);
	TArray<TSubclassOf<AT66EnemyBase>> SpawnPlan;
	SpawnPlan.Reserve(ToSpawn);
	for (int32 i = 0; i < MobToSpawn; ++i) SpawnPlan.Add(RegularClass);

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
		bool bSpawnFromWall = false;
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
				bSpawnFromWall = true;
				if (!IsInAnySafeZone2D(SpawnLoc)
					&& !IsInBlockedTraversalZone2D(SpawnLoc)
					&& IsFarEnoughFromPlayers2D(SpawnLoc, EffectiveSpawnMin))
				{
					bFoundSpawnLoc = true;
					break;
				}
			}

			if (!bFoundSpawnLoc && bHasTowerLayout && PlayerTowerFloorNumber != INDEX_NONE)
			{
				for (int32 Try = 0; Try < 18; ++Try)
				{
					FVector SurfaceSpawn = FVector::ZeroVector;
					if (!T66TowerMapTerrain::TryGetRandomSurfaceLocationOnFloor(
						World,
						TowerLayout,
						PlayerTowerFloorNumber,
						Rng,
						SurfaceSpawn,
						600.0f,
						850.0f))
					{
						continue;
					}

					SpawnLoc = ResolveGroundedSpawnLocation(SurfaceSpawn);
					SpawnWallNormal = FVector::ZeroVector;
					bSpawnFromWall = false;
					if (!IsInAnySafeZone2D(SpawnLoc)
						&& !IsInBlockedTraversalZone2D(SpawnLoc)
						&& IsFarEnoughFromPlayers2D(SpawnLoc, EffectiveSpawnMin))
					{
						bFoundSpawnLoc = true;
						break;
					}
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
			for (const TWeakObjectPtr<AT66NPCBase>& WeakNPC : Registry->GetNPCs())
			{
				AT66NPCBase* NPC = WeakNPC.Get();
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
			for (const TWeakObjectPtr<UT66SafeZoneComponent>& WeakSafeZone : Registry->GetSafeZones())
			{
				const UT66SafeZoneComponent* SafeZone = WeakSafeZone.Get();
				if (!SafeZone) continue;
				const FVector SafeZoneLocation = SafeZone->GetComponentLocation();
				const float R = SafeZone->GetSafeZoneRadius();
				FVector ToSpawnPt = SpawnLoc - SafeZoneLocation;
				ToSpawnPt.Z = 0.f;
				const float Dist2D = ToSpawnPt.Size();
				if (Dist2D < R && Dist2D > 1.f)
				{
					FVector Dir = ToSpawnPt / Dist2D;
					SpawnLoc = SafeZoneLocation + FVector(Dir.X, Dir.Y, 0.f) * (R + SafeZonePushMargin);
					SpawnLoc.Z = PlayerGroundZ;
					break;
				}
				else if (Dist2D <= 1.f)
				{
					FVector Dir(Rng.FRandRange(-1.f, 1.f), Rng.FRandRange(-1.f, 1.f), 0.f);
					Dir.Z = 0.f;
					if (Dir.Normalize())
					{
						SpawnLoc = SafeZoneLocation + Dir * (R + SafeZonePushMargin);
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

		const FName MobID = MobIDs[RngSub ? RngSub->RunRandRange(0, MobIDs.Num() - 1) : Rng.RandRange(0, MobIDs.Num() - 1)];
		const ET66EnemyFamily MobFamily = T66ResolveStageEnemyFamily(T66GI, MobID);
		const FName MobArchetype = T66ResolveStageEnemyArchetype(T66GI, MobID);
		ClassToSpawn = T66ResolveStageEnemyClass(T66GI, MobID, RegularClass);

		FPendingEnemySpawn Slot;
		Slot.GroundLocation = SpawnLoc;
		Slot.ClassToSpawn = ClassToSpawn;
		Slot.MobID = MobID;
		Slot.Family = MobFamily;
		Slot.Archetype = MobArchetype;
		Slot.bIsMiniBoss = false;
		Slot.bSpawnFromWall = bTowerLayout && bSpawnFromWall;
		Slot.DifficultyScalar = Scalar;
		Slot.FinaleScalar = FinaleScalar;
		Slot.EnemyProgressionScalar = Snapshot.EnemyStatScalar;
		Slot.StageNum = RunState ? RunState->GetCurrentStage() : 1;
		Slot.WallNormal = SpawnWallNormal;
		Slot.Channel = ET66EnemySpawnChannel::RuntimeTrickle;
		PendingSpawns.Add(Slot);
	}

	if (PendingSpawns.Num() <= 0)
	{
		if (bTowerLayout)
		{
			ScheduleNextTowerRuntimeWave(0.25f);
		}
		return;
	}

	ActiveMaxSpawnsPerStaggeredBatch = bTowerLayout
		? (bPandemoniumMode
			? FMath::Clamp(PandemoniumMaxSpawnsPerBatch, 1, 128)
			: FMath::Clamp(Snapshot.SpawnBudget.RuntimeMaxSpawnsPerStaggeredBatch, 1, 128))
		: FMath::Max(1, MaxSpawnsPerStaggeredBatch);
	const int32 TotalStaggeredBatches = FMath::CeilToInt(static_cast<float>(PendingSpawns.Num()) / static_cast<float>(ActiveMaxSpawnsPerStaggeredBatch));
	if (bTowerLayout && Snapshot.SpawnBudget.RuntimeWaveStaggerDurationSeconds > 0.0f && TotalStaggeredBatches > 1)
	{
		ActiveStaggeredSpawnIntervalSeconds = Snapshot.SpawnBudget.RuntimeWaveStaggerDurationSeconds / static_cast<float>(TotalStaggeredBatches - 1);
	}
	else
	{
		ActiveStaggeredSpawnIntervalSeconds = FMath::Max(0.0f, StaggeredSpawnIntervalSeconds);
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
	UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>();
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	const UT66RngTuningConfig* Tuning = RngSub ? RngSub->GetTuning() : nullptr;
	FRandomStream LocalRng(static_cast<int32>(FPlatformTime::Cycles()));
	FRandomStream& Rng = RngSub ? RngSub->GetRunStream() : LocalRng;
	AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	const bool bTowerLayout = GameMode && GameMode->IsUsingTowerMainMapLayout();

	const int32 BatchSize = FMath::Max(1, ActiveMaxSpawnsPerStaggeredBatch);
	int32 ProcessedCount = 0;
	while (ProcessedCount < PendingSpawns.Num() && ProcessedCount < BatchSize)
	{
		const FPendingEnemySpawn Slot = PendingSpawns[ProcessedCount];
		++ProcessedCount;

		const bool bIsMob = !Slot.MobID.IsNone();
		AT66EnemyBase* Enemy = nullptr;

		const bool bShouldRouteLightweight = ShouldRouteSpawnToLightweightMob(Slot.MobID, Slot.Family, Slot.bIsMiniBoss, !bIsMob);
		const ET66RouteAttributionChannel RouteChannel = Slot.Channel == ET66EnemySpawnChannel::InitialPopulation
			? ET66RouteAttributionChannel::InitialPopulation
			: ET66RouteAttributionChannel::RuntimeTrickle;
		if (bShouldRouteLightweight)
		{
			const FTransform Xform(FRotator::ZeroRotator, Slot.GroundLocation);
			bool bRequiresFinishSpawning = false;
			AT66MobBase* Mob = MobManager ? MobManager->AcquireMob(AT66MobBase::StaticClass(), Xform, &bRequiresFinishSpawning) : nullptr;
			if (Mob)
			{
				Mob->OwningDirector = this;
				Mob->MobID = Slot.MobID;
				Mob->CharacterVisualID = Slot.MobID;
				Mob->ConfigureAsMob(
					Slot.MobID,
					Slot.Family,
					Slot.Archetype,
					Slot.StageNum,
					Slot.DifficultyScalar,
					Slot.EnemyProgressionScalar,
					Slot.FinaleScalar,
					false);
				if (bRequiresFinishSpawning)
				{
					UGameplayStatics::FinishSpawningActor(Mob, Xform);
				}
				++LightweightAliveCount;
				T66IncrementLightweightFamilyCounter(Mob->GetEnemyFamily(), LightweightMeleeAliveCount, LightweightRushAliveCount, LightweightFlyingAliveCount, LightweightRangedAliveCount);
				if (MobManager && Mob->GetEnemyFamily() == ET66EnemyFamily::Ranged)
				{
					MobManager->RecordRangedMobSpawn(true, Mob->MobID);
				}
				if (MobManager)
				{
					MobManager->RecordRouteAttribution(Mob->GetEnemyFamily(), ET66RouteAttributionReason::RoutedLightweight_BasicFamily, RouteChannel);
				}
				UE_LOG(
					LogT66EnemyDirector,
					VeryVerbose,
					TEXT("[LightweightMob] Routed spawn MobID=%s family=%d channel=%d richAlive=%d lightweightAlive=%d lightweightMelee=%d lightweightRush=%d lightweightFlying=%d lightweightRanged=%d totalAlive=%d location=%s"),
					*Slot.MobID.ToString(),
					static_cast<int32>(Slot.Family),
					static_cast<int32>(Slot.Channel),
					AliveCount,
					LightweightAliveCount,
					LightweightMeleeAliveCount,
					LightweightRushAliveCount,
					LightweightFlyingAliveCount,
					LightweightRangedAliveCount,
					GetAliveEnemyCount(),
					*Slot.GroundLocation.ToCompactString());
				continue;
			}

			if (MobManager)
			{
				MobManager->RecordLightweightAcquireFailed(Slot.Family, RouteChannel);
			}
			UE_LOG(
				LogT66EnemyDirector,
				Warning,
				TEXT("[LightweightMob] Failed to spawn lightweight MobID=%s; skipping basic-mob spawn because rich basic routing is deprecated."),
				*Slot.MobID.ToString());
			continue;
		}

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
			if (RunState)
			{
				Enemy->ApplyStageScaling(Slot.StageNum);
				Enemy->ApplyDifficultyScalar(Slot.DifficultyScalar);
				Enemy->ApplyProgressionEnemyScalar(Slot.EnemyProgressionScalar);
			}
			Enemy->ApplyFinaleScaling(Slot.FinaleScalar);
			if (RunState)
			{
				Enemy->FreezeScoreAwardAtSpawn(Slot.DifficultyScalar);
				RunState->RegisterSpawnedEnemyScoreBudget(Enemy->GetResolvedScoreAward(), Slot.StageNum);
			}
			AliveCount++;
			if (MobManager && Enemy->EnemyFamily == ET66EnemyFamily::Ranged)
			{
				MobManager->RecordRangedMobSpawn(false, Enemy->MobID);
			}
			if (MobManager)
			{
				MobManager->RecordRouteAttribution(
					Slot.Family,
					T66ResolveRouteAttributionReason(Slot.MobID, Slot.Family, Slot.bIsMiniBoss, !bIsMob),
					RouteChannel);
			}
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
			FMath::Max(0.0f, ActiveStaggeredSpawnIntervalSeconds),
			false);
	}
	else
	{
		World->GetTimerManager().ClearTimer(StaggeredSpawnTimerHandle);
		ActiveStaggeredSpawnIntervalSeconds = FMath::Max(0.0f, StaggeredSpawnIntervalSeconds);
		ActiveMaxSpawnsPerStaggeredBatch = FMath::Max(1, MaxSpawnsPerStaggeredBatch);
		if (bTowerLayout && !bSpawningPaused && bSpawningArmed && RunState && RunState->GetStageTimerActive())
		{
			ScheduleNextTowerRuntimeWave(ActiveRuntimeWaveCooldownSeconds);
		}
	}
}

