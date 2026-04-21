// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiasmaManager.h"
#include "Gameplay/T66LavaShared.h"

#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66GameplayLayout.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Gameplay/T66CircusInteractable.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66LavaPatch.h"
#include "Gameplay/T66MainMapTerrain.h"
#include "Gameplay/T66TowerMapTerrain.h"
#include "Algo/Sort.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66MiasmaManager, Log, All);

namespace
{
	static constexpr bool T66EnableLegacyLavaPatches = false;

	static bool T66ShouldUseMainBoardCoverage(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		return !MapName.Contains(TEXT("Tutorial"))
			&& !MapName.Contains(TEXT("Lab"));
	}

	static UTexture* T66GetFallbackTexture()
	{
		static TObjectPtr<UTexture> Cached = nullptr;
		if (!Cached)
		{
			Cached = LoadObject<UTexture>(nullptr, T66LavaShared::DefaultTexturePath);
		}
		return Cached.Get();
	}
}

AT66MiasmaManager::AT66MiasmaManager()
{
	PrimaryActorTick.bCanEverTick = true;

	TileInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TileInstances"));
	TileInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TileInstances->SetGenerateOverlapEvents(false);
	TileInstances->SetCanEverAffectNavigation(false);
	RootComponent = TileInstances;

	if (UStaticMesh* Plane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane")))
	{
		TileInstances->SetStaticMesh(Plane);
	}
}

void AT66MiasmaManager::BeginPlay()
{
	Super::BeginPlay();

	GenerateAnimationFrames();
	EnsureVisualMaterial();

	if (UWorld* World = GetWorld())
	{
		AnimationStartTimeSeconds = World->GetTimeSeconds();
	}

	if (!bSpawningPaused)
	{
		BuildGrid();
		UpdateFromRunState();
	}
}

void AT66MiasmaManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateFromRunState();
	TickDamageOverActiveTiles(DeltaTime);

	if (!LavaMID)
	{
		return;
	}

	const bool bTowerBlood = ShouldUseTowerBloodLook();
	ApplyMaterialLookIfNeeded(
		bTowerBlood ? FLinearColor(0.95f, 0.22f, 0.24f, 1.0f) : FLinearColor::White,
		bTowerBlood ? 1.45f : Brightness);

	UWorld* World = GetWorld();
	if (!World || AnimationFPS <= KINDA_SMALL_NUMBER || GeneratedFrames.Num() <= 1)
	{
		return;
	}

	const float Elapsed = FMath::Max(World->GetTimeSeconds() - AnimationStartTimeSeconds, 0.f);
	const int32 FrameIndex = FMath::FloorToInt(Elapsed * AnimationFPS) % GeneratedFrames.Num();
	if (FrameIndex != CurrentFrameIndex)
	{
		ApplyAnimationFrame(FrameIndex);
	}
}

void AT66MiasmaManager::BuildMainMapCellGrid()
{
	TileCenters.Reset();
	TileFloorNumbers.Reset();
	TowerDefaultSourceAnchors.Reset();

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!World || !T66GI)
	{
		return;
	}

	const int32 StageNum = RunState ? RunState->GetCurrentStage() : 1;
	const FT66MapPreset Preset = T66MainMapTerrain::BuildPresetForDifficulty(T66GI->SelectedDifficulty, T66GI->RunSeed);
	T66MainMapTerrain::FBoard Board;
	if (!T66MainMapTerrain::Generate(Preset, Board))
	{
		return;
	}

	TileSize = Board.Settings.CellSize;

	auto AddCellCenter = [&](const T66MainMapTerrain::FCell& Cell)
	{
		if (!Cell.bOccupied)
		{
			return;
		}

		FVector CellCenter = FVector::ZeroVector;
		const float CoverHeightOffset = T66MainMapTerrain::GetSurfaceFeatureLavaCoverHeight(Cell);
		if (!T66MainMapTerrain::TryGetCellLocation(Board, FIntPoint(Cell.X, Cell.Z), TileZ + CoverHeightOffset, CellCenter))
		{
			return;
		}

		TileCenters.Add(CellCenter);
	};

	for (const T66MainMapTerrain::FCell& Cell : Board.Cells)
	{
		AddCellCenter(Cell);
	}
	for (const T66MainMapTerrain::FCell& Cell : Board.ExtraCells)
	{
		AddCellCenter(Cell);
	}

	FRandomStream Stream(T66GI->RunSeed + (StageNum * 97));
	for (int32 Index = TileCenters.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = Stream.RandRange(0, Index);
		if (Index != SwapIndex)
		{
			TileCenters.Swap(Index, SwapIndex);
		}
	}

	UE_LOG(
		LogT66MiasmaManager,
		Log,
		TEXT("[LAVA] Built main-map lava grid with %d full-cell patches (cellSize=%.0f, stage=%d)."),
		TileCenters.Num(),
		TileSize,
		StageNum);
}

