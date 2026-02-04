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
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66SkillRatingSubsystem.h"
#include "Gameplay/T66RecruitableCompanion.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/LightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "EngineUtils.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "Gameplay/T66ProceduralLandscapeGenerator.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeEdit.h"
#include "T66.h"

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
		// Per design: companions unlock only for stages 1..60 excluding checkpoint stages (ending in 0/5).
		return StageNum >= 1 && StageNum <= 60 && (StageNum % 5) != 0;
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
		return Count; // 1..48
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
}

AT66GameMode::AT66GameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	// Default to our hero base class
	DefaultPawnClass = AT66HeroBase::StaticClass();
	DefaultHeroClass = AT66HeroBase::StaticClass();

	// Coliseum arena lives inside GameplayLevel, off to the side (walled off).
	ColiseumCenter = FVector(-20000.f, -10400.f, 200.f);

	// Default ground materials (4 rotation variants); pick one per floor by position
	GroundFloorMaterials.Empty();
	GroundFloorMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Ground/M_GroundAtlas_2x2_R0.M_GroundAtlas_2x2_R0"))));
	GroundFloorMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Ground/M_GroundAtlas_2x2_R90.M_GroundAtlas_2x2_R90"))));
	GroundFloorMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Ground/M_GroundAtlas_2x2_R180.M_GroundAtlas_2x2_R180"))));
	GroundFloorMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Ground/M_GroundAtlas_2x2_R270.M_GroundAtlas_2x2_R270"))));
}

