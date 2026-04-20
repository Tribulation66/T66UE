// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Gameplay/T66TowerMapTerrain.h"
#include "T66GameMode.generated.h"

class AActor;
class AT66HeroBase;
class AT66CompanionBase;
class AT66EnemyBase;
class AT66EnemyDirector;
class AT66StartGate;
class AT66StageGate;
class AT66BossBase;
class AT66MiasmaManager;
class AT66LoanShark;
class AT66CowardiceGate;
class AT66TricksterNPC;
class AT66BossGate;
class AT66IdolAltar;
class AT66TowerDescentHole;
class AT66StageCatchUpGate;
class AT66StageCatchUpGoldInteractable;
class AT66StageCatchUpLootInteractable;
class AT66Shroom;
class AT66SpawnPlateau;
class AT66TutorialManager;
class AT66SaintNPC;
class APawn;
class UT66GameInstance;
class AT66RecruitableCompanion;
class UT66LoadingScreenWidget;
class APlayerStart;
class UMaterialInterface;
struct FStreamableHandle;

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

	/** Hill-side material variants used for terrain walls and ramp side faces. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level Setup")
	TArray<TSoftObjectPtr<UMaterialInterface>> CliffSideMaterials;

	/** Default spawn height when no PlayerStart exists */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level Setup")
	float DefaultSpawnHeight = 200.f;

	/**
	 * Dev switch: force the hero to spawn in the Tutorial Arena instead of the normal Stage 1 Start Area.
	 * Default OFF (spawn in normal start area).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tutorial")
	bool bForceSpawnInTutorialArea = false;

	/** Offset from world origin for Stage Gate (far side of map). Start Gate is spawned near player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gates")
	FVector StageGateSpawnOffset = FVector(10000.f, 0.f, 200.f);

	/** Cowardice gate / Trickster spawn is placed before the boss area. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gates")
	FVector CowardiceGateSpawnOffset = FVector(5200.f, 0.f, 200.f);

	/** Development/test helper: spawn a cow, pig, and goat near the stage-start altar as stationary idol test dummies. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Idols")
	bool bSpawnIdolVFXTestTargetsAtStageStart = true;

	/**
	 * Spawn the hero based on current Game Instance selections
	 * Called automatically or can be called manually for testing
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	AT66HeroBase* SpawnSelectedHero(AController* Controller);

	/** Spawn Stage Gate (interact F to next stage) at a specific location (typically boss death). */
	void SpawnStageGateAtLocation(const FVector& Location);

	/** Swap active companion during a run (spawns old companion as a recruitable at its current location). */
	bool SwapCompanionForPlayer(AController* Player, FName NewCompanionID);

	/** Clear all miasma tiles (e.g. when boss is defeated). */
	void ClearMiasma();

	/** Called by BossBase on death so GameMode can decide what happens (normal vs Coliseum) and award score. */
	void HandleBossDefeated(AT66BossBase* Boss);

	/** Destroy existing main map terrain geometry and spawn a fresh difficulty-driven terrain run. */
	void RegenerateMainMapTerrain(int32 Seed);

	bool IsUsingTowerMainMapLayout() const;
	bool GetTowerMainMapLayout(T66TowerMapTerrain::FLayout& OutLayout) const;
	int32 GetTowerFloorIndexForLocation(const FVector& Location) const;
	int32 GetCurrentTowerFloorIndex() const;
	bool TryGetTowerEnemySpawnLocation(const FVector& PlayerLocation, float MinDistance, float MaxDistance, FRandomStream& Rng, FVector& OutLocation) const;
	bool TryGetTowerEnemySpawnLocation(const FVector& PlayerLocation, float MinDistance, float MaxDistance, FRandomStream& Rng, FVector& OutLocation, FVector& OutWallNormal) const;
	void HandleTowerDescentHoleTriggered(APawn* Pawn, int32 FromFloorNumber, int32 ToFloorNumber);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/** Get the Game Instance cast to our type */
	UT66GameInstance* GetT66GameInstance() const;

	/** Set up basic level elements if missing (floor, lighting, player start) */
	void EnsureLevelSetup();

	/** Apply configured ground material to all tagged runtime floors (async-load safe). */
	void TryApplyGroundFloorMaterialToAllFloors();

	/** Spawn a floor plane if none exists */
	void SpawnFloorIfNeeded();

	/** Spawn a very short red wall fully around the start area (inside main map). */
	void SpawnStartAreaWallsIfNeeded();
	/** Spawn a very short red wall fully around the boss area (same style as start). */
	void SpawnBossAreaWallsIfNeeded();

	/** Spawn a PlayerStart if none exists */
	void SpawnPlayerStartIfNeeded();

	UFUNCTION()
	void HandleSettingsChanged();

	/** Lab only: one central floor ~1/4 gameplay map size so the hero doesn't fall. */
	void SpawnLabFloorIfNeeded();

	/** Lab only: spawn The Collector NPC if not present. */
	void SpawnLabCollectorIfNeeded();

	/** Spawn the selected companion (if any) and attach follow behavior */
	void SpawnCompanionForPlayer(AController* Player);

	/** Spawn vendor NPC near the hero (big cylinder) */
	void SpawnVendorForPlayer(AController* Player);

	/** Spawn Start Gate (walk-through, starts timer) near the hero spawn. */
	void SpawnStartGateForPlayer(AController* Player);
	/** Spawn the stage-entry idol altar near the start area. */
	void SpawnIdolAltarForPlayer(AController* Player);
	void SpawnIdolAltarAtLocation(const FVector& Location);
	void SpawnIdolVFXTestTargets();

	/** Spawn Boss Gate (walk-through, awakens boss) between main and boss areas. */
	void SpawnBossGateIfNeeded();
	void SpawnWorldInteractablesForStage();
	void SpawnGuaranteedStartAreaInteractables();
	void SpawnModelShowcaseRow();
	void SpawnStageCatchUpPlatformAndInteractables();
	void SpawnStageEffectsForStage();
	void SpawnColiseumArenaIfNeeded();
	void SpawnTutorialArenaIfNeeded();
	void SpawnAllOwedBossesInColiseum();
	void SpawnFinalDifficultyBossInColiseum();
	void BeginFinalDifficultySurvival(const FVector& BossDeathLocation);
	void TickFinalDifficultySurvival(float DeltaTime);
	void UpdateFinalDifficultySurvivalScaling(bool bForce = false);
	void SpawnFinalDifficultyTotem(const FVector& SpawnLocation);
	void SpawnFinalDifficultySaint(const FVector& SpawnLocation);
	void SpawnTutorialIfNeeded();

	/** Spawn boss for current stage (dormant until player approaches). */
	void SpawnBossForCurrentStage();
	void SpawnBossBeaconIfNeeded();

	void SpawnCircusInteractableIfNeeded();
	void SpawnSupportVendorAtStartIfNeeded();

	/** Called one frame after BeginPlay so the landscape/collision is ready. Spawns all ground-dependent content (NPCs, interactables, tiles, boss, etc.). */
	void SpawnLevelContentAfterLandscapeReady();

	/** Spawn the runtime main map terrain for the active gameplay difficulty. */
	void SpawnMainMapTerrain();

	/** Spawn a flat plateau so its top surface is at TopCenterLoc (used under NPCs and world interactables). */
	void SpawnPlateauAtLocation(UWorld* World, const FVector& TopCenterLoc);

	void SpawnTricksterAndCowardiceGate();
	void SpawnTowerDescentHolesIfNeeded();
	void InitializeRunStateForBeginPlay();
	bool HandleSpecialModeBeginPlay();
	void HandleColiseumBeginPlay();
	void HandleLabBeginPlay();
	void ConsumePendingStageCatchUp();
	void ScheduleDeferredGameplayLevelSpawn();
	TWeakObjectPtr<UT66LoadingScreenWidget> CreateGameplayWarmupOverlay(UWorld* World, bool bUsingMainMapTerrain) const;
	void ScheduleGameplayVisualCleanup(UWorld* World);
	void ScheduleGameplayWarmupOverlayHide(UWorld* World, TWeakObjectPtr<UT66LoadingScreenWidget> GameplayWarmupOverlay);
	void SpawnStageStructuresAndInteractables(UWorld* World, bool bUsingMainMapTerrain);
	void SpawnStageDecorativeProps(bool bUsingMainMapTerrain);
	void PrepareMainMapStage(UWorld* World);
	void ScheduleStandardStageCombatBootstrap(UWorld* World);
	void PreloadStageCharacterVisuals();
	void SpawnStageMiasmaSystems();
	void SpawnStageEnemyDirector();
	void FinalizeStandardStageCombatBootstrap();

	bool IsColiseumStage() const;
	bool IsFinalDifficultyBossColiseumStage() const;
	/** True when current level is The Lab (practice room). */
	bool IsLabLevel() const;
	void ResetColiseumState();

	/** Called when stage timer changes (we use it to detect "timer started"). */
	UFUNCTION()
	void HandleStageTimerChanged();

	UFUNCTION()
	void HandleIdolStateChanged();

	UFUNCTION()
	void HandleStageChanged();

	UFUNCTION()
	void HandleDifficultyChanged();

	void RefreshProgressionDrivenSystems(bool bRescaleLiveEnemies);
	void ApplyStageProgressionVisuals();
	AT66EnemyDirector* FindOrCacheEnemyDirector(UWorld* World);
	AT66EnemyDirector* EnsureEnemyDirector(UWorld* World);
	void DestroyEnemyDirectors(UWorld* World);
	void ResetTowerMiasmaState();
	void UpdateTowerMiasma(float DeltaTime);
	void TryStartTowerMiasma(const FVector* SourceAnchor = nullptr, int32 SourceFloorNumber = INDEX_NONE);
	void SyncTowerMiasmaSourceAnchor(int32 FloorNumber, const FVector& WorldAnchor) const;
	float GetTowerMiasmaElapsedSeconds() const;

	void TrySpawnLoanSharkIfNeeded();

