// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameModePrivate.h"
#include "Core/T66ShelvedFeatureGate.h"
#include "Core/T66TowerTuningConfig.h"
#include "Gameplay/T66SafeZoneComponent.h"

using namespace T66GameModePrivate;

namespace
{
	static constexpr int32 T66OuroborosHiddenSpawnFloorNumber = 4;
	static constexpr float T66OuroborosHiddenSpawnChance = 0.10f;

	static const FName T66StartGalleryShowcaseTag(TEXT("T66_StartGallery_Showcase"));
	static const FName T66StartGalleryHeroesTag(TEXT("T66_StartGallery_Heroes"));
	static const FName T66StartGalleryEnemiesTag(TEXT("T66_StartGallery_Enemies"));
	static const FName T66StartGalleryBossesTag(TEXT("T66_StartGallery_Bosses"));
	static const FName T66StartGalleryWorldTag(TEXT("T66_StartGallery_World"));

	TArray<FName> T66LoadDataTableRowNamesForStartGallery(const TCHAR* DataTablePath)
	{
		TArray<FName> RowNames;
		if (const UDataTable* DataTable = LoadObject<UDataTable>(nullptr, DataTablePath))
		{
			RowNames = DataTable->GetRowNames();
		}
		return RowNames;
	}

	const T66TowerMapTerrain::FStartGalleryWing* T66FindStartGalleryWing(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::ET66TowerStartGalleryCategory Category)
	{
		for (const T66TowerMapTerrain::FStartGalleryWing& Wing : Layout.StartGalleryWings)
		{
			if (Wing.Category == Category)
			{
				return &Wing;
			}
		}
		return nullptr;
	}

	FVector T66ComputeStartGallerySlotLocation(
		const T66TowerMapTerrain::FStartGalleryWing& Wing,
		const int32 SlotIndex,
		const int32 TotalSlots,
		const float AcrossSpacing,
		const float DepthSpacing)
	{
		const int32 ClampedTotal = FMath::Max(1, TotalSlots);
		const float UsableAcross = FMath::Max(AcrossSpacing, Wing.AcrossHalfExtent * 2.0f - 900.0f);
		const int32 MaxColumns = FMath::Max(1, FMath::FloorToInt(UsableAcross / FMath::Max(1.0f, AcrossSpacing)) + 1);
		const int32 Columns = FMath::Clamp(MaxColumns, 1, ClampedTotal);
		const int32 Rows = FMath::Max(1, FMath::DivideAndRoundUp(ClampedTotal, Columns));

		const int32 Row = FMath::Clamp(SlotIndex / Columns, 0, Rows - 1);
		const int32 Column = FMath::Clamp(SlotIndex % Columns, 0, Columns - 1);
		const int32 RowCount = FMath::Clamp(ClampedTotal - Row * Columns, 1, Columns);
		const float AcrossOffset = (static_cast<float>(Column) - (static_cast<float>(RowCount) - 1.0f) * 0.5f) * AcrossSpacing;
		const float DepthOffset = (static_cast<float>(Row) - (static_cast<float>(Rows) - 1.0f) * 0.5f) * DepthSpacing;

		FVector Location = Wing.Center + Wing.SideDirection * AcrossOffset + Wing.Direction * DepthOffset;
		Location.Z = Wing.SurfaceZ + 18.0f;
		return Location;
	}

	FRotator T66BuildStartGalleryFacingRotation(const T66TowerMapTerrain::FStartGalleryWing& Wing)
	{
		FVector Facing(-Wing.Direction.X, -Wing.Direction.Y, 0.0f);
		Facing.Normalize();
		return Facing.IsNearlyZero() ? FRotator::ZeroRotator : Facing.Rotation();
	}

	void T66TagStartGalleryActor(
		AActor* Actor,
		const T66TowerMapTerrain::FStartGalleryWing& Wing,
		const FName CategoryTag)
	{
		if (!Actor)
		{
			return;
		}

		Actor->Tags.AddUnique(T66StartGalleryShowcaseTag);
		Actor->Tags.AddUnique(CategoryTag);
		T66AssignTowerFloorTag(Actor, Wing.FloorNumber);
	}

	bool T66IsNormalTowerInteractableFloor(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor)
	{
		return Floor.bMobFloor
			&& Floor.FloorNumber != Layout.StartFloorNumber
			&& Floor.FloorNumber != Layout.BossFloorNumber
			&& Floor.FloorNumber >= Layout.FirstMobFloorNumber
			&& Floor.FloorNumber <= Layout.LastMobFloorNumber;
	}

	int32 T66ResolveTowerFloorForActorPhysical(
		const AT66GameMode* GameMode,
		const T66TowerMapTerrain::FLayout& Layout,
		const AActor* Actor)
	{
		if (!GameMode || !Actor)
		{
			return INDEX_NONE;
		}

		const int32 LocationFloor = GameMode->GetTowerFloorIndexForLocation(Actor->GetActorLocation());
		if (LocationFloor != INDEX_NONE)
		{
			return LocationFloor;
		}

		FVector BoundsOrigin = FVector::ZeroVector;
		FVector BoundsExtent = FVector::ZeroVector;
		Actor->GetActorBounds(true, BoundsOrigin, BoundsExtent);

		const float CenterTolerance = FMath::Clamp(BoundsExtent.Z + 250.0f, 900.0f, 2200.0f);
		const int32 BoundsCenterFloor = T66TowerMapTerrain::FindFloorIndexForLocation(Layout, BoundsOrigin, CenterTolerance);
		if (BoundsCenterFloor != INDEX_NONE)
		{
			return BoundsCenterFloor;
		}

		const FVector BoundsBottom(BoundsOrigin.X, BoundsOrigin.Y, BoundsOrigin.Z - BoundsExtent.Z);
		const int32 BoundsBottomFloor = T66TowerMapTerrain::FindFloorIndexForLocation(Layout, BoundsBottom, 350.0f);
		if (BoundsBottomFloor != INDEX_NONE)
		{
			return BoundsBottomFloor;
		}

		return INDEX_NONE;
	}

	int32 T66ResolveTowerFloorForActorLoose(
		const AT66GameMode* GameMode,
		const T66TowerMapTerrain::FLayout& Layout,
		const AActor* Actor)
	{
		if (!GameMode || !Actor)
		{
			return INDEX_NONE;
		}

		const int32 TaggedFloor = T66ReadTowerFloorTag(Actor);
		if (TaggedFloor != INDEX_NONE)
		{
			return TaggedFloor;
		}

		return T66ResolveTowerFloorForActorPhysical(GameMode, Layout, Actor);
	}

	AT66GalleryDisplayActor* T66SpawnStartGalleryDisplayActor(
		UWorld* World,
		const T66TowerMapTerrain::FStartGalleryWing& Wing,
		const FName CategoryTag,
		const FName VisualID,
		const int32 SlotIndex,
		const int32 TotalSlots,
		const float AcrossSpacing,
		const float DepthSpacing,
		const float ActorScale = 1.0f)
	{
		if (!World || VisualID.IsNone())
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const FVector Location = T66ComputeStartGallerySlotLocation(Wing, SlotIndex, TotalSlots, AcrossSpacing, DepthSpacing);
		const FRotator Rotation = T66BuildStartGalleryFacingRotation(Wing);
		AT66GalleryDisplayActor* Display = World->SpawnActor<AT66GalleryDisplayActor>(
			AT66GalleryDisplayActor::StaticClass(),
			Location,
			Rotation,
			SpawnParams);
		if (Display)
		{
			T66TagStartGalleryActor(Display, Wing, CategoryTag);
			Display->ConfigureDisplayVisual(VisualID, ActorScale);
		}
		return Display;
	}
}

void AT66GameMode::SpawnStageEffectsForStage()
{
	if (IsUsingTowerMainMapLayout()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!RunState || !T66GI) return;

	const ET66Difficulty Difficulty = T66GI->SelectedDifficulty;
	const int32 StageNum = RunState->GetCurrentStage();

	const int32 RunSeed = (T66GI && T66GI->RunSeed != 0) ? T66GI->RunSeed : FMath::Rand();
	FRandomStream Rng(RunSeed + StageNum * 971 + 17);

	static constexpr float MainHalfExtent = 50000.f;
	static constexpr float SpawnZ = 40.f;
	static constexpr float SafeBubbleMargin = 350.f;

	const int32 ShroomCount = 0;

	struct FUsedStageEffectLoc
	{
		FVector Loc = FVector::ZeroVector;
		float ExclusionRadius = 0.f;
	};
	TArray<FUsedStageEffectLoc> UsedLocs;

	auto IsInsideNoSpawnZone = [&](const FVector& L) -> bool
	{
		if (T66GameplayLayout::IsInsideReservedTraversalZone2D(L, 455.f))
		{
			return true;
		}

		static constexpr float ArenaHalf = 9091.f;
		static constexpr float ArenaMargin = 682.f;
		static constexpr float TutorialArenaHalf = 9091.f;
		struct FArena2D { float X; float Y; float Half; };
		static constexpr FArena2D Arenas[] = {
			{      0.f, 61364.f, TutorialArenaHalf }, // Tutorial
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

	auto IsGoodLoc = [&](const FVector& L, float CandidateRadius) -> bool
	{
		if (IsInsideNoSpawnZone(L)) return false;
		for (const FUsedStageEffectLoc& Used : UsedLocs)
		{
			const float RequiredRadius = FMath::Max(CandidateRadius, Used.ExclusionRadius);
			if (FVector::DistSquared2D(L, Used.Loc) < (RequiredRadius * RequiredRadius)) return false;
		}
		UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		const TArray<TWeakObjectPtr<AT66NPCBase>>& NPCs = Registry ? Registry->GetNPCs() : TArray<TWeakObjectPtr<AT66NPCBase>>();
		for (const TWeakObjectPtr<AT66NPCBase>& WeakNPC : NPCs)
		{
			const AT66NPCBase* NPC = WeakNPC.Get();
			if (!NPC) continue;
			const float R = NPC->GetSafeZoneRadius() + SafeBubbleMargin + CandidateRadius * 0.35f;
			if (FVector::DistSquared2D(L, NPC->GetActorLocation()) < (R * R)) return false;
		}
		if (Registry)
		{
			for (const TWeakObjectPtr<UT66SafeZoneComponent>& WeakSafeZone : Registry->GetSafeZones())
			{
				const UT66SafeZoneComponent* SafeZone = WeakSafeZone.Get();
				if (!SafeZone) continue;
				const float R = SafeZone->GetSafeZoneRadius() + SafeBubbleMargin + CandidateRadius * 0.35f;
				if (FVector::DistSquared2D(L, SafeZone->GetComponentLocation()) < (R * R)) return false;
			}
		}
		return true;
	};

	struct FStageEffectSpawnHit
	{
		FVector Loc = FVector::ZeroVector;
		FVector Normal = FVector::UpVector;
		bool bFound = false;
	};

	auto FindSpawnLoc = [&](float CandidateRadius, int32 MaxTries) -> FStageEffectSpawnHit
	{
		const FVector Up = FVector::UpVector;
		for (int32 Try = 0; Try < MaxTries; ++Try)
		{
			const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			FVector Loc(X, Y, SpawnZ);
			FVector Normal = Up;

			FHitResult Hit;
			const FVector Start = Loc + FVector(0.f, 0.f, 2000.f);
			const FVector End = Loc - FVector(0.f, 0.f, 6000.f);
			if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
			{
				continue;
			}
			if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
			{
				continue;
			}

			Loc = Hit.ImpactPoint;
			Normal = Hit.ImpactNormal.GetSafeNormal(1e-4f, Up);

			if (IsGoodLoc(Loc, CandidateRadius))
			{
				return { Loc, Normal, true };
			}
		}
		return {};
	};

	auto NoteUsedLoc = [&](const FVector& Loc, float Radius)
	{
		UsedLocs.Add({ Loc, Radius });
	};

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (!IsValid(MiasmaManager))
	{
		MiasmaManager = World->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, P);
	}

	const int32 SpawnedLavaCount = MiasmaManager ? MiasmaManager->SpawnLegacyStageLavaPatchesForCurrentStage() : 0;
	int32 SpawnedShroomCount = 0;

	static constexpr float ShroomExclusionRadius = 420.f;
	for (int32 i = 0; i < ShroomCount; ++i)
	{
		const FStageEffectSpawnHit Th = FindSpawnLoc(ShroomExclusionRadius, 80);
		if (!Th.bFound)
		{
			continue;
		}

		AT66Shroom* Shroom = World->SpawnActor<AT66Shroom>(AT66Shroom::StaticClass(), Th.Loc, FRotator::ZeroRotator, P);
		if (Shroom)
		{
			NoteUsedLoc(Th.Loc, ShroomExclusionRadius);
			++SpawnedShroomCount;
		}
	}

	UE_LOG(LogT66GameMode, Log,
		TEXT("[StageEffects] Spawned %d lava patches and %d shrooms for stage %d (diff=%d)."),
		SpawnedLavaCount,
		SpawnedShroomCount,
		StageNum,
		static_cast<int32>(Difficulty));
}

void AT66GameMode::SpawnCasinoNPCIfNeeded()
{
	if (IsLabRun())
	{
		return;
	}

	if (IsUsingTowerMainMapLayout())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UT66GameInstance* T66GI = GetT66GameInstance();
	if (T66UsesMainMapTerrainStage(World) && (!T66GI || T66GI->CurrentMainMapLayoutVariant == ET66MainMapLayoutVariant::Tower))
	{
		return;
	}

	if (T66HasRegisteredCasinoNPC(World))
	{
		return;
	}

	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	float TraceStartZ = 2000.f;
	float TraceEndZ = -4000.f;
	float RefX = 0.f;
	float RefY = 0.f;
	if (bUsingMainMapTerrain)
	{
		FVector CasinoLoc = FVector::ZeroVector;
		if (!TryFindRandomMainMapSurfaceLocation(3201, CasinoLoc, 450.f))
		{
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (AT66CasinoNPC* CasinoNPC = World->SpawnActor<AT66CasinoNPC>(AT66CasinoNPC::StaticClass(), CasinoLoc, FRotator::ZeroRotator, SpawnParams))
		{
			CasinoNPC->SetActorScale3D(FVector(0.82f));
		}
		return;
	}

	auto FindClosestFlatSurface = [World, TraceStartZ, TraceEndZ](float InRefX, float InRefY) -> FVector
	{
		static constexpr float MinNormalZ = 0.92f;
		static constexpr float SearchRadiusMax = 2200.f;
		static constexpr float RadiusStep = 140.f;
		static constexpr int32 NumAngles = 20;

		FVector Fallback(InRefX, InRefY, 60.f);
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, FVector(InRefX, InRefY, TraceStartZ), FVector(InRefX, InRefY, TraceEndZ), ECC_WorldStatic))
		{
			Fallback = Hit.ImpactPoint;
		}

		for (float R = 0.f; R <= SearchRadiusMax; R += RadiusStep)
		{
			const int32 AngleSteps = (R <= 0.f) ? 1 : NumAngles;
			for (int32 AngleIndex = 0; AngleIndex < AngleSteps; ++AngleIndex)
			{
				const float Angle = (AngleSteps == 1) ? 0.f : (2.f * PI * static_cast<float>(AngleIndex) / static_cast<float>(AngleSteps));
				const float X = InRefX + R * FMath::Cos(Angle);
				const float Y = InRefY + R * FMath::Sin(Angle);
				if (World->LineTraceSingleByChannel(Hit, FVector(X, Y, TraceStartZ), FVector(X, Y, TraceEndZ), ECC_WorldStatic) && Hit.ImpactNormal.Z >= MinNormalZ)
				{
					return Hit.ImpactPoint;
				}
			}
		}

		return Fallback;
	};

	const FVector FlatLoc = FindClosestFlatSurface(RefX, RefY);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66CasinoNPC* CasinoNPC = World->SpawnActor<AT66CasinoNPC>(AT66CasinoNPC::StaticClass(), FlatLoc, FRotator::ZeroRotator, SpawnParams))
	{
		CasinoNPC->SetActorScale3D(FVector(0.82f));
	}
}

