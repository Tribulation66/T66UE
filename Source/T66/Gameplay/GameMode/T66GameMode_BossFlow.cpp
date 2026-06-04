// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameModePrivate.h"
#include "Core/T66SkinSubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

using namespace T66GameModePrivate;

namespace
{
	static constexpr float T66BossRewardCompanionOffsetX = -760.0f;
	static constexpr float T66BossRewardPetOffsetY = -900.0f;
	static constexpr float T66BossRewardIdolOffsetY = 900.0f;
	static constexpr float T66BossRewardGateOffsetX = 1250.0f;
	static constexpr float T66BossRewardCompanionSpacingY = 420.0f;

	void T66RetireSourceActorForEndgameTransform(AActor* Actor)
	{
		if (!Actor)
		{
			return;
		}

		Actor->SetActorHiddenInGame(true);
		Actor->SetActorEnableCollision(false);
		Actor->SetActorTickEnabled(false);
		TArray<UActorComponent*> Components;
		Actor->GetComponents(Components);
		for (UActorComponent* Component : Components)
		{
			if (Component)
			{
				Component->SetComponentTickEnabled(false);
			}
		}
	}
}

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
		Saint->SetEndgameSaint(true);
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

	RunState->SetBossInactive();
	RunState->ApplySaintBlessingStatBoosts();
	RunState->SetFinalSurvivalEnemyScalar(T66PandemoniumEnemyScalar);
	RunState->SetStageTimerActive(true);

	ClearMiasma();
	if (FinalDifficultySaintActor.IsValid())
	{
		FinalDifficultySaintActor->Destroy();
		FinalDifficultySaintActor.Reset();
	}
	if (FinalDifficultyTotemActor.IsValid())
	{
		FinalDifficultyTotemActor->Destroy();
		FinalDifficultyTotemActor.Reset();
	}

	if (UWorld* World = GetWorld())
	{
		if (AT66EnemyDirector* ExistingEnemyDirector = EnsureEnemyDirector(World))
		{
			ExistingEnemyDirector->SetPandemoniumMode(
				true,
				T66PandemoniumRuntimeSpawnIntervalSeconds,
				T66PandemoniumRuntimeEnemiesPerWave,
				T66PandemoniumRuntimeMaxAliveEnemies,
				T66PandemoniumRuntimeMaxSpawnsPerBatch);
			ExistingEnemyDirector->SetSpawningPaused(false);
		}
	}

	UpdateFinalDifficultySurvivalScaling(true);

	UE_LOG(LogT66GameMode, Log, TEXT("[T66Endgame] PandemoniumStarted Anchor=(%.0f,%.0f,%.0f) Scalar=%.2f SpawnInterval=%.2f EnemiesPerWave=%d MaxAlive=%d"),
		BossDeathLocation.X,
		BossDeathLocation.Y,
		BossDeathLocation.Z,
		T66PandemoniumEnemyScalar,
		T66PandemoniumRuntimeSpawnIntervalSeconds,
		T66PandemoniumRuntimeEnemiesPerWave,
		T66PandemoniumRuntimeMaxAliveEnemies);
}

void AT66GameMode::StopFinalDifficultySurvival()
{
	if (!bFinalDifficultySurvivalActive)
	{
		return;
	}

	bFinalDifficultySurvivalActive = false;
	FinalDifficultySurvivalElapsedSeconds = 0.f;
	LastAppliedFinalDifficultyEnemyScalar = 1.f;

	UGameInstance* GI = GetGameInstance();
	if (UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		RunState->ClearSaintBlessingStatBoosts();
		RunState->SetFinalSurvivalEnemyScalar(1.f);
		RunState->SetStageTimerActive(false);
	}

	if (UWorld* World = GetWorld())
	{
		if (AT66EnemyDirector* ExistingEnemyDirector = FindOrCacheEnemyDirector(World))
		{
			ExistingEnemyDirector->SetPandemoniumMode(false, 0.30f, 18, 180, 6);
		}
	}
}

void AT66GameMode::UpdateFinalDifficultySurvivalScaling(const bool bForce)
{
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	const float NewScalar = T66PandemoniumEnemyScalar;
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

	FinalDifficultySurvivalElapsedSeconds += FMath::Max(0.f, DeltaTime);
	UpdateFinalDifficultySurvivalScaling();
}

void AT66GameMode::HandleSaintEndgameChoice(AT66PlayerController* PlayerController, const int32 ChoiceIndex, AT66SaintNPC* Saint)
{
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();
	const FVector ChoiceLocation = PlayerController && PlayerController->GetPawn()
		? PlayerController->GetPawn()->GetActorLocation()
		: (Saint ? Saint->GetActorLocation() : FVector::ZeroVector);

	auto DestroyChosenSaint = [&]()
	{
		if (Saint)
		{
			FinalDifficultySaintActor.Reset();
			Saint->Destroy();
		}
	};

	switch (ChoiceIndex)
	{
	case 0:
		UE_LOG(LogT66GameMode, Log, TEXT("[T66Endgame] SaintChoice=Leave"));
		DestroyChosenSaint();
		CompleteDifficultyAndOpenRunSummary();
		return;
	case 1:
		UE_LOG(LogT66GameMode, Log, TEXT("[T66Endgame] SaintChoice=BlessingPandemonium"));
		DestroyChosenSaint();
		BeginFinalDifficultySurvival(ChoiceLocation);
		return;
	case 2:
		if (!RunState || !RunState->HasKromerItem())
		{
			UE_LOG(LogT66GameMode, Warning, TEXT("[T66Endgame] SaintChoice=GiveKromer Failed=MissingKromer"));
			if (PlayerController && Saint)
			{
				PlayerController->OpenWorldDialogueSaintMissingKromer(Saint);
			}
			return;
		}

		if (!RunState->ConsumeKromerItem())
		{
			UE_LOG(LogT66GameMode, Warning, TEXT("[T66Endgame] SaintChoice=GiveKromer Failed=ConsumeKromer"));
			if (PlayerController && Saint)
			{
				PlayerController->OpenWorldDialogueSaintMissingKromer(Saint);
			}
			return;
		}

		DestroyChosenSaint();
		if (UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr)
		{
			Achievements->UnlockEnterTheKingdomAchievement();
		}
		if (UT66SkinSubsystem* Skins = GI ? GI->GetSubsystem<UT66SkinSubsystem>() : nullptr)
		{
			const FName HeroID = T66GI ? T66GI->SelectedHeroID : NAME_None;
			if (!HeroID.IsNone())
			{
				Skins->GrantHeroSkin(HeroID, UT66SkinSubsystem::SaintSkinID, true);
			}
		}
		UE_LOG(LogT66GameMode, Log, TEXT("[T66Endgame] SaintChoice=GiveKromer SecretEnding=1"));
		CompleteDifficultyAndOpenRunSummary();
		return;
	default:
		UE_LOG(LogT66GameMode, Warning, TEXT("[T66Endgame] SaintChoice invalid index=%d"), ChoiceIndex);
		return;
	}
}

