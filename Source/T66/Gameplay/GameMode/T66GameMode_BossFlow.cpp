// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameModePrivate.h"

using namespace T66GameModePrivate;

bool AT66GameMode::IsBossRushFinaleStage() const
{
	const UT66GameInstance* T66GI = GetT66GameInstance();
	const UGameInstance* GI = GetGameInstance();
	const UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState)
	{
		return false;
	}

	const UT66DifficultyTuningSubsystem* DifficultyTuning = GI ? GI->GetSubsystem<UT66DifficultyTuningSubsystem>() : nullptr;
	const int32 DifficultyEndStage = DifficultyTuning
		? DifficultyTuning->GetDifficultyEndStage(T66GI->SelectedDifficulty)
		: 20;
	return RunState->GetCurrentStage() == DifficultyEndStage;
}

void AT66GameMode::SpawnBossGateIfNeeded()
{
	if (IsUsingTowerMainMapLayout()) return;
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
		TSoftObjectPtr<UStaticMesh> TotemMesh(FSoftObjectPath(TEXT("/Game/World/Interactables/DifficultyTotem/SM_DifficultyTotem_Pixal3D.SM_DifficultyTotem_Pixal3D")));
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
	RunState->SetSaintBlessingActive(true);
	RunState->SetFinalSurvivalEnemyScalar(1.f);
	RunState->SetStageTimerActive(true);

	ClearMiasma();
	SpawnFinalDifficultyTotem(BossDeathLocation);
	SpawnFinalDifficultySaint(BossDeathLocation);

	if (UWorld* World = GetWorld())
	{
		if (AT66EnemyDirector* ExistingEnemyDirector = EnsureEnemyDirector(World))
		{
			ExistingEnemyDirector->SetSpawningPaused(false);
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
	RunState->SetStageTimerActive(false);

	UWorld* World = GetWorld();
	if (World)
	{
		const FVector ExitLocation = !MainMapBossSpawnSurfaceLocation.IsNearlyZero()
			? (MainMapBossSpawnSurfaceLocation + FVector(0.f, 0.f, 200.f))
			: (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero()
				? (MainMapBossAreaCenterSurfaceLocation + FVector(0.f, 0.f, 200.f))
				: FVector(0.f, 0.f, 200.f));
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
		UE_LOG(LogT66GameMode, Warning, TEXT("Difficulty clear reached but no player controllers were available to open Run Summary."));
	}
}

void AT66GameMode::HandleBossDefeated(AT66BossBase* Boss)
{
	UWorld* World = GetWorld();
	if (!World) return;
	const FVector Location = Boss ? Boss->GetActorLocation() : FVector::ZeroVector;
	if (Boss && StageBoss.Get() == Boss)
	{
		StageBoss = nullptr;
	}
	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const UT66PlayerExperienceSubSystem* PlayerExperience = GI ? GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	const UT66DifficultyTuningSubsystem* DifficultyTuning = GI ? GI->GetSubsystem<UT66DifficultyTuningSubsystem>() : nullptr;
	const UT66GameInstance* CurrentT66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;

	// Award boss score using the same totem-only scalar used when the boss was budgeted.
	if (RunState && Boss)
	{
		const ET66Difficulty Difficulty = CurrentT66GI ? CurrentT66GI->SelectedDifficulty : ET66Difficulty::Easy;
		const int32 AwardPoints = PlayerExperience
			? PlayerExperience->ResolveBossScore(Difficulty, Boss->GetPointValue(), RunState->GetDifficultyScalar())
			: FMath::Max(0, FMath::RoundToInt(static_cast<float>(Boss->GetPointValue()) * RunState->GetDifficultyScalar()));
		if (AwardPoints > 0)
		{
			RunState->AddBossKillScore(AwardPoints, Boss->BossID);
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

	// Chad Coupons: 1 per boss kill.
	if (RunState)
	{
		RunState->AddPowerCrystalsEarnedThisRun(1);
	}

	AT66BossBase* RemainingBoss = nullptr;
	int32 RemainingBossCount = 0;
	if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
	{
		for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
		{
			AT66BossBase* CandidateBoss = WeakBoss.Get();
			if (!CandidateBoss || CandidateBoss == Boss || !CandidateBoss->IsAlive())
			{
				continue;
			}

			++RemainingBossCount;
			if (!RemainingBoss)
			{
				RemainingBoss = CandidateBoss;
			}
		}
	}

	if (RemainingBossCount > 0)
	{
		if (RunState && RemainingBoss)
		{
			RemainingBoss->RefreshRunStateBossState();
		}

		UE_LOG(LogT66GameMode, Log, TEXT("Boss defeated, but %d boss(es) remain active on this stage."), RemainingBossCount);
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
		const int32 DifficultyEndStage = DifficultyTuning
			? DifficultyTuning->GetDifficultyEndStage(SelectedDifficulty)
			: 20;
		bCompletedSelectedDifficulty = (RunState->GetCurrentStage() >= DifficultyEndStage);
	}

	// First-time stage clear unlock => one companion per stage through stage 16.
	// The final difficulty does not award additional companions.
	if (RunState)
	{
		const int32 StageNum = RunState->GetCurrentStage();
		TArray<FName> CompanionUnlockIDs;
		T66_AppendCompanionUnlockIDsForStage(StageNum, CompanionUnlockIDs);
		for (int32 CompanionIndex = 0; CompanionIndex < CompanionUnlockIDs.Num(); ++CompanionIndex)
		{
			const FName CompanionToUnlock = CompanionUnlockIDs[CompanionIndex];
			if (CompanionToUnlock.IsNone())
			{
				continue;
			}

			if (UT66CompanionUnlockSubsystem* Unlocks = GI ? GI->GetSubsystem<UT66CompanionUnlockSubsystem>() : nullptr)
			{
				const bool bNewlyUnlocked = Unlocks->UnlockCompanion(CompanionToUnlock);
				if (!bNewlyUnlocked)
				{
					continue;
				}

				if (UT66GameInstance* T66GI2 = GetT66GameInstance())
				{
					FCompanionData Data;
					if (T66GI2->GetCompanionData(CompanionToUnlock, Data))
					{
						const int32 UnlockCount = CompanionUnlockIDs.Num();
						const float HorizontalOffset = (static_cast<float>(CompanionIndex) - ((static_cast<float>(UnlockCount) - 1.f) * 0.5f)) * 180.f;
						const FVector RecruitSpawnLocation = Location + FVector(HorizontalOffset, 0.f, 0.f);

						FActorSpawnParameters SpawnParams;
						SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
						if (AT66RecruitableCompanion* Recruit = World->SpawnActor<AT66RecruitableCompanion>(AT66RecruitableCompanion::StaticClass(), RecruitSpawnLocation, FRotator::ZeroRotator, SpawnParams))
						{
							Recruit->InitializeRecruit(Data);
						}
					}
				}
			}
		}
	}

	const UT66GameInstance* T66GI = GetT66GameInstance();
	const ET66Difficulty SelectedDifficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	if (bCompletedSelectedDifficulty && (DifficultyTuning ? DifficultyTuning->DoesDifficultyUseFinalSequence(SelectedDifficulty) : SelectedDifficulty == ET66Difficulty::Impossible))
	{
		if (RunState)
		{
			RunState->ClearOwedBosses();
		}
		BeginFinalDifficultySurvival(Location);
		return;
	}

	// Normal stage: boss dead => miasma disappears and Stage Gate appears.
	ClearMiasma();
	if (bCompletedSelectedDifficulty)
	{
		if (RunState)
		{
			RunState->ClearOwedBosses();
		}

		if (UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr)
		{
			if (const UT66GameInstance* DifficultyClearGI = GetT66GameInstance())
			{
				if (!DifficultyClearGI->SelectedHeroID.IsNone())
				{
					Achievements->RecordHeroDifficultyClear(DifficultyClearGI->SelectedHeroID, DifficultyClearGI->SelectedDifficulty);
				}
				if (!DifficultyClearGI->SelectedCompanionID.IsNone())
				{
					Achievements->RecordCompanionDifficultyClear(DifficultyClearGI->SelectedCompanionID, DifficultyClearGI->SelectedDifficulty);
				}
			}
		}

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

	SpawnIdolAltarAtLocation(Location);
	SpawnStageGateAtLocation(Location);

	if (RunState)
	{
		const int32 ClearedStage = RunState->GetCurrentStage();
		UE_LOG(LogT66GameMode, Verbose, TEXT("Stage %d cleared; post-boss idol altar and stage gate spawned."), ClearedStage);
	}
}

void AT66GameMode::ClearMiasma()
{
	if (MiasmaManager)
	{
		MiasmaManager->ClearAllMiasma();
	}
}

void AT66GameMode::SpawnStageGateAtLocation(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World) return;

	const FVector RequestedLocation = Location;
	FVector SpawnLoc(Location.X, Location.Y, Location.Z);
	FVector FallbackSpawnLoc = SpawnLoc;
	const TCHAR* SpawnAnchorSource = TEXT("BossDeath");
	if (IsUsingTowerMainMapLayout())
	{
		if (!MainMapBossSpawnSurfaceLocation.IsNearlyZero())
		{
			FallbackSpawnLoc = MainMapBossSpawnSurfaceLocation;
		}
		else if (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero())
		{
			FallbackSpawnLoc = MainMapBossAreaCenterSurfaceLocation;
		}
	}

	FVector PlayerLocation = FVector::ZeroVector;
	int32 PlayerFloorNumber = INDEX_NONE;
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		PlayerLocation = PlayerPawn->GetActorLocation();
		if (IsUsingTowerMainMapLayout())
		{
			PlayerFloorNumber = GetTowerFloorIndexForLocation(PlayerLocation);
		}
	}

	const int32 RequestedFloorNumber = IsUsingTowerMainMapLayout() ? GetTowerFloorIndexForLocation(RequestedLocation) : INDEX_NONE;
	const int32 AnchorFloorNumber = IsUsingTowerMainMapLayout() ? GetTowerFloorIndexForLocation(FallbackSpawnLoc) : INDEX_NONE;
	const int32 TargetTowerFloorNumber = IsUsingTowerMainMapLayout()
		? (RequestedFloorNumber != INDEX_NONE
			? RequestedFloorNumber
			: (AnchorFloorNumber != INDEX_NONE ? AnchorFloorNumber : CachedTowerMainMapLayout.BossFloorNumber))
		: INDEX_NONE;
	UE_LOG(
		LogT66GameMode,
		Warning,
		TEXT("SpawnStageGateAtLocation: request=(%.0f, %.0f, %.0f) requestFloor=%d anchorSource=%s resolvedAnchor=(%.0f, %.0f, %.0f) anchorFloor=%d player=(%.0f, %.0f, %.0f) playerFloor=%d bossSpawnSurface=(%.0f, %.0f, %.0f) bossAreaCenter=(%.0f, %.0f, %.0f)"),
		RequestedLocation.X,
		RequestedLocation.Y,
		RequestedLocation.Z,
		RequestedFloorNumber,
		SpawnAnchorSource,
		FallbackSpawnLoc.X,
		FallbackSpawnLoc.Y,
		FallbackSpawnLoc.Z,
		AnchorFloorNumber,
		PlayerLocation.X,
		PlayerLocation.Y,
		PlayerLocation.Z,
		PlayerFloorNumber,
		MainMapBossSpawnSurfaceLocation.X,
		MainMapBossSpawnSurfaceLocation.Y,
		MainMapBossSpawnSurfaceLocation.Z,
		MainMapBossAreaCenterSurfaceLocation.X,
		MainMapBossAreaCenterSurfaceLocation.Y,
		MainMapBossAreaCenterSurfaceLocation.Z);

	// Prefer the exact boss-death location. Only fall back to the boss-room anchor when that
	// position cannot be grounded (for example if the boss dies above a hole or invalid surface).
	bool bHasGroundedSpawn = false;
	auto TryGroundStageGateLocation = [World](const FVector& DesiredLocation, FVector& OutGroundLocation) -> bool
	{
		FHitResult Hit;
		const FVector Start = DesiredLocation + FVector(0.f, 0.f, 3000.f);
		const FVector End = DesiredLocation - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic) ||
			World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
		{
			OutGroundLocation = Hit.ImpactPoint;
			return true;
		}
		return false;
	};

	if (!IsUsingTowerMainMapLayout())
	{
		bHasGroundedSpawn = TryGroundStageGateLocation(SpawnLoc, SpawnLoc);
	}
	else
	{
		SpawnLoc = (RequestedFloorNumber != INDEX_NONE) ? RequestedLocation : FallbackSpawnLoc;
	}

	if (!bHasGroundedSpawn && !IsUsingTowerMainMapLayout())
	{
		if (TryGroundStageGateLocation(FallbackSpawnLoc, SpawnLoc))
		{
			bHasGroundedSpawn = true;
			if (!FallbackSpawnLoc.Equals(RequestedLocation, 1.0f))
			{
				SpawnAnchorSource =
					FallbackSpawnLoc.Equals(MainMapBossSpawnSurfaceLocation, 1.0f) ? TEXT("BossSpawnSurface") :
					(FallbackSpawnLoc.Equals(MainMapBossAreaCenterSurfaceLocation, 1.0f) ? TEXT("BossAreaCenterSurface") : TEXT("TowerFallback"));
			}
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66StageGate* StageGate = World->SpawnActor<AT66StageGate>(AT66StageGate::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (StageGate)
	{
		const FVector SpawnedActorInitialLoc = StageGate->GetActorLocation();
		if (IsUsingTowerMainMapLayout())
		{
			if (TargetTowerFloorNumber != INDEX_NONE)
			{
				T66TrySnapActorToTowerFloor(World, StageGate, CachedTowerMainMapLayout, TargetTowerFloorNumber, SpawnLoc);
				T66AssignTowerFloorTag(StageGate, TargetTowerFloorNumber);
			}
		}

		const FVector SpawnedActorFinalLoc = StageGate->GetActorLocation();
		const FVector GateMeshLoc = StageGate->GateMesh ? StageGate->GateMesh->GetComponentLocation() : FVector::ZeroVector;
		UE_LOG(LogT66GameMode, Warning, TEXT("Spawned Stage Gate desired=(%.0f, %.0f, %.0f) initial=(%.0f, %.0f, %.0f) final=(%.0f, %.0f, %.0f) mesh=(%.0f, %.0f, %.0f)%s"),
			SpawnLoc.X,
			SpawnLoc.Y,
			SpawnLoc.Z,
			SpawnedActorInitialLoc.X,
			SpawnedActorInitialLoc.Y,
			SpawnedActorInitialLoc.Z,
			SpawnedActorFinalLoc.X,
			SpawnedActorFinalLoc.Y,
			SpawnedActorFinalLoc.Z,
			GateMeshLoc.X,
			GateMeshLoc.Y,
			GateMeshLoc.Z,
			bHasGroundedSpawn ? TEXT("") : TEXT(" [location not grounded]"));
	}

	// Tower boss floors keep a second normal Stage Gate as a visibility fallback.
	if (IsUsingTowerMainMapLayout())
	{
		const FVector VisibleFallbackLoc = SpawnLoc + FVector(220.f, 0.f, 0.f);
		AT66StageGate* VisibleExitGate = World->SpawnActor<AT66StageGate>(AT66StageGate::StaticClass(), VisibleFallbackLoc, FRotator::ZeroRotator, SpawnParams);
		if (VisibleExitGate)
		{
			const FVector SpawnedActorInitialLoc = VisibleExitGate->GetActorLocation();
			if (TargetTowerFloorNumber != INDEX_NONE)
			{
				T66TrySnapActorToTowerFloor(World, VisibleExitGate, CachedTowerMainMapLayout, TargetTowerFloorNumber, VisibleFallbackLoc);
				T66AssignTowerFloorTag(VisibleExitGate, TargetTowerFloorNumber);
			}

			const FVector SpawnedActorFinalLoc = VisibleExitGate->GetActorLocation();
			const FVector GateMeshLoc = VisibleExitGate->GateMesh ? VisibleExitGate->GateMesh->GetComponentLocation() : FVector::ZeroVector;
			const float PlayerDistance2D = !PlayerLocation.IsZero() ? FVector::Dist2D(PlayerLocation, SpawnedActorFinalLoc) : -1.0f;
			const float PlayerDeltaZ = !PlayerLocation.IsZero() ? (SpawnedActorFinalLoc.Z - PlayerLocation.Z) : 0.0f;
			UE_LOG(LogT66GameMode, Warning, TEXT("Spawned visible tower Stage Gate fallback desired=(%.0f, %.0f, %.0f) initial=(%.0f, %.0f, %.0f) final=(%.0f, %.0f, %.0f) mesh=(%.0f, %.0f, %.0f)%s"),
				VisibleFallbackLoc.X,
				VisibleFallbackLoc.Y,
				VisibleFallbackLoc.Z,
				SpawnedActorInitialLoc.X,
				SpawnedActorInitialLoc.Y,
				SpawnedActorInitialLoc.Z,
				SpawnedActorFinalLoc.X,
				SpawnedActorFinalLoc.Y,
				SpawnedActorFinalLoc.Z,
				GateMeshLoc.X,
				GateMeshLoc.Y,
				GateMeshLoc.Z,
				bHasGroundedSpawn ? TEXT("") : TEXT(" [location not grounded]"));
			UE_LOG(LogT66GameMode, Warning, TEXT("Visible Tower Exit Gate relation: playerDistance2D=%.0f playerDeltaZ=%.0f gateFloor=%d"), PlayerDistance2D, PlayerDeltaZ, TargetTowerFloorNumber);
		}
	}
}

void AT66GameMode::SpawnBossForCurrentStage()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const UT66PlayerExperienceSubSystem* PlayerExperience = GI ? GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	const UT66DifficultyTuningSubsystem* DifficultyTuning = GI ? GI->GetSubsystem<UT66DifficultyTuningSubsystem>() : nullptr;
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

	int32 SelectedDifficultyEndStage = INDEX_NONE;
	if (DifficultyTuning)
	{
		SelectedDifficultyEndStage = DifficultyTuning->GetDifficultyEndStage(T66GI->SelectedDifficulty);
	}

	TArray<FName> FinalFloorOwedBossIDs;
	if (StageNum == SelectedDifficultyEndStage)
	{
		for (const FName OwedBossID : RunState->GetOwedBossIDs())
		{
			if (!OwedBossID.IsNone())
			{
				FinalFloorOwedBossIDs.Add(OwedBossID);
			}
		}
	}

	TArray<FName> EncounterBossIDs;
	if (!StageData.BossEncounterID.IsNone())
	{
		TArray<FT66BossEncounterMemberData> EncounterMembers;
		T66GI->GetBossEncounterMemberData(StageData.BossEncounterID, EncounterMembers);
		for (const FT66BossEncounterMemberData& Member : EncounterMembers)
		{
			if (!Member.BossID.IsNone())
			{
				EncounterBossIDs.Add(Member.BossID);
			}
		}
	}
	if (EncounterBossIDs.Num() <= 0 && !StageData.BossID.IsNone())
	{
		EncounterBossIDs.Add(StageData.BossID);
	}
	if (EncounterBossIDs.Num() <= 0)
	{
		EncounterBossIDs.Add(FName(*FString::Printf(TEXT("FallbackStageBoss_%02d"), StageNum)));
	}
	StageData.BossID = EncounterBossIDs[0];

	const int32 StageEncounterBossCount = FMath::Max(1, EncounterBossIDs.Num());
	const int32 FinalFloorBossCount = StageEncounterBossCount + FinalFloorOwedBossIDs.Num();

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
	T66BuildFallbackBossData(StageNum, StageData.BossID, BossData);

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
	const FVector StageBossSpawnLocation = FinalFloorBossCount > 1
		? T66ComputeBossClusterLocation(StageData.BossSpawnLocation, 0, FinalFloorBossCount)
		: StageData.BossSpawnLocation;
	AActor* Spawned = World->SpawnActor<AActor>(BossClass, StageBossSpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (AT66BossBase* Boss = Cast<AT66BossBase>(Spawned))
	{
		Boss->InitializeBoss(BossData);
		if (IsUsingTowerMainMapLayout())
		{
			if (!T66TrySnapActorToTowerFloor(World, Boss, CachedTowerMainMapLayout, CachedTowerMainMapLayout.BossFloorNumber, StageBossSpawnLocation))
			{
				T66TrySnapActorToTowerFloor(World, Boss, CachedTowerMainMapLayout, CachedTowerMainMapLayout.BossFloorNumber, StageData.BossSpawnLocation);
			}
			T66AssignTowerFloorTag(Boss, CachedTowerMainMapLayout.BossFloorNumber);
		}
		else
		{
			TrySnapActorToTerrainAtLocation(Boss, StageBossSpawnLocation);
		}
		StageBoss = Boss;
		const int32 BossScoreBudget = PlayerExperience
			? PlayerExperience->ResolveBossScore(T66GI->SelectedDifficulty, Boss->GetPointValue(), RunState->GetDifficultyScalar())
			: FMath::Max(0, FMath::RoundToInt(static_cast<float>(Boss->GetPointValue()) * RunState->GetDifficultyScalar()));
		RunState->RegisterSpawnedBossScoreBudget(BossScoreBudget, StageNum, Boss->BossID);
		UE_LOG(LogT66GameMode, Log, TEXT("Spawned boss for Stage %d (BossID=%s)"), StageNum, *BossData.BossID.ToString());

		// If the boss class is a soft reference and isn't loaded yet, load asynchronously and replace the dormant boss.
		if (bWantsSpecificBossClass && !bBossClassLoaded)
		{
			const FSoftObjectPath ClassPath = BossData.BossClass.ToSoftObjectPath();
			const TWeakObjectPtr<AT66BossBase> WeakExistingBoss(Boss);
			const FBossData BossDataCopy = BossData;
			const FVector BossFloorFallbackLocation = StageData.BossSpawnLocation;

			TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
				ClassPath,
				FStreamableDelegate::CreateWeakLambda(this, [this, WeakExistingBoss, BossDataCopy, BossFloorFallbackLocation]()
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
						if (IsUsingTowerMainMapLayout())
						{
							if (!T66TrySnapActorToTowerFloor(World2, NewBoss, CachedTowerMainMapLayout, CachedTowerMainMapLayout.BossFloorNumber, Loc))
							{
								T66TrySnapActorToTowerFloor(World2, NewBoss, CachedTowerMainMapLayout, CachedTowerMainMapLayout.BossFloorNumber, BossFloorFallbackLocation);
							}
							T66AssignTowerFloorTag(NewBoss, CachedTowerMainMapLayout.BossFloorNumber);
						}
						else
						{
							TrySnapActorToTerrainAtLocation(NewBoss, Loc);
						}
						StageBoss = NewBoss;
					}
				}));
			if (Handle.IsValid())
			{
				ActiveAsyncLoadHandles.Add(Handle);
			}
		}
	}

	for (int32 EncounterBossIndex = 1; EncounterBossIndex < EncounterBossIDs.Num(); ++EncounterBossIndex)
	{
		const FName EncounterBossID = EncounterBossIDs[EncounterBossIndex];
		FBossData EncounterBossData;
		T66BuildFallbackBossData(StageNum, EncounterBossID, EncounterBossData);
		if (FBossData FromBossTable; T66GI->GetBossData(EncounterBossID, FromBossTable))
		{
			EncounterBossData = FromBossTable;
		}

		const FVector EncounterBossSpawnLocation = T66ComputeBossClusterLocation(StageData.BossSpawnLocation, EncounterBossIndex, FinalFloorBossCount);
		FActorSpawnParameters EncounterSpawnParams;
		EncounterSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* EncounterSpawnedActor = World->SpawnActor<AActor>(T66LoadBossClassSync(EncounterBossData), EncounterBossSpawnLocation, FRotator::ZeroRotator, EncounterSpawnParams);
		if (AT66BossBase* EncounterBoss = Cast<AT66BossBase>(EncounterSpawnedActor))
		{
			EncounterBoss->InitializeBoss(EncounterBossData);
			if (IsUsingTowerMainMapLayout())
			{
				if (!T66TrySnapActorToTowerFloor(World, EncounterBoss, CachedTowerMainMapLayout, CachedTowerMainMapLayout.BossFloorNumber, EncounterBossSpawnLocation))
				{
					T66TrySnapActorToTowerFloor(World, EncounterBoss, CachedTowerMainMapLayout, CachedTowerMainMapLayout.BossFloorNumber, StageData.BossSpawnLocation);
				}
				T66AssignTowerFloorTag(EncounterBoss, CachedTowerMainMapLayout.BossFloorNumber);
			}
			else
			{
				TrySnapActorToTerrainAtLocation(EncounterBoss, EncounterBossSpawnLocation);
			}
			const int32 BossScoreBudget = PlayerExperience
				? PlayerExperience->ResolveBossScore(T66GI->SelectedDifficulty, EncounterBoss->GetPointValue(), RunState->GetDifficultyScalar())
				: FMath::Max(0, FMath::RoundToInt(static_cast<float>(EncounterBoss->GetPointValue()) * RunState->GetDifficultyScalar()));
			RunState->RegisterSpawnedBossScoreBudget(BossScoreBudget, StageNum, EncounterBoss->BossID);

			UE_LOG(LogT66GameMode, Log, TEXT("Spawned encounter boss member for Stage %d (BossID=%s EncounterID=%s)"),
				StageNum, *EncounterBossData.BossID.ToString(), *StageData.BossEncounterID.ToString());
		}
	}

	for (int32 BossIndex = 0; BossIndex < FinalFloorOwedBossIDs.Num(); ++BossIndex)
	{
		const FName OwedBossID = FinalFloorOwedBossIDs[BossIndex];
		FBossData OwedBossData;
		T66BuildFallbackBossData(T66ResolveFallbackBossStageNum(OwedBossID, StageNum), OwedBossID, OwedBossData);
		if (FBossData FromBossTable; T66GI->GetBossData(OwedBossID, FromBossTable))
		{
			OwedBossData = FromBossTable;
		}

		const FVector OwedBossSpawnLocation = T66ComputeBossClusterLocation(StageData.BossSpawnLocation, StageEncounterBossCount + BossIndex, FinalFloorBossCount);
		FActorSpawnParameters OwedSpawnParams;
		OwedSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* OwedSpawnedActor = World->SpawnActor<AActor>(T66LoadBossClassSync(OwedBossData), OwedBossSpawnLocation, FRotator::ZeroRotator, OwedSpawnParams);
		if (AT66BossBase* OwedBoss = Cast<AT66BossBase>(OwedSpawnedActor))
		{
			OwedBoss->InitializeBoss(OwedBossData);
			if (IsUsingTowerMainMapLayout())
			{
				if (!T66TrySnapActorToTowerFloor(World, OwedBoss, CachedTowerMainMapLayout, CachedTowerMainMapLayout.BossFloorNumber, OwedBossSpawnLocation))
				{
					T66TrySnapActorToTowerFloor(World, OwedBoss, CachedTowerMainMapLayout, CachedTowerMainMapLayout.BossFloorNumber, StageData.BossSpawnLocation);
				}
				T66AssignTowerFloorTag(OwedBoss, CachedTowerMainMapLayout.BossFloorNumber);
			}
			else
			{
				TrySnapActorToTerrainAtLocation(OwedBoss, OwedBossSpawnLocation);
			}
			const int32 BossScoreBudget = PlayerExperience
				? PlayerExperience->ResolveBossScore(T66GI->SelectedDifficulty, OwedBoss->GetPointValue(), RunState->GetDifficultyScalar())
				: FMath::Max(0, FMath::RoundToInt(static_cast<float>(OwedBoss->GetPointValue()) * RunState->GetDifficultyScalar()));
			RunState->RegisterSpawnedBossScoreBudget(BossScoreBudget, StageNum, OwedBoss->BossID);

			UE_LOG(LogT66GameMode, Log, TEXT("Spawned owed boss on final boss floor (BossID=%s)"), *OwedBossData.BossID.ToString());
		}
	}
}
