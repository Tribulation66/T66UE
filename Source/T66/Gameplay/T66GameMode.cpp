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
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66GoblinThiefEnemy.h"
#include "Gameplay/T66IdolAltar.h"
#include "Gameplay/T66FountainOfLifeInteractable.h"
#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66WheelSpinInteractable.h"
#include "Gameplay/T66CrateInteractable.h"
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
#include "Core/T66PosterizeSubsystem.h"
#include "Gameplay/T66EclipseActor.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66SkillRatingSubsystem.h"
#include "Core/T66PropSubsystem.h"
#include "Gameplay/T66RecruitableCompanion.h"
#include "Gameplay/T66PlayerController.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/LightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/TextureCube.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"

#if WITH_EDITORONLY_DATA
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "Math/Float16.h"
#endif
#include "Materials/MaterialInterface.h"
#include "EngineUtils.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "Gameplay/T66ProceduralLandscapeGenerator.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Landscape.h"
#include "UI/Style/T66Style.h"
#include "T66.h"
#include <initializer_list>

namespace
{
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
		// Per design: 24 total companions unlock on every non-boss stage in 1..30.
		// Stages 31..33 never unlock companions.
		return StageNum >= 1 && StageNum <= 30 && (StageNum % 5) != 0;
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
		return Count; // 1..24
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

	struct FT66MeshSectionBuffers
	{
		TArray<FVector> Verts;
		TArray<int32> Tris;
		TArray<FVector> Normals;
		TArray<FVector2D> UVs;

		bool HasGeometry() const
		{
			return Verts.Num() > 0 && Tris.Num() > 0;
		}
	};

	static int32 T66PickQuarterTurns(uint32 Hash)
	{
		return static_cast<int32>(Hash & 3u);
	}

	static int32 T66PickCellVariant(int32 Seed, int32 Row, int32 Col, int32 VariantCount)
	{
		if (VariantCount <= 1)
		{
			return 0;
		}

		uint32 Hash = GetTypeHash(FIntPoint(Row, Col));
		Hash = HashCombineFast(Hash, GetTypeHash(Seed));
		return static_cast<int32>(Hash % static_cast<uint32>(VariantCount));
	}

	static int32 T66PickCellQuarterTurns(int32 Seed, int32 Row, int32 Col)
	{
		uint32 Hash = GetTypeHash(FIntPoint(Row, Col));
		Hash = HashCombineFast(Hash, GetTypeHash(Seed));
		return T66PickQuarterTurns(Hash);
	}

	static int32 T66PickBoundaryVariant(int32 Seed, int32 AIndex, int32 BIndex, int32 VariantCount)
	{
		if (VariantCount <= 1)
		{
			return 0;
		}

		uint32 Hash = GetTypeHash(FIntPoint(FMath::Min(AIndex, BIndex), FMath::Max(AIndex, BIndex)));
		Hash = HashCombineFast(Hash, GetTypeHash(Seed));
		return static_cast<int32>(Hash % static_cast<uint32>(VariantCount));
	}

	static int32 T66PickBoundaryQuarterTurns(int32 Seed, int32 AIndex, int32 BIndex)
	{
		uint32 Hash = GetTypeHash(FIntPoint(FMath::Min(AIndex, BIndex), FMath::Max(AIndex, BIndex)));
		Hash = HashCombineFast(Hash, GetTypeHash(Seed));
		return T66PickQuarterTurns(Hash);
	}

	static FVector2D T66RotateUnitUV(const FVector2D& UV, int32 QuarterTurns)
	{
		switch (QuarterTurns & 3)
		{
		case 1:
			return FVector2D(1.f - UV.Y, UV.X);
		case 2:
			return FVector2D(1.f - UV.X, 1.f - UV.Y);
		case 3:
			return FVector2D(UV.Y, 1.f - UV.X);
		default:
			return UV;
		}
	}

	static void T66AppendRotatedQuadUVs(TArray<FVector2D>& UVs, int32 QuarterTurns)
	{
		UVs.Add(T66RotateUnitUV(FVector2D(0.f, 0.f), QuarterTurns));
		UVs.Add(T66RotateUnitUV(FVector2D(1.f, 0.f), QuarterTurns));
		UVs.Add(T66RotateUnitUV(FVector2D(1.f, 1.f), QuarterTurns));
		UVs.Add(T66RotateUnitUV(FVector2D(0.f, 1.f), QuarterTurns));
	}
}

AT66GameMode::AT66GameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	// Default to our hero base class
	DefaultPawnClass = AT66HeroBase::StaticClass();
	DefaultHeroClass = AT66HeroBase::StaticClass();

	// Coliseum arena lives inside GameplayLevel, off to the side (walled off). Scaled for 100k map.
	ColiseumCenter = FVector(-45455.f, -23636.f, 200.f);

	// Default terrain materials; used as a fallback for runtime floors and terrain when source files are absent.
	GroundFloorMaterials.Empty();
	GroundFloorMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Ground/MI_GroundTile1.MI_GroundTile1"))));
	GroundFloorMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Ground/MI_GroundTile2.MI_GroundTile2"))));
	GroundFloorMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Ground/MI_GroundTile3.MI_GroundTile3"))));
	GroundFloorMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Ground/MI_GroundTile4.MI_GroundTile4"))));
	GroundFloorMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Ground/MI_GroundTile5.MI_GroundTile5"))));

	CliffSideMaterials.Empty();
	CliffSideMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile1.MI_HillTile1"))));
	CliffSideMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile2.MI_HillTile2"))));
	CliffSideMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile3.MI_HillTile3"))));
	CliffSideMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile4.MI_HillTile4"))));
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

	// Reset run state when entering gameplay level unless this is a stage transition (keep progress)
	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (GI && T66GI)
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			// Bind to timer changes so we can spawn LoanShark exactly when timer starts.
			RunState->StageTimerChanged.AddDynamic(this, &AT66GameMode::HandleStageTimerChanged);
			RunState->DifficultyChanged.AddDynamic(this, &AT66GameMode::HandleDifficultyChanged);

			// Robust: treat any stage > 1 as a stage transition even if a gate forgot to set the flag.
			const bool bKeepProgress = T66GI->bIsStageTransition || (RunState->GetCurrentStage() > 1);
			if (bKeepProgress)
			{
				T66GI->bIsStageTransition = false;
				RunState->ResetStageTimerToFull(); // New stage: timer frozen at 60 until start gate
				RunState->ResetBossState(); // New stage: boss is dormant again; hide boss UI
			}
			else
			{
				T66GI->bRunIneligibleForLeaderboard = false; // Fresh run is eligible
				RunState->ResetForNewRun();
				if (UT66DamageLogSubsystem* DamageLog = GI->GetSubsystem<UT66DamageLogSubsystem>())
				{
					DamageLog->ResetForNewRun();
				}
			}
		}
	}

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

	// Coliseum mode: only spawn owed bosses + miasma (no houses, no waves, no NPCs, no start gate/pillars).
	if (IsColiseumStage())
	{
		ResetColiseumState();
		if (UGameInstance* GI2 = GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI2->GetSubsystem<UT66RunStateSubsystem>())
			{
				// Coliseum countdown begins immediately (no start gate).
				RunState->ResetStageTimerToFull();
				RunState->SetStageTimerActive(true);
			}
		}

		// Coliseum still has miasma.
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			MiasmaManager = GetWorld()->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}

		SpawnColiseumArenaIfNeeded();
		SpawnAllOwedBossesInColiseum();
		UE_LOG(LogTemp, Log, TEXT("T66GameMode BeginPlay - Coliseum"));
		return;
	}

	// The Lab: minimal arena, hero + companion only; no waves, NPCs, gates, or run progress.
	if (IsLabLevel())
	{
		if (UGameInstance* GILab = GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GILab->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->ResetForNewRun();
			}
			if (UT66DamageLogSubsystem* DamageLog = GILab->GetSubsystem<UT66DamageLogSubsystem>())
			{
				DamageLog->ResetForNewRun();
			}
		}
		if (bAutoSetupLevel)
		{
			EnsureLevelSetup();
		}
		UE_LOG(LogTemp, Log, TEXT("T66GameMode BeginPlay - Lab"));
		return;
	}

	UT66GameInstance* GIAsT66 = GetT66GameInstance();
	const bool bStageCatchUp = (GIAsT66 && GIAsT66->bStageCatchUpPending);

	// Stage Catch Up: spawn a separate platform and skip normal stage content.
	if (bStageCatchUp)
	{
		SpawnStageCatchUpPlatformAndInteractables();
		UE_LOG(LogTemp, Log, TEXT("T66GameMode BeginPlay - StageCatchUp"));
		return;
	}

	// Normal stage: defer all ground-dependent spawns until next tick so the landscape is fully formed and collision is ready.
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimerForNextTick(this, &AT66GameMode::SpawnLevelContentAfterLandscapeReady);
	}
	UE_LOG(LogTemp, Log, TEXT("T66GameMode BeginPlay - Level setup scheduled; content will spawn after landscape is ready."));
}

void AT66GameMode::SpawnLevelContentAfterLandscapeReady()
{
	// Phase 0: Main flat floor (no external asset packs).
	SpawnLowPolyNatureEnvironment();

	// Start gate: spawned here (after floor) so the ground trace hits the flat floor.
	if (!IsColiseumStage() && !IsLabLevel())
	{
		UWorld* W = GetWorld();
		AController* PC = W ? W->GetFirstPlayerController() : nullptr;
		if (PC)
		{
			SpawnStartGateForPlayer(PC);
		}
	}

	// Phase 1: Spawn ground-dependent structures (houses, NPCs, world interactables, tiles).
	SpawnCornerHousesAndNPCs();
	SpawnTricksterAndCowardiceGate();
	SpawnWorldInteractablesForStage();
	SpawnModelShowcaseRow();

	// Scatter decorative props.
	if (UGameInstance* PropGI = GetGameInstance())
	{
		if (UT66PropSubsystem* PropSub = PropGI->GetSubsystem<UT66PropSubsystem>())
		{
			UT66GameInstance* T66GI_Props = GetT66GameInstance();
			const int32 PropSeed = (T66GI_Props && T66GI_Props->RunSeed != 0) ? T66GI_Props->RunSeed : FMath::Rand();
			PropSub->SpawnPropsForStage(GetWorld(), PropSeed);
		}
	}

	SpawnStageEffectsForStage();
	SpawnTutorialIfNeeded();

	UE_LOG(LogTemp, Log, TEXT("T66GameMode - Phase 1 content spawned (structures + NPCs)."));

	// [GOLD] Phase 2 is now staggered across 4 ticks to eliminate the ~1 second hitch:
	//   Tick 1: Preload character visuals for this stage's mobs + boss (sync loads happen here, before combat)
	//   Tick 2: Spawn miasma boundary + miasma manager (wall visuals + dynamic materials)
	//   Tick 3: Spawn enemy director (triggers first wave ??? enemies are cheap now because visuals are cached)
	//   Tick 4: Spawn boss + boss gate
	UWorld* World = GetWorld();
	if (World)
	{
		// --- Tick 1: Preload character visuals ---
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
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
					// Preload mob visuals (these are the ones that cause sync loads on first enemy spawn).
					if (!StageData.EnemyA.IsNone()) Visuals->PreloadCharacterVisual(StageData.EnemyA);
					if (!StageData.EnemyB.IsNone()) Visuals->PreloadCharacterVisual(StageData.EnemyB);
					if (!StageData.EnemyC.IsNone()) Visuals->PreloadCharacterVisual(StageData.EnemyC);
					// Preload shared boss visual (all stages use "Boss" mesh).
					Visuals->PreloadCharacterVisual(FName(TEXT("Boss")));

					UE_LOG(LogTemp, Log, TEXT("[GOLD] Phase2-Preload: pre-resolved visuals for stage %d (EnemyA=%s, EnemyB=%s, EnemyC=%s, Boss=%s) in %.1fms"),
						StageNum,
						*StageData.EnemyA.ToString(), *StageData.EnemyB.ToString(), *StageData.EnemyC.ToString(),
						*StageData.BossID.ToString(),
						(FPlatformTime::Seconds() - PreloadStart) * 1000.0);
				}
				// Also preload fallback mob IDs in case DT_Stages isn't fully wired.
				const FName FallbackA = FName(*FString::Printf(TEXT("Mob_Stage%02d_A"), RunState->GetCurrentStage()));
				const FName FallbackB = FName(*FString::Printf(TEXT("Mob_Stage%02d_B"), RunState->GetCurrentStage()));
				const FName FallbackC = FName(*FString::Printf(TEXT("Mob_Stage%02d_C"), RunState->GetCurrentStage()));
				Visuals->PreloadCharacterVisual(FallbackA);
				Visuals->PreloadCharacterVisual(FallbackB);
				Visuals->PreloadCharacterVisual(FallbackC);
			}

			UE_LOG(LogTemp, Log, TEXT("[GOLD] Phase2-Preload: total preload time %.1fms"), (FPlatformTime::Seconds() - PreloadStart) * 1000.0);

			// --- Tick 2: Spawn miasma systems ---
			if (UWorld* W2 = GetWorld())
			{
				W2->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					UWorld* W = GetWorld();
					if (W)
					{
						MiasmaManager = W->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
						W->SpawnActor<AT66MiasmaBoundary>(AT66MiasmaBoundary::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
					}
					UE_LOG(LogTemp, Log, TEXT("[GOLD] Phase2-Tick2: miasma systems spawned."));

					// --- Tick 3: Spawn enemy director ---
					if (UWorld* W3 = GetWorld())
					{
						W3->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
						{
							FActorSpawnParameters SpawnParams;
							SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
							UWorld* W = GetWorld();
							if (W)
							{
								W->SpawnActor<AT66EnemyDirector>(AT66EnemyDirector::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
							}
							UE_LOG(LogTemp, Log, TEXT("[GOLD] Phase2-Tick3: enemy director spawned."));

							// --- Tick 4: Spawn boss + boss gate ---
							if (UWorld* W4 = GetWorld())
							{
								W4->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
								{
									SpawnBossForCurrentStage();
									SpawnBossGateIfNeeded();
									UE_LOG(LogTemp, Log, TEXT("[GOLD] Phase2-Tick4: boss + boss gate spawned. Phase 2 complete."));
									UE_LOG(LogTemp, Log, TEXT("T66GameMode - Phase 2 content spawned (combat systems + boss)."));
								}));
							}
						}));
					}
				}));
			}
		}));
	}
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

	// Only run tutorial when forced by the dev switch (spawn in tutorial arena).
	if (!bForceSpawnInTutorialArea) return;

	UWorld* World = GetWorld();
	if (!World) return;

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
	if (IsColiseumStage()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!RunState || !T66GI) return;
	if (T66GI->bStageCatchUpPending) return;

	// Stage effects are per-difficulty. Easy = Shroom; other difficulties have none yet.
	if (T66GI->SelectedDifficulty != ET66Difficulty::Easy) return;

	const int32 StageNum = RunState->GetCurrentStage();

	const int32 RunSeed = (T66GI && T66GI->RunSeed != 0) ? T66GI->RunSeed : FMath::Rand();
	FRandomStream Rng(RunSeed + StageNum * 971 + 17);

	static constexpr float MainHalfExtent = 50000.f;
	static constexpr float SpawnZ = 40.f;
	static constexpr float MinDistBetween = 420.f;
	static constexpr float SafeBubbleMargin = 350.f;
	static constexpr int32 ShroomCount = 12;

	TArray<FVector> UsedLocs;

	auto IsInsideNoSpawnZone = [&](const FVector& L) -> bool
	{
		static constexpr float StartBoxWest = -40000.f, StartBoxEast = -30909.f;
		static constexpr float StartBoxNorth = 4545.f, StartBoxSouth = -4545.f;
		static constexpr float StartMargin = 455.f;
		if (L.X >= (StartBoxWest - StartMargin) && L.X <= (StartBoxEast + StartMargin) &&
		    L.Y >= (StartBoxSouth - StartMargin) && L.Y <= (StartBoxNorth + StartMargin))
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

	auto IsGoodLoc = [&](const FVector& L) -> bool
	{
		if (IsInsideNoSpawnZone(L)) return false;
		for (const FVector& U : UsedLocs)
		{
			if (FVector::DistSquared2D(L, U) < (MinDistBetween * MinDistBetween)) return false;
		}
		for (TActorIterator<AT66HouseNPCBase> It(World); It; ++It)
		{
			const AT66HouseNPCBase* NPC = *It;
			if (!NPC) continue;
			const float R = NPC->GetSafeZoneRadius() + SafeBubbleMargin;
			if (FVector::DistSquared2D(L, NPC->GetActorLocation()) < (R * R)) return false;
		}
		return true;
	};

	struct FStageEffectSpawnHit { FVector Loc; FVector Normal; };
	auto FindSpawnLoc = [&]() -> FStageEffectSpawnHit
	{
		const FVector Up = FVector::UpVector;
		for (int32 Try = 0; Try < 80; ++Try)
		{
			const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			FVector Loc(X, Y, SpawnZ);
			FVector Normal = Up;

			FHitResult Hit;
			const FVector Start = Loc + FVector(0.f, 0.f, 2000.f);
			const FVector End = Loc - FVector(0.f, 0.f, 6000.f);
			if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
			{
				Loc = Hit.ImpactPoint;
				Normal = Hit.ImpactNormal.GetSafeNormal(1e-4f, Up);
			}

			if (IsGoodLoc(Loc)) return { Loc, Normal };
		}
		return { FVector(0.f, 0.f, SpawnZ), Up };
	};

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < ShroomCount; ++i)
	{
		const FStageEffectSpawnHit Th = FindSpawnLoc();
		AT66Shroom* Shroom = World->SpawnActor<AT66Shroom>(AT66Shroom::StaticClass(), Th.Loc, FRotator::ZeroRotator, P);
		if (Shroom)
		{
			UsedLocs.Add(Th.Loc);
		}
	}
}

