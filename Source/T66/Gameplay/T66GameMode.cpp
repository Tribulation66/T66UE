// Copyright Tribulation 66. All Rights Reserved.

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
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66GoblinThiefEnemy.h"
#include "Gameplay/T66IdolAltar.h"
#include "Gameplay/T66FountainOfLifeInteractable.h"
#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66WheelSpinInteractable.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66CrateInteractable.h"
#include "Gameplay/T66CircusInteractable.h"
#include "Gameplay/T66PilotableTractor.h"
#include "Gameplay/T66QuickReviveVendingMachine.h"
#include "Gameplay/T66TeleportPadInteractable.h"
#include "Gameplay/T66StageCatchUpGate.h"
#include "Gameplay/T66StageCatchUpGoldInteractable.h"
#include "Gameplay/T66StageCatchUpLootInteractable.h"
#include "Gameplay/T66StageEffects.h"
#include "Gameplay/T66LavaPatch.h"
#include "Gameplay/T66SpawnPlateau.h"
#include "Gameplay/T66LabCollector.h"
#include "Gameplay/T66QuakeSkyActor.h"
#include "Gameplay/T66TutorialManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66RetroFXSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66PosterizeSubsystem.h"
#include "Gameplay/T66EclipseActor.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66TrapSubsystem.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66SkillRatingSubsystem.h"
#include "Core/T66GameplayLayout.h"
#include "Core/T66PropSubsystem.h"
#include "Gameplay/T66RecruitableCompanion.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "UI/T66LoadingScreenWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/LightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
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
#include "Gameplay/T66LightingSubsystem.h"
#include "Gameplay/T66TerrainThemeAssets.h"
#include "T66.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66GameMode, Log, All);

namespace
{
	static constexpr float T66TowerStageTransitionDropHeight = 7800.0f;

	// Helper: avoid PIE warnings ("StaticMeshComponent has to be 'Movable' if you'd like to move")
	// by temporarily setting mobility to Movable while we apply transforms.
	static void T66_SetStaticMeshActorMobility(AStaticMeshActor* Actor, EComponentMobility::Type Mobility)
	{
		if (!Actor) return;
		if (UStaticMeshComponent* SMC = Actor->GetStaticMeshComponent())
		{
			if (SMC->Mobility != Mobility)
			{
				SMC->SetMobility(Mobility);
			}
		}
	}

	static bool T66_IsCompanionUnlockStage(int32 StageNum)
	{
		// Stage clears unlock Companion_01..Companion_23 directly.
		// Companion_24 is granted as a bonus unlock on stage 23.
		return StageNum >= 1 && StageNum <= 23;
	}

	static bool T66_IsDifficultyBossStage(int32 StageNum)
	{
		return StageNum == 5 || StageNum == 10 || StageNum == 15 || StageNum == 20 || StageNum == 23;
	}

	static int32 T66_CompanionIndexForStage(int32 StageNum)
	{
		if (!T66_IsCompanionUnlockStage(StageNum))
		{
			return INDEX_NONE;
		}

		int32 Count = 0;
		for (int32 S = 1; S <= StageNum; ++S)
		{
			if (T66_IsCompanionUnlockStage(S))
			{
				++Count;
			}
		}
		return Count; // 1..23
	}

	static FName T66_CompanionIDForStage(int32 StageNum)
	{
		const int32 Index = T66_CompanionIndexForStage(StageNum);
		if (Index == INDEX_NONE)
		{
			return NAME_None;
		}
		return FName(*FString::Printf(TEXT("Companion_%02d"), Index));
	}

	static bool T66_HasAnyFloorTag(const AActor* A)
	{
		if (!A) return false;
		// Covers: T66_Floor_Main/Start/Boss/Conn*, and also Boost/Tutorial/Coliseum floors.
		static const FName Prefix(TEXT("T66_Floor"));
		for (const FName& Tag : A->Tags)
		{
			const FString S = Tag.ToString();
			if (S.StartsWith(Prefix.ToString()))
			{
				return true;
			}
		}
		return false;
	}

	static const FName T66MainMapTerrainVisualTag(TEXT("T66_MainMapTerrain_Visual"));
	static const FName T66MainMapTerrainMaterialsReadyTag(TEXT("T66_MainMapTerrain_MaterialsReady"));
	static const FName T66TowerCeilingTag(TEXT("T66_Tower_Ceiling"));
	static const FName T66TowerTraceBarrierTag(TEXT("T66_Map_TraversalBarrier"));
	static const TCHAR* T66TowerFloorTagPrefix = TEXT("T66_TowerFloor_");
	static constexpr bool T66EnableWheelSpinSpawns = false;
	static constexpr float T66MainMapRoomReserveRadiusCells = 2.70f;
	static constexpr float T66MainMapCorridorReserveRadiusCells = 0.80f;

	static bool T66ShouldIgnoreTowerCeilingHit(const FHitResult& Hit)
	{
		const AActor* HitActor = Hit.GetActor();
		return HitActor
			&& (HitActor->ActorHasTag(T66TowerCeilingTag) || HitActor->ActorHasTag(T66TowerTraceBarrierTag));
	}

	static FName T66MakeTowerFloorTag(const int32 FloorNumber)
	{
		return FName(*FString::Printf(TEXT("%s%02d"), T66TowerFloorTagPrefix, FloorNumber));
	}

	static int32 T66ReadTowerFloorTag(const AActor* Actor)
	{
		if (!Actor)
		{
			return INDEX_NONE;
		}

		for (const FName& Tag : Actor->Tags)
		{
			const FString TagString = Tag.ToString();
			if (!TagString.StartsWith(T66TowerFloorTagPrefix))
			{
				continue;
			}

			const FString NumberString = TagString.RightChop(FCString::Strlen(T66TowerFloorTagPrefix));
			return FCString::Atoi(*NumberString);
		}

		return INDEX_NONE;
	}

	static void T66AssignTowerFloorTag(AActor* Actor, const int32 FloorNumber)
	{
		if (!Actor || FloorNumber == INDEX_NONE)
		{
			return;
		}

		for (int32 Index = Actor->Tags.Num() - 1; Index >= 0; --Index)
		{
			if (Actor->Tags[Index].ToString().StartsWith(T66TowerFloorTagPrefix))
			{
				Actor->Tags.RemoveAt(Index);
			}
		}

		Actor->Tags.AddUnique(T66MakeTowerFloorTag(FloorNumber));
	}

	static const T66TowerMapTerrain::FFloor* T66FindTowerFloorByNumber(const T66TowerMapTerrain::FLayout& Layout, const int32 FloorNumber)
	{
		for (const T66TowerMapTerrain::FFloor& Floor : Layout.Floors)
		{
			if (Floor.FloorNumber == FloorNumber)
			{
				return &Floor;
			}
		}

		return nullptr;
	}

	static bool T66TrySnapActorToTowerFloor(UWorld* World, AActor* Actor, const T66TowerMapTerrain::FLayout& Layout, const int32 FloorNumber, const FVector& DesiredLocation)
	{
		if (!World || !Actor)
		{
			return false;
		}

		const T66TowerMapTerrain::FFloor* Floor = T66FindTowerFloorByNumber(Layout, FloorNumber);
		if (!Floor)
		{
			return false;
		}

		const float TraceUp = FMath::Max(700.0f, Layout.FloorSpacing * 0.45f);
		const float TraceDown = FMath::Max(1000.0f, Layout.FloorThickness + 1200.0f);
		const FVector TraceStart(DesiredLocation.X, DesiredLocation.Y, Floor->SurfaceZ + TraceUp);
		const FVector TraceEnd(DesiredLocation.X, DesiredLocation.Y, Floor->SurfaceZ - TraceDown);

		FCollisionQueryParams Params(SCENE_QUERY_STAT(T66SnapActorToTowerFloor), false, Actor);
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			Params.AddIgnoredActor(*It);
		}

		FHitResult GroundHit;
		bool bFoundGround = false;
		TArray<FHitResult> Hits;
		if (World->LineTraceMultiByChannel(Hits, TraceStart, TraceEnd, ECC_WorldStatic, Params))
		{
			const float SurfaceTolerance = FMath::Max(120.0f, (Layout.FloorThickness * 0.5f) + 20.0f);
			for (const FHitResult& Hit : Hits)
			{
				if (T66ShouldIgnoreTowerCeilingHit(Hit))
				{
					continue;
				}

				if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
				{
					continue;
				}

				if (FMath::Abs(Hit.ImpactPoint.Z - Floor->SurfaceZ) > SurfaceTolerance)
				{
					continue;
				}

				GroundHit = Hit;
				bFoundGround = true;
				break;
			}
		}

		float HalfHeight = 0.0f;
		if (const UCapsuleComponent* Capsule = Actor->FindComponentByClass<UCapsuleComponent>())
		{
			HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		const float GroundZ = bFoundGround ? GroundHit.ImpactPoint.Z : Floor->SurfaceZ;
		Actor->SetActorLocation(
			FVector(DesiredLocation.X, DesiredLocation.Y, GroundZ + HalfHeight + 5.0f),
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
		return true;
	}

	static bool T66AreMainMapTerrainMaterialsReady(UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor || !Actor->ActorHasTag(T66MainMapTerrainVisualTag))
			{
				continue;
			}

			return Actor->ActorHasTag(T66MainMapTerrainMaterialsReadyTag);
		}

		return false;
	}

	static bool T66UsesMainMapTerrainStage(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		return !MapName.Contains(TEXT("Coliseum"))
			&& !MapName.Contains(TEXT("Tutorial"))
			&& !MapName.Contains(TEXT("Lab"));
	}

	static bool T66IsStandaloneColiseumMap(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		return MapName.Contains(TEXT("Coliseum"));
	}

	static constexpr float T66FinalDifficultySurvivalDurationSeconds = 60.f;

	static int32 T66EnsureRunSeed(UT66GameInstance* GI)
	{
		if (!GI)
		{
			return FMath::Rand();
		}

		if (GI->RunSeed == 0)
		{
			GI->RunSeed = FMath::Rand();
		}

		return GI->RunSeed;
	}

	static FT66MapPreset T66BuildMainMapPreset(UT66GameInstance* GI)
	{
		const ET66Difficulty Difficulty = GI ? GI->SelectedDifficulty : ET66Difficulty::Easy;
		const ET66MainMapLayoutVariant LayoutVariant = UT66GameInstance::ResolveMainMapLayoutVariant(GI);
		return T66MainMapTerrain::BuildPresetForDifficulty(Difficulty, T66EnsureRunSeed(GI), LayoutVariant);
	}

	static const TCHAR* T66GetMainMapLayoutVariantLabel(const ET66MainMapLayoutVariant LayoutVariant)
	{
		switch (LayoutVariant)
		{
		case ET66MainMapLayoutVariant::Tower:
			return TEXT("Tower");
		case ET66MainMapLayoutVariant::Flat:
			return TEXT("Flat");
		case ET66MainMapLayoutVariant::Hilly:
		default:
			return TEXT("Hilly");
		}
	}

	static const AT66SessionPlayerState* T66GetSessionPlayerState(const AController* Controller)
	{
		return Controller ? Controller->GetPlayerState<AT66SessionPlayerState>() : nullptr;
	}

	static FName T66GetSelectedHeroID(const UT66GameInstance* GI, const AController* Controller)
	{
		if (const AT66SessionPlayerState* SessionPlayerState = T66GetSessionPlayerState(Controller))
		{
			if (!SessionPlayerState->GetSelectedHeroID().IsNone())
			{
				return SessionPlayerState->GetSelectedHeroID();
			}
		}

		return (GI && !GI->SelectedHeroID.IsNone()) ? GI->SelectedHeroID : FName(TEXT("Hero_1"));
	}

	static FName T66GetSelectedCompanionID(const UT66GameInstance* GI, const AController* Controller)
	{
		if (const AT66SessionPlayerState* SessionPlayerState = T66GetSessionPlayerState(Controller))
		{
			return SessionPlayerState->GetSelectedCompanionID();
		}

		return GI ? GI->SelectedCompanionID : NAME_None;
	}

	static ET66BodyType T66GetSelectedHeroBodyType(const UT66GameInstance* GI, const AController* Controller)
	{
		if (const AT66SessionPlayerState* SessionPlayerState = T66GetSessionPlayerState(Controller))
		{
			return SessionPlayerState->GetSelectedHeroBodyType();
		}

		return GI ? GI->SelectedHeroBodyType : ET66BodyType::TypeA;
	}

	static FName T66GetSelectedHeroSkinID(const UT66GameInstance* GI, const AController* Controller)
	{
		if (const AT66SessionPlayerState* SessionPlayerState = T66GetSessionPlayerState(Controller))
		{
			const FName SkinID = SessionPlayerState->GetSelectedHeroSkinID();
			if (!SkinID.IsNone())
			{
				return SkinID;
			}
		}

		if (GI && !GI->SelectedHeroSkinID.IsNone())
		{
			return GI->SelectedHeroSkinID;
		}

		return FName(TEXT("Default"));
	}

	static int32 T66GetConnectedPlayerCount(const UWorld* World)
	{
		if (!World)
		{
			return 1;
		}

		int32 Count = 0;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (It->Get())
			{
				++Count;
			}
		}

		return FMath::Max(1, Count);
	}

	static int32 T66GetPlayerSlotIndex(const UWorld* World, const AController* Controller)
	{
		if (!World || !Controller)
		{
			return 0;
		}

		int32 Index = 0;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (It->Get() == Controller)
			{
				return Index;
			}

			++Index;
		}

		return 0;
	}

	static bool T66IsStandaloneTutorialMap(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		return MapName.Contains(TEXT("Tutorial"));
	}

	static bool T66TryGetTaggedActorTransform(const UWorld* World, const FName Tag, FVector& OutLocation, FRotator& OutRotation)
	{
		if (!World || Tag.IsNone())
		{
			return false;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->ActorHasTag(Tag))
			{
				OutLocation = Actor->GetActorLocation();
				OutRotation = Actor->GetActorRotation();
				return true;
			}
		}

		return false;
	}

	static void T66FaceActorTowardLocation2D(AActor* Actor, const FVector& TargetLocation)
	{
		if (!Actor)
		{
			return;
		}

		FVector FacingDirection = TargetLocation - Actor->GetActorLocation();
		FacingDirection.Z = 0.f;
		if (FacingDirection.IsNearlyZero())
		{
			return;
		}

		Actor->SetActorRotation(FacingDirection.Rotation());
	}

	static bool T66TryBuildFacingRotation2D(const FVector& FromLocation, const FVector& TargetLocation, FRotator& OutRotation)
	{
		FVector FacingDirection = TargetLocation - FromLocation;
		FacingDirection.Z = 0.f;
		if (FacingDirection.IsNearlyZero())
		{
			return false;
		}

		OutRotation = FacingDirection.Rotation();
		OutRotation.Pitch = 0.f;
		OutRotation.Roll = 0.f;
		return true;
	}

	static void T66SyncPawnAndControllerRotation(APawn* Pawn, AController* Controller, const FRotator& DesiredRotation)
	{
		if (!Pawn)
		{
			return;
		}

		FRotator FlatRotation = DesiredRotation;
		FlatRotation.Pitch = 0.f;
		FlatRotation.Roll = 0.f;
		Pawn->SetActorRotation(FlatRotation, ETeleportType::TeleportPhysics);

		if (!Controller)
		{
			return;
		}

		Controller->SetControlRotation(FlatRotation);
		if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			PlayerController->ClientSetRotation(FlatRotation, true);
		}
	}

}

AT66GameMode::AT66GameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	// Default to our hero base class
	DefaultPawnClass = AT66HeroBase::StaticClass();
	DefaultHeroClass = AT66HeroBase::StaticClass();
	PlayerStateClass = AT66SessionPlayerState::StaticClass();
	bUseSeamlessTravel = true;

	// Coliseum arena lives inside GameplayLevel, off to the side (walled off). Scaled for 100k map.
	ColiseumCenter = FVector(-45455.f, -23636.f, 200.f);

	FT66TerrainThemeAssets::FillDefaultCliffSideMaterials(CliffSideMaterials);
}

UStaticMesh* AT66GameMode::GetCubeMesh()
{
	if (!CachedCubeMesh)
	{
		CachedCubeMesh = FT66VisualUtil::GetBasicShapeCube();
	}
	return CachedCubeMesh;
}


void AT66GameMode::BeginPlay()
{
	Super::BeginPlay();

	// Main flat floor is spawned in SpawnLevelContentAfterLandscapeReady (no external asset packs).
	InitializeRunStateForBeginPlay();

	if (bAutoSetupLevel)
	{
		EnsureLevelSetup();
	}

	if (UGameInstance* GIP = GetGameInstance())
	{
		if (UT66PlayerSettingsSubsystem* PS = GIP->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.AddDynamic(this, &AT66GameMode::HandleSettingsChanged);
		}
	}
	HandleSettingsChanged();

	if (HandleSpecialModeBeginPlay())
	{
		return;
	}

	ConsumePendingStageCatchUp();
	ScheduleDeferredGameplayLevelSpawn();
	UE_LOG(LogT66GameMode, Log, TEXT("T66GameMode BeginPlay - Level setup scheduled; content will spawn after landscape is ready."));
}

void AT66GameMode::InitializeRunStateForBeginPlay()
{
	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!GI || !T66GI)
	{
		return;
	}

	if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
	{
		// Bind to timer changes so we can spawn LoanShark exactly when timer starts.
		RunState->StageTimerChanged.AddDynamic(this, &AT66GameMode::HandleStageTimerChanged);
		RunState->DifficultyChanged.AddDynamic(this, &AT66GameMode::HandleDifficultyChanged);

		if (T66GI->bApplyLoadedRunSnapshot)
		{
			T66GI->bApplyLoadedRunSnapshot = false;
			RunState->ResetForNewRun();
			RunState->ImportSavedRunSnapshot(T66GI->PendingLoadedRunSnapshot);
			T66GI->PendingLoadedRunSnapshot = FT66SavedRunSnapshot{};
			if (UT66DamageLogSubsystem* DamageLog = GI->GetSubsystem<UT66DamageLogSubsystem>())
			{
				DamageLog->ResetForNewRun();
			}
			return;
		}

		// Robust: treat any stage > 1 as a stage transition even if a gate forgot to set the flag.
		const bool bKeepProgress = T66GI->bIsStageTransition || (RunState->GetCurrentStage() > 1);
		if (bKeepProgress)
		{
			T66GI->bIsStageTransition = false;
			RunState->ResetStageTimerToFull(); // New stage: timer frozen at 60 until start gate
			RunState->ResetBossState(); // New stage: boss is dormant again; hide boss UI
			return;
		}

		T66GI->bRunIneligibleForLeaderboard = false; // Fresh run is eligible
		T66GI->bPendingTowerStageDropIntro = false;
		RunState->ResetForNewRun();
		RunState->ActivatePendingSingleUseBuffsForRunStart();
		if (UT66DamageLogSubsystem* DamageLog = GI->GetSubsystem<UT66DamageLogSubsystem>())
		{
			DamageLog->ResetForNewRun();
		}
	}
}

bool AT66GameMode::HandleSpecialModeBeginPlay()
{
	if (IsColiseumStage())
	{
		HandleColiseumBeginPlay();
		return true;
	}

	if (IsLabLevel())
	{
		HandleLabBeginPlay();
		return true;
	}

	return false;
}

void AT66GameMode::HandleColiseumBeginPlay()
{
	ResetColiseumState();
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			// Coliseum countdown begins immediately (no start gate).
			RunState->ResetStageTimerToFull();
			RunState->SetStageTimerActive(true);
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (UWorld* World = GetWorld())
	{
		MiasmaManager = World->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}

	SpawnColiseumArenaIfNeeded();
	if (IsFinalDifficultyBossColiseumStage())
	{
		SpawnFinalDifficultyBossInColiseum();
	}
	else
	{
		SpawnAllOwedBossesInColiseum();
	}
	UE_LOG(LogT66GameMode, Log, TEXT("T66GameMode BeginPlay - Coliseum"));
}

void AT66GameMode::HandleLabBeginPlay()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->ResetForNewRun();
		}
		if (UT66DamageLogSubsystem* DamageLog = GI->GetSubsystem<UT66DamageLogSubsystem>())
		{
			DamageLog->ResetForNewRun();
		}
	}

	if (bAutoSetupLevel)
	{
		EnsureLevelSetup();
	}

	UE_LOG(LogT66GameMode, Log, TEXT("T66GameMode BeginPlay - Lab"));
}

void AT66GameMode::ConsumePendingStageCatchUp()
{
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!T66GI || !T66GI->bStageCatchUpPending)
	{
		return;
	}

	T66GI->bStageCatchUpPending = false;
	if (UT66RunStateSubsystem* RunState = T66GI->GetSubsystem<UT66RunStateSubsystem>())
	{
		RunState->SetInStageCatchUp(false);
	}
}

void AT66GameMode::ScheduleDeferredGameplayLevelSpawn()
{
	if (UWorld* World = GetWorld())
	{
		// Normal stage: defer all ground-dependent spawns until next tick so the landscape is fully formed and collision is ready.
		World->GetTimerManager().SetTimerForNextTick(this, &AT66GameMode::SpawnLevelContentAfterLandscapeReady);
	}
}

void AT66GameMode::SpawnLevelContentAfterLandscapeReady()
{
	UWorld* World = GetWorld();
	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	TWeakObjectPtr<UT66LoadingScreenWidget> GameplayWarmupOverlay = CreateGameplayWarmupOverlay(World, bUsingMainMapTerrain);

	// Phase 0: Spawn the runtime main map terrain before any ground-traced content.
	const double TerrainSpawnStartSeconds = FPlatformTime::Seconds();
	SpawnMainMapTerrain();
	UE_LOG(LogT66GameMode, Log, TEXT("[LOAD] SpawnMainMapTerrain finished in %.1f ms."),
		(FPlatformTime::Seconds() - TerrainSpawnStartSeconds) * 1000.0);

	const double StructureSpawnStartSeconds = FPlatformTime::Seconds();
	SpawnStageStructuresAndInteractables(World, bUsingMainMapTerrain);
	UE_LOG(LogT66GameMode, Log, TEXT("[LOAD] SpawnStageStructuresAndInteractables finished in %.1f ms."),
		(FPlatformTime::Seconds() - StructureSpawnStartSeconds) * 1000.0);

	if (bUsingMainMapTerrain)
	{
		if (World)
		{
			FTimerHandle DeferredPropsHandle;
			World->GetTimerManager().SetTimer(
				DeferredPropsHandle,
				FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					const double PropSpawnStartSeconds = FPlatformTime::Seconds();
					SpawnStageDecorativeProps(true);
					UE_LOG(LogT66GameMode, Log, TEXT("[LOAD] Deferred main-map props spawned in %.1f ms."),
						(FPlatformTime::Seconds() - PropSpawnStartSeconds) * 1000.0);
				}),
				0.35f,
				false);
		}

		const double PrepareStageStartSeconds = FPlatformTime::Seconds();
		PrepareMainMapStage(World);
		UE_LOG(LogT66GameMode, Log, TEXT("[LOAD] PrepareMainMapStage finished in %.1f ms."),
			(FPlatformTime::Seconds() - PrepareStageStartSeconds) * 1000.0);
		ScheduleGameplayLightingRefresh(World);
		ScheduleGameplayWarmupOverlayHide(World, GameplayWarmupOverlay);
		UE_LOG(LogT66GameMode, Log, TEXT("T66GameMode - Main map terrain content spawned. Main-board combat and random interactables are waiting for the player to enter the board."));
		return;
	}

	const double PropSpawnStartSeconds = FPlatformTime::Seconds();
	SpawnStageDecorativeProps(false);
	UE_LOG(LogT66GameMode, Log, TEXT("[LOAD] SpawnStageDecorativeProps finished in %.1f ms."),
		(FPlatformTime::Seconds() - PropSpawnStartSeconds) * 1000.0);
	UE_LOG(LogT66GameMode, Log, TEXT("T66GameMode - Phase 1 content spawned (structures + NPCs)."));
	ScheduleStandardStageCombatBootstrap(World);
	ScheduleGameplayLightingRefresh(World);
	ScheduleGameplayWarmupOverlayHide(World, GameplayWarmupOverlay);
}

TWeakObjectPtr<UT66LoadingScreenWidget> AT66GameMode::CreateGameplayWarmupOverlay(UWorld* World, bool bUsingMainMapTerrain) const
{
	if (!bUsingMainMapTerrain || !World)
	{
		return nullptr;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return nullptr;
	}

	if (UT66LoadingScreenWidget* Overlay = CreateWidget<UT66LoadingScreenWidget>(PC, UT66LoadingScreenWidget::StaticClass()))
	{
		Overlay->AddToViewport(10000);
		return Overlay;
	}

	return nullptr;
}

void AT66GameMode::ScheduleGameplayLightingRefresh(UWorld* World)
{
	if (!World)
	{
		return;
	}

	// Recapture after runtime terrain/props register so first PIE does not keep a cold sky capture.
	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		ApplyThemeToAtmosphereAndLighting();
	}));

	FTimerHandle DelayedLightingRefreshHandle;
	World->GetTimerManager().SetTimer(
		DelayedLightingRefreshHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			ApplyThemeToAtmosphereAndLighting();
		}),
		0.35f,
		false);

	FTimerHandle FinalLightingRefreshHandle;
	World->GetTimerManager().SetTimer(
		FinalLightingRefreshHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			ApplyThemeToAtmosphereAndLighting();
		}),
		0.65f,
		false);
}

void AT66GameMode::ScheduleGameplayWarmupOverlayHide(UWorld* World, TWeakObjectPtr<UT66LoadingScreenWidget> GameplayWarmupOverlay)
{
	if (!World)
	{
		return;
	}

	// Tutorial / non-main-map stages do not create the terrain warmup overlay, but they
	// still need to clear the persistent black transition curtain after the level loads.
	if (!GameplayWarmupOverlay.IsValid())
	{
		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance()))
		{
			T66GI->HidePersistentGameplayTransitionCurtain();
		}
		return;
	}

	TSharedPtr<int32> OverlayPollCount = MakeShared<int32>(0);
	TSharedPtr<FTimerHandle> HideOverlayHandle = MakeShared<FTimerHandle>();
	World->GetTimerManager().SetTimer(
		*HideOverlayHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, World, GameplayWarmupOverlay, OverlayPollCount, HideOverlayHandle]()
		{
			if (!World)
			{
				return;
			}

			if (!GameplayWarmupOverlay.IsValid())
			{
				World->GetTimerManager().ClearTimer(*HideOverlayHandle);
				return;
			}

			const bool bTerrainReady = bTerrainCollisionReady || T66AreMainMapTerrainMaterialsReady(World);
			++(*OverlayPollCount);
			const bool bTimedOut = *OverlayPollCount >= 50;
			if (bTerrainReady || bTimedOut)
			{
				if (GameplayWarmupOverlay.IsValid())
				{
					GameplayWarmupOverlay->RemoveFromParent();
				}

				if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance()))
				{
					T66GI->HidePersistentGameplayTransitionCurtain();
				}

				World->GetTimerManager().ClearTimer(*HideOverlayHandle);
				if (bTimedOut && !bTerrainReady)
				{
					UE_LOG(LogT66GameMode, Warning, TEXT("T66GameMode - Gameplay warmup overlay timed out before main map terrain reported ready."));
				}
			}
		}),
		0.10f,
		true);
}

void AT66GameMode::SpawnStageStructuresAndInteractables(UWorld* World, bool bUsingMainMapTerrain)
{
	if (!bUsingMainMapTerrain && !IsColiseumStage() && !IsLabLevel())
	{
		if (AController* PC = World ? World->GetFirstPlayerController() : nullptr)
		{
			SpawnStartGateForPlayer(PC);
		}
	}

	if (AController* PC = World ? World->GetFirstPlayerController() : nullptr)
	{
		SpawnIdolAltarForPlayer(PC);
	}

	if (!IsUsingTowerMainMapLayout())
	{
		SpawnCircusInteractableIfNeeded();
		SpawnSupportVendorAtStartIfNeeded();
		SpawnGuaranteedStartAreaInteractables();
	}
	else
	{
		SpawnWorldInteractablesForStage();
	}

	if (!bUsingMainMapTerrain)
	{
		SpawnTricksterAndCowardiceGate();
		SpawnBossBeaconIfNeeded();
		SpawnWorldInteractablesForStage();
		SpawnTutorialIfNeeded();
	}
}

void AT66GameMode::SpawnStageDecorativeProps(bool bUsingMainMapTerrain)
{
	if (bUsingMainMapTerrain && IsUsingTowerMainMapLayout())
	{
		// Tower uses a dedicated terrain scaffold; grouped prop placement still assumes the legacy board generator.
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PropSubsystem* PropSub = GI->GetSubsystem<UT66PropSubsystem>())
		{
			UT66GameInstance* T66GI = GetT66GameInstance();
			const int32 PropSeed = (T66GI && T66GI->RunSeed != 0) ? T66GI->RunSeed : FMath::Rand();
			if (bUsingMainMapTerrain)
			{
				const TArray<FName> MainMapPropRows = {
					FName(TEXT("Barn")),
					FName(TEXT("Boulder")),
					FName(TEXT("Fence")),
					FName(TEXT("Fence2")),
					FName(TEXT("Fence3")),
					FName(TEXT("Haybell")),
					FName(TEXT("Log")),
					FName(TEXT("Rocks")),
					FName(TEXT("Scarecrow")),
					FName(TEXT("Silo")),
					FName(TEXT("Stump")),
					FName(TEXT("Tractor")),
					FName(TEXT("Tree")),
					FName(TEXT("Tree2")),
					FName(TEXT("Tree3")),
					FName(TEXT("Troth")),
					FName(TEXT("Windmill"))
				};
				PropSub->SpawnMainMapPropsForStage(GetWorld(), PropSeed, MainMapPropRows);
			}
			else
			{
				PropSub->SpawnPropsForStage(GetWorld(), PropSeed);
			}
		}
	}
}

void AT66GameMode::PrepareMainMapStage(UWorld* World)
{
	SpawnTricksterAndCowardiceGate();
	SpawnBossForCurrentStage();
	if (IsUsingTowerMainMapLayout())
	{
		SpawnTowerDescentHolesIfNeeded();
		SyncTowerBossEntryState();
	}

	if (!IsValid(MiasmaManager))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		MiasmaManager = World->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}
	else
	{
		MiasmaManager->RebuildForCurrentStage();
	}

	if (IsUsingTowerMainMapLayout())
	{
		for (TActorIterator<AT66MiasmaBoundary> It(World); It; ++It)
		{
			if (AT66MiasmaBoundary* ExistingBoundary = *It)
			{
				ExistingBoundary->Destroy();
			}
		}
	}
	else
	{
		bool bHasBoundaryActor = false;
		for (TActorIterator<AT66MiasmaBoundary> It(World); It; ++It)
		{
			if (AT66MiasmaBoundary* ExistingBoundary = *It)
			{
				bHasBoundaryActor = true;
				break;
			}
		}

		if (!bHasBoundaryActor)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			World->SpawnActor<AT66MiasmaBoundary>(AT66MiasmaBoundary::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}
	}

	TArray<AT66EnemyDirector*> ExistingEnemyDirectors;
	for (TActorIterator<AT66EnemyDirector> It(World); It; ++It)
	{
		if (AT66EnemyDirector* ExistingDirector = *It)
		{
			ExistingEnemyDirectors.Add(ExistingDirector);
		}
	}
	for (AT66EnemyDirector* ExistingDirector : ExistingEnemyDirectors)
	{
		if (ExistingDirector)
		{
			ExistingDirector->Destroy();
		}
	}

	bMainMapCombatStarted = false;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->ResetStageTimerToFull();
			RunState->SetStageTimerActive(false);
		}
	}
}

void AT66GameMode::ScheduleStandardStageCombatBootstrap(UWorld* World)
{
	if (!World)
	{
		return;
	}

	// Phase 2 is staggered across 4 ticks to eliminate the large startup hitch:
	// preload visuals -> spawn miasma systems -> spawn enemy director -> spawn boss/gate.
	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		PreloadStageCharacterVisuals();
	}));
}

