// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66EnemyDirector.generated.h"

class AT66EnemyBase;
class UT66RunStateSubsystem;

UCLASS(Blueprintable)
class T66_API AT66EnemyDirector : public AActor
{
	GENERATED_BODY()

public:
	AT66EnemyDirector();

	/** Enemy class to spawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<AT66EnemyBase> EnemyClass;

	/** Special: Leprechaun (runs away, grants gold on hit) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<AT66EnemyBase> LeprechaunClass;

	/** Special: Goblin Thief (steals gold on touch) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<AT66EnemyBase> GoblinThiefClass;

	/** Chance per spawned enemy to be a Leprechaun */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float LeprechaunChance = 0.05f;

	/** Chance per spawned enemy to be a Goblin Thief */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float GoblinThiefChance = 0.05f;

	/** Unique enemy (flying debuff shooter). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<AT66EnemyBase> UniqueEnemyClass;

	/** Chance per spawn wave to also spawn a Unique enemy (if none active). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float UniqueEnemyChancePerWave = 0.25f;

	/** Spawn interval in seconds */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float SpawnIntervalSeconds = 20.f;

	/** Enemies per spawn wave */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	int32 EnemiesPerWave = 3;

	/** Max alive enemies */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	int32 MaxAliveEnemies = 12;

	/** Min distance from player to spawn (uu) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float SpawnMinDistance = 400.f;

	/** Max distance from player to spawn (uu) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float SpawnMaxDistance = 1000.f;

	/** Called by enemy when it dies */
	UFUNCTION(BlueprintCallable, Category = "Director")
	void NotifyEnemyDied(AT66EnemyBase* Enemy);

protected:
	virtual void BeginPlay() override;

	void SpawnWave();

	UFUNCTION()
	void HandleStageTimerChanged();

	FTimerHandle SpawnTimerHandle;
	int32 AliveCount = 0;
	bool bSpawningArmed = false;

	TWeakObjectPtr<AT66EnemyBase> ActiveUniqueEnemy;
};