void AT66GameMode::SpawnKromerLootBag(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector SpawnLocation = Location + FVector(0.f, 0.f, 120.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66LootBagPickup* LootBag = World->SpawnActor<AT66LootBagPickup>(AT66LootBagPickup::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		LootBag->SetItemID(UT66RunStateSubsystem::KromerItemID);
		LootBag->SetLootRarity(ET66Rarity::White);
		if (IsUsingTowerMainMapLayout())
		{
			T66TrySnapActorToTowerFloor(World, LootBag, CachedTowerMainMapLayout, CachedTowerMainMapLayout.BossFloorNumber, SpawnLocation);
			T66AssignTowerFloorTag(LootBag, CachedTowerMainMapLayout.BossFloorNumber);
		}
		else
		{
			TrySnapActorToTerrainAtLocation(LootBag, SpawnLocation);
		}

		UE_LOG(LogT66GameMode, Log, TEXT("[T66Endgame] KromerLootBagSpawned ItemID=%s Location=(%.0f,%.0f,%.0f)"),
			*UT66RunStateSubsystem::KromerItemID.ToString(),
			Location.X,
			Location.Y,
			Location.Z);
	}
}

AT66BossBase* AT66GameMode::SpawnEndgameBossAt(
	const FName BossID,
	const FVector& Location,
	const bool bForceAwaken,
	const bool bZeroDamageUnkillable,
	const FName ZeroDamageReason,
	const float HealthScalar,
	const float DamageScalar,
	const float ScaleScalar,
	const bool bTrackAsStageBoss)
{
	UWorld* World = GetWorld();
	UT66GameInstance* T66GI = GetT66GameInstance();
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const UT66PlayerExperienceSubSystem* PlayerExperience = GI ? GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	if (!World || !T66GI || BossID.IsNone())
	{
		return nullptr;
	}

	const int32 StageNum = RunState ? RunState->GetCurrentStage() : T66MaxGlobalStage;
	FBossData BossData;
	T66BuildFallbackBossData(T66ResolveFallbackBossStageNum(BossID, StageNum), BossID, BossData);
	if (FBossData FromBossTable; T66GI->GetBossData(BossID, FromBossTable))
	{
		BossData = FromBossTable;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector SpawnLocation = Location + FVector(0.f, 0.f, 120.f);
	AActor* SpawnedActor = World->SpawnActor<AActor>(T66LoadBossClassSync(BossData), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	AT66BossBase* Boss = Cast<AT66BossBase>(SpawnedActor);
	if (!Boss)
	{
		return nullptr;
	}

	Boss->InitializeBoss(BossData);
	if (IsUsingTowerMainMapLayout())
	{
		if (!T66TrySnapActorToTowerFloor(World, Boss, CachedTowerMainMapLayout, CachedTowerMainMapLayout.BossFloorNumber, SpawnLocation))
		{
			T66TrySnapActorToTowerFloor(World, Boss, CachedTowerMainMapLayout, CachedTowerMainMapLayout.BossFloorNumber, Location);
		}
		T66AssignTowerFloorTag(Boss, CachedTowerMainMapLayout.BossFloorNumber);
	}
	else
	{
		TrySnapActorToTerrainAtLocation(Boss, SpawnLocation);
	}

	Boss->ApplyEndgameBossMultipliers(HealthScalar, DamageScalar, ScaleScalar);
	Boss->SetZeroDamageUnkillable(bZeroDamageUnkillable, ZeroDamageReason);
	if (bForceAwaken)
	{
		Boss->ForceAwaken();
		Boss->RefreshRunStateBossState();
	}
	if (bTrackAsStageBoss)
	{
		StageBoss = Boss;
	}

	if (RunState)
	{
		const int32 BossScoreBudget = PlayerExperience
			? PlayerExperience->ResolveBossScore(T66GI->SelectedDifficulty, Boss->GetPointValue(), RunState->GetDifficultyScalar())
			: FMath::Max(0, FMath::RoundToInt(static_cast<float>(Boss->GetPointValue()) * RunState->GetDifficultyScalar()));
		RunState->RegisterSpawnedBossScoreBudget(BossScoreBudget, StageNum, Boss->BossID);
	}

	UE_LOG(LogT66GameMode, Log, TEXT("[T66Endgame] EndgameBossSpawned BossID=%s ZeroDamage=%d HealthScalar=%.2f DamageScalar=%.2f"),
		*BossID.ToString(),
		bZeroDamageUnkillable ? 1 : 0,
		HealthScalar,
		DamageScalar);
	return Boss;
}

void AT66GameMode::SpawnFinalBossSecondPhase(const FVector& Location)
{
	bFinalBossSecondPhaseActive = true;
	bFinalBossUnwinnableEndingActive = false;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->SetStageTimerActive(true);
		}
	}

	if (!SpawnEndgameBossAt(FName(TEXT("Hell_GreatDragon_Phase2")), Location, true, false, NAME_None, 1.f, 1.f, 1.f, true))
	{
		UE_LOG(LogT66GameMode, Warning, TEXT("[T66Endgame] Failed to spawn Great Dragon phase 2; spawning Saint fallback."));
		bFinalBossSecondPhaseActive = false;
		SpawnFinalDifficultySaint(Location);
		return;
	}

	UE_LOG(LogT66GameMode, Log, TEXT("[T66Endgame] FinalBossMatrix=NoCompanionNoPet Phase2Started=1"));
}

FName AT66GameMode::ResolveBossIDForActivePet() const
{
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!T66GI)
	{
		return NAME_None;
	}

	const FName SelectedPetID = T66GI->SelectedPetID;
	if (SelectedPetID.IsNone())
	{
		return NAME_None;
	}

	FPetData PetData;
	if (T66GI->GetPetData(SelectedPetID, PetData))
	{
		return PetData.SourceBossID.IsNone() ? (PetData.PetID.IsNone() ? SelectedPetID : PetData.PetID) : PetData.SourceBossID;
	}

	return SelectedPetID;
}

void AT66GameMode::SpawnFinalCompanionTransformBosses(const FVector& Location)
{
	UT66GameInstance* T66GI = GetT66GameInstance();
	int32 SpawnedCount = 0;

	TArray<TPair<TWeakObjectPtr<AController>, TWeakObjectPtr<AT66CompanionBase>>> CompanionPairs;
	for (const TPair<TWeakObjectPtr<AController>, TWeakObjectPtr<AT66CompanionBase>>& Pair : PlayerCompanions)
	{
		CompanionPairs.Add(Pair);
	}

	for (const TPair<TWeakObjectPtr<AController>, TWeakObjectPtr<AT66CompanionBase>>& Pair : CompanionPairs)
	{
		AT66CompanionBase* Companion = Pair.Value.Get();
		if (!Companion)
		{
			continue;
		}

		const FVector TransformLocation = Companion->GetActorLocation();
		T66RetireSourceActorForEndgameTransform(Companion);
		PlayerCompanions.Remove(Pair.Key);
		if (SpawnEndgameBossAt(FName(TEXT("Lilith")), TransformLocation, true, true, FName(TEXT("FinalCompanionTransform")), T66EndgameTransformedBossHealthScalar, T66EndgameTransformedBossDamageScalar, T66EndgameTransformedBossScaleScalar))
		{
			++SpawnedCount;
		}
	}

	if (SpawnedCount <= 0 && T66GI && !T66GI->SelectedCompanionID.IsNone())
	{
		if (SpawnEndgameBossAt(FName(TEXT("Lilith")), Location + FVector(0.f, -360.f, 0.f), true, true, FName(TEXT("FinalCompanionTransform")), T66EndgameTransformedBossHealthScalar, T66EndgameTransformedBossDamageScalar, T66EndgameTransformedBossScaleScalar))
		{
			++SpawnedCount;
		}
	}

	TArray<TPair<TWeakObjectPtr<AController>, TWeakObjectPtr<AT66PetActor>>> PetPairs;
	for (const TPair<TWeakObjectPtr<AController>, TWeakObjectPtr<AT66PetActor>>& Pair : PlayerPets)
	{
		PetPairs.Add(Pair);
	}

	bool bSpawnedPetBoss = false;
	for (const TPair<TWeakObjectPtr<AController>, TWeakObjectPtr<AT66PetActor>>& Pair : PetPairs)
	{
		AT66PetActor* Pet = Pair.Value.Get();
		if (!Pet)
		{
			continue;
		}

		const FName PetBossID = Pet->PetData.SourceBossID.IsNone()
			? (Pet->PetID.IsNone() ? ResolveBossIDForActivePet() : Pet->PetID)
			: Pet->PetData.SourceBossID;
		const FVector TransformLocation = Pet->GetActorLocation();
		T66RetireSourceActorForEndgameTransform(Pet);
		PlayerPets.Remove(Pair.Key);
		if (!PetBossID.IsNone()
			&& SpawnEndgameBossAt(PetBossID, TransformLocation, true, true, FName(TEXT("FinalPetTransform")), T66EndgameTransformedBossHealthScalar, T66EndgameTransformedBossDamageScalar, T66EndgameTransformedBossScaleScalar))
		{
			++SpawnedCount;
			bSpawnedPetBoss = true;
		}
	}

	if (!bSpawnedPetBoss)
	{
		const FName PetBossID = ResolveBossIDForActivePet();
		if (!PetBossID.IsNone()
			&& SpawnEndgameBossAt(PetBossID, Location + FVector(0.f, 360.f, 0.f), true, true, FName(TEXT("FinalPetTransform")), T66EndgameTransformedBossHealthScalar, T66EndgameTransformedBossDamageScalar, T66EndgameTransformedBossScaleScalar))
		{
			++SpawnedCount;
		}
	}

	bFinalBossUnwinnableEndingActive = SpawnedCount > 0;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->SetStageTimerActive(bFinalBossUnwinnableEndingActive);
		}
	}

	UE_LOG(LogT66GameMode, Log, TEXT("[T66Endgame] FinalBossMatrix=CompanionOrPet TransformedBosses=%d Unwinnable=%d"),
		SpawnedCount,
		bFinalBossUnwinnableEndingActive ? 1 : 0);
}

