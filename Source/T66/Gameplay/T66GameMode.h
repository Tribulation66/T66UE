// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T66GameMode.generated.h"

class AT66HeroBase;
class AT66CompanionBase;
class AT66StartGate;
class AT66StageGate;
class AT66BossBase;
class AT66MiasmaManager;
class AT66LoanShark;
class AT66CowardiceGate;
class AT66TricksterNPC;
class AT66BossGate;
class AT66IdolAltar;
class AT66StageBoostGate;
class AT66StageBoostGoldInteractable;
class AT66StageBoostLootInteractable;
class AT66StageEffectTile;
class AT66TutorialManager;
class UT66GameInstance;
class ADirectionalLight;
class ASkyLight;
class APlayerStart;

/**
 * Game Mode for Tribulation 66 gameplay levels
 *
 * Responsibilities:
 * - Spawn the correct hero based on Game Instance selection
 * - Initialize hero with data from DataTable
 * - Handle basic game flow
 * - Auto-setup level with floor/lighting if missing (for development)
 */
UCLASS(Blueprintable)
class T66_API AT66GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AT66GameMode();

	/** Default hero class to spawn (can be overridden per-hero in DataTable) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<AT66HeroBase> DefaultHeroClass;

	/** Whether to auto-create floor/lighting if missing (useful for development) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level Setup")
	bool bAutoSetupLevel = true;

	/** Default spawn height when no PlayerStart exists */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level Setup")
	float DefaultSpawnHeight = 200.f;

	/** Offset from world origin for Stage Gate (far side of map). Start Gate is spawned near player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gates")
	FVector StageGateSpawnOffset = FVector(10000.f, 0.f, 200.f);

	/** Cowardice gate / Trickster spawn is placed before the boss area. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gates")
	FVector CowardiceGateSpawnOffset = FVector(5200.f, 0.f, 200.f);

	/**
	 * Spawn the hero based on current Game Instance selections
	 * Called automatically or can be called manually for testing
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	AT66HeroBase* SpawnSelectedHero(AController* Controller);

	/** Spawn Stage Gate (interact F to next stage) at a specific location (typically boss death). */
	void SpawnStageGateAtLocation(const FVector& Location);

	/** Clear all miasma tiles (e.g. when boss is defeated). */
	void ClearMiasma();

	/** Called by BossBase on death so GameMode can decide what happens (normal vs Coliseum). */
	void HandleBossDefeatedAtLocation(const FVector& Location);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/** Get the Game Instance cast to our type */
	UT66GameInstance* GetT66GameInstance() const;

	/** Set up basic level elements if missing (floor, lighting, player start) */
	void EnsureLevelSetup();

	/** Spawn a floor plane if none exists */
	void SpawnFloorIfNeeded();

	/** Spawn boundary walls so the player can't fall off. */
	void SpawnBoundaryWallsIfNeeded();
	void SpawnStartAreaExitWallsIfNeeded();
	void SpawnPlatformEdgeWallsIfNeeded();

	/** Spawn lighting if none exists */
	void SpawnLightingIfNeeded();

	/** Spawn a PlayerStart if none exists */
	void SpawnPlayerStartIfNeeded();

	/** Spawn the selected companion (if any) and attach follow behavior */
	void SpawnCompanionForPlayer(AController* Player);

	/** Spawn vendor NPC near the hero (big cylinder) */
	void SpawnVendorForPlayer(AController* Player);

	/** Spawn Start Gate (walk-through, starts timer) near the hero spawn. */
	void SpawnStartGateForPlayer(AController* Player);
	void SpawnIdolAltarForPlayer(AController* Player);
	void SpawnIdolAltarAtLocation(const FVector& Location);

	/** Spawn Boss Gate (walk-through, awakens boss) between main and boss areas. */
	void SpawnBossGateIfNeeded();
	void SpawnWorldInteractablesForStage();
	void SpawnStageBoostPlatformAndInteractables();
	void SpawnStageEffectTilesForStage();
	void SpawnColiseumArenaIfNeeded();
	void SpawnAllOwedBossesInColiseum();
	void SpawnTutorialIfNeeded();

	/** Spawn boss for current stage (dormant until player approaches). */
	void SpawnBossForCurrentStage();

	/** Spawn the 4 corner houses + 4 NPC cylinders (vendor/gambler/ouroboros/saint). */
	void SpawnCornerHousesAndNPCs();

	void SpawnTricksterAndCowardiceGate();

	bool IsColiseumStage() const;
	void ResetColiseumState();

	/** Called when stage timer changes (we use it to detect "timer started"). */
	UFUNCTION()
	void HandleStageTimerChanged();

	UFUNCTION()
	void HandleDifficultyChanged();

	void TrySpawnLoanSharkIfNeeded();

private:
	/** Track spawned setup actors for cleanup */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedSetupActors;

	UPROPERTY()
	TObjectPtr<AT66MiasmaManager> MiasmaManager;

	UPROPERTY()
	TObjectPtr<AT66LoanShark> LoanShark;

	UPROPERTY()
	TObjectPtr<AT66CowardiceGate> CowardiceGate;

	UPROPERTY()
	TObjectPtr<AT66TricksterNPC> TricksterNPC;

	UPROPERTY()
	TObjectPtr<AT66StartGate> StartGate;

	UPROPERTY()
	TObjectPtr<AT66BossGate> BossGate;

	UPROPERTY()
	TObjectPtr<AT66IdolAltar> IdolAltar;

	// Coliseum state (only used when bForceColiseumMode is true).
	int32 ColiseumBossesRemaining = 0;
	bool bColiseumExitGateSpawned = false;
	FVector ColiseumCenter = FVector(0.f, -5200.f, 200.f);
};