void AT66GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Frame-level lag: log when a frame exceeded budget (e.g. >20ms).
	if (DeltaTime > 0.02f)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66LagTrackerSubsystem* Lag = GI->GetSubsystem<UT66LagTrackerSubsystem>())
			{
				if (Lag->IsEnabled())
				{
					UE_LOG(LogTemp, Warning, TEXT("[LAG] Frame: %.2fms (%.1f FPS)"), DeltaTime * 1000.f, 1.f / FMath::Max(0.001f, DeltaTime));
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
	for (TActorIterator<AT66EnemyBase> It(GetWorld()); It; ++It)
	{
		if (AT66EnemyBase* E = *It)
		{
			E->ApplyStageScaling(Stage);
			E->ApplyDifficultyScalar(Scalar);
		}
	}

	// Bosses: scale HP + projectile damage (count unchanged).
	for (TActorIterator<AT66BossBase> It(GetWorld()); It; ++It)
	{
		if (AT66BossBase* B = *It)
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

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
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
		for (TActorIterator<AT66HouseNPCBase> It(World); It; ++It)
		{
			AT66HouseNPCBase* NPC = *It;
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
		Pawn->SetActorTransform(GI->PendingLoadedTransform);
		GI->bApplyLoadedTransform = false;
		GI->PendingLoadedTransform = FTransform();
	}

	if (GI && GI->bLoadAsPreview && NewPlayer)
	{
		if (AT66PlayerController* PC = Cast<AT66PlayerController>(NewPlayer))
		{
			PC->SetPause(true);
			PC->ShowLoadPreviewOverlay();
		}
	}
}

void AT66GameMode::SpawnBossGateIfNeeded()
{
	if (IsColiseumStage()) return;
	if (BossGate) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Between main square and boss square (pillars).
	FVector BossGateLoc(27273.f, 0.f, 0.f);
	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, BossGateLoc + FVector(0.f, 0.f, 3000.f), BossGateLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
	{
		BossGateLoc.Z = Hit.ImpactPoint.Z;
	}
	BossGate = World->SpawnActor<AT66BossGate>(AT66BossGate::StaticClass(), BossGateLoc, FRotator::ZeroRotator, SpawnParams);
}

bool AT66GameMode::IsColiseumStage() const
{
	if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->bForceColiseumMode;
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

bool AT66GameMode::IsDemoMapForTribulation() const
{
	UWorld* World = GetWorld();
	if (!World) return false;
	FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
	return MapName.Equals(UT66GameInstance::GetDemoMapLevelNameForTribulation().ToString(), ESearchCase::IgnoreCase);
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


void AT66GameMode::SpawnTricksterAndCowardiceGate()
{
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	const int32 StageNum = RunState->GetCurrentStage();
	// Only on normal stages (NOT ending in 5 or 0) => stage % 5 != 0
	if ((StageNum % 5) == 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Place right before the boss pillars (main->boss connector).
	const FVector BossGateRef(27273.f, 0.f, 0.f);
	FVector GateLoc = BossGateRef + FVector(-800.f, 0.f, 0.f);
	FVector TricksterLoc = GateLoc + FVector(-250.f, 200.f, 0.f);

	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, GateLoc + FVector(0.f, 0.f, 3000.f), GateLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
	{
		GateLoc.Z = Hit.ImpactPoint.Z;
	}
	if (World->LineTraceSingleByChannel(Hit, TricksterLoc + FVector(0.f, 0.f, 3000.f), TricksterLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
	{
		TricksterLoc.Z = Hit.ImpactPoint.Z;
	}

	CowardiceGate = World->SpawnActor<AT66CowardiceGate>(AT66CowardiceGate::StaticClass(), GateLoc, FRotator::ZeroRotator, SpawnParams);
	TricksterNPC = World->SpawnActor<AT66TricksterNPC>(AT66TricksterNPC::StaticClass(), TricksterLoc, FRotator::ZeroRotator, SpawnParams);
	if (TricksterNPC)
	{
		TricksterNPC->ApplyVisuals();
	}
}

void AT66GameMode::ResetColiseumState()
{
	ColiseumBossesRemaining = 0;
	bColiseumExitGateSpawned = false;
}

void AT66GameMode::HandleBossDefeated(AT66BossBase* Boss)
{
	UWorld* World = GetWorld();
	if (!World) return;
	const FVector Location = Boss ? Boss->GetActorLocation() : FVector::ZeroVector;
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

	// Power Coupons: 1 per boss kill (stage boss or Coliseum).
	if (RunState)
	{
		RunState->AddPowerCrystalsEarnedThisRun(1);
	}

	if (IsColiseumStage())
	{
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

	// First-time boss defeat unlock => spawn recruitable companion (stages 1..60 excluding 0/5).
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
	}

	// Normal stage: boss dead => miasma disappears and Stage Gate appears.
	ClearMiasma();
	SpawnStageGateAtLocation(Location);

	// Idol altar cadence for the 33-stage structure:
	// - spawn after most cleared stages
	// - skip checkpoint / difficulty-ending stages (5,10,15,20,25,30)
	// - skip the final segment entirely (31,32,33)
	if (RunState)
	{
		const int32 ClearedStage = RunState->GetCurrentStage();
		const bool bIsCheckpointStage = (ClearedStage > 0) && ((ClearedStage % 5) == 0);
		const bool bIsFinalSegmentStage = (ClearedStage >= 31);
		if (!bIsCheckpointStage && !bIsFinalSegmentStage)
		{
			SpawnIdolAltarAtLocation(Location);
		}
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
	if (!GI || GI->SelectedCompanionID.IsNone()) return;

	FCompanionData CompanionData;
	if (!GI->GetCompanionData(GI->SelectedCompanionID, CompanionData)) return;

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
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FName CompanionSkinID = FName(TEXT("Default"));
	if (UGameInstance* GII = GetGameInstance())
	{
		if (UT66AchievementsSubsystem* Ach = GII->GetSubsystem<UT66AchievementsSubsystem>())
		{
			CompanionSkinID = Ach->GetEquippedCompanionSkinID(GI->SelectedCompanionID);
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
		UE_LOG(LogTemp, Log, TEXT("Spawned companion: %s"), *CompanionData.DisplayName.ToString());
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
		UE_LOG(LogTemp, Log, TEXT("Spawned vendor NPC near hero"));
	}
}

void AT66GameMode::SpawnCornerHousesAndNPCs()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Hero spawn location (same as PlayerStart / default spawn); NPCs spawn around it.
	float HeroX = -35455.f;
	float HeroY = 0.f;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		const FVector Loc = (*It)->GetActorLocation();
		HeroX = Loc.X;
		HeroY = Loc.Y;
		break;
	}
	const float Offset = 600.f;  // NPCs in a cross around hero spawn
	struct FNPCPosition { float X; float Y; };
	const FNPCPosition NPCPositions[] = {
		{ HeroX + Offset, HeroY },
		{ HeroX - Offset, HeroY },
		{ HeroX, HeroY + Offset },
		{ HeroX, HeroY - Offset },
	};

	struct FNPCDef
	{
		TSubclassOf<AActor> NPCClass;
		FText Name;
		FLinearColor Color;
	};
	TArray<FNPCDef> NPCDefs = {
		{ AT66VendorNPC::StaticClass(),    NSLOCTEXT("T66.NPC", "Vendor", "Vendor"),        FLinearColor(0.05f,0.05f,0.05f,1.f) },
		{ AT66GamblerNPC::StaticClass(),   NSLOCTEXT("T66.NPC", "Gambler", "Gambler"),      FLinearColor(0.8f,0.1f,0.1f,1.f) },
		{ AT66OuroborosNPC::StaticClass(), NSLOCTEXT("T66.NPC", "Ouroboros", "Ouroboros"),  FLinearColor(0.1f,0.8f,0.2f,1.f) },
		{ AT66SaintNPC::StaticClass(),     NSLOCTEXT("T66.NPC", "Saint", "Saint"),          FLinearColor(0.9f,0.9f,0.9f,1.f) },
	};

	// Shuffle NPC assignment to corners so each run has different NPCs at each corner.
	UT66GameInstance* T66GI = GetT66GameInstance();
	const int32 RunSeed = (T66GI && T66GI->RunSeed != 0) ? T66GI->RunSeed : FMath::Rand();
	FRandomStream ShuffleRng(RunSeed + 991);
	for (int32 i = NPCDefs.Num() - 1; i > 0; --i)
	{
		const int32 j = ShuffleRng.RandRange(0, i);
		NPCDefs.Swap(i, j);
	}

	// Find closest flat surface to a reference point (flat = normal nearly vertical). Search in rings outward.
	auto FindClosestFlatSurface = [World](float RefX, float RefY) -> FVector
	{
		static constexpr float MinNormalZ = 0.92f;   // ~22?? max slope
		static constexpr float SearchRadiusMax = 1200.f;
		static constexpr float RadiusStep = 120.f;
		static constexpr int32 NumAngles = 16;

		FVector Fallback(RefX, RefY, 60.f);
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, FVector(RefX, RefY, 2000.f), FVector(RefX, RefY, -4000.f), ECC_WorldStatic))
		{
			Fallback = Hit.ImpactPoint;
		}

		for (float R = 0.f; R <= SearchRadiusMax; R += RadiusStep)
		{
			const int32 AngleSteps = (R <= 0.f) ? 1 : NumAngles;
			for (int32 a = 0; a < AngleSteps; ++a)
			{
				const float Angle = (AngleSteps == 1) ? 0.f : (2.f * PI * static_cast<float>(a) / static_cast<float>(AngleSteps));
				const float X = RefX + R * FMath::Cos(Angle);
				const float Y = RefY + R * FMath::Sin(Angle);
				const FVector Start(X, Y, 2000.f);
				const FVector End(X, Y, -4000.f);
				if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic) && Hit.ImpactNormal.Z >= MinNormalZ)
				{
					return Hit.ImpactPoint;
				}
			}
		}
		return Fallback;
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < 4; ++i)
	{
		const FVector FlatLoc = FindClosestFlatSurface(NPCPositions[i].X, NPCPositions[i].Y);
		AActor* SpawnedNPC = World->SpawnActor<AActor>(NPCDefs[i].NPCClass, FlatLoc, FRotator::ZeroRotator, SpawnParams);
		if (AT66HouseNPCBase* NPC = Cast<AT66HouseNPCBase>(SpawnedNPC))
		{
			NPC->NPCName = NPCDefs[i].Name;
			NPC->NPCColor = NPCDefs[i].Color;
			NPC->ApplyVisuals();
		}
	}
}

void AT66GameMode::SpawnStartGateForPlayer(AController* Player)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Spawn once per level (gate is a world landmark).
	if (StartGate) return;

	// Hero spawn location (match SpawnCornerHousesAndNPCs). NPCs are at Offset 600 in a cross; gate just outside (east).
	float HeroX = -35455.f;
	float HeroY = 0.f;
	APawn* Pawn = Player ? Player->GetPawn() : nullptr;
	if (Pawn)
	{
		const FVector Loc = Pawn->GetActorLocation();
		HeroX = Loc.X;
		HeroY = Loc.Y;
	}
	else
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			const FVector Loc = (*It)->GetActorLocation();
			HeroX = Loc.X;
			HeroY = Loc.Y;
			break;
		}
	}

	// Gate right in front of hero (short offset). Place on ground like NPCs (trace ImpactPoint, no float).
	static constexpr float GateOffsetEast = 200.f;
	static constexpr float GateZOffset = 5.f; // small offset to avoid z-fight
	FVector GateLoc(HeroX + GateOffsetEast, HeroY, 0.f);
	float HeroZ = Pawn ? Pawn->GetActorLocation().Z : 200.f;
	float GateGroundZ = HeroZ; // fallback if trace misses
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
		UE_LOG(LogTemp, Log, TEXT("Spawned Start Gate near hero at (%.0f, %.0f, %.0f)"), GateLoc.X, GateLoc.Y, GateLoc.Z);
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

void AT66GameMode::SpawnWorldInteractablesForStage()
{
	if (IsColiseumStage()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();

	// Run-level seed so positions change every time "Enter the Tribulation" or PIE is started (like procedural terrain).
	const int32 RunSeed = (T66GI && T66GI->RunSeed != 0) ? T66GI->RunSeed : FMath::Rand();
	const int32 StageNum = RunState->GetCurrentStage();
	FRandomStream Rng(RunSeed + StageNum * 1337 + 42);

	// Main map square bounds. 100k map: half-extent 50000. Match miasma SafeHalfExtent.
	static constexpr float MainHalfExtent = 50000.f;
	static constexpr float SpawnZ = 220.f;
	static constexpr float MinDistBetweenInteractables = 900.f;
	static constexpr float SafeBubbleMargin = 250.f;

	TArray<FVector> UsedLocs;

	// No-spawn zones: keep gameplay spawns out of start box and special arenas (scaled for 100k).
	auto IsInsideNoSpawnZone = [&](const FVector& L) -> bool
	{
		static constexpr float StartBoxWest = -40000.f, StartBoxEast = -30909.f;
		static constexpr float StartBoxNorth = 4545.f, StartBoxSouth = -4545.f;
		static constexpr float StartMargin = 455.f;
		if (L.X >= (StartBoxWest - StartMargin) && L.X <= (StartBoxEast + StartMargin) &&
		    L.Y >= (StartBoxSouth - StartMargin) && L.Y <= (StartBoxNorth + StartMargin))
		{
			return true;
		}

		static constexpr float ArenaHalf = 9091.f;
		static constexpr float ArenaMargin = 682.f;
		static constexpr float TutorialArenaHalf = 9091.f;
		struct FArena2D { float X; float Y; float Half; };
		static constexpr FArena2D Arenas[] = {
			{ -22727.f, 34091.f, ArenaHalf }, // Boost
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

	auto IsGoodLoc = [&](const FVector& L) -> bool
	{
		if (IsInsideNoSpawnZone(L))
		{
			return false;
		}
		for (const FVector& U : UsedLocs)
		{
			if (FVector::DistSquared2D(L, U) < (MinDistBetweenInteractables * MinDistBetweenInteractables))
			{
				return false;
			}
		}
		for (TActorIterator<AT66HouseNPCBase> It(World); It; ++It)
		{
			const AT66HouseNPCBase* NPC = *It;
			if (!NPC) continue;
			const float R = NPC->GetSafeZoneRadius() + SafeBubbleMargin;
			if (FVector::DistSquared2D(L, NPC->GetActorLocation()) < (R * R))
			{
				return false;
			}
		}
		return true;
	};

	struct FSpawnHitResult { FVector Loc; };
	auto FindSpawnLoc = [&]() -> FSpawnHitResult
	{
		for (int32 Try = 0; Try < 40; ++Try)
		{
			const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			FVector Loc(X, Y, SpawnZ);

			FHitResult Hit;
			const FVector Start = Loc + FVector(0.f, 0.f, 8000.f);
			const FVector End = Loc - FVector(0.f, 0.f, 16000.f);
			if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
			{
				continue;
			}
			Loc = Hit.ImpactPoint;

			if (IsGoodLoc(Loc))
			{
				return { Loc };
			}
		}
		return { FVector(0.f, 0.f, SpawnZ) };
	};

	auto SpawnOne = [&](UClass* Cls) -> AActor*
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		const FSpawnHitResult HitResult = FindSpawnLoc();
		AActor* A = World->SpawnActor<AActor>(Cls, HitResult.Loc, FRotator::ZeroRotator, P);
		if (A) UsedLocs.Add(HitResult.Loc);
		return A;
	};

	if (RngSub)
	{
		RngSub->UpdateLuckStat(RunState->GetLuckStat());
	}

	const UT66RngTuningConfig* Tuning = RngSub ? RngSub->GetTuning() : nullptr;

	// Luck-affected counts use central tuning. Locations are still stage-seeded (not luck-affected).
	const int32 CountFountains = (RngSub && Tuning) ? RngSub->RollIntRangeBiased(Tuning->TreesPerStage, Rng) : Rng.RandRange(2, 5);
	const int32 CountChests = (RngSub && Tuning) ? RngSub->RollIntRangeBiased(Tuning->ChestsPerStage, Rng) : Rng.RandRange(2, 5);
	const int32 CountWheels = (RngSub && Tuning) ? RngSub->RollIntRangeBiased(Tuning->WheelsPerStage, Rng) : Rng.RandRange(2, 5);

	// Luck Rating tracking (quantity).
	if (RunState)
	{
		const int32 FountainsMin = (Tuning ? Tuning->TreesPerStage.Min : 2);
		const int32 FountainsMax = (Tuning ? Tuning->TreesPerStage.Max : 5);
		const int32 ChestsMin = (Tuning ? Tuning->ChestsPerStage.Min : 2);
		const int32 ChestsMax = (Tuning ? Tuning->ChestsPerStage.Max : 5);
		const int32 WheelsMin = (Tuning ? Tuning->WheelsPerStage.Min : 2);
		const int32 WheelsMax = (Tuning ? Tuning->WheelsPerStage.Max : 5);
		RunState->RecordLuckQuantityRoll(FName(TEXT("FountainsPerStage")), CountFountains, FountainsMin, FountainsMax);
		RunState->RecordLuckQuantityRoll(FName(TEXT("ChestsPerStage")), CountChests, ChestsMin, ChestsMax);
		RunState->RecordLuckQuantityRoll(FName(TEXT("WheelsPerStage")), CountWheels, WheelsMin, WheelsMax);
	}

	// Not luck-affected (for now).
	const int32 CountTotems = Rng.RandRange(2, 5);

	for (int32 i = 0; i < CountFountains; ++i)
	{
		if (AT66FountainOfLifeInteractable* Fountain = Cast<AT66FountainOfLifeInteractable>(SpawnOne(AT66FountainOfLifeInteractable::StaticClass())))
		{
			Fountain->SetRarity(ET66Rarity::Black);
		}
	}
	for (int32 i = 0; i < CountChests; ++i)
	{
		if (AT66ChestInteractable* Chest = Cast<AT66ChestInteractable>(SpawnOne(AT66ChestInteractable::StaticClass())))
		{
			const float MimicChance = Tuning ? FMath::Clamp(Tuning->ChestMimicChance, 0.f, 1.f) : 0.20f;
			Chest->bIsMimic = (Rng.GetFraction() < MimicChance);

			const FT66RarityWeights Weights = Tuning ? Tuning->InteractableRarityBase : FT66RarityWeights{};
			const ET66Rarity R = (RngSub && Tuning) ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
			Chest->SetRarity(R);
			if (RunState)
			{
				RunState->RecordLuckQualityRarity(FName(TEXT("ChestRarity")), R);
			}
		}
	}
	for (int32 i = 0; i < CountWheels; ++i)
	{
		if (AT66WheelSpinInteractable* Wheel = Cast<AT66WheelSpinInteractable>(SpawnOne(AT66WheelSpinInteractable::StaticClass())))
		{
			const FT66RarityWeights Weights = Tuning ? Tuning->WheelRarityBase : FT66RarityWeights{};
			const ET66Rarity R = (RngSub && Tuning) ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
			Wheel->SetRarity(R);
			if (RunState)
			{
				RunState->RecordLuckQualityRarity(FName(TEXT("WheelRarity")), R);
			}
		}
	}

	// Always spawn one wheel right next to the hero (at hero spawn position + offset).
	if (!IsColiseumStage() && !IsLabLevel())
	{
		float HeroX = -35455.f;
		float HeroY = 0.f;
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			const FVector Loc = (*It)->GetActorLocation();
			HeroX = Loc.X;
			HeroY = Loc.Y;
			break;
		}
		static constexpr float HeroWheelOffset = 380.f;  // to the right of hero spawn
		const float WheelX = HeroX + HeroWheelOffset;
		const float WheelY = HeroY;
		FVector WheelLoc(WheelX, WheelY, SpawnZ);
		FHitResult Hit;
		const FVector TraceStart(WheelX, WheelY, 8000.f);
		const FVector TraceEnd(WheelX, WheelY, -16000.f);
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			WheelLoc = Hit.ImpactPoint;
		}
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (AT66WheelSpinInteractable* HeroWheel = World->SpawnActor<AT66WheelSpinInteractable>(AT66WheelSpinInteractable::StaticClass(), WheelLoc, FRotator::ZeroRotator, P))
		{
			const FT66RarityWeights Weights = Tuning ? Tuning->WheelRarityBase : FT66RarityWeights{};
			const ET66Rarity R = (RngSub && Tuning) ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
			HeroWheel->SetRarity(R);
			if (RunState)
			{
				RunState->RecordLuckQualityRarity(FName(TEXT("WheelRarity")), R);
			}
			UsedLocs.Add(WheelLoc);
		}
	}

	const int32 CountCrates = (RngSub && Tuning) ? RngSub->RollIntRangeBiased(Tuning->CratesPerStage, Rng) : Rng.RandRange(1, 3);
	for (int32 i = 0; i < CountCrates; ++i)
	{
		SpawnOne(AT66CrateInteractable::StaticClass());
	}

	for (int32 i = 0; i < CountTotems; ++i)
	{
		if (AT66DifficultyTotem* Totem = Cast<AT66DifficultyTotem>(SpawnOne(AT66DifficultyTotem::StaticClass())))
		{
			// Not luck-affected (per current scope).
			Totem->SetRarity(FT66RarityUtil::RollDefaultRarity(Rng));
		}
	}
}

void AT66GameMode::SpawnModelShowcaseRow()
{
	if (IsColiseumStage() || IsLabLevel()) return;
	UWorld* World = GetWorld();
	if (!World) return;

	constexpr float ShowcaseX = -32955.f;
	constexpr float BagShowcaseX = -34205.f;
	constexpr float Spacing = 800.f;

	static const TCHAR* MeshPaths[] = {
		TEXT("/Game/World/Interactables/Chests/Black/ChestBlack.ChestBlack"),
		TEXT("/Game/World/Interactables/Chests/Red/ChestRed.ChestRed"),
		TEXT("/Game/World/Interactables/Chests/White/ChestWhite.ChestWhite"),
		TEXT("/Game/World/Interactables/Chests/Yellow/ChestYellow.ChestYellow"),
		TEXT("/Game/World/Interactables/Crate.Crate"),
		TEXT("/Game/World/Interactables/Totem.Totem"),
		TEXT("/Game/World/Interactables/Wheels/Wheel.Wheel"),
		TEXT("/Game/World/Props/Barn.Barn"),
		TEXT("/Game/World/Props/Hay.Hay"),
		TEXT("/Game/World/Props/Hay2.Hay2"),
		TEXT("/Game/World/Props/Tree.Tree"),
		TEXT("/Game/World/Props/Tree2.Tree2"),
	};
	static const TCHAR* BagMeshPaths[] = {
		TEXT("/Game/World/LootBags/Black/SM_LootBag_Black.SM_LootBag_Black"),
		TEXT("/Game/World/LootBags/Red/SM_LootBag_Red.SM_LootBag_Red"),
		TEXT("/Game/World/LootBags/White/SM_LootBag_White.SM_LootBag_White"),
		TEXT("/Game/World/LootBags/Yellow/SM_LootBag_Yellow.SM_LootBag_Yellow"),
	};

	const int32 Count = UE_ARRAY_COUNT(MeshPaths);
	const float TotalWidth = (Count - 1) * Spacing;
	const float StartY = -TotalWidth * 0.5f;

	FActorSpawnParameters SP;
	SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	auto SpawnShowcaseMesh = [&](const TCHAR* MeshPath, const FVector& TraceAnchor)
	{
		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath);
		if (!Mesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Showcase] Mesh not found: %s"), MeshPath);
			return;
		}

		// Ground trace to find terrain surface
		FVector SpawnLoc(TraceAnchor.X, TraceAnchor.Y, 220.f);
		FHitResult Hit;
		const FVector TraceStart(TraceAnchor.X, TraceAnchor.Y, 2000.f);
		const FVector TraceEnd(TraceAnchor.X, TraceAnchor.Y, -4000.f);
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			SpawnLoc = Hit.ImpactPoint;
		}

		// Offset so the mesh bottom sits on the ground
		const FBoxSphereBounds B = Mesh->GetBounds();
		const float MeshBottomZ = B.Origin.Z - B.BoxExtent.Z;
		SpawnLoc.Z -= MeshBottomZ;

		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SP);
		if (!Actor) return;

		UStaticMeshComponent* SMC = Actor->GetStaticMeshComponent();
		if (SMC)
		{
			SMC->SetMobility(EComponentMobility::Movable);
			SMC->SetStaticMesh(Mesh);
			SMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	};

	for (int32 i = 0; i < Count; ++i)
	{
		const float Y = StartY + i * Spacing;
		SpawnShowcaseMesh(MeshPaths[i], FVector(ShowcaseX, Y, 0.f));
	}

	const int32 BagCount = UE_ARRAY_COUNT(BagMeshPaths);
	for (int32 i = 0; i < BagCount; ++i)
	{
		const float Y = StartY + i * Spacing;
		SpawnShowcaseMesh(BagMeshPaths[i], FVector(BagShowcaseX, Y, 0.f));
	}

	UE_LOG(LogTemp, Log, TEXT("[Showcase] Spawned %d showcase actors including %d loot bags."), Count + BagCount, BagCount);
}

