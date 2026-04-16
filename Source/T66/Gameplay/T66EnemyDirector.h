// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66EnemyDirector.generated.h"

class AT66EnemyBase;
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

	/** Special: Goblin Thief (steals gold on touch) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<AT66EnemyBase> GoblinThiefClass;

	/** Chance per spawned enemy to be a Goblin Thief */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float GoblinThiefChance = 0.05f;

	/** Chance per spawn wave to spawn a mini-boss (one of this stage's mobs, scaled up). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float MiniBossChancePerWave = 0.10f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float MiniBossScale = 1.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float MiniBossHPScalar = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float MiniBossDamageScalar = 2.0f;

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
	int32 InitialTowerEnemiesPerGameplayFloor = 3;

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

	UFUNCTION(BlueprintCallable, Category = "Director")
	void SetSpawningPaused(bool bPaused);

	UFUNCTION(BlueprintCallable, Category = "Director")
	void RefreshSpawningFromProgression();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SpawnInitialPopulationForStage();
	void SpawnRuntimeTrickleWave();

	/** Spawns 1-2 from PendingSpawns, then re-arms timer if more remain. */
	void SpawnNextStaggeredBatch();

	UFUNCTION()
	void HandleStageTimerChanged();

	FTimerHandle SpawnTimerHandle;
	FTimerHandle StaggeredSpawnTimerHandle;
	int32 AliveCount = 0;
	bool bSpawningArmed = false;

	TArray<FPendingEnemySpawn> PendingSpawns;

	TWeakObjectPtr<AT66EnemyBase> ActiveMiniBoss;

	// Cache base spawn counts so difficulty scaling doesn't compound.
	int32 BaseEnemiesPerWave = 0;
	int32 BaseMaxAliveEnemies = 0;
	float BaseSpawnIntervalSeconds = 0.f;
};
