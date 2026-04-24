// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameModePrivate.h"

using namespace T66GameModePrivate;

void AT66GameMode::BeginPlay()
{
	Super::BeginPlay();

	EnsureGameplayStartupInitialized(TEXT("BeginPlay"));
}

void AT66GameMode::StartPlay()
{
	const bool bRecoveringSkippedBeginPlay = !bGameplayStartupInitialized;
	Super::StartPlay();

	EnsureGameplayStartupInitialized(TEXT("StartPlay"));

	if (bRecoveringSkippedBeginPlay)
	{
		UE_LOG(LogT66GameMode, Warning, TEXT("AT66GameMode recovered gameplay startup from StartPlay because BeginPlay initialization was skipped."));
	}
}

void AT66GameMode::EnsureGameplayStartupInitialized(const TCHAR* TriggerContext)
{
	if (bGameplayStartupInitialized)
	{
		return;
	}

	bGameplayStartupInitialized = true;

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
			PS->OnSettingsChanged.RemoveDynamic(this, &AT66GameMode::HandleSettingsChanged);
			PS->OnSettingsChanged.AddDynamic(this, &AT66GameMode::HandleSettingsChanged);
		}
	}
	HandleSettingsChanged();

	if (HandleSpecialModeBeginPlay())
	{
		UE_LOG(LogT66GameMode, Log, TEXT("T66GameMode %s - special-mode startup initialized."), TriggerContext);
		return;
	}

	ConsumePendingStageCatchUp();
	ScheduleDeferredGameplayLevelSpawn();
	UE_LOG(LogT66GameMode, Log, TEXT("T66GameMode %s - level setup scheduled; content will spawn after landscape is ready."), TriggerContext);
}

void AT66GameMode::InitializeRunStateForBeginPlay()
{
	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GetT66GameInstance();
	PendingRunStartItemId = NAME_None;
	if (!GI || !T66GI)
	{
		return;
	}

	if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
	{
		// Bind to timer changes so we can spawn LoanShark exactly when timer starts.
		RunState->StageTimerChanged.AddDynamic(this, &AT66GameMode::HandleStageTimerChanged);
		RunState->StageChanged.AddDynamic(this, &AT66GameMode::HandleStageChanged);
		RunState->DifficultyChanged.AddDynamic(this, &AT66GameMode::HandleDifficultyChanged);
		RunState->EndSaintBlessingEmpowerment();
		RunState->SetSaintBlessingActive(false);
		RunState->SetFinalSurvivalEnemyScalar(1.f);
		if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
		{
			IdolManager->IdolStateChanged.RemoveDynamic(this, &AT66GameMode::HandleIdolStateChanged);
			IdolManager->IdolStateChanged.AddDynamic(this, &AT66GameMode::HandleIdolStateChanged);
		}

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
			if (UT66PlayerExperienceSubSystem* PlayerExperience = GI->GetSubsystem<UT66PlayerExperienceSubSystem>())
			{
				const int32 DifficultyStartStage = PlayerExperience->GetDifficultyStartStage(T66GI->SelectedDifficulty);
				const bool bEnteringDifficultyStartStage =
					DifficultyStartStage > 1
					&& RunState->GetCurrentStage() == DifficultyStartStage;
				if (bEnteringDifficultyStartStage)
				{
					RunState->ResetDifficultyPacing();
					RunState->ResetDifficultyScore();
				}
			}

			T66GI->bIsStageTransition = false;
			RunState->ResetStageTimerToFull(); // New stage: timer is frozen at full until combat starts.
			RunState->ResetBossState(); // New stage: boss is dormant again; hide boss UI
			return;
		}

		if (UT66RunIntegritySubsystem* Integrity = GI->GetSubsystem<UT66RunIntegritySubsystem>())
		{
			Integrity->CaptureFreshRunBaseline();

			if (T66GI->HasSelectedRunModifier())
			{
				const FString ModifierKind =
					(T66GI->SelectedRunModifierKind == ET66RunModifierKind::Challenge)
					? TEXT("challenge")
					: ((T66GI->SelectedRunModifierKind == ET66RunModifierKind::Mod) ? TEXT("mod") : TEXT("unknown"));
				Integrity->MarkRunModifierSelected(ModifierKind, T66GI->SelectedRunModifierID);
			}

			T66GI->bRunIneligibleForLeaderboard = !Integrity->GetCurrentContext().ShouldAllowRankedSubmission();
		}
		else
		{
			T66GI->bRunIneligibleForLeaderboard = T66GI->HasSelectedRunModifier();
		}
		T66GI->bPendingTowerStageDropIntro = false;
		RunState->ResetForNewRun();
		PendingRunStartItemId = RunState->ConsumeDeferredRunStartItemId();
		if (!PendingRunStartItemId.IsNone())
		{
			UE_LOG(LogT66GameMode, Log, TEXT("[Community] Queued deferred run-start item grant: %s"), *PendingRunStartItemId.ToString());
		}
		RunState->ActivatePendingSingleUseBuffsForRunStart();
		if (T66GI->IsDailyClimbRunActive())
		{
			const int32 BonusGold = T66GI->GetDailyClimbIntRuleValue(ET66DailyClimbRuleType::StartBonusGold, 0);
			if (BonusGold > 0)
			{
				RunState->AddGold(BonusGold);
			}

			const int32 RandomItemCount = T66GI->GetDailyClimbIntRuleValue(ET66DailyClimbRuleType::StartRandomItems, 0);
			if (RandomItemCount > 0)
			{
				FRandomStream DailyItemStream((T66GI->RunSeed != 0 ? T66GI->RunSeed : 1) ^ 0x4441494C);
				for (int32 ItemIndex = 0; ItemIndex < RandomItemCount; ++ItemIndex)
				{
					const FName ItemID = T66GI->GetRandomItemIDFromStream(DailyItemStream);
					if (!ItemID.IsNone())
					{
						RunState->AddItem(ItemID);
					}
				}
			}
		}
		if (UT66DamageLogSubsystem* DamageLog = GI->GetSubsystem<UT66DamageLogSubsystem>())
		{
			DamageLog->ResetForNewRun();
		}
	}
}