void AT66MiasmaManager::BuildTowerFloorGrid()
{
	TileCenters.Reset();
	TileFloorNumbers.Reset();
	TowerDefaultSourceAnchors.Reset();

	UWorld* World = GetWorld();
	AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	T66TowerMapTerrain::FLayout Layout;
	if (!World || !GameMode || !GameMode->GetTowerMainMapLayout(Layout))
	{
		return;
	}

	TileSize = FMath::Max(Layout.PlacementCellSize * 0.25f, 300.0f);
	for (const T66TowerMapTerrain::FFloor& Floor : Layout.Floors)
	{
		if (Floor.FloorRole == T66TowerMapTerrain::ET66TowerFloorRole::Boss)
		{
			continue;
		}

		const FVector DefaultAnchor =
			(Floor.FloorRole == T66TowerMapTerrain::ET66TowerFloorRole::Start)
				? FVector(Floor.Center.X, Floor.Center.Y, Floor.SurfaceZ + TileZ)
				: FVector(Floor.ArrivalPoint.X, Floor.ArrivalPoint.Y, Floor.SurfaceZ + TileZ);
		TowerDefaultSourceAnchors.Add(Floor.FloorNumber, DefaultAnchor);

		for (float X = -Floor.WalkableHalfExtent + (TileSize * 0.5f); X < Floor.WalkableHalfExtent; X += TileSize)
		{
			for (float Y = -Floor.WalkableHalfExtent + (TileSize * 0.5f); Y < Floor.WalkableHalfExtent; Y += TileSize)
			{
				const FVector Candidate = Floor.Center + FVector(X, Y, TileZ);
				if (Floor.bHasDropHole
					&& FMath::Abs(Candidate.X - Floor.HoleCenter.X) <= (Floor.HoleHalfExtent.X + TileSize * 0.45f)
					&& FMath::Abs(Candidate.Y - Floor.HoleCenter.Y) <= (Floor.HoleHalfExtent.Y + TileSize * 0.45f))
				{
					continue;
				}

				TileCenters.Add(FVector(Candidate.X, Candidate.Y, Floor.SurfaceZ + TileZ));
				TileFloorNumbers.Add(Floor.FloorNumber);
			}
		}
	}

	ApplyTowerCoverageOrdering();

	UE_LOG(
		LogT66MiasmaManager,
		Log,
		TEXT("[LAVA] Built tower miasma grid with %d tiles across %d non-boss floors (tileSize=%.0f)."),
		TileCenters.Num(),
		TowerDefaultSourceAnchors.Num(),
		TileSize);
}

void AT66MiasmaManager::ApplyTowerCoverageOrdering()
{
	if (!ShouldUseTowerBloodLook() || TileCenters.Num() <= 0 || TileCenters.Num() != TileFloorNumbers.Num())
	{
		return;
	}

	struct FSortedTowerTile
	{
		FVector Center = FVector::ZeroVector;
		int32 FloorNumber = INDEX_NONE;
		float DistanceSq = 0.0f;
		float Angle = 0.0f;
		int32 OriginalIndex = INDEX_NONE;
	};

	TArray<FSortedTowerTile> OrderedTiles;
	OrderedTiles.Reserve(TileCenters.Num());
	for (int32 TileIndex = 0; TileIndex < TileCenters.Num(); ++TileIndex)
	{
		const FVector& Center = TileCenters[TileIndex];
		const int32 FloorNumber = TileFloorNumbers[TileIndex];
		const FVector SourceAnchor = ResolveTowerSourceAnchor(FloorNumber, Center);
		const FVector2D Delta(Center.X - SourceAnchor.X, Center.Y - SourceAnchor.Y);

		FSortedTowerTile& OrderedTile = OrderedTiles.AddDefaulted_GetRef();
		OrderedTile.Center = Center;
		OrderedTile.FloorNumber = FloorNumber;
		OrderedTile.DistanceSq = Delta.SizeSquared();
		OrderedTile.Angle = FMath::Atan2(Delta.Y, Delta.X);
		OrderedTile.OriginalIndex = TileIndex;
	}

	Algo::StableSort(
		OrderedTiles,
		[](const FSortedTowerTile& A, const FSortedTowerTile& B)
		{
			if (A.FloorNumber != B.FloorNumber)
			{
				return A.FloorNumber < B.FloorNumber;
			}
			if (!FMath::IsNearlyEqual(A.DistanceSq, B.DistanceSq))
			{
				return A.DistanceSq < B.DistanceSq;
			}
			if (!FMath::IsNearlyEqual(A.Angle, B.Angle))
			{
				return A.Angle < B.Angle;
			}
			return A.OriginalIndex < B.OriginalIndex;
		}

	);

	for (int32 TileIndex = 0; TileIndex < OrderedTiles.Num(); ++TileIndex)
	{
		TileCenters[TileIndex] = OrderedTiles[TileIndex].Center;
		TileFloorNumbers[TileIndex] = OrderedTiles[TileIndex].FloorNumber;
	}
}