void AT66GameMode::SpawnIdolAltarForPlayer(AController* Player)
{
	if (IsColiseumStage()) return;
	// v1: Idol altars only spawn after boss kills (one per stage).
}

void AT66GameMode::SpawnIdolAltarAtLocation(const FVector& Location)
{
	if (IsColiseumStage()) return;
	if (IdolAltar) return;

	UWorld* World = GetWorld();
	if (!World) return;

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
}

void AT66GameMode::SpawnStageCatchUpPlatformAndInteractables()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return;

	RunState->SetInStageCatchUp(true);

	// Difficulty index: Easy=0, Medium=1, Hard=2, ...
	const int32 DiffIndex = FMath::Max(0, static_cast<int32>(T66GI->SelectedDifficulty));
	const int32 StartStage = FMath::Clamp(1 + (DiffIndex * 5), 1, 33);

	const int32 GoldAmount = 200 * DiffIndex;
	const int32 LootBags = 2 + (DiffIndex * 2);

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

	// Snap to ground at the boss death XY so the gate is never floating/sunk.
	FVector SpawnLoc(Location.X, Location.Y, Location.Z);
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
		UE_LOG(LogTemp, Log, TEXT("Spawned Stage Gate at boss death location"));
	}
}

void AT66GameMode::SpawnBossForCurrentStage()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (IsColiseumStage())
	{
		SpawnAllOwedBossesInColiseum();
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

	// Map layout: spawn the stage boss in the Boss Area square (right after pillars, inside safe zone).
	{
		FVector BossLoc(35455.f, 0.f, 200.f);
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
		// Stage-scaled fallback so all 33 stages work even if DT_Bosses isn't reimported yet.
		const int32 S = FMath::Clamp(StageNum, 1, 33);
		const float T = static_cast<float>(S - 1) / 32.f; // 0..1

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
		// Snap to ground so the boss never floats/sinks.
		FHitResult Hit;
		const FVector Start = Boss->GetActorLocation() + FVector(0.f, 0.f, 4000.f);
		const FVector End = Boss->GetActorLocation() - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			const float HalfHeight = Boss->GetCapsuleComponent() ? Boss->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.f;
			Boss->SetActorLocation(Hit.ImpactPoint + FVector(0.f, 0.f, HalfHeight), false, nullptr, ETeleportType::TeleportPhysics);
		}

		Boss->InitializeBoss(BossData);
		StageBoss = Boss;
		UE_LOG(LogTemp, Log, TEXT("Spawned boss for Stage %d (BossID=%s)"), StageNum, *BossData.BossID.ToString());

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
						StageBoss = NewBoss;
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
	UE_LOG(LogTemp, Log, TEXT("Checking level setup..."));

	// Destroy any Landscape or editor-placed foliage actors saved in the level (legacy from external asset packs).
	UWorld* CleanupWorld = GetWorld();
	if (CleanupWorld)
	{
		for (TActorIterator<ALandscape> It(CleanupWorld); It; ++It)
		{
			UE_LOG(LogTemp, Log, TEXT("[MAP] Destroying saved Landscape actor: %s"), *It->GetName());
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
			UE_LOG(LogTemp, Log, TEXT("[MAP] Destroyed %d legacy foliage actors."), ToDestroy.Num());
		}
	}

	if (IsLabLevel())
	{
		SpawnLabFloorIfNeeded();
		SpawnLightingIfNeeded();  // Same sky and lighting as gameplay: SkyAtmosphere (blue sky), sun, sky light, fog, post process
		SpawnLabCollectorIfNeeded();
		SpawnPlayerStartIfNeeded();
		return;
	}

	SpawnFloorIfNeeded();
	SpawnColiseumArenaIfNeeded();
	SpawnTutorialArenaIfNeeded();
	SpawnStartAreaWallsIfNeeded();
	SpawnBossAreaWallsIfNeeded();
	TryApplyGroundFloorMaterialToAllFloors();
	SpawnLightingIfNeeded();
	SpawnPlayerStartIfNeeded();
}

