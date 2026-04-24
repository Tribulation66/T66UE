// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66CompanionBase.h"
#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66StartGate.h"
#include "Gameplay/T66StageGate.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66MiasmaManager.h"
#include "Gameplay/T66MiasmaBoundary.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66SaintNPC.h"
#include "Gameplay/T66OuroborosNPC.h"
#include "Gameplay/T66LoanShark.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66CowardiceGate.h"
#include "Gameplay/T66VisualUtil.h"
#include "Gameplay/T66TricksterNPC.h"
#include "Gameplay/T66DifficultyTotem.h"
#include "Gameplay/T66BossGate.h"
#include "Gameplay/T66TowerDescentHole.h"
#include "Core/T66RunIntegritySubsystem.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/Enemies/T66EnemyFamilyResolver.h"
#include "Gameplay/T66GoblinThiefEnemy.h"
#include "Gameplay/T66UniqueDebuffEnemy.h"
#include "Gameplay/T66IdolAltar.h"
#include "Gameplay/T66FountainOfLifeInteractable.h"
#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66WheelSpinInteractable.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66CrateInteractable.h"
#include "Gameplay/T66CasinoInteractable.h"
#include "Gameplay/T66PilotableTractor.h"
#include "Gameplay/T66ArcadeTruckInteractable.h"
#include "Gameplay/T66WhackAMoleArcadeInteractable.h"
#include "Gameplay/T66QuickReviveVendingMachine.h"
#include "Gameplay/T66TeleportPadInteractable.h"
#include "Gameplay/T66StageCatchUpGate.h"
#include "Gameplay/T66StageCatchUpGoldInteractable.h"
#include "Gameplay/T66StageCatchUpLootInteractable.h"
#include "Gameplay/T66StageEffects.h"
#include "Gameplay/T66SpawnPlateau.h"
#include "Gameplay/T66LabCollector.h"
#include "Gameplay/T66TutorialManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66RetroFXSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66StageProgressionSubsystem.h"
#include "Core/T66TrapSubsystem.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66SkillRatingSubsystem.h"
#include "Core/T66GameplayLayout.h"
#include "Core/T66PropSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Gameplay/T66RecruitableCompanion.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "Gameplay/T66StageProgressionVisuals.h"
#include "UI/T66LoadingScreenWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"
#include "GameFramework/PlayerStart.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "EngineUtils.h"
#include "Gameplay/T66MainMapTerrain.h"
#include "Gameplay/T66TowerMapTerrain.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Landscape.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66TerrainThemeAssets.h"
#include "Gameplay/T66WorldVisualSetup.h"
#include "T66.h"

DECLARE_LOG_CATEGORY_EXTERN(LogT66GameMode, Log, All);

namespace T66GameModePrivate
{
	inline constexpr float T66TowerStageTransitionDropHeight = 7800.0f;
	inline constexpr int32 T66MaxGlobalStage = 23;
	inline constexpr bool T66EnableWheelSpinSpawns = false;
	inline constexpr float T66MainMapRoomReserveRadiusCells = 2.70f;
	inline constexpr float T66MainMapCorridorReserveRadiusCells = 0.80f;
	inline constexpr float T66FinalDifficultySurvivalDurationSeconds = 60.0f;

	extern const FName T66MainMapTerrainVisualTag;
	extern const FName T66MainMapTerrainMaterialsReadyTag;
	extern const FName T66TowerCeilingTag;
	extern const FName T66TowerTraceBarrierTag;
	extern const TCHAR* T66TowerFloorTagPrefix;