void AT66MiasmaManager::BuildGrid()
{
	if (ShouldUseTowerBloodLook())
	{
		BuildTowerFloorGrid();
		if (TileCenters.Num() > 0)
		{
			return;
		}
	}

	if (T66ShouldUseMainBoardCoverage(GetWorld()))
	{
		BuildMainMapCellGrid();
		if (TileCenters.Num() > 0)
		{
			return;
		}
	}

	TileCenters.Reset();
	TileFloorNumbers.Reset();
	TowerDefaultSourceAnchors.Reset();

	const int32 Num = FMath::Max(1, FMath::FloorToInt((CoverageHalfExtent * 2.f) / TileSize));
	const float Start = -CoverageHalfExtent + (TileSize * 0.5f);

	for (int32 ix = 0; ix < Num; ++ix)
	{
		for (int32 iy = 0; iy < Num; ++iy)
		{
			const float X = Start + ix * TileSize;
			const float Y = Start + iy * TileSize;
			TileCenters.Add(FVector(X, Y, TileZ));
		}
	}

	FRandomStream Stream(Seed);
	for (int32 i = TileCenters.Num() - 1; i > 0; --i)
	{
		const int32 j = Stream.RandRange(0, i);
		if (i != j)
		{
			TileCenters.Swap(i, j);
		}
	}
}

void AT66MiasmaManager::UpdateFromRunState()
{
	if (bSpawningPaused)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (TileCenters.Num() <= 0)
	{
		BuildGrid();
	}

	FLagScopedScope LagScope(World, TEXT("MiasmaManager::UpdateFromRunState (EnsureSpawnedCount)"));
	if (TileCenters.Num() <= 0)
	{
		return;
	}

	float ElapsedSeconds = 0.0f;
	if (!TryGetExpansionElapsedSeconds(ElapsedSeconds))
	{
		return;
	}

	const int32 Total = TileCenters.Num();
	int32 Desired = 0;
	if (FullCoverageSeconds <= KINDA_SMALL_NUMBER)
	{
		Desired = Total;
	}
	else
	{
		const float ExpansionAlpha = FMath::Clamp(ElapsedSeconds / FullCoverageSeconds, 0.0f, 1.0f);
		Desired = FMath::Clamp(FMath::CeilToInt(ExpansionAlpha * static_cast<float>(Total)), 0, Total);
	}

	EnsureSpawnedCount(Desired);
}

void AT66MiasmaManager::RebuildForCurrentStage()
{
	if (TileInstances)
	{
		TileInstances->ClearInstances();
	}

	TileCenters.Reset();
	TileFloorNumbers.Reset();
	SpawnedTileCount = 0;
	DamageTickAccumulator = 0.f;
	TowerDefaultSourceAnchors.Reset();
	bExplicitExpansionActive = false;
	ExplicitExpansionStartTimeSeconds = 0.0f;
	ClearLegacyLavaPatches();

	GenerateAnimationFrames();
	EnsureVisualMaterial();
	BuildGrid();
	UpdateFromRunState();
}