bool AT66GameMode::ShouldEndgameDeathOpenRunSummary() const
{
	return bFinalDifficultySurvivalActive || bFinalBossUnwinnableEndingActive;
}

void AT66GameMode::HandleEndgameDeathRunSummary(AT66PlayerController* PlayerController)
{
	StopFinalDifficultySurvival();
	bFinalBossUnwinnableEndingActive = false;
	bFinalBossSecondPhaseActive = false;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->SetBossInactive();
			RunState->ClearOwedBosses();
		}
	}

	if (PlayerController)
	{
		PlayerController->ShowVictoryRunSummary();
		UE_LOG(LogT66GameMode, Log, TEXT("[T66Endgame] DeathResolvedAsDifficultyComplete"));
	}
}

void AT66GameMode::CompleteDifficultyAndOpenRunSummary()
{
	StopFinalDifficultySurvival();
	bFinalBossUnwinnableEndingActive = false;
	bFinalBossSecondPhaseActive = false;

	UGameInstance* GI = GetGameInstance();
	if (UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		RunState->SetBossInactive();
		RunState->ClearOwedBosses();
		RunState->SetStageTimerActive(false);
	}

	UWorld* World = GetWorld();
	bool bOpenedSummary = false;
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(It->Get()))
			{
				T66PC->ClientShowVictoryRunSummary();
				bOpenedSummary = true;
			}
		}
	}

	if (!bOpenedSummary)
	{
		UE_LOG(LogT66GameMode, Warning, TEXT("Difficulty clear reached but no T66PlayerController was available to open Run Summary."));
	}
}