	bool T66TryGetNextDifficulty(ET66Difficulty Current, ET66Difficulty& OutNextDifficulty);
	int32 T66GetDifficultyEndStage(ET66Difficulty Difficulty);
	bool T66IsReachableProgressionStage(int32 StageNum);
	void T66_SetStaticMeshActorMobility(AStaticMeshActor* Actor, EComponentMobility::Type Mobility);
	bool T66_IsCompanionUnlockStage(int32 StageNum);
	bool T66_IsDifficultyBossStage(int32 StageNum);
	int32 T66_CountReachableProgressionStagesUpTo(int32 StageNum);
	int32 T66_CountFinaleBonusUnlocksBeforeStage(int32 StageNum);
	int32 T66_CompanionBaseIndexForStage(int32 StageNum);
	void T66_AppendCompanionUnlockIDsForStage(int32 StageNum, TArray<FName>& OutCompanionIDs);
	int32 T66ResolveFallbackBossStageNum(FName BossID, int32 DefaultStageNum);
	void T66BuildFallbackBossData(int32 StageNum, FName BossID, FBossData& OutBossData);
	UClass* T66LoadBossClassSync(const FBossData& BossData);
	FVector T66ComputeBossClusterLocation(const FVector& Center, int32 BossIndex, int32 BossCount);
	bool T66_HasAnyFloorTag(const AActor* Actor);
	void T66ResetTaggedActorCache(UWorld* World);
	void T66InvalidatePlayerStartCache(UWorld* World);
	void T66RefreshPlayerStartCache(UWorld* World, bool bForceRefresh = false);
	void T66AddCachedPlayerStartsToTraceIgnore(UWorld* World, FCollisionQueryParams& Params);
	bool T66HasAnyCachedPlayerStarts(UWorld* World);
	void T66DestroyCachedPlayerStarts(UWorld* World);
	AActor* T66FindMainMapTerrainVisualActor(UWorld* World);
	AActor* T66FindTaggedActor(UWorld* World, FName Tag);
	void T66RememberTaggedActor(AActor* Actor, FName Tag);
	void T66ForgetTaggedActor(UWorld* World, FName Tag);
	bool T66HasRegisteredCasino(UWorld* World);
	bool T66ShouldIgnoreTowerCeilingHit(const FHitResult& Hit);
	FName T66MakeTowerFloorTag(int32 FloorNumber);
	int32 T66ReadTowerFloorTag(const AActor* Actor);
	void T66AssignTowerFloorTag(AActor* Actor, int32 FloorNumber);
	const T66TowerMapTerrain::FFloor* T66FindTowerFloorByNumber(const T66TowerMapTerrain::FLayout& Layout, int32 FloorNumber);
	bool T66ShouldLogTowerGatePlacement(const AActor* Actor);
	bool T66TrySnapActorToTowerFloor(UWorld* World, AActor* Actor, const T66TowerMapTerrain::FLayout& Layout, int32 FloorNumber, const FVector& DesiredLocation);
	bool T66AreMainMapTerrainMaterialsReady(UWorld* World);
	bool T66UsesMainMapTerrainStage(const UWorld* World);
	void T66DestroyMiasmaBoundaryActors(UWorld* World);
	bool T66HasRegisteredMiasmaBoundary(UWorld* World);
	AT66MiasmaBoundary* T66EnsureMiasmaBoundaryActor(UWorld* World, const FActorSpawnParameters& SpawnParams);
	void T66PauseTowerMiasma(AT66MiasmaManager* MiasmaManager);
	void T66ActivateStageMiasma(AT66MiasmaManager* MiasmaManager);
	int32 T66EnsureRunSeed(UT66GameInstance* GI);
	FT66MapPreset T66BuildMainMapPreset(UT66GameInstance* GI);
	const TCHAR* T66GetMainMapLayoutVariantLabel();
	const AT66SessionPlayerState* T66GetSessionPlayerState(const AController* Controller);
	FName T66GetSelectedHeroID(const UT66GameInstance* GI, const AController* Controller);
	FName T66GetSelectedCompanionID(const UT66GameInstance* GI, const AController* Controller);
	ET66BodyType T66GetSelectedHeroBodyType(const UT66GameInstance* GI, const AController* Controller);
	FName T66GetSelectedHeroSkinID(const UT66GameInstance* GI, const AController* Controller);
	int32 T66GetConnectedPlayerCount(const UWorld* World);
	int32 T66GetPlayerSlotIndex(const UWorld* World, const AController* Controller);
	bool T66IsStandaloneTutorialMap(const UWorld* World);
	bool T66TryGetTaggedActorTransform(const UWorld* World, FName Tag, FVector& OutLocation, FRotator& OutRotation);
	void T66FaceActorTowardLocation2D(AActor* Actor, const FVector& TargetLocation);
	bool T66TryBuildFacingRotation2D(const FVector& FromLocation, const FVector& TargetLocation, FRotator& OutRotation);
	void T66SyncPawnAndControllerRotation(APawn* Pawn, AController* Controller, const FRotator& DesiredRotation);
}