int32 AT66MiasmaManager::SpawnLegacyStageLavaPatchesForCurrentStage()
{
	ClearLegacyLavaPatches();

	if (!T66EnableLegacyLavaPatches)
	{
		return 0;
	}

	UWorld* World = GetWorld();
	AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	if (!World || !GameMode || !RunState || !T66GI || !T66ShouldUseMainBoardCoverage(World) || GameMode->IsUsingTowerMainMapLayout() || T66GI->bStageCatchUpPending)
	{
		return 0;
	}

	const ET66Difficulty Difficulty = T66GI->SelectedDifficulty;
	const int32 StageNum = RunState->GetCurrentStage();
	int32 LavaPatchCount = 0;
	switch (Difficulty)
	{
	case ET66Difficulty::Easy:       LavaPatchCount = 4;  break;
	case ET66Difficulty::Medium:     LavaPatchCount = 5;  break;
	case ET66Difficulty::Hard:       LavaPatchCount = 6;  break;
	case ET66Difficulty::VeryHard:   LavaPatchCount = 7;  break;
	case ET66Difficulty::Impossible: LavaPatchCount = 8;  break;
	default:                         LavaPatchCount = 5;  break;
	}

	if (LavaPatchCount <= 0)
	{
		return 0;
	}

	const int32 RunSeed = (T66GI->RunSeed != 0) ? T66GI->RunSeed : FMath::Rand();
	FRandomStream Rng(RunSeed + StageNum * 971 + 17);
	UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();

	static constexpr float MainHalfExtent = 50000.f;
	static constexpr float SpawnZ = 40.f;
	static constexpr float SafeBubbleMargin = 350.f;
	static constexpr float LavaExclusionRadius = 1800.f;

	struct FUsedStageEffectLoc
	{
		FVector Loc = FVector::ZeroVector;
		float ExclusionRadius = 0.f;
	};

	TArray<FUsedStageEffectLoc> UsedLocs;

	auto IsInsideNoSpawnZone = [&](const FVector& Location) -> bool
	{
		if (T66GameplayLayout::IsInsideReservedTraversalZone2D(Location, 455.f))
		{
			return true;
		}

		static constexpr float ArenaHalf = 9091.f;
		static constexpr float ArenaMargin = 682.f;
		static constexpr float TutorialArenaHalf = 9091.f;
		struct FArena2D { float X; float Y; float Half; };
		static constexpr FArena2D Arenas[] = {
			{ -22727.f, 34091.f, ArenaHalf },
			{      0.f, 34091.f, ArenaHalf },
			{      0.f, 61364.f, TutorialArenaHalf },
		};

		for (const FArena2D& Arena : Arenas)
		{
			if (FMath::Abs(Location.X - Arena.X) <= (Arena.Half + ArenaMargin)
				&& FMath::Abs(Location.Y - Arena.Y) <= (Arena.Half + ArenaMargin))
			{
				return true;
			}
		}

		return false;
	};

	auto IsGoodLoc = [&](const FVector& Location, const float CandidateRadius) -> bool
	{
		if (IsInsideNoSpawnZone(Location))
		{
			return false;
		}

		for (const FUsedStageEffectLoc& Used : UsedLocs)
		{
			const float RequiredRadius = FMath::Max(CandidateRadius, Used.ExclusionRadius);
			if (FVector::DistSquared2D(Location, Used.Loc) < (RequiredRadius * RequiredRadius))
			{
				return false;
			}
		}

		if (Registry)
		{
			for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Registry->GetNPCs())
			{
				const AT66HouseNPCBase* NPC = WeakNPC.Get();
				if (!NPC)
				{
					continue;
				}

				const float Radius = NPC->GetSafeZoneRadius() + SafeBubbleMargin + CandidateRadius * 0.35f;
				if (FVector::DistSquared2D(Location, NPC->GetActorLocation()) < (Radius * Radius))
				{
					return false;
				}
			}

			for (const TWeakObjectPtr<AT66CircusInteractable>& WeakCircus : Registry->GetCircuses())
			{
				const AT66CircusInteractable* Circus = WeakCircus.Get();
				if (!Circus)
				{
					continue;
				}

				const float Radius = Circus->GetSafeZoneRadius() + SafeBubbleMargin + CandidateRadius * 0.35f;
				if (FVector::DistSquared2D(Location, Circus->GetActorLocation()) < (Radius * Radius))
				{
					return false;
				}
			}
		}

		return true;
	};

	auto NoteUsedLoc = [&](const FVector& Location, const float Radius)
	{
		UsedLocs.Add({ Location, Radius });
	};

	struct FStageLavaSpawnHit
	{
		FVector Location = FVector::ZeroVector;
		bool bFound = false;
	};

	auto FindSpawnLoc = [&](const float CandidateRadius, const int32 MaxTries) -> FStageLavaSpawnHit
	{
		for (int32 TryIndex = 0; TryIndex < MaxTries; ++TryIndex)
		{
			const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			FVector Location(X, Y, SpawnZ);

			FHitResult Hit;
			const FVector TraceStart = Location + FVector(0.f, 0.f, 2000.f);
			const FVector TraceEnd = Location - FVector(0.f, 0.f, 6000.f);
			if (!World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
			{
				continue;
			}

			if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
			{
				continue;
			}

			Location = Hit.ImpactPoint;
			if (IsGoodLoc(Location, CandidateRadius))
			{
				return { Location, true };
			}
		}

		return {};
	};

	auto GetLavaDamagePerTick = [&]() -> int32
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Easy:       return 6;
		case ET66Difficulty::Medium:     return 8;
		case ET66Difficulty::Hard:       return 10;
		case ET66Difficulty::VeryHard:   return 12;
		case ET66Difficulty::Impossible: return 14;
		default:                         return 8;
		}
	};

	auto ConfigureLavaPatch = [&](AT66LavaPatch* Lava)
	{
		if (!Lava)
		{
			return;
		}

		FVector2D Flow(Rng.FRandRange(-1.f, 1.f), Rng.FRandRange(-1.f, 1.f));
		if (Flow.IsNearlyZero())
		{
			Flow = FVector2D(1.f, 0.f);
		}
		else
		{
			Flow.Normalize();
		}

		Lava->PatchSize = Rng.FRandRange(1400.f, 2400.f);
		Lava->HoverHeight = 1.5f;
		Lava->CollisionHeight = 160.f;
		Lava->TextureResolution = 64;
		Lava->AnimationFrames = 18;
		Lava->AnimationFPS = 12.f;
		Lava->WarpSpeed = Rng.FRandRange(0.90f, 1.25f);
		Lava->WarpIntensity = Rng.FRandRange(0.10f, 0.15f);
		Lava->WarpCloseness = Rng.FRandRange(1.80f, 2.40f);
		Lava->FlowDir = Flow;
		Lava->FlowSpeed = Rng.FRandRange(0.12f, 0.26f);
		Lava->UVScale = Rng.FRandRange(3.60f, 5.40f);
		Lava->CellDensity = Rng.FRandRange(5.20f, 7.40f);
		Lava->EdgeContrast = Rng.FRandRange(6.40f, 8.80f);
		Lava->Brightness = Rng.FRandRange(2.10f, 2.90f);
		Lava->PatternOffset = FVector2D(Rng.FRandRange(-12.f, 12.f), Rng.FRandRange(-12.f, 12.f));
		Lava->DamagePerTick = GetLavaDamagePerTick();
		Lava->DamageIntervalSeconds = 0.45f;
		Lava->bDamageHero = true;
	};

	int32 SpawnedLavaCount = 0;
	for (int32 PatchIndex = 0; PatchIndex < LavaPatchCount; ++PatchIndex)
	{
		const FStageLavaSpawnHit SpawnHit = FindSpawnLoc(LavaExclusionRadius, 120);
		if (!SpawnHit.bFound)
		{
			continue;
		}

		const FRotator LavaRotation(0.f, Rng.FRandRange(0.f, 360.f), 0.f);
		AT66LavaPatch* Lava = World->SpawnActorDeferred<AT66LavaPatch>(
			AT66LavaPatch::StaticClass(),
			FTransform(LavaRotation, SpawnHit.Location),
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!Lava)
		{
			continue;
		}

		ConfigureLavaPatch(Lava);
		Lava->FinishSpawning(FTransform(LavaRotation, SpawnHit.Location));
		LegacyLavaPatches.Add(Lava);
		NoteUsedLoc(SpawnHit.Location, LavaExclusionRadius);
		++SpawnedLavaCount;
	}

	return SpawnedLavaCount;
}