void AT66GameMode::SpawnPlateauAtLocation(UWorld* World, const FVector& TopCenterLoc)
{
	if (!World) return;
	static constexpr float PlateauHalfHeight = 10.f; // top of disc at TopCenterLoc.Z
	const FVector PlateauLoc(TopCenterLoc.X, TopCenterLoc.Y, TopCenterLoc.Z - PlateauHalfHeight);
	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AT66SpawnPlateau>(AT66SpawnPlateau::StaticClass(), PlateauLoc, FRotator::ZeroRotator, P);
}

void AT66GameMode::SpawnWorldInteractablesForStage()
{
	if (bWorldInteractablesSpawnedForStage)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();

	// Run seed so positions change every time "Enter the Tribulation" or PIE is started (like procedural terrain).
	const int32 RunSeed = T66EnsureRunSeed(T66GI);
	const int32 StageNum = RunState->GetCurrentStage();
	FRandomStream Rng(RunSeed + StageNum * 1337 + 42);
	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	const bool bTowerLayout = IsUsingTowerMainMapLayout();
	TArray<int32> TowerMobFloorNumbers;
	TMap<int32, int32> TowerChestCountByFloor;
	TMap<int32, int32> TowerCrateCountByFloor;
	if (bTowerLayout)
	{
		const UT66TowerTuningConfig& TowerTuning = UT66TowerTuningConfig::GetRuntimeConfig();
		const FT66IntRange TowerChestRange = TowerTuning.GetTowerChestCountRange();
		const FT66IntRange TowerCrateRange = TowerTuning.GetTowerCrateCountRange();
		FRandomStream TowerCountRng(RunSeed + StageNum * 1901 + 39);
		for (const T66TowerMapTerrain::FFloor& Floor : CachedTowerMainMapLayout.Floors)
		{
			if (!T66IsNormalTowerInteractableFloor(CachedTowerMainMapLayout, Floor))
			{
				continue;
			}

			TowerMobFloorNumbers.Add(Floor.FloorNumber);
			TowerChestCountByFloor.Add(Floor.FloorNumber, TowerCountRng.RandRange(TowerChestRange.Min, TowerChestRange.Max));
			TowerCrateCountByFloor.Add(Floor.FloorNumber, TowerCountRng.RandRange(TowerCrateRange.Min, TowerCrateRange.Max));
		}
		TowerMobFloorNumbers.Sort();
		TowerMobFloorNumbers.Remove(CachedTowerMainMapLayout.StartFloorNumber);
		TowerMobFloorNumbers.Remove(CachedTowerMainMapLayout.BossFloorNumber);
	}

	FT66MapPreset MainMapPreset;
	T66MainMapTerrain::FSettings MainMapSettings;
	float MainHalfExtent = 50000.f;
	float TraceStartZ = 8000.f;
	float TraceEndZ = -16000.f;
	if (bUsingMainMapTerrain && !bTowerLayout)
	{
		MainMapPreset = T66BuildMainMapPreset(T66GI);
		MainMapSettings = T66MainMapTerrain::MakeSettings(MainMapPreset);
		MainHalfExtent = FMath::Max(0.0f, MainMapSettings.HalfExtent - MainMapSettings.CellSize * 1.25f);
		TraceStartZ = T66MainMapTerrain::GetTraceZ(MainMapPreset);
		TraceEndZ = T66MainMapTerrain::GetLowestCollisionBottomZ(MainMapPreset) - MainMapSettings.StepHeight;
	}

	static constexpr float SpawnZ = 220.f;
	static constexpr float MinDistBetweenInteractables = 900.f;
	static constexpr float SafeBubbleMargin = 250.f;

	TArray<FVector> UsedLocs;

	// No-spawn zones: keep gameplay spawns out of reserved traversal spaces and special arenas.
	auto IsInsideNoSpawnZone = [&](const FVector& L) -> bool
	{
		if (bTowerLayout)
		{
			return false;
		}

		if (bUsingMainMapTerrain)
		{
			const float RoomReserveRadius = MainMapSettings.CellSize * T66MainMapRoomReserveRadiusCells;
			const float CorridorReserveRadius = MainMapSettings.CellSize * T66MainMapCorridorReserveRadiusCells;
			if (!MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapStartAreaCenterSurfaceLocation) < FMath::Square(RoomReserveRadius))
			{
				return true;
			}
			if (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapBossAreaCenterSurfaceLocation) < FMath::Square(RoomReserveRadius))
			{
				return true;
			}
			if (!MainMapStartPathSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapStartPathSurfaceLocation) < FMath::Square(CorridorReserveRadius))
			{
				return true;
			}
			if (!MainMapStartAnchorSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapStartAnchorSurfaceLocation) < FMath::Square(CorridorReserveRadius))
			{
				return true;
			}
			if (!MainMapBossAnchorSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapBossAnchorSurfaceLocation) < FMath::Square(CorridorReserveRadius))
			{
				return true;
			}
			return false;
		}

		if (T66GameplayLayout::IsInsideReservedTraversalZone2D(L, 455.f))
		{
			return true;
		}

		static constexpr float ArenaHalf = 9091.f;
		static constexpr float ArenaMargin = 682.f;
		static constexpr float TutorialArenaHalf = 9091.f;
		struct FArena2D { float X; float Y; float Half; };
		static constexpr FArena2D Arenas[] = {
			{      0.f, 61364.f, TutorialArenaHalf }, // Tutorial
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

	auto IsSameTowerFloor = [&](const FVector& A, const FVector& B) -> bool
	{
		if (!bTowerLayout)
		{
			return true;
		}

		const int32 FloorA = GetTowerFloorIndexForLocation(A);
		const int32 FloorB = GetTowerFloorIndexForLocation(B);
		return FloorA != INDEX_NONE && FloorA == FloorB;
	};

	auto ResolveTowerFloorForActor = [&](const AActor* Actor) -> int32
	{
		if (!bTowerLayout || !Actor)
		{
			return INDEX_NONE;
		}

		return T66ResolveTowerFloorForActorLoose(this, CachedTowerMainMapLayout, Actor);
	};

	auto IsGoodLoc = [&](const FVector& L) -> bool
	{
		if (IsInsideNoSpawnZone(L))
		{
			return false;
		}
		static constexpr float CasinoKeepClearRadius = 2200.f;
		if (!bUsingMainMapTerrain && FVector::DistSquared2D(L, FVector::ZeroVector) < (CasinoKeepClearRadius * CasinoKeepClearRadius))
		{
			return false;
		}
		for (const FVector& U : UsedLocs)
		{
			if (!IsSameTowerFloor(L, U))
			{
				continue;
			}
			if (FVector::DistSquared2D(L, U) < (MinDistBetweenInteractables * MinDistBetweenInteractables))
			{
				return false;
			}
		}
		UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		const TArray<TWeakObjectPtr<AT66NPCBase>>& NPCs = Registry ? Registry->GetNPCs() : TArray<TWeakObjectPtr<AT66NPCBase>>();
		for (const TWeakObjectPtr<AT66NPCBase>& WeakNPC : NPCs)
		{
			const AT66NPCBase* NPC = WeakNPC.Get();
			if (!NPC) continue;
			if (bTowerLayout)
			{
				const int32 CandidateFloor = GetTowerFloorIndexForLocation(L);
				const int32 NPCFloor = ResolveTowerFloorForActor(NPC);
				if (CandidateFloor == INDEX_NONE || NPCFloor == INDEX_NONE || CandidateFloor != NPCFloor)
				{
					continue;
				}
			}
			else if (!IsSameTowerFloor(L, NPC->GetActorLocation()))
			{
				continue;
			}
			const float R = NPC->GetSafeZoneRadius() + SafeBubbleMargin;
			if (FVector::DistSquared2D(L, NPC->GetActorLocation()) < (R * R))
			{
				return false;
			}
		}
		if (Registry)
		{
			for (const TWeakObjectPtr<UT66SafeZoneComponent>& WeakSafeZone : Registry->GetSafeZones())
			{
				const UT66SafeZoneComponent* SafeZone = WeakSafeZone.Get();
				if (!SafeZone) continue;
				const FVector SafeZoneLocation = SafeZone->GetComponentLocation();
				if (bTowerLayout)
				{
					const int32 CandidateFloor = GetTowerFloorIndexForLocation(L);
					const int32 SafeZoneFloor = GetTowerFloorIndexForLocation(SafeZoneLocation);
					if (CandidateFloor == INDEX_NONE || SafeZoneFloor == INDEX_NONE || CandidateFloor != SafeZoneFloor)
					{
						continue;
					}
				}
				else if (!IsSameTowerFloor(L, SafeZoneLocation))
				{
					continue;
				}
				const float R = SafeZone->GetSafeZoneRadius() + SafeBubbleMargin;
				if (FVector::DistSquared2D(L, SafeZoneLocation) < (R * R))
				{
					return false;
				}
			}
		}
		return true;
	};

	struct FSpawnHitResult
	{
		FVector Loc = FVector::ZeroVector;
		bool bFound = false;
	};
	auto FindSpawnLoc = [&]() -> FSpawnHitResult
	{
		for (int32 Try = 0; Try < 40; ++Try)
		{
			FVector Loc = FVector::ZeroVector;
			if (bTowerLayout)
			{
				if (!T66TowerMapTerrain::TryGetRandomGameplaySurfaceLocation(World, CachedTowerMainMapLayout, Rng, Loc))
				{
					continue;
				}
			}
			else
			{
				const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
				const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
				Loc = FVector(X, Y, SpawnZ);

				FHitResult Hit;
				const FVector Start(X, Y, TraceStartZ);
				const FVector End(X, Y, TraceEndZ);
				if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
				{
					continue;
				}
				if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
				{
					continue;
				}
				Loc = Hit.ImpactPoint;
			}

			if (IsGoodLoc(Loc))
			{
				return { Loc, true };
			}
		}
		return {};
	};

	auto SpawnOne = [&](UClass* Cls) -> AActor*
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		const FSpawnHitResult HitResult = FindSpawnLoc();
		if (!HitResult.bFound)
		{
			return nullptr;
		}
		AActor* A = World->SpawnActor<AActor>(Cls, HitResult.Loc, FRotator::ZeroRotator, P);
		if (A)
		{
			if (bTowerLayout)
			{
				const int32 FloorNumber = GetTowerFloorIndexForLocation(HitResult.Loc);
				if (FloorNumber != INDEX_NONE)
				{
					T66TrySnapActorToTowerFloor(World, A, CachedTowerMainMapLayout, FloorNumber, HitResult.Loc);
					T66AssignTowerFloorTag(A, FloorNumber);
				}
			}
			UsedLocs.Add(HitResult.Loc);
		}
		return A;
	};

	if (RngSub)
	{
		RngSub->UpdateLuckStat(RunState->GetEffectiveLuckBiasStat());
	}

	const UT66RngTuningConfig* Tuning = RngSub ? RngSub->GetTuning() : nullptr;
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI ? T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const FT66IntRange ChestCountRange = PlayerExperience
		? PlayerExperience->GetDifficultyChestCountRange(Difficulty)
		: FT66IntRange{ 4, 10 };
	const FT66IntRange CrateCountRange = PlayerExperience
		? PlayerExperience->GetDifficultyCrateCountRange(Difficulty)
		: FT66IntRange{ 3, 6 };
	const FT66IntRange LootWheelCountRange = PlayerExperience
		? PlayerExperience->GetDifficultyLootWheelCountRange(Difficulty)
		: FT66IntRange{ 1, 2 };

	// Luck-affected counts use central tuning. Locations are still stage-seeded (not luck-affected).
	int32 CountFountains = 0;
	int32 FountainsDrawIndex = INDEX_NONE;
	int32 FountainsPreDrawSeed = 0;
	TSet<int32> TowerFountainFloorNumbers;
	if (bTowerLayout)
	{
		const float TowerFountainChancePerFloor = UT66TowerTuningConfig::GetRuntimeConfig().TowerFountainChancePerFloor;
		FRandomStream TowerFountainRng(RunSeed + StageNum * 1901 + 38);
		FountainsPreDrawSeed = RunSeed + StageNum * 1901 + 38;
		for (const int32 FloorNumber : TowerMobFloorNumbers)
		{
			if (TowerFountainRng.GetFraction() < TowerFountainChancePerFloor)
			{
				TowerFountainFloorNumbers.Add(FloorNumber);
			}
		}
		CountFountains = TowerFountainFloorNumbers.Num();
	}
	else
	{
		CountFountains = (RngSub && Tuning) ? RngSub->RollIntRangeBiased(Tuning->FountainsPerStage, Rng) : Rng.RandRange(2, 5);
		FountainsDrawIndex = (RngSub && Tuning) ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		FountainsPreDrawSeed = (RngSub && Tuning) ? RngSub->GetLastRunPreDrawSeed() : 0;
	}
	int32 CountChests = 0;
	int32 ChestsDrawIndex = INDEX_NONE;
	int32 ChestsPreDrawSeed = 0;
	if (bTowerLayout)
	{
		for (const int32 FloorNumber : TowerMobFloorNumbers)
		{
			CountChests += TowerChestCountByFloor.FindRef(FloorNumber);
		}
		ChestsPreDrawSeed = RunSeed + StageNum * 1901 + 39;
	}
	else
	{
		CountChests = RngSub ? RngSub->RollIntRangeBiased(ChestCountRange, Rng) : Rng.RandRange(FMath::Min(ChestCountRange.Min, ChestCountRange.Max), FMath::Max(ChestCountRange.Min, ChestCountRange.Max));
		ChestsDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		ChestsPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
	}
	int32 CountCrates = 0;
	int32 CratesDrawIndex = INDEX_NONE;
	int32 CratesPreDrawSeed = 0;
	if (bTowerLayout)
	{
		for (const int32 FloorNumber : TowerMobFloorNumbers)
		{
			CountCrates += TowerCrateCountByFloor.FindRef(FloorNumber);
		}
		CratesPreDrawSeed = RunSeed + StageNum * 1901 + 40;
	}
	else
	{
		CountCrates = RngSub ? RngSub->RollIntRangeBiased(CrateCountRange, Rng) : Rng.RandRange(FMath::Min(CrateCountRange.Min, CrateCountRange.Max), FMath::Max(CrateCountRange.Min, CrateCountRange.Max));
		CratesDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		CratesPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
	}
	int32 CountLootWheels = 0;
	int32 LootWheelsDrawIndex = INDEX_NONE;
	int32 LootWheelsPreDrawSeed = 0;
	if (bTowerLayout)
	{
		FRandomStream TowerLootWheelRng(RunSeed + StageNum * 1901 + 41);
		LootWheelsPreDrawSeed = RunSeed + StageNum * 1901 + 41;
		const int32 MinLootWheels = FMath::Min(LootWheelCountRange.Min, LootWheelCountRange.Max);
		const int32 MaxLootWheels = FMath::Max(LootWheelCountRange.Min, LootWheelCountRange.Max);
		CountLootWheels = FMath::Clamp(TowerLootWheelRng.RandRange(MinLootWheels, MaxLootWheels), 0, TowerMobFloorNumbers.Num());
	}
	else
	{
		CountLootWheels = RngSub ? RngSub->RollIntRangeBiased(LootWheelCountRange, Rng) : Rng.RandRange(FMath::Min(LootWheelCountRange.Min, LootWheelCountRange.Max), FMath::Max(LootWheelCountRange.Min, LootWheelCountRange.Max));
		LootWheelsDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		LootWheelsPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
	}

	// Luck Rating tracking (quantity).
	if (RunState)
	{
		const int32 FountainsMin = bTowerLayout ? 0 : (Tuning ? Tuning->FountainsPerStage.Min : 2);
		const int32 FountainsMax = bTowerLayout ? TowerMobFloorNumbers.Num() : (Tuning ? Tuning->FountainsPerStage.Max : 5);
		const int32 ChestsMin = bTowerLayout ? TowerMobFloorNumbers.Num() : FMath::Min(ChestCountRange.Min, ChestCountRange.Max);
		const int32 ChestsMax = bTowerLayout ? (TowerMobFloorNumbers.Num() * 3) : FMath::Max(ChestCountRange.Min, ChestCountRange.Max);
		const int32 CratesMin = bTowerLayout ? TowerMobFloorNumbers.Num() : FMath::Min(CrateCountRange.Min, CrateCountRange.Max);
		const int32 CratesMax = bTowerLayout ? (TowerMobFloorNumbers.Num() * 3) : FMath::Max(CrateCountRange.Min, CrateCountRange.Max);
		const int32 LootWheelsMin = bTowerLayout ? 0 : FMath::Min(LootWheelCountRange.Min, LootWheelCountRange.Max);
		const int32 LootWheelsMax = bTowerLayout ? TowerMobFloorNumbers.Num() : FMath::Max(LootWheelCountRange.Min, LootWheelCountRange.Max);
		RunState->RecordLuckQuantityRoll(FName(TEXT("FountainsPerStage")), CountFountains, FountainsMin, FountainsMax, FountainsDrawIndex, FountainsPreDrawSeed);
		RunState->RecordLuckQuantityRoll(FName(TEXT("ChestsPerStage")), CountChests, ChestsMin, ChestsMax, ChestsDrawIndex, ChestsPreDrawSeed);
		RunState->RecordLuckQuantityRoll(FName(TEXT("CratesPerStage")), CountCrates, CratesMin, CratesMax, CratesDrawIndex, CratesPreDrawSeed);
		RunState->RecordLuckQuantityRoll(FName(TEXT("LootWheelsPerStage")), CountLootWheels, LootWheelsMin, LootWheelsMax, LootWheelsDrawIndex, LootWheelsPreDrawSeed);
	}

	int32 CountTotems = 0;
	if (bTowerLayout)
	{
		for (const int32 FloorNumber : TowerMobFloorNumbers)
		{
			if (!PlayerExperience
				|| PlayerExperience->ShouldSpawnDifficultyTotemOnTowerFloor(
					Difficulty,
					false,
					FloorNumber,
					CachedTowerMainMapLayout.FirstMobFloorNumber,
					CachedTowerMainMapLayout.LastMobFloorNumber))
			{
				++CountTotems;
			}
		}
	}
	int32 RemainingFountains = CountFountains;
	int32 RemainingChests = CountChests;
	int32 RemainingCrates = CountCrates;
	int32 RemainingLootWheels = CountLootWheels;
	int32 RemainingTotems = CountTotems;

	auto ConfigureChest = [&](AT66ChestInteractable* Chest)
	{
		if (!Chest)
		{
			return;
		}

		const float ChestMimicChance = PlayerExperience ? PlayerExperience->GetDifficultyChestMimicChance(Difficulty) : 0.20f;
		Chest->bIsMimic = RngSub ? RngSub->RollChance01(ChestMimicChance) : (Rng.GetFraction() < ChestMimicChance);
		const FT66RarityWeights Weights = PlayerExperience
			? PlayerExperience->GetDifficultyChestRarityWeights(Difficulty)
			: FT66RarityWeights{};
		const bool bChestRarityReplayable = (RngSub && PlayerExperience);
		const ET66Rarity ChestRarity = bChestRarityReplayable ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
		const int32 ChestRarityDrawIndex = bChestRarityReplayable ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		const int32 ChestRarityPreDrawSeed = bChestRarityReplayable ? RngSub->GetLastRunPreDrawSeed() : 0;
		Chest->SetRarity(ChestRarity);
		if (RunState)
		{
			RunState->RecordLuckQuantityBool(
				FName(TEXT("ChestMimicAvoided")),
				!Chest->bIsMimic,
				1.f - FMath::Clamp(ChestMimicChance, 0.f, 1.f));
			RunState->RecordLuckQualityRarity(FName(TEXT("ChestRarity")), ChestRarity, ChestRarityDrawIndex, ChestRarityPreDrawSeed, bChestRarityReplayable ? &Weights : nullptr);
		}
	};

	auto ConfigureCrate = [&](AT66CrateInteractable* Crate)
	{
		if (!Crate)
		{
			return;
		}

		const FT66RarityWeights Weights = PlayerExperience
			? PlayerExperience->GetDifficultyCrateRarityWeights(Difficulty)
			: FT66RarityWeights{};
		const bool bCrateRarityReplayable = (RngSub && PlayerExperience);
		const ET66Rarity Rarity = bCrateRarityReplayable ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
		const int32 CrateRarityDrawIndex = bCrateRarityReplayable ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		const int32 CrateRarityPreDrawSeed = bCrateRarityReplayable ? RngSub->GetLastRunPreDrawSeed() : 0;
		Crate->SetRarity(Rarity);
		if (RunState)
		{
			RunState->RecordLuckQualityRarity(FName(TEXT("CrateRarity")), Rarity, CrateRarityDrawIndex, CrateRarityPreDrawSeed, bCrateRarityReplayable ? &Weights : nullptr);
		}
	};

	auto ConfigureLootWheel = [&](AT66LootWheelInteractable* LootWheel)
	{
		if (!LootWheel)
		{
			return;
		}

		const FT66RarityWeights Weights = PlayerExperience
			? PlayerExperience->GetDifficultyLootWheelRarityWeights(Difficulty)
			: FT66RarityWeights{};
		const bool bLootWheelRarityReplayable = (RngSub && PlayerExperience);
		const ET66Rarity WheelRarity = bLootWheelRarityReplayable ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
		const int32 LootWheelRarityDrawIndex = bLootWheelRarityReplayable ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		const int32 LootWheelRarityPreDrawSeed = bLootWheelRarityReplayable ? RngSub->GetLastRunPreDrawSeed() : 0;
		LootWheel->SetRarity(WheelRarity);
		if (RunState)
		{
			RunState->RecordLuckQualityRarity(FName(TEXT("LootWheelRarity")), WheelRarity, LootWheelRarityDrawIndex, LootWheelRarityPreDrawSeed, bLootWheelRarityReplayable ? &Weights : nullptr);
		}
	};

	auto ConfigureLootBag = [&](AT66LootBagPickup* LootBag, const FName ContextName)
	{
		if (!LootBag)
		{
			return;
		}

		const FT66RarityWeights Weights = PlayerExperience
			? PlayerExperience->GetDifficultyEnemyLootBagRarityWeights(Difficulty)
			: FT66RarityWeights{};
		const bool bBagRarityReplayable = (RngSub && PlayerExperience);
		const ET66Rarity BagRarity = bBagRarityReplayable
			? RngSub->RollRarityWeighted(Weights, Rng)
			: FT66RarityUtil::RollDefaultRarity(Rng);
		LootBag->SetLootRarity(BagRarity);
		if (T66GI)
		{
			LootBag->SetItemID(T66GI->GetRandomItemIDForLootRarityFromStream(BagRarity, Rng));
		}
		if (RunState)
		{
			RunState->RecordLuckQualityRarity(
				ContextName.IsNone() ? FName(TEXT("LootBagRarity")) : ContextName,
				BagRarity,
				bBagRarityReplayable ? RngSub->GetLastRunDrawIndex() : INDEX_NONE,
				bBagRarityReplayable ? RngSub->GetLastRunPreDrawSeed() : 0,
				bBagRarityReplayable ? &Weights : nullptr);
		}
	};

	auto ConfigureTotem = [&](AT66DifficultyTotem* Totem)
	{
		if (Totem)
		{
			Totem->SetRarity(FT66RarityUtil::RollDefaultRarity(Rng));
		}
	};

	auto FindExistingTaggedActor = [&](const FName Tag) -> AActor*
	{
		return T66FindTaggedActor(World, Tag);
	};

	if (bTowerLayout)
	{
		FRandomStream TowerFloorRng(RunSeed + StageNum * 1901 + 77);
		TArray<int32> MobFloorNumbers = TowerMobFloorNumbers;
		const int32 UtilityVehicleFloorNumber = CachedTowerMainMapLayout.FirstMobFloorNumber;
		TSet<FName> ExistingTowerOccupantTags;
		int32 TowerSpawnedChests = 0;
		int32 TowerSpawnedCrates = 0;
		int32 TowerSpawnedLootWheels = 0;
		int32 TowerSpawnedFountains = 0;
		int32 TowerSpawnedLootBags = 0;
		int32 TowerSpawnedCasinoNPCs = 0;
		int32 TowerSpawnedVendors = 0;
		int32 TowerSpawnedVehicles = 0;
		int32 TowerSpawnedOuroboros = 0;
		int32 TowerSpawnedSaints = 0;
		int32 TowerSpawnedTotems = 0;
		// Stage setup snapshot: collected once before placing tower interactables.
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (AActor* ExistingActor = *It)
			{
				for (const FName& ExistingTag : ExistingActor->Tags)
				{
					ExistingTowerOccupantTags.Add(ExistingTag);
				}
			}
		}

		auto ResnapTowerActorToFloor = [&](AActor* Actor, const int32 FloorNumber) -> bool
		{
			if (!Actor)
			{
				return false;
			}

			T66TrySnapActorToTowerFloor(World, Actor, CachedTowerMainMapLayout, FloorNumber, Actor->GetActorLocation());
			const int32 ResolvedFloor = T66ResolveTowerFloorForActorPhysical(this, CachedTowerMainMapLayout, Actor);
			if (ResolvedFloor != FloorNumber)
			{
				return false;
			}

			T66AssignTowerFloorTag(Actor, FloorNumber);
			return true;
		};

		auto TryFindTowerFloorLocation = [&](const int32 FloorNumber, const int32 SeedOffset, const float EdgePadding, const float HolePadding, FVector& OutLocation) -> bool
		{
			if (FloorNumber == INDEX_NONE
				|| FloorNumber == CachedTowerMainMapLayout.StartFloorNumber
				|| FloorNumber == CachedTowerMainMapLayout.BossFloorNumber)
			{
				return false;
			}

			FRandomStream FloorRng(RunSeed + StageNum * 1901 + SeedOffset + FloorNumber * 53);
			const T66TowerMapTerrain::FFloor* Floor = T66FindTowerFloorByNumber(CachedTowerMainMapLayout, FloorNumber);
			if (!Floor)
			{
				return false;
			}

			auto TryKnownSlotSet = [&](const TArray<FVector>& Slots) -> bool
			{
				if (Slots.Num() <= 0)
				{
					return false;
				}

				const int32 StartIndex = FloorRng.RandRange(0, Slots.Num() - 1);
				for (int32 Offset = 0; Offset < Slots.Num(); ++Offset)
				{
					const FVector Candidate = Slots[(StartIndex + Offset) % Slots.Num()];
					if (GetTowerFloorIndexForLocation(Candidate) != FloorNumber)
					{
						continue;
					}

					if (IsGoodLoc(Candidate))
					{
						OutLocation = Candidate;
						return true;
					}
				}

				return false;
			};

			if (TryKnownSlotSet(Floor->CachedContentSpawnSlots)
				|| TryKnownSlotSet(Floor->CachedOptionalSpawnSlots)
				|| TryKnownSlotSet(Floor->CachedMainPathSpawnSlots))
			{
				return true;
			}

			for (int32 Attempt = 0; Attempt < 8; ++Attempt)
			{
				if (!T66TowerMapTerrain::TryGetFloorTileCenterSpawnLocation(
					World,
					CachedTowerMainMapLayout,
					FloorNumber,
					FloorRng,
					OutLocation,
					FMath::Min(EdgePadding, 900.f),
					FMath::Min(HolePadding, 1000.f),
					500.f))
				{
					continue;
				}

				if (GetTowerFloorIndexForLocation(OutLocation) != FloorNumber)
				{
					continue;
				}

				if (IsGoodLoc(OutLocation))
				{
					return true;
				}
			}

			for (int32 Attempt = 0; Attempt < 24; ++Attempt)
			{
				if (!T66TowerMapTerrain::TryGetRandomSurfaceLocationOnFloor(
					World,
					CachedTowerMainMapLayout,
					FloorNumber,
					FloorRng,
					OutLocation,
					EdgePadding,
					HolePadding))
				{
					continue;
				}

				if (GetTowerFloorIndexForLocation(OutLocation) != FloorNumber)
				{
					continue;
				}

				if (IsGoodLoc(OutLocation))
				{
					return true;
				}
			}

			static const FVector2D FallbackOffsets[] =
			{
				FVector2D(1800.f, 0.f),
				FVector2D(-1800.f, 0.f),
				FVector2D(0.f, 1800.f),
				FVector2D(0.f, -1800.f),
				FVector2D(2500.f, 1400.f),
				FVector2D(-2500.f, 1400.f),
				FVector2D(2500.f, -1400.f),
				FVector2D(-2500.f, -1400.f)
			};
			const FVector Anchor = !Floor->ArrivalPoint.IsNearlyZero() ? Floor->ArrivalPoint : Floor->Center;
			const int32 FallbackStartIndex = FloorRng.RandRange(0, UE_ARRAY_COUNT(FallbackOffsets) - 1);
			for (int32 Offset = 0; Offset < UE_ARRAY_COUNT(FallbackOffsets); ++Offset)
			{
				const FVector2D FallbackOffset = FallbackOffsets[(FallbackStartIndex + Offset) % UE_ARRAY_COUNT(FallbackOffsets)];
				const FVector Candidate(Anchor.X + FallbackOffset.X, Anchor.Y + FallbackOffset.Y, Floor->SurfaceZ);
				if (GetTowerFloorIndexForLocation(Candidate) != FloorNumber)
				{
					continue;
				}

				if (IsGoodLoc(Candidate))
				{
					OutLocation = Candidate;
					return true;
				}
			}

			return false;
		};

		FActorSpawnParameters OccupantSpawnParams;
		OccupantSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		auto SpawnTowerActorOnFloor = [&](UClass* Cls, const int32 FloorNumber, const int32 SeedOffset, const float EdgePadding, const float HolePadding) -> AActor*
		{
			if (FloorNumber == INDEX_NONE
				|| FloorNumber == CachedTowerMainMapLayout.StartFloorNumber
				|| FloorNumber == CachedTowerMainMapLayout.BossFloorNumber)
			{
				return nullptr;
			}

			for (int32 SpawnAttempt = 0; SpawnAttempt < 5; ++SpawnAttempt)
			{
				FVector SpawnLoc = FVector::ZeroVector;
				if (!TryFindTowerFloorLocation(FloorNumber, SeedOffset + (SpawnAttempt * 97), EdgePadding, HolePadding, SpawnLoc))
				{
					continue;
				}

				AActor* SpawnedActor = World->SpawnActor<AActor>(Cls, SpawnLoc, FRotator::ZeroRotator, OccupantSpawnParams);
				if (!SpawnedActor)
				{
					continue;
				}

				if (!ResnapTowerActorToFloor(SpawnedActor, FloorNumber))
				{
					SpawnedActor->Destroy();
					continue;
				}

				const int32 StrictLocationFloorNumber = GetTowerFloorIndexForLocation(SpawnedActor->GetActorLocation());
				if (StrictLocationFloorNumber != FloorNumber)
				{
					SpawnedActor->Destroy();
					continue;
				}

				const int32 ResolvedFloorNumber = T66ResolveTowerFloorForActorPhysical(this, CachedTowerMainMapLayout, SpawnedActor);
				if (ResolvedFloorNumber != FloorNumber)
				{
					SpawnedActor->Destroy();
					continue;
				}

				UsedLocs.Add(SpawnedActor->GetActorLocation());
				return SpawnedActor;
			}

			return nullptr;
		};

		auto RememberTaggedTowerActor = [&](AActor* Actor, const FName Tag)
		{
			if (!Actor || Tag.IsNone())
			{
				return;
			}

			Actor->Tags.AddUnique(Tag);
			ExistingTowerOccupantTags.Add(Tag);
			T66RememberTaggedActor(Actor, Tag);
		};

		auto SpawnTaggedTowerActorOnFloor = [&](
			UClass* Cls,
			const int32 FloorNumber,
			const int32 SeedOffset,
			const float EdgePadding,
			const float HolePadding,
			const FName Tag) -> AActor*
		{
			if (!Cls || FloorNumber == INDEX_NONE || Tag.IsNone() || ExistingTowerOccupantTags.Contains(Tag))
			{
				return nullptr;
			}

			AActor* SpawnedActor = SpawnTowerActorOnFloor(Cls, FloorNumber, SeedOffset, EdgePadding, HolePadding);
			if (SpawnedActor)
			{
				RememberTaggedTowerActor(SpawnedActor, Tag);
			}
			return SpawnedActor;
		};

		auto TryFindTowerRoomLocation =
			[&](const T66TowerMapTerrain::FFloor& Floor, const T66TowerMapTerrain::FRoom& Room, const int32 SeedOffset, const float EdgePadding, const float HolePadding, FVector& OutLocation) -> bool
		{
			if (Floor.FloorNumber == INDEX_NONE
				|| Floor.FloorNumber == CachedTowerMainMapLayout.StartFloorNumber
				|| Floor.FloorNumber == CachedTowerMainMapLayout.BossFloorNumber)
			{
				return false;
			}

			for (int32 Attempt = 0; Attempt < 8; ++Attempt)
			{
				FRandomStream RoomRng(RunSeed + StageNum * 1901 + SeedOffset + Floor.FloorNumber * 53 + Room.RoomId * 101 + Attempt * 97);
				if (!T66TowerMapTerrain::TryGetRoomSurfaceLocation(
					World,
					CachedTowerMainMapLayout,
					Floor,
					Room,
					RoomRng,
					OutLocation,
					EdgePadding,
					HolePadding,
					500.f))
				{
					continue;
				}

				if (GetTowerFloorIndexForLocation(OutLocation) != Floor.FloorNumber)
				{
					continue;
				}

				if (IsGoodLoc(OutLocation))
				{
					return true;
				}
			}

			return false;
		};

		auto SpawnTowerActorInRoom =
			[&](UClass* Cls, const T66TowerMapTerrain::FFloor& Floor, const T66TowerMapTerrain::FRoom& Room, const int32 SeedOffset, const float EdgePadding, const float HolePadding) -> AActor*
		{
			if (!Cls
				|| Floor.FloorNumber == INDEX_NONE
				|| Floor.FloorNumber == CachedTowerMainMapLayout.StartFloorNumber
				|| Floor.FloorNumber == CachedTowerMainMapLayout.BossFloorNumber)
			{
				return nullptr;
			}

			auto TrySpawnAtLocation = [&](const FVector& SpawnLoc) -> AActor*
			{
				AActor* SpawnedActor = World->SpawnActor<AActor>(Cls, SpawnLoc, FRotator::ZeroRotator, OccupantSpawnParams);
				if (!SpawnedActor)
				{
					return nullptr;
				}

				if (!ResnapTowerActorToFloor(SpawnedActor, Floor.FloorNumber))
				{
					SpawnedActor->Destroy();
					return nullptr;
				}

				const int32 StrictLocationFloorNumber = GetTowerFloorIndexForLocation(SpawnedActor->GetActorLocation());
				if (StrictLocationFloorNumber != Floor.FloorNumber)
				{
					SpawnedActor->Destroy();
					return nullptr;
				}

				const int32 ResolvedFloorNumber = T66ResolveTowerFloorForActorPhysical(this, CachedTowerMainMapLayout, SpawnedActor);
				if (ResolvedFloorNumber != Floor.FloorNumber)
				{
					SpawnedActor->Destroy();
					return nullptr;
				}

				UsedLocs.Add(SpawnedActor->GetActorLocation());
				return SpawnedActor;
			};

			// Reward slots first (design ref section 1.7): the room course designed
			// its payoff points (deck tops, ring pits, bridge ends) — content lands
			// there, marked by the reward beacons, instead of random open ground.
			for (const FVector& RewardSlot : Room.RewardSlots)
			{
				bool bSlotTaken = false;
				for (const FVector& Used : UsedLocs)
				{
					if (FVector::DistSquared2D(Used, RewardSlot) < FMath::Square(250.0f))
					{
						bSlotTaken = true;
						break;
					}
				}
				if (bSlotTaken)
				{
					continue;
				}

				if (AActor* SpawnedActor = TrySpawnAtLocation(RewardSlot + FVector(0.0f, 0.0f, 80.0f)))
				{
					return SpawnedActor;
				}
			}

			for (int32 SpawnAttempt = 0; SpawnAttempt < 5; ++SpawnAttempt)
			{
				FVector SpawnLoc = FVector::ZeroVector;
				if (!TryFindTowerRoomLocation(Floor, Room, SeedOffset + (SpawnAttempt * 197), EdgePadding, HolePadding, SpawnLoc))
				{
					continue;
				}

				if (AActor* SpawnedActor = TrySpawnAtLocation(SpawnLoc))
				{
					return SpawnedActor;
				}
			}

			return nullptr;
		};

		const UT66TowerTuningConfig& TowerRoomTuning = UT66TowerTuningConfig::GetRuntimeConfig();
		bool bTowerRoomContentMode = false;
		int32 TowerRoomContentExpectedRooms = 0;
		for (const int32 FloorNumber : TowerMobFloorNumbers)
		{
			const T66TowerMapTerrain::FFloor* Floor = T66FindTowerFloorByNumber(CachedTowerMainMapLayout, FloorNumber);
			if (!Floor)
			{
				continue;
			}

			for (const T66TowerMapTerrain::FRoom& Room : Floor->Rooms)
			{
				const FT66TowerRoomRuleTuning* RoomRule = TowerRoomTuning.FindRoomRule(Room.RoomRuleID);
				if (RoomRule && RoomRule->RewardContentSlots.Max > 0)
				{
					bTowerRoomContentMode = true;
					++TowerRoomContentExpectedRooms;
				}
			}
		}

		if (bTowerRoomContentMode)
		{
			int32 TowerRoomContentPlaced = 0;
			TSet<int32> FloorsWithLootBag;
			TSet<int32> FloorsWithTotem;

			auto RememberRoomContent = [&](AActor* Actor, const FName ContentTag, const FName ExtraTag = NAME_None)
			{
				if (!Actor)
				{
					return;
				}

				RememberTaggedTowerActor(Actor, ContentTag);
				if (!ExtraTag.IsNone())
				{
					RememberTaggedTowerActor(Actor, ExtraTag);
				}
			};

			for (const int32 FloorNumber : TowerMobFloorNumbers)
			{
				const T66TowerMapTerrain::FFloor* Floor = T66FindTowerFloorByNumber(CachedTowerMainMapLayout, FloorNumber);
				if (!Floor)
				{
					continue;
				}

				TArray<const T66TowerMapTerrain::FRoom*, TInlineAllocator<16>> ContentRooms;
				for (const T66TowerMapTerrain::FRoom& Room : Floor->Rooms)
				{
					const FT66TowerRoomRuleTuning* RoomRule = TowerRoomTuning.FindRoomRule(Room.RoomRuleID);
					if (RoomRule && RoomRule->RewardContentSlots.Max > 0)
					{
						ContentRooms.Add(&Room);
					}
				}
				if (ContentRooms.Num() <= 0)
				{
					continue;
				}

				int32 VendorRoomIndex = INDEX_NONE;
				for (int32 RoomIndex = 0; RoomIndex < ContentRooms.Num(); ++RoomIndex)
				{
					const T66TowerMapTerrain::FRoom* Room = ContentRooms[RoomIndex];
					if (Room && !Room->bContainsArrival && !Room->bContainsExit)
					{
						VendorRoomIndex = RoomIndex;
						break;
					}
				}
				if (VendorRoomIndex == INDEX_NONE)
				{
					VendorRoomIndex = 0;
				}

				for (int32 RoomIndex = 0; RoomIndex < ContentRooms.Num(); ++RoomIndex)
				{
					const T66TowerMapTerrain::FRoom* Room = ContentRooms[RoomIndex];
					if (!Room)
					{
						continue;
					}

					const FName ContentTag(*FString::Printf(TEXT("T66_Tower_RoomContent_%02d_%02d"), FloorNumber, Room->RoomId));
					if (ExistingTowerOccupantTags.Contains(ContentTag))
					{
						++TowerRoomContentPlaced;
						continue;
					}

					AActor* SpawnedContent = nullptr;
					if (RoomIndex == VendorRoomIndex)
					{
						const FName VendorTag(*FString::Printf(TEXT("T66_Tower_Vendor_%02d"), FloorNumber));
						if (!ExistingTowerOccupantTags.Contains(VendorTag))
						{
							if (AT66VendorNPC* Vendor = Cast<AT66VendorNPC>(SpawnTowerActorInRoom(
								AT66VendorNPC::StaticClass(),
								*Floor,
								*Room,
								5100,
								900.f,
								1200.f)))
							{
								Vendor->SetActorScale3D(FVector(0.82f));
								RememberRoomContent(Vendor, ContentTag, VendorTag);
								SpawnedContent = Vendor;
								++TowerSpawnedVendors;
								UE_LOG(
									LogT66GameMode,
									Log,
									TEXT("[T66Proof][VendorPerFloor] Stage=%d Floor=%d Spawned=1 ExpectedRule=GuaranteedPerMobFloor"),
									StageNum,
									FloorNumber);
							}
						}
					}

					if (!SpawnedContent
						&& RemainingTotems > 0
						&& !FloorsWithTotem.Contains(FloorNumber)
						&& (!PlayerExperience
							|| PlayerExperience->ShouldSpawnDifficultyTotemOnTowerFloor(
								Difficulty,
								false,
								FloorNumber,
								CachedTowerMainMapLayout.FirstMobFloorNumber,
								CachedTowerMainMapLayout.LastMobFloorNumber)))
					{
						const FName TotemTag(*FString::Printf(TEXT("T66_Tower_DifficultyTotem_%02d"), FloorNumber));
						if (!ExistingTowerOccupantTags.Contains(TotemTag))
						{
							if (AT66DifficultyTotem* Totem = Cast<AT66DifficultyTotem>(SpawnTowerActorInRoom(
								AT66DifficultyTotem::StaticClass(),
								*Floor,
								*Room,
								8500,
								900.f,
								1200.f)))
							{
								ConfigureTotem(Totem);
								RememberRoomContent(Totem, ContentTag, TotemTag);
								SpawnedContent = Totem;
								FloorsWithTotem.Add(FloorNumber);
								--RemainingTotems;
								++TowerSpawnedTotems;
							}
						}
					}

					if (!SpawnedContent && !FloorsWithLootBag.Contains(FloorNumber))
					{
						const FName LootBagTag(*FString::Printf(TEXT("T66_Tower_LootBag_%02d"), FloorNumber));
						if (!ExistingTowerOccupantTags.Contains(LootBagTag))
						{
							if (AT66LootBagPickup* LootBag = Cast<AT66LootBagPickup>(SpawnTowerActorInRoom(
								AT66LootBagPickup::StaticClass(),
								*Floor,
								*Room,
								4900,
								900.f,
								1200.f)))
							{
								ConfigureLootBag(LootBag, FName(TEXT("TowerLootBagRarity")));
								RememberRoomContent(LootBag, ContentTag, LootBagTag);
								SpawnedContent = LootBag;
								FloorsWithLootBag.Add(FloorNumber);
								++TowerSpawnedLootBags;
							}
						}
					}

					if (!SpawnedContent)
					{
						const bool bPreferChest = ((FloorNumber + RoomIndex) % 2) == 0;
						if (bPreferChest)
						{
							if (AT66ChestInteractable* Chest = Cast<AT66ChestInteractable>(SpawnTowerActorInRoom(
								AT66ChestInteractable::StaticClass(),
								*Floor,
								*Room,
								7600 + RoomIndex * 41,
								900.f,
								1200.f)))
							{
								ConfigureChest(Chest);
								RememberRoomContent(Chest, ContentTag);
								SpawnedContent = Chest;
								++TowerSpawnedChests;
							}
						}

						if (!SpawnedContent)
						{
							if (AT66CrateInteractable* Crate = Cast<AT66CrateInteractable>(SpawnTowerActorInRoom(
								AT66CrateInteractable::StaticClass(),
								*Floor,
								*Room,
								7700 + RoomIndex * 41,
								900.f,
								1200.f)))
							{
								ConfigureCrate(Crate);
								RememberRoomContent(Crate, ContentTag);
								SpawnedContent = Crate;
								++TowerSpawnedCrates;
							}
						}
					}

					if (SpawnedContent)
					{
						++TowerRoomContentPlaced;

						// Second reward (section 1.7 "one or two rewards"): rooms with
						// two designed payoff points may fill both — the bonus chest
						// lands on the remaining beacon slot.
						const FT66TowerRoomRuleTuning* BonusRule = TowerRoomTuning.FindRoomRule(Room->RoomRuleID);
						if (BonusRule
							&& BonusRule->RewardContentSlots.Max >= 2
							&& Room->RewardSlots.Num() >= 2
							&& ((FloorNumber + RoomIndex) % 2) == 0)
						{
							if (AT66ChestInteractable* BonusChest = Cast<AT66ChestInteractable>(SpawnTowerActorInRoom(
								AT66ChestInteractable::StaticClass(),
								*Floor,
								*Room,
								9900 + RoomIndex * 53,
								900.f,
								1200.f)))
							{
								ConfigureChest(BonusChest);
								RememberRoomContent(BonusChest, ContentTag);
								++TowerSpawnedChests;
							}
						}
					}
				}
			}

			CountChests = TowerSpawnedChests;
			CountCrates = TowerSpawnedCrates;
			CountLootWheels = TowerSpawnedLootWheels;
			CountFountains = TowerSpawnedFountains;
			CountTotems = TowerSpawnedTotems;
			RemainingChests = 0;
			RemainingCrates = 0;
			RemainingLootWheels = 0;
			RemainingFountains = 0;
			RemainingTotems = 0;

			const bool bRoomContentProofPass = TowerRoomContentPlaced == TowerRoomContentExpectedRooms
				&& TowerSpawnedVendors == TowerMobFloorNumbers.Num();
			if (bRoomContentProofPass)
			{
				UE_LOG(
					LogT66GameMode,
					Log,
					TEXT("[T66Proof][TowerRoomContentSummary] Stage=%d Result=PASS Floors=%d Rooms=%d ContentRooms=%d Vendors=%d ExpectedVendors=%d Rule=RoomRewardContentSlots"),
					StageNum,
					TowerMobFloorNumbers.Num(),
					TowerRoomContentExpectedRooms,
					TowerRoomContentPlaced,
					TowerSpawnedVendors,
					TowerMobFloorNumbers.Num());
			}
			else
			{
				UE_LOG(
					LogT66GameMode,
					Warning,
					TEXT("[T66Proof][TowerRoomContentSummary] Stage=%d Result=FAIL Floors=%d Rooms=%d ContentRooms=%d Vendors=%d ExpectedVendors=%d Rule=RoomRewardContentSlots"),
					StageNum,
					TowerMobFloorNumbers.Num(),
					TowerRoomContentExpectedRooms,
					TowerRoomContentPlaced,
					TowerSpawnedVendors,
					TowerMobFloorNumbers.Num());
			}
		}

		if (!bTowerRoomContentMode)
		{
		int32 GuaranteedUtilityFountains = 0;
		for (const int32 FloorNumber : MobFloorNumbers)
		{
			const int32 FloorChestCount = TowerChestCountByFloor.FindRef(FloorNumber);
			for (int32 ChestIndex = 0; ChestIndex < FloorChestCount; ++ChestIndex)
			{
				for (int32 SpawnAttempt = 0; SpawnAttempt < 4; ++SpawnAttempt)
				{
					if (AT66ChestInteractable* Chest = Cast<AT66ChestInteractable>(SpawnTowerActorOnFloor(
						AT66ChestInteractable::StaticClass(),
						FloorNumber,
						7600 + (ChestIndex * 41) + (SpawnAttempt * 211),
						1100.f,
						1200.f)))
					{
						ConfigureChest(Chest);
						if (!ResnapTowerActorToFloor(Chest, FloorNumber))
						{
							Chest->Destroy();
							continue;
						}
						++TowerSpawnedChests;
						break;
					}
				}
			}

			const int32 FloorCrateCount = TowerCrateCountByFloor.FindRef(FloorNumber);
			for (int32 CrateIndex = 0; CrateIndex < FloorCrateCount; ++CrateIndex)
			{
				for (int32 SpawnAttempt = 0; SpawnAttempt < 4; ++SpawnAttempt)
				{
					if (AT66CrateInteractable* Crate = Cast<AT66CrateInteractable>(SpawnTowerActorOnFloor(
						AT66CrateInteractable::StaticClass(),
						FloorNumber,
						7700 + (CrateIndex * 41) + (SpawnAttempt * 211),
						1100.f,
						1200.f)))
					{
						ConfigureCrate(Crate);
						if (!ResnapTowerActorToFloor(Crate, FloorNumber))
						{
							Crate->Destroy();
							continue;
						}
						++TowerSpawnedCrates;
						break;
					}
				}
			}

			if (TowerFountainFloorNumbers.Contains(FloorNumber))
			{
				if (SpawnTowerActorOnFloor(AT66FountainInteractable::StaticClass(), FloorNumber, 7800, 1200.f, 1300.f))
				{
					++GuaranteedUtilityFountains;
					++TowerSpawnedFountains;
				}
			}
		}

		RemainingChests = 0;
		RemainingCrates = 0;
		RemainingFountains = FMath::Max(0, RemainingFountains - GuaranteedUtilityFountains);
		RemainingTotems = CountTotems;

		if (RemainingTotems > 0)
		{
			for (const int32 FloorNumber : TowerMobFloorNumbers)
			{
				if (!PlayerExperience
					|| PlayerExperience->ShouldSpawnDifficultyTotemOnTowerFloor(
						Difficulty,
						false,
						FloorNumber,
						CachedTowerMainMapLayout.FirstMobFloorNumber,
						CachedTowerMainMapLayout.LastMobFloorNumber))
				{
					const FName TotemTag(*FString::Printf(TEXT("T66_Tower_DifficultyTotem_%02d"), FloorNumber));
					if (ExistingTowerOccupantTags.Contains(TotemTag))
					{
						--RemainingTotems;
						++TowerSpawnedTotems;
						continue;
					}

					bool bPlacedTotem = false;
					for (int32 SpawnAttempt = 0; SpawnAttempt < 4; ++SpawnAttempt)
					{
						if (AT66DifficultyTotem* Totem = Cast<AT66DifficultyTotem>(SpawnTowerActorOnFloor(
							AT66DifficultyTotem::StaticClass(),
							FloorNumber,
							8500 + (SpawnAttempt * 211),
							1300.f,
							1700.f)))
						{
							ConfigureTotem(Totem);
							if (!ResnapTowerActorToFloor(Totem, FloorNumber))
							{
								Totem->Destroy();
								continue;
							}

							RememberTaggedTowerActor(Totem, TotemTag);
							--RemainingTotems;
							++TowerSpawnedTotems;
							bPlacedTotem = true;
							break;
						}
					}

					if (!bPlacedTotem)
					{
						UE_LOG(
							LogT66GameMode,
							Warning,
							TEXT("[MAP] Tower population could not place required difficulty totem on floor %d."),
							FloorNumber);
					}
				}
			}

			RemainingTotems = 0;
		}

		for (const int32 FloorNumber : MobFloorNumbers)
		{
			const FName LootBagTag(*FString::Printf(TEXT("T66_Tower_LootBag_%02d"), FloorNumber));
			if (AT66LootBagPickup* LootBag = Cast<AT66LootBagPickup>(SpawnTaggedTowerActorOnFloor(
				AT66LootBagPickup::StaticClass(),
				FloorNumber,
				4900,
				1200.f,
				1400.f,
				LootBagTag)))
			{
				ConfigureLootBag(LootBag, FName(TEXT("TowerLootBagRarity")));
				++TowerSpawnedLootBags;
			}

		}

		for (const int32 FloorNumber : MobFloorNumbers)
		{
			const FName VendorTag(*FString::Printf(TEXT("T66_Tower_Vendor_%02d"), FloorNumber));
			if (AT66VendorNPC* Vendor = Cast<AT66VendorNPC>(SpawnTaggedTowerActorOnFloor(
				AT66VendorNPC::StaticClass(),
				FloorNumber,
				5100,
				1800.f,
				2200.f,
				VendorTag)))
			{
				Vendor->SetActorScale3D(FVector(0.82f));
				++TowerSpawnedVendors;
				UE_LOG(
					LogT66GameMode,
					Log,
					TEXT("[T66Proof][VendorPerFloor] Stage=%d Floor=%d Spawned=1 ExpectedRule=GuaranteedPerMobFloor"),
					StageNum,
					FloorNumber);
			}
		}

		}

		for (const int32 FloorNumber : MobFloorNumbers)
		{
			const FName CasinoTag(*FString::Printf(TEXT("T66_Tower_Casino_%02d"), FloorNumber));
			const int32 CasinoSeed = RunSeed + StageNum * 1901 + 5600 + FloorNumber * 53;
			FRandomStream CasinoFloorRng(CasinoSeed);
			const bool bShouldSpawnCasino = CasinoFloorRng.FRand() <= T66TowerCasinoSpawnChance;
			if (RunState)
			{
				RunState->RecordLuckQuantityBool(
					FName(TEXT("TowerCasinoNPCFloorSpawned")),
					bShouldSpawnCasino,
					T66TowerCasinoSpawnChance,
					INDEX_NONE,
					CasinoSeed);
			}

			if (!bShouldSpawnCasino)
			{
				continue;
			}

			if (AT66CasinoNPC* CasinoNPC = Cast<AT66CasinoNPC>(SpawnTaggedTowerActorOnFloor(
				AT66CasinoNPC::StaticClass(),
				FloorNumber,
				5600,
				1800.f,
				2200.f,
				CasinoTag)))
			{
				CasinoNPC->SetActorScale3D(FVector(0.82f));
				++TowerSpawnedCasinoNPCs;
				UE_LOG(
					LogT66GameMode,
					Log,
					TEXT("[T66Proof][CasinoPerFloorChance] Stage=%d Floor=%d Spawned=1 Chance=%.3f Seed=%d"),
					StageNum,
					FloorNumber,
					T66TowerCasinoSpawnChance,
					CasinoSeed);
			}
		}

		if (UtilityVehicleFloorNumber != INDEX_NONE && FT66ShelvedFeatureGate::IsVehicleInteractablesEnabled())
		{
			const FName VehicleTag(*FString::Printf(TEXT("T66_Tower_Vehicle_%02d"), UtilityVehicleFloorNumber));
			if (AT66VehicleInteractable* Vehicle = Cast<AT66VehicleInteractable>(SpawnTaggedTowerActorOnFloor(
				AT66VehicleInteractable::StaticClass(),
				UtilityVehicleFloorNumber,
				6600,
				2000.f,
				2300.f,
				VehicleTag)))
			{
				Vehicle->SetActorScale3D(FVector(0.82f));
				++TowerSpawnedVehicles;
			}
		}

		const bool bOuroborosEligibleStage = StageNum >= 1 && StageNum <= 15 && !T66_IsDifficultyBossStage(StageNum);
		const FName OuroborosTag(TEXT("T66_Tower_Ouroboros"));
		if (bOuroborosEligibleStage
			&& MobFloorNumbers.Contains(T66OuroborosHiddenSpawnFloorNumber)
			&& !ExistingTowerOccupantTags.Contains(OuroborosTag))
		{
			const int32 OuroborosSeed = RunSeed + StageNum * 1901 + 7600 + T66OuroborosHiddenSpawnFloorNumber * 53;
			FRandomStream OuroborosRng(OuroborosSeed);
			const bool bShouldSpawnOuroboros = OuroborosRng.FRand() <= T66OuroborosHiddenSpawnChance;
			if (bShouldSpawnOuroboros)
			{
				if (AT66OuroborosNPC* Ouroboros = Cast<AT66OuroborosNPC>(SpawnTaggedTowerActorOnFloor(
					AT66OuroborosNPC::StaticClass(),
					T66OuroborosHiddenSpawnFloorNumber,
					7600,
					2200.f,
					2300.f,
					OuroborosTag)))
				{
					Ouroboros->Tags.AddUnique(OuroborosTag);
					ExistingTowerOccupantTags.Add(OuroborosTag);
					T66RememberTaggedActor(Ouroboros, OuroborosTag);
					++TowerSpawnedOuroboros;
				}
			}

			UE_LOG(
				LogT66GameMode,
				Log,
				TEXT("[T66Proof][OuroborosHiddenSpawn] Stage=%d Eligible=%d Floor=%d Spawned=%d Chance=%.3f Seed=%d"),
				StageNum,
				bOuroborosEligibleStage ? 1 : 0,
				T66OuroborosHiddenSpawnFloorNumber,
				TowerSpawnedOuroboros,
				T66OuroborosHiddenSpawnChance,
				OuroborosSeed);
		}

		const FName SaintTag(TEXT("T66_Tower_Saint"));
		if (!ExistingTowerOccupantTags.Contains(SaintTag))
		{
			for (int32 Index = MobFloorNumbers.Num() - 1; Index > 0; --Index)
			{
				const int32 SwapIndex = TowerFloorRng.RandRange(0, Index);
				if (SwapIndex != Index)
				{
					MobFloorNumbers.Swap(Index, SwapIndex);
				}
			}

			for (const int32 FloorNumber : MobFloorNumbers)
			{
				FVector SaintLoc = FVector::ZeroVector;
				if (!TryFindTowerFloorLocation(FloorNumber, 7100, 1800.f, 2200.f, SaintLoc))
				{
					continue;
				}

				if (AT66SaintNPC* Saint = World->SpawnActor<AT66SaintNPC>(
					AT66SaintNPC::StaticClass(), SaintLoc, FRotator::ZeroRotator, OccupantSpawnParams))
				{
					const FName SaintFloorTag(*FString::Printf(TEXT("T66_Tower_Saint_%02d"), FloorNumber));
					if (!ResnapTowerActorToFloor(Saint, FloorNumber))
					{
						Saint->Destroy();
						continue;
					}
					Saint->Tags.AddUnique(SaintTag);
					Saint->Tags.AddUnique(SaintFloorTag);
					ExistingTowerOccupantTags.Add(SaintTag);
					ExistingTowerOccupantTags.Add(SaintFloorTag);
					T66RememberTaggedActor(Saint, SaintTag);
					T66RememberTaggedActor(Saint, SaintFloorTag);
					UsedLocs.Add(Saint->GetActorLocation());
					++TowerSpawnedSaints;
				}
				break;
			}
		}

		for (const int32 FloorNumber : MobFloorNumbers)
		{
			if (RemainingChests > 0)
			{
				if (AT66ChestInteractable* Chest = Cast<AT66ChestInteractable>(SpawnTowerActorOnFloor(AT66ChestInteractable::StaticClass(), FloorNumber, 8100, 1300.f, 1700.f)))
				{
					ConfigureChest(Chest);
					if (!ResnapTowerActorToFloor(Chest, FloorNumber))
					{
						Chest->Destroy();
						continue;
					}
					++TowerSpawnedChests;
					--RemainingChests;
				}
			}

			if (RemainingCrates > 0)
			{
				if (AT66CrateInteractable* Crate = Cast<AT66CrateInteractable>(SpawnTowerActorOnFloor(AT66CrateInteractable::StaticClass(), FloorNumber, 8300, 1300.f, 1700.f)))
				{
					ConfigureCrate(Crate);
					if (!ResnapTowerActorToFloor(Crate, FloorNumber))
					{
						Crate->Destroy();
						continue;
					}
					--RemainingCrates;
					++TowerSpawnedCrates;
					continue;
				}
			}

			if (RemainingLootWheels > 0)
			{
				if (AT66LootWheelInteractable* LootWheel = Cast<AT66LootWheelInteractable>(SpawnTowerActorOnFloor(AT66LootWheelInteractable::StaticClass(), FloorNumber, 8350, 1300.f, 1700.f)))
				{
					ConfigureLootWheel(LootWheel);
					if (!ResnapTowerActorToFloor(LootWheel, FloorNumber))
					{
						LootWheel->Destroy();
						continue;
					}
					--RemainingLootWheels;
					++TowerSpawnedLootWheels;
					continue;
				}
			}

			if (RemainingFountains > 0)
			{
				if (SpawnTowerActorOnFloor(AT66FountainInteractable::StaticClass(), FloorNumber, 8400, 1300.f, 1700.f))
				{
					--RemainingFountains;
					++TowerSpawnedFountains;
					continue;
				}
			}
		}

		if (UT66TrapSubsystem* TrapSubsystem = World->GetSubsystem<UT66TrapSubsystem>())
		{
			TrapSubsystem->SpawnTowerStageTraps(CachedTowerMainMapLayout, StageNum, Difficulty, RunSeed);
			SyncTowerTrapActivation(true);
		}

		int32 TowerRemovedStartFloorLeaks = 0;
		FString TowerRemovedStartFloorLeakClasses;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			const bool bExtraWorldInteractable =
				Actor->IsA<AT66VendorNPC>()
				|| Actor->IsA<AT66CasinoNPC>()
				|| Actor->IsA<AT66ChestInteractable>()
				|| Actor->IsA<AT66CrateInteractable>()
				|| Actor->IsA<AT66LootWheelInteractable>()
				|| Actor->IsA<AT66LootBagPickup>()
				|| Actor->IsA<AT66FountainInteractable>()
				|| Actor->IsA<AT66DifficultyTotem>()
				|| Actor->IsA<AT66VehicleInteractable>()
				|| Actor->IsA<AT66SaintNPC>()
				|| Actor->IsA<AT66OuroborosNPC>();
			if (!bExtraWorldInteractable)
			{
				continue;
			}

			const int32 TaggedFloorNumber = T66ReadTowerFloorTag(Actor);
			const int32 StrictLocationFloorNumber = GetTowerFloorIndexForLocation(Actor->GetActorLocation());
			const int32 PhysicalFloorNumber = T66ResolveTowerFloorForActorPhysical(this, CachedTowerMainMapLayout, Actor);
			if (TaggedFloorNumber != CachedTowerMainMapLayout.StartFloorNumber
				&& StrictLocationFloorNumber != CachedTowerMainMapLayout.StartFloorNumber
				&& PhysicalFloorNumber != CachedTowerMainMapLayout.StartFloorNumber)
			{
				continue;
			}

			++TowerRemovedStartFloorLeaks;
			if (TowerRemovedStartFloorLeakClasses.Len() < 256)
			{
				if (!TowerRemovedStartFloorLeakClasses.IsEmpty())
				{
					TowerRemovedStartFloorLeakClasses += TEXT(",");
				}
				TowerRemovedStartFloorLeakClasses += FString::Printf(
					TEXT("%s(Tag=%d,Loc=%d,Phys=%d)"),
					*Actor->GetClass()->GetName(),
					TaggedFloorNumber,
					StrictLocationFloorNumber,
					PhysicalFloorNumber);
			}
			Actor->Destroy();
		}
		if (TowerRemovedStartFloorLeaks > 0)
		{
			UE_LOG(
				LogT66GameMode,
				Warning,
				TEXT("[MAP] Removed %d invalid tower interactables from reserved start floor: %s"),
				TowerRemovedStartFloorLeaks,
				TowerRemovedStartFloorLeakClasses.IsEmpty() ? TEXT("Unknown") : *TowerRemovedStartFloorLeakClasses);
		}

		UE_LOG(
			LogT66GameMode,
			Log,
				TEXT("[MAP] Tower population spawned on mob floors=%d: chests %d/%d, crates %d/%d, loot wheels %d/%d, fountains %d/%d, loot bags %d/%d, vendors %d/%d, casino NPCs %d/%d chance %.0f%%, vehicles %d, ouroboros %d, saints %d, totems %d/%d, removed start-floor leaks %d."),
			TowerMobFloorNumbers.Num(),
			TowerSpawnedChests,
			CountChests,
			TowerSpawnedCrates,
			CountCrates,
			TowerSpawnedLootWheels,
			CountLootWheels,
			TowerSpawnedFountains,
			CountFountains,
			TowerSpawnedLootBags,
			TowerMobFloorNumbers.Num(),
			TowerSpawnedVendors,
			TowerMobFloorNumbers.Num(),
			TowerSpawnedCasinoNPCs,
			TowerMobFloorNumbers.Num(),
			T66TowerCasinoSpawnChance * 100.f,
			TowerSpawnedVehicles,
			TowerSpawnedOuroboros,
			TowerSpawnedSaints,
			TowerSpawnedTotems,
			CountTotems,
			TowerRemovedStartFloorLeaks);

		const bool bVendorProofPass = TowerSpawnedVendors == TowerMobFloorNumbers.Num();
		if (bVendorProofPass)
		{
			UE_LOG(
				LogT66GameMode,
				Log,
				TEXT("[T66Proof][VendorPerFloorSummary] Stage=%d Result=PASS Spawned=%d Expected=%d Rule=GuaranteedPerMobFloor"),
				StageNum,
				TowerSpawnedVendors,
				TowerMobFloorNumbers.Num());
		}
		else
		{
			UE_LOG(
				LogT66GameMode,
				Warning,
				TEXT("[T66Proof][VendorPerFloorSummary] Stage=%d Result=FAIL Spawned=%d Expected=%d Rule=GuaranteedPerMobFloor"),
				StageNum,
				TowerSpawnedVendors,
				TowerMobFloorNumbers.Num());
		}

		UE_LOG(
			LogT66GameMode,
			Log,
			TEXT("[T66Proof][CasinoChanceSummary] Stage=%d Spawned=%d Floors=%d Chance=%.3f Rule=PerFloorChance"),
			StageNum,
			TowerSpawnedCasinoNPCs,
			TowerMobFloorNumbers.Num(),
			T66TowerCasinoSpawnChance);

		UE_LOG(LogT66GameMode, Verbose, TEXT("[StartGallery] Skipped for tower stage start; floor 1 is reserved for the Weapon Altar."));
		bWorldInteractablesSpawnedForStage = true;
		return;
	}

	for (int32 i = 0; i < RemainingFountains; ++i)
	{
		SpawnOne(AT66FountainInteractable::StaticClass());
	}
	for (int32 i = 0; i < RemainingChests; ++i)
	{
		if (AT66ChestInteractable* Chest = Cast<AT66ChestInteractable>(SpawnOne(AT66ChestInteractable::StaticClass())))
		{
			ConfigureChest(Chest);
		}
	}

	for (int32 i = 0; i < RemainingCrates; ++i)
	{
		if (AT66CrateInteractable* Crate = Cast<AT66CrateInteractable>(SpawnOne(AT66CrateInteractable::StaticClass())))
		{
			ConfigureCrate(Crate);
		}
	}

	for (int32 i = 0; i < RemainingLootWheels; ++i)
	{
		if (AT66LootWheelInteractable* LootWheel = Cast<AT66LootWheelInteractable>(SpawnOne(AT66LootWheelInteractable::StaticClass())))
		{
			ConfigureLootWheel(LootWheel);
		}
	}

	for (int32 i = 0; i < RemainingTotems; ++i)
	{
		if (AT66DifficultyTotem* Totem = Cast<AT66DifficultyTotem>(SpawnOne(AT66DifficultyTotem::StaticClass())))
		{
			ConfigureTotem(Totem);
		}
	}

	bWorldInteractablesSpawnedForStage = true;
}

