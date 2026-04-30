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
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"

namespace
{
	static const FName T66TowerMapTerrainVisualTag(TEXT("T66_MainMapTerrain_Visual"));
	static const FName T66TowerMapTerrainMaterialsReadyTag(TEXT("T66_MainMapTerrain_MaterialsReady"));
	static const FName T66TowerMapTerrainCollisionProxyTag(TEXT("T66_MainMapTerrain_CollisionProxy"));
	static const FName T66TowerMapTraversalBarrierTag(TEXT("T66_Map_TraversalBarrier"));
	static const FName T66TowerMapCeilingTag(TEXT("T66_Tower_Ceiling"));
	static const FName T66TowerMapFloorStartTag(TEXT("T66_Floor_Start"));
	static const FName T66TowerMapFloorMainTag(TEXT("T66_Floor_Main"));
	static const FName T66TowerMapFloorBossTag(TEXT("T66_Floor_Boss"));
	static constexpr int32 T66TowerFloorVertexCount = 4;
	static constexpr float T66TowerRoofSkinThickness = 12.0f;
	static constexpr float T66TowerStartFloorHeadroom = 2000.0f;
	static constexpr float T66TowerStandardFloorHeadroom = 1200.0f;
	static constexpr float T66TowerDungeonKitUnitSize = 1300.0f;
	static constexpr float T66TowerDungeonKitWallDepth = 120.0f;
	static constexpr float T66TowerGeneratedDungeonKitFloorThickness = 24.0f;
	static constexpr float T66TowerMazeWallHalfThicknessScale = (T66TowerDungeonKitWallDepth * 0.5f) / T66TowerDungeonKitUnitSize;
	static constexpr float T66TowerStartRoomSquareSize = T66TowerDungeonKitUnitSize * 5.0f;
	static constexpr int32 T66TowerGridDefaultColumns = 25;
	static constexpr int32 T66TowerGridDefaultRows = 25;
	static constexpr float T66TowerGridDefaultCellSize = T66TowerDungeonKitUnitSize;
	static constexpr float T66TowerGridDefaultDoorWidth = T66TowerDungeonKitUnitSize;
	static constexpr int32 T66TowerDungeonMinRooms = 15;
	static constexpr int32 T66TowerDungeonMaxRooms = 20;
	static constexpr int32 T66TowerDungeonMinRoomTiles = 2;
	static constexpr int32 T66TowerDungeonMaxRoomTiles = 5;
	static constexpr float T66TowerGridBranchChance = 0.35f;
	static constexpr int32 T66TowerGridMaxBranchCells = 3;
	static constexpr int32 T66TowerStartFloorNumber = 1;
	static constexpr int32 T66TowerFirstGameplayFloorNumber = 2;
	static constexpr int32 T66TowerLastGameplayFloorNumber = 4;
	static constexpr int32 T66TowerBossFloorNumber = 5;
	static constexpr int32 T66TowerTotalFloorCount = T66TowerBossFloorNumber - T66TowerStartFloorNumber + 1;
	static TAutoConsoleVariable<int32> CVarT66TowerIgnoreCameraCollision(
		TEXT("T66.Camera.IgnoreTowerWallCameraCollision"),
		1,
		TEXT("0 lets tower walls block camera traces, 1 ignores the camera channel for fixed-distance tower gameplay cameras."),
		ECVF_Default);

	static TAutoConsoleVariable<int32> CVarT66TowerMazeMode(
		TEXT("T66.Tower.MazeMode"),
		1,
		TEXT("0 = legacy lane maze, 1 = grid-graph maze."),
		ECVF_Default);

	static TAutoConsoleVariable<int32> CVarT66TowerUseGeneratedDungeonKit(
		TEXT("T66.Tower.UseGeneratedDungeonKit"),
		1,
		TEXT("0 uses legacy material-only dungeon wall cubes, 1 uses generated DungeonKit01 visuals with lightweight collision proxies."),
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

	static void T66ConfigureTowerCollisionResponses(UPrimitiveComponent* PrimitiveComponent, const bool bIgnoreCameraChannel = false)
	{
		if (!PrimitiveComponent)
		{
			return;
		}

		PrimitiveComponent->SetCollisionResponseToChannel(ECC_Camera, bIgnoreCameraChannel ? ECR_Ignore : ECR_Block);
	}

	static bool T66ShouldIgnoreTowerWallCameraCollision()
	{
		return CVarT66TowerIgnoreCameraCollision.GetValueOnAnyThread() != 0;
	}

	static bool T66ShouldUseGeneratedDungeonKit()
	{
		return CVarT66TowerUseGeneratedDungeonKit.GetValueOnAnyThread() != 0;
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
			MeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
			T66ConfigureTowerCollisionResponses(MeshComponent, bIgnoreCameraChannel);
			MeshComponent->SetCollisionEnabled(bEnableCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
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

		Actor->Tags.AddUnique(T66TowerMapTerrainVisualTag);
		Actor->Tags.AddUnique(T66TowerMapTerrainMaterialsReadyTag);
		for (const FName& Tag : ExtraTags)
		{
			if (!Tag.IsNone())
			{
				Actor->Tags.AddUnique(Tag);
			}
		}
		return Actor;
	}

	static AActor* T66SpawnHiddenCollisionProxyActor(
		UWorld* World,
		const FVector& Location,
		const FRotator& Rotation,
		const FVector& DesiredHalfExtents,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& ExtraTags,
		const bool bIgnoreCameraChannel = false)
	{
		if (!World)
		{
			return nullptr;
		}

		AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation, SpawnParams);
		if (!Actor)
		{
			return nullptr;
		}

		UBoxComponent* CollisionComponent = NewObject<UBoxComponent>(Actor, TEXT("CollisionProxy"));
		if (!CollisionComponent)
		{
			Actor->Destroy();
			return nullptr;
		}

		CollisionComponent->SetMobility(EComponentMobility::Movable);
		CollisionComponent->SetBoxExtent(DesiredHalfExtents, false);
		CollisionComponent->SetGenerateOverlapEvents(false);
		CollisionComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		T66ConfigureTowerCollisionResponses(CollisionComponent, bIgnoreCameraChannel);
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CollisionComponent->SetCanEverAffectNavigation(false);
		Actor->SetRootComponent(CollisionComponent);
		Actor->AddInstanceComponent(CollisionComponent);
		CollisionComponent->RegisterComponent();
		CollisionComponent->SetRelativeLocation(FVector::ZeroVector);
		CollisionComponent->SetRelativeRotation(FRotator::ZeroRotator);
		Actor->SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
		CollisionComponent->SetMobility(EComponentMobility::Static);

		Actor->SetActorHiddenInGame(true);
		Actor->Tags.AddUnique(T66TowerMapTerrainCollisionProxyTag);
		for (const FName& Tag : ExtraTags)
		{
			if (!Tag.IsNone())
			{
				Actor->Tags.AddUnique(Tag);
			}
		}
		return Actor;
	}

	static AActor* T66SpawnGroundedMeshActor(
		UWorld* World,
		UStaticMesh* Mesh,
		UMaterialInterface* Material,
		const FVector& Location,
		const FRotator& Rotation,
		const FVector& Scale,
		const FActorSpawnParameters& SpawnParams,
		bool bEnableCollision,
		const TArray<FName>& ExtraTags,
		const bool bIgnoreCameraChannel = false,
		const bool bCenterBoundsAtActorXY = false)
	{
		if (!World || !Mesh)
		{
			return nullptr;
		}

		AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation, SpawnParams);
		if (!Actor)
		{
			return nullptr;
		}

		USceneComponent* RootComponent = NewObject<USceneComponent>(Actor, TEXT("VisualRoot"));
		if (!RootComponent)
		{
			Actor->Destroy();
			return nullptr;
		}

		RootComponent->SetMobility(EComponentMobility::Movable);
		Actor->SetRootComponent(RootComponent);
		Actor->AddInstanceComponent(RootComponent);
		RootComponent->RegisterComponent();
		Actor->SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);

		UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(Actor, TEXT("VisualMesh"));
		if (!MeshComponent)
		{
			Actor->Destroy();
			return nullptr;
		}

		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->SetupAttachment(RootComponent);
		MeshComponent->SetStaticMesh(Mesh);
		MeshComponent->SetGenerateOverlapEvents(false);
		MeshComponent->SetRelativeScale3D(Scale);
		FT66VisualUtil::GroundMeshToActorOrigin(MeshComponent, Mesh);
		if (bCenterBoundsAtActorXY)
		{
			const FBoxSphereBounds Bounds = Mesh->GetBounds();
			const FVector RelativeLocation = MeshComponent->GetRelativeLocation();
			MeshComponent->SetRelativeLocation(FVector(
				-Bounds.Origin.X * Scale.X,
				-Bounds.Origin.Y * Scale.Y,
				RelativeLocation.Z));
		}
		MeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		T66ConfigureTowerCollisionResponses(MeshComponent, bIgnoreCameraChannel);
		MeshComponent->SetCollisionEnabled(bEnableCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
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
		Actor->AddInstanceComponent(MeshComponent);
		MeshComponent->RegisterComponent();
		MeshComponent->SetMobility(EComponentMobility::Static);
		RootComponent->SetMobility(EComponentMobility::Static);

		Actor->Tags.AddUnique(T66TowerMapTerrainVisualTag);
		Actor->Tags.AddUnique(T66TowerMapTerrainMaterialsReadyTag);
		for (const FName& Tag : ExtraTags)
		{
			if (!Tag.IsNone())
			{
				Actor->Tags.AddUnique(Tag);
			}
		}
		return Actor;
	}

	static float T66GetMeshAxisSize(UStaticMesh* Mesh, const int32 AxisIndex)
	{
		if (!Mesh)
		{
			return 1.0f;
		}

		const FVector Extents = Mesh->GetBounds().BoxExtent;
		if (AxisIndex == 0)
		{
			return FMath::Max(Extents.X * 2.0f, 1.0f);
		}
		if (AxisIndex == 1)
		{
			return FMath::Max(Extents.Y * 2.0f, 1.0f);
		}
		return FMath::Max(Extents.Z * 2.0f, 1.0f);
	}

	static int32 T66GetNativeDungeonKitModuleCount(const float SpanLength, const float NativeModuleSize)
	{
		if (SpanLength <= 10.0f || NativeModuleSize <= 1.0f)
		{
			return 0;
		}

		return FMath::Max(1, FMath::RoundToInt(SpanLength / NativeModuleSize));
	}

	static float T66GetNativeDungeonKitModuleStart(const float SpanMin, const float SpanMax, const int32 ModuleCount, const float NativeModuleSize)
	{
		const float SpanCenter = (SpanMin + SpanMax) * 0.5f;
		return SpanCenter - (static_cast<float>(ModuleCount) * NativeModuleSize * 0.5f);
	}

