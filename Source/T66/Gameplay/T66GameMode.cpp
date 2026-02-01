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
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66SaintNPC.h"
#include "Gameplay/T66OuroborosNPC.h"
#include "Gameplay/T66LoanShark.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66CowardiceGate.h"
#include "Gameplay/T66TricksterNPC.h"
#include "Gameplay/T66DifficultyTotem.h"
#include "Gameplay/T66BossGate.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66IdolAltar.h"
#include "Gameplay/T66TreeOfLifeInteractable.h"
#include "Gameplay/T66CashTruckInteractable.h"
#include "Gameplay/T66WheelSpinInteractable.h"
#include "Gameplay/T66StageBoostGate.h"
#include "Gameplay/T66StageBoostGoldInteractable.h"
#include "Gameplay/T66StageBoostLootInteractable.h"
#include "Gameplay/T66StageEffectTile.h"
#include "Gameplay/T66TutorialManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RunStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"

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
}

AT66GameMode::AT66GameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	// Default to our hero base class
	DefaultPawnClass = AT66HeroBase::StaticClass();
	DefaultHeroClass = AT66HeroBase::StaticClass();

	// Coliseum arena lives inside GameplayLevel, off to the side (walled off).
	ColiseumCenter = FVector(-10000.f, -5200.f, 200.f);
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

	UT66GameInstance* GIAsT66 = GetT66GameInstance();
	const bool bStageBoost = (GIAsT66 && GIAsT66->bStageBoostPending);

	// Stage Boost: spawn a separate platform and skip normal stage content.
	if (bStageBoost)
	{
		SpawnStageBoostPlatformAndInteractables();
		UE_LOG(LogTemp, Log, TEXT("T66GameMode BeginPlay - StageBoost"));
		return;
	}

	// Normal stage: houses + waves + miasma + boss + trickster/cowardice gate
	SpawnCornerHousesAndNPCs();
	SpawnTricksterAndCowardiceGate();
	SpawnWorldInteractablesForStage();
	SpawnStageEffectTilesForStage();
	SpawnTutorialIfNeeded();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<AT66EnemyDirector>(AT66EnemyDirector::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	MiasmaManager = GetWorld()->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	SpawnBossForCurrentStage();
	SpawnBossGateIfNeeded();

	UE_LOG(LogTemp, Log, TEXT("T66GameMode BeginPlay - Level setup complete, hero spawning handled by SpawnDefaultPawnFor"));
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
	}

	Super::EndPlay(EndPlayReason);
}

void AT66GameMode::SpawnTutorialIfNeeded()
{
	if (IsColiseumStage()) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66AchievementsSubsystem* Ach = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!RunState || !T66GI) return;
	if (T66GI->bStageBoostPending) return;

	// v0 per your request: stage 1 always shows tutorial prompts.
	if (RunState->GetCurrentStage() != 1) return;

	// Only run tutorial if first-time (profile) or forced by dev switch.
	const bool bShouldSpawnTutorial = bForceSpawnInTutorialArea || (Ach && !Ach->HasCompletedTutorial());
	if (!bShouldSpawnTutorial) return;

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