void AT66GameMode::SpawnStartGalleryShowcase()
{
	if (bStartGalleryShowcaseSpawned || !IsUsingTowerMainMapLayout())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (CachedTowerMainMapLayout.StartGalleryWings.Num() == 0)
	{
		bStartGalleryShowcaseSpawned = true;
		UE_LOG(LogT66GameMode, Display, TEXT("[StartGallery] No start-gallery wings in this layout; showcase spawn skipped."));
		return;
	}

	const T66TowerMapTerrain::FStartGalleryWing* HeroesWing = T66FindStartGalleryWing(
		CachedTowerMainMapLayout,
		T66TowerMapTerrain::ET66TowerStartGalleryCategory::Heroes);
	const T66TowerMapTerrain::FStartGalleryWing* EnemiesWing = T66FindStartGalleryWing(
		CachedTowerMainMapLayout,
		T66TowerMapTerrain::ET66TowerStartGalleryCategory::Enemies);
	const T66TowerMapTerrain::FStartGalleryWing* BossesWing = T66FindStartGalleryWing(
		CachedTowerMainMapLayout,
		T66TowerMapTerrain::ET66TowerStartGalleryCategory::Bosses);
	const T66TowerMapTerrain::FStartGalleryWing* WorldWing = T66FindStartGalleryWing(
		CachedTowerMainMapLayout,
		T66TowerMapTerrain::ET66TowerStartGalleryCategory::World);

	if (!HeroesWing || !EnemiesWing || !BossesWing || !WorldWing)
	{
		UE_LOG(LogT66GameMode, Warning, TEXT("[StartGallery] Missing one or more start-gallery wings; showcase spawn skipped."));
		return;
	}

	int32 SpawnedHeroes = 0;
	int32 SpawnedEnemies = 0;
	int32 SpawnedBosses = 0;
	int32 SpawnedNPCs = 0;
	int32 SpawnedInteractables = 0;
	int32 SpawnedTraps = 0;

	TArray<FName> HeroVisualIDs;
	for (const FName HeroID : T66LoadDataTableRowNamesForStartGallery(TEXT("/Game/Data/DT_Heroes.DT_Heroes")))
	{
		if (HeroID.IsNone())
		{
			continue;
		}

		const FString HeroIDString = HeroID.ToString();
		HeroVisualIDs.Add(FName(*FString::Printf(TEXT("%s_Chad"), *HeroIDString)));
		HeroVisualIDs.Add(FName(*FString::Printf(TEXT("%s_Stacy"), *HeroIDString)));
	}

	for (int32 Index = 0; Index < HeroVisualIDs.Num(); ++Index)
	{
		if (T66SpawnStartGalleryDisplayActor(World, *HeroesWing, T66StartGalleryHeroesTag, HeroVisualIDs[Index], Index, HeroVisualIDs.Num(), 1160.0f, 1320.0f, 1.0f))
		{
			++SpawnedHeroes;
		}
	}

	const TArray<FName> EnemyVisualIDs = T66LoadDataTableRowNamesForStartGallery(TEXT("/Game/Data/DT_Enemies.DT_Enemies"));
	for (int32 Index = 0; Index < EnemyVisualIDs.Num(); ++Index)
	{
		if (T66SpawnStartGalleryDisplayActor(World, *EnemiesWing, T66StartGalleryEnemiesTag, EnemyVisualIDs[Index], Index, EnemyVisualIDs.Num(), 1160.0f, 1320.0f, 0.95f))
		{
			++SpawnedEnemies;
		}
	}

	const TArray<FName> BossVisualIDs = T66LoadDataTableRowNamesForStartGallery(TEXT("/Game/Data/DT_Bosses.DT_Bosses"));
	for (int32 Index = 0; Index < BossVisualIDs.Num(); ++Index)
	{
		if (T66SpawnStartGalleryDisplayActor(World, *BossesWing, T66StartGalleryBossesTag, BossVisualIDs[Index], Index, BossVisualIDs.Num(), 1180.0f, 1340.0f, 1.0f))
		{
			++SpawnedBosses;
		}
	}

	TArray<FName> NPCVisualIDs = T66LoadDataTableRowNamesForStartGallery(TEXT("/Game/Data/DT_NPCs.DT_NPCs"));
	const TArray<FName> ItemRowIDs = T66LoadDataTableRowNamesForStartGallery(TEXT("/Game/Data/DT_Items.DT_Items"));
	const FName ShowcaseLootItemID = ItemRowIDs.Num() > 0 ? ItemRowIDs[0] : FName(TEXT("Item_LootCrate"));
	const int32 CoreInteractableCount = 6;
	const int32 VehicleInteractableCount = 1;
	const int32 LootBagCount = 4;
	const int32 TrapCount = 4;
	const int32 WorldTotalSlots = NPCVisualIDs.Num() + CoreInteractableCount + VehicleInteractableCount + LootBagCount + TrapCount;
	int32 WorldSlotIndex = 0;

	for (const FName NPCVisualID : NPCVisualIDs)
	{
		if (AT66GalleryDisplayActor* NPCDisplay = T66SpawnStartGalleryDisplayActor(World, *WorldWing, T66StartGalleryWorldTag, NPCVisualID, WorldSlotIndex++, WorldTotalSlots, 1210.0f, 1330.0f, 1.0f))
		{
			NPCDisplay->ConfigureInteractionPromptTarget(FText::FromString(NPCVisualID.ToString()));
			++SpawnedNPCs;
		}
	}

	auto SpawnWorldGalleryActor = [&](UClass* ActorClass) -> AActor*
	{
		if (!ActorClass)
		{
			++WorldSlotIndex;
			return nullptr;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const FVector DesiredLocation = T66ComputeStartGallerySlotLocation(*WorldWing, WorldSlotIndex++, WorldTotalSlots, 1210.0f, 1330.0f);
		const FRotator Rotation = T66BuildStartGalleryFacingRotation(*WorldWing);
		AActor* Actor = World->SpawnActor<AActor>(ActorClass, DesiredLocation, Rotation, SpawnParams);
		if (!Actor)
		{
			return nullptr;
		}

		T66TagStartGalleryActor(Actor, *WorldWing, T66StartGalleryWorldTag);
		T66TrySnapActorToTowerFloor(World, Actor, CachedTowerMainMapLayout, WorldWing->FloorNumber, DesiredLocation);

		if (AT66WorldInteractableBase* Interactable = Cast<AT66WorldInteractableBase>(Actor))
		{
			Interactable->SetShowcaseReusable(true);
		}

		return Actor;
	};

	if (SpawnWorldGalleryActor(AT66CrateInteractable::StaticClass()))
	{
		++SpawnedInteractables;
	}
	if (SpawnWorldGalleryActor(AT66LootWheelInteractable::StaticClass()))
	{
		++SpawnedInteractables;
	}
	if (AT66ChestInteractable* Chest = Cast<AT66ChestInteractable>(SpawnWorldGalleryActor(AT66ChestInteractable::StaticClass())))
	{
		Chest->bIsMimic = false;
		++SpawnedInteractables;
	}
	if (SpawnWorldGalleryActor(AT66FountainInteractable::StaticClass()))
	{
		++SpawnedInteractables;
	}
	if (AT66DifficultyTotem* Totem = Cast<AT66DifficultyTotem>(SpawnWorldGalleryActor(AT66DifficultyTotem::StaticClass())))
	{
		Totem->SetShowcaseReusable(true);
		++SpawnedInteractables;
	}
	if (AT66IdolAltar* GalleryIdolAltar = Cast<AT66IdolAltar>(SpawnWorldGalleryActor(AT66IdolAltar::StaticClass())))
	{
		GalleryIdolAltar->RemainingSelections = 999;
		SpawnPixalTestDisplayModelsNearIdolAltar(GalleryIdolAltar);
		++SpawnedInteractables;
	}

	if (FT66ShelvedFeatureGate::IsVehicleInteractablesEnabled())
	{
		if (AT66VehicleInteractable* Vehicle = Cast<AT66VehicleInteractable>(SpawnWorldGalleryActor(AT66VehicleInteractable::StaticClass())))
		{
			Vehicle->SetVehicleRowID(FName(TEXT("Vehicle")));
			Vehicle->SetShowcaseReusable(true);
			++SpawnedInteractables;
		}
	}

	const ET66Rarity LootBagRarities[] = {
		ET66Rarity::Black,
		ET66Rarity::Red,
		ET66Rarity::Yellow,
		ET66Rarity::White,
	};
	for (const ET66Rarity LootBagRarity : LootBagRarities)
	{
		if (AT66LootBagPickup* LootBag = Cast<AT66LootBagPickup>(SpawnWorldGalleryActor(AT66LootBagPickup::StaticClass())))
		{
			LootBag->SetShowcaseReusable(true);
			LootBag->SetItemID(ShowcaseLootItemID);
			LootBag->SetLootRarity(LootBagRarity);
			++SpawnedInteractables;
		}
	}

	auto ConfigureGalleryTrap = [&](AActor* Actor)
	{
		if (!Actor)
		{
			return;
		}

		if (AT66TrapBase* Trap = Cast<AT66TrapBase>(Actor))
		{
			Trap->SetTowerFloorNumber(WorldWing->FloorNumber);
			Trap->SetActivationMode(ET66TrapActivationMode::Triggered);
			Trap->SetTriggerTargetMode(ET66TrapTriggerTarget::EnemiesOnly);
			Trap->SetDamagesHeroes(false);
			Trap->SetDamagesEnemies(false);
			Trap->SetTrapEnabled(false);
		}
		else if (AT66TrapPressurePlate* PressurePlate = Cast<AT66TrapPressurePlate>(Actor))
		{
			PressurePlate->SetTowerFloorNumber(WorldWing->FloorNumber);
			PressurePlate->SetTriggerTargetMode(ET66TrapTriggerTarget::EnemiesOnly);
		}
	};

	if (AActor* Trap = SpawnWorldGalleryActor(AT66FloorFlameTrap::StaticClass()))
	{
		ConfigureGalleryTrap(Trap);
		++SpawnedTraps;
	}
	if (AActor* Trap = SpawnWorldGalleryActor(AT66FloorSpikePatchTrap::StaticClass()))
	{
		ConfigureGalleryTrap(Trap);
		++SpawnedTraps;
	}
	if (AActor* Trap = SpawnWorldGalleryActor(AT66WallArrowTrap::StaticClass()))
	{
		ConfigureGalleryTrap(Trap);
		++SpawnedTraps;
	}
	if (AActor* Trap = SpawnWorldGalleryActor(AT66TrapPressurePlate::StaticClass()))
	{
		ConfigureGalleryTrap(Trap);
		++SpawnedTraps;
	}

	bStartGalleryShowcaseSpawned = true;
	UE_LOG(
		LogT66GameMode,
		Log,
		TEXT("[StartGallery] Spawned inert showcase: heroes=%d, enemies=%d, bosses=%d, npcs=%d, interactables=%d, traps=%d."),
		SpawnedHeroes,
		SpawnedEnemies,
		SpawnedBosses,
		SpawnedNPCs,
		SpawnedInteractables,
		SpawnedTraps);
}

void AT66GameMode::SpawnIdolAltarForPlayer(AController* Player)
{
	if (IsLabRun()) return;

	UWorld* World = GetWorld();
	if (!World || !Player) return;

	if (IsValid(IdolAltar))
	{
		return;
	}
	IdolAltar = nullptr;

	UGameInstance* GI = World->GetGameInstance();
	UT66IdolManagerSubsystem* IdolManager = GI ? GI->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!IdolManager)
	{
		return;
	}

	if (IsUsingTowerMainMapLayout())
	{
		UE_LOG(LogT66GameMode, Verbose, TEXT("Skipping tower stage-entry idol altar spawn; tower idol altars are linked to descent gates."));
		return;
	}

	const int32 SelectionBudget = 1;

	const int32 CurrentStage = RunState ? RunState->GetCurrentStage() : 1;
	UE_LOG(
		LogT66GameMode,
		Log,
		TEXT("Spawning stage-entry idol altar for stage %d with %d selection."),
		CurrentStage,
		SelectionBudget);

	FVector SpawnLoc = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	if (T66UsesMainMapTerrainStage(World))
	{
		if (IsUsingTowerMainMapLayout())
		{
			SpawnLoc = CachedTowerMainMapLayout.StartAnchorSurfaceLocation;
		}
		else if (!TryGetMainMapStartPlacementLocation(0.f, 0.f, SpawnLoc))
		{
			return;
		}

		FVector Center = FVector::ZeroVector;
		FVector InwardDirection = FVector::ForwardVector;
		FVector SideDirection = FVector::RightVector;
		float CellSize = 0.f;
		if (TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize) && !InwardDirection.IsNearlyZero())
		{
			SpawnRotation = (-InwardDirection).Rotation();
			if (IsUsingTowerMainMapLayout() && !SideDirection.IsNearlyZero())
			{
				SpawnLoc -= SideDirection.GetSafeNormal2D() * 430.f;
			}
		}
	}
	else
	{
		SpawnLoc = T66GameplayLayout::GetStartAreaCenter();

		FHitResult Hit;
		const FVector TraceStart = SpawnLoc + FVector(0.f, 0.f, 3000.f);
		const FVector TraceEnd = SpawnLoc - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic)
			|| World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility))
		{
			SpawnLoc = Hit.ImpactPoint;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	IdolAltar = World->SpawnActor<AT66IdolAltar>(AT66IdolAltar::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams);
	if (IdolAltar)
	{
		if (IsUsingTowerMainMapLayout())
		{
			T66TrySnapActorToTowerFloor(World, IdolAltar, CachedTowerMainMapLayout, CachedTowerMainMapLayout.StartFloorNumber, SpawnLoc);
			T66AssignTowerFloorTag(IdolAltar, CachedTowerMainMapLayout.StartFloorNumber);
		}
		else
		{
			TrySnapActorToTerrainAtLocation(IdolAltar, SpawnLoc);
		}

		if (APawn* PlayerPawn = Player->GetPawn())
		{
			T66FaceActorTowardLocation2D(IdolAltar, PlayerPawn->GetActorLocation());
			if (RunState && RunState->GetCurrentStage() <= 1)
			{
				FRotator FacingRotation = PlayerPawn->GetActorRotation();
				if (T66TryBuildFacingRotation2D(PlayerPawn->GetActorLocation(), IdolAltar->GetActorLocation(), FacingRotation))
				{
					T66SyncPawnAndControllerRotation(PlayerPawn, Player, FacingRotation);
				}
			}
		}

		IdolAltar->RemainingSelections = FMath::Max(1, SelectionBudget);
		TowerIdolSelectionsAtStageStart = IdolAltar->RemainingSelections;
		if (IsUsingTowerMainMapLayout())
		{
			SyncTowerMiasmaSourceAnchor(CachedTowerMainMapLayout.StartFloorNumber, IdolAltar->GetActorLocation());
		}
	}
}