void AT66GameMode::TryApplyGroundFloorMaterialToAllFloors()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const int32 GroundVariantCount = GroundFloorMaterials.Num();
	if (GroundVariantCount == 0)
	{
		return;
	}

	// Check if all configured materials are loaded
	bool bAllLoaded = true;
	for (int32 i = 0; i < GroundVariantCount; ++i)
	{
		if (!GroundFloorMaterials[i].Get())
		{
			bAllLoaded = false;
			break;
		}
	}

	if (!bAllLoaded)
	{
		if (!bGroundFloorMaterialLoadRequested)
		{
			bGroundFloorMaterialLoadRequested = true;
			TArray<FSoftObjectPath> Paths;
			for (int32 i = 0; i < GroundVariantCount; ++i)
			{
				if (!GroundFloorMaterials[i].IsNull())
				{
					Paths.Add(GroundFloorMaterials[i].ToSoftObjectPath());
				}
			}
			if (Paths.Num() > 0)
			{
				TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
					Paths, FStreamableDelegate::CreateWeakLambda(this, [this]()
					{
						TryApplyGroundFloorMaterialToAllFloors();
					}));
				if (Handle.IsValid())
				{
					ActiveAsyncLoadHandles.Add(Handle);
				}
			}
		}
		return;
	}

	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		AStaticMeshActor* A = *It;
		if (!A) continue;
		if (!T66_HasAnyFloorTag(A)) continue;
		if (UStaticMeshComponent* SMC = A->GetStaticMeshComponent())
		{
			UMaterialInterface* Mat = GroundFloorMaterials[0].Get();  // fallback
			const FVector Loc = A->GetActorLocation();
			const float Seed = Loc.X * 0.000123f + Loc.Y * 0.000456f;
			const int32 Idx = FMath::Clamp(
				FMath::FloorToInt(FMath::Frac(FMath::Abs(Seed)) * static_cast<float>(GroundVariantCount)),
				0,
				GroundVariantCount - 1);
			if (Idx < GroundFloorMaterials.Num())
			{
				UMaterialInterface* M = GroundFloorMaterials[Idx].Get();
				if (M) Mat = M;
			}
			SMC->SetMaterial(0, Mat);
		}
	}

	// Also apply to non-StaticMeshActor floor-tagged actors (HISM walls, ProceduralMesh tops/ramps).
	UMaterialInterface* FallbackMat = GroundFloorMaterials[0].Get();
	if (FallbackMat)
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* A = *It;
			if (!A || Cast<AStaticMeshActor>(A)) continue;
			if (!T66_HasAnyFloorTag(A)) continue;

			TArray<UMeshComponent*> MeshComps;
			A->GetComponents<UMeshComponent>(MeshComps);
			for (UMeshComponent* MC : MeshComps)
			{
				if (MC)
				{
					MC->SetMaterial(0, FallbackMat);
				}
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

	UT66GameInstance* T66GI = GetT66GameInstance();
	if (T66GI && T66GI->bStageCatchUpPending)
	{
		// Keep the catch up platform area clean; tutorial arena not needed there.
		return;
	}

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

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	auto HasTag = [&](FName Tag) -> bool
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
			if (It->Tags.Contains(Tag)) return true;
		return false;
	};

	// Force synchronous load of ground materials so walls get the same Unlit atlas as platform walls.
	for (int32 i = 0; i < GroundFloorMaterials.Num(); ++i)
	{
		if (!GroundFloorMaterials[i].IsNull() && !GroundFloorMaterials[i].Get())
		{
			GroundFloorMaterials[i].LoadSynchronous();
		}
	}
	UMaterialInterface* FloorMat = (GroundFloorMaterials.Num() > 0) ? GroundFloorMaterials[0].Get() : nullptr;

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
	// T66_Floor_Main is spawned by SpawnLowPolyNatureEnvironment(); do not destroy it here.
	DestroyTaggedIfExists(FName("T66_Floor_Conn1"));
	DestroyTaggedIfExists(FName("T66_Floor_Conn2"));
	DestroyTaggedIfExists(FName("T66_Floor_Start"));
	DestroyTaggedIfExists(FName("T66_Floor_Boss"));

	// Island floors only: Coliseum and Boost (small islands). Tutorial floor is spawned in SpawnTutorialIfNeeded.
	constexpr float IslandFloorZ = -50.f;
	const TArray<FFloorSpec> Floors = {
		{ FName("T66_Floor_Coliseum"), FVector(ColiseumCenter.X, ColiseumCenter.Y, IslandFloorZ), FVector(40.f, 40.f, 1.f), FLinearColor(0.30f, 0.30f, 0.35f, 1.f) },
		{ FName("T66_Floor_CatchUp"),   FVector(-45455.f, 23636.f, IslandFloorZ), FVector(40.f, 40.f, 1.f), FLinearColor(0.30f, 0.30f, 0.35f, 1.f) },
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

		// Pick one of the available ground material variants by floor position (deterministic variety).
		UMaterialInterface* GroundMat = nullptr;
		const int32 GroundVariantCount = GroundFloorMaterials.Num();
		if (GroundVariantCount > 0)
		{
			const float Seed = Spec.Location.X * 0.000123f + Spec.Location.Y * 0.000456f;
			const int32 Idx = FMath::Clamp(
				FMath::FloorToInt(FMath::Frac(FMath::Abs(Seed)) * static_cast<float>(GroundVariantCount)),
				0,
				GroundVariantCount - 1);
			GroundMat = GroundFloorMaterials[Idx].Get();
		}
		if (GroundMat)
		{
			Floor->GetStaticMeshComponent()->SetMaterial(0, GroundMat);
		}
		else
		{
			// Kick off async load once, then rerun SpawnFloorIfNeeded() to apply material to tagged floors.
			if (!bGroundFloorMaterialLoadRequested)
			{
				bool bAnyToLoad = false;
				for (int32 i = 0; i < GroundFloorMaterials.Num(); ++i)
				{
					if (!GroundFloorMaterials[i].IsNull()) bAnyToLoad = true;
				}
				if (bAnyToLoad)
				{
					bGroundFloorMaterialLoadRequested = true;
					TArray<FSoftObjectPath> Paths;
					for (int32 i = 0; i < GroundFloorMaterials.Num(); ++i)
					{
						if (!GroundFloorMaterials[i].IsNull())
						{
							Paths.Add(GroundFloorMaterials[i].ToSoftObjectPath());
						}
					}
					if (Paths.Num() > 0)
					{
						TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
							Paths, FStreamableDelegate::CreateWeakLambda(this, [this]() { SpawnFloorIfNeeded(); }));
						if (Handle.IsValid())
						{
							ActiveAsyncLoadHandles.Add(Handle);
						}
					}
				}
			}

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
// Procedural platform+ramp map generation
// ============================================================================

static const FName T66MapPlatformTag(TEXT("T66_Map_Platform"));
static const FName T66MapRampTag(TEXT("T66_Map_Ramp"));
static const FName T66FloorMainTag(TEXT("T66_Floor_Main"));

static void DestroyActorsWithTag(UWorld* World, FName Tag)
{
	TArray<AActor*> ToDestroy;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->Tags.Contains(Tag)) ToDestroy.Add(*It);
	}
	for (AActor* A : ToDestroy)
	{
		A->Destroy();
	}
}

static uint64 T66MakeBoundaryKey(int32 A, int32 B)
{
	const uint32 Low = static_cast<uint32>(FMath::Min(A, B));
	const uint32 High = static_cast<uint32>(FMath::Max(A, B));
	return (static_cast<uint64>(Low) << 32) | static_cast<uint64>(High);
}

static void T66ConfigureTerrainCollisionComponent(UPrimitiveComponent* Component)
{
	if (!Component) return;

	Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Component->SetCollisionObjectType(ECC_WorldStatic);
	Component->SetCollisionResponseToAllChannels(ECR_Block);
	Component->SetGenerateOverlapEvents(false);
	Component->SetCanEverAffectNavigation(false);
	Component->SetVisibility(false, true);
	Component->SetHiddenInGame(true, true);
}

