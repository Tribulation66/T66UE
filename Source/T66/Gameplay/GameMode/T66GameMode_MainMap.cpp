// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameModePrivate.h"

using namespace T66GameModePrivate;

static const FName T66TraversalBarrierTag(TEXT("T66_Map_TraversalBarrier"));
static const FName T66MapPlatformTag(TEXT("T66_Map_Platform"));
static const FName T66MapRampTag(TEXT("T66_Map_Ramp"));
static const FName T66FloorMainTag(TEXT("T66_Floor_Main"));

static void DestroyActorsWithTag(UWorld* World, FName Tag)
{
	if (!World)
	{
		return;
	}

	TArray<AActor*> ToDestroy;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->Tags.Contains(Tag))
		{
			ToDestroy.Add(*It);
		}
	}

	for (AActor* Actor : ToDestroy)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
}

bool AT66GameMode::TryGetMainMapStartAxes(FVector& OutCenter, FVector& OutInwardDirection, FVector& OutSideDirection, float& OutCellSize) const
{
	if (!bHasMainMapSpawnSurfaceLocation || MainMapStartPathSurfaceLocation.IsNearlyZero() || MainMapStartAnchorSurfaceLocation.IsNearlyZero())
	{
		return false;
	}

	const FVector Center = MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
		? MainMapSpawnSurfaceLocation
		: MainMapStartAreaCenterSurfaceLocation;
	const FVector Inward2D = (MainMapStartAnchorSurfaceLocation - MainMapStartPathSurfaceLocation).GetSafeNormal2D();
	if (Inward2D.IsNearlyZero())
	{
		return false;
	}

	float CellSize = 0.0f;
	if (IsUsingTowerMainMapLayout())
	{
		CellSize = CachedTowerMainMapLayout.PlacementCellSize;
	}
	else
	{
		const FT66MapPreset Preset = T66BuildMainMapPreset(GetT66GameInstance());
		const T66MainMapTerrain::FSettings Settings = T66MainMapTerrain::MakeSettings(Preset);
		CellSize = Settings.CellSize;
	}

	if (CellSize <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	OutCenter = Center;
	OutInwardDirection = FVector(Inward2D.X, Inward2D.Y, 0.f);
	OutSideDirection = FVector(-Inward2D.Y, Inward2D.X, 0.f);
	OutCellSize = CellSize;
	return true;
}

bool AT66GameMode::TryGetMainMapStartPlacementLocation(float SideCells, float InwardCells, FVector& OutLocation) const
{
	if (IsUsingTowerMainMapLayout())
	{
		return T66TowerMapTerrain::TryGetStartPlacementLocation(GetWorld(), CachedTowerMainMapLayout, SideCells, InwardCells, OutLocation);
	}

	FVector Center = FVector::ZeroVector;
	FVector InwardDirection = FVector::ZeroVector;
	FVector SideDirection = FVector::ZeroVector;
	float CellSize = 0.f;
	if (!TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize))
	{
		return false;
	}

	const FVector DesiredLocation = Center + (SideDirection * (SideCells * CellSize)) + (InwardDirection * (InwardCells * CellSize));
	OutLocation = DesiredLocation;

	UWorld* World = GetWorld();
	if (!World)
	{
		return true;
	}

	const FT66MapPreset Preset = T66BuildMainMapPreset(GetT66GameInstance());
	const T66MainMapTerrain::FSettings Settings = T66MainMapTerrain::MakeSettings(Preset);
	const float TraceStartZ = T66MainMapTerrain::GetTraceZ(Preset);
	const float TraceEndZ = T66MainMapTerrain::GetLowestCollisionBottomZ(Preset) - Settings.StepHeight;

	FHitResult Hit;
	if (World->LineTraceSingleByChannel(
		Hit,
		FVector(DesiredLocation.X, DesiredLocation.Y, TraceStartZ),
		FVector(DesiredLocation.X, DesiredLocation.Y, TraceEndZ),
		ECC_WorldStatic))
	{
		OutLocation = Hit.ImpactPoint;
	}
	else
	{
		OutLocation.Z = Center.Z;
	}

	return true;
}

