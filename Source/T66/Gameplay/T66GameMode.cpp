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
#include "Gameplay/T66HouseBlock.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66SaintNPC.h"
#include "Gameplay/T66OuroborosNPC.h"
#include "Gameplay/T66LoanShark.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66CowardiceGate.h"
#include "Gameplay/T66TricksterNPC.h"
#include "Gameplay/T66ColiseumExitGate.h"
#include "Gameplay/T66DifficultyTotem.h"
#include "Gameplay/T66BossGate.h"
#include "Gameplay/T66EnemyBase.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"

AT66GameMode::AT66GameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	// Default to our hero base class
	DefaultPawnClass = AT66HeroBase::StaticClass();
	DefaultHeroClass = AT66HeroBase::StaticClass();
}

void AT66GameMode::BeginPlay()
{
	Super::BeginPlay();

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
				RunState->ResetForNewRun();
			}
		}
	}

	if (bAutoSetupLevel)
	{
		EnsureLevelSetup();
	}

	// ColiseumLevel (or forced Coliseum mode): only spawn owed bosses + miasma (no houses, no waves, no NPCs, no start gate/pillars).
	if (IsColiseumLevel())
	{
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

		SpawnNextColiseumBossOrExit();
		UE_LOG(LogTemp, Log, TEXT("T66GameMode BeginPlay - ColiseumLevel"));
		return;
	}

	// Normal stage: houses + waves + miasma + boss + trickster/cowardice gate
	SpawnCornerHousesAndNPCs();
	SpawnTricksterAndCowardiceGate();

	// Spawn a difficulty totem near spawn (for testing)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<AT66DifficultyTotem>(AT66DifficultyTotem::StaticClass(), FVector(400.f, -600.f, 200.f), FRotator::ZeroRotator, SpawnParams);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<AT66EnemyDirector>(AT66EnemyDirector::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	MiasmaManager = GetWorld()->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	SpawnBossForCurrentStage();
	SpawnBossGateIfNeeded();

	UE_LOG(LogTemp, Log, TEXT("T66GameMode BeginPlay - Level setup complete, hero spawning handled by SpawnDefaultPawnFor"));
}

void AT66GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->TickStageTimer(DeltaTime);

			// Robust: if the timer is already active (even if we missed the delegate),
			// try spawning the LoanShark when pending.
			if (!IsColiseumLevel() && RunState->GetStageTimerActive())
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

	if (MiasmaManager)
	{
		MiasmaManager->UpdateFromRunState();
	}
}

void AT66GameMode::HandleStageTimerChanged()
{
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	if (!IsColiseumLevel() && RunState->GetStageTimerActive())
	{
		TrySpawnLoanSharkIfNeeded();
	}
}

void AT66GameMode::HandleDifficultyChanged()
{
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	const int32 Tier = RunState->GetDifficultyTier();
	for (TActorIterator<AT66EnemyBase> It(GetWorld()); It; ++It)
	{
		if (AT66EnemyBase* E = *It)
		{
			E->ApplyDifficultyTier(Tier);
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
	if (!IsColiseumLevel())
	{
		SpawnStartGateForPlayer(NewPlayer);
	}

	UT66GameInstance* GI = GetT66GameInstance();
	APawn* Pawn = NewPlayer ? NewPlayer->GetPawn() : nullptr;
	if (GI && Pawn && GI->bApplyLoadedTransform)
	{
		Pawn->SetActorTransform(GI->PendingLoadedTransform);
		GI->bApplyLoadedTransform = false;
		GI->PendingLoadedTransform = FTransform();
	}
}

void AT66GameMode::SpawnBossGateIfNeeded()
{
	if (IsColiseumLevel()) return;
	if (BossGate) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Between main square and boss square (pillars).
	const FVector BossGateLoc(6000.f, 0.f, 200.f);
	BossGate = World->SpawnActor<AT66BossGate>(AT66BossGate::StaticClass(), BossGateLoc, FRotator::ZeroRotator, SpawnParams);
}

bool AT66GameMode::IsColiseumLevel() const
{
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this);
	if (LevelName.Equals(TEXT("ColiseumLevel")))
	{
		return true;
	}

	if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->bForceColiseumMode;
	}
	return false;
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
	const FVector BossGateLoc(6000.f, 0.f, 200.f);
	const FVector GateLoc = BossGateLoc + FVector(-800.f, 0.f, 0.f);
	const FVector TricksterLoc = GateLoc + FVector(-250.f, 200.f, 0.f);

	CowardiceGate = World->SpawnActor<AT66CowardiceGate>(AT66CowardiceGate::StaticClass(), GateLoc, FRotator::ZeroRotator, SpawnParams);
	TricksterNPC = World->SpawnActor<AT66TricksterNPC>(AT66TricksterNPC::StaticClass(), TricksterLoc, FRotator::ZeroRotator, SpawnParams);
	if (TricksterNPC)
	{
		TricksterNPC->ApplyVisuals();
	}
}