#if !UE_BUILD_SHIPPING
bool AT66GameMode::RunEndgameSaintSmoke(UWorld* ProofWorld, const FString& OutputPath)
{
	bool bAllPassed = true;
	TArray<FString> Checks;
	auto Escape = [](FString Value)
	{
		Value.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
		Value.ReplaceInline(TEXT("\""), TEXT("\\\""));
		Value.ReplaceInline(TEXT("\r"), TEXT("\\r"));
		Value.ReplaceInline(TEXT("\n"), TEXT("\\n"));
		return Value;
	};
	auto Check = [&Checks, &bAllPassed, &Escape](const TCHAR* Name, const bool bPassed, const FString& Detail)
	{
		bAllPassed = bAllPassed && bPassed;
		Checks.Add(FString::Printf(TEXT("    { \"name\": \"%s\", \"ok\": %s, \"detail\": \"%s\" }"), Name, bPassed ? TEXT("true") : TEXT("false"), *Escape(Detail)));
		UE_LOG(LogT66GameMode, Log, TEXT("[EndgameSaintSmoke] Check=%s Result=%s Detail=%s"), Name, bPassed ? TEXT("PASS") : TEXT("FAIL"), *Detail);
	};

	UGameInstance* GI = ProofWorld ? ProofWorld->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66SkinSubsystem* Skins = GI ? GI->GetSubsystem<UT66SkinSubsystem>() : nullptr;
	AT66PlayerController* PC = ProofWorld ? Cast<AT66PlayerController>(ProofWorld->GetFirstPlayerController()) : nullptr;

	Check(TEXT("Context available"), ProofWorld && T66GI && RunState && Achievements && Skins && PC, TEXT("Requires world, GI, RunState, achievements, skins, and player controller."));
	if (!ProofWorld || !T66GI || !RunState || !Achievements || !Skins || !PC)
	{
		return false;
	}

	auto DestroyProofActors = [&]()
	{
		for (TActorIterator<AT66BossBase> It(ProofWorld); It; ++It) { It->Destroy(); }
		for (TActorIterator<AT66CompanionBase> It(ProofWorld); It; ++It) { It->Destroy(); }
		for (TActorIterator<AT66PetActor> It(ProofWorld); It; ++It) { It->Destroy(); }
		for (TActorIterator<AT66RecruitableCompanion> It(ProofWorld); It; ++It) { It->Destroy(); }
		for (TActorIterator<AT66PetCaptureInteractable> It(ProofWorld); It; ++It) { It->Destroy(); }
		for (TActorIterator<AT66LootBagPickup> It(ProofWorld); It; ++It) { It->Destroy(); }
		for (TActorIterator<AT66SaintNPC> It(ProofWorld); It; ++It) { It->Destroy(); }
		for (TActorIterator<AT66IdolAltar> It(ProofWorld); It; ++It) { It->Destroy(); }
		for (TActorIterator<AT66StageGate> It(ProofWorld); It; ++It) { It->Destroy(); }
		ClearCagedStageCompanions(true);
		PlayerCompanions.Reset();
		PlayerPets.Reset();
		StageBoss.Reset();
		FinalDifficultySaintActor.Reset();
		FinalDifficultyTotemActor.Reset();
		bFinalBossSecondPhaseActive = false;
		bFinalBossUnwinnableEndingActive = false;
		StopFinalDifficultySurvival();
	};

	auto CountActorsOfClass = [&](UClass* ActorClass)
	{
		int32 Count = 0;
		for (TActorIterator<AActor> It(ProofWorld, ActorClass); It; ++It) { ++Count; }
		return Count;
	};

	auto CountBossesByID = [&](const FName BossID)
	{
		int32 Count = 0;
		for (TActorIterator<AT66BossBase> It(ProofWorld); It; ++It)
		{
			if (It->BossID == BossID)
			{
				++Count;
			}
		}
		return Count;
	};

	auto FindBossByID = [&](const FName BossID) -> AT66BossBase*
	{
		for (TActorIterator<AT66BossBase> It(ProofWorld); It; ++It)
		{
			if (It->BossID == BossID)
			{
				return *It;
			}
		}
		return nullptr;
	};

	auto CheckZeroDamage = [&](AT66BossBase* Boss, const TCHAR* Name)
	{
		const int32 HPBefore = Boss ? Boss->CurrentHP : INDEX_NONE;
		const bool bDied = Boss ? Boss->TakeDamageFromHeroHit(Boss->MaxHP + 100000, FName(TEXT("EndgameSaintSmoke")), FName(TEXT("EndgameSaintSmoke"))) : true;
		const int32 HPAfter = Boss ? Boss->CurrentHP : INDEX_NONE;
		Check(Name, Boss && Boss->IsZeroDamageUnkillable() && !bDied && HPBefore == HPAfter,
			Boss ? FString::Printf(TEXT("BossID=%s HP=%d->%d ZeroDamage=%d Died=%d"), *Boss->BossID.ToString(), HPBefore, HPAfter, Boss->IsZeroDamageUnkillable() ? 1 : 0, bDied ? 1 : 0) : TEXT("Boss missing."));
	};

	FName SaintProofHeroID = FName(TEXT("Hero_1"));
	for (const FName CandidateHeroID : T66GI->GetPlayableHeroIDs())
	{
		if (!Skins->IsHeroSkinOwned(CandidateHeroID, UT66SkinSubsystem::SaintSkinID))
		{
			SaintProofHeroID = CandidateHeroID;
			break;
		}
	}
	const bool bProofHeroHadSaintSkinBefore = Skins->IsHeroSkinOwned(SaintProofHeroID, UT66SkinSubsystem::SaintSkinID);

	T66GI->SelectedHeroID = SaintProofHeroID;
	T66GI->SelectedDifficulty = ET66Difficulty::Impossible;
	T66GI->SelectedCompanionID = NAME_None;
	T66GI->SelectedPetID = NAME_None;
	RunState->ResetForNewRun();
	RunState->SetCurrentStage(20);
	RunState->ClearInventory();
	Achievements->ResetCurrentRunAchievementUnlockSummary();
	DestroyProofActors();

	FItemData KromerData;
	FBossData Phase2Data;
	FBossData LilithData;
	Check(TEXT("Kromer data exists"), T66GI->GetItemData(UT66RunStateSubsystem::KromerItemID, KromerData), TEXT("Item_Kromer loaded."));
	Check(TEXT("Phase2 boss data exists"), T66GI->GetBossData(FName(TEXT("Hell_GreatDragon_Phase2")), Phase2Data), TEXT("Hell_GreatDragon_Phase2 loaded."));
	Check(TEXT("Lilith boss data exists"), T66GI->GetBossData(FName(TEXT("Lilith")), LilithData), TEXT("Lilith loaded."));

	AT66SaintNPC* RandomDialogueSaint = ProofWorld->SpawnActor<AT66SaintNPC>(AT66SaintNPC::StaticClass(), FVector(100.f, 0.f, 120.f), FRotator::ZeroRotator);
	if (RandomDialogueSaint)
	{
		PC->OpenWorldDialogueSaint(RandomDialogueSaint);
	}
	const FString ExpectedInitialKromerText = bProofHeroHadSaintSkinBefore ? TEXT("Hello fellow saint") : TEXT("Give him a Kromer");
	Check(TEXT("RandomSaintKromerOnlyDialogue"), RandomDialogueSaint && !RandomDialogueSaint->IsEndgameSaint() && PC->GetWorldDialogueNumOptionsForAutomation() == 1 && PC->GetWorldDialogueOptionTextForAutomation(0).ToString() == ExpectedInitialKromerText,
		FString::Printf(TEXT("Endgame=%d Options=%d Option0=%s Expected=%s Hero=%s HadSaintSkin=%d"), RandomDialogueSaint && RandomDialogueSaint->IsEndgameSaint() ? 1 : 0, PC->GetWorldDialogueNumOptionsForAutomation(), *PC->GetWorldDialogueOptionTextForAutomation(0).ToString(), *ExpectedInitialKromerText, *SaintProofHeroID.ToString(), bProofHeroHadSaintSkinBefore ? 1 : 0));
	DestroyProofActors();

	AT66SaintNPC* EndgameDialogueSaint = ProofWorld->SpawnActor<AT66SaintNPC>(AT66SaintNPC::StaticClass(), FVector(110.f, 0.f, 120.f), FRotator::ZeroRotator);
	if (EndgameDialogueSaint)
	{
		EndgameDialogueSaint->SetEndgameSaint(true);
		PC->OpenWorldDialogueSaint(EndgameDialogueSaint);
	}
	Check(TEXT("EndgameSaintDialogueThreeOptions"), EndgameDialogueSaint && EndgameDialogueSaint->IsEndgameSaint() && PC->GetWorldDialogueNumOptionsForAutomation() == 3 && PC->GetWorldDialogueOptionTextForAutomation(2).ToString() == ExpectedInitialKromerText,
		FString::Printf(TEXT("Endgame=%d Options=%d KromerText=%s Expected=%s Hero=%s HadSaintSkin=%d"), EndgameDialogueSaint && EndgameDialogueSaint->IsEndgameSaint() ? 1 : 0, PC->GetWorldDialogueNumOptionsForAutomation(), *PC->GetWorldDialogueOptionTextForAutomation(2).ToString(), *ExpectedInitialKromerText, *SaintProofHeroID.ToString(), bProofHeroHadSaintSkinBefore ? 1 : 0));
	DestroyProofActors();

	AT66SaintNPC* NoKromerSaint = ProofWorld->SpawnActor<AT66SaintNPC>(AT66SaintNPC::StaticClass(), FVector(120.f, 0.f, 120.f), FRotator::ZeroRotator);
	const bool bNoKromerSkinBefore = Skins->IsHeroSkinOwned(SaintProofHeroID, UT66SkinSubsystem::SaintSkinID);
	HandleSaintEndgameChoice(PC, 2, NoKromerSaint);
	Check(TEXT("SaintKromerOptionWithoutItemShowsResponse"), IsValid(NoKromerSaint) && !RunState->HasRunEnded() && !RunState->HasKromerItem() && Skins->IsHeroSkinOwned(SaintProofHeroID, UT66SkinSubsystem::SaintSkinID) == bNoKromerSkinBefore && PC->GetWorldDialogueNumOptionsForAutomation() == 1 && PC->GetWorldDialogueOptionTextForAutomation(0).ToString() == TEXT("You don't have any Kromer"),
		FString::Printf(TEXT("SaintValid=%d Ended=%d HasKromer=%d SkinBefore=%d SkinAfter=%d Options=%d Option0=%s"), IsValid(NoKromerSaint) ? 1 : 0, RunState->HasRunEnded() ? 1 : 0, RunState->HasKromerItem() ? 1 : 0, bNoKromerSkinBefore ? 1 : 0, Skins->IsHeroSkinOwned(SaintProofHeroID, UT66SkinSubsystem::SaintSkinID) ? 1 : 0, PC->GetWorldDialogueNumOptionsForAutomation(), *PC->GetWorldDialogueOptionTextForAutomation(0).ToString()));
	DestroyProofActors();

	AT66SaintNPC* LeaveSaint = ProofWorld->SpawnActor<AT66SaintNPC>(AT66SaintNPC::StaticClass(), FVector(140.f, 0.f, 120.f), FRotator::ZeroRotator);
	HandleSaintEndgameChoice(PC, 0, LeaveSaint);
	Check(TEXT("SaintLeaveOpensVictorySummary"), RunState->HasRunEnded() && RunState->DidRunEndInVictory(),
		FString::Printf(TEXT("Ended=%d Victory=%d"), RunState->HasRunEnded() ? 1 : 0, RunState->DidRunEndInVictory() ? 1 : 0));
	PC->SetPause(false);
	RunState->ResetForNewRun();
	RunState->SetCurrentStage(20);
	DestroyProofActors();

	AT66SaintNPC* BlessingSaint = ProofWorld->SpawnActor<AT66SaintNPC>(AT66SaintNPC::StaticClass(), FVector(180.f, 0.f, 120.f), FRotator::ZeroRotator);
	HandleSaintEndgameChoice(PC, 1, BlessingSaint);
	const bool bPrimaryBoosts = RunState->GetDamageStat() >= T66SaintBlessingStatBoostPoints && RunState->GetAttackSpeedStat() >= T66SaintBlessingStatBoostPoints && RunState->GetScaleStat() >= T66SaintBlessingStatBoostPoints && RunState->GetAccuracyStat() >= T66SaintBlessingStatBoostPoints && RunState->GetArmorStat() >= T66SaintBlessingStatBoostPoints && RunState->GetEvasionStat() >= T66SaintBlessingStatBoostPoints && RunState->GetLuckStat() >= T66SaintBlessingStatBoostPoints && RunState->GetSpeedStat() >= T66SaintBlessingStatBoostPoints;
	const float ExpectedElementPower = 1.f + (static_cast<float>(T66SaintBlessingStatBoostPoints) * T66SaintBlessingElementPowerPerPoint);
	const bool bElementBoosts = RunState->GetSecondaryStatValue(ET66SecondaryStatType::FirePower) >= ExpectedElementPower && RunState->GetSecondaryStatValue(ET66SecondaryStatType::IcePower) >= ExpectedElementPower && RunState->GetSecondaryStatValue(ET66SecondaryStatType::ElectricityPower) >= ExpectedElementPower && RunState->GetSecondaryStatValue(ET66SecondaryStatType::NaturePower) >= ExpectedElementPower;
	Check(TEXT("SaintBlessingStartsPandemonium"), bFinalDifficultySurvivalActive && ShouldEndgameDeathOpenRunSummary() && RunState->IsSaintBlessingActive() && bPrimaryBoosts && bElementBoosts && FMath::IsNearlyEqual(RunState->GetFinalSurvivalEnemyScalar(), T66PandemoniumEnemyScalar),
		FString::Printf(TEXT("Active=%d SummaryOnDeath=%d Blessing=%d Scalar=%.2f Primary=%d Element=%d Fire=%.2f Expected=%.2f"), bFinalDifficultySurvivalActive ? 1 : 0, ShouldEndgameDeathOpenRunSummary() ? 1 : 0, RunState->IsSaintBlessingActive() ? 1 : 0, RunState->GetFinalSurvivalEnemyScalar(), bPrimaryBoosts ? 1 : 0, bElementBoosts ? 1 : 0, RunState->GetSecondaryStatValue(ET66SecondaryStatType::FirePower), ExpectedElementPower));
	const int32 ScoreBeforePandemoniumProof = RunState->GetCurrentScore();
	const int32 EnemyScoreBeforePandemoniumProof = RunState->GetScoreBudgetContext().EnemyScoreAwarded;
	RunState->RegisterSpawnedEnemyScoreBudget(T66PandemoniumSmokeEnemyScoreProofPoints, RunState->GetCurrentStage());
	RunState->AddEnemyKillScore(T66PandemoniumSmokeEnemyScoreProofPoints);
	Check(TEXT("PandemoniumUsesNormalEnemyScorePath"), RunState->GetCurrentScore() == ScoreBeforePandemoniumProof + T66PandemoniumSmokeEnemyScoreProofPoints && RunState->GetScoreBudgetContext().EnemyScoreAwarded == EnemyScoreBeforePandemoniumProof + T66PandemoniumSmokeEnemyScoreProofPoints,
		FString::Printf(TEXT("Score=%d->%d EnemyScore=%d->%d Points=%d"), ScoreBeforePandemoniumProof, RunState->GetCurrentScore(), EnemyScoreBeforePandemoniumProof, RunState->GetScoreBudgetContext().EnemyScoreAwarded, T66PandemoniumSmokeEnemyScoreProofPoints));
	HandleEndgameDeathRunSummary(PC);
	Check(TEXT("PandemoniumDeathCountsAsDifficultyComplete"), RunState->HasRunEnded() && RunState->DidRunEndInVictory(),
		FString::Printf(TEXT("Ended=%d Victory=%d"), RunState->HasRunEnded() ? 1 : 0, RunState->DidRunEndInVictory() ? 1 : 0));
	PC->SetPause(false);
	RunState->ResetForNewRun();
	RunState->SetCurrentStage(20);
	DestroyProofActors();

	SpawnFinalBossSecondPhase(FVector(300.f, 0.f, 120.f));
	AT66BossBase* Phase2Boss = FindBossByID(FName(TEXT("Hell_GreatDragon_Phase2")));
	Check(TEXT("FinalNoCompanionNoPetStartsSecondPhase"), bFinalBossSecondPhaseActive && Phase2Boss && Phase2Boss->IsAwakened() && Phase2Boss->CurrentHP == Phase2Boss->MaxHP,
		Phase2Boss ? FString::Printf(TEXT("Phase2Active=%d HP=%d/%d"), bFinalBossSecondPhaseActive ? 1 : 0, Phase2Boss->CurrentHP, Phase2Boss->MaxHP) : TEXT("Phase2 missing."));
	if (Phase2Boss)
	{
		HandleBossDefeated(Phase2Boss);
	}
	Check(TEXT("FinalPhase2DropsKromerAndSaint"), CountActorsOfClass(AT66LootBagPickup::StaticClass()) > 0 && CountActorsOfClass(AT66SaintNPC::StaticClass()) > 0,
		FString::Printf(TEXT("LootBags=%d Saints=%d"), CountActorsOfClass(AT66LootBagPickup::StaticClass()), CountActorsOfClass(AT66SaintNPC::StaticClass())));
	DestroyProofActors();

	T66GI->SelectedCompanionID = FName(TEXT("Companion_01"));
	T66GI->SelectedPetID = NAME_None;
	const FVector GFTransformLocation(400.f, -60.f, 120.f);
	AT66CompanionBase* ProofCompanion = ProofWorld->SpawnActor<AT66CompanionBase>(AT66CompanionBase::StaticClass(), GFTransformLocation, FRotator::ZeroRotator);
	if (ProofCompanion)
	{
		ProofCompanion->CompanionID = FName(TEXT("Companion_01"));
		PlayerCompanions.Add(PC, ProofCompanion);
	}
	SpawnFinalCompanionTransformBosses(FVector(400.f, 0.f, 120.f));
	AT66BossBase* GFOnlyLilith = FindBossByID(FName(TEXT("Lilith")));
	Check(TEXT("FinalGFOnlyTransformsToLilith"), bFinalBossUnwinnableEndingActive && CountBossesByID(FName(TEXT("Lilith"))) == 1,
		FString::Printf(TEXT("LilithCount=%d Unwinnable=%d"), CountBossesByID(FName(TEXT("Lilith"))), bFinalBossUnwinnableEndingActive ? 1 : 0));
	Check(TEXT("FinalGFOnlyRetiresSourceInPlace"), ProofCompanion && ProofCompanion->IsHidden() && !ProofCompanion->IsActorTickEnabled() && GFOnlyLilith && FVector::DistSquared2D(GFOnlyLilith->GetActorLocation(), GFTransformLocation) <= 25.f,
		FString::Printf(TEXT("SourceHidden=%d SourceTick=%d BossLoc=(%.1f,%.1f,%.1f) SourceLoc=(%.1f,%.1f,%.1f)"), ProofCompanion && ProofCompanion->IsHidden() ? 1 : 0, ProofCompanion && ProofCompanion->IsActorTickEnabled() ? 1 : 0, GFOnlyLilith ? GFOnlyLilith->GetActorLocation().X : 0.f, GFOnlyLilith ? GFOnlyLilith->GetActorLocation().Y : 0.f, GFOnlyLilith ? GFOnlyLilith->GetActorLocation().Z : 0.f, GFTransformLocation.X, GFTransformLocation.Y, GFTransformLocation.Z));
	CheckZeroDamage(GFOnlyLilith, TEXT("FinalGFOnlyLilithZeroDamage"));
	HandleEndgameDeathRunSummary(PC);
	Check(TEXT("FinalGFOnlyDeathOpensVictorySummary"), RunState->HasRunEnded() && RunState->DidRunEndInVictory(),
		FString::Printf(TEXT("Ended=%d Victory=%d"), RunState->HasRunEnded() ? 1 : 0, RunState->DidRunEndInVictory() ? 1 : 0));
	PC->SetPause(false);
	RunState->ResetForNewRun();
	RunState->SetCurrentStage(20);
	DestroyProofActors();

	T66GI->SelectedCompanionID = NAME_None;
	T66GI->SelectedPetID = FName(TEXT("Dungeon_SewerSlimeKing"));
	const FVector PetTransformLocation(500.f, 60.f, 120.f);
	AT66PetActor* ProofPet = ProofWorld->SpawnActor<AT66PetActor>(AT66PetActor::StaticClass(), PetTransformLocation, FRotator::ZeroRotator);
	if (ProofPet)
	{
		ProofPet->PetID = FName(TEXT("Dungeon_SewerSlimeKing"));
		ProofPet->PetData.PetID = FName(TEXT("Dungeon_SewerSlimeKing"));
		ProofPet->PetData.SourceBossID = FName(TEXT("Dungeon_SewerSlimeKing"));
		PlayerPets.Add(PC, ProofPet);
	}
	SpawnFinalCompanionTransformBosses(FVector(500.f, 0.f, 120.f));
	AT66BossBase* PetOnlyBoss = FindBossByID(FName(TEXT("Dungeon_SewerSlimeKing")));
	Check(TEXT("FinalPetOnlyTransformsOriginalBoss"), bFinalBossUnwinnableEndingActive && CountBossesByID(FName(TEXT("Dungeon_SewerSlimeKing"))) == 1,
		FString::Printf(TEXT("PetBossCount=%d Unwinnable=%d"), CountBossesByID(FName(TEXT("Dungeon_SewerSlimeKing"))), bFinalBossUnwinnableEndingActive ? 1 : 0));
	Check(TEXT("FinalPetOnlyRetiresSourceInPlace"), ProofPet && ProofPet->IsHidden() && !ProofPet->IsActorTickEnabled() && PetOnlyBoss && FVector::DistSquared2D(PetOnlyBoss->GetActorLocation(), PetTransformLocation) <= 25.f,
		FString::Printf(TEXT("SourceHidden=%d SourceTick=%d BossLoc=(%.1f,%.1f,%.1f) SourceLoc=(%.1f,%.1f,%.1f)"), ProofPet && ProofPet->IsHidden() ? 1 : 0, ProofPet && ProofPet->IsActorTickEnabled() ? 1 : 0, PetOnlyBoss ? PetOnlyBoss->GetActorLocation().X : 0.f, PetOnlyBoss ? PetOnlyBoss->GetActorLocation().Y : 0.f, PetOnlyBoss ? PetOnlyBoss->GetActorLocation().Z : 0.f, PetTransformLocation.X, PetTransformLocation.Y, PetTransformLocation.Z));
	CheckZeroDamage(PetOnlyBoss, TEXT("FinalPetOnlyZeroDamage"));
	HandleEndgameDeathRunSummary(PC);
	Check(TEXT("FinalPetOnlyDeathOpensVictorySummary"), RunState->HasRunEnded() && RunState->DidRunEndInVictory(),
		FString::Printf(TEXT("Ended=%d Victory=%d"), RunState->HasRunEnded() ? 1 : 0, RunState->DidRunEndInVictory() ? 1 : 0));
	PC->SetPause(false);
	RunState->ResetForNewRun();
	RunState->SetCurrentStage(20);
	DestroyProofActors();

	T66GI->SelectedCompanionID = FName(TEXT("Companion_01"));
	T66GI->SelectedPetID = FName(TEXT("Dungeon_SewerSlimeKing"));
	SpawnFinalCompanionTransformBosses(FVector(600.f, 0.f, 120.f));
	Check(TEXT("FinalBothTransformsBoth"), bFinalBossUnwinnableEndingActive && CountBossesByID(FName(TEXT("Lilith"))) == 1 && CountBossesByID(FName(TEXT("Dungeon_SewerSlimeKing"))) == 1,
		FString::Printf(TEXT("Lilith=%d PetBoss=%d Unwinnable=%d"), CountBossesByID(FName(TEXT("Lilith"))), CountBossesByID(FName(TEXT("Dungeon_SewerSlimeKing"))), bFinalBossUnwinnableEndingActive ? 1 : 0));
	CheckZeroDamage(FindBossByID(FName(TEXT("Lilith"))), TEXT("FinalBothLilithZeroDamage"));
	CheckZeroDamage(FindBossByID(FName(TEXT("Dungeon_SewerSlimeKing"))), TEXT("FinalBothPetZeroDamage"));
	HandleEndgameDeathRunSummary(PC);
	Check(TEXT("FinalBothDeathOpensVictorySummary"), RunState->HasRunEnded() && RunState->DidRunEndInVictory(),
		FString::Printf(TEXT("Ended=%d Victory=%d"), RunState->HasRunEnded() ? 1 : 0, RunState->DidRunEndInVictory() ? 1 : 0));
	PC->SetPause(false);
	RunState->ResetForNewRun();
	RunState->SetCurrentStage(20);
	DestroyProofActors();

	T66GI->SelectedHeroID = SaintProofHeroID;
	T66GI->SelectedCompanionID = NAME_None;
	T66GI->SelectedPetID = NAME_None;
	RunState->ClearInventory();
	RunState->AddItemSlot(FT66InventorySlot(UT66RunStateSubsystem::KromerItemID, ET66ItemRarity::White, 1));
	const bool bKromerBefore = RunState->HasKromerItem();
	AT66SaintNPC* KromerSaint = ProofWorld->SpawnActor<AT66SaintNPC>(AT66SaintNPC::StaticClass(), FVector(700.f, 0.f, 120.f), FRotator::ZeroRotator);
	HandleSaintEndgameChoice(PC, 2, KromerSaint);
	const bool bKromerConsumed = bKromerBefore && !RunState->HasKromerItem();
	const bool bAchievementUnlocked = Achievements->IsAchievementClaimed(UT66AchievementsSubsystem::EnterTheKingdomAchievementID);
	const bool bSkinUnlocked = Skins->IsHeroSkinOwned(SaintProofHeroID, UT66SkinSubsystem::SaintSkinID);
	Check(TEXT("KromerSecretEndingUnlocksAchievementAndSkin"), bKromerConsumed && bAchievementUnlocked && bSkinUnlocked,
		FString::Printf(TEXT("KromerConsumed=%d AchievementClaimed=%d Skin=%d Hero=%s HadSaintSkinBefore=%d"), bKromerConsumed ? 1 : 0, bAchievementUnlocked ? 1 : 0, bSkinUnlocked ? 1 : 0, *SaintProofHeroID.ToString(), bProofHeroHadSaintSkinBefore ? 1 : 0));
	Check(TEXT("SaintSkinChangesKromerOptionText"), PC->GetSaintKromerDialogueOptionTextForAutomation().ToString() == TEXT("Hello fellow saint"),
		FString::Printf(TEXT("Variant=%s"), *PC->GetSaintKromerDialogueOptionTextForAutomation().ToString()));
	PC->SetPause(false);
	RunState->ResetForNewRun();
	RunState->SetCurrentStage(4);
	T66GI->SelectedDifficulty = ET66Difficulty::Easy;
	DestroyProofActors();

	AT66BossBase* DifficultyBoss = ProofWorld->SpawnActor<AT66BossBase>(AT66BossBase::StaticClass(), FVector(800.f, 0.f, 120.f), FRotator::ZeroRotator);
	if (DifficultyBoss)
	{
		DifficultyBoss->BossID = FName(TEXT("Dungeon_BaelFallenChad"));
		DifficultyBoss->MaxHP = 1000;
		DifficultyBoss->CurrentHP = 0;
		HandleBossDefeated(DifficultyBoss);
	}
	Check(TEXT("DifficultyBossDeathSpawnsSaintOnly"), CountActorsOfClass(AT66SaintNPC::StaticClass()) == 1 && CountActorsOfClass(AT66RecruitableCompanion::StaticClass()) == 0 && CountActorsOfClass(AT66PetCaptureInteractable::StaticClass()) == 0 && CountActorsOfClass(AT66IdolAltar::StaticClass()) == 0 && CountActorsOfClass(AT66StageGate::StaticClass()) == 0 && !RunState->HasRunEnded(),
		FString::Printf(TEXT("Saints=%d GFRewards=%d PetCaptures=%d Altars=%d Gates=%d Ended=%d"), CountActorsOfClass(AT66SaintNPC::StaticClass()), CountActorsOfClass(AT66RecruitableCompanion::StaticClass()), CountActorsOfClass(AT66PetCaptureInteractable::StaticClass()), CountActorsOfClass(AT66IdolAltar::StaticClass()), CountActorsOfClass(AT66StageGate::StaticClass()), RunState->HasRunEnded() ? 1 : 0));

	const FString Json = FString::Printf(TEXT("{\n  \"ok\": %s,\n  \"checks\": [\n%s\n  ]\n}\n"), bAllPassed ? TEXT("true") : TEXT("false"), *FString::Join(Checks, TEXT(",\n")));
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutputPath), true);
	const bool bSaved = FFileHelper::SaveStringToFile(Json, *OutputPath);
	UE_LOG(LogT66GameMode, Log, TEXT("[EndgameSaintSmokeSummary] Terminal=1 Pass=%d Saved=%d Output=%s"), bAllPassed ? 1 : 0, bSaved ? 1 : 0, *OutputPath);
	return bAllPassed && bSaved;
}
#endif

