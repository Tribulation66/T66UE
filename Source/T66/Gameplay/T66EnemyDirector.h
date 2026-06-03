// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Enemies/T66EnemyFamilyTypes.h"
#include "T66EnemyDirector.generated.h"

class AT66EnemyBase;
class AT66MobBase;
class UT66RunStateSubsystem;

UENUM(BlueprintType)
enum class ET66EnemySpawnChannel : uint8
{
	InitialPopulation UMETA(DisplayName = "Initial Population"),
	RuntimeTrickle UMETA(DisplayName = "Runtime Trickle"),
};

/** One queued spawn for staggered spawning (1-2 at a time). */
USTRUCT(BlueprintType)
struct FPendingEnemySpawn
{
	GENERATED_BODY()

	FVector GroundLocation = FVector::ZeroVector;
	TSubclassOf<AT66EnemyBase> ClassToSpawn;
	FName MobID;
	ET66EnemyFamily Family = ET66EnemyFamily::Special;
	FName Archetype = NAME_None;
	bool bIsMiniBoss = false;
	bool bSpawnFromWall = false;
	float DifficultyScalar = 1.f;
	float FinaleScalar = 1.f;
	float EnemyProgressionScalar = 1.f;
	int32 StageNum = 1;
	FVector WallNormal = FVector::ZeroVector;
	ET66EnemySpawnChannel Channel = ET66EnemySpawnChannel::RuntimeTrickle;
};

UCLASS(Blueprintable)
class T66_API AT66EnemyDirector : public AActor
{
	GENERATED_BODY()

public:
	AT66EnemyDirector();

	/** Enemy class to spawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<AT66EnemyBase> EnemyClass;

	/** Seconds between spawn ticks (continuous spawning). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float SpawnIntervalSeconds = 2.f;

	/** Enemies spawned per staggered batch while a wave is being materialized. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	int32 MaxSpawnsPerStaggeredBatch = 2;

	/** Delay between staggered spawn batches so large waves do not land in one frame. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float StaggeredSpawnIntervalSeconds = 0.05f;

	/** Enemies spawned per tick. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	int32 EnemiesPerWave = 1;

	/** Max alive enemies (hard cap to protect FPS). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	int32 MaxAliveEnemies = 50;

	/** Min distance from player to spawn (uu) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float SpawnMinDistance = 400.f;

	/** Max distance from player to spawn (uu) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float SpawnMaxDistance = 1000.f;

	/** Enemies must not spawn closer than one full gameplay grid cell to any player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float MinimumPlayerSpawnClearance = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	int32 InitialTowerEnemiesPerMobFloor = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	float InitialTowerSpawnEdgePadding = 1500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower")
	float InitialTowerSpawnHolePadding = 1800.f;

	/** When true, no enemy waves are spawned (timer not armed, SpawnWave no-op). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
	bool bSpawningPaused = false;

	/** Called by enemy when it dies */
	UFUNCTION(BlueprintCallable, Category = "Director")
	void NotifyEnemyDied(AT66EnemyBase* Enemy);

	void NotifyMobDied(AT66MobBase* Mob);

	UFUNCTION(BlueprintCallable, Category = "Director")
	void SetSpawningPaused(bool bPaused);

	UFUNCTION(BlueprintCallable, Category = "Director")
	void RefreshSpawningFromProgression();

	UFUNCTION(BlueprintCallable, Category = "Director|Pandemonium")
	void SetPandemoniumMode(bool bEnabled, float RuntimeSpawnIntervalSeconds, int32 RuntimeEnemiesPerWave, int32 RuntimeMaxAliveEnemies, int32 RuntimeMaxSpawnsPerBatch);

	UFUNCTION(BlueprintCallable, Category = "Director|Tower")
	int32 SpawnInitialPopulationForTowerFloor(int32 FloorNumber);

	int32 GetAliveEnemyCount() const { return AliveCount + LightweightAliveCount; }
	int32 GetAliveRichEnemyCount() const { return AliveCount; }
	int32 GetAliveLightweightMobCount() const { return LightweightAliveCount; }
	int32 GetAliveLightweightMeleeMobCount() const { return LightweightMeleeAliveCount; }
	int32 GetAliveLightweightRushMobCount() const { return LightweightRushAliveCount; }
	int32 GetAliveLightweightFlyingMobCount() const { return LightweightFlyingAliveCount; }
	int32 GetAliveLightweightRangedMobCount() const { return LightweightRangedAliveCount; }
	int32 GetPendingSpawnCount() const { return PendingSpawns.Num(); }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SpawnInitialPopulationForStage();
	void SpawnRuntimeTrickleWave();

	/** Spawns 1-2 from PendingSpawns, then re-arms timer if more remain. */
	void SpawnNextStaggeredBatch();

	void ScheduleNextTowerRuntimeWave(float DelaySeconds);
	bool ShouldRouteSpawnToLightweightMob(FName MobID, ET66EnemyFamily Family, bool bIsMiniBoss, bool bIsSpecialSpawn) const;
	int32 GetAliveEnemyCountForSpawnBudget();

	UFUNCTION()
	void HandleStageTimerChanged();

	FTimerHandle SpawnTimerHandle;
	FTimerHandle StaggeredSpawnTimerHandle;
	int32 AliveCount = 0;
	int32 LightweightAliveCount = 0;
	int32 LightweightMeleeAliveCount = 0;
	int32 LightweightRushAliveCount = 0;
	int32 LightweightFlyingAliveCount = 0;
	int32 LightweightRangedAliveCount = 0;
	bool bSpawningArmed = false;
	bool bLoggedLightweightCountWidening = false;

	TArray<FPendingEnemySpawn> PendingSpawns;
	float ActiveStaggeredSpawnIntervalSeconds = 0.05f;
	float ActiveRuntimeWaveCooldownSeconds = 0.05f;
	int32 ActiveMaxSpawnsPerStaggeredBatch = 1;
	bool bPandemoniumMode = false;
	float PandemoniumRuntimeSpawnIntervalSeconds = 0.30f;
	int32 PandemoniumRuntimeEnemiesPerWave = 18;
	int32 PandemoniumRuntimeMaxAliveEnemies = 180;
	int32 PandemoniumMaxSpawnsPerBatch = 6;

	TSet<int32> TowerFloorsWithInitialPopulation;

	// Cache base spawn counts so difficulty scaling doesn't compound.
	int32 BaseEnemiesPerWave = 0;
	int32 BaseMaxAliveEnemies = 0;
	float BaseSpawnIntervalSeconds = 0.f;
};