static bool T66BuildRampCollisionTransform(
	const FT66ProceduralMapResult& MapData,
	const FT66RampEdge& Edge,
	float Thickness,
	FTransform& OutTransform,
	FVector& OutBoxExtent)
{
	if (!MapData.Platforms.IsValidIndex(Edge.LowerIndex) || !MapData.Platforms.IsValidIndex(Edge.HigherIndex))
	{
		return false;
	}

	const FT66PlatformNode& Lo = MapData.Platforms[Edge.LowerIndex];
	const FT66PlatformNode& Hi = MapData.Platforms[Edge.HigherIndex];
	const float HeightDiff = Hi.TopZ - Lo.TopZ;
	if (HeightDiff <= KINDA_SMALL_NUMBER || Edge.Depth <= KINDA_SMALL_NUMBER || Edge.Width <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	FVector RampPos = FVector::ZeroVector;
	FQuat RampYaw = FQuat::Identity;
	if (Edge.bAlongX)
	{
		const bool bHigherRight = (Hi.Position.X > Lo.Position.X);
		const float BoundaryX = (Lo.Position.X + Hi.Position.X) * 0.5f;
		const float CenterX = bHigherRight ? (BoundaryX - Edge.Depth * 0.5f) : (BoundaryX + Edge.Depth * 0.5f);
		const float CenterY = (Lo.Position.Y + Hi.Position.Y) * 0.5f + Edge.PerpOffset;
		RampPos = FVector(CenterX, CenterY, Lo.TopZ + HeightDiff * 0.5f);
		RampYaw = FQuat(FRotator(0.f, bHigherRight ? 0.f : 180.f, 0.f));
	}
	else
	{
		const bool bHigherUp = (Hi.Position.Y > Lo.Position.Y);
		const float BoundaryY = (Lo.Position.Y + Hi.Position.Y) * 0.5f;
		const float CenterY = bHigherUp ? (BoundaryY - Edge.Depth * 0.5f) : (BoundaryY + Edge.Depth * 0.5f);
		const float CenterX = (Lo.Position.X + Hi.Position.X) * 0.5f + Edge.PerpOffset;
		RampPos = FVector(CenterX, CenterY, Lo.TopZ + HeightDiff * 0.5f);
		RampYaw = FQuat(FRotator(0.f, bHigherUp ? 90.f : -90.f, 0.f));
	}

	const FVector HorizontalDir = RampYaw.GetForwardVector();
	const FVector SideDir = RampYaw.GetRightVector();
	const FVector Tangent = (HorizontalDir * Edge.Depth + FVector::UpVector * HeightDiff).GetSafeNormal();
	FVector Normal = FVector::CrossProduct(Tangent, SideDir).GetSafeNormal();
	if (Normal.Z < 0.f)
	{
		Normal *= -1.f;
	}

	const float SurfaceLength = FMath::Sqrt(FMath::Square(Edge.Depth) + FMath::Square(HeightDiff));
	OutTransform = FTransform(
		FRotationMatrix::MakeFromXZ(Tangent, Normal).ToQuat(),
		RampPos - Normal * (Thickness * 0.5f));
	OutBoxExtent = FVector(
		SurfaceLength * 0.5f + 40.f,
		Edge.Width * 0.5f + 30.f,
		Thickness * 0.5f);
	return true;
}

static void AppendBoxFace(
	TArray<FVector>& V, TArray<int32>& T, TArray<FVector>& N, TArray<FVector2D>& UV,
	const FVector& V0, const FVector& V1, const FVector& V2, const FVector& V3,
	const FVector& Normal,
	int32 UVQuarterTurns)
{
	const int32 Base = V.Num();
	V.Add(V0); V.Add(V1); V.Add(V2); V.Add(V3);
	N.Add(Normal); N.Add(Normal); N.Add(Normal); N.Add(Normal);
	T66AppendRotatedQuadUVs(UV, UVQuarterTurns);
	T.Add(Base); T.Add(Base + 1); T.Add(Base + 2);
	T.Add(Base); T.Add(Base + 2); T.Add(Base + 3);
}

static void AppendWallBox(
	TArray<FVector>& SideV, TArray<int32>& SideT, TArray<FVector>& SideN, TArray<FVector2D>& SideUV,
	bool bAlongX,
	float BoundaryCoord,
	float CenterPerp,
	float LengthAlongBoundary,
	float HighTopZ,
	float LowTopZ,
	float WallThickness,
	float BottomPadding,
	bool bHighOnPositiveSide,
	int32 UVQuarterTurns)
{
	if (LengthAlongBoundary <= 1.f || HighTopZ <= LowTopZ + 1.f) return;

	const float BottomZ = LowTopZ - BottomPadding;
	const float Height = HighTopZ - BottomZ;
	if (Height <= 1.f) return;

	const float CenterZ = BottomZ + Height * 0.5f;
	FVector Center;
	FVector HE;

	if (bAlongX)
	{
		Center = FVector(
			BoundaryCoord + (bHighOnPositiveSide ? WallThickness * 0.5f : -WallThickness * 0.5f),
			CenterPerp,
			CenterZ);
		HE = FVector(WallThickness * 0.5f, LengthAlongBoundary * 0.5f, Height * 0.5f);
	}
	else
	{
		Center = FVector(
			CenterPerp,
			BoundaryCoord + (bHighOnPositiveSide ? WallThickness * 0.5f : -WallThickness * 0.5f),
			CenterZ);
		HE = FVector(LengthAlongBoundary * 0.5f, WallThickness * 0.5f, Height * 0.5f);
	}

	const FVector C = Center;
	const float hx = HE.X, hy = HE.Y, hz = HE.Z;

	// Side faces ??? brown material
	AppendBoxFace(SideV, SideT, SideN, SideUV,
		C + FVector(hx, -hy, -hz), C + FVector(hx, hy, -hz), C + FVector(hx, hy, hz), C + FVector(hx, -hy, hz),
		FVector(1, 0, 0), UVQuarterTurns);
	AppendBoxFace(SideV, SideT, SideN, SideUV,
		C + FVector(-hx, hy, -hz), C + FVector(-hx, -hy, -hz), C + FVector(-hx, -hy, hz), C + FVector(-hx, hy, hz),
		FVector(-1, 0, 0), UVQuarterTurns);
	AppendBoxFace(SideV, SideT, SideN, SideUV,
		C + FVector(hx, hy, -hz), C + FVector(-hx, hy, -hz), C + FVector(-hx, hy, hz), C + FVector(hx, hy, hz),
		FVector(0, 1, 0), UVQuarterTurns);
	AppendBoxFace(SideV, SideT, SideN, SideUV,
		C + FVector(-hx, -hy, -hz), C + FVector(hx, -hy, -hz), C + FVector(hx, -hy, hz), C + FVector(-hx, -hy, hz),
		FVector(0, -1, 0), UVQuarterTurns);
	AppendBoxFace(SideV, SideT, SideN, SideUV,
		C + FVector(-hx, hy, -hz), C + FVector(hx, hy, -hz), C + FVector(hx, -hy, -hz), C + FVector(-hx, -hy, -hz),
		FVector(0, 0, -1), UVQuarterTurns);
}

// ---- Unit wedge geometry (100x100x100, slope LOW at -X to HIGH at +X) ----

static const FVector UnitWedgeVerts[] = {
	// Bottom face (4 verts)
	FVector(-50, -50, -50), FVector(50, -50, -50), FVector(50, 50, -50), FVector(-50, 50, -50),
	// Back wall X=+50 (4 verts)
	FVector(50, -50, -50), FVector(50, -50, 50), FVector(50, 50, 50), FVector(50, 50, -50),
	// Slope (4 verts)
	FVector(-50, -50, -50), FVector(-50, 50, -50), FVector(50, 50, 50), FVector(50, -50, 50),
	// Left cap Y=-50 (3 verts)
	FVector(-50, -50, -50), FVector(50, -50, 50), FVector(50, -50, -50),
	// Right cap Y=+50 (3 verts)
	FVector(-50, 50, -50), FVector(50, 50, -50), FVector(50, 50, 50),
};

static const FVector UnitWedgeNormals[] = {
	FVector(0,0,-1), FVector(0,0,-1), FVector(0,0,-1), FVector(0,0,-1),
	FVector(1,0,0), FVector(1,0,0), FVector(1,0,0), FVector(1,0,0),
	FVector(-0.7071f,0,0.7071f), FVector(-0.7071f,0,0.7071f), FVector(-0.7071f,0,0.7071f), FVector(-0.7071f,0,0.7071f),
	FVector(0,-1,0), FVector(0,-1,0), FVector(0,-1,0),
	FVector(0,1,0), FVector(0,1,0), FVector(0,1,0),
};

static const FVector2D UnitWedgeUVs[] = {
	FVector2D(0,0), FVector2D(1,0), FVector2D(1,1), FVector2D(0,1),
	FVector2D(0,0), FVector2D(0,1), FVector2D(1,1), FVector2D(1,0),
	FVector2D(0,0), FVector2D(0,1), FVector2D(1,1), FVector2D(1,0),
	FVector2D(0,0), FVector2D(1,1), FVector2D(1,0),
	FVector2D(0,0), FVector2D(1,0), FVector2D(1,1),
};

static const int32 UnitWedgeTris[] = {
	0,2,1, 0,3,2,       // bottom
	4,5,6, 4,6,7,       // back wall
	8,10,9, 8,11,10,    // slope
	12,13,14,            // left cap
	15,16,17,            // right cap
};

static constexpr int32 WedgeVertCount = 18;
static constexpr int32 WedgeTriIdxCount = 24;

// Slope face is verts 8-11, tri indices 12-17 in the unit wedge.
static constexpr int32 WedgeSlopeVertStart = 8;
static constexpr int32 WedgeSlopeVertEnd   = 12;
static constexpr int32 WedgeSlopeTriStart  = 12;
static constexpr int32 WedgeSlopeTriEnd    = 18;

static void AppendTransformedWedge(
	TArray<FVector>& SlopeV, TArray<int32>& SlopeT, TArray<FVector>& SlopeN, TArray<FVector2D>& SlopeUV,
	TArray<FVector>& SideV, TArray<int32>& SideT, TArray<FVector>& SideN, TArray<FVector2D>& SideUV,
	const FVector& Pos, const FQuat& Rot, const FVector& Scale,
	int32 SlopeUVQuarterTurns,
	int32 SideUVQuarterTurns)
{
	auto AppendSide = [&](bool bReverse)
	{
		// Transform all verts/normals once, then distribute to slope vs side arrays.
		TArray<FVector> TransV;
		TArray<FVector> TransN;
		TransV.SetNum(WedgeVertCount);
		TransN.SetNum(WedgeVertCount);

		for (int32 I = 0; I < WedgeVertCount; ++I)
		{
			TransV[I] = Pos + Rot.RotateVector(UnitWedgeVerts[I] * Scale);
			const FVector& N = UnitWedgeNormals[I];
			FVector NScaled(
				N.X / FMath::Max(Scale.X, 0.001f),
				N.Y / FMath::Max(Scale.Y, 0.001f),
				N.Z / FMath::Max(Scale.Z, 0.001f));
			TransN[I] = Rot.RotateVector(NScaled).GetSafeNormal() * (bReverse ? -1.f : 1.f);
		}

		// Slope triangles ??? FloorMat arrays
		{
			TMap<int32, int32> Remap;
			for (int32 I = WedgeSlopeTriStart; I < WedgeSlopeTriEnd; I += 3)
			{
				int32 Idx[3] = { UnitWedgeTris[I], UnitWedgeTris[I+1], UnitWedgeTris[I+2] };
				int32 Mapped[3];
				for (int32 J = 0; J < 3; ++J)
				{
					if (int32* Found = Remap.Find(Idx[J]))
					{
						Mapped[J] = *Found;
					}
					else
					{
						Mapped[J] = SlopeV.Num();
						Remap.Add(Idx[J], Mapped[J]);
						SlopeV.Add(TransV[Idx[J]]);
						SlopeN.Add(TransN[Idx[J]]);
						SlopeUV.Add(T66RotateUnitUV(UnitWedgeUVs[Idx[J]], SlopeUVQuarterTurns));
					}
				}
				if (bReverse) { SlopeT.Add(Mapped[0]); SlopeT.Add(Mapped[2]); SlopeT.Add(Mapped[1]); }
				else          { SlopeT.Add(Mapped[0]); SlopeT.Add(Mapped[1]); SlopeT.Add(Mapped[2]); }
			}
		}

		// Non-slope triangles ??? brown side arrays
		{
			TMap<int32, int32> Remap;
			for (int32 I = 0; I < WedgeTriIdxCount; I += 3)
			{
				if (I >= WedgeSlopeTriStart && I < WedgeSlopeTriEnd) continue;

				int32 Idx[3] = { UnitWedgeTris[I], UnitWedgeTris[I+1], UnitWedgeTris[I+2] };
				int32 Mapped[3];
				for (int32 J = 0; J < 3; ++J)
				{
					if (int32* Found = Remap.Find(Idx[J]))
					{
						Mapped[J] = *Found;
					}
					else
					{
						Mapped[J] = SideV.Num();
						Remap.Add(Idx[J], Mapped[J]);
						SideV.Add(TransV[Idx[J]]);
						SideN.Add(TransN[Idx[J]]);
						SideUV.Add(T66RotateUnitUV(UnitWedgeUVs[Idx[J]], SideUVQuarterTurns));
					}
				}
				if (bReverse) { SideT.Add(Mapped[0]); SideT.Add(Mapped[2]); SideT.Add(Mapped[1]); }
				else          { SideT.Add(Mapped[0]); SideT.Add(Mapped[1]); SideT.Add(Mapped[2]); }
			}
		}
	};

	AppendSide(false);
	AppendSide(true);
}

static void AppendPlatformTopQuad(
	TArray<FVector>& OutVerts,
	TArray<int32>& OutTris,
	TArray<FVector>& OutNormals,
	TArray<FVector2D>& OutUVs,
	const FVector& Center,
	float SizeX,
	float SizeY,
	float Z,
	int32 UVQuarterTurns)
{
	const int32 Base = OutVerts.Num();
	const float HX = SizeX * 0.5f;
	const float HY = SizeY * 0.5f;

	OutVerts.Add(FVector(Center.X - HX, Center.Y - HY, Z));
	OutVerts.Add(FVector(Center.X + HX, Center.Y - HY, Z));
	OutVerts.Add(FVector(Center.X + HX, Center.Y + HY, Z));
	OutVerts.Add(FVector(Center.X - HX, Center.Y + HY, Z));

	OutNormals.Add(FVector::UpVector);
	OutNormals.Add(FVector::UpVector);
	OutNormals.Add(FVector::UpVector);
	OutNormals.Add(FVector::UpVector);

	T66AppendRotatedQuadUVs(OutUVs, UVQuarterTurns);

	OutTris.Add(Base + 0);
	OutTris.Add(Base + 2);
	OutTris.Add(Base + 1);
	OutTris.Add(Base + 0);
	OutTris.Add(Base + 3);
	OutTris.Add(Base + 2);
}

// ---- Main spawn function ----

void AT66GameMode::SpawnLowPolyNatureEnvironment()
{
	UWorld* World = GetWorld();
	if (!World) return;
	if (IsLabLevel() || IsColiseumStage()) return;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->Tags.Contains(T66MapPlatformTag)) return;
	}

	bTerrainCollisionReady = false;

	UT66GameInstance* GI = GetT66GameInstance();
	const ET66MapTheme Theme = GI ? GI->MapTheme : ET66MapTheme::Farm;
	int32 Seed = GI ? GI->RunSeed : 0;
	if (Seed == 0) Seed = FMath::Rand();

	FT66MapPreset Preset = FT66MapPreset::GetDefaultForTheme(Theme);
	Preset.Seed = Seed;

	FT66ProceduralMapResult MapData = FT66ProceduralMapGenerator::Generate(Preset);
	if (!MapData.bValid) return;

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	TArray<UMaterialInterface*> LoadedGroundMaterials;
	LoadedGroundMaterials.Reserve(GroundFloorMaterials.Num());
	for (int32 i = 0; i < GroundFloorMaterials.Num(); ++i)
	{
		if (!GroundFloorMaterials[i].IsNull() && !GroundFloorMaterials[i].Get())
		{
			GroundFloorMaterials[i].LoadSynchronous();
		}
		if (UMaterialInterface* Mat = GroundFloorMaterials[i].Get())
		{
			LoadedGroundMaterials.Add(Mat);
		}
	}

	TArray<UMaterialInterface*> LoadedCliffMaterials;
	LoadedCliffMaterials.Reserve(CliffSideMaterials.Num());
	for (int32 i = 0; i < CliffSideMaterials.Num(); ++i)
	{
		if (!CliffSideMaterials[i].IsNull() && !CliffSideMaterials[i].Get())
		{
			CliffSideMaterials[i].LoadSynchronous();
		}
		if (UMaterialInterface* Mat = CliffSideMaterials[i].Get())
		{
			LoadedCliffMaterials.Add(Mat);
		}
	}

	UMaterialInterface* EnvUnlitBase = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
	auto LoadTerrainMaterialsFromSource = [&](const TCHAR* Prefix, std::initializer_list<int32> Indices, TArray<UMaterialInterface*>& OutMaterials)
	{
		if (!EnvUnlitBase)
		{
			return;
		}

		const FString SourceDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("SourceAssets/Import"));
		for (int32 Index : Indices)
		{
			const FString Filename = SourceDir / FString::Printf(TEXT("%s%d.png"), Prefix, Index);
			if (!FPaths::FileExists(Filename))
			{
				continue;
			}

			UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(Filename);
			if (!Texture)
			{
				continue;
			}

			Texture->SRGB = true;
			Texture->LODGroup = TEXTUREGROUP_World;
			Texture->UpdateResource();

			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(EnvUnlitBase, this);
			if (!MID)
			{
				continue;
			}

			MID->SetTextureParameterValue(TEXT("DiffuseColorMap"), Texture);
			MID->SetVectorParameterValue(FName("Tint"), FLinearColor::White);
			MID->SetScalarParameterValue(FName("Brightness"), 1.f);
			OutMaterials.Add(MID);
		}
	};
	auto LoadTerrainMaterialAssets = [&](const TCHAR* AssetDir, const TCHAR* AssetPrefix, std::initializer_list<int32> Indices, TArray<UMaterialInterface*>& OutMaterials)
	{
		for (int32 Index : Indices)
		{
			const FString AssetPath = FString::Printf(TEXT("%s/%s%d.%s%d"), AssetDir, AssetPrefix, Index, AssetPrefix, Index);
			if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *AssetPath))
			{
				OutMaterials.Add(Material);
			}
		}
	};

	TArray<UMaterialInterface*> TerrainGroundMaterials;
	LoadTerrainMaterialsFromSource(TEXT("GroundTile"), { 1, 2, 3, 4, 5 }, TerrainGroundMaterials);
	if (TerrainGroundMaterials.Num() == 0)
	{
		LoadTerrainMaterialAssets(TEXT("/Game/World/Ground"), TEXT("MI_GroundTile"), { 1, 2, 3, 4, 5 }, TerrainGroundMaterials);
	}
	if (TerrainGroundMaterials.Num() == 0)
	{
		LoadTerrainMaterialsFromSource(TEXT("Grass"), { 1, 2, 3, 4 }, TerrainGroundMaterials);
	}
	if (TerrainGroundMaterials.Num() == 0)
	{
		LoadTerrainMaterialAssets(TEXT("/Game/World/Ground"), TEXT("MI_Grass"), { 1, 2, 3, 4 }, TerrainGroundMaterials);
	}
	if (TerrainGroundMaterials.Num() == 0)
	{
		TerrainGroundMaterials = LoadedGroundMaterials;
	}

	TArray<UMaterialInterface*> TerrainCliffMaterials;
	LoadTerrainMaterialsFromSource(TEXT("HillTile"), { 1, 2, 3, 4 }, TerrainCliffMaterials);
	if (TerrainCliffMaterials.Num() == 0)
	{
		LoadTerrainMaterialAssets(TEXT("/Game/World/Cliffs"), TEXT("MI_HillTile"), { 1, 2, 3, 4 }, TerrainCliffMaterials);
	}
	if (TerrainCliffMaterials.Num() == 0)
	{
		LoadTerrainMaterialsFromSource(TEXT("Cliff"), { 2, 3 }, TerrainCliffMaterials);
	}
	if (TerrainCliffMaterials.Num() == 0)
	{
		LoadTerrainMaterialAssets(TEXT("/Game/World/Cliffs"), TEXT("MI_Cliff"), { 2, 3 }, TerrainCliffMaterials);
	}
	if (TerrainCliffMaterials.Num() == 0)
	{
		TerrainCliffMaterials = LoadedCliffMaterials;
	}

	FActorSpawnParameters SP;
	SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const float MapSize  = Preset.MapHalfExtent * 2.f;
	const float CellSize = MapSize / FMath::Max(Preset.GridSize, 2);
	const int32 GroundVariantCount = FMath::Max(TerrainGroundMaterials.Num(), 1);
	const int32 CliffVariantCount = FMath::Max(TerrainCliffMaterials.Num(), 1);

	// Build a lookup so walls can leave openings where a ramp exists.
	TMap<uint64, const FT66RampEdge*> RampLookup;
	for (const FT66RampEdge& Edge : MapData.Ramps)
	{
		RampLookup.Add(T66MakeBoundaryKey(Edge.LowerIndex, Edge.HigherIndex), &Edge);
	}

	// --- Top surfaces use procedural quads bucketed by seeded grass material ---
	AActor* TopActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SP);
	if (!TopActor) return;
	TopActor->Tags.Add(T66MapPlatformTag);
	TopActor->Tags.Add(T66FloorMainTag);

	UProceduralMeshComponent* TopPMC = NewObject<UProceduralMeshComponent>(TopActor, TEXT("PlatformTops"));
	TopActor->SetRootComponent(TopPMC);

	TArray<FT66MeshSectionBuffers> TopSections;
	TopSections.SetNum(GroundVariantCount);
	for (const FT66PlatformNode& P : MapData.Platforms)
	{
		const int32 Bucket = T66PickCellVariant(Preset.Seed, P.GridRow, P.GridCol, GroundVariantCount);
		const int32 QuarterTurns = T66PickCellQuarterTurns(Preset.Seed ^ 0x47524E44, P.GridRow, P.GridCol);
		FT66MeshSectionBuffers& Section = TopSections[Bucket];
		AppendPlatformTopQuad(Section.Verts, Section.Tris, Section.Normals, Section.UVs, FVector(P.Position, 0.f), P.SizeX, P.SizeY, P.TopZ, QuarterTurns);
	}

	TArray<FLinearColor> EmptyColors;
	auto CreateMeshSection = [&](UProceduralMeshComponent* PMC, int32 SectionIndex, const FT66MeshSectionBuffers& Buffers, UMaterialInterface* Material)
	{
		TArray<FVector> Normals = Buffers.Normals;
		TArray<FProcMeshTangent> Tangents;
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Buffers.Verts, Buffers.Tris, Buffers.UVs, Normals, Tangents);
		PMC->CreateMeshSection_LinearColor(SectionIndex, Buffers.Verts, Buffers.Tris, Normals, Buffers.UVs, EmptyColors, Tangents, true);
		if (Material)
		{
			PMC->SetMaterial(SectionIndex, Material);
		}
	};

	int32 TopSectionIndex = 0;
	for (int32 Bucket = 0; Bucket < TopSections.Num(); ++Bucket)
	{
		if (!TopSections[Bucket].HasGeometry())
		{
			continue;
		}

		UMaterialInterface* Material = TerrainGroundMaterials.IsValidIndex(Bucket) ? TerrainGroundMaterials[Bucket] : nullptr;
		CreateMeshSection(TopPMC, TopSectionIndex++, TopSections[Bucket], Material);
	}
	TopPMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TopPMC->RegisterComponent();

	// --- Wall sides as procedural mesh (same approach as tops/ramps ??? no engine default material) ---
	const float WallThickness = FMath::Clamp(Preset.SurfaceThickness, 60.f, 140.f);
	const int32 GridSize = FMath::Max(Preset.GridSize, 2);

	// Brown side material: Unlit solid color for wall sides and ramp undersides.
	UMaterialInstanceDynamic* BrownSideMat = nullptr;
	{
		UTexture* WhiteTexture = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
		if (!WhiteTexture)
		{
			WhiteTexture = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));
		}
		if (EnvUnlitBase)
		{
			BrownSideMat = UMaterialInstanceDynamic::Create(EnvUnlitBase, GetTransientPackage());
			if (BrownSideMat && WhiteTexture)
			{
				BrownSideMat->SetTextureParameterValue(TEXT("DiffuseColorMap"), WhiteTexture);
			}
			if (BrownSideMat)
			{
				BrownSideMat->SetVectorParameterValue(FName("Tint"), FLinearColor(0.35f, 0.2f, 0.08f, 1.f));
			}
		}
	}

	TArray<FT66MeshSectionBuffers> WallSideSections;
	WallSideSections.SetNum(CliffVariantCount);

	for (int32 R = 0; R < GridSize; ++R)
	{
		for (int32 C = 0; C < GridSize; ++C)
		{
			const int32 Idx = R * GridSize + C;
			const FT66PlatformNode& A = MapData.Platforms[Idx];

			auto BuildBoundaryWall = [&](int32 OtherR, int32 OtherC, bool bAlongX)
			{
				if (OtherR < 0 || OtherR >= GridSize || OtherC < 0 || OtherC >= GridSize)
				{
					return;
				}

				const int32 OtherIdx = OtherR * GridSize + OtherC;
				const FT66PlatformNode& B = MapData.Platforms[OtherIdx];
				if (A.TopZ <= B.TopZ + 1.f)
				{
					return;
				}

				const bool bHighOnPositiveSide = bAlongX ? (A.Position.X > B.Position.X) : (A.Position.Y > B.Position.Y);
				const float BoundaryCoord = bAlongX
					? 0.5f * (A.Position.X + B.Position.X)
					: 0.5f * (A.Position.Y + B.Position.Y);
				const float PerpBase = bAlongX
					? 0.5f * (A.Position.Y + B.Position.Y)
					: 0.5f * (A.Position.X + B.Position.X);
				const int32 CliffBucket = T66PickBoundaryVariant(Preset.Seed, Idx, OtherIdx, CliffVariantCount);
				const int32 CliffQuarterTurns = T66PickBoundaryQuarterTurns(Preset.Seed ^ 0x48494C4C, Idx, OtherIdx);
				FT66MeshSectionBuffers& WallSideSection = WallSideSections[CliffBucket];

				const FT66RampEdge* Ramp = RampLookup.FindRef(T66MakeBoundaryKey(Idx, OtherIdx));
				if (!Ramp)
				{
					AppendWallBox(
						WallSideSection.Verts, WallSideSection.Tris, WallSideSection.Normals, WallSideSection.UVs,
						bAlongX,
						BoundaryCoord,
						PerpBase,
						CellSize,
						A.TopZ,
						B.TopZ,
						WallThickness,
						Preset.WallBottomPadding,
						bHighOnPositiveSide,
						CliffQuarterTurns);
					return;
				}

				const float HalfCell = CellSize * 0.5f;
				const float OpenStart = FMath::Clamp(Ramp->PerpOffset - Ramp->Width * 0.5f, -HalfCell, HalfCell);
				const float OpenEnd = FMath::Clamp(Ramp->PerpOffset + Ramp->Width * 0.5f, -HalfCell, HalfCell);

				const float LeftLen = OpenStart + HalfCell;
				if (LeftLen > 1.f)
				{
					AppendWallBox(
						WallSideSection.Verts, WallSideSection.Tris, WallSideSection.Normals, WallSideSection.UVs,
						bAlongX,
						BoundaryCoord,
						PerpBase + (-HalfCell + OpenStart) * 0.5f,
						LeftLen,
						A.TopZ,
						B.TopZ,
						WallThickness,
						Preset.WallBottomPadding,
						bHighOnPositiveSide,
						CliffQuarterTurns);
				}

				const float RightLen = HalfCell - OpenEnd;
				if (RightLen > 1.f)
				{
					AppendWallBox(
						WallSideSection.Verts, WallSideSection.Tris, WallSideSection.Normals, WallSideSection.UVs,
						bAlongX,
						BoundaryCoord,
						PerpBase + (OpenEnd + HalfCell) * 0.5f,
						RightLen,
						A.TopZ,
						B.TopZ,
						WallThickness,
						Preset.WallBottomPadding,
						bHighOnPositiveSide,
						CliffQuarterTurns);
				}
			};

			if (C + 1 < GridSize) BuildBoundaryWall(R, C + 1, true);
			if (C - 1 >= 0)      BuildBoundaryWall(R, C - 1, true);
			if (R + 1 < GridSize) BuildBoundaryWall(R + 1, C, false);
			if (R - 1 >= 0)      BuildBoundaryWall(R - 1, C, false);
		}
	}

	bool bHasWallGeometry = false;
	for (const FT66MeshSectionBuffers& Section : WallSideSections)
	{
		bHasWallGeometry |= Section.HasGeometry();
	}

	if (bHasWallGeometry)
	{
		AActor* WallActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SP);
		if (WallActor)
		{
			WallActor->Tags.Add(T66MapPlatformTag);
			WallActor->Tags.Add(T66FloorMainTag);

			UProceduralMeshComponent* WallPMC = NewObject<UProceduralMeshComponent>(WallActor, TEXT("PlatformWalls"));
			WallActor->SetRootComponent(WallPMC);

			int32 SectionIndex = 0;
			for (int32 Bucket = 0; Bucket < WallSideSections.Num(); ++Bucket)
			{
				if (!WallSideSections[Bucket].HasGeometry())
				{
					continue;
				}

				UMaterialInterface* Material = TerrainCliffMaterials.IsValidIndex(Bucket) ? TerrainCliffMaterials[Bucket] : BrownSideMat;
				CreateMeshSection(WallPMC, SectionIndex++, WallSideSections[Bucket], Material);
			}

			WallPMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			WallPMC->RegisterComponent();
		}
	}

	// --- Ramps: slope surface uses grass buckets; side faces use cliff buckets ---
	const float MinRampHeight = 50.f;

	TArray<FT66MeshSectionBuffers> RampSlopeSections;
	TArray<FT66MeshSectionBuffers> RampSideSections;
	RampSlopeSections.SetNum(GroundVariantCount);
	RampSideSections.SetNum(CliffVariantCount);

	int32 RampCount = 0;
	for (const FT66RampEdge& Edge : MapData.Ramps)
	{
		const FT66PlatformNode& Lo = MapData.Platforms[Edge.LowerIndex];
		const FT66PlatformNode& Hi = MapData.Platforms[Edge.HigherIndex];
		const float HeightDiff = Hi.TopZ - Lo.TopZ;
		if (HeightDiff < MinRampHeight) continue;

		FVector RampPos;
		FQuat RampRot = FQuat::Identity;

		if (Edge.bAlongX)
		{
			const bool bHigherRight = (Hi.Position.X > Lo.Position.X);
			const float BoundaryX = (Lo.Position.X + Hi.Position.X) * 0.5f;
			const float CenterX = bHigherRight ? (BoundaryX - Edge.Depth * 0.5f) : (BoundaryX + Edge.Depth * 0.5f);
			const float CenterY = (Lo.Position.Y + Hi.Position.Y) * 0.5f + Edge.PerpOffset;
			RampPos = FVector(CenterX, CenterY, Lo.TopZ + HeightDiff * 0.5f);
			RampRot = FQuat(FRotator(0.f, bHigherRight ? 0.f : 180.f, 0.f));
		}
		else
		{
			const bool bHigherUp = (Hi.Position.Y > Lo.Position.Y);
			const float BoundaryY = (Lo.Position.Y + Hi.Position.Y) * 0.5f;
			const float CenterY = bHigherUp ? (BoundaryY - Edge.Depth * 0.5f) : (BoundaryY + Edge.Depth * 0.5f);
			const float CenterX = (Lo.Position.X + Hi.Position.X) * 0.5f + Edge.PerpOffset;
			RampPos = FVector(CenterX, CenterY, Lo.TopZ + HeightDiff * 0.5f);
			RampRot = FQuat(FRotator(0.f, bHigherUp ? 90.f : -90.f, 0.f));
		}

		const FVector RampScale(Edge.Depth / 100.f, Edge.Width / 100.f, HeightDiff / 100.f);
		const int32 GroundBucket = T66PickBoundaryVariant(Preset.Seed, Edge.LowerIndex, Edge.HigherIndex, GroundVariantCount);
		const int32 CliffBucket = T66PickBoundaryVariant(Preset.Seed ^ 0x4C494646, Edge.LowerIndex, Edge.HigherIndex, CliffVariantCount);
		const int32 GroundQuarterTurns = T66PickBoundaryQuarterTurns(Preset.Seed ^ 0x534C4F50, Edge.LowerIndex, Edge.HigherIndex);
		const int32 CliffQuarterTurns = T66PickBoundaryQuarterTurns(Preset.Seed ^ 0x53494445, Edge.LowerIndex, Edge.HigherIndex);
		AppendTransformedWedge(
			RampSlopeSections[GroundBucket].Verts, RampSlopeSections[GroundBucket].Tris, RampSlopeSections[GroundBucket].Normals, RampSlopeSections[GroundBucket].UVs,
			RampSideSections[CliffBucket].Verts, RampSideSections[CliffBucket].Tris, RampSideSections[CliffBucket].Normals, RampSideSections[CliffBucket].UVs,
			RampPos, RampRot, RampScale, GroundQuarterTurns, CliffQuarterTurns);
		++RampCount;
	}

	if (RampCount > 0)
	{
		AActor* RampActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SP);
		if (RampActor)
		{
			RampActor->Tags.Add(T66MapRampTag);
			RampActor->Tags.Add(T66FloorMainTag);

			UProceduralMeshComponent* PMC = NewObject<UProceduralMeshComponent>(RampActor, TEXT("Ramps"));
			RampActor->SetRootComponent(PMC);

			int32 SectionIndex = 0;
			for (int32 Bucket = 0; Bucket < RampSlopeSections.Num(); ++Bucket)
			{
				if (!RampSlopeSections[Bucket].HasGeometry())
				{
					continue;
				}

				UMaterialInterface* Material = TerrainGroundMaterials.IsValidIndex(Bucket) ? TerrainGroundMaterials[Bucket] : nullptr;
				CreateMeshSection(PMC, SectionIndex++, RampSlopeSections[Bucket], Material);
			}
			for (int32 Bucket = 0; Bucket < RampSideSections.Num(); ++Bucket)
			{
				if (!RampSideSections[Bucket].HasGeometry())
				{
					continue;
				}

				UMaterialInterface* Material = TerrainCliffMaterials.IsValidIndex(Bucket) ? TerrainCliffMaterials[Bucket] : BrownSideMat;
				CreateMeshSection(PMC, SectionIndex++, RampSideSections[Bucket], Material);
			}

			PMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			PMC->RegisterComponent();
		}
	}

	bool bBuiltCollisionLayer = false;
	AActor* CollisionActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SP);
	if (CollisionActor)
	{
		bBuiltCollisionLayer = true;
		CollisionActor->Tags.Add(T66FloorMainTag);
		CollisionActor->SetActorHiddenInGame(true);

		USceneComponent* CollisionRoot = NewObject<USceneComponent>(CollisionActor, TEXT("TerrainCollisionRoot"));
		CollisionActor->AddInstanceComponent(CollisionRoot);
		CollisionActor->SetRootComponent(CollisionRoot);
		CollisionRoot->RegisterComponent();

		const float PlatformCollisionThickness = FMath::Max(Preset.SurfaceThickness + 240.f, 600.f);
		for (int32 Index = 0; Index < MapData.Platforms.Num(); ++Index)
		{
			const FT66PlatformNode& P = MapData.Platforms[Index];
			UBoxComponent* Box = NewObject<UBoxComponent>(CollisionActor, *FString::Printf(TEXT("PlatformCollision_%d"), Index));
			CollisionActor->AddInstanceComponent(Box);
			Box->SetupAttachment(CollisionRoot);
			Box->SetRelativeLocation(FVector(P.Position, P.TopZ - PlatformCollisionThickness * 0.5f));
			Box->SetBoxExtent(FVector(P.SizeX * 0.5f, P.SizeY * 0.5f, PlatformCollisionThickness * 0.5f));
			T66ConfigureTerrainCollisionComponent(Box);
			Box->RegisterComponent();
		}

		const float RampCollisionThickness = FMath::Clamp(Preset.SurfaceThickness, 80.f, 140.f);
		for (int32 Index = 0; Index < MapData.Ramps.Num(); ++Index)
		{
			FTransform RampTransform;
			FVector RampExtent = FVector::ZeroVector;
			if (!T66BuildRampCollisionTransform(MapData, MapData.Ramps[Index], RampCollisionThickness, RampTransform, RampExtent))
			{
				continue;
			}

			UBoxComponent* Box = NewObject<UBoxComponent>(CollisionActor, *FString::Printf(TEXT("RampCollision_%d"), Index));
			CollisionActor->AddInstanceComponent(Box);
			Box->SetupAttachment(CollisionRoot);
			Box->SetRelativeTransform(RampTransform);
			Box->SetBoxExtent(RampExtent);
			T66ConfigureTerrainCollisionComponent(Box);
			Box->RegisterComponent();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MAP] Failed to spawn terrain collision actor; floor collision will be missing."));
	}

	bTerrainCollisionReady = bBuiltCollisionLayer;
	if (bTerrainCollisionReady)
	{
		SnapPlayersToTerrain();
	}

	UE_LOG(LogTemp, Log, TEXT("[MAP] Procedural map spawned: %d platforms, %d ramps (theme=%d, seed=%d, cell=%.0f)"),
		MapData.Platforms.Num(), RampCount, static_cast<int32>(Theme), Seed, CellSize);
}