void AT66GameMode::SpawnNextColiseumBossOrExit()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return;

	const TArray<FName>& Owed = RunState->GetOwedBossIDs();
	if (Owed.Num() <= 0)
	{
		// Nothing owed: spawn a stage gate to return to the checkpoint stage.
		SpawnStageGateAtLocation(FVector(0.f, 0.f, 190.f));
		return;
	}

	const FName BossID = Owed[0];
	FBossData BossData;
	if (!T66GI->GetBossData(BossID, BossData))
	{
		BossData.BossID = BossID;
		BossData.MaxHP = 100;
		BossData.AwakenDistance = 999999.f;
		BossData.MoveSpeed = 350.f;
		BossData.FireIntervalSeconds = 2.0f;
		BossData.ProjectileSpeed = 900.f;
		BossData.ProjectileDamageHearts = 1;
	}

	UClass* BossClass = AT66BossBase::StaticClass();
	if (!BossData.BossClass.IsNull())
	{
		if (UClass* Loaded = BossData.BossClass.LoadSynchronous())
		{
			if (Loaded->IsChildOf(AT66BossBase::StaticClass()))
			{
				BossClass = Loaded;
			}
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector SpawnLoc(1400.f, 0.f, 200.f);
	AActor* Spawned = World->SpawnActor<AActor>(BossClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (AT66BossBase* Boss = Cast<AT66BossBase>(Spawned))
	{
		Boss->InitializeBoss(BossData);
		// Coliseum should start immediately.
		Boss->ForceAwaken();
	}
}

void AT66GameMode::HandleBossDefeatedAtLocation(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World) return;
	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;

	if (IsColiseumLevel())
	{
		// In Coliseum, keep the timer/miasma running; just advance owed-boss queue.
		if (RunState)
		{
			RunState->SetBossInactive();
		}

		// Remove the cleared owed boss, then spawn next or exit.
		if (RunState && RunState->GetOwedBossIDs().Num() > 0)
		{
			RunState->RemoveFirstOwedBoss();
		}

		// If nothing left, spawn stage gate at the final boss death location (to return to checkpoint stage).
		if (!RunState || RunState->GetOwedBossIDs().Num() <= 0)
		{
			SpawnStageGateAtLocation(Location);
		}
		else
		{
			SpawnNextColiseumBossOrExit();
		}
		return;
	}

	if (RunState)
	{
		RunState->SetBossInactive();
		RunState->SetStageTimerActive(false);
	}

	// Normal stage: boss dead => miasma disappears and Stage Gate appears.
	ClearMiasma();
	SpawnStageGateAtLocation(Location);
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

	UClass* CompanionClass = AT66CompanionBase::StaticClass();
	if (!CompanionData.CompanionClass.IsNull())
	{
		UClass* Loaded = CompanionData.CompanionClass.LoadSynchronous();
		if (Loaded) CompanionClass = Loaded;
	}

	FVector SpawnLoc = HeroPawn->GetActorLocation() + FVector(-150.f, 100.f, 0.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66CompanionBase* Companion = World->SpawnActor<AT66CompanionBase>(CompanionClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (Companion)
	{
		Companion->InitializeCompanion(CompanionData);
		Companion->SetPreviewMode(false); // gameplay: follow hero
		UE_LOG(LogTemp, Log, TEXT("Spawned companion: %s"), *CompanionData.DisplayName.ToString());
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

	// Assume development floor roughly spans +/-5000; place houses at corners inside that.
	const float Corner = 4200.f;
	const float HouseZ = 100.f;
	const float NPCZ = 200.f;
	const float NPCOffset = 700.f;

	struct FCornerDef
	{
		FVector CornerLoc;
		TSubclassOf<AActor> NPCClass;
		FText Name;
		FLinearColor Color;
	};

	// Placeholder colors: Vendor=Black, Gambler=Red, Ouroboros=Green, Saint=White
	const TArray<FCornerDef> Corners = {
		{ FVector( Corner,  Corner, HouseZ), AT66VendorNPC::StaticClass(),    FText::FromString(TEXT("Vendor")),     FLinearColor(0.05f,0.05f,0.05f,1.f) },
		{ FVector( Corner, -Corner, HouseZ), AT66GamblerNPC::StaticClass(),   FText::FromString(TEXT("Gambler")),    FLinearColor(0.8f,0.1f,0.1f,1.f) },
		{ FVector(-Corner,  Corner, HouseZ), AT66OuroborosNPC::StaticClass(), FText::FromString(TEXT("Ouroboros")),  FLinearColor(0.1f,0.8f,0.2f,1.f) },
		{ FVector(-Corner, -Corner, HouseZ), AT66SaintNPC::StaticClass(),     FText::FromString(TEXT("Saint")),      FLinearColor(0.9f,0.9f,0.9f,1.f) },
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FCornerDef& D : Corners)
	{
		AT66HouseBlock* House = World->SpawnActor<AT66HouseBlock>(AT66HouseBlock::StaticClass(), D.CornerLoc, FRotator::ZeroRotator, SpawnParams);
		if (House)
		{
			House->HouseColor = D.Color;
			House->ApplyVisuals();
		}

		// NPC cylinder next to house (interactable; for now all sell items)
		const FVector NPCLoc(D.CornerLoc.X + NPCOffset, D.CornerLoc.Y, NPCZ);
		AActor* SpawnedNPC = World->SpawnActor<AActor>(D.NPCClass, NPCLoc, FRotator::ZeroRotator, SpawnParams);
		if (AT66HouseNPCBase* NPC = Cast<AT66HouseNPCBase>(SpawnedNPC))
		{
			NPC->NPCName = D.Name;
			NPC->NPCColor = D.Color;
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

	// Start Gate between start square and main square (pillars).
	const FVector SpawnLoc(-6000.f, 0.f, 200.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	StartGate = World->SpawnActor<AT66StartGate>(AT66StartGate::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (StartGate)
	{
		UE_LOG(LogTemp, Log, TEXT("Spawned Start Gate near hero"));
	}
}

void AT66GameMode::SpawnStageGateAtLocation(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Nudge upward slightly so the rectangle is above ground.
	FVector SpawnLoc = Location + FVector(0.f, 0.f, 10.f);
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

	if (IsColiseumLevel())
	{
		SpawnNextColiseumBossOrExit();
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
	StageData.BossID = FName(TEXT("Boss_01"));
	StageData.BossSpawnLocation = StageGateSpawnOffset; // far side by default

	FStageData FromDT;
	if (T66GI->GetStageData(StageNum, FromDT))
	{
		StageData = FromDT;
	}

	// Map layout: always spawn the stage boss in the Boss Area square.
	// (Fixes Stage 1 incorrectly spawning near the center if DT values were authored for the old layout.)
	StageData.BossSpawnLocation = FVector(10000.f, 0.f, 200.f);

	// Default/fallback boss data (if DT_Bosses is not wired yet)
	FBossData BossData;
	BossData.BossID = StageData.BossID;
	BossData.MaxHP = 100;
	BossData.AwakenDistance = 900.f;
	BossData.MoveSpeed = 350.f;
	BossData.FireIntervalSeconds = 2.0f;
	BossData.ProjectileSpeed = 900.f;
	BossData.ProjectileDamageHearts = 1;

	FBossData FromBossDT;
	if (!StageData.BossID.IsNone() && T66GI->GetBossData(StageData.BossID, FromBossDT))
	{
		BossData = FromBossDT;
	}

	UClass* BossClass = AT66BossBase::StaticClass();
	if (!BossData.BossClass.IsNull())
	{
		if (UClass* Loaded = BossData.BossClass.LoadSynchronous())
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
		UE_LOG(LogTemp, Log, TEXT("Spawned boss for Stage %d (BossID=%s)"), StageNum, *BossData.BossID.ToString());
	}
}

void AT66GameMode::EnsureLevelSetup()
{
	UE_LOG(LogTemp, Log, TEXT("Checking level setup..."));
	
	SpawnFloorIfNeeded();
	SpawnLightingIfNeeded();
	SpawnPlayerStartIfNeeded();
}

void AT66GameMode::SpawnFloorIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Layout: Start Square -> Main Square -> Boss Square, with two small connector floors.
	// Pillars (gates) are spawned separately (no pillars in Coliseum).
	const FVector MainCenter(0.f, 0.f, -50.f);
	const FVector StartCenter(-10000.f, 0.f, -50.f);
	const FVector BossCenter(10000.f, 0.f, -50.f);
	const FVector StartConnectorCenter(-6000.f, 0.f, -50.f);
	const FVector BossConnectorCenter(6000.f, 0.f, -50.f);

	struct FFloorSpec
	{
		FName Tag;
		FVector Location;
		FVector Scale;
		FLinearColor Color;
	};

	const TArray<FFloorSpec> Floors = {
		{ FName("T66_Floor_Main"),   MainCenter,          FVector(100.f, 100.f, 1.f), FLinearColor(0.30f, 0.30f, 0.35f, 1.f) },
		{ FName("T66_Floor_Start"),  StartCenter,         FVector(60.f,  60.f,  1.f), FLinearColor(0.22f, 0.24f, 0.28f, 1.f) },
		{ FName("T66_Floor_Boss"),   BossCenter,          FVector(60.f,  60.f,  1.f), FLinearColor(0.26f, 0.22f, 0.22f, 1.f) },
		{ FName("T66_Floor_Conn1"),  StartConnectorCenter, FVector(20.f, 30.f, 1.f), FLinearColor(0.25f, 0.25f, 0.28f, 1.f) },
		{ FName("T66_Floor_Conn2"),  BossConnectorCenter,  FVector(20.f, 30.f, 1.f), FLinearColor(0.25f, 0.25f, 0.28f, 1.f) },
	};

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
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

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FFloorSpec& Spec : Floors)
	{
		if (HasTag(Spec.Tag)) continue;

		AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Spec.Location, FRotator::ZeroRotator, SpawnParams);
		if (!Floor || !Floor->GetStaticMeshComponent()) continue;

		Floor->Tags.Add(Spec.Tag);
		Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		Floor->SetActorScale3D(Spec.Scale);

		if (UMaterialInstanceDynamic* FloorMat = Floor->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
		{
			FloorMat->SetVectorParameterValue(TEXT("BaseColor"), Spec.Color);
		}

		SpawnedSetupActors.Add(Floor);
	}
}

void AT66GameMode::SpawnLightingIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Check for existing directional light
	bool bHasDirectionalLight = false;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		bHasDirectionalLight = true;
		break;
	}

	// Check for existing sky light
	bool bHasSkyLight = false;
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		bHasSkyLight = true;
		break;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn directional light (sun) if needed
	if (!bHasDirectionalLight)
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
			if (ULightComponent* LightComp = Sun->GetLightComponent())
			{
				LightComp->SetIntensity(3.f);
				LightComp->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f)); // Warm sunlight
			}
			#if WITH_EDITOR
			Sun->SetActorLabel(TEXT("DEV_Sun"));
			#endif
			SpawnedSetupActors.Add(Sun);
			UE_LOG(LogTemp, Log, TEXT("Spawned development directional light"));
		}
	}

	// Spawn sky light (ambient) if needed
	if (!bHasSkyLight)
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
			if (USkyLightComponent* SkyComp = Sky->GetLightComponent())
			{
				SkyComp->SetIntensity(1.0f);
				SkyComp->SetLightColor(FLinearColor(0.5f, 0.6f, 0.8f)); // Bluish ambient
				SkyComp->RecaptureSky();
			}
			#if WITH_EDITOR
			Sky->SetActorLabel(TEXT("DEV_SkyLight"));
			#endif
			SpawnedSetupActors.Add(Sky);
			UE_LOG(LogTemp, Log, TEXT("Spawned development sky light"));
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
		// - GameplayLevel: start area (so timer starts after passing start pillars)
		// - ColiseumLevel: main area (timer starts immediately; no pillars)
		const FVector SpawnLoc = IsColiseumLevel()
			? FVector(0.f, 0.f, DefaultSpawnHeight)
			: FVector(-10000.f, 0.f, DefaultSpawnHeight);

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
			UE_LOG(LogTemp, Log, TEXT("Spawned development PlayerStart at (0, 0, %.1f)"), DefaultSpawnHeight);
		}
	}
}