bool AT66GameMode::TryFindRandomMainMapSurfaceLocation(int32 SeedOffset, FVector& OutLocation, float ExtraSafeBubbleMargin) const
{
	UWorld* World = GetWorld();
	if (!World || !T66UsesMainMapTerrainStage(World))
	{
		return false;
	}

	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!T66GI)
	{
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const FT66MapPreset MainMapPreset = T66BuildMainMapPreset(T66GI);
	const bool bTowerLayout = IsUsingTowerMainMapLayout();
	const T66MainMapTerrain::FSettings MainMapSettings = bTowerLayout
		? T66MainMapTerrain::FSettings{}
		: T66MainMapTerrain::MakeSettings(MainMapPreset);
	const float MainHalfExtent = bTowerLayout
		? 0.0f
		: FMath::Max(0.0f, MainMapSettings.HalfExtent - MainMapSettings.CellSize * 1.25f);
	const float TraceStartZ = bTowerLayout ? CachedTowerMainMapLayout.TraceStartZ : T66MainMapTerrain::GetTraceZ(MainMapPreset);
	const float TraceEndZ = bTowerLayout
		? CachedTowerMainMapLayout.TraceEndZ
		: T66MainMapTerrain::GetLowestCollisionBottomZ(MainMapPreset) - MainMapSettings.StepHeight;
	const float PlacementCellSize = bTowerLayout ? CachedTowerMainMapLayout.PlacementCellSize : MainMapSettings.CellSize;
	const float RoomReserveRadius = PlacementCellSize * T66MainMapRoomReserveRadiusCells;
	const float CorridorReserveRadius = PlacementCellSize * T66MainMapCorridorReserveRadiusCells;
	const float SafeBubbleMargin = 250.f + ExtraSafeBubbleMargin;

	const int32 RunSeed = T66EnsureRunSeed(T66GI);
	const int32 StageNum = RunState ? RunState->GetCurrentStage() : 1;
	FRandomStream Rng(RunSeed + StageNum * 1337 + SeedOffset);

	auto IsInsideReservedZone = [&](const FVector& Location) -> bool
	{
		if (!MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapStartAreaCenterSurfaceLocation) < FMath::Square(RoomReserveRadius))
		{
			return true;
		}
		if (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapBossAreaCenterSurfaceLocation) < FMath::Square(RoomReserveRadius))
		{
			return true;
		}
		if (!MainMapStartPathSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapStartPathSurfaceLocation) < FMath::Square(CorridorReserveRadius))
		{
			return true;
		}
		if (!MainMapStartAnchorSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapStartAnchorSurfaceLocation) < FMath::Square(CorridorReserveRadius))
		{
			return true;
		}
		if (!MainMapBossAnchorSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapBossAnchorSurfaceLocation) < FMath::Square(CorridorReserveRadius))
		{
			return true;
		}
		return false;
	};

	UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
	for (int32 Try = 0; Try < 60; ++Try)
	{
		FVector Candidate = FVector::ZeroVector;
		if (bTowerLayout)
		{
			if (!T66TowerMapTerrain::TryGetRandomGameplaySurfaceLocation(World, CachedTowerMainMapLayout, Rng, Candidate))
			{
				continue;
			}
		}
		else
		{
			const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);

			FHitResult Hit;
			if (!World->LineTraceSingleByChannel(
				Hit,
				FVector(X, Y, TraceStartZ),
				FVector(X, Y, TraceEndZ),
				ECC_WorldStatic))
			{
				continue;
			}
			if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
			{
				continue;
			}

			Candidate = Hit.ImpactPoint;
		}

		if (IsInsideReservedZone(Candidate))
		{
			continue;
		}

		bool bBlockedByNPC = false;
		if (Registry)
		{
			for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Registry->GetNPCs())
			{
				const AT66HouseNPCBase* NPC = WeakNPC.Get();
				if (!NPC)
				{
					continue;
				}
				const float Clearance = NPC->GetSafeZoneRadius() + SafeBubbleMargin;
				if (FVector::DistSquared2D(Candidate, NPC->GetActorLocation()) < FMath::Square(Clearance))
				{
					bBlockedByNPC = true;
					break;
				}
			}
			if (!bBlockedByNPC)
			{
				for (const TWeakObjectPtr<AT66CircusInteractable>& WeakCircus : Registry->GetCircuses())
				{
					const AT66CircusInteractable* Circus = WeakCircus.Get();
					if (!Circus)
					{
						continue;
					}
					const float Clearance = Circus->GetSafeZoneRadius() + SafeBubbleMargin;
					if (FVector::DistSquared2D(Candidate, Circus->GetActorLocation()) < FMath::Square(Clearance))
					{
						bBlockedByNPC = true;
						break;
					}
				}
			}
		}

		if (bBlockedByNPC)
		{
			continue;
		}

		OutLocation = Candidate;
		return true;
	}

	return false;
}