bool AT66GameMode::HandleSpecialModeBeginPlay()
{
	if (IsLabLevel())
	{
		HandleLabBeginPlay();
		return true;
	}

	return false;
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
	if (bGameplayLevelSpawnScheduled || bGameplayLevelSpawnCompleted)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		bGameplayLevelSpawnScheduled = true;
		// Normal stage: defer all ground-dependent spawns until next tick so the landscape is fully formed and collision is ready.
		World->GetTimerManager().SetTimerForNextTick(this, &AT66GameMode::SpawnLevelContentAfterLandscapeReady);
	}
}

void AT66GameMode::SpawnLevelContentAfterLandscapeReady()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		bGameplayLevelSpawnScheduled = false;
		return;
	}

	if (bGameplayLevelSpawnCompleted)
	{
		return;
	}

	bGameplayLevelSpawnScheduled = false;
	bGameplayLevelSpawnCompleted = true;
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
		ScheduleGameplayVisualCleanup(World);
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
	ScheduleGameplayVisualCleanup(World);
	ScheduleGameplayWarmupOverlayHide(World, GameplayWarmupOverlay);
}

TWeakObjectPtr<UT66LoadingScreenWidget> AT66GameMode::CreateGameplayWarmupOverlay(UWorld* World, bool bUsingMainMapTerrain) const
{
	if (!bUsingMainMapTerrain || !World)
	{
		return nullptr;
	}

	APlayerController* PC = nullptr;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* Candidate = It->Get())
		{
			if (Candidate->IsLocalController())
			{
				PC = Candidate;
				break;
			}
		}
	}
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

