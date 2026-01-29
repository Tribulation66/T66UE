// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66EnemyDirector.generated.h"

class AT66EnemyBase;

UCLASS(Blueprintable)
class T66_API AT66EnemyDirector : public AActor
{
	GENERATED_BODY()

public:
	AT66EnemyDirector();

	/** Enemy class to spawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<AT66EnemyBase> EnemyClass;

	/** Spawn interval in seconds */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	float SpawnIntervalSeconds = 10.f;

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

	FTimerHandle SpawnTimerHandle;
	int32 AliveCount = 0;
};
