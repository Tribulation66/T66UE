// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "GameFramework/GameModeBase.h"
#include "UI/T66UITypes.h"
#include "T66MiniGameMode.generated.h"

enum class ET66ScreenType : uint8;

class AT66MiniArena;
class AT66MiniCompanionBase;
class AT66MiniEnemyBase;
class AT66MiniHazardTrap;
class AT66MiniGroundTelegraphActor;
class AT66MiniInteractable;
class AT66MiniPickup;
class AT66MiniPlayerPawn;
class UAudioComponent;
class UT66MiniRunSaveGame;

struct FT66MiniCombatTextEntry
{
	FVector WorldLocation = FVector::ZeroVector;
	FVector Velocity = FVector(0.f, 0.f, 120.f);
	FString Label;
	FLinearColor Color = FLinearColor::White;
	float RemainingTime = 0.9f;
};

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
	AT66MiniEnemyBase* GetActiveBossEnemy() const;
	AT66MiniPlayerPawn* FindClosestPlayerPawn(const FVector& WorldLocation, bool bRequireAlive) const;
	const TArray<TObjectPtr<AT66MiniEnemyBase>>& GetLiveEnemies() const { return LiveEnemies; }
	const TArray<TObjectPtr<AT66MiniInteractable>>& GetLiveInteractables() const { return LiveInteractables; }
	const TArray<TObjectPtr<AT66MiniPickup>>& GetLivePickups() const { return LivePickups; }
	void RegisterLiveTrap(AT66MiniHazardTrap* Trap);
	void UnregisterLiveTrap(const AT66MiniHazardTrap* Trap);
	void RegisterLiveInteractable(AT66MiniInteractable* Interactable);
	void UnregisterLiveInteractable(const AT66MiniInteractable* Interactable);
	void RegisterLivePickup(AT66MiniPickup* Pickup);
	void UnregisterLivePickup(const AT66MiniPickup* Pickup);
	void AddCombatText(const FVector& WorldLocation, float Value, const FLinearColor& Color, float Duration = 0.9f, const FString& Prefix = FString());
	const TArray<FT66MiniCombatTextEntry>& GetCombatTexts() const { return CombatTexts; }
	bool TryInteractNearest(class AT66MiniPlayerPawn* PlayerPawn, float MaxRange = 190.f);
	void RequestPartySaveAndReturnToFrontend(const FString& ResultLabel, ET66ScreenType PendingScreen = ET66ScreenType::MiniMainMenu);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Logout(AController* Exiting) override;

private:
	void PositionPartyPawns();
	void SpawnArenaAndPositionPlayer();
	void StartWave(int32 WaveIndex, bool bResetTimer);
	const struct FT66MiniWaveDefinition* GetWaveDefinition() const;
	const struct FT66MiniDifficultyDefinition* GetDifficultyDefinition() const;
	const struct FT66MiniEnemyDefinition* ChooseEnemyDefinition() const;
	void SpawnWaveEnemy();
	void BeginBossSpawnTelegraph();
	void SpawnBossEnemy();
	void SpawnRandomInteractable();
	void SpawnRandomTrap();
	void ReturnToShopIntermission();
	void RecordClearedMiniStageProgression();
	void FinalizeRun(bool bWasVictory, const FString& ResultLabel);
	void TryApplySavedPawnState();
	void SpawnCompanionActor();
	void RestoreTransientWaveState(const UT66MiniRunSaveGame* RunSave);
	void RestoreWorldState(const UT66MiniRunSaveGame* RunSave);
	void CapturePartyPlayerSnapshots(UT66MiniRunSaveGame* RunSave) const;
	void CaptureWorldState(UT66MiniRunSaveGame* RunSave) const;
	void PersistActiveRunSnapshot(bool bMarkMidWaveSnapshot);
	void UpdateLiveEnemyCache();
	void UpdateLiveTrapCache();
	void UpdateLiveInteractableCache();
	void UpdateLivePickupCache();
	void UpdateCombatTexts(float DeltaSeconds);
	bool IsOnlinePartyMiniRun() const;
	void CompleteOnlineFrontendTravel();
	void AbortOnlinePartyRunToFrontend(const FString& ResultLabel, ET66ScreenType PendingScreen);

	float AutosaveAccumulator = 0.f;
	bool bAppliedSavedPawnState = false;
	float EnemySpawnAccumulator = 0.f;
	float InteractableSpawnAccumulator = 0.f;
	float TrapSpawnAccumulator = 0.f;
	float PostBossDelayRemaining = 0.f;
	bool bBossSpawnedForWave = false;
	float BossTelegraphRemaining = 0.f;
	bool bRunCompleted = false;
	bool bRunFinalized = false;
	bool bFrontendTravelInProgress = false;
	FName PendingBossID = NAME_None;
	FVector PendingBossSpawnLocation = FVector::ZeroVector;
	FVector ArenaOrigin = FVector(450000.f, 450000.f, 1200.f);
	float ArenaHalfExtent = 2200.f;

	UPROPERTY()
	TObjectPtr<AT66MiniArena> MiniArenaActor;

	UPROPERTY()
	TArray<TObjectPtr<AT66MiniCompanionBase>> ActiveCompanionActors;

	UPROPERTY()
	TObjectPtr<AT66MiniGroundTelegraphActor> ActiveBossTelegraphActor;

	UPROPERTY()
	TArray<TObjectPtr<AT66MiniEnemyBase>> LiveEnemies;

	UPROPERTY()
	TArray<TObjectPtr<AT66MiniHazardTrap>> LiveTraps;

	UPROPERTY()
	TArray<TObjectPtr<AT66MiniInteractable>> LiveInteractables;

	UPROPERTY()
	TArray<TObjectPtr<AT66MiniPickup>> LivePickups;

	UPROPERTY()
	TObjectPtr<UAudioComponent> BattleMusicComponent;

	TArray<FT66MiniCombatTextEntry> CombatTexts;
	TSet<TWeakObjectPtr<class AT66MiniPlayerPawn>> PositionedPlayerPawns;
	FTimerHandle OnlineFrontendTravelTimer;
};
