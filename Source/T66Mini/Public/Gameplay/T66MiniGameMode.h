// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T66MiniGameMode.generated.h"

class AT66MiniArena;
class AT66MiniCompanionBase;
class AT66MiniEnemyBase;
class AT66MiniGroundTelegraphActor;
class UAudioComponent;

UCLASS()
class T66MINI_API AT66MiniGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AT66MiniGameMode();

	void HandleEnemyKilled(AT66MiniEnemyBase* Enemy);
	void HandlePlayerDefeated();
	FVector ClampPointToArena(const FVector& WorldLocation) const;
	void SaveRunProgressNow(bool bMarkMidWaveSnapshot = true);
	void RefreshAudioMix();
	FVector GetArenaOrigin() const { return ArenaOrigin; }
	float GetArenaHalfExtent() const { return ArenaHalfExtent; }
	bool HasBossTelegraphActive() const { return BossTelegraphRemaining > 0.f; }
	float GetBossTelegraphRemaining() const { return BossTelegraphRemaining; }
	FVector GetBossTelegraphLocation() const { return PendingBossSpawnLocation; }
	FName GetPendingBossID() const { return PendingBossID; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void SpawnArenaAndPositionPlayer();
	void StartWave(int32 WaveIndex, bool bResetTimer);
	const struct FT66MiniWaveDefinition* GetWaveDefinition() const;
	const struct FT66MiniDifficultyDefinition* GetDifficultyDefinition() const;
	const struct FT66MiniEnemyDefinition* ChooseEnemyDefinition() const;
	void SpawnWaveEnemy();
	void BeginBossSpawnTelegraph();
	void SpawnBossEnemy();
	void SpawnRandomInteractable();
	void ReturnToShopIntermission();
	void RecordClearedMiniStageProgression();
	void FinalizeRun(bool bWasVictory, const FString& ResultLabel);
	void TryApplySavedPawnState();
	void SpawnCompanionActor();
	void RestoreTransientWaveState(const class UT66MiniRunSaveGame* RunSave);
	void RestoreWorldState(const class UT66MiniRunSaveGame* RunSave);
	void CaptureWorldState(class UT66MiniRunSaveGame* RunSave) const;
	void PersistActiveRunSnapshot(bool bMarkMidWaveSnapshot);
	void UpdateLiveEnemyCache();

	float AutosaveAccumulator = 0.f;
	bool bAppliedSavedPawnState = false;
	float EnemySpawnAccumulator = 0.f;
	float InteractableSpawnAccumulator = 0.f;
	float PostBossDelayRemaining = 0.f;
	bool bBossSpawnedForWave = false;
	float BossTelegraphRemaining = 0.f;
	bool bRunCompleted = false;
	bool bRunFinalized = false;
	FName PendingBossID = NAME_None;
	FVector PendingBossSpawnLocation = FVector::ZeroVector;
	FVector ArenaOrigin = FVector(450000.f, 450000.f, 1200.f);
	float ArenaHalfExtent = 2200.f;

	UPROPERTY()
	TObjectPtr<AT66MiniArena> MiniArenaActor;

	UPROPERTY()
	TObjectPtr<AT66MiniCompanionBase> ActiveCompanionActor;

	UPROPERTY()
	TObjectPtr<AT66MiniGroundTelegraphActor> ActiveBossTelegraphActor;

	UPROPERTY()
	TArray<TObjectPtr<AT66MiniEnemyBase>> LiveEnemies;

	UPROPERTY()
	TObjectPtr<UAudioComponent> BattleMusicComponent;
};