// ============================================================================
// HDRI Equirectangular ??? TextureCube conversion (editor-only, runs once)
// ============================================================================
#if WITH_EDITORONLY_DATA

static const TCHAR* HDRIEquirectPath = TEXT("/Game/Lighting/T_HDRI_Studio.T_HDRI_Studio");
static const TCHAR* HDRICubePackagePath = TEXT("/Game/Lighting/TC_HDRI_Studio");
static const TCHAR* HDRICubeAssetPath = TEXT("/Game/Lighting/TC_HDRI_Studio.TC_HDRI_Studio");

/** Map cube face + UV to a 3D direction vector (UE coordinate system: X=fwd, Y=right, Z=up). */
static FVector CubeFaceDirection(int32 Face, float U, float V)
{
	const float S = 2.f * U - 1.f;
	const float T = 2.f * V - 1.f;
	switch (Face)
	{
	case 0: return FVector( 1.f,  S,   -T);    // +X
	case 1: return FVector(-1.f, -S,   -T);    // -X
	case 2: return FVector(-S,    1.f, -T);     // +Y
	case 3: return FVector( S,   -1.f, -T);     // -Y
	case 4: return FVector(-S,   -T,    1.f);   // +Z
	case 5: return FVector(-S,    T,   -1.f);   // -Z
	default: return FVector::ForwardVector;
	}
}

/** Convert a 3D direction to equirectangular UV coordinates [0,1]. */
static FVector2D DirToEquirectUV(const FVector& Dir)
{
	const FVector N = Dir.GetSafeNormal();
	const float Phi   = FMath::Atan2(N.Y, N.X);                            // azimuth [-PI, PI]
	const float Theta = FMath::Asin(FMath::Clamp(N.Z, -1.f, 1.f));        // elevation [-PI/2, PI/2]
	return FVector2D(
		FMath::Frac(Phi / (2.f * UE_PI) + 0.5f),
		FMath::Clamp(0.5f - Theta / UE_PI, 0.f, 1.f)
	);
}

/**
 * If the HDRI Texture2D exists but no TextureCube has been created yet,
 * sample the equirectangular map into 6 cube faces and save as a persistent asset.
 */
static UTextureCube* EnsureHDRICubemap()
{
	// Already exists?
	UTextureCube* Existing = LoadObject<UTextureCube>(nullptr, HDRICubeAssetPath);
	if (Existing) return Existing;

	// Source equirectangular texture?
	UTexture2D* Src = LoadObject<UTexture2D>(nullptr, HDRIEquirectPath);
	if (!Src)
	{
		UE_LOG(LogTemp, Log, TEXT("[HDRI] No equirectangular source at %s ??? skipping cubemap creation"), HDRIEquirectPath);
		return nullptr;
	}

	FTextureSource& SrcSource = Src->Source;
	if (!SrcSource.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[HDRI] Source texture has no valid FTextureSource data"));
		return nullptr;
	}

	const int32 SrcW = SrcSource.GetSizeX();
	const int32 SrcH = SrcSource.GetSizeY();
	const ETextureSourceFormat SrcFmt = SrcSource.GetFormat();
	const int32 SrcBpp = SrcSource.GetBytesPerPixel();

	TArray64<uint8> SrcData;
	SrcSource.GetMipData(SrcData, 0);
	if (SrcData.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HDRI] Could not read source mip data"));
		return nullptr;
	}

	// Face size: half the equirect height, clamped for sanity.
	const int32 FaceSize = FMath::Clamp(SrcH / 2, 64, 1024);
	const int32 FacePixels = FaceSize * FaceSize;
	const int32 OutBpp = 8; // RGBA16F = 4 channels ?? 2 bytes (FFloat16)

	UE_LOG(LogTemp, Log, TEXT("[HDRI] Creating cubemap from %dx%d equirect (format %d) ??? %dx%d faces"), SrcW, SrcH, (int32)SrcFmt, FaceSize, FaceSize);

	// Create the package and TextureCube
	UPackage* Package = CreatePackage(HDRICubePackagePath);
	UTextureCube* Cube = NewObject<UTextureCube>(Package, TEXT("TC_HDRI_Studio"), RF_Public | RF_Standalone);
	Cube->Source.Init(FaceSize, FaceSize, 6, 1, TSF_RGBA16F);
	Cube->SRGB = false;
	Cube->CompressionSettings = TC_HDR;
	Cube->LODGroup = TEXTUREGROUP_Skybox;

	uint8* RawOut = Cube->Source.LockMip(0);

	for (int32 Face = 0; Face < 6; ++Face)
	{
		for (int32 Y = 0; Y < FaceSize; ++Y)
		{
			for (int32 X = 0; X < FaceSize; ++X)
			{
				const float U = (X + 0.5f) / (float)FaceSize;
				const float V = (Y + 0.5f) / (float)FaceSize;

				const FVector Dir = CubeFaceDirection(Face, U, V);
				const FVector2D EqUV = DirToEquirectUV(Dir);

				// Nearest-neighbour sample from source (good enough for SkyLight ambient)
				const int32 SrcX = FMath::Clamp((int32)(EqUV.X * SrcW), 0, SrcW - 1);
				const int32 SrcY = FMath::Clamp((int32)(EqUV.Y * SrcH), 0, SrcH - 1);
				const int64 SrcOff = ((int64)SrcY * SrcW + SrcX) * SrcBpp;

				// Read source pixel as linear color
				FLinearColor Color = FLinearColor::Black;
				if (SrcFmt == TSF_RGBA16F && SrcOff + 8 <= SrcData.Num())
				{
					const FFloat16* H = reinterpret_cast<const FFloat16*>(&SrcData[SrcOff]);
					Color = FLinearColor(H[0].GetFloat(), H[1].GetFloat(), H[2].GetFloat(), 1.f);
				}
				else if (SrcFmt == TSF_BGRA8 && SrcOff + 4 <= SrcData.Num())
				{
					Color.B = SrcData[SrcOff + 0] / 255.f;
					Color.G = SrcData[SrcOff + 1] / 255.f;
					Color.R = SrcData[SrcOff + 2] / 255.f;
					Color.A = 1.f;
				}
				else if (SrcFmt == TSF_RGBA16 && SrcOff + 8 <= SrcData.Num())
				{
					const uint16* U16 = reinterpret_cast<const uint16*>(&SrcData[SrcOff]);
					Color.R = U16[0] / 65535.f;
					Color.G = U16[1] / 65535.f;
					Color.B = U16[2] / 65535.f;
					Color.A = 1.f;
				}

				// Write as RGBA16F
				const int64 DstOff = ((int64)Face * FacePixels + (int64)Y * FaceSize + X) * OutBpp;
				FFloat16* Out = reinterpret_cast<FFloat16*>(RawOut + DstOff);
				Out[0].Set(Color.R);
				Out[1].Set(Color.G);
				Out[2].Set(Color.B);
				Out[3].Set(1.f);
			}
		}
	}

	Cube->Source.UnlockMip(0);
	Cube->UpdateResource();
	Cube->PostEditChange();
	Package->MarkPackageDirty();

	// Persist to disk so it's available on next editor launch without re-generation.
	const FString Filename = FPackageName::LongPackageNameToFilename(HDRICubePackagePath, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	UPackage::Save(Package, Cube, *Filename, SaveArgs);

	UE_LOG(LogTemp, Log, TEXT("[HDRI] Saved cubemap: %s (%dx%d per face, RGBA16F)"), HDRICubeAssetPath, FaceSize, FaceSize);
	return Cube;
}