void AT66GameMode::ScheduleGameplayVisualCleanup(UWorld* World)
{
	if (!World)
	{
		return;
	}

	// Re-run world visual cleanup after runtime terrain/props register so no legacy sky/light actors survive PIE startup.
	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(GetWorld());
		ApplyStageProgressionVisuals();
	}));

	FTimerHandle DelayedVisualCleanupHandle;
	World->GetTimerManager().SetTimer(
		DelayedVisualCleanupHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(GetWorld());
			ApplyStageProgressionVisuals();
		}),
		0.35f,
		false);

	FTimerHandle FinalVisualCleanupHandle;
	World->GetTimerManager().SetTimer(
		FinalVisualCleanupHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(GetWorld());
			ApplyStageProgressionVisuals();
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
	if (!bUsingMainMapTerrain && !IsLabLevel())
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
		SpawnCasinoInteractableIfNeeded();
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

	ResetTowerMiasmaState();
	if (IsUsingTowerMainMapLayout())
	{
		T66DestroyMiasmaBoundaryActors(World);
	}
	else
	{
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

		T66ActivateStageMiasma(MiasmaManager);

		if (!T66HasRegisteredMiasmaBoundary(World))
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			T66EnsureMiasmaBoundaryActor(World, SpawnParams);
		}
	}

	DestroyEnemyDirectors(World);

	bMainMapCombatStarted = false;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->ResetStageTimerToFull();
			RunState->SetStageTimerActive(false);
			RunState->StartSpeedRunTimer(true);
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
		if (IsUsingTowerMainMapLayout())
		{
			if (!IsValid(MiasmaManager))
			{
				MiasmaManager = World->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			}
			if (MiasmaManager)
			{
				MiasmaManager->RebuildForCurrentStage();
				MiasmaManager->SetExpansionActive(false);
				MiasmaManager->ClearTowerSourceAnchors();
				if (IsValid(IdolAltar))
				{
					SyncTowerMiasmaSourceAnchor(CachedTowerMainMapLayout.StartFloorNumber, IdolAltar->GetActorLocation());
				}
				if (bTowerMiasmaActive)
				{
					T66ActivateStageMiasma(MiasmaManager);
					MiasmaManager->SetExpansionActive(true, GetTowerMiasmaElapsedSeconds());
					MiasmaManager->UpdateFromRunState();
				}
				else
				{
					T66PauseTowerMiasma(MiasmaManager);
				}
			}
			T66DestroyMiasmaBoundaryActors(World);
		}
		else
		{
			if (!IsValid(MiasmaManager))
			{
				MiasmaManager = World->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			}
			else
			{
				MiasmaManager->RebuildForCurrentStage();
			}

			T66ActivateStageMiasma(MiasmaManager);

			T66EnsureMiasmaBoundaryActor(World, SpawnParams);

			if (MiasmaManager)
			{
				MiasmaManager->UpdateFromRunState();
			}
		}
	}

	UE_LOG(LogT66GameMode, Log, TEXT("[GOLD] Phase2-Tick2: %s."),
		IsUsingTowerMainMapLayout()
			? (bTowerMiasmaActive ? TEXT("tower miasma armed") : TEXT("tower miasma primed"))
			: TEXT("miasma systems spawned"));

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
	if (IsUsingTowerMainMapLayout() && IsBossRushFinaleStage())
	{
		UE_LOG(LogT66GameMode, Log, TEXT("[GOLD] Phase2-Tick3: skipped enemy director for boss-rush finale stage."));
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				FinalizeStandardStageCombatBootstrap();
			}));
		}
		return;
	}

	if (UWorld* World = GetWorld())
	{
		const bool bHadEnemyDirector = (FindOrCacheEnemyDirector(World) != nullptr);
		EnsureEnemyDirector(World);
		UE_LOG(LogT66GameMode, Log, TEXT("[GOLD] Phase2-Tick3: enemy director %s."), bHadEnemyDirector ? TEXT("reused") : TEXT("spawned"));
	}

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

	RefreshProgressionDrivenSystems(false);
	UE_LOG(LogT66GameMode, Log, TEXT("[GOLD] Phase2-Tick4: boss + traversal flow spawned. Phase 2 complete."));
	UE_LOG(LogT66GameMode, Log, TEXT("T66GameMode - Phase 2 content spawned (combat systems + boss)."));
}

void AT66GameMode::SpawnTutorialIfNeeded()
{
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

	if (TutorialManager.IsValid())
	{
		return;
	}

	for (TActorIterator<AT66TutorialManager> It(World); It; ++It)
	{
		if (AT66TutorialManager* ExistingTutorialManager = *It)
		{
			TutorialManager = ExistingTutorialManager;
			return;
		}
	}

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AT66TutorialManager* M = World->SpawnActor<AT66TutorialManager>(AT66TutorialManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, P);
	if (M)
	{
		TutorialManager = M;
		SpawnedSetupActors.Add(M);
	}
}