void AT66GameMode::TryActivateMainMapCombat()
{
	UWorld* World = GetWorld();
	if (!World || !T66UsesMainMapTerrainStage(World) || !bTerrainCollisionReady || bMainMapCombatStarted)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	if (MainMapStartAnchorSurfaceLocation.IsNearlyZero() || MainMapStartPathSurfaceLocation.IsNearlyZero())
	{
		return;
	}

	if (IsUsingTowerMainMapLayout())
	{
		bool bAnyPlayerEnteredGameplayFloor = false;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			const APlayerController* PC = It->Get();
			const APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
			if (!PlayerPawn)
			{
				continue;
			}

			const int32 FloorNumber = GetTowerFloorIndexForLocation(PlayerPawn->GetActorLocation());
			if (FloorNumber >= CachedTowerMainMapLayout.FirstGameplayFloorNumber)
			{
				bAnyPlayerEnteredGameplayFloor = true;
				break;
			}
		}

		if (!bAnyPlayerEnteredGameplayFloor)
		{
			return;
		}
	}
	else
	{
	const FVector2D EntryMidpoint = FVector2D(
		(MainMapStartAnchorSurfaceLocation.X + MainMapStartPathSurfaceLocation.X) * 0.5f,
		(MainMapStartAnchorSurfaceLocation.Y + MainMapStartPathSurfaceLocation.Y) * 0.5f);
	const FVector2D EntryNormal = FVector2D(
		MainMapStartAnchorSurfaceLocation.X - MainMapStartPathSurfaceLocation.X,
		MainMapStartAnchorSurfaceLocation.Y - MainMapStartPathSurfaceLocation.Y).GetSafeNormal();
	if (EntryNormal.IsNearlyZero())
	{
		return;
	}

	bool bAnyPlayerEnteredCombat = false;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PC = It->Get();
		const APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
		if (!PlayerPawn)
		{
			continue;
		}

		const FVector PlayerLocation = PlayerPawn->GetActorLocation();
		const float SignedDistance = FVector2D::DotProduct(FVector2D(PlayerLocation.X, PlayerLocation.Y) - EntryMidpoint, EntryNormal);
		if (SignedDistance >= 0.f)
		{
			bAnyPlayerEnteredCombat = true;
			break;
		}
	}

	if (!bAnyPlayerEnteredCombat)
	{
		return;
	}
	}

	const bool bSkipStandardEnemySpawning = IsUsingTowerMainMapLayout() && IsBossRushFinaleStage();

	RunState->ResetStageTimerToFull();
	if (!bWorldInteractablesSpawnedForStage)
	{
		SpawnWorldInteractablesForStage();
	}
	RunState->SetStageTimerActive(true);

	if (IsUsingTowerMainMapLayout())
	{
		T66PauseTowerMiasma(MiasmaManager);
		T66DestroyMiasmaBoundaryActors(World);
	}
	else
	{
		if (!IsValid(MiasmaManager))
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			MiasmaManager = World->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (MiasmaManager)
			{
				MiasmaManager->RebuildForCurrentStage();
			}
		}

		if (MiasmaManager)
		{
			T66ActivateStageMiasma(MiasmaManager);
			MiasmaManager->UpdateFromRunState();
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		T66EnsureMiasmaBoundaryActor(World, SpawnParams);
	}

	if (!bSkipStandardEnemySpawning && !EnsureEnemyDirector(World))
	{
		return;
	}

	bMainMapCombatStarted = true;
	UE_LOG(LogT66GameMode, Log, TEXT("T66GameMode - Main map combat activated."));
}

