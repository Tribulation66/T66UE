// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameModePrivate.h"

using namespace T66GameModePrivate;

bool AT66GameMode::IsBossRushFinaleStage() const
{
	const UT66GameInstance* T66GI = GetT66GameInstance();
	const UGameInstance* GI = GetGameInstance();
	const UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState || T66GI->SelectedDifficulty == ET66Difficulty::Impossible)
	{
		return false;
	}

	return RunState->GetCurrentStage() == T66GetDifficultyEndStage(T66GI->SelectedDifficulty);
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
	RunState->SetPendingDifficultyClearSummary(true);
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
			T66PC->ClientShowDifficultyClearSummary();
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
		DestroyBossBeacon();
	}
	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const UT66PlayerExperienceSubSystem* PlayerExperience = GI ? GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
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

		const int32 BossXPAmount = RunState->GetHeroXPToNextLevel();
		if (BossXPAmount > 0)
		{
			RunState->AddHeroXP(BossXPAmount);
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
		if (PlayerExperience)
		{
			const int32 DifficultyEndStage = PlayerExperience->GetDifficultyEndStage(SelectedDifficulty);
			bCompletedSelectedDifficulty = (RunState->GetCurrentStage() >= DifficultyEndStage);
		}
	}

	// First-time stage clear unlock => stage finales now award an extra companion so the
	// reduced 4/4/4/4/3 progression still unlocks the full 24-companion roster.
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
	if (bCompletedSelectedDifficulty && SelectedDifficulty == ET66Difficulty::Impossible)
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

		ET66Difficulty NextDifficulty = SelectedDifficulty;
		const bool bHasNextDifficulty = T66TryGetNextDifficulty(SelectedDifficulty, NextDifficulty);
		if (RunState)
		{
			RunState->SetPendingDifficultyClearSummary(bHasNextDifficulty);
		}

		if (bHasNextDifficulty)
		{
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
		}

		bool bOpenedSummary = false;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(It->Get()))
			{
				if (bHasNextDifficulty)
				{
					T66PC->ClientShowDifficultyClearSummary();
				}
				else
				{
					T66PC->ClientShowVictoryRunSummary();
				}
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
		else if (!MainMapBossBeaconSurfaceLocation.IsNearlyZero())
		{
			FallbackSpawnLoc = MainMapBossBeaconSurfaceLocation;
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
		TEXT("SpawnStageGateAtLocation: request=(%.0f, %.0f, %.0f) requestFloor=%d anchorSource=%s resolvedAnchor=(%.0f, %.0f, %.0f) anchorFloor=%d player=(%.0f, %.0f, %.0f) playerFloor=%d bossSpawnSurface=(%.0f, %.0f, %.0f) bossAreaCenter=(%.0f, %.0f, %.0f) bossBeacon=(%.0f, %.0f, %.0f)"),
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
		MainMapBossAreaCenterSurfaceLocation.Z,
		MainMapBossBeaconSurfaceLocation.X,
		MainMapBossBeaconSurfaceLocation.Y,
		MainMapBossBeaconSurfaceLocation.Z);

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
					(FallbackSpawnLoc.Equals(MainMapBossAreaCenterSurfaceLocation, 1.0f) ? TEXT("BossAreaCenterSurface") :
						(FallbackSpawnLoc.Equals(MainMapBossBeaconSurfaceLocation, 1.0f) ? TEXT("BossBeaconSurface") : TEXT("TowerFallback")));
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

	// Tower boss floors currently need a brute-force visible fallback.
	// Spawn the known-good catch-up gate actor in stage-advance mode at the same location so the
	// player always gets an obvious interactable even if the legacy StageGate visual fails.
	if (IsUsingTowerMainMapLayout())
	{
		FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLoc);
		AT66StageCatchUpGate* VisibleExitGate = World->SpawnActorDeferred<AT66StageCatchUpGate>(AT66StageCatchUpGate::StaticClass(), SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (VisibleExitGate)
		{
			VisibleExitGate->bActsAsStageAdvanceGate = true;
			UGameplayStatics::FinishSpawningActor(VisibleExitGate, SpawnTransform);

			const FVector SpawnedActorInitialLoc = VisibleExitGate->GetActorLocation();
			if (TargetTowerFloorNumber != INDEX_NONE)
			{
				T66TrySnapActorToTowerFloor(World, VisibleExitGate, CachedTowerMainMapLayout, TargetTowerFloorNumber, SpawnLoc);
				T66AssignTowerFloorTag(VisibleExitGate, TargetTowerFloorNumber);
			}

			const FVector SpawnedActorFinalLoc = VisibleExitGate->GetActorLocation();
			const FVector GateMeshLoc = VisibleExitGate->GateMesh ? VisibleExitGate->GateMesh->GetComponentLocation() : FVector::ZeroVector;
			const float PlayerDistance2D = !PlayerLocation.IsZero() ? FVector::Dist2D(PlayerLocation, SpawnedActorFinalLoc) : -1.0f;
			const float PlayerDeltaZ = !PlayerLocation.IsZero() ? (SpawnedActorFinalLoc.Z - PlayerLocation.Z) : 0.0f;
			UE_LOG(LogT66GameMode, Warning, TEXT("Spawned Visible Tower Exit Gate desired=(%.0f, %.0f, %.0f) initial=(%.0f, %.0f, %.0f) final=(%.0f, %.0f, %.0f) mesh=(%.0f, %.0f, %.0f)%s"),
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
	if (PlayerExperience)
	{
		SelectedDifficultyEndStage = PlayerExperience->GetDifficultyEndStage(T66GI->SelectedDifficulty);
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

	const int32 FinalFloorBossCount = 1 + FinalFloorOwedBossIDs.Num();

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
		SpawnBossBeaconIfNeeded();
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
						SpawnBossBeaconIfNeeded();
					}
				}));
			if (Handle.IsValid())
			{
				ActiveAsyncLoadHandles.Add(Handle);
			}
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

		const FVector OwedBossSpawnLocation = T66ComputeBossClusterLocation(StageData.BossSpawnLocation, BossIndex + 1, FinalFloorBossCount);
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

bool AT66GameMode::TryComputeBossBeaconBase(FVector& OutBeaconBase) const
{
	if (IsLabLevel() || IsUsingTowerMainMapLayout() || !StageBoss.IsValid())
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
	if (IsLabLevel() || IsUsingTowerMainMapLayout() || !StageBoss.IsValid())
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
	if (IsLabLevel() || IsUsingTowerMainMapLayout())
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