void AT66GameMode::SpawnStageEffectTilesForStage()
{
	if (IsColiseumStage()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!RunState || !T66GI) return;
	if (T66GI->bStageBoostPending) return;

	const int32 StageNum = RunState->GetCurrentStage();

	// Stage data (optional); fall back to deterministic mapping.
	FStageData StageData;
	const bool bHasDT = T66GI->GetStageData(StageNum, StageData);
	ET66StageEffectType EffectType = bHasDT ? StageData.StageEffectType : ET66StageEffectType::None;
	FLinearColor EffectColor = bHasDT ? StageData.StageEffectColor : FLinearColor::White;
	float Strength = bHasDT ? StageData.StageEffectStrength : 1.f;

	if (EffectType == ET66StageEffectType::None)
	{
		const int32 Mod = StageNum % 3;
		EffectType = (Mod == 1) ? ET66StageEffectType::Speed : (Mod == 2) ? ET66StageEffectType::Launch : ET66StageEffectType::Slide;
		// Unique-ish color per stage (HSV wheel).
		const float Hue = FMath::Fmod(static_cast<float>(StageNum) * 37.f, 360.f);
		EffectColor = FLinearColor::MakeFromHSV8(static_cast<uint8>(Hue / 360.f * 255.f), 210, 240);
		EffectColor.A = 1.f;
		Strength = 1.f;
	}

	FRandomStream Rng(StageNum * 971 + 17);

	// Main map square bounds (centered at 0,0). Keep some margin from edges.
	static constexpr float MainHalfExtent = 9200.f;
	static constexpr float SpawnZ = 40.f;
	static constexpr float MinDistBetweenTiles = 420.f;
	static constexpr float SafeBubbleMargin = 350.f;
	static constexpr int32 TileCount = 12;

	TArray<FVector> UsedLocs;

	auto IsGoodLoc = [&](const FVector& L) -> bool
	{
		for (const FVector& U : UsedLocs)
		{
			if (FVector::DistSquared2D(L, U) < (MinDistBetweenTiles * MinDistBetweenTiles))
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

	auto FindSpawnLoc = [&]() -> FVector
	{
		for (int32 Try = 0; Try < 80; ++Try)
		{
			const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			FVector Loc(X, Y, SpawnZ);

			// Trace to ground.
			FHitResult Hit;
			const FVector Start = Loc + FVector(0.f, 0.f, 2000.f);
			const FVector End = Loc - FVector(0.f, 0.f, 6000.f);
			if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
			{
				Loc = Hit.ImpactPoint;
			}

			if (IsGoodLoc(Loc))
			{
				return Loc;
			}
		}
		return FVector(0.f, 0.f, SpawnZ);
	};

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < TileCount; ++i)
	{
		const FVector L = FindSpawnLoc();
		AT66StageEffectTile* Tile = World->SpawnActor<AT66StageEffectTile>(AT66StageEffectTile::StaticClass(), L, FRotator::ZeroRotator, P);
		if (Tile)
		{
			Tile->EffectType = EffectType;
			Tile->EffectColor = EffectColor;
			Tile->Strength = Strength;
			Tile->ApplyVisuals();
			UsedLocs.Add(L);
		}
	}
}

void AT66GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->TickStageTimer(DeltaTime);
			RunState->TickSpeedRunTimer(DeltaTime);
			RunState->TickHeroTimers(DeltaTime);

			// Robust: if the timer is already active (even if we missed the delegate),
			// try spawning the LoanShark when pending.
			if (!IsColiseumStage() && RunState->GetStageTimerActive())
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
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

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
	if (!IsColiseumStage())
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
	if (IsColiseumStage()) return;
	if (BossGate) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Between main square and boss square (pillars).
	const FVector BossGateLoc(6000.f, 0.f, 0.f);
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
			if (UClass* Loaded = BossData.BossClass.LoadSynchronous())
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
	const FVector BossGateLoc(6000.f, 0.f, 0.f);
	const FVector GateLoc = BossGateLoc + FVector(-800.f, 0.f, 0.f);
	const FVector TricksterLoc = GateLoc + FVector(-250.f, 200.f, 0.f);

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

void AT66GameMode::HandleBossDefeatedAtLocation(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World) return;
	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;

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

	// Normal stage: boss dead => miasma disappears and Stage Gate appears.
	ClearMiasma();
	SpawnStageGateAtLocation(Location);

	// Idol Altar unlock: after bosses of stages ending in 5 or 0 (i.e. Stage % 5 == 0),
	// spawn an altar near the boss death so the player can pick an idol before leaving.
	if (RunState)
	{
		SpawnIdolAltarAtLocation(Location);
	}
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

	// Main area: corner NPCs are placed near the 4 corners of the main square.
	// NOTE: NPC safe-zones are centered on the NPC cylinders (not the house blocks),
	// so we place the NPCs *inward* toward the map center to keep the full bubble inside bounds.
	const float Corner = 9000.f;
	const float NPCZ = 60.f;    // small cylinder NPC sits on ground-ish
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
		{ FVector( Corner,  Corner, NPCZ), AT66VendorNPC::StaticClass(),    NSLOCTEXT("T66.NPC", "Vendor", "Vendor"),        FLinearColor(0.05f,0.05f,0.05f,1.f) },
		{ FVector( Corner, -Corner, NPCZ), AT66GamblerNPC::StaticClass(),   NSLOCTEXT("T66.NPC", "Gambler", "Gambler"),      FLinearColor(0.8f,0.1f,0.1f,1.f) },
		{ FVector(-Corner,  Corner, NPCZ), AT66OuroborosNPC::StaticClass(), NSLOCTEXT("T66.NPC", "Ouroboros", "Ouroboros"),  FLinearColor(0.1f,0.8f,0.2f,1.f) },
		{ FVector(-Corner, -Corner, NPCZ), AT66SaintNPC::StaticClass(),     NSLOCTEXT("T66.NPC", "Saint", "Saint"),          FLinearColor(0.9f,0.9f,0.9f,1.f) },
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FCornerDef& D : Corners)
	{
		// Corner NPC (interactable). Place inward from the corner so the safe-zone disc stays fully on-map.
		const float SX = (D.CornerLoc.X >= 0.f) ? 1.f : -1.f;
		const float SY = (D.CornerLoc.Y >= 0.f) ? 1.f : -1.f;
		const FVector NPCLoc(
			D.CornerLoc.X - (SX * NPCOffset),
			D.CornerLoc.Y - (SY * NPCOffset),
			NPCZ);
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
	const FVector SpawnLoc(-6000.f, 0.f, 0.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	StartGate = World->SpawnActor<AT66StartGate>(AT66StartGate::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (StartGate)
	{
		UE_LOG(LogTemp, Log, TEXT("Spawned Start Gate near hero"));
	}
}

void AT66GameMode::SpawnWorldInteractablesForStage()
{
	if (IsColiseumStage()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	const int32 StageNum = RunState->GetCurrentStage();
	FRandomStream Rng(StageNum * 1337 + 42);

	// Main map square bounds (centered at 0,0). Keep some margin from walls.
	static constexpr float MainHalfExtent = 9200.f;
	static constexpr float SpawnZ = 220.f;
	static constexpr float MinDistBetweenInteractables = 900.f;
	static constexpr float SafeBubbleMargin = 250.f;

	TArray<FVector> UsedLocs;

	auto IsGoodLoc = [&](const FVector& L) -> bool
	{
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

	auto FindSpawnLoc = [&]() -> FVector
	{
		for (int32 Try = 0; Try < 40; ++Try)
		{
			const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			FVector Loc(X, Y, SpawnZ);

			// Trace to ground.
			FHitResult Hit;
			const FVector Start = Loc + FVector(0.f, 0.f, 1000.f);
			const FVector End = Loc - FVector(0.f, 0.f, 4000.f);
			if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
			{
				Loc = Hit.ImpactPoint;
			}

			if (IsGoodLoc(Loc))
			{
				return Loc;
			}
		}
		// Fallback: center-ish.
		return FVector(0.f, 0.f, SpawnZ);
	};

	auto SpawnOne = [&](UClass* Cls) -> AActor*
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		const FVector L = FindSpawnLoc();
		AActor* A = World->SpawnActor<AActor>(Cls, L, FRotator::ZeroRotator, P);
		if (A) UsedLocs.Add(L);
		return A;
	};

	// Randomized counts per play (2-5 each).
	const int32 CountTrees = Rng.RandRange(2, 5);
	const int32 CountTrucks = Rng.RandRange(2, 5);
	const int32 CountWheels = Rng.RandRange(2, 5);
	const int32 CountTotems = Rng.RandRange(2, 5);

	const int32 LuckStat = RunState->GetLuckStat();

	for (int32 i = 0; i < CountTrees; ++i)
	{
		if (AT66TreeOfLifeInteractable* Tree = Cast<AT66TreeOfLifeInteractable>(SpawnOne(AT66TreeOfLifeInteractable::StaticClass())))
		{
			Tree->SetRarity(FT66RarityUtil::RollRarityWithLuck(Rng, LuckStat));
		}
	}
	for (int32 i = 0; i < CountTrucks; ++i)
	{
		if (AT66CashTruckInteractable* Truck = Cast<AT66CashTruckInteractable>(SpawnOne(AT66CashTruckInteractable::StaticClass())))
		{
			Truck->bIsMimic = (Rng.FRand() < 0.20f);
			Truck->SetRarity(FT66RarityUtil::RollRarityWithLuck(Rng, LuckStat));
		}
	}
	for (int32 i = 0; i < CountWheels; ++i)
	{
		if (AT66WheelSpinInteractable* Wheel = Cast<AT66WheelSpinInteractable>(SpawnOne(AT66WheelSpinInteractable::StaticClass())))
		{
			Wheel->SetRarity(FT66RarityUtil::RollRarityWithLuck(Rng, LuckStat));
		}
	}
	for (int32 i = 0; i < CountTotems; ++i)
	{
		if (AT66DifficultyTotem* Totem = Cast<AT66DifficultyTotem>(SpawnOne(AT66DifficultyTotem::StaticClass())))
		{
			Totem->SetRarity(FT66RarityUtil::RollRarityWithLuck(Rng, LuckStat));
		}
	}
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

void AT66GameMode::SpawnStageBoostPlatformAndInteractables()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return;

	RunState->SetInStageBoost(true);

	// Difficulty index: Easy=0, Medium=1, Hard=2, ...
	const int32 DiffIndex = FMath::Max(0, static_cast<int32>(T66GI->SelectedDifficulty));
	const int32 StartStage = FMath::Clamp(1 + (DiffIndex * 10), 1, 66);

	// Tuned v0 amounts (increase with difficulty step).
	const int32 GoldAmount = 200 * DiffIndex;          // Medium=200, Hard=400, ...
	const int32 LootBags = 2 + (DiffIndex * 2);        // Medium=4, Hard=6, ...

	// Place boost platform in the Start Area, but offset so it's clearly separate.
	const FVector PlatformCenter(-10000.f, 5200.f, -50.f);

	// Platform floor
	{
		UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (CubeMesh)
		{
			const FName Tag(TEXT("T66_Floor_Boost"));
			bool bHas = false;
			for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
			{
				if (It->Tags.Contains(Tag)) { bHas = true; break; }
			}
			if (!bHas)
			{
				FActorSpawnParameters P;
				P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), PlatformCenter, FRotator::ZeroRotator, P);
				if (Floor && Floor->GetStaticMeshComponent())
				{
					Floor->Tags.Add(Tag);
					Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
				T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Movable);
					Floor->SetActorScale3D(FVector(22.f, 22.f, 1.f));
				T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Static);
					if (UMaterialInstanceDynamic* Mat = Floor->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
					{
						Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.12f, 0.12f, 0.14f, 1.f));
					}
				}
			}
		}
	}

	// Spawn interactables
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AT66StageBoostGoldInteractable* Gold = World->SpawnActor<AT66StageBoostGoldInteractable>(
			AT66StageBoostGoldInteractable::StaticClass(),
			PlatformCenter + FVector(-260.f, -120.f, 120.f),
			FRotator::ZeroRotator,
			P);
		if (Gold)
		{
			Gold->GoldAmount = GoldAmount;
		}

		AT66StageBoostLootInteractable* Loot = World->SpawnActor<AT66StageBoostLootInteractable>(
			AT66StageBoostLootInteractable::StaticClass(),
			PlatformCenter + FVector(-260.f, 220.f, 120.f),
			FRotator::ZeroRotator,
			P);
		if (Loot)
		{
			Loot->LootBagCount = LootBags;
		}

		AT66StageBoostGate* Gate = World->SpawnActor<AT66StageBoostGate>(
			AT66StageBoostGate::StaticClass(),
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

	// Map layout: always spawn the stage boss in the Boss Area square.
	// (Fixes Stage 1 incorrectly spawning near the center if DT values were authored for the old layout.)
	StageData.BossSpawnLocation = FVector(10000.f, 0.f, 200.f);

	// Default/fallback boss data (if DT_Bosses is not wired yet)
	FBossData BossData;
	BossData.BossID = StageData.BossID;
	{
		// Stage-scaled fallback so all 66 stages work even if DT_Bosses isn't reimported yet.
		const int32 S = FMath::Clamp(StageNum, 1, 66);
		const float T = static_cast<float>(S - 1) / 65.f; // 0..1

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
		UE_LOG(LogTemp, Log, TEXT("Spawned boss for Stage %d (BossID=%s)"), StageNum, *BossData.BossID.ToString());
	}
}