void AT66GameMode::EnsureLevelSetup()
{
	UE_LOG(LogT66GameMode, Log, TEXT("Checking level setup..."));

	// Destroy any Landscape or editor-placed foliage actors saved in the level (legacy from external asset packs).
	UWorld* CleanupWorld = GetWorld();
	if (CleanupWorld)
	{
		for (TActorIterator<ALandscape> It(CleanupWorld); It; ++It)
		{
			UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Destroying saved Landscape actor: %s"), *It->GetName());
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
			UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Destroyed %d legacy foliage actors."), ToDestroy.Num());
		}
	}

	if (IsLabLevel())
	{
		SpawnLabFloorIfNeeded();
		FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(GetWorld());
		SpawnLabCollectorIfNeeded();
		SpawnPlayerStartIfNeeded();
		return;
	}

	SpawnFloorIfNeeded();
	SpawnTutorialArenaIfNeeded();
	SpawnStartAreaWallsIfNeeded();
	if (!T66UsesMainMapTerrainStage(CleanupWorld))
	{
		SpawnBossAreaWallsIfNeeded();
	}
	TryApplyGroundFloorMaterialToAllFloors();
	FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(GetWorld());
	SpawnPlayerStartIfNeeded();
}

void AT66GameMode::TryApplyGroundFloorMaterialToAllFloors()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const UT66GameInstance* T66GI = GetT66GameInstance();
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	UMaterialInterface* DifficultyFloorMaterial = FT66TerrainThemeAssets::ResolveDifficultyGroundMaterial(this, Difficulty);
	if (!DifficultyFloorMaterial)
	{
		return;
	}

	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		AStaticMeshActor* A = *It;
		if (!A) continue;
		if (!T66_HasAnyFloorTag(A)) continue;
		if (UStaticMeshComponent* SMC = A->GetStaticMeshComponent())
		{
			SMC->SetMaterial(0, DifficultyFloorMaterial);
		}
	}

	// Do not touch the runtime main-map terrain actor here; it owns its own
	// difficulty-specific block/slope/dirt materials and this pass used to
	// flatten the entire terrain into one old ground material on first load.
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* A = *It;
		if (!A || Cast<AStaticMeshActor>(A)) continue;
		if (!T66_HasAnyFloorTag(A)) continue;
		if (A->ActorHasTag(T66MainMapTerrainVisualTag)) continue;

		TArray<UMeshComponent*> MeshComps;
		A->GetComponents<UMeshComponent>(MeshComps);
		for (UMeshComponent* MC : MeshComps)
		{
			if (MC)
			{
				MC->SetMaterial(0, DifficultyFloorMaterial);
			}
		}
	}
}

void AT66GameMode::SpawnTutorialArenaIfNeeded()
{
	// Tutorial Arena is an enclosed side-area inside GameplayLevel. It is only relevant for normal gameplay stages.
	UWorld* World = GetWorld();
	if (!World) return;
	if (!T66IsStandaloneTutorialMap(World)) return;

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	auto HasTag = [&](FName Tag) -> bool
	{
		return T66FindTaggedActor(World, Tag) != nullptr;
	};

	FVector AuthoredMapMarkerLocation = FVector::ZeroVector;
	FRotator AuthoredMapMarkerRotation = FRotator::ZeroRotator;
	if (T66TryGetTaggedActorTransform(World, FName(TEXT("T66_Tutorial_MapReady")), AuthoredMapMarkerLocation, AuthoredMapMarkerRotation))
	{
		return;
	}

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
			T66RememberTaggedActor(Floor, FloorTag);
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
		if (AActor* ExistingWall = T66FindTaggedActor(World, Tag))
		{
			ExistingWall->Destroy();
			T66ForgetTaggedActor(World, Tag);
		}
	}
	return;

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	auto HasTag = [&](FName Tag) -> bool
	{
		return T66FindTaggedActor(World, Tag) != nullptr;
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

	if (T66UsesMainMapTerrainStage(World))
	{
		static const FName BossWallTags[] = {
			FName("T66_Wall_Boss_W"),
			FName("T66_Wall_Boss_E"),
			FName("T66_Wall_Boss_N"),
			FName("T66_Wall_Boss_S")
		};
		for (FName Tag : BossWallTags)
		{
			if (AActor* ExistingWall = T66FindTaggedActor(World, Tag))
			{
				ExistingWall->Destroy();
				T66ForgetTaggedActor(World, Tag);
			}
		}
		return;
	}

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	auto HasTag = [&](FName Tag) -> bool
	{
		return T66FindTaggedActor(World, Tag) != nullptr;
	};

	const UT66GameInstance* T66GI = GetT66GameInstance();
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	UMaterialInterface* FloorMat = FT66TerrainThemeAssets::ResolveDifficultyGroundMaterial(this, Difficulty);

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
		T66RememberTaggedActor(Wall, Spec.Tag);
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

	// Main run uses dedicated runtime terrain. Helper floors are spawned explicitly for catch-up and tutorial content.
	const TArray<FName> CleanupTags = {
		FName("T66_Floor_Conn1"),
		FName("T66_Floor_Conn2"),
		FName("T66_Floor_Start"),
		FName("T66_Floor_Boss"),
		FName("T66_Floor_CatchUp"),
		FName("T66_Floor_Tutorial")
	};

	for (const FName& CleanupTag : CleanupTags)
	{
		if (AActor* ExistingFloor = T66FindTaggedActor(World, CleanupTag))
		{
			ExistingFloor->Destroy();
			T66ForgetTaggedActor(World, CleanupTag);
		}
	}
	// T66_Floor_Main is spawned by SpawnMainMapTerrain(); do not destroy it here.

	if (T66UsesMainMapTerrainStage(World) || T66IsStandaloneTutorialMap(World))
	{
		return;
	}
}