void AT66MiasmaManager::SetExpansionActive(const bool bActive, const float ElapsedSeconds)
{
	bExplicitExpansionActive = bActive;

	if (!bExplicitExpansionActive)
	{
		ExplicitExpansionStartTimeSeconds = 0.0f;
		return;
	}

	if (UWorld* World = GetWorld())
	{
		ExplicitExpansionStartTimeSeconds = World->GetTimeSeconds() - FMath::Max(ElapsedSeconds, 0.0f);
	}
	else
	{
		ExplicitExpansionStartTimeSeconds = 0.0f;
	}

	UpdateFromRunState();
}

void AT66MiasmaManager::SetTowerSourceAnchor(const int32 FloorNumber, const FVector& WorldAnchor)
{
	if (FloorNumber <= 0)
	{
		return;
	}

	TowerSourceAnchorOverrides.Add(FloorNumber, WorldAnchor);
	if (ShouldUseTowerBloodLook() && TileCenters.Num() > 0)
	{
		ApplyTowerCoverageOrdering();
		RebuildSpawnedInstances();
	}
}

void AT66MiasmaManager::ClearTowerSourceAnchors()
{
	if (TowerSourceAnchorOverrides.Num() <= 0)
	{
		return;
	}

	TowerSourceAnchorOverrides.Reset();
	if (ShouldUseTowerBloodLook() && TileCenters.Num() > 0)
	{
		ApplyTowerCoverageOrdering();
		RebuildSpawnedInstances();
	}
}