UT66GameInstance* AT66GameMode::GetT66GameInstance() const
{
	return Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
}

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
				UClass* HeroClass = HeroData.HeroClass.LoadSynchronous();
				if (HeroClass)
				{
					return HeroClass;
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
		// No PlayerStart found - spawn at a safe default location
		SpawnLocation = IsColiseumLevel()
			? FVector(0.f, 0.f, 200.f)         // Coliseum: spawn in main arena
			: FVector(-10000.f, 0.f, 200.f);   // Gameplay: spawn in starting area
		UE_LOG(LogTemp, Warning, TEXT("No PlayerStart found! Spawning at default location (%.0f, %.0f, %.0f)."),
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
	}

	// Robust: always ensure Gameplay spawns in the Start Area regardless of where a PlayerStart was placed.
	// (Coliseum spawns in the Main Area and starts timer immediately.)
	if (!IsColiseumLevel())
	{
		SpawnLocation = FVector(-10000.f, 0.f, 200.f);
		SpawnRotation = FRotator::ZeroRotator;
	}

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn the pawn
	APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnLocation, SpawnRotation, SpawnParams);

	// If it's our hero class, initialize it with hero data and body type
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(SpawnedPawn))
	{
		if (UT66GameInstance* GI = GetT66GameInstance())
		{
			FHeroData HeroData;
			if (GI->GetSelectedHeroData(HeroData))
			{
				// Pass both hero data AND body type from Game Instance
				ET66BodyType SelectedBodyType = GI->SelectedHeroBodyType;
				Hero->InitializeHero(HeroData, SelectedBodyType);
				
				UE_LOG(LogTemp, Log, TEXT("Spawned hero: %s, BodyType: %s, Color: (%.2f, %.2f, %.2f)"),
					*HeroData.DisplayName.ToString(),
					SelectedBodyType == ET66BodyType::TypeA ? TEXT("TypeA") : TEXT("TypeB"),
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