void AT66GameMode::PreloadStageCharacterVisuals()
{
	const double PreloadStart = FPlatformTime::Seconds();

	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66CharacterVisualSubsystem* Visuals = GI ? GI->GetSubsystem<UT66CharacterVisualSubsystem>() : nullptr;

	if (T66GI && RunState && Visuals)
	{
		const int32 StageNum = RunState->GetCurrentStage();
		FStageData StageData;
		if (T66GI->GetStageData(StageNum, StageData))
		{
			// Preload mob visuals before the first enemy spawn to avoid sync-load hitches.
			if (!StageData.EnemyA.IsNone()) Visuals->PreloadCharacterVisual(StageData.EnemyA);
			if (!StageData.EnemyB.IsNone()) Visuals->PreloadCharacterVisual(StageData.EnemyB);
			if (!StageData.EnemyC.IsNone()) Visuals->PreloadCharacterVisual(StageData.EnemyC);
			Visuals->PreloadCharacterVisual(FName(TEXT("Boss")));

			UE_LOG(LogT66GameMode, Log, TEXT("[GOLD] Phase2-Preload: pre-resolved visuals for stage %d (EnemyA=%s, EnemyB=%s, EnemyC=%s, Boss=%s) in %.1fms"),
				StageNum,
				*StageData.EnemyA.ToString(), *StageData.EnemyB.ToString(), *StageData.EnemyC.ToString(),
				*StageData.BossID.ToString(),
				(FPlatformTime::Seconds() - PreloadStart) * 1000.0);
		}

		const FName FallbackA = FName(*FString::Printf(TEXT("Mob_Stage%02d_A"), RunState->GetCurrentStage()));
		const FName FallbackB = FName(*FString::Printf(TEXT("Mob_Stage%02d_B"), RunState->GetCurrentStage()));
		const FName FallbackC = FName(*FString::Printf(TEXT("Mob_Stage%02d_C"), RunState->GetCurrentStage()));
		Visuals->PreloadCharacterVisual(FallbackA);
		Visuals->PreloadCharacterVisual(FallbackB);
		Visuals->PreloadCharacterVisual(FallbackC);
	}

	UE_LOG(LogT66GameMode, Log, TEXT("[GOLD] Phase2-Preload: total preload time %.1fms"), (FPlatformTime::Seconds() - PreloadStart) * 1000.0);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			SpawnStageMiasmaSystems();
		}));
	}
}

void AT66GameMode::SpawnStageMiasmaSystems()
{
	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (!IsValid(MiasmaManager))
		{
			MiasmaManager = World->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}
		else
		{
			MiasmaManager->RebuildForCurrentStage();
		}

		if (IsUsingTowerMainMapLayout())
		{
			for (TActorIterator<AT66MiasmaBoundary> It(World); It; ++It)
			{
				if (AT66MiasmaBoundary* ExistingBoundary = *It)
				{
					ExistingBoundary->Destroy();
				}
			}
		}
		else
		{
			bool bHasBoundaryActor = false;
			for (TActorIterator<AT66MiasmaBoundary> It(World); It; ++It)
			{
				if (AT66MiasmaBoundary* ExistingBoundary = *It)
				{
					bHasBoundaryActor = true;
					break;
				}
			}

			if (!bHasBoundaryActor)
			{
				World->SpawnActor<AT66MiasmaBoundary>(AT66MiasmaBoundary::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			}
		}

		if (MiasmaManager)
		{
			MiasmaManager->UpdateFromRunState();
		}
	}

	UE_LOG(LogT66GameMode, Log, TEXT("[GOLD] Phase2-Tick2: %s."),
		IsUsingTowerMainMapLayout() ? TEXT("tower blood miasma manager ready") : TEXT("miasma systems spawned"));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			SpawnStageEnemyDirector();
		}));
	}
}

void AT66GameMode::SpawnStageEnemyDirector()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (UWorld* World = GetWorld())
	{
		World->SpawnActor<AT66EnemyDirector>(AT66EnemyDirector::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}

	UE_LOG(LogT66GameMode, Log, TEXT("[GOLD] Phase2-Tick3: enemy director spawned."));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			FinalizeStandardStageCombatBootstrap();
		}));
	}
}

void AT66GameMode::FinalizeStandardStageCombatBootstrap()
{
	SpawnBossForCurrentStage();
	if (IsUsingTowerMainMapLayout())
	{
		SpawnTowerDescentHolesIfNeeded();
		SyncTowerBossEntryState();
	}
	else
	{
		SpawnBossGateIfNeeded();
	}
	UE_LOG(LogT66GameMode, Log, TEXT("[GOLD] Phase2-Tick4: boss + traversal flow spawned. Phase 2 complete."));
	UE_LOG(LogT66GameMode, Log, TEXT("T66GameMode - Phase 2 content spawned (combat systems + boss)."));
}

void AT66GameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind from long-lived RunState delegates (RunState is a GameInstanceSubsystem).
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->StageTimerChanged.RemoveDynamic(this, &AT66GameMode::HandleStageTimerChanged);
			RunState->DifficultyChanged.RemoveDynamic(this, &AT66GameMode::HandleDifficultyChanged);
		}
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.RemoveDynamic(this, &AT66GameMode::HandleSettingsChanged);
		}
	}

	ActiveAsyncLoadHandles.Reset();
	PlayerCompanions.Reset();
	StageBoss = nullptr;
	IdolAltar = nullptr;
	BossBeaconActor = nullptr;
	BossBeaconUpdateAccumulator = 0.f;

	Super::EndPlay(EndPlayReason);
}

void AT66GameMode::SpawnTutorialIfNeeded()
{
	if (IsColiseumStage()) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!RunState || !T66GI) return;
	if (T66GI->bStageCatchUpPending) return;

	// v0 per your request: stage 1 always shows tutorial prompts.
	if (RunState->GetCurrentStage() != 1) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// The duplicated tutorial map should always run the tutorial flow.
	if (!T66IsStandaloneTutorialMap(World) && !bForceSpawnInTutorialArea) return;

	for (TActorIterator<AT66TutorialManager> It(World); It; ++It)
	{
		if (IsValid(*It))
		{
			return;
		}
	}

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AT66TutorialManager* M = World->SpawnActor<AT66TutorialManager>(AT66TutorialManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, P);
	if (M)
	{
		SpawnedSetupActors.Add(M);
	}
}

void AT66GameMode::SpawnStageEffectsForStage()
{
	if (IsColiseumStage() || IsUsingTowerMainMapLayout()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!RunState || !T66GI) return;
	if (T66GI->bStageCatchUpPending) return;

	const ET66Difficulty Difficulty = T66GI->SelectedDifficulty;
	const int32 StageNum = RunState->GetCurrentStage();

	const int32 RunSeed = (T66GI && T66GI->RunSeed != 0) ? T66GI->RunSeed : FMath::Rand();
	FRandomStream Rng(RunSeed + StageNum * 971 + 17);

	static constexpr float MainHalfExtent = 50000.f;
	static constexpr float SpawnZ = 40.f;
	static constexpr float SafeBubbleMargin = 350.f;

	int32 LavaPatchCount = 0;
	switch (Difficulty)
	{
	case ET66Difficulty::Easy:       LavaPatchCount = 4;  break;
	case ET66Difficulty::Medium:     LavaPatchCount = 5;  break;
	case ET66Difficulty::Hard:       LavaPatchCount = 6;  break;
	case ET66Difficulty::VeryHard:   LavaPatchCount = 7;  break;
	case ET66Difficulty::Impossible: LavaPatchCount = 8;  break;
	default:                         LavaPatchCount = 5;  break;
	}

	const int32 ShroomCount = (Difficulty == ET66Difficulty::Easy) ? 12 : 0;
	if (LavaPatchCount <= 0 && ShroomCount <= 0) return;

	struct FUsedStageEffectLoc
	{
		FVector Loc = FVector::ZeroVector;
		float ExclusionRadius = 0.f;
	};
	TArray<FUsedStageEffectLoc> UsedLocs;

	auto IsInsideNoSpawnZone = [&](const FVector& L) -> bool
	{
		if (T66GameplayLayout::IsInsideReservedTraversalZone2D(L, 455.f))
		{
			return true;
		}

		static constexpr float ArenaHalf = 9091.f;
		static constexpr float ArenaMargin = 682.f;
		static constexpr float TutorialArenaHalf = 9091.f;
		struct FArena2D { float X; float Y; float Half; };
		static constexpr FArena2D Arenas[] = {
			{ -22727.f, 34091.f, ArenaHalf }, // Catch Up
			{      0.f, 34091.f, ArenaHalf },  // Coliseum
			{      0.f, 61364.f, TutorialArenaHalf }, // Tutorial
		};
		for (const FArena2D& A : Arenas)
		{
			if (FMath::Abs(L.X - A.X) <= (A.Half + ArenaMargin) && FMath::Abs(L.Y - A.Y) <= (A.Half + ArenaMargin))
			{
				return true;
			}
		}

		return false;
	};

	auto IsGoodLoc = [&](const FVector& L, float CandidateRadius) -> bool
	{
		if (IsInsideNoSpawnZone(L)) return false;
		for (const FUsedStageEffectLoc& Used : UsedLocs)
		{
			const float RequiredRadius = FMath::Max(CandidateRadius, Used.ExclusionRadius);
			if (FVector::DistSquared2D(L, Used.Loc) < (RequiredRadius * RequiredRadius)) return false;
		}
		UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		const TArray<TWeakObjectPtr<AT66HouseNPCBase>>& NPCs = Registry ? Registry->GetNPCs() : TArray<TWeakObjectPtr<AT66HouseNPCBase>>();
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : NPCs)
		{
			const AT66HouseNPCBase* NPC = WeakNPC.Get();
			if (!NPC) continue;
			const float R = NPC->GetSafeZoneRadius() + SafeBubbleMargin + CandidateRadius * 0.35f;
			if (FVector::DistSquared2D(L, NPC->GetActorLocation()) < (R * R)) return false;
		}
		const TArray<TWeakObjectPtr<AT66CircusInteractable>>& Circuses = Registry ? Registry->GetCircuses() : TArray<TWeakObjectPtr<AT66CircusInteractable>>();
		for (const TWeakObjectPtr<AT66CircusInteractable>& WeakCircus : Circuses)
		{
			const AT66CircusInteractable* Circus = WeakCircus.Get();
			if (!Circus) continue;
			const float R = Circus->GetSafeZoneRadius() + SafeBubbleMargin + CandidateRadius * 0.35f;
			if (FVector::DistSquared2D(L, Circus->GetActorLocation()) < (R * R)) return false;
		}
		return true;
	};

	struct FStageEffectSpawnHit
	{
		FVector Loc = FVector::ZeroVector;
		FVector Normal = FVector::UpVector;
		bool bFound = false;
	};

	auto FindSpawnLoc = [&](float CandidateRadius, int32 MaxTries) -> FStageEffectSpawnHit
	{
		const FVector Up = FVector::UpVector;
		for (int32 Try = 0; Try < MaxTries; ++Try)
		{
			const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			FVector Loc(X, Y, SpawnZ);
			FVector Normal = Up;

			FHitResult Hit;
			const FVector Start = Loc + FVector(0.f, 0.f, 2000.f);
			const FVector End = Loc - FVector(0.f, 0.f, 6000.f);
			if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
			{
				continue;
			}
			if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
			{
				continue;
			}

			Loc = Hit.ImpactPoint;
			Normal = Hit.ImpactNormal.GetSafeNormal(1e-4f, Up);

			if (IsGoodLoc(Loc, CandidateRadius))
			{
				return { Loc, Normal, true };
			}
		}
		return {};
	};

	auto NoteUsedLoc = [&](const FVector& Loc, float Radius)
	{
		UsedLocs.Add({ Loc, Radius });
	};

	auto GetLavaDamagePerTick = [&]() -> int32
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Easy:       return 6;
		case ET66Difficulty::Medium:     return 8;
		case ET66Difficulty::Hard:       return 10;
		case ET66Difficulty::VeryHard:   return 12;
		case ET66Difficulty::Impossible: return 14;
		default:                         return 8;
		}
	};

	auto ConfigureLavaPatch = [&](AT66LavaPatch* Lava)
	{
		if (!Lava) return;

		FVector2D Flow(Rng.FRandRange(-1.f, 1.f), Rng.FRandRange(-1.f, 1.f));
		if (Flow.IsNearlyZero())
		{
			Flow = FVector2D(1.f, 0.f);
		}
		else
		{
			Flow.Normalize();
		}

		Lava->PatchSize = Rng.FRandRange(1400.f, 2400.f);
		Lava->HoverHeight = 1.5f;
		Lava->CollisionHeight = 160.f;
		Lava->TextureResolution = 64;
		Lava->AnimationFrames = 18;
		Lava->AnimationFPS = 12.f;
		Lava->WarpSpeed = Rng.FRandRange(0.90f, 1.25f);
		Lava->WarpIntensity = Rng.FRandRange(0.10f, 0.15f);
		Lava->WarpCloseness = Rng.FRandRange(1.80f, 2.40f);
		Lava->FlowDir = Flow;
		Lava->FlowSpeed = Rng.FRandRange(0.12f, 0.26f);
		Lava->UVScale = Rng.FRandRange(3.60f, 5.40f);
		Lava->CellDensity = Rng.FRandRange(5.20f, 7.40f);
		Lava->EdgeContrast = Rng.FRandRange(6.40f, 8.80f);
		Lava->Brightness = Rng.FRandRange(2.10f, 2.90f);
		Lava->PatternOffset = FVector2D(Rng.FRandRange(-12.f, 12.f), Rng.FRandRange(-12.f, 12.f));
		Lava->DamagePerTick = GetLavaDamagePerTick();
		Lava->DamageIntervalSeconds = 0.45f;
		Lava->bDamageHero = true;
	};

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	int32 SpawnedLavaCount = 0;
	int32 SpawnedShroomCount = 0;

	static constexpr float LavaExclusionRadius = 1800.f;
	for (int32 i = 0; i < LavaPatchCount; ++i)
	{
		const FStageEffectSpawnHit Th = FindSpawnLoc(LavaExclusionRadius, 120);
		if (!Th.bFound)
		{
			continue;
		}

		const FRotator LavaRot(0.f, Rng.FRandRange(0.f, 360.f), 0.f);
		AT66LavaPatch* Lava = World->SpawnActorDeferred<AT66LavaPatch>(
			AT66LavaPatch::StaticClass(),
			FTransform(LavaRot, Th.Loc),
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Lava)
		{
			ConfigureLavaPatch(Lava);
			Lava->FinishSpawning(FTransform(LavaRot, Th.Loc));
			NoteUsedLoc(Th.Loc, LavaExclusionRadius);
			++SpawnedLavaCount;
		}
	}

	static constexpr float ShroomExclusionRadius = 420.f;
	for (int32 i = 0; i < ShroomCount; ++i)
	{
		const FStageEffectSpawnHit Th = FindSpawnLoc(ShroomExclusionRadius, 80);
		if (!Th.bFound)
		{
			continue;
		}

		AT66Shroom* Shroom = World->SpawnActor<AT66Shroom>(AT66Shroom::StaticClass(), Th.Loc, FRotator::ZeroRotator, P);
		if (Shroom)
		{
			NoteUsedLoc(Th.Loc, ShroomExclusionRadius);
			++SpawnedShroomCount;
		}
	}

	UE_LOG(LogT66GameMode, Log,
		TEXT("[StageEffects] Spawned %d lava patches and %d shrooms for stage %d (diff=%d)."),
		SpawnedLavaCount,
		SpawnedShroomCount,
		StageNum,
		static_cast<int32>(Difficulty));
}

void AT66GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	MaintainPlayerTerrainSafety();
	TryActivateMainMapCombat();
	SyncTowerBossEntryState();

	BossBeaconUpdateAccumulator += DeltaTime;
	if (BossBeaconUpdateAccumulator >= 0.10f)
	{
		BossBeaconUpdateAccumulator = 0.f;
		UpdateBossBeaconTransform(true);
	}

	// Frame-level lag: log when a frame exceeded budget (e.g. >20ms).
	if (DeltaTime > 0.02f)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66LagTrackerSubsystem* Lag = GI->GetSubsystem<UT66LagTrackerSubsystem>())
			{
				if (Lag->IsEnabled())
				{
					UE_LOG(LogT66GameMode, Verbose, TEXT("[LAG] Frame: %.2fms (%.1f FPS)"), DeltaTime * 1000.f, 1.f / FMath::Max(0.001f, DeltaTime));
				}
			}
		}
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->TickStageTimer(DeltaTime);
			RunState->TickSpeedRunTimer(DeltaTime);
			RunState->TickHeroTimers(DeltaTime);
			TickFinalDifficultySurvival(DeltaTime);

			// Skill Rating: time is driven here; damage is an input event from RunState.
			if (UT66SkillRatingSubsystem* Skill = GI->GetSubsystem<UT66SkillRatingSubsystem>())
			{
				// Track only during combat time (stage timer active) and not during last-stand invulnerability.
				Skill->SetTrackingActive(RunState->GetStageTimerActive() && !RunState->IsInLastStand());
				Skill->TickSkillRating(DeltaTime);
			}

			// Robust: if the timer is already active (even if we missed the delegate),
			// try spawning the LoanShark when pending.
			if (!IsColiseumStage() && !IsLabLevel() && RunState->GetStageTimerActive())
			{
				TrySpawnLoanSharkIfNeeded();
			}

			// Despawn loan shark immediately once debt is paid.
			if (LoanShark && RunState->GetCurrentDebt() <= 0)
			{
				LoanShark->Destroy();
				LoanShark = nullptr;
			}
		}
	}
}

void AT66GameMode::HandleStageTimerChanged()
{
	if (IsLabLevel()) return;
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	FLagScopedScope LagScope(GetWorld(), TEXT("HandleStageTimerChanged (Miasma+LoanShark)"));

	// Perf: miasma updates are event-driven (StageTimerChanged broadcasts at most once per second).
	if (MiasmaManager)
	{
		MiasmaManager->UpdateFromRunState();
	}

	if (!IsColiseumStage() && RunState->GetStageTimerActive())
	{
		TrySpawnLoanSharkIfNeeded();
	}
}

void AT66GameMode::HandleDifficultyChanged()
{
	if (IsLabLevel()) return;
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	const int32 Stage = RunState->GetCurrentStage();
	const float Scalar = RunState->GetDifficultyScalar();
	const float FinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
	UT66ActorRegistrySubsystem* Registry = GetWorld() ? GetWorld()->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
	const TArray<TWeakObjectPtr<AT66EnemyBase>>& Enemies = Registry ? Registry->GetEnemies() : TArray<TWeakObjectPtr<AT66EnemyBase>>();
	for (const TWeakObjectPtr<AT66EnemyBase>& WeakEnemy : Enemies)
	{
		if (AT66EnemyBase* E = WeakEnemy.Get())
		{
			E->ApplyStageScaling(Stage);
			E->ApplyDifficultyScalar(Scalar);
			E->ApplyFinaleScaling(FinaleScalar);
		}
	}

	// Bosses: scale HP + projectile damage (count unchanged).
	const TArray<TWeakObjectPtr<AT66BossBase>>& Bosses = Registry ? Registry->GetBosses() : TArray<TWeakObjectPtr<AT66BossBase>>();
	for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Bosses)
	{
		if (AT66BossBase* B = WeakBoss.Get())
		{
			B->ApplyDifficultyScalar(Scalar);
		}
	}
}

void AT66GameMode::TrySpawnLoanSharkIfNeeded()
{
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	// Only spawn if the previous stage was exited with debt.
	if (!RunState->GetLoanSharkPending())
	{
		return;
	}
	if (RunState->GetCurrentDebt() <= 0)
	{
		RunState->SetLoanSharkPending(false);
		return;
	}
	if (LoanShark)
	{
		RunState->SetLoanSharkPending(false);
		return;
	}

	APawn* PlayerPawn = nullptr;
	if (UWorld* SearchWorld = GetWorld())
	{
		for (FConstPlayerControllerIterator It = SearchWorld->GetPlayerControllerIterator(); It; ++It)
		{
			PlayerPawn = It->Get() ? It->Get()->GetPawn() : nullptr;
			if (PlayerPawn)
			{
				break;
			}
		}
	}
	if (!PlayerPawn) return;
	UWorld* World = GetWorld();
	if (!World) return;

	FVector PlayerLoc = PlayerPawn->GetActorLocation();
	FVector SpawnLoc = PlayerLoc + FVector(-1200.f, 0.f, 0.f);

	// Try a few times to avoid spawning inside safe zones
	for (int32 Try = 0; Try < 10; ++Try)
	{
		float Angle = FMath::RandRange(0.f, 2.f * PI);
		float Dist = FMath::RandRange(900.f, 1400.f);
		FVector Offset(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
		SpawnLoc = PlayerLoc + Offset;

		bool bInSafe = false;
		UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		const TArray<TWeakObjectPtr<AT66HouseNPCBase>>& NPCs = Registry ? Registry->GetNPCs() : TArray<TWeakObjectPtr<AT66HouseNPCBase>>();
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : NPCs)
		{
			AT66HouseNPCBase* NPC = WeakNPC.Get();
			if (!NPC) continue;
			const float R = NPC->GetSafeZoneRadius();
			if (FVector::DistSquared2D(SpawnLoc, NPC->GetActorLocation()) < (R * R))
			{
				bInSafe = true;
				break;
			}
		}
		if (!bInSafe)
		{
			const TArray<TWeakObjectPtr<AT66CircusInteractable>>& Circuses = Registry ? Registry->GetCircuses() : TArray<TWeakObjectPtr<AT66CircusInteractable>>();
			for (const TWeakObjectPtr<AT66CircusInteractable>& WeakCircus : Circuses)
			{
				AT66CircusInteractable* Circus = WeakCircus.Get();
				if (!Circus) continue;
				const float R = Circus->GetSafeZoneRadius();
				if (FVector::DistSquared2D(SpawnLoc, Circus->GetActorLocation()) < (R * R))
				{
					bInSafe = true;
					break;
				}
			}
		}
		if (!bInSafe)
		{
			break;
		}
	}

	// Trace down for ground
	FHitResult Hit;
	FVector Start = SpawnLoc + FVector(0.f, 0.f, 500.f);
	FVector End = SpawnLoc - FVector(0.f, 0.f, 1000.f);
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
	{
		SpawnLoc = Hit.ImpactPoint + FVector(0.f, 0.f, 90.f);
	}
	else
	{
		SpawnLoc.Z = PlayerLoc.Z;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	LoanShark = World->SpawnActor<AT66LoanShark>(AT66LoanShark::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	RunState->SetLoanSharkPending(false);
}

void AT66GameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
	SpawnCompanionForPlayer(NewPlayer);
	// Start gate is spawned in SpawnLevelContentAfterLandscapeReady (after floor exists) so its ground trace hits the floor.

	UT66GameInstance* GI = GetT66GameInstance();
	APawn* Pawn = NewPlayer ? NewPlayer->GetPawn() : nullptr;
	if (GI && Pawn && GI->bApplyLoadedTransform)
	{
		const FRotator LoadedRotation = GI->PendingLoadedTransform.Rotator();
		Pawn->SetActorTransform(GI->PendingLoadedTransform);
		T66SyncPawnAndControllerRotation(Pawn, NewPlayer, LoadedRotation);
		GI->bApplyLoadedTransform = false;
		GI->PendingLoadedTransform = FTransform();
	}

	if (GI && NewPlayer)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GameInstance->GetSubsystem<UT66RunStateSubsystem>())
			{
				if (RunState->HasPendingDifficultyClearSummary())
				{
					if (AT66PlayerController* PC = Cast<AT66PlayerController>(NewPlayer))
					{
						PC->ShowDifficultyClearSummary();
					}
				}
			}
		}
	}

	MaintainPlayerTerrainSafety();

}

void AT66GameMode::SpawnBossGateIfNeeded()
{
	if (IsColiseumStage() || IsUsingTowerMainMapLayout()) return;
	if (BossGate) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Trigger right at the boss-area threshold. The visible pillars are hidden so the fight starts on entry.
	FVector BossGateLoc = T66GameplayLayout::GetBossGateLocation();
	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, BossGateLoc + FVector(0.f, 0.f, 3000.f), BossGateLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
	{
		BossGateLoc.Z = Hit.ImpactPoint.Z;
	}
	BossGate = World->SpawnActor<AT66BossGate>(AT66BossGate::StaticClass(), BossGateLoc, FRotator::ZeroRotator, SpawnParams);
	if (BossGate)
	{
		BossGate->TriggerDistance2D = 220.f;
		if (BossGate->TriggerBox)
		{
			BossGate->TriggerBox->SetBoxExtent(FVector(120.f, T66GameplayLayout::CorridorHalfHeightY * 0.92f, 220.f));
		}
		if (BossGate->PoleLeft)
		{
			BossGate->PoleLeft->SetVisibility(false, true);
			BossGate->PoleLeft->SetHiddenInGame(true, true);
		}
		if (BossGate->PoleRight)
		{
			BossGate->PoleRight->SetVisibility(false, true);
			BossGate->PoleRight->SetHiddenInGame(true, true);
		}
	}
}

bool AT66GameMode::IsColiseumStage() const
{
	if (T66IsStandaloneColiseumMap(GetWorld()))
	{
		return true;
	}

	if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->bForceColiseumMode || GI->ColiseumFlowMode != ET66ColiseumFlowMode::None;
	}
	return false;
}

bool AT66GameMode::IsFinalDifficultyBossColiseumStage() const
{
	if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->ColiseumFlowMode == ET66ColiseumFlowMode::FinalDifficultyBoss;
	}

	return false;
}

bool AT66GameMode::IsLabLevel() const
{
	if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->bIsLabLevel;
	}
	return false;
}

void AT66GameMode::SpawnAllOwedBossesInColiseum()
{
	// Spawn ALL owed bosses at once. No tick polling: each boss death calls HandleBossDefeatedAtLocation.
	UWorld* World = GetWorld();
	if (!World) return;
	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return;

	if (bColiseumExitGateSpawned)
	{
		// Coliseum already cleared this session.
		return;
	}

	const TArray<FName>& Owed = RunState->GetOwedBossIDs();
	if (Owed.Num() <= 0)
	{
		// Nothing owed: exit gate immediately.
		bColiseumExitGateSpawned = true;
		SpawnStageGateAtLocation(ColiseumCenter);
		return;
	}

	// Preload any BossClass soft references asynchronously to avoid hitches.
	// If a load fails/misses, we still spawn with AT66BossBase as a fallback.
	if (!bColiseumBossesAsyncLoadAttempted && !bColiseumBossesAsyncLoadInFlight)
	{
		TArray<FSoftObjectPath> PathsToLoad;
		PathsToLoad.Reserve(Owed.Num());
		for (const FName& BossID : Owed)
		{
			if (BossID.IsNone()) continue;

			FBossData BossData;
			if (T66GI->GetBossData(BossID, BossData))
			{
				if (!BossData.BossClass.IsNull())
				{
					// Only request async load if the class isn't already resident.
					if (!BossData.BossClass.Get())
					{
						PathsToLoad.AddUnique(BossData.BossClass.ToSoftObjectPath());
					}
				}
			}
		}

		if (PathsToLoad.Num() > 0)
		{
			bColiseumBossesAsyncLoadAttempted = true;
			bColiseumBossesAsyncLoadInFlight = true;

			TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
				PathsToLoad,
				FStreamableDelegate::CreateWeakLambda(this, [this]()
				{
					bColiseumBossesAsyncLoadInFlight = false;
					SpawnAllOwedBossesInColiseum();
				}));
			if (Handle.IsValid())
			{
				ActiveAsyncLoadHandles.Add(Handle);
				return;
			}
			// If async load couldn't start, fall through to immediate spawning (with fallbacks).
			bColiseumBossesAsyncLoadInFlight = false;
		}
	}

	// Reset and spawn.
	ColiseumBossesRemaining = 0;

	// Simple ring layout around ColiseumCenter.
	const int32 N = FMath::Clamp(Owed.Num(), 1, 12);
	const float Radius = 900.f + (static_cast<float>(N) * 55.f);
	const float AngleStep = (2.f * PI) / static_cast<float>(N);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < Owed.Num(); ++i)
	{
		const FName BossID = Owed[i];
		if (BossID.IsNone()) continue;

		FBossData BossData;
		if (!T66GI->GetBossData(BossID, BossData))
		{
			// Fallback boss tuning if DT row missing.
			BossData.BossID = BossID;
			BossData.MaxHP = 6000;
			BossData.AwakenDistance = 999999.f;
			BossData.MoveSpeed = 380.f;
			BossData.FireIntervalSeconds = 1.6f;
			BossData.ProjectileSpeed = 1100.f;
			BossData.ProjectileDamageHearts = 2;
			BossData.PlaceholderColor = FLinearColor(0.85f, 0.15f, 0.15f, 1.f);
		}

		UClass* BossClass = AT66BossBase::StaticClass();
		if (!BossData.BossClass.IsNull())
		{
			if (UClass* Loaded = BossData.BossClass.Get())
			{
				if (Loaded->IsChildOf(AT66BossBase::StaticClass()))
				{
					BossClass = Loaded;
				}
			}
		}

		const float A = static_cast<float>(i) * AngleStep;
		FVector SpawnLoc = ColiseumCenter + FVector(FMath::Cos(A) * Radius, FMath::Sin(A) * Radius, 0.f);

		// Trace down to ground.
		FHitResult Hit;
		const FVector Start = SpawnLoc + FVector(0.f, 0.f, 4000.f);
		const FVector End = SpawnLoc - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			SpawnLoc = Hit.ImpactPoint + FVector(0.f, 0.f, 200.f);
		}

		AActor* Spawned = World->SpawnActor<AActor>(BossClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (AT66BossBase* Boss = Cast<AT66BossBase>(Spawned))
		{
			Boss->InitializeBoss(BossData);
			Boss->ForceAwaken();
			ColiseumBossesRemaining++;
		}
	}

	// If something went wrong and none spawned, still provide an exit.
	if (ColiseumBossesRemaining <= 0)
	{
		bColiseumExitGateSpawned = true;
		RunState->ClearOwedBosses();
		SpawnStageGateAtLocation(ColiseumCenter);
	}
}

void AT66GameMode::SpawnFinalDifficultyBossInColiseum()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return;

	const int32 StageNum = RunState->GetCurrentStage();

	FStageData StageData;
	StageData.StageNumber = StageNum;
	StageData.BossID = FName(*FString::Printf(TEXT("Boss_%02d"), StageNum));
	if (FStageData FromDT; T66GI->GetStageData(StageNum, FromDT))
	{
		StageData = FromDT;
	}

	FBossData BossData;
	BossData.BossID = StageData.BossID;
	BossData.MaxHP = 1000 + (StageNum * 250);
	BossData.AwakenDistance = 999999.f;
	BossData.MoveSpeed = 350.f + (StageNum * 2.f);
	BossData.FireIntervalSeconds = FMath::Clamp(2.0f - (StageNum * 0.015f), 0.65f, 3.5f);
	BossData.ProjectileSpeed = 900.f + (StageNum * 15.f);
	BossData.ProjectileDamageHearts = 1 + (StageNum / 20);
	BossData.PlaceholderColor = FLinearColor(0.95f, 0.18f, 0.10f, 1.f);

	if (FBossData FromBossDT; !StageData.BossID.IsNone() && T66GI->GetBossData(StageData.BossID, FromBossDT))
	{
		BossData = FromBossDT;
		BossData.AwakenDistance = 999999.f;
	}

	UClass* BossClass = AT66BossBase::StaticClass();
	if (!BossData.BossClass.IsNull())
	{
		if (UClass* LoadedClass = BossData.BossClass.LoadSynchronous())
		{
			if (LoadedClass->IsChildOf(AT66BossBase::StaticClass()))
			{
				BossClass = LoadedClass;
			}
		}
	}

	FVector SpawnLoc = ColiseumCenter;
	FHitResult Hit;
	const FVector TraceStart = SpawnLoc + FVector(0.f, 0.f, 4000.f);
	const FVector TraceEnd = SpawnLoc - FVector(0.f, 0.f, 9000.f);
	if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
	{
		SpawnLoc = Hit.ImpactPoint + FVector(0.f, 0.f, 150.f);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* Spawned = World->SpawnActor<AActor>(BossClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (AT66BossBase* Boss = Cast<AT66BossBase>(Spawned))
	{
		Boss->InitializeBoss(BossData);
		Boss->ForceAwaken();
		StageBoss = Boss;
		ColiseumBossesRemaining = 1;
		UE_LOG(LogT66GameMode, Log, TEXT("Spawned final-difficulty Coliseum boss for Stage %d (BossID=%s)"), StageNum, *BossData.BossID.ToString());
	}
}

void AT66GameMode::SpawnFinalDifficultyTotem(const FVector& SpawnLocation)
{
	if (FinalDifficultyTotemActor.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector GroundedLocation = SpawnLocation + FVector(420.f, 0.f, 0.f);
	FHitResult Hit;
	const FVector TraceStart = GroundedLocation + FVector(0.f, 0.f, 3000.f);
	const FVector TraceEnd = GroundedLocation - FVector(0.f, 0.f, 9000.f);
	if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
	{
		GroundedLocation = Hit.ImpactPoint;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* TotemActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), GroundedLocation, FRotator::ZeroRotator, SpawnParams);
	if (!TotemActor)
	{
		return;
	}

	if (UStaticMeshComponent* SMC = TotemActor->GetStaticMeshComponent())
	{
		TSoftObjectPtr<UStaticMesh> TotemMesh(FSoftObjectPath(TEXT("/Game/World/Interactables/Totem.Totem")));
		if (UStaticMesh* LoadedMesh = TotemMesh.LoadSynchronous())
		{
			SMC->SetStaticMesh(LoadedMesh);
			SMC->SetRelativeScale3D(FVector(2.4f, 2.4f, 4.2f));
		}
		else if (UStaticMesh* CubeMesh = GetCubeMesh())
		{
			SMC->SetStaticMesh(CubeMesh);
			SMC->SetRelativeScale3D(FVector(0.8f, 0.8f, 10.f));
		}

		if (UMaterialInstanceDynamic* Mat = SMC->CreateAndSetMaterialInstanceDynamic(0))
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.86f, 0.72f, 0.18f, 1.f));
		}
	}

	FinalDifficultyTotemActor = TotemActor;
}