void AT66MiasmaManager::EnsureSpawnedCount(int32 DesiredCount)
{
	if (!TileInstances)
	{
		return;
	}

	const FVector InstanceScale(FMath::Max(TileSize / 100.f, 0.1f), FMath::Max(TileSize / 100.f, 0.1f), 1.f);
	const int32 TargetCount = FMath::Min(DesiredCount, TileCenters.Num());
	if (SpawnedTileCount >= TargetCount)
	{
		return;
	}

	TArray<FTransform> InstanceTransforms;
	InstanceTransforms.Reserve(TargetCount - SpawnedTileCount);
	for (int32 TileIndex = SpawnedTileCount; TileIndex < TargetCount; ++TileIndex)
	{
		InstanceTransforms.Add(FTransform(FRotator::ZeroRotator, TileCenters[TileIndex], InstanceScale));
	}

	if (InstanceTransforms.Num() > 0)
	{
		TileInstances->AddInstances(InstanceTransforms, false, false, false);
		SpawnedTileCount = TargetCount;
	}
}

void AT66MiasmaManager::RebuildSpawnedInstances()
{
	if (!TileInstances)
	{
		return;
	}

	const int32 DesiredCount = SpawnedTileCount;
	TileInstances->ClearInstances();
	SpawnedTileCount = 0;
	EnsureSpawnedCount(DesiredCount);
}

void AT66MiasmaManager::TickDamageOverActiveTiles(float DeltaTime)
{
	if (bSpawningPaused || SpawnedTileCount <= 0 || DamageIntervalSeconds <= 0.f)
	{
		return;
	}

	DamageTickAccumulator += DeltaTime;
	if (DamageTickAccumulator < DamageIntervalSeconds)
	{
		return;
	}
	DamageTickAccumulator = 0.f;

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	AT66HeroBase* Hero = Cast<AT66HeroBase>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!RunState || !Hero)
	{
		return;
	}

	const FVector HeroLocation = Hero->GetActorLocation();
	const float HalfExtent = TileSize * 0.5f;
	const bool bTowerBlood = ShouldUseTowerBloodLook();
	for (int32 Index = 0; Index < SpawnedTileCount; ++Index)
	{
		const FVector& TileCenter = TileCenters[Index];
		if ((!bTowerBlood || FMath::Abs(HeroLocation.Z - TileCenter.Z) <= 1600.0f)
			&& FMath::Abs(HeroLocation.X - TileCenter.X) <= HalfExtent
			&& FMath::Abs(HeroLocation.Y - TileCenter.Y) <= HalfExtent)
		{
			RunState->ApplyDamage(20, this);
			return;
		}
	}
}

