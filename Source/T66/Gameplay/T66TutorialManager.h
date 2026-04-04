// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66TutorialManager.generated.h"

class AT66DifficultyTotem;
class AT66EnemyBase;
class AT66IdolAltar;
class AT66TreeOfLifeInteractable;
class AT66TutorialGuideCompanion;
class AT66TutorialPortal;
class APawn;
class UT66RunStateSubsystem;

/**
 * Authored tutorial flow for Gameplay_Tutorial.
 * Route markers and encounter anchors come from actors placed in the map, not hardcoded combat coordinates.
 */
UCLASS(Blueprintable)
class T66_API AT66TutorialManager : public AActor
{
	GENERATED_BODY()

public:
	AT66TutorialManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	enum class ET66TutorialStep : uint8
	{
		None,
		Arrival,
		MoveLookJump,
		AutoAttack,
		LockTarget,
		PickupItem,
		IdolLesson,
		FountainLesson,
		TotemLesson,
		FinalArena,
		PortalReady,
		Done,
	};

	void StartArrivalStep();
	void StartMoveLookJumpStep();
	void StartAutoAttackStep();
	void StartLockTargetStep();
	void StartPickupItemStep();
	void StartIdolLessonStep();
	void StartFountainLessonStep();
	void StartTotemLessonStep();
	void StartFinalArenaStep();
	void SpawnPortalAndFinish();

	bool TrySpawnGuide();
	APawn* GetPlayerPawn() const;
	AActor* FindTaggedActor(FName Tag) const;
	FVector GetTaggedLocation(FName Tag, const FVector& Fallback) const;
	FRotator GetTaggedRotation(FName Tag, const FRotator& Fallback) const;

	void AdvanceGuideToMarker(FName Tag, float AcceptanceRadius = 120.f);
	void SetTutorialPresentation(const FText& SubtitleText, const FText& HintLine1, const FText& HintLine2 = FText::GetEmpty()) const;
	bool HasElectricIdolEquipped() const;
	void SpawnTotemPreviewWave();
	void SpawnFinalArenaWave();
	void TryFinishFinalArenaStep();

	FName PickTutorialPrimaryStatItemID() const;
	FName PickStage1MobID() const;
	FVector GetGroundPoint(const FVector& InLocation) const;
	FVector SnapEnemySpawnToGround(const FVector& InLocation) const;
	AT66EnemyBase* SpawnTutorialEnemyAt(const FVector& InLocation, FName MobID, bool bMiniBoss);

	UFUNCTION()
	void HandleTutorialInputChanged();

	UFUNCTION()
	void HandleInventoryChanged();

	UFUNCTION()
	void HandleIdolsChanged();

	UFUNCTION()
	void HandleHeartsChanged();

	UFUNCTION()
	void HandleDifficultyChanged();

	UFUNCTION()
	void HandleAutoAttackEnemyDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void HandleLockEnemyDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void HandleTotemWaveEnemyDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void HandleFinalArenaEnemyDestroyed(AActor* DestroyedActor);

	ET66TutorialStep Step = ET66TutorialStep::None;

	UPROPERTY()
	TObjectPtr<UT66RunStateSubsystem> RunState;

	UPROPERTY()
	TObjectPtr<AT66TutorialGuideCompanion> GuideCompanion;

	UPROPERTY()
	TObjectPtr<AT66EnemyBase> AutoAttackEnemy;

	UPROPERTY()
	TObjectPtr<AT66EnemyBase> LockEnemy;

	UPROPERTY()
	TObjectPtr<AT66IdolAltar> TutorialIdolAltar;

	UPROPERTY()
	TObjectPtr<AT66TreeOfLifeInteractable> TutorialFountain;

	UPROPERTY()
	TObjectPtr<AT66DifficultyTotem> TutorialDifficultyTotem;

	UPROPERTY()
	TObjectPtr<AT66TutorialPortal> TutorialPortal;

	int32 InventoryCountAtItemStepStart = 0;
	float MaxHPBeforeFountainUse = 0.f;
	int32 TotemsActivatedCountAtStepStart = 0;
	int32 RemainingTotemWaveEnemies = 0;
	int32 RemainingFinalArenaEnemies = 0;
	bool bTotemPreviewWaveSpawned = false;
};