#endif // WITH_EDITORONLY_DATA

void AT66GameMode::SpawnLightingIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	static const FName MoonTag(TEXT("T66Moon"));

	ASkyAtmosphere* Atmosphere = nullptr;
	for (TActorIterator<ASkyAtmosphere> It(World); It; ++It)
	{
		Atmosphere = *It;
		break;
	}

	ADirectionalLight* SunForAtmosphere = nullptr;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		SunForAtmosphere = *It;
		break;
	}

	ASkyLight* SkyForCapture = nullptr;
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		SkyForCapture = *It;
		break;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Sky atmosphere (blue mid-day sky) if needed.
	if (!Atmosphere)
	{
		UE_LOG(LogTemp, Log, TEXT("No SkyAtmosphere found - spawning development sky atmosphere"));
		ASkyAtmosphere* SpawnedAtmosphere = World->SpawnActor<ASkyAtmosphere>(
			ASkyAtmosphere::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (SpawnedAtmosphere)
		{
			#if WITH_EDITOR
			SpawnedAtmosphere->SetActorLabel(TEXT("DEV_SkyAtmosphere"));
			#endif
			SpawnedSetupActors.Add(SpawnedAtmosphere);
			Atmosphere = SpawnedAtmosphere;
		}
	}

	// Spawn directional light (sun) if needed
	if (!SunForAtmosphere)
	{
		UE_LOG(LogTemp, Log, TEXT("No directional light found - spawning development sun"));

		ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(
			ADirectionalLight::StaticClass(),
			FVector(0.f, 0.f, 1000.f),
			FRotator(-50.f, -45.f, 0.f), // Angled down
			SpawnParams
		);

		if (Sun)
		{
			if (UDirectionalLightComponent* LightComp = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
			{
				LightComp->SetMobility(EComponentMobility::Movable); // Dynamic lighting so landscape stays lit without Build Lighting
				LightComp->SetIntensity(3.f);  // Fill light ??? SkyLight is primary ambient
				LightComp->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f)); // Warm sunlight
				LightComp->CastShadows = false; // Shadows disabled ??? prevents dark bands on characters

				// Drive SkyAtmosphere sun/sky scattering (mid-day blue sky).
				LightComp->bAtmosphereSunLight = true;
				LightComp->AtmosphereSunLightIndex = 0;
				LightComp->SetForwardShadingPriority(1); // Primary for forward shading / translucency / water / volumetric fog
			}
			#if WITH_EDITOR
			Sun->SetActorLabel(TEXT("DEV_Sun"));
			#endif
			SpawnedSetupActors.Add(Sun);
			SunForAtmosphere = Sun;
			UE_LOG(LogTemp, Log, TEXT("Spawned development directional light"));
		}
	}

	// Ensure we have two directional lights (sun + moon) for theme-driven day/night.
	int32 DirLightCount = 0;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It) { ++DirLightCount; }
	if (DirLightCount < 2)
	{
		ADirectionalLight* Moon = World->SpawnActor<ADirectionalLight>(
			ADirectionalLight::StaticClass(),
			FVector(0.f, 0.f, 1000.f),
			FRotator(50.f, 135.f, 0.f), // ~180?? from sun so moon is "up" at night
			SpawnParams
		);
		if (Moon)
		{
			Moon->Tags.Add(MoonTag);
			if (UDirectionalLightComponent* LC = Cast<UDirectionalLightComponent>(Moon->GetLightComponent()))
			{
				LC->SetMobility(EComponentMobility::Movable);
				LC->SetIntensity(0.f); // ApplyThemeToDirectionalLights() will set when Dark
				LC->SetLightColor(FLinearColor(0.72f, 0.8f, 1.f)); // Cool moonlight
				LC->CastShadows = false; // Shadows disabled globally
				LC->bAtmosphereSunLight = true;
				LC->AtmosphereSunLightIndex = 1;
				LC->SetForwardShadingPriority(0); // Secondary; sun is primary for forward shading
			}
#if WITH_EDITOR
			Moon->SetActorLabel(TEXT("DEV_Moon"));
#endif
			SpawnedSetupActors.Add(Moon);
			UE_LOG(LogTemp, Log, TEXT("Spawned development moon light for dark-mode sky"));
		}
	}

	// Spawn sky light (ambient) if needed
	if (!SkyForCapture)
	{
		UE_LOG(LogTemp, Log, TEXT("No sky light found - spawning development ambient light"));

		ASkyLight* Sky = World->SpawnActor<ASkyLight>(
			ASkyLight::StaticClass(),
			FVector(0.f, 0.f, 500.f),
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Sky)
		{
			#if WITH_EDITOR
			Sky->SetActorLabel(TEXT("DEV_SkyLight"));
			#endif
			SpawnedSetupActors.Add(Sky);
			SkyForCapture = Sky;
			UE_LOG(LogTemp, Log, TEXT("Spawned development sky light"));
		}
	}

	// Exponential Height Fog for atmospheric depth (distant terrain lighter/softer, less flat/gamey).
	AExponentialHeightFog* HeightFog = nullptr;
	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		HeightFog = *It;
		break;
	}
	if (!HeightFog)
	{
		UE_LOG(LogTemp, Log, TEXT("No Exponential Height Fog found - spawning for atmospheric depth"));
		AExponentialHeightFog* SpawnedFog = World->SpawnActor<AExponentialHeightFog>(
			AExponentialHeightFog::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);
		if (SpawnedFog)
		{
			UExponentialHeightFogComponent* FogComp = SpawnedFog->FindComponentByClass<UExponentialHeightFogComponent>();
			if (!FogComp) FogComp = Cast<UExponentialHeightFogComponent>(SpawnedFog->GetRootComponent());
			if (FogComp)
			{
				// Larger clear radius (5000 uu), then thicker fog beyond.
				FogComp->SetStartDistance(10000.f);
				FogComp->SetFogDensity(0.4f);
				FogComp->SetFogHeightFalloff(0.2f);
				FogComp->SetFogMaxOpacity(0.98f);
				FogComp->SetFogInscatteringColor(FLinearColor(0.6f, 0.65f, 0.78f));
			}
#if WITH_EDITOR
			SpawnedFog->SetActorLabel(TEXT("DEV_ExponentialHeightFog"));
#endif
			SpawnedSetupActors.Add(SpawnedFog);
			HeightFog = SpawnedFog;
		}
	}
	else if (HeightFog)
	{
		// Align level-placed fog with frontend/gameplay defaults.
		UExponentialHeightFogComponent* FogComp = HeightFog->FindComponentByClass<UExponentialHeightFogComponent>();
		if (!FogComp) FogComp = Cast<UExponentialHeightFogComponent>(HeightFog->GetRootComponent());
		if (FogComp)
		{
			FogComp->SetStartDistance(10000.f);
			FogComp->SetFogDensity(0.4f);
			FogComp->SetFogHeightFalloff(0.2f);
			FogComp->SetFogMaxOpacity(0.98f);
			FogComp->SetFogInscatteringColor(FLinearColor(0.6f, 0.65f, 0.78f));
		}
	}

	// Apply player setting: fog off by default (Settings ??? Graphics ??? Fog).
	if (HeightFog)
	{
		UExponentialHeightFogComponent* FogComp = HeightFog->FindComponentByClass<UExponentialHeightFogComponent>();
		if (!FogComp) FogComp = Cast<UExponentialHeightFogComponent>(HeightFog->GetRootComponent());
		if (FogComp)
		{
			UGameInstance* GI = World->GetGameInstance();
			UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
			FogComp->SetVisibility(PS && PS->GetFogEnabled());
		}
	}

	// PostProcessVolume (unbound) for exposure and color grading so distance reads less punchy.
	APostProcessVolume* PPVolume = nullptr;
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		APostProcessVolume* P = *It;
		if (P && P->bUnbound)
		{
			PPVolume = P;
			break;
		}
	}
	if (!PPVolume)
	{
		// Prefer finding any post process volume and making it unbound
		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			PPVolume = *It;
			break;
		}
	}
	if (!PPVolume)
	{
		UE_LOG(LogTemp, Log, TEXT("No PostProcessVolume found - spawning for exposure/color grading"));
		APostProcessVolume* SpawnedPP = World->SpawnActor<APostProcessVolume>(
			APostProcessVolume::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);
		if (SpawnedPP)
		{
			SpawnedPP->bUnbound = true;
			FPostProcessSettings& PPS = SpawnedPP->Settings;
			PPS.bOverride_AutoExposureMinBrightness = true;
			PPS.AutoExposureMinBrightness = 1.0f;  // Locked exposure ??? matches asset-preview consistency
			PPS.bOverride_AutoExposureMaxBrightness = true;
			PPS.AutoExposureMaxBrightness = 1.0f;  // Same as min = no auto-exposure variation
			PPS.bOverride_AmbientOcclusionIntensity = true;
			PPS.AmbientOcclusionIntensity = 0.0f;  // AO off ??? eliminates dark creases on characters
			PPS.bOverride_ColorSaturation = true;
			PPS.ColorSaturation = FVector4(0.95f, 0.95f, 0.95f, 1.f); // Slight desaturation so scene isn't uniformly punchy
#if WITH_EDITOR
			SpawnedPP->SetActorLabel(TEXT("DEV_PostProcessVolume"));
#endif
			SpawnedSetupActors.Add(SpawnedPP);
		}
	}
	else if (PPVolume)
	{
		// Align level-placed PP with frontend/gameplay defaults so preview and gameplay match.
		PPVolume->bUnbound = true;
		FPostProcessSettings& PPS = PPVolume->Settings;
		PPS.bOverride_AutoExposureMinBrightness = true;
		PPS.AutoExposureMinBrightness = 1.0f;  // Locked exposure
		PPS.bOverride_AutoExposureMaxBrightness = true;
		PPS.AutoExposureMaxBrightness = 1.0f;  // Locked exposure
		PPS.bOverride_AmbientOcclusionIntensity = true;
		PPS.AmbientOcclusionIntensity = 0.0f;  // AO off ??? eliminates dark creases on characters
		PPS.bOverride_ColorSaturation = true;
		PPS.ColorSaturation = FVector4(0.95f, 0.95f, 0.95f, 1.f);
	}

	// Force ALL directional and sky lights to Movable, no shadows, aligned colors (theme sets intensities).
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		ADirectionalLight* DirLight = *It;
		if (UDirectionalLightComponent* LC = Cast<UDirectionalLightComponent>(DirLight->GetLightComponent()))
		{
			LC->SetMobility(EComponentMobility::Movable);
			LC->CastShadows = false; // Shadows disabled globally ??? replicates asset-preview look
			LC->bAtmosphereSunLight = true;
			LC->AtmosphereSunLightIndex = DirLight->Tags.Contains(MoonTag) ? 1 : 0;
			LC->SetForwardShadingPriority(DirLight->Tags.Contains(MoonTag) ? 0 : 1); // Sun primary for forward shading
			// Align level-placed light colors with frontend so preview and gameplay match.
			if (DirLight->Tags.Contains(MoonTag))
			{
				LC->SetLightColor(FLinearColor(0.72f, 0.8f, 1.f));
			}
			else
			{
				LC->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f));
				LC->SetIntensity(3.f); // Fill light ??? SkyLight is primary ambient
			}
		}
		if (USceneComponent* Root = DirLight->GetRootComponent())
		{
			Root->SetMobility(EComponentMobility::Movable);
		}
	}
	ApplyThemeToDirectionalLights();

	// Configure all SkyLights: HDRI cubemap (asset-preview quality) or boosted sky capture fallback.
	// Lumen is disabled; the SkyLight is the primary source of ambient/indirect light.

	// In editor: auto-create the TextureCube from the equirectangular HDRI if it doesn't exist yet.
#if WITH_EDITORONLY_DATA
	EnsureHDRICubemap();
#endif

	UTextureCube* HDRICubemap = LoadObject<UTextureCube>(nullptr, TEXT("/Game/Lighting/TC_HDRI_Studio.TC_HDRI_Studio"));

	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		if (USkyLightComponent* SC = Cast<USkyLightComponent>(It->GetLightComponent()))
		{
			SC->SetMobility(EComponentMobility::Movable);

			if (HDRICubemap)
			{
				// Studio HDRI cubemap: replicates asset-preview lighting quality.
				SC->SourceType = ESkyLightSourceType::SLS_SpecifiedCubemap;
				SC->Cubemap = HDRICubemap;
				SC->SetIntensity(8.0f); // Dominant ambient ??? overshooting intentionally for bright characters
				SC->bLowerHemisphereIsBlack = false;
				SC->SetLowerHemisphereColor(FLinearColor(0.95f, 0.95f, 0.95f)); // Near-white underside fill
				UE_LOG(LogTemp, Log, TEXT("[LIGHT] SkyLight using HDRI cubemap (studio lighting, intensity 8.0)"));
			}
			else
			{
				// Fallback: capture sky atmosphere with boosted intensity + lower hemisphere fill.
				SC->SourceType = ESkyLightSourceType::SLS_CapturedScene;
				SC->SetIntensity(8.0f);
				SC->bLowerHemisphereIsBlack = false;
				SC->SetLowerHemisphereColor(FLinearColor(0.95f, 0.95f, 0.95f));
				UE_LOG(LogTemp, Log, TEXT("[LIGHT] SkyLight using boosted sky capture (no HDRI cubemap found, intensity 8.0)"));
			}

			SC->SetLightColor(FLinearColor::White);
			SC->RecaptureSky();
		}
		if (USceneComponent* Root = It->GetRootComponent())
		{
			Root->SetMobility(EComponentMobility::Movable);
		}
	}

	ApplyThemeToAtmosphereAndLighting();
}

