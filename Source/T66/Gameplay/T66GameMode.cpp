// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66CompanionBase.h"
#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66StartGate.h"
#include "Gameplay/T66StageGate.h"
#include "Gameplay/T66BossBase.h"
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
			if (T66GI->bIsStageTransition)
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

	// Spawn enemy director so waves spawn (3 every 15s, max 12)
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<AT66EnemyDirector>(AT66EnemyDirector::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	// Spawn the boss for the current stage (Stage Gate appears only after boss is killed)
	SpawnBossForCurrentStage();

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
		}
	}
}

void AT66GameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
	SpawnCompanionForPlayer(NewPlayer);
	SpawnVendorForPlayer(NewPlayer);
	SpawnStartGateForPlayer(NewPlayer);

	UT66GameInstance* GI = GetT66GameInstance();
	APawn* Pawn = NewPlayer ? NewPlayer->GetPawn() : nullptr;
	if (GI && Pawn && GI->bApplyLoadedTransform)
	{
		Pawn->SetActorTransform(GI->PendingLoadedTransform);
		GI->bApplyLoadedTransform = false;
		GI->PendingLoadedTransform = FTransform();
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

void AT66GameMode::SpawnStartGateForPlayer(AController* Player)
{
	APawn* HeroPawn = Player ? Player->GetPawn() : nullptr;
	if (!HeroPawn) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// Start Gate just in front of spawn so player walks through it to start the timer
	FVector SpawnLoc = HeroPawn->GetActorLocation() + FVector(150.f, 0.f, 0.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66StartGate* StartGate = World->SpawnActor<AT66StartGate>(AT66StartGate::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
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

	// Check if there's already a floor-like actor (large static mesh at ground level)
	bool bHasFloor = false;
	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		FVector Location = It->GetActorLocation();
		FVector Scale = It->GetActorScale3D();
		// Consider it a floor if it's near ground level and scaled large
		if (FMath::Abs(Location.Z) < 100.f && (Scale.X > 5.f || Scale.Y > 5.f))
		{
			bHasFloor = true;
			break;
		}
	}

	if (!bHasFloor)
	{
		UE_LOG(LogTemp, Log, TEXT("No floor found - spawning development floor"));

		// Spawn a large cube as floor
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(),
			FVector(0.f, 0.f, -50.f), // Slightly below ground level
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Floor)
		{
			// Set the mesh to a cube
			UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
			if (CubeMesh && Floor->GetStaticMeshComponent())
			{
				Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
				Floor->SetActorScale3D(FVector(100.f, 100.f, 1.f)); // Large flat floor
				
				// Create a simple gray material
				UMaterialInstanceDynamic* FloorMat = Floor->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0);
				if (FloorMat)
				{
					FloorMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.3f, 0.3f, 0.35f, 1.f));
				}
			}

			#if WITH_EDITOR
			Floor->SetActorLabel(TEXT("DEV_Floor"));
			#endif
			SpawnedSetupActors.Add(Floor);
			UE_LOG(LogTemp, Log, TEXT("Spawned development floor at (0, 0, -50)"));
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

		APlayerStart* Start = World->SpawnActor<APlayerStart>(
			APlayerStart::StaticClass(),
			FVector(0.f, 0.f, DefaultSpawnHeight),
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
		// No PlayerStart found - spawn at a safe default location above origin
		// This ensures the player doesn't fall through the void
		SpawnLocation = FVector(0.f, 0.f, 200.f);
		UE_LOG(LogTemp, Warning, TEXT("No PlayerStart found! Spawning at default location (0, 0, 200). Add a PlayerStart actor to GameplayLevel."));
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