void AT66MiasmaManager::EnsureVisualMaterial()
{
	if (!TileInstances)
	{
		return;
	}

	if (!LavaMID)
	{
		if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, T66LavaShared::BaseMaterialPath))
		{
			LavaMID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			if (LavaMID)
			{
				TileInstances->SetMaterial(0, LavaMID);
			}
		}
	}

	if (!LavaMID)
	{
		return;
	}

	UTexture* TextureToUse = GeneratedFrames.IsValidIndex(CurrentFrameIndex)
		? GeneratedFrames[CurrentFrameIndex].Get()
		: T66GetFallbackTexture();
	if (TextureToUse)
	{
		LavaMID->SetTextureParameterValue(TEXT("DiffuseColorMap"), TextureToUse);
		LavaMID->SetTextureParameterValue(TEXT("BaseColorTexture"), TextureToUse);
	}

	ApplyMaterialLookIfNeeded(FLinearColor::White, Brightness);

	if (ShouldUseTowerBloodLook())
	{
		CoreColor = FLinearColor(0.06f, 0.00f, 0.00f, 1.0f);
		MidColor = FLinearColor(0.48f, 0.00f, 0.02f, 1.0f);
		GlowColor = FLinearColor(0.86f, 0.05f, 0.08f, 1.0f);
		Brightness = 1.45f;
		ApplyMaterialLookIfNeeded(FLinearColor(0.95f, 0.22f, 0.24f, 1.0f), Brightness);
	}
}

bool AT66MiasmaManager::ShouldUseTowerBloodLook() const
{
	const UWorld* World = GetWorld();
	const AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	return GameMode && GameMode->IsUsingTowerMainMapLayout();
}

bool AT66MiasmaManager::TryGetExpansionElapsedSeconds(float& OutElapsedSeconds) const
{
	OutElapsedSeconds = 0.0f;

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	if (bExplicitExpansionActive)
	{
		OutElapsedSeconds = FMath::Max(World->GetTimeSeconds() - ExplicitExpansionStartTimeSeconds, 0.0f);
		return true;
	}

	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState || !RunState->GetStageTimerActive())
	{
		return false;
	}

	const float Remaining = RunState->GetStageTimerSecondsRemaining();
	const float Duration = UT66RunStateSubsystem::StageTimerDurationSeconds;
	OutElapsedSeconds = FMath::Clamp(Duration - Remaining, 0.0f, Duration);
	return true;
}

FVector AT66MiasmaManager::ResolveTowerSourceAnchor(const int32 FloorNumber, const FVector& FallbackAnchor) const
{
	if (const FVector* OverrideAnchor = TowerSourceAnchorOverrides.Find(FloorNumber))
	{
		return *OverrideAnchor;
	}

	if (const FVector* DefaultAnchor = TowerDefaultSourceAnchors.Find(FloorNumber))
	{
		return *DefaultAnchor;
	}

	return FallbackAnchor;
}

void AT66MiasmaManager::GenerateAnimationFrames()
{
	GeneratedFrames.Reset();
	CurrentFrameIndex = INDEX_NONE;

	const int32 Resolution = FMath::Clamp(TextureResolution, 16, 256);
	const int32 ClampedFrames = FMath::Clamp(AnimationFrames, 4, 64);

	GeneratedFrames.Reserve(ClampedFrames);
	for (int32 FrameIndex = 0; FrameIndex < ClampedFrames; ++FrameIndex)
	{
		if (UTexture2D* Texture = BuildFrameTexture(FrameIndex, ClampedFrames, Resolution))
		{
			GeneratedFrames.Add(Texture);
		}
	}

	if (GeneratedFrames.Num() > 0)
	{
		CurrentFrameIndex = 0;
	}
}

UTexture2D* AT66MiasmaManager::BuildFrameTexture(const int32 FrameIndex, const int32 ClampedFrames, const int32 Resolution) const
{
	if (Resolution <= 0 || ClampedFrames <= 0)
	{
		return nullptr;
	}

	TArray<FColor> Pixels;
	Pixels.SetNumUninitialized(Resolution * Resolution);

	const float Phase = (static_cast<float>(FrameIndex) / static_cast<float>(ClampedFrames)) * T66LavaShared::TwoPi;

	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			const FVector2D UV(
				(static_cast<float>(X) + 0.5f) / static_cast<float>(Resolution),
				(static_cast<float>(Y) + 0.5f) / static_cast<float>(Resolution));

			Pixels[Y * Resolution + X] = SampleLavaColor(UV, Phase).ToFColorSRGB();
		}
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(Resolution, Resolution, PF_B8G8R8A8);
	if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
	{
		return nullptr;
	}

	Texture->SRGB = true;
	Texture->Filter = TF_Nearest;
	Texture->AddressX = TA_Wrap;
	Texture->AddressY = TA_Wrap;
	Texture->LODGroup = TEXTUREGROUP_Pixels2D;
	Texture->NeverStream = true;
#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TMGS_NoMipmaps;
#endif

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* MipData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(MipData, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
	Mip.BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}

