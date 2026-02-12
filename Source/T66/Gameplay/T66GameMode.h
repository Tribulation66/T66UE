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
class AT66SpawnPlateau;
class AT66TutorialManager;
class UT66GameInstance;
class AT66RecruitableCompanion;
class ADirectionalLight;
class ASkyLight;
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

	/**
	 * Four ground material variants (0°, 90°, 180°, 270° rotation). One is picked per floor
	 * based on floor position for deterministic variety. All must be loaded for grass texture.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level Setup")
	TArray<TSoftObjectPtr<UMaterialInterface>> GroundFloorMaterials;

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

	/** Apply UI theme (Dark = moonlight, Light = sun) to directional lights in the given world. Shared with frontend. */
	static void ApplyThemeToDirectionalLightsForWorld(UWorld* World);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/** Get the Game Instance cast to our type */
	UT66GameInstance* GetT66GameInstance() const;

	/** Generate procedural hills terrain if level has a Landscape and GameInstance has a seed. Logs [MAP] on failure. */
	void GenerateProceduralTerrainIfNeeded();

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

	/** Spawn lighting if none exists */
	void SpawnLightingIfNeeded();

	/** Spawn a PlayerStart if none exists */
	void SpawnPlayerStartIfNeeded();

	/** Apply theme to this world's directional lights (calls ApplyThemeToDirectionalLightsForWorld). */
	void ApplyThemeToDirectionalLights();

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
	void SpawnIdolAltarForPlayer(AController* Player);
	void SpawnIdolAltarAtLocation(const FVector& Location);

	/** Spawn Boss Gate (walk-through, awakens boss) between main and boss areas. */
	void SpawnBossGateIfNeeded();
	void SpawnWorldInteractablesForStage();
	void SpawnStageBoostPlatformAndInteractables();
	void SpawnStageEffectTilesForStage();
	void SpawnColiseumArenaIfNeeded();
	void SpawnTutorialArenaIfNeeded();
	void SpawnAllOwedBossesInColiseum();
	void SpawnTutorialIfNeeded();

	/** Spawn boss for current stage (dormant until player approaches). */
	void SpawnBossForCurrentStage();

	/** Spawn the 4 corner houses + 4 NPC cylinders (vendor/gambler/ouroboros/saint). */
	void SpawnCornerHousesAndNPCs();

	/** Called one frame after BeginPlay so the landscape/collision is ready. Spawns all ground-dependent content (NPCs, interactables, tiles, boss, etc.). */
	void SpawnLevelContentAfterLandscapeReady();

	/** Spawn a flat plateau so its top surface is at TopCenterLoc (used under NPCs and world interactables). */
	void SpawnPlateauAtLocation(UWorld* World, const FVector& TopCenterLoc);

	void SpawnTricksterAndCowardiceGate();

	bool IsColiseumStage() const;
	/** True when current level is The Lab (practice room). */
	bool IsLabLevel() const;
	void ResetColiseumState();

	/** Called when stage timer changes (we use it to detect "timer started"). */
	UFUNCTION()
	void HandleStageTimerChanged();

	UFUNCTION()
	void HandleDifficultyChanged();

	void TrySpawnLoanSharkIfNeeded();

public:
	// ============================================
	// The Lab: spawn / reset (only used when IsLabLevel())
	// ============================================

	/** Spawn one mob in the Lab (CharacterVisualID: RegularEnemy, Leprechaun, GoblinThief, UniqueEnemy). Returns spawned actor or null. */
	UFUNCTION(BlueprintCallable, Category = "Lab")
	AActor* SpawnLabMob(FName CharacterVisualID);

	/** Spawn one boss in the Lab from Bosses DataTable. Returns spawned actor or null. */
	UFUNCTION(BlueprintCallable, Category = "Lab")
	AActor* SpawnLabBoss(FName BossID);

	/** Spawn Tree of Life in the Lab (NPC tab). Returns spawned actor or null. */
	UFUNCTION(BlueprintCallable, Category = "Lab")
	AActor* SpawnLabTreeOfLife();

	/** Spawn an interactable in the Lab (TreeOfLife, CashTruck, WheelSpin, IdolAltar). Returns spawned actor or null. */
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

	// Async load tracking (prevents gameplay hitching from sync loads).
	TArray<TSharedPtr<FStreamableHandle>> ActiveAsyncLoadHandles;

	// Track currently spawned companions per player so respawns don't duplicate them.
	TMap<TWeakObjectPtr<AController>, TWeakObjectPtr<AT66CompanionBase>> PlayerCompanions;

	// Track the current stage boss so we can safely replace it after async load.
	TWeakObjectPtr<AT66BossBase> StageBoss;

	// Cached engine primitive mesh (loaded once, reused across all spawn functions).
	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> CachedCubeMesh;

	/** Returns the cached /Engine/BasicShapes/Cube mesh, loading once on first call. */
	UStaticMesh* GetCubeMesh();

	// Floor material soft-load.
	bool bGroundFloorMaterialLoadRequested = false;

	// Coliseum: async-load boss classes before spawning.
	bool bColiseumBossesAsyncLoadInFlight = false;
	bool bColiseumBossesAsyncLoadAttempted = false;

	// Coliseum state (only used when bForceColiseumMode is true).
	int32 ColiseumBossesRemaining = 0;
	bool bColiseumExitGateSpawned = false;
	FVector ColiseumCenter = FVector(-45455.f, -23636.f, 200.f);

	// The Lab: actors spawned from Lab panels (for Reset Enemies).
	UPROPERTY()
	TArray<TObjectPtr<AActor>> LabSpawnedActors;

	FVector GetLabSpawnLocation() const;
};
