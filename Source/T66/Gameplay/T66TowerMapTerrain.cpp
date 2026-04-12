// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TowerMapTerrain.h"

#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameplayLayout.h"
#include "Gameplay/T66TerrainThemeAssets.h"
#include "Gameplay/T66VisualUtil.h"
#include "Engine/CollisionProfile.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	static const FName T66MainMapTerrainVisualTag(TEXT("T66_MainMapTerrain_Visual"));
	static const FName T66MainMapTerrainMaterialsReadyTag(TEXT("T66_MainMapTerrain_MaterialsReady"));
	static const FName T66TraversalBarrierTag(TEXT("T66_Map_TraversalBarrier"));
	static const FName T66TowerCeilingTag(TEXT("T66_Tower_Ceiling"));
	static const FName T66FloorStartTag(TEXT("T66_Floor_Start"));
	static const FName T66FloorMainTag(TEXT("T66_Floor_Main"));
	static const FName T66FloorBossTag(TEXT("T66_Floor_Boss"));
	static constexpr int32 T66TowerFloorVertexCount = 4;

	enum class ET66TowerStageTheme : uint8
	{
		Dungeon,
		Forest,
	};

	static int32 T66GetTowerStageNumber(const UWorld* World)
	{
		if (!World)
		{
			return 1;
		}

		if (const UGameInstance* GI = World->GetGameInstance())
		{
			if (const UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				return FMath::Max(1, RunState->GetCurrentStage());
			}
		}

		return 1;
	}

	static ET66TowerStageTheme T66ResolveTowerStageTheme(const UWorld* World)
	{
		return T66GetTowerStageNumber(World) == 2
			? ET66TowerStageTheme::Forest
			: ET66TowerStageTheme::Dungeon;
	}

	static float T66GetTowerStartRoofOffset(const T66TowerMapTerrain::FLayout& Layout)
	{
		return FMath::Min(Layout.FloorSpacing - 120.0f, 1600.0f);
	}

	static float T66GetTowerSubFloorCeilingOffset(const T66TowerMapTerrain::FLayout& Layout)
	{
		return Layout.FloorSpacing;
	}

	static float T66GetTowerCeilingOffset(const T66TowerMapTerrain::FLayout& Layout, const T66TowerMapTerrain::FFloor& Floor)
	{
		return (Floor.FloorNumber == Layout.StartFloorNumber)
			? T66GetTowerStartRoofOffset(Layout)
			: T66GetTowerSubFloorCeilingOffset(Layout);
	}

	static bool T66ShouldOpenTowerStartRoofForDrop(const UWorld* World)
	{
		return false;
	}

	static FVector T66ComputeMeshScaleForHalfExtents(UStaticMesh* Mesh, const FVector& DesiredHalfExtents)
	{
		if (!Mesh)
		{
			return FVector(1.0f, 1.0f, 1.0f);
		}

		const FVector MeshHalfExtents = Mesh->GetBounds().BoxExtent;
		return FVector(
			DesiredHalfExtents.X / FMath::Max(MeshHalfExtents.X, 1.0f),
			DesiredHalfExtents.Y / FMath::Max(MeshHalfExtents.Y, 1.0f),
			DesiredHalfExtents.Z / FMath::Max(MeshHalfExtents.Z, 1.0f));
	}

	static AStaticMeshActor* T66SpawnStaticMeshActor(
		UWorld* World,
		UStaticMesh* Mesh,
		UMaterialInterface* Material,
		const FVector& Location,
		const FRotator& Rotation,
		const FVector& DesiredHalfExtents,
		const FActorSpawnParameters& SpawnParams,
		bool bEnableCollision,
		const TArray<FName>& ExtraTags)
	{
		if (!World || !Mesh)
		{
			return nullptr;
		}

		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
		if (!Actor)
		{
			return nullptr;
		}

		if (UStaticMeshComponent* MeshComponent = Actor->GetStaticMeshComponent())
		{
			MeshComponent->SetMobility(EComponentMobility::Movable);
			MeshComponent->SetStaticMesh(Mesh);
			MeshComponent->SetGenerateOverlapEvents(false);
			MeshComponent->SetRelativeScale3D(T66ComputeMeshScaleForHalfExtents(Mesh, DesiredHalfExtents));
			MeshComponent->SetCollisionEnabled(bEnableCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
			MeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
			MeshComponent->SetMobility(EComponentMobility::Static);
			if (Material)
			{
				const int32 MaterialCount = MeshComponent->GetNumMaterials();
				for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
				{
					MeshComponent->SetMaterial(MaterialIndex, Material);
				}
			}
			else
			{
				FT66VisualUtil::EnsureUnlitMaterials(MeshComponent, World);
			}

			MeshComponent->SetMobility(EComponentMobility::Static);
		}

		Actor->Tags.AddUnique(T66MainMapTerrainVisualTag);
		Actor->Tags.AddUnique(T66MainMapTerrainMaterialsReadyTag);
		for (const FName& Tag : ExtraTags)
		{
			if (!Tag.IsNone())
			{
				Actor->Tags.AddUnique(Tag);
			}
		}
		return Actor;
	}

	static AStaticMeshActor* T66SpawnGroundedMeshActor(
		UWorld* World,
		UStaticMesh* Mesh,
		UMaterialInterface* Material,
		const FVector& Location,
		const FRotator& Rotation,
		const FVector& Scale,
		const FActorSpawnParameters& SpawnParams,
		bool bEnableCollision,
		const TArray<FName>& ExtraTags)
	{
		if (!World || !Mesh)
		{
			return nullptr;
		}

		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
		if (!Actor)
		{
			return nullptr;
		}

		if (UStaticMeshComponent* MeshComponent = Actor->GetStaticMeshComponent())
		{
			MeshComponent->SetMobility(EComponentMobility::Movable);
			MeshComponent->SetStaticMesh(Mesh);
			MeshComponent->SetGenerateOverlapEvents(false);
			MeshComponent->SetRelativeScale3D(Scale);
			FT66VisualUtil::GroundMeshToActorOrigin(MeshComponent, Mesh);
			MeshComponent->SetCollisionEnabled(bEnableCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
			MeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
			if (Material)
			{
				const int32 MaterialCount = MeshComponent->GetNumMaterials();
				for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
				{
					MeshComponent->SetMaterial(MaterialIndex, Material);
				}
			}
			FT66VisualUtil::EnsureUnlitMaterials(MeshComponent, World);
			MeshComponent->SetMobility(EComponentMobility::Static);
		}

		Actor->Tags.AddUnique(T66MainMapTerrainVisualTag);
		Actor->Tags.AddUnique(T66MainMapTerrainMaterialsReadyTag);
		for (const FName& Tag : ExtraTags)
		{
			if (!Tag.IsNone())
			{
				Actor->Tags.AddUnique(Tag);
			}
		}
		return Actor;
	}

	static void T66LoadTowerTreeMeshes(TArray<UStaticMesh*>& OutMeshes)
	{
		OutMeshes.Reset();

		static const TSoftObjectPtr<UStaticMesh> TreeMeshPath(FSoftObjectPath(TEXT("/Game/World/Props/Tree.Tree")));
		static const TSoftObjectPtr<UStaticMesh> TreeMesh2Path(FSoftObjectPath(TEXT("/Game/World/Props/Tree2.Tree2")));
		static const TSoftObjectPtr<UStaticMesh> TreeMesh3Path(FSoftObjectPath(TEXT("/Game/World/Props/Tree3.Tree3")));

		if (UStaticMesh* TreeMesh = TreeMeshPath.LoadSynchronous())
		{
			OutMeshes.Add(TreeMesh);
		}
		if (UStaticMesh* TreeMesh2 = TreeMesh2Path.LoadSynchronous())
		{
			OutMeshes.Add(TreeMesh2);
		}
		if (UStaticMesh* TreeMesh3 = TreeMesh3Path.LoadSynchronous())
		{
			OutMeshes.Add(TreeMesh3);
		}
	}

	static bool T66ShouldIgnoreTowerTraceHit(const FHitResult& Hit)
	{
		const AActor* HitActor = Hit.GetActor();
		return HitActor
			&& (HitActor->ActorHasTag(T66TowerCeilingTag) || HitActor->ActorHasTag(T66TraversalBarrierTag));
	}

	static bool T66TraceDownToSurface(UWorld* World, const T66TowerMapTerrain::FLayout& Layout, const FVector& DesiredLocation, FVector& OutLocation)
	{
		if (!World)
		{
			return false;
		}

		const float LocalTraceUp = FMath::Max(900.0f, Layout.FloorSpacing - Layout.FloorThickness - 250.0f);
		const float LocalTraceDown = FMath::Max(Layout.FloorThickness + 2600.0f, 3600.0f);
		const FVector TraceStart(DesiredLocation.X, DesiredLocation.Y, DesiredLocation.Z + LocalTraceUp);
		const FVector TraceEnd(DesiredLocation.X, DesiredLocation.Y, DesiredLocation.Z - LocalTraceDown);

		TArray<FHitResult> Hits;
		if (!World->LineTraceMultiByChannel(Hits, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			return false;
		}

		for (const FHitResult& Hit : Hits)
		{
			if (T66ShouldIgnoreTowerTraceHit(Hit))
			{
				continue;
			}

			if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
			{
				continue;
			}

			const float SurfaceTolerance = FMath::Max(120.0f, (Layout.FloorThickness * 0.5f) + 20.0f);
			if (FMath::Abs(Hit.ImpactPoint.Z - DesiredLocation.Z) > SurfaceTolerance)
			{
				continue;
			}

			OutLocation = Hit.ImpactPoint;
			return true;
		}

		return false;
	}

	static bool T66TryGetFloor(const T66TowerMapTerrain::FLayout& Layout, int32 FloorNumber, const T66TowerMapTerrain::FFloor*& OutFloor)
	{
		for (const T66TowerMapTerrain::FFloor& Floor : Layout.Floors)
		{
			if (Floor.FloorNumber == FloorNumber)
			{
				OutFloor = &Floor;
				return true;
			}
		}

		OutFloor = nullptr;
		return false;
	}

	static void T66BuildFloorPolygonVertices(const T66TowerMapTerrain::FFloor& Floor, TArray<FVector2D>& OutVertices)
	{
		OutVertices.Reset();
		OutVertices.Reserve(T66TowerFloorVertexCount);

		const float HalfExtent = Floor.BoundsHalfExtent;
		OutVertices.Add(FVector2D(Floor.Center.X - HalfExtent, Floor.Center.Y - HalfExtent));
		OutVertices.Add(FVector2D(Floor.Center.X + HalfExtent, Floor.Center.Y - HalfExtent));
		OutVertices.Add(FVector2D(Floor.Center.X + HalfExtent, Floor.Center.Y + HalfExtent));
		OutVertices.Add(FVector2D(Floor.Center.X - HalfExtent, Floor.Center.Y + HalfExtent));
	}

	static bool T66IsLocationInsideFloorPolygon(const T66TowerMapTerrain::FFloor& Floor, const FVector& Location, const float Margin = 0.0f)
	{
		const float BoundsLimit = Floor.BoundsHalfExtent + Margin;
		return BoundsLimit > 1.0f
			&& FMath::Abs(Location.X - Floor.Center.X) <= BoundsLimit
			&& FMath::Abs(Location.Y - Floor.Center.Y) <= BoundsLimit;
	}

	static bool T66TryGetPolygonXRangeAtY(const TArray<FVector2D>& PolygonVertices, const float WorldY, float& OutMinX, float& OutMaxX)
	{
		if (PolygonVertices.Num() < 3)
		{
			return false;
		}

		TArray<float, TInlineAllocator<T66TowerFloorVertexCount * 2>> Intersections;
		for (int32 VertexIndex = 0; VertexIndex < PolygonVertices.Num(); ++VertexIndex)
		{
			const FVector2D A = PolygonVertices[VertexIndex];
			const FVector2D B = PolygonVertices[(VertexIndex + 1) % PolygonVertices.Num()];
			if (FMath::IsNearlyEqual(A.Y, B.Y))
			{
				continue;
			}

			const float MinY = FMath::Min(A.Y, B.Y);
			const float MaxY = FMath::Max(A.Y, B.Y);
			if (WorldY < MinY || WorldY >= MaxY)
			{
				continue;
			}

			const float Alpha = (WorldY - A.Y) / (B.Y - A.Y);
			Intersections.Add(FMath::Lerp(A.X, B.X, Alpha));
		}

		if (Intersections.Num() < 2)
		{
			return false;
		}

		Intersections.Sort();
		OutMinX = Intersections[0];
		OutMaxX = Intersections.Last();
		return true;
	}

	static bool T66TryGetPolygonBandXRange(const TArray<FVector2D>& PolygonVertices, const float BandMinY, const float BandMaxY, float& OutMinX, float& OutMaxX)
	{
		const float CenterY = (BandMinY + BandMaxY) * 0.5f;
		float MinXLow = 0.0f;
		float MaxXLow = 0.0f;
		float MinXCenter = 0.0f;
		float MaxXCenter = 0.0f;
		float MinXHigh = 0.0f;
		float MaxXHigh = 0.0f;
		if (!T66TryGetPolygonXRangeAtY(PolygonVertices, BandMinY + 1.0f, MinXLow, MaxXLow)
			|| !T66TryGetPolygonXRangeAtY(PolygonVertices, CenterY, MinXCenter, MaxXCenter)
			|| !T66TryGetPolygonXRangeAtY(PolygonVertices, BandMaxY - 1.0f, MinXHigh, MaxXHigh))
		{
			return false;
		}

		OutMinX = FMath::Max3(MinXLow, MinXCenter, MinXHigh);
		OutMaxX = FMath::Min3(MaxXLow, MaxXCenter, MaxXHigh);
		return OutMaxX > OutMinX;
	}

	static bool T66IsLocationInsideFloorBounds(const T66TowerMapTerrain::FFloor& Floor, const FVector& Location, const float Margin = 0.0f)
	{
		return T66IsLocationInsideFloorPolygon(Floor, Location, Margin);
	}

	static bool T66IsLocationInsideFloorHole(const T66TowerMapTerrain::FFloor& Floor, const FVector& Location, const float Margin = 0.0f)
	{
		if (!Floor.bHasDropHole)
		{
			return false;
		}

		return FMath::Abs(Location.X - Floor.HoleCenter.X) <= (Floor.HoleHalfExtent.X + Margin)
			&& FMath::Abs(Location.Y - Floor.HoleCenter.Y) <= (Floor.HoleHalfExtent.Y + Margin);
	}

	static bool T66IsLocationInsideMazeWallBox(const FBox2D& WallBox, const FVector& Location, const float Margin = 0.0f)
	{
		return Location.X >= (WallBox.Min.X - Margin)
			&& Location.X <= (WallBox.Max.X + Margin)
			&& Location.Y >= (WallBox.Min.Y - Margin)
			&& Location.Y <= (WallBox.Max.Y + Margin);
	}

	static bool T66IsLocationInsideFloorMazeWalls(const T66TowerMapTerrain::FFloor& Floor, const FVector& Location, const float Margin = 0.0f)
	{
		for (const FBox2D& WallBox : Floor.MazeWallBoxes)
		{
			if (T66IsLocationInsideMazeWallBox(WallBox, Location, Margin))
			{
				return true;
			}
		}

		return false;
	}

	static bool T66IsWalkableTowerLocation(
		const T66TowerMapTerrain::FFloor& Floor,
		const FVector& Location,
		const float EdgePadding = 0.0f,
		const float HolePadding = 0.0f,
		const float WallPadding = 0.0f)
	{
		if (!T66IsLocationInsideFloorBounds(Floor, Location, -EdgePadding))
		{
			return false;
		}

		return !T66IsLocationInsideFloorHole(Floor, Location, HolePadding)
			&& !T66IsLocationInsideFloorMazeWalls(Floor, Location, WallPadding);
	}

	static void T66AddInterval(TArray<FVector2D>& Intervals, const float Center, const float HalfWidth, const float MinLimit, const float MaxLimit)
	{
		const float IntervalMin = FMath::Clamp(Center - HalfWidth, MinLimit, MaxLimit);
		const float IntervalMax = FMath::Clamp(Center + HalfWidth, MinLimit, MaxLimit);
		if ((IntervalMax - IntervalMin) > 10.0f)
		{
			Intervals.Add(FVector2D(IntervalMin, IntervalMax));
		}
	}

	static void T66NormalizeIntervals(TArray<FVector2D>& Intervals)
	{
		if (Intervals.Num() <= 1)
		{
			return;
		}

		Intervals.Sort([](const FVector2D& A, const FVector2D& B)
		{
			return A.X < B.X;
		});

		TArray<FVector2D> Merged;
		Merged.Reserve(Intervals.Num());
		FVector2D Current = Intervals[0];
		for (int32 Index = 1; Index < Intervals.Num(); ++Index)
		{
			const FVector2D& Next = Intervals[Index];
			if (Next.X <= (Current.Y + 1.0f))
			{
				Current.Y = FMath::Max(Current.Y, Next.Y);
				continue;
			}

			Merged.Add(Current);
			Current = Next;
		}

		Merged.Add(Current);
		Intervals = MoveTemp(Merged);
	}

	static void T66AddMazeWallBox(
		T66TowerMapTerrain::FFloor& Floor,
		const bool bVertical,
		const float FixedCoordinate,
		const float SpanMin,
		const float SpanMax,
		const float HalfThickness)
	{
		if ((SpanMax - SpanMin) <= 10.0f)
		{
			return;
		}

		const FVector2D Min = bVertical
			? FVector2D(FixedCoordinate - HalfThickness, SpanMin)
			: FVector2D(SpanMin, FixedCoordinate - HalfThickness);
		const FVector2D Max = bVertical
			? FVector2D(FixedCoordinate + HalfThickness, SpanMax)
			: FVector2D(SpanMax, FixedCoordinate + HalfThickness);
		Floor.MazeWallBoxes.Add(FBox2D(Min, Max));
	}

	static void T66BuildMazeWallLane(
		T66TowerMapTerrain::FFloor& Floor,
		const bool bVertical,
		const float FixedCoordinate,
		const float SpanHalfExtent,
		const TArray<FVector2D>& ClearIntervals,
		const float HalfThickness,
		const float MinSegmentLength)
	{
		TArray<FVector2D> MergedClearIntervals = ClearIntervals;
		T66NormalizeIntervals(MergedClearIntervals);

		float Cursor = -SpanHalfExtent;
		for (const FVector2D& Interval : MergedClearIntervals)
		{
			const float ClampedMin = FMath::Clamp(Interval.X, -SpanHalfExtent, SpanHalfExtent);
			const float ClampedMax = FMath::Clamp(Interval.Y, -SpanHalfExtent, SpanHalfExtent);
			if ((ClampedMin - Cursor) >= MinSegmentLength)
			{
				T66AddMazeWallBox(Floor, bVertical, FixedCoordinate, Cursor, ClampedMin, HalfThickness);
			}
			Cursor = FMath::Max(Cursor, ClampedMax);
		}

		if ((SpanHalfExtent - Cursor) >= MinSegmentLength)
		{
			T66AddMazeWallBox(Floor, bVertical, FixedCoordinate, Cursor, SpanHalfExtent, HalfThickness);
		}
	}

	static float T66SnapCoordinateToTowerTileGrid(const float LocalCoordinate, const float BoundsHalfExtent, const float TileSize)
	{
		const float TileHalfSize = TileSize * 0.5f;
		const float GridMin = -BoundsHalfExtent + TileHalfSize;
		const float GridMax = BoundsHalfExtent - TileHalfSize;
		if (GridMax <= GridMin)
		{
			return 0.0f;
		}

		const float GridIndex = FMath::RoundToFloat((LocalCoordinate - GridMin) / TileSize);
		return FMath::Clamp(GridMin + (GridIndex * TileSize), GridMin, GridMax);
	}

	static FVector T66BuildSquareHoleOffset(
		FRandomStream& Rng,
		const float WalkableHalfExtent,
		const float BoundsHalfExtent,
		const FVector2D& HoleHalfExtent,
		const float TileSize)
	{
		const float HoleMaxHalfExtent = HoleHalfExtent.GetMax();
		const float SideBandMin = FMath::Max(HoleMaxHalfExtent + 900.0f, WalkableHalfExtent * 0.46f);
		const float SideBandMax = FMath::Max(SideBandMin, WalkableHalfExtent - HoleMaxHalfExtent - 700.0f);
		const float SweepLimit = FMath::Max(700.0f, FMath::Min(WalkableHalfExtent * 0.38f, WalkableHalfExtent - HoleMaxHalfExtent - 700.0f));

		FVector Candidate = FVector::ZeroVector;
		switch (Rng.RandRange(0, 3))
		{
		case 0:
			Candidate = FVector(Rng.FRandRange(SideBandMin, SideBandMax), Rng.FRandRange(-SweepLimit, SweepLimit), 0.0f);
			break;
		case 1:
			Candidate = FVector(Rng.FRandRange(-SideBandMax, -SideBandMin), Rng.FRandRange(-SweepLimit, SweepLimit), 0.0f);
			break;
		case 2:
			Candidate = FVector(Rng.FRandRange(-SweepLimit, SweepLimit), Rng.FRandRange(SideBandMin, SideBandMax), 0.0f);
			break;
		default:
			Candidate = FVector(Rng.FRandRange(-SweepLimit, SweepLimit), Rng.FRandRange(-SideBandMax, -SideBandMin), 0.0f);
			break;
		}

		const float MaxSnapCenterX = FMath::Max(0.0f, BoundsHalfExtent - HoleHalfExtent.X);
		const float MaxSnapCenterY = FMath::Max(0.0f, BoundsHalfExtent - HoleHalfExtent.Y);
		Candidate.X = FMath::Clamp(
			T66SnapCoordinateToTowerTileGrid(Candidate.X, BoundsHalfExtent, TileSize),
			-MaxSnapCenterX,
			MaxSnapCenterX);
		Candidate.Y = FMath::Clamp(
			T66SnapCoordinateToTowerTileGrid(Candidate.Y, BoundsHalfExtent, TileSize),
			-MaxSnapCenterY,
			MaxSnapCenterY);
		return Candidate;
	}

	static void T66BuildFloorMazeWalls(
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor,
		FRandomStream& Rng)
	{
		Floor.MazeWallBoxes.Reset();
		if (!Floor.bGameplayFloor
			|| Floor.FloorNumber == Layout.StartFloorNumber
			|| Floor.FloorNumber == Layout.BossFloorNumber)
		{
			return;
		}

		const float LaneSpacing = Layout.PlacementCellSize * 3.0f;
		const float HalfThickness = FMath::Max(120.0f, Layout.PlacementCellSize * 0.18f);
		const float DoorHalfWidth = Layout.PlacementCellSize * 0.95f;
		const float CenterDoorHalfWidth = Layout.PlacementCellSize * 0.80f;
		const float HoleDoorHalfWidth = Layout.PlacementCellSize * 1.35f;
		const float SpanHalfExtent = Floor.BoundsHalfExtent - FMath::Max(HalfThickness, Layout.WallThickness * 0.5f);
		const float MinSegmentLength = Layout.PlacementCellSize * 0.90f;
		if (SpanHalfExtent <= (LaneSpacing + DoorHalfWidth))
		{
			return;
		}

		const FVector2D HoleLocal = FVector2D(Floor.HoleCenter.X - Floor.Center.X, Floor.HoleCenter.Y - Floor.Center.Y);
		const int32 LaneCount = FMath::FloorToInt(SpanHalfExtent / LaneSpacing);
		for (int32 LaneIndex = -LaneCount; LaneIndex <= LaneCount; ++LaneIndex)
		{
			if (LaneIndex == 0)
			{
				continue;
			}

			const float LaneCoordinate = static_cast<float>(LaneIndex) * LaneSpacing;
			if (FMath::Abs(LaneCoordinate) >= (SpanHalfExtent - (Layout.PlacementCellSize * 0.4f)))
			{
				continue;
			}

			if ((FMath::Abs(LaneIndex) % 2) == 0)
			{
				TArray<FVector2D> VerticalClears;
				T66AddInterval(
					VerticalClears,
					Rng.FRandRange(-SpanHalfExtent * 0.62f, -SpanHalfExtent * 0.18f),
					DoorHalfWidth,
					-SpanHalfExtent,
					SpanHalfExtent);
				T66AddInterval(
					VerticalClears,
					Rng.FRandRange(SpanHalfExtent * 0.18f, SpanHalfExtent * 0.62f),
					DoorHalfWidth,
					-SpanHalfExtent,
					SpanHalfExtent);
				if (((LaneIndex + Floor.FloorNumber) & 1) == 0)
				{
					T66AddInterval(VerticalClears, 0.0f, CenterDoorHalfWidth, -SpanHalfExtent, SpanHalfExtent);
				}
				if (Floor.bHasDropHole
					&& FMath::Abs(LaneCoordinate - HoleLocal.X) <= (Floor.HoleHalfExtent.X + HalfThickness + (Layout.PlacementCellSize * 0.45f)))
				{
					T66AddInterval(
						VerticalClears,
						HoleLocal.Y,
						Floor.HoleHalfExtent.Y + HoleDoorHalfWidth,
						-SpanHalfExtent,
						SpanHalfExtent);
				}

				T66BuildMazeWallLane(Floor, true, Floor.Center.X + LaneCoordinate, SpanHalfExtent, VerticalClears, HalfThickness, MinSegmentLength);
			}
			else
			{
				TArray<FVector2D> HorizontalClears;
				T66AddInterval(
					HorizontalClears,
					Rng.FRandRange(-SpanHalfExtent * 0.62f, -SpanHalfExtent * 0.18f),
					DoorHalfWidth,
					-SpanHalfExtent,
					SpanHalfExtent);
				T66AddInterval(
					HorizontalClears,
					Rng.FRandRange(SpanHalfExtent * 0.18f, SpanHalfExtent * 0.62f),
					DoorHalfWidth,
					-SpanHalfExtent,
					SpanHalfExtent);
				if (((LaneIndex + Floor.FloorNumber) & 1) != 0)
				{
					T66AddInterval(HorizontalClears, 0.0f, CenterDoorHalfWidth, -SpanHalfExtent, SpanHalfExtent);
				}
				if (Floor.bHasDropHole
					&& FMath::Abs(LaneCoordinate - HoleLocal.Y) <= (Floor.HoleHalfExtent.Y + HalfThickness + (Layout.PlacementCellSize * 0.45f)))
				{
					T66AddInterval(
						HorizontalClears,
						HoleLocal.X,
						Floor.HoleHalfExtent.X + HoleDoorHalfWidth,
						-SpanHalfExtent,
						SpanHalfExtent);
				}

				T66BuildMazeWallLane(Floor, false, Floor.Center.Y + LaneCoordinate, SpanHalfExtent, HorizontalClears, HalfThickness, MinSegmentLength);
			}
		}
	}

	static constexpr float T66TowerStartSafeSpawnSideCells = 0.0f;
	static constexpr float T66TowerStartSafeSpawnInwardCells = -1.10f;

	static bool T66TryComputeStartPlacementLocation(
		const T66TowerMapTerrain::FLayout& Layout,
		float SideCells,
		float InwardCells,
		FVector& OutLocation)
	{
		if (Layout.StartAreaCenterSurfaceLocation.IsNearlyZero()
			|| Layout.StartAnchorSurfaceLocation.IsNearlyZero()
			|| Layout.StartPathSurfaceLocation.IsNearlyZero())
		{
			return false;
		}

		const FVector InwardDirection = (Layout.StartAnchorSurfaceLocation - Layout.StartPathSurfaceLocation).GetSafeNormal2D();
		if (InwardDirection.IsNearlyZero())
		{
			return false;
		}

		const FVector SideDirection(-InwardDirection.Y, InwardDirection.X, 0.0f);
		OutLocation = Layout.StartAreaCenterSurfaceLocation
			+ (SideDirection * (SideCells * Layout.PlacementCellSize))
			+ (InwardDirection * (InwardCells * Layout.PlacementCellSize));
		return true;
	}

	static void T66BuildStartFloorRoom(
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor,
		const FVector& SpawnLocation,
		const FVector& AltarLocation)
	{
		Floor.MazeWallBoxes.Reset();

		const float HalfThickness = FMath::Max(Layout.WallThickness * 0.5f, Layout.PlacementCellSize * 0.18f);
		// Keep the closed start room tight, but leave enough clearance for the default third-person camera.
		const float SidePadding = Layout.PlacementCellSize * 2.00f;
		const float FrontPadding = Layout.PlacementCellSize * 2.35f;
		const float BackPadding = Layout.PlacementCellSize * 0.95f;
		const float MinRoomWidth = Layout.PlacementCellSize * 4.20f;
		const float MinRoomDepth = Layout.PlacementCellSize * 5.50f;
		const float RoomBoundsInset = Layout.WallThickness + (Layout.PlacementCellSize * 0.20f);
		const float BoundsMinX = Floor.Center.X - (Floor.BoundsHalfExtent - RoomBoundsInset);
		const float BoundsMaxX = Floor.Center.X + (Floor.BoundsHalfExtent - RoomBoundsInset);
		const float BoundsMinY = Floor.Center.Y - (Floor.BoundsHalfExtent - RoomBoundsInset);
		const float BoundsMaxY = Floor.Center.Y + (Floor.BoundsHalfExtent - RoomBoundsInset);

		float MinX = FMath::Clamp(
			FMath::Min3(SpawnLocation.X, AltarLocation.X, Floor.HoleCenter.X) - SidePadding,
			BoundsMinX,
			BoundsMaxX);
		float MaxX = FMath::Clamp(
			FMath::Max3(SpawnLocation.X, AltarLocation.X, Floor.HoleCenter.X) + SidePadding,
			BoundsMinX,
			BoundsMaxX);
		float MinY = FMath::Clamp(Floor.HoleCenter.Y - BackPadding, BoundsMinY, BoundsMaxY);
		float MaxY = FMath::Clamp(SpawnLocation.Y + FrontPadding, BoundsMinY, BoundsMaxY);

		auto ExpandRangeToMinimum = [](float& RangeMin, float& RangeMax, const float DesiredSize, const float ClampMin, const float ClampMax)
		{
			float CurrentSize = RangeMax - RangeMin;
			if (CurrentSize >= DesiredSize)
			{
				return;
			}

			const float Center = (RangeMin + RangeMax) * 0.5f;
			const float HalfDesiredSize = DesiredSize * 0.5f;
			RangeMin = FMath::Clamp(Center - HalfDesiredSize, ClampMin, ClampMax);
			RangeMax = FMath::Clamp(Center + HalfDesiredSize, ClampMin, ClampMax);
			CurrentSize = RangeMax - RangeMin;
			if (CurrentSize >= DesiredSize)
			{
				return;
			}

			if (FMath::IsNearlyEqual(RangeMin, ClampMin))
			{
				RangeMax = FMath::Clamp(RangeMin + DesiredSize, ClampMin, ClampMax);
			}
			else
			{
				RangeMin = FMath::Clamp(RangeMax - DesiredSize, ClampMin, ClampMax);
			}
		};

		ExpandRangeToMinimum(MinX, MaxX, MinRoomWidth, BoundsMinX, BoundsMaxX);
		ExpandRangeToMinimum(MinY, MaxY, MinRoomDepth, BoundsMinY, BoundsMaxY);

		Floor.MazeWallBoxes.Add(FBox2D(FVector2D(MinX - HalfThickness, MinY), FVector2D(MinX + HalfThickness, MaxY)));
		Floor.MazeWallBoxes.Add(FBox2D(FVector2D(MaxX - HalfThickness, MinY), FVector2D(MaxX + HalfThickness, MaxY)));
		Floor.MazeWallBoxes.Add(FBox2D(FVector2D(MinX, MinY - HalfThickness), FVector2D(MaxX, MinY + HalfThickness)));
		Floor.MazeWallBoxes.Add(FBox2D(FVector2D(MinX, MaxY - HalfThickness), FVector2D(MaxX, MaxY + HalfThickness)));
	}

	static void T66SpawnFloorSlab(
		UWorld* World,
		UStaticMesh* CubeMesh,
		UMaterialInterface* GroundMaterial,
		const FVector& FloorCenter,
		const float SurfaceZ,
		const float FloorThickness,
		const FVector2D& Min,
		const FVector2D& Max,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags)
	{
		const FVector2D Span = Max - Min;
		if (Span.X <= 1.0f || Span.Y <= 1.0f)
		{
			return;
		}

		const FVector SlabLocation(
			FloorCenter.X + ((Min.X + Max.X) * 0.5f),
			FloorCenter.Y + ((Min.Y + Max.Y) * 0.5f),
			SurfaceZ - (FloorThickness * 0.5f));
		T66SpawnStaticMeshActor(
			World,
			CubeMesh,
			GroundMaterial,
			SlabLocation,
			FRotator::ZeroRotator,
			FVector(Span.X * 0.5f, Span.Y * 0.5f, FloorThickness * 0.5f),
			SpawnParams,
			true,
			Tags);
	}

	static void T66SpawnPolygonFloor(
		UWorld* World,
		UStaticMesh* CubeMesh,
		UMaterialInterface* GroundMaterial,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags)
	{
		const float TileSize = FMath::Max(600.0f, Layout.PlacementCellSize);
		const float TileHalfSize = TileSize * 0.5f;
		const float HoleTileMargin = FMath::Min(40.0f, TileHalfSize * 0.10f);
		const float PolygonMinY = Floor.Center.Y - Floor.BoundsHalfExtent;
		const float PolygonMaxY = Floor.Center.Y + Floor.BoundsHalfExtent;
		const float PolygonMinX = Floor.Center.X - Floor.BoundsHalfExtent;
		const float PolygonMaxX = Floor.Center.X + Floor.BoundsHalfExtent;

		for (float TileMinY = PolygonMinY; TileMinY < PolygonMaxY - KINDA_SMALL_NUMBER; TileMinY += TileSize)
		{
			const float TileMaxY = FMath::Min(TileMinY + TileSize, PolygonMaxY);
			const float TileCenterY = (TileMinY + TileMaxY) * 0.5f;
			for (float TileMinX = PolygonMinX; TileMinX < PolygonMaxX - KINDA_SMALL_NUMBER; TileMinX += TileSize)
			{
				const float TileMaxX = FMath::Min(TileMinX + TileSize, PolygonMaxX);
				const float TileCenterX = (TileMinX + TileMaxX) * 0.5f;
				const FVector TileCenter(TileCenterX, TileCenterY, Floor.SurfaceZ);
				if (!T66IsLocationInsideFloorBounds(Floor, TileCenter))
				{
					continue;
				}

				const bool bInsideHoleTile = Floor.bHasDropHole
					&& FMath::Abs(TileCenterX - Floor.HoleCenter.X) <= FMath::Max(0.0f, Floor.HoleHalfExtent.X - HoleTileMargin)
					&& FMath::Abs(TileCenterY - Floor.HoleCenter.Y) <= FMath::Max(0.0f, Floor.HoleHalfExtent.Y - HoleTileMargin);
				if (bInsideHoleTile)
				{
					continue;
				}

				T66SpawnFloorSlab(
					World,
					CubeMesh,
					GroundMaterial,
					Floor.Center,
					Floor.SurfaceZ,
					Layout.FloorThickness,
					FVector2D(TileMinX - Floor.Center.X, TileMinY - Floor.Center.Y),
					FVector2D(TileMaxX - Floor.Center.X, TileMaxY - Floor.Center.Y),
					SpawnParams,
					Tags);
			}
		}
	}

	static void T66SpawnTreeWallBox(
		UWorld* World,
		const TArray<UStaticMesh*>& TreeMeshes,
		const FBox2D& WallBox,
		const float BaseZ,
		const float DesiredHeight,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags,
		const int32 Seed)
	{
		if (!World || TreeMeshes.Num() <= 0)
		{
			return;
		}

		const FVector2D WallCenter = (WallBox.Min + WallBox.Max) * 0.5f;
		const FVector2D WallHalfExtents = (WallBox.Max - WallBox.Min) * 0.5f;
		if (WallHalfExtents.X <= 5.0f || WallHalfExtents.Y <= 5.0f)
		{
			return;
		}

		const bool bVertical = WallHalfExtents.Y >= WallHalfExtents.X;
		const float SpanLength = bVertical ? (WallHalfExtents.Y * 2.0f) : (WallHalfExtents.X * 2.0f);
		const float WallThickness = bVertical ? (WallHalfExtents.X * 2.0f) : (WallHalfExtents.Y * 2.0f);
		const float DesiredHalfWidth = FMath::Max(220.0f, WallThickness * 1.15f);
		const float Spacing = FMath::Max(720.0f, DesiredHalfWidth * 2.90f);
		const int32 TreeCount = FMath::Max(2, FMath::CeilToInt(SpanLength / Spacing) + 1);

		FRandomStream Rng(Seed);
		const FVector2D LineStart = bVertical
			? FVector2D(WallCenter.X, WallBox.Min.Y)
			: FVector2D(WallBox.Min.X, WallCenter.Y);
		const FVector2D LineEnd = bVertical
			? FVector2D(WallCenter.X, WallBox.Max.Y)
			: FVector2D(WallBox.Max.X, WallCenter.Y);

		for (int32 TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
			UStaticMesh* Mesh = TreeMeshes[(Seed + TreeIndex) % TreeMeshes.Num()];
			if (!Mesh)
			{
				continue;
			}

			const float Alpha = (TreeCount <= 1)
				? 0.5f
				: (static_cast<float>(TreeIndex) / static_cast<float>(TreeCount - 1));
			const FVector2D TreePosition2D = FMath::Lerp(LineStart, LineEnd, Alpha);
			const FVector MeshHalfExtents = Mesh->GetBounds().BoxExtent;
			const float MeshRadius = FMath::Max(FMath::Max(MeshHalfExtents.X, MeshHalfExtents.Y), 1.0f);
			const float XYScale = FMath::Max(3.0f, DesiredHalfWidth / MeshRadius);
			const float ZScale = (DesiredHeight * 0.5f) / FMath::Max(MeshHalfExtents.Z, 1.0f);
			const float JitterYaw = Rng.FRandRange(-14.0f, 14.0f);
			const FVector Scale(XYScale, XYScale, ZScale);

			T66SpawnGroundedMeshActor(
				World,
				Mesh,
				nullptr,
				FVector(TreePosition2D.X, TreePosition2D.Y, BaseZ),
				FRotator(0.0f, JitterYaw, 0.0f),
				Scale,
				SpawnParams,
				true,
				Tags);
		}
	}

	static void T66SpawnTowerCeiling(
		UWorld* World,
		UStaticMesh* CubeMesh,
		UMaterialInterface* RoofMaterial,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& SourceFloor,
		const float CeilingOffset,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags,
		const bool bOpenDropSkylight)
	{
		T66TowerMapTerrain::FFloor CeilingFloor = SourceFloor;
		CeilingFloor.SurfaceZ = SourceFloor.SurfaceZ + CeilingOffset;
		CeilingFloor.Center.Z = CeilingFloor.SurfaceZ;
		CeilingFloor.PolygonApothem = FMath::Max(1200.0f, SourceFloor.PolygonApothem - 900.0f);
		CeilingFloor.BoundsHalfExtent = CeilingFloor.PolygonApothem;
		CeilingFloor.WalkableHalfExtent = CeilingFloor.BoundsHalfExtent;
		CeilingFloor.bHasDropHole = bOpenDropSkylight;
		if (bOpenDropSkylight)
		{
			CeilingFloor.HoleCenter = FVector(Layout.SpawnSurfaceLocation.X, Layout.SpawnSurfaceLocation.Y, CeilingFloor.SurfaceZ);
			CeilingFloor.HoleHalfExtent = FVector2D(Layout.PlacementCellSize * 0.85f, Layout.PlacementCellSize * 0.85f);
		}
		else
		{
			CeilingFloor.HoleCenter = CeilingFloor.Center;
			CeilingFloor.HoleHalfExtent = FVector2D::ZeroVector;
		}

		T66SpawnPolygonFloor(
			World,
			CubeMesh,
			RoofMaterial,
			Layout,
			CeilingFloor,
			SpawnParams,
			Tags);
	}

	static void T66SpawnMazeWalls(
		UWorld* World,
		UStaticMesh* CubeMesh,
		UMaterialInterface* WallMaterial,
		const TArray<UStaticMesh*>& TreeWallMeshes,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const FActorSpawnParameters& SpawnParams)
	{
		if (!World || Floor.MazeWallBoxes.Num() <= 0)
		{
			return;
		}

		const float CeilingOffset = T66GetTowerCeilingOffset(Layout, Floor);
		const float WallHeight = FMath::Max(600.0f, CeilingOffset - Layout.FloorThickness);
		const float WallHalfHeight = WallHeight * 0.5f;
		const float WallCenterZ = Floor.SurfaceZ + WallHalfHeight;
		const TArray<FName> WallTags = {
			T66TraversalBarrierTag,
			FName(*FString::Printf(TEXT("T66_Floor_Tower_%02d"), Floor.FloorNumber)),
			FName(*FString::Printf(TEXT("T66_Floor_Tower_Maze_%02d"), Floor.FloorNumber))
		};

		for (const FBox2D& WallBox : Floor.MazeWallBoxes)
		{
			const FVector2D WallCenter = (WallBox.Min + WallBox.Max) * 0.5f;
			const FVector2D WallHalfExtents = (WallBox.Max - WallBox.Min) * 0.5f;
			if (WallHalfExtents.X <= 5.0f || WallHalfExtents.Y <= 5.0f)
			{
				continue;
			}

			if (TreeWallMeshes.Num() > 0)
			{
				T66SpawnTreeWallBox(
					World,
					TreeWallMeshes,
					WallBox,
					Floor.SurfaceZ,
					WallHeight,
					SpawnParams,
					WallTags,
					Layout.Preset.Seed + (Floor.FloorNumber * 913) + static_cast<int32>(WallCenter.X + WallCenter.Y));
			}
			else if (CubeMesh)
			{
				T66SpawnStaticMeshActor(
					World,
					CubeMesh,
					WallMaterial,
					FVector(WallCenter.X, WallCenter.Y, WallCenterZ),
					FRotator::ZeroRotator,
					FVector(WallHalfExtents.X, WallHalfExtents.Y, WallHalfHeight),
					SpawnParams,
					true,
					WallTags);
			}
		}
	}

	static void T66SpawnPropActors(
		UWorld* World,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const FActorSpawnParameters& SpawnParams)
	{
		if (!World || !Floor.bGameplayFloor)
		{
			return;
		}

		static const TSoftObjectPtr<UStaticMesh> RockMeshPath(FSoftObjectPath(TEXT("/Game/World/Props/Rocks.Rocks")));
		UStaticMesh* RockMesh = RockMeshPath.LoadSynchronous();

		TArray<UStaticMesh*> CandidateMeshes;
		const int32 StageNumber = T66GetTowerStageNumber(World);
		const ET66TowerStageTheme Theme = T66ResolveTowerStageTheme(World);
		if (Theme == ET66TowerStageTheme::Forest)
		{
			return;
		}

		if (StageNumber == 1)
		{
			if (RockMesh) CandidateMeshes.Add(RockMesh);
		}
		else
		{
			TArray<UStaticMesh*> TreeMeshes;
			T66LoadTowerTreeMeshes(TreeMeshes);
			for (UStaticMesh* Mesh : TreeMeshes)
			{
				if (Mesh)
				{
					CandidateMeshes.Add(Mesh);
				}
			}
			if (RockMesh) CandidateMeshes.Add(RockMesh);
		}
		if (CandidateMeshes.Num() <= 0)
		{
			return;
		}

		FRandomStream Rng(Layout.Preset.Seed + Floor.FloorNumber * 911);
		static constexpr int32 GameplayFloorPropCount = 42;
		for (int32 PropIndex = 0; PropIndex < GameplayFloorPropCount; ++PropIndex)
		{
			FVector SpawnLocation = FVector::ZeroVector;
			bool bPlaced = false;
			for (int32 Attempt = 0; Attempt < 36; ++Attempt)
			{
				if (!T66TowerMapTerrain::TryGetRandomSurfaceLocationOnFloor(
					World,
					Layout,
					Floor.FloorNumber,
					Rng,
					SpawnLocation,
					750.0f,
					1150.0f))
				{
					continue;
				}

				bPlaced = true;
				break;
			}

			if (!bPlaced)
			{
				continue;
			}

			UStaticMesh* Mesh = CandidateMeshes[Rng.RandRange(0, CandidateMeshes.Num() - 1)];
			AStaticMeshActor* PropActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLocation, FRotator(0.0f, Rng.FRandRange(0.0f, 360.0f), 0.0f), SpawnParams);
			if (!PropActor)
			{
				continue;
			}

			if (UStaticMeshComponent* MeshComponent = PropActor->GetStaticMeshComponent())
			{
				MeshComponent->SetMobility(EComponentMobility::Movable);
				MeshComponent->SetStaticMesh(Mesh);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				MeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
				MeshComponent->SetGenerateOverlapEvents(false);
				MeshComponent->SetMobility(EComponentMobility::Static);
				const float Scale = (Mesh == RockMesh)
					? Rng.FRandRange(0.75f, 1.10f)
					: Rng.FRandRange(0.70f, 1.20f);
				PropActor->SetActorScale3D(FVector(Scale));
				FT66VisualUtil::EnsureUnlitMaterials(MeshComponent, World);
			}

			PropActor->Tags.AddUnique(T66MainMapTerrainVisualTag);
			PropActor->Tags.AddUnique(T66MainMapTerrainMaterialsReadyTag);
			PropActor->Tags.AddUnique(FName(*FString::Printf(TEXT("T66_Floor_Tower_Deco_%02d"), Floor.FloorNumber)));
		}
	}
}

namespace T66TowerMapTerrain
{
	bool BuildLayout(const FT66MapPreset& Preset, FLayout& OutLayout)
	{
		OutLayout = FLayout{};
		OutLayout.Preset = Preset;
		OutLayout.PlacementCellSize = 1300.0f;
		OutLayout.FloorThickness = 280.0f;
		OutLayout.FloorSpacing = 2000.0f;
		OutLayout.WallThickness = 280.0f;
		OutLayout.ShellRadius = 20000.0f;
		OutLayout.StartFloorNumber = 1;
		OutLayout.FirstGameplayFloorNumber = 2;
		OutLayout.LastGameplayFloorNumber = 5;
		OutLayout.BossFloorNumber = 6;

		const float TopFloorZ = Preset.BaselineZ + 1600.0f;
		const float FloorSpacing = OutLayout.FloorSpacing;
		const FVector2D HoleHalfExtent(OutLayout.PlacementCellSize * 0.5f, OutLayout.PlacementCellSize * 0.5f);
		const float FloorBottomZ = TopFloorZ - (5.0f * FloorSpacing) - OutLayout.FloorThickness;
		FRandomStream HoleRng(Preset.Seed * 977 + 311);

		OutLayout.TraceStartZ = TopFloorZ + 12000.0f;
		OutLayout.TraceEndZ = FloorBottomZ - 12000.0f;

		for (int32 FloorIndex = 0; FloorIndex < 6; ++FloorIndex)
		{
			FFloor& Floor = OutLayout.Floors.AddDefaulted_GetRef();
			Floor.FloorNumber = FloorIndex + 1;
			Floor.bGameplayFloor = Floor.FloorNumber >= OutLayout.FirstGameplayFloorNumber
				&& Floor.FloorNumber <= OutLayout.LastGameplayFloorNumber;
			Floor.bHasDropHole = Floor.FloorNumber < OutLayout.BossFloorNumber;
			Floor.Center = FVector(0.0f, 0.0f, TopFloorZ - (static_cast<float>(FloorIndex) * FloorSpacing));
			Floor.SurfaceZ = Floor.Center.Z;
			Floor.PolygonApothem = OutLayout.ShellRadius - (OutLayout.WallThickness * 0.5f) + 20.0f;
			Floor.BoundsHalfExtent = Floor.PolygonApothem;
			Floor.WalkableHalfExtent = Floor.PolygonApothem - ((Floor.FloorNumber == OutLayout.BossFloorNumber) ? 1600.0f : 1300.0f);
			Floor.FloorTag =
				(Floor.FloorNumber == OutLayout.StartFloorNumber) ? T66FloorStartTag :
				(Floor.FloorNumber == OutLayout.BossFloorNumber) ? T66FloorBossTag :
				T66FloorMainTag;

			if (Floor.bHasDropHole)
			{
				FVector HoleOffset = FVector::ZeroVector;
				if (Floor.FloorNumber == OutLayout.StartFloorNumber)
				{
					HoleOffset = FVector(0.0f, -(OutLayout.PlacementCellSize * 0.9f), 0.0f);
				}
				else
				{
					HoleOffset = T66BuildSquareHoleOffset(
						HoleRng,
						Floor.WalkableHalfExtent,
						Floor.BoundsHalfExtent,
						HoleHalfExtent,
						OutLayout.PlacementCellSize);
				}
				Floor.HoleCenter = Floor.Center + HoleOffset;
				Floor.HoleCenter.Z = Floor.SurfaceZ;
				Floor.HoleHalfExtent = HoleHalfExtent;
			}
			else
			{
				Floor.HoleCenter = Floor.Center;
				Floor.HoleCenter.Z = Floor.SurfaceZ;
				Floor.HoleHalfExtent = FVector2D::ZeroVector;
			}
		}

		const FFloor* StartFloor = nullptr;
		const FFloor* PreBossFloor = nullptr;
		const FFloor* BossFloor = nullptr;
		if (!T66TryGetFloor(OutLayout, OutLayout.StartFloorNumber, StartFloor)
			|| !T66TryGetFloor(OutLayout, OutLayout.LastGameplayFloorNumber, PreBossFloor)
			|| !T66TryGetFloor(OutLayout, OutLayout.BossFloorNumber, BossFloor))
		{
			return false;
		}

		const FVector StartInwardDirection(0.0f, 1.0f, 0.0f);
		const FVector BossHoleDirection = (PreBossFloor->HoleCenter - PreBossFloor->Center).GetSafeNormal2D();
		if (BossHoleDirection.IsNearlyZero())
		{
			return false;
		}

		const float StartAreaOffset = OutLayout.PlacementCellSize * 1.65f;
		const float StartAnchorOffset = OutLayout.PlacementCellSize * 0.45f;
		const float StartPathOffset = OutLayout.PlacementCellSize * 1.05f;
		OutLayout.StartAreaCenterSurfaceLocation = StartFloor->Center + (StartInwardDirection * StartAreaOffset);
		OutLayout.StartAnchorSurfaceLocation = StartFloor->Center + (StartInwardDirection * StartAnchorOffset);
		OutLayout.StartPathSurfaceLocation = StartFloor->Center + (StartInwardDirection * StartPathOffset);
		OutLayout.SpawnSurfaceLocation = OutLayout.StartAreaCenterSurfaceLocation;
		T66TryComputeStartPlacementLocation(
			OutLayout,
			T66TowerStartSafeSpawnSideCells,
			T66TowerStartSafeSpawnInwardCells,
			OutLayout.SpawnSurfaceLocation);

		FRandomStream MazeRng(Preset.Seed * 1297 + 733);
		for (FFloor& Floor : OutLayout.Floors)
		{
			T66BuildFloorMazeWalls(OutLayout, Floor, MazeRng);
		}

		OutLayout.BossAreaCenterSurfaceLocation = BossFloor->Center;
		OutLayout.BossSpawnSurfaceLocation = BossFloor->Center;
		OutLayout.BossAnchorSurfaceLocation = PreBossFloor->HoleCenter - (BossHoleDirection * 950.0f);
		OutLayout.BossBeaconSurfaceLocation = BossFloor->Center + (BossHoleDirection * 600.0f);

		OutLayout.RescueAnchorLocations.Reset();
		OutLayout.RescueAnchorLocations.Reserve(OutLayout.Floors.Num() * 3);
		OutLayout.RescueAnchorLocations.Add(OutLayout.SpawnSurfaceLocation);
		OutLayout.RescueAnchorLocations.Add(OutLayout.StartAnchorSurfaceLocation);
		OutLayout.RescueAnchorLocations.Add(OutLayout.BossAnchorSurfaceLocation);
		OutLayout.RescueAnchorLocations.Add(OutLayout.BossAreaCenterSurfaceLocation);
		for (const FFloor& Floor : OutLayout.Floors)
		{
			OutLayout.RescueAnchorLocations.Add(Floor.Center);
			OutLayout.RescueAnchorLocations.Add(Floor.Center + FVector(Floor.WalkableHalfExtent * 0.35f, 0.0f, 0.0f));
			OutLayout.RescueAnchorLocations.Add(Floor.Center + FVector(0.0f, Floor.WalkableHalfExtent * 0.35f, 0.0f));
		}

		return true;
	}

	FVector GetPreferredSpawnLocation(const FLayout& Layout, float HeightOffset)
	{
		return Layout.SpawnSurfaceLocation + FVector(0.0f, 0.0f, HeightOffset);
	}

	int32 FindFloorIndexForLocation(const FLayout& Layout, const FVector& Location, float VerticalTolerance)
	{
		int32 BestFloorNumber = INDEX_NONE;
		float BestAbsZ = TNumericLimits<float>::Max();
		for (const FFloor& Floor : Layout.Floors)
		{
			if (!T66IsLocationInsideFloorBounds(Floor, Location, 1800.0f))
			{
				continue;
			}

			const float AbsZ = FMath::Abs(Location.Z - Floor.SurfaceZ);
			if (AbsZ < BestAbsZ)
			{
				BestAbsZ = AbsZ;
				BestFloorNumber = Floor.FloorNumber;
			}
		}

		if (BestFloorNumber != INDEX_NONE && BestAbsZ <= VerticalTolerance)
		{
			return BestFloorNumber;
		}

		return INDEX_NONE;
	}

	bool TryGetFloorBounds(const FLayout& Layout, int32 FloorNumber, FVector2D& OutCenter, FVector2D& OutHalfExtents)
	{
		const FFloor* Floor = nullptr;
		if (!T66TryGetFloor(Layout, FloorNumber, Floor))
		{
			return false;
		}

		OutCenter = FVector2D(Floor->Center.X, Floor->Center.Y);
		OutHalfExtents = FVector2D(Floor->BoundsHalfExtent, Floor->BoundsHalfExtent);
		return true;
	}

	bool TryGetFloorPolygon(const FLayout& Layout, int32 FloorNumber, TArray<FVector2D>& OutVertices)
	{
		const FFloor* Floor = nullptr;
		if (!T66TryGetFloor(Layout, FloorNumber, Floor))
		{
			OutVertices.Reset();
			return false;
		}

		T66BuildFloorPolygonVertices(*Floor, OutVertices);
		return OutVertices.Num() >= 3;
	}

	bool TryGetPolygonBandXRange(const TArray<FVector2D>& PolygonVertices, float BandMinY, float BandMaxY, float& OutMinX, float& OutMaxX)
	{
		return T66TryGetPolygonBandXRange(PolygonVertices, BandMinY, BandMaxY, OutMinX, OutMaxX);
	}

	bool TryGetFloorHoleLocation(const FLayout& Layout, int32 FloorNumber, FVector& OutLocation)
	{
		const FFloor* Floor = nullptr;
		if (!T66TryGetFloor(Layout, FloorNumber, Floor) || !Floor->bHasDropHole)
		{
			return false;
		}

		OutLocation = Floor->HoleCenter;
		return true;
	}

	bool TryGetStartPlacementLocation(UWorld* World, const FLayout& Layout, float SideCells, float InwardCells, FVector& OutLocation)
	{
		FVector DesiredLocation = FVector::ZeroVector;
		if (!T66TryComputeStartPlacementLocation(Layout, SideCells, InwardCells, DesiredLocation))
		{
			return false;
		}

		OutLocation = DesiredLocation;
		FVector SnappedLocation = FVector::ZeroVector;
		if (T66TraceDownToSurface(World, Layout, DesiredLocation, SnappedLocation))
		{
			OutLocation = SnappedLocation;
		}

		return true;
	}

	bool TryGetRandomGameplaySurfaceLocation(UWorld* World, const FLayout& Layout, FRandomStream& Rng, FVector& OutLocation)
	{
		TArray<const FFloor*, TInlineAllocator<4>> GameplayFloors;
		for (const FFloor& Floor : Layout.Floors)
		{
			if (Floor.bGameplayFloor)
			{
				GameplayFloors.Add(&Floor);
			}
		}

		if (GameplayFloors.Num() <= 0)
		{
			return false;
		}

		for (int32 Attempt = 0; Attempt < 36; ++Attempt)
		{
			const FFloor& Floor = *GameplayFloors[Rng.RandRange(0, GameplayFloors.Num() - 1)];
			const float CandidateHalfExtent = Floor.BoundsHalfExtent - 900.0f;
			const FVector Candidate(
				Floor.Center.X + Rng.FRandRange(-CandidateHalfExtent, CandidateHalfExtent),
				Floor.Center.Y + Rng.FRandRange(-CandidateHalfExtent, CandidateHalfExtent),
				Floor.SurfaceZ);
			if (!T66IsWalkableTowerLocation(Floor, Candidate, 800.0f, 1000.0f, 700.0f))
			{
				continue;
			}

			FVector SnappedLocation = FVector::ZeroVector;
			if (T66TraceDownToSurface(World, Layout, Candidate, SnappedLocation))
			{
				OutLocation = SnappedLocation;
				return true;
			}
		}

		return false;
	}

	bool TryGetRandomSurfaceLocationOnFloor(
		UWorld* World,
		const FLayout& Layout,
		int32 FloorNumber,
		FRandomStream& Rng,
		FVector& OutLocation,
		float EdgePadding,
		float HolePadding)
	{
		const FFloor* Floor = nullptr;
		if (!T66TryGetFloor(Layout, FloorNumber, Floor))
		{
			return false;
		}

		const float EffectiveEdgePadding = FMath::Max(0.0f, EdgePadding);
		const float EffectiveHolePadding = FMath::Max(0.0f, HolePadding);
		const float CandidateHalfExtent = Floor->BoundsHalfExtent - EffectiveEdgePadding;
		if (CandidateHalfExtent <= 100.0f)
		{
			return false;
		}

		for (int32 Attempt = 0; Attempt < 36; ++Attempt)
		{
			const FVector Candidate(
				Floor->Center.X + Rng.FRandRange(-CandidateHalfExtent, CandidateHalfExtent),
				Floor->Center.Y + Rng.FRandRange(-CandidateHalfExtent, CandidateHalfExtent),
				Floor->SurfaceZ);
			if (!T66IsWalkableTowerLocation(*Floor, Candidate, EffectiveEdgePadding, EffectiveHolePadding, EffectiveEdgePadding))
			{
				continue;
			}

			FVector SnappedLocation = FVector::ZeroVector;
			if (T66TraceDownToSurface(World, Layout, Candidate, SnappedLocation))
			{
				OutLocation = SnappedLocation;
				return true;
			}
		}

		return false;
	}

	bool TryGetWallSpawnLocation(UWorld* World, const FLayout& Layout, const FVector& PlayerLocation, float MinDistance, float MaxDistance, FRandomStream& Rng, FVector& OutLocation)
	{
		FVector WallNormal = FVector::ZeroVector;
		return TryGetWallSpawnLocation(World, Layout, PlayerLocation, MinDistance, MaxDistance, Rng, OutLocation, WallNormal);
	}

	bool TryGetWallSpawnLocation(
		UWorld* World,
		const FLayout& Layout,
		const FVector& PlayerLocation,
		float MinDistance,
		float MaxDistance,
		FRandomStream& Rng,
		FVector& OutLocation,
		FVector& OutWallNormal)
	{
		const int32 FloorNumber = FindFloorIndexForLocation(Layout, PlayerLocation);
		const FFloor* Floor = nullptr;
		if (FloorNumber == INDEX_NONE || !T66TryGetFloor(Layout, FloorNumber, Floor))
		{
			return false;
		}

		const float ShellInset = FMath::Max(240.0f, Layout.WallThickness + 120.0f);
		const float WallSpawnHalfExtent = FMath::Max(Floor->BoundsHalfExtent - ShellInset, 0.0f);
		const float SideSweep = FMath::Max(700.0f, WallSpawnHalfExtent - 1200.0f);
		for (int32 Attempt = 0; Attempt < 32; ++Attempt)
		{
			FVector Candidate = Floor->Center;
			FVector CandidateWallNormal = FVector::ZeroVector;
			switch (Rng.RandRange(0, 3))
			{
			case 0:
				Candidate = Floor->Center + FVector(WallSpawnHalfExtent, Rng.FRandRange(-SideSweep, SideSweep), 0.0f);
				CandidateWallNormal = FVector(-1.0f, 0.0f, 0.0f);
				break;
			case 1:
				Candidate = Floor->Center + FVector(-WallSpawnHalfExtent, Rng.FRandRange(-SideSweep, SideSweep), 0.0f);
				CandidateWallNormal = FVector(1.0f, 0.0f, 0.0f);
				break;
			case 2:
				Candidate = Floor->Center + FVector(Rng.FRandRange(-SideSweep, SideSweep), WallSpawnHalfExtent, 0.0f);
				CandidateWallNormal = FVector(0.0f, -1.0f, 0.0f);
				break;
			default:
				Candidate = Floor->Center + FVector(Rng.FRandRange(-SideSweep, SideSweep), -WallSpawnHalfExtent, 0.0f);
				CandidateWallNormal = FVector(0.0f, 1.0f, 0.0f);
				break;
			}

			const float Dist2D = FVector::Dist2D(Candidate, PlayerLocation);
			if (Dist2D < MinDistance || Dist2D > (MaxDistance + 1200.0f))
			{
				continue;
			}

			if (!T66IsWalkableTowerLocation(*Floor, Candidate, 150.0f, 400.0f, 220.0f))
			{
				continue;
			}

			FVector SnappedLocation = FVector::ZeroVector;
			if (!T66TraceDownToSurface(World, Layout, Candidate, SnappedLocation))
			{
				continue;
			}

			OutLocation = SnappedLocation;
			OutWallNormal = CandidateWallNormal;
			return true;
		}

		return false;
	}

	bool Spawn(UWorld* World, const FLayout& Layout, ET66Difficulty Difficulty, const FActorSpawnParameters& SpawnParams, bool& bOutCollisionReady)
	{
		bOutCollisionReady = false;
		if (!World || Layout.Floors.Num() <= 0)
		{
			return false;
		}

		UStaticMesh* CubeMesh = FT66VisualUtil::GetBasicShapeCube();
		if (!CubeMesh)
		{
			return false;
		}

		const int32 StageNumber = T66GetTowerStageNumber(World);
		const ET66TowerStageTheme StageTheme = T66ResolveTowerStageTheme(World);
		TArray<UStaticMesh*> TreeWallMeshes;

		UMaterialInterface* FloorMaterial = nullptr;
		UMaterialInterface* WallMaterial = nullptr;
		UMaterialInterface* RoofMaterial = nullptr;
		if (StageTheme == ET66TowerStageTheme::Forest)
		{
			FloorMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Terrain/TowerForest/MI_TowerForestGround.MI_TowerForestGround"));
			RoofMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Terrain/TowerForest/MI_TowerForestRoof.MI_TowerForestRoof"));
			T66LoadTowerTreeMeshes(TreeWallMeshes);
		}
		else
		{
			FloorMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonGround.MI_TowerDungeonGround"));
			WallMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonWall.MI_TowerDungeonWall"));
			RoofMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof.MI_TowerDungeonRoof"));
		}
		if (!FloorMaterial)
		{
			FloorMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock"));
		}
		if (!FloorMaterial)
		{
			FloorMaterial = FT66TerrainThemeAssets::ResolveDifficultyGroundMaterial(World, Difficulty);
		}
		if (!WallMaterial)
		{
			WallMaterial = FloorMaterial;
		}
		if (!RoofMaterial)
		{
			RoofMaterial = WallMaterial;
		}

		const float RoofHeightOffset = T66GetTowerStartRoofOffset(Layout);
		const float TopWallZ = Layout.Floors[0].SurfaceZ + RoofHeightOffset;
		const float BottomWallZ = Layout.Floors.Last().SurfaceZ - Layout.FloorThickness - 160.0f;
		const float WallHalfHeight = FMath::Max(2000.0f, (TopWallZ - BottomWallZ) * 0.5f);
		const float WallCenterZ = (TopWallZ + BottomWallZ) * 0.5f;
		const float WallHalfDepth = Layout.WallThickness * 0.5f;
		const float WallHalfSpan = Layout.ShellRadius + WallHalfDepth;
		const TArray<FName> ShellTags = { T66TraversalBarrierTag, FName(TEXT("T66_Floor_Tower_Shell")) };
		if (TreeWallMeshes.Num() > 0)
		{
			T66SpawnTreeWallBox(
				World,
				TreeWallMeshes,
				FBox2D(FVector2D(Layout.ShellRadius - WallHalfDepth, -WallHalfSpan), FVector2D(Layout.ShellRadius + WallHalfDepth, WallHalfSpan)),
				BottomWallZ,
				WallHalfHeight * 2.0f,
				SpawnParams,
				ShellTags,
				Layout.Preset.Seed + 4101);
			T66SpawnTreeWallBox(
				World,
				TreeWallMeshes,
				FBox2D(FVector2D(-Layout.ShellRadius - WallHalfDepth, -WallHalfSpan), FVector2D(-Layout.ShellRadius + WallHalfDepth, WallHalfSpan)),
				BottomWallZ,
				WallHalfHeight * 2.0f,
				SpawnParams,
				ShellTags,
				Layout.Preset.Seed + 4102);
			T66SpawnTreeWallBox(
				World,
				TreeWallMeshes,
				FBox2D(FVector2D(-WallHalfSpan, Layout.ShellRadius - WallHalfDepth), FVector2D(WallHalfSpan, Layout.ShellRadius + WallHalfDepth)),
				BottomWallZ,
				WallHalfHeight * 2.0f,
				SpawnParams,
				ShellTags,
				Layout.Preset.Seed + 4103);
			T66SpawnTreeWallBox(
				World,
				TreeWallMeshes,
				FBox2D(FVector2D(-WallHalfSpan, -Layout.ShellRadius - WallHalfDepth), FVector2D(WallHalfSpan, -Layout.ShellRadius + WallHalfDepth)),
				BottomWallZ,
				WallHalfHeight * 2.0f,
				SpawnParams,
				ShellTags,
				Layout.Preset.Seed + 4104);
		}
		else
		{
			T66SpawnStaticMeshActor(
				World,
				CubeMesh,
				WallMaterial,
				FVector(Layout.ShellRadius, 0.0f, WallCenterZ),
				FRotator::ZeroRotator,
				FVector(WallHalfDepth, WallHalfSpan, WallHalfHeight),
				SpawnParams,
				true,
				ShellTags);
			T66SpawnStaticMeshActor(
				World,
				CubeMesh,
				WallMaterial,
				FVector(-Layout.ShellRadius, 0.0f, WallCenterZ),
				FRotator::ZeroRotator,
				FVector(WallHalfDepth, WallHalfSpan, WallHalfHeight),
				SpawnParams,
				true,
				ShellTags);
			T66SpawnStaticMeshActor(
				World,
				CubeMesh,
				WallMaterial,
				FVector(0.0f, Layout.ShellRadius, WallCenterZ),
				FRotator::ZeroRotator,
				FVector(WallHalfSpan, WallHalfDepth, WallHalfHeight),
				SpawnParams,
				true,
				ShellTags);
			T66SpawnStaticMeshActor(
				World,
				CubeMesh,
				WallMaterial,
				FVector(0.0f, -Layout.ShellRadius, WallCenterZ),
				FRotator::ZeroRotator,
				FVector(WallHalfSpan, WallHalfDepth, WallHalfHeight),
				SpawnParams,
				true,
				ShellTags);
		}

		for (const FFloor& Floor : Layout.Floors)
		{
			const TArray<FName> FloorTags = {
				Floor.FloorTag,
				FName(*FString::Printf(TEXT("T66_Floor_Tower_%02d"), Floor.FloorNumber))
			};

			T66SpawnPolygonFloor(World, CubeMesh, FloorMaterial, Layout, Floor, SpawnParams, FloorTags);
			T66SpawnMazeWalls(World, CubeMesh, WallMaterial, TreeWallMeshes, Layout, Floor, SpawnParams);

		}

		T66SpawnTowerCeiling(
			World,
			CubeMesh,
			RoofMaterial,
			Layout,
			Layout.Floors[0],
			RoofHeightOffset,
			SpawnParams,
			{ T66TraversalBarrierTag, T66TowerCeilingTag, FName(TEXT("T66_Floor_Tower_Roof")) },
			T66ShouldOpenTowerStartRoofForDrop(World));

		for (const FFloor& Floor : Layout.Floors)
		{
			if (Floor.FloorNumber == Layout.BossFloorNumber)
			{
				continue;
			}

			if (StageNumber >= 1)
			{
				T66SpawnPropActors(World, Layout, Floor, SpawnParams);
			}
		}

		bOutCollisionReady = true;
		return true;
	}
}