void AT66GameMode::SpawnFinalDifficultySaint(const FVector& SpawnLocation)
{
	if (FinalDifficultySaintActor.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector GroundedLocation = SpawnLocation + FVector(-420.f, 0.f, 0.f);
	FHitResult Hit;
	const FVector TraceStart = GroundedLocation + FVector(0.f, 0.f, 3000.f);
	const FVector TraceEnd = GroundedLocation - FVector(0.f, 0.f, 9000.f);
	if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
	{
		GroundedLocation = Hit.ImpactPoint + FVector(0.f, 0.f, 5.f);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66SaintNPC* Saint = World->SpawnActor<AT66SaintNPC>(AT66SaintNPC::StaticClass(), GroundedLocation, FRotator::ZeroRotator, SpawnParams))
	{
		if (Saint->InteractionSphere)
		{
			Saint->InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		FinalDifficultySaintActor = Saint;
	}
}

void AT66GameMode::BeginFinalDifficultySurvival(const FVector& BossDeathLocation)
{
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	bFinalDifficultySurvivalActive = true;
	FinalDifficultySurvivalElapsedSeconds = 0.f;
	LastAppliedFinalDifficultyEnemyScalar = 1.f;

	RunState->BeginSaintBlessingEmpowerment();
	RunState->SetBossInactive();
	RunState->SetPendingDifficultyClearSummary(false);
	RunState->SetSaintBlessingActive(true);
	RunState->SetFinalSurvivalEnemyScalar(1.f);
	RunState->SetStageTimerActive(true);

	ClearMiasma();
	SpawnFinalDifficultyTotem(BossDeathLocation);
	SpawnFinalDifficultySaint(BossDeathLocation);

	bool bHasEnemyDirector = false;
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AT66EnemyDirector> It(World); It; ++It)
		{
			if (AT66EnemyDirector* ExistingDirector = *It)
			{
				ExistingDirector->SetSpawningPaused(false);
				bHasEnemyDirector = true;
			}
		}

		if (!bHasEnemyDirector)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			World->SpawnActor<AT66EnemyDirector>(AT66EnemyDirector::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}
	}

	UpdateFinalDifficultySurvivalScaling(true);
}

void AT66GameMode::UpdateFinalDifficultySurvivalScaling(const bool bForce)
{
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	const float Alpha = FMath::Clamp(FinalDifficultySurvivalElapsedSeconds / T66FinalDifficultySurvivalDurationSeconds, 0.f, 1.f);
	const float NewScalar = FMath::Clamp(FMath::Pow(6.0f, Alpha), 1.f, 99.f);
	RunState->SetFinalSurvivalEnemyScalar(NewScalar);

	if (!bForce && FMath::IsNearlyEqual(NewScalar, LastAppliedFinalDifficultyEnemyScalar, 0.05f))
	{
		return;
	}

	LastAppliedFinalDifficultyEnemyScalar = NewScalar;

	UWorld* World = GetWorld();
	UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
	if (!Registry)
	{
		return;
	}

	const int32 Stage = RunState->GetCurrentStage();
	const float BaseDifficultyScalar = RunState->GetDifficultyScalar();
	for (const TWeakObjectPtr<AT66EnemyBase>& WeakEnemy : Registry->GetEnemies())
	{
		if (AT66EnemyBase* Enemy = WeakEnemy.Get())
		{
			Enemy->ApplyStageScaling(Stage);
			Enemy->ApplyDifficultyScalar(BaseDifficultyScalar);
			Enemy->ApplyFinaleScaling(NewScalar);
		}
	}
}

void AT66GameMode::SpawnTowerDescentHolesIfNeeded()
{
	if (!IsUsingTowerMainMapLayout())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || TowerDescentHoles.Num() > 0)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const T66TowerMapTerrain::FFloor& Floor : CachedTowerMainMapLayout.Floors)
	{
		if (!Floor.bHasDropHole)
		{
			continue;
		}

		const T66TowerMapTerrain::FFloor* DestinationFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& CandidateFloor : CachedTowerMainMapLayout.Floors)
		{
			if (CandidateFloor.FloorNumber == (Floor.FloorNumber + 1))
			{
				DestinationFloor = &CandidateFloor;
				break;
			}
		}
		if (!DestinationFloor)
		{
			continue;
		}

		const float DropHeight = FMath::Max(Floor.SurfaceZ - DestinationFloor->SurfaceZ, 1200.0f);
		const float VerticalExtent = FMath::Clamp((DropHeight * 0.5f) - 550.0f, 800.0f, 1400.0f);
		const FVector BoxExtent(
			FMath::Max(250.0f, Floor.HoleHalfExtent.X * 0.88f),
			FMath::Max(250.0f, Floor.HoleHalfExtent.Y * 0.88f),
			VerticalExtent);
		const FVector HoleLocation = Floor.HoleCenter + FVector(0.0f, 0.0f, -VerticalExtent + 120.0f);

		AT66TowerDescentHole* HoleActor = World->SpawnActor<AT66TowerDescentHole>(
			AT66TowerDescentHole::StaticClass(),
			HoleLocation,
			FRotator::ZeroRotator,
			SpawnParams);
		if (!HoleActor)
		{
			continue;
		}

		HoleActor->InitializeHole(Floor.FloorNumber, DestinationFloor->FloorNumber, BoxExtent);
		HoleActor->Tags.AddUnique(FName(TEXT("T66_Tower_DescentHole")));
		TowerDescentHoles.Add(HoleActor);
	}
}

void AT66GameMode::TickFinalDifficultySurvival(float DeltaTime)
{
	if (!bFinalDifficultySurvivalActive)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	FinalDifficultySurvivalElapsedSeconds = FMath::Min(T66FinalDifficultySurvivalDurationSeconds, FinalDifficultySurvivalElapsedSeconds + FMath::Max(0.f, DeltaTime));
	UpdateFinalDifficultySurvivalScaling();

	if (FinalDifficultySurvivalElapsedSeconds < T66FinalDifficultySurvivalDurationSeconds)
	{
		return;
	}

	bFinalDifficultySurvivalActive = false;
	RunState->EndSaintBlessingEmpowerment();
	RunState->SetSaintBlessingActive(false);
	RunState->SetFinalSurvivalEnemyScalar(1.f);
	RunState->SetPendingDifficultyClearSummary(true);
	RunState->SetStageTimerActive(false);

	UWorld* World = GetWorld();
	if (World)
	{
		const FVector ExitLocation = ColiseumCenter + FVector(0.f, 0.f, 200.f);
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (APawn* Pawn = It->Get() ? It->Get()->GetPawn() : nullptr)
			{
				Pawn->SetActorLocation(ExitLocation, false, nullptr, ETeleportType::TeleportPhysics);
			}
		}
	}

	if (UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr)
	{
		if (const UT66GameInstance* T66GI = GetT66GameInstance())
		{
			if (!T66GI->SelectedHeroID.IsNone())
			{
				Achievements->RecordHeroDifficultyClear(T66GI->SelectedHeroID, T66GI->SelectedDifficulty);
			}
			if (!T66GI->SelectedCompanionID.IsNone())
			{
				Achievements->RecordCompanionDifficultyClear(T66GI->SelectedCompanionID, T66GI->SelectedDifficulty);
			}
		}
	}

	if (UT66GameInstance* T66GI = GetT66GameInstance())
	{
		T66GI->bForceColiseumMode = false;
		T66GI->ColiseumFlowMode = ET66ColiseumFlowMode::None;
	}

	bool bOpenedSummary = false;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(It->Get()))
		{
			T66PC->ClientShowDifficultyClearSummary();
			bOpenedSummary = true;
		}
	}

	if (!bOpenedSummary)
	{
		UE_LOG(LogT66GameMode, Warning, TEXT("Difficulty clear reached but no player controllers were available to open Run Summary."));
	}
}


void AT66GameMode::SpawnTricksterAndCowardiceGate()
{
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	const int32 StageNum = RunState->GetCurrentStage();
	// Only on non-boss stages. Stage 23 is also a difficulty boss.
	if (T66_IsDifficultyBossStage(StageNum)) return;

	UWorld* World = GetWorld();
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector GateLoc = T66GameplayLayout::GetCowardiceGateLocation();
	FVector TricksterLoc = T66GameplayLayout::GetTricksterLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	if (CowardiceGate)
	{
		CowardiceGate->Destroy();
		CowardiceGate = nullptr;
	}
	if (TricksterNPC)
	{
		TricksterNPC->Destroy();
		TricksterNPC = nullptr;
	}

	if (T66UsesMainMapTerrainStage(World) && !MainMapBossAnchorSurfaceLocation.IsNearlyZero())
	{
		const FVector SideOffset = IsUsingTowerMainMapLayout()
			? FVector(260.f, 0.f, 0.f)
			: FVector(T66MainMapTerrain::MakeSettings(T66BuildMainMapPreset(T66GI)).CellSize * 0.28f, 0.f, 0.f);
		GateLoc = MainMapBossAnchorSurfaceLocation;
		TricksterLoc = MainMapBossAnchorSurfaceLocation + SideOffset;
		SpawnRotation = FRotator(0.f, 180.f, 0.f);
	}
	else
	{
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, GateLoc + FVector(0.f, 0.f, 3000.f), GateLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
		{
			GateLoc.Z = Hit.ImpactPoint.Z;
		}
		if (World->LineTraceSingleByChannel(Hit, TricksterLoc + FVector(0.f, 0.f, 3000.f), TricksterLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
		{
			TricksterLoc.Z = Hit.ImpactPoint.Z;
		}
	}

	CowardiceGate = World->SpawnActor<AT66CowardiceGate>(AT66CowardiceGate::StaticClass(), GateLoc, SpawnRotation, SpawnParams);
	if (CowardiceGate)
	{
		if (IsUsingTowerMainMapLayout())
		{
			T66TrySnapActorToTowerFloor(World, CowardiceGate, CachedTowerMainMapLayout, CachedTowerMainMapLayout.LastGameplayFloorNumber, GateLoc);
			T66AssignTowerFloorTag(CowardiceGate, CachedTowerMainMapLayout.LastGameplayFloorNumber);
		}
		else
		{
			TrySnapActorToTerrain(CowardiceGate);
		}
	}
	if (!IsUsingTowerMainMapLayout())
	{
		TricksterNPC = World->SpawnActor<AT66TricksterNPC>(AT66TricksterNPC::StaticClass(), TricksterLoc, SpawnRotation, SpawnParams);
		if (TricksterNPC)
		{
			TricksterNPC->ApplyVisuals();
			TrySnapActorToTerrain(TricksterNPC);
		}
	}
}

bool AT66GameMode::IsUsingTowerMainMapLayout() const
{
	return bUsingTowerMainMapLayout && CachedTowerMainMapLayout.Floors.Num() > 0;
}

bool AT66GameMode::GetTowerMainMapLayout(T66TowerMapTerrain::FLayout& OutLayout) const
{
	if (!IsUsingTowerMainMapLayout())
	{
		return false;
	}

	OutLayout = CachedTowerMainMapLayout;
	return true;
}

int32 AT66GameMode::GetTowerFloorIndexForLocation(const FVector& Location) const
{
	if (!IsUsingTowerMainMapLayout())
	{
		return INDEX_NONE;
	}

	return T66TowerMapTerrain::FindFloorIndexForLocation(CachedTowerMainMapLayout, Location);
}

int32 AT66GameMode::GetCurrentTowerFloorIndex() const
{
	if (!IsUsingTowerMainMapLayout())
	{
		return INDEX_NONE;
	}

	if (const UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			const APawn* Pawn = It->Get() ? It->Get()->GetPawn() : nullptr;
			if (!Pawn)
			{
				continue;
			}

			const int32 FloorNumber = GetTowerFloorIndexForLocation(Pawn->GetActorLocation());
			if (FloorNumber != INDEX_NONE)
			{
				return FloorNumber;
			}
		}
	}

	return CachedTowerMainMapLayout.StartFloorNumber;
}

bool AT66GameMode::TryGetTowerEnemySpawnLocation(const FVector& PlayerLocation, float MinDistance, float MaxDistance, FRandomStream& Rng, FVector& OutLocation) const
{
	FVector WallNormal = FVector::ZeroVector;
	return TryGetTowerEnemySpawnLocation(PlayerLocation, MinDistance, MaxDistance, Rng, OutLocation, WallNormal);
}

bool AT66GameMode::TryGetTowerEnemySpawnLocation(
	const FVector& PlayerLocation,
	float MinDistance,
	float MaxDistance,
	FRandomStream& Rng,
	FVector& OutLocation,
	FVector& OutWallNormal) const
{
	if (!IsUsingTowerMainMapLayout())
	{
		return false;
	}

	return T66TowerMapTerrain::TryGetWallSpawnLocation(
		GetWorld(),
		CachedTowerMainMapLayout,
		PlayerLocation,
		MinDistance,
		MaxDistance,
		Rng,
		OutLocation,
		OutWallNormal);
}

void AT66GameMode::HandleTowerDescentHoleTriggered(APawn* Pawn, const int32 FromFloorNumber, const int32 ToFloorNumber)
{
	if (!IsUsingTowerMainMapLayout() || !Pawn)
	{
		return;
	}

	UE_LOG(
		LogT66GameMode,
		Log,
		TEXT("[MAP] Tower descent entered by %s (%d -> %d)"),
		*Pawn->GetName(),
		FromFloorNumber,
		ToFloorNumber);

	if (ToFloorNumber == CachedTowerMainMapLayout.BossFloorNumber)
	{
		bTowerBossEntryTriggered = true;
		bTowerBossEntryApplied = false;
		SyncTowerBossEntryState();
	}
}

bool AT66GameMode::TryGetMainMapStartAxes(FVector& OutCenter, FVector& OutInwardDirection, FVector& OutSideDirection, float& OutCellSize) const
{
	if (!bHasMainMapSpawnSurfaceLocation || MainMapStartPathSurfaceLocation.IsNearlyZero() || MainMapStartAnchorSurfaceLocation.IsNearlyZero())
	{
		return false;
	}

	const FVector Center = MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
		? MainMapSpawnSurfaceLocation
		: MainMapStartAreaCenterSurfaceLocation;
	const FVector Inward2D = (MainMapStartAnchorSurfaceLocation - MainMapStartPathSurfaceLocation).GetSafeNormal2D();
	if (Inward2D.IsNearlyZero())
	{
		return false;
	}

	float CellSize = 0.0f;
	if (IsUsingTowerMainMapLayout())
	{
		CellSize = CachedTowerMainMapLayout.PlacementCellSize;
	}
	else
	{
		const FT66MapPreset Preset = T66BuildMainMapPreset(GetT66GameInstance());
		const T66MainMapTerrain::FSettings Settings = T66MainMapTerrain::MakeSettings(Preset);
		CellSize = Settings.CellSize;
	}

	if (CellSize <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	OutCenter = Center;
	OutInwardDirection = FVector(Inward2D.X, Inward2D.Y, 0.f);
	OutSideDirection = FVector(-Inward2D.Y, Inward2D.X, 0.f);
	OutCellSize = CellSize;
	return true;
}

bool AT66GameMode::TryGetMainMapStartPlacementLocation(float SideCells, float InwardCells, FVector& OutLocation) const
{
	if (IsUsingTowerMainMapLayout())
	{
		return T66TowerMapTerrain::TryGetStartPlacementLocation(GetWorld(), CachedTowerMainMapLayout, SideCells, InwardCells, OutLocation);
	}

	FVector Center = FVector::ZeroVector;
	FVector InwardDirection = FVector::ZeroVector;
	FVector SideDirection = FVector::ZeroVector;
	float CellSize = 0.f;
	if (!TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize))
	{
		return false;
	}

	const FVector DesiredLocation = Center + (SideDirection * (SideCells * CellSize)) + (InwardDirection * (InwardCells * CellSize));
	OutLocation = DesiredLocation;

	UWorld* World = GetWorld();
	if (!World)
	{
		return true;
	}

	const FT66MapPreset Preset = T66BuildMainMapPreset(GetT66GameInstance());
	const T66MainMapTerrain::FSettings Settings = T66MainMapTerrain::MakeSettings(Preset);
	const float TraceStartZ = T66MainMapTerrain::GetTraceZ(Preset);
	const float TraceEndZ = T66MainMapTerrain::GetLowestCollisionBottomZ(Preset) - Settings.StepHeight;

	FHitResult Hit;
	if (World->LineTraceSingleByChannel(
		Hit,
		FVector(DesiredLocation.X, DesiredLocation.Y, TraceStartZ),
		FVector(DesiredLocation.X, DesiredLocation.Y, TraceEndZ),
		ECC_WorldStatic))
	{
		OutLocation = Hit.ImpactPoint;
	}
	else
	{
		OutLocation.Z = Center.Z;
	}

	return true;
}

bool AT66GameMode::TryFindRandomMainMapSurfaceLocation(int32 SeedOffset, FVector& OutLocation, float ExtraSafeBubbleMargin) const
{
	UWorld* World = GetWorld();
	if (!World || !T66UsesMainMapTerrainStage(World))
	{
		return false;
	}

	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!T66GI)
	{
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const FT66MapPreset MainMapPreset = T66BuildMainMapPreset(T66GI);
	const bool bTowerLayout = IsUsingTowerMainMapLayout();
	const T66MainMapTerrain::FSettings MainMapSettings = bTowerLayout
		? T66MainMapTerrain::FSettings{}
		: T66MainMapTerrain::MakeSettings(MainMapPreset);
	const float MainHalfExtent = bTowerLayout
		? 0.0f
		: FMath::Max(0.0f, MainMapSettings.HalfExtent - MainMapSettings.CellSize * 1.25f);
	const float TraceStartZ = bTowerLayout ? CachedTowerMainMapLayout.TraceStartZ : T66MainMapTerrain::GetTraceZ(MainMapPreset);
	const float TraceEndZ = bTowerLayout
		? CachedTowerMainMapLayout.TraceEndZ
		: T66MainMapTerrain::GetLowestCollisionBottomZ(MainMapPreset) - MainMapSettings.StepHeight;
	const float PlacementCellSize = bTowerLayout ? CachedTowerMainMapLayout.PlacementCellSize : MainMapSettings.CellSize;
	const float RoomReserveRadius = PlacementCellSize * T66MainMapRoomReserveRadiusCells;
	const float CorridorReserveRadius = PlacementCellSize * T66MainMapCorridorReserveRadiusCells;
	const float SafeBubbleMargin = 250.f + ExtraSafeBubbleMargin;

	const int32 RunSeed = T66EnsureRunSeed(T66GI);
	const int32 StageNum = RunState ? RunState->GetCurrentStage() : 1;
	FRandomStream Rng(RunSeed + StageNum * 1337 + SeedOffset);

	auto IsInsideReservedZone = [&](const FVector& Location) -> bool
	{
		if (!MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapStartAreaCenterSurfaceLocation) < FMath::Square(RoomReserveRadius))
		{
			return true;
		}
		if (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapBossAreaCenterSurfaceLocation) < FMath::Square(RoomReserveRadius))
		{
			return true;
		}
		if (!MainMapStartPathSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapStartPathSurfaceLocation) < FMath::Square(CorridorReserveRadius))
		{
			return true;
		}
		if (!MainMapStartAnchorSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapStartAnchorSurfaceLocation) < FMath::Square(CorridorReserveRadius))
		{
			return true;
		}
		if (!MainMapBossAnchorSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapBossAnchorSurfaceLocation) < FMath::Square(CorridorReserveRadius))
		{
			return true;
		}
		return false;
	};

	UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
	for (int32 Try = 0; Try < 60; ++Try)
	{
		FVector Candidate = FVector::ZeroVector;
		if (bTowerLayout)
		{
			if (!T66TowerMapTerrain::TryGetRandomGameplaySurfaceLocation(World, CachedTowerMainMapLayout, Rng, Candidate))
			{
				continue;
			}
		}
		else
		{
			const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);

			FHitResult Hit;
			if (!World->LineTraceSingleByChannel(
				Hit,
				FVector(X, Y, TraceStartZ),
				FVector(X, Y, TraceEndZ),
				ECC_WorldStatic))
			{
				continue;
			}
			if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
			{
				continue;
			}

			Candidate = Hit.ImpactPoint;
		}

		if (IsInsideReservedZone(Candidate))
		{
			continue;
		}

		bool bBlockedByNPC = false;
		if (Registry)
		{
			for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Registry->GetNPCs())
			{
				const AT66HouseNPCBase* NPC = WeakNPC.Get();
				if (!NPC)
				{
					continue;
				}
				const float Clearance = NPC->GetSafeZoneRadius() + SafeBubbleMargin;
				if (FVector::DistSquared2D(Candidate, NPC->GetActorLocation()) < FMath::Square(Clearance))
				{
					bBlockedByNPC = true;
					break;
				}
			}
			if (!bBlockedByNPC)
			{
				for (const TWeakObjectPtr<AT66CircusInteractable>& WeakCircus : Registry->GetCircuses())
				{
					const AT66CircusInteractable* Circus = WeakCircus.Get();
					if (!Circus)
					{
						continue;
					}
					const float Clearance = Circus->GetSafeZoneRadius() + SafeBubbleMargin;
					if (FVector::DistSquared2D(Candidate, Circus->GetActorLocation()) < FMath::Square(Clearance))
					{
						bBlockedByNPC = true;
						break;
					}
				}
			}
		}

		if (bBlockedByNPC)
		{
			continue;
		}

		OutLocation = Candidate;
		return true;
	}

	return false;
}

void AT66GameMode::TryActivateMainMapCombat()
{
	UWorld* World = GetWorld();
	if (!World || !T66UsesMainMapTerrainStage(World) || !bTerrainCollisionReady || bMainMapCombatStarted)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	if (MainMapStartAnchorSurfaceLocation.IsNearlyZero() || MainMapStartPathSurfaceLocation.IsNearlyZero())
	{
		return;
	}

	if (IsUsingTowerMainMapLayout())
	{
		bool bAnyPlayerEnteredGameplayFloor = false;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			const APlayerController* PC = It->Get();
			const APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
			if (!PlayerPawn)
			{
				continue;
			}

			const int32 FloorNumber = GetTowerFloorIndexForLocation(PlayerPawn->GetActorLocation());
			if (FloorNumber >= CachedTowerMainMapLayout.FirstGameplayFloorNumber)
			{
				bAnyPlayerEnteredGameplayFloor = true;
				break;
			}
		}

		if (!bAnyPlayerEnteredGameplayFloor)
		{
			return;
		}
	}
	else
	{
	const FVector2D EntryMidpoint = FVector2D(
		(MainMapStartAnchorSurfaceLocation.X + MainMapStartPathSurfaceLocation.X) * 0.5f,
		(MainMapStartAnchorSurfaceLocation.Y + MainMapStartPathSurfaceLocation.Y) * 0.5f);
	const FVector2D EntryNormal = FVector2D(
		MainMapStartAnchorSurfaceLocation.X - MainMapStartPathSurfaceLocation.X,
		MainMapStartAnchorSurfaceLocation.Y - MainMapStartPathSurfaceLocation.Y).GetSafeNormal();
	if (EntryNormal.IsNearlyZero())
	{
		return;
	}

	bool bAnyPlayerEnteredCombat = false;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PC = It->Get();
		const APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
		if (!PlayerPawn)
		{
			continue;
		}

		const FVector PlayerLocation = PlayerPawn->GetActorLocation();
		const float SignedDistance = FVector2D::DotProduct(FVector2D(PlayerLocation.X, PlayerLocation.Y) - EntryMidpoint, EntryNormal);
		if (SignedDistance >= 0.f)
		{
			bAnyPlayerEnteredCombat = true;
			break;
		}
	}

	if (!bAnyPlayerEnteredCombat)
	{
		return;
	}
	}

	RunState->ResetStageTimerToFull();
	if (!bWorldInteractablesSpawnedForStage)
	{
		SpawnWorldInteractablesForStage();
	}
	RunState->SetStageTimerActive(true);

	if (!IsValid(MiasmaManager))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		MiasmaManager = World->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (MiasmaManager)
		{
			MiasmaManager->RebuildForCurrentStage();
		}
	}

	if (MiasmaManager)
	{
		MiasmaManager->UpdateFromRunState();
	}

	if (!IsUsingTowerMainMapLayout())
	{
		bool bHasBoundaryActor = false;
		for (TActorIterator<AT66MiasmaBoundary> It(World); It; ++It)
		{
			if (AT66MiasmaBoundary* ExistingBoundary = *It)
			{
				bHasBoundaryActor = true;
				break;
			}
		}

		if (!bHasBoundaryActor)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			World->SpawnActor<AT66MiasmaBoundary>(AT66MiasmaBoundary::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}
	}

	bool bHasEnemyDirector = false;
	for (TActorIterator<AT66EnemyDirector> It(World); It; ++It)
	{
		bHasEnemyDirector = true;
		break;
	}

	if (!bHasEnemyDirector)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AT66EnemyDirector>(AT66EnemyDirector::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}

	bMainMapCombatStarted = true;
	UE_LOG(LogT66GameMode, Log, TEXT("T66GameMode - Main map combat activated."));
}

void AT66GameMode::SyncTowerBossEntryState()
{
	if (!IsUsingTowerMainMapLayout() || !bTowerBossEntryTriggered || bTowerBossEntryApplied)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bool bHasEnemyDirector = false;
	for (TActorIterator<AT66EnemyDirector> It(World); It; ++It)
	{
		if (AT66EnemyDirector* ExistingDirector = *It)
		{
			ExistingDirector->SetSpawningPaused(true);
			bHasEnemyDirector = true;
		}
	}

	bool bHasBoss = false;
	if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
	{
		for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
		{
			if (AT66BossBase* Boss = WeakBoss.Get())
			{
				bHasBoss = true;
				if (Boss->IsAlive() && !Boss->IsAwakened())
				{
					Boss->ForceAwaken();
				}
				break;
			}
		}
	}

	if (bHasEnemyDirector && bHasBoss)
	{
		bTowerBossEntryApplied = true;
		UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Tower boss-floor entry activated via descent hole."));
	}
}

void AT66GameMode::ResetColiseumState()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->EndSaintBlessingEmpowerment();
			RunState->SetSaintBlessingActive(false);
			RunState->SetFinalSurvivalEnemyScalar(1.f);
		}
	}

	ColiseumBossesRemaining = 0;
	bColiseumExitGateSpawned = false;
	bFinalDifficultySurvivalActive = false;
	FinalDifficultySurvivalElapsedSeconds = 0.f;
	LastAppliedFinalDifficultyEnemyScalar = 1.f;
	FinalDifficultyTotemActor = nullptr;
	FinalDifficultySaintActor = nullptr;
}

void AT66GameMode::HandleBossDefeated(AT66BossBase* Boss)
{
	UWorld* World = GetWorld();
	if (!World) return;
	const FVector Location = Boss ? Boss->GetActorLocation() : FVector::ZeroVector;
	if (Boss && StageBoss.Get() == Boss)
	{
		StageBoss = nullptr;
		DestroyBossBeacon();
	}
	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;

	// Award boss score (scaled by current difficulty).
	if (RunState && Boss)
	{
		const float Scalar = RunState->GetDifficultyScalar();
		const int32 AwardPoints = FMath::Max(0, FMath::RoundToInt(static_cast<float>(Boss->GetPointValue()) * Scalar));
		if (AwardPoints > 0)
		{
			RunState->AddScore(AwardPoints);
		}
	}

	// Lab unlock + achievement: mark this boss as unlocked for The Lab and notify boss killed.
	if (GI)
	{
		if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achieve->NotifyBossKilled(1);
			if (Boss && !Boss->BossID.IsNone())
			{
				Achieve->AddLabUnlockedEnemy(Boss->BossID);
			}
		}
	}

	// Chad Coupons: 1 per boss kill (stage boss or Coliseum).
	if (RunState)
	{
		RunState->AddPowerCrystalsEarnedThisRun(1);
	}

	if (IsColiseumStage())
	{
		if (IsFinalDifficultyBossColiseumStage())
		{
			if (RunState)
			{
				RunState->SetBossInactive();
			}
			ColiseumBossesRemaining = 0;
			BeginFinalDifficultySurvival(Location);
			return;
		}

		// Coliseum: keep the timer/miasma running; open the exit gate only when ALL owed bosses are dead.
		if (RunState)
		{
			RunState->SetBossInactive();
		}

		if (ColiseumBossesRemaining > 0)
		{
			ColiseumBossesRemaining = FMath::Max(0, ColiseumBossesRemaining - 1);
		}

		if (!bColiseumExitGateSpawned && ColiseumBossesRemaining <= 0)
		{
			bColiseumExitGateSpawned = true;
			if (RunState)
			{
				RunState->ClearOwedBosses();
			}
			SpawnStageGateAtLocation(Location);
		}
		return;
	}

	if (RunState)
	{
		RunState->SetBossInactive();
		RunState->SetStageTimerActive(false);
	}

	bool bCompletedSelectedDifficulty = false;
	if (RunState)
	{
		const ET66Difficulty SelectedDifficulty = GetT66GameInstance() ? GetT66GameInstance()->SelectedDifficulty : ET66Difficulty::Easy;
		if (UT66PlayerExperienceSubSystem* PlayerExperience = GI ? GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr)
		{
			const int32 DifficultyEndStage = PlayerExperience->GetDifficultyEndStage(SelectedDifficulty);
			bCompletedSelectedDifficulty = (RunState->GetCurrentStage() >= DifficultyEndStage);
		}
	}

	// First-time stage clear unlock => spawn recruitable companion for stages 1..23.
	if (RunState)
	{
		const int32 StageNum = RunState->GetCurrentStage();
		const FName CompanionToUnlock = T66_CompanionIDForStage(StageNum);
		if (!CompanionToUnlock.IsNone())
		{
			if (UT66CompanionUnlockSubsystem* Unlocks = GI ? GI->GetSubsystem<UT66CompanionUnlockSubsystem>() : nullptr)
			{
				const bool bNewlyUnlocked = Unlocks->UnlockCompanion(CompanionToUnlock);
				if (bNewlyUnlocked)
				{
					if (UT66GameInstance* T66GI = GetT66GameInstance())
					{
						FCompanionData Data;
						if (T66GI->GetCompanionData(CompanionToUnlock, Data))
						{
							FActorSpawnParameters SpawnParams;
							SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
							if (AT66RecruitableCompanion* Recruit = World->SpawnActor<AT66RecruitableCompanion>(AT66RecruitableCompanion::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams))
							{
								Recruit->InitializeRecruit(Data);
							}
						}
					}
				}
			}
		}
		if (StageNum == 23)
		{
			if (UT66CompanionUnlockSubsystem* Unlocks = GI ? GI->GetSubsystem<UT66CompanionUnlockSubsystem>() : nullptr)
			{
				Unlocks->UnlockCompanion(FName(TEXT("Companion_24")));
			}
		}
	}

	// Normal stage: boss dead => miasma disappears and Stage Gate appears.
	ClearMiasma();
	if (bCompletedSelectedDifficulty)
	{
		bool bOpenedSummary = false;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(It->Get()))
			{
				T66PC->ClientShowVictoryRunSummary();
				bOpenedSummary = true;
			}
		}

		if (!bOpenedSummary)
		{
			UE_LOG(LogT66GameMode, Warning, TEXT("Difficulty clear reached but no T66PlayerController was available to open Run Summary."));
		}
		return;
	}

	SpawnStageGateAtLocation(Location);

	if (RunState)
	{
		const int32 ClearedStage = RunState->GetCurrentStage();
		UE_LOG(LogT66GameMode, Verbose, TEXT("Stage %d cleared; idol altar progression now happens at stage entry."), ClearedStage);
	}
}