void AT66GameMode::SpawnWeaponAltarForPlayer(AController* Player)
{
	if (IsLabRun()) return;

	UWorld* World = GetWorld();
	if (!World || !Player) return;

	if (IsValid(WeaponAltar))
	{
		return;
	}
	WeaponAltar = nullptr;

	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66WeaponManagerSubsystem* WeaponManager = GI ? GI->GetSubsystem<UT66WeaponManagerSubsystem>() : nullptr;
	UT66DifficultyTuningSubsystem* DifficultyTuning = GI ? GI->GetSubsystem<UT66DifficultyTuningSubsystem>() : nullptr;
	if (!T66GI || !RunState || !WeaponManager || !DifficultyTuning)
	{
		return;
	}

	const int32 CurrentStage = RunState->GetCurrentStage();
	const int32 DifficultyStartStage = DifficultyTuning->GetDifficultyStartStage(T66GI->SelectedDifficulty);
	const bool bTowerStageStart = IsUsingTowerMainMapLayout();
	if (!bTowerStageStart && (CurrentStage != DifficultyStartStage || !WeaponManager->GetEquippedWeaponID().IsNone()))
	{
		return;
	}

	FVector SpawnLoc = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	FVector SideDirection = FVector::RightVector;
	if (T66UsesMainMapTerrainStage(World))
	{
		if (IsUsingTowerMainMapLayout())
		{
			SpawnLoc = CachedTowerMainMapLayout.StartAnchorSurfaceLocation;
		}
		else if (!TryGetMainMapStartPlacementLocation(0.f, 0.f, SpawnLoc))
		{
			return;
		}

		FVector Center = FVector::ZeroVector;
		FVector InwardDirection = FVector::ForwardVector;
		float CellSize = 0.f;
		if (TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize) && !InwardDirection.IsNearlyZero())
		{
			SpawnRotation = (-InwardDirection).Rotation();
		}
	}
	else
	{
		SpawnLoc = T66GameplayLayout::GetStartAreaCenter();
	}

	if (SideDirection.IsNearlyZero())
	{
		SideDirection = FVector::RightVector;
	}
	SpawnLoc += SideDirection.GetSafeNormal2D() * 430.f;

	if (!T66UsesMainMapTerrainStage(World))
	{
		FHitResult Hit;
		const FVector TraceStart = SpawnLoc + FVector(0.f, 0.f, 3000.f);
		const FVector TraceEnd = SpawnLoc - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic)
			|| World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility))
		{
			SpawnLoc = Hit.ImpactPoint;
		}
	}

	const int32 LocalStageNumber = DifficultyTuning->GetDifficultyLocalStage(T66GI->SelectedDifficulty, CurrentStage);
	const ET66WeaponRarity OfferRarity = DifficultyTuning->GetLocalStageWeaponRarity(LocalStageNumber);
	if (bTowerStageStart)
	{
		WeaponManager->ResetForStageWeaponSelection(T66GI->SelectedHeroID);
	}
	WeaponManager->BuildWeaponOffers(T66GI->SelectedHeroID, OfferRarity);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	WeaponAltar = World->SpawnActor<AT66WeaponAltar>(AT66WeaponAltar::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams);
	if (!WeaponAltar)
	{
		return;
	}

	if (IsUsingTowerMainMapLayout())
	{
		T66TrySnapActorToTowerFloor(World, WeaponAltar, CachedTowerMainMapLayout, CachedTowerMainMapLayout.StartFloorNumber, SpawnLoc);
		T66AssignTowerFloorTag(WeaponAltar, CachedTowerMainMapLayout.StartFloorNumber);
	}
	else
	{
		TrySnapActorToTerrainAtLocation(WeaponAltar, SpawnLoc);
	}

	WeaponAltar->RemainingSelections = 1;
	WeaponAltar->WeaponOfferRarity = OfferRarity;
	if (IsUsingTowerMainMapLayout())
	{
		WeaponAltar->LinkToTowerGateFloor(CachedTowerMainMapLayout.StartFloorNumber);
	}
	if (APawn* PlayerPawn = Player->GetPawn())
	{
		T66FaceActorTowardLocation2D(WeaponAltar, PlayerPawn->GetActorLocation());
	}
}