void AT66GameMode::ClearCagedStageCompanions(const bool bDestroyActors)
{
	if (bDestroyActors)
	{
		for (const TWeakObjectPtr<AT66RecruitableCompanion>& WeakCompanion : CagedStageCompanions)
		{
			if (AT66RecruitableCompanion* Companion = WeakCompanion.Get())
			{
				Companion->Destroy();
			}
		}
	}

	CagedStageCompanions.Reset();
}

int32 AT66GameMode::SpawnCagedCompanionsForCurrentStage(const FVector& AnchorLocation)
{
	UWorld* World = GetWorld();
	UT66GameInstance* T66GI = GetT66GameInstance();
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!World || !T66GI || !RunState)
	{
		return 0;
	}

	CagedStageCompanions.RemoveAll([](const TWeakObjectPtr<AT66RecruitableCompanion>& WeakCompanion)
	{
		return !WeakCompanion.IsValid();
	});
	if (CagedStageCompanions.Num() > 0)
	{
		return CagedStageCompanions.Num();
	}

	const int32 StageNum = RunState->GetCurrentStage();
	TArray<FName> CompanionUnlockIDs;
	T66_AppendCompanionUnlockIDsForStage(StageNum, CompanionUnlockIDs);
	if (CompanionUnlockIDs.Num() <= 0)
	{
		return 0;
	}

	int32 SpawnedCount = 0;
	const int32 UnlockCount = CompanionUnlockIDs.Num();
	for (int32 CompanionIndex = 0; CompanionIndex < CompanionUnlockIDs.Num(); ++CompanionIndex)
	{
		const FName CompanionToUnlock = CompanionUnlockIDs[CompanionIndex];
		if (CompanionToUnlock.IsNone())
		{
			continue;
		}

		FCompanionData Data;
		if (!T66GI->GetCompanionData(CompanionToUnlock, Data))
		{
			UE_LOG(LogT66GameMode, Warning, TEXT("[CompanionCage] Missing companion data for Stage=%d CompanionID=%s; cage reward not spawned."),
				StageNum,
				*CompanionToUnlock.ToString());
			continue;
		}

		const float HorizontalOffset = (static_cast<float>(CompanionIndex) - ((static_cast<float>(UnlockCount) - 1.f) * 0.5f)) * 180.f;
		const FVector RecruitSpawnLocation = AnchorLocation + FVector(-420.f, HorizontalOffset, 0.f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66RecruitableCompanion* Recruit = World->SpawnActor<AT66RecruitableCompanion>(
			AT66RecruitableCompanion::StaticClass(),
			RecruitSpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams);
		if (!Recruit)
		{
			UE_LOG(LogT66GameMode, Warning, TEXT("[CompanionCage] Failed to spawn caged companion for Stage=%d CompanionID=%s."),
				StageNum,
				*CompanionToUnlock.ToString());
			continue;
		}

		Recruit->InitializeRecruit(Data);
		Recruit->SetCagedForBossReward();
		if (IsUsingTowerMainMapLayout())
		{
			const int32 TargetFloorNumber = CachedTowerMainMapLayout.BossFloorNumber;
			auto TryPlaceRecruitOnBossFloor = [&](const FVector& DesiredLocation) -> bool
			{
				if (!T66TrySnapActorToTowerFloor(World, Recruit, CachedTowerMainMapLayout, TargetFloorNumber, DesiredLocation))
				{
					return false;
				}

				return true;
			};

			bool bSnappedToBossFloor = TryPlaceRecruitOnBossFloor(RecruitSpawnLocation);
			bool bUsedBossFloorFallback = false;
			if (!bSnappedToBossFloor)
			{
				FVector FallbackAnchor = ResolveTowerBossWaitingLocation();
				if (FallbackAnchor.IsNearlyZero())
				{
					if (const T66TowerMapTerrain::FFloor* BossFloor = T66FindTowerFloorByNumber(CachedTowerMainMapLayout, TargetFloorNumber))
					{
						FallbackAnchor = BossFloor->Center;
						FallbackAnchor.Z = BossFloor->SurfaceZ;
					}
				}

				if (!FallbackAnchor.IsNearlyZero())
				{
					bUsedBossFloorFallback = true;
					const FVector FallbackRecruitLocation = FallbackAnchor + FVector(-420.f, HorizontalOffset, 0.f);
					bSnappedToBossFloor = TryPlaceRecruitOnBossFloor(FallbackRecruitLocation)
						|| TryPlaceRecruitOnBossFloor(FallbackAnchor);
				}
			}

			if (bSnappedToBossFloor)
			{
				T66AssignTowerFloorTag(Recruit, TargetFloorNumber);
			}
			else
			{
				UE_LOG(
					LogT66GameMode,
					Warning,
					TEXT("[CompanionCage] Destroying caged companion because it could not be placed on boss floor. Stage=%d CompanionID=%s Requested=%s UsedFallback=%d"),
					StageNum,
					*CompanionToUnlock.ToString(),
					*RecruitSpawnLocation.ToCompactString(),
					bUsedBossFloorFallback ? 1 : 0);
				Recruit->Destroy();
				continue;
			}
		}
		else
		{
			TrySnapActorToTerrainAtLocation(Recruit, RecruitSpawnLocation);
		}

		CagedStageCompanions.Add(Recruit);
		++SpawnedCount;
		UE_LOG(LogT66GameMode, Log, TEXT("[CompanionCage] SpawnedCaged Stage=%d CompanionID=%s Actor=%s Anchor=(%.0f, %.0f, %.0f)"),
			StageNum,
			*CompanionToUnlock.ToString(),
			*GetNameSafe(Recruit),
			AnchorLocation.X,
			AnchorLocation.Y,
			AnchorLocation.Z);
	}

	return SpawnedCount;
}

