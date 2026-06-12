// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "GameFramework/GameModeBase.h"
#include "Gameplay/T66TowerMapTerrain.h"
#include "T66GameMode.generated.h"

class AActor;
class AT66HeroBase;
class AT66CompanionBase;
class AT66PetActor;
class AT66EnemyBase;
class AT66EnemyDirector;
class AT66StageGate;
class AT66BossBase;
class AT66MiasmaManager;
class AT66LoanShark;
class AT66CowardiceGate;
class AT66IdolAltar;
class AT66WeaponAltar;
class AT66TowerDescentHole;
class AT66BackroomsDoorInteractable;
class AT66BackroomsChaser;
class AT66Shroom;
class AT66SpawnPlateau;
class AT66TutorialManager;
class AT66SaintNPC;
class AT66PlayerController;
class AStaticMeshActor;
class APawn;
class UT66GameInstance;
class AT66RecruitableCompanion;
class UT66LoadingScreenWidget;
class APlayerStart;
class UMaterialInterface;
class UStaticMesh;
class UTexture2D;
class UActorComponent;
struct FStreamableHandle;

/**
 * Game Mode for Tribulation 66 gameplay maps
 *
 * Responsibilities:
 * - Spawn the correct hero based on Game Instance selection
 * - Initialize hero with data from DataTable
 * - Handle basic game flow
 * - Auto-setup map with floor/lighting if missing (for development)
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

	/** Offset from world origin for the post-boss Stage Gate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gates")
	FVector StageGateSpawnOffset = FVector(10000.f, 0.f, 200.f);

	/** Cowardice gate spawn is placed before the boss area. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gates")
	FVector CowardiceGateSpawnOffset = FVector(5200.f, 0.f, 200.f);

	/** Development/test helper: show Pixal3D experiment meshes beside explicit lab/gallery Idol Altars. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development|Pixal3D")
	bool bSpawnPixalTestModelsAtIdolAltar = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development|Pixal3D")
	TSoftObjectPtr<UStaticMesh> PixalTestMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development|Pixal3D")
	TSoftObjectPtr<UTexture2D> PixalTestTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development|Pixal3D")
	TSoftObjectPtr<UStaticMesh> PixalTest2Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development|Pixal3D")
	TSoftObjectPtr<UTexture2D> PixalTest2Texture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development|Pixal3D")
	TSoftObjectPtr<UStaticMesh> PixalSlimeMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development|Pixal3D")
	TSoftObjectPtr<UTexture2D> PixalSlimeTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development|Pixal3D")
	TSoftObjectPtr<UStaticMesh> PixalSlimeHifiMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development|Pixal3D")
	TSoftObjectPtr<UTexture2D> PixalSlimeHifiTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development|Pixal3D")
	TArray<TSoftObjectPtr<UStaticMesh>> PixalStandaloneTestMeshes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development|Pixal3D")
	TArray<TSoftObjectPtr<UTexture2D>> PixalStandaloneTestTextures;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development|Pixal3D")
	TArray<TSoftObjectPtr<UStaticMesh>> PixalEasyDungeonHifiMeshes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development|Pixal3D")
	TArray<TSoftObjectPtr<UTexture2D>> PixalEasyDungeonHifiTextures;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development|Pixal3D")
	TSoftObjectPtr<UMaterialInterface> PixalTestSharedMaterial;

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

	/** Called by BossBase on death so GameMode can advance stage flow and award score. */
	void HandleBossDefeated(AT66BossBase* Boss);
	void HandleSaintEndgameChoice(AT66PlayerController* PlayerController, int32 ChoiceIndex, AT66SaintNPC* Saint);
	bool ShouldEndgameDeathOpenRunSummary() const;
	void HandleEndgameDeathRunSummary(AT66PlayerController* PlayerController);

	/** Destroy existing main map terrain geometry and spawn a fresh difficulty-driven terrain run. */
	void RegenerateMainMapTerrain(int32 Seed);

	bool IsUsingTowerMainMapLayout() const;
	bool GetTowerMainMapLayout(T66TowerMapTerrain::FLayout& OutLayout) const;
	bool GetTowerFloorLayout(int32 FloorNumber, T66TowerMapTerrain::FFloor& OutFloor) const;
	int32 GetTowerFloorIndexForLocation(const FVector& Location) const;
	int32 GetCurrentTowerFloorIndex() const;
	/** Explicit hero floor transition (spawn / descent hole / rescue). Altitude never changes the floor. */
	void SetHeroTowerFloorNumber(int32 FloorNumber, const TCHAR* Reason);
	int32 ResolveTowerFloorNumberForActor(const AActor* Actor) const;
	bool ShouldApplyTowerFloorDamage(const AActor* SourceActor, const FVector& DamageOrigin, const AActor* TargetActor) const;
	bool TryGetTowerEnemySpawnLocation(const FVector& PlayerLocation, float MinDistance, float MaxDistance, FRandomStream& Rng, FVector& OutLocation) const;
	bool TryGetTowerEnemySpawnLocation(const FVector& PlayerLocation, float MinDistance, float MaxDistance, FRandomStream& Rng, FVector& OutLocation, FVector& OutWallNormal) const;
	void HandleTowerDescentGateOpened(int32 FromFloorNumber, int32 ToFloorNumber);
	void HandleTowerDescentHoleTriggered(APawn* Pawn, int32 FromFloorNumber, int32 ToFloorNumber);
	int32 DespawnTowerEnemiesAboveFloor(int32 CurrentFloorNumber);
	void HandleTowerGateGuardianDefeated(AT66EnemyBase* Guardian);
	void NotifyTowerIdolSelectionForGate(int32 FromFloorNumber);
	void NotifyTowerWeaponSelectionForGate(int32 FromFloorNumber);
	void SetEnemyDirectorSpawningPaused(bool bPaused);
	AT66EnemyDirector* GetEnemyDirectorForDiagnostics();