void AT66GameMode::EnsureLevelSetup()
{
	UE_LOG(LogTemp, Log, TEXT("Checking level setup..."));
	
	SpawnFloorIfNeeded();
	SpawnColiseumArenaIfNeeded();
	SpawnTutorialArenaIfNeeded();
	SpawnBoundaryWallsIfNeeded();
	SpawnPlatformEdgeWallsIfNeeded();
	SpawnStartAreaExitWallsIfNeeded();
	SpawnLightingIfNeeded();
	SpawnPlayerStartIfNeeded();
}

void AT66GameMode::SpawnTutorialArenaIfNeeded()
{
	// Tutorial Arena is an enclosed side-area inside GameplayLevel. It is only relevant for normal gameplay stages.
	if (IsColiseumStage()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UT66GameInstance* T66GI = GetT66GameInstance();
	if (T66GI && T66GI->bStageBoostPending)
	{
		// Keep the boost platform area clean; tutorial arena not needed there.
		return;
	}

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

	// Place near the north side of the main square, fully inside global bounds.
	// (Can't be constexpr in UE due to FVector not being constexpr-friendly on this toolchain.)
	const FVector TutorialCenter(0.f, 7500.f, -50.f);
	static constexpr float Half = 2000.f;
	static constexpr float WallHeight = 2200.f;
	static constexpr float WallThickness = 240.f;

	const FName FloorTag(TEXT("T66_Floor_Tutorial"));

	// Floor
	if (!HasTag(FloorTag))
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), TutorialCenter, FRotator::ZeroRotator, P);
		if (Floor && Floor->GetStaticMeshComponent())
		{
			Floor->Tags.Add(FloorTag);
			Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
			T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Movable);
			Floor->SetActorScale3D(FVector(40.f, 40.f, 1.f)); // 4000x4000
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

	// Walls
	struct FWallSpec
	{
		FName Tag;
		FVector Loc;
		FVector Scale;
	};

	const float WallZ = 0.f + (WallHeight * 0.5f);
	const float Thick = WallThickness / 100.f;
	const float Tall = WallHeight / 100.f;
	const float Long = ((Half * 2.f) + WallThickness) / 100.f;

	const TArray<FWallSpec> Walls = {
		{ FName(TEXT("T66_Wall_Tutorial_N")), FVector(TutorialCenter.X, TutorialCenter.Y + Half + (WallThickness * 0.5f), WallZ), FVector(Long, Thick, Tall) },
		{ FName(TEXT("T66_Wall_Tutorial_S")), FVector(TutorialCenter.X, TutorialCenter.Y - Half - (WallThickness * 0.5f), WallZ), FVector(Long, Thick, Tall) },
		{ FName(TEXT("T66_Wall_Tutorial_E")), FVector(TutorialCenter.X + Half + (WallThickness * 0.5f), TutorialCenter.Y, WallZ), FVector(Thick, Long, Tall) },
		{ FName(TEXT("T66_Wall_Tutorial_W")), FVector(TutorialCenter.X - Half - (WallThickness * 0.5f), TutorialCenter.Y, WallZ), FVector(Thick, Long, Tall) },
	};

	for (const FWallSpec& Spec : Walls)
	{
		if (HasTag(Spec.Tag)) continue;
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AStaticMeshActor* Wall = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Spec.Loc, FRotator::ZeroRotator, P);
		if (!Wall || !Wall->GetStaticMeshComponent()) continue;
		Wall->Tags.Add(Spec.Tag);
		Wall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Movable);
		Wall->SetActorScale3D(Spec.Scale);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Static);
		if (UMaterialInstanceDynamic* Mat = Wall->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.08f, 0.08f, 0.10f, 1.f));
		}
		if (!SpawnedSetupActors.Contains(Wall))
		{
			SpawnedSetupActors.Add(Wall);
		}
	}
}