void AT66GameMode::SpawnPixalTestDisplayModelsNearIdolAltar(AT66IdolAltar* AnchorAltar, const bool bTrackAsLabSpawned)
{
#if UE_BUILD_SHIPPING
	(void)AnchorAltar;
	(void)bTrackAsLabSpawned;
	return;
#else
	UWorld* World = GetWorld();
	if (!World || !AnchorAltar || !bSpawnPixalTestModelsAtIdolAltar)
	{
		return;
	}

	UMaterialInterface* SharedMaterial = PixalTestSharedMaterial.LoadSynchronous();
	if (!SharedMaterial)
	{
		UE_LOG(LogT66GameMode, Warning, TEXT("[PixalTestDisplay] Missing shared material; test models not spawned."));
		return;
	}

	FVector Forward = AnchorAltar->GetActorForwardVector().GetSafeNormal2D();
	if (Forward.IsNearlyZero())
	{
		Forward = FVector::ForwardVector;
	}

	FVector Side = AnchorAltar->GetActorRightVector().GetSafeNormal2D();
	if (Side.IsNearlyZero())
	{
		Side = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
	}
	if (Side.IsNearlyZero())
	{
		Side = FVector::RightVector;
	}

	const FName DisplayTag(TEXT("T66_PixalTest_IdolAltar_Display"));
	const FVector AnchorLocation = AnchorAltar->GetActorLocation();
	const int32 TowerFloorNumber = T66ReadTowerFloorTag(AnchorAltar);
	constexpr float MeshFacingYawCorrection = 90.0f;
	constexpr float DisplayYawOffsetRightDegrees = 45.0f;

	const APawn* HeroPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	const FVector FacingTarget = HeroPawn
		? HeroPawn->GetActorLocation()
		: AnchorLocation + Forward * 1000.0f;

	auto HasExistingDisplay = [&](const FName AssetTag, const FVector& DesiredLocation) -> bool
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			const AStaticMeshActor* Existing = *It;
			if (!Existing || !Existing->Tags.Contains(DisplayTag) || !Existing->Tags.Contains(AssetTag))
			{
				continue;
			}

			if (FVector::DistSquared2D(Existing->GetActorLocation(), DesiredLocation) <= FMath::Square(150.f))
			{
				return true;
			}
		}
		return false;
	};

	auto ApplyPixalDisplayMaterial = [&](UStaticMeshComponent* MeshComponent, UTexture2D* Texture, const FName AssetTag)
	{
		if (!MeshComponent || !Texture)
		{
			return;
		}

		const int32 NumMaterials = FMath::Max(1, MeshComponent->GetNumMaterials());
		for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; ++MaterialIndex)
		{
			UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(SharedMaterial, MeshComponent);
			if (!DynamicMaterial)
			{
				continue;
			}

			DynamicMaterial->SetTextureParameterValue(TEXT("EmissiveTexture"), Texture);
			DynamicMaterial->SetTextureParameterValue(TEXT("BaseColorTexture"), Texture);
			DynamicMaterial->SetTextureParameterValue(TEXT("DiffuseColorMap"), Texture);
			DynamicMaterial->SetScalarParameterValue(TEXT("DiffuseColorMapWeight"), 1.0f);
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColorFactor"), FLinearColor::Black);
			MeshComponent->SetMaterial(MaterialIndex, DynamicMaterial);
		}

		UE_LOG(LogT66GameMode, Log, TEXT("[PixalTestDisplay] Applied direct display material for %s using %s."), *AssetTag.ToString(), *Texture->GetPathName());
	};

	auto GroundMeshBottomToActorOrigin = [](UStaticMeshComponent* MeshComponent, AActor* DisplayActor)
	{
		if (!MeshComponent || !DisplayActor)
		{
			return;
		}

		MeshComponent->UpdateBounds();
		const FBoxSphereBounds WorldBounds = MeshComponent->CalcBounds(MeshComponent->GetComponentTransform());
		const float BottomZ = WorldBounds.Origin.Z - WorldBounds.BoxExtent.Z;
		const float DeltaZ = DisplayActor->GetActorLocation().Z - BottomZ;
		FVector RelativeLocation = MeshComponent->GetRelativeLocation();
		RelativeLocation.Z += DeltaZ;
		MeshComponent->SetRelativeLocation(RelativeLocation);
		MeshComponent->UpdateBounds();
	};

	auto SpawnDisplay = [&](
		const FName AssetTag,
		TSoftObjectPtr<UStaticMesh>& MeshPtr,
		TSoftObjectPtr<UTexture2D>& TexturePtr,
		const float SideOffsetCm,
		const float DesiredDisplayHeightCm,
		const float ForwardOffsetCm)
	{
		UStaticMesh* Mesh = MeshPtr.LoadSynchronous();
		UTexture2D* Texture = TexturePtr.LoadSynchronous();
		if (!Mesh || !Texture)
		{
			UE_LOG(
				LogT66GameMode,
				Warning,
				TEXT("[PixalTestDisplay] Missing mesh or texture for %s. Mesh=%s Texture=%s"),
				*AssetTag.ToString(),
				Mesh ? *Mesh->GetPathName() : TEXT("(null)"),
				Texture ? *Texture->GetPathName() : TEXT("(null)"));
			return;
		}

		const FVector DesiredLocation = AnchorLocation + Side * SideOffsetCm + Forward * ForwardOffsetCm;
		FVector FacingDirection = (FacingTarget - DesiredLocation).GetSafeNormal2D();
		if (FacingDirection.IsNearlyZero())
		{
			FacingDirection = -Forward;
		}
		if (FacingDirection.IsNearlyZero())
		{
			FacingDirection = FVector::ForwardVector;
		}

		const FRotator DisplayRotation(0.0f, FacingDirection.Rotation().Yaw - MeshFacingYawCorrection + DisplayYawOffsetRightDegrees, 0.0f);
		if (HasExistingDisplay(AssetTag, DesiredLocation))
		{
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = AnchorAltar;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AStaticMeshActor* DisplayActor = World->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(),
			DesiredLocation,
			DisplayRotation,
			SpawnParams);
		if (!DisplayActor)
		{
			return;
		}

		DisplayActor->Tags.AddUnique(DisplayTag);
		DisplayActor->Tags.AddUnique(AssetTag);
		if (TowerFloorNumber != INDEX_NONE)
		{
			T66AssignTowerFloorTag(DisplayActor, TowerFloorNumber);
		}

		T66_SetStaticMeshActorMobility(DisplayActor, EComponentMobility::Movable);
		if (UStaticMeshComponent* MeshComponent = DisplayActor->GetStaticMeshComponent())
		{
			MeshComponent->SetStaticMesh(Mesh);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			MeshComponent->SetGenerateOverlapEvents(false);
			MeshComponent->SetCanEverAffectNavigation(false);
			MeshComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
			const float MeshHeight = FMath::Max(1.0f, Mesh->GetBounds().BoxExtent.Z * 2.0f);
			MeshComponent->SetRelativeScale3D(FVector(DesiredDisplayHeightCm / MeshHeight));
			ApplyPixalDisplayMaterial(MeshComponent, Texture, AssetTag);
		}

		if (IsUsingTowerMainMapLayout() && TowerFloorNumber != INDEX_NONE)
		{
			T66TrySnapActorToTowerFloor(World, DisplayActor, CachedTowerMainMapLayout, TowerFloorNumber, DesiredLocation);
		}
		else
		{
			TrySnapActorToTerrainAtLocation(DisplayActor, DesiredLocation);
		}

		if (UStaticMeshComponent* MeshComponent = DisplayActor->GetStaticMeshComponent())
		{
			GroundMeshBottomToActorOrigin(MeshComponent, DisplayActor);
		}

		PixalTestDisplayActors.Add(DisplayActor);
		if (bTrackAsLabSpawned)
		{
			LabSpawnedActors.Add(DisplayActor);
		}

		UE_LOG(LogT66GameMode, Log, TEXT("[PixalTestDisplay] Spawned %s beside Idol Altar at %s."), *AssetTag.ToString(), *DisplayActor->GetActorLocation().ToCompactString());
	};

	auto SpawnEasyDungeonHifiDisplay = [&](
		const int32 AssetIndex,
		const FName AssetTag,
		const float SideOffsetCm,
		const float DesiredDisplayHeightCm)
	{
		if (!PixalEasyDungeonHifiMeshes.IsValidIndex(AssetIndex) || !PixalEasyDungeonHifiTextures.IsValidIndex(AssetIndex))
		{
			UE_LOG(LogT66GameMode, Warning, TEXT("[PixalTestDisplay] Missing Easy/Dungeon Pixal3D display reference for %s at index %d."), *AssetTag.ToString(), AssetIndex);
			return;
		}

		SpawnDisplay(
			AssetTag,
			PixalEasyDungeonHifiMeshes[AssetIndex],
			PixalEasyDungeonHifiTextures[AssetIndex],
			SideOffsetCm,
			DesiredDisplayHeightCm,
			520.f);
	};

	auto SpawnStandaloneTestDisplay = [&](
		const int32 AssetIndex,
		const FName AssetTag,
		const float SideOffsetCm)
	{
		if (!PixalStandaloneTestMeshes.IsValidIndex(AssetIndex) || !PixalStandaloneTestTextures.IsValidIndex(AssetIndex))
		{
			UE_LOG(LogT66GameMode, Warning, TEXT("[PixalTestDisplay] Missing standalone Pixal3D display reference for %s at index %d."), *AssetTag.ToString(), AssetIndex);
			return;
		}

		SpawnDisplay(
			AssetTag,
			PixalStandaloneTestMeshes[AssetIndex],
			PixalStandaloneTestTextures[AssetIndex],
			SideOffsetCm,
			200.f,
			150.f);
	};

	SpawnDisplay(FName(TEXT("PIXALTEST")), PixalTestMesh, PixalTestTexture, -460.f, 200.f, 150.f);
	SpawnDisplay(FName(TEXT("PIXALSLIME")), PixalSlimeMesh, PixalSlimeTexture, -860.f, 110.f, 150.f);
	SpawnDisplay(FName(TEXT("PIXALSLIME_HIFIRUSH")), PixalSlimeHifiMesh, PixalSlimeHifiTexture, -1220.f, 110.f, 150.f);
	SpawnDisplay(FName(TEXT("PIXALTEST2")), PixalTest2Mesh, PixalTest2Texture, 460.f, 200.f, 150.f);
	SpawnStandaloneTestDisplay(0, FName(TEXT("PIXALTEST3")), 860.f);
	SpawnStandaloneTestDisplay(1, FName(TEXT("PIXALTEST4")), 1220.f);
	SpawnStandaloneTestDisplay(2, FName(TEXT("PIXALTEST5")), 1580.f);
	SpawnEasyDungeonHifiDisplay(0, FName(TEXT("PIXALBONEWALKER_HIFIRUSH")), -1220.f, 360.f);
	SpawnEasyDungeonHifiDisplay(1, FName(TEXT("PIXALRATPACK_HIFIRUSH")), -860.f, 180.f);
	SpawnEasyDungeonHifiDisplay(2, FName(TEXT("PIXALCAVEBAT_HIFIRUSH")), -500.f, 280.f);
	SpawnEasyDungeonHifiDisplay(3, FName(TEXT("PIXALHEXSLINGER_HIFIRUSH")), -140.f, 360.f);
	SpawnEasyDungeonHifiDisplay(4, FName(TEXT("PIXALTOMBSPIDER_HIFIRUSH")), 220.f, 300.f);
	SpawnEasyDungeonHifiDisplay(5, FName(TEXT("PIXALSTONESENTINEL_HIFIRUSH")), 580.f, 360.f);
	SpawnEasyDungeonHifiDisplay(6, FName(TEXT("PIXALMIMICLURE_HIFIRUSH")), 940.f, 200.f);
	SpawnEasyDungeonHifiDisplay(7, FName(TEXT("PIXALBONECONJURER_HIFIRUSH")), 1300.f, 400.f);
	SpawnEasyDungeonHifiDisplay(8, FName(TEXT("PIXALCRYPTWRAITH_HIFIRUSH")), 1660.f, 340.f);
#endif
}

AT66IdolAltar* AT66GameMode::SpawnIdolAltarAtLocation(const FVector& Location, const bool bAllowMultiple)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}
	if (!bAllowMultiple && IsValid(IdolAltar))
	{
		return IdolAltar.Get();
	}
	if (!IsValid(IdolAltar))
	{
		IdolAltar = nullptr;
	}

	// Caller provides the final desired reward location so boss rewards can be spaced deterministically.
	FVector SpawnLoc = Location;

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
	AT66IdolAltar* SpawnedIdolAltar = World->SpawnActor<AT66IdolAltar>(AT66IdolAltar::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (SpawnedIdolAltar)
	{
		IdolAltar = SpawnedIdolAltar;
		IdolAltar->RemainingSelections = 1;
	}
	return SpawnedIdolAltar;
}