bool AT66GameMode::SwapCompanionForPlayer(AController* Player, FName NewCompanionID)
{
	if (!Player) return false;
	if (NewCompanionID.IsNone()) return false;

	UT66GameInstance* GI = GetT66GameInstance();
	if (!GI) return false;

	// If this companion is already selected, treat as handled (no swap).
	if (GI->SelectedCompanionID == NewCompanionID)
	{
		return true;
	}

	// Validate new companion exists in DT_Companions (prevents setting an invalid ID).
	{
		FCompanionData NewData;
		if (!GI->GetCompanionData(NewCompanionID, NewData))
		{
			return false;
		}
	}

	UWorld* World = GetWorld();
	if (!World) return false;

	// If a companion is currently following, spawn a recruitable version where it currently is.
	FName OldCompanionID = NAME_None;
	FVector OldCompanionLoc = FVector::ZeroVector;
	if (TWeakObjectPtr<AT66CompanionBase>* Existing = PlayerCompanions.Find(Player))
	{
		if (AT66CompanionBase* ExistingComp = Existing->Get())
		{
			OldCompanionID = ExistingComp->CompanionID;
			OldCompanionLoc = ExistingComp->GetActorLocation();
			ExistingComp->Destroy();
		}
		PlayerCompanions.Remove(Player);
	}

	if (!OldCompanionID.IsNone())
	{
		FCompanionData OldData;
		if (GI->GetCompanionData(OldCompanionID, OldData))
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AT66RecruitableCompanion* OldRecruit = World->SpawnActor<AT66RecruitableCompanion>(AT66RecruitableCompanion::StaticClass(), OldCompanionLoc, FRotator::ZeroRotator, SpawnParams))
			{
				OldRecruit->InitializeRecruit(OldData);
			}
		}
	}

	// Persist for the rest of the run (stage transitions read SelectedCompanionID).
	GI->SelectedCompanionID = NewCompanionID;

	SpawnCompanionForPlayer(Player);
	return true;
}

void AT66GameMode::ClearMiasma()
{
	if (MiasmaManager)
	{
		MiasmaManager->ClearAllMiasma();
	}
}

void AT66GameMode::SpawnCompanionForPlayer(AController* Player)
{
	UT66GameInstance* GI = GetT66GameInstance();
	if (!GI) return;
	if (T66IsStandaloneTutorialMap(GetWorld()))
	{
		return;
	}

	const FName SelectedCompanionID = T66GetSelectedCompanionID(GI, Player);
	if (SelectedCompanionID.IsNone()) return;

	FCompanionData CompanionData;
	if (!GI->GetCompanionData(SelectedCompanionID, CompanionData)) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APawn* HeroPawn = Player ? Player->GetPawn() : nullptr;
	if (!HeroPawn) return;

	// Prevent duplicate companions on respawn.
	if (Player)
	{
		if (TWeakObjectPtr<AT66CompanionBase>* Existing = PlayerCompanions.Find(Player))
		{
			if (AT66CompanionBase* ExistingComp = Existing->Get())
			{
				ExistingComp->Destroy();
			}
			PlayerCompanions.Remove(Player);
		}
	}

	UClass* CompanionClass = AT66CompanionBase::StaticClass();
	const bool bWantsSpecificClass = !CompanionData.CompanionClass.IsNull();
	const bool bHasLoadedClass = bWantsSpecificClass && (CompanionData.CompanionClass.Get() != nullptr);
	if (bHasLoadedClass)
	{
		if (UClass* Loaded = CompanionData.CompanionClass.Get())
		{
			if (Loaded->IsChildOf(AT66CompanionBase::StaticClass()))
			{
				CompanionClass = Loaded;
			}
		}
	}

	FVector SpawnLoc = HeroPawn->GetActorLocation() + FVector(-150.f, 100.f, 0.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = HeroPawn;
	SpawnParams.Instigator = HeroPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FName CompanionSkinID = FName(TEXT("Default"));
	if (UGameInstance* GII = GetGameInstance())
	{
		if (UT66AchievementsSubsystem* Ach = GII->GetSubsystem<UT66AchievementsSubsystem>())
		{
			CompanionSkinID = Ach->GetEquippedCompanionSkinID(SelectedCompanionID);
		}
	}

	AT66CompanionBase* Companion = World->SpawnActor<AT66CompanionBase>(CompanionClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (Companion)
	{
		Companion->InitializeCompanion(CompanionData, CompanionSkinID);
		Companion->SetPreviewMode(false); // gameplay: follow hero
		if (Player)
		{
			PlayerCompanions.Add(Player, Companion);
		}
		// Snap companion to ground so it doesn't float.
		FHitResult Hit;
		const FVector Start = Companion->GetActorLocation() + FVector(0.f, 0.f, 2000.f);
		const FVector End = Companion->GetActorLocation() - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			Companion->SetActorLocation(Hit.ImpactPoint, false, nullptr, ETeleportType::TeleportPhysics);
		}
		UE_LOG(LogT66GameMode, Log, TEXT("Spawned companion: %s"), *CompanionData.DisplayName.ToString());
	}

	// If the companion class is a soft reference and isn't loaded yet, load asynchronously and replace.
	if (bWantsSpecificClass && !bHasLoadedClass)
	{
		const FSoftObjectPath ClassPath = CompanionData.CompanionClass.ToSoftObjectPath();
		const TWeakObjectPtr<AController> WeakPlayer(Player);
		const TWeakObjectPtr<AT66CompanionBase> WeakExisting(Companion);
		const FCompanionData CompanionDataCopy = CompanionData;

		TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			ClassPath,
			FStreamableDelegate::CreateWeakLambda(this, [this, WeakPlayer, WeakExisting, CompanionDataCopy]()
			{
				AController* PlayerCtrl = WeakPlayer.Get();
				if (!PlayerCtrl) return;

				UWorld* World2 = GetWorld();
				if (!World2) return;

				UClass* Loaded = CompanionDataCopy.CompanionClass.Get();
				if (!Loaded || !Loaded->IsChildOf(AT66CompanionBase::StaticClass()))
				{
					return;
				}

				AT66CompanionBase* ExistingComp = WeakExisting.Get();
				// If the existing companion is already the correct class (or was destroyed), do nothing.
				if (ExistingComp && ExistingComp->GetClass() == Loaded)
				{
					return;
				}

				// Remove the old companion if it's still around.
				if (TWeakObjectPtr<AT66CompanionBase>* Current = PlayerCompanions.Find(PlayerCtrl))
				{
					if (AT66CompanionBase* C = Current->Get())
					{
						C->Destroy();
					}
					PlayerCompanions.Remove(PlayerCtrl);
				}

				APawn* HeroPawn2 = PlayerCtrl->GetPawn();
				if (!HeroPawn2) return;

				FVector SpawnLoc2 = HeroPawn2->GetActorLocation() + FVector(-150.f, 100.f, 0.f);
				FActorSpawnParameters SpawnParams2;
				SpawnParams2.Owner = HeroPawn2;
				SpawnParams2.Instigator = HeroPawn2;
				SpawnParams2.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				FName AsyncSkinID = FName(TEXT("Default"));
				if (UGameInstance* GII = GetGameInstance())
				{
					if (UT66AchievementsSubsystem* Ach = GII->GetSubsystem<UT66AchievementsSubsystem>())
					{
						AsyncSkinID = Ach->GetEquippedCompanionSkinID(CompanionDataCopy.CompanionID);
					}
				}
				AT66CompanionBase* NewComp = World2->SpawnActor<AT66CompanionBase>(Loaded, SpawnLoc2, FRotator::ZeroRotator, SpawnParams2);
				if (NewComp)
				{
					NewComp->InitializeCompanion(CompanionDataCopy, AsyncSkinID);
					NewComp->SetPreviewMode(false);
					PlayerCompanions.Add(PlayerCtrl, NewComp);
				}
			}));
		if (Handle.IsValid())
		{
			ActiveAsyncLoadHandles.Add(Handle);
		}
	}
}

void AT66GameMode::SpawnVendorForPlayer(AController* Player)
{
	APawn* HeroPawn = Player ? Player->GetPawn() : nullptr;
	if (!HeroPawn) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FVector SpawnLoc = HeroPawn->GetActorLocation() + FVector(300.f, 0.f, 0.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66VendorNPC* Vendor = World->SpawnActor<AT66VendorNPC>(AT66VendorNPC::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (Vendor)
	{
		UE_LOG(LogT66GameMode, Log, TEXT("Spawned vendor NPC near hero"));
	}
}

void AT66GameMode::SpawnCircusInteractableIfNeeded()
{
	if (IsColiseumStage() || IsLabLevel())
	{
		return;
	}

	if (IsUsingTowerMainMapLayout())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AT66CircusInteractable> It(World); It; ++It)
	{
		return;
	}

	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	float TraceStartZ = 2000.f;
	float TraceEndZ = -4000.f;
	float RefX = 0.f;
	float RefY = 0.f;
	if (bUsingMainMapTerrain)
	{
		FVector CircusLoc = FVector::ZeroVector;
		if (!TryFindRandomMainMapSurfaceLocation(3201, CircusLoc, 450.f))
		{
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AT66CircusInteractable>(AT66CircusInteractable::StaticClass(), CircusLoc, FRotator::ZeroRotator, SpawnParams);
		return;
	}

	auto FindClosestFlatSurface = [World, TraceStartZ, TraceEndZ](float InRefX, float InRefY) -> FVector
	{
		static constexpr float MinNormalZ = 0.92f;
		static constexpr float SearchRadiusMax = 2200.f;
		static constexpr float RadiusStep = 140.f;
		static constexpr int32 NumAngles = 20;

		FVector Fallback(InRefX, InRefY, 60.f);
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, FVector(InRefX, InRefY, TraceStartZ), FVector(InRefX, InRefY, TraceEndZ), ECC_WorldStatic))
		{
			Fallback = Hit.ImpactPoint;
		}

		for (float R = 0.f; R <= SearchRadiusMax; R += RadiusStep)
		{
			const int32 AngleSteps = (R <= 0.f) ? 1 : NumAngles;
			for (int32 AngleIndex = 0; AngleIndex < AngleSteps; ++AngleIndex)
			{
				const float Angle = (AngleSteps == 1) ? 0.f : (2.f * PI * static_cast<float>(AngleIndex) / static_cast<float>(AngleSteps));
				const float X = InRefX + R * FMath::Cos(Angle);
				const float Y = InRefY + R * FMath::Sin(Angle);
				if (World->LineTraceSingleByChannel(Hit, FVector(X, Y, TraceStartZ), FVector(X, Y, TraceEndZ), ECC_WorldStatic) && Hit.ImpactNormal.Z >= MinNormalZ)
				{
					return Hit.ImpactPoint;
				}
			}
		}

		return Fallback;
	};

	const FVector FlatLoc = FindClosestFlatSurface(RefX, RefY);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AT66CircusInteractable>(AT66CircusInteractable::StaticClass(), FlatLoc, FRotator::ZeroRotator, SpawnParams);
}

void AT66GameMode::SpawnSupportVendorAtStartIfNeeded()
{
	if (IsColiseumStage() || IsLabLevel())
	{
		return;
	}

	if (IsUsingTowerMainMapLayout())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!T66GI)
	{
		return;
	}

	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>();
	if (!PlayerExperience || !PlayerExperience->ShouldSpawnSupportVendorAtRunStart(T66GI->SelectedDifficulty))
	{
		return;
	}

	static const FName SupportVendorTag(TEXT("T66_StartSupportVendor"));
	for (TActorIterator<AT66VendorNPC> It(World); It; ++It)
	{
		if (AT66VendorNPC* ExistingVendor = *It)
		{
			if (ExistingVendor->ActorHasTag(SupportVendorTag))
			{
				return;
			}
		}
	}

	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	float TraceStartZ = 2000.f;
	float TraceEndZ = -4000.f;
	FVector ReferenceLoc = FVector(-35455.f, 0.f, 60.f);
	float SideOffset = 700.f;
	if (bUsingMainMapTerrain)
	{
		FVector SpawnLoc = FVector::ZeroVector;
		if (!TryGetMainMapStartPlacementLocation(1.0f, 0.0f, SpawnLoc))
		{
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (AT66VendorNPC* SupportVendor = World->SpawnActor<AT66VendorNPC>(AT66VendorNPC::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams))
		{
			SupportVendor->ConfigureAsSupportVendor(PlayerExperience->ShouldSupportVendorAllowSteal(T66GI->SelectedDifficulty));
			SupportVendor->Tags.AddUnique(SupportVendorTag);
			SupportVendor->ApplyVisuals();
		}
		return;
	}
	else if (AController* PC = World->GetFirstPlayerController())
	{
		if (APawn* HeroPawn = PC->GetPawn())
		{
			ReferenceLoc = HeroPawn->GetActorLocation();
		}
	}

	auto FindClosestFlatSurface = [World, TraceStartZ, TraceEndZ](const FVector& DesiredLoc) -> FVector
	{
		static constexpr float MinNormalZ = 0.92f;
		static constexpr float SearchRadiusMax = 900.f;
		static constexpr float RadiusStep = 100.f;
		static constexpr int32 NumAngles = 16;

		FVector Fallback = DesiredLoc;
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(
			Hit,
			FVector(DesiredLoc.X, DesiredLoc.Y, TraceStartZ),
			FVector(DesiredLoc.X, DesiredLoc.Y, TraceEndZ),
			ECC_WorldStatic))
		{
			Fallback = Hit.ImpactPoint;
		}

		for (float Radius = 0.f; Radius <= SearchRadiusMax; Radius += RadiusStep)
		{
			const int32 AngleSteps = (Radius <= 0.f) ? 1 : NumAngles;
			for (int32 AngleIndex = 0; AngleIndex < AngleSteps; ++AngleIndex)
			{
				const float Angle = (AngleSteps == 1) ? 0.f : (2.f * PI * static_cast<float>(AngleIndex) / static_cast<float>(AngleSteps));
				const float X = DesiredLoc.X + Radius * FMath::Cos(Angle);
				const float Y = DesiredLoc.Y + Radius * FMath::Sin(Angle);
				if (World->LineTraceSingleByChannel(Hit, FVector(X, Y, TraceStartZ), FVector(X, Y, TraceEndZ), ECC_WorldStatic)
					&& Hit.ImpactNormal.Z >= MinNormalZ)
				{
					return Hit.ImpactPoint;
				}
			}
		}

		return Fallback;
	};

	const FVector DesiredLoc = ReferenceLoc + FVector(SideOffset, 0.f, 0.f);
	const FVector SpawnLoc = FindClosestFlatSurface(DesiredLoc);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66VendorNPC* SupportVendor = World->SpawnActor<AT66VendorNPC>(AT66VendorNPC::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams))
	{
		SupportVendor->ConfigureAsSupportVendor(PlayerExperience->ShouldSupportVendorAllowSteal(T66GI->SelectedDifficulty));
		SupportVendor->Tags.AddUnique(SupportVendorTag);
		SupportVendor->ApplyVisuals();
	}
}

void AT66GameMode::SpawnStartGateForPlayer(AController* Player)
{
	(void)Player;
	UWorld* World = GetWorld();
	if (!World) return;
	if (T66UsesMainMapTerrainStage(World)) return;

	// Spawn once per level (gate is a world landmark).
	if (StartGate) return;

	// Gate at the start-corridor exit so combat starts only when the player enters the main arena.
	static constexpr float GateZOffset = 5.f;
	FVector GateLoc = T66GameplayLayout::GetStartGateLocation();
	float GateGroundZ = 200.f;
	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, GateLoc + FVector(0.f, 0.f, 3000.f), GateLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
	{
		GateGroundZ = Hit.ImpactPoint.Z;
		GateLoc.Z = GateGroundZ + GateZOffset;
	}
	else
	{
		GateLoc.Z = GateGroundZ;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	StartGate = World->SpawnActor<AT66StartGate>(AT66StartGate::StaticClass(), GateLoc, FRotator::ZeroRotator, SpawnParams);
	if (StartGate)
	{
		StartGate->TriggerDistance2D = 220.f;
		if (StartGate->TriggerBox)
		{
			StartGate->TriggerBox->SetBoxExtent(FVector(120.f, T66GameplayLayout::CorridorHalfHeightY * 0.92f, 220.f));
		}
		if (StartGate->PoleLeft)
		{
			StartGate->PoleLeft->SetVisibility(false, true);
			StartGate->PoleLeft->SetHiddenInGame(true, true);
		}
		if (StartGate->PoleRight)
		{
			StartGate->PoleRight->SetVisibility(false, true);
			StartGate->PoleRight->SetHiddenInGame(true, true);
		}
		UE_LOG(LogT66GameMode, Log, TEXT("Spawned Start Gate at main-area entrance (%.0f, %.0f, %.0f)"), GateLoc.X, GateLoc.Y, GateLoc.Z);
	}

}

void AT66GameMode::SpawnPlateauAtLocation(UWorld* World, const FVector& TopCenterLoc)
{
	if (!World) return;
	static constexpr float PlateauHalfHeight = 10.f; // top of disc at TopCenterLoc.Z
	const FVector PlateauLoc(TopCenterLoc.X, TopCenterLoc.Y, TopCenterLoc.Z - PlateauHalfHeight);
	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AT66SpawnPlateau>(AT66SpawnPlateau::StaticClass(), PlateauLoc, FRotator::ZeroRotator, P);
}

void AT66GameMode::SpawnGuaranteedStartAreaInteractables()
{
	if (IsColiseumStage() || IsLabLevel() || IsUsingTowerMainMapLayout())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!T66GI)
	{
		return;
	}

	UT66RunStateSubsystem* RunState = T66GI->GetSubsystem<UT66RunStateSubsystem>();
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>();
	UT66RngSubsystem* RngSub = T66GI->GetSubsystem<UT66RngSubsystem>();
	if (!RunState)
	{
		return;
	}

	if (RngSub)
	{
		RngSub->UpdateLuckStat(RunState->GetLuckStat());
	}

	const ET66Difficulty Difficulty = T66GI->SelectedDifficulty;
	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	const int32 RunSeed = T66EnsureRunSeed(T66GI);
	const int32 StageNum = RunState->GetCurrentStage();
	FRandomStream Rng(RunSeed + StageNum * 1337 + 807);

	FVector StartCenter = FVector::ZeroVector;
	if (bUsingMainMapTerrain)
	{
		StartCenter = MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
			? MainMapSpawnSurfaceLocation
			: MainMapStartAreaCenterSurfaceLocation;
	}
	else
	{
		StartCenter = T66GameplayLayout::GetStartAreaCenter();

		FHitResult Hit;
		const FVector TraceStart = StartCenter + FVector(0.f, 0.f, 3000.f);
		const FVector TraceEnd = StartCenter - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic)
			|| World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility))
		{
			StartCenter = Hit.ImpactPoint;
		}
	}

	auto FindExistingTaggedActor = [World](const FName Tag) -> AActor*
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Existing = *It;
			if (IsValid(Existing) && Existing->ActorHasTag(Tag))
			{
				return Existing;
			}
		}
		return nullptr;
	};

	auto FindClassicStartAreaSurface = [World](const FVector& DesiredLoc) -> FVector
	{
		static constexpr float TraceStartZ = 8000.f;
		static constexpr float TraceEndZ = -16000.f;
		static constexpr float MinNormalZ = 0.92f;
		static constexpr float SearchRadiusMax = 650.f;
		static constexpr float RadiusStep = 100.f;
		static constexpr int32 NumAngles = 12;

		FVector Fallback = DesiredLoc;
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(
			Hit,
			FVector(DesiredLoc.X, DesiredLoc.Y, TraceStartZ),
			FVector(DesiredLoc.X, DesiredLoc.Y, TraceEndZ),
			ECC_WorldStatic))
		{
			Fallback = Hit.ImpactPoint;
		}

		for (float Radius = 0.f; Radius <= SearchRadiusMax; Radius += RadiusStep)
		{
			const int32 AngleSteps = (Radius <= 0.f) ? 1 : NumAngles;
			for (int32 AngleIndex = 0; AngleIndex < AngleSteps; ++AngleIndex)
			{
				const float Angle = (AngleSteps == 1) ? 0.f : (2.f * PI * static_cast<float>(AngleIndex) / static_cast<float>(AngleSteps));
				const float X = DesiredLoc.X + Radius * FMath::Cos(Angle);
				const float Y = DesiredLoc.Y + Radius * FMath::Sin(Angle);
				if (World->LineTraceSingleByChannel(Hit, FVector(X, Y, TraceStartZ), FVector(X, Y, TraceEndZ), ECC_WorldStatic)
					&& Hit.ImpactNormal.Z >= MinNormalZ)
				{
					return Hit.ImpactPoint;
				}
			}
		}

		return Fallback;
	};

	auto ResolveStartSpawnLocation = [&](const FVector& ClassicOffset, float SideCells, float InwardCells, FVector& OutLocation) -> bool
	{
		if (bUsingMainMapTerrain)
		{
			return TryGetMainMapStartPlacementLocation(SideCells, InwardCells, OutLocation);
		}

		OutLocation = FindClassicStartAreaSurface(StartCenter + ClassicOffset);
		return true;
	};

	auto BuildFacingRotation = [&](const FVector& SpawnLocation) -> FRotator
	{
		FRotator FacingRotation = FRotator::ZeroRotator;
		if (!T66TryBuildFacingRotation2D(SpawnLocation, StartCenter, FacingRotation))
		{
			FacingRotation = FRotator::ZeroRotator;
		}
		return FacingRotation;
	};

	auto RollWeightedRarity = [&](const FT66RarityWeights& Weights) -> ET66Rarity
	{
		return (RngSub && PlayerExperience)
			? RngSub->RollRarityWeighted(Weights, Rng)
			: FT66RarityUtil::RollDefaultRarity(Rng);
	};

	enum class EGuaranteedStartInteractable : uint8
	{
		QuickRevive,
		Fountain,
		Chest,
		Wheel,
		LootBag,
		Crate,
	};

	struct FGuaranteedStartSpec
	{
		EGuaranteedStartInteractable Kind;
		const TCHAR* TagName = nullptr;
		FVector ClassicOffset = FVector::ZeroVector;
		float SideCells = 0.f;
		float InwardCells = 0.f;
	};

	const FGuaranteedStartSpec SpawnSpecs[] = {
		{ EGuaranteedStartInteractable::QuickRevive, TEXT("T66_StartGuaranteed_QuickRevive"), FVector(-900.f, -1400.f, 0.f), -1.10f, -0.35f },
		{ EGuaranteedStartInteractable::Fountain,    TEXT("T66_StartGuaranteed_Fountain"),    FVector(-900.f,  1400.f, 0.f), -1.10f,  0.75f },
		{ EGuaranteedStartInteractable::Chest,       TEXT("T66_StartGuaranteed_Chest"),       FVector(   0.f,  1800.f, 0.f), -0.20f,  1.30f },
		{ EGuaranteedStartInteractable::LootBag,     TEXT("T66_StartGuaranteed_LootBag"),     FVector( 900.f, -1400.f, 0.f),  0.35f,  0.45f },
		{ EGuaranteedStartInteractable::Crate,       TEXT("T66_StartGuaranteed_Crate"),       FVector(   0.f, -1800.f, 0.f),  0.00f,  0.55f },
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FGuaranteedStartSpec& Spec : SpawnSpecs)
	{
		if (Spec.TagName == nullptr)
		{
			continue;
		}

		const FName Tag(Spec.TagName);
		if (FindExistingTaggedActor(Tag))
		{
			continue;
		}

		FVector SpawnLoc = FVector::ZeroVector;
		if (!ResolveStartSpawnLocation(Spec.ClassicOffset, Spec.SideCells, Spec.InwardCells, SpawnLoc))
		{
			continue;
		}

		const FRotator SpawnRotation = BuildFacingRotation(SpawnLoc);
		AActor* SpawnedActor = nullptr;

		switch (Spec.Kind)
		{
		case EGuaranteedStartInteractable::QuickRevive:
			SpawnedActor = World->SpawnActor<AT66QuickReviveVendingMachine>(
				AT66QuickReviveVendingMachine::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams);
			break;

		case EGuaranteedStartInteractable::Fountain:
			SpawnedActor = World->SpawnActor<AT66FountainOfLifeInteractable>(
				AT66FountainOfLifeInteractable::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams);
			break;

		case EGuaranteedStartInteractable::Chest:
			if (AT66ChestInteractable* Chest = World->SpawnActor<AT66ChestInteractable>(
				AT66ChestInteractable::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams))
			{
				const FT66RarityWeights Weights = PlayerExperience
					? PlayerExperience->GetDifficultyChestRarityWeights(Difficulty)
					: FT66RarityWeights{};
				const ET66Rarity ChestRarity = RollWeightedRarity(Weights);
				const int32 ChestRarityDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
				const int32 ChestRarityPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
				Chest->bIsMimic = false;
				Chest->SetRarity(ChestRarity);
				if (RunState)
				{
					RunState->RecordLuckQualityRarity(FName(TEXT("ChestRarity")), ChestRarity, ChestRarityDrawIndex, ChestRarityPreDrawSeed, &Weights);
				}
				SpawnedActor = Chest;
			}
			break;

		case EGuaranteedStartInteractable::Wheel:
			if (AT66WheelSpinInteractable* Wheel = World->SpawnActor<AT66WheelSpinInteractable>(
				AT66WheelSpinInteractable::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams))
			{
				const FT66RarityWeights Weights = PlayerExperience
					? PlayerExperience->GetDifficultyWheelRarityWeights(Difficulty)
					: FT66RarityWeights{};
				const ET66Rarity WheelRarity = RollWeightedRarity(Weights);
				const int32 WheelRarityDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
				const int32 WheelRarityPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
				Wheel->SetRarity(WheelRarity);
				if (RunState)
				{
					RunState->RecordLuckQualityRarity(FName(TEXT("WheelRarity")), WheelRarity, WheelRarityDrawIndex, WheelRarityPreDrawSeed, &Weights);
				}
				SpawnedActor = Wheel;
			}
			break;

		case EGuaranteedStartInteractable::LootBag:
			if (AT66LootBagPickup* LootBag = World->SpawnActor<AT66LootBagPickup>(
				AT66LootBagPickup::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams))
			{
				const FT66RarityWeights Weights = PlayerExperience
					? PlayerExperience->GetDifficultyEnemyLootBagRarityWeights(Difficulty)
					: FT66RarityWeights{};
				const ET66Rarity BagRarity = RollWeightedRarity(Weights);
				LootBag->SetLootRarity(BagRarity);
				LootBag->SetItemID(T66GI->GetRandomItemIDForLootRarityFromStream(BagRarity, Rng));
				if (RunState)
				{
					RunState->RecordLuckQualityRarity(
						FName(TEXT("GuaranteedStartLootBagRarity")),
						BagRarity,
						RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE,
						RngSub ? RngSub->GetLastRunPreDrawSeed() : 0,
						&Weights);
				}
				SpawnedActor = LootBag;
			}
			break;

		case EGuaranteedStartInteractable::Crate:
			if (AT66CrateInteractable* Crate = World->SpawnActor<AT66CrateInteractable>(
				AT66CrateInteractable::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams))
			{
				const FT66RarityWeights Weights = PlayerExperience
					? PlayerExperience->GetDifficultyCrateRarityWeights(Difficulty)
					: FT66RarityWeights{};
				const ET66Rarity CrateRarity = RollWeightedRarity(Weights);
				const int32 CrateRarityDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
				const int32 CrateRarityPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
				Crate->SetRarity(CrateRarity);
				if (RunState)
				{
					RunState->RecordLuckQualityRarity(FName(TEXT("CrateRarity")), CrateRarity, CrateRarityDrawIndex, CrateRarityPreDrawSeed, &Weights);
				}
				SpawnedActor = Crate;
			}
			break;
		}

		if (SpawnedActor)
		{
			SpawnedActor->Tags.AddUnique(Tag);
		}
	}
}