public:
	// ============================================
	// The Lab: spawn / reset (only used when IsLabLevel())
	// ============================================

	/** Spawn one mob in the Lab (Cow, Pig, Goat, Roost, GoblinThief, UniqueEnemy). Returns spawned actor or null. */
	UFUNCTION(BlueprintCallable, Category = "Lab")
	AActor* SpawnLabMob(FName CharacterVisualID);

	/** Spawn one boss in the Lab from Bosses DataTable. Returns spawned actor or null. */
	UFUNCTION(BlueprintCallable, Category = "Lab")
	AActor* SpawnLabBoss(FName BossID);

	/** Spawn Fountain of Life in the Lab (NPC tab). Returns spawned actor or null. */
	UFUNCTION(BlueprintCallable, Category = "Lab")
	AActor* SpawnLabFountainOfLife();

	/** Spawn an interactable in the Lab (Fountain, Chest, WheelSpin, IdolAltar, Crate). Returns spawned actor or null. */
	UFUNCTION(BlueprintCallable, Category = "Lab")
	AActor* SpawnLabInteractable(FName InteractableID);

	/** Random position on the Lab floor with min distance from existing Lab-spawned actors. */
	FVector GetRandomLabSpawnLocation() const;

	/** Destroy all actors spawned via Lab panels and clear the list. */
	UFUNCTION(BlueprintCallable, Category = "Lab")
	void ResetLabSpawnedActors();

	/** Number of actors currently tracked as Lab-spawned (for UI). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lab")
	int32 GetLabSpawnedActorsCount() const { return LabSpawnedActors.Num(); }

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

	UPROPERTY()
	TArray<TObjectPtr<AT66EnemyBase>> IdolVFXTestTargets;

	UPROPERTY()
	TArray<TObjectPtr<AT66TowerDescentHole>> TowerDescentHoles;

	// Async load tracking (prevents gameplay hitching from sync loads).
	TArray<TSharedPtr<FStreamableHandle>> ActiveAsyncLoadHandles;

	// Track currently spawned companions per player so respawns don't duplicate them.
	TMap<TWeakObjectPtr<AController>, TWeakObjectPtr<AT66CompanionBase>> PlayerCompanions;

	// Track the current stage boss so we can safely replace it after async load.
	TWeakObjectPtr<AT66BossBase> StageBoss;
	TWeakObjectPtr<AT66EnemyDirector> EnemyDirector;
	TWeakObjectPtr<AT66TutorialManager> TutorialManager;
	TWeakObjectPtr<AActor> BossBeaconActor;
	float BossBeaconUpdateAccumulator = 0.f;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> CachedCubeMesh;

	UStaticMesh* GetCubeMesh();
	bool TrySnapActorToTerrain(AActor* Actor) const;
	bool TrySnapActorToTerrainAtLocation(AActor* Actor, const FVector& TraceLocation) const;
	void RestartPlayersMissingPawns();
	bool TryComputeBossBeaconBase(FVector& OutBeaconBase) const;
	void UpdateBossBeaconTransform(bool bForceSpawnIfMissing);
	void DestroyBossBeacon();
	void SyncTowerBossEntryState();
	void SyncTowerTrapActivation(bool bForce = false);
	void SnapPlayersToTerrain();
	void MaintainPlayerTerrainSafety();
	void TryActivateMainMapCombat();
	bool TryGetMainMapStartAxes(FVector& OutCenter, FVector& OutInwardDirection, FVector& OutSideDirection, float& OutCellSize) const;
	bool TryGetMainMapStartPlacementLocation(float SideCells, float InwardCells, FVector& OutLocation) const;
	bool TryFindRandomMainMapSurfaceLocation(int32 SeedOffset, FVector& OutLocation, float ExtraSafeBubbleMargin = 0.f) const;

	bool bTerrainCollisionReady = false;
	bool bMainMapCombatStarted = false;
	bool bWorldInteractablesSpawnedForStage = false;
	bool bHasMainMapSpawnSurfaceLocation = false;
	FVector MainMapSpawnSurfaceLocation = FVector::ZeroVector;
	FVector MainMapStartAnchorSurfaceLocation = FVector::ZeroVector;
	FVector MainMapStartPathSurfaceLocation = FVector::ZeroVector;
	FVector MainMapStartAreaCenterSurfaceLocation = FVector::ZeroVector;
	FVector MainMapBossAnchorSurfaceLocation = FVector::ZeroVector;
	FVector MainMapBossSpawnSurfaceLocation = FVector::ZeroVector;
	FVector MainMapBossBeaconSurfaceLocation = FVector::ZeroVector;
	FVector MainMapBossAreaCenterSurfaceLocation = FVector::ZeroVector;
	TArray<FVector> MainMapRescueAnchorLocations;
	bool bUsingTowerMainMapLayout = false;
	T66TowerMapTerrain::FLayout CachedTowerMainMapLayout;
	bool bTowerBossEntryTriggered = false;
	bool bTowerBossEntryApplied = false;
	float TowerTerrainSafetyAccumulator = 0.f;
	float TowerTrapActivationAccumulator = 0.f;
	bool bTowerMiasmaActive = false;
	float TowerMiasmaStartWorldSeconds = 0.f;
	float TowerMiasmaUpdateAccumulator = 0.f;
	int32 TowerIdolSelectionsAtStageStart = 0;
	int32 ActiveTowerTrapFloorNumber = INDEX_NONE;

	// Coliseum: async-load boss classes before spawning.
	bool bColiseumBossesAsyncLoadInFlight = false;
	bool bColiseumBossesAsyncLoadAttempted = false;

	// Coliseum state (only used when bForceColiseumMode is true).
	int32 ColiseumBossesRemaining = 0;
	bool bColiseumExitGateSpawned = false;
	FVector ColiseumCenter = FVector(-45455.f, -23636.f, 200.f);
	bool bFinalDifficultySurvivalActive = false;
	float FinalDifficultySurvivalElapsedSeconds = 0.f;
	float LastAppliedFinalDifficultyEnemyScalar = 1.f;
	TWeakObjectPtr<AActor> FinalDifficultyTotemActor;
	TWeakObjectPtr<AT66SaintNPC> FinalDifficultySaintActor;

	// The Lab: actors spawned from Lab panels (for Reset Enemies).
	UPROPERTY()
	TArray<TObjectPtr<AActor>> LabSpawnedActors;

	FVector GetLabSpawnLocation() const;
};