void AT66GameMode::SpawnColiseumArenaIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

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

	// Coliseum arena: off to the side, walled off so it is not visible from the main map.
	const FVector ArenaCenter(ColiseumCenter.X, ColiseumCenter.Y, -50.f);
	const FName FloorTag(TEXT("T66_Floor_Coliseum"));
	const FName WallTagPrefix(TEXT("T66_Wall_Coliseum"));

	// Floor
	if (!HasTag(FloorTag))
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), ArenaCenter, FRotator::ZeroRotator, P);
		if (Floor && Floor->GetStaticMeshComponent())
		{
			Floor->Tags.Add(FloorTag);
			Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
			T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Movable);
			Floor->SetActorScale3D(FVector(34.f, 34.f, 1.f)); // 3400x3400
			T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Static);
			if (UMaterialInstanceDynamic* Mat = Floor->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
			{
				Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.12f, 0.10f, 0.10f, 1.f));
			}
			SpawnedSetupActors.Add(Floor);
		}
	}

	// Walls around arena (local enclosure; avoids needing huge global bounds)
	static constexpr float WallHeight = 2200.f;
	static constexpr float WallThickness = 240.f;
	const float WallZ = 0.f + (WallHeight * 0.5f);
	const float Half = 1700.f;
	const float Thick = WallThickness / 100.f;
	const float Tall = WallHeight / 100.f;
	const float Long = ((Half * 2.f) + WallThickness) / 100.f;

	struct FWallSpec
	{
		FName Tag;
		FVector Loc;
		FVector Scale;
	};

	const TArray<FWallSpec> Walls = {
		{ FName(TEXT("T66_Wall_Coliseum_N")), FVector(ArenaCenter.X, ArenaCenter.Y + Half + (WallThickness * 0.5f), WallZ), FVector(Long, Thick, Tall) },
		{ FName(TEXT("T66_Wall_Coliseum_S")), FVector(ArenaCenter.X, ArenaCenter.Y - Half - (WallThickness * 0.5f), WallZ), FVector(Long, Thick, Tall) },
		{ FName(TEXT("T66_Wall_Coliseum_E")), FVector(ArenaCenter.X + Half + (WallThickness * 0.5f), ArenaCenter.Y, WallZ), FVector(Thick, Long, Tall) },
		{ FName(TEXT("T66_Wall_Coliseum_W")), FVector(ArenaCenter.X - Half - (WallThickness * 0.5f), ArenaCenter.Y, WallZ), FVector(Thick, Long, Tall) },
	};

	for (const FWallSpec& Spec : Walls)
	{
		if (HasTag(Spec.Tag)) continue;
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AStaticMeshActor* Wall = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Spec.Loc, FRotator::ZeroRotator, P);
		if (!Wall || !Wall->GetStaticMeshComponent()) continue;
		Wall->Tags.Add(Spec.Tag);
		Wall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Movable);
		Wall->SetActorScale3D(Spec.Scale);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Static);
		if (UMaterialInstanceDynamic* Mat = Wall->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.08f, 0.06f, 0.06f, 1.f));
		}
		SpawnedSetupActors.Add(Wall);
	}
}