void AT66GameMode::SpawnWorldInteractablesForStage()
{
	if (IsColiseumStage()) return;
	if (bWorldInteractablesSpawnedForStage) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();

	// Run-level seed so positions change every time "Enter the Tribulation" or PIE is started (like procedural terrain).
	const int32 RunSeed = T66EnsureRunSeed(T66GI);
	const int32 StageNum = RunState->GetCurrentStage();
	FRandomStream Rng(RunSeed + StageNum * 1337 + 42);
	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	const bool bTowerLayout = IsUsingTowerMainMapLayout();
	TArray<int32, TInlineAllocator<4>> TowerGameplayFloorNumbers;
	TMap<int32, int32> TowerChestCountByFloor;
	TMap<int32, int32> TowerCrateCountByFloor;
	if (bTowerLayout)
	{
		FRandomStream TowerCountRng(RunSeed + StageNum * 1901 + 39);
		for (const T66TowerMapTerrain::FFloor& Floor : CachedTowerMainMapLayout.Floors)
		{
			if (!Floor.bGameplayFloor
				|| Floor.FloorNumber < CachedTowerMainMapLayout.FirstGameplayFloorNumber
				|| Floor.FloorNumber > CachedTowerMainMapLayout.LastGameplayFloorNumber)
			{
				continue;
			}

			TowerGameplayFloorNumbers.Add(Floor.FloorNumber);
			TowerChestCountByFloor.Add(Floor.FloorNumber, TowerCountRng.RandRange(1, 3));
			TowerCrateCountByFloor.Add(Floor.FloorNumber, TowerCountRng.RandRange(1, 3));
		}
		TowerGameplayFloorNumbers.Sort();
	}

	FT66MapPreset MainMapPreset;
	T66MainMapTerrain::FSettings MainMapSettings;
	float MainHalfExtent = 50000.f;
	float TraceStartZ = 8000.f;
	float TraceEndZ = -16000.f;
	if (bUsingMainMapTerrain && !bTowerLayout)
	{
		MainMapPreset = T66BuildMainMapPreset(T66GI);
		MainMapSettings = T66MainMapTerrain::MakeSettings(MainMapPreset);
		MainHalfExtent = FMath::Max(0.0f, MainMapSettings.HalfExtent - MainMapSettings.CellSize * 1.25f);
		TraceStartZ = T66MainMapTerrain::GetTraceZ(MainMapPreset);
		TraceEndZ = T66MainMapTerrain::GetLowestCollisionBottomZ(MainMapPreset) - MainMapSettings.StepHeight;
	}

	static constexpr float SpawnZ = 220.f;
	static constexpr float MinDistBetweenInteractables = 900.f;
	static constexpr float SafeBubbleMargin = 250.f;

	TArray<FVector> UsedLocs;

	// No-spawn zones: keep gameplay spawns out of reserved traversal spaces and special arenas.
	auto IsInsideNoSpawnZone = [&](const FVector& L) -> bool
	{
		if (bTowerLayout)
		{
			return false;
		}

		if (bUsingMainMapTerrain)
		{
			const float RoomReserveRadius = MainMapSettings.CellSize * T66MainMapRoomReserveRadiusCells;
			const float CorridorReserveRadius = MainMapSettings.CellSize * T66MainMapCorridorReserveRadiusCells;
			if (!MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapStartAreaCenterSurfaceLocation) < FMath::Square(RoomReserveRadius))
			{
				return true;
			}
			if (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapBossAreaCenterSurfaceLocation) < FMath::Square(RoomReserveRadius))
			{
				return true;
			}
			if (!MainMapStartPathSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapStartPathSurfaceLocation) < FMath::Square(CorridorReserveRadius))
			{
				return true;
			}
			if (!MainMapStartAnchorSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapStartAnchorSurfaceLocation) < FMath::Square(CorridorReserveRadius))
			{
				return true;
			}
			if (!MainMapBossAnchorSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapBossAnchorSurfaceLocation) < FMath::Square(CorridorReserveRadius))
			{
				return true;
			}
			return false;
		}

		if (T66GameplayLayout::IsInsideReservedTraversalZone2D(L, 455.f))
		{
			return true;
		}

		static constexpr float ArenaHalf = 9091.f;
		static constexpr float ArenaMargin = 682.f;
		static constexpr float TutorialArenaHalf = 9091.f;
		struct FArena2D { float X; float Y; float Half; };
		static constexpr FArena2D Arenas[] = {
			{      0.f, 34091.f, ArenaHalf },  // Coliseum
			{      0.f, 61364.f, TutorialArenaHalf }, // Tutorial
		};
		for (const FArena2D& A : Arenas)
		{
			if (FMath::Abs(L.X - A.X) <= (A.Half + ArenaMargin) && FMath::Abs(L.Y - A.Y) <= (A.Half + ArenaMargin))
			{
				return true;
			}
		}

		return false;
	};

	auto IsSameTowerFloor = [&](const FVector& A, const FVector& B) -> bool
	{
		if (!bTowerLayout)
		{
			return true;
		}

		const int32 FloorA = GetTowerFloorIndexForLocation(A);
		const int32 FloorB = GetTowerFloorIndexForLocation(B);
		return FloorA != INDEX_NONE && FloorA == FloorB;
	};

	auto ResolveTowerFloorForActor = [&](const AActor* Actor) -> int32
	{
		if (!bTowerLayout || !Actor)
		{
			return INDEX_NONE;
		}

		const int32 TaggedFloor = T66ReadTowerFloorTag(Actor);
		if (TaggedFloor != INDEX_NONE)
		{
			return TaggedFloor;
		}

		return GetTowerFloorIndexForLocation(Actor->GetActorLocation());
	};

	auto IsGoodLoc = [&](const FVector& L) -> bool
	{
		if (IsInsideNoSpawnZone(L))
		{
			return false;
		}
		static constexpr float CasinoKeepClearRadius = 2200.f;
		if (!bUsingMainMapTerrain && FVector::DistSquared2D(L, FVector::ZeroVector) < (CasinoKeepClearRadius * CasinoKeepClearRadius))
		{
			return false;
		}
		for (const FVector& U : UsedLocs)
		{
			if (!IsSameTowerFloor(L, U))
			{
				continue;
			}
			if (FVector::DistSquared2D(L, U) < (MinDistBetweenInteractables * MinDistBetweenInteractables))
			{
				return false;
			}
		}
		UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		const TArray<TWeakObjectPtr<AT66HouseNPCBase>>& NPCs = Registry ? Registry->GetNPCs() : TArray<TWeakObjectPtr<AT66HouseNPCBase>>();
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : NPCs)
		{
			const AT66HouseNPCBase* NPC = WeakNPC.Get();
			if (!NPC) continue;
			if (bTowerLayout)
			{
				const int32 CandidateFloor = GetTowerFloorIndexForLocation(L);
				const int32 NPCFloor = ResolveTowerFloorForActor(NPC);
				if (CandidateFloor == INDEX_NONE || NPCFloor == INDEX_NONE || CandidateFloor != NPCFloor)
				{
					continue;
				}
			}
			else if (!IsSameTowerFloor(L, NPC->GetActorLocation()))
			{
				continue;
			}
			const float R = NPC->GetSafeZoneRadius() + SafeBubbleMargin;
			if (FVector::DistSquared2D(L, NPC->GetActorLocation()) < (R * R))
			{
				return false;
			}
		}
		const TArray<TWeakObjectPtr<AT66CircusInteractable>>& Circuses = Registry ? Registry->GetCircuses() : TArray<TWeakObjectPtr<AT66CircusInteractable>>();
		for (const TWeakObjectPtr<AT66CircusInteractable>& WeakCircus : Circuses)
		{
			const AT66CircusInteractable* Circus = WeakCircus.Get();
			if (!Circus) continue;
			if (bTowerLayout)
			{
				const int32 CandidateFloor = GetTowerFloorIndexForLocation(L);
				const int32 CircusFloor = ResolveTowerFloorForActor(Circus);
				if (CandidateFloor == INDEX_NONE || CircusFloor == INDEX_NONE || CandidateFloor != CircusFloor)
				{
					continue;
				}
			}
			else if (!IsSameTowerFloor(L, Circus->GetActorLocation()))
			{
				continue;
			}
			const float R = Circus->GetSafeZoneRadius() + SafeBubbleMargin;
			if (FVector::DistSquared2D(L, Circus->GetActorLocation()) < (R * R))
			{
				return false;
			}
		}
		return true;
	};

	struct FSpawnHitResult
	{
		FVector Loc = FVector::ZeroVector;
		bool bFound = false;
	};
	auto FindSpawnLoc = [&]() -> FSpawnHitResult
	{
		for (int32 Try = 0; Try < 40; ++Try)
		{
			FVector Loc = FVector::ZeroVector;
			if (bTowerLayout)
			{
				if (!T66TowerMapTerrain::TryGetRandomGameplaySurfaceLocation(World, CachedTowerMainMapLayout, Rng, Loc))
				{
					continue;
				}
			}
			else
			{
				const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
				const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
				Loc = FVector(X, Y, SpawnZ);

				FHitResult Hit;
				const FVector Start(X, Y, TraceStartZ);
				const FVector End(X, Y, TraceEndZ);
				if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
				{
					continue;
				}
				if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
				{
					continue;
				}
				Loc = Hit.ImpactPoint;
			}

			if (IsGoodLoc(Loc))
			{
				return { Loc, true };
			}
		}
		return {};
	};

	auto SpawnOne = [&](UClass* Cls) -> AActor*
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		const FSpawnHitResult HitResult = FindSpawnLoc();
		if (!HitResult.bFound)
		{
			return nullptr;
		}
		AActor* A = World->SpawnActor<AActor>(Cls, HitResult.Loc, FRotator::ZeroRotator, P);
		if (A)
		{
			if (bTowerLayout)
			{
				const int32 FloorNumber = GetTowerFloorIndexForLocation(HitResult.Loc);
				if (FloorNumber != INDEX_NONE)
				{
					T66TrySnapActorToTowerFloor(World, A, CachedTowerMainMapLayout, FloorNumber, HitResult.Loc);
					T66AssignTowerFloorTag(A, FloorNumber);
				}
			}
			UsedLocs.Add(HitResult.Loc);
		}
		return A;
	};

	if (RngSub)
	{
		RngSub->UpdateLuckStat(RunState->GetLuckStat());
	}

	const UT66RngTuningConfig* Tuning = RngSub ? RngSub->GetTuning() : nullptr;
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI ? T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const FT66IntRange ChestCountRange = PlayerExperience
		? PlayerExperience->GetDifficultyChestCountRange(Difficulty)
		: FT66IntRange{ 4, 10 };
	const FT66IntRange WheelCountRange = T66EnableWheelSpinSpawns
		? (PlayerExperience
			? PlayerExperience->GetDifficultyWheelCountRange(Difficulty)
			: FT66IntRange{ 5, 11 })
		: FT66IntRange{ 0, 0 };
	const FT66IntRange CrateCountRange = PlayerExperience
		? PlayerExperience->GetDifficultyCrateCountRange(Difficulty)
		: FT66IntRange{ 3, 6 };

	// Luck-affected counts use central tuning. Locations are still stage-seeded (not luck-affected).
	const int32 CountFountains = (RngSub && Tuning) ? RngSub->RollIntRangeBiased(Tuning->TreesPerStage, Rng) : Rng.RandRange(2, 5);
	const int32 FountainsDrawIndex = (RngSub && Tuning) ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
	const int32 FountainsPreDrawSeed = (RngSub && Tuning) ? RngSub->GetLastRunPreDrawSeed() : 0;
	int32 CountChests = 0;
	int32 ChestsDrawIndex = INDEX_NONE;
	int32 ChestsPreDrawSeed = 0;
	if (bTowerLayout)
	{
		for (const int32 FloorNumber : TowerGameplayFloorNumbers)
		{
			CountChests += TowerChestCountByFloor.FindRef(FloorNumber);
		}
		ChestsPreDrawSeed = RunSeed + StageNum * 1901 + 39;
	}
	else
	{
		CountChests = RngSub ? RngSub->RollIntRangeBiased(ChestCountRange, Rng) : Rng.RandRange(FMath::Min(ChestCountRange.Min, ChestCountRange.Max), FMath::Max(ChestCountRange.Min, ChestCountRange.Max));
		ChestsDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		ChestsPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
	}
	const int32 CountWheels = RngSub ? RngSub->RollIntRangeBiased(WheelCountRange, Rng) : Rng.RandRange(FMath::Min(WheelCountRange.Min, WheelCountRange.Max), FMath::Max(WheelCountRange.Min, WheelCountRange.Max));
	const int32 WheelsDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
	const int32 WheelsPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
	int32 CountCrates = 0;
	int32 CratesDrawIndex = INDEX_NONE;
	int32 CratesPreDrawSeed = 0;
	if (bTowerLayout)
	{
		for (const int32 FloorNumber : TowerGameplayFloorNumbers)
		{
			CountCrates += TowerCrateCountByFloor.FindRef(FloorNumber);
		}
		CratesPreDrawSeed = RunSeed + StageNum * 1901 + 40;
	}
	else
	{
		CountCrates = RngSub ? RngSub->RollIntRangeBiased(CrateCountRange, Rng) : Rng.RandRange(FMath::Min(CrateCountRange.Min, CrateCountRange.Max), FMath::Max(CrateCountRange.Min, CrateCountRange.Max));
		CratesDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		CratesPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
	}

	// Luck Rating tracking (quantity).
	if (RunState)
	{
		const int32 FountainsMin = (Tuning ? Tuning->TreesPerStage.Min : 2);
		const int32 FountainsMax = (Tuning ? Tuning->TreesPerStage.Max : 5);
		const int32 ChestsMin = bTowerLayout ? TowerGameplayFloorNumbers.Num() : FMath::Min(ChestCountRange.Min, ChestCountRange.Max);
		const int32 ChestsMax = bTowerLayout ? (TowerGameplayFloorNumbers.Num() * 3) : FMath::Max(ChestCountRange.Min, ChestCountRange.Max);
		const int32 WheelsMin = FMath::Min(WheelCountRange.Min, WheelCountRange.Max);
		const int32 WheelsMax = FMath::Max(WheelCountRange.Min, WheelCountRange.Max);
		const int32 CratesMin = bTowerLayout ? TowerGameplayFloorNumbers.Num() : FMath::Min(CrateCountRange.Min, CrateCountRange.Max);
		const int32 CratesMax = bTowerLayout ? (TowerGameplayFloorNumbers.Num() * 3) : FMath::Max(CrateCountRange.Min, CrateCountRange.Max);
		RunState->RecordLuckQuantityRoll(FName(TEXT("FountainsPerStage")), CountFountains, FountainsMin, FountainsMax, FountainsDrawIndex, FountainsPreDrawSeed);
		RunState->RecordLuckQuantityRoll(FName(TEXT("ChestsPerStage")), CountChests, ChestsMin, ChestsMax, ChestsDrawIndex, ChestsPreDrawSeed);
		RunState->RecordLuckQuantityRoll(FName(TEXT("WheelsPerStage")), CountWheels, WheelsMin, WheelsMax, WheelsDrawIndex, WheelsPreDrawSeed);
		RunState->RecordLuckQuantityRoll(FName(TEXT("CratesPerStage")), CountCrates, CratesMin, CratesMax, CratesDrawIndex, CratesPreDrawSeed);
	}

	// Not luck-affected (for now).
	const int32 CountTotems = Rng.RandRange(4, 10);
	int32 RemainingFountains = CountFountains;
	int32 RemainingChests = CountChests;
	int32 RemainingWheels = CountWheels;
	int32 RemainingCrates = CountCrates;
	int32 RemainingTotems = CountTotems;

	auto ConfigureChest = [&](AT66ChestInteractable* Chest)
	{
		if (!Chest)
		{
			return;
		}

		const FT66RarityWeights Weights = PlayerExperience
			? PlayerExperience->GetDifficultyChestRarityWeights(Difficulty)
			: FT66RarityWeights{};
		const float ChestMimicChance = PlayerExperience ? PlayerExperience->GetDifficultyChestMimicChance(Difficulty) : 0.20f;
		Chest->bIsMimic = RngSub ? RngSub->RollChance01(ChestMimicChance) : (Rng.GetFraction() < ChestMimicChance);
		const int32 ChestMimicDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		const int32 ChestMimicPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
		const ET66Rarity Rarity = (RngSub && PlayerExperience) ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
		const int32 ChestRarityDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		const int32 ChestRarityPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
		Chest->SetRarity(Rarity);
		if (RunState)
		{
			RunState->RecordLuckQuantityBool(
				FName(TEXT("ChestMimicAvoided")),
				!Chest->bIsMimic,
				1.f - FMath::Clamp(ChestMimicChance, 0.f, 1.f),
				ChestMimicDrawIndex,
				ChestMimicPreDrawSeed);
			RunState->RecordLuckQualityRarity(FName(TEXT("ChestRarity")), Rarity, ChestRarityDrawIndex, ChestRarityPreDrawSeed, &Weights);
		}
	};

	auto ConfigureWheel = [&](AT66WheelSpinInteractable* Wheel)
	{
		if (!Wheel)
		{
			return;
		}

		const FT66RarityWeights Weights = PlayerExperience
			? PlayerExperience->GetDifficultyWheelRarityWeights(Difficulty)
			: FT66RarityWeights{};
		const ET66Rarity Rarity = (RngSub && PlayerExperience) ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
		const int32 WheelRarityDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		const int32 WheelRarityPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
		Wheel->SetRarity(Rarity);
		if (RunState)
		{
			RunState->RecordLuckQualityRarity(FName(TEXT("WheelRarity")), Rarity, WheelRarityDrawIndex, WheelRarityPreDrawSeed, &Weights);
		}
	};

	auto ConfigureCrate = [&](AT66CrateInteractable* Crate)
	{
		if (!Crate)
		{
			return;
		}

		const FT66RarityWeights Weights = PlayerExperience
			? PlayerExperience->GetDifficultyCrateRarityWeights(Difficulty)
			: FT66RarityWeights{};
		const ET66Rarity Rarity = (RngSub && PlayerExperience) ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
		const int32 CrateRarityDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		const int32 CrateRarityPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
		Crate->SetRarity(Rarity);
		if (RunState)
		{
			RunState->RecordLuckQualityRarity(FName(TEXT("CrateRarity")), Rarity, CrateRarityDrawIndex, CrateRarityPreDrawSeed, &Weights);
		}
	};

	auto ConfigureTotem = [&](AT66DifficultyTotem* Totem)
	{
		if (Totem)
		{
			Totem->SetRarity(FT66RarityUtil::RollDefaultRarity(Rng));
		}
	};

	auto FindExistingTaggedActor = [&](const FName Tag) -> AActor*
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* ExistingActor = *It;
			if (ExistingActor && ExistingActor->ActorHasTag(Tag))
			{
				return ExistingActor;
			}
		}

		return nullptr;
	};

	if (bTowerLayout)
	{
		static constexpr float TowerCircusChancePerFloor = 0.38f;
		static constexpr float TowerQuickReviveChancePerFloor = 0.34f;
		FRandomStream TowerFloorRng(RunSeed + StageNum * 1901 + 77);
		TArray<int32, TInlineAllocator<4>> GameplayFloorNumbers = TowerGameplayFloorNumbers;

		auto ResnapTowerActorToFloor = [&](AActor* Actor, const int32 FloorNumber) -> bool
		{
			if (!Actor)
			{
				return false;
			}

			T66TrySnapActorToTowerFloor(World, Actor, CachedTowerMainMapLayout, FloorNumber, Actor->GetActorLocation());
			T66AssignTowerFloorTag(Actor, FloorNumber);
			const int32 ResolvedFloor = GetTowerFloorIndexForLocation(Actor->GetActorLocation());
			return ResolvedFloor == INDEX_NONE || ResolvedFloor == FloorNumber;
		};

		auto TryFindTowerFloorLocation = [&](const int32 FloorNumber, const int32 SeedOffset, const float EdgePadding, const float HolePadding, FVector& OutLocation) -> bool
		{
			FRandomStream FloorRng(RunSeed + StageNum * 1901 + SeedOffset + FloorNumber * 53);
			for (int32 Attempt = 0; Attempt < 24; ++Attempt)
			{
				if (!T66TowerMapTerrain::TryGetRandomSurfaceLocationOnFloor(
					World,
					CachedTowerMainMapLayout,
					FloorNumber,
					FloorRng,
					OutLocation,
					EdgePadding,
					HolePadding))
				{
					continue;
				}

				if (GetTowerFloorIndexForLocation(OutLocation) != FloorNumber)
				{
					continue;
				}

				if (IsGoodLoc(OutLocation))
				{
					return true;
				}
			}

			return false;
		};

		FActorSpawnParameters OccupantSpawnParams;
		OccupantSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		auto SpawnTowerActorOnFloor = [&](UClass* Cls, const int32 FloorNumber, const int32 SeedOffset, const float EdgePadding, const float HolePadding) -> AActor*
		{
			for (int32 SpawnAttempt = 0; SpawnAttempt < 5; ++SpawnAttempt)
			{
				FVector SpawnLoc = FVector::ZeroVector;
				if (!TryFindTowerFloorLocation(FloorNumber, SeedOffset + (SpawnAttempt * 97), EdgePadding, HolePadding, SpawnLoc))
				{
					continue;
				}

				AActor* SpawnedActor = World->SpawnActor<AActor>(Cls, SpawnLoc, FRotator::ZeroRotator, OccupantSpawnParams);
				if (!SpawnedActor)
				{
					continue;
				}

				if (!ResnapTowerActorToFloor(SpawnedActor, FloorNumber))
				{
					SpawnedActor->Destroy();
					continue;
				}

				UsedLocs.Add(SpawnedActor->GetActorLocation());
				return SpawnedActor;
			}

			return nullptr;
		};

		int32 GuaranteedUtilityFountains = 0;
		int32 GuaranteedUtilityTotems = 0;
		for (const int32 FloorNumber : GameplayFloorNumbers)
		{
			const int32 FloorChestCount = TowerChestCountByFloor.FindRef(FloorNumber);
			for (int32 ChestIndex = 0; ChestIndex < FloorChestCount; ++ChestIndex)
			{
				for (int32 SpawnAttempt = 0; SpawnAttempt < 4; ++SpawnAttempt)
				{
					if (AT66ChestInteractable* Chest = Cast<AT66ChestInteractable>(SpawnTowerActorOnFloor(
						AT66ChestInteractable::StaticClass(),
						FloorNumber,
						7600 + (ChestIndex * 41) + (SpawnAttempt * 211),
						1100.f,
						1200.f)))
					{
						ConfigureChest(Chest);
						if (!ResnapTowerActorToFloor(Chest, FloorNumber))
						{
							Chest->Destroy();
							continue;
						}
						break;
					}
				}
			}

			const int32 FloorCrateCount = TowerCrateCountByFloor.FindRef(FloorNumber);
			for (int32 CrateIndex = 0; CrateIndex < FloorCrateCount; ++CrateIndex)
			{
				for (int32 SpawnAttempt = 0; SpawnAttempt < 4; ++SpawnAttempt)
				{
					if (AT66CrateInteractable* Crate = Cast<AT66CrateInteractable>(SpawnTowerActorOnFloor(
						AT66CrateInteractable::StaticClass(),
						FloorNumber,
						7700 + (CrateIndex * 41) + (SpawnAttempt * 211),
						1100.f,
						1200.f)))
					{
						ConfigureCrate(Crate);
						if (!ResnapTowerActorToFloor(Crate, FloorNumber))
						{
							Crate->Destroy();
							continue;
						}
						break;
					}
				}
			}

			if ((FloorNumber % 2) == 0)
			{
				if (SpawnTowerActorOnFloor(AT66FountainOfLifeInteractable::StaticClass(), FloorNumber, 7800, 1200.f, 1300.f))
				{
					++GuaranteedUtilityFountains;
				}
			}
			else
			{
				if (AT66DifficultyTotem* Totem = Cast<AT66DifficultyTotem>(SpawnTowerActorOnFloor(AT66DifficultyTotem::StaticClass(), FloorNumber, 7900, 1200.f, 1300.f)))
				{
					ConfigureTotem(Totem);
					ResnapTowerActorToFloor(Totem, FloorNumber);
					++GuaranteedUtilityTotems;
				}
			}
		}

		RemainingChests = 0;
		RemainingCrates = 0;
		RemainingFountains = FMath::Max(0, RemainingFountains - GuaranteedUtilityFountains);
		RemainingTotems = FMath::Max(0, RemainingTotems - GuaranteedUtilityTotems);
		bool bQuickReviveSpawned = false;

		for (const int32 FloorNumber : GameplayFloorNumbers)
		{
			const FName CircusTag(*FString::Printf(TEXT("T66_Tower_Circus_%02d"), FloorNumber));
			if (!FindExistingTaggedActor(CircusTag))
			{
				const int32 CircusPreDrawSeed = TowerFloorRng.GetCurrentSeed();
				const bool bSpawnCircus = TowerFloorRng.GetFraction() < TowerCircusChancePerFloor;
				if (RunState)
				{
					RunState->RecordLuckQuantityBool(
						FName(TEXT("TowerCircusFloorSpawned")),
						bSpawnCircus,
						TowerCircusChancePerFloor,
						INDEX_NONE,
						CircusPreDrawSeed);
				}
				if (bSpawnCircus)
				{
					FVector CircusLoc = FVector::ZeroVector;
					if (TryFindTowerFloorLocation(FloorNumber, 5100, 1800.f, 2200.f, CircusLoc))
					{
						if (AT66CircusInteractable* Circus = World->SpawnActor<AT66CircusInteractable>(
							AT66CircusInteractable::StaticClass(), CircusLoc, FRotator::ZeroRotator, OccupantSpawnParams))
						{
							T66TrySnapActorToTowerFloor(World, Circus, CachedTowerMainMapLayout, FloorNumber, CircusLoc);
							T66AssignTowerFloorTag(Circus, FloorNumber);
							Circus->ConfigureCompactTowerVariant();
							Circus->Tags.AddUnique(CircusTag);
							UsedLocs.Add(CircusLoc);
						}
					}
				}
			}

			const FName QuickReviveTag(*FString::Printf(TEXT("T66_Tower_QuickRevive_%02d"), FloorNumber));
			if (!FindExistingTaggedActor(QuickReviveTag))
			{
				const int32 QuickRevivePreDrawSeed = TowerFloorRng.GetCurrentSeed();
				const bool bSpawnQuickRevive = TowerFloorRng.GetFraction() < TowerQuickReviveChancePerFloor;
				if (RunState)
				{
					RunState->RecordLuckQuantityBool(
						FName(TEXT("TowerQuickReviveFloorSpawned")),
						bSpawnQuickRevive,
						TowerQuickReviveChancePerFloor,
						INDEX_NONE,
						QuickRevivePreDrawSeed);
				}
				if (bSpawnQuickRevive)
				{
					FVector QuickReviveLoc = FVector::ZeroVector;
					if (TryFindTowerFloorLocation(FloorNumber, 6100, 1300.f, 1700.f, QuickReviveLoc))
					{
						if (AT66QuickReviveVendingMachine* QuickReviveMachine = World->SpawnActor<AT66QuickReviveVendingMachine>(
							AT66QuickReviveVendingMachine::StaticClass(), QuickReviveLoc, FRotator::ZeroRotator, OccupantSpawnParams))
						{
							T66TrySnapActorToTowerFloor(World, QuickReviveMachine, CachedTowerMainMapLayout, FloorNumber, QuickReviveLoc);
							T66AssignTowerFloorTag(QuickReviveMachine, FloorNumber);
							QuickReviveMachine->Tags.AddUnique(QuickReviveTag);
							UsedLocs.Add(QuickReviveLoc);
							bQuickReviveSpawned = true;
						}
					}
				}
			}
		}

		if (!bQuickReviveSpawned && GameplayFloorNumbers.Num() > 0)
		{
			const int32 GuaranteedQuickReviveFloor = GameplayFloorNumbers[TowerFloorRng.RandRange(0, GameplayFloorNumbers.Num() - 1)];
			FVector QuickReviveLoc = FVector::ZeroVector;
			if (TryFindTowerFloorLocation(GuaranteedQuickReviveFloor, 6900, 1300.f, 1700.f, QuickReviveLoc))
			{
				if (AT66QuickReviveVendingMachine* QuickReviveMachine = World->SpawnActor<AT66QuickReviveVendingMachine>(
					AT66QuickReviveVendingMachine::StaticClass(), QuickReviveLoc, FRotator::ZeroRotator, OccupantSpawnParams))
				{
					T66TrySnapActorToTowerFloor(World, QuickReviveMachine, CachedTowerMainMapLayout, GuaranteedQuickReviveFloor, QuickReviveLoc);
					T66AssignTowerFloorTag(QuickReviveMachine, GuaranteedQuickReviveFloor);
					QuickReviveMachine->Tags.AddUnique(FName(*FString::Printf(TEXT("T66_Tower_QuickRevive_%02d"), GuaranteedQuickReviveFloor)));
					UsedLocs.Add(QuickReviveLoc);
				}
			}
		}

		const FName SaintTag(TEXT("T66_Tower_Saint"));
		if (!FindExistingTaggedActor(SaintTag))
		{
			for (int32 Index = GameplayFloorNumbers.Num() - 1; Index > 0; --Index)
			{
				const int32 SwapIndex = TowerFloorRng.RandRange(0, Index);
				if (SwapIndex != Index)
				{
					GameplayFloorNumbers.Swap(Index, SwapIndex);
				}
			}

			for (const int32 FloorNumber : GameplayFloorNumbers)
			{
				FVector SaintLoc = FVector::ZeroVector;
				if (!TryFindTowerFloorLocation(FloorNumber, 7100, 1800.f, 2200.f, SaintLoc))
				{
					continue;
				}

				if (AT66SaintNPC* Saint = World->SpawnActor<AT66SaintNPC>(
					AT66SaintNPC::StaticClass(), SaintLoc, FRotator::ZeroRotator, OccupantSpawnParams))
				{
					T66TrySnapActorToTowerFloor(World, Saint, CachedTowerMainMapLayout, FloorNumber, SaintLoc);
					T66AssignTowerFloorTag(Saint, FloorNumber);
					Saint->Tags.AddUnique(SaintTag);
					Saint->Tags.AddUnique(FName(*FString::Printf(TEXT("T66_Tower_Saint_%02d"), FloorNumber)));
					UsedLocs.Add(SaintLoc);
				}
				break;
			}
		}

		for (const int32 FloorNumber : GameplayFloorNumbers)
		{
			if (RemainingChests > 0)
			{
				if (AT66ChestInteractable* Chest = Cast<AT66ChestInteractable>(SpawnTowerActorOnFloor(AT66ChestInteractable::StaticClass(), FloorNumber, 8100, 1300.f, 1700.f)))
				{
					ConfigureChest(Chest);
					if (!ResnapTowerActorToFloor(Chest, FloorNumber))
					{
						Chest->Destroy();
						continue;
					}
					--RemainingChests;
				}
			}

			if (RemainingWheels > 0)
			{
				if (AT66WheelSpinInteractable* Wheel = Cast<AT66WheelSpinInteractable>(SpawnTowerActorOnFloor(AT66WheelSpinInteractable::StaticClass(), FloorNumber, 8200, 1300.f, 1700.f)))
				{
					ConfigureWheel(Wheel);
					if (!ResnapTowerActorToFloor(Wheel, FloorNumber))
					{
						Wheel->Destroy();
						continue;
					}
					--RemainingWheels;
					continue;
				}
			}

			if (RemainingCrates > 0)
			{
				if (AT66CrateInteractable* Crate = Cast<AT66CrateInteractable>(SpawnTowerActorOnFloor(AT66CrateInteractable::StaticClass(), FloorNumber, 8300, 1300.f, 1700.f)))
				{
					ConfigureCrate(Crate);
					if (!ResnapTowerActorToFloor(Crate, FloorNumber))
					{
						Crate->Destroy();
						continue;
					}
					--RemainingCrates;
					continue;
				}
			}

			if (RemainingFountains > 0)
			{
				if (SpawnTowerActorOnFloor(AT66FountainOfLifeInteractable::StaticClass(), FloorNumber, 8400, 1300.f, 1700.f))
				{
					--RemainingFountains;
					continue;
				}
			}

			if (RemainingTotems > 0)
			{
				if (AT66DifficultyTotem* Totem = Cast<AT66DifficultyTotem>(SpawnTowerActorOnFloor(AT66DifficultyTotem::StaticClass(), FloorNumber, 8500, 1300.f, 1700.f)))
				{
					ConfigureTotem(Totem);
					if (!ResnapTowerActorToFloor(Totem, FloorNumber))
					{
						Totem->Destroy();
						continue;
					}
					--RemainingTotems;
				}
			}
		}
	}

	for (int32 i = 0; i < RemainingFountains; ++i)
	{
		SpawnOne(AT66FountainOfLifeInteractable::StaticClass());
	}
	for (int32 i = 0; i < RemainingChests; ++i)
	{
		if (AT66ChestInteractable* Chest = Cast<AT66ChestInteractable>(SpawnOne(AT66ChestInteractable::StaticClass())))
		{
			ConfigureChest(Chest);
		}
	}
	for (int32 i = 0; i < RemainingWheels; ++i)
	{
		if (AT66WheelSpinInteractable* Wheel = Cast<AT66WheelSpinInteractable>(SpawnOne(AT66WheelSpinInteractable::StaticClass())))
		{
			ConfigureWheel(Wheel);
		}
	}

	if (!IsColiseumStage() && !IsLabLevel())
	{
		FVector TractorLoc = FVector::ZeroVector;
		bool bShouldSpawnGuaranteedTractor = false;
		if (!bUsingMainMapTerrain)
		{
			static constexpr float GuaranteedTractorOffsetX = 850.f; // just east of the start gate, outside the start corridor
			const FVector StartGateLoc = T66GameplayLayout::GetStartGateLocation();
			const float TractorX = StartGateLoc.X + GuaranteedTractorOffsetX;
			const float TractorY = StartGateLoc.Y;
			FHitResult TractorHit;
			const FVector TractorTraceStart(TractorX, TractorY, 8000.f);
			const FVector TractorTraceEnd(TractorX, TractorY, -16000.f);
			TractorLoc = FVector(TractorX, TractorY, SpawnZ);
			if (World->LineTraceSingleByChannel(TractorHit, TractorTraceStart, TractorTraceEnd, ECC_WorldStatic))
			{
				TractorLoc = TractorHit.ImpactPoint;
			}
			bShouldSpawnGuaranteedTractor = true;
		}

		if (bShouldSpawnGuaranteedTractor)
		{
			FActorSpawnParameters TractorSpawnParams;
			TractorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AT66PilotableTractor* GuaranteedTractor = World->SpawnActor<AT66PilotableTractor>(
				AT66PilotableTractor::StaticClass(), TractorLoc, FRotator::ZeroRotator, TractorSpawnParams))
			{
				UsedLocs.Add(TractorLoc);
			}
		}

		if (!bUsingMainMapTerrain)
		{
			SpawnModelShowcaseRow();
		}
	}

	// Teleport pads are deprecated and no longer part of stage population.

	for (int32 i = 0; i < RemainingCrates; ++i)
	{
		if (AT66CrateInteractable* Crate = Cast<AT66CrateInteractable>(SpawnOne(AT66CrateInteractable::StaticClass())))
		{
			ConfigureCrate(Crate);
		}
	}

	for (int32 i = 0; i < RemainingTotems; ++i)
	{
		if (AT66DifficultyTotem* Totem = Cast<AT66DifficultyTotem>(SpawnOne(AT66DifficultyTotem::StaticClass())))
		{
			ConfigureTotem(Totem);
		}
	}

	if (bTowerLayout)
	{
		if (UT66TrapSubsystem* TrapSubsystem = World->GetSubsystem<UT66TrapSubsystem>())
		{
			TrapSubsystem->SpawnTowerStageTraps(CachedTowerMainMapLayout, StageNum, Difficulty, RunSeed);
		}
	}

	bWorldInteractablesSpawnedForStage = true;
}

