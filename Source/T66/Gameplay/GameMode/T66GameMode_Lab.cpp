// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameModePrivate.h"

using namespace T66GameModePrivate;

bool AT66GameMode::IsLabLevel() const
{
	if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->bIsLabLevel;
	}
	return false;
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

	TSubclassOf<AT66EnemyBase> ClassToSpawn = nullptr;
	const FName NormalizedEnemyID = FT66EnemyFamilyResolver::NormalizeMobID(CharacterVisualID);
	if (NormalizedEnemyID == FName(TEXT("GoblinThief")))
	{
		ClassToSpawn = AT66GoblinThiefEnemy::StaticClass();
	}
	else if (NormalizedEnemyID == FName(TEXT("UniqueEnemy")))
	{
		ClassToSpawn = AT66UniqueDebuffEnemy::StaticClass();
	}
	else
	{
		ClassToSpawn = FT66EnemyFamilyResolver::ResolveEnemyClass(NormalizedEnemyID, nullptr);
	}

	if (!ClassToSpawn)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FVector Loc = GetRandomLabSpawnLocation();
	FRotator Rot = FRotator::ZeroRotator;
	AT66EnemyBase* Mob = World->SpawnActor<AT66EnemyBase>(ClassToSpawn, Loc, Rot, SpawnParams);
	if (Mob)
	{
		if (FT66EnemyFamilyResolver::IsStageMobID(NormalizedEnemyID))
		{
			Mob->ConfigureAsMob(NormalizedEnemyID);
		}
		else if (NormalizedEnemyID == FName(TEXT("GoblinThief")))
		{
			Mob->CharacterVisualID = NormalizedEnemyID;
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