	static float T66YawForGeneratedWallNormal(const FVector2D& Normal)
	{
		return FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.X));
	}

	static UStaticMesh* T66ChooseGeneratedDungeonWallMesh(
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const int32 Seed,
		const int32 SideIndex,
		const int32 SegmentIndex)
	{
		if (Theme.WallMeshes.Num() <= 0)
		{
			return nullptr;
		}

		const int32 MeshIndex = FMath::Abs(Seed + (SideIndex * 977) + (SegmentIndex * 37)) % Theme.WallMeshes.Num();
		return Theme.WallMeshes.IsValidIndex(MeshIndex) ? Theme.WallMeshes[MeshIndex] : nullptr;
	}

	static int32 T66SpawnGeneratedDungeonWallVisualsForSide(
		UWorld* World,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const FBox2D& WallBox,
		const FVector2D& Normal,
		const float BaseZ,
		const float DesiredHeight,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags,
		const int32 Seed,
		const int32 SideIndex,
		const bool bSpawnCollision,
		const bool bIgnoreCameraChannel)
	{
		if (!World || Theme.WallMeshes.Num() <= 0)
		{
			return 0;
		}

		const FVector2D WallCenter = (WallBox.Min + WallBox.Max) * 0.5f;
		const FVector2D WallSize = WallBox.Max - WallBox.Min;
		const bool bWallRunsAlongX = WallSize.X >= WallSize.Y;
		const float SpanMin = bWallRunsAlongX ? WallBox.Min.X : WallBox.Min.Y;
		const float SpanMax = bWallRunsAlongX ? WallBox.Max.X : WallBox.Max.Y;
		const float FixedCoord = bWallRunsAlongX ? WallCenter.Y : WallCenter.X;
		const float WallDepth = FMath::Max(bWallRunsAlongX ? WallSize.Y : WallSize.X, 10.0f);
		const float SpanLength = SpanMax - SpanMin;
		if (SpanLength <= 10.0f)
		{
			return 0;
		}

		const float WallUnitLength = T66GetMeshAxisSize(Theme.WallMeshes[0], 1);
		const int32 SegmentCount = T66GetNativeDungeonKitModuleCount(SpanLength, WallUnitLength);
		if (SegmentCount <= 0)
		{
			return 0;
		}

		const float SegmentStart = T66GetNativeDungeonKitModuleStart(SpanMin, SpanMax, SegmentCount, WallUnitLength);
		TArray<FName> VisualTags = Tags;
		VisualTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit")));
		VisualTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_Wall")));
		TArray<FName> CollisionTags = Tags;
		CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit")));
		CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_CollisionProxy")));
		CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_WallCollision")));

		int32 SpawnedCount = 0;
		bool bSpawnedCollisionProxy = false;
		for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
		{
			UStaticMesh* WallMesh = T66ChooseGeneratedDungeonWallMesh(Theme, Seed, SideIndex, SegmentIndex);
			if (!WallMesh)
			{
				continue;
			}

			const float SegmentCenter = SegmentStart + (WallUnitLength * (static_cast<float>(SegmentIndex) + 0.5f));
			const FVector2D SegmentCenter2D = bWallRunsAlongX
				? FVector2D(SegmentCenter, FixedCoord)
				: FVector2D(FixedCoord, SegmentCenter);
			const FVector Location(SegmentCenter2D.X, SegmentCenter2D.Y, BaseZ);
			const FVector Scale(
				WallDepth / T66GetMeshAxisSize(WallMesh, 0),
				WallUnitLength / T66GetMeshAxisSize(WallMesh, 1),
				DesiredHeight / T66GetMeshAxisSize(WallMesh, 2));
			const FRotator Rotation(0.0f, T66YawForGeneratedWallNormal(Normal), 0.0f);

			if (T66SpawnGroundedMeshActor(
				World,
				WallMesh,
				nullptr,
				Location,
				Rotation,
				Scale,
				SpawnParams,
				false,
				VisualTags,
				bIgnoreCameraChannel,
				true))
			{
				++SpawnedCount;
				if (bSpawnCollision && !bSpawnedCollisionProxy)
				{
					const FVector CollisionCenter(WallCenter.X, WallCenter.Y, BaseZ + (DesiredHeight * 0.5f));
					const FVector CollisionHalfExtents = bWallRunsAlongX
						? FVector(SpanLength * 0.5f, WallDepth * 0.5f, DesiredHeight * 0.5f)
						: FVector(WallDepth * 0.5f, SpanLength * 0.5f, DesiredHeight * 0.5f);
					T66SpawnHiddenCollisionProxyActor(
						World,
						CollisionCenter,
						FRotator::ZeroRotator,
						CollisionHalfExtents,
						SpawnParams,
						CollisionTags,
						bIgnoreCameraChannel);
					bSpawnedCollisionProxy = true;
				}
			}
		}

		return SpawnedCount;
	}

	static bool T66SpawnGeneratedDungeonWallVisuals(
		UWorld* World,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const FBox2D& WallBox,
		const float BaseZ,
		const float DesiredHeight,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags,
		const int32 Seed,
		const bool bSingleSide,
		const FVector2D& SingleSideNormal,
		const bool bSpawnCollision,
		const bool bIgnoreCameraChannel)
	{
		if (!T66ShouldUseGeneratedDungeonKit() || !World || Theme.WallMeshes.Num() <= 0)
		{
			return false;
		}

		const FVector2D WallCenter = (WallBox.Min + WallBox.Max) * 0.5f;
		const FVector2D WallHalfExtents = (WallBox.Max - WallBox.Min) * 0.5f;
		if (WallHalfExtents.X <= 5.0f || WallHalfExtents.Y <= 5.0f)
		{
			return false;
		}

		TArray<FVector2D, TInlineAllocator<2>> Normals;
		if (bSingleSide && !SingleSideNormal.IsNearlyZero())
		{
			Normals.Add(SingleSideNormal.GetSafeNormal());
		}
		else if (WallHalfExtents.X >= WallHalfExtents.Y)
		{
			Normals.Add(WallCenter.Y >= 0.0f ? FVector2D(0.0f, -1.0f) : FVector2D(0.0f, 1.0f));
		}
		else
		{
			Normals.Add(WallCenter.X >= 0.0f ? FVector2D(-1.0f, 0.0f) : FVector2D(1.0f, 0.0f));
		}

		int32 SpawnedCount = 0;
		for (int32 SideIndex = 0; SideIndex < Normals.Num(); ++SideIndex)
		{
			SpawnedCount += T66SpawnGeneratedDungeonWallVisualsForSide(
				World,
				Theme,
				WallBox,
				Normals[SideIndex],
				BaseZ,
				DesiredHeight,
				SpawnParams,
				Tags,
				Seed,
				SideIndex,
				bSpawnCollision,
				bIgnoreCameraChannel);
		}

		return SpawnedCount > 0;
	}

	static bool T66ShouldIgnoreTowerTraceHit(const FHitResult& Hit)
	{
		const AActor* HitActor = Hit.GetActor();
		return HitActor
			&& (HitActor->ActorHasTag(T66TowerMapCeilingTag) || HitActor->ActorHasTag(T66TowerMapTraversalBarrierTag));
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

	static bool T66IsLocationInsideWalkableFloorBoxes(const T66TowerMapTerrain::FFloor& Floor, const FVector& Location, const float Margin = 0.0f)
	{
		for (const FBox2D& WalkableBox : Floor.WalkableFloorBoxes)
		{
			if (Location.X >= (WalkableBox.Min.X - Margin)
				&& Location.X <= (WalkableBox.Max.X + Margin)
				&& Location.Y >= (WalkableBox.Min.Y - Margin)
				&& Location.Y <= (WalkableBox.Max.Y + Margin))
			{
				return true;
			}
		}

		return false;
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
		if (Floor.WalkableFloorBoxes.Num() > 0)
		{
			return T66IsLocationInsideWalkableFloorBoxes(Floor, Location, Margin);
		}

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

	static uint32 T66RotateLeft32(const uint32 Value, const uint32 Shift)
	{
		const uint32 MaskedShift = Shift & 31u;
		return MaskedShift == 0u ? Value : ((Value << MaskedShift) | (Value >> (32u - MaskedShift)));
	}

	static int32 T66BuildTowerFloorSeed(
		const int32 BaseSeed,
		const int32 FloorNumber,
		const int32 GameplayLevelNumber,
		const T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme)
	{
		uint32 Hash = static_cast<uint32>(BaseSeed);
		Hash ^= T66RotateLeft32(static_cast<uint32>(FMath::Max(0, GameplayLevelNumber) + 1) * 0x9E3779B1u, 11u);
		Hash ^= T66RotateLeft32(static_cast<uint32>(FloorNumber + 17) * 0x85EBCA6Bu, 17u);
		Hash ^= (static_cast<uint32>(Theme) + 3u) * 0xC2B2AE35u;
		Hash ^= Hash >> 16u;
		Hash *= 0x7FEB352Du;
		Hash ^= Hash >> 15u;
		Hash *= 0x846CA68Bu;
		Hash ^= Hash >> 16u;
		return static_cast<int32>(Hash & 0x7fffffffu);
	}

	static void T66ResetFloorMazeMetadata(T66TowerMapTerrain::FFloor& Floor)
	{
		Floor.ArrivalCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		Floor.ExitCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		Floor.GridCells.Reset();
		Floor.MainPathCells.Reset();
		Floor.OptionalCells.Reset();
		Floor.WalkableFloorBoxes.Reset();
		Floor.MazeWallBoxes.Reset();
		Floor.DoorwayHeaderBoxes.Reset();
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

	static bool T66AreGridIndicesAdjacent(const T66TowerMapTerrain::FLayout& Layout, const int32 A, const int32 B)
	{
		const FIntPoint Delta = T66GetGridCoordFromIndex(Layout, A) - T66GetGridCoordFromIndex(Layout, B);
		return (FMath::Abs(Delta.X) + FMath::Abs(Delta.Y)) == 1;
	}

	static void T66ShuffleGridDirections(FRandomStream& Rng, TArray<int32, TInlineAllocator<4>>& InOutDirectionOrder)
	{
		InOutDirectionOrder.Reset();
		InOutDirectionOrder.Add(0);
		InOutDirectionOrder.Add(1);
		InOutDirectionOrder.Add(2);
		InOutDirectionOrder.Add(3);
		for (int32 Index = InOutDirectionOrder.Num() - 1; Index > 0; --Index)
		{
			const int32 SwapIndex = Rng.RandRange(0, Index);
			InOutDirectionOrder.Swap(Index, SwapIndex);
		}
	}

	static bool T66TryBuildDungeonLoopRecursive(
		const T66TowerMapTerrain::FLayout& Layout,
		const int32 StartIndex,
		const int32 CurrentIndex,
		const int32 TargetRoomCount,
		TArray<bool>& Visited,
		TArray<int32>& InOutPath,
		FRandomStream& Rng)
	{
		if (InOutPath.Num() >= TargetRoomCount)
		{
			return T66AreGridIndicesAdjacent(Layout, CurrentIndex, StartIndex);
		}

		struct FScoredNeighbor
		{
			int32 Index = INDEX_NONE;
			float Score = 0.0f;
		};

		TArray<FScoredNeighbor, TInlineAllocator<4>> Candidates;
		TArray<int32, TInlineAllocator<4>> DirectionOrder;
		T66ShuffleGridDirections(Rng, DirectionOrder);

		const FIntPoint CurrentCoord = T66GetGridCoordFromIndex(Layout, CurrentIndex);
		const FIntPoint StartCoord = T66GetGridCoordFromIndex(Layout, StartIndex);
		const int32 ParentIndex = InOutPath.Num() >= 2 ? InOutPath[InOutPath.Num() - 2] : INDEX_NONE;
		const FIntPoint PreviousDelta = ParentIndex != INDEX_NONE
			? CurrentCoord - T66GetGridCoordFromIndex(Layout, ParentIndex)
			: FIntPoint::ZeroValue;

		for (const int32 DirectionIndex : DirectionOrder)
		{
			FIntPoint NeighborCoord;
			if (!T66TryGetGridNeighbor(Layout, CurrentCoord, DirectionIndex, NeighborCoord))
			{
				continue;
			}

			const int32 NeighborIndex = T66GetGridCellIndex(Layout, NeighborCoord);
			if (NeighborIndex == StartIndex || !Visited.IsValidIndex(NeighborIndex) || Visited[NeighborIndex])
			{
				continue;
			}

			const FIntPoint NextDelta = NeighborCoord - CurrentCoord;
			const bool bTurns = ParentIndex == INDEX_NONE || NextDelta != PreviousDelta;
			const int32 StartDistance = T66GridManhattanDistance(NeighborCoord, StartCoord);
			const int32 RemainingAfterThis = TargetRoomCount - (InOutPath.Num() + 1);
			if (RemainingAfterThis <= 0 && StartDistance != 1)
			{
				continue;
			}

			FScoredNeighbor Candidate;
			Candidate.Index = NeighborIndex;
			Candidate.Score = Rng.FRandRange(0.0f, 1.0f)
				+ (bTurns ? 0.45f : 0.0f)
				+ (StartDistance <= RemainingAfterThis + 1 ? 0.25f : 0.0f);
			Candidates.Add(Candidate);
		}

		Candidates.Sort([](const FScoredNeighbor& A, const FScoredNeighbor& B)
		{
			return A.Score > B.Score;
		});

		for (const FScoredNeighbor& Candidate : Candidates)
		{
			Visited[Candidate.Index] = true;
			InOutPath.Add(Candidate.Index);
			if (T66TryBuildDungeonLoopRecursive(Layout, StartIndex, Candidate.Index, TargetRoomCount, Visited, InOutPath, Rng))
			{
				return true;
			}

			InOutPath.Pop();
			Visited[Candidate.Index] = false;
		}

		return false;
	}

	static bool T66TryBuildDungeonLoopPath(
		const T66TowerMapTerrain::FLayout& Layout,
		const int32 StartIndex,
		FRandomStream& Rng,
		TArray<int32>& OutPath)
	{
		const int32 CellCount = Layout.GridColumns * Layout.GridRows;
		if (CellCount <= 0 || StartIndex < 0 || StartIndex >= CellCount)
		{
			return false;
		}

		for (int32 Attempt = 0; Attempt < 24; ++Attempt)
		{
			const int32 TargetRoomCount = T66TowerDungeonMinRooms + (Rng.RandRange(0, (T66TowerDungeonMaxRooms - T66TowerDungeonMinRooms) / 2) * 2);
			TArray<bool> Visited;
			Visited.Init(false, CellCount);
			Visited[StartIndex] = true;

			OutPath.Reset();
			OutPath.Add(StartIndex);
			if (T66TryBuildDungeonLoopRecursive(Layout, StartIndex, StartIndex, TargetRoomCount, Visited, OutPath, Rng))
			{
				return true;
			}
		}

		return false;
	}

	static bool T66TryBuildRectangularDungeonLoopPath(
		const T66TowerMapTerrain::FLayout& Layout,
		const int32 StartIndex,
		FRandomStream& Rng,
		TArray<int32>& OutPath)
	{
		struct FLoopProfile
		{
			int32 Width = 0;
			int32 Height = 0;
		};

		const FLoopProfile Profiles[] =
		{
			{ 6, 6 },
			{ 6, 5 },
			{ 5, 6 },
			{ 5, 5 },
		};

		const FIntPoint StartCoord = T66GetGridCoordFromIndex(Layout, StartIndex);
		TArray<FLoopProfile, TInlineAllocator<4>> ValidProfiles;
		for (const FLoopProfile& Profile : Profiles)
		{
			if (Profile.Width <= 1 || Profile.Height <= 1 || Profile.Width > Layout.GridColumns || Profile.Height > Layout.GridRows)
			{
				continue;
			}

			bool bHasValidPlacement = false;
			for (int32 MinY = 0; MinY <= Layout.GridRows - Profile.Height && !bHasValidPlacement; ++MinY)
			{
				for (int32 MinX = 0; MinX <= Layout.GridColumns - Profile.Width; ++MinX)
				{
					const int32 MaxX = MinX + Profile.Width - 1;
					const int32 MaxY = MinY + Profile.Height - 1;
					const bool bInside = StartCoord.X >= MinX && StartCoord.X <= MaxX && StartCoord.Y >= MinY && StartCoord.Y <= MaxY;
					const bool bOnPerimeter = bInside && (StartCoord.X == MinX || StartCoord.X == MaxX || StartCoord.Y == MinY || StartCoord.Y == MaxY);
					if (bOnPerimeter)
					{
						bHasValidPlacement = true;
						break;
					}
				}
			}

			if (bHasValidPlacement)
			{
				ValidProfiles.Add(Profile);
			}
		}

		if (ValidProfiles.Num() <= 0)
		{
			return false;
		}

		const FLoopProfile ChosenProfile = ValidProfiles[Rng.RandRange(0, ValidProfiles.Num() - 1)];
		TArray<FIntPoint, TInlineAllocator<32>> CandidateMins;
		for (int32 MinY = 0; MinY <= Layout.GridRows - ChosenProfile.Height; ++MinY)
		{
			for (int32 MinX = 0; MinX <= Layout.GridColumns - ChosenProfile.Width; ++MinX)
			{
				const int32 MaxX = MinX + ChosenProfile.Width - 1;
				const int32 MaxY = MinY + ChosenProfile.Height - 1;
				const bool bInside = StartCoord.X >= MinX && StartCoord.X <= MaxX && StartCoord.Y >= MinY && StartCoord.Y <= MaxY;
				const bool bOnPerimeter = bInside && (StartCoord.X == MinX || StartCoord.X == MaxX || StartCoord.Y == MinY || StartCoord.Y == MaxY);
				if (bOnPerimeter)
				{
					CandidateMins.Add(FIntPoint(MinX, MinY));
				}
			}
		}

		if (CandidateMins.Num() <= 0)
		{
			return false;
		}

		const FIntPoint MinCoord = CandidateMins[Rng.RandRange(0, CandidateMins.Num() - 1)];
		const int32 MaxX = MinCoord.X + ChosenProfile.Width - 1;
		const int32 MaxY = MinCoord.Y + ChosenProfile.Height - 1;
		TArray<FIntPoint, TInlineAllocator<32>> RingCoords;
		for (int32 X = MinCoord.X; X <= MaxX; ++X)
		{
			RingCoords.Add(FIntPoint(X, MinCoord.Y));
		}
		for (int32 Y = MinCoord.Y + 1; Y <= MaxY; ++Y)
		{
			RingCoords.Add(FIntPoint(MaxX, Y));
		}
		for (int32 X = MaxX - 1; X >= MinCoord.X; --X)
		{
			RingCoords.Add(FIntPoint(X, MaxY));
		}
		for (int32 Y = MaxY - 1; Y > MinCoord.Y; --Y)
		{
			RingCoords.Add(FIntPoint(MinCoord.X, Y));
		}

		int32 StartPathIndex = INDEX_NONE;
		for (int32 PathIndex = 0; PathIndex < RingCoords.Num(); ++PathIndex)
		{
			if (RingCoords[PathIndex] == StartCoord)
			{
				StartPathIndex = PathIndex;
				break;
			}
		}

		if (StartPathIndex == INDEX_NONE)
		{
			return false;
		}

		OutPath.Reset();
		OutPath.Reserve(RingCoords.Num());
		const bool bClockwise = Rng.FRand() >= 0.5f;
		for (int32 Offset = 0; Offset < RingCoords.Num(); ++Offset)
		{
			const int32 RingIndex = bClockwise
				? (StartPathIndex + Offset) % RingCoords.Num()
				: (StartPathIndex - Offset + RingCoords.Num()) % RingCoords.Num();
			OutPath.Add(T66GetGridCellIndex(Layout, RingCoords[RingIndex]));
		}

		return OutPath.Num() >= T66TowerDungeonMinRooms && OutPath.Num() <= T66TowerDungeonMaxRooms;
	}

	static bool T66BuildFallbackDungeonRing(
		const T66TowerMapTerrain::FLayout& Layout,
		const int32 StartIndex,
		FRandomStream& Rng,
		TArray<int32>& OutPath)
	{
		const FIntPoint StartCoord = T66GetGridCoordFromIndex(Layout, StartIndex);
		TArray<FIntPoint, TInlineAllocator<8>> OffsetOrder;
		for (int32 OffsetY = 0; OffsetY < 3; ++OffsetY)
		{
			for (int32 OffsetX = 0; OffsetX < 3; ++OffsetX)
			{
				if (OffsetX == 1 && OffsetY == 1)
				{
					continue;
				}

				OffsetOrder.Add(FIntPoint(OffsetX, OffsetY));
			}
		}

		for (int32 Index = OffsetOrder.Num() - 1; Index > 0; --Index)
		{
			const int32 SwapIndex = Rng.RandRange(0, Index);
			OffsetOrder.Swap(Index, SwapIndex);
		}

		for (const FIntPoint& StartOffset : OffsetOrder)
		{
			const FIntPoint MinCoord(StartCoord.X - StartOffset.X, StartCoord.Y - StartOffset.Y);
			if (MinCoord.X < 0 || MinCoord.Y < 0 || MinCoord.X + 2 >= Layout.GridColumns || MinCoord.Y + 2 >= Layout.GridRows)
			{
				continue;
			}

			const FIntPoint RingCoords[] =
			{
				FIntPoint(MinCoord.X, MinCoord.Y),
				FIntPoint(MinCoord.X + 1, MinCoord.Y),
				FIntPoint(MinCoord.X + 2, MinCoord.Y),
				FIntPoint(MinCoord.X + 2, MinCoord.Y + 1),
				FIntPoint(MinCoord.X + 2, MinCoord.Y + 2),
				FIntPoint(MinCoord.X + 1, MinCoord.Y + 2),
				FIntPoint(MinCoord.X, MinCoord.Y + 2),
				FIntPoint(MinCoord.X, MinCoord.Y + 1),
			};

			int32 StartPathIndex = INDEX_NONE;
			for (int32 PathIndex = 0; PathIndex < UE_ARRAY_COUNT(RingCoords); ++PathIndex)
			{
				if (RingCoords[PathIndex] == StartCoord)
				{
					StartPathIndex = PathIndex;
					break;
				}
			}

			if (StartPathIndex == INDEX_NONE)
			{
				continue;
			}

			OutPath.Reset();
			OutPath.Reserve(UE_ARRAY_COUNT(RingCoords));
			for (int32 Offset = 0; Offset < UE_ARRAY_COUNT(RingCoords); ++Offset)
			{
				const FIntPoint Coord = RingCoords[(StartPathIndex + Offset) % UE_ARRAY_COUNT(RingCoords)];
				OutPath.Add(T66GetGridCellIndex(Layout, Coord));
			}

			return true;
		}

		return false;
	}

	static void T66UpdateFloorBoundsFromWalkableBoxes(T66TowerMapTerrain::FFloor& Floor)
	{
		if (Floor.WalkableFloorBoxes.Num() <= 0)
		{
			return;
		}

		float MinX = TNumericLimits<float>::Max();
		float MinY = TNumericLimits<float>::Max();
		float MaxX = TNumericLimits<float>::Lowest();
		float MaxY = TNumericLimits<float>::Lowest();
		for (const FBox2D& WalkableBox : Floor.WalkableFloorBoxes)
		{
			MinX = FMath::Min(MinX, WalkableBox.Min.X);
			MinY = FMath::Min(MinY, WalkableBox.Min.Y);
			MaxX = FMath::Max(MaxX, WalkableBox.Max.X);
			MaxY = FMath::Max(MaxY, WalkableBox.Max.Y);
		}

		const float RequiredHalfExtent = FMath::Max(
			FMath::Max(FMath::Abs(MinX - Floor.Center.X), FMath::Abs(MaxX - Floor.Center.X)),
			FMath::Max(FMath::Abs(MinY - Floor.Center.Y), FMath::Abs(MaxY - Floor.Center.Y)));
		Floor.BoundsHalfExtent = FMath::Max(RequiredHalfExtent + 900.0f, 3600.0f);
		Floor.PolygonApothem = Floor.BoundsHalfExtent;
		Floor.WalkableHalfExtent = Floor.BoundsHalfExtent;
	}

	enum class ET66DungeonTileKind : uint8
	{
		Solid,
		Room,
		Corridor,
	};

	struct FT66DungeonRoom
	{
		int32 MinX = 0;
		int32 MinY = 0;
		int32 MaxX = 0;
		int32 MaxY = 0;

		FIntPoint Center() const
		{
			return FIntPoint((MinX + MaxX - 1) / 2, (MinY + MaxY - 1) / 2);
		}
	};

	struct FT66DungeonGraphEdge
	{
		int32 A = INDEX_NONE;
		int32 B = INDEX_NONE;
		float Weight = 0.0f;
		bool bSelected = false;
	};

	static bool T66DungeonRoomsOverlap(const FT66DungeonRoom& A, const FT66DungeonRoom& B, const int32 Padding)
	{
		return A.MinX < B.MaxX + Padding
			&& A.MaxX > B.MinX - Padding
			&& A.MinY < B.MaxY + Padding
			&& A.MaxY > B.MinY - Padding;
	}

	static bool T66DungeonRoomIsInsideGrid(const T66TowerMapTerrain::FLayout& Layout, const FT66DungeonRoom& Room)
	{
		return Room.MinX >= 1
			&& Room.MinY >= 1
			&& Room.MaxX <= Layout.GridColumns - 1
			&& Room.MaxY <= Layout.GridRows - 1
			&& Room.MaxX > Room.MinX
			&& Room.MaxY > Room.MinY;
	}

	static bool T66TryAddDungeonRoom(
		const T66TowerMapTerrain::FLayout& Layout,
		const TArray<FT66DungeonRoom>& ExistingRooms,
		const FT66DungeonRoom& Candidate,
		const int32 Padding)
	{
		if (!T66DungeonRoomIsInsideGrid(Layout, Candidate))
		{
			return false;
		}

		for (const FT66DungeonRoom& ExistingRoom : ExistingRooms)
		{
			if (T66DungeonRoomsOverlap(Candidate, ExistingRoom, Padding))
			{
				return false;
			}
		}

		return true;
	}

	static float T66DungeonRoomDistanceScore(const FT66DungeonRoom& Candidate, const TArray<FT66DungeonRoom>& ExistingRooms)
	{
		if (ExistingRooms.Num() <= 0)
		{
			return 10000.0f;
		}

		const FIntPoint CandidateCenter = Candidate.Center();
		float BestDistanceSq = TNumericLimits<float>::Max();
		for (const FT66DungeonRoom& ExistingRoom : ExistingRooms)
		{
			const FIntPoint ExistingCenter = ExistingRoom.Center();
			const float DeltaX = static_cast<float>(CandidateCenter.X - ExistingCenter.X);
			const float DeltaY = static_cast<float>(CandidateCenter.Y - ExistingCenter.Y);
			BestDistanceSq = FMath::Min(BestDistanceSq, (DeltaX * DeltaX) + (DeltaY * DeltaY));
		}

		return BestDistanceSq;
	}

	static bool T66TryFindScatteredDungeonRoom(
		const T66TowerMapTerrain::FLayout& Layout,
		const TArray<FT66DungeonRoom>& ExistingRooms,
		FRandomStream& Rng,
		const int32 Padding,
		FT66DungeonRoom& OutRoom)
	{
		float BestScore = -1.0f;
		bool bFound = false;
		for (int32 SampleIndex = 0; SampleIndex < 48; ++SampleIndex)
		{
			const int32 Width = Rng.RandRange(T66TowerDungeonMinRoomTiles, T66TowerDungeonMaxRoomTiles);
			const int32 Height = Rng.RandRange(T66TowerDungeonMinRoomTiles, T66TowerDungeonMaxRoomTiles);
			const int32 MaxMinX = FMath::Max(1, Layout.GridColumns - Width - 1);
			const int32 MaxMinY = FMath::Max(1, Layout.GridRows - Height - 1);
			FT66DungeonRoom Candidate;
			Candidate.MinX = Rng.RandRange(1, MaxMinX);
			Candidate.MinY = Rng.RandRange(1, MaxMinY);
			Candidate.MaxX = Candidate.MinX + Width;
			Candidate.MaxY = Candidate.MinY + Height;
			if (!T66TryAddDungeonRoom(Layout, ExistingRooms, Candidate, Padding))
			{
				continue;
			}

			const float Score = T66DungeonRoomDistanceScore(Candidate, ExistingRooms) + Rng.FRandRange(0.0f, 3.0f);
			if (!bFound || Score > BestScore)
			{
				bFound = true;
				BestScore = Score;
				OutRoom = Candidate;
			}
		}

		return bFound;
	}

	static bool T66BuildDungeonRoomSet(
		const T66TowerMapTerrain::FLayout& Layout,
		const FIntPoint& ArrivalCoord,
		FRandomStream& Rng,
		TArray<FT66DungeonRoom>& OutRooms)
	{
		OutRooms.Reset();

		const int32 TargetRoomCount = Rng.RandRange(T66TowerDungeonMinRooms, T66TowerDungeonMaxRooms);
		const int32 StartRoomWidth = Rng.RandRange(3, 4);
		const int32 StartRoomHeight = Rng.RandRange(3, 4);
		FT66DungeonRoom StartRoom;
		StartRoom.MinX = FMath::Clamp(ArrivalCoord.X - (StartRoomWidth / 2), 1, Layout.GridColumns - StartRoomWidth - 1);
		StartRoom.MinY = FMath::Clamp(ArrivalCoord.Y - (StartRoomHeight / 2), 1, Layout.GridRows - StartRoomHeight - 1);
		StartRoom.MaxX = StartRoom.MinX + StartRoomWidth;
		StartRoom.MaxY = StartRoom.MinY + StartRoomHeight;
		if (!T66DungeonRoomIsInsideGrid(Layout, StartRoom))
		{
			return false;
		}

		OutRooms.Add(StartRoom);
		for (int32 Attempt = 0; Attempt < 420 && OutRooms.Num() < TargetRoomCount; ++Attempt)
		{
			FT66DungeonRoom Candidate;
			if (T66TryFindScatteredDungeonRoom(Layout, OutRooms, Rng, 1, Candidate))
			{
				OutRooms.Add(Candidate);
			}
		}

		for (int32 Attempt = 0; Attempt < 220 && OutRooms.Num() < T66TowerDungeonMinRooms; ++Attempt)
		{
			FT66DungeonRoom Candidate;
			if (T66TryFindScatteredDungeonRoom(Layout, OutRooms, Rng, 0, Candidate))
			{
				OutRooms.Add(Candidate);
			}
		}

		return OutRooms.Num() >= T66TowerDungeonMinRooms;
	}

	static bool T66BuildDungeonRoomGraph(
		const TArray<FT66DungeonRoom>& Rooms,
		FRandomStream& Rng,
		TArray<FT66DungeonGraphEdge>& OutEdges,
		TArray<int32>& OutDegree)
	{
		const int32 RoomCount = Rooms.Num();
		if (RoomCount < 2)
		{
			return false;
		}

		OutEdges.Reset();
		OutDegree.Init(0, RoomCount);
		for (int32 A = 0; A < RoomCount; ++A)
		{
			for (int32 B = A + 1; B < RoomCount; ++B)
			{
				const int32 Distance = T66GridManhattanDistance(Rooms[A].Center(), Rooms[B].Center());
				FT66DungeonGraphEdge& Edge = OutEdges.AddDefaulted_GetRef();
				Edge.A = A;
				Edge.B = B;
				Edge.Weight = static_cast<float>(Distance) + Rng.FRandRange(0.0f, 0.25f);
			}
		}

		TArray<bool> bConnected;
		bConnected.Init(false, RoomCount);
		bConnected[0] = true;
		int32 ConnectedCount = 1;
		while (ConnectedCount < RoomCount)
		{
			int32 BestEdgeIndex = INDEX_NONE;
			float BestWeight = TNumericLimits<float>::Max();
			for (int32 EdgeIndex = 0; EdgeIndex < OutEdges.Num(); ++EdgeIndex)
			{
				const FT66DungeonGraphEdge& Edge = OutEdges[EdgeIndex];
				const bool bAConnected = bConnected.IsValidIndex(Edge.A) && bConnected[Edge.A];
				const bool bBConnected = bConnected.IsValidIndex(Edge.B) && bConnected[Edge.B];
				if (bAConnected == bBConnected || Edge.Weight >= BestWeight)
				{
					continue;
				}

				BestWeight = Edge.Weight;
				BestEdgeIndex = EdgeIndex;
			}

			if (!OutEdges.IsValidIndex(BestEdgeIndex))
			{
				return false;
			}

			FT66DungeonGraphEdge& ChosenEdge = OutEdges[BestEdgeIndex];
			ChosenEdge.bSelected = true;
			++OutDegree[ChosenEdge.A];
			++OutDegree[ChosenEdge.B];
			const int32 NewlyConnectedRoom = bConnected[ChosenEdge.A] ? ChosenEdge.B : ChosenEdge.A;
			bConnected[NewlyConnectedRoom] = true;
			++ConnectedCount;
		}

		TArray<int32> ExtraEdgeIndices;
		for (int32 EdgeIndex = 0; EdgeIndex < OutEdges.Num(); ++EdgeIndex)
		{
			if (!OutEdges[EdgeIndex].bSelected)
			{
				ExtraEdgeIndices.Add(EdgeIndex);
			}
		}
		ExtraEdgeIndices.Sort([&OutEdges](const int32 A, const int32 B)
		{
			return OutEdges[A].Weight < OutEdges[B].Weight;
		});

		const int32 ExtraBudget = FMath::Clamp(RoomCount / 4, 2, 6);
		int32 ExtrasAdded = 0;
		for (const int32 EdgeIndex : ExtraEdgeIndices)
		{
			FT66DungeonGraphEdge& Edge = OutEdges[EdgeIndex];
			if (ExtrasAdded >= ExtraBudget)
			{
				break;
			}
			if (OutDegree[Edge.A] >= 4 || OutDegree[Edge.B] >= 4)
			{
				continue;
			}
			if (Rng.FRand() > 0.38f)
			{
				continue;
			}

			Edge.bSelected = true;
			++OutDegree[Edge.A];
			++OutDegree[Edge.B];
			++ExtrasAdded;
		}

		return true;
	}

	static int32 T66ChooseDungeonExitRoom(const TArray<FT66DungeonRoom>& Rooms, const TArray<FT66DungeonGraphEdge>& Edges)
	{
		const int32 RoomCount = Rooms.Num();
		TArray<int32> Distance;
		Distance.Init(MAX_int32, RoomCount);
		TArray<int32> Queue;
		Distance[0] = 0;
		Queue.Add(0);
		for (int32 QueueIndex = 0; QueueIndex < Queue.Num(); ++QueueIndex)
		{
			const int32 CurrentRoom = Queue[QueueIndex];
			for (const FT66DungeonGraphEdge& Edge : Edges)
			{
				if (!Edge.bSelected)
				{
					continue;
				}

				const int32 NeighborRoom =
					(Edge.A == CurrentRoom) ? Edge.B :
					(Edge.B == CurrentRoom) ? Edge.A :
					INDEX_NONE;
				if (NeighborRoom == INDEX_NONE || !Distance.IsValidIndex(NeighborRoom) || Distance[NeighborRoom] != MAX_int32)
				{
					continue;
				}

				Distance[NeighborRoom] = Distance[CurrentRoom] + 1;
				Queue.Add(NeighborRoom);
			}
		}

		int32 BestRoom = RoomCount > 1 ? 1 : 0;
		int32 BestScore = MIN_int32;
		for (int32 RoomIndex = 1; RoomIndex < RoomCount; ++RoomIndex)
		{
			const int32 GraphDistance = Distance.IsValidIndex(RoomIndex) && Distance[RoomIndex] != MAX_int32 ? Distance[RoomIndex] : 0;
			const int32 SpatialDistance = T66GridManhattanDistance(Rooms[0].Center(), Rooms[RoomIndex].Center());
			const int32 Score = (GraphDistance * 100) + SpatialDistance;
			if (Score > BestScore)
			{
				BestScore = Score;
				BestRoom = RoomIndex;
			}
		}

		return BestRoom;
	}

	static bool T66IsDungeonInteriorCoord(const T66TowerMapTerrain::FLayout& Layout, const FIntPoint& Coord)
	{
		return Coord.X > 0
			&& Coord.Y > 0
			&& Coord.X < Layout.GridColumns - 1
			&& Coord.Y < Layout.GridRows - 1;
	}

	static void T66CarveFallbackDungeonCorridor(
		const T66TowerMapTerrain::FLayout& Layout,
		const FIntPoint& Start,
		const FIntPoint& Goal,
		FRandomStream& Rng,
		TArray<ET66DungeonTileKind>& InOutTiles)
	{
		FIntPoint Cursor = Start;
		const bool bHorizontalFirst = Rng.FRand() >= 0.5f;
		auto MarkCursor = [&]()
		{
			if (!T66IsDungeonInteriorCoord(Layout, Cursor))
			{
				return;
			}

			const int32 Index = T66GetGridCellIndex(Layout, Cursor);
			if (InOutTiles.IsValidIndex(Index) && InOutTiles[Index] == ET66DungeonTileKind::Solid)
			{
				InOutTiles[Index] = ET66DungeonTileKind::Corridor;
			}
		};

		auto StepX = [&]()
		{
			while (Cursor.X != Goal.X)
			{
				Cursor.X += (Goal.X > Cursor.X) ? 1 : -1;
				MarkCursor();
			}
		};
		auto StepY = [&]()
		{
			while (Cursor.Y != Goal.Y)
			{
				Cursor.Y += (Goal.Y > Cursor.Y) ? 1 : -1;
				MarkCursor();
			}
		};

		if (bHorizontalFirst)
		{
			StepX();
			StepY();
		}
		else
		{
			StepY();
			StepX();
		}
	}

	static bool T66CarveDungeonCorridor(
		const T66TowerMapTerrain::FLayout& Layout,
		const TArray<FT66DungeonRoom>& Rooms,
		const int32 StartRoomIndex,
		const int32 GoalRoomIndex,
		FRandomStream& Rng,
		TArray<ET66DungeonTileKind>& InOutTiles,
		const TArray<int32>& RoomIdByTile)
	{
		if (!Rooms.IsValidIndex(StartRoomIndex) || !Rooms.IsValidIndex(GoalRoomIndex))
		{
			return false;
		}

		const FIntPoint Start = Rooms[StartRoomIndex].Center();
		const FIntPoint Goal = Rooms[GoalRoomIndex].Center();
		const int32 CellCount = Layout.GridColumns * Layout.GridRows;
		const int32 StartIndex = T66GetGridCellIndex(Layout, Start);
		const int32 GoalIndex = T66GetGridCellIndex(Layout, Goal);
		if (!InOutTiles.IsValidIndex(StartIndex) || !InOutTiles.IsValidIndex(GoalIndex))
		{
			return false;
		}

		TArray<float> GScore;
		TArray<float> FScore;
		TArray<int32> Parent;
		TArray<int32> ParentDirection;
		TArray<bool> bClosed;
		TArray<int32> Open;
		GScore.Init(TNumericLimits<float>::Max(), CellCount);
		FScore.Init(TNumericLimits<float>::Max(), CellCount);
		Parent.Init(INDEX_NONE, CellCount);
		ParentDirection.Init(INDEX_NONE, CellCount);
		bClosed.Init(false, CellCount);
		GScore[StartIndex] = 0.0f;
		FScore[StartIndex] = static_cast<float>(T66GridManhattanDistance(Start, Goal));
		Open.Add(StartIndex);

		static const FIntPoint Deltas[] =
		{
			FIntPoint(0, -1),
			FIntPoint(1, 0),
			FIntPoint(0, 1),
			FIntPoint(-1, 0),
		};

		while (Open.Num() > 0)
		{
			int32 BestOpenSlot = 0;
			float BestScore = FScore[Open[0]];
			for (int32 OpenSlot = 1; OpenSlot < Open.Num(); ++OpenSlot)
			{
				const float CandidateScore = FScore[Open[OpenSlot]];
				if (CandidateScore < BestScore)
				{
					BestScore = CandidateScore;
					BestOpenSlot = OpenSlot;
				}
			}

			const int32 CurrentIndex = Open[BestOpenSlot];
			Open.RemoveAtSwap(BestOpenSlot);
			if (CurrentIndex == GoalIndex)
			{
				int32 PathIndex = GoalIndex;
				while (PathIndex != INDEX_NONE && PathIndex != StartIndex)
				{
					if (InOutTiles.IsValidIndex(PathIndex) && InOutTiles[PathIndex] == ET66DungeonTileKind::Solid)
					{
						InOutTiles[PathIndex] = ET66DungeonTileKind::Corridor;
					}
					PathIndex = Parent.IsValidIndex(PathIndex) ? Parent[PathIndex] : INDEX_NONE;
				}
				return true;
			}

			if (!bClosed.IsValidIndex(CurrentIndex) || bClosed[CurrentIndex])
			{
				continue;
			}
			bClosed[CurrentIndex] = true;

			const FIntPoint CurrentCoord = T66GetGridCoordFromIndex(Layout, CurrentIndex);
			for (int32 DirectionIndex = 0; DirectionIndex < UE_ARRAY_COUNT(Deltas); ++DirectionIndex)
			{
				const FIntPoint NeighborCoord = CurrentCoord + Deltas[DirectionIndex];
				if (!T66IsDungeonInteriorCoord(Layout, NeighborCoord) && NeighborCoord != Goal)
				{
					continue;
				}

				const int32 NeighborIndex = T66GetGridCellIndex(Layout, NeighborCoord);
				if (!InOutTiles.IsValidIndex(NeighborIndex) || (bClosed.IsValidIndex(NeighborIndex) && bClosed[NeighborIndex]))
				{
					continue;
				}

				float StepCost = 1.0f;
				if (InOutTiles[NeighborIndex] == ET66DungeonTileKind::Corridor)
				{
					StepCost = 0.35f;
				}
				else if (InOutTiles[NeighborIndex] == ET66DungeonTileKind::Room)
				{
					const int32 RoomId = RoomIdByTile.IsValidIndex(NeighborIndex) ? RoomIdByTile[NeighborIndex] : INDEX_NONE;
					StepCost = (RoomId == StartRoomIndex || RoomId == GoalRoomIndex) ? 0.60f : 42.0f;
				}

				if (ParentDirection.IsValidIndex(CurrentIndex)
					&& ParentDirection[CurrentIndex] != INDEX_NONE
					&& ParentDirection[CurrentIndex] != DirectionIndex)
				{
					StepCost += 0.15f;
				}
				StepCost += Rng.FRandRange(0.0f, 0.025f);

				const float TentativeScore = GScore[CurrentIndex] + StepCost;
				if (!GScore.IsValidIndex(NeighborIndex) || TentativeScore >= GScore[NeighborIndex])
				{
					continue;
				}

				Parent[NeighborIndex] = CurrentIndex;
				ParentDirection[NeighborIndex] = DirectionIndex;
				GScore[NeighborIndex] = TentativeScore;
				FScore[NeighborIndex] = TentativeScore + (static_cast<float>(T66GridManhattanDistance(NeighborCoord, Goal)) * 0.85f);
				Open.AddUnique(NeighborIndex);
			}
		}

		T66CarveFallbackDungeonCorridor(Layout, Start, Goal, Rng, InOutTiles);
		return true;
	}

	static void T66EmitDungeonTileEdgeWall(
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor,
		T66TowerMapTerrain::FGridCell& Cell,
		const int32 DirectionIndex,
		const bool bDoorway,
		const float HalfThickness)
	{
		const float DoorHalfWidth = FMath::Min(Layout.GridDoorWidth * 0.5f, (Layout.GridCellSize * 0.5f) - (HalfThickness * 1.5f));
		auto EmitAndTrack = [&](const float MinX, const float MinY, const float MaxX, const float MaxY)
		{
			if (MaxX <= MinX + KINDA_SMALL_NUMBER || MaxY <= MinY + KINDA_SMALL_NUMBER)
			{
				return;
			}

			const FBox2D Box(FVector2D(MinX, MinY), FVector2D(MaxX, MaxY));
			Floor.MazeWallBoxes.Add(Box);
			Floor.TrapEligibleWallBoxes.Add(Box);
			Cell.EmittedWallBoxes.Add(Box);
		};

		auto EmitHeader = [&](const float MinX, const float MinY, const float MaxX, const float MaxY)
		{
			if (MaxX <= MinX + KINDA_SMALL_NUMBER || MaxY <= MinY + KINDA_SMALL_NUMBER)
			{
				return;
			}

			Floor.DoorwayHeaderBoxes.Add(FBox2D(FVector2D(MinX, MinY), FVector2D(MaxX, MaxY)));
		};

		if (DirectionIndex == 0 || DirectionIndex == 2)
		{
			const bool bNorth = DirectionIndex == 0;
			const float SideY = bNorth ? Cell.Bounds.Min.Y : Cell.Bounds.Max.Y;
			const float MinY = SideY - HalfThickness;
			const float MaxY = SideY + HalfThickness;
			const float MinX = Cell.Bounds.Min.X;
			const float MaxX = Cell.Bounds.Max.X;
			if (!bDoorway)
			{
				EmitAndTrack(MinX, MinY, MaxX, MaxY);
				return;
			}

			const float DoorCenterX = Cell.WorldCenter.X;
			EmitAndTrack(MinX, MinY, DoorCenterX - DoorHalfWidth, MaxY);
			EmitAndTrack(DoorCenterX + DoorHalfWidth, MinY, MaxX, MaxY);
			EmitHeader(DoorCenterX - DoorHalfWidth, MinY, DoorCenterX + DoorHalfWidth, MaxY);
			return;
		}

		const bool bWest = DirectionIndex == 3;
		const float SideX = bWest ? Cell.Bounds.Min.X : Cell.Bounds.Max.X;
		const float MinX = SideX - HalfThickness;
		const float MaxX = SideX + HalfThickness;
		const float MinY = Cell.Bounds.Min.Y;
		const float MaxY = Cell.Bounds.Max.Y;
		if (!bDoorway)
		{
			EmitAndTrack(MinX, MinY, MaxX, MaxY);
			return;
		}

		const float DoorCenterY = Cell.WorldCenter.Y;
		EmitAndTrack(MinX, MinY, MaxX, DoorCenterY - DoorHalfWidth);
		EmitAndTrack(MinX, DoorCenterY + DoorHalfWidth, MaxX, MaxY);
		EmitHeader(MinX, DoorCenterY - DoorHalfWidth, MaxX, DoorCenterY + DoorHalfWidth);
	}

	static bool T66IsDungeonDoorwayEdge(
		const int32 CurrentIndex,
		const int32 NeighborIndex,
		const TArray<ET66DungeonTileKind>& Tiles,
		const TArray<int32>& RoomIdByTile)
	{
		if (!Tiles.IsValidIndex(CurrentIndex) || !Tiles.IsValidIndex(NeighborIndex))
		{
			return false;
		}

		const ET66DungeonTileKind A = Tiles[CurrentIndex];
		const ET66DungeonTileKind B = Tiles[NeighborIndex];
		if ((A == ET66DungeonTileKind::Room && B == ET66DungeonTileKind::Corridor)
			|| (A == ET66DungeonTileKind::Corridor && B == ET66DungeonTileKind::Room))
		{
			return true;
		}

		return A == ET66DungeonTileKind::Room
			&& B == ET66DungeonTileKind::Room
			&& RoomIdByTile.IsValidIndex(CurrentIndex)
			&& RoomIdByTile.IsValidIndex(NeighborIndex)
			&& RoomIdByTile[CurrentIndex] != RoomIdByTile[NeighborIndex];
	}

	static bool T66BuildFloorDungeonLoop(
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
		if (!T66IsDungeonInteriorCoord(Layout, Floor.ArrivalCell))
		{
			Floor.ArrivalCell.X = FMath::Clamp(Floor.ArrivalCell.X, 1, Layout.GridColumns - 2);
			Floor.ArrivalCell.Y = FMath::Clamp(Floor.ArrivalCell.Y, 1, Layout.GridRows - 2);
		}

		TArray<FT66DungeonRoom> Rooms;
		if (!T66BuildDungeonRoomSet(Layout, Floor.ArrivalCell, Rng, Rooms))
		{
			return false;
		}

		TArray<ET66DungeonTileKind> Tiles;
		TArray<int32> RoomIdByTile;
		Tiles.Init(ET66DungeonTileKind::Solid, CellCount);
		RoomIdByTile.Init(INDEX_NONE, CellCount);

		for (int32 RoomIndex = 0; RoomIndex < Rooms.Num(); ++RoomIndex)
		{
			const FT66DungeonRoom& Room = Rooms[RoomIndex];
			for (int32 Y = Room.MinY; Y < Room.MaxY; ++Y)
			{
				for (int32 X = Room.MinX; X < Room.MaxX; ++X)
				{
					const int32 TileIndex = T66GetGridCellIndex(Layout, FIntPoint(X, Y));
					if (Tiles.IsValidIndex(TileIndex))
					{
						Tiles[TileIndex] = ET66DungeonTileKind::Room;
						RoomIdByTile[TileIndex] = RoomIndex;
					}
				}
			}
		}

		TArray<FT66DungeonGraphEdge> GraphEdges;
		TArray<int32> RoomDegree;
		if (!T66BuildDungeonRoomGraph(Rooms, Rng, GraphEdges, RoomDegree))
		{
			return false;
		}

		for (const FT66DungeonGraphEdge& Edge : GraphEdges)
		{
			if (!Edge.bSelected)
			{
				continue;
			}

			T66CarveDungeonCorridor(Layout, Rooms, Edge.A, Edge.B, Rng, Tiles, RoomIdByTile);
		}

		const int32 ArrivalIndex = T66GetGridCellIndex(Layout, Floor.ArrivalCell);
		const int32 ExitRoomIndex = T66ChooseDungeonExitRoom(Rooms, GraphEdges);
		Floor.ExitCell = Rooms.IsValidIndex(ExitRoomIndex) ? Rooms[ExitRoomIndex].Center() : Floor.ArrivalCell;
		const int32 ExitIndex = T66GetGridCellIndex(Layout, Floor.ExitCell);
		if (!Floor.GridCells.IsValidIndex(ArrivalIndex) || !Floor.GridCells.IsValidIndex(ExitIndex))
		{
			return false;
		}

		Floor.HoleCenter = Floor.GridCells[ExitIndex].WorldCenter;
		Floor.HoleCenter.Z = Floor.SurfaceZ;
		Floor.ExitPoint = Floor.HoleCenter;
		Floor.ExitPoint.Z = Floor.SurfaceZ;

		int32 MainPathIndex = 0;
		for (int32 CellIndex = 0; CellIndex < Floor.GridCells.Num(); ++CellIndex)
		{
			if (!Tiles.IsValidIndex(CellIndex) || Tiles[CellIndex] == ET66DungeonTileKind::Solid)
			{
				continue;
			}

			T66TowerMapTerrain::FGridCell& Cell = Floor.GridCells[CellIndex];
			Cell.MainPathIndex = MainPathIndex++;
			Cell.bContainsArrival = CellIndex == ArrivalIndex;
			Cell.bContainsExit = CellIndex == ExitIndex;
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
		}

		for (int32 Y = 0; Y < Layout.GridRows; ++Y)
		{
			int32 RunStartX = INDEX_NONE;
			for (int32 X = 0; X <= Layout.GridColumns; ++X)
			{
				const bool bWalkable = X < Layout.GridColumns
					&& Tiles.IsValidIndex(T66GetGridCellIndex(Layout, FIntPoint(X, Y)))
					&& Tiles[T66GetGridCellIndex(Layout, FIntPoint(X, Y))] != ET66DungeonTileKind::Solid;
				if (bWalkable && RunStartX == INDEX_NONE)
				{
					RunStartX = X;
				}
				else if (!bWalkable && RunStartX != INDEX_NONE)
				{
					const FBox2D& StartBox = Floor.GridCells[T66GetGridCellIndex(Layout, FIntPoint(RunStartX, Y))].Bounds;
					const FBox2D& EndBox = Floor.GridCells[T66GetGridCellIndex(Layout, FIntPoint(X - 1, Y))].Bounds;
					Floor.WalkableFloorBoxes.Add(FBox2D(StartBox.Min, EndBox.Max));
					RunStartX = INDEX_NONE;
				}
			}
		}

		T66UpdateFloorBoundsFromWalkableBoxes(Floor);

		static const FIntPoint Deltas[] =
		{
			FIntPoint(0, -1),
			FIntPoint(1, 0),
			FIntPoint(0, 1),
			FIntPoint(-1, 0),
		};

		for (int32 CellIndex = 0; CellIndex < Floor.GridCells.Num(); ++CellIndex)
		{
			if (!Tiles.IsValidIndex(CellIndex) || Tiles[CellIndex] == ET66DungeonTileKind::Solid)
			{
				continue;
			}

			const FIntPoint Coord = T66GetGridCoordFromIndex(Layout, CellIndex);
			for (int32 DirectionIndex = 0; DirectionIndex < UE_ARRAY_COUNT(Deltas); ++DirectionIndex)
			{
				const FIntPoint NeighborCoord = Coord + Deltas[DirectionIndex];
				if (!T66IsValidGridCoord(Layout, NeighborCoord))
				{
					continue;
				}

				const int32 NeighborIndex = T66GetGridCellIndex(Layout, NeighborCoord);
				if (!Tiles.IsValidIndex(NeighborIndex) || Tiles[NeighborIndex] == ET66DungeonTileKind::Solid)
				{
					continue;
				}

				T66ConnectGridCells(Floor, Layout, CellIndex, NeighborIndex);
			}
		}

		const float HalfThickness = FMath::Max(Layout.WallThickness * 0.40f, Layout.PlacementCellSize * T66TowerMazeWallHalfThicknessScale);
		for (int32 CellIndex = 0; CellIndex < Floor.GridCells.Num(); ++CellIndex)
		{
			if (!Tiles.IsValidIndex(CellIndex) || Tiles[CellIndex] == ET66DungeonTileKind::Solid)
			{
				continue;
			}

			T66TowerMapTerrain::FGridCell& Cell = Floor.GridCells[CellIndex];
			Cell.Template = T66ResolveGridCellTemplate(Cell);
			for (int32 DirectionIndex = 0; DirectionIndex < UE_ARRAY_COUNT(Deltas); ++DirectionIndex)
			{
				const FIntPoint NeighborCoord = Cell.Coord + Deltas[DirectionIndex];
				const bool bNeighborValid = T66IsValidGridCoord(Layout, NeighborCoord);
				const int32 NeighborIndex = bNeighborValid ? T66GetGridCellIndex(Layout, NeighborCoord) : INDEX_NONE;
				const bool bNeighborWalkable = bNeighborValid
					&& Tiles.IsValidIndex(NeighborIndex)
					&& Tiles[NeighborIndex] != ET66DungeonTileKind::Solid;

				if (bNeighborWalkable)
				{
					if (DirectionIndex != 1 && DirectionIndex != 2)
					{
						continue;
					}

					if (T66IsDungeonDoorwayEdge(CellIndex, NeighborIndex, Tiles, RoomIdByTile))
					{
						T66EmitDungeonTileEdgeWall(Layout, Floor, Cell, DirectionIndex, true, HalfThickness);
					}
					continue;
				}

				T66EmitDungeonTileEdgeWall(Layout, Floor, Cell, DirectionIndex, false, HalfThickness);
			}
		}

		return true;
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
		FRandomStream GridRng = Rng;
		bBuiltGridGraph = T66BuildFloorDungeonLoop(Layout, Floor, GridRng);
		if (bBuiltGridGraph)
		{
			Rng = GridRng;
		}

		if (!bBuiltGridGraph && Layout.MazeMode == T66TowerMapTerrain::ET66TowerMazeMode::GridGraph)
		{
			T66ResetFloorMazeMetadata(Floor);
			FRandomStream FallbackGridRng = Rng;
			bBuiltGridGraph = T66BuildFloorMazeWalls_GridGraph(Layout, Floor, FallbackGridRng);
			if (bBuiltGridGraph)
			{
				Rng = FallbackGridRng;
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

		Floor.WalkableFloorBoxes.Add(FBox2D(FVector2D(MinX, MinY), FVector2D(MaxX, MaxY)));
		Floor.MazeWallBoxes.Add(FBox2D(FVector2D(MinX - HalfThickness, MinY), FVector2D(MinX + HalfThickness, MaxY)));
		Floor.MazeWallBoxes.Add(FBox2D(FVector2D(MaxX - HalfThickness, MinY), FVector2D(MaxX + HalfThickness, MaxY)));
		Floor.MazeWallBoxes.Add(FBox2D(FVector2D(MinX, MinY - HalfThickness), FVector2D(MaxX, MinY + HalfThickness)));
		Floor.MazeWallBoxes.Add(FBox2D(FVector2D(MinX, MaxY - HalfThickness), FVector2D(MaxX, MaxY + HalfThickness)));
		T66FinalizeFloorMazeMetadata(Layout, Floor);
	}

	static void T66BuildBossFloorRoom(
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor)
	{
		T66ResetFloorMazeMetadata(Floor);

		const float HalfThickness = FMath::Max(Layout.WallThickness * 0.25f, Layout.PlacementCellSize * T66TowerMazeWallHalfThicknessScale);
		const float RoomBoundsInset = Layout.WallThickness + (Layout.PlacementCellSize * 0.20f);
		const float RoomHalfExtent = FMath::Max(1400.0f, Floor.BoundsHalfExtent - RoomBoundsInset);
		const float MinX = Floor.Center.X - RoomHalfExtent;
		const float MaxX = Floor.Center.X + RoomHalfExtent;
		const float MinY = Floor.Center.Y - RoomHalfExtent;
		const float MaxY = Floor.Center.Y + RoomHalfExtent;

		Floor.WalkableFloorBoxes.Add(FBox2D(FVector2D(MinX, MinY), FVector2D(MaxX, MaxY)));
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

		auto SpawnRectWithHole = [&](const float MinX, const float MinY, const float MaxX, const float MaxY)
		{
			if (!Floor.bHasDropHole || Floor.HoleHalfExtent.X <= 1.0f || Floor.HoleHalfExtent.Y <= 1.0f)
			{
				SpawnRect(MinX, MinY, MaxX, MaxY);
				return;
			}

			const float HoleMinX = FMath::Clamp(Floor.HoleCenter.X - Floor.HoleHalfExtent.X, MinX, MaxX);
			const float HoleMaxX = FMath::Clamp(Floor.HoleCenter.X + Floor.HoleHalfExtent.X, MinX, MaxX);
			const float HoleMinY = FMath::Clamp(Floor.HoleCenter.Y - Floor.HoleHalfExtent.Y, MinY, MaxY);
			const float HoleMaxY = FMath::Clamp(Floor.HoleCenter.Y + Floor.HoleHalfExtent.Y, MinY, MaxY);
			if (HoleMaxX <= HoleMinX || HoleMaxY <= HoleMinY)
			{
				SpawnRect(MinX, MinY, MaxX, MaxY);
				return;
			}

			SpawnRect(MinX, MinY, HoleMinX, MaxY);
			SpawnRect(HoleMaxX, MinY, MaxX, MaxY);
			SpawnRect(HoleMinX, MinY, HoleMaxX, HoleMinY);
			SpawnRect(HoleMinX, HoleMaxY, HoleMaxX, MaxY);
		};

		if (Floor.WalkableFloorBoxes.Num() > 0)
		{
			for (const FBox2D& WalkableBox : Floor.WalkableFloorBoxes)
			{
				SpawnRectWithHole(WalkableBox.Min.X, WalkableBox.Min.Y, WalkableBox.Max.X, WalkableBox.Max.Y);
			}
			return;
		}

		// Tower floors are axis-aligned squares with an axis-aligned drop hole. Representing the
		// surface as four coarse slabs keeps the exact shape while removing hundreds of runtime actors.
		SpawnRectWithHole(PolygonMinX, PolygonMinY, PolygonMaxX, PolygonMaxY);
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

	static bool T66SpawnGeneratedDungeonFloorTiles(
		UWorld* World,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags)
	{
		if (!T66ShouldUseGeneratedDungeonKit()
			|| !World
			|| Theme.FloorMeshes.Num() <= 0
			|| Floor.BoundsHalfExtent <= 1.0f)
		{
			return false;
		}

		UStaticMesh* FloorMesh = Theme.FloorMeshes[0];
		if (!FloorMesh)
		{
			return false;
		}

		TArray<FName> FloorTags = Tags;
		FloorTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit")));
		FloorTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_Floor")));
		FloorTags.AddUnique(FName(*FString::Printf(TEXT("T66_Floor_Tower_GeneratedDungeonKit_Floor_%02d"), Floor.FloorNumber)));

		const float MeshSizeX = T66GetMeshAxisSize(FloorMesh, 0);
		const float MeshSizeY = T66GetMeshAxisSize(FloorMesh, 1);
		const float MeshSizeZ = T66GetMeshAxisSize(FloorMesh, 2);
		const float CollisionThickness = FMath::Max(Layout.FloorThickness + T66TowerRoofSkinThickness, T66TowerGeneratedDungeonKitFloorThickness);

		int32 SpawnedCount = 0;
		auto SpawnTileSetForBox = [&](const FBox2D& SourceBox)
		{
			const FVector2D BoxSize = SourceBox.Max - SourceBox.Min;
			if (BoxSize.X <= 10.0f || BoxSize.Y <= 10.0f)
			{
				return;
			}

			const int32 CountX = T66GetNativeDungeonKitModuleCount(BoxSize.X, MeshSizeX);
			const int32 CountY = T66GetNativeDungeonKitModuleCount(BoxSize.Y, MeshSizeY);
			if (CountX <= 0 || CountY <= 0)
			{
				return;
			}

			const float TileStartX = T66GetNativeDungeonKitModuleStart(SourceBox.Min.X, SourceBox.Max.X, CountX, MeshSizeX);
			const float TileStartY = T66GetNativeDungeonKitModuleStart(SourceBox.Min.Y, SourceBox.Max.Y, CountY, MeshSizeY);

			for (int32 TileY = 0; TileY < CountY; ++TileY)
			{
				for (int32 TileX = 0; TileX < CountX; ++TileX)
				{
					const float CenterX = TileStartX + (MeshSizeX * (static_cast<float>(TileX) + 0.5f));
					const float CenterY = TileStartY + (MeshSizeY * (static_cast<float>(TileY) + 0.5f));
					const FVector Location(CenterX, CenterY, Floor.SurfaceZ - T66TowerGeneratedDungeonKitFloorThickness);
					const FVector Scale(
						1.0f,
						1.0f,
						T66TowerGeneratedDungeonKitFloorThickness / MeshSizeZ);

					if (T66SpawnGroundedMeshActor(
						World,
						FloorMesh,
						nullptr,
						Location,
						FRotator::ZeroRotator,
						Scale,
						SpawnParams,
						false,
						FloorTags,
						true,
						true))
					{
						++SpawnedCount;
					}
				}
			}
		};

		auto SpawnCollisionSlabForBox = [&](const FBox2D& SourceBox)
		{
			const FVector2D BoxSize = SourceBox.Max - SourceBox.Min;
			if (BoxSize.X <= 10.0f || BoxSize.Y <= 10.0f)
			{
				return;
			}

			TArray<FName> CollisionTags = Tags;
			CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit")));
			CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_CollisionProxy")));
			CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_FloorCollision")));
			CollisionTags.AddUnique(FName(*FString::Printf(TEXT("T66_Floor_Tower_GeneratedDungeonKit_FloorCollision_%02d"), Floor.FloorNumber)));

			const FVector2D Center = (SourceBox.Min + SourceBox.Max) * 0.5f;
			T66SpawnHiddenCollisionProxyActor(
				World,
				FVector(Center.X, Center.Y, Floor.SurfaceZ - (CollisionThickness * 0.5f)),
				FRotator::ZeroRotator,
				FVector(BoxSize.X * 0.5f, BoxSize.Y * 0.5f, CollisionThickness * 0.5f),
				SpawnParams,
				CollisionTags,
				true);
		};

		auto SpawnSurfaceBox = [&](const FBox2D& SourceBox)
		{
			SpawnTileSetForBox(SourceBox);
			SpawnCollisionSlabForBox(SourceBox);
		};

		auto SpawnBoxWithHole = [&](const FBox2D& SourceBox)
		{
			if (!Floor.bHasDropHole || Floor.HoleHalfExtent.X <= 1.0f || Floor.HoleHalfExtent.Y <= 1.0f)
			{
				SpawnSurfaceBox(SourceBox);
				return;
			}

			const float HoleMinX = FMath::Clamp(Floor.HoleCenter.X - Floor.HoleHalfExtent.X, SourceBox.Min.X, SourceBox.Max.X);
			const float HoleMaxX = FMath::Clamp(Floor.HoleCenter.X + Floor.HoleHalfExtent.X, SourceBox.Min.X, SourceBox.Max.X);
			const float HoleMinY = FMath::Clamp(Floor.HoleCenter.Y - Floor.HoleHalfExtent.Y, SourceBox.Min.Y, SourceBox.Max.Y);
			const float HoleMaxY = FMath::Clamp(Floor.HoleCenter.Y + Floor.HoleHalfExtent.Y, SourceBox.Min.Y, SourceBox.Max.Y);
			if (HoleMaxX <= HoleMinX || HoleMaxY <= HoleMinY)
			{
				SpawnSurfaceBox(SourceBox);
				return;
			}

			SpawnSurfaceBox(FBox2D(FVector2D(SourceBox.Min.X, SourceBox.Min.Y), FVector2D(HoleMinX, SourceBox.Max.Y)));
			SpawnSurfaceBox(FBox2D(FVector2D(HoleMaxX, SourceBox.Min.Y), FVector2D(SourceBox.Max.X, SourceBox.Max.Y)));
			SpawnSurfaceBox(FBox2D(FVector2D(HoleMinX, SourceBox.Min.Y), FVector2D(HoleMaxX, HoleMinY)));
			SpawnSurfaceBox(FBox2D(FVector2D(HoleMinX, HoleMaxY), FVector2D(HoleMaxX, SourceBox.Max.Y)));
		};

		if (Floor.WalkableFloorBoxes.Num() > 0)
		{
			for (const FBox2D& WalkableBox : Floor.WalkableFloorBoxes)
			{
				SpawnBoxWithHole(WalkableBox);
			}
		}
		else
		{
			SpawnBoxWithHole(FBox2D(
				FVector2D(Floor.Center.X - Floor.BoundsHalfExtent, Floor.Center.Y - Floor.BoundsHalfExtent),
				FVector2D(Floor.Center.X + Floor.BoundsHalfExtent, Floor.Center.Y + Floor.BoundsHalfExtent)));
		}

		return SpawnedCount > 0;
	}

	static bool T66SpawnGeneratedDungeonFloorUndersideTiles(
		UWorld* World,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& CarrierFloor,
		const float CeilingBottomZ,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags,
		const bool bRespectCarrierDropHole,
		const bool bSpawnCeilingCollision)
	{
		if (!T66ShouldUseGeneratedDungeonKit()
			|| !World
			|| Theme.FloorMeshes.Num() <= 0
			|| CarrierFloor.BoundsHalfExtent <= 1.0f)
		{
			return false;
		}

		UStaticMesh* FloorMesh = Theme.FloorMeshes[0];
		if (!FloorMesh)
		{
			return false;
		}

		TArray<FName> CeilingTags = Tags;
		CeilingTags.AddUnique(T66TowerMapCeilingTag);
		CeilingTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit")));
		CeilingTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_FloorUnderside")));
		CeilingTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_Ceiling")));
		CeilingTags.AddUnique(FName(*FString::Printf(TEXT("T66_Floor_Tower_GeneratedDungeonKit_Ceiling_%02d"), CarrierFloor.FloorNumber)));

		const float MeshSizeX = T66GetMeshAxisSize(FloorMesh, 0);
		const float MeshSizeY = T66GetMeshAxisSize(FloorMesh, 1);
		const float MeshSizeZ = T66GetMeshAxisSize(FloorMesh, 2);
		const float CeilingVisualZ = CeilingBottomZ + T66TowerGeneratedDungeonKitFloorThickness;

		int32 SpawnedCount = 0;
		auto SpawnTileSetForBox = [&](const FBox2D& SourceBox)
		{
			const FVector2D BoxSize = SourceBox.Max - SourceBox.Min;
			if (BoxSize.X <= 10.0f || BoxSize.Y <= 10.0f)
			{
				return;
			}

			const int32 CountX = T66GetNativeDungeonKitModuleCount(BoxSize.X, MeshSizeX);
			const int32 CountY = T66GetNativeDungeonKitModuleCount(BoxSize.Y, MeshSizeY);
			if (CountX <= 0 || CountY <= 0)
			{
				return;
			}

			const float TileStartX = T66GetNativeDungeonKitModuleStart(SourceBox.Min.X, SourceBox.Max.X, CountX, MeshSizeX);
			const float TileStartY = T66GetNativeDungeonKitModuleStart(SourceBox.Min.Y, SourceBox.Max.Y, CountY, MeshSizeY);

			for (int32 TileY = 0; TileY < CountY; ++TileY)
			{
				for (int32 TileX = 0; TileX < CountX; ++TileX)
				{
					const float CenterX = TileStartX + (MeshSizeX * (static_cast<float>(TileX) + 0.5f));
					const float CenterY = TileStartY + (MeshSizeY * (static_cast<float>(TileY) + 0.5f));
					const FVector Scale(
						1.0f,
						1.0f,
						T66TowerGeneratedDungeonKitFloorThickness / MeshSizeZ);

					if (T66SpawnGroundedMeshActor(
						World,
						FloorMesh,
						nullptr,
						FVector(CenterX, CenterY, CeilingVisualZ),
						FRotator(180.0f, 0.0f, 0.0f),
						Scale,
						SpawnParams,
						false,
						CeilingTags,
						true,
						true))
					{
						++SpawnedCount;
					}
				}
			}
		};

		auto SpawnCeilingCollisionForBox = [&](const FBox2D& SourceBox)
		{
			if (!bSpawnCeilingCollision)
			{
				return;
			}

			const FVector2D BoxSize = SourceBox.Max - SourceBox.Min;
			if (BoxSize.X <= 10.0f || BoxSize.Y <= 10.0f)
			{
				return;
			}

			TArray<FName> CollisionTags = Tags;
			CollisionTags.AddUnique(T66TowerMapTraversalBarrierTag);
			CollisionTags.AddUnique(T66TowerMapCeilingTag);
			CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit")));
			CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_CollisionProxy")));
			CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_CeilingCollision")));
			CollisionTags.AddUnique(FName(*FString::Printf(TEXT("T66_Floor_Tower_GeneratedDungeonKit_CeilingCollision_%02d"), CarrierFloor.FloorNumber)));

			const float CollisionThickness = FMath::Max(Layout.FloorThickness, T66TowerGeneratedDungeonKitFloorThickness);
			const FVector2D Center = (SourceBox.Min + SourceBox.Max) * 0.5f;
			T66SpawnHiddenCollisionProxyActor(
				World,
				FVector(Center.X, Center.Y, CeilingBottomZ + (CollisionThickness * 0.5f)),
				FRotator::ZeroRotator,
				FVector(BoxSize.X * 0.5f, BoxSize.Y * 0.5f, CollisionThickness * 0.5f),
				SpawnParams,
				CollisionTags,
				false);
		};

		auto SpawnSurfaceBox = [&](const FBox2D& SourceBox)
		{
			SpawnTileSetForBox(SourceBox);
			SpawnCeilingCollisionForBox(SourceBox);
		};

		auto SpawnBoxWithHole = [&](const FBox2D& SourceBox)
		{
			if (!bRespectCarrierDropHole
				|| !CarrierFloor.bHasDropHole
				|| CarrierFloor.HoleHalfExtent.X <= 1.0f
				|| CarrierFloor.HoleHalfExtent.Y <= 1.0f)
			{
				SpawnSurfaceBox(SourceBox);
				return;
			}

			const float HoleMinX = FMath::Clamp(CarrierFloor.HoleCenter.X - CarrierFloor.HoleHalfExtent.X, SourceBox.Min.X, SourceBox.Max.X);
			const float HoleMaxX = FMath::Clamp(CarrierFloor.HoleCenter.X + CarrierFloor.HoleHalfExtent.X, SourceBox.Min.X, SourceBox.Max.X);
			const float HoleMinY = FMath::Clamp(CarrierFloor.HoleCenter.Y - CarrierFloor.HoleHalfExtent.Y, SourceBox.Min.Y, SourceBox.Max.Y);
			const float HoleMaxY = FMath::Clamp(CarrierFloor.HoleCenter.Y + CarrierFloor.HoleHalfExtent.Y, SourceBox.Min.Y, SourceBox.Max.Y);
			if (HoleMaxX <= HoleMinX || HoleMaxY <= HoleMinY)
			{
				SpawnSurfaceBox(SourceBox);
				return;
			}

			SpawnSurfaceBox(FBox2D(FVector2D(SourceBox.Min.X, SourceBox.Min.Y), FVector2D(HoleMinX, SourceBox.Max.Y)));
			SpawnSurfaceBox(FBox2D(FVector2D(HoleMaxX, SourceBox.Min.Y), FVector2D(SourceBox.Max.X, SourceBox.Max.Y)));
			SpawnSurfaceBox(FBox2D(FVector2D(HoleMinX, SourceBox.Min.Y), FVector2D(HoleMaxX, HoleMinY)));
			SpawnSurfaceBox(FBox2D(FVector2D(HoleMinX, HoleMaxY), FVector2D(HoleMaxX, SourceBox.Max.Y)));
		};

		if (CarrierFloor.WalkableFloorBoxes.Num() > 0)
		{
			for (const FBox2D& WalkableBox : CarrierFloor.WalkableFloorBoxes)
			{
				SpawnBoxWithHole(WalkableBox);
			}
		}
		else
		{
			SpawnBoxWithHole(FBox2D(
				FVector2D(CarrierFloor.Center.X - CarrierFloor.BoundsHalfExtent, CarrierFloor.Center.Y - CarrierFloor.BoundsHalfExtent),
				FVector2D(CarrierFloor.Center.X + CarrierFloor.BoundsHalfExtent, CarrierFloor.Center.Y + CarrierFloor.BoundsHalfExtent)));
		}

		return SpawnedCount > 0;
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
		const int32 Seed,
		const bool bIgnoreCameraChannel = false,
		const bool bSingleGeneratedVisualSide = false,
		const FVector2D& SingleGeneratedVisualNormal = FVector2D::ZeroVector)
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
					Tags,
					bIgnoreCameraChannel);
			}
			return;
		}

		case T66TowerThemeVisuals::EWallFamily::SplitCollisionVisual:
		{
			const bool bSpawnedGeneratedVisuals = T66SpawnGeneratedDungeonWallVisuals(
				World,
				Theme,
				WallBox,
				BaseZ,
				DesiredHeight,
				SpawnParams,
				Tags,
				Seed,
				bSingleGeneratedVisualSide,
				SingleGeneratedVisualNormal,
				true,
				bIgnoreCameraChannel);

			if (bSpawnedGeneratedVisuals)
			{
				return;
			}

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
					Tags,
					bIgnoreCameraChannel);
			}
			return;
		}

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
					Tags,
					bIgnoreCameraChannel);
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
			T66TowerMapTraversalBarrierTag,
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
			Layout.Preset.Seed + (Floor.FloorNumber * 4101),
			T66ShouldIgnoreTowerWallCameraCollision(),
			true,
			FVector2D(-1.0f, 0.0f));
		T66SpawnThemedWallBox(
			World,
			CubeMesh,
			Theme,
			FBox2D(FVector2D(-Layout.ShellRadius - WallHalfDepth, -WallHalfSpan), FVector2D(-Layout.ShellRadius + WallHalfDepth, WallHalfSpan)),
			Floor.SurfaceZ,
			WallHeight,
			SpawnParams,
			ShellTags,
			Layout.Preset.Seed + (Floor.FloorNumber * 4102),
			T66ShouldIgnoreTowerWallCameraCollision(),
			true,
			FVector2D(1.0f, 0.0f));
		T66SpawnThemedWallBox(
			World,
			CubeMesh,
			Theme,
			FBox2D(FVector2D(-WallHalfSpan, Layout.ShellRadius - WallHalfDepth), FVector2D(WallHalfSpan, Layout.ShellRadius + WallHalfDepth)),
			Floor.SurfaceZ,
			WallHeight,
			SpawnParams,
			ShellTags,
			Layout.Preset.Seed + (Floor.FloorNumber * 4103),
			T66ShouldIgnoreTowerWallCameraCollision(),
			true,
			FVector2D(0.0f, -1.0f));
		T66SpawnThemedWallBox(
			World,
			CubeMesh,
			Theme,
			FBox2D(FVector2D(-WallHalfSpan, -Layout.ShellRadius - WallHalfDepth), FVector2D(WallHalfSpan, -Layout.ShellRadius + WallHalfDepth)),
			Floor.SurfaceZ,
			WallHeight,
			SpawnParams,
			ShellTags,
			Layout.Preset.Seed + (Floor.FloorNumber * 4104),
			T66ShouldIgnoreTowerWallCameraCollision(),
			true,
			FVector2D(0.0f, 1.0f));
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
		if (!World || (Floor.MazeWallBoxes.Num() <= 0 && Floor.DoorwayHeaderBoxes.Num() <= 0))
		{
			return;
		}

		const TArray<FName> WallTags = {
			T66TowerMapTraversalBarrierTag,
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
				Layout.Preset.Seed + (Floor.FloorNumber * 913) + static_cast<int32>(WallCenter.X + WallCenter.Y),
				T66ShouldIgnoreTowerWallCameraCollision());
		}

		if (!CubeMesh || Floor.DoorwayHeaderBoxes.Num() <= 0)
		{
			return;
		}

		if (Theme.WallFamily == T66TowerThemeVisuals::EWallFamily::SplitCollisionVisual
			&& T66ShouldUseGeneratedDungeonKit())
		{
			return;
		}

		const TArray<FName> DoorwayTags = {
			FName(*FString::Printf(TEXT("T66_Floor_Tower_%02d"), Floor.FloorNumber)),
			FName(*FString::Printf(TEXT("T66_Floor_Tower_Doorway_%02d"), Floor.FloorNumber))
		};
		const float HeaderHeight = FMath::Clamp(WallHeight * 0.24f, 260.0f, 520.0f);
		const float HeaderZ = Floor.SurfaceZ + WallHeight - HeaderHeight - 80.0f;
		UMaterialInterface* HeaderMaterial = Theme.WallMaterial ? Theme.WallMaterial : Theme.FloorMaterial;
		for (const FBox2D& HeaderBox : Floor.DoorwayHeaderBoxes)
		{
			const FVector2D HeaderCenter = (HeaderBox.Min + HeaderBox.Max) * 0.5f;
			const FVector2D HeaderHalfExtents = (HeaderBox.Max - HeaderBox.Min) * 0.5f;
			if (HeaderHalfExtents.X <= 5.0f || HeaderHalfExtents.Y <= 5.0f)
			{
				continue;
			}

			T66SpawnStaticMeshActor(
				World,
				CubeMesh,
				HeaderMaterial,
				FVector(HeaderCenter.X, HeaderCenter.Y, HeaderZ + (HeaderHeight * 0.5f)),
				FRotator::ZeroRotator,
				FVector(HeaderHalfExtents.X, HeaderHalfExtents.Y, HeaderHeight * 0.5f),
				SpawnParams,
				false,
				DoorwayTags);
		}
	}

	static void T66SpawnPropActors(
		UWorld* World,
		UStaticMesh* CubeMesh,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const FActorSpawnParameters& SpawnParams)
	{
		if (!World || !CubeMesh || !Floor.bGameplayFloor)
		{
			return;
		}

		if (Floor.WalkableFloorBoxes.Num() > 0)
		{
			return;
		}

		UMaterialInterface* StoneMaterial = Theme.DecorationMaterialOverride
			? Theme.DecorationMaterialOverride
			: (Theme.WallMaterial ? Theme.WallMaterial : Theme.FloorMaterial);
		FRandomStream Rng(Layout.Preset.Seed + Floor.FloorNumber * 911);
		static constexpr float T66TowerFloodRockFootprintScale = 0.25f;
		const float MajorStoneHalfExtent = FMath::Clamp(Layout.PlacementCellSize * 0.72f, 760.0f, 960.0f) * T66TowerFloodRockFootprintScale;
		const float MinorStoneHalfExtent = MajorStoneHalfExtent * 0.82f;
		const float MajorStoneHalfHeight = FMath::Clamp(Layout.PlacementCellSize * 0.16f, 150.0f, 220.0f);
		const float MinorStoneHalfHeight = MajorStoneHalfHeight * 0.78f;
		const float ConnectionOffset = FMath::Clamp((Layout.GridCellSize * 0.5f) - MajorStoneHalfExtent - 180.0f, 1850.0f, 2250.0f);
		const TArray<FName> PropTags = {
			FName(*FString::Printf(TEXT("T66_Floor_Tower_Deco_%02d"), Floor.FloorNumber)),
			FName(*FString::Printf(TEXT("T66_Floor_Tower_Rock_%02d"), Floor.FloorNumber))
		};

		auto SpawnStoneAt = [&](const FVector& SurfaceLocation, const float HalfExtentXY, const float HalfHeight) -> bool
		{
			if (!T66IsWalkableTowerLocation(Floor, SurfaceLocation, HalfExtentXY * 0.18f, HalfExtentXY * 0.45f, 180.0f))
			{
				return false;
			}

			const FVector SpawnLocation = SurfaceLocation + FVector(0.0f, 0.0f, HalfHeight);
			return T66SpawnStaticMeshActor(
				World,
				CubeMesh,
				StoneMaterial,
				SpawnLocation,
				FRotator(0.0f, Rng.FRandRange(0.0f, 360.0f), 0.0f),
				FVector(HalfExtentXY, HalfExtentXY, HalfHeight),
				SpawnParams,
				true,
				PropTags) != nullptr;
		};

		bool bSpawnedAny = false;
		if (Floor.GridCells.Num() > 0)
		{
			for (const T66TowerMapTerrain::FGridCell& Cell : Floor.GridCells)
			{
				if (Cell.Semantic == ET66TowerGridCellSemantic::Unused)
				{
					continue;
				}

				const bool bOptionalCell = Cell.Semantic == ET66TowerGridCellSemantic::OptionalLoop;
				if (bOptionalCell && Rng.FRand() > 0.55f)
				{
					continue;
				}

				bSpawnedAny |= SpawnStoneAt(Cell.WorldCenter, MajorStoneHalfExtent, MajorStoneHalfHeight);

				auto SpawnConnectionStone = [&](const uint8 ConnectionMask, const FVector& Direction)
				{
					if ((Cell.ConnectionMask & ConnectionMask) == 0)
					{
						return;
					}

					if (bOptionalCell && Rng.FRand() > 0.45f)
					{
						return;
					}

					bSpawnedAny |= SpawnStoneAt(Cell.WorldCenter + (Direction * ConnectionOffset), MinorStoneHalfExtent, MinorStoneHalfHeight);
				};

				SpawnConnectionStone(T66TowerMapTerrain::GridNorth, FVector(0.0f, -1.0f, 0.0f));
				SpawnConnectionStone(T66TowerMapTerrain::GridEast, FVector(1.0f, 0.0f, 0.0f));
				SpawnConnectionStone(T66TowerMapTerrain::GridSouth, FVector(0.0f, 1.0f, 0.0f));
				SpawnConnectionStone(T66TowerMapTerrain::GridWest, FVector(-1.0f, 0.0f, 0.0f));
			}
		}

		if (bSpawnedAny || Floor.CachedMainPathSpawnSlots.Num() <= 0)
		{
			return;
		}

		TArray<FVector> FallbackSlots = Floor.CachedMainPathSpawnSlots;
		FallbackSlots.Sort([&Floor](const FVector& A, const FVector& B)
		{
			return FVector::DistSquared2D(A, Floor.ArrivalPoint) < FVector::DistSquared2D(B, Floor.ArrivalPoint);
		});

		for (int32 SlotIndex = 0; SlotIndex < FallbackSlots.Num(); SlotIndex += 2)
		{
			SpawnStoneAt(FallbackSlots[SlotIndex], MajorStoneHalfExtent, MajorStoneHalfHeight);
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

	ET66TowerGameplayLevelTheme ResolveGameplayLevelThemeForDifficulty(const ET66Difficulty Difficulty)
	{
		return ResolveGameplayLevelTheme(ResolveGameplayLevelNumberForDifficulty(Difficulty));
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

	bool BuildLayout(const FT66MapPreset& Preset, FLayout& OutLayout, const bool bBossRushFinaleStage)
	{
		OutLayout = FLayout{};
		OutLayout.Preset = Preset;
		OutLayout.PlacementCellSize = 1300.0f;
		OutLayout.FloorThickness = 280.0f;
		OutLayout.FloorSpacing = T66TowerStandardFloorHeadroom + OutLayout.FloorThickness + T66TowerRoofSkinThickness;
		OutLayout.WallThickness = T66TowerDungeonKitWallDepth;
		OutLayout.ShellRadius = 20000.0f;
		OutLayout.MazeMode = T66GetConfiguredTowerMazeMode();
		OutLayout.GridColumns = T66TowerGridDefaultColumns;
		OutLayout.GridRows = T66TowerGridDefaultRows;
		OutLayout.GridCellSize = T66TowerGridDefaultCellSize;
		OutLayout.GridDoorWidth = T66TowerGridDefaultDoorWidth;
		OutLayout.StartFloorNumber = T66TowerStartFloorNumber;
		OutLayout.BossFloorNumber = bBossRushFinaleStage ? 2 : T66TowerBossFloorNumber;
		OutLayout.FirstGameplayFloorNumber = bBossRushFinaleStage ? OutLayout.BossFloorNumber : T66TowerFirstGameplayFloorNumber;
		OutLayout.LastGameplayFloorNumber = bBossRushFinaleStage ? OutLayout.StartFloorNumber : T66TowerLastGameplayFloorNumber;

		const float TopFloorZ = Preset.BaselineZ + 1600.0f;
		const float FloorSpacing = OutLayout.FloorSpacing;
		const FVector2D HoleHalfExtent(OutLayout.PlacementCellSize * 0.5f, OutLayout.PlacementCellSize * 0.5f);
		const FVector AlignedHoleOffset(0.0f, -OutLayout.PlacementCellSize, 0.0f);
		const float StartRoomHalfExtent = T66TowerStartRoomSquareSize * 0.5f;
		const int32 TotalFloorCount = bBossRushFinaleStage ? 2 : T66TowerTotalFloorCount;
		const float FloorBottomZ = TopFloorZ - (static_cast<float>(TotalFloorCount - 1) * FloorSpacing) - OutLayout.FloorThickness;

		OutLayout.TraceStartZ = TopFloorZ + 12000.0f;
		OutLayout.TraceEndZ = FloorBottomZ - 12000.0f;

		for (int32 FloorIndex = 0; FloorIndex < TotalFloorCount; ++FloorIndex)
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
			Floor.Theme = ET66TowerGameplayLevelTheme::Dungeon;
			Floor.bHasDropHole = Floor.FloorNumber < OutLayout.BossFloorNumber;
			Floor.Center = FVector(0.0f, 0.0f, TopFloorZ - (static_cast<float>(FloorIndex) * FloorSpacing));
			Floor.SurfaceZ = Floor.Center.Z;
			Floor.PolygonApothem = OutLayout.ShellRadius - (OutLayout.WallThickness * 0.5f) + 20.0f;
			if (Floor.FloorNumber == OutLayout.BossFloorNumber)
			{
				// Match the final boss arena to the idol-room footprint instead of the full outer shell.
				Floor.PolygonApothem = StartRoomHalfExtent;
			}
			Floor.BoundsHalfExtent = Floor.PolygonApothem;
			Floor.WalkableHalfExtent = (Floor.FloorNumber == OutLayout.BossFloorNumber)
				? Floor.PolygonApothem
				: (Floor.PolygonApothem - 1300.0f);
			Floor.FloorTag =
				(Floor.FloorRole == ET66TowerFloorRole::Start) ? T66TowerMapFloorStartTag :
				(Floor.FloorRole == ET66TowerFloorRole::Boss) ? T66TowerMapFloorBossTag :
				T66TowerMapFloorMainTag;

			if (Floor.bHasDropHole)
			{
				const FVector HoleOffset = AlignedHoleOffset;
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
		const int32 PreBossFloorNumber = FMath::Max(OutLayout.StartFloorNumber, OutLayout.BossFloorNumber - 1);
		if (!T66TryGetFloor(OutLayout, OutLayout.StartFloorNumber, StartFloor)
			|| !T66TryGetFloor(OutLayout, PreBossFloorNumber, PreBossFloor)
			|| !T66TryGetFloor(OutLayout, OutLayout.BossFloorNumber, BossFloor))
		{
			return false;
		}

		const FVector StartInwardDirection(0.0f, 1.0f, 0.0f);
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
			FRandomStream FloorMazeRng(T66BuildTowerFloorSeed(Preset.Seed, Floor.FloorNumber, Floor.GameplayLevelNumber, Floor.Theme));
			T66BuildFloorMazeWalls(OutLayout, Floor, FloorMazeRng);
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
		for (FFloor& Floor : OutLayout.Floors)
		{
			if (Floor.FloorNumber == OutLayout.BossFloorNumber)
			{
				T66BuildBossFloorRoom(OutLayout, Floor);
				break;
			}
		}

		const FVector BossHoleDirection = (PreBossFloor->HoleCenter - PreBossFloor->Center).GetSafeNormal2D();
		if (BossHoleDirection.IsNearlyZero())
		{
			return false;
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
			if (T66IsWalkableTowerLocation(Floor, Floor.ArrivalPoint, 250.0f, 900.0f, 250.0f))
			{
				OutLayout.RescueAnchorLocations.Add(Floor.ArrivalPoint);
			}

			if (Floor.CachedWalkableSpawnSlots.Num() > 0)
			{
				OutLayout.RescueAnchorLocations.Add(Floor.CachedWalkableSpawnSlots[0]);
				OutLayout.RescueAnchorLocations.Add(Floor.CachedWalkableSpawnSlots[Floor.CachedWalkableSpawnSlots.Num() / 2]);
				OutLayout.RescueAnchorLocations.Add(Floor.CachedWalkableSpawnSlots.Last());
			}
			else
			{
				OutLayout.RescueAnchorLocations.Add(Floor.Center);
				OutLayout.RescueAnchorLocations.Add(Floor.Center + FVector(Floor.WalkableHalfExtent * 0.35f, 0.0f, 0.0f));
				OutLayout.RescueAnchorLocations.Add(Floor.Center + FVector(0.0f, Floor.WalkableHalfExtent * 0.35f, 0.0f));
			}
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
		const ET66TowerGameplayLevelTheme StageTheme = ResolveGameplayLevelThemeForDifficulty(Difficulty);
		for (const FFloor& Floor : Layout.Floors)
		{
			FFloor ThemedFloor = Floor;
			ThemedFloor.Theme = StageTheme;
			T66TowerThemeVisuals::FResolvedTheme Theme;
			T66TowerThemeVisuals::ResolveFloorTheme(World, ThemedFloor, Theme);
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
			const bool bSpawnedGeneratedFloor = T66SpawnGeneratedDungeonFloorTiles(World, Theme, Layout, Floor, SpawnParams, FloorTags);
			if (!bSpawnedGeneratedFloor)
			{
				T66SpawnPolygonFloor(World, CubeMesh, Theme.FloorMaterial, Layout, Floor, SpawnParams, FloorTags);
			}
			T66SpawnMazeWalls(World, CubeMesh, Theme, Layout, Floor, ModuleWallHeight, SpawnParams);
			T66SpawnPropActors(World, CubeMesh, Theme, Layout, Floor, SpawnParams);

			const bool bUsingGeneratedDungeonKitForTheme =
				T66ShouldUseGeneratedDungeonKit()
				&& Theme.WallFamily == T66TowerThemeVisuals::EWallFamily::SplitCollisionVisual;
			if (bUsingGeneratedDungeonKitForTheme)
			{
				T66SpawnGeneratedDungeonFloorUndersideTiles(
					World,
					Theme,
					Layout,
					Floor,
					CeilingBottomZ,
					SpawnParams,
					FloorTags,
					false,
					FloorIndex == 0);
			}
			else
			{
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
							T66TowerMapTraversalBarrierTag,
							T66TowerMapCeilingTag,
							FName(*FString::Printf(TEXT("T66_Floor_Tower_%02d"), Floor.FloorNumber)),
							FName(TEXT("T66_Floor_Tower_Roof")),
							FName(*FString::Printf(TEXT("T66_Floor_Tower_Roof_%02d"), Floor.FloorNumber))
						});
				}
			}
		}

		bOutCollisionReady = true;
		return true;
	}
}