void AT66GameMode::SpawnModelShowcaseRow()
{
	if (IsColiseumStage() || IsLabLevel())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	struct FShowcaseMeshSpec
	{
		const TCHAR* MeshPath = nullptr;
		FVector Offset = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		FVector Scale = FVector(1.f, 1.f, 1.f);
		FName Tag = NAME_None;
	};

	const FVector StartAreaCenter = T66GameplayLayout::GetStartAreaCenter();
	const FShowcaseMeshSpec ShowcaseSpecs[] = {
		{ TEXT("/Game/World/Props/StartAreaDecor/Cow.Cow"),      FVector(1800.f,  1500.f, 0.f), FRotator(0.f, -140.f, 0.f), FVector(2.f, 2.f, 2.f), FName(TEXT("T66StartAreaCow")) },
		{ TEXT("/Game/World/Props/StartAreaDecor/RoboCow.RoboCow"), FVector(1800.f, -1500.f, 0.f), FRotator(0.f,  140.f, 0.f), FVector(2.f, 2.f, 2.f), FName(TEXT("T66StartAreaRoboCow")) },
		{ TEXT("/Game/World/Props/StartAreaDecor/FullBody.FullBody"), FVector(-1800.f,    0.f, 0.f), FRotator(0.f,    0.f, 0.f), FVector(2.f, 2.f, 2.f), FName(TEXT("T66StartAreaFullBody")) },
	};

	auto FindExistingShowcaseActor = [&](const FName Tag) -> AStaticMeshActor*
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			AStaticMeshActor* Existing = *It;
			if (Existing && Existing->ActorHasTag(Tag))
			{
				return Existing;
			}
		}
		return nullptr;
	};

	for (const FShowcaseMeshSpec& Spec : ShowcaseSpecs)
	{
		if (Spec.MeshPath == nullptr || Spec.Tag.IsNone())
		{
			continue;
		}

		AStaticMeshActor* Actor = FindExistingShowcaseActor(Spec.Tag);
		if (!Actor)
		{
			FVector SpawnLoc = StartAreaCenter + Spec.Offset;
			FHitResult Hit;
			const FVector TraceStart = SpawnLoc + FVector(0.f, 0.f, 8000.f);
			const FVector TraceEnd = SpawnLoc - FVector(0.f, 0.f, 16000.f);
			if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
			{
				SpawnLoc = Hit.ImpactPoint;
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Actor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLoc, Spec.Rotation, SpawnParams);
			if (!Actor)
			{
				continue;
			}
			Actor->Tags.AddUnique(Spec.Tag);
		}

		TSoftObjectPtr<UStaticMesh> MeshSoft(FSoftObjectPath(Spec.MeshPath));
		UStaticMesh* Mesh = MeshSoft.LoadSynchronous();
		if (!Mesh)
		{
			Actor->Destroy();
			continue;
		}

		T66_SetStaticMeshActorMobility(Actor, EComponentMobility::Movable);
		if (UStaticMeshComponent* MeshComponent = Actor->GetStaticMeshComponent())
		{
			MeshComponent->SetStaticMesh(Mesh);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			MeshComponent->SetGenerateOverlapEvents(false);
			MeshComponent->SetRelativeScale3D(Spec.Scale);
			FT66VisualUtil::GroundMeshToActorOrigin(MeshComponent, Mesh);
		}
		Actor->SetActorRotation(Spec.Rotation);
		T66_SetStaticMeshActorMobility(Actor, EComponentMobility::Static);
	}
}

void AT66GameMode::SpawnIdolAltarForPlayer(AController* Player)
{
	if (IsColiseumStage() || IsLabLevel()) return;

	UWorld* World = GetWorld();
	if (!World || !Player) return;

	if (IsValid(IdolAltar))
	{
		return;
	}
	IdolAltar = nullptr;

	UGameInstance* GI = World->GetGameInstance();
	UT66IdolManagerSubsystem* IdolManager = GI ? GI->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!IdolManager)
	{
		return;
	}

	int32 SelectionBudget = 1;
	int32 CatchUpSelections = 0;
	if (IdolManager->HasCatchUpIdolPicksRemaining())
	{
		CatchUpSelections = IdolManager->GetRemainingCatchUpIdolPicks();
		SelectionBudget += CatchUpSelections;
	}

	const int32 CurrentStage = RunState ? RunState->GetCurrentStage() : 1;
	UE_LOG(
		LogT66GameMode,
		Log,
		TEXT("Spawning stage-entry idol altar for stage %d with %d total selections (%d catch-up)."),
		CurrentStage,
		SelectionBudget,
		CatchUpSelections);

	FVector SpawnLoc = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	if (T66UsesMainMapTerrainStage(World))
	{
		if (IsUsingTowerMainMapLayout())
		{
			SpawnLoc = CachedTowerMainMapLayout.StartAnchorSurfaceLocation;
		}
		else if (!TryGetMainMapStartPlacementLocation(0.f, 0.f, SpawnLoc))
		{
			return;
		}

		FVector Center = FVector::ZeroVector;
		FVector InwardDirection = FVector::ForwardVector;
		FVector SideDirection = FVector::RightVector;
		float CellSize = 0.f;
		if (TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize) && !InwardDirection.IsNearlyZero())
		{
			SpawnRotation = (-InwardDirection).Rotation();
		}
	}
	else
	{
		SpawnLoc = T66GameplayLayout::GetStartAreaCenter();

		FHitResult Hit;
		const FVector TraceStart = SpawnLoc + FVector(0.f, 0.f, 3000.f);
		const FVector TraceEnd = SpawnLoc - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic)
			|| World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility))
		{
			SpawnLoc = Hit.ImpactPoint;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	IdolAltar = World->SpawnActor<AT66IdolAltar>(AT66IdolAltar::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams);
	if (IdolAltar)
	{
		if (IsUsingTowerMainMapLayout())
		{
			T66TrySnapActorToTowerFloor(World, IdolAltar, CachedTowerMainMapLayout, CachedTowerMainMapLayout.StartFloorNumber, SpawnLoc);
			T66AssignTowerFloorTag(IdolAltar, CachedTowerMainMapLayout.StartFloorNumber);
		}
		else
		{
			TrySnapActorToTerrainAtLocation(IdolAltar, SpawnLoc);
		}

		if (APawn* PlayerPawn = Player->GetPawn())
		{
			T66FaceActorTowardLocation2D(IdolAltar, PlayerPawn->GetActorLocation());
			if (RunState && RunState->GetCurrentStage() <= 1)
			{
				FRotator FacingRotation = PlayerPawn->GetActorRotation();
				if (T66TryBuildFacingRotation2D(PlayerPawn->GetActorLocation(), IdolAltar->GetActorLocation(), FacingRotation))
				{
					T66SyncPawnAndControllerRotation(PlayerPawn, Player, FacingRotation);
				}
			}
		}

		IdolAltar->RemainingSelections = FMath::Max(1, SelectionBudget);
		IdolAltar->CatchUpSelectionsRemaining = FMath::Max(0, CatchUpSelections);
	}
}

void AT66GameMode::SpawnIdolVFXTestTargets()
{
	if (!bSpawnIdolVFXTestTargetsAtStageStart || IsColiseumStage() || IsLabLevel())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (AT66EnemyBase* ExistingTarget : IdolVFXTestTargets)
	{
		if (ExistingTarget)
		{
			ExistingTarget->Destroy();
		}
	}
	IdolVFXTestTargets.Reset();

	TArray<FVector> SpawnLocations;
	SpawnLocations.Reserve(3);

	APawn* HeroPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	const FVector HeroLocation = HeroPawn ? HeroPawn->GetActorLocation() : FVector::ZeroVector;
	FVector ForwardDirection = HeroPawn ? HeroPawn->GetActorForwardVector().GetSafeNormal2D() : FVector::ZeroVector;
	FVector SideDirection = FVector::RightVector;
	float CellSize = 400.f;
	FVector ThresholdMidpoint = FVector::ZeroVector;
	bool bHasMainMapThreshold = false;
	if (T66UsesMainMapTerrainStage(World))
	{
		FVector Center = FVector::ZeroVector;
		FVector InwardDirection = FVector::ForwardVector;
		if (TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize))
		{
			ForwardDirection = InwardDirection.GetSafeNormal2D();
			if (!MainMapStartAnchorSurfaceLocation.IsNearlyZero() && !MainMapStartPathSurfaceLocation.IsNearlyZero())
			{
				ThresholdMidpoint = (MainMapStartAnchorSurfaceLocation + MainMapStartPathSurfaceLocation) * 0.5f;
				bHasMainMapThreshold = true;
			}
		}
	}
	if (ForwardDirection.IsNearlyZero() && IdolAltar && !HeroLocation.IsNearlyZero())
	{
		ForwardDirection = (IdolAltar->GetActorLocation() - HeroLocation).GetSafeNormal2D();
	}
	if (ForwardDirection.IsNearlyZero())
	{
		ForwardDirection = FVector::ForwardVector;
	}

	SideDirection = FVector::CrossProduct(FVector::UpVector, ForwardDirection).GetSafeNormal();
	if (SideDirection.IsNearlyZero())
	{
		SideDirection = FVector::RightVector;
	}

	const FVector StartCenter = !MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
		? MainMapStartAreaCenterSurfaceLocation
		: T66GameplayLayout::GetStartAreaCenter(DefaultSpawnHeight);
	const FVector AnchorCenter = IdolAltar ? IdolAltar->GetActorLocation() : StartCenter;
	FVector FrontCenter = AnchorCenter + ForwardDirection * 650.f + FVector(0.f, 0.f, 180.f);
	if (bHasMainMapThreshold)
	{
		const float SafeSideOffset = FMath::Max(140.f, CellSize * 0.40f);
		FrontCenter = ThresholdMidpoint - ForwardDirection * SafeSideOffset + FVector(0.f, 0.f, 180.f);
	}
	static constexpr float TargetSideSpacing = 340.f;
	SpawnLocations.Add(FrontCenter - SideDirection * TargetSideSpacing);
	SpawnLocations.Add(FrontCenter);
	SpawnLocations.Add(FrontCenter + SideDirection * TargetSideSpacing);

	static const FName TargetIds[] = {
		FName(TEXT("Cow")),
		FName(TEXT("Pig")),
		FName(TEXT("Goat"))
	};

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(TargetIds) && SpawnLocations.IsValidIndex(Index); ++Index)
	{
		FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocations[Index]);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AT66EnemyBase* TargetEnemy = World->SpawnActorDeferred<AT66EnemyBase>(
			AT66EnemyBase::StaticClass(),
			SpawnTransform,
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!TargetEnemy)
		{
			continue;
		}

		TargetEnemy->CharacterVisualID = TargetIds[Index];
		TargetEnemy->MobID = TargetIds[Index];
		TargetEnemy->MaxHP = 20000;
		TargetEnemy->CurrentHP = 20000;
		TargetEnemy->TouchDamageHearts = 0;
		TargetEnemy->PointValue = 0;
		TargetEnemy->XPValue = 0;
		TargetEnemy->bDropsLoot = false;
		TargetEnemy->Armor = 0.f;

		UGameplayStatics::FinishSpawningActor(TargetEnemy, SpawnTransform);
		TargetEnemy->ConfigureAsMob(TargetIds[Index]);
		TrySnapActorToTerrainAtLocation(TargetEnemy, SpawnLocations[Index]);

		if (UCharacterMovementComponent* Movement = TargetEnemy->GetCharacterMovement())
		{
			Movement->MaxWalkSpeed = 0.f;
			Movement->StopMovementImmediately();
			Movement->DisableMovement();
			Movement->SetMovementMode(MOVE_None);
		}

		if (!HeroLocation.IsNearlyZero())
		{
			FVector Facing = HeroLocation - TargetEnemy->GetActorLocation();
			Facing.Z = 0.f;
			if (!Facing.Normalize())
			{
				Facing = FVector::BackwardVector;
			}
			TargetEnemy->SetActorRotation(Facing.Rotation());
		}

		TargetEnemy->CurrentHP = TargetEnemy->MaxHP;
		TargetEnemy->UpdateHealthBar();
		TargetEnemy->Tags.AddUnique(FName(TEXT("IdolVFXTestTarget")));
		UE_LOG(LogT66GameMode, Log, TEXT("Spawned idol VFX test target %s at %s"), *TargetIds[Index].ToString(), *TargetEnemy->GetActorLocation().ToCompactString());
		IdolVFXTestTargets.Add(TargetEnemy);
	}
}

void AT66GameMode::SpawnIdolAltarAtLocation(const FVector& Location)
{
	if (IsColiseumStage()) return;

	UWorld* World = GetWorld();
	if (!World) return;
	if (IsValid(IdolAltar)) return;
	IdolAltar = nullptr;

	// Place near boss death, but offset so it doesn't overlap the Stage Gate.
	FVector SpawnLoc = Location + FVector(420.f, 260.f, 0.f);

	// Trace down so altar sits on the ground.
	FHitResult Hit;
	const FVector TraceStart = SpawnLoc + FVector(0.f, 0.f, 3000.f);
	const FVector TraceEnd = SpawnLoc - FVector(0.f, 0.f, 9000.f);
	if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
	{
		SpawnLoc = Hit.ImpactPoint;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	IdolAltar = World->SpawnActor<AT66IdolAltar>(AT66IdolAltar::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (IdolAltar)
	{
		IdolAltar->RemainingSelections = 1;
		IdolAltar->CatchUpSelectionsRemaining = 0;
	}
}

void AT66GameMode::SpawnStageCatchUpPlatformAndInteractables()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return;
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>();

	RunState->SetInStageCatchUp(true);

	const int32 DiffIndex = FMath::Max(0, static_cast<int32>(T66GI->SelectedDifficulty));
	const int32 StartStage = PlayerExperience
		? PlayerExperience->GetDifficultyStartStage(T66GI->SelectedDifficulty)
		: FMath::Clamp(1 + (DiffIndex * 5), 1, 23);

	const int32 GoldAmount = PlayerExperience
		? PlayerExperience->GetDifficultyStartGoldBonus(T66GI->SelectedDifficulty)
		: 200 * DiffIndex;
	const int32 LootBags = PlayerExperience
		? PlayerExperience->GetDifficultyStartLootBags(T66GI->SelectedDifficulty)
		: 2 + (DiffIndex * 2);

	const FVector PlatformCenter(-45455.f, 23636.f, -50.f);

	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AT66StageCatchUpGoldInteractable* Gold = World->SpawnActor<AT66StageCatchUpGoldInteractable>(
			AT66StageCatchUpGoldInteractable::StaticClass(),
			PlatformCenter + FVector(-260.f, -120.f, 120.f),
			FRotator::ZeroRotator,
			P);
		if (Gold)
		{
			Gold->GoldAmount = GoldAmount;
		}

		AT66StageCatchUpLootInteractable* Loot = World->SpawnActor<AT66StageCatchUpLootInteractable>(
			AT66StageCatchUpLootInteractable::StaticClass(),
			PlatformCenter + FVector(-260.f, 220.f, 120.f),
			FRotator::ZeroRotator,
			P);
		if (Loot)
		{
			Loot->LootBagCount = LootBags;
		}

		AT66StageCatchUpGate* Gate = World->SpawnActor<AT66StageCatchUpGate>(
			AT66StageCatchUpGate::StaticClass(),
			PlatformCenter + FVector(520.f, 0.f, 200.f),
			FRotator::ZeroRotator,
			P);
		if (Gate)
		{
			Gate->TargetStage = StartStage;
		}
	}
}

void AT66GameMode::SpawnStageGateAtLocation(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FVector SpawnLoc(Location.X, Location.Y, Location.Z);
	if (IsUsingTowerMainMapLayout())
	{
		if (!MainMapBossBeaconSurfaceLocation.IsNearlyZero())
		{
			SpawnLoc = MainMapBossBeaconSurfaceLocation;
		}
		else if (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero())
		{
			SpawnLoc = MainMapBossAreaCenterSurfaceLocation;
		}
	}

	// Snap to ground at the chosen XY so the gate is never floating/sunk.
	{
		FHitResult Hit;
		const FVector Start = SpawnLoc + FVector(0.f, 0.f, 3000.f);
		const FVector End = SpawnLoc - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic) ||
			World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
		{
			SpawnLoc = Hit.ImpactPoint;
		}
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66StageGate* StageGate = World->SpawnActor<AT66StageGate>(AT66StageGate::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (StageGate)
	{
		UE_LOG(LogT66GameMode, Log, TEXT("Spawned Stage Gate at boss death location"));
	}
}

void AT66GameMode::SpawnBossForCurrentStage()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (IsColiseumStage())
	{
		if (IsFinalDifficultyBossColiseumStage())
		{
			SpawnFinalDifficultyBossInColiseum();
		}
		else
		{
			SpawnAllOwedBossesInColiseum();
		}
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return;

	const int32 StageNum = RunState->GetCurrentStage();

	// Default/fallback stage config (if DT_Stages is not wired yet)
	FStageData StageData;
	StageData.StageNumber = StageNum;
	StageData.BossID = FName(*FString::Printf(TEXT("Boss_%02d"), StageNum));
	StageData.BossSpawnLocation = StageGateSpawnOffset; // far side by default

	FStageData FromDT;
	if (T66GI->GetStageData(StageNum, FromDT))
	{
		StageData = FromDT;
	}

	if (T66UsesMainMapTerrainStage(World) && !MainMapBossSpawnSurfaceLocation.IsNearlyZero())
	{
		StageData.BossSpawnLocation = MainMapBossSpawnSurfaceLocation + FVector(0.f, 0.f, 100.f);
	}
	else
	{
		// Map layout: spawn the stage boss in the dedicated boss square at the far end of the run.
		FVector BossLoc = T66GameplayLayout::GetBossAreaCenter(200.f);
		FHitResult BossHit;
		if (World->LineTraceSingleByChannel(BossHit, BossLoc + FVector(0.f, 0.f, 3000.f), BossLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
		{
			BossLoc.Z = BossHit.ImpactPoint.Z + 100.f;
		}
		StageData.BossSpawnLocation = BossLoc;
	}

	// Default/fallback boss data (if DT_Bosses is not wired yet)
	FBossData BossData;
	BossData.BossID = StageData.BossID;
	{
		// Stage-scaled fallback so all 23 stages work even if DT_Bosses isn't reimported yet.
		const int32 S = FMath::Clamp(StageNum, 1, 23);
		const float T = static_cast<float>(S - 1) / 22.f; // 0..1

		// Bosses are intended to be 1000+ HP always (BossBase will clamp too).
		BossData.MaxHP = 1000 + (S * 250);
		BossData.AwakenDistance = 900.f;
		BossData.MoveSpeed = 350.f + (S * 2.f);
		BossData.FireIntervalSeconds = FMath::Clamp(2.0f - (S * 0.015f), 0.65f, 3.5f);
		BossData.ProjectileSpeed = 900.f + (S * 15.f);
		BossData.ProjectileDamageHearts = 1 + (S / 20);

		// Visually differentiate bosses by stage (HSV wheel).
		const float Hue = FMath::Fmod(static_cast<float>(S) * 31.f, 360.f);
		FLinearColor C = FLinearColor::MakeFromHSV8(static_cast<uint8>(Hue / 360.f * 255.f), 210, 245);
		C.A = 1.f;
		// Slightly darken early, brighten later.
		C.R = FMath::Lerp(C.R * 0.85f, C.R, T);
		C.G = FMath::Lerp(C.G * 0.85f, C.G, T);
		C.B = FMath::Lerp(C.B * 0.85f, C.B, T);
		BossData.PlaceholderColor = C;
	}

	FBossData FromBossDT;
	if (!StageData.BossID.IsNone() && T66GI->GetBossData(StageData.BossID, FromBossDT))
	{
		BossData = FromBossDT;
	}

	UClass* BossClass = AT66BossBase::StaticClass();
	const bool bWantsSpecificBossClass = !BossData.BossClass.IsNull();
	const bool bBossClassLoaded = bWantsSpecificBossClass && (BossData.BossClass.Get() != nullptr);
	if (bBossClassLoaded)
	{
		if (UClass* Loaded = BossData.BossClass.Get())
		{
			if (Loaded->IsChildOf(AT66BossBase::StaticClass()))
			{
				BossClass = Loaded;
			}
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* Spawned = World->SpawnActor<AActor>(BossClass, StageData.BossSpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (AT66BossBase* Boss = Cast<AT66BossBase>(Spawned))
	{
		Boss->InitializeBoss(BossData);
		TrySnapActorToTerrain(Boss);
		StageBoss = Boss;
		SpawnBossBeaconIfNeeded();
		UE_LOG(LogT66GameMode, Log, TEXT("Spawned boss for Stage %d (BossID=%s)"), StageNum, *BossData.BossID.ToString());

		// If the boss class is a soft reference and isn't loaded yet, load asynchronously and replace the dormant boss.
		if (bWantsSpecificBossClass && !bBossClassLoaded)
		{
			const FSoftObjectPath ClassPath = BossData.BossClass.ToSoftObjectPath();
			const TWeakObjectPtr<AT66BossBase> WeakExistingBoss(Boss);
			const FBossData BossDataCopy = BossData;

			TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
				ClassPath,
				FStreamableDelegate::CreateWeakLambda(this, [this, WeakExistingBoss, BossDataCopy]()
				{
					UWorld* World2 = GetWorld();
					if (!World2) return;

					UClass* Loaded = BossDataCopy.BossClass.Get();
					if (!Loaded || !Loaded->IsChildOf(AT66BossBase::StaticClass()))
					{
						return;
					}

					AT66BossBase* ExistingBoss = WeakExistingBoss.Get();
					if (!ExistingBoss) return;
					if (ExistingBoss->GetClass() == Loaded) return;

					// Preserve the already-snapped location so the replacement doesn't float/sink.
					const FVector Loc = ExistingBoss->GetActorLocation();
					ExistingBoss->Destroy();

					FActorSpawnParameters SpawnParams2;
					SpawnParams2.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					AActor* Spawned2 = World2->SpawnActor<AActor>(Loaded, Loc, FRotator::ZeroRotator, SpawnParams2);
					if (AT66BossBase* NewBoss = Cast<AT66BossBase>(Spawned2))
					{
						NewBoss->InitializeBoss(BossDataCopy);
						TrySnapActorToTerrain(NewBoss);
						StageBoss = NewBoss;
						SpawnBossBeaconIfNeeded();
					}
				}));
			if (Handle.IsValid())
			{
				ActiveAsyncLoadHandles.Add(Handle);
			}
		}
	}
}

void AT66GameMode::EnsureLevelSetup()
{
	UE_LOG(LogT66GameMode, Log, TEXT("Checking level setup..."));

	// Destroy any Landscape or editor-placed foliage actors saved in the level (legacy from external asset packs).
	UWorld* CleanupWorld = GetWorld();
	if (CleanupWorld)
	{
		for (TActorIterator<ALandscape> It(CleanupWorld); It; ++It)
		{
			UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Destroying saved Landscape actor: %s"), *It->GetName());
			It->Destroy();
		}
		static const FName OldFoliageTag(TEXT("T66ProceduralFoliage"));
		TArray<AActor*> ToDestroy;
		for (TActorIterator<AActor> It(CleanupWorld); It; ++It)
		{
			if (It->Tags.Contains(OldFoliageTag))
			{
				ToDestroy.Add(*It);
			}
		}
		for (AActor* A : ToDestroy)
		{
			A->Destroy();
		}
		if (ToDestroy.Num() > 0)
		{
			UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Destroyed %d legacy foliage actors."), ToDestroy.Num());
		}
	}

	if (IsLabLevel())
	{
		SpawnLabFloorIfNeeded();
		SpawnLightingIfNeeded();  // Same sky and lighting as gameplay: SkyAtmosphere (blue sky), sun, sky light, fog, post process
		SpawnQuakeSkyIfNeeded();
		SpawnLabCollectorIfNeeded();
		SpawnPlayerStartIfNeeded();
		return;
	}

	SpawnFloorIfNeeded();
	SpawnColiseumArenaIfNeeded();
	SpawnTutorialArenaIfNeeded();
	SpawnStartAreaWallsIfNeeded();
	if (!T66UsesMainMapTerrainStage(CleanupWorld))
	{
		SpawnBossAreaWallsIfNeeded();
	}
	TryApplyGroundFloorMaterialToAllFloors();
	SpawnLightingIfNeeded();
	SpawnQuakeSkyIfNeeded();
	SpawnPlayerStartIfNeeded();
}

void AT66GameMode::TryApplyGroundFloorMaterialToAllFloors()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const UT66GameInstance* T66GI = GetT66GameInstance();
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	UMaterialInterface* DifficultyFloorMaterial = FT66TerrainThemeAssets::ResolveDifficultyGroundMaterial(this, Difficulty);
	if (!DifficultyFloorMaterial)
	{
		return;
	}

	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		AStaticMeshActor* A = *It;
		if (!A) continue;
		if (!T66_HasAnyFloorTag(A)) continue;
		if (UStaticMeshComponent* SMC = A->GetStaticMeshComponent())
		{
			SMC->SetMaterial(0, DifficultyFloorMaterial);
		}
	}

	// Do not touch the runtime main-map terrain actor here; it owns its own
	// difficulty-specific block/slope/dirt materials and this pass used to
	// flatten the entire terrain into one old ground material on first load.
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* A = *It;
		if (!A || Cast<AStaticMeshActor>(A)) continue;
		if (!T66_HasAnyFloorTag(A)) continue;
		if (A->ActorHasTag(T66MainMapTerrainVisualTag)) continue;

		TArray<UMeshComponent*> MeshComps;
		A->GetComponents<UMeshComponent>(MeshComps);
		for (UMeshComponent* MC : MeshComps)
		{
			if (MC)
			{
				MC->SetMaterial(0, DifficultyFloorMaterial);
			}
		}
	}
}

void AT66GameMode::SpawnTutorialArenaIfNeeded()
{
	// Tutorial Arena is an enclosed side-area inside GameplayLevel. It is only relevant for normal gameplay stages.
	if (IsColiseumStage()) return;

	UWorld* World = GetWorld();
	if (!World) return;
	if (!T66IsStandaloneTutorialMap(World)) return;

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	auto HasTag = [&](FName Tag) -> bool
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			if (It->Tags.Contains(Tag))
			{
				return true;
			}
		}
		return false;
	};

	FVector AuthoredMapMarkerLocation = FVector::ZeroVector;
	FRotator AuthoredMapMarkerRotation = FRotator::ZeroRotator;
	if (T66TryGetTaggedActorTransform(World, FName(TEXT("T66_Tutorial_MapReady")), AuthoredMapMarkerLocation, AuthoredMapMarkerRotation))
	{
		return;
	}

	// Tutorial: to the side of the main map, separated by empty space. Scaled for 100k map.
	const FVector TutorialCenter(0.f, 61364.f, -50.f);
	const FName FloorTag(TEXT("T66_Floor_Tutorial"));

	// Floor (no overlap with main)
	if (!HasTag(FloorTag))
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), TutorialCenter, FRotator::ZeroRotator, P);
		if (Floor && Floor->GetStaticMeshComponent())
		{
			Floor->Tags.Add(FloorTag);
			T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Movable);
			Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
			Floor->SetActorScale3D(FVector(80.f, 80.f, 1.f)); // 8000x8000
			T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Static);
			if (UMaterialInstanceDynamic* Mat = Floor->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
			{
				Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.14f, 0.14f, 0.16f, 1.f));
			}
			if (!SpawnedSetupActors.Contains(Floor))
			{
				SpawnedSetupActors.Add(Floor);
			}
		}
	}
}

void AT66GameMode::SpawnColiseumArenaIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	auto HasTag = [&](FName Tag) -> bool
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			if (It->Tags.Contains(Tag))
			{
				return true;
			}
		}
		return false;
	};

	// Coliseum arena: off to the side. No separate floor (hard rule: no overlapping grounds; main floor covers it).
}

bool AT66GameMode::TryComputeBossBeaconBase(FVector& OutBeaconBase) const
{
	if (IsColiseumStage() || IsLabLevel() || IsUsingTowerMainMapLayout() || !StageBoss.IsValid())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	if (T66UsesMainMapTerrainStage(World))
	{
		if (!MainMapBossBeaconSurfaceLocation.IsNearlyZero())
		{
			OutBeaconBase = MainMapBossBeaconSurfaceLocation;
			return true;
		}

		if (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero())
		{
			OutBeaconBase = MainMapBossAreaCenterSurfaceLocation;
			return true;
		}
	}

	FVector DesiredBase = StageBoss->GetActorLocation() + FVector(3400.f, 0.f, 0.f);
	DesiredBase.X = FMath::Clamp(DesiredBase.X, T66GameplayLayout::BossAreaWestX + 1000.f, T66GameplayLayout::BossPartitionEastX - 900.f);
	DesiredBase.Y = FMath::Clamp(DesiredBase.Y, -T66GameplayLayout::AreaHalfHeightY + 600.f, T66GameplayLayout::AreaHalfHeightY - 600.f);
	const float MinNormalZ = 0.88f;

	auto TryTraceBossBeaconSurface = [&](float X, float Y, FVector& OutLoc) -> bool
	{
		FHitResult Hit;
		const FVector TraceStart(X, Y, 5000.f);
		const FVector TraceEnd(X, Y, -12000.f);
		if (!World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			return false;
		}

		if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal, MinNormalZ))
		{
			return false;
		}

		OutLoc = Hit.ImpactPoint;
		return true;
	};

	FVector FallbackLoc = DesiredBase;
	{
		FHitResult FallbackHit;
		const FVector TraceStart = DesiredBase + FVector(0.f, 0.f, 5000.f);
		const FVector TraceEnd = DesiredBase - FVector(0.f, 0.f, 12000.f);
		if (World->LineTraceSingleByChannel(FallbackHit, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			FallbackLoc = FallbackHit.ImpactPoint;
		}
	}

	FVector BeaconBase = DesiredBase;
	BeaconBase.Z = 0.f;
	bool bFoundFlatSurface = false;
	for (float Radius = 0.f; Radius <= 1600.f && !bFoundFlatSurface; Radius += 220.f)
	{
		const int32 AngleSteps = (Radius <= KINDA_SMALL_NUMBER) ? 1 : 14;
		for (int32 AngleIndex = 0; AngleIndex < AngleSteps; ++AngleIndex)
		{
			const float Angle = (AngleSteps == 1) ? 0.f : (2.f * PI * static_cast<float>(AngleIndex) / static_cast<float>(AngleSteps));
			const float X = DesiredBase.X + (Radius * FMath::Cos(Angle));
			const float Y = DesiredBase.Y + (Radius * FMath::Sin(Angle));
			if (TryTraceBossBeaconSurface(X, Y, BeaconBase))
			{
				bFoundFlatSurface = true;
				break;
			}
		}
	}

	OutBeaconBase = bFoundFlatSurface ? BeaconBase : FallbackLoc;
	return true;
}

void AT66GameMode::DestroyBossBeacon()
{
	if (AActor* BeaconActor = BossBeaconActor.Get())
	{
		BeaconActor->Destroy();
	}
	BossBeaconActor = nullptr;
}

void AT66GameMode::UpdateBossBeaconTransform(bool bForceSpawnIfMissing)
{
	if (IsColiseumStage() || IsLabLevel() || IsUsingTowerMainMapLayout() || !StageBoss.IsValid())
	{
		DestroyBossBeacon();
		return;
	}

	FVector BeaconBase = FVector::ZeroVector;
	if (!TryComputeBossBeaconBase(BeaconBase))
	{
		return;
	}

	if (AActor* BeaconActor = BossBeaconActor.Get())
	{
		BeaconActor->SetActorLocation(BeaconBase, false, nullptr, ETeleportType::TeleportPhysics);
		return;
	}

	if (bForceSpawnIfMissing)
	{
		SpawnBossBeaconIfNeeded();
	}
}