void AT66GameMode::SpawnPlatformEdgeWallsIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

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

	// Add local walls around the narrower Start/Boss squares and the connector strips,
	// so you can't fall off the sides before reaching the global boundary walls.
	static constexpr float FloorTopZ = 0.f;
	static constexpr float WallHeight = 1500.f;
	static constexpr float WallThickness = 200.f;
	const float WallZ = FloorTopZ + (WallHeight * 0.5f);

	static constexpr float StartBossHalfY = 3000.f;   // start/boss floors are 6000 wide
	static constexpr float StartBossHalfX = 3000.f;   // start/boss floors are 6000 long
	static constexpr float ConnHalfY = 1500.f;        // connector floors are 3000 wide
	static constexpr float ConnHalfX = 1000.f;        // connector floors are 2000 long

	const float Thick = WallThickness / 100.f;
	const float Tall = WallHeight / 100.f;

	struct FWallSpec
	{
		FName Tag;
		FVector Location;
		FVector Scale;
		FLinearColor Color;
	};

	auto MakeNSWallsForPlatform = [&](const TCHAR* Prefix, const FVector& Center, float HalfX, float HalfY, const FLinearColor& Color)
	{
		const float LongX = (HalfX * 2.f + WallThickness) / 100.f;
		const float Y = HalfY + (WallThickness * 0.5f);
		return TArray<FWallSpec>{
			{ FName(*FString::Printf(TEXT("%s_N"), Prefix)), FVector(Center.X, Center.Y + Y, WallZ), FVector(LongX, Thick, Tall), Color },
			{ FName(*FString::Printf(TEXT("%s_S"), Prefix)), FVector(Center.X, Center.Y - Y, WallZ), FVector(LongX, Thick, Tall), Color },
		};
	};

	TArray<FWallSpec> Walls;
	Walls.Append(MakeNSWallsForPlatform(TEXT("T66_Wall_StartEdge"), FVector(-10000.f, 0.f, 0.f), StartBossHalfX, StartBossHalfY, FLinearColor(0.08f,0.08f,0.10f,1.f)));
	Walls.Append(MakeNSWallsForPlatform(TEXT("T66_Wall_BossEdge"),  FVector( 10000.f, 0.f, 0.f), StartBossHalfX, StartBossHalfY, FLinearColor(0.08f,0.08f,0.10f,1.f)));
	Walls.Append(MakeNSWallsForPlatform(TEXT("T66_Wall_ConnStart"), FVector( -6000.f, 0.f, 0.f), ConnHalfX,     ConnHalfY,     FLinearColor(0.07f,0.07f,0.09f,1.f)));
	Walls.Append(MakeNSWallsForPlatform(TEXT("T66_Wall_ConnBoss"),  FVector(  6000.f, 0.f, 0.f), ConnHalfX,     ConnHalfY,     FLinearColor(0.07f,0.07f,0.09f,1.f)));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FWallSpec& Spec : Walls)
	{
		if (HasTag(Spec.Tag)) continue;
		AStaticMeshActor* Wall = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Spec.Location, FRotator::ZeroRotator, SpawnParams);
		if (!Wall || !Wall->GetStaticMeshComponent()) continue;

		Wall->Tags.Add(Spec.Tag);
		Wall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Movable);
		Wall->SetActorScale3D(Spec.Scale);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Static);
		if (UMaterialInstanceDynamic* Mat = Wall->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Spec.Color);
		}
		SpawnedSetupActors.Add(Wall);
	}
}