#if !UE_BUILD_SHIPPING
	void RunBossProjectileManagerSmokeSpawnBossForCurrentStage() { SpawnBossForCurrentStage(); }
	void RunBossProjectileManagerSmokeEnsureBossPathReady() { SpawnCowardiceGateIfNeeded(); }
	bool RunEndgameSaintSmoke(UWorld* ProofWorld, const FString& OutputPath);
#endif
	bool IsBackroomsChallengeActive() const { return bBackroomsChallengeActive; }
	void HandleBackroomsDoorInteracted(AT66BackroomsDoorInteractable* Door, AT66HeroBase* Hero);
	void HandleBackroomsChaserTouchedHero(AT66BackroomsChaser* Chaser, AT66HeroBase* Hero);
	bool GetBackroomsChaserMoveTarget(const FVector& ChaserLocation, const FVector& HeroLocation, FVector& OutTarget) const;

protected:
	virtual void BeginPlay() override;
	virtual void StartPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/** Get the Game Instance cast to our type */
	UT66GameInstance* GetT66GameInstance() const;

	/** Set up basic map elements if missing (floor, lighting, player start) */
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
	/** Spawn the selected active pet (if any). Mob Loot collection remains disabled until Foundation API lands. */
	void SpawnPetForPlayer(AController* Player);
	/** Spawn guaranteed capture interactable for an uncaptured non-final stage boss. */
	bool TrySpawnPetCaptureForBoss(AT66BossBase* Boss, const FVector& Location);
	int32 SpawnCagedCompanionsForCurrentStage(const FVector& AnchorLocation);
	int32 FreeCagedCompanionsForBossClear(const FVector& FallbackLocation);
	void ClearCagedStageCompanions(bool bDestroyActors);

	/** Spawn the stage-entry idol altar near the start area. */
	void SpawnIdolAltarForPlayer(AController* Player);
	void SpawnWeaponAltarForPlayer(AController* Player);
	AT66IdolAltar* SpawnIdolAltarAtLocation(const FVector& Location, bool bAllowMultiple = false);
	void SpawnPixalTestDisplayModelsNearIdolAltar(AT66IdolAltar* AnchorAltar, bool bTrackAsLabSpawned = false);

	/** Spawn the optional Cowardice Gate skip choice before the boss area. */
	void SpawnCowardiceGateIfNeeded();
	void SpawnWorldInteractablesForStage();
	void SpawnStartGalleryShowcase();
	void SpawnStageEffectsForStage();
	void SpawnTutorialArenaIfNeeded();
	void BeginFinalDifficultySurvival(const FVector& BossDeathLocation);
	void StopFinalDifficultySurvival();
	void TickFinalDifficultySurvival(float DeltaTime);
	void UpdateFinalDifficultySurvivalScaling(bool bForce = false);
	void SpawnFinalDifficultyTotem(const FVector& SpawnLocation);
	void SpawnFinalDifficultySaint(const FVector& SpawnLocation);
	void CompleteDifficultyAndOpenRunSummary();
	void SpawnKromerLootBag(const FVector& Location);
	AT66BossBase* SpawnEndgameBossAt(FName BossID, const FVector& Location, bool bForceAwaken, bool bZeroDamageUnkillable, FName ZeroDamageReason, float HealthScalar = 1.f, float DamageScalar = 1.f, float ScaleScalar = 1.f, bool bTrackAsStageBoss = false);
	void SpawnFinalBossSecondPhase(const FVector& Location);
	void SpawnFinalCompanionTransformBosses(const FVector& Location);
	FName ResolveBossIDForActivePet() const;
	void SpawnTutorialIfNeeded();

	/** Spawn boss for current stage (dormant until player approaches). */
	void SpawnBossForCurrentStage();
	FVector ResolveTowerBossWaitingLocation() const;
	bool EnsureTowerBossEntryBossReady();

	void SpawnCasinoNPCIfNeeded();

	/** Called one frame after BeginPlay so the landscape/collision is ready. Spawns all ground-dependent content (NPCs, interactables, tiles, boss, etc.). */
	void SpawnLevelContentAfterLandscapeReady();

	/** Spawn the runtime main map terrain for the active gameplay difficulty. */
	void SpawnMainMapTerrain();

	/** Spawn a flat plateau so its top surface is at TopCenterLoc (used under NPCs and world interactables). */
	void SpawnPlateauAtLocation(UWorld* World, const FVector& TopCenterLoc);

	void SpawnTowerDescentHolesIfNeeded();
	bool IsPlacedTowerMinibossFloor(int32 FloorNumber) const;
	AT66TowerDescentHole* FindTowerDescentHoleForFloor(int32 FloorNumber) const;
	AT66EnemyBase* EnsurePlacedTowerMinibossForFloor(int32 FloorNumber);
	void SpawnBackroomsPocketIfNeeded();
	void DestroyBackroomsPocket();
	void EnsureGameplayStartupInitialized(const TCHAR* TriggerContext);
	void InitializeRunStateForBeginPlay();
	bool HandleSpecialModeBeginPlay();
	void HandleLabBeginPlay();
	void ScheduleDeferredGameplayLevelSpawn();
	TWeakObjectPtr<UT66LoadingScreenWidget> CreateGameplayWarmupOverlay(UWorld* World, bool bUsingMainMapTerrain) const;
	void ScheduleGameplayVisualCleanup(UWorld* World);
	void ScheduleGameplayWarmupOverlayHide(UWorld* World, TWeakObjectPtr<UT66LoadingScreenWidget> GameplayWarmupOverlay);
	void SpawnStageStructuresAndInteractables(UWorld* World, bool bUsingMainMapTerrain);
	void PrepareMainMapStage(UWorld* World);
	void ScheduleStandardStageCombatBootstrap(UWorld* World);
	void PreloadStageCharacterVisuals();
	void SpawnStageMiasmaSystems();
	void SpawnStageEnemyDirector();
	void FinalizeStandardStageCombatBootstrap();

	/** True when current run category is The Lab (practice room). */
	bool IsLabRun() const;

	/** Called when stage timer changes (we use it to detect "timer started"). */
	UFUNCTION()
	void HandleStageTimerChanged();

	UFUNCTION()
	void HandleIdolStateChanged();

	UFUNCTION()
	void HandleStageChanged();

	UFUNCTION()
	void HandleDifficultyChanged();
	bool IsBossRushFinaleStage() const;

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
	void EnterBackrooms(AT66HeroBase* Hero, AT66BackroomsDoorInteractable* EntryDoor);
	void CompleteBackrooms(bool bSucceeded, AT66HeroBase* Hero, AT66BackroomsChaser* Chaser = nullptr);
	void ApplyBackroomsPauseState(bool bPaused);
	void RestoreBackroomsInventoryAndWeapon();
	bool HasBackroomsQuickReviveReward() const;
	bool TryBuildBackroomsMaze();
	bool IsBackroomsMazeOpen(int32 X, int32 Y) const;
	FVector GetBackroomsCellCenter(int32 X, int32 Y, float ZOffset = 90.f) const;
	FIntPoint WorldToBackroomsCell(const FVector& Location) const;