void AT66GameMode::SpawnMainMapTerrain()
{
	UWorld* World = GetWorld();
	if (!World || IsLabLevel() || !T66UsesMainMapTerrainStage(World))
	{
		return;
	}

	TArray<AActor*> CleanupActors;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor)
		{
			continue;
		}

		if (Actor->Tags.Contains(T66MapPlatformTag)
			|| Actor->Tags.Contains(T66MapRampTag)
			|| Actor->Tags.Contains(T66FloorMainTag)
			|| Actor->Tags.Contains(T66TraversalBarrierTag)
			|| T66_HasAnyFloorTag(Actor))
		{
			CleanupActors.Add(Actor);
		}
	}

	for (AActor* Actor : CleanupActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}

	bTerrainCollisionReady = false;
	bMainMapCombatStarted = false;
	bWorldInteractablesSpawnedForStage = false;
	bHasMainMapSpawnSurfaceLocation = false;
	bUsingTowerMainMapLayout = false;
	CachedTowerMainMapLayout = T66TowerMapTerrain::FLayout{};
	bTowerBossEntryTriggered = false;
	bTowerBossEntryApplied = false;
	TowerTerrainSafetyAccumulator = 0.f;
	TowerTrapActivationAccumulator = 0.f;
	ActiveTowerTrapFloorNumber = INDEX_NONE;
	MainMapSpawnSurfaceLocation = FVector::ZeroVector;
	MainMapStartAnchorSurfaceLocation = FVector::ZeroVector;
	MainMapStartPathSurfaceLocation = FVector::ZeroVector;
	MainMapStartAreaCenterSurfaceLocation = FVector::ZeroVector;
	MainMapBossAnchorSurfaceLocation = FVector::ZeroVector;
	MainMapBossSpawnSurfaceLocation = FVector::ZeroVector;
	MainMapBossBeaconSurfaceLocation = FVector::ZeroVector;
	MainMapBossAreaCenterSurfaceLocation = FVector::ZeroVector;
	MainMapRescueAnchorLocations.Reset();
	for (AT66TowerDescentHole* Hole : TowerDescentHoles)
	{
		if (Hole)
		{
			Hole->Destroy();
		}
	}
	TowerDescentHoles.Reset();

	UT66GameInstance* GI = GetT66GameInstance();
	const FT66MapPreset Preset = T66BuildMainMapPreset(GI);
	const T66MainMapTerrain::FSettings MainMapSettings = T66MainMapTerrain::MakeSettings(Preset);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Generating main map terrain (layout=%s, seed=%d, hilliness=%.2f, grid=%d, cell=%.0f, step=%.0f, scale=%.2f)"),
		T66GetMainMapLayoutVariantLabel(),
		Preset.Seed,
		Preset.FarmHilliness,
		MainMapSettings.BoardSize,
		MainMapSettings.CellSize,
		MainMapSettings.StepHeight,
		MainMapSettings.BoardScale);

	if (Preset.LayoutVariant == ET66MainMapLayoutVariant::Tower)
	{
		if (!T66TowerMapTerrain::BuildLayout(Preset, CachedTowerMainMapLayout, IsBossRushFinaleStage()))
		{
			UE_LOG(LogT66GameMode, Error, TEXT("[MAP] Tower main map layout generation failed (seed=%d)"), Preset.Seed);
			CachedTowerMainMapLayout = T66TowerMapTerrain::FLayout{};
			return;
		}

		bUsingTowerMainMapLayout = true;
		MainMapSpawnSurfaceLocation = CachedTowerMainMapLayout.SpawnSurfaceLocation;
		bHasMainMapSpawnSurfaceLocation = !MainMapSpawnSurfaceLocation.IsNearlyZero();
		MainMapStartAnchorSurfaceLocation = CachedTowerMainMapLayout.StartAnchorSurfaceLocation;
		MainMapStartPathSurfaceLocation = CachedTowerMainMapLayout.StartPathSurfaceLocation;
		MainMapStartAreaCenterSurfaceLocation = CachedTowerMainMapLayout.StartAreaCenterSurfaceLocation;
		MainMapBossAnchorSurfaceLocation = CachedTowerMainMapLayout.BossAnchorSurfaceLocation;
		MainMapBossSpawnSurfaceLocation = CachedTowerMainMapLayout.BossSpawnSurfaceLocation;
		MainMapBossBeaconSurfaceLocation = CachedTowerMainMapLayout.BossBeaconSurfaceLocation;
		MainMapBossAreaCenterSurfaceLocation = CachedTowerMainMapLayout.BossAreaCenterSurfaceLocation;
		MainMapRescueAnchorLocations = CachedTowerMainMapLayout.RescueAnchorLocations;

		UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Cached tower-map spawn surface at %s"), *MainMapSpawnSurfaceLocation.ToString());

		bool bTowerCollisionReady = false;
		if (!T66TowerMapTerrain::Spawn(
			World,
			CachedTowerMainMapLayout,
			GI ? GI->SelectedDifficulty : ET66Difficulty::Easy,
			SpawnParams,
			bTowerCollisionReady))
		{
			bUsingTowerMainMapLayout = false;
			CachedTowerMainMapLayout = T66TowerMapTerrain::FLayout{};
			return;
		}

		bTerrainCollisionReady = bTowerCollisionReady;
		if (bTerrainCollisionReady)
		{
			RestartPlayersMissingPawns();
		}
		return;
	}

	T66MainMapTerrain::FBoard Board;
	if (!T66MainMapTerrain::Generate(Preset, Board))
	{
		UE_LOG(LogT66GameMode, Error, TEXT("[MAP] Main map terrain generation failed to fill the board (seed=%d, occupied=%d/%d)"),
			Preset.Seed,
			Board.OccupiedCount,
			Board.Cells.Num());
		return;
	}

	MainMapSpawnSurfaceLocation = T66MainMapTerrain::GetPreferredSpawnLocation(Board, 0.f);
	bHasMainMapSpawnSurfaceLocation = true;
	MainMapRescueAnchorLocations.Add(MainMapSpawnSurfaceLocation);
	T66MainMapTerrain::TryGetCellLocation(Board, Board.StartAnchor, 0.f, MainMapStartAnchorSurfaceLocation);
	T66MainMapTerrain::TryGetCellLocation(Board, Board.StartPathCell, 0.f, MainMapStartPathSurfaceLocation);
	if (!T66MainMapTerrain::TryGetRegionCenterLocation(Board, T66MainMapTerrain::ECellRegion::StartArea, 0.f, MainMapStartAreaCenterSurfaceLocation))
	{
		MainMapStartAreaCenterSurfaceLocation = MainMapSpawnSurfaceLocation;
	}
	if (!MainMapStartAnchorSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapStartAnchorSurfaceLocation);
	}
	if (!MainMapStartAreaCenterSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapStartAreaCenterSurfaceLocation);
	}
	T66MainMapTerrain::TryGetCellLocation(Board, Board.BossAnchor, 0.f, MainMapBossAnchorSurfaceLocation);
	T66MainMapTerrain::TryGetCellLocation(Board, Board.BossSpawnCell, 0.f, MainMapBossSpawnSurfaceLocation);
	if (!T66MainMapTerrain::TryGetRegionCenterLocation(Board, T66MainMapTerrain::ECellRegion::BossArea, 0.f, MainMapBossAreaCenterSurfaceLocation))
	{
		MainMapBossAreaCenterSurfaceLocation = MainMapBossSpawnSurfaceLocation;
	}

	FVector BossBeaconLocation = FVector::ZeroVector;
	if (T66MainMapTerrain::TryGetCellLocation(Board, Board.BossSpawnCell - Board.BossOutwardDirection, 0.f, BossBeaconLocation))
	{
		MainMapBossBeaconSurfaceLocation = BossBeaconLocation;
	}
	else
	{
		MainMapBossBeaconSurfaceLocation = MainMapBossAreaCenterSurfaceLocation;
	}

	if (!MainMapBossAnchorSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapBossAnchorSurfaceLocation);
	}
	if (!MainMapBossSpawnSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapBossSpawnSurfaceLocation);
	}
	if (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapBossAreaCenterSurfaceLocation);
	}
	UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Cached main-map spawn surface at %s"), *MainMapSpawnSurfaceLocation.ToString());

	bool bMainMapCollisionReady = false;
	if (!T66MainMapTerrain::Spawn(World, Board, Preset, SpawnParams, bMainMapCollisionReady))
	{
		return;
	}

	bTerrainCollisionReady = bMainMapCollisionReady;
	if (bTerrainCollisionReady)
	{
		RestartPlayersMissingPawns();
		if (!T66UsesMainMapTerrainStage(World))
		{
			SnapPlayersToTerrain();
		}
	}

	UE_LOG(LogT66GameMode, Verbose, TEXT("[MAP] Terrain collision ready=%d"), bTerrainCollisionReady ? 1 : 0);
	UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Main map terrain spawned: %d tiles (seed=%d, cell=%.0f)"),
		Board.OccupiedCount,
		Preset.Seed,
		Board.Settings.CellSize);
}