void AT66GameMode::SpawnStartAreaExitWallsIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

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

	// Build a simple corridor so you can only leave Start Area through the Start Gate pillars.
	// Layout: Start square ends at X=-7000. Main square begins at X=-7000.
	// We block the X=-7000 boundary except a small opening near Y=0, then funnel to X=-6000.
	static constexpr float FloorTopZ = 0.f;
	static constexpr float WallHeight = 1500.f;
	static constexpr float WallThickness = 200.f;
	static constexpr float WallZ = FloorTopZ + (WallHeight * 0.5f);

	static constexpr float StartBoundaryX = -7000.f;
	static constexpr float GateX = -6000.f;
	static constexpr float StartHalfY = 3000.f;
	static constexpr float GateHalfY = 1500.f; // connector width
	static constexpr float GapHalfY = 160.f;   // opening to pass through

	const float Thick = WallThickness / 100.f;
	const float Tall = WallHeight / 100.f;

	struct FWallSpec
	{
		FName Tag;
		FVector Location;
		FVector Scale;
		FLinearColor Color;
	};

	const float SegStartCenterY = (StartHalfY + GapHalfY) * 0.5f;
	const float SegStartScaleY = (StartHalfY - GapHalfY) / 100.f;

	const float SegGateCenterY = (GateHalfY + GapHalfY) * 0.5f;
	const float SegGateScaleY = (GateHalfY - GapHalfY) / 100.f;

	const float CorridorY = GapHalfY + (WallThickness * 0.5f);
	const float CorridorLenX = (GateX - StartBoundaryX);
	const float CorridorScaleX = CorridorLenX / 100.f;

	const TArray<FWallSpec> Walls = {
		// Block Start->Main boundary at X=-7000, leaving opening around Y=0
		{ FName("T66_Wall_StartExit_N1"), FVector(StartBoundaryX,  SegStartCenterY, WallZ), FVector(Thick, SegStartScaleY, Tall), FLinearColor(0.06f,0.06f,0.08f,1.f) },
		{ FName("T66_Wall_StartExit_S1"), FVector(StartBoundaryX, -SegStartCenterY, WallZ), FVector(Thick, SegStartScaleY, Tall), FLinearColor(0.06f,0.06f,0.08f,1.f) },

		// Corridor side walls from X=-7000 to X=-6000
		{ FName("T66_Wall_StartExit_SideN"), FVector(StartBoundaryX + (CorridorLenX * 0.5f),  CorridorY, WallZ), FVector(CorridorScaleX, Thick, Tall), FLinearColor(0.06f,0.06f,0.08f,1.f) },
		{ FName("T66_Wall_StartExit_SideS"), FVector(StartBoundaryX + (CorridorLenX * 0.5f), -CorridorY, WallZ), FVector(CorridorScaleX, Thick, Tall), FLinearColor(0.06f,0.06f,0.08f,1.f) },

		// Block corridor->main at X=-6000, leaving opening where pillars are
		{ FName("T66_Wall_StartExit_N2"), FVector(GateX,  SegGateCenterY, WallZ), FVector(Thick, SegGateScaleY, Tall), FLinearColor(0.06f,0.06f,0.08f,1.f) },
		{ FName("T66_Wall_StartExit_S2"), FVector(GateX, -SegGateCenterY, WallZ), FVector(Thick, SegGateScaleY, Tall), FLinearColor(0.06f,0.06f,0.08f,1.f) },
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FWallSpec& Spec : Walls)
	{
		if (HasTag(Spec.Tag)) continue;
		AStaticMeshActor* Wall = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Spec.Location, FRotator::ZeroRotator, SpawnParams);
		if (!Wall || !Wall->GetStaticMeshComponent()) continue;
		Wall->Tags.Add(Spec.Tag);
		Wall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Movable);
		Wall->SetActorScale3D(Spec.Scale);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Static);
		if (UMaterialInstanceDynamic* Mat = Wall->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Spec.Color);
		}
		SpawnedSetupActors.Add(Wall);
	}
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
		{ FName("T66_Floor_Main"),   MainCenter,          FVector(200.f, 200.f, 1.f), FLinearColor(0.30f, 0.30f, 0.35f, 1.f) },
		{ FName("T66_Floor_Start"),  StartCenter,         FVector(60.f,  60.f,  1.f), FLinearColor(0.22f, 0.24f, 0.28f, 1.f) },
		{ FName("T66_Floor_Boss"),   BossCenter,          FVector(60.f,  60.f,  1.f), FLinearColor(0.26f, 0.22f, 0.22f, 1.f) },
		{ FName("T66_Floor_Conn1"),  StartConnectorCenter, FVector(20.f, 30.f, 1.f), FLinearColor(0.25f, 0.25f, 0.28f, 1.f) },
		{ FName("T66_Floor_Conn2"),  BossConnectorCenter,  FVector(20.f, 30.f, 1.f), FLinearColor(0.25f, 0.25f, 0.28f, 1.f) },
	};

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
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

		if (UMaterialInstanceDynamic* FloorMat = Floor->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
		{
			FloorMat->SetVectorParameterValue(TEXT("BaseColor"), Spec.Color);
		}

		if (!SpawnedSetupActors.Contains(Floor))
		{
			SpawnedSetupActors.Add(Floor);
		}
	}
}

