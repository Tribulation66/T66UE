// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TowerMapTerrain.h"

#include "Core/T66GameplayLayout.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66TowerThemeVisuals.h"
#include "Gameplay/T66VisualUtil.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "HAL/IConsoleManager.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

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
	static constexpr float T66TowerRoofSkinThickness = 12.0f;
	static constexpr float T66TowerStartFloorHeadroom = 2000.0f;
	static constexpr float T66TowerStandardFloorHeadroom = 1200.0f;
	static constexpr float T66TowerMazeWallHalfThicknessScale = 0.09f;
	static constexpr float T66TowerStartRoomSquareSize = 7000.0f;
	static constexpr int32 T66TowerGridDefaultColumns = 6;
	static constexpr int32 T66TowerGridDefaultRows = 6;
	static constexpr float T66TowerGridDefaultCellSize = 6500.0f;
	static constexpr float T66TowerGridDefaultDoorWidth = 2600.0f;
	static constexpr float T66TowerGridBranchChance = 0.35f;
	static constexpr int32 T66TowerGridMaxBranchCells = 3;

	static TAutoConsoleVariable<int32> CVarT66TowerMazeMode(
		TEXT("T66.Tower.MazeMode"),
		1,
		TEXT("0 = legacy lane maze, 1 = grid-graph maze."),
		ECVF_Default);

	using ET66TowerGridCellSemantic = T66TowerMapTerrain::ET66TowerGridCellSemantic;
	using ET66TowerGridTemplate = T66TowerMapTerrain::ET66TowerGridTemplate;

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

	static void T66ConfigureTowerCollisionResponses(UStaticMeshComponent* MeshComponent, const bool bIgnoreCameraChannel = false)
	{
		if (!MeshComponent)
		{
			return;
		}

		MeshComponent->SetCollisionResponseToChannel(ECC_Camera, bIgnoreCameraChannel ? ECR_Ignore : ECR_Block);
	}

	static void T66OptimizeTowerMeshComponent(UStaticMeshComponent* MeshComponent)
	{
		if (!MeshComponent)
		{
			return;
		}

		// Runtime tower geometry is fully unlit and only serves traversal/readability.
		MeshComponent->SetCastShadow(false);
		MeshComponent->bCastDynamicShadow = false;
		MeshComponent->bCastStaticShadow = false;
		MeshComponent->bAffectDistanceFieldLighting = false;
		MeshComponent->bAffectDynamicIndirectLighting = false;
		MeshComponent->bReceivesDecals = false;
		MeshComponent->SetCanEverAffectNavigation(false);
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
		const TArray<FName>& ExtraTags,
		const bool bIgnoreCameraChannel = false)
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
			T66ConfigureTowerCollisionResponses(MeshComponent, bIgnoreCameraChannel);
			T66OptimizeTowerMeshComponent(MeshComponent);
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

	static AStaticMeshActor* T66SpawnCollisionBlockerActor(
		UWorld* World,
		UStaticMesh* Mesh,
		const FVector& Location,
		const FRotator& Rotation,
		const FVector& DesiredHalfExtents,
		const FActorSpawnParameters& SpawnParams,
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
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			MeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
			T66ConfigureTowerCollisionResponses(MeshComponent, true);
			T66OptimizeTowerMeshComponent(MeshComponent);
			MeshComponent->SetVisibility(false, true);
			MeshComponent->SetHiddenInGame(true, true);
			MeshComponent->SetMobility(EComponentMobility::Static);
		}

		Actor->SetActorHiddenInGame(true);
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
		const TArray<FName>& ExtraTags,
		const bool bIgnoreCameraChannel = false)
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
			T66ConfigureTowerCollisionResponses(MeshComponent, bIgnoreCameraChannel);
			T66OptimizeTowerMeshComponent(MeshComponent);
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

	static void T66BuildFloorMazeWalls_Legacy(
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
		const float HalfThickness = FMath::Max(60.0f, Layout.PlacementCellSize * T66TowerMazeWallHalfThicknessScale);
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

	static T66TowerMapTerrain::ET66TowerMazeMode T66GetConfiguredTowerMazeMode()
	{
		return CVarT66TowerMazeMode.GetValueOnAnyThread() <= 0
			? T66TowerMapTerrain::ET66TowerMazeMode::LegacyLanes
			: T66TowerMapTerrain::ET66TowerMazeMode::GridGraph;
	}

	static void T66ResetFloorMazeMetadata(T66TowerMapTerrain::FFloor& Floor)
	{
		Floor.ArrivalCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		Floor.ExitCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		Floor.GridCells.Reset();
		Floor.MainPathCells.Reset();
		Floor.OptionalCells.Reset();
		Floor.MazeWallBoxes.Reset();
		Floor.TrapEligibleWallBoxes.Reset();
		Floor.CachedWalkableSpawnSlots.Reset();
		Floor.CachedMainPathSpawnSlots.Reset();
		Floor.CachedOptionalSpawnSlots.Reset();
		Floor.CachedContentSpawnSlots.Reset();
	}

	static int32 T66GetGridCellIndex(const T66TowerMapTerrain::FLayout& Layout, const FIntPoint& Coord)
	{
		return (Coord.Y * Layout.GridColumns) + Coord.X;
	}

	static FIntPoint T66GetGridCoordFromIndex(const T66TowerMapTerrain::FLayout& Layout, const int32 Index)
	{
		if (Layout.GridColumns <= 0)
		{
			return FIntPoint(INDEX_NONE, INDEX_NONE);
		}

		return FIntPoint(Index % Layout.GridColumns, Index / Layout.GridColumns);
	}

	static bool T66IsValidGridCoord(const T66TowerMapTerrain::FLayout& Layout, const FIntPoint& Coord)
	{
		return Coord.X >= 0
			&& Coord.Y >= 0
			&& Coord.X < Layout.GridColumns
			&& Coord.Y < Layout.GridRows;
	}

	static FVector2D T66GetGridMinCorner(const T66TowerMapTerrain::FLayout& Layout, const T66TowerMapTerrain::FFloor& Floor)
	{
		const float GridWidth = static_cast<float>(Layout.GridColumns) * Layout.GridCellSize;
		const float GridHeight = static_cast<float>(Layout.GridRows) * Layout.GridCellSize;
		return FVector2D(
			Floor.Center.X - (GridWidth * 0.5f),
			Floor.Center.Y - (GridHeight * 0.5f));
	}

	static FIntPoint T66ProjectPointToGridCell(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const FVector& Point)
	{
		const FVector2D GridMin = T66GetGridMinCorner(Layout, Floor);
		const FVector2D Local(Point.X - GridMin.X, Point.Y - GridMin.Y);
		const int32 CellX = FMath::Clamp(FMath::FloorToInt(Local.X / Layout.GridCellSize), 0, Layout.GridColumns - 1);
		const int32 CellY = FMath::Clamp(FMath::FloorToInt(Local.Y / Layout.GridCellSize), 0, Layout.GridRows - 1);
		return FIntPoint(CellX, CellY);
	}

	static int32 T66GridManhattanDistance(const FIntPoint& A, const FIntPoint& B)
	{
		return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
	}

	static bool T66TryGetGridNeighbor(
		const T66TowerMapTerrain::FLayout& Layout,
		const FIntPoint& Coord,
		const int32 DirectionIndex,
		FIntPoint& OutCoord)
	{
		static const FIntPoint Deltas[] =
		{
			FIntPoint(0, -1),
			FIntPoint(1, 0),
			FIntPoint(0, 1),
			FIntPoint(-1, 0),
		};

		if (DirectionIndex < 0 || DirectionIndex >= UE_ARRAY_COUNT(Deltas))
		{
			return false;
		}

		OutCoord = Coord + Deltas[DirectionIndex];
		return T66IsValidGridCoord(Layout, OutCoord);
	}

	static uint8 T66GetGridConnectionFlagForDelta(const FIntPoint& Delta)
	{
		if (Delta.X == 0 && Delta.Y < 0)
		{
			return T66TowerMapTerrain::GridNorth;
		}
		if (Delta.X > 0 && Delta.Y == 0)
		{
			return T66TowerMapTerrain::GridEast;
		}
		if (Delta.X == 0 && Delta.Y > 0)
		{
			return T66TowerMapTerrain::GridSouth;
		}
		if (Delta.X < 0 && Delta.Y == 0)
		{
			return T66TowerMapTerrain::GridWest;
		}

		return 0;
	}

	static uint8 T66GetOppositeGridConnectionFlag(const uint8 Flag)
	{
		switch (Flag)
		{
		case T66TowerMapTerrain::GridNorth:
			return T66TowerMapTerrain::GridSouth;
		case T66TowerMapTerrain::GridEast:
			return T66TowerMapTerrain::GridWest;
		case T66TowerMapTerrain::GridSouth:
			return T66TowerMapTerrain::GridNorth;
		case T66TowerMapTerrain::GridWest:
			return T66TowerMapTerrain::GridEast;
		default:
			return 0;
		}
	}

	static int32 T66CountGridConnections(const uint8 ConnectionMask)
	{
		int32 Count = 0;
		Count += (ConnectionMask & T66TowerMapTerrain::GridNorth) ? 1 : 0;
		Count += (ConnectionMask & T66TowerMapTerrain::GridEast) ? 1 : 0;
		Count += (ConnectionMask & T66TowerMapTerrain::GridSouth) ? 1 : 0;
		Count += (ConnectionMask & T66TowerMapTerrain::GridWest) ? 1 : 0;
		return Count;
	}

	static void T66ConnectGridCells(
		T66TowerMapTerrain::FFloor& Floor,
		const T66TowerMapTerrain::FLayout& Layout,
		const int32 FromIndex,
		const int32 ToIndex)
	{
		if (!Floor.GridCells.IsValidIndex(FromIndex) || !Floor.GridCells.IsValidIndex(ToIndex))
		{
			return;
		}

		const FIntPoint Delta = T66GetGridCoordFromIndex(Layout, ToIndex) - T66GetGridCoordFromIndex(Layout, FromIndex);
		const uint8 FromFlag = T66GetGridConnectionFlagForDelta(Delta);
		if (FromFlag == 0)
		{
			return;
		}

		Floor.GridCells[FromIndex].ConnectionMask |= FromFlag;
		Floor.GridCells[ToIndex].ConnectionMask |= T66GetOppositeGridConnectionFlag(FromFlag);
	}

	static bool T66TryBuildMainPathRecursive(
		const T66TowerMapTerrain::FLayout& Layout,
		const int32 CurrentIndex,
		const int32 GoalIndex,
		TArray<bool>& Visited,
		TArray<int32>& Path,
		FRandomStream& Rng)
	{
		if (CurrentIndex == GoalIndex)
		{
			return true;
		}

		struct FScoredNeighbor
		{
			int32 Index = INDEX_NONE;
			float Score = 0.0f;
		};

		TArray<FScoredNeighbor, TInlineAllocator<4>> Candidates;
		const FIntPoint CurrentCoord = T66GetGridCoordFromIndex(Layout, CurrentIndex);
		const FIntPoint GoalCoord = T66GetGridCoordFromIndex(Layout, GoalIndex);
		const int32 CurrentDistance = T66GridManhattanDistance(CurrentCoord, GoalCoord);

		for (int32 DirectionIndex = 0; DirectionIndex < 4; ++DirectionIndex)
		{
			FIntPoint NeighborCoord;
			if (!T66TryGetGridNeighbor(Layout, CurrentCoord, DirectionIndex, NeighborCoord))
			{
				continue;
			}

			const int32 NeighborIndex = T66GetGridCellIndex(Layout, NeighborCoord);
			if (!Visited.IsValidIndex(NeighborIndex) || Visited[NeighborIndex])
			{
				continue;
			}

			const int32 NeighborDistance = T66GridManhattanDistance(NeighborCoord, GoalCoord);
			float Score = (NeighborDistance < CurrentDistance) ? 3.0f : 1.0f;
			Score += Rng.FRandRange(0.0f, 0.75f);
			FScoredNeighbor Candidate;
			Candidate.Index = NeighborIndex;
			Candidate.Score = Score;
			Candidates.Add(Candidate);
		}

		Candidates.Sort([](const FScoredNeighbor& A, const FScoredNeighbor& B)
		{
			return A.Score > B.Score;
		});

		for (const FScoredNeighbor& Candidate : Candidates)
		{
			Visited[Candidate.Index] = true;
			Path.Add(Candidate.Index);
			if (T66TryBuildMainPathRecursive(Layout, Candidate.Index, GoalIndex, Visited, Path, Rng))
			{
				return true;
			}

			Path.Pop();
			Visited[Candidate.Index] = false;
		}

		return false;
	}

	static int32 T66ChooseMainPathWaypointIndex(
		const T66TowerMapTerrain::FLayout& Layout,
		const int32 StartIndex,
		const int32 GoalIndex,
		FRandomStream& Rng)
	{
		int32 BestIndex = INDEX_NONE;
		float BestScore = -1.0f;
		const int32 CellCount = Layout.GridColumns * Layout.GridRows;
		const FIntPoint StartCoord = T66GetGridCoordFromIndex(Layout, StartIndex);
		const FIntPoint GoalCoord = T66GetGridCoordFromIndex(Layout, GoalIndex);
		for (int32 Index = 0; Index < CellCount; ++Index)
		{
			if (Index == StartIndex || Index == GoalIndex)
			{
				continue;
			}

			const FIntPoint Coord = T66GetGridCoordFromIndex(Layout, Index);
			const int32 StartDistance = T66GridManhattanDistance(StartCoord, Coord);
			const int32 GoalDistance = T66GridManhattanDistance(GoalCoord, Coord);
			if (StartDistance < 2 || GoalDistance < 2)
			{
				continue;
			}

			const float Score = static_cast<float>(StartDistance + GoalDistance) + Rng.FRandRange(0.0f, 0.5f);
			if (Score > BestScore)
			{
				BestScore = Score;
				BestIndex = Index;
			}
		}

		return BestIndex;
	}

	static bool T66BuildMainPathIndices(
		const T66TowerMapTerrain::FLayout& Layout,
		const int32 StartIndex,
		const int32 GoalIndex,
		FRandomStream& Rng,
		TArray<int32>& OutPath)
	{
		const int32 CellCount = Layout.GridColumns * Layout.GridRows;
		TArray<bool> Visited;
		Visited.Init(false, CellCount);

		auto TrySolveSegment = [&](const int32 SegmentStart, const int32 SegmentGoal, TArray<bool>& InOutVisited, TArray<int32>& InOutPath)
		{
			return T66TryBuildMainPathRecursive(Layout, SegmentStart, SegmentGoal, InOutVisited, InOutPath, Rng);
		};

		const int32 DirectDistance = T66GridManhattanDistance(
			T66GetGridCoordFromIndex(Layout, StartIndex),
			T66GetGridCoordFromIndex(Layout, GoalIndex));
		if (StartIndex == GoalIndex || DirectDistance <= 1)
		{
			const int32 WaypointIndex = T66ChooseMainPathWaypointIndex(Layout, StartIndex, GoalIndex, Rng);
			if (WaypointIndex != INDEX_NONE)
			{
				OutPath.Reset();
				OutPath.Add(StartIndex);
				Visited.Init(false, CellCount);
				Visited[StartIndex] = true;
				if (TrySolveSegment(StartIndex, WaypointIndex, Visited, OutPath)
					&& TrySolveSegment(WaypointIndex, GoalIndex, Visited, OutPath))
				{
					return true;
				}
			}
		}

		OutPath.Reset();
		OutPath.Add(StartIndex);
		Visited.Init(false, CellCount);
		Visited[StartIndex] = true;
		return TrySolveSegment(StartIndex, GoalIndex, Visited, OutPath);
	}

	static bool T66TryBuildLoopBranchRecursive(
		const T66TowerMapTerrain::FLayout& Layout,
		const int32 StartIndex,
		const int32 CurrentIndex,
		const int32 ParentIndex,
		const int32 RemainingNewCells,
		const TArray<bool>& Occupied,
		FRandomStream& Rng,
		TArray<int32>& InOutBranchPath,
		int32& OutReconnectIndex)
	{
		TArray<int32, TInlineAllocator<4>> DirectionOrder;
		DirectionOrder.Add(0);
		DirectionOrder.Add(1);
		DirectionOrder.Add(2);
		DirectionOrder.Add(3);
		for (int32 Index = DirectionOrder.Num() - 1; Index > 0; --Index)
		{
			const int32 SwapIndex = Rng.RandRange(0, Index);
			DirectionOrder.Swap(Index, SwapIndex);
		}

		const FIntPoint CurrentCoord = T66GetGridCoordFromIndex(Layout, CurrentIndex);
		for (const int32 DirectionIndex : DirectionOrder)
		{
			FIntPoint NeighborCoord;
			if (!T66TryGetGridNeighbor(Layout, CurrentCoord, DirectionIndex, NeighborCoord))
			{
				continue;
			}

			const int32 NeighborIndex = T66GetGridCellIndex(Layout, NeighborCoord);
			if (NeighborIndex == ParentIndex)
			{
				continue;
			}

			if (Occupied.IsValidIndex(NeighborIndex) && Occupied[NeighborIndex])
			{
				if (NeighborIndex != StartIndex)
				{
					OutReconnectIndex = NeighborIndex;
					return true;
				}
				continue;
			}

			if (RemainingNewCells <= 0 || InOutBranchPath.Contains(NeighborIndex))
			{
				continue;
			}

			InOutBranchPath.Add(NeighborIndex);
			if (T66TryBuildLoopBranchRecursive(
				Layout,
				StartIndex,
				NeighborIndex,
				CurrentIndex,
				RemainingNewCells - 1,
				Occupied,
				Rng,
				InOutBranchPath,
				OutReconnectIndex))
			{
				return true;
			}

			InOutBranchPath.Pop();
		}

		return false;
	}

	static bool T66TryAddLoopBranch(
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor,
		TArray<bool>& Occupied,
		const int32 StartIndex,
		FRandomStream& Rng,
		int32 LoopId)
	{
		const FIntPoint StartCoord = T66GetGridCoordFromIndex(Layout, StartIndex);
		TArray<int32, TInlineAllocator<4>> DirectionOrder;
		DirectionOrder.Add(0);
		DirectionOrder.Add(1);
		DirectionOrder.Add(2);
		DirectionOrder.Add(3);
		for (int32 Index = DirectionOrder.Num() - 1; Index > 0; --Index)
		{
			const int32 SwapIndex = Rng.RandRange(0, Index);
			DirectionOrder.Swap(Index, SwapIndex);
		}

		for (const int32 DirectionIndex : DirectionOrder)
		{
			FIntPoint NeighborCoord;
			if (!T66TryGetGridNeighbor(Layout, StartCoord, DirectionIndex, NeighborCoord))
			{
				continue;
			}

			const int32 SeedIndex = T66GetGridCellIndex(Layout, NeighborCoord);
			if (!Occupied.IsValidIndex(SeedIndex) || Occupied[SeedIndex])
			{
				continue;
			}

			TArray<int32> BranchPath;
			BranchPath.Add(SeedIndex);
			int32 ReconnectIndex = INDEX_NONE;
			const int32 MaxNewCells = Rng.RandRange(1, T66TowerGridMaxBranchCells);
			if (!T66TryBuildLoopBranchRecursive(
				Layout,
				StartIndex,
				SeedIndex,
				StartIndex,
				MaxNewCells - 1,
				Occupied,
				Rng,
				BranchPath,
				ReconnectIndex))
			{
				continue;
			}

			T66ConnectGridCells(Floor, Layout, StartIndex, BranchPath[0]);
			for (int32 PathIndex = 0; PathIndex < BranchPath.Num(); ++PathIndex)
			{
				const int32 CellIndex = BranchPath[PathIndex];
				Occupied[CellIndex] = true;
				T66TowerMapTerrain::FGridCell& Cell = Floor.GridCells[CellIndex];
				Cell.Semantic = T66TowerMapTerrain::ET66TowerGridCellSemantic::OptionalLoop;
				Cell.LoopId = LoopId;
				Floor.OptionalCells.AddUnique(Cell.Coord);

				if (PathIndex > 0)
				{
					T66ConnectGridCells(Floor, Layout, BranchPath[PathIndex - 1], CellIndex);
				}
			}

			T66ConnectGridCells(Floor, Layout, BranchPath.Last(), ReconnectIndex);
			return true;
		}

		return false;
	}

	static void T66EmitWallRect(TArray<FBox2D>& OutBoxes, float MinX, float MinY, float MaxX, float MaxY)
	{
		if (MaxX <= MinX + KINDA_SMALL_NUMBER || MaxY <= MinY + KINDA_SMALL_NUMBER)
		{
			return;
		}

		OutBoxes.Add(FBox2D(FVector2D(MinX, MinY), FVector2D(MaxX, MaxY)));
	}

	static void T66MergeWallBoxes(TArray<FBox2D>& InOutBoxes)
	{
		constexpr float MergeTolerance = 1.0f;
		bool bMergedAny = true;
		while (bMergedAny)
		{
			bMergedAny = false;
			for (int32 Index = 0; Index < InOutBoxes.Num() && !bMergedAny; ++Index)
			{
				for (int32 OtherIndex = Index + 1; OtherIndex < InOutBoxes.Num(); ++OtherIndex)
				{
					const FBox2D& A = InOutBoxes[Index];
					const FBox2D& B = InOutBoxes[OtherIndex];
					const bool bSameXSpan = FMath::Abs(A.Min.X - B.Min.X) <= MergeTolerance
						&& FMath::Abs(A.Max.X - B.Max.X) <= MergeTolerance
						&& A.Max.Y >= (B.Min.Y - MergeTolerance)
						&& B.Max.Y >= (A.Min.Y - MergeTolerance);
					const bool bSameYSpan = FMath::Abs(A.Min.Y - B.Min.Y) <= MergeTolerance
						&& FMath::Abs(A.Max.Y - B.Max.Y) <= MergeTolerance
						&& A.Max.X >= (B.Min.X - MergeTolerance)
						&& B.Max.X >= (A.Min.X - MergeTolerance);
					if (!bSameXSpan && !bSameYSpan)
					{
						continue;
					}

					InOutBoxes[Index] = FBox2D(
						FVector2D(FMath::Min(A.Min.X, B.Min.X), FMath::Min(A.Min.Y, B.Min.Y)),
						FVector2D(FMath::Max(A.Max.X, B.Max.X), FMath::Max(A.Max.Y, B.Max.Y)));
					InOutBoxes.RemoveAtSwap(OtherIndex);
					bMergedAny = true;
					break;
				}
			}
		}
	}

	static T66TowerMapTerrain::ET66TowerGridTemplate T66ResolveGridCellTemplate(const T66TowerMapTerrain::FGridCell& Cell)
	{
		if (Cell.Semantic == T66TowerMapTerrain::ET66TowerGridCellSemantic::Unused)
		{
			return T66TowerMapTerrain::ET66TowerGridTemplate::Solid;
		}

		if (Cell.bContainsArrival || Cell.bContainsExit)
		{
			return T66TowerMapTerrain::ET66TowerGridTemplate::Arena;
		}

		if (Cell.Semantic == T66TowerMapTerrain::ET66TowerGridCellSemantic::OptionalLoop)
		{
			return T66TowerMapTerrain::ET66TowerGridTemplate::OptionalPocket;
		}

		const int32 ConnectionCount = T66CountGridConnections(Cell.ConnectionMask);
		if (ConnectionCount >= 4)
		{
			return T66TowerMapTerrain::ET66TowerGridTemplate::Cross;
		}
		if (ConnectionCount == 3)
		{
			return T66TowerMapTerrain::ET66TowerGridTemplate::TJunction;
		}
		if (ConnectionCount == 2)
		{
			const bool bOpposite = ((Cell.ConnectionMask & T66TowerMapTerrain::GridNorth) && (Cell.ConnectionMask & T66TowerMapTerrain::GridSouth))
				|| ((Cell.ConnectionMask & T66TowerMapTerrain::GridEast) && (Cell.ConnectionMask & T66TowerMapTerrain::GridWest));
			return bOpposite ? T66TowerMapTerrain::ET66TowerGridTemplate::Straight : T66TowerMapTerrain::ET66TowerGridTemplate::Corner;
		}

		return T66TowerMapTerrain::ET66TowerGridTemplate::Arena;
	}

	static void T66EmitGridCellSideWalls(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		T66TowerMapTerrain::FGridCell& Cell,
		const float HalfThickness,
		TArray<FBox2D>& OutBoxes,
		TArray<FBox2D>& OutTrapBoxes)
	{
		const float FloorMinX = Floor.Center.X - Floor.BoundsHalfExtent;
		const float FloorMaxX = Floor.Center.X + Floor.BoundsHalfExtent;
		const float FloorMinY = Floor.Center.Y - Floor.BoundsHalfExtent;
		const float FloorMaxY = Floor.Center.Y + Floor.BoundsHalfExtent;
		const float DoorHalfWidth = FMath::Min(Layout.GridDoorWidth * 0.5f, (Layout.GridCellSize * 0.5f) - (HalfThickness * 1.5f));
		const bool bTrapEligible = Cell.Semantic != T66TowerMapTerrain::ET66TowerGridCellSemantic::Unused;

		auto EmitAndTrack = [&](const float MinX, const float MinY, const float MaxX, const float MaxY)
		{
			if (MaxX <= MinX + KINDA_SMALL_NUMBER || MaxY <= MinY + KINDA_SMALL_NUMBER)
			{
				return;
			}

			const FBox2D Box(FVector2D(MinX, MinY), FVector2D(MaxX, MaxY));
			OutBoxes.Add(Box);
			Cell.EmittedWallBoxes.Add(Box);
			if (bTrapEligible)
			{
				OutTrapBoxes.Add(Box);
			}
		};

		auto EmitHorizontalSide = [&](const bool bNorth, const bool bOpen)
		{
			const float SideY = bNorth ? Cell.Bounds.Min.Y : Cell.Bounds.Max.Y;
			const bool bOuterEdge = bNorth ? (Cell.Coord.Y == 0) : (Cell.Coord.Y == Layout.GridRows - 1);
			const float MinY = bNorth ? (bOuterEdge ? FloorMinY : SideY - HalfThickness) : SideY - HalfThickness;
			const float MaxY = bNorth ? SideY + HalfThickness : (bOuterEdge ? FloorMaxY : SideY + HalfThickness);
			const float MinX = Cell.Bounds.Min.X;
			const float MaxX = Cell.Bounds.Max.X;
			if (!bOpen)
			{
				EmitAndTrack(MinX, MinY, MaxX, MaxY);
				return;
			}

			const float DoorCenterX = Cell.WorldCenter.X;
			EmitAndTrack(MinX, MinY, DoorCenterX - DoorHalfWidth, MaxY);
			EmitAndTrack(DoorCenterX + DoorHalfWidth, MinY, MaxX, MaxY);
		};

		auto EmitVerticalSide = [&](const bool bWest, const bool bOpen)
		{
			const float SideX = bWest ? Cell.Bounds.Min.X : Cell.Bounds.Max.X;
			const bool bOuterEdge = bWest ? (Cell.Coord.X == 0) : (Cell.Coord.X == Layout.GridColumns - 1);
			const float MinX = bWest ? (bOuterEdge ? FloorMinX : SideX - HalfThickness) : SideX - HalfThickness;
			const float MaxX = bWest ? SideX + HalfThickness : (bOuterEdge ? FloorMaxX : SideX + HalfThickness);
			const float MinY = Cell.Bounds.Min.Y;
			const float MaxY = Cell.Bounds.Max.Y;
			if (!bOpen)
			{
				EmitAndTrack(MinX, MinY, MaxX, MaxY);
				return;
			}

			const float DoorCenterY = Cell.WorldCenter.Y;
			EmitAndTrack(MinX, MinY, MaxX, DoorCenterY - DoorHalfWidth);
			EmitAndTrack(MinX, DoorCenterY + DoorHalfWidth, MaxX, MaxY);
		};

		EmitHorizontalSide(true, (Cell.ConnectionMask & T66TowerMapTerrain::GridNorth) != 0);
		EmitVerticalSide(false, (Cell.ConnectionMask & T66TowerMapTerrain::GridEast) != 0);
		EmitHorizontalSide(false, (Cell.ConnectionMask & T66TowerMapTerrain::GridSouth) != 0);
		EmitVerticalSide(true, (Cell.ConnectionMask & T66TowerMapTerrain::GridWest) != 0);
	}

	static void T66EmitGridCellInteriorTemplate(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FGridCell& Cell,
		const float HalfThickness,
		TArray<FBox2D>& OutBoxes)
	{
		if (Cell.Template == T66TowerMapTerrain::ET66TowerGridTemplate::Solid || Cell.Template == T66TowerMapTerrain::ET66TowerGridTemplate::Arena)
		{
			return;
		}

		const float BlockHalfThickness = FMath::Max(HalfThickness, Layout.PlacementCellSize * 0.24f);
		const float LongHalfSpan = Layout.GridCellSize * 0.34f;
		const float Offset = Layout.PlacementCellSize * 1.05f;
		const float PillarHalfExtent = Layout.PlacementCellSize * 0.45f;
		const FVector Center = Cell.WorldCenter;

		switch (Cell.Template)
		{
		case T66TowerMapTerrain::ET66TowerGridTemplate::Straight:
		{
			const bool bVerticalTravel = (Cell.ConnectionMask & T66TowerMapTerrain::GridNorth)
				&& (Cell.ConnectionMask & T66TowerMapTerrain::GridSouth);
			if (bVerticalTravel)
			{
				T66EmitWallRect(OutBoxes, Center.X - Offset - BlockHalfThickness, Center.Y - LongHalfSpan, Center.X - Offset + BlockHalfThickness, Center.Y + LongHalfSpan);
				T66EmitWallRect(OutBoxes, Center.X + Offset - BlockHalfThickness, Center.Y - LongHalfSpan, Center.X + Offset + BlockHalfThickness, Center.Y + LongHalfSpan);
			}
			else
			{
				T66EmitWallRect(OutBoxes, Center.X - LongHalfSpan, Center.Y - Offset - BlockHalfThickness, Center.X + LongHalfSpan, Center.Y - Offset + BlockHalfThickness);
				T66EmitWallRect(OutBoxes, Center.X - LongHalfSpan, Center.Y + Offset - BlockHalfThickness, Center.X + LongHalfSpan, Center.Y + Offset + BlockHalfThickness);
			}
			break;
		}
		case T66TowerMapTerrain::ET66TowerGridTemplate::Corner:
		{
			float PillarCenterX = Center.X;
			float PillarCenterY = Center.Y;
			if ((Cell.ConnectionMask & T66TowerMapTerrain::GridWest) == 0)
			{
				PillarCenterX -= Offset;
			}
			else if ((Cell.ConnectionMask & T66TowerMapTerrain::GridEast) == 0)
			{
				PillarCenterX += Offset;
			}

			if ((Cell.ConnectionMask & T66TowerMapTerrain::GridNorth) == 0)
			{
				PillarCenterY -= Offset;
			}
			else if ((Cell.ConnectionMask & T66TowerMapTerrain::GridSouth) == 0)
			{
				PillarCenterY += Offset;
			}

			T66EmitWallRect(
				OutBoxes,
				PillarCenterX - PillarHalfExtent,
				PillarCenterY - PillarHalfExtent,
				PillarCenterX + PillarHalfExtent,
				PillarCenterY + PillarHalfExtent);
			break;
		}
		case T66TowerMapTerrain::ET66TowerGridTemplate::TJunction:
		{
			if ((Cell.ConnectionMask & T66TowerMapTerrain::GridNorth) == 0)
			{
				T66EmitWallRect(OutBoxes, Center.X - LongHalfSpan, Center.Y - Offset - BlockHalfThickness, Center.X + LongHalfSpan, Center.Y - Offset + BlockHalfThickness);
			}
			else if ((Cell.ConnectionMask & T66TowerMapTerrain::GridSouth) == 0)
			{
				T66EmitWallRect(OutBoxes, Center.X - LongHalfSpan, Center.Y + Offset - BlockHalfThickness, Center.X + LongHalfSpan, Center.Y + Offset + BlockHalfThickness);
			}
			else if ((Cell.ConnectionMask & T66TowerMapTerrain::GridWest) == 0)
			{
				T66EmitWallRect(OutBoxes, Center.X - Offset - BlockHalfThickness, Center.Y - LongHalfSpan, Center.X - Offset + BlockHalfThickness, Center.Y + LongHalfSpan);
			}
			else
			{
				T66EmitWallRect(OutBoxes, Center.X + Offset - BlockHalfThickness, Center.Y - LongHalfSpan, Center.X + Offset + BlockHalfThickness, Center.Y + LongHalfSpan);
			}
			break;
		}
		case T66TowerMapTerrain::ET66TowerGridTemplate::Cross:
			T66EmitWallRect(OutBoxes, Center.X - PillarHalfExtent, Center.Y - PillarHalfExtent, Center.X + PillarHalfExtent, Center.Y + PillarHalfExtent);
			break;
		case T66TowerMapTerrain::ET66TowerGridTemplate::OptionalPocket:
			T66EmitWallRect(OutBoxes, Center.X - (PillarHalfExtent * 0.85f), Center.Y - (PillarHalfExtent * 0.85f), Center.X + (PillarHalfExtent * 0.85f), Center.Y + (PillarHalfExtent * 0.85f));
			break;
		default:
			break;
		}
	}

	static void T66BuildCachedFloorSpawnSlots(const T66TowerMapTerrain::FLayout& Layout, T66TowerMapTerrain::FFloor& Floor)
	{
		Floor.CachedWalkableSpawnSlots.Reset();
		Floor.CachedMainPathSpawnSlots.Reset();
		Floor.CachedOptionalSpawnSlots.Reset();
		Floor.CachedContentSpawnSlots.Reset();
		for (T66TowerMapTerrain::FGridCell& Cell : Floor.GridCells)
		{
			Cell.CachedSpawnSlots.Reset();
		}

		const float TileSize = FMath::Max(600.0f, Layout.PlacementCellSize);
		const float PolygonMinY = Floor.Center.Y - Floor.BoundsHalfExtent;
		const float PolygonMaxY = Floor.Center.Y + Floor.BoundsHalfExtent;
		const float PolygonMinX = Floor.Center.X - Floor.BoundsHalfExtent;
		const float PolygonMaxX = Floor.Center.X + Floor.BoundsHalfExtent;
		const float ContentSafeRadiusSq = FMath::Square(Layout.PlacementCellSize * 1.15f);

		for (float TileMinY = PolygonMinY; TileMinY < PolygonMaxY - KINDA_SMALL_NUMBER; TileMinY += TileSize)
		{
			const float TileMaxY = FMath::Min(TileMinY + TileSize, PolygonMaxY);
			const float TileCenterY = (TileMinY + TileMaxY) * 0.5f;
			for (float TileMinX = PolygonMinX; TileMinX < PolygonMaxX - KINDA_SMALL_NUMBER; TileMinX += TileSize)
			{
				const float TileMaxX = FMath::Min(TileMinX + TileSize, PolygonMaxX);
				const float TileCenterX = (TileMinX + TileMaxX) * 0.5f;
				const FVector Candidate(TileCenterX, TileCenterY, Floor.SurfaceZ);
				if (!T66IsWalkableTowerLocation(Floor, Candidate, 0.0f, 0.0f, 0.0f))
				{
					continue;
				}

				Floor.CachedWalkableSpawnSlots.Add(Candidate);
				if (Floor.GridCells.Num() <= 0)
				{
					continue;
				}

				const FIntPoint CellCoord = T66ProjectPointToGridCell(Layout, Floor, Candidate);
				if (!T66IsValidGridCoord(Layout, CellCoord))
				{
					continue;
				}

				const int32 CellIndex = T66GetGridCellIndex(Layout, CellCoord);
				if (!Floor.GridCells.IsValidIndex(CellIndex))
				{
					continue;
				}

				T66TowerMapTerrain::FGridCell& Cell = Floor.GridCells[CellIndex];
				if (Cell.Semantic == T66TowerMapTerrain::ET66TowerGridCellSemantic::Unused)
				{
					continue;
				}

				Cell.CachedSpawnSlots.Add(Candidate);
				const bool bSafeForContent =
					FVector::DistSquared2D(Candidate, Floor.ArrivalPoint) > ContentSafeRadiusSq
					&& FVector::DistSquared2D(Candidate, Floor.ExitPoint) > ContentSafeRadiusSq;
				if (Cell.Semantic == T66TowerMapTerrain::ET66TowerGridCellSemantic::OptionalLoop)
				{
					Floor.CachedOptionalSpawnSlots.Add(Candidate);
					if (bSafeForContent)
					{
						Floor.CachedContentSpawnSlots.Add(Candidate);
					}
				}
				else
				{
					Floor.CachedMainPathSpawnSlots.Add(Candidate);
					if (bSafeForContent)
					{
						Floor.CachedContentSpawnSlots.Add(Candidate);
					}
				}
			}
		}
	}

	static void T66FinalizeFloorMazeMetadata(const T66TowerMapTerrain::FLayout& Layout, T66TowerMapTerrain::FFloor& Floor)
	{
		if (Floor.MazeWallBoxes.Num() > 1)
		{
			T66MergeWallBoxes(Floor.MazeWallBoxes);
		}

		if (Floor.TrapEligibleWallBoxes.Num() <= 0)
		{
			Floor.TrapEligibleWallBoxes = Floor.MazeWallBoxes;
		}
		else if (Floor.TrapEligibleWallBoxes.Num() > 1)
		{
			T66MergeWallBoxes(Floor.TrapEligibleWallBoxes);
		}

		T66BuildCachedFloorSpawnSlots(Layout, Floor);
	}

	static bool T66BuildFloorMazeWalls_GridGraph(
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor,
		FRandomStream& Rng)
	{
		if (Layout.GridColumns <= 0 || Layout.GridRows <= 0 || Layout.GridCellSize <= KINDA_SMALL_NUMBER)
		{
			return false;
		}

		const int32 CellCount = Layout.GridColumns * Layout.GridRows;
		const FVector2D GridMin = T66GetGridMinCorner(Layout, Floor);
		Floor.GridCells.SetNum(CellCount);
		for (int32 CellY = 0; CellY < Layout.GridRows; ++CellY)
		{
			for (int32 CellX = 0; CellX < Layout.GridColumns; ++CellX)
			{
				T66TowerMapTerrain::FGridCell& Cell = Floor.GridCells[T66GetGridCellIndex(Layout, FIntPoint(CellX, CellY))];
				const float MinX = GridMin.X + (static_cast<float>(CellX) * Layout.GridCellSize);
				const float MinY = GridMin.Y + (static_cast<float>(CellY) * Layout.GridCellSize);
				const float MaxX = MinX + Layout.GridCellSize;
				const float MaxY = MinY + Layout.GridCellSize;
				Cell.Coord = FIntPoint(CellX, CellY);
				Cell.Bounds = FBox2D(FVector2D(MinX, MinY), FVector2D(MaxX, MaxY));
				Cell.WorldCenter = FVector((MinX + MaxX) * 0.5f, (MinY + MaxY) * 0.5f, Floor.SurfaceZ);
				Cell.ConnectionMask = 0;
				Cell.Semantic = T66TowerMapTerrain::ET66TowerGridCellSemantic::Unused;
				Cell.Template = T66TowerMapTerrain::ET66TowerGridTemplate::Solid;
				Cell.bContainsArrival = false;
				Cell.bContainsExit = false;
				Cell.MainPathIndex = INDEX_NONE;
				Cell.LoopId = INDEX_NONE;
				Cell.EmittedWallBoxes.Reset();
				Cell.CachedSpawnSlots.Reset();
			}
		}

		Floor.ArrivalCell = T66ProjectPointToGridCell(Layout, Floor, Floor.ArrivalPoint);
		Floor.ExitCell = T66ProjectPointToGridCell(Layout, Floor, Floor.ExitPoint);
		const int32 ArrivalIndex = T66GetGridCellIndex(Layout, Floor.ArrivalCell);
		const int32 ExitIndex = T66GetGridCellIndex(Layout, Floor.ExitCell);
		if (!Floor.GridCells.IsValidIndex(ArrivalIndex) || !Floor.GridCells.IsValidIndex(ExitIndex))
		{
			return false;
		}

		Floor.GridCells[ArrivalIndex].bContainsArrival = true;
		Floor.GridCells[ExitIndex].bContainsExit = true;

		TArray<int32> MainPathIndices;
		if (!T66BuildMainPathIndices(Layout, ArrivalIndex, ExitIndex, Rng, MainPathIndices) || MainPathIndices.Num() <= 0)
		{
			return false;
		}

		TArray<bool> Occupied;
		Occupied.Init(false, CellCount);
		for (int32 PathIndex = 0; PathIndex < MainPathIndices.Num(); ++PathIndex)
		{
			const int32 CellIndex = MainPathIndices[PathIndex];
			if (!Floor.GridCells.IsValidIndex(CellIndex))
			{
				return false;
			}

			T66TowerMapTerrain::FGridCell& Cell = Floor.GridCells[CellIndex];
			Occupied[CellIndex] = true;
			Cell.MainPathIndex = PathIndex;
			if (Cell.bContainsArrival && Cell.bContainsExit)
			{
				Cell.Semantic = T66TowerMapTerrain::ET66TowerGridCellSemantic::ArrivalExit;
			}
			else if (Cell.bContainsArrival)
			{
				Cell.Semantic = T66TowerMapTerrain::ET66TowerGridCellSemantic::Arrival;
			}
			else if (Cell.bContainsExit)
			{
				Cell.Semantic = T66TowerMapTerrain::ET66TowerGridCellSemantic::Exit;
			}
			else
			{
				Cell.Semantic = T66TowerMapTerrain::ET66TowerGridCellSemantic::MainPath;
			}

			Floor.MainPathCells.Add(Cell.Coord);
			if (PathIndex > 0)
			{
				T66ConnectGridCells(Floor, Layout, MainPathIndices[PathIndex - 1], CellIndex);
			}
		}

		int32 LoopId = 0;
		for (const int32 StartIndex : MainPathIndices)
		{
			if (Rng.FRand() > T66TowerGridBranchChance)
			{
				continue;
			}

			if (T66TryAddLoopBranch(Layout, Floor, Occupied, StartIndex, Rng, LoopId))
			{
				++LoopId;
			}
		}

		const float HalfThickness = FMath::Max(Layout.WallThickness * 0.40f, Layout.PlacementCellSize * T66TowerMazeWallHalfThicknessScale);
		const float FloorMinX = Floor.Center.X - Floor.BoundsHalfExtent;
		const float FloorMaxX = Floor.Center.X + Floor.BoundsHalfExtent;
		const float FloorMinY = Floor.Center.Y - Floor.BoundsHalfExtent;
		const float FloorMaxY = Floor.Center.Y + Floor.BoundsHalfExtent;
		for (T66TowerMapTerrain::FGridCell& Cell : Floor.GridCells)
		{
			Cell.Template = T66ResolveGridCellTemplate(Cell);
			if (Cell.Semantic == T66TowerMapTerrain::ET66TowerGridCellSemantic::Unused)
			{
				const float MinX = (Cell.Coord.X == 0) ? FloorMinX : Cell.Bounds.Min.X;
				const float MaxX = (Cell.Coord.X == Layout.GridColumns - 1) ? FloorMaxX : Cell.Bounds.Max.X;
				const float MinY = (Cell.Coord.Y == 0) ? FloorMinY : Cell.Bounds.Min.Y;
				const float MaxY = (Cell.Coord.Y == Layout.GridRows - 1) ? FloorMaxY : Cell.Bounds.Max.Y;
				T66EmitWallRect(Floor.MazeWallBoxes, MinX, MinY, MaxX, MaxY);
				Cell.EmittedWallBoxes.Add(FBox2D(FVector2D(MinX, MinY), FVector2D(MaxX, MaxY)));
				continue;
			}

			T66EmitGridCellSideWalls(Layout, Floor, Cell, HalfThickness, Floor.MazeWallBoxes, Floor.TrapEligibleWallBoxes);
			T66EmitGridCellInteriorTemplate(Layout, Cell, HalfThickness, Floor.MazeWallBoxes);
		}

		return true;
	}

	static void T66BuildFloorMazeWalls(
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor,
		FRandomStream& Rng)
	{
		T66ResetFloorMazeMetadata(Floor);
		if (!Floor.bGameplayFloor
			|| Floor.FloorNumber == Layout.StartFloorNumber
			|| Floor.FloorNumber == Layout.BossFloorNumber)
		{
			return;
		}

		bool bBuiltGridGraph = false;
		if (Layout.MazeMode == T66TowerMapTerrain::ET66TowerMazeMode::GridGraph)
		{
			FRandomStream GridRng = Rng;
			bBuiltGridGraph = T66BuildFloorMazeWalls_GridGraph(Layout, Floor, GridRng);
			if (bBuiltGridGraph)
			{
				Rng = GridRng;
			}
		}

		if (!bBuiltGridGraph)
		{
			T66ResetFloorMazeMetadata(Floor);
			T66BuildFloorMazeWalls_Legacy(Layout, Floor, Rng);
		}

		T66FinalizeFloorMazeMetadata(Layout, Floor);
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
		(void)SpawnLocation;
		T66ResetFloorMazeMetadata(Floor);

		const float HalfThickness = FMath::Max(Layout.WallThickness * 0.25f, Layout.PlacementCellSize * T66TowerMazeWallHalfThicknessScale);
		const float RoomHalfExtent = T66TowerStartRoomSquareSize * 0.5f;
		const float RoomBoundsInset = Layout.WallThickness + (Layout.PlacementCellSize * 0.20f);
		const float BoundsMinX = Floor.Center.X - (Floor.BoundsHalfExtent - RoomBoundsInset);
		const float BoundsMaxX = Floor.Center.X + (Floor.BoundsHalfExtent - RoomBoundsInset);
		const float BoundsMinY = Floor.Center.Y - (Floor.BoundsHalfExtent - RoomBoundsInset);
		const float BoundsMaxY = Floor.Center.Y + (Floor.BoundsHalfExtent - RoomBoundsInset);

		const float RoomCenterX = FMath::Clamp(AltarLocation.X, BoundsMinX + RoomHalfExtent, BoundsMaxX - RoomHalfExtent);
		const float RoomCenterY = FMath::Clamp(AltarLocation.Y, BoundsMinY + RoomHalfExtent, BoundsMaxY - RoomHalfExtent);
		const float MinX = RoomCenterX - RoomHalfExtent;
		const float MaxX = RoomCenterX + RoomHalfExtent;
		const float MinY = RoomCenterY - RoomHalfExtent;
		const float MaxY = RoomCenterY + RoomHalfExtent;

		Floor.MazeWallBoxes.Add(FBox2D(FVector2D(MinX - HalfThickness, MinY), FVector2D(MinX + HalfThickness, MaxY)));
		Floor.MazeWallBoxes.Add(FBox2D(FVector2D(MaxX - HalfThickness, MinY), FVector2D(MaxX + HalfThickness, MaxY)));
		Floor.MazeWallBoxes.Add(FBox2D(FVector2D(MinX, MinY - HalfThickness), FVector2D(MaxX, MinY + HalfThickness)));
		Floor.MazeWallBoxes.Add(FBox2D(FVector2D(MinX, MaxY - HalfThickness), FVector2D(MaxX, MaxY + HalfThickness)));
		T66FinalizeFloorMazeMetadata(Layout, Floor);
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
		const bool bEnableCollision,
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
			bEnableCollision,
			Tags);
	}

	static void T66SpawnPolygonSurface(
		UWorld* World,
		UStaticMesh* CubeMesh,
		UMaterialInterface* SurfaceMaterial,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const float SurfaceZ,
		const float SurfaceThickness,
		const FActorSpawnParameters& SpawnParams,
		const bool bEnableCollision,
		const TArray<FName>& Tags)
	{
		if (!World || !CubeMesh || !SurfaceMaterial || SurfaceThickness <= 1.0f)
		{
			return;
		}

		const float PolygonMinY = Floor.Center.Y - Floor.BoundsHalfExtent;
		const float PolygonMaxY = Floor.Center.Y + Floor.BoundsHalfExtent;
		const float PolygonMinX = Floor.Center.X - Floor.BoundsHalfExtent;
		const float PolygonMaxX = Floor.Center.X + Floor.BoundsHalfExtent;

		auto SpawnRect = [&](const float MinX, const float MinY, const float MaxX, const float MaxY)
		{
			T66SpawnFloorSlab(
				World,
				CubeMesh,
				SurfaceMaterial,
				Floor.Center,
				SurfaceZ,
				SurfaceThickness,
				FVector2D(MinX - Floor.Center.X, MinY - Floor.Center.Y),
				FVector2D(MaxX - Floor.Center.X, MaxY - Floor.Center.Y),
				SpawnParams,
				bEnableCollision,
				Tags);
		};

		if (!Floor.bHasDropHole || Floor.HoleHalfExtent.X <= 1.0f || Floor.HoleHalfExtent.Y <= 1.0f)
		{
			SpawnRect(PolygonMinX, PolygonMinY, PolygonMaxX, PolygonMaxY);
			return;
		}

		const float HoleMinX = FMath::Clamp(Floor.HoleCenter.X - Floor.HoleHalfExtent.X, PolygonMinX, PolygonMaxX);
		const float HoleMaxX = FMath::Clamp(Floor.HoleCenter.X + Floor.HoleHalfExtent.X, PolygonMinX, PolygonMaxX);
		const float HoleMinY = FMath::Clamp(Floor.HoleCenter.Y - Floor.HoleHalfExtent.Y, PolygonMinY, PolygonMaxY);
		const float HoleMaxY = FMath::Clamp(Floor.HoleCenter.Y + Floor.HoleHalfExtent.Y, PolygonMinY, PolygonMaxY);

		// Tower floors are axis-aligned squares with an axis-aligned drop hole. Representing the
		// surface as four coarse slabs keeps the exact shape while removing hundreds of runtime actors.
		SpawnRect(PolygonMinX, PolygonMinY, HoleMinX, PolygonMaxY);
		SpawnRect(HoleMaxX, PolygonMinY, PolygonMaxX, PolygonMaxY);
		SpawnRect(HoleMinX, PolygonMinY, HoleMaxX, HoleMinY);
		SpawnRect(HoleMinX, HoleMaxY, HoleMaxX, PolygonMaxY);
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
		T66SpawnPolygonSurface(
			World,
			CubeMesh,
			GroundMaterial,
			Layout,
			Floor,
			Floor.SurfaceZ,
			Layout.FloorThickness,
			SpawnParams,
			true,
			Tags);
	}

	static void T66SpawnMeshWallBox(
		UWorld* World,
		const TArray<UStaticMesh*>& WallMeshes,
		UMaterialInterface* MeshMaterialOverride,
		const FBox2D& WallBox,
		const float BaseZ,
		const float DesiredHeight,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags,
		const int32 Seed,
		const bool bEnableCollision)
	{
		if (!World || WallMeshes.Num() <= 0)
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
		const float DesiredHalfWidth = FMath::Max(340.0f, WallThickness * 1.65f);
		const float Spacing = FMath::Max(180.0f, DesiredHalfWidth * 0.80f);
		const int32 ColumnCount = FMath::Max(3, FMath::CeilToInt(SpanLength / Spacing) + 1);
		const int32 DepthRowCount = WallThickness >= 260.0f ? 2 : 1;
		const float DepthOffset = (DepthRowCount > 1) ? FMath::Max(90.0f, WallThickness * 0.28f) : 0.0f;
		const FVector2D AlongAxis = bVertical ? FVector2D(0.0f, 1.0f) : FVector2D(1.0f, 0.0f);
		const FVector2D CrossAxis = bVertical ? FVector2D(1.0f, 0.0f) : FVector2D(0.0f, 1.0f);

		FRandomStream Rng(Seed);
		const FVector2D LineStart = bVertical
			? FVector2D(WallCenter.X, WallBox.Min.Y)
			: FVector2D(WallBox.Min.X, WallCenter.Y);
		const FVector2D LineEnd = bVertical
			? FVector2D(WallCenter.X, WallBox.Max.Y)
			: FVector2D(WallBox.Max.X, WallCenter.Y);

		for (int32 RowIndex = 0; RowIndex < DepthRowCount; ++RowIndex)
		{
			const float DepthAlpha = (DepthRowCount <= 1)
				? 0.0f
				: (static_cast<float>(RowIndex) / static_cast<float>(DepthRowCount - 1)) * 2.0f - 1.0f;
			const FVector2D DepthShift = CrossAxis * (DepthAlpha * DepthOffset);

			for (int32 ColumnIndex = 0; ColumnIndex < ColumnCount; ++ColumnIndex)
			{
				const int32 MeshOffset = Seed + RowIndex + ColumnIndex;
				const int32 MeshIndex = ((MeshOffset % WallMeshes.Num()) + WallMeshes.Num()) % WallMeshes.Num();
				UStaticMesh* Mesh = WallMeshes[MeshIndex];
				if (!Mesh)
				{
					continue;
				}

				const float Alpha = (ColumnCount <= 1)
					? 0.5f
					: (static_cast<float>(ColumnIndex) / static_cast<float>(ColumnCount - 1));
				const float AlongJitter = (ColumnCount > 3)
					? Rng.FRandRange(-Spacing * 0.06f, Spacing * 0.06f)
					: 0.0f;
				const FVector2D WallPosition2D = FMath::Lerp(LineStart, LineEnd, Alpha) + DepthShift + (AlongAxis * AlongJitter);
				const FVector MeshHalfExtents = Mesh->GetBounds().BoxExtent;
				const float MeshRadius = FMath::Max(FMath::Max(MeshHalfExtents.X, MeshHalfExtents.Y), 1.0f);
				const float XYScale = FMath::Max(4.5f, DesiredHalfWidth / MeshRadius);
				const float ZScale = (DesiredHeight * 0.5f) / FMath::Max(MeshHalfExtents.Z, 1.0f);
				const float BaseYaw = bVertical ? 90.0f : 0.0f;
				const float JitterYaw = Rng.FRandRange(-16.0f, 16.0f);
				const FVector Scale(XYScale, XYScale, ZScale);

				T66SpawnGroundedMeshActor(
					World,
					Mesh,
					MeshMaterialOverride,
					FVector(WallPosition2D.X, WallPosition2D.Y, BaseZ),
					FRotator(0.0f, BaseYaw + JitterYaw, 0.0f),
					Scale,
					SpawnParams,
					bEnableCollision,
					Tags);
			}
		}
	}

	static float T66ResolveFloorCeilingBottomZ(
		const T66TowerMapTerrain::FLayout& Layout,
		const TArray<T66TowerThemeVisuals::FResolvedTheme>& FloorThemes,
		const int32 FloorIndex)
	{
		(void)FloorThemes;
		if (!Layout.Floors.IsValidIndex(FloorIndex))
		{
			return 0.0f;
		}

		const T66TowerMapTerrain::FFloor& Floor = Layout.Floors[FloorIndex];
		if (FloorIndex == 0)
		{
			return Floor.SurfaceZ + T66TowerStartFloorHeadroom;
		}

		const T66TowerMapTerrain::FFloor& CarrierFloor = Layout.Floors[FloorIndex - 1];
		return CarrierFloor.SurfaceZ - Layout.FloorThickness - T66TowerRoofSkinThickness;
	}

	static bool T66BuildFloorRoofSurface(
		const T66TowerMapTerrain::FLayout& Layout,
		const TArray<T66TowerThemeVisuals::FResolvedTheme>& FloorThemes,
		const int32 FloorIndex,
		T66TowerMapTerrain::FFloor& OutRoofGeometryFloor,
		float& OutRoofSurfaceZ,
		float& OutRoofThickness,
		bool& bOutEnableCollision)
	{
		(void)FloorThemes;
		if (!Layout.Floors.IsValidIndex(FloorIndex) || !FloorThemes.IsValidIndex(FloorIndex))
		{
			return false;
		}

		const T66TowerMapTerrain::FFloor& Floor = Layout.Floors[FloorIndex];
		if (FloorIndex == 0)
		{
			OutRoofGeometryFloor = Floor;
			OutRoofGeometryFloor.bHasDropHole = false;
			OutRoofGeometryFloor.HoleCenter = OutRoofGeometryFloor.Center;
			OutRoofGeometryFloor.HoleCenter.Z = OutRoofGeometryFloor.SurfaceZ;
			OutRoofGeometryFloor.HoleHalfExtent = FVector2D::ZeroVector;
			OutRoofGeometryFloor.PolygonApothem = FMath::Max(1200.0f, Floor.PolygonApothem - 900.0f);
			OutRoofGeometryFloor.BoundsHalfExtent = OutRoofGeometryFloor.PolygonApothem;
			OutRoofGeometryFloor.WalkableHalfExtent = OutRoofGeometryFloor.BoundsHalfExtent;
			OutRoofSurfaceZ = Floor.SurfaceZ + T66TowerStartFloorHeadroom + Layout.FloorThickness;
			OutRoofThickness = Layout.FloorThickness;
			bOutEnableCollision = true;
			return true;
		}

		OutRoofGeometryFloor = Layout.Floors[FloorIndex - 1];
		OutRoofSurfaceZ = OutRoofGeometryFloor.SurfaceZ - Layout.FloorThickness;
		OutRoofThickness = T66TowerRoofSkinThickness;
		bOutEnableCollision = false;
		return true;
	}

	static void T66SpawnFloorRoofSurface(
		UWorld* World,
		UStaticMesh* CubeMesh,
		UMaterialInterface* RoofMaterial,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& RoofGeometryFloor,
		const float RoofSurfaceZ,
		const float RoofThickness,
		const FActorSpawnParameters& SpawnParams,
		const bool bEnableCollision,
		const TArray<FName>& Tags)
	{
		if (!World || !CubeMesh || !RoofMaterial || RoofThickness <= 1.0f)
		{
			return;
		}

		T66SpawnPolygonSurface(
			World,
			CubeMesh,
			RoofMaterial,
			Layout,
			RoofGeometryFloor,
			RoofSurfaceZ,
			RoofThickness,
			SpawnParams,
			bEnableCollision,
			Tags);
	}

	static void T66SpawnThemedWallBox(
		UWorld* World,
		UStaticMesh* CubeMesh,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const FBox2D& WallBox,
		const float BaseZ,
		const float DesiredHeight,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags,
		const int32 Seed)
	{
		const FVector2D WallCenter = (WallBox.Min + WallBox.Max) * 0.5f;
		const FVector2D WallHalfExtents = (WallBox.Max - WallBox.Min) * 0.5f;
		if (WallHalfExtents.X <= 5.0f || WallHalfExtents.Y <= 5.0f)
		{
			return;
		}

		const float WallHalfHeight = DesiredHeight * 0.5f;
		const FVector WallLocation(WallCenter.X, WallCenter.Y, BaseZ + WallHalfHeight);
		const FVector WallExtents(WallHalfExtents.X, WallHalfExtents.Y, WallHalfHeight);

		switch (Theme.WallFamily)
		{
		case T66TowerThemeVisuals::EWallFamily::MeshCluster:
		{
			if (CubeMesh)
			{
				T66SpawnStaticMeshActor(
					World,
					CubeMesh,
					Theme.WallMaterial ? Theme.WallMaterial : Theme.FloorMaterial,
					WallLocation,
					FRotator::ZeroRotator,
					WallExtents,
					SpawnParams,
					true,
					Tags);
			}
			return;
		}

		case T66TowerThemeVisuals::EWallFamily::SplitCollisionVisual:
			if (CubeMesh)
			{
				T66SpawnStaticMeshActor(
					World,
					CubeMesh,
					Theme.WallMaterial ? Theme.WallMaterial : Theme.FloorMaterial,
					WallLocation,
					FRotator::ZeroRotator,
					WallExtents,
					SpawnParams,
					true,
					Tags);
			}
			return;

		case T66TowerThemeVisuals::EWallFamily::SolidMaterial:
		default:
			if (CubeMesh)
			{
				T66SpawnStaticMeshActor(
					World,
					CubeMesh,
					Theme.WallMaterial,
					WallLocation,
					FRotator::ZeroRotator,
					WallExtents,
					SpawnParams,
					true,
					Tags);
			}
			return;
		}
	}

	static void T66SpawnShellWallsForFloor(
		UWorld* World,
		UStaticMesh* CubeMesh,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const float WallHeight,
		const FActorSpawnParameters& SpawnParams)
	{
		const float WallHalfDepth = Layout.WallThickness * 0.5f;
		const float WallHalfSpan = Layout.ShellRadius + WallHalfDepth;
		const TArray<FName> ShellTags = {
			T66TraversalBarrierTag,
			FName(TEXT("T66_Floor_Tower_Shell")),
			FName(*FString::Printf(TEXT("T66_Floor_Tower_%02d"), Floor.FloorNumber))
		};

		T66SpawnThemedWallBox(
			World,
			CubeMesh,
			Theme,
			FBox2D(FVector2D(Layout.ShellRadius - WallHalfDepth, -WallHalfSpan), FVector2D(Layout.ShellRadius + WallHalfDepth, WallHalfSpan)),
			Floor.SurfaceZ,
			WallHeight,
			SpawnParams,
			ShellTags,
			Layout.Preset.Seed + (Floor.FloorNumber * 4101));
		T66SpawnThemedWallBox(
			World,
			CubeMesh,
			Theme,
			FBox2D(FVector2D(-Layout.ShellRadius - WallHalfDepth, -WallHalfSpan), FVector2D(-Layout.ShellRadius + WallHalfDepth, WallHalfSpan)),
			Floor.SurfaceZ,
			WallHeight,
			SpawnParams,
			ShellTags,
			Layout.Preset.Seed + (Floor.FloorNumber * 4102));
		T66SpawnThemedWallBox(
			World,
			CubeMesh,
			Theme,
			FBox2D(FVector2D(-WallHalfSpan, Layout.ShellRadius - WallHalfDepth), FVector2D(WallHalfSpan, Layout.ShellRadius + WallHalfDepth)),
			Floor.SurfaceZ,
			WallHeight,
			SpawnParams,
			ShellTags,
			Layout.Preset.Seed + (Floor.FloorNumber * 4103));
		T66SpawnThemedWallBox(
			World,
			CubeMesh,
			Theme,
			FBox2D(FVector2D(-WallHalfSpan, -Layout.ShellRadius - WallHalfDepth), FVector2D(WallHalfSpan, -Layout.ShellRadius + WallHalfDepth)),
			Floor.SurfaceZ,
			WallHeight,
			SpawnParams,
			ShellTags,
			Layout.Preset.Seed + (Floor.FloorNumber * 4104));
	}

	static void T66SpawnMazeWalls(
		UWorld* World,
		UStaticMesh* CubeMesh,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const float WallHeight,
		const FActorSpawnParameters& SpawnParams)
	{
		if (!World || Floor.MazeWallBoxes.Num() <= 0)
		{
			return;
		}

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

			T66SpawnThemedWallBox(
				World,
				CubeMesh,
				Theme,
				WallBox,
				Floor.SurfaceZ,
				WallHeight,
				SpawnParams,
				WallTags,
				Layout.Preset.Seed + (Floor.FloorNumber * 913) + static_cast<int32>(WallCenter.X + WallCenter.Y));
		}
	}

	static void T66SpawnPropActors(
		UWorld* World,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const FActorSpawnParameters& SpawnParams)
	{
		if (!World || !Floor.bGameplayFloor)
		{
			return;
		}

		if (Theme.DecorationMeshes.Num() <= 0)
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

			UStaticMesh* Mesh = Theme.DecorationMeshes[Rng.RandRange(0, Theme.DecorationMeshes.Num() - 1)];
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
				T66ConfigureTowerCollisionResponses(MeshComponent, false);
				T66OptimizeTowerMeshComponent(MeshComponent);
				MeshComponent->SetMobility(EComponentMobility::Static);
				const float Scale = (Theme.ThemeName == FName(TEXT("Martian")))
					? Rng.FRandRange(0.85f, 1.35f)
					: Rng.FRandRange(0.70f, 1.20f);
				PropActor->SetActorScale3D(FVector(Scale));
				if (Theme.DecorationMaterialOverride)
				{
					const int32 MaterialCount = MeshComponent->GetNumMaterials();
					for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
					{
						MeshComponent->SetMaterial(MaterialIndex, Theme.DecorationMaterialOverride);
					}
				}
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
	int32 ResolveGameplayLevelNumberForDifficulty(const ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Easy:       return 1;
		case ET66Difficulty::Medium:     return 2;
		case ET66Difficulty::Hard:       return 3;
		case ET66Difficulty::VeryHard:   return 4;
		case ET66Difficulty::Impossible: return 5;
		default:                         return 1;
		}
	}

	ET66TowerGameplayLevelTheme ResolveGameplayLevelTheme(const int32 GameplayLevelNumber)
	{
		switch (GameplayLevelNumber)
		{
		case 1:  return ET66TowerGameplayLevelTheme::Dungeon;
		case 2:  return ET66TowerGameplayLevelTheme::Forest;
		case 3:  return ET66TowerGameplayLevelTheme::Ocean;
		case 4:  return ET66TowerGameplayLevelTheme::Martian;
		case 5:  return ET66TowerGameplayLevelTheme::Hell;
		default: return ET66TowerGameplayLevelTheme::Dungeon;
		}
	}

	FText GetFloorDisplayName(const FFloor& Floor)
	{
		switch (Floor.FloorRole)
		{
		case ET66TowerFloorRole::Start:
			return NSLOCTEXT("T66.Tower", "StartLevel", "Start Level");
		case ET66TowerFloorRole::Boss:
			return NSLOCTEXT("T66.Tower", "BossLevel", "Boss Level");
		case ET66TowerFloorRole::Gameplay:
			if (Floor.GameplayLevelNumber > 0)
			{
				return FText::Format(
					NSLOCTEXT("T66.Tower", "GameplayLevel", "Level {0}"),
					FText::AsNumber(Floor.GameplayLevelNumber));
			}
			break;
		default:
			break;
		}

		return FText::Format(
			NSLOCTEXT("T66.Tower", "FloorFallback", "Floor {0}"),
			FText::AsNumber(Floor.FloorNumber));
	}

	FText GetFloorDisplayName(const FLayout& Layout, const int32 FloorNumber)
	{
		const FFloor* Floor = nullptr;
		if (T66TryGetFloor(Layout, FloorNumber, Floor) && Floor)
		{
			return GetFloorDisplayName(*Floor);
		}

		if (FloorNumber == Layout.StartFloorNumber)
		{
			return NSLOCTEXT("T66.Tower", "StartLevelByNumber", "Start Level");
		}
		if (FloorNumber == Layout.BossFloorNumber)
		{
			return NSLOCTEXT("T66.Tower", "BossLevelByNumber", "Boss Level");
		}

		return FText::Format(
			NSLOCTEXT("T66.Tower", "FloorFallbackByNumber", "Floor {0}"),
			FText::AsNumber(FloorNumber));
	}

	bool BuildLayout(const FT66MapPreset& Preset, FLayout& OutLayout)
	{
		OutLayout = FLayout{};
		OutLayout.Preset = Preset;
		OutLayout.PlacementCellSize = 1300.0f;
		OutLayout.FloorThickness = 280.0f;
		OutLayout.FloorSpacing = T66TowerStandardFloorHeadroom + OutLayout.FloorThickness + T66TowerRoofSkinThickness;
		OutLayout.WallThickness = 280.0f;
		OutLayout.ShellRadius = 20000.0f;
		OutLayout.MazeMode = T66GetConfiguredTowerMazeMode();
		OutLayout.GridColumns = T66TowerGridDefaultColumns;
		OutLayout.GridRows = T66TowerGridDefaultRows;
		OutLayout.GridCellSize = T66TowerGridDefaultCellSize;
		OutLayout.GridDoorWidth = T66TowerGridDefaultDoorWidth;
		OutLayout.StartFloorNumber = 1;
		OutLayout.FirstGameplayFloorNumber = 2;
		OutLayout.LastGameplayFloorNumber = 6;
		OutLayout.BossFloorNumber = 7;

		const float TopFloorZ = Preset.BaselineZ + 1600.0f;
		const float FloorSpacing = OutLayout.FloorSpacing;
		const FVector2D HoleHalfExtent(OutLayout.PlacementCellSize * 0.5f, OutLayout.PlacementCellSize * 0.5f);
		const float FloorBottomZ = TopFloorZ - (6.0f * FloorSpacing) - OutLayout.FloorThickness;
		FRandomStream HoleRng(Preset.Seed * 977 + 311);

		OutLayout.TraceStartZ = TopFloorZ + 12000.0f;
		OutLayout.TraceEndZ = FloorBottomZ - 12000.0f;

		for (int32 FloorIndex = 0; FloorIndex < 7; ++FloorIndex)
		{
			FFloor& Floor = OutLayout.Floors.AddDefaulted_GetRef();
			Floor.FloorNumber = FloorIndex + 1;
			Floor.FloorRole =
				(Floor.FloorNumber == OutLayout.StartFloorNumber) ? ET66TowerFloorRole::Start :
				(Floor.FloorNumber == OutLayout.BossFloorNumber) ? ET66TowerFloorRole::Boss :
				ET66TowerFloorRole::Gameplay;
			Floor.bGameplayFloor = Floor.FloorRole == ET66TowerFloorRole::Gameplay;
			Floor.GameplayLevelNumber = Floor.bGameplayFloor
				? (Floor.FloorNumber - OutLayout.FirstGameplayFloorNumber + 1)
				: INDEX_NONE;
			Floor.Theme = Floor.bGameplayFloor
				? ResolveGameplayLevelTheme(Floor.GameplayLevelNumber)
				: ((Floor.FloorRole == ET66TowerFloorRole::Boss)
					? ET66TowerGameplayLevelTheme::Hell
					: ET66TowerGameplayLevelTheme::Dungeon);
			Floor.bHasDropHole = Floor.FloorNumber < OutLayout.BossFloorNumber;
			Floor.Center = FVector(0.0f, 0.0f, TopFloorZ - (static_cast<float>(FloorIndex) * FloorSpacing));
			Floor.SurfaceZ = Floor.Center.Z;
			Floor.PolygonApothem = OutLayout.ShellRadius - (OutLayout.WallThickness * 0.5f) + 20.0f;
			Floor.BoundsHalfExtent = Floor.PolygonApothem;
			Floor.WalkableHalfExtent = Floor.PolygonApothem - ((Floor.FloorNumber == OutLayout.BossFloorNumber) ? 1600.0f : 1300.0f);
			Floor.FloorTag =
				(Floor.FloorRole == ET66TowerFloorRole::Start) ? T66FloorStartTag :
				(Floor.FloorRole == ET66TowerFloorRole::Boss) ? T66FloorBossTag :
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

		for (int32 FloorIndex = 0; FloorIndex < OutLayout.Floors.Num(); ++FloorIndex)
		{
			FFloor& Floor = OutLayout.Floors[FloorIndex];
			Floor.ExitPoint = Floor.bHasDropHole ? Floor.HoleCenter : Floor.Center;
			Floor.ExitPoint.Z = Floor.SurfaceZ;

			if (FloorIndex > 0)
			{
				Floor.ArrivalPoint = OutLayout.Floors[FloorIndex - 1].HoleCenter;
			}
			else
			{
				Floor.ArrivalPoint = Floor.Center;
			}

			Floor.ArrivalPoint.Z = Floor.SurfaceZ;
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
		for (FFloor& Floor : OutLayout.Floors)
		{
			if (Floor.FloorNumber == OutLayout.StartFloorNumber)
			{
				T66BuildStartFloorRoom(
					OutLayout,
					Floor,
					OutLayout.SpawnSurfaceLocation,
					OutLayout.StartAnchorSurfaceLocation);
				break;
			}
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
		TArray<const FFloor*> GameplayFloors;
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
			const TArray<FVector>& PreferredSlots = Floor.CachedContentSpawnSlots.Num() > 0
				? Floor.CachedContentSpawnSlots
				: Floor.CachedWalkableSpawnSlots;
			if (PreferredSlots.Num() > 0)
			{
				const int32 StartIndex = Rng.RandRange(0, PreferredSlots.Num() - 1);
				for (int32 Offset = 0; Offset < PreferredSlots.Num(); ++Offset)
				{
					const FVector Candidate = PreferredSlots[(StartIndex + Offset) % PreferredSlots.Num()];
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
			}

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

	bool TryGetFloorTileCenterSpawnLocation(
		UWorld* World,
		const FLayout& Layout,
		const int32 FloorNumber,
		FRandomStream& Rng,
		FVector& OutLocation,
		const float EdgePadding,
		const float HolePadding,
		const float WallPadding)
	{
		const FFloor* Floor = nullptr;
		if (!T66TryGetFloor(Layout, FloorNumber, Floor) || !Floor)
		{
			return false;
		}

		const float EffectiveEdgePadding = FMath::Max(0.0f, EdgePadding);
		const float EffectiveHolePadding = FMath::Max(0.0f, HolePadding);
		const float EffectiveWallPadding = FMath::Max(0.0f, WallPadding);
		const float TileSize = FMath::Max(600.0f, Layout.PlacementCellSize);
		const float PolygonMinY = Floor->Center.Y - Floor->BoundsHalfExtent;
		const float PolygonMaxY = Floor->Center.Y + Floor->BoundsHalfExtent;
		const float PolygonMinX = Floor->Center.X - Floor->BoundsHalfExtent;
		const float PolygonMaxX = Floor->Center.X + Floor->BoundsHalfExtent;

		TArray<FVector, TInlineAllocator<64>> CandidateCenters;
		if (Floor->CachedWalkableSpawnSlots.Num() > 0)
		{
			for (const FVector& Candidate : Floor->CachedWalkableSpawnSlots)
			{
				if (T66IsWalkableTowerLocation(*Floor, Candidate, EffectiveEdgePadding, EffectiveHolePadding, EffectiveWallPadding))
				{
					CandidateCenters.Add(Candidate);
				}
			}
		}
		else
		{
			for (float TileMinY = PolygonMinY; TileMinY < PolygonMaxY - KINDA_SMALL_NUMBER; TileMinY += TileSize)
			{
				const float TileMaxY = FMath::Min(TileMinY + TileSize, PolygonMaxY);
				const float TileCenterY = (TileMinY + TileMaxY) * 0.5f;
				for (float TileMinX = PolygonMinX; TileMinX < PolygonMaxX - KINDA_SMALL_NUMBER; TileMinX += TileSize)
				{
					const float TileMaxX = FMath::Min(TileMinX + TileSize, PolygonMaxX);
					const float TileCenterX = (TileMinX + TileMaxX) * 0.5f;
					const FVector Candidate(TileCenterX, TileCenterY, Floor->SurfaceZ);
					if (!T66IsWalkableTowerLocation(*Floor, Candidate, EffectiveEdgePadding, EffectiveHolePadding, EffectiveWallPadding))
					{
						continue;
					}

					CandidateCenters.Add(Candidate);
				}
			}
		}

		if (CandidateCenters.Num() <= 0)
		{
			return false;
		}

		const int32 StartIndex = Rng.RandRange(0, CandidateCenters.Num() - 1);
		for (int32 Offset = 0; Offset < CandidateCenters.Num(); ++Offset)
		{
			const FVector Candidate = CandidateCenters[(StartIndex + Offset) % CandidateCenters.Num()];
			FVector SnappedLocation = FVector::ZeroVector;
			if (T66TraceDownToSurface(World, Layout, Candidate, SnappedLocation))
			{
				OutLocation = SnappedLocation;
				return true;
			}
		}

		return false;
	}

	bool TryGetMazeWallSpawnLocation(
		UWorld* World,
		const FLayout& Layout,
		const int32 FloorNumber,
		FRandomStream& Rng,
		FVector& OutLocation,
		FVector& OutWallNormal,
		const float EndPadding)
	{
		const FFloor* Floor = nullptr;
		if (!World || !T66TryGetFloor(Layout, FloorNumber, Floor) || !Floor)
		{
			return false;
		}

		const TArray<FBox2D>& CandidateWallBoxes = Floor->TrapEligibleWallBoxes.Num() > 0
			? Floor->TrapEligibleWallBoxes
			: Floor->MazeWallBoxes;
		if (CandidateWallBoxes.Num() <= 0)
		{
			return false;
		}

		const float EffectiveEndPadding = FMath::Max(0.0f, EndPadding);
		for (int32 Attempt = 0; Attempt < 24; ++Attempt)
		{
			const FBox2D& WallBox = CandidateWallBoxes[Rng.RandRange(0, CandidateWallBoxes.Num() - 1)];
			const FVector2D WallCenter = (WallBox.Min + WallBox.Max) * 0.5f;
			const FVector2D WallHalfExtents = (WallBox.Max - WallBox.Min) * 0.5f;
			if (WallHalfExtents.X <= KINDA_SMALL_NUMBER || WallHalfExtents.Y <= KINDA_SMALL_NUMBER)
			{
				continue;
			}

			const bool bVertical = WallHalfExtents.Y >= WallHalfExtents.X;
			const float HalfThickness = bVertical ? WallHalfExtents.X : WallHalfExtents.Y;
			const float HalfSpan = bVertical ? WallHalfExtents.Y : WallHalfExtents.X;
			const float SampleMin = -HalfSpan + EffectiveEndPadding;
			const float SampleMax = HalfSpan - EffectiveEndPadding;
			if (SampleMax <= SampleMin)
			{
				continue;
			}

			const float AlongWall = Rng.FRandRange(SampleMin, SampleMax);
			const FVector WallLinePoint = bVertical
				? FVector(WallCenter.X, WallCenter.Y + AlongWall, Floor->SurfaceZ)
				: FVector(WallCenter.X + AlongWall, WallCenter.Y, Floor->SurfaceZ);

			const FVector CandidateNormals[2] =
			{
				bVertical ? FVector(1.f, 0.f, 0.f) : FVector(0.f, 1.f, 0.f),
				bVertical ? FVector(-1.f, 0.f, 0.f) : FVector(0.f, -1.f, 0.f)
			};
			const float SurfaceOffset = HalfThickness + 90.f;

			TArray<int32, TInlineAllocator<2>> ValidSideIndices;
			for (int32 SideIndex = 0; SideIndex < UE_ARRAY_COUNT(CandidateNormals); ++SideIndex)
			{
				const FVector Candidate = WallLinePoint + (CandidateNormals[SideIndex] * SurfaceOffset);
				if (T66IsWalkableTowerLocation(*Floor, Candidate, 200.0f, 450.0f, 120.0f))
				{
					ValidSideIndices.Add(SideIndex);
				}
			}

			if (ValidSideIndices.Num() <= 0)
			{
				continue;
			}

			const int32 ChosenSideIndex = ValidSideIndices[Rng.RandRange(0, ValidSideIndices.Num() - 1)];
			const FVector Candidate = WallLinePoint + (CandidateNormals[ChosenSideIndex] * SurfaceOffset);
			FVector SnappedLocation = FVector::ZeroVector;
			if (!T66TraceDownToSurface(World, Layout, Candidate, SnappedLocation))
			{
				continue;
			}

			OutLocation = SnappedLocation;
			OutWallNormal = CandidateNormals[ChosenSideIndex];
			return true;
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

		if (Floor->CachedWalkableSpawnSlots.Num() > 0)
		{
			const int32 StartIndex = Rng.RandRange(0, Floor->CachedWalkableSpawnSlots.Num() - 1);
			for (int32 Offset = 0; Offset < Floor->CachedWalkableSpawnSlots.Num(); ++Offset)
			{
				const FVector Candidate = Floor->CachedWalkableSpawnSlots[(StartIndex + Offset) % Floor->CachedWalkableSpawnSlots.Num()];
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
		(void)Difficulty;
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

		TArray<T66TowerThemeVisuals::FResolvedTheme> FloorThemes;
		FloorThemes.Reserve(Layout.Floors.Num());
		for (const FFloor& Floor : Layout.Floors)
		{
			T66TowerThemeVisuals::FResolvedTheme Theme;
			T66TowerThemeVisuals::ResolveFloorTheme(World, Floor, Theme);
			FloorThemes.Add(MoveTemp(Theme));
		}
		for (int32 FloorIndex = 0; FloorIndex < Layout.Floors.Num(); ++FloorIndex)
		{
			const FFloor& Floor = Layout.Floors[FloorIndex];
			const T66TowerThemeVisuals::FResolvedTheme& Theme = FloorThemes[FloorIndex];
			const float CeilingBottomZ = T66ResolveFloorCeilingBottomZ(Layout, FloorThemes, FloorIndex);
			const float ModuleWallHeight = FMath::Max(600.0f, CeilingBottomZ - Floor.SurfaceZ);
			const TArray<FName> FloorTags = {
				Floor.FloorTag,
				FName(*FString::Printf(TEXT("T66_Floor_Tower_%02d"), Floor.FloorNumber))
			};

			T66SpawnShellWallsForFloor(World, CubeMesh, Layout, Floor, Theme, ModuleWallHeight, SpawnParams);
			T66SpawnPolygonFloor(World, CubeMesh, Theme.FloorMaterial, Layout, Floor, SpawnParams, FloorTags);
			T66SpawnMazeWalls(World, CubeMesh, Theme, Layout, Floor, ModuleWallHeight, SpawnParams);

			T66TowerMapTerrain::FFloor RoofGeometryFloor;
			float RoofSurfaceZ = 0.0f;
			float RoofThickness = 0.0f;
			bool bEnableRoofCollision = false;
			if (T66BuildFloorRoofSurface(Layout, FloorThemes, FloorIndex, RoofGeometryFloor, RoofSurfaceZ, RoofThickness, bEnableRoofCollision))
			{
				T66SpawnFloorRoofSurface(
					World,
					CubeMesh,
					Theme.RoofMaterial,
					Layout,
					RoofGeometryFloor,
					RoofSurfaceZ,
					RoofThickness,
					SpawnParams,
					bEnableRoofCollision,
					{
						T66TraversalBarrierTag,
						T66TowerCeilingTag,
						FName(*FString::Printf(TEXT("T66_Floor_Tower_%02d"), Floor.FloorNumber)),
						FName(TEXT("T66_Floor_Tower_Roof")),
						FName(*FString::Printf(TEXT("T66_Floor_Tower_Roof_%02d"), Floor.FloorNumber))
					});
			}
		}

		bOutCollisionReady = true;
		return true;
	}
}
