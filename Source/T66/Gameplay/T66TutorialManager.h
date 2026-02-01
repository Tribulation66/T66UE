// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66TutorialManager.generated.h"

class AT66EnemyBase;
class AT66LootBagPickup;
class AT66TutorialPortal;
class UT66RunStateSubsystem;

/** Stage 1 tutorial flow (v1): on-screen text above crosshair + event-driven steps. */
UCLASS(Blueprintable)
class T66_API AT66TutorialManager : public AActor
{
	GENERATED_BODY()

public:
	AT66TutorialManager();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	enum class ET66TutorialStep : uint8
	{
		None,
		MoveAndJump,
		AutoAttackEnemy,
		PickupFirstBag,
		ItemExplanation,
		UltMiniBoss,
		WhiteItemExplanation,
		BigWave,
		PortalReady,
		Done,
	};

	ET66TutorialStep Step = ET66TutorialStep::None;

	UPROPERTY()
	TObjectPtr<UT66RunStateSubsystem> RunState;

	UPROPERTY()
	TObjectPtr<AT66EnemyBase> FirstEnemy;

	UPROPERTY()
	TObjectPtr<AT66EnemyBase> MiniBossEnemy;

	UPROPERTY()
	TObjectPtr<AT66TutorialPortal> TutorialPortal;

	int32 InventoryCountAtFirstBagSpawn = 0;
	int32 RemainingWaveEnemies = 0;

	FTimerHandle StepTimeoutHandle;

	void StartMoveAndJumpStep();
	void StartAutoAttackEnemyStep();
	void StartPickupFirstBagStep(const FVector& BagWorldLocation);
	void StartItemExplanationStep();
	void StartUltMiniBossStep();
	void StartWhiteItemExplanationStep(const FVector& BagWorldLocation);
	void StartBigWaveStep();
	void SpawnPortalAndFinish();

	FName PickTutorialPrimaryStatItemID() const;
	FName PickStage1MobID() const;
	FVector GetGroundPoint(const FVector& InLocation) const;
	FVector SnapEnemySpawnToGround(const FVector& InLocation) const;
	AT66EnemyBase* SpawnTutorialEnemyAt(const FVector& InLocation, FName MobID, bool bMiniBoss);

	void ClearStepTimeout();

	UFUNCTION()
	void HandleTutorialInputChanged();

	UFUNCTION()
	void HandleInventoryChanged();

	UFUNCTION()
	void HandleFirstEnemyDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void HandleMiniBossDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void HandleWaveEnemyDestroyed(AActor* DestroyedActor);
};