void AT66GameMode::SpawnBoundaryWallsIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
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

	// Walls around the entire playable footprint (start + main + boss).
	static constexpr float FloorTopZ = 0.f;      // dev floors sit at Z=-50 with thickness 100 => top at 0
	static constexpr float WallHeight = 1500.f;
	static constexpr float WallThickness = 200.f;

	static constexpr float StartBossHalf = 3000.f; // start/boss floors are 6000 wide
	static constexpr float TotalHalfX = 10000.f + StartBossHalf; // start center -10000, boss center +10000
	static constexpr float MainHalfY = 10000.f; // main floor is 20000 wide
	static constexpr float TotalHalfY = MainHalfY;

	const float WallZ = FloorTopZ + (WallHeight * 0.5f);

	struct FWallSpec
	{
		FName Tag;
		FVector Location;
		FVector Scale; // in AStaticMeshActor scale units (cube is 100uu)
		FLinearColor Color;
	};

	const float LongX = (TotalHalfX * 2.f + WallThickness) / 100.f;
	const float LongY = (TotalHalfY * 2.f + WallThickness) / 100.f;
	const float Thick = WallThickness / 100.f;
	const float Tall = WallHeight / 100.f;

	TArray<FWallSpec> Walls = {
		{ FName("T66_Wall_N"), FVector(0.f,  TotalHalfY + (WallThickness * 0.5f), WallZ), FVector(LongX, Thick, Tall), FLinearColor(0.08f,0.08f,0.10f,1.f) },
		{ FName("T66_Wall_S"), FVector(0.f, -TotalHalfY - (WallThickness * 0.5f), WallZ), FVector(LongX, Thick, Tall), FLinearColor(0.08f,0.08f,0.10f,1.f) },
		{ FName("T66_Wall_E"), FVector( TotalHalfX + (WallThickness * 0.5f), 0.f, WallZ), FVector(Thick, LongY, Tall), FLinearColor(0.08f,0.08f,0.10f,1.f) },
		{ FName("T66_Wall_W"), FVector(-TotalHalfX - (WallThickness * 0.5f), 0.f, WallZ), FVector(Thick, LongY, Tall), FLinearColor(0.08f,0.08f,0.10f,1.f) },
	};

	// Critical: the main square ends at X=±8000, but the overall footprint extends to X=±13000 due to the start/boss squares.
	// Without extra "main edge" walls, the player can walk to the east/west edges of the main square and fall off.
	{
		static constexpr float MainHalfX = 10000.f;
		const float MainEdgeX = MainHalfX + (WallThickness * 0.5f);
		const float GapY = StartBossHalf; // keep the boss/start overlap corridor (|Y| <= 3000) open
		const float SegLenY = (MainHalfY - GapY);
		const float SegScaleY = SegLenY / 100.f;
		const float CenterY = (GapY + MainHalfY) * 0.5f;

		// North segments (+Y)
		Walls.Add({ FName("T66_Wall_MainEdge_E_N"), FVector( MainEdgeX,  CenterY, WallZ), FVector(Thick, SegScaleY, Tall), FLinearColor(0.07f,0.07f,0.09f,1.f) });
		Walls.Add({ FName("T66_Wall_MainEdge_W_N"), FVector(-MainEdgeX,  CenterY, WallZ), FVector(Thick, SegScaleY, Tall), FLinearColor(0.07f,0.07f,0.09f,1.f) });

		// South segments (-Y)
		Walls.Add({ FName("T66_Wall_MainEdge_E_S"), FVector( MainEdgeX, -CenterY, WallZ), FVector(Thick, SegScaleY, Tall), FLinearColor(0.07f,0.07f,0.09f,1.f) });
		Walls.Add({ FName("T66_Wall_MainEdge_W_S"), FVector(-MainEdgeX, -CenterY, WallZ), FVector(Thick, SegScaleY, Tall), FLinearColor(0.07f,0.07f,0.09f,1.f) });
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FWallSpec& Spec : Walls)
	{
		AStaticMeshActor* Wall = FindTaggedActor(Spec.Tag);
		if (!Wall)
		{
			Wall = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Spec.Location, FRotator::ZeroRotator, SpawnParams);
		}
		if (!Wall || !Wall->GetStaticMeshComponent()) continue;

		if (!Wall->Tags.Contains(Spec.Tag))
		{
			Wall->Tags.Add(Spec.Tag);
		}
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Movable);
		Wall->SetActorLocation(Spec.Location);
		Wall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		Wall->SetActorScale3D(Spec.Scale);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Static);

		if (UMaterialInstanceDynamic* Mat = Wall->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Spec.Color);
		}

		if (!SpawnedSetupActors.Contains(Wall))
		{
			SpawnedSetupActors.Add(Wall);
		}
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
		// - Normal stage: start area (so timer starts after passing start pillars)
		// - Coliseum mode: coliseum arena (timer starts immediately; no pillars)
		const FVector SpawnLoc = IsColiseumStage()
			? FVector(ColiseumCenter.X, ColiseumCenter.Y, DefaultSpawnHeight)
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
			UE_LOG(LogTemp, Log, TEXT("Spawned development PlayerStart at %s"), *SpawnLoc.ToString());
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
		SpawnLocation = IsColiseumStage()
			? FVector(ColiseumCenter.X, ColiseumCenter.Y, 200.f)  // Coliseum: spawn in arena
			: FVector(-10000.f, 0.f, 200.f);   // Gameplay: spawn in starting area
		UE_LOG(LogTemp, Warning, TEXT("No PlayerStart found! Spawning at default location (%.0f, %.0f, %.0f)."),
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
	}

	// Robust: always ensure Gameplay spawns in the Start Area regardless of where a PlayerStart was placed.
	// (Coliseum spawns in the Main Area and starts timer immediately.)
	if (!IsColiseumStage())
	{
		SpawnLocation = FVector(-10000.f, 0.f, 200.f);
		SpawnRotation = FRotator::ZeroRotator;

		// Difficulty Boost: spawn on the Boost platform instead of the normal Start Area.
		if (UT66GameInstance* T66GI = GetT66GameInstance())
		{
			if (T66GI->bStageBoostPending)
			{
				SpawnLocation = FVector(-10000.f, 5200.f, 200.f);
			}
			else
			{
				// Tutorial Arena spawn:
				// - forced by dev switch, OR
				// - first-time player (profile flag not completed yet)
				bool bShouldTutorialSpawn = bForceSpawnInTutorialArea;
				if (!bShouldTutorialSpawn)
				{
					if (UGameInstance* GI = GetGameInstance())
					{
						if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
						{
							bShouldTutorialSpawn = !Ach->HasCompletedTutorial();
						}
					}
				}

				if (bShouldTutorialSpawn)
				{
					// Center is (0, 7500). Spawn near the "south" side so the player faces into the space.
					SpawnLocation = FVector(-1600.f, 6100.f, 200.f);
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
			if (T66GI->bForceColiseumMode)
			{
				SpawnLocation = ColiseumCenter;
				SpawnRotation = FRotator::ZeroRotator;
			}
		}
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
		// Snap to ground so the hero is never floating or sunk.
		if (UWorld* World = GetWorld())
		{
			FHitResult Hit;
			const FVector Start = Hero->GetActorLocation() + FVector(0.f, 0.f, 2000.f);
			const FVector End = Hero->GetActorLocation() - FVector(0.f, 0.f, 6000.f);
			if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
			{
				const float HalfHeight = Hero->GetCapsuleComponent() ? Hero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.f;
				Hero->SetActorLocation(Hit.ImpactPoint + FVector(0.f, 0.f, HalfHeight), false, nullptr, ETeleportType::TeleportPhysics);
			}
		}

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