int32 AT66GameMode::FreeCagedCompanionsForBossClear(const FVector& FallbackLocation)
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const int32 StageNum = RunState ? RunState->GetCurrentStage() : INDEX_NONE;

	CagedStageCompanions.RemoveAll([](const TWeakObjectPtr<AT66RecruitableCompanion>& WeakCompanion)
	{
		return !WeakCompanion.IsValid();
	});
	if (CagedStageCompanions.Num() <= 0)
	{
		SpawnCagedCompanionsForCurrentStage(FallbackLocation);
		CagedStageCompanions.RemoveAll([](const TWeakObjectPtr<AT66RecruitableCompanion>& WeakCompanion)
		{
			return !WeakCompanion.IsValid();
		});
	}

	int32 FreedCount = 0;
	int32 TrackedRewardCount = 0;
	int32 RewardIndex = 0;
	int32 RewardCompanionCount = 0;
	for (const TWeakObjectPtr<AT66RecruitableCompanion>& WeakCompanion : CagedStageCompanions)
	{
		const AT66RecruitableCompanion* Companion = WeakCompanion.Get();
		if (Companion && Companion->IsBossCageUnlockReward())
		{
			++RewardCompanionCount;
		}
	}
	for (const TWeakObjectPtr<AT66RecruitableCompanion>& WeakCompanion : CagedStageCompanions)
	{
		AT66RecruitableCompanion* Companion = WeakCompanion.Get();
		if (!Companion || !Companion->IsBossCageUnlockReward())
		{
			continue;
		}

		++TrackedRewardCount;
		if (Companion->IsLockedInBossCage())
		{
			const float HorizontalOffset = (static_cast<float>(RewardIndex) - ((static_cast<float>(FMath::Max(1, RewardCompanionCount)) - 1.0f) * 0.5f)) * T66BossRewardCompanionSpacingY;
			const FVector DesiredLocation = FallbackLocation + FVector(0.0f, HorizontalOffset, 0.0f);
			if (IsUsingTowerMainMapLayout())
			{
				const int32 RequestedFloorNumber = GetTowerFloorIndexForLocation(FallbackLocation);
				const int32 TargetFloorNumber = RequestedFloorNumber != INDEX_NONE
					? RequestedFloorNumber
					: CachedTowerMainMapLayout.BossFloorNumber;
				if (TargetFloorNumber != INDEX_NONE)
				{
					if (T66TrySnapActorToTowerFloor(World, Companion, CachedTowerMainMapLayout, TargetFloorNumber, DesiredLocation))
					{
						T66AssignTowerFloorTag(Companion, TargetFloorNumber);
					}
					else
					{
						Companion->SetActorLocation(DesiredLocation);
					}
				}
			}
			else
			{
				Companion->SetActorLocation(DesiredLocation);
				TrySnapActorToTerrainAtLocation(Companion, DesiredLocation);
			}

			Companion->FreeFromBossCage();
			++FreedCount;
		}
		++RewardIndex;
	}

	if (TrackedRewardCount > 0)
	{
		UE_LOG(LogT66GameMode, Log, TEXT("[CompanionCage] BossClearFreed Stage=%d Freed=%d Tracked=%d DirectUnlock=0"),
			StageNum,
			FreedCount,
			TrackedRewardCount);
	}

	return FreedCount;
}