void AT66GameMode::BeginPlay()
{
	Super::BeginPlay();

	// Procedural hills terrain: apply heightfield from GameInstance seed if level has a Landscape
	GenerateProceduralTerrainIfNeeded();

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
	GetWorld()->SpawnActor<AT66MiasmaBoundary>(AT66MiasmaBoundary::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
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
	if (T66GI->bStageBoostPending) return;

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

	// Main map square bounds (centered at 0,0). Keep some margin from edges. (4x area = 2x linear)
	static constexpr float MainHalfExtent = 18400.f;
	static constexpr float SpawnZ = 40.f;
	static constexpr float MinDistBetweenTiles = 420.f;
	static constexpr float SafeBubbleMargin = 350.f;
	static constexpr int32 TileCount = 12;

	TArray<FVector> UsedLocs;

	// No-spawn zones: keep gameplay spawns out of start box (inside main) and special arenas.
	auto IsInsideNoSpawnZone = [&](const FVector& L) -> bool
	{
		// Start area: square inside main map X -17600..-13600, Y -2000..2000 (gap from miasma wall).
		static constexpr float StartBoxWest = -17600.f, StartBoxEast = -13600.f;
		static constexpr float StartBoxNorth = 2000.f, StartBoxSouth = -2000.f;
		static constexpr float StartMargin = 200.f;
		if (L.X >= (StartBoxWest - StartMargin) && L.X <= (StartBoxEast + StartMargin) &&
		    L.Y >= (StartBoxSouth - StartMargin) && L.Y <= (StartBoxNorth + StartMargin))
		{
			return true;
		}

		// Boost and Coliseum at Y=+15000; Tutorial to the side of main map (Y=+27000, separated by gap).
		static constexpr float ArenaHalf = 4000.f;
		static constexpr float ArenaMargin = 300.f;
		static constexpr float TutorialArenaHalf = 4000.f;
		struct FArena2D { float X; float Y; float Half; };
		static constexpr FArena2D Arenas[] = {
			{ -10000.f, 15000.f, ArenaHalf }, // Boost
			{      0.f, 15000.f, ArenaHalf }, // Coliseum
			{      0.f, 27000.f, TutorialArenaHalf }, // Tutorial (north of main, gap from Y=20000)
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

			// Skill Rating: time is driven here; damage is an input event from RunState.
			if (UT66SkillRatingSubsystem* Skill = GI->GetSubsystem<UT66SkillRatingSubsystem>())
			{
				// Track only during combat time (stage timer active) and not during last-stand invulnerability.
				Skill->SetTrackingActive(RunState->GetStageTimerActive() && !RunState->IsInLastStand());
				Skill->TickSkillRating(DeltaTime);
			}

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

	const float Scalar = RunState->GetDifficultyScalar();
	for (TActorIterator<AT66EnemyBase> It(GetWorld()); It; ++It)
	{
		if (AT66EnemyBase* E = *It)
		{
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
	const FVector BossGateLoc(12000.f, 0.f, 0.f);
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
	const FVector BossGateLoc(12000.f, 0.f, 0.f);
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

	// Idol Altar unlock: after bosses of stages ending in 5 or 0 (i.e. Stage % 5 == 0),
	// spawn an altar near the boss death so the player can pick an idol before leaving.
	if (RunState)
	{
		SpawnIdolAltarAtLocation(Location);
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

	// Start Gate at the opening of the start-area box (east side).
	const FVector SpawnLoc(-13100.f, 0.f, 0.f);
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
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;

	const int32 StageNum = RunState->GetCurrentStage();
	FRandomStream Rng(StageNum * 1337 + 42);

	// Main map square bounds (centered at 0,0). Keep some margin from walls. (4x area = 2x linear)
	static constexpr float MainHalfExtent = 18400.f;
	static constexpr float SpawnZ = 220.f;
	static constexpr float MinDistBetweenInteractables = 900.f;
	static constexpr float SafeBubbleMargin = 250.f;

	TArray<FVector> UsedLocs;

	// No-spawn zones: keep gameplay spawns out of start box (inside main) and special arenas.
	auto IsInsideNoSpawnZone = [&](const FVector& L) -> bool
	{
		// Start area: square inside main map X -17600..-13600, Y -2000..2000 (gap from miasma wall).
		static constexpr float StartBoxWest = -17600.f, StartBoxEast = -13600.f;
		static constexpr float StartBoxNorth = 2000.f, StartBoxSouth = -2000.f;
		static constexpr float StartMargin = 200.f;
		if (L.X >= (StartBoxWest - StartMargin) && L.X <= (StartBoxEast + StartMargin) &&
		    L.Y >= (StartBoxSouth - StartMargin) && L.Y <= (StartBoxNorth + StartMargin))
		{
			return true;
		}

		// Boost and Coliseum at Y=+15000; Tutorial to the side of main map (Y=+27000, separated by gap).
		static constexpr float ArenaHalf = 4000.f;
		static constexpr float ArenaMargin = 300.f;
		static constexpr float TutorialArenaHalf = 4000.f;
		struct FArena2D { float X; float Y; float Half; };
		static constexpr FArena2D Arenas[] = {
			{ -10000.f, 15000.f, ArenaHalf }, // Boost
			{      0.f, 15000.f, ArenaHalf }, // Coliseum
			{      0.f, 27000.f, TutorialArenaHalf }, // Tutorial (north of main, gap from Y=20000)
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

	if (RngSub)
	{
		RngSub->UpdateLuckStat(RunState->GetLuckStat());
	}

	const UT66RngTuningConfig* Tuning = RngSub ? RngSub->GetTuning() : nullptr;

	// Luck-affected counts (trees/trucks/wheels) use central tuning. Locations are still stage-seeded (not luck-affected).
	const int32 CountTrees = (RngSub && Tuning) ? RngSub->RollIntRangeBiased(Tuning->TreesPerStage, Rng) : Rng.RandRange(2, 5);
	const int32 CountTrucks = (RngSub && Tuning) ? RngSub->RollIntRangeBiased(Tuning->TrucksPerStage, Rng) : Rng.RandRange(2, 5);
	const int32 CountWheels = (RngSub && Tuning) ? RngSub->RollIntRangeBiased(Tuning->WheelsPerStage, Rng) : Rng.RandRange(2, 5);

	// Luck Rating tracking (quantity).
	if (RunState)
	{
		const int32 TreesMin = (Tuning ? Tuning->TreesPerStage.Min : 2);
		const int32 TreesMax = (Tuning ? Tuning->TreesPerStage.Max : 5);
		const int32 TrucksMin = (Tuning ? Tuning->TrucksPerStage.Min : 2);
		const int32 TrucksMax = (Tuning ? Tuning->TrucksPerStage.Max : 5);
		const int32 WheelsMin = (Tuning ? Tuning->WheelsPerStage.Min : 2);
		const int32 WheelsMax = (Tuning ? Tuning->WheelsPerStage.Max : 5);
		RunState->RecordLuckQuantityRoll(FName(TEXT("TreesPerStage")), CountTrees, TreesMin, TreesMax);
		RunState->RecordLuckQuantityRoll(FName(TEXT("TrucksPerStage")), CountTrucks, TrucksMin, TrucksMax);
		RunState->RecordLuckQuantityRoll(FName(TEXT("WheelsPerStage")), CountWheels, WheelsMin, WheelsMax);
	}

	// Not luck-affected (for now).
	const int32 CountTotems = Rng.RandRange(2, 5);

	for (int32 i = 0; i < CountTrees; ++i)
	{
		if (AT66TreeOfLifeInteractable* Tree = Cast<AT66TreeOfLifeInteractable>(SpawnOne(AT66TreeOfLifeInteractable::StaticClass())))
		{
			const FT66RarityWeights Weights = Tuning ? Tuning->InteractableRarityBase : FT66RarityWeights{};
			const ET66Rarity R = (RngSub && Tuning) ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
			Tree->SetRarity(R);
			if (RunState)
			{
				RunState->RecordLuckQualityRarity(FName(TEXT("TreeRarity")), R);
			}
		}
	}
	for (int32 i = 0; i < CountTrucks; ++i)
	{
		if (AT66CashTruckInteractable* Truck = Cast<AT66CashTruckInteractable>(SpawnOne(AT66CashTruckInteractable::StaticClass())))
		{
			const float MimicChance = Tuning ? FMath::Clamp(Tuning->TruckMimicChance, 0.f, 1.f) : 0.20f;
			Truck->bIsMimic = (Rng.GetFraction() < MimicChance); // explicitly not luck-affected

			const FT66RarityWeights Weights = Tuning ? Tuning->InteractableRarityBase : FT66RarityWeights{};
			const ET66Rarity R = (RngSub && Tuning) ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
			Truck->SetRarity(R);
			if (RunState)
			{
				RunState->RecordLuckQualityRarity(FName(TEXT("TruckRarity")), R);
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
	for (int32 i = 0; i < CountTotems; ++i)
	{
		if (AT66DifficultyTotem* Totem = Cast<AT66DifficultyTotem>(SpawnOne(AT66DifficultyTotem::StaticClass())))
		{
			// Not luck-affected (per current scope).
			Totem->SetRarity(FT66RarityUtil::RollDefaultRarity(Rng));
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

	// Place boost interactables on the main floor (no separate platform; hard rule: no overlapping grounds).
	const FVector PlatformCenter(-20000.f, 10400.f, -50.f);

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

	// Map layout: spawn the stage boss in the Boss Area square (right after pillars, inside safe zone).
	StageData.BossSpawnLocation = FVector(15600.f, 0.f, 200.f);

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
	
	SpawnFloorIfNeeded();
	SpawnColiseumArenaIfNeeded();
	SpawnTutorialArenaIfNeeded();
	TryApplyGroundFloorMaterialToAllFloors();
	SpawnStartAreaWallsIfNeeded();
	SpawnBossAreaWallsIfNeeded();
	SpawnLightingIfNeeded();
	SpawnPlayerStartIfNeeded();
}

void AT66GameMode::TryApplyGroundFloorMaterialToAllFloors()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (GroundFloorMaterials.Num() < 4)
	{
		return;
	}

	// Check if all 4 materials are loaded
	bool bAllLoaded = true;
	for (int32 i = 0; i < 4; ++i)
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
			for (int32 i = 0; i < 4 && i < GroundFloorMaterials.Num(); ++i)
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
			const int32 Idx = FMath::Clamp(FMath::FloorToInt(FMath::Frac(FMath::Abs(Seed)) * 4.f), 0, 3);
			if (Idx < GroundFloorMaterials.Num())
			{
				UMaterialInterface* M = GroundFloorMaterials[Idx].Get();
				if (M) Mat = M;
			}
			SMC->SetMaterial(0, Mat);
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

	// Tutorial: to the side of the main map, separated by empty space (no overlapping ground).
	// Main map Y ends at 20000; tutorial at Y=27000 so gap of 7000.
	const FVector TutorialCenter(0.f, 27000.f, -50.f);
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
			Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
			T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Movable);
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

	// Coliseum arena: off to the side. No separate floor (hard rule: no overlapping grounds; main floor covers it).
}

void AT66GameMode::SpawnStartAreaWallsIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
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
	const float OpeningHalfY = 400.f;   // gap on east for Start Gate pillars
	const float EastNorthLen = BoxNorthY - OpeningHalfY;
	const float EastSouthLen = OpeningHalfY - BoxSouthY;
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
		Wall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Movable);
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

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!CubeMesh) return;

	auto HasTag = [&](FName Tag) -> bool
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
			if (It->Tags.Contains(Tag)) return true;
		return false;
	};

	// Boss area: right after the boss pillars, fully inside safe zone with gap from miasma (|X|<=18400).
	// Square 4000x4000: X 13600..17600 (800 uu gap from miasma at 18400), Y -2000..2000. Center (15600, 0).
	static constexpr float FloorTopZ = 0.f;
	static constexpr float WallHeightShort = 80.f;
	static constexpr float WallThickness = 120.f;
	const float WallZ = FloorTopZ + (WallHeightShort * 0.5f);
	const float Thick = WallThickness / 100.f;
	const float Tall = WallHeightShort / 100.f;
	const FLinearColor Red(0.75f, 0.12f, 0.12f, 1.f);

	const float BoxWestX = 13600.f;
	const float BoxEastX = 17600.f;
	const float BoxNorthY = 2000.f;
	const float BoxSouthY = -2000.f;
	const float BoxWidthX = BoxEastX - BoxWestX;
	const float BoxHeightY = BoxNorthY - BoxSouthY;
	const float BoxCenterX = 15600.f;

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
		Wall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Movable);
		Wall->SetActorScale3D(Spec.Scale);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Static);
		if (UMaterialInstanceDynamic* Mat = Wall->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Red);
		}
		SpawnedSetupActors.Add(Wall);
	}
}

void AT66GameMode::SpawnFloorIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Preload all 4 ground materials during level setup so floors get grass texture immediately.
	for (int32 i = 0; i < GroundFloorMaterials.Num() && i < 4; ++i)
	{
		if (!GroundFloorMaterials[i].IsNull() && !GroundFloorMaterials[i].Get())
		{
			GroundFloorMaterials[i].LoadSynchronous();
		}
	}

	// Hard rule: no overlapping grounds (causes z-fighting). One main run floor; Tutorial separate with gap.
	// Main: single rectangle X -20000..+26000, Y -20000..+20000 (covers start area + main + boss).
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
	DestroyTaggedIfExists(FName("T66_Floor_Conn1"));
	DestroyTaggedIfExists(FName("T66_Floor_Conn2"));
	DestroyTaggedIfExists(FName("T66_Floor_Start"));  // Start is inside main; no separate patch
	DestroyTaggedIfExists(FName("T66_Floor_Boss"));   // Boss area is same main floor; no overlap

	// Main floor only: center (3000,0) so X -20000..+26000, Y -20000..+20000 (cube 100uu => scale 460,400).
	const FVector MainCenter(3000.f, 0.f, -50.f);
	const TArray<FFloorSpec> Floors = {
		{ FName("T66_Floor_Main"), MainCenter, FVector(460.f, 400.f, 1.f), FLinearColor(0.30f, 0.30f, 0.35f, 1.f) },
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

		// Pick one of 4 ground material variants by floor position (deterministic variety)
		UMaterialInterface* GroundMat = nullptr;
		if (GroundFloorMaterials.Num() >= 4)
		{
			const float Seed = Spec.Location.X * 0.000123f + Spec.Location.Y * 0.000456f;
			const int32 Idx = FMath::Clamp(FMath::FloorToInt(FMath::Frac(FMath::Abs(Seed)) * 4.f), 0, 3);
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
				for (int32 i = 0; i < GroundFloorMaterials.Num() && i < 4; ++i)
				{
					if (!GroundFloorMaterials[i].IsNull()) bAnyToLoad = true;
				}
				if (bAnyToLoad)
				{
					bGroundFloorMaterialLoadRequested = true;
					TArray<FSoftObjectPath> Paths;
					for (int32 i = 0; i < 4 && i < GroundFloorMaterials.Num(); ++i)
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

void AT66GameMode::SpawnLightingIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

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
				LightComp->SetIntensity(3.f);
				LightComp->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f)); // Warm sunlight

				// Drive SkyAtmosphere sun/sky scattering (mid-day blue sky).
				LightComp->bAtmosphereSunLight = true;
				LightComp->AtmosphereSunLightIndex = 0;
			}
			#if WITH_EDITOR
			Sun->SetActorLabel(TEXT("DEV_Sun"));
			#endif
			SpawnedSetupActors.Add(Sun);
			SunForAtmosphere = Sun;
			UE_LOG(LogTemp, Log, TEXT("Spawned development directional light"));
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
			if (USkyLightComponent* SkyComp = Sky->GetLightComponent())
			{
				SkyComp->SetIntensity(1.0f);
				// Keep the sky light neutral; the blue tint should come from the actual sky capture.
				SkyComp->SetLightColor(FLinearColor::White);
			}
			#if WITH_EDITOR
			Sky->SetActorLabel(TEXT("DEV_SkyLight"));
			#endif
			SpawnedSetupActors.Add(Sky);
			SkyForCapture = Sky;
			UE_LOG(LogTemp, Log, TEXT("Spawned development sky light"));
		}
	}

	// If we have an existing sun, ensure it drives the atmosphere (even if it was authored in the map).
	if (SunForAtmosphere)
	{
		if (UDirectionalLightComponent* LightComp = Cast<UDirectionalLightComponent>(SunForAtmosphere->GetLightComponent()))
		{
			LightComp->bAtmosphereSunLight = true;
			LightComp->AtmosphereSunLightIndex = 0;
		}
	}

	// Capture after SkyAtmosphere exists and sun is configured, so the skylight picks up a blue mid-day sky.
	if (SkyForCapture)
	{
		if (USkyLightComponent* SkyComp = SkyForCapture->GetLightComponent())
		{
			SkyComp->RecaptureSky();
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
			: FVector(-15600.f, 0.f, DefaultSpawnHeight);

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

void AT66GameMode::GenerateProceduralTerrainIfNeeded()
{
#if WITH_EDITOR
	UT66GameInstance* GI = GetT66GameInstance();
	if (!GI)
	{
		UE_LOG(LogT66, Warning, TEXT("[MAP] GenerateProceduralTerrainIfNeeded: no T66GameInstance"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogT66, Warning, TEXT("[MAP] GenerateProceduralTerrainIfNeeded: no World"));
		return;
	}

	ALandscape* Landscape = nullptr;
	for (TActorIterator<ALandscape> It(World); It; ++It)
	{
		Landscape = *It;
		break;
	}
	if (!Landscape)
	{
		UE_LOG(LogT66, Log, TEXT("[MAP] GenerateProceduralTerrainIfNeeded: no Landscape in level, skipping"));
		return;
	}

	ULandscapeInfo* Info = Landscape->GetLandscapeInfo();
	if (!Info)
	{
		UE_LOG(LogT66, Error, TEXT("[MAP] GenerateProceduralTerrainIfNeeded: Landscape has no LandscapeInfo"));
		return;
	}

	FIntRect Extent;
	if (!Info->GetLandscapeExtent(Extent))
	{
		UE_LOG(LogT66, Error, TEXT("[MAP] GenerateProceduralTerrainIfNeeded: GetLandscapeExtent failed"));
		return;
	}

	const int32 SizeX = Extent.Width() + 1;
	const int32 SizeY = Extent.Height() + 1;
	if (SizeX < 2 || SizeY < 2)
	{
		UE_LOG(LogT66, Error, TEXT("[MAP] GenerateProceduralTerrainIfNeeded: invalid extent size %dx%d"), SizeX, SizeY);
		return;
	}

	FT66ProceduralLandscapeParams Params;
	Params.Seed = GI->ProceduralTerrainSeed;
	Params.SizePreset = (SizeX <= 505 && SizeY <= 505) ? ET66LandscapeSizePreset::Small : ET66LandscapeSizePreset::Large;

	TArray<float> Heights;
	if (!FT66ProceduralLandscapeGenerator::GenerateHeightfield(
		Params, SizeX, SizeY, FT66ProceduralLandscapeGenerator::DefaultQuadSizeUU, Heights))
	{
		UE_LOG(LogT66, Error, TEXT("[MAP] GenerateProceduralTerrainIfNeeded: GenerateHeightfield failed"));
		return;
	}

	TArray<uint16> HeightData;
	const float ScaleZ = FMath::Max(Landscape->GetActorScale3D().Z, 1.f);
	FT66ProceduralLandscapeGenerator::FloatsToLandscapeHeightData(Heights, ScaleZ, HeightData);

	if (HeightData.Num() != SizeX * SizeY)
	{
		UE_LOG(LogT66, Error, TEXT("[MAP] GenerateProceduralTerrainIfNeeded: height data size mismatch"));
		return;
	}

	const int32 MinX = Extent.Min.X;
	const int32 MinY = Extent.Min.Y;
	const int32 MaxX = Extent.Max.X;
	const int32 MaxY = Extent.Max.Y;

	FHeightmapAccessor<false> HeightmapAccessor(Info);
	TArray<uint16> RegionData;
	RegionData.SetNumUninitialized(SizeX * SizeY);
	for (int32 Jy = 0; Jy < SizeY; ++Jy)
	{
		for (int32 Ix = 0; Ix < SizeX; ++Ix)
		{
			RegionData[Jy * SizeX + Ix] = HeightData[Jy * SizeX + Ix];
		}
	}
	HeightmapAccessor.SetData(MinX, MinY, MaxX, MaxY, RegionData.GetData());
	HeightmapAccessor.Flush();

	UE_LOG(LogT66, Log, TEXT("[MAP] GenerateProceduralTerrainIfNeeded: applied procedural hills (seed=%d, %dx%d)"), Params.Seed, SizeX, SizeY);
#else
	// Runtime (packaged) build: landscape height editing is editor-only. Use level as saved or pre-generate in editor.
	UE_LOG(LogT66, Verbose, TEXT("[MAP] GenerateProceduralTerrainIfNeeded: skipped (editor-only API in packaged build)"));
#endif
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
		// No PlayerStart found - spawn at a safe default location
		SpawnLocation = IsColiseumStage()
			? FVector(ColiseumCenter.X, ColiseumCenter.Y, 200.f)  // Coliseum: spawn in arena
			: FVector(-15600.f, 0.f, 200.f);   // Gameplay: spawn inside start-area square
		UE_LOG(LogTemp, Warning, TEXT("No PlayerStart found! Spawning at default location (%.0f, %.0f, %.0f)."),
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
	}

	// Robust: always ensure Gameplay spawns in the Start Area regardless of where a PlayerStart was placed.
	// (Coliseum spawns in the Main Area and starts timer immediately.)
	if (!IsColiseumStage())
	{
		// Start area is a square inside main map (X -17600..-13600, Y ±2000). Spawn at center.
		SpawnLocation = FVector(-15600.f, 0.f, 200.f);
		SpawnRotation = FRotator::ZeroRotator;

		// Difficulty Boost: spawn on the Boost platform instead of the normal Start Area.
		if (UT66GameInstance* T66GI = GetT66GameInstance())
		{
			if (T66GI->bStageBoostPending)
			{
				SpawnLocation = FVector(-20000.f, 10400.f, 200.f);
			}
			else
			{
				// Tutorial Arena spawn: forced by dev switch only. Tutorial is north of main, separated by gap.
				if (bForceSpawnInTutorialArea)
				{
					SpawnLocation = FVector(-1600.f, 25000.f, 200.f);
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