void AT66GameMode::RegenerateMainMapTerrain(int32 Seed)
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
	if (UT66TrapSubsystem* TrapSubsystem = World->GetSubsystem<UT66TrapSubsystem>())
	{
		TrapSubsystem->ClearManagedTrapActors();
		TrapSubsystem->SetActiveTowerFloor(INDEX_NONE);
	}

	bTerrainCollisionReady = false;
	bMainMapCombatStarted = false;
	bHasMainMapSpawnSurfaceLocation = false;
	bUsingTowerMainMapLayout = false;
	CachedTowerMainMapLayout = T66TowerMapTerrain::FLayout{};
	bTowerBossEntryTriggered = false;
	bTowerBossEntryApplied = false;
	MainMapSpawnSurfaceLocation = FVector::ZeroVector;
	MainMapStartAnchorSurfaceLocation = FVector::ZeroVector;
	MainMapStartPathSurfaceLocation = FVector::ZeroVector;
	MainMapStartAreaCenterSurfaceLocation = FVector::ZeroVector;
	MainMapBossAnchorSurfaceLocation = FVector::ZeroVector;
	MainMapBossSpawnSurfaceLocation = FVector::ZeroVector;
	MainMapBossBeaconSurfaceLocation = FVector::ZeroVector;
	MainMapBossAreaCenterSurfaceLocation = FVector::ZeroVector;
	MainMapRescueAnchorLocations.Reset();
	for (AT66TowerDescentHole* Hole : TowerDescentHoles)
	{
		if (Hole)
		{
			Hole->Destroy();
		}
	}
	TowerDescentHoles.Reset();

	DestroyActorsWithTag(World, T66MapPlatformTag);
	DestroyActorsWithTag(World, T66MapRampTag);
	DestroyActorsWithTag(World, T66FloorMainTag);
	DestroyActorsWithTag(World, T66TraversalBarrierTag);

	if (StartGate)
	{
		StartGate->Destroy();
		StartGate = nullptr;
	}
	if (BossGate)
	{
		BossGate->Destroy();
		BossGate = nullptr;
	}
	if (CowardiceGate)
	{
		CowardiceGate->Destroy();
		CowardiceGate = nullptr;
	}
	if (TricksterNPC)
	{
		TricksterNPC->Destroy();
		TricksterNPC = nullptr;
	}
	if (AT66BossBase* ExistingBoss = StageBoss.Get())
	{
		ExistingBoss->Destroy();
	}
	StageBoss = nullptr;
	DestroyBossBeacon();

	UT66GameInstance* GI = GetT66GameInstance();
	if (GI)
	{
		GI->RunSeed = Seed;
	}

	SpawnMainMapTerrain();
	if (T66UsesMainMapTerrainStage(World))
	{
		SpawnTricksterAndCowardiceGate();
		SpawnBossForCurrentStage();
	}
	else
	{
		SpawnBossBeaconIfNeeded();
	}
}