#if !UE_BUILD_SHIPPING
	bool RunContentCorrectionsSmoke(UWorld* ProofWorld);
	void ScheduleBackroomsAutomationIfRequested();
	void RunBackroomsAutomationStart(FString Mode);
	void RunBackroomsAutomationFinish(FString Mode, int32 InventoryCountBeforeEntry, FName SeedItemID, FName WeaponIDBeforeEntry, float ChaserDistanceAtEntry);
#endif

public:
	// ============================================
	// The Lab: spawn / reset (only used when IsLabRun())
	// ============================================

	/** Spawn one mob in the Lab from the current enemy roster. Returns spawned actor or null. */
	UFUNCTION(BlueprintCallable, Category = "Lab")
	AActor* SpawnLabMob(FName CharacterVisualID);

	/** Spawn one boss in the Lab from Bosses DataTable. Returns spawned actor or null. */
	UFUNCTION(BlueprintCallable, Category = "Lab")
	AActor* SpawnLabBoss(FName BossID);

	/** Spawn Fountain in the Lab (NPC tab). Returns spawned actor or null. */
	UFUNCTION(BlueprintCallable, Category = "Lab")
	AActor* SpawnLabFountain();

	/** Spawn an interactable in the Lab (Fountain, Chest, IdolAltar, Crate). Returns spawned actor or null. */
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
	TObjectPtr<AT66IdolAltar> IdolAltar;

	UPROPERTY()
	TObjectPtr<AT66WeaponAltar> WeaponAltar;

	UPROPERTY()
	TArray<TObjectPtr<AStaticMeshActor>> PixalTestDisplayActors;

	UPROPERTY()
	TArray<TObjectPtr<AT66TowerDescentHole>> TowerDescentHoles;

	UPROPERTY()
	TSet<int32> TowerPlacedMinibossSpawnedFloors;

	UPROPERTY()
	TSet<int32> TowerPlacedMinibossDefeatedFloors;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> BackroomsActors;

	UPROPERTY()
	TObjectPtr<AT66BackroomsDoorInteractable> BackroomsEntryDoor;

	UPROPERTY()
	TObjectPtr<AT66BackroomsDoorInteractable> BackroomsExitDoor;

	UPROPERTY()
	TObjectPtr<AT66BackroomsDoorInteractable> BackroomsClosedEntranceDoor;

	UPROPERTY()
	TObjectPtr<AT66BackroomsChaser> BackroomsChaser;

	// Async load tracking (prevents gameplay hitching from sync loads).
	TArray<TSharedPtr<FStreamableHandle>> ActiveAsyncLoadHandles;

	// Track currently spawned companions per player so respawns don't duplicate them.
	TMap<TWeakObjectPtr<AController>, TWeakObjectPtr<AT66CompanionBase>> PlayerCompanions;

	// Track currently spawned pets per player so respawns don't duplicate them.
	TMap<TWeakObjectPtr<AController>, TWeakObjectPtr<AT66PetActor>> PlayerPets;

	// Track the current stage boss so we can safely replace it after async load.
	TWeakObjectPtr<AT66BossBase> StageBoss;
	TArray<TWeakObjectPtr<AT66RecruitableCompanion>> CagedStageCompanions;
	TWeakObjectPtr<AT66EnemyDirector> EnemyDirector;
	TWeakObjectPtr<AT66TutorialManager> TutorialManager;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> CachedCubeMesh;

	UStaticMesh* GetCubeMesh();
	bool TrySnapActorToTerrain(AActor* Actor) const;
	bool TrySnapActorToTerrainAtLocation(AActor* Actor, const FVector& TraceLocation) const;
	void RestartPlayersMissingPawns();
	void SyncTowerBossEntryState();
	void SyncTowerTrapActivation(bool bForce = false);
	void SnapPlayersToTerrain();
	void MaintainPlayerTerrainSafety();
	void TryActivateMainMapCombat();
	bool TryGetMainMapStartAxes(FVector& OutCenter, FVector& OutInwardDirection, FVector& OutSideDirection, float& OutCellSize) const;
	bool TryGetMainMapStartPlacementLocation(float SideCells, float InwardCells, FVector& OutLocation) const;
	bool TryFindRandomMainMapSurfaceLocation(int32 SeedOffset, FVector& OutLocation, float ExtraSafeBubbleMargin = 0.f) const;

	bool bTerrainCollisionReady = false;
	bool bGameplayStartupInitialized = false;
	bool bGameplayLevelSpawnScheduled = false;
	bool bGameplayLevelSpawnCompleted = false;
	FName PendingRunStartItemId = NAME_None;
	bool bMainMapCombatStarted = false;
	bool bWorldInteractablesSpawnedForStage = false;
	bool bStartGalleryShowcaseSpawned = false;
	bool bHasMainMapSpawnSurfaceLocation = false;
	FVector MainMapSpawnSurfaceLocation = FVector::ZeroVector;
	FVector MainMapStartAnchorSurfaceLocation = FVector::ZeroVector;
	FVector MainMapStartPathSurfaceLocation = FVector::ZeroVector;
	FVector MainMapStartAreaCenterSurfaceLocation = FVector::ZeroVector;
	FVector MainMapBossAnchorSurfaceLocation = FVector::ZeroVector;
	FVector MainMapBossSpawnSurfaceLocation = FVector::ZeroVector;
	FVector MainMapBossAreaCenterSurfaceLocation = FVector::ZeroVector;
	TArray<FVector> MainMapRescueAnchorLocations;
	bool bUsingTowerMainMapLayout = false;
	T66TowerMapTerrain::FLayout CachedTowerMainMapLayout;
	bool bTowerBossEntryTriggered = false;
	bool bTowerBossEntryApplied = false;
	bool bTowerBossDefeated = false;
	float TowerTerrainSafetyAccumulator = 0.f;
	bool bBackroomsPocketSpawned = false;
	bool bBackroomsChallengeActive = false;
	bool bBackroomsInventorySuppressed = false;
	bool bBackroomsStageTimerWasActive = false;
	bool bBackroomsSpeedRunWasActive = false;
	FTransform BackroomsReturnTransform;
	FName BackroomsSavedWeaponID = NAME_None;
	TArray<FT66InventorySlot> BackroomsSavedInventory;
	TMap<TWeakObjectPtr<AActor>, bool> BackroomsTickSnapshot;
	TMap<TWeakObjectPtr<UActorComponent>, bool> BackroomsComponentTickSnapshot;
	FVector BackroomsOrigin = FVector::ZeroVector;
	float BackroomsCellSize = 600.f;
	int32 BackroomsMazeWidth = 0;
	int32 BackroomsMazeHeight = 0;
	TArray<uint8> BackroomsMazeOpenCells;
	float TowerTrapActivationAccumulator = 0.f;
	bool bTowerMiasmaActive = false;
	float TowerMiasmaStartWorldSeconds = 0.f;
	float TowerMiasmaUpdateAccumulator = 0.f;
	int32 TowerIdolSelectionsAtStageStart = 0;
	int32 ActiveTowerTrapFloorNumber = INDEX_NONE;
	int32 ActiveTowerTerrainVisualFloorNumber = INDEX_NONE;
	/**
	 * Stateful hero floor membership: changes ONLY through explicit transitions
	 * (layout build/spawn, descent holes, fall rescue) — never from altitude, so
	 * jumps can never flip the active floor (2026-06-10 black-map root cause).
	 */
	int32 StatefulHeroTowerFloorNumber = INDEX_NONE;

	bool bFinalDifficultySurvivalActive = false;
	float FinalDifficultySurvivalElapsedSeconds = 0.f;
	float LastAppliedFinalDifficultyEnemyScalar = 1.f;
	bool bFinalBossSecondPhaseActive = false;
	bool bFinalBossUnwinnableEndingActive = false;
	TWeakObjectPtr<AActor> FinalDifficultyTotemActor;
	TWeakObjectPtr<AT66SaintNPC> FinalDifficultySaintActor;

	// The Lab: actors spawned from Lab panels (for Reset Enemies).
	UPROPERTY()
	TArray<TObjectPtr<AActor>> LabSpawnedActors;

	FVector GetLabSpawnLocation() const;
};