void AT66GameMode::SpawnBossBeaconIfNeeded()
{
	if (IsColiseumStage() || IsLabLevel() || IsUsingTowerMainMapLayout())
	{
		DestroyBossBeacon();
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	static const FName BossBeaconTag(TEXT("T66_Boss_Beacon"));

	TArray<AActor*> ExistingBeacons;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->Tags.Contains(BossBeaconTag))
		{
			ExistingBeacons.Add(*It);
		}
	}
	for (AActor* Existing : ExistingBeacons)
	{
		if (Existing)
		{
			Existing->Destroy();
		}
	}
	BossBeaconActor = nullptr;

	// The beacon is meant to sit behind the live stage boss.
	// If the boss has not spawned yet, do not place a fallback beam in the arena.
	FVector BeaconBase = FVector::ZeroVector;
	if (!TryComputeBossBeaconBase(BeaconBase))
	{
		return;
	}

	constexpr float OuterHeight = 34000.f;
	constexpr float InnerHeight = 32000.f;
	constexpr float OuterRadius = 250.f;
	constexpr float InnerRadius = 90.f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* BeaconActor = World->SpawnActor<AActor>(AActor::StaticClass(), BeaconBase, FRotator::ZeroRotator, SpawnParams);
	if (!BeaconActor)
	{
		return;
	}

	BeaconActor->Tags.Add(BossBeaconTag);

	USceneComponent* Root = NewObject<USceneComponent>(BeaconActor, TEXT("BossBeaconRoot"));
	if (!Root)
	{
		BeaconActor->Destroy();
		return;
	}

	BeaconActor->SetRootComponent(Root);
	Root->SetMobility(EComponentMobility::Movable);
	Root->RegisterComponent();

	UStaticMesh* CylinderMesh = FT66VisualUtil::GetBasicShapeCylinder();
	UMaterialInterface* EnvUnlitBase = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
	UTexture* WhiteTexture = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));

	auto CreateBeamMaterial = [&](const FLinearColor& Tint, float Brightness) -> UMaterialInterface*
	{
		if (!EnvUnlitBase)
		{
			return nullptr;
		}

		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(EnvUnlitBase, BeaconActor);
		if (!MID)
		{
			return nullptr;
		}

		if (WhiteTexture)
		{
			MID->SetTextureParameterValue(TEXT("DiffuseColorMap"), WhiteTexture);
			MID->SetTextureParameterValue(TEXT("BaseColorTexture"), WhiteTexture);
		}
		MID->SetVectorParameterValue(TEXT("Tint"), Tint);
		MID->SetVectorParameterValue(TEXT("BaseColor"), Tint);
		MID->SetScalarParameterValue(TEXT("Brightness"), Brightness);
		return MID;
	};

	auto ConfigureBeamMesh = [&](const TCHAR* ComponentName, float Radius, float Height, const FLinearColor& Tint, float Brightness)
	{
		if (!CylinderMesh)
		{
			return;
		}

		UStaticMeshComponent* BeamMesh = NewObject<UStaticMeshComponent>(BeaconActor, ComponentName);
		if (!BeamMesh)
		{
			return;
		}

		BeamMesh->SetStaticMesh(CylinderMesh);
		BeamMesh->SetRelativeLocation(FVector(0.f, 0.f, Height * 0.5f));
		BeamMesh->SetRelativeScale3D(FVector(Radius / 50.f, Radius / 50.f, Height / 100.f));
		BeamMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BeamMesh->SetCastShadow(false);
		BeamMesh->SetReceivesDecals(false);
		BeamMesh->SetCanEverAffectNavigation(false);
		BeamMesh->SetMobility(EComponentMobility::Movable);
		BeamMesh->SetupAttachment(Root);
		if (UMaterialInterface* BeamMaterial = CreateBeamMaterial(Tint, Brightness))
		{
			BeamMesh->SetMaterial(0, BeamMaterial);
		}
		BeamMesh->RegisterComponent();
	};

	ConfigureBeamMesh(TEXT("BossBeaconOuter"), OuterRadius, OuterHeight, FLinearColor(0.70f, 0.90f, 1.00f, 1.f), 9.5f);
	ConfigureBeamMesh(TEXT("BossBeaconInner"), InnerRadius, InnerHeight, FLinearColor(1.00f, 0.93f, 0.60f, 1.f), 26.0f);

	auto ConfigureGlowLight = [&](const TCHAR* ComponentName, float HeightAlpha, float Intensity, float AttenuationRadius)
	{
		UPointLightComponent* Glow = NewObject<UPointLightComponent>(BeaconActor, ComponentName);
		if (!Glow)
		{
			return;
		}

		Glow->SetMobility(EComponentMobility::Movable);
		Glow->SetupAttachment(Root);
		Glow->SetRelativeLocation(FVector(0.f, 0.f, OuterHeight * HeightAlpha));
		Glow->SetIntensity(Intensity);
		Glow->SetLightColor(FLinearColor(1.00f, 0.92f, 0.72f));
		Glow->SetAttenuationRadius(AttenuationRadius);
		Glow->SetCastShadows(false);
		Glow->bUseInverseSquaredFalloff = false;
		Glow->LightFalloffExponent = 3.0f;
		Glow->RegisterComponent();
	};

	ConfigureGlowLight(TEXT("BossBeaconMidGlow"), 0.42f, 85000.f, 9000.f);
	ConfigureGlowLight(TEXT("BossBeaconTopGlow"), 0.86f, 125000.f, 14000.f);

	BossBeaconActor = BeaconActor;
	SpawnedSetupActors.Add(BeaconActor);
}

void AT66GameMode::SpawnStartAreaWallsIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Start area walls disabled: remove any that were saved in the level so player can leave freely.
	static const FName WallTags[] = {
		FName("T66_Wall_Start_W"), FName("T66_Wall_Start_N"), FName("T66_Wall_Start_S"),
		FName("T66_Wall_Start_E_N"), FName("T66_Wall_Start_E_S")
	};
	for (FName Tag : WallTags)
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			if (It->Tags.Contains(Tag))
			{
				It->Destroy();
				break;
			}
		}
	}
	return;

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	auto HasTag = [&](FName Tag) -> bool
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			if (It->Tags.Contains(Tag)) return true;
		}
		return false;
	};

	// Very short red wall fully around the start area. Square, with gap from miasma wall (so back wall is not on the boundary).
	// Square 4000x4000: X -17600..-13600, Y -2000..2000 (800 uu gap from miasma at -18400).
	static constexpr float FloorTopZ = 0.f;
	static constexpr float WallHeightShort = 80.f;
	static constexpr float WallThickness = 120.f;
	const float WallZ = FloorTopZ + (WallHeightShort * 0.5f);
	const float Thick = WallThickness / 100.f;
	const float Tall = WallHeightShort / 100.f;
	const FLinearColor Red(0.75f, 0.12f, 0.12f, 1.f);

	const float BoxWestX = -17600.f;
	const float BoxEastX = -13600.f;
	const float BoxNorthY = 2000.f;
	const float BoxSouthY = -2000.f;
	const float BoxWidthX = BoxEastX - BoxWestX;   // 4000
	const float BoxHeightY = BoxNorthY - BoxSouthY; // 4000
	const float OpeningHalfY = 400.f;   // gap on east for Start Gate pillars (opening Y from -400 to +400)
	const float EastNorthLen = BoxNorthY - OpeningHalfY;   // north segment: 400 to 2000 -> length 1600
	const float EastSouthLen = (-OpeningHalfY) - BoxSouthY; // south segment: -2000 to -400 -> length 1600 (was 2400, closed the gap)
	const float EastNorthCenterY = BoxNorthY - (EastNorthLen * 0.5f);
	const float EastSouthCenterY = BoxSouthY + (EastSouthLen * 0.5f);

	struct FWallSpec { FName Tag; FVector Location; FVector Scale; };
	const TArray<FWallSpec> Walls = {
		{ FName("T66_Wall_Start_W"),   FVector(BoxWestX - (WallThickness * 0.5f), 0.f, WallZ), FVector(Thick, BoxHeightY / 100.f, Tall) },
		{ FName("T66_Wall_Start_N"),   FVector(BoxWestX + (BoxWidthX * 0.5f), BoxNorthY + (WallThickness * 0.5f), WallZ), FVector(BoxWidthX / 100.f, Thick, Tall) },
		{ FName("T66_Wall_Start_S"),   FVector(BoxWestX + (BoxWidthX * 0.5f), BoxSouthY - (WallThickness * 0.5f), WallZ), FVector(BoxWidthX / 100.f, Thick, Tall) },
		{ FName("T66_Wall_Start_E_N"), FVector(BoxEastX + (WallThickness * 0.5f), EastNorthCenterY, WallZ), FVector(Thick, EastNorthLen / 100.f, Tall) },
		{ FName("T66_Wall_Start_E_S"), FVector(BoxEastX + (WallThickness * 0.5f), EastSouthCenterY, WallZ), FVector(Thick, EastSouthLen / 100.f, Tall) },
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for (const FWallSpec& Spec : Walls)
	{
		if (HasTag(Spec.Tag)) continue;
		AStaticMeshActor* Wall = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Spec.Location, FRotator::ZeroRotator, SpawnParams);
		if (!Wall || !Wall->GetStaticMeshComponent()) continue;
		Wall->Tags.Add(Spec.Tag);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Movable);
		Wall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		Wall->SetActorScale3D(Spec.Scale);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Static);
		if (UMaterialInstanceDynamic* Mat = Wall->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Red);
		}
		SpawnedSetupActors.Add(Wall);
	}
}

void AT66GameMode::SpawnBossAreaWallsIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (T66UsesMainMapTerrainStage(World))
	{
		static const FName BossWallTags[] = {
			FName("T66_Wall_Boss_W"),
			FName("T66_Wall_Boss_E"),
			FName("T66_Wall_Boss_N"),
			FName("T66_Wall_Boss_S")
		};
		for (FName Tag : BossWallTags)
		{
			for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
			{
				if (It->Tags.Contains(Tag))
				{
					It->Destroy();
					break;
				}
			}
		}
		return;
	}

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	auto HasTag = [&](FName Tag) -> bool
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
			if (It->Tags.Contains(Tag)) return true;
		return false;
	};

	const UT66GameInstance* T66GI = GetT66GameInstance();
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	UMaterialInterface* FloorMat = FT66TerrainThemeAssets::ResolveDifficultyGroundMaterial(this, Difficulty);

	// Boss area: right after the boss pillars, inside safe zone. Scaled for 100k map.
	static constexpr float FloorTopZ = 0.f;
	static constexpr float WallHeightShort = 80.f;
	static constexpr float WallThickness = 120.f;
	const float WallZ = FloorTopZ + (WallHeightShort * 0.5f);
	const float Thick = WallThickness / 100.f;
	const float Tall = WallHeightShort / 100.f;

	const float BoxWestX = 30909.f;
	const float BoxEastX = 40000.f;
	const float BoxNorthY = 4545.f;
	const float BoxSouthY = -4545.f;
	const float BoxWidthX = BoxEastX - BoxWestX;
	const float BoxHeightY = BoxNorthY - BoxSouthY;
	const float BoxCenterX = 35455.f;

	struct FWallSpec { FName Tag; FVector Location; FVector Scale; };
	const TArray<FWallSpec> Walls = {
		{ FName("T66_Wall_Boss_W"), FVector(BoxWestX - (WallThickness * 0.5f), 0.f, WallZ), FVector(Thick, BoxHeightY / 100.f, Tall) },
		{ FName("T66_Wall_Boss_E"), FVector(BoxEastX + (WallThickness * 0.5f), 0.f, WallZ), FVector(Thick, BoxHeightY / 100.f, Tall) },
		{ FName("T66_Wall_Boss_N"), FVector(BoxCenterX, BoxNorthY + (WallThickness * 0.5f), WallZ), FVector(BoxWidthX / 100.f, Thick, Tall) },
		{ FName("T66_Wall_Boss_S"), FVector(BoxCenterX, BoxSouthY - (WallThickness * 0.5f), WallZ), FVector(BoxWidthX / 100.f, Thick, Tall) },
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for (const FWallSpec& Spec : Walls)
	{
		if (HasTag(Spec.Tag)) continue;
		AStaticMeshActor* Wall = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Spec.Location, FRotator::ZeroRotator, SpawnParams);
		if (!Wall || !Wall->GetStaticMeshComponent()) continue;
		Wall->Tags.Add(Spec.Tag);
		Wall->Tags.Add(FName("T66_Floor_Main"));
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Movable);
		Wall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		Wall->SetActorScale3D(Spec.Scale);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Static);
		if (FloorMat)
		{
			Wall->GetStaticMeshComponent()->SetMaterial(0, FloorMat);
		}
		SpawnedSetupActors.Add(Wall);
	}
}

void AT66GameMode::SpawnFloorIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;
	if (IsLabLevel()) return;  // Lab uses SpawnLabFloorIfNeeded() only

	// Ground materials are loaded async: if not yet resident the floor spawns without material,
	// and the async callback at the bottom of this function re-applies once loaded.

	// Main run uses Landscape only (no main floor). Coliseum, Boost, and Tutorial are small island floors to the side.
	struct FFloorSpec
	{
		FName Tag;
		FVector Location;
		FVector Scale;
		FLinearColor Color;
	};

	auto DestroyTaggedIfExists = [&](FName Tag)
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			if (It->Tags.Contains(Tag))
			{
				It->Destroy();
				break;
			}
		}
	};
	// T66_Floor_Main is spawned by SpawnMainMapTerrain(); do not destroy it here.
	DestroyTaggedIfExists(FName("T66_Floor_Conn1"));
	DestroyTaggedIfExists(FName("T66_Floor_Conn2"));
	DestroyTaggedIfExists(FName("T66_Floor_Start"));
	DestroyTaggedIfExists(FName("T66_Floor_Boss"));
	DestroyTaggedIfExists(FName("T66_Floor_Coliseum"));
	DestroyTaggedIfExists(FName("T66_Floor_CatchUp"));
	DestroyTaggedIfExists(FName("T66_Floor_Tutorial"));

	if (T66UsesMainMapTerrainStage(World) || T66IsStandaloneTutorialMap(World))
	{
		return;
	}

	if (!T66IsStandaloneColiseumMap(World))
	{
		return;
	}

	// Standalone coliseum map: spawn only the coliseum floor.
	constexpr float IslandFloorZ = -50.f;
	const TArray<FFloorSpec> Floors = {
		{ FName("T66_Floor_Coliseum"), FVector(ColiseumCenter.X, ColiseumCenter.Y, IslandFloorZ), FVector(40.f, 40.f, 1.f), FLinearColor(0.30f, 0.30f, 0.35f, 1.f) },
	};

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	auto FindTaggedActor = [&](FName Tag) -> AStaticMeshActor*
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			if (It->Tags.Contains(Tag))
			{
				return *It;
			}
		}
		return nullptr;
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FFloorSpec& Spec : Floors)
	{
		AStaticMeshActor* Floor = FindTaggedActor(Spec.Tag);
		if (!Floor)
		{
			Floor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Spec.Location, FRotator::ZeroRotator, SpawnParams);
		}
		if (!Floor || !Floor->GetStaticMeshComponent()) continue;

		if (!Floor->Tags.Contains(Spec.Tag))
		{
			Floor->Tags.Add(Spec.Tag);
		}
		T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Movable);
		Floor->SetActorLocation(Spec.Location);
		Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		Floor->SetActorScale3D(Spec.Scale);
		T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Static);

		const UT66GameInstance* T66GI = GetT66GameInstance();
		const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
		UMaterialInterface* GroundMat = FT66TerrainThemeAssets::ResolveDifficultyGroundMaterial(this, Difficulty);
		if (GroundMat)
		{
			Floor->GetStaticMeshComponent()->SetMaterial(0, GroundMat);
		}
		else
		{
			if (UMaterialInstanceDynamic* FloorMat = Floor->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
			{
				FloorMat->SetVectorParameterValue(TEXT("BaseColor"), Spec.Color);
			}
		}

		if (!SpawnedSetupActors.Contains(Floor))
		{
			SpawnedSetupActors.Add(Floor);
		}
	}
}

// ============================================================================
// Main map terrain
// ============================================================================

static const FName T66TraversalBarrierTag(TEXT("T66_Map_TraversalBarrier"));
static const FName T66MapPlatformTag(TEXT("T66_Map_Platform"));
static const FName T66MapRampTag(TEXT("T66_Map_Ramp"));
static const FName T66FloorMainTag(TEXT("T66_Floor_Main"));

static void DestroyActorsWithTag(UWorld* World, FName Tag)
{
	if (!World)
	{
		return;
	}

	TArray<AActor*> ToDestroy;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->Tags.Contains(Tag))
		{
			ToDestroy.Add(*It);
		}
	}

	for (AActor* Actor : ToDestroy)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
}

void AT66GameMode::SpawnMainMapTerrain()
{
	UWorld* World = GetWorld();
	if (!World || IsLabLevel() || IsColiseumStage() || !T66UsesMainMapTerrainStage(World))
	{
		return;
	}

	TArray<AActor*> CleanupActors;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor)
		{
			continue;
		}

		if (Actor->Tags.Contains(T66MapPlatformTag)
			|| Actor->Tags.Contains(T66MapRampTag)
			|| Actor->Tags.Contains(T66FloorMainTag)
			|| Actor->Tags.Contains(T66TraversalBarrierTag)
			|| T66_HasAnyFloorTag(Actor))
		{
			CleanupActors.Add(Actor);
		}
	}

	for (AActor* Actor : CleanupActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}

	bTerrainCollisionReady = false;
	bMainMapCombatStarted = false;
	bWorldInteractablesSpawnedForStage = false;
	bHasMainMapSpawnSurfaceLocation = false;
	bUsingTowerMainMapLayout = false;
	CachedTowerMainMapLayout = T66TowerMapTerrain::FLayout{};
	bTowerBossEntryTriggered = false;
	bTowerBossEntryApplied = false;
	MainMapSpawnSurfaceLocation = FVector::ZeroVector;
	MainMapStartAnchorSurfaceLocation = FVector::ZeroVector;
	MainMapStartPathSurfaceLocation = FVector::ZeroVector;
	MainMapStartAreaCenterSurfaceLocation = FVector::ZeroVector;
	MainMapBossAnchorSurfaceLocation = FVector::ZeroVector;
	MainMapBossSpawnSurfaceLocation = FVector::ZeroVector;
	MainMapBossBeaconSurfaceLocation = FVector::ZeroVector;
	MainMapBossAreaCenterSurfaceLocation = FVector::ZeroVector;
	MainMapRescueAnchorLocations.Reset();
	for (AT66TowerDescentHole* Hole : TowerDescentHoles)
	{
		if (Hole)
		{
			Hole->Destroy();
		}
	}
	TowerDescentHoles.Reset();

	UT66GameInstance* GI = GetT66GameInstance();
	const FT66MapPreset Preset = T66BuildMainMapPreset(GI);
	const T66MainMapTerrain::FSettings MainMapSettings = T66MainMapTerrain::MakeSettings(Preset);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Generating main map terrain (layout=%s, seed=%d, hilliness=%.2f, grid=%d, cell=%.0f, step=%.0f, scale=%.2f)"),
		T66GetMainMapLayoutVariantLabel(Preset.LayoutVariant),
		Preset.Seed,
		Preset.FarmHilliness,
		MainMapSettings.BoardSize,
		MainMapSettings.CellSize,
		MainMapSettings.StepHeight,
		MainMapSettings.BoardScale);

	if (Preset.LayoutVariant == ET66MainMapLayoutVariant::Tower)
	{
		if (!T66TowerMapTerrain::BuildLayout(Preset, CachedTowerMainMapLayout))
		{
			UE_LOG(LogT66GameMode, Error, TEXT("[MAP] Tower main map layout generation failed (seed=%d)"), Preset.Seed);
			CachedTowerMainMapLayout = T66TowerMapTerrain::FLayout{};
			return;
		}

		bUsingTowerMainMapLayout = true;
		MainMapSpawnSurfaceLocation = CachedTowerMainMapLayout.SpawnSurfaceLocation;
		bHasMainMapSpawnSurfaceLocation = !MainMapSpawnSurfaceLocation.IsNearlyZero();
		MainMapStartAnchorSurfaceLocation = CachedTowerMainMapLayout.StartAnchorSurfaceLocation;
		MainMapStartPathSurfaceLocation = CachedTowerMainMapLayout.StartPathSurfaceLocation;
		MainMapStartAreaCenterSurfaceLocation = CachedTowerMainMapLayout.StartAreaCenterSurfaceLocation;
		MainMapBossAnchorSurfaceLocation = CachedTowerMainMapLayout.BossAnchorSurfaceLocation;
		MainMapBossSpawnSurfaceLocation = CachedTowerMainMapLayout.BossSpawnSurfaceLocation;
		MainMapBossBeaconSurfaceLocation = CachedTowerMainMapLayout.BossBeaconSurfaceLocation;
		MainMapBossAreaCenterSurfaceLocation = CachedTowerMainMapLayout.BossAreaCenterSurfaceLocation;
		MainMapRescueAnchorLocations = CachedTowerMainMapLayout.RescueAnchorLocations;

		UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Cached tower-map spawn surface at %s"), *MainMapSpawnSurfaceLocation.ToString());

		bool bTowerCollisionReady = false;
		if (!T66TowerMapTerrain::Spawn(
			World,
			CachedTowerMainMapLayout,
			GI ? GI->SelectedDifficulty : ET66Difficulty::Easy,
			SpawnParams,
			bTowerCollisionReady))
		{
			bUsingTowerMainMapLayout = false;
			CachedTowerMainMapLayout = T66TowerMapTerrain::FLayout{};
			return;
		}

		bTerrainCollisionReady = bTowerCollisionReady;
		if (bTerrainCollisionReady)
		{
			RestartPlayersMissingPawns();
		}
		return;
	}

	T66MainMapTerrain::FBoard Board;
	if (!T66MainMapTerrain::Generate(Preset, Board))
	{
		UE_LOG(LogT66GameMode, Error, TEXT("[MAP] Main map terrain generation failed to fill the board (seed=%d, occupied=%d/%d)"),
			Preset.Seed,
			Board.OccupiedCount,
			Board.Cells.Num());
		return;
	}

	MainMapSpawnSurfaceLocation = T66MainMapTerrain::GetPreferredSpawnLocation(Board, 0.f);
	bHasMainMapSpawnSurfaceLocation = true;
	MainMapRescueAnchorLocations.Add(MainMapSpawnSurfaceLocation);
	T66MainMapTerrain::TryGetCellLocation(Board, Board.StartAnchor, 0.f, MainMapStartAnchorSurfaceLocation);
	T66MainMapTerrain::TryGetCellLocation(Board, Board.StartPathCell, 0.f, MainMapStartPathSurfaceLocation);
	if (!T66MainMapTerrain::TryGetRegionCenterLocation(Board, T66MainMapTerrain::ECellRegion::StartArea, 0.f, MainMapStartAreaCenterSurfaceLocation))
	{
		MainMapStartAreaCenterSurfaceLocation = MainMapSpawnSurfaceLocation;
	}
	if (!MainMapStartAnchorSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapStartAnchorSurfaceLocation);
	}
	if (!MainMapStartAreaCenterSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapStartAreaCenterSurfaceLocation);
	}
	T66MainMapTerrain::TryGetCellLocation(Board, Board.BossAnchor, 0.f, MainMapBossAnchorSurfaceLocation);
	T66MainMapTerrain::TryGetCellLocation(Board, Board.BossSpawnCell, 0.f, MainMapBossSpawnSurfaceLocation);
	if (!T66MainMapTerrain::TryGetRegionCenterLocation(Board, T66MainMapTerrain::ECellRegion::BossArea, 0.f, MainMapBossAreaCenterSurfaceLocation))
	{
		MainMapBossAreaCenterSurfaceLocation = MainMapBossSpawnSurfaceLocation;
	}

	FVector BossBeaconLocation = FVector::ZeroVector;
	if (T66MainMapTerrain::TryGetCellLocation(Board, Board.BossSpawnCell - Board.BossOutwardDirection, 0.f, BossBeaconLocation))
	{
		MainMapBossBeaconSurfaceLocation = BossBeaconLocation;
	}
	else
	{
		MainMapBossBeaconSurfaceLocation = MainMapBossAreaCenterSurfaceLocation;
	}

	if (!MainMapBossAnchorSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapBossAnchorSurfaceLocation);
	}
	if (!MainMapBossSpawnSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapBossSpawnSurfaceLocation);
	}
	if (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapBossAreaCenterSurfaceLocation);
	}
	UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Cached main-map spawn surface at %s"), *MainMapSpawnSurfaceLocation.ToString());

	bool bMainMapCollisionReady = false;
	if (!T66MainMapTerrain::Spawn(World, Board, Preset, SpawnParams, bMainMapCollisionReady))
	{
		return;
	}

	bTerrainCollisionReady = bMainMapCollisionReady;
	if (bTerrainCollisionReady)
	{
		RestartPlayersMissingPawns();
		if (!T66UsesMainMapTerrainStage(World))
		{
			SnapPlayersToTerrain();
		}
	}

	UE_LOG(LogT66GameMode, Verbose, TEXT("[MAP] Terrain collision ready=%d"), bTerrainCollisionReady ? 1 : 0);
	UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Main map terrain spawned: %d tiles (seed=%d, cell=%.0f)"),
		Board.OccupiedCount,
		Preset.Seed,
		Board.Settings.CellSize);
}

void AT66GameMode::SpawnLightingIfNeeded()
{
	UT66LightingSubsystem::EnsureSharedLightingForWorld(GetWorld());
}

void AT66GameMode::SpawnQuakeSkyIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const bool bUseDungeonLighting = UT66GameInstance::GetEffectiveLightingPreset(World) == ET66LightingPreset::Dungeon;
	if (T66UsesMainMapTerrainStage(World) || bUseDungeonLighting)
	{
		TArray<AT66QuakeSkyActor*> SkyActorsToDestroy;
		for (TActorIterator<AT66QuakeSkyActor> It(World); It; ++It)
		{
			SkyActorsToDestroy.Add(*It);
		}
		for (AT66QuakeSkyActor* SkyActor : SkyActorsToDestroy)
		{
			if (SkyActor)
			{
				SkyActor->Destroy();
			}
		}
		if (SkyActorsToDestroy.Num() > 0)
		{
			UE_LOG(LogT66GameMode, Log, TEXT("[QuakeSky] Removed %d Quake sky actor(s) for current lighting preset"), SkyActorsToDestroy.Num());
		}
		return;
	}

	for (TActorIterator<AT66QuakeSkyActor> It(World); It; ++It)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66QuakeSkyActor* SkyActor = World->SpawnActor<AT66QuakeSkyActor>(
		AT66QuakeSkyActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams);
	if (!SkyActor) return;

#if WITH_EDITOR
	SkyActor->SetActorLabel(TEXT("DEV_QuakeSky"));
#endif
	SpawnedSetupActors.Add(SkyActor);
	UE_LOG(LogT66GameMode, Log, TEXT("[QuakeSky] Spawned retro sky dome"));
}

void AT66GameMode::ApplyThemeToDirectionalLights()
{
	UT66LightingSubsystem::ApplyThemeToDirectionalLightsForWorld(GetWorld());
}

void AT66GameMode::ApplyThemeToAtmosphereAndLighting()
{
	UT66LightingSubsystem::ApplyThemeToAtmosphereAndLightingForWorld(GetWorld());
}

void AT66GameMode::HandleSettingsChanged()
{
	ApplyThemeToDirectionalLights();
	ApplyThemeToAtmosphereAndLighting();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RetroFXSubsystem* RetroFX = GI->GetSubsystem<UT66RetroFXSubsystem>())
		{
			if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
			{
				FT66RetroFXSettings RetroSettings = PS->GetRetroFXSettings();
				if (T66UsesMainMapTerrainStage(GetWorld()))
				{
					RetroSettings.bEnableWorldGeometry = false;
					RetroSettings.WorldVertexSnapPercent = 0.0f;
					RetroSettings.WorldVertexSnapResolutionPercent = 0.0f;
					RetroSettings.WorldVertexNoisePercent = 0.0f;
					RetroSettings.WorldAffineBlendPercent = 0.0f;
					RetroSettings.WorldAffineDistance1Percent = 0.0f;
					RetroSettings.WorldAffineDistance2Percent = 0.0f;
					RetroSettings.WorldAffineDistance3Percent = 0.0f;
				}
				RetroFX->ApplySettings(RetroSettings, GetWorld());
			}
			else
			{
				RetroFX->ApplyCurrentSettings(GetWorld());
			}
		}
	}
}

void AT66GameMode::SpawnPlayerStartIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);

	if (bUsingMainMapTerrain)
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			(*It)->Destroy();
		}
		return;
	}

	// Check for existing player start
	bool bHasPlayerStart = false;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		bHasPlayerStart = true;
		break;
	}

	if (!bHasPlayerStart)
	{
		UE_LOG(LogT66GameMode, Log, TEXT("No PlayerStart found - spawning development PlayerStart"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		UT66GameInstance* GI = GetT66GameInstance();

		// Default spawn:
		// - Coliseum mode: coliseum arena (timer starts immediately; no pillars)
		// - Normal stage: start area (so timer starts after passing start pillars)
		FVector SpawnLoc;
		if (bUsingMainMapTerrain)
		{
			const FT66MapPreset Preset = T66BuildMainMapPreset(GI);
			SpawnLoc = T66MainMapTerrain::GetPreferredSpawnLocation(Preset, DefaultSpawnHeight);
		}
		else if (IsColiseumStage())
		{
			SpawnLoc = FVector(ColiseumCenter.X, ColiseumCenter.Y, DefaultSpawnHeight);
		}
		else
		{
			SpawnLoc = T66GameplayLayout::GetStartAreaCenter(DefaultSpawnHeight);
		}

		APlayerStart* Start = World->SpawnActor<APlayerStart>(
			APlayerStart::StaticClass(),
			SpawnLoc,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Start)
		{
			#if WITH_EDITOR
			Start->SetActorLabel(TEXT("DEV_PlayerStart"));
			#endif
			SpawnedSetupActors.Add(Start);
			UE_LOG(LogT66GameMode, Log, TEXT("Spawned development PlayerStart at %s"), *SpawnLoc.ToString());
		}
	}
}

void AT66GameMode::RestartPlayersMissingPawns()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || PC->GetPawn())
		{
			continue;
		}

		RestartPlayer(PC);
	}
}

void AT66GameMode::SpawnLabFloorIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World || !IsLabLevel()) return;

	// One central floor: ~1/4 of gameplay map (100k: MainHalfExtent 50000 -> Lab half 12500)
	static const FName LabFloorTag(TEXT("T66_Floor_Lab"));
	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		if (It->Tags.Contains(LabFloorTag))
		{
			It->Destroy();
			break;
		}
	}

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	const float LabHalfExtent = 12500.f;
	const float LabFloorHeight = 100.f;
	const FVector LabFloorScale(LabHalfExtent * 2.f / 100.f, LabHalfExtent * 2.f / 100.f, LabFloorHeight / 100.f);
	const FVector LabFloorLoc(0.f, 0.f, LabFloorHeight * 0.5f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), LabFloorLoc, FRotator::ZeroRotator, SpawnParams);
	if (!Floor || !Floor->GetStaticMeshComponent()) return;

	Floor->Tags.Add(LabFloorTag);
	T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Movable);
	Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
	Floor->SetActorScale3D(LabFloorScale);
	Floor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Floor->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Static);

	const UT66GameInstance* T66GI = GetT66GameInstance();
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	if (UMaterialInterface* LabFloorMaterial = FT66TerrainThemeAssets::ResolveDifficultyGroundMaterial(this, Difficulty))
	{
		Floor->GetStaticMeshComponent()->SetMaterial(0, LabFloorMaterial);
	}
	SpawnedSetupActors.Add(Floor);
	UE_LOG(LogT66GameMode, Log, TEXT("Spawned Lab central floor (half-extent %.0f, top Z %.0f)"), LabHalfExtent, LabFloorHeight);
}

void AT66GameMode::SpawnLabCollectorIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World || !IsLabLevel()) return;

	for (TActorIterator<AT66LabCollector> It(World); It; ++It)
	{
		return;  // Already have a Collector
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	// Place Collector in front of spawn (player spawns at 0,0,120)
	const FVector CollectorLoc(400.f, 0.f, 120.f);
	AT66LabCollector* Collector = World->SpawnActor<AT66LabCollector>(AT66LabCollector::StaticClass(), CollectorLoc, FRotator::ZeroRotator, SpawnParams);
	if (Collector)
	{
		SpawnedSetupActors.Add(Collector);
		UE_LOG(LogT66GameMode, Log, TEXT("Spawned The Collector in Lab"));
	}
}

FVector AT66GameMode::GetRandomLabSpawnLocation() const
{
	// Lab floor: half extent 12500, top Z = 100. Keep margin from edge and min distance between spawns.
	static constexpr float LabHalfExtent = 12500.f;
	static constexpr float Margin = 800.f;
	static constexpr float MinDistBetween = 400.f;
	static constexpr float SpawnZ = 100.f;
	static constexpr int32 MaxAttempts = 20;

	UWorld* World = GetWorld();
	if (!World) return FVector(400.f, 0.f, SpawnZ);

	FRandomStream Rng(FMath::Rand());
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		const float X = Rng.FRandRange(-(LabHalfExtent - Margin), LabHalfExtent - Margin);
		const float Y = Rng.FRandRange(-(LabHalfExtent - Margin), LabHalfExtent - Margin);
		const FVector Candidate(X, Y, SpawnZ);

		bool bTooClose = false;
		for (AActor* A : LabSpawnedActors)
		{
			if (!A) continue;
			if (FVector::Dist2D(Candidate, A->GetActorLocation()) < MinDistBetween)
			{
				bTooClose = true;
				break;
			}
		}
		if (!bTooClose) return Candidate;
	}
	// Fallback: offset from last spawn or default
	if (LabSpawnedActors.Num() > 0 && LabSpawnedActors.Last())
	{
		FVector Last = LabSpawnedActors.Last()->GetActorLocation();
		return Last + FVector(MinDistBetween, 0.f, 0.f);
	}
	return FVector(400.f, 0.f, SpawnZ);
}