FLinearColor AT66MiasmaManager::SampleLavaColor(const FVector2D& BaseUV, const float Phase) const
{
	FVector2D UV = BaseUV * FMath::Max(UVScale, 0.1f) + PatternOffset;

	const FVector2D FlowNormal = FlowDir.IsNearlyZero() ? FVector2D(1.f, 0.f) : FlowDir.GetSafeNormal();
	UV += FlowNormal * Phase * FlowSpeed * 0.20f;

	const float SafeCloseness = FMath::Max(WarpCloseness, 0.01f);
	const float Nx = UV.X / SafeCloseness;
	const float Ny = UV.Y / SafeCloseness;
	const float WarpT = Phase * WarpSpeed;
	const float WarpedX = Nx + WarpIntensity * FMath::Sin(WarpT + Ny * 2.0f);
	const float WarpedY = Ny + WarpIntensity * FMath::Sin(WarpT + Nx * 2.0f);
	const FVector2D FinalUV = FVector2D(WarpedX, WarpedY) * SafeCloseness;

	const T66LavaShared::FCellSample Cells = T66LavaShared::SampleCells(FinalUV, FMath::Max(CellDensity, 1.0f), Phase);
	const float Border = FMath::Max(Cells.SecondClosest - Cells.Closest, 0.0f);
	const float Crack = FMath::Pow(T66LavaShared::Saturate(1.0f - Border * FMath::Max(EdgeContrast, 0.1f)), 2.3f);
	const float Pulse = 0.5f + 0.5f * FMath::Sin((FinalUV.X * 1.8f + FinalUV.Y * 1.25f) + Phase * 2.2f);
	const float Ember = T66LavaShared::Saturate(1.0f - T66LavaShared::SmoothStep(0.18f, 0.52f, Cells.Closest + Pulse * 0.06f));
	const float Heat = T66LavaShared::Saturate(Crack * 1.18f + Ember * 0.22f);

	FLinearColor Color = FMath::Lerp(CoreColor, MidColor, T66LavaShared::Saturate(Heat * 0.72f + Pulse * 0.10f));
	Color = FMath::Lerp(Color, GlowColor, T66LavaShared::Saturate(FMath::Pow(Crack, 0.72f)));

	const float PoolMask = T66LavaShared::SmoothStep(0.20f, 0.48f, Cells.Closest);
	Color = FMath::Lerp(Color, CoreColor * 0.55f, PoolMask * 0.35f);
	Color.A = 1.f;
	return Color.GetClamped(0.f, 1.f);
}

void AT66MiasmaManager::ApplyAnimationFrame(const int32 FrameIndex)
{
	if (!GeneratedFrames.IsValidIndex(FrameIndex))
	{
		return;
	}

	CurrentFrameIndex = FrameIndex;
	EnsureVisualMaterial();
}

void AT66MiasmaManager::ApplyMaterialLookIfNeeded(const FLinearColor& Tint, float InBrightness)
{
	if (!LavaMID)
	{
		return;
	}

	if (!bMaterialLookApplied
		|| !LastAppliedTint.Equals(Tint)
		|| !FMath::IsNearlyEqual(LastAppliedBrightness, InBrightness))
	{
		LavaMID->SetVectorParameterValue(TEXT("Tint"), Tint);
		LavaMID->SetVectorParameterValue(TEXT("BaseColor"), Tint);
		LavaMID->SetScalarParameterValue(TEXT("Brightness"), InBrightness);
		LastAppliedTint = Tint;
		LastAppliedBrightness = InBrightness;
		bMaterialLookApplied = true;
	}
}

void AT66MiasmaManager::ClearAllMiasma()
{
	if (TileInstances)
	{
		TileInstances->ClearInstances();
	}
	SpawnedTileCount = 0;
	DamageTickAccumulator = 0.f;
	ClearLegacyLavaPatches();
}

void AT66MiasmaManager::ClearLegacyLavaPatches()
{
	TArray<TWeakObjectPtr<AT66LavaPatch>> LavaPatchesToDestroy = LegacyLavaPatches;
	LegacyLavaPatches.Reset();

	for (const TWeakObjectPtr<AT66LavaPatch>& WeakLava : LavaPatchesToDestroy)
	{
		if (AT66LavaPatch* LavaPatch = WeakLava.Get())
		{
			LavaPatch->Destroy();
		}
	}
}