void AT66GameMode::ApplyThemeToDirectionalLightsForWorld(UWorld* World)
{
	if (!World) return;

	static const FName MoonTag(TEXT("T66Moon"));
	ADirectionalLight* SunLight = nullptr;
	ADirectionalLight* MoonLight = nullptr;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		ADirectionalLight* L = *It;
		if (L->Tags.Contains(MoonTag))
			MoonLight = L;
		else
			SunLight = L;
	}
	if (!SunLight) return;

	UDirectionalLightComponent* SunComp = Cast<UDirectionalLightComponent>(SunLight->GetLightComponent());
	if (!SunComp) return;

	// Demo map (e.g. Map_Summer): lower sun intensity so it's not as harsh
	FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
	const bool bDemoMap = MapName.Equals(UT66GameInstance::GetDemoMapLevelNameForTribulation().ToString(), ESearchCase::IgnoreCase);
	const float SunIntensityLight = bDemoMap ? 2.f : 3.f;
	const float SunIntensityDark = bDemoMap ? 1.2f : 2.f;

	const ET66UITheme Theme = FT66Style::GetTheme();
	if (Theme == ET66UITheme::Light)
	{
		SunComp->SetIntensity(SunIntensityLight);
		SunComp->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f));
		SunComp->bAtmosphereSunLight = true;
		SunComp->AtmosphereSunLightIndex = 0;
	}
	else
	{
		// Dark theme: Eclipse Sun ??? sun off, eclipsed sun (moon light) is the primary light.
		SunComp->SetIntensity(0.f);
		SunComp->bAtmosphereSunLight = false;
	}
	if (MoonLight)
	{
		UDirectionalLightComponent* MoonComp = Cast<UDirectionalLightComponent>(MoonLight->GetLightComponent());
		if (MoonComp)
		{
			if (Theme == ET66UITheme::Light)
			{
				MoonComp->SetIntensity(0.f);
				MoonComp->SetLightColor(FLinearColor(0.72f, 0.8f, 1.f));
				MoonComp->bAtmosphereSunLight = true;
				MoonComp->AtmosphereSunLightIndex = 1;
				MoonComp->SetAtmosphereSunDiskColorScale(FLinearColor(1.f, 1.f, 1.f));
				MoonLight->SetActorRotation(FRotator(50.f, 135.f, 0.f));
			}
		else
		{
			// Eclipse dusk: dimmed sun, warm white light, no shadows, hide built-in sun disk
			MoonComp->SetIntensity(5.0f);
			MoonComp->SetLightColor(FLinearColor(1.0f, 0.95f, 0.9f));
			MoonComp->bAtmosphereSunLight = true;
			MoonComp->AtmosphereSunLightIndex = 0;
			MoonComp->SetAtmosphereSunDiskColorScale(FLinearColor(0.f, 0.f, 0.f));
			MoonComp->SetCastShadows(false);
			MoonLight->SetActorRotation(FRotator(-50.f, 135.f, 0.f));
		}
		}
	}
}

void AT66GameMode::ApplyThemeToDirectionalLights()
{
	ApplyThemeToDirectionalLightsForWorld(GetWorld());
}

void AT66GameMode::ApplyThemeToAtmosphereAndLightingForWorld(UWorld* World)
{
	if (!World) return;

	const ET66UITheme Theme = FT66Style::GetTheme();
	const bool bDark = (Theme == ET66UITheme::Dark);

	// --- SkyAtmosphere: red Rayleigh scattering for blood-red sky in Dark mode ---
	for (TActorIterator<ASkyAtmosphere> It(World); It; ++It)
	{
		USkyAtmosphereComponent* Atmos = It->FindComponentByClass<USkyAtmosphereComponent>();
		if (!Atmos) continue;

		if (bDark)
		{
			Atmos->RayleighScattering = FLinearColor(0.028f, 0.005f, 0.004f);
			Atmos->RayleighScatteringScale = 0.8f;
			Atmos->MieScatteringScale = 0.01f;
			Atmos->MultiScatteringFactor = 0.5f;
		}
		else
		{
			Atmos->RayleighScattering = FLinearColor(0.005802f, 0.013558f, 0.033100f);
			Atmos->RayleighScatteringScale = 1.0f;
			Atmos->MieScatteringScale = 0.003996f;
			Atmos->MultiScatteringFactor = 1.0f;
		}
		Atmos->MarkRenderStateDirty();
		break;
	}

	// --- SkyLight: captured scene with red tint in Dark, HDRI cubemap in Light ---
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		USkyLightComponent* SC = Cast<USkyLightComponent>(It->GetLightComponent());
		if (!SC) continue;

		if (bDark)
		{
			SC->SourceType = ESkyLightSourceType::SLS_CapturedScene;
			SC->Cubemap = nullptr;
			SC->SetIntensity(4.0f);
			SC->SetLightColor(FLinearColor(0.25f, 0.2f, 0.25f));
			SC->bLowerHemisphereIsBlack = false;
			SC->SetLowerHemisphereColor(FLinearColor(0.12f, 0.1f, 0.12f));
		}
		else
		{
			UTextureCube* HDRICubemap = LoadObject<UTextureCube>(nullptr, TEXT("/Game/Lighting/TC_HDRI_Studio.TC_HDRI_Studio"));
			if (HDRICubemap)
			{
				SC->SourceType = ESkyLightSourceType::SLS_SpecifiedCubemap;
				SC->Cubemap = HDRICubemap;
			}
			else
			{
				SC->SourceType = ESkyLightSourceType::SLS_CapturedScene;
			}
			SC->SetIntensity(8.0f);
			SC->SetLightColor(FLinearColor::White);
			SC->bLowerHemisphereIsBlack = false;
			SC->SetLowerHemisphereColor(FLinearColor(0.95f, 0.95f, 0.95f));
		}
		SC->RecaptureSky();
		break;
	}

	// --- ExponentialHeightFog: black inscattering in Dark (moon color dominates) ---
	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		UExponentialHeightFogComponent* FogComp = It->FindComponentByClass<UExponentialHeightFogComponent>();
		if (!FogComp) FogComp = Cast<UExponentialHeightFogComponent>(It->GetRootComponent());
		if (!FogComp) continue;

		if (bDark)
		{
			FogComp->SetFogInscatteringColor(FLinearColor(0.0f, 0.0f, 0.0f));
		}
		else
		{
			FogComp->SetFogInscatteringColor(FLinearColor(0.6f, 0.65f, 0.78f));
		}
		break;
	}

	// --- PostProcess color grading: neutral in Dark (red comes from sky, not color grade) ---
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		if (!It->bUnbound) continue;
		FPostProcessSettings& PPS = It->Settings;

		if (bDark)
		{
			PPS.bOverride_ColorSaturation = true;
			PPS.ColorSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.f);
			PPS.bOverride_ColorGamma = false;
			PPS.bOverride_ColorGain = false;

			PPS.bOverride_BloomIntensity = true;
			PPS.BloomIntensity = 0.0f;
			PPS.bOverride_BloomThreshold = true;
			PPS.BloomThreshold = 10.0f;
		}
		else
		{
			PPS.bOverride_ColorSaturation = true;
			PPS.ColorSaturation = FVector4(0.95f, 0.95f, 0.95f, 1.f);
			PPS.bOverride_ColorGamma = false;
			PPS.bOverride_ColorGain = false;

			PPS.bOverride_BloomIntensity = false;
			PPS.bOverride_BloomThreshold = false;
		}
		break;
	}

	// --- Posterize: on in Dark, off in Light ---
	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UT66PosterizeSubsystem* Post = GI->GetSubsystem<UT66PosterizeSubsystem>())
		{
			Post->SetEnabled(bDark);
		}
	}

	// --- Eclipse corona: visible in Dark, destroyed in Light ---
	{
		AT66EclipseActor* ExistingEclipse = nullptr;
		for (TActorIterator<AT66EclipseActor> It(World); It; ++It)
		{
			ExistingEclipse = *It;
			break;
		}

		if (bDark && !ExistingEclipse)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			World->SpawnActor<AT66EclipseActor>(AT66EclipseActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}
		else if (!bDark && ExistingEclipse)
		{
			ExistingEclipse->Destroy();
		}
	}

}

void AT66GameMode::ApplyThemeToAtmosphereAndLighting()
{
	ApplyThemeToAtmosphereAndLightingForWorld(GetWorld());
}

void AT66GameMode::HandleSettingsChanged()
{
	ApplyThemeToDirectionalLights();
	ApplyThemeToAtmosphereAndLighting();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RetroFXSubsystem* RetroFX = GI->GetSubsystem<UT66RetroFXSubsystem>())
		{
			RetroFX->ApplyCurrentSettings(GetWorld());
		}
	}
}

void AT66GameMode::SpawnPlayerStartIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Check for existing player start
	bool bHasPlayerStart = false;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		bHasPlayerStart = true;
		break;
	}

	if (!bHasPlayerStart)
	{
		UE_LOG(LogTemp, Log, TEXT("No PlayerStart found - spawning development PlayerStart"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Default spawn:
		// - Demo map (e.g. Map_Summer): origin (0, 0, DefaultSpawnHeight)
		// - Coliseum mode: coliseum arena (timer starts immediately; no pillars)
		// - Normal stage: start area (so timer starts after passing start pillars)
		FVector SpawnLoc;
		FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		if (MapName.Equals(UT66GameInstance::GetDemoMapLevelNameForTribulation().ToString(), ESearchCase::IgnoreCase))
		{
			SpawnLoc = FVector(0.f, 0.f, DefaultSpawnHeight);
		}
		else if (IsColiseumStage())
		{
			SpawnLoc = FVector(ColiseumCenter.X, ColiseumCenter.Y, DefaultSpawnHeight);
		}
		else
		{
			SpawnLoc = FVector(-35455.f, 0.f, DefaultSpawnHeight);
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
			UE_LOG(LogTemp, Log, TEXT("Spawned development PlayerStart at %s"), *SpawnLoc.ToString());
		}
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

	if (GroundFloorMaterials.Num() > 0 && GroundFloorMaterials[0].Get())
	{
		Floor->GetStaticMeshComponent()->SetMaterial(0, GroundFloorMaterials[0].Get());
	}
	SpawnedSetupActors.Add(Floor);
	UE_LOG(LogTemp, Log, TEXT("Spawned Lab central floor (half-extent %.0f, top Z %.0f)"), LabHalfExtent, LabFloorHeight);
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
		UE_LOG(LogTemp, Log, TEXT("Spawned The Collector in Lab"));
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
	UWorld* World = GetWorld();
	if (!World || !Actor)
	{
		return false;
	}

	FHitResult GroundHit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66SnapActorToTerrain), false, Actor);
	const FVector ActorLoc = Actor->GetActorLocation();
	const FVector TraceStart = ActorLoc + FVector(0.f, 0.f, 8000.f);
	const FVector TraceEnd = ActorLoc - FVector(0.f, 0.f, 16000.f);
	if (!World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic, Params) &&
		!World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, Params))
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

void AT66GameMode::RegenerateMap(ET66MapTheme Theme, int32 Seed)
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

	DestroyActorsWithTag(World, T66MapPlatformTag);
	DestroyActorsWithTag(World, T66MapRampTag);
	DestroyActorsWithTag(World, T66FloorMainTag);

	UT66GameInstance* GI = GetT66GameInstance();
	if (GI)
	{
		GI->MapTheme = Theme;
		GI->RunSeed  = Seed;
	}

	SpawnLowPolyNatureEnvironment();
}

// Console command: T66.Map <Farm|Ocean|Mountain> [seed]
static FAutoConsoleCommandWithWorldAndArgs T66MapCmd(
	TEXT("T66.Map"),
	TEXT("Regenerate procedural platform map. Usage: T66.Map <Farm|Ocean|Mountain> [seed]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args, UWorld* World)
		{
			if (!World) return;
			AGameModeBase* GM = World->GetAuthGameMode();
			AT66GameMode* T66GM = Cast<AT66GameMode>(GM);
			if (!T66GM)
			{
				UE_LOG(LogTemp, Error, TEXT("T66.Map: no T66GameMode active"));
				return;
			}

			ET66MapTheme Theme = ET66MapTheme::Farm;
			if (Args.Num() >= 1)
			{
				const FString& ThemeStr = Args[0];
				if (ThemeStr.Equals(TEXT("Ocean"), ESearchCase::IgnoreCase))
					Theme = ET66MapTheme::Ocean;
				else if (ThemeStr.Equals(TEXT("Mountain"), ESearchCase::IgnoreCase))
					Theme = ET66MapTheme::Mountain;
				else if (!ThemeStr.Equals(TEXT("Farm"), ESearchCase::IgnoreCase))
					UE_LOG(LogTemp, Warning, TEXT("T66.Map: unknown theme '%s', defaulting to Farm"), *ThemeStr);
			}

			int32 Seed = FMath::Rand();
			if (Args.Num() >= 2)
			{
				Seed = FCString::Atoi(*Args[1]);
			}

			UE_LOG(LogTemp, Log, TEXT("T66.Map: Regenerating map (theme=%s, seed=%d)"), *Args[0], Seed);
			T66GM->RegenerateMap(Theme, Seed);
		})
);

UClass* AT66GameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// Check if we have a specific hero class from the DataTable
	if (UT66GameInstance* GI = GetT66GameInstance())
	{
		FHeroData HeroData;
		if (GI->GetSelectedHeroData(HeroData))
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
		UE_LOG(LogTemp, Error, TEXT("No pawn class for spawning!"));
		return nullptr;
	}

	// Get spawn transform - use a safe default height if no PlayerStart exists
	FVector SpawnLocation;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	
	if (StartSpot)
	{
		SpawnLocation = StartSpot->GetActorLocation();
		SpawnRotation = StartSpot->GetActorRotation();
		UE_LOG(LogTemp, Log, TEXT("Spawning at PlayerStart: (%.1f, %.1f, %.1f)"), 
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
	}
	else
	{
		// No PlayerStart found - spawn at a safe default location (100k map or demo map).
		UWorld* World = GetWorld();
		FString MapName = World ? UWorld::RemovePIEPrefix(World->GetMapName()) : FString();
		if (MapName.Equals(UT66GameInstance::GetDemoMapLevelNameForTribulation().ToString(), ESearchCase::IgnoreCase))
		{
			SpawnLocation = FVector(0.f, 0.f, 200.f);
		}
		else
		{
			SpawnLocation = IsColiseumStage()
				? FVector(ColiseumCenter.X, ColiseumCenter.Y, 200.f)
				: FVector(-35455.f, 0.f, 200.f);
		}
		UE_LOG(LogTemp, Warning, TEXT("No PlayerStart found! Spawning at default location (%.0f, %.0f, %.0f)."),
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
	}

	// Robust: always ensure Gameplay spawns in the Start Area (scaled for 100k map).
	if (IsLabLevel())
	{
		SpawnLocation = FVector(0.f, 0.f, 120.f);
		SpawnRotation = FRotator::ZeroRotator;
	}
	else if (IsDemoMapForTribulation())
	{
		SpawnLocation = FVector(0.f, 0.f, 200.f);
		SpawnRotation = FRotator::ZeroRotator;
	}
	else if (!IsColiseumStage())
	{
		SpawnLocation = FVector(-35455.f, 0.f, 200.f);
		SpawnRotation = FRotator::ZeroRotator;

		if (UT66GameInstance* T66GI = GetT66GameInstance())
		{
		if (T66GI->bStageCatchUpPending)
		{
			SpawnLocation = FVector(-45455.f, 23636.f, 200.f);
			}
			else if (bForceSpawnInTutorialArea)
			{
				SpawnLocation = FVector(-3636.f, 56818.f, 200.f);
				SpawnRotation = FRotator::ZeroRotator;
			}
		}
	}
	else
	{
		// Forced coliseum mode uses an enclosed arena off to the side (not the main arena).
		if (UT66GameInstance* T66GI = GetT66GameInstance())
		{
			if (T66GI->bForceColiseumMode)
			{
				SpawnLocation = ColiseumCenter;
				SpawnRotation = FRotator::ZeroRotator;
			}
		}
	}

	// Sky-drop: spawn the hero at a comfortable altitude; gravity + Landed() handle the rest.
	// This avoids trace-before-terrain-exists timing issues and gives a cinematic entrance.

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn the pawn
	APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (SpawnedPawn)
	{
		const bool bNeedsProceduralTerrain = !IsLabLevel() && !IsColiseumStage();
		if (bTerrainCollisionReady)
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

	// If it's our hero class, initialize it with hero data and body type
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(SpawnedPawn))
	{

		if (UT66GameInstance* GI = GetT66GameInstance())
		{
			FHeroData HeroData;
			if (GI->GetSelectedHeroData(HeroData))
			{
				// Pass hero data, body type, and skin from Game Instance
				ET66BodyType SelectedBodyType = GI->SelectedHeroBodyType;
				FName SelectedSkinID = GI->SelectedHeroSkinID.IsNone() ? FName(TEXT("Default")) : GI->SelectedHeroSkinID;
				Hero->InitializeHero(HeroData, SelectedBodyType, SelectedSkinID, false);

				UE_LOG(LogTemp, Log, TEXT("Spawned hero: %s, BodyType: %s, Skin: %s, Color: (%.2f, %.2f, %.2f)"),
					*HeroData.DisplayName.ToString(),
					SelectedBodyType == ET66BodyType::TypeA ? TEXT("TypeA") : TEXT("TypeB"),
					*SelectedSkinID.ToString(),
					HeroData.PlaceholderColor.R,
					HeroData.PlaceholderColor.G,
					HeroData.PlaceholderColor.B);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No hero selected in Game Instance - spawning with defaults"));
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
		Fountain->SetRarity(ET66Rarity::Black);
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