UT66GameInstance* AT66GameMode::GetT66GameInstance() const
{
	return Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
}

bool AT66GameMode::TrySnapActorToTerrain(AActor* Actor) const
{
	return TrySnapActorToTerrainAtLocation(Actor, Actor ? Actor->GetActorLocation() : FVector::ZeroVector);
}

bool AT66GameMode::TrySnapActorToTerrainAtLocation(AActor* Actor, const FVector& TraceLocation) const
{
	UWorld* World = GetWorld();
	if (!World || !Actor)
	{
		return false;
	}

	FHitResult GroundHit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66SnapActorToTerrain), false, Actor);
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		Params.AddIgnoredActor(*It);
	}
	FVector TraceStart = FVector::ZeroVector;
	FVector TraceEnd = FVector::ZeroVector;
	if (IsUsingTowerMainMapLayout())
	{
		const float LocalTraceUp = FMath::Max(900.0f, CachedTowerMainMapLayout.FloorSpacing - CachedTowerMainMapLayout.FloorThickness - 350.0f);
		const float LocalTraceDown = FMath::Max(CachedTowerMainMapLayout.FloorSpacing + CachedTowerMainMapLayout.FloorThickness + 900.0f, 3200.0f);
		TraceStart = FVector(TraceLocation.X, TraceLocation.Y, TraceLocation.Z + LocalTraceUp);
		TraceEnd = FVector(TraceLocation.X, TraceLocation.Y, TraceLocation.Z - LocalTraceDown);
	}
	else
	{
		TraceStart = FVector(TraceLocation.X, TraceLocation.Y, TraceLocation.Z + 8000.f);
		TraceEnd = FVector(TraceLocation.X, TraceLocation.Y, TraceLocation.Z - 16000.f);
	}

	bool bFoundGroundHit = false;
	if (IsUsingTowerMainMapLayout())
	{
		TArray<FHitResult> Hits;
		if (World->LineTraceMultiByChannel(Hits, TraceStart, TraceEnd, ECC_WorldStatic, Params))
		{
			for (const FHitResult& Hit : Hits)
			{
				if (T66ShouldIgnoreTowerCeilingHit(Hit))
				{
					continue;
				}

				GroundHit = Hit;
				bFoundGroundHit = true;
				break;
			}
		}

		if (!bFoundGroundHit && World->LineTraceMultiByChannel(Hits, TraceStart, TraceEnd, ECC_Visibility, Params))
		{
			for (const FHitResult& Hit : Hits)
			{
				if (T66ShouldIgnoreTowerCeilingHit(Hit))
				{
					continue;
				}

				GroundHit = Hit;
				bFoundGroundHit = true;
				break;
			}
		}
	}
	else
	{
		bFoundGroundHit =
			World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic, Params)
			|| World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, Params);
	}

	if (!bFoundGroundHit)
	{
		return false;
	}

	float HalfHeight = 0.f;
	if (const UCapsuleComponent* Capsule = Actor->FindComponentByClass<UCapsuleComponent>())
	{
		HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	}

	const FVector GroundedLoc = GroundHit.ImpactPoint + FVector(0.f, 0.f, HalfHeight + 5.f);
	Actor->SetActorLocation(GroundedLoc, false, nullptr, ETeleportType::TeleportPhysics);
	return true;
}

void AT66GameMode::MaintainPlayerTerrainSafety()
{
	UWorld* World = GetWorld();
	if (!World || IsLabLevel() || IsColiseumStage())
	{
		return;
	}

	UT66GameInstance* GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const FT66MapPreset Preset = T66BuildMainMapPreset(GI);
	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	const bool bTowerLayout = bUsingMainMapTerrain && IsUsingTowerMainMapLayout();
	float RescueThresholdZ = bUsingMainMapTerrain
		? (T66MainMapTerrain::GetLowestCollisionBottomZ(Preset) - 100.0f)
		: (Preset.ElevationMin - 200.f);
	if (bTowerLayout && CachedTowerMainMapLayout.Floors.Num() > 0)
	{
		const T66TowerMapTerrain::FFloor& LowestFloor = CachedTowerMainMapLayout.Floors.Last();
		RescueThresholdZ = LowestFloor.SurfaceZ - FMath::Max(CachedTowerMainMapLayout.FloorThickness + 1400.0f, 1800.0f);
	}
	const float AnchorTraceZ = bUsingMainMapTerrain
		? T66MainMapTerrain::GetTraceZ(Preset)
		: (Preset.ElevationMax + 3000.f);
	const bool bStageTimerActive = RunState && RunState->GetStageTimerActive();

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		if (!Pawn)
		{
			continue;
		}

		ACharacter* Character = Cast<ACharacter>(Pawn);
		UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;

		if (!bTerrainCollisionReady)
		{
			if (Movement && Movement->MovementMode != MOVE_None)
			{
				Movement->StopMovementImmediately();
				Movement->SetMovementMode(MOVE_None);
			}
			continue;
		}

		const FVector PawnLoc = Pawn->GetActorLocation();
		const int32 TowerFloorNumber = bTowerLayout ? GetTowerFloorIndexForLocation(PawnLoc) : INDEX_NONE;
		const bool bMovementLost = Movement && Movement->MovementMode == MOVE_None;
		const bool bNeedsRescue = bTowerLayout
			? (PawnLoc.Z <= RescueThresholdZ || (bMovementLost && TowerFloorNumber == INDEX_NONE))
			: (PawnLoc.Z <= RescueThresholdZ || bMovementLost);
		if (!bNeedsRescue)
		{
			continue;
		}

		TArray<FVector> RescueAnchors;
		if (bUsingMainMapTerrain)
		{
			if (MainMapRescueAnchorLocations.Num() > 0)
			{
				RescueAnchors = MainMapRescueAnchorLocations;
			}
			else
			{
				const int32 GridSize = T66MainMapTerrain::MakeSettings(Preset).BoardSize;
				RescueAnchors.Reserve(5);
				RescueAnchors.Add(T66MainMapTerrain::GetSpawnLocation(Preset, AnchorTraceZ));
				RescueAnchors.Add(T66MainMapTerrain::GetCellCenter(Preset, GridSize / 2, FMath::Max(0, GridSize / 2 - 1), AnchorTraceZ));
				RescueAnchors.Add(T66MainMapTerrain::GetCellCenter(Preset, GridSize / 2, FMath::Min(GridSize - 1, GridSize / 2 + 1), AnchorTraceZ));
				RescueAnchors.Add(T66MainMapTerrain::GetCellCenter(Preset, FMath::Max(0, GridSize / 2 - 1), GridSize / 2, AnchorTraceZ));
				RescueAnchors.Add(T66MainMapTerrain::GetCellCenter(Preset, FMath::Min(GridSize - 1, GridSize / 2 + 1), GridSize / 2, AnchorTraceZ));
			}
		}
		else if (!bStageTimerActive || PawnLoc.X <= 0.f)
		{
			RescueAnchors.Reserve(4);
			RescueAnchors.Add(T66GameplayLayout::GetStartAreaCenter(AnchorTraceZ));
			RescueAnchors.Add(T66GameplayLayout::GetStartGateLocation(AnchorTraceZ));
		}
		if (!bUsingMainMapTerrain && bStageTimerActive && PawnLoc.X > 0.f)
		{
			RescueAnchors.Add(T66GameplayLayout::GetBossGateLocation(AnchorTraceZ));
			RescueAnchors.Add(T66GameplayLayout::GetBossAreaCenter(AnchorTraceZ));
		}

		bool bRecovered = TrySnapActorToTerrain(Pawn);
		if (!bRecovered)
		{
			RescueAnchors.Sort([PawnLoc, bTowerLayout](const FVector& A, const FVector& B)
			{
				return bTowerLayout
					? (FVector::DistSquared(A, PawnLoc) < FVector::DistSquared(B, PawnLoc))
					: (FVector::DistSquared2D(A, PawnLoc) < FVector::DistSquared2D(B, PawnLoc));
			});

			for (const FVector& Anchor : RescueAnchors)
			{
				if (TrySnapActorToTerrainAtLocation(Pawn, Anchor))
				{
					bRecovered = true;
					break;
				}
			}
		}

		if (Movement)
		{
			Movement->StopMovementImmediately();
			Movement->SetMovementMode(bRecovered ? MOVE_Walking : MOVE_None);
		}
	}
}

void AT66GameMode::SnapPlayersToTerrain()
{
	UWorld* World = GetWorld();
	if (!World || !bTerrainCollisionReady)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		if (!Pawn)
		{
			continue;
		}

		const bool bSnapped = TrySnapActorToTerrain(Pawn);
		if (!bSnapped)
		{
			continue;
		}

		if (ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
				if (Movement->MovementMode == MOVE_None)
				{
					Movement->SetMovementMode(MOVE_Walking);
				}
			}
		}
	}
}

void AT66GameMode::RegenerateMainMapTerrain(int32 Seed)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PropSubsystem* PropSub = GI->GetSubsystem<UT66PropSubsystem>())
		{
			PropSub->ClearProps();
		}
	}

	bTerrainCollisionReady = false;
	bMainMapCombatStarted = false;
	bHasMainMapSpawnSurfaceLocation = false;
	bUsingTowerMainMapLayout = false;
	CachedTowerMainMapLayout = T66TowerMapTerrain::FLayout{};
	bTowerBossEntryTriggered = false;
	bTowerBossEntryApplied = false;
	MainMapSpawnSurfaceLocation = FVector::ZeroVector;
	MainMapStartAnchorSurfaceLocation = FVector::ZeroVector;
	MainMapStartPathSurfaceLocation = FVector::ZeroVector;
	MainMapStartAreaCenterSurfaceLocation = FVector::ZeroVector;
	MainMapBossAnchorSurfaceLocation = FVector::ZeroVector;
	MainMapBossSpawnSurfaceLocation = FVector::ZeroVector;
	MainMapBossBeaconSurfaceLocation = FVector::ZeroVector;
	MainMapBossAreaCenterSurfaceLocation = FVector::ZeroVector;
	MainMapRescueAnchorLocations.Reset();
	for (AT66TowerDescentHole* Hole : TowerDescentHoles)
	{
		if (Hole)
		{
			Hole->Destroy();
		}
	}
	TowerDescentHoles.Reset();

	DestroyActorsWithTag(World, T66MapPlatformTag);
	DestroyActorsWithTag(World, T66MapRampTag);
	DestroyActorsWithTag(World, T66FloorMainTag);
	DestroyActorsWithTag(World, T66TraversalBarrierTag);

	if (StartGate)
	{
		StartGate->Destroy();
		StartGate = nullptr;
	}
	if (BossGate)
	{
		BossGate->Destroy();
		BossGate = nullptr;
	}
	if (CowardiceGate)
	{
		CowardiceGate->Destroy();
		CowardiceGate = nullptr;
	}
	if (TricksterNPC)
	{
		TricksterNPC->Destroy();
		TricksterNPC = nullptr;
	}
	if (AT66BossBase* ExistingBoss = StageBoss.Get())
	{
		ExistingBoss->Destroy();
	}
	StageBoss = nullptr;
	DestroyBossBeacon();

	UT66GameInstance* GI = GetT66GameInstance();
	if (GI)
	{
		GI->RunSeed = Seed;
	}

	SpawnMainMapTerrain();
	if (T66UsesMainMapTerrainStage(World))
	{
		SpawnTricksterAndCowardiceGate();
		SpawnBossForCurrentStage();
	}
	else
	{
		SpawnBossBeaconIfNeeded();
	}
}

// Console command: T66.MainMap [seed]
static FAutoConsoleCommandWithWorldAndArgs T66MainMapCmd(
	TEXT("T66.MainMap"),
	TEXT("Regenerate the difficulty-driven main map terrain. Usage: T66.MainMap [seed]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args, UWorld* World)
		{
			if (!World) return;
			AGameModeBase* GM = World->GetAuthGameMode();
			AT66GameMode* T66GM = Cast<AT66GameMode>(GM);
			if (!T66GM)
			{
				UE_LOG(LogT66GameMode, Error, TEXT("T66.MainMap: no T66GameMode active"));
				return;
			}

			int32 Seed = FMath::Rand();
			if (Args.Num() >= 1)
			{
				Seed = FCString::Atoi(*Args[0]);
			}

			UE_LOG(LogT66GameMode, Log, TEXT("T66.MainMap: Regenerating main map terrain (seed=%d)"), Seed);
			T66GM->RegenerateMainMapTerrain(Seed);
		})
);

UClass* AT66GameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// Check if we have a specific hero class from the DataTable
	if (UT66GameInstance* GI = GetT66GameInstance())
	{
		const FName EffectiveHeroID = T66GetSelectedHeroID(GI, InController);
		FHeroData HeroData;
		if (!EffectiveHeroID.IsNone() && GI->GetHeroData(EffectiveHeroID, HeroData))
		{
			// If the hero has a specific class defined, use that
			if (!HeroData.HeroClass.IsNull())
			{
				// Avoid sync loads during spawn; prefer already-loaded class, otherwise fall back safely.
				if (UClass* HeroClass = HeroData.HeroClass.Get())
				{
					return HeroClass;
				}

				// Kick off an async load so subsequent spawns can use the intended class without hitching.
				TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
					HeroData.HeroClass.ToSoftObjectPath(),
					FStreamableDelegate());
				if (Handle.IsValid())
				{
					ActiveAsyncLoadHandles.Add(Handle);
				}
			}
		}
	}

	// Fall back to default hero class
	return DefaultHeroClass ? DefaultHeroClass.Get() : AT66HeroBase::StaticClass();
}

APawn* AT66GameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	// Get the pawn class
	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	if (!PawnClass)
	{
		UE_LOG(LogT66GameMode, Error, TEXT("No pawn class for spawning!"));
		return nullptr;
	}

	// Get spawn transform - use a safe default height if no PlayerStart exists
	FVector SpawnLocation;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(GetWorld());
	UT66GameInstance* CurrentGI = GetT66GameInstance();
	UT66RunStateSubsystem* CurrentRunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	FVector MainMapSafeSpawnLocation = FVector::ZeroVector;
	const bool bHasMainMapSafeSpawnLocation = bUsingMainMapTerrain
		&& TryGetMainMapStartPlacementLocation(0.f, -1.10f, MainMapSafeSpawnLocation);
	const bool bUseTowerStageDropIntro = bUsingMainMapTerrain
		&& IsUsingTowerMainMapLayout()
		&& CurrentGI
		&& CurrentGI->bPendingTowerStageDropIntro
		&& CurrentRunState
		&& CurrentRunState->GetCurrentStage() > 1;

	if (bUsingMainMapTerrain && !bTerrainCollisionReady)
	{
		UE_LOG(LogT66GameMode, Log, TEXT("Deferring main-map hero spawn until terrain collision is ready."));
		return nullptr;
	}

	if (bUsingMainMapTerrain && bHasMainMapSpawnSurfaceLocation)
	{
		SpawnLocation = (bHasMainMapSafeSpawnLocation ? MainMapSafeSpawnLocation : MainMapSpawnSurfaceLocation) + FVector(0.f, 0.f, DefaultSpawnHeight);
		FVector Center = FVector::ZeroVector;
		FVector InwardDirection = FVector::ForwardVector;
		FVector SideDirection = FVector::RightVector;
		float CellSize = 0.f;
		if (TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize) && !InwardDirection.IsNearlyZero())
		{
			SpawnRotation = InwardDirection.Rotation();
		}
		else
		{
			SpawnRotation = FRotator::ZeroRotator;
		}
	}
	else if (StartSpot)
	{
		SpawnLocation = StartSpot->GetActorLocation();
		SpawnRotation = StartSpot->GetActorRotation();
		UE_LOG(LogT66GameMode, Log, TEXT("Spawning at PlayerStart: (%.1f, %.1f, %.1f)"),
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
	}
	else
	{
		// No PlayerStart found - spawn at a safe default location.
		SpawnLocation = IsColiseumStage()
			? FVector(ColiseumCenter.X, ColiseumCenter.Y, 200.f)
			: FVector(-35455.f, 0.f, 200.f);
		UE_LOG(LogT66GameMode, Warning, TEXT("No PlayerStart found! Spawning at default location (%.0f, %.0f, %.0f)."),
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
	}

	// Robust: always ensure Gameplay spawns in the Start Area (scaled for 100k map).
	if (IsLabLevel())
	{
		SpawnLocation = FVector(0.f, 0.f, 120.f);
		SpawnRotation = FRotator::ZeroRotator;
	}
	else if (bUsingMainMapTerrain)
	{
		if (bHasMainMapSafeSpawnLocation)
		{
			SpawnLocation = MainMapSafeSpawnLocation + FVector(0.f, 0.f, 200.f);
		}
		else if (bHasMainMapSpawnSurfaceLocation)
		{
			SpawnLocation = MainMapSpawnSurfaceLocation + FVector(0.f, 0.f, 200.f);
		}
		else
		{
			if (IsUsingTowerMainMapLayout())
			{
				SpawnLocation = T66TowerMapTerrain::GetPreferredSpawnLocation(CachedTowerMainMapLayout, 200.f);
			}
			else
			{
				const FT66MapPreset Preset = T66BuildMainMapPreset(GetT66GameInstance());
				SpawnLocation = T66MainMapTerrain::GetPreferredSpawnLocation(Preset, 200.f);
			}
		}
		FVector Center = FVector::ZeroVector;
		FVector InwardDirection = FVector::ForwardVector;
		FVector SideDirection = FVector::RightVector;
		float CellSize = 0.f;
		if (TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize) && !InwardDirection.IsNearlyZero())
		{
			SpawnRotation = InwardDirection.Rotation();
		}
		else
		{
			SpawnRotation = FRotator::ZeroRotator;
		}

		if (bUseTowerStageDropIntro && bHasMainMapSpawnSurfaceLocation)
		{
			SpawnLocation = MainMapSpawnSurfaceLocation + FVector(0.f, 0.f, T66TowerStageTransitionDropHeight);
		}
	}
	else if (!IsColiseumStage())
	{
		SpawnLocation = FVector(-35455.f, 0.f, 200.f);
		SpawnRotation = FRotator::ZeroRotator;

		if (UT66GameInstance* T66GI = GetT66GameInstance())
		{
			if (T66IsStandaloneTutorialMap(GetWorld()) || bForceSpawnInTutorialArea)
			{
				FVector TutorialSpawnLocation = FVector(-3636.f, 56818.f, 200.f);
				FRotator TutorialSpawnRotation = FRotator::ZeroRotator;
				if (T66TryGetTaggedActorTransform(GetWorld(), FName(TEXT("T66_Tutorial_PlayerSpawn")), TutorialSpawnLocation, TutorialSpawnRotation))
				{
					SpawnLocation = TutorialSpawnLocation;
					SpawnRotation = TutorialSpawnRotation;
				}
				else
				{
					SpawnLocation = FVector(-3636.f, 56818.f, 200.f);
					SpawnRotation = FRotator::ZeroRotator;
				}
			}
		}
	}
	else
	{
		// Forced coliseum mode uses an enclosed arena off to the side (not the main arena).
		if (UT66GameInstance* T66GI = GetT66GameInstance())
		{
			if (T66GI->bForceColiseumMode || T66GI->ColiseumFlowMode != ET66ColiseumFlowMode::None)
			{
				SpawnLocation = ColiseumCenter;
				SpawnRotation = FRotator::ZeroRotator;
			}
		}
	}

	const UWorld* SpawnWorld = GetWorld();
	const int32 ConnectedPlayerCount = T66GetConnectedPlayerCount(SpawnWorld);
	const int32 PlayerSlotIndex = T66GetPlayerSlotIndex(SpawnWorld, NewPlayer);
	const float CenteredSlotOffset = static_cast<float>(PlayerSlotIndex) - (static_cast<float>(ConnectedPlayerCount - 1) * 0.5f);
	FVector MultiplayerSpawnOffset = FVector::ZeroVector;
	if (ConnectedPlayerCount > 1 && !FMath::IsNearlyZero(CenteredSlotOffset))
	{
		if (bUsingMainMapTerrain)
		{
			FVector Center = FVector::ZeroVector;
			FVector InwardDirection = FVector::ForwardVector;
			FVector SideDirection = FVector::RightVector;
			float CellSize = 0.f;
			if (TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize) && !SideDirection.IsNearlyZero())
			{
				MultiplayerSpawnOffset = SideDirection * (CenteredSlotOffset * FMath::Max(CellSize * 0.55f, 180.f));
			}
		}
		else if (!IsColiseumStage())
		{
			MultiplayerSpawnOffset = FVector(0.f, CenteredSlotOffset * 220.f, 0.f);
		}
		else
		{
			MultiplayerSpawnOffset = FVector(CenteredSlotOffset * 180.f, 0.f, 0.f);
		}

		SpawnLocation += MultiplayerSpawnOffset;
	}

	if (bUsingMainMapTerrain)
	{
		const FVector FacingTarget = !MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
			? MainMapStartAreaCenterSurfaceLocation
			: (!MainMapStartAnchorSurfaceLocation.IsNearlyZero()
				? MainMapStartAnchorSurfaceLocation
				: MainMapSpawnSurfaceLocation);
		FRotator FacingRotation = SpawnRotation;
		if (T66TryBuildFacingRotation2D(SpawnLocation, FacingTarget, FacingRotation))
		{
			SpawnRotation = FacingRotation;
		}
	}

	// Sky-drop: spawn the hero at a comfortable altitude; gravity + Landed() handle the rest.
	// This avoids trace-before-terrain-exists timing issues and gives a cinematic entrance.

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;
	SpawnParams.SpawnCollisionHandlingOverride = bUsingMainMapTerrain
		? ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		: ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn the pawn
	APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (SpawnedPawn)
	{
		T66SyncPawnAndControllerRotation(SpawnedPawn, NewPlayer, SpawnRotation);

		const bool bNeedsProceduralTerrain = !IsLabLevel() && !IsColiseumStage();
		if (bUsingMainMapTerrain && bTerrainCollisionReady)
		{
			float HalfHeight = 0.f;
			if (const UCapsuleComponent* Capsule = SpawnedPawn->FindComponentByClass<UCapsuleComponent>())
			{
				HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
			}

			FVector ExactSpawnLocation = SpawnLocation;
			if (bHasMainMapSafeSpawnLocation)
			{
				ExactSpawnLocation = MainMapSafeSpawnLocation + MultiplayerSpawnOffset + FVector(0.f, 0.f, HalfHeight + 5.f);
			}
			else if (bHasMainMapSpawnSurfaceLocation)
			{
				ExactSpawnLocation = MainMapSpawnSurfaceLocation + MultiplayerSpawnOffset + FVector(0.f, 0.f, HalfHeight + 5.f);
			}
			else
			{
				if (IsUsingTowerMainMapLayout())
				{
					ExactSpawnLocation = T66TowerMapTerrain::GetPreferredSpawnLocation(CachedTowerMainMapLayout, HalfHeight + 5.f) + MultiplayerSpawnOffset;
				}
				else
				{
					const FT66MapPreset Preset = T66BuildMainMapPreset(GetT66GameInstance());
					ExactSpawnLocation = T66MainMapTerrain::GetPreferredSpawnLocation(Preset, HalfHeight + 5.f) + MultiplayerSpawnOffset;
				}
			}
			if (bUseTowerStageDropIntro)
			{
				const FVector DropStartLocation = (bHasMainMapSpawnSurfaceLocation
					? MainMapSpawnSurfaceLocation
					: ExactSpawnLocation)
					+ MultiplayerSpawnOffset
					+ FVector(0.f, 0.f, T66TowerStageTransitionDropHeight);
				SpawnedPawn->SetActorLocation(DropStartLocation, false, nullptr, ETeleportType::TeleportPhysics);
				T66SyncPawnAndControllerRotation(SpawnedPawn, NewPlayer, SpawnRotation);
				if (ACharacter* Character = Cast<ACharacter>(SpawnedPawn))
				{
					if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
					{
						Movement->StopMovementImmediately();
						Movement->SetMovementMode(MOVE_Falling);
					}
				}
				if (CurrentGI)
				{
					CurrentGI->bPendingTowerStageDropIntro = false;
				}
			}
			else
			{
				SpawnedPawn->SetActorLocation(ExactSpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
				T66SyncPawnAndControllerRotation(SpawnedPawn, NewPlayer, SpawnRotation);
				if (ACharacter* Character = Cast<ACharacter>(SpawnedPawn))
				{
					if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
					{
						Movement->StopMovementImmediately();
						Movement->SetMovementMode(MOVE_Walking);
					}
				}
			}
		}
		else if (bTerrainCollisionReady)
		{
			TrySnapActorToTerrain(SpawnedPawn);
		}
		else if (bNeedsProceduralTerrain)
		{
			if (ACharacter* Character = Cast<ACharacter>(SpawnedPawn))
			{
				if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
					Movement->SetMovementMode(MOVE_None);
				}
			}
		}
	}

	if (SpawnedPawn && IsValid(IdolAltar))
	{
		T66FaceActorTowardLocation2D(IdolAltar, SpawnedPawn->GetActorLocation());
	}

	// If it's our hero class, initialize it with hero data and body type
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(SpawnedPawn))
	{

		if (UT66GameInstance* GI = GetT66GameInstance())
		{
			FHeroData HeroData;
			const FName EffectiveHeroID = T66GetSelectedHeroID(GI, NewPlayer);

			if (!EffectiveHeroID.IsNone() && GI->GetHeroData(EffectiveHeroID, HeroData))
			{
				if (GI->SelectedHeroID.IsNone() && NewPlayer && NewPlayer->IsLocalController())
				{
					GI->SelectedHeroID = EffectiveHeroID;
				}

				ET66BodyType SelectedBodyType = T66GetSelectedHeroBodyType(GI, NewPlayer);
				FName SelectedSkinID = T66GetSelectedHeroSkinID(GI, NewPlayer);
				Hero->InitializeHero(HeroData, SelectedBodyType, SelectedSkinID, false);

				UE_LOG(LogT66GameMode, Log, TEXT("Spawned hero: %s (%s), BodyType: %s, Skin: %s, Color: (%.2f, %.2f, %.2f)"),
					*HeroData.DisplayName.ToString(),
					*EffectiveHeroID.ToString(),
					SelectedBodyType == ET66BodyType::TypeA ? TEXT("TypeA") : TEXT("TypeB"),
					*SelectedSkinID.ToString(),
					HeroData.PlaceholderColor.R,
					HeroData.PlaceholderColor.G,
					HeroData.PlaceholderColor.B);
			}
			else
			{
				UE_LOG(LogT66GameMode, Warning, TEXT("No hero selected in Game Instance - spawning with defaults"));
			}
		}
	}

	return SpawnedPawn;
}

AT66HeroBase* AT66GameMode::SpawnSelectedHero(AController* Controller)
{
	// Find a player start
	AActor* StartSpot = FindPlayerStart(Controller);

	// Use our spawn logic
	APawn* SpawnedPawn = SpawnDefaultPawnFor(Controller, StartSpot);

	// Possess the pawn
	if (SpawnedPawn && Controller)
	{
		Controller->Possess(SpawnedPawn);
	}

	return Cast<AT66HeroBase>(SpawnedPawn);
}

// ============================================
// The Lab: spawn / reset
// ============================================

FVector AT66GameMode::GetLabSpawnLocation() const
{
	UWorld* World = GetWorld();
	if (!World) return FVector(400.f, 0.f, 200.f);
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			APawn* Pawn = PC->GetPawn();
			if (Pawn)
			{
				FVector Loc = Pawn->GetActorLocation();
				FVector Fwd = Pawn->GetActorForwardVector();
				Fwd.Z = 0.f;
				Fwd.Normalize();
				return Loc + Fwd * 500.f + FVector(0.f, 0.f, 0.f);
			}
		}
	}
	return FVector(400.f, 0.f, 200.f);
}

AActor* AT66GameMode::SpawnLabMob(FName CharacterVisualID)
{
	if (!IsLabLevel()) return nullptr;
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	TSubclassOf<AT66EnemyBase> ClassToSpawn;
	if (CharacterVisualID == FName(TEXT("GoblinThief")))
		ClassToSpawn = AT66GoblinThiefEnemy::StaticClass();
	else
		ClassToSpawn = AT66EnemyBase::StaticClass();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FVector Loc = GetRandomLabSpawnLocation();
	FRotator Rot = FRotator::ZeroRotator;
	AT66EnemyBase* Mob = World->SpawnActor<AT66EnemyBase>(ClassToSpawn, Loc, Rot, SpawnParams);
	if (Mob)
	{
		if (!CharacterVisualID.IsNone())
		{
			Mob->CharacterVisualID = CharacterVisualID;
		}
		LabSpawnedActors.Add(Mob);
	}
	return Mob;
}

AActor* AT66GameMode::SpawnLabBoss(FName BossID)
{
	if (!IsLabLevel()) return nullptr;
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	UT66GameInstance* GI = GetT66GameInstance();
	if (!GI) return nullptr;

	FBossData BossData;
	if (!GI->GetBossData(BossID, BossData)) return nullptr;

	UClass* ClassToSpawn = BossData.BossClass.Get();
	if (!ClassToSpawn)
	{
		ClassToSpawn = AT66BossBase::StaticClass();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FVector Loc = GetRandomLabSpawnLocation();
	FRotator Rot = FRotator::ZeroRotator;
	AT66BossBase* Boss = World->SpawnActor<AT66BossBase>(ClassToSpawn, Loc, Rot, SpawnParams);
	if (Boss)
	{
		Boss->InitializeBoss(BossData);
		Boss->bAwakened = true;
		Boss->CurrentHP = Boss->MaxHP;
		LabSpawnedActors.Add(Boss);
	}
	return Boss;
}

AActor* AT66GameMode::SpawnLabFountainOfLife()
{
	if (!IsLabLevel()) return nullptr;
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FVector Loc = GetRandomLabSpawnLocation();
	FRotator Rot = FRotator::ZeroRotator;
	AT66FountainOfLifeInteractable* Fountain = World->SpawnActor<AT66FountainOfLifeInteractable>(AT66FountainOfLifeInteractable::StaticClass(), Loc, Rot, SpawnParams);
	if (Fountain)
	{
		LabSpawnedActors.Add(Fountain);
	}
	return Fountain;
}

AActor* AT66GameMode::SpawnLabInteractable(FName InteractableID)
{
	if (!IsLabLevel()) return nullptr;
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	UClass* ClassToSpawn = nullptr;
	if (InteractableID == FName(TEXT("Fountain")))
		ClassToSpawn = AT66FountainOfLifeInteractable::StaticClass();
	else if (InteractableID == FName(TEXT("Chest")))
		ClassToSpawn = AT66ChestInteractable::StaticClass();
	else if (InteractableID == FName(TEXT("WheelSpin")))
		ClassToSpawn = AT66WheelSpinInteractable::StaticClass();
	else if (InteractableID == FName(TEXT("IdolAltar")))
		ClassToSpawn = AT66IdolAltar::StaticClass();
	else if (InteractableID == FName(TEXT("Crate")))
		ClassToSpawn = AT66CrateInteractable::StaticClass();
	else if (InteractableID == FName(TEXT("QuickReviveVending")))
		ClassToSpawn = AT66QuickReviveVendingMachine::StaticClass();
	if (!ClassToSpawn) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FVector Loc = GetRandomLabSpawnLocation();
	FRotator Rot = FRotator::ZeroRotator;
	AActor* Spawned = World->SpawnActor(ClassToSpawn, &Loc, &Rot, SpawnParams);
	if (Spawned)
	{
		LabSpawnedActors.Add(Spawned);
	}
	return Spawned;
}

void AT66GameMode::ResetLabSpawnedActors()
{
	for (AActor* A : LabSpawnedActors)
	{
		if (A)
		{
			A->Destroy();
		}
	}
	LabSpawnedActors.Empty();
}