void AT66GameMode::HandleBossDefeated(AT66BossBase* Boss)
{
	UWorld* World = GetWorld();
	if (!World) return;
	const FVector Location = Boss ? Boss->GetActorLocation() : FVector::ZeroVector;
	if (IsUsingTowerMainMapLayout() && Boss)
	{
		const int32 BossTaggedFloorNumber = T66ReadTowerFloorTag(Boss);
		const int32 BossPhysicalFloorNumber = GetTowerFloorIndexForLocation(Location);
		if (BossTaggedFloorNumber == CachedTowerMainMapLayout.BossFloorNumber
			|| BossPhysicalFloorNumber == CachedTowerMainMapLayout.BossFloorNumber)
		{
			bTowerBossDefeated = true;
			bTowerBossEntryApplied = true;
			UE_LOG(
				LogT66GameMode,
				Log,
				TEXT("[MAP] Tower boss floor defeated; suppressing future boss-entry respawns. boss=%s taggedFloor=%d physicalFloor=%d bossFloor=%d"),
				*GetNameSafe(Boss),
				BossTaggedFloorNumber,
				BossPhysicalFloorNumber,
				CachedTowerMainMapLayout.BossFloorNumber);
		}
	}
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

	const UT66GameInstance* T66GI = GetT66GameInstance();
	const ET66Difficulty SelectedDifficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const bool bUsesFinalSequence = bCompletedSelectedDifficulty
		&& (DifficultyTuning ? DifficultyTuning->DoesDifficultyUseFinalSequence(SelectedDifficulty) : SelectedDifficulty == ET66Difficulty::Impossible);

	if (bFinalBossSecondPhaseActive)
	{
		bFinalBossSecondPhaseActive = false;
		if (RunState)
		{
			RunState->ClearOwedBosses();
		}
		ClearMiasma();
		SpawnKromerLootBag(Location);
		SpawnFinalDifficultySaint(Location);
		UE_LOG(LogT66GameMode, Log, TEXT("[T66Endgame] FinalBossPhase2Defeated KromerDropped=1 SaintSpawned=1"));
		return;
	}

	if (bCompletedSelectedDifficulty)
	{
		if (RunState)
		{
			RunState->ClearOwedBosses();
		}

		ClearMiasma();
		if (bUsesFinalSequence)
		{
			const bool bHasCompanion = T66GI && !T66GI->SelectedCompanionID.IsNone();
			const bool bHasPet = T66GI && !T66GI->SelectedPetID.IsNone();
			if (!bHasCompanion && !bHasPet)
			{
				SpawnFinalBossSecondPhase(Location);
				return;
			}

			SpawnFinalCompanionTransformBosses(Location);
			return;
		}

		SpawnFinalDifficultySaint(Location);
		UE_LOG(LogT66GameMode, Log, TEXT("[T66Endgame] DifficultyBossDefeated SaintOnly=1 Stage=%d"), RunState ? RunState->GetCurrentStage() : INDEX_NONE);
		return;
	}

	const FVector RewardAnchor = Location;
	const FVector CompanionRewardLocation = RewardAnchor + FVector(T66BossRewardCompanionOffsetX, 0.0f, 0.0f);
	const FVector PetRewardLocation = RewardAnchor + FVector(0.0f, T66BossRewardPetOffsetY, 0.0f);
	const FVector IdolRewardLocation = RewardAnchor + FVector(0.0f, T66BossRewardIdolOffsetY, 0.0f);
	const FVector GateRewardLocation = RewardAnchor + FVector(T66BossRewardGateOffsetX, 0.0f, 0.0f);

	// Boss clear frees the staged cage reward only on non-difficulty-ending stages; the companion is unlocked only by interacting with the freed recruit.
	FreeCagedCompanionsForBossClear(CompanionRewardLocation);

	// Normal stage: boss dead => miasma disappears and Stage Gate appears.
	ClearMiasma();
	TrySpawnPetCaptureForBoss(Boss, PetRewardLocation);
	SpawnIdolAltarAtLocation(IdolRewardLocation);
	SpawnStageGateAtLocation(GateRewardLocation);

	if (RunState)
	{
		const int32 ClearedStage = RunState->GetCurrentStage();
		UE_LOG(LogT66GameMode, Verbose, TEXT("Stage %d cleared; post-boss idol altar and stage gate spawned."), ClearedStage);
	}
}

bool AT66GameMode::TrySpawnPetCaptureForBoss(AT66BossBase* Boss, const FVector& Location)
{
	UWorld* World = GetWorld();
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!World || !T66GI || !Boss || Boss->BossID.IsNone())
	{
		const FString BossIDText = Boss ? Boss->BossID.ToString() : FString(TEXT("None"));
		UE_LOG(LogT66GameMode, Warning, TEXT("[Pets] Capture spawn skipped: invalid context world=%d gi=%d boss=%s."),
			World ? 1 : 0,
			T66GI ? 1 : 0,
			*BossIDText);
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	const UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const int32 StageNum = RunState ? RunState->GetCurrentStage() : INDEX_NONE;
	if (StageNum >= 17 || T66_IsDifficultyBossStage(StageNum))
	{
		UE_LOG(LogT66GameMode, Verbose, TEXT("[Pets] Stage %d is a no-pet reward stage; no boss pet capture spawned for %s."),
			StageNum,
			*Boss->BossID.ToString());
		return false;
	}

	const FName PetID = T66GI->ResolvePetIDForBossID(Boss->BossID);
	if (PetID.IsNone())
	{
		UE_LOG(LogT66GameMode, Warning, TEXT("[Pets] Boss %s has no resolvable pet ID; no capture interactable spawned."),
			*Boss->BossID.ToString());
		return false;
	}

	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	if (Achievements && Achievements->IsPetCaptured(PetID))
	{
		UE_LOG(LogT66GameMode, Verbose, TEXT("[Pets] Boss %s pet %s already captured; no capture interactable spawned."),
			*Boss->BossID.ToString(),
			*PetID.ToString());
		return false;
	}

	FPetData PetData;
	if (!T66GI->GetPetData(PetID, PetData))
	{
		UE_LOG(LogT66GameMode, Warning, TEXT("[Pets] Pet data missing for boss %s pet %s; no capture interactable spawned."),
			*Boss->BossID.ToString(),
			*PetID.ToString());
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FVector CaptureSpawnLocation = Location;
	if (AT66PetCaptureInteractable* Capture = World->SpawnActor<AT66PetCaptureInteractable>(
		AT66PetCaptureInteractable::StaticClass(),
		CaptureSpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams))
	{
		Capture->InitializePetCapture(PetData);
		if (IsUsingTowerMainMapLayout())
		{
			const int32 RequestedFloorNumber = GetTowerFloorIndexForLocation(Location);
			const int32 TargetFloorNumber = RequestedFloorNumber != INDEX_NONE
				? RequestedFloorNumber
				: CachedTowerMainMapLayout.BossFloorNumber;
			if (TargetFloorNumber != INDEX_NONE)
			{
				auto TryPlaceCaptureOnTargetFloor = [&](const FVector& DesiredLocation) -> bool
				{
					if (!T66TrySnapActorToTowerFloor(World, Capture, CachedTowerMainMapLayout, TargetFloorNumber, DesiredLocation))
					{
						return false;
					}

					return true;
				};

				bool bSnappedToTargetFloor = TryPlaceCaptureOnTargetFloor(CaptureSpawnLocation);
				bool bUsedBossFloorFallback = false;
				if (!bSnappedToTargetFloor)
				{
					FVector FallbackAnchor = ResolveTowerBossWaitingLocation();
					if (FallbackAnchor.IsNearlyZero())
					{
						if (const T66TowerMapTerrain::FFloor* BossFloor = T66FindTowerFloorByNumber(CachedTowerMainMapLayout, TargetFloorNumber))
						{
							FallbackAnchor = BossFloor->Center;
							FallbackAnchor.Z = BossFloor->SurfaceZ;
						}
					}

					if (!FallbackAnchor.IsNearlyZero())
					{
						bUsedBossFloorFallback = true;
						bSnappedToTargetFloor = TryPlaceCaptureOnTargetFloor(FallbackAnchor + FVector(0.f, T66BossRewardPetOffsetY, 0.f))
							|| TryPlaceCaptureOnTargetFloor(FallbackAnchor);
					}
				}

				if (bSnappedToTargetFloor)
				{
					T66AssignTowerFloorTag(Capture, TargetFloorNumber);
				}
				else
				{
					UE_LOG(
						LogT66GameMode,
						Warning,
						TEXT("[Pets] Destroying capture interactable because it could not be placed on floor %d. Boss=%s Pet=%s Requested=%s UsedFallback=%d"),
						TargetFloorNumber,
						*Boss->BossID.ToString(),
						*PetID.ToString(),
						*CaptureSpawnLocation.ToCompactString(),
						bUsedBossFloorFallback ? 1 : 0);
					Capture->Destroy();
					return false;
				}
			}
		}
		else
		{
			TrySnapActorToTerrainAtLocation(Capture, CaptureSpawnLocation);
		}
		UE_LOG(LogT66GameMode, Log, TEXT("[Pets] Spawned guaranteed capture interactable for boss %s pet %s at %s."),
			*Boss->BossID.ToString(),
			*PetID.ToString(),
			*Capture->GetActorLocation().ToCompactString());
		return true;
	}

	UE_LOG(LogT66GameMode, Warning, TEXT("[Pets] SpawnActor failed for boss %s pet %s."),
		*Boss->BossID.ToString(),
		*PetID.ToString());
	return false;
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

}

void AT66GameMode::SpawnBossForCurrentStage()
{
	UWorld* World = GetWorld();
	if (!World) return;
	ClearCagedStageCompanions(true);

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

	if (IsUsingTowerMainMapLayout())
	{
		const FVector TowerBossWaitingLocation = ResolveTowerBossWaitingLocation();
		if (!TowerBossWaitingLocation.IsNearlyZero())
		{
			StageData.BossSpawnLocation = TowerBossWaitingLocation;
		}
		else if (!MainMapBossSpawnSurfaceLocation.IsNearlyZero())
		{
			StageData.BossSpawnLocation = MainMapBossSpawnSurfaceLocation + FVector(0.f, 0.f, 100.f);
		}
	}
	else if (T66UsesMainMapTerrainStage(World) && !MainMapBossSpawnSurfaceLocation.IsNearlyZero())
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

	SpawnCagedCompanionsForCurrentStage(StageData.BossSpawnLocation);
}
