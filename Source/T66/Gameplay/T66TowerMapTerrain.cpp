// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TowerMapTerrain.h"

#include "Core/T66GameplayLayout.h"
#include "Core/T66TowerTuningConfig.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/MapGeneration/Composition/T66RoomComposer.h"
#include "Gameplay/MapGeneration/Composition/T66RoomFeaturePlacer.h"
#include "Gameplay/MapGeneration/Validation/T66RoomValidation.h"
#include "Gameplay/T66BouncePadObstacle.h"
#include "Gameplay/T66TowerLiftPlatform.h"
#include "Gameplay/T66TowerThemeVisuals.h"
#include "Gameplay/T66VisualUtil.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "Components/BoxComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/BodySetup.h"

namespace
{
	DEFINE_LOG_CATEGORY_STATIC(LogT66TowerMapTerrain, Log, All);

	static const FName T66TowerMapTerrainVisualTag(TEXT("T66_MainMapTerrain_Visual"));
	static const FName T66TowerMapTerrainMaterialsReadyTag(TEXT("T66_MainMapTerrain_MaterialsReady"));
	static const FName T66TowerMapTerrainCollisionProxyTag(TEXT("T66_MainMapTerrain_CollisionProxy"));
	static const FName T66TowerMapTraversalBarrierTag(TEXT("T66_Map_TraversalBarrier"));
	// Elevated walkable deck slabs (mesa tops, Tier 2 platforms): the camera fade
	// pass accepts this tag with a thin-slab shape exemption so the hero never
	// plays blind under a bridge (T66PlayerController occluder filter).
	static const FName T66TowerMapDeckVisualTag(TEXT("T66_Tower_DeckVisual"));

	static const FName T66HazardTypeSweeperArm = T66MapGeneration::HazardSweeperArm;
	static const FName T66HazardTypeCeilingHammer = T66MapGeneration::HazardCeilingHammer;
	static const FName T66TowerMapCameraOccludingWallVisualTag(TEXT("T66_CameraOccludingWallVisual"));
	static const FName T66TowerMapCeilingTag(TEXT("T66_Tower_Ceiling"));
	static const FName T66TowerTerrainNoSurfaceBounceTag(TEXT("T66_NoSurfaceBounce"));
	static const FName T66TowerMapFloorStartTag(TEXT("T66_Floor_Start"));
	static const FName T66TowerMapFloorMainTag(TEXT("T66_Floor_Main"));
	static const FName T66TowerMapFloorBossTag(TEXT("T66_Floor_Boss"));
	static constexpr int32 T66TowerFloorVertexCount = 4;
	static constexpr float T66TowerDungeonKitUnitSize = 1300.0f;
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
		TEXT("0 uses legacy material-only dungeon wall cubes, 1 uses generated tower theme visuals with lightweight collision proxies."),
		ECVF_Default);

	static TAutoConsoleVariable<int32> CVarT66TowerFloorBaffles(
		TEXT("t66.Tower.FloorBaffles"),
		// 2026-06-10 user direction: tubes are parked until the map design is nailed —
		// the tower renders clean Fall Guys slabs by default. Flip to 1 to bring the
		// inflated baffle look back.
		0,
		TEXT("0 uses clean slab/box visuals (Fall Guys look), 1 replaces generated floor, wall, and ceiling visuals with HISM baffle tube instances. Collision proxies stay unchanged."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66TowerFloorBafflePitch(
		TEXT("t66.Tower.FloorBafflePitch"),
		450.0f,
		TEXT("Target center spacing in cm for visual-only dungeon baffle tubes. Collision proxies stay unchanged."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66TowerFloorBaffleDiameter(
		TEXT("t66.Tower.FloorBaffleDiameter"),
		550.0f,
		TEXT("Visual-only dungeon baffle tube diameter in cm. Collision proxies stay unchanged."),
		ECVF_Default);

	static TAutoConsoleVariable<int32> CVarT66TowerWallBaffles(
		TEXT("t66.Tower.WallBaffles"),
		1,
		TEXT("When non-zero and floor baffles are enabled, generated dungeon wall visuals use baffle tubes. Collision proxies stay unchanged."),
		ECVF_Default);

	static TAutoConsoleVariable<int32> CVarT66TowerCeilingBaffles(
		TEXT("t66.Tower.CeilingBaffles"),
		1,
		TEXT("When non-zero and floor baffles are enabled, generated dungeon ceiling visuals use baffle tubes. Collision proxies stay unchanged."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66TowerGeneratedKitWallVisualSegmentLength(
		TEXT("T66.Tower.GeneratedKitWallVisualSegmentLength"),
		T66TowerDungeonKitUnitSize * 2.0f,
		TEXT("Target world-space length for generated wall visual meshes. Larger values reduce high-poly HISM instances; collision proxies stay layout-authored."),
		ECVF_Default);

	static TAutoConsoleVariable<int32> CVarT66TowerTerrainTimingLogs(
		TEXT("T66.Tower.TerrainTimingLogs"),
		1,
		TEXT("0 disables tower terrain spawn timing logs, 1 logs per-floor and total spawn timing/counter summaries."),
		ECVF_Default);

	using ET66TowerGridCellSemantic = T66TowerMapTerrain::ET66TowerGridCellSemantic;
	using ET66TowerGridTemplate = T66TowerMapTerrain::ET66TowerGridTemplate;

	struct FT66TowerTerrainSpawnStats
	{
		int32 CollisionProxyActors = 0;
		int32 InstancedVisualActors = 0;
		int32 HISMComponents = 0;
		int32 HISMInstances = 0;
	};

	static FT66TowerTerrainSpawnStats* GT66TowerTerrainActiveSpawnStats = nullptr;

	struct FT66ScopedTowerTerrainSpawnStats
	{
		explicit FT66ScopedTowerTerrainSpawnStats(FT66TowerTerrainSpawnStats& InStats)
			: PreviousStats(GT66TowerTerrainActiveSpawnStats)
		{
			GT66TowerTerrainActiveSpawnStats = &InStats;
		}

		~FT66ScopedTowerTerrainSpawnStats()
		{
			GT66TowerTerrainActiveSpawnStats = PreviousStats;
		}

	private:
		FT66TowerTerrainSpawnStats* PreviousStats = nullptr;
	};

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

	static bool T66ShouldUseFloorBaffles()
	{
		return CVarT66TowerFloorBaffles.GetValueOnAnyThread() != 0;
	}

	static bool T66ShouldUseWallBaffles()
	{
		return T66ShouldUseFloorBaffles() && CVarT66TowerWallBaffles.GetValueOnAnyThread() != 0;
	}

	static bool T66ShouldUseCeilingBaffles()
	{
		return T66ShouldUseFloorBaffles() && CVarT66TowerCeilingBaffles.GetValueOnAnyThread() != 0;
	}

	static const UT66TowerTuningConfig& T66GetTowerTuning()
	{
		return UT66TowerTuningConfig::GetRuntimeConfig();
	}

	static bool T66IsCameraOccludingTowerWallVisual(const TArray<FName>& Tags)
	{
		return Tags.Contains(T66TowerMapCameraOccludingWallVisualTag) && !Tags.Contains(T66TowerMapCeilingTag);
	}

	static float T66GetGeneratedKitWallVisualTargetSegmentLength()
	{
		const UT66TowerTuningConfig& TowerTuning = T66GetTowerTuning();
		const float RequestedLength = CVarT66TowerGeneratedKitWallVisualSegmentLength.GetValueOnAnyThread();
		return FMath::Clamp(RequestedLength, TowerTuning.PlacementCellSize, TowerTuning.PlacementCellSize * 6.0f);
	}

	static float T66GetMazeWallHalfThicknessScale(const T66TowerMapTerrain::FLayout& Layout)
	{
		return (Layout.WallThickness * 0.5f) / FMath::Max(1.0f, Layout.PlacementCellSize);
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
		if (GT66TowerTerrainActiveSpawnStats)
		{
			++GT66TowerTerrainActiveSpawnStats->InstancedVisualActors;
		}

		if (UStaticMeshComponent* MeshComponent = Actor->GetStaticMeshComponent())
		{
			const bool bCameraOccludingWallVisual = T66IsCameraOccludingTowerWallVisual(ExtraTags);
			MeshComponent->SetMobility(EComponentMobility::Movable);
			MeshComponent->SetStaticMesh(Mesh);
			MeshComponent->SetGenerateOverlapEvents(false);
			MeshComponent->SetRelativeScale3D(T66ComputeMeshScaleForHalfExtents(Mesh, DesiredHalfExtents));
			for (const FName& Tag : ExtraTags)
			{
				if (!Tag.IsNone())
				{
					MeshComponent->ComponentTags.AddUnique(Tag);
				}
			}
			MeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
			T66ConfigureTowerCollisionResponses(MeshComponent, bIgnoreCameraChannel);
			MeshComponent->SetCollisionEnabled(bEnableCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
			if (bCameraOccludingWallVisual)
			{
				if (!bEnableCollision)
				{
					MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
					MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
				}
				MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
			}
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
		if (GT66TowerTerrainActiveSpawnStats)
		{
			++GT66TowerTerrainActiveSpawnStats->CollisionProxyActors;
		}
		return Actor;
	}

	/** Reconfigures a hidden proxy to block ONLY the camera channel (no gameplay impact). */
	static void T66MakeCollisionProxyCameraOnly(AActor* ProxyActor)
	{
		if (!ProxyActor)
		{
			return;
		}

		TInlineComponentArray<UBoxComponent*> BoxComponents;
		ProxyActor->GetComponents(BoxComponents);
		for (UBoxComponent* BoxComponent : BoxComponents)
		{
			if (!BoxComponent)
			{
				continue;
			}

			BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			BoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
			BoxComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
		}
	}

	static AStaticMeshActor* T66SpawnEnvironmentRectangle(
		UWorld* World,
		UStaticMesh* CubeMesh,
		UMaterialInterface* Material,
		const FVector& Location,
		const FVector& DesiredHalfExtents,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& ExtraTags,
		const bool bIgnoreCameraChannel = false)
	{
		if (!World || !CubeMesh || !Material)
		{
			return nullptr;
		}

		return T66SpawnStaticMeshActor(
			World,
			CubeMesh,
			Material,
			Location,
			FRotator::ZeroRotator,
			DesiredHalfExtents,
			SpawnParams,
			false,
			ExtraTags,
			bIgnoreCameraChannel);
	}

	static UMaterialInterface* T66ResolveEnvironmentWallMaterialForBox(
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const FBox2D& WallBox)
	{
		const FVector2D WallSize = WallBox.Max - WallBox.Min;
		const bool bWallRunsAlongX = WallSize.X >= WallSize.Y;
		if (bWallRunsAlongX)
		{
			return Theme.WallXZMaterial ? Theme.WallXZMaterial : Theme.WallMaterial;
		}

		return Theme.WallYZMaterial ? Theme.WallYZMaterial : Theme.WallMaterial;
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
		if (GT66TowerTerrainActiveSpawnStats)
		{
			++GT66TowerTerrainActiveSpawnStats->InstancedVisualActors;
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
		for (const FName& Tag : ExtraTags)
		{
			if (!Tag.IsNone())
			{
				MeshComponent->ComponentTags.AddUnique(Tag);
			}
		}
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
		if (T66IsCameraOccludingTowerWallVisual(ExtraTags))
		{
			if (!bEnableCollision)
			{
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
			}
			MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
		}
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

	static FTransform T66MakeGroundedMeshInstanceTransform(
		UStaticMesh* Mesh,
		const FVector& ActorLocation,
		const FRotator& ActorRotation,
		const FVector& Scale,
		const bool bCenterBoundsAtActorXY)
	{
		if (!Mesh)
		{
			return FTransform(ActorRotation, ActorLocation, Scale);
		}

		const FBoxSphereBounds Bounds = Mesh->GetBounds();
		const float ScaleZ = FMath::Abs(Scale.Z);
		const float BottomZ = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * ScaleZ;
		FVector RelativeLocation(0.0f, 0.0f, -BottomZ);
		if (bCenterBoundsAtActorXY)
		{
			RelativeLocation.X = -Bounds.Origin.X * Scale.X;
			RelativeLocation.Y = -Bounds.Origin.Y * Scale.Y;
		}

		return FTransform(FRotator::ZeroRotator, RelativeLocation, Scale)
			* FTransform(ActorRotation, ActorLocation, FVector::OneVector);
	}

	struct FT66GeneratedDungeonWallBatch
	{
		TArray<TArray<FTransform>> InstanceTransformsByMesh;
		int32 InstanceCount = 0;

		void Reset(const int32 MeshCount)
		{
			InstanceTransformsByMesh.Reset();
			InstanceTransformsByMesh.SetNum(MeshCount);
			InstanceCount = 0;
		}

		void EnsureMeshCount(const int32 MeshCount)
		{
			if (InstanceTransformsByMesh.Num() != MeshCount)
			{
				Reset(MeshCount);
			}
		}

		void Add(const int32 MeshIndex, const FTransform& Transform)
		{
			if (!InstanceTransformsByMesh.IsValidIndex(MeshIndex))
			{
				return;
			}

			InstanceTransformsByMesh[MeshIndex].Add(Transform);
			++InstanceCount;
		}
	};

	static AActor* T66SpawnGeneratedDungeonInstancedMeshActor(
		UWorld* World,
		const TArray<UStaticMesh*>& Meshes,
		const TArray<TArray<FTransform>>& InstanceTransformsByMesh,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& ExtraTags,
		const TCHAR* ComponentNamePrefix,
		const bool bIgnoreCameraChannel)
	{
		if (!World || Meshes.Num() <= 0 || InstanceTransformsByMesh.Num() <= 0)
		{
			return nullptr;
		}

		int32 TotalInstanceCount = 0;
		for (const TArray<FTransform>& InstanceTransforms : InstanceTransformsByMesh)
		{
			TotalInstanceCount += InstanceTransforms.Num();
		}

		if (TotalInstanceCount <= 0)
		{
			return nullptr;
		}

		AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (!Actor)
		{
			return nullptr;
		}

		USceneComponent* RootComponent = NewObject<USceneComponent>(Actor, TEXT("InstancedVisualRoot"));
		if (!RootComponent)
		{
			Actor->Destroy();
			return nullptr;
		}

		RootComponent->SetMobility(EComponentMobility::Movable);
		Actor->SetRootComponent(RootComponent);
		Actor->AddInstanceComponent(RootComponent);
		RootComponent->RegisterComponent();

		Actor->Tags.AddUnique(T66TowerMapTerrainVisualTag);
		Actor->Tags.AddUnique(T66TowerMapTerrainMaterialsReadyTag);
		for (const FName& Tag : ExtraTags)
		{
			if (!Tag.IsNone())
			{
				Actor->Tags.AddUnique(Tag);
			}
		}

		const bool bCameraOccludingWallVisual = T66IsCameraOccludingTowerWallVisual(ExtraTags);
		for (int32 MeshIndex = 0; MeshIndex < Meshes.Num(); ++MeshIndex)
		{
			if (!InstanceTransformsByMesh.IsValidIndex(MeshIndex) || InstanceTransformsByMesh[MeshIndex].Num() <= 0)
			{
				continue;
			}

			UStaticMesh* Mesh = Meshes[MeshIndex];
			if (!Mesh)
			{
				continue;
			}

			const FString ComponentName = FString::Printf(TEXT("%s_%02d"), ComponentNamePrefix ? ComponentNamePrefix : TEXT("InstancedVisualMesh"), MeshIndex);
			UHierarchicalInstancedStaticMeshComponent* MeshComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(
				Actor,
				MakeUniqueObjectName(Actor, UHierarchicalInstancedStaticMeshComponent::StaticClass(), FName(*ComponentName)));
			if (!MeshComponent)
			{
				continue;
			}

			Actor->AddInstanceComponent(MeshComponent);
			MeshComponent->SetupAttachment(RootComponent);
			MeshComponent->SetMobility(EComponentMobility::Movable);
			MeshComponent->SetStaticMesh(Mesh);
			MeshComponent->SetGenerateOverlapEvents(false);
			MeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
			T66ConfigureTowerCollisionResponses(MeshComponent, bIgnoreCameraChannel);
			if (bCameraOccludingWallVisual)
			{
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
				MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
			}
			else
			{
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			MeshComponent->SetCanEverAffectNavigation(false);
			MeshComponent->SetCullDistances(0, T66GetTowerTuning().GeneratedDungeonKitCullDistance);
			for (const FName& Tag : ExtraTags)
			{
				if (!Tag.IsNone())
				{
					MeshComponent->ComponentTags.AddUnique(Tag);
				}
			}
			T66OptimizeTowerMeshComponent(MeshComponent);
			FT66VisualUtil::EnsureUnlitMaterials(MeshComponent, World);
			MeshComponent->PreAllocateInstancesMemory(InstanceTransformsByMesh[MeshIndex].Num());
			MeshComponent->AddInstances(InstanceTransformsByMesh[MeshIndex], false, false, false);
			if (GT66TowerTerrainActiveSpawnStats)
			{
				++GT66TowerTerrainActiveSpawnStats->HISMComponents;
				GT66TowerTerrainActiveSpawnStats->HISMInstances += InstanceTransformsByMesh[MeshIndex].Num();
			}
			MeshComponent->RegisterComponent();
			MeshComponent->SetMobility(EComponentMobility::Static);
		}

		RootComponent->SetMobility(EComponentMobility::Static);
		return Actor;
	}

	// Elevated-deck architecture (design ref section 1.5): walkable slabs held up
	// by round pillars, ground underneath stays playable. Deck thickness pairs
	// with the visual slab exactly (exact-collision contract).
	static constexpr float T66TowerDeckThicknessUU = 60.0f;
	static constexpr float T66TowerMesaPillarRadiusUU = 110.0f;
	static constexpr float T66TowerMesaPillarGridPitchUU = 950.0f;
	static constexpr float T66TowerMesaPillarCornerInsetUU = 170.0f;

	/** Exact-collision contract: round elements may only ship when the basic shape carries its own simple collision. */
	static bool T66MeshHasSimpleCollision(const UStaticMesh* Mesh)
	{
		const UBodySetup* BodySetup = Mesh ? Mesh->GetBodySetup() : nullptr;
		return BodySetup && BodySetup->AggGeom.GetElementCount() > 0;
	}

	/**
	 * One batched HISM actor whose instances CARRY their mesh's simple collision
	 * (deck pillars, round stepping stones). Carries both terrain sync tags so the
	 * stateful floor pass hides it AND disables its collision off-floor. Camera
	 * channel is ignored (pillars/stones never gate the boom or the fade).
	 */
	static AActor* T66SpawnCollidingInstancedMeshActor(
		UWorld* World,
		UStaticMesh* Mesh,
		UMaterialInterface* Material,
		const TArray<FTransform>& InstanceTransforms,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& ExtraTags,
		const TCHAR* ComponentName)
	{
		if (!World || !Mesh || InstanceTransforms.Num() <= 0 || !T66MeshHasSimpleCollision(Mesh))
		{
			return nullptr;
		}

		AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (!Actor)
		{
			return nullptr;
		}

		UHierarchicalInstancedStaticMeshComponent* MeshComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(
			Actor,
			MakeUniqueObjectName(Actor, UHierarchicalInstancedStaticMeshComponent::StaticClass(), FName(ComponentName)));
		if (!MeshComponent)
		{
			Actor->Destroy();
			return nullptr;
		}

		Actor->SetRootComponent(MeshComponent);
		Actor->AddInstanceComponent(MeshComponent);
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->SetStaticMesh(Mesh);
		MeshComponent->SetGenerateOverlapEvents(false);
		MeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		T66ConfigureTowerCollisionResponses(MeshComponent, true);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCanEverAffectNavigation(false);
		MeshComponent->SetCullDistances(0, T66GetTowerTuning().GeneratedDungeonKitCullDistance);
		for (const FName& Tag : ExtraTags)
		{
			if (!Tag.IsNone())
			{
				MeshComponent->ComponentTags.AddUnique(Tag);
			}
		}
		T66OptimizeTowerMeshComponent(MeshComponent);
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
		MeshComponent->PreAllocateInstancesMemory(InstanceTransforms.Num());
		MeshComponent->AddInstances(InstanceTransforms, false, false, false);
		MeshComponent->RegisterComponent();
		MeshComponent->SetMobility(EComponentMobility::Static);

		Actor->Tags.AddUnique(T66TowerMapTerrainVisualTag);
		Actor->Tags.AddUnique(T66TowerMapTerrainMaterialsReadyTag);
		Actor->Tags.AddUnique(T66TowerMapTerrainCollisionProxyTag);
		for (const FName& Tag : ExtraTags)
		{
			if (!Tag.IsNone())
			{
				Actor->Tags.AddUnique(Tag);
			}
		}
		if (GT66TowerTerrainActiveSpawnStats)
		{
			++GT66TowerTerrainActiveSpawnStats->HISMComponents;
			GT66TowerTerrainActiveSpawnStats->HISMInstances += InstanceTransforms.Num();
		}
		return Actor;
	}

	/** Scale transform that maps a basic shape's native bounds onto the desired half extents. */
	static FTransform T66MakeBasicShapeInstanceTransform(
		const UStaticMesh* Mesh,
		const FVector& Center,
		const FVector& DesiredHalfExtents,
		const float YawDegrees = 0.0f)
	{
		const FVector NativeExtents = Mesh ? Mesh->GetBounds().BoxExtent : FVector(50.0f);
		const FVector Scale(
			DesiredHalfExtents.X / FMath::Max(NativeExtents.X, 1.0f),
			DesiredHalfExtents.Y / FMath::Max(NativeExtents.Y, 1.0f),
			DesiredHalfExtents.Z / FMath::Max(NativeExtents.Z, 1.0f));
		// 90-degree yaw steps only (kit AABBs are square, so scale-then-rotate
		// keeps the footprint inside the recorded Bounds).
		return FTransform(FRotator(0.0f, YawDegrees, 0.0f), Center, Scale);
	}

	static constexpr float T66TowerFloorBaffleTubeNativeLength = 100.0f;
	static constexpr float T66TowerFloorBaffleTubeNativeDiameter = 60.0f;

	static UStaticMesh* T66GetFloorBaffleTubeMesh()
	{
		// Function-local statics are invisible to the GC: without rooting, the mesh is
		// collected after the first gameplay world tears down and the cached pointer
		// dangles, crashing the next tower spawn inside SetStaticMesh (2026-06-10
		// EXCEPTION_ACCESS_VIOLATION on re-entering the Tribulation).
		static TObjectPtr<UStaticMesh> Cached = nullptr;
		if (!Cached)
		{
			Cached = LoadObject<UStaticMesh>(
				nullptr,
				TEXT("/Game/World/Terrain/TowerDungeon/Baffles/SM_BaffleTube.SM_BaffleTube"));
			if (Cached)
			{
				Cached->AddToRoot();
			}
		}
		return Cached.Get();
	}

	// Fall Guys candy accents for course geometry (rooted caches; null-safe callers
	// fall back to the theme floor material).
	static UMaterialInterface* T66LoadRootedAccentMaterial(const TCHAR* ObjectPath, TObjectPtr<UMaterialInterface>& Cached)
	{
		if (!Cached)
		{
			Cached = LoadObject<UMaterialInterface>(nullptr, ObjectPath);
			if (Cached)
			{
				Cached->AddToRoot();
			}
		}
		return Cached.Get();
	}

	static UMaterialInterface* T66GetFallGuysPlatformMaterial()
	{
		static TObjectPtr<UMaterialInterface> Cached = nullptr;
		return T66LoadRootedAccentMaterial(TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Platform.MI_FallGuys_Platform"), Cached);
	}

	static UMaterialInterface* T66GetFallGuysRampMaterial()
	{
		static TObjectPtr<UMaterialInterface> Cached = nullptr;
		return T66LoadRootedAccentMaterial(TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Ramp.MI_FallGuys_Ramp"), Cached);
	}

	static UMaterialInterface* T66GetFallGuysMesaMaterial()
	{
		static TObjectPtr<UMaterialInterface> Cached = nullptr;
		return T66LoadRootedAccentMaterial(TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Mesa.MI_FallGuys_Mesa"), Cached);
	}

	static UMaterialInterface* T66GetFallGuysLiftMaterial()
	{
		static TObjectPtr<UMaterialInterface> Cached = nullptr;
		return T66LoadRootedAccentMaterial(TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Lift.MI_FallGuys_Lift"), Cached);
	}

	// FallGuysShapeKit01 platform prisms (exact 100^3 AABBs, 1-hull convex
	// collision baked at import — design ref section 1.6). Rooted like every
	// function-local asset cache (world-teardown GC crash class).
	static UStaticMesh* T66LoadRootedShapeMesh(const TCHAR* ObjectPath, TObjectPtr<UStaticMesh>& Cached)
	{
		if (!Cached)
		{
			Cached = LoadObject<UStaticMesh>(nullptr, ObjectPath);
			if (Cached)
			{
				Cached->AddToRoot();
			}
		}
		return Cached.Get();
	}

	static UStaticMesh* T66GetFGShapeHexMesh()
	{
		static TObjectPtr<UStaticMesh> Cached = nullptr;
		return T66LoadRootedShapeMesh(TEXT("/Game/World/Terrain/FallGuysKit/SM_FGShape_Hex.SM_FGShape_Hex"), Cached);
	}

	static UStaticMesh* T66GetFGShapeTriMesh()
	{
		static TObjectPtr<UStaticMesh> Cached = nullptr;
		return T66LoadRootedShapeMesh(TEXT("/Game/World/Terrain/FallGuysKit/SM_FGShape_Tri.SM_FGShape_Tri"), Cached);
	}

	// Beveled kit (Tier A, FALLGUYS_MAP_ANALYSIS.md P8): rounded chunky geometry.
	// BevelCube replaces the sharp basic cube for COURSE elements (stones, decks,
	// ramps, lift slabs); BevelPuck replaces the engine cylinder for round stones
	// and decks. Bevels only cut INWARD from the 100^3 AABB, so all scale and
	// box-gap math is unchanged.
	static UStaticMesh* T66GetFGShapeBevelCubeMesh()
	{
		static TObjectPtr<UStaticMesh> Cached = nullptr;
		return T66LoadRootedShapeMesh(TEXT("/Game/World/Terrain/FallGuysKit/SM_FGShape_BevelCube.SM_FGShape_BevelCube"), Cached);
	}

	static UStaticMesh* T66GetFGShapeBevelPuckMesh()
	{
		static TObjectPtr<UStaticMesh> Cached = nullptr;
		return T66LoadRootedShapeMesh(TEXT("/Game/World/Terrain/FallGuysKit/SM_FGShape_BevelPuck.SM_FGShape_BevelPuck"), Cached);
	}

	static UMaterialInterface* T66GetFallGuysDeckMaterial()
	{
		static TObjectPtr<UMaterialInterface> Cached = nullptr;
		return T66LoadRootedAccentMaterial(TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Deck.MI_FallGuys_Deck"), Cached);
	}

	static UMaterialInterface* T66GetFallGuysTrimMaterial()
	{
		static TObjectPtr<UMaterialInterface> Cached = nullptr;
		return T66LoadRootedAccentMaterial(TEXT("/Game/World/Terrain/FallGuysKit/MI_FallGuys_Trim.MI_FallGuys_Trim"), Cached);
	}

	// Hazard signature stripes (analysis P9): bounce pads launch the hero, so they
	// wear the same diagonal stripes as every other punting surface.
	static UMaterialInterface* T66GetHazardStripesMaterial()
	{
		static TObjectPtr<UMaterialInterface> Cached = nullptr;
		return T66LoadRootedAccentMaterial(TEXT("/Game/World/Traps/Inflatable/MI_Inflatable_StripesDiag.MI_Inflatable_StripesDiag"), Cached);
	}

	static float T66GetBafflePitch()
	{
		return FMath::Max(120.0f, CVarT66TowerFloorBafflePitch.GetValueOnAnyThread());
	}

	static float T66GetBaffleDiameter(const float Pitch)
	{
		const float RequestedDiameter = FMath::Max(120.0f, CVarT66TowerFloorBaffleDiameter.GetValueOnAnyThread());
		return FMath::Min(RequestedDiameter, Pitch * 1.25f);
	}

	static float T66GetBaffleSeamOverlap(const float TubeRadius)
	{
		return FMath::Clamp(TubeRadius * 0.75f, 60.0f, 320.0f);
	}

	static void T66ApplyBaffleMaterial(AActor* Actor, UMaterialInterface* Material)
	{
		if (!Actor || !Material)
		{
			return;
		}

		TInlineComponentArray<UStaticMeshComponent*> MeshComponents;
		Actor->GetComponents(MeshComponents);
		for (UStaticMeshComponent* MeshComponent : MeshComponents)
		{
			if (MeshComponent)
			{
				MeshComponent->SetMaterial(0, Material);
			}
		}
	}

	static bool T66BuildHorizontalBaffleTransforms(
		const FBox2D& SourceBox,
		const float SurfaceZ,
		const float GridOriginY,
		const bool bBodyExtendsDown,
		TArray<FTransform>& OutTransforms,
		const float PitchOverride = 0.0f,
		const float DiameterOverride = 0.0f)
	{
		const FVector2D BoxSize = SourceBox.Max - SourceBox.Min;
		if (BoxSize.X <= 10.0f || BoxSize.Y <= 10.0f)
		{
			return false;
		}

		const float Pitch = PitchOverride > 0.0f ? PitchOverride : T66GetBafflePitch();
		const float TubeDiameter = DiameterOverride > 0.0f
			? FMath::Min(DiameterOverride, Pitch * 1.25f)
			: T66GetBaffleDiameter(Pitch);
		const float TubeRadius = TubeDiameter * 0.5f;
		const float SeamOverlap = T66GetBaffleSeamOverlap(TubeRadius);
		const FVector2D VisualMin = SourceBox.Min - FVector2D(SeamOverlap, SeamOverlap);
		const FVector2D VisualMax = SourceBox.Max + FVector2D(SeamOverlap, SeamOverlap);
		const float VisualLengthX = VisualMax.X - VisualMin.X;
		if (VisualLengthX <= 10.0f)
		{
			return false;
		}

		const float CrossAxisScale = TubeDiameter / T66TowerFloorBaffleTubeNativeDiameter;
		const float XScale = VisualLengthX / T66TowerFloorBaffleTubeNativeLength;
		const float CenterX = (VisualMin.X + VisualMax.X) * 0.5f;
		const float CenterZ = bBodyExtendsDown ? SurfaceZ - TubeRadius : SurfaceZ + TubeRadius;
		const int32 FirstRow = FMath::FloorToInt((VisualMin.Y - TubeRadius - GridOriginY) / Pitch);
		const int32 LastRow = FMath::CeilToInt((VisualMax.Y + TubeRadius - GridOriginY) / Pitch);
		const int32 MaxRows = FMath::Max(1, LastRow - FirstRow + 1);

		OutTransforms.Reserve(OutTransforms.Num() + MaxRows);
		for (int32 Row = FirstRow; Row <= LastRow; ++Row)
		{
			const float CenterY = GridOriginY + (static_cast<float>(Row) * Pitch);
			if (CenterY < VisualMin.Y - TubeRadius || CenterY > VisualMax.Y + TubeRadius)
			{
				continue;
			}

			OutTransforms.Add(FTransform(
				FRotator::ZeroRotator,
				FVector(CenterX, CenterY, CenterZ),
				FVector(XScale, CrossAxisScale, CrossAxisScale)));
		}

		return OutTransforms.Num() > 0;
	}

	static bool T66SpawnFloorBaffleTubeVisualsForBoxAtZ(
		UWorld* World,
		const FBox2D& SourceBox,
		const T66TowerMapTerrain::FFloor& Floor,
		const float SurfaceZ,
		UMaterialInterface* FloorMaterial,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags)
	{
		const FVector2D BoxSize = SourceBox.Max - SourceBox.Min;
		if (!World || BoxSize.X <= 10.0f || BoxSize.Y <= 10.0f)
		{
			return false;
		}

		UStaticMesh* TubeMesh = T66GetFloorBaffleTubeMesh();
		if (!TubeMesh || !FloorMaterial)
		{
			static bool bLoggedMissingAsset = false;
			if (!bLoggedMissingAsset)
			{
				bLoggedMissingAsset = true;
				UE_LOG(
					LogT66TowerMapTerrain,
					Warning,
					TEXT("[MAP] Floor baffle visual fallback: missing TubeMesh=%d Material=%d."),
					TubeMesh ? 1 : 0,
					FloorMaterial ? 1 : 0);
			}
			return false;
		}

		TArray<FTransform> TubeTransforms;
		if (!T66BuildHorizontalBaffleTransforms(SourceBox, SurfaceZ, Floor.Center.Y, true, TubeTransforms))
		{
			return false;
		}

		TArray<FName> BaffleTags = Tags;
		BaffleTags.AddUnique(FName(TEXT("T66_Floor_Tower_Baffles")));
		BaffleTags.AddUnique(FName(*FString::Printf(TEXT("T66_Floor_Tower_Baffles_%02d"), Floor.FloorNumber)));

		TArray<UStaticMesh*> Meshes;
		Meshes.Add(TubeMesh);
		TArray<TArray<FTransform>> InstanceTransformsByMesh;
		InstanceTransformsByMesh.Add(MoveTemp(TubeTransforms));
		AActor* BaffleActor = T66SpawnGeneratedDungeonInstancedMeshActor(
			World,
			Meshes,
			InstanceTransformsByMesh,
			SpawnParams,
			BaffleTags,
			TEXT("FloorBaffleTube"),
			true);
		if (!BaffleActor)
		{
			return false;
		}

		T66ApplyBaffleMaterial(BaffleActor, FloorMaterial);
		return true;
	}

	static bool T66SpawnFloorBaffleTubeVisualsForBox(
		UWorld* World,
		const FBox2D& SourceBox,
		const T66TowerMapTerrain::FFloor& Floor,
		UMaterialInterface* FloorMaterial,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags)
	{
		return T66SpawnFloorBaffleTubeVisualsForBoxAtZ(World, SourceBox, Floor, Floor.SurfaceZ, FloorMaterial, SpawnParams, Tags);
	}

	static bool T66SpawnCeilingBaffleTubeVisualsForBox(
		UWorld* World,
		const FBox2D& SourceBox,
		const T66TowerMapTerrain::FFloor& Floor,
		const float CeilingBottomZ,
		UMaterialInterface* CeilingMaterial,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags)
	{
		const FVector2D BoxSize = SourceBox.Max - SourceBox.Min;
		if (!World || BoxSize.X <= 10.0f || BoxSize.Y <= 10.0f)
		{
			return false;
		}

		UStaticMesh* TubeMesh = T66GetFloorBaffleTubeMesh();
		if (!TubeMesh || !CeilingMaterial)
		{
			static bool bLoggedMissingAsset = false;
			if (!bLoggedMissingAsset)
			{
				bLoggedMissingAsset = true;
				UE_LOG(
					LogT66TowerMapTerrain,
					Warning,
					TEXT("[MAP] Ceiling baffle visual fallback: missing TubeMesh=%d Material=%d."),
					TubeMesh ? 1 : 0,
					CeilingMaterial ? 1 : 0);
			}
			return false;
		}

		TArray<FTransform> TubeTransforms;
		if (!T66BuildHorizontalBaffleTransforms(SourceBox, CeilingBottomZ, Floor.Center.Y, false, TubeTransforms))
		{
			return false;
		}

		TArray<FName> BaffleTags = Tags;
		BaffleTags.AddUnique(FName(TEXT("T66_Ceiling_Tower_Baffles")));
		BaffleTags.AddUnique(FName(*FString::Printf(TEXT("T66_Ceiling_Tower_Baffles_%02d"), Floor.FloorNumber)));

		TArray<UStaticMesh*> Meshes;
		Meshes.Add(TubeMesh);
		TArray<TArray<FTransform>> InstanceTransformsByMesh;
		InstanceTransformsByMesh.Add(MoveTemp(TubeTransforms));
		AActor* BaffleActor = T66SpawnGeneratedDungeonInstancedMeshActor(
			World,
			Meshes,
			InstanceTransformsByMesh,
			SpawnParams,
			BaffleTags,
			TEXT("CeilingBaffleTube"),
			true);
		if (!BaffleActor)
		{
			return false;
		}

		T66ApplyBaffleMaterial(BaffleActor, CeilingMaterial);
		return true;
	}

	static bool T66SpawnWallBaffleTubeVisualsForBox(
		UWorld* World,
		const FBox2D& WallBox,
		const float BaseZ,
		const float DesiredHeight,
		UMaterialInterface* WallMaterial,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags,
		const bool bIgnoreCameraChannel)
	{
		const FVector2D BoxSize = WallBox.Max - WallBox.Min;
		if (!World || BoxSize.X <= 10.0f || BoxSize.Y <= 10.0f || DesiredHeight <= 10.0f)
		{
			return false;
		}

		UStaticMesh* TubeMesh = T66GetFloorBaffleTubeMesh();
		if (!TubeMesh || !WallMaterial)
		{
			static bool bLoggedMissingAsset = false;
			if (!bLoggedMissingAsset)
			{
				bLoggedMissingAsset = true;
				UE_LOG(
					LogT66TowerMapTerrain,
					Warning,
					TEXT("[MAP] Wall baffle visual fallback: missing TubeMesh=%d Material=%d."),
					TubeMesh ? 1 : 0,
					WallMaterial ? 1 : 0);
			}
			return false;
		}

		const float Pitch = T66GetBafflePitch();
		const float TubeDiameter = T66GetBaffleDiameter(Pitch);
		const float TubeRadius = TubeDiameter * 0.5f;
		const float SeamOverlap = T66GetBaffleSeamOverlap(TubeRadius);
		const float CrossAxisScale = TubeDiameter / T66TowerFloorBaffleTubeNativeDiameter;
		const bool bRunAlongX = BoxSize.X >= BoxSize.Y;
		const float SpanLength = (bRunAlongX ? BoxSize.X : BoxSize.Y) + (SeamOverlap * 2.0f);
		const float AxisScale = SpanLength / T66TowerFloorBaffleTubeNativeLength;
		const float CenterX = (WallBox.Min.X + WallBox.Max.X) * 0.5f;
		const float CenterY = (WallBox.Min.Y + WallBox.Max.Y) * 0.5f;
		const float StartZ = BaseZ + TubeRadius - SeamOverlap;
		const float EndZ = BaseZ + DesiredHeight - TubeRadius + SeamOverlap;
		const int32 TubeCount = FMath::Max(1, FMath::CeilToInt(FMath::Max(0.0f, EndZ - StartZ) / Pitch) + 1);
		const FRotator TubeRotation = bRunAlongX ? FRotator::ZeroRotator : FRotator(0.0f, 90.0f, 0.0f);

		TArray<FTransform> TubeTransforms;
		TubeTransforms.Reserve(TubeCount);
		for (int32 TubeIndex = 0; TubeIndex < TubeCount; ++TubeIndex)
		{
			float CenterZ = StartZ + (static_cast<float>(TubeIndex) * Pitch);
			if (TubeIndex == TubeCount - 1)
			{
				CenterZ = FMath::Min(CenterZ, EndZ);
			}
			TubeTransforms.Add(FTransform(
				TubeRotation,
				FVector(CenterX, CenterY, CenterZ),
				FVector(AxisScale, CrossAxisScale, CrossAxisScale)));
		}

		TArray<FName> BaffleTags = Tags;
		BaffleTags.AddUnique(FName(TEXT("T66_Wall_Tower_Baffles")));

		TArray<UStaticMesh*> Meshes;
		Meshes.Add(TubeMesh);
		TArray<TArray<FTransform>> InstanceTransformsByMesh;
		InstanceTransformsByMesh.Add(MoveTemp(TubeTransforms));
		AActor* BaffleActor = T66SpawnGeneratedDungeonInstancedMeshActor(
			World,
			Meshes,
			InstanceTransformsByMesh,
			SpawnParams,
			BaffleTags,
			TEXT("WallBaffleTube"),
			bIgnoreCameraChannel);
		if (!BaffleActor)
		{
			return false;
		}

		T66ApplyBaffleMaterial(BaffleActor, WallMaterial);
		return true;
	}

	// -----------------------------------------------------------------------
	// Tier terrain spawning: solid hidden box proxies for mesas, rotated wedge
	// proxies for tier ramps, with three distinct visual tube scales — thinner
	// mesa-top baffles, stacked skirt tubes on exposed mesa edges, and small
	// roller tubes packed across ramp slopes ("smaller cylinders composing a
	// ramp"). All tube visuals batch into one instanced actor per floor.
	// -----------------------------------------------------------------------

	static void T66SpawnTierTerrainForFloor(
		UWorld* World,
		UStaticMesh* CubeMesh,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const FActorSpawnParameters& SpawnParams)
	{
		if (!World || (Floor.TierMesas.Num() <= 0 && Floor.TierRamps.Num() <= 0 && Floor.TierLifts.Num() <= 0))
		{
			return;
		}

		const TArray<FName> TierTags = {
			FName(*FString::Printf(TEXT("T66_Floor_Tower_%02d"), Floor.FloorNumber)),
			FName(*FString::Printf(TEXT("T66_Floor_Tower_Tier_%02d"), Floor.FloorNumber))
		};
		UMaterialInterface* SurfaceMaterial = Theme.FloorMaterial ? Theme.FloorMaterial : Theme.WallMaterial;
		UStaticMesh* TubeMesh = T66GetFloorBaffleTubeMesh();
		const bool bUseBaffles = T66ShouldUseFloorBaffles() && TubeMesh && SurfaceMaterial;
		const float TierTopZ = Floor.SurfaceZ + Layout.TierHeight;

		TArray<FTransform> BatchedTubeTransforms;

		// Mesas: elevated walkable DECKS, not solid blocks (design ref section 1.5).
		// A 60uu slab proxy owns the walkable top; round pillars hold it up; the
		// ground underneath stays open, playable, and connected to the room ring.
		UStaticMesh* PillarMesh = FT66VisualUtil::GetBasicShapeCylinder();
		TArray<FTransform> MesaPillarTransforms;
		// Beveled deck visual + white edge bands (analysis P8/A2); sharp cube and
		// no-trim remain the fallbacks.
		UStaticMesh* MesaDeckMesh = T66GetFGShapeBevelCubeMesh();
		if (!MesaDeckMesh)
		{
			MesaDeckMesh = CubeMesh;
		}
		UMaterialInterface* MesaTrimMaterial = T66GetFallGuysTrimMaterial();
		TArray<FTransform> MesaTrimTransforms;
		auto AppendMesaTrims = [&](const FVector2D& RectCenter, const FVector2D& RectHalf, const float DeckTopZ)
		{
			if (!CubeMesh || !MesaTrimMaterial)
			{
				return;
			}
			const float BandHalfHeight = 12.0f;
			const float BandHalfThickness = 3.0f;
			const float BandCenterZ = DeckTopZ - BandHalfHeight;
			MesaTrimTransforms.Add(T66MakeBasicShapeInstanceTransform(CubeMesh,
				FVector(RectCenter.X, RectCenter.Y - RectHalf.Y - BandHalfThickness, BandCenterZ),
				FVector(RectHalf.X + (BandHalfThickness * 2.0f), BandHalfThickness, BandHalfHeight)));
			MesaTrimTransforms.Add(T66MakeBasicShapeInstanceTransform(CubeMesh,
				FVector(RectCenter.X, RectCenter.Y + RectHalf.Y + BandHalfThickness, BandCenterZ),
				FVector(RectHalf.X + (BandHalfThickness * 2.0f), BandHalfThickness, BandHalfHeight)));
			MesaTrimTransforms.Add(T66MakeBasicShapeInstanceTransform(CubeMesh,
				FVector(RectCenter.X - RectHalf.X - BandHalfThickness, RectCenter.Y, BandCenterZ),
				FVector(BandHalfThickness, RectHalf.Y, BandHalfHeight)));
			MesaTrimTransforms.Add(T66MakeBasicShapeInstanceTransform(CubeMesh,
				FVector(RectCenter.X + RectHalf.X + BandHalfThickness, RectCenter.Y, BandCenterZ),
				FVector(BandHalfThickness, RectHalf.Y, BandHalfHeight)));
		};
		for (const T66TowerMapTerrain::FTierMesa& Mesa : Floor.TierMesas)
		{
			const float DeckBottomZ = TierTopZ - T66TowerDeckThicknessUU;
			UMaterialInterface* MesaMaterial = T66GetFallGuysMesaMaterial();
			TArray<FName> DeckTags = TierTags;
			DeckTags.AddUnique(T66TowerMapCameraOccludingWallVisualTag);
			DeckTags.AddUnique(T66TowerMapDeckVisualTag);

			// Proxy + matching visual for one deck rectangle. Visual matches the
			// collision slab EXACTLY (floating — no floor seam, no sink needed);
			// camera-occluding + deck tags let the boom and the occluder fade
			// handle the hero walking beneath.
			auto SpawnDeckPiece = [&](const FBox2D& Rect)
			{
				const FVector2D PieceCenter = (Rect.Min + Rect.Max) * 0.5f;
				const FVector2D PieceHalf = (Rect.Max - Rect.Min) * 0.5f;
				if (PieceHalf.X <= 10.0f || PieceHalf.Y <= 10.0f)
				{
					return;
				}

				T66SpawnHiddenCollisionProxyActor(
					World,
					FVector(PieceCenter.X, PieceCenter.Y, TierTopZ - (T66TowerDeckThicknessUU * 0.5f)),
					FRotator::ZeroRotator,
					FVector(PieceHalf.X, PieceHalf.Y, T66TowerDeckThicknessUU * 0.5f),
					SpawnParams,
					TierTags,
					true);

				bool bSpawnedTop = false;
				if (bUseBaffles)
				{
					// Parked baffle mode keeps the top tube read; side skirts are
					// gone — the sides are open air under a deck.
					const int32 CountBefore = BatchedTubeTransforms.Num();
					T66BuildHorizontalBaffleTransforms(
						Rect,
						TierTopZ,
						Floor.Center.Y,
						true,
						BatchedTubeTransforms,
						Layout.MesaTopBafflePitch,
						Layout.MesaTopBaffleDiameter);
					bSpawnedTop = BatchedTubeTransforms.Num() > CountBefore;
				}

				if (!bSpawnedTop && MesaDeckMesh && SurfaceMaterial)
				{
					T66SpawnEnvironmentRectangle(
						World,
						MesaDeckMesh,
						MesaMaterial ? MesaMaterial : SurfaceMaterial,
						FVector(PieceCenter.X, PieceCenter.Y, TierTopZ - (T66TowerDeckThicknessUU * 0.5f)),
						FVector(PieceHalf.X, PieceHalf.Y, T66TowerDeckThicknessUU * 0.5f),
						SpawnParams,
						DeckTags);
					AppendMesaTrims(PieceCenter, PieceHalf, TierTopZ);
				}
			};

			if (Mesa.HasHole())
			{
				// Ring deck: four frame slabs around the center drop hole.
				SpawnDeckPiece(FBox2D(
					FVector2D(Mesa.Bounds.Min.X, Mesa.HoleBounds.Max.Y),
					FVector2D(Mesa.Bounds.Max.X, Mesa.Bounds.Max.Y)));
				SpawnDeckPiece(FBox2D(
					FVector2D(Mesa.Bounds.Min.X, Mesa.Bounds.Min.Y),
					FVector2D(Mesa.Bounds.Max.X, Mesa.HoleBounds.Min.Y)));
				SpawnDeckPiece(FBox2D(
					FVector2D(Mesa.Bounds.Min.X, Mesa.HoleBounds.Min.Y),
					FVector2D(Mesa.HoleBounds.Min.X, Mesa.HoleBounds.Max.Y)));
				SpawnDeckPiece(FBox2D(
					FVector2D(Mesa.HoleBounds.Max.X, Mesa.HoleBounds.Min.Y),
					FVector2D(Mesa.Bounds.Max.X, Mesa.HoleBounds.Max.Y)));
			}
			else
			{
				SpawnDeckPiece(Mesa.Bounds);
			}

			// Pillar grid: corner-inset perimeter + interior pitch so spans read
			// supported. Pillars sink 2uu into floor and deck to avoid seam gaps.
			// Ring mesas skip stops in the hole (no deck above them) and rim the
			// hole corners instead so the inner frame edges read held up.
			if (PillarMesh)
			{
				const float PillarBottomZ = Floor.SurfaceZ - 2.0f;
				const float PillarTopZ = DeckBottomZ + 2.0f;
				const float PillarHalfZ = (PillarTopZ - PillarBottomZ) * 0.5f;
				const float PillarCenterZ = (PillarTopZ + PillarBottomZ) * 0.5f;
				auto AddPillar = [&](const float X, const float Y)
				{
					MesaPillarTransforms.Add(T66MakeBasicShapeInstanceTransform(
						PillarMesh,
						FVector(X, Y, PillarCenterZ),
						FVector(T66TowerMesaPillarRadiusUU, T66TowerMesaPillarRadiusUU, PillarHalfZ)));
				};
				auto BuildAxisStops = [](const float Min, const float Max, TArray<float>& OutStops)
				{
					OutStops.Reset();
					const float Lo = Min + T66TowerMesaPillarCornerInsetUU;
					const float Hi = Max - T66TowerMesaPillarCornerInsetUU;
					const int32 SpanCount = FMath::Max(1, FMath::CeilToInt((Hi - Lo) / T66TowerMesaPillarGridPitchUU));
					for (int32 Stop = 0; Stop <= SpanCount; ++Stop)
					{
						OutStops.Add(FMath::Lerp(Lo, Hi, static_cast<float>(Stop) / static_cast<float>(SpanCount)));
					}
				};
				TArray<float> StopsX;
				TArray<float> StopsY;
				BuildAxisStops(Mesa.Bounds.Min.X, Mesa.Bounds.Max.X, StopsX);
				BuildAxisStops(Mesa.Bounds.Min.Y, Mesa.Bounds.Max.Y, StopsY);
				for (const float X : StopsX)
				{
					for (const float Y : StopsY)
					{
						const bool bInsideHole = Mesa.HasHole()
							&& X > Mesa.HoleBounds.Min.X - T66TowerMesaPillarRadiusUU
							&& X < Mesa.HoleBounds.Max.X + T66TowerMesaPillarRadiusUU
							&& Y > Mesa.HoleBounds.Min.Y - T66TowerMesaPillarRadiusUU
							&& Y < Mesa.HoleBounds.Max.Y + T66TowerMesaPillarRadiusUU;
						if (!bInsideHole)
						{
							AddPillar(X, Y);
						}
					}
				}
				if (Mesa.HasHole())
				{
					const float RimInset = T66TowerMesaPillarRadiusUU + 30.0f;
					AddPillar(Mesa.HoleBounds.Min.X - RimInset, Mesa.HoleBounds.Min.Y - RimInset);
					AddPillar(Mesa.HoleBounds.Max.X + RimInset, Mesa.HoleBounds.Min.Y - RimInset);
					AddPillar(Mesa.HoleBounds.Min.X - RimInset, Mesa.HoleBounds.Max.Y + RimInset);
					AddPillar(Mesa.HoleBounds.Max.X + RimInset, Mesa.HoleBounds.Max.Y + RimInset);
				}
			}
		}

		if (MesaPillarTransforms.Num() > 0)
		{
			UMaterialInterface* MesaMaterial = T66GetFallGuysMesaMaterial();
			T66SpawnCollidingInstancedMeshActor(
				World,
				PillarMesh,
				MesaMaterial ? MesaMaterial : SurfaceMaterial,
				MesaPillarTransforms,
				SpawnParams,
				TierTags,
				TEXT("MesaDeckPillar"));
		}
		if (MesaTrimTransforms.Num() > 0 && CubeMesh && MesaTrimMaterial)
		{
			TArray<UStaticMesh*> TrimMeshes;
			TrimMeshes.Add(CubeMesh);
			TArray<TArray<FTransform>> TrimTransformsByMesh;
			TrimTransformsByMesh.Add(MoveTemp(MesaTrimTransforms));
			AActor* TrimActor = T66SpawnGeneratedDungeonInstancedMeshActor(
				World,
				TrimMeshes,
				TrimTransformsByMesh,
				SpawnParams,
				TierTags,
				TEXT("MesaEdgeTrim"),
				true);
			if (TrimActor)
			{
				T66ApplyBaffleMaterial(TrimActor, MesaTrimMaterial);
			}
		}

		// Tier ramps: rotated wedge proxy + roller tubes laid across the slope.
		for (const T66TowerMapTerrain::FTierRamp& Ramp : Floor.TierRamps)
		{
			const FVector2D Center = (Ramp.Bounds.Min + Ramp.Bounds.Max) * 0.5f;
			const FVector2D HalfExtents = (Ramp.Bounds.Max - Ramp.Bounds.Min) * 0.5f;
			const bool bAlongX = Ramp.AscentSign.X != 0;
			const float RunLength = 2.0f * (bAlongX ? HalfExtents.X : HalfExtents.Y);
			const float AcrossHalf = bAlongX ? HalfExtents.Y : HalfExtents.X;
			const float Rise = Layout.TierHeight;
			if (RunLength <= 10.0f || AcrossHalf <= 10.0f)
			{
				continue;
			}

			const float SlopeRadians = FMath::Atan2(Rise, RunLength);
			const float HypotenuseHalf = 0.5f * FMath::Sqrt((RunLength * RunLength) + (Rise * Rise));
			const float Yaw = bAlongX
				? ((Ramp.AscentSign.X > 0) ? 0.0f : 180.0f)
				: ((Ramp.AscentSign.Y > 0) ? 90.0f : -90.0f);
			const FRotator RampRotation(FMath::RadiansToDegrees(SlopeRadians), Yaw, 0.0f);
			const FVector RampCenter(Center.X, Center.Y, Floor.SurfaceZ + (Rise * 0.5f));

			T66SpawnHiddenCollisionProxyActor(
				World,
				RampCenter,
				RampRotation,
				FVector(HypotenuseHalf, AcrossHalf, 12.0f),
				SpawnParams,
				TierTags,
				true);

			if (bUseBaffles)
			{
				// Roller tubes: small cylinders side by side climbing the slope.
				const float RollerDiameter = Layout.RampRollerDiameter;
				const float RollerSpacing = RollerDiameter * 1.05f;
				const float SlopeLength = HypotenuseHalf * 2.0f;
				const int32 RollerCount = FMath::Max(2, FMath::FloorToInt(SlopeLength / RollerSpacing));
				const FVector2D AscentDir(static_cast<float>(Ramp.AscentSign.X), static_cast<float>(Ramp.AscentSign.Y));
				const FVector2D StartXY = Center - (AscentDir * (RunLength * 0.5f));
				const float AcrossScale = (AcrossHalf * 2.0f) / T66TowerFloorBaffleTubeNativeLength;
				const float CrossScale = RollerDiameter / T66TowerFloorBaffleTubeNativeDiameter;
				const FRotator RollerRotation = bAlongX ? FRotator(0.0f, 90.0f, 0.0f) : FRotator::ZeroRotator;
				for (int32 RollerIndex = 0; RollerIndex < RollerCount; ++RollerIndex)
				{
					const float T = (static_cast<float>(RollerIndex) + 0.5f) / static_cast<float>(RollerCount);
					const FVector2D XY = StartXY + (AscentDir * (T * RunLength));
					const float Z = Floor.SurfaceZ + (T * Rise) + (RollerDiameter * 0.15f);
					BatchedTubeTransforms.Add(FTransform(
						RollerRotation,
						FVector(XY.X, XY.Y, Z),
						FVector(AcrossScale, CrossScale, CrossScale)));
				}
			}
			else if (CubeMesh && SurfaceMaterial)
			{
				UMaterialInterface* RampMaterial = T66GetFallGuysRampMaterial();
				T66SpawnStaticMeshActor(
					World,
					MesaDeckMesh ? MesaDeckMesh : CubeMesh,
					RampMaterial ? RampMaterial : SurfaceMaterial,
					RampCenter,
					RampRotation,
					FVector(HypotenuseHalf, AcrossHalf, 12.0f),
					SpawnParams,
					false,
					TierTags,
					true);
			}
		}

		if (BatchedTubeTransforms.Num() > 0)
		{
			TArray<FName> BaffleTags = TierTags;
			BaffleTags.AddUnique(FName(TEXT("T66_Floor_Tower_Baffles")));
			BaffleTags.AddUnique(FName(*FString::Printf(TEXT("T66_Tier_Tower_Baffles_%02d"), Floor.FloorNumber)));
			TArray<UStaticMesh*> Meshes;
			Meshes.Add(TubeMesh);
			TArray<TArray<FTransform>> InstanceTransformsByMesh;
			InstanceTransformsByMesh.Add(MoveTemp(BatchedTubeTransforms));
			AActor* BaffleActor = T66SpawnGeneratedDungeonInstancedMeshActor(
				World,
				Meshes,
				InstanceTransformsByMesh,
				SpawnParams,
				BaffleTags,
				TEXT("TierTerrainTube"),
				true);
			if (BaffleActor)
			{
				T66ApplyBaffleMaterial(BaffleActor, SurfaceMaterial);
			}
		}

		// Moving lift platforms: one self-ticking actor per lift owns BOTH the
		// moving hidden box proxy and the candy slab visual, so it carries both
		// terrain sync tags — the stateful floor visibility pass hides it AND
		// disables its collision when the hero is on another floor. The deck
		// parks 30uu above the floor (below step height: boarding is a walk-on)
		// and tops out flush with the mesa surface.
		int32 SpawnedLiftCount = 0;
		for (const T66TowerMapTerrain::FTierLift& Lift : Floor.TierLifts)
		{
			const FVector2D LiftCenter = (Lift.Bounds.Min + Lift.Bounds.Max) * 0.5f;
			const FVector2D LiftHalfExtents = (Lift.Bounds.Max - Lift.Bounds.Min) * 0.5f;
			if (LiftHalfExtents.X <= 10.0f || LiftHalfExtents.Y <= 10.0f || Lift.TopZ - Lift.BaseZ <= 10.0f)
			{
				continue;
			}

			AT66TowerLiftPlatform* LiftActor = World->SpawnActor<AT66TowerLiftPlatform>(
				AT66TowerLiftPlatform::StaticClass(),
				FVector(LiftCenter.X, LiftCenter.Y, Lift.BaseZ),
				FRotator::ZeroRotator,
				SpawnParams);
			if (!LiftActor)
			{
				continue;
			}

			UMaterialInterface* LiftMaterial = T66GetFallGuysLiftMaterial();
			if (!LiftMaterial)
			{
				LiftMaterial = T66GetFallGuysPlatformMaterial();
			}
			LiftActor->InitLift(
				MesaDeckMesh ? MesaDeckMesh : CubeMesh,
				LiftMaterial ? LiftMaterial : SurfaceMaterial,
				LiftCenter,
				LiftHalfExtents,
				Lift.BaseZ + 30.0f,
				Lift.TopZ,
				Layout.LiftTravelSeconds,
				Layout.LiftDwellSeconds,
				Lift.PhaseFraction);

			LiftActor->Tags.AddUnique(T66TowerMapTerrainVisualTag);
			LiftActor->Tags.AddUnique(T66TowerMapTerrainCollisionProxyTag);
			LiftActor->Tags.AddUnique(FName(TEXT("T66_Tower_LiftPlatform")));
			for (const FName& Tag : TierTags)
			{
				LiftActor->Tags.AddUnique(Tag);
			}
			++SpawnedLiftCount;
		}

		// Bounce pads (analysis C3): striped trampolines the room composer placed —
		// pit escapes, deck shortcuts. Walk-on disc, capsule launch only.
		int32 SpawnedPadCount = 0;
		UStaticMesh* PadMesh = T66GetFGShapeBevelPuckMesh();
		if (!PadMesh || !T66MeshHasSimpleCollision(PadMesh))
		{
			PadMesh = FT66VisualUtil::GetBasicShapeCylinder();
		}
		for (const FVector& PadSpot : Floor.BouncePadSpots)
		{
			AT66BouncePadObstacle* Pad = World->SpawnActor<AT66BouncePadObstacle>(
				AT66BouncePadObstacle::StaticClass(),
				FVector(PadSpot.X, PadSpot.Y, PadSpot.Z + (AT66BouncePadObstacle::PadThickness * 0.5f) - 1.0f),
				FRotator::ZeroRotator,
				SpawnParams);
			if (!Pad)
			{
				continue;
			}

			UMaterialInterface* PadMaterial = T66GetHazardStripesMaterial();
			Pad->InitPad(PadMesh, PadMaterial ? PadMaterial : SurfaceMaterial, 240.0f);
			Pad->Tags.AddUnique(T66TowerMapTerrainVisualTag);
			Pad->Tags.AddUnique(T66TowerMapTerrainCollisionProxyTag);
			Pad->Tags.AddUnique(FName(TEXT("T66_Tower_BouncePad")));
			for (const FName& Tag : TierTags)
			{
				Pad->Tags.AddUnique(Tag);
			}
			++SpawnedPadCount;
		}

		UE_LOG(
			LogT66TowerMapTerrain,
			Log,
			TEXT("[MAP] Tier terrain spawned floor=%d mesas=%d ramps=%d lifts=%d pads=%d baffles=%d."),
			Floor.FloorNumber,
			Floor.TierMesas.Num(),
			Floor.TierRamps.Num(),
			SpawnedLiftCount,
			SpawnedPadCount,
			bUseBaffles ? 1 : 0);
	}

	// -----------------------------------------------------------------------
	// Bounce platform / ramp spawning: hidden box proxies own all collision,
	// baffle tubes (tops + inflated sides) own the visuals, with themed cube
	// prisms as the no-baffle fallback. Platform surfaces intentionally carry
	// no T66_NoSurfaceBounce tag so the hero's surface-bounce impulses apply.
	// -----------------------------------------------------------------------

	static void T66SpawnBounceCourseForFloor(
		UWorld* World,
		UStaticMesh* CubeMesh,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const FActorSpawnParameters& SpawnParams)
	{
		if (!World || (Floor.BouncePlatforms.Num() <= 0 && Floor.BounceRamps.Num() <= 0))
		{
			return;
		}

		const TArray<FName> PlatformTags = {
			FName(*FString::Printf(TEXT("T66_Floor_Tower_%02d"), Floor.FloorNumber)),
			FName(*FString::Printf(TEXT("T66_Floor_Tower_Platform_%02d"), Floor.FloorNumber))
		};
		UMaterialInterface* SurfaceMaterial = Theme.FloorMaterial ? Theme.FloorMaterial : Theme.WallMaterial;
		UStaticMesh* TubeMesh = T66GetFloorBaffleTubeMesh();
		const bool bUseBaffles = T66ShouldUseFloorBaffles() && TubeMesh && SurfaceMaterial;

		// A floor carries ~100+ platforms, so all platform tubes batch into ONE
		// instanced actor per floor (the 2026-05-05 lag fix budget is instance
		// counts on few actors, not many small actors). Collision proxies stay
		// per-platform like maze wall proxies.
		TArray<FTransform> BatchedTubeTransforms;
		const float Pitch = T66GetBafflePitch();
		const float TubeDiameter = T66GetBaffleDiameter(Pitch);
		const float TubeRadius = TubeDiameter * 0.5f;
		const float SeamOverlap = T66GetBaffleSeamOverlap(TubeRadius);
		const float CrossAxisScale = TubeDiameter / T66TowerFloorBaffleTubeNativeDiameter;
		auto AppendSkirtTransforms = [&](const FBox2D& SideBox, const float BaseZ, const float Height)
		{
			const FVector2D BoxSize = SideBox.Max - SideBox.Min;
			if (BoxSize.X <= 10.0f || BoxSize.Y <= 10.0f || Height <= 10.0f)
			{
				return;
			}

			const bool bRunAlongX = BoxSize.X >= BoxSize.Y;
			const float SpanLength = (bRunAlongX ? BoxSize.X : BoxSize.Y) + (SeamOverlap * 2.0f);
			const float AxisScale = SpanLength / T66TowerFloorBaffleTubeNativeLength;
			const float CenterX = (SideBox.Min.X + SideBox.Max.X) * 0.5f;
			const float CenterY = (SideBox.Min.Y + SideBox.Max.Y) * 0.5f;
			const float StartZ = BaseZ + TubeRadius - SeamOverlap;
			const float EndZ = BaseZ + Height - TubeRadius + SeamOverlap;
			const int32 TubeCount = FMath::Max(1, FMath::CeilToInt(FMath::Max(0.0f, EndZ - StartZ) / Pitch) + 1);
			const FRotator TubeRotation = bRunAlongX ? FRotator::ZeroRotator : FRotator(0.0f, 90.0f, 0.0f);
			for (int32 TubeIndex = 0; TubeIndex < TubeCount; ++TubeIndex)
			{
				float CenterZ = StartZ + (static_cast<float>(TubeIndex) * Pitch);
				if (TubeIndex == TubeCount - 1)
				{
					CenterZ = FMath::Min(CenterZ, EndZ);
				}
				BatchedTubeTransforms.Add(FTransform(
					TubeRotation,
					FVector(CenterX, CenterY, CenterZ),
					FVector(AxisScale, CrossAxisScale, CrossAxisScale)));
			}
		};

		// Elevated-deck architecture + shape language (design ref sections 1.5/1.6):
		// Tier 1 platforms are grounded stepping stones (the 200uu hero cannot fit
		// under +200 anyway); Tier 2 platforms are floating 60uu deck slabs on one
		// round center pillar with the ground underneath playable. Shaped variants
		// (round/hex/triangle) carry their mesh's own EXACT simple collision.
		UMaterialInterface* PlatformMaterial = T66GetFallGuysPlatformMaterial();
		// Tier color-coding (analysis P9): grounded stones stay sunny yellow, Tier 2
		// floating decks read coral so height-as-progress is legible at a glance.
		UMaterialInterface* DeckMaterial = T66GetFallGuysDeckMaterial();
		if (!DeckMaterial)
		{
			DeckMaterial = PlatformMaterial;
		}
		UMaterialInterface* TrimMaterial = T66GetFallGuysTrimMaterial();
		// Beveled kit (analysis P8): rounded edges everywhere course geometry lives.
		// Engine primitives remain the defensive fallback.
		UStaticMesh* RoundMesh = T66GetFGShapeBevelPuckMesh();
		if (!RoundMesh || !T66MeshHasSimpleCollision(RoundMesh))
		{
			RoundMesh = FT66VisualUtil::GetBasicShapeCylinder();
		}
		UStaticMesh* SquareMesh = T66GetFGShapeBevelCubeMesh();
		UStaticMesh* HexMesh = T66GetFGShapeHexMesh();
		UStaticMesh* TriMesh = T66GetFGShapeTriMesh();
		const bool bRoundShapesAvailable = RoundMesh && T66MeshHasSimpleCollision(RoundMesh);
		auto ResolveShapeMesh = [&](const T66TowerMapTerrain::ET66BouncePlatformShape Shape) -> UStaticMesh*
		{
			// Defensive fallback: a shape only ships when its mesh carries exact
			// simple collision (exact-collision contract); squares fall back to
			// the box-proxy + sharp-cube path below.
			UStaticMesh* Mesh = nullptr;
			switch (Shape)
			{
			case T66TowerMapTerrain::ET66BouncePlatformShape::Square: Mesh = SquareMesh; break;
			case T66TowerMapTerrain::ET66BouncePlatformShape::Round: Mesh = RoundMesh; break;
			case T66TowerMapTerrain::ET66BouncePlatformShape::Hex: Mesh = HexMesh; break;
			case T66TowerMapTerrain::ET66BouncePlatformShape::Triangle: Mesh = TriMesh; break;
			default: break;
			}
			return (Mesh && T66MeshHasSimpleCollision(Mesh)) ? Mesh : nullptr;
		};
		TArray<FTransform> SquareStoneTransforms;
		TArray<FTransform> RoundStoneTransforms;
		TArray<FTransform> HexStoneTransforms;
		TArray<FTransform> TriStoneTransforms;
		TArray<FTransform> DeckPillarTransforms;
		// White edge bands on rectangular decks (analysis A2): thin visual strips at
		// the deck top edge — the Fall Guys platform side-band read. Visual-only
		// (6uu decorative protrusion past the side face, same accepted class as the
		// 2uu floor sink).
		TArray<FTransform> TrimTransforms;
		UStaticMesh* TrimMesh = FT66VisualUtil::GetBasicShapeCube();
		auto AppendDeckTrims = [&](const FVector2D& RectCenter, const FVector2D& RectHalf, const float DeckTopZ)
		{
			if (!TrimMesh || !TrimMaterial)
			{
				return;
			}
			const float BandHalfHeight = 12.0f;
			const float BandHalfThickness = 3.0f;
			const float BandCenterZ = DeckTopZ - BandHalfHeight;
			TrimTransforms.Add(T66MakeBasicShapeInstanceTransform(TrimMesh,
				FVector(RectCenter.X, RectCenter.Y - RectHalf.Y - BandHalfThickness, BandCenterZ),
				FVector(RectHalf.X + (BandHalfThickness * 2.0f), BandHalfThickness, BandHalfHeight)));
			TrimTransforms.Add(T66MakeBasicShapeInstanceTransform(TrimMesh,
				FVector(RectCenter.X, RectCenter.Y + RectHalf.Y + BandHalfThickness, BandCenterZ),
				FVector(RectHalf.X + (BandHalfThickness * 2.0f), BandHalfThickness, BandHalfHeight)));
			TrimTransforms.Add(T66MakeBasicShapeInstanceTransform(TrimMesh,
				FVector(RectCenter.X - RectHalf.X - BandHalfThickness, RectCenter.Y, BandCenterZ),
				FVector(BandHalfThickness, RectHalf.Y, BandHalfHeight)));
			TrimTransforms.Add(T66MakeBasicShapeInstanceTransform(TrimMesh,
				FVector(RectCenter.X + RectHalf.X + BandHalfThickness, RectCenter.Y, BandCenterZ),
				FVector(BandHalfThickness, RectHalf.Y, BandHalfHeight)));
		};
		int32 SpawnedPlatformCount = 0;
		for (const T66TowerMapTerrain::FBouncePlatform& Platform : Floor.BouncePlatforms)
		{
			const FVector2D Center = (Platform.Bounds.Min + Platform.Bounds.Max) * 0.5f;
			const FVector2D HalfExtents = (Platform.Bounds.Max - Platform.Bounds.Min) * 0.5f;
			const float Height = Platform.TopZ - Floor.SurfaceZ;
			if (HalfExtents.X <= 10.0f || HalfExtents.Y <= 10.0f || Height <= 10.0f)
			{
				continue;
			}

			const bool bElevatedDeck = Platform.Tier >= 2 && Height > T66TowerDeckThicknessUU + 20.0f;
			UStaticMesh* ShapeMesh = ResolveShapeMesh(Platform.Shape);
			const float ShapeYaw = static_cast<float>(Platform.YawSteps) * 90.0f;

			if (!bElevatedDeck)
			{
				if (ShapeMesh)
				{
					// Grounded shaped stone: one batched instance owns BOTH the
					// visual and its exact collision (2uu floor sink, no proxy).
					TArray<FTransform>& StoneBatch = ShapeMesh == SquareMesh
						? SquareStoneTransforms
						: (ShapeMesh == RoundMesh
							? RoundStoneTransforms
							: (ShapeMesh == HexMesh ? HexStoneTransforms : TriStoneTransforms));
					StoneBatch.Add(T66MakeBasicShapeInstanceTransform(
						ShapeMesh,
						FVector(Center.X, Center.Y, Floor.SurfaceZ + (Height * 0.5f) - 1.0f),
						FVector(HalfExtents.X, HalfExtents.Y, (Height * 0.5f) + 1.0f),
						ShapeYaw));
					++SpawnedPlatformCount;
					continue;
				}

				T66SpawnHiddenCollisionProxyActor(
					World,
					FVector(Center.X, Center.Y, Floor.SurfaceZ + (Height * 0.5f)),
					FRotator::ZeroRotator,
					FVector(HalfExtents.X, HalfExtents.Y, Height * 0.5f),
					SpawnParams,
					PlatformTags,
					true);

				bool bSpawnedVisual = false;
				if (bUseBaffles)
				{
					// Tube deck flush with the collision top; side skirts only make
					// sense on grounded prisms (solid sides).
					const int32 TransformCountBefore = BatchedTubeTransforms.Num();
					T66BuildHorizontalBaffleTransforms(Platform.Bounds, Platform.TopZ, Floor.Center.Y, true, BatchedTubeTransforms);

					const float SkirtThickness = 40.0f;
					const float SkirtInset = 20.0f;
					AppendSkirtTransforms(
						FBox2D(
							FVector2D(Platform.Bounds.Min.X - SkirtInset, Platform.Bounds.Min.Y - SkirtInset),
							FVector2D(Platform.Bounds.Max.X + SkirtInset, Platform.Bounds.Min.Y + SkirtThickness)),
						Floor.SurfaceZ,
						Height);
					AppendSkirtTransforms(
						FBox2D(
							FVector2D(Platform.Bounds.Min.X - SkirtInset, Platform.Bounds.Max.Y - SkirtThickness),
							FVector2D(Platform.Bounds.Max.X + SkirtInset, Platform.Bounds.Max.Y + SkirtInset)),
						Floor.SurfaceZ,
						Height);
					AppendSkirtTransforms(
						FBox2D(
							FVector2D(Platform.Bounds.Min.X - SkirtInset, Platform.Bounds.Min.Y - SkirtInset),
							FVector2D(Platform.Bounds.Min.X + SkirtThickness, Platform.Bounds.Max.Y + SkirtInset)),
						Floor.SurfaceZ,
						Height);
					AppendSkirtTransforms(
						FBox2D(
							FVector2D(Platform.Bounds.Max.X - SkirtThickness, Platform.Bounds.Min.Y - SkirtInset),
							FVector2D(Platform.Bounds.Max.X + SkirtInset, Platform.Bounds.Max.Y + SkirtInset)),
						Floor.SurfaceZ,
						Height);

					bSpawnedVisual = BatchedTubeTransforms.Num() > TransformCountBefore;
				}

				if (!bSpawnedVisual && CubeMesh && SurfaceMaterial)
				{
					// Sink the prism 2uu into the floor so the coplanar bottom face
					// cannot z-fight the floor slab at the seam.
					T66SpawnEnvironmentRectangle(
						World,
						CubeMesh,
						PlatformMaterial ? PlatformMaterial : SurfaceMaterial,
						FVector(Center.X, Center.Y, Floor.SurfaceZ + (Height * 0.5f) - 1.0f),
						FVector(HalfExtents.X, HalfExtents.Y, (Height * 0.5f) + 1.0f),
						SpawnParams,
						PlatformTags,
						true);
				}

				++SpawnedPlatformCount;
				continue;
			}

			// Floating deck (Tier 2): 60uu slab at the top, one round center pillar,
			// open playable ground underneath (Tail Tag bridge read). Coral deck
			// color = height-as-progress (analysis P9).
			const float DeckBottomZ = Platform.TopZ - T66TowerDeckThicknessUU;
			if (ShapeMesh)
			{
				// Shaped deck: the prism/cylinder visual carries its exact collision,
				// blocks the camera channel (boom + fade), and both sync tags keep
				// the stateful floor pass toggling visibility AND collision.
				TArray<FName> ShapedDeckTags = PlatformTags;
				ShapedDeckTags.AddUnique(T66TowerMapCameraOccludingWallVisualTag);
				ShapedDeckTags.AddUnique(T66TowerMapDeckVisualTag);
				ShapedDeckTags.AddUnique(T66TowerMapTerrainCollisionProxyTag);
				T66SpawnStaticMeshActor(
					World,
					ShapeMesh,
					DeckMaterial ? DeckMaterial : SurfaceMaterial,
					FVector(Center.X, Center.Y, Platform.TopZ - (T66TowerDeckThicknessUU * 0.5f)),
					FRotator(0.0f, ShapeYaw, 0.0f),
					FVector(HalfExtents.X, HalfExtents.Y, T66TowerDeckThicknessUU * 0.5f),
					SpawnParams,
					true,
					ShapedDeckTags);
				if (Platform.Shape == T66TowerMapTerrain::ET66BouncePlatformShape::Square)
				{
					AppendDeckTrims(Center, HalfExtents, Platform.TopZ);
				}
			}
			else
			{
				T66SpawnHiddenCollisionProxyActor(
					World,
					FVector(Center.X, Center.Y, Platform.TopZ - (T66TowerDeckThicknessUU * 0.5f)),
					FRotator::ZeroRotator,
					FVector(HalfExtents.X, HalfExtents.Y, T66TowerDeckThicknessUU * 0.5f),
					SpawnParams,
					PlatformTags,
					true);

				bool bSpawnedVisual = false;
				if (bUseBaffles)
				{
					const int32 TransformCountBefore = BatchedTubeTransforms.Num();
					T66BuildHorizontalBaffleTransforms(Platform.Bounds, Platform.TopZ, Floor.Center.Y, true, BatchedTubeTransforms);
					bSpawnedVisual = BatchedTubeTransforms.Num() > TransformCountBefore;
				}

				if (!bSpawnedVisual && CubeMesh && SurfaceMaterial)
				{
					TArray<FName> DeckTags = PlatformTags;
					DeckTags.AddUnique(T66TowerMapCameraOccludingWallVisualTag);
					DeckTags.AddUnique(T66TowerMapDeckVisualTag);
					T66SpawnEnvironmentRectangle(
						World,
						CubeMesh,
						DeckMaterial ? DeckMaterial : SurfaceMaterial,
						FVector(Center.X, Center.Y, Platform.TopZ - (T66TowerDeckThicknessUU * 0.5f)),
						FVector(HalfExtents.X, HalfExtents.Y, T66TowerDeckThicknessUU * 0.5f),
						SpawnParams,
						DeckTags);
					AppendDeckTrims(Center, HalfExtents, Platform.TopZ);
				}
			}

			if (bRoundShapesAvailable)
			{
				const float PillarRadius = FMath::Clamp(FMath::Min(HalfExtents.X, HalfExtents.Y) * 0.32f, 60.0f, 110.0f);
				const float PillarBottomZ = Floor.SurfaceZ - 2.0f;
				const float PillarTopZ = DeckBottomZ + 2.0f;
				DeckPillarTransforms.Add(T66MakeBasicShapeInstanceTransform(
					RoundMesh,
					FVector(Center.X, Center.Y, (PillarTopZ + PillarBottomZ) * 0.5f),
					FVector(PillarRadius, PillarRadius, (PillarTopZ - PillarBottomZ) * 0.5f)));
			}

			++SpawnedPlatformCount;
		}

		if (SquareStoneTransforms.Num() > 0)
		{
			T66SpawnCollidingInstancedMeshActor(
				World,
				SquareMesh,
				PlatformMaterial ? PlatformMaterial : SurfaceMaterial,
				SquareStoneTransforms,
				SpawnParams,
				PlatformTags,
				TEXT("SquareBounceStone"));
		}
		if (RoundStoneTransforms.Num() > 0)
		{
			T66SpawnCollidingInstancedMeshActor(
				World,
				RoundMesh,
				PlatformMaterial ? PlatformMaterial : SurfaceMaterial,
				RoundStoneTransforms,
				SpawnParams,
				PlatformTags,
				TEXT("RoundBounceStone"));
		}
		if (TrimTransforms.Num() > 0 && TrimMesh && TrimMaterial)
		{
			TArray<UStaticMesh*> TrimMeshes;
			TrimMeshes.Add(TrimMesh);
			TArray<TArray<FTransform>> TrimTransformsByMesh;
			TrimTransformsByMesh.Add(MoveTemp(TrimTransforms));
			AActor* TrimActor = T66SpawnGeneratedDungeonInstancedMeshActor(
				World,
				TrimMeshes,
				TrimTransformsByMesh,
				SpawnParams,
				PlatformTags,
				TEXT("DeckEdgeTrim"),
				true);
			if (TrimActor)
			{
				T66ApplyBaffleMaterial(TrimActor, TrimMaterial);
			}
		}
		if (HexStoneTransforms.Num() > 0)
		{
			T66SpawnCollidingInstancedMeshActor(
				World,
				HexMesh,
				PlatformMaterial ? PlatformMaterial : SurfaceMaterial,
				HexStoneTransforms,
				SpawnParams,
				PlatformTags,
				TEXT("HexBounceStone"));
		}
		if (TriStoneTransforms.Num() > 0)
		{
			T66SpawnCollidingInstancedMeshActor(
				World,
				TriMesh,
				PlatformMaterial ? PlatformMaterial : SurfaceMaterial,
				TriStoneTransforms,
				SpawnParams,
				PlatformTags,
				TEXT("TriBounceStone"));
		}
		if (DeckPillarTransforms.Num() > 0)
		{
			T66SpawnCollidingInstancedMeshActor(
				World,
				RoundMesh,
				PlatformMaterial ? PlatformMaterial : SurfaceMaterial,
				DeckPillarTransforms,
				SpawnParams,
				PlatformTags,
				TEXT("DeckPillar"));
		}

		if (BatchedTubeTransforms.Num() > 0)
		{
			TArray<FName> BaffleTags = PlatformTags;
			BaffleTags.AddUnique(FName(TEXT("T66_Floor_Tower_Baffles")));
			BaffleTags.AddUnique(FName(*FString::Printf(TEXT("T66_Platform_Tower_Baffles_%02d"), Floor.FloorNumber)));
			TArray<UStaticMesh*> Meshes;
			Meshes.Add(TubeMesh);
			TArray<TArray<FTransform>> InstanceTransformsByMesh;
			InstanceTransformsByMesh.Add(MoveTemp(BatchedTubeTransforms));
			AActor* BaffleActor = T66SpawnGeneratedDungeonInstancedMeshActor(
				World,
				Meshes,
				InstanceTransformsByMesh,
				SpawnParams,
				BaffleTags,
				TEXT("PlatformBaffleTube"),
				true);
			if (BaffleActor)
			{
				T66ApplyBaffleMaterial(BaffleActor, SurfaceMaterial);
			}
		}

		int32 SpawnedRampCount = 0;
		for (const T66TowerMapTerrain::FBounceRamp& Ramp : Floor.BounceRamps)
		{
			const FVector2D Center = (Ramp.Bounds.Min + Ramp.Bounds.Max) * 0.5f;
			const FVector2D HalfExtents = (Ramp.Bounds.Max - Ramp.Bounds.Min) * 0.5f;
			const float Rise = Ramp.TopZ - Ramp.BaseZ;
			if (HalfExtents.X <= 10.0f || HalfExtents.Y <= 10.0f || Rise <= 10.0f)
			{
				continue;
			}

			const bool bAlongX = Ramp.AscentSign.X != 0;
			const float RunLength = 2.0f * (bAlongX ? HalfExtents.X : HalfExtents.Y);
			const float AcrossHalf = bAlongX ? HalfExtents.Y : HalfExtents.X;
			const float SlopeRadians = FMath::Atan2(Rise, RunLength);
			const float HypotenuseHalf = 0.5f * FMath::Sqrt((RunLength * RunLength) + (Rise * Rise));
			const float Yaw = bAlongX
				? ((Ramp.AscentSign.X > 0) ? 0.0f : 180.0f)
				: ((Ramp.AscentSign.Y > 0) ? 90.0f : -90.0f);
			// Local +X points along the ascent after yaw; positive pitch raises the high end.
			const FRotator RampRotation(FMath::RadiansToDegrees(SlopeRadians), Yaw, 0.0f);
			const FVector RampCenter(Center.X, Center.Y, Ramp.BaseZ + (Rise * 0.5f));
			const FVector RampHalfExtents(HypotenuseHalf, AcrossHalf, 12.0f);

			T66SpawnHiddenCollisionProxyActor(
				World,
				RampCenter,
				RampRotation,
				RampHalfExtents,
				SpawnParams,
				PlatformTags,
				true);

			if (CubeMesh && SurfaceMaterial)
			{
				UMaterialInterface* RampMaterial = T66GetFallGuysRampMaterial();
				T66SpawnStaticMeshActor(
					World,
					SquareMesh ? SquareMesh : CubeMesh,
					RampMaterial ? RampMaterial : SurfaceMaterial,
					RampCenter,
					RampRotation,
					RampHalfExtents,
					SpawnParams,
					false,
					PlatformTags,
					true);
			}

			++SpawnedRampCount;
		}

		// Reward beacons (analysis P6): a slim white column marks every payoff
		// point so objectives read from across the room — "what do I want, and
		// what's between me and it" answers at a glance.
		int32 BeaconCount = 0;
		UStaticMesh* BeaconMesh = FT66VisualUtil::GetBasicShapeCylinder();
		if (BeaconMesh && TrimMaterial)
		{
			TArray<FTransform> BeaconTransforms;
			for (const T66TowerMapTerrain::FRoom& Room : Floor.Rooms)
			{
				for (const FVector& Slot : Room.RewardSlots)
				{
					BeaconTransforms.Add(T66MakeBasicShapeInstanceTransform(
						BeaconMesh,
						FVector(Slot.X, Slot.Y, Slot.Z + 520.0f),
						FVector(14.0f, 14.0f, 520.0f)));
				}
			}
			BeaconCount = BeaconTransforms.Num();
			if (BeaconCount > 0)
			{
				TArray<UStaticMesh*> BeaconMeshes;
				BeaconMeshes.Add(BeaconMesh);
				TArray<TArray<FTransform>> BeaconTransformsByMesh;
				BeaconTransformsByMesh.Add(MoveTemp(BeaconTransforms));
				AActor* BeaconActor = T66SpawnGeneratedDungeonInstancedMeshActor(
					World,
					BeaconMeshes,
					BeaconTransformsByMesh,
					SpawnParams,
					PlatformTags,
					TEXT("RewardBeacon"),
					true);
				if (BeaconActor)
				{
					T66ApplyBaffleMaterial(BeaconActor, TrimMaterial);
				}
			}
		}

		UE_LOG(
			LogT66TowerMapTerrain,
			Log,
			TEXT("[MAP] Bounce course spawned floor=%d platforms=%d ramps=%d beacons=%d baffles=%d."),
			Floor.FloorNumber,
			SpawnedPlatformCount,
			SpawnedRampCount,
			BeaconCount,
			bUseBaffles ? 1 : 0);
	}

	// Inflatable doorway arch: baffle-tube segments along a half-ellipse spanning the
	// doorway opening, replacing the flat lintel cube. Visual-only, matching the old
	// header's no-collision behavior; jamb-adjacent segments rise from the ground so
	// the arch reads as a bouncy-castle entrance.
	static int32 T66SpawnDoorwayArchTubes(
		UWorld* World,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const float WallHeight,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& DoorwayTags)
	{
		if (!World || !Layout.bDoorwayArches || Floor.DoorwayHeaderBoxes.Num() <= 0)
		{
			return 0;
		}

		UStaticMesh* TubeMesh = T66GetFloorBaffleTubeMesh();
		if (!TubeMesh)
		{
			return 0;
		}

		const float TubeDiameter = Layout.ArchTubeDiameter;
		const float CrossScale = TubeDiameter / T66TowerFloorBaffleTubeNativeDiameter;
		const int32 SegmentCount = FMath::Clamp(Layout.ArchSegments, 4, 24);
		const float ApexHeight = FMath::Min(WallHeight - (TubeDiameter * 0.5f) - 60.0f, 1100.0f);
		if (ApexHeight <= 200.0f)
		{
			return 0;
		}

		TArray<FTransform> TransformsXZ;
		TArray<FTransform> TransformsYZ;
		for (const FBox2D& HeaderBox : Floor.DoorwayHeaderBoxes)
		{
			const FVector2D HeaderCenter = (HeaderBox.Min + HeaderBox.Max) * 0.5f;
			const FVector2D HeaderSize = HeaderBox.Max - HeaderBox.Min;
			const bool bSpanAlongX = HeaderSize.X >= HeaderSize.Y;
			const float Span = bSpanAlongX ? HeaderSize.X : HeaderSize.Y;
			const float HalfSpan = (Span * 0.5f) - 40.0f;
			if (HalfSpan <= 120.0f)
			{
				continue;
			}

			TArray<FTransform>& TargetTransforms = bSpanAlongX ? TransformsXZ : TransformsYZ;
			FVector PreviousPoint = FVector::ZeroVector;
			for (int32 PointIndex = 0; PointIndex <= SegmentCount; ++PointIndex)
			{
				const float Theta = PI * static_cast<float>(PointIndex) / static_cast<float>(SegmentCount);
				const float Along = HalfSpan * FMath::Cos(Theta);
				const float Up = ApexHeight * FMath::Sin(Theta);
				const FVector Point = bSpanAlongX
					? FVector(HeaderCenter.X + Along, HeaderCenter.Y, Floor.SurfaceZ + Up)
					: FVector(HeaderCenter.X, HeaderCenter.Y + Along, Floor.SurfaceZ + Up);
				if (PointIndex > 0)
				{
					const FVector SegmentDelta = Point - PreviousPoint;
					const float SegmentLength = SegmentDelta.Size();
					if (SegmentLength > 5.0f)
					{
						const FVector Midpoint = (Point + PreviousPoint) * 0.5f;
						const FRotator SegmentRotation = FRotationMatrix::MakeFromX(SegmentDelta / SegmentLength).Rotator();
						// 12% length overlap hides the segment seams on the curve.
						const float LengthScale = (SegmentLength * 1.12f) / T66TowerFloorBaffleTubeNativeLength;
						TargetTransforms.Add(FTransform(
							SegmentRotation,
							Midpoint,
							FVector(LengthScale, CrossScale, CrossScale)));
					}
				}

				PreviousPoint = Point;
			}
		}

		int32 SpawnedArchCount = 0;
		auto SpawnArchGroup = [&](TArray<FTransform>& Transforms, UMaterialInterface* Material, const TCHAR* Label)
		{
			if (Transforms.Num() <= 0 || !Material)
			{
				return;
			}

			TArray<FName> ArchTags = DoorwayTags;
			ArchTags.AddUnique(FName(*FString::Printf(TEXT("T66_Doorway_Tower_Arch_%02d"), Floor.FloorNumber)));

			TArray<UStaticMesh*> Meshes;
			Meshes.Add(TubeMesh);
			TArray<TArray<FTransform>> InstanceTransformsByMesh;
			InstanceTransformsByMesh.Add(MoveTemp(Transforms));
			AActor* ArchActor = T66SpawnGeneratedDungeonInstancedMeshActor(
				World,
				Meshes,
				InstanceTransformsByMesh,
				SpawnParams,
				ArchTags,
				Label,
				true);
			if (ArchActor)
			{
				T66ApplyBaffleMaterial(ArchActor, Material);
				++SpawnedArchCount;
			}
		};

		UMaterialInterface* MaterialXZ = Theme.WallXZMaterial ? Theme.WallXZMaterial : Theme.WallMaterial;
		UMaterialInterface* MaterialYZ = Theme.WallYZMaterial ? Theme.WallYZMaterial : Theme.WallMaterial;
		SpawnArchGroup(TransformsXZ, MaterialXZ, TEXT("DoorwayArchTubeXZ"));
		SpawnArchGroup(TransformsYZ, MaterialYZ, TEXT("DoorwayArchTubeYZ"));
		return SpawnedArchCount;
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

		return FMath::Max(1, FMath::CeilToInt(SpanLength / NativeModuleSize));
	}

	static float T66YawForGeneratedWallNormal(const FVector2D& Normal)
	{
		return FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.X));
	}

	static int32 T66ChooseGeneratedDungeonWallMeshIndex(
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const int32 Seed,
		const int32 SideIndex)
	{
		if (Theme.WallMeshes.Num() <= 0)
		{
			return INDEX_NONE;
		}

		return FMath::Abs(Seed + (SideIndex * 977)) % Theme.WallMeshes.Num();
	}

	static UStaticMesh* T66ChooseGeneratedDungeonWallMesh(
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const int32 Seed,
		const int32 SideIndex)
	{
		const int32 MeshIndex = T66ChooseGeneratedDungeonWallMeshIndex(Theme, Seed, SideIndex);
		return Theme.WallMeshes.IsValidIndex(MeshIndex) ? Theme.WallMeshes[MeshIndex] : nullptr;
	}

	static int32 T66ChooseGeneratedDungeonFloorMeshIndex(
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const int32 Seed,
		const int32 TileX,
		const int32 TileY)
	{
		if (Theme.FloorMeshes.Num() <= 0)
		{
			return INDEX_NONE;
		}

		return FMath::Abs(Seed + (TileX * 53) + (TileY * 997)) % Theme.FloorMeshes.Num();
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
		const bool bIgnoreCameraChannel,
		FT66GeneratedDungeonWallBatch* WallBatch = nullptr)
	{
		(void)Normal;
		(void)Seed;
		(void)SideIndex;
		(void)WallBatch;

		if (!World || !T66ShouldUseGeneratedDungeonKit())
		{
			return 0;
		}

		UMaterialInterface* WallMaterial = T66ResolveEnvironmentWallMaterialForBox(Theme, WallBox);
		if (!WallMaterial)
		{
			return 0;
		}

		const FVector2D WallCenter = (WallBox.Min + WallBox.Max) * 0.5f;
		const FVector2D WallSize = WallBox.Max - WallBox.Min;
		if (WallSize.X <= 10.0f || WallSize.Y <= 10.0f || DesiredHeight <= 10.0f)
		{
			return 0;
		}

		TArray<FName> VisualTags = Tags;
		VisualTags.AddUnique(T66TowerMapCameraOccludingWallVisualTag);
		VisualTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit")));
		VisualTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_Wall")));

		bool bSpawnedVisual = false;
		if (T66ShouldUseWallBaffles())
		{
			bSpawnedVisual = T66SpawnWallBaffleTubeVisualsForBox(
				World,
				WallBox,
				BaseZ,
				DesiredHeight,
				WallMaterial,
				SpawnParams,
				VisualTags,
				bIgnoreCameraChannel);
		}

		if (!bSpawnedVisual)
		{
			UStaticMesh* CubeMesh = FT66VisualUtil::GetBasicShapeCube();
			if (!CubeMesh)
			{
				return 0;
			}

			AStaticMeshActor* WallVisual = T66SpawnEnvironmentRectangle(
				World,
				CubeMesh,
				WallMaterial,
				FVector(WallCenter.X, WallCenter.Y, BaseZ + (DesiredHeight * 0.5f)),
				FVector(WallSize.X * 0.5f, WallSize.Y * 0.5f, DesiredHeight * 0.5f),
				SpawnParams,
				VisualTags,
				bIgnoreCameraChannel);

			if (!WallVisual)
			{
				return 0;
			}
		}

		if (bSpawnCollision)
		{
			TArray<FName> CollisionTags = Tags;
			CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit")));
			CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_CollisionProxy")));
			CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_WallCollision")));
			T66SpawnHiddenCollisionProxyActor(
				World,
				FVector(WallCenter.X, WallCenter.Y, BaseZ + (DesiredHeight * 0.5f)),
				FRotator::ZeroRotator,
				FVector(WallSize.X * 0.5f, WallSize.Y * 0.5f, DesiredHeight * 0.5f),
				SpawnParams,
				CollisionTags,
				bIgnoreCameraChannel);
		}

		return 1;
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
		const bool bIgnoreCameraChannel,
		FT66GeneratedDungeonWallBatch* WallBatch = nullptr)
	{
		if (!T66ShouldUseGeneratedDungeonKit() || !World)
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
				bIgnoreCameraChannel,
				WallBatch);
		}

		return SpawnedCount > 0;
	}

	static void T66FlushGeneratedDungeonWallBatch(
		UWorld* World,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const FT66GeneratedDungeonWallBatch& WallBatch,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags,
		const TCHAR* ComponentNamePrefix,
		const bool bIgnoreCameraChannel)
	{
		if (!World || WallBatch.InstanceCount <= 0)
		{
			return;
		}

		T66SpawnGeneratedDungeonInstancedMeshActor(
			World,
			Theme.WallMeshes,
			WallBatch.InstanceTransformsByMesh,
			SpawnParams,
			Tags,
			ComponentNamePrefix,
			bIgnoreCameraChannel);
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
		if (World->LineTraceMultiByChannel(Hits, TraceStart, TraceEnd, ECC_WorldStatic))
		{
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
		}

		const float SurfaceTolerance = FMath::Max(120.0f, (Layout.FloorThickness * 0.5f) + 20.0f);
		for (const T66TowerMapTerrain::FFloor& Floor : Layout.Floors)
		{
			if (FMath::Abs(DesiredLocation.Z - Floor.SurfaceZ) <= SurfaceTolerance)
			{
				OutLocation = FVector(DesiredLocation.X, DesiredLocation.Y, Floor.SurfaceZ);
				return true;
			}
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

	static bool T66IsLocationInsideBounceObstacle(
		const T66TowerMapTerrain::FFloor& Floor,
		const FVector& Location,
		const float Padding)
	{
		const FVector2D Point(Location.X, Location.Y);
		for (const T66TowerMapTerrain::FBouncePlatform& Platform : Floor.BouncePlatforms)
		{
			if (Point.X >= Platform.Bounds.Min.X - Padding && Point.X <= Platform.Bounds.Max.X + Padding
				&& Point.Y >= Platform.Bounds.Min.Y - Padding && Point.Y <= Platform.Bounds.Max.Y + Padding)
			{
				return true;
			}
		}

		for (const T66TowerMapTerrain::FBounceRamp& Ramp : Floor.BounceRamps)
		{
			if (Point.X >= Ramp.Bounds.Min.X - Padding && Point.X <= Ramp.Bounds.Max.X + Padding
				&& Point.Y >= Ramp.Bounds.Min.Y - Padding && Point.Y <= Ramp.Bounds.Max.Y + Padding)
			{
				return true;
			}
		}

		// Tier ramps stay clear of content, and placement avoids the strip around a
		// mesa's cliff edge (content well inside a mesa top is fine — traces snap Z).
		for (const T66TowerMapTerrain::FTierRamp& TierRamp : Floor.TierRamps)
		{
			if (Point.X >= TierRamp.Bounds.Min.X - Padding && Point.X <= TierRamp.Bounds.Max.X + Padding
				&& Point.Y >= TierRamp.Bounds.Min.Y - Padding && Point.Y <= TierRamp.Bounds.Max.Y + Padding)
			{
				return true;
			}
		}

		// Lift travel columns stay clear of content: anything spawned inside one
		// would intersect the moving slab.
		for (const T66TowerMapTerrain::FTierLift& TierLift : Floor.TierLifts)
		{
			if (Point.X >= TierLift.Bounds.Min.X - Padding && Point.X <= TierLift.Bounds.Max.X + Padding
				&& Point.Y >= TierLift.Bounds.Min.Y - Padding && Point.Y <= TierLift.Bounds.Max.Y + Padding)
			{
				return true;
			}
		}

		for (const T66TowerMapTerrain::FTierMesa& Mesa : Floor.TierMesas)
		{
			const bool bInsideExpanded =
				Point.X >= Mesa.Bounds.Min.X - Padding && Point.X <= Mesa.Bounds.Max.X + Padding
				&& Point.Y >= Mesa.Bounds.Min.Y - Padding && Point.Y <= Mesa.Bounds.Max.Y + Padding;
			if (!bInsideExpanded)
			{
				continue;
			}

			const bool bWellInside =
				Point.X >= Mesa.Bounds.Min.X + Padding && Point.X <= Mesa.Bounds.Max.X - Padding
				&& Point.Y >= Mesa.Bounds.Min.Y + Padding && Point.Y <= Mesa.Bounds.Max.Y - Padding;
			if (!bWellInside)
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

		// Bounce platforms/ramps are solid prisms; placement traces over them would land
		// content on the obstacle tops, so the base-floor walkability test excludes them.
		return !T66IsLocationInsideFloorHole(Floor, Location, HolePadding)
			&& !T66IsLocationInsideFloorMazeWalls(Floor, Location, WallPadding)
			&& !T66IsLocationInsideBounceObstacle(Floor, Location, FMath::Max(WallPadding * 0.5f, 60.0f));
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
		if (!Floor.bMobFloor
			|| Floor.FloorNumber == Layout.StartFloorNumber
			|| Floor.FloorNumber == Layout.BossFloorNumber)
		{
			return;
		}

		const float LaneSpacing = Layout.PlacementCellSize * 3.0f;
		const float HalfThickness = FMath::Max(60.0f, Layout.PlacementCellSize * T66GetMazeWallHalfThicknessScale(Layout));
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
		Floor.Rooms.Reset();
		Floor.BouncePlatforms.Reset();
		Floor.BounceRamps.Reset();
		Floor.SafeChainCells.Reset();
		Floor.CellTiers.Reset();
		Floor.TierMesas.Reset();
		Floor.TierRamps.Reset();
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
			const int32 MaxNewCells = Rng.RandRange(1, Layout.GridMaxBranchCells);
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

	static bool T66BoxOverlapsPointRadius2D(const FBox2D& Box, const FVector& Point, const float Radius)
	{
		const float ClosestX = FMath::Clamp(Point.X, Box.Min.X, Box.Max.X);
		const float ClosestY = FMath::Clamp(Point.Y, Box.Min.Y, Box.Max.Y);
		return FVector2D::DistSquared(FVector2D(ClosestX, ClosestY), FVector2D(Point.X, Point.Y)) <= FMath::Square(Radius);
	}

	static int32 T66RemoveWallBoxesNearPoint(TArray<FBox2D>& InOutBoxes, const FVector& Point, const float Radius)
	{
		int32 RemovedCount = 0;
		for (int32 Index = InOutBoxes.Num() - 1; Index >= 0; --Index)
		{
			if (T66BoxOverlapsPointRadius2D(InOutBoxes[Index], Point, Radius))
			{
				InOutBoxes.RemoveAtSwap(Index, 1, EAllowShrinking::No);
				++RemovedCount;
			}
		}
		return RemovedCount;
	}

	static void T66FinalizeFloorMazeMetadata(const T66TowerMapTerrain::FLayout& Layout, T66TowerMapTerrain::FFloor& Floor)
	{
		if (Floor.FloorNumber != Layout.StartFloorNumber && !Floor.ArrivalPoint.IsNearlyZero())
		{
			const float UnderpassClearanceRadius = FMath::Max(Layout.PlacementCellSize * 0.70f, 850.0f);
			const int32 RemovedMazeWalls = T66RemoveWallBoxesNearPoint(Floor.MazeWallBoxes, Floor.ArrivalPoint, UnderpassClearanceRadius);
			const int32 RemovedTrapWalls = T66RemoveWallBoxesNearPoint(Floor.TrapEligibleWallBoxes, Floor.ArrivalPoint, UnderpassClearanceRadius);
			if (RemovedMazeWalls > 0 || RemovedTrapWalls > 0)
			{
				UE_LOG(
					LogT66TowerMapTerrain,
					Display,
					TEXT("[MAP] Tower arrival underpass clearance floor %d removed mazeWallBoxes=%d trapWallBoxes=%d radius=%.1f arrival=%s."),
					Floor.FloorNumber,
					RemovedMazeWalls,
					RemovedTrapWalls,
					UnderpassClearanceRadius,
					*Floor.ArrivalPoint.ToCompactString());
			}
		}

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
			const int32 TargetRoomCount = Layout.DungeonMinRooms + (Rng.RandRange(0, (Layout.DungeonMaxRooms - Layout.DungeonMinRooms) / 2) * 2);
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

		return OutPath.Num() >= Layout.DungeonMinRooms && OutPath.Num() <= Layout.DungeonMaxRooms;
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

	static bool T66DungeonRoomContainsCoord(const FT66DungeonRoom& Room, const FIntPoint& Coord)
	{
		return Coord.X >= Room.MinX
			&& Coord.X < Room.MaxX
			&& Coord.Y >= Room.MinY
			&& Coord.Y < Room.MaxY;
	}

	static void T66AddGridRoomRecord(
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor,
		const FT66DungeonRoom& SourceRoom,
		const int32 RoomId,
		const FName RoomRuleID,
		const FName RoomRoleID)
	{
		const FVector2D GridMin = T66GetGridMinCorner(Layout, Floor);
		const FVector2D BoundsMin(
			GridMin.X + static_cast<float>(SourceRoom.MinX) * Layout.GridCellSize,
			GridMin.Y + static_cast<float>(SourceRoom.MinY) * Layout.GridCellSize);
		const FVector2D BoundsMax(
			GridMin.X + static_cast<float>(SourceRoom.MaxX) * Layout.GridCellSize,
			GridMin.Y + static_cast<float>(SourceRoom.MaxY) * Layout.GridCellSize);

		T66TowerMapTerrain::FRoom& Room = Floor.Rooms.AddDefaulted_GetRef();
		Room.RoomId = RoomId;
		Room.FloorNumber = Floor.FloorNumber;
		Room.RoomRuleID = RoomRuleID;
		Room.RoomRoleID = RoomRoleID;
		Room.MinCell = FIntPoint(SourceRoom.MinX, SourceRoom.MinY);
		Room.MaxCellExclusive = FIntPoint(SourceRoom.MaxX, SourceRoom.MaxY);
		Room.CenterCell = SourceRoom.Center();
		Room.Bounds = FBox2D(BoundsMin, BoundsMax);
		Room.WorldCenter = FVector((BoundsMin.X + BoundsMax.X) * 0.5f, (BoundsMin.Y + BoundsMax.Y) * 0.5f, Floor.SurfaceZ);
		Room.WidthTiles = FMath::Max(0, SourceRoom.MaxX - SourceRoom.MinX);
		Room.HeightTiles = FMath::Max(0, SourceRoom.MaxY - SourceRoom.MinY);
		Room.bContainsArrival = T66DungeonRoomContainsCoord(SourceRoom, Floor.ArrivalCell);
		Room.bContainsExit = T66DungeonRoomContainsCoord(SourceRoom, Floor.ExitCell);
	}

	static void T66AddBoxRoomRecord(
		T66TowerMapTerrain::FFloor& Floor,
		const int32 RoomId,
		const FBox2D& Bounds,
		const FName RoomRuleID,
		const FName RoomRoleID)
	{
		T66TowerMapTerrain::FRoom& Room = Floor.Rooms.AddDefaulted_GetRef();
		Room.RoomId = RoomId;
		Room.FloorNumber = Floor.FloorNumber;
		Room.RoomRuleID = RoomRuleID;
		Room.RoomRoleID = RoomRoleID;
		Room.Bounds = Bounds;
		Room.WorldCenter = FVector((Bounds.Min.X + Bounds.Max.X) * 0.5f, (Bounds.Min.Y + Bounds.Max.Y) * 0.5f, Floor.SurfaceZ);
		Room.WidthTiles = 1;
		Room.HeightTiles = 1;
		Room.bContainsArrival = Bounds.IsInside(FVector2D(Floor.ArrivalPoint.X, Floor.ArrivalPoint.Y));
		Room.bContainsExit = Bounds.IsInside(FVector2D(Floor.ExitPoint.X, Floor.ExitPoint.Y));
	}

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

	/** Edge-to-edge gap (in cells) between the candidate and its nearest existing room.
	 *  GapX + GapY approximates the L-shaped corridor a graph edge would carve. */
	static int32 T66DungeonRoomNearestGapCells(const FT66DungeonRoom& Candidate, const TArray<FT66DungeonRoom>& ExistingRooms)
	{
		int32 BestGap = TNumericLimits<int32>::Max();
		for (const FT66DungeonRoom& ExistingRoom : ExistingRooms)
		{
			const int32 GapX = FMath::Max(0, FMath::Max(Candidate.MinX - ExistingRoom.MaxX, ExistingRoom.MinX - Candidate.MaxX));
			const int32 GapY = FMath::Max(0, FMath::Max(Candidate.MinY - ExistingRoom.MaxY, ExistingRoom.MinY - Candidate.MaxY));
			BestGap = FMath::Min(BestGap, GapX + GapY);
		}
		return BestGap;
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
			const int32 Width = Rng.RandRange(Layout.DungeonMinRoomTiles, Layout.DungeonMaxRoomTiles);
			const int32 Height = Rng.RandRange(Layout.DungeonMinRoomTiles, Layout.DungeonMaxRoomTiles);
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

			// Short-halls rule (constructive, 2026-06-10 user direction): rooms grow
			// as a tight cluster — a candidate beyond RoomMaxGapCells of the nearest
			// existing room is rejected outright, so every corridor is born short
			// instead of being trimmed later. Inside the band, prefer the gap sweet
			// spot (a couple of cells) with jitter for variety. The old behavior
			// scored by distance-to-nearest and picked the MAXIMUM, which actively
			// maximized hall length.
			if (ExistingRooms.Num() > 0)
			{
				const int32 NearestGap = T66DungeonRoomNearestGapCells(Candidate, ExistingRooms);
				if (NearestGap > Layout.RoomMaxGapCells)
				{
					continue;
				}

				const float Score = 100.0f - (FMath::Abs(static_cast<float>(NearestGap) - 3.0f) * 10.0f) + Rng.FRandRange(0.0f, 8.0f);
				if (!bFound || Score > BestScore)
				{
					bFound = true;
					BestScore = Score;
					OutRoom = Candidate;
				}
				continue;
			}

			if (!bFound)
			{
				bFound = true;
				BestScore = 0.0f;
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

		const int32 TargetRoomCount = Rng.RandRange(Layout.DungeonMinRooms, Layout.DungeonMaxRooms);
		const int32 StartRoomWidth = Rng.RandRange(Layout.StartRoomMinTiles, Layout.StartRoomMaxTiles);
		const int32 StartRoomHeight = Rng.RandRange(Layout.StartRoomMinTiles, Layout.StartRoomMaxTiles);
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

		for (int32 Attempt = 0; Attempt < 220 && OutRooms.Num() < Layout.DungeonMinRooms; ++Attempt)
		{
			FT66DungeonRoom Candidate;
			if (T66TryFindScatteredDungeonRoom(Layout, OutRooms, Rng, 0, Candidate))
			{
				OutRooms.Add(Candidate);
			}
		}

		return OutRooms.Num() >= Layout.DungeonMinRooms;
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
			// Area bias (section 1.7): the exit-gate room hosts the descent ceremony
			// (hole, idol altar, guardian) — prefer the BIGGER of the far rooms.
			const int32 Area = (Rooms[RoomIndex].MaxX - Rooms[RoomIndex].MinX) * (Rooms[RoomIndex].MaxY - Rooms[RoomIndex].MinY);
			const int32 Score = (GraphDistance * 100) + SpatialDistance + (Area * 2);
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

		Floor.Rooms.Reset();
		for (int32 RoomIndex = 0; RoomIndex < Rooms.Num(); ++RoomIndex)
		{
			T66AddGridRoomRecord(
				Layout,
				Floor,
				Rooms[RoomIndex],
				RoomIndex,
				Layout.DefaultRoomRuleID,
				TEXT("Combat"));
		}

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

		const float HalfThickness = FMath::Max(Layout.WallThickness * 0.40f, Layout.PlacementCellSize * T66GetMazeWallHalfThicknessScale(Layout));
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
			if (Rng.FRand() > Layout.GridBranchChance)
			{
				continue;
			}

			if (T66TryAddLoopBranch(Layout, Floor, Occupied, StartIndex, Rng, LoopId))
			{
				++LoopId;
			}
		}

		const float HalfThickness = FMath::Max(Layout.WallThickness * 0.40f, Layout.PlacementCellSize * T66GetMazeWallHalfThicknessScale(Layout));
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

	// -----------------------------------------------------------------------
	// Tier terrain (accessibility infrastructure).
	//
	// Constructive-connectivity rules adapted from the MegabonkTerrainGenerator
	// reference and Fall Guys Tail Tag: rooms get a central raised mesa with a
	// guaranteed walkable ground ring and 2+ direction-locked ramps placed at
	// creation time; corridors stay ground; downhill drops are always free.
	// Accessibility is therefore constructed, then additionally verified by a
	// full directed BFS over the (cell, tier) walk graph.
	// -----------------------------------------------------------------------

	static uint8 T66GetCellTier(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const FIntPoint& Coord)
	{
		if (!T66IsValidGridCoord(Layout, Coord))
		{
			return 0;
		}

		const int32 CellIndex = T66GetGridCellIndex(Layout, Coord);
		return Floor.CellTiers.IsValidIndex(CellIndex) ? Floor.CellTiers[CellIndex] : 0;
	}

	static const T66TowerMapTerrain::FTierRamp* T66FindTierRampAt(
		const T66TowerMapTerrain::FFloor& Floor,
		const FIntPoint& Coord)
	{
		for (const T66TowerMapTerrain::FTierRamp& Ramp : Floor.TierRamps)
		{
			if (Ramp.Cell == Coord)
			{
				return &Ramp;
			}
		}
		return nullptr;
	}

	static const T66TowerMapTerrain::FTierLift* T66FindTierLiftAt(
		const T66TowerMapTerrain::FFloor& Floor,
		const FIntPoint& Coord)
	{
		for (const T66TowerMapTerrain::FTierLift& Lift : Floor.TierLifts)
		{
			if (Lift.Cell == Coord)
			{
				return &Lift;
			}
		}
		return nullptr;
	}

	/** True when the cell lies inside any mesa's cell rect (deck cells AND ring-hole cells). */
	static bool T66IsCellInsideAnyMesa(
		const T66TowerMapTerrain::FFloor& Floor,
		const FIntPoint& Coord)
	{
		for (const T66TowerMapTerrain::FTierMesa& Mesa : Floor.TierMesas)
		{
			if (Coord.X >= Mesa.MinCell.X && Coord.X < Mesa.MaxCellExclusive.X
				&& Coord.Y >= Mesa.MinCell.Y && Coord.Y < Mesa.MaxCellExclusive.Y)
			{
				return true;
			}
		}
		return false;
	}

	/** Directed walk edge: same tier, any downhill drop, or uphill through a ramp or lift cell. */
	static bool T66CanWalkDirectedTier(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const FIntPoint& FromCoord,
		const FIntPoint& ToCoord)
	{
		const uint8 FromTier = T66GetCellTier(Layout, Floor, FromCoord);
		const uint8 ToTier = T66GetCellTier(Layout, Floor, ToCoord);
		if (ToTier <= FromTier)
		{
			return true;
		}

		const T66TowerMapTerrain::FTierRamp* Ramp = T66FindTierRampAt(Floor, FromCoord);
		if (Ramp && Ramp->AscentSign == (ToCoord - FromCoord))
		{
			return true;
		}

		// Lift edges count as up-edges: riding the cycling slab reaches the mesa.
		const T66TowerMapTerrain::FTierLift* Lift = T66FindTierLiftAt(Floor, FromCoord);
		return Lift && Lift->AscentSign == (ToCoord - FromCoord);
	}

	static void T66BuildFloorTierTerrain(
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor,
		FRandomStream& Rng)
	{
		Floor.CellTiers.Reset();
		Floor.TierMesas.Reset();
		Floor.TierRamps.Reset();
		Floor.TierLifts.Reset();
		T66MapGeneration::ResetFloorPlacementOutputs(Floor);

		if (!Layout.bTierTerrain || !Floor.bMobFloor || Floor.GridCells.Num() <= 0)
		{
			return;
		}

		Floor.CellTiers.Init(0, Layout.GridColumns * Layout.GridRows);

		// Room composition: assign reusable structures through the composer. The
		// output still feeds existing platform/mesa/lift arrays, but room identity
		// now comes from profiles, structures, and hazards.
		for (T66TowerMapTerrain::FRoom& Room : Floor.Rooms)
		{
			T66MapGeneration::ComposeRoomStructures(Layout, Room, Rng);
		}

		auto IsWalkableGroundCell = [&](const FIntPoint& Coord)
		{
			if (!T66IsValidGridCoord(Layout, Coord))
			{
				return false;
			}
			const int32 CellIndex = T66GetGridCellIndex(Layout, Coord);
			return Floor.GridCells.IsValidIndex(CellIndex)
				&& Floor.GridCells[CellIndex].Semantic != T66TowerMapTerrain::ET66TowerGridCellSemantic::Unused
				&& Floor.CellTiers[CellIndex] == 0;
		};

		static const FIntPoint SideOutward[4] = { FIntPoint(0, -1), FIntPoint(1, 0), FIntPoint(0, 1), FIntPoint(-1, 0) };

		for (T66TowerMapTerrain::FRoom& Room : Floor.Rooms)
		{
			// Arrival and exit rooms stay flat so floor entry and the descent hole
			// keep their established clear space. Mesa structures build here;
			// stepping stones, bridges, and scatter compose in the bounce pass.
			const bool bMesaStructure = T66MapGeneration::IsMesaStructure(Room);
			if (Room.bContainsArrival || Room.bContainsExit || !bMesaStructure)
			{
				continue;
			}

			const int32 AvailX = (Room.MaxCellExclusive.X - Room.MinCell.X) - (Layout.MesaInsetCells * 2);
			const int32 AvailY = (Room.MaxCellExclusive.Y - Room.MinCell.Y) - (Layout.MesaInsetCells * 2);
			if (AvailX < Layout.MesaMinSpanCells || AvailY < Layout.MesaMinSpanCells)
			{
				T66MapGeneration::DowngradeRoomToFlatCombat(Room);
				continue;
			}

			// Cap the mesa so it reads as a central platform, not a second floor.
			const int32 SpanX = FMath::Min(AvailX, FMath::Max(Layout.MesaMinSpanCells, 8));
			const int32 SpanY = FMath::Min(AvailY, FMath::Max(Layout.MesaMinSpanCells, 8));
			const int32 SlackX = AvailX - SpanX;
			const int32 SlackY = AvailY - SpanY;
			const FIntPoint MesaMin(
				Room.MinCell.X + Layout.MesaInsetCells + (SlackX > 0 ? Rng.RandRange(0, SlackX) : 0),
				Room.MinCell.Y + Layout.MesaInsetCells + (SlackY > 0 ? Rng.RandRange(0, SlackY) : 0));
			const FIntPoint MesaMaxExclusive(MesaMin.X + SpanX, MesaMin.Y + SpanY);

			bool bAllWalkable = true;
			for (int32 Y = MesaMin.Y; Y < MesaMaxExclusive.Y && bAllWalkable; ++Y)
			{
				for (int32 X = MesaMin.X; X < MesaMaxExclusive.X && bAllWalkable; ++X)
				{
					bAllWalkable = IsWalkableGroundCell(FIntPoint(X, Y));
				}
			}
			if (!bAllWalkable)
			{
				T66MapGeneration::DowngradeRoomToFlatCombat(Room);
				continue;
			}

			// Constructive ramps BEFORE committing the mesa: 2-4 ramps on distinct
			// sides, never on corners, each occupying a walkable ground-ring cell.
			struct FT66PendingRamp
			{
				FIntPoint Cell;
				FIntPoint AscentSign;
			};
			TArray<FT66PendingRamp> PendingRamps;
			int32 SideOrder[4] = { 0, 1, 2, 3 };
			for (int32 Index = 3; Index > 0; --Index)
			{
				const int32 Swap = Rng.RandRange(0, Index);
				const int32 Temp = SideOrder[Index];
				SideOrder[Index] = SideOrder[Swap];
				SideOrder[Swap] = Temp;
			}
			const int32 WantedRamps = Rng.RandRange(Layout.MesaRampsMin, Layout.MesaRampsMax);
			for (int32 SideSlot = 0; SideSlot < 4 && PendingRamps.Num() < WantedRamps; ++SideSlot)
			{
				const FIntPoint Outward = SideOutward[SideOrder[SideSlot]];
				const bool bHorizontalSide = Outward.Y != 0;
				const int32 EdgeMin = bHorizontalSide ? MesaMin.X : MesaMin.Y;
				const int32 EdgeMaxExclusive = bHorizontalSide ? MesaMaxExclusive.X : MesaMaxExclusive.Y;
				if (EdgeMaxExclusive - EdgeMin < 3)
				{
					continue;
				}

				// avoid mesa corners so the ramp always meets a flat mesa edge
				const int32 Along = Rng.RandRange(EdgeMin + 1, EdgeMaxExclusive - 2);
				const FIntPoint EdgeCell = bHorizontalSide
					? FIntPoint(Along, Outward.Y > 0 ? MesaMaxExclusive.Y - 1 : MesaMin.Y)
					: FIntPoint(Outward.X > 0 ? MesaMaxExclusive.X - 1 : MesaMin.X, Along);
				const FIntPoint RampCell = EdgeCell + Outward;
				const bool bRampCellInsideRoom =
					RampCell.X >= Room.MinCell.X && RampCell.X < Room.MaxCellExclusive.X
					&& RampCell.Y >= Room.MinCell.Y && RampCell.Y < Room.MaxCellExclusive.Y;
				if (!bRampCellInsideRoom || !IsWalkableGroundCell(RampCell)
					|| T66FindTierRampAt(Floor, RampCell) || T66FindTierLiftAt(Floor, RampCell))
				{
					continue;
				}

				bool bAlreadyPending = false;
				for (const FT66PendingRamp& Pending : PendingRamps)
				{
					bAlreadyPending |= Pending.Cell == RampCell;
				}
				if (bAlreadyPending)
				{
					continue;
				}

				PendingRamps.Add({ RampCell, FIntPoint(-Outward.X, -Outward.Y) });
			}

			if (PendingRamps.Num() < FMath::Max(2, Layout.MesaRampsMin))
			{
				// No mesa without guaranteed multi-side access (Tail Tag rule).
				T66MapGeneration::DowngradeRoomToFlatCombat(Room);
				continue;
			}

			// Moving lift alternative: when the mesa gathered a SURPLUS ramp beyond
			// the access minimum, a seeded roll converts the last candidate into a
			// cycling lift platform. Total routes per mesa never drop below the Tail
			// Tag minimum AND at least MesaRampsMin always-walkable ramps remain (a
			// lift has cycle downtime; static ramps stay the any-time guarantee).
			// Ring mesa structures force the lift whenever a surplus exists.
			const bool bRingMesaRoom = T66MapGeneration::RoomHasStructure(Room, T66MapGeneration::StructureRingMesa);
			bool bConvertLastRampToLift = false;
			if (Layout.bTierLifts
				&& PendingRamps.Num() >= FMath::Max(2, Layout.MesaRampsMin) + 1
				&& (bRingMesaRoom || Rng.FRand() < Layout.LiftChance))
			{
				bConvertLastRampToLift = true;
			}

			for (int32 Y = MesaMin.Y; Y < MesaMaxExclusive.Y; ++Y)
			{
				for (int32 X = MesaMin.X; X < MesaMaxExclusive.X; ++X)
				{
					Floor.CellTiers[T66GetGridCellIndex(Layout, FIntPoint(X, Y))] = 1;
				}
			}

			T66TowerMapTerrain::FTierMesa& Mesa = Floor.TierMesas.AddDefaulted_GetRef();
			Mesa.RoomId = Room.RoomId;
			Mesa.MinCell = MesaMin;
			Mesa.MaxCellExclusive = MesaMaxExclusive;
			const FBox2D& MinCellBounds = Floor.GridCells[T66GetGridCellIndex(Layout, MesaMin)].Bounds;
			const FBox2D& MaxCellBounds = Floor.GridCells[T66GetGridCellIndex(Layout, FIntPoint(MesaMaxExclusive.X - 1, MesaMaxExclusive.Y - 1))].Bounds;
			Mesa.Bounds = FBox2D(MinCellBounds.Min, MaxCellBounds.Max);

			// Ring mesa structures always get a center pit; central mesa structures
			// stay solid. The pit remains a legal drop into open under-deck ground,
			// and walking out between pillars is the built-in escape.
			// Hole cells revert to tier 0 — falling through is a legal drop into
			// the open under-deck ground, and walking out between the pillars is
			// the built-in escape (no softlock by construction). The ring stays
			// >= 2 cells wide, so deck circulation survives.
			if (bRingMesaRoom && SpanX >= 5 && SpanY >= 5)
			{
				const int32 HoleSpanX = FMath::Clamp(SpanX - 4, 1, 2);
				const int32 HoleSpanY = FMath::Clamp(SpanY - 4, 1, 2);
				Mesa.HoleMinCell = FIntPoint(
					MesaMin.X + ((SpanX - HoleSpanX) / 2),
					MesaMin.Y + ((SpanY - HoleSpanY) / 2));
				Mesa.HoleMaxCellExclusive = FIntPoint(
					Mesa.HoleMinCell.X + HoleSpanX,
					Mesa.HoleMinCell.Y + HoleSpanY);
				for (int32 Y = Mesa.HoleMinCell.Y; Y < Mesa.HoleMaxCellExclusive.Y; ++Y)
				{
					for (int32 X = Mesa.HoleMinCell.X; X < Mesa.HoleMaxCellExclusive.X; ++X)
					{
						Floor.CellTiers[T66GetGridCellIndex(Layout, FIntPoint(X, Y))] = 0;
					}
				}
				const FBox2D& HoleMinCellBounds = Floor.GridCells[T66GetGridCellIndex(Layout, Mesa.HoleMinCell)].Bounds;
				const FBox2D& HoleMaxCellBounds = Floor.GridCells[T66GetGridCellIndex(Layout, FIntPoint(Mesa.HoleMaxCellExclusive.X - 1, Mesa.HoleMaxCellExclusive.Y - 1))].Bounds;
				Mesa.HoleBounds = FBox2D(HoleMinCellBounds.Min, HoleMaxCellBounds.Max);
			}

			for (int32 PendingIndex = 0; PendingIndex < PendingRamps.Num(); ++PendingIndex)
			{
				const FT66PendingRamp& Pending = PendingRamps[PendingIndex];
				const FBox2D& CellBounds = Floor.GridCells[T66GetGridCellIndex(Layout, Pending.Cell)].Bounds;
				if (bConvertLastRampToLift && PendingIndex == PendingRamps.Num() - 1)
				{
					T66TowerMapTerrain::FTierLift& Lift = Floor.TierLifts.AddDefaulted_GetRef();
					Lift.Cell = Pending.Cell;
					Lift.AscentSign = Pending.AscentSign;
					// Slab footprint pushed toward the mesa face, 20uu clearance so the
					// moving slab never scrapes the mesa collision proxy.
					const FVector2D CellCenter = (CellBounds.Min + CellBounds.Max) * 0.5f;
					const FVector2D CellHalf = (CellBounds.Max - CellBounds.Min) * 0.5f;
					const float HalfFootprint = Layout.LiftFootprint * 0.5f;
					const FVector2D AscentDir(
						static_cast<float>(Pending.AscentSign.X),
						static_cast<float>(Pending.AscentSign.Y));
					const float AlongHalf = Pending.AscentSign.X != 0 ? CellHalf.X : CellHalf.Y;
					const float TowardMesa = FMath::Max(AlongHalf - HalfFootprint - 20.0f, 0.0f);
					const FVector2D LiftCenter = CellCenter + (AscentDir * TowardMesa);
					Lift.Bounds = FBox2D(
						LiftCenter - FVector2D(HalfFootprint, HalfFootprint),
						LiftCenter + FVector2D(HalfFootprint, HalfFootprint));
					Lift.BaseZ = Floor.SurfaceZ;
					Lift.TopZ = Floor.SurfaceZ + Layout.TierHeight;
					Lift.PhaseFraction = Rng.FRand();
					continue;
				}

				T66TowerMapTerrain::FTierRamp& Ramp = Floor.TierRamps.AddDefaulted_GetRef();
				Ramp.Cell = Pending.Cell;
				Ramp.AscentSign = Pending.AscentSign;
				Ramp.Bounds = FBox2D(CellBounds.Min + FVector2D(100.0f, 100.0f), CellBounds.Max - FVector2D(100.0f, 100.0f));
			}

			// Structure payoff + signature hazard placement: rewards sit at the
			// structure's designed payoff point, and one hazard may guard it.
			const FVector2D MesaCenter2D = (Mesa.Bounds.Min + Mesa.Bounds.Max) * 0.5f;
			const float MesaDeckTopZ = Floor.SurfaceZ + Layout.TierHeight;
			if (bRingMesaRoom && Mesa.HasHole())
			{
				// Ring mesa: prize down in the pit + one on the rim; a bounce pad
				// in the pit is the slapstick route back up to the rim.
				const FVector2D HoleCenter2D = (Mesa.HoleBounds.Min + Mesa.HoleBounds.Max) * 0.5f;
				T66MapGeneration::AddRewardSlot(Room, FVector(HoleCenter2D.X, HoleCenter2D.Y, Floor.SurfaceZ));
				T66MapGeneration::AddRewardSlot(Room, FVector(Mesa.Bounds.Min.X + 320.0f, Mesa.Bounds.Min.Y + 320.0f, MesaDeckTopZ));
				Floor.BouncePadSpots.Add(FVector(HoleCenter2D.X + 300.0f, HoleCenter2D.Y, Floor.SurfaceZ));
			}
			else
			{
				// Central mesa: prize on the deck corner, the rotating sweeper arm
				// guards the deck center.
				T66MapGeneration::AddRewardSlot(Room, FVector(Mesa.Bounds.Max.X - 360.0f, Mesa.Bounds.Max.Y - 360.0f, MesaDeckTopZ));
				T66MapGeneration::AddHazardAnchor(Floor, FVector(MesaCenter2D.X, MesaCenter2D.Y, MesaDeckTopZ), T66HazardTypeSweeperArm);
			}
		}
	}

	/**
	 * No-softlock proof: directed BFS over the (cell, tier) walk graph from arrival.
	 * Returns reached/total walkable cells and whether the exit is reachable. The
	 * return path is guaranteed by construction (drops always reach the connected
	 * tier-0 network), so full forward coverage proves free movement everywhere.
	 */
	static void T66ValidateTierAccess(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		int32& OutReached,
		int32& OutTotalWalkable,
		bool& bOutExitReached)
	{
		OutReached = 0;
		OutTotalWalkable = 0;
		bOutExitReached = false;

		const int32 CellCount = Layout.GridColumns * Layout.GridRows;
		for (int32 CellIndex = 0; CellIndex < CellCount; ++CellIndex)
		{
			if (Floor.GridCells.IsValidIndex(CellIndex)
				&& Floor.GridCells[CellIndex].Semantic != T66TowerMapTerrain::ET66TowerGridCellSemantic::Unused)
			{
				++OutTotalWalkable;
			}
		}

		if (!T66IsValidGridCoord(Layout, Floor.ArrivalCell))
		{
			return;
		}

		TArray<bool> Visited;
		Visited.Init(false, CellCount);
		TArray<FIntPoint> Queue;
		Queue.Add(Floor.ArrivalCell);
		Visited[T66GetGridCellIndex(Layout, Floor.ArrivalCell)] = true;

		static const FIntPoint Deltas[] = { FIntPoint(0, -1), FIntPoint(1, 0), FIntPoint(0, 1), FIntPoint(-1, 0) };
		for (int32 QueueIndex = 0; QueueIndex < Queue.Num(); ++QueueIndex)
		{
			const FIntPoint Coord = Queue[QueueIndex];
			++OutReached;
			if (Coord == Floor.ExitCell)
			{
				bOutExitReached = true;
			}

			for (const FIntPoint& Delta : Deltas)
			{
				const FIntPoint Next = Coord + Delta;
				if (!T66IsValidGridCoord(Layout, Next))
				{
					continue;
				}
				const int32 NextIndex = T66GetGridCellIndex(Layout, Next);
				if (Visited[NextIndex]
					|| !Floor.GridCells.IsValidIndex(NextIndex)
					|| Floor.GridCells[NextIndex].Semantic == T66TowerMapTerrain::ET66TowerGridCellSemantic::Unused)
				{
					continue;
				}
				if (!T66CanWalkDirectedTier(Layout, Floor, Coord, Next))
				{
					continue;
				}
				Visited[NextIndex] = true;
				Queue.Add(Next);
			}
		}
	}

	// -----------------------------------------------------------------------
	// Bouncy obstacle-course platforms.
	//
	// Tier 1 tops sit one jump above the base floor; Tier 2 tops sit one jump
	// above Tier 1 and stay above the lava-rise cap. Every cell of a BFS
	// arrival->exit path carries a Tier 2 "safe chain" platform, so even at
	// full lava flood the descent hole stays reachable by platform hopping
	// without taking damage. Room interiors get scattered Tier 1/2 platforms
	// plus ramps for the Fall Guys obstacle-course read; collision stays on
	// hidden box proxies per the world collision contract.
	// -----------------------------------------------------------------------

	static bool T66IsWalkableBounceCell(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const FIntPoint& Coord)
	{
		if (!T66IsValidGridCoord(Layout, Coord))
		{
			return false;
		}

		const int32 CellIndex = T66GetGridCellIndex(Layout, Coord);
		return Floor.GridCells.IsValidIndex(CellIndex)
			&& Floor.GridCells[CellIndex].Semantic != T66TowerMapTerrain::ET66TowerGridCellSemantic::Unused;
	}

	static bool T66BuildBounceSafeChainPath(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		TArray<FIntPoint>& OutPathCells)
	{
		OutPathCells.Reset();
		if (!T66IsWalkableBounceCell(Layout, Floor, Floor.ArrivalCell)
			|| !T66IsWalkableBounceCell(Layout, Floor, Floor.ExitCell))
		{
			return false;
		}

		const int32 CellCount = Layout.GridColumns * Layout.GridRows;
		const int32 StartIndex = T66GetGridCellIndex(Layout, Floor.ArrivalCell);
		const int32 GoalIndex = T66GetGridCellIndex(Layout, Floor.ExitCell);
		if (StartIndex == GoalIndex)
		{
			OutPathCells.Add(Floor.ArrivalCell);
			return true;
		}

		TArray<int32> Parent;
		Parent.Init(INDEX_NONE, CellCount);
		TArray<bool> Visited;
		Visited.Init(false, CellCount);
		TArray<int32> Queue;
		Queue.Reserve(CellCount / 4);
		Queue.Add(StartIndex);
		Visited[StartIndex] = true;

		static const FIntPoint Deltas[] = { FIntPoint(0, -1), FIntPoint(1, 0), FIntPoint(0, 1), FIntPoint(-1, 0) };
		bool bReachedGoal = false;
		for (int32 QueueIndex = 0; QueueIndex < Queue.Num() && !bReachedGoal; ++QueueIndex)
		{
			const int32 CurrentIndex = Queue[QueueIndex];
			const FIntPoint CurrentCoord = T66GetGridCoordFromIndex(Layout, CurrentIndex);

			// The dry chain stays on the ground tier: the tier-0 network is connected
			// by construction (rooms always keep a ground ring around mesas), and
			// ramp cells are deprioritized so the chain detours around their wedges
			// whenever the ring allows it.
			for (int32 Pass = 0; Pass < 2 && !bReachedGoal; ++Pass)
			{
				for (const FIntPoint& Delta : Deltas)
				{
					const FIntPoint NeighborCoord = CurrentCoord + Delta;
					if (!T66IsWalkableBounceCell(Layout, Floor, NeighborCoord))
					{
						continue;
					}

					if (T66GetCellTier(Layout, Floor, NeighborCoord) != 0)
					{
						continue;
					}

					// Ring-mesa hole cells are tier 0 but sit UNDER the deck slab —
					// a chain platform there would be unusable. Mesa-rect tier-0
					// cells are exactly the hole cells; route around the mesa.
					if (T66IsCellInsideAnyMesa(Floor, NeighborCoord))
					{
						continue;
					}

					// Lift cells never join the dry chain: the slab parks submerged
					// at the bottom of its cycle, so it cannot be a dry anchor. Room
					// ground rings are >= 2 cells wide, so one excluded cell cannot
					// disconnect the chain.
					if (T66FindTierLiftAt(Floor, NeighborCoord))
					{
						continue;
					}

					const bool bNeighborIsRamp = T66FindTierRampAt(Floor, NeighborCoord) != nullptr;
					if ((Pass == 0) == bNeighborIsRamp)
					{
						continue;
					}

					const int32 NeighborIndex = T66GetGridCellIndex(Layout, NeighborCoord);
					if (Visited[NeighborIndex])
					{
						continue;
					}

					Visited[NeighborIndex] = true;
					Parent[NeighborIndex] = CurrentIndex;
					if (NeighborIndex == GoalIndex)
					{
						bReachedGoal = true;
						break;
					}

					Queue.Add(NeighborIndex);
				}
			}
		}

		if (!bReachedGoal)
		{
			return false;
		}

		for (int32 WalkIndex = GoalIndex; WalkIndex != INDEX_NONE; WalkIndex = Parent[WalkIndex])
		{
			OutPathCells.Add(T66GetGridCoordFromIndex(Layout, WalkIndex));
		}

		for (int32 SwapIndex = 0; SwapIndex < OutPathCells.Num() / 2; ++SwapIndex)
		{
			OutPathCells.Swap(SwapIndex, OutPathCells.Num() - 1 - SwapIndex);
		}

		return OutPathCells.Num() > 0;
	}

	/**
	 * Per-room shape theme (design ref section 1.6): each room commits to a shape
	 * signature so its course reads designed, not sprinkled. The signature shape
	 * replaces a square at RoundPlatformChance; chains use Round/Hex only (axis
	 * hops contact the AABB faces exactly), triangles are scatter-only.
	 */
	enum class ET66RoomShapeTheme : uint8
	{
		Mixed,
		Rounds,
		HexField,
		SquareTri,
	};

	static T66TowerMapTerrain::ET66BouncePlatformShape T66PickThemeShape(
		const T66TowerMapTerrain::FLayout& Layout,
		const ET66RoomShapeTheme Theme,
		FRandomStream& Rng,
		const bool bChainSafeOnly)
	{
		using ET66Shape = T66TowerMapTerrain::ET66BouncePlatformShape;
		if (Rng.FRand() >= Layout.RoundPlatformChance)
		{
			return ET66Shape::Square;
		}

		switch (Theme)
		{
		case ET66RoomShapeTheme::Rounds:
			return ET66Shape::Round;
		case ET66RoomShapeTheme::HexField:
			return ET66Shape::Hex;
		case ET66RoomShapeTheme::SquareTri:
			return bChainSafeOnly ? ET66Shape::Hex : ET66Shape::Triangle;
		case ET66RoomShapeTheme::Mixed:
		default:
		{
			const int32 Pick = Rng.RandRange(0, bChainSafeOnly ? 1 : 2);
			return Pick == 0 ? ET66Shape::Round : (Pick == 1 ? ET66Shape::Hex : ET66Shape::Triangle);
		}
		}
	}

	static void T66AddBouncePlatform(
		T66TowerMapTerrain::FFloor& Floor,
		const FIntPoint& Cell,
		const FVector2D& Center,
		const float Footprint,
		const int32 Tier,
		const float TopZ,
		const bool bSafeChain,
		const T66TowerMapTerrain::ET66BouncePlatformShape Shape = T66TowerMapTerrain::ET66BouncePlatformShape::Square,
		const uint8 YawSteps = 0)
	{
		T66TowerMapTerrain::FBouncePlatform& Platform = Floor.BouncePlatforms.AddDefaulted_GetRef();
		const FVector2D HalfFootprint(Footprint * 0.5f, Footprint * 0.5f);
		Platform.Bounds = FBox2D(Center - HalfFootprint, Center + HalfFootprint);
		Platform.TopZ = TopZ;
		Platform.Tier = Tier;
		Platform.Cell = Cell;
		Platform.bSafeChain = bSafeChain;
		// Triangles never carry the chain (pointy sides); every other kit shape's
		// centerline contact equals its AABB face, so the box-gap proof stays true.
		Platform.Shape = (bSafeChain && Shape == T66TowerMapTerrain::ET66BouncePlatformShape::Triangle)
			? T66TowerMapTerrain::ET66BouncePlatformShape::Square
			: Shape;
		Platform.YawSteps = YawSteps % 4;
	}

	/** Edge-to-edge gap between two axis-aligned 2D boxes (0 when touching or overlapping). */
	static float T66BoxEdgeGap2D(const FBox2D& A, const FBox2D& B)
	{
		const float GapX = FMath::Max(FMath::Max(A.Min.X - B.Max.X, B.Min.X - A.Max.X), 0.0f);
		const float GapY = FMath::Max(FMath::Max(A.Min.Y - B.Max.Y, B.Min.Y - A.Max.Y), 0.0f);
		return FMath::Max(GapX, GapY);
	}

	static void T66BuildFloorBouncePlatforms(
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor,
		FRandomStream& Rng)
	{
		Floor.BouncePlatforms.Reset();
		Floor.BounceRamps.Reset();
		Floor.SafeChainCells.Reset();

		if (!Layout.bBounceCoursePlatforms || !Floor.bMobFloor || Floor.GridCells.Num() <= 0)
		{
			return;
		}

		const float Tier1TopZ = Floor.SurfaceZ + Layout.PlatformTier1Height;
		const float Tier2TopZ = Floor.SurfaceZ + Layout.PlatformTier2Height;
		const float CellSize = Layout.GridCellSize;
		const int32 ArrivalIndex = T66GetGridCellIndex(Layout, Floor.ArrivalCell);
		const int32 ExitIndex = T66GetGridCellIndex(Layout, Floor.ExitCell);

		TArray<FIntPoint> PathCells;
		const bool bHasChainPath = T66BuildBounceSafeChainPath(Layout, Floor, PathCells);

		TSet<int32> OccupiedCells;
		OccupiedCells.Add(ArrivalIndex);
		OccupiedCells.Add(ExitIndex);

		// Tier-terrain integration: ramp and lift cells never host platforms or
		// stones, and rooms that received a mesa keep their ground ring clear of
		// scatter (the mesa is that room's obstacle feature). A static platform in
		// a lift cell would sit inside the moving slab's travel column.
		for (const T66TowerMapTerrain::FTierRamp& TierRamp : Floor.TierRamps)
		{
			OccupiedCells.Add(T66GetGridCellIndex(Layout, TierRamp.Cell));
		}
		for (const T66TowerMapTerrain::FTierLift& TierLift : Floor.TierLifts)
		{
			OccupiedCells.Add(T66GetGridCellIndex(Layout, TierLift.Cell));
		}
		TSet<int32> MesaRoomIds;
		for (const T66TowerMapTerrain::FTierMesa& Mesa : Floor.TierMesas)
		{
			MesaRoomIds.Add(Mesa.RoomId);
		}

		auto GetCellCenter = [&](const FIntPoint& Coord) -> FVector2D
		{
			const T66TowerMapTerrain::FGridCell& Cell = Floor.GridCells[T66GetGridCellIndex(Layout, Coord)];
			return FVector2D(Cell.WorldCenter.X, Cell.WorldCenter.Y);
		};

		// Dry-anchor map for the full-flood validation walk: chain platforms and
		// ramp wedges both count as dry surfaces along the path.
		TMap<int32, FBox2D> DryAnchorByCell;
		for (const T66TowerMapTerrain::FTierRamp& TierRamp : Floor.TierRamps)
		{
			DryAnchorByCell.Add(T66GetGridCellIndex(Layout, TierRamp.Cell), TierRamp.Bounds);
		}

		static const FIntPoint NeighborDeltas[] = { FIntPoint(0, -1), FIntPoint(1, 0), FIntPoint(0, 1), FIntPoint(-1, 0) };

		// Tier 2 safe chain along the arrival->exit path. Chain rhythm + shape mix
		// (design ref section 1.6): a seeded big/small footprint alternation and a
		// cross-axis meander kill the conveyor read; squares, rounds, and hexes mix
		// freely because chain hops are axis-aligned and every kit shape's
		// centerline contact equals its AABB face. Pair-sum invariant: every two
		// adjacent footprints sum to >= 2*(pitch - PlatformChainMaxGap), so no hop
		// exceeds the validated cap (anchors and their neighbors are pinned big).
		const float ChainPitch = Layout.GridCellSize;
		const float ChainBigMax = ChainPitch - 140.0f;
		const float ChainBigMin = FMath::Clamp(ChainPitch - Layout.PlatformChainMaxGap + 80.0f, Layout.ChainPlatformFootprint, ChainBigMax);
		const float ChainSmallMin = FMath::Clamp(2.0f * (ChainPitch - Layout.PlatformChainMaxGap) - ChainBigMin + 20.0f, 400.0f, ChainBigMax);
		const float ChainSmallMax = FMath::Clamp(ChainSmallMin + 100.0f, ChainSmallMin, ChainBigMax);
		const ET66RoomShapeTheme ChainTheme = static_cast<ET66RoomShapeTheme>(Rng.RandRange(0, 2));
		int32 ChainPlatformCount = 0;
		for (int32 PathIndex = 0; PathIndex < PathCells.Num(); ++PathIndex)
		{
			const FIntPoint& PathCell = PathCells[PathIndex];
			const int32 PathCellIndex = T66GetGridCellIndex(Layout, PathCell);
			Floor.SafeChainCells.Add(PathCell);
			if (PathCellIndex == ArrivalIndex || PathCellIndex == ExitIndex)
			{
				continue;
			}

			// Ramp wedges are already dry anchors; never stack a platform on one.
			if (T66FindTierRampAt(Floor, PathCell))
			{
				continue;
			}

			// First/last two platforms anchor the ramp wedges and the descent-hole
			// reach measurement: square, full-size, centered. Platforms adjacent to
			// an anchor pin to the big range so the pair-sum invariant holds.
			const bool bAnchorPlatform = PathIndex <= 1 || PathIndex >= PathCells.Num() - 2;
			const bool bNextToAnchor = PathIndex == 2 || PathIndex == PathCells.Num() - 3;
			float Footprint = Layout.ChainPlatformFootprint + ((PathIndex % 2 == 0) ? 0.0f : 40.0f);
			FVector2D Center = GetCellCenter(PathCell);
			T66TowerMapTerrain::ET66BouncePlatformShape Shape = T66TowerMapTerrain::ET66BouncePlatformShape::Square;
			uint8 YawSteps = 0;
			if (!bAnchorPlatform)
			{
				Footprint = (bNextToAnchor || PathIndex % 2 == 0)
					? Rng.FRandRange(ChainBigMin, ChainBigMax)
					: Rng.FRandRange(ChainSmallMin, ChainSmallMax);
				Shape = T66PickThemeShape(Layout, ChainTheme, Rng, true);
				YawSteps = static_cast<uint8>(Rng.RandRange(0, 3));

				// Cross-axis meander, STRAIGHT segments only: perpendicular offsets
				// never widen the hop gap (boxes keep overlapping in the cross
				// axis), but at corners any offset bleeds into a hop axis — skip.
				const FIntPoint Dir = PathCells[PathIndex + 1] - PathCells[PathIndex - 1];
				if (Dir.X == 0 || Dir.Y == 0)
				{
					FVector2D Perp(static_cast<float>(-Dir.Y), static_cast<float>(Dir.X));
					if (!Perp.IsNearlyZero())
					{
						Perp.Normalize();
						Center += Perp * Rng.FRandRange(-170.0f, 170.0f);
					}
				}
			}

			T66AddBouncePlatform(Floor, PathCell, Center, Footprint, 2, Tier2TopZ, true, Shape, YawSteps);
			DryAnchorByCell.Add(PathCellIndex, Floor.BouncePlatforms.Last().Bounds);
			OccupiedCells.Add(PathCellIndex);
			++ChainPlatformCount;

			// Tier 1 hop-on stones beside every third chain platform keep the chain
			// mountable from the base floor mid-route (ground -> T1 -> T2).
			if ((ChainPlatformCount % 3) == 1)
			{
				for (const FIntPoint& Delta : NeighborDeltas)
				{
					const FIntPoint StoneCell = PathCell + Delta;
					if (!T66IsWalkableBounceCell(Layout, Floor, StoneCell))
					{
						continue;
					}

					if (T66GetCellTier(Layout, Floor, StoneCell) != 0)
					{
						continue;
					}

					const int32 StoneCellIndex = T66GetGridCellIndex(Layout, StoneCell);
					if (OccupiedCells.Contains(StoneCellIndex))
					{
						continue;
					}

					// Gate clearance: keep the drop landing zone open (arrival +-1).
					if (FMath::Abs(StoneCell.X - Floor.ArrivalCell.X) <= 1
						&& FMath::Abs(StoneCell.Y - Floor.ArrivalCell.Y) <= 1)
					{
						continue;
					}

					const FVector2D TowardChain(static_cast<float>(-Delta.X), static_cast<float>(-Delta.Y));
					const FVector2D StoneCenter = GetCellCenter(StoneCell) + (TowardChain * 150.0f);
					T66AddBouncePlatform(
						Floor, StoneCell, StoneCenter, 600.0f, 1, Tier1TopZ, false,
						T66PickThemeShape(Layout, ChainTheme, Rng, true),
						static_cast<uint8>(Rng.RandRange(0, 3)));
					OccupiedCells.Add(StoneCellIndex);
					break;
				}
			}
		}

		// Route choice (analysis D1): where the dry chain crosses a room, a FAST
		// risky lane of small Tier 1 stones cuts straight across the corner the
		// safe Tier 2 chain walks around — the triangular choice, visible at a
		// glance. The risk is built in: at full lava flood the low fork drowns
		// while the chain stays dry.
		if (PathCells.Num() > 0)
		{
			auto IsForkCellFree = [&](const FIntPoint& Cell) -> bool
			{
				if (!T66IsWalkableBounceCell(Layout, Floor, Cell)
					|| T66GetCellTier(Layout, Floor, Cell) != 0
					|| T66IsCellInsideAnyMesa(Floor, Cell)
					|| T66FindTierRampAt(Floor, Cell)
					|| T66FindTierLiftAt(Floor, Cell)
					|| OccupiedCells.Contains(T66GetGridCellIndex(Layout, Cell)))
				{
					return false;
				}
				const bool bNearHole = FMath::Abs(Cell.X - Floor.ExitCell.X) <= 1
					&& FMath::Abs(Cell.Y - Floor.ExitCell.Y) <= 1;
				const bool bNearArrival = FMath::Abs(Cell.X - Floor.ArrivalCell.X) <= 1
					&& FMath::Abs(Cell.Y - Floor.ArrivalCell.Y) <= 1;
				return !bNearHole && !bNearArrival;
			};

			for (const T66TowerMapTerrain::FRoom& Room : Floor.Rooms)
			{
				int32 FirstInRoom = INDEX_NONE;
				int32 LastInRoom = INDEX_NONE;
				for (int32 PathIndex = 0; PathIndex < PathCells.Num(); ++PathIndex)
				{
					const FIntPoint& PathCell = PathCells[PathIndex];
					const bool bInside = PathCell.X >= Room.MinCell.X && PathCell.X < Room.MaxCellExclusive.X
						&& PathCell.Y >= Room.MinCell.Y && PathCell.Y < Room.MaxCellExclusive.Y;
					if (!bInside)
					{
						continue;
					}
					if (FirstInRoom == INDEX_NONE)
					{
						FirstInRoom = PathIndex;
					}
					LastInRoom = PathIndex;
				}

				// Only fork when the chain takes a real detour through the room.
				if (FirstInRoom == INDEX_NONE || LastInRoom - FirstInRoom < 5)
				{
					continue;
				}

				FIntPoint Cursor = PathCells[FirstInRoom];
				const FIntPoint ForkTarget = PathCells[LastInRoom];
				bool bStepX = true;
				int32 Guard = 48;
				int32 ForkPlaced = 0;
				while (Cursor != ForkTarget && Guard-- > 0 && ForkPlaced < 6)
				{
					if (bStepX && Cursor.X != ForkTarget.X)
					{
						Cursor.X += (ForkTarget.X > Cursor.X) ? 1 : -1;
					}
					else if (Cursor.Y != ForkTarget.Y)
					{
						Cursor.Y += (ForkTarget.Y > Cursor.Y) ? 1 : -1;
					}
					else if (Cursor.X != ForkTarget.X)
					{
						Cursor.X += (ForkTarget.X > Cursor.X) ? 1 : -1;
					}
					bStepX = !bStepX;
					if (Cursor == ForkTarget || !IsForkCellFree(Cursor))
					{
						continue;
					}

					T66AddBouncePlatform(
						Floor, Cursor, GetCellCenter(Cursor), 620.0f, 1, Tier1TopZ, false,
						T66TowerMapTerrain::ET66BouncePlatformShape::Round, 0);
					OccupiedCells.Add(T66GetGridCellIndex(Layout, Cursor));
					++ForkPlaced;
				}
			}
		}

		// Room structures: the reusable structure placer owns room-level bounce
		// features while this pass still supplies terrain-local grid callbacks.
		TArray<T66MapGeneration::FScatterRampCandidate> RampCandidates;
		for (T66TowerMapTerrain::FRoom& Room : Floor.Rooms)
		{
			// Mesa rooms keep their ground ring open: the mesa + its ramps are the
			// room's primary structure, scatter stays out, and the mesa pass already
			// registered reward slots.
			if (Room.bContainsArrival || Room.bContainsExit || MesaRoomIds.Contains(Room.RoomId))
			{
				continue;
			}

			if (Room.WidthTiles < 3 || Room.HeightTiles < 3)
			{
				if (Room.StructureIDs.Num() > 0 && Room.RewardSlots.Num() <= 0)
				{
					T66MapGeneration::AddRewardSlot(Room, FVector(Room.WorldCenter.X, Room.WorldCenter.Y, Floor.SurfaceZ));
				}
				continue;
			}

			const int32 RoomArea = Room.WidthTiles * Room.HeightTiles;
			const int32 TargetCount = FMath::Clamp(RoomArea / FMath::Max(2, Layout.RoomPlatformDensityTiles), 1, 4);
			// Per-room shape theme: each room commits to one shape signature so its
			// course reads designed, not sprinkled (design ref section 1.6).
			const ET66RoomShapeTheme RoomTheme = static_cast<ET66RoomShapeTheme>(Rng.RandRange(0, 3));

			T66MapGeneration::FRoomPlacementContext PlacementContext;
			PlacementContext.Layout = &Layout;
			PlacementContext.Floor = &Floor;
			PlacementContext.Rng = &Rng;
			PlacementContext.OccupiedCells = &OccupiedCells;
			PlacementContext.RampCandidates = &RampCandidates;
			PlacementContext.NeighborDeltas.Append(NeighborDeltas, UE_ARRAY_COUNT(NeighborDeltas));
			PlacementContext.CellSize = CellSize;
			PlacementContext.Tier1TopZ = Tier1TopZ;
			PlacementContext.Tier2TopZ = Tier2TopZ;
			PlacementContext.SurfaceZ = Floor.SurfaceZ;
			PlacementContext.IsWalkableBounceCell = [&](const FIntPoint& Cell)
			{
				return T66IsWalkableBounceCell(Layout, Floor, Cell);
			};
			PlacementContext.GetCellTier = [&](const FIntPoint& Cell)
			{
				return T66GetCellTier(Layout, Floor, Cell);
			};
			PlacementContext.IsCellInsideAnyMesa = [&](const FIntPoint& Cell)
			{
				return T66IsCellInsideAnyMesa(Floor, Cell);
			};
			PlacementContext.GetGridCellIndex = [&](const FIntPoint& Cell)
			{
				return T66GetGridCellIndex(Layout, Cell);
			};
			PlacementContext.GetCellCenter = [&](const FIntPoint& Cell)
			{
				return GetCellCenter(Cell);
			};
			PlacementContext.AddBouncePlatform = [&](
				const FIntPoint& Cell,
				const FVector2D& Center,
				const float Footprint,
				const int32 Tier,
				const float TopZ,
				const bool bSafeChain,
				const T66TowerMapTerrain::ET66BouncePlatformShape Shape,
				const uint8 YawSteps)
			{
				T66AddBouncePlatform(Floor, Cell, Center, Footprint, Tier, TopZ, bSafeChain, Shape, YawSteps);
			};
			PlacementContext.PickShape = [&](const bool bForceSquareCompatible)
			{
				return T66PickThemeShape(Layout, RoomTheme, Rng, bForceSquareCompatible);
			};

			bool bComposed = T66MapGeneration::PlaceSteppingStoneStructure(PlacementContext, Room);
			if (!bComposed)
			{
				bComposed = T66MapGeneration::PlaceBridgeDeckStructure(PlacementContext, Room, T66HazardTypeCeilingHammer);
			}

			if (bComposed)
			{
				if (T66MapGeneration::RoomHasStructure(Room, T66MapGeneration::StructureScatterStones))
				{
					T66MapGeneration::PlaceScatterStoneStructure(PlacementContext, Room, FMath::Min(2, TargetCount));
				}
				continue;
			}

			// Scatter structure / degraded composition: sparse breather scatter +
			// a ground reward in the open.
			T66MapGeneration::PlaceScatterStoneStructure(PlacementContext, Room, TargetCount);

			// Every structured room ends with at least one payoff point; the
			// interactable population fills these first.
			if (Room.StructureIDs.Num() > 0 && Room.RewardSlots.Num() <= 0)
			{
				T66MapGeneration::AddRewardSlot(Room, FVector(Room.WorldCenter.X, Room.WorldCenter.Y, Floor.SurfaceZ));
			}
		}

		// Ramps: walkable wedges from the base floor onto scattered Tier 1 platforms.
		int32 RampsPlaced = 0;
		for (const T66MapGeneration::FScatterRampCandidate& RampTarget : RampCandidates)
		{
			if (RampsPlaced >= 2)
			{
				break;
			}

			for (const FIntPoint& Delta : NeighborDeltas)
			{
				const FIntPoint RampCell = RampTarget.Cell + Delta;
				if (!T66IsWalkableBounceCell(Layout, Floor, RampCell))
				{
					continue;
				}

				const int32 RampCellIndex = T66GetGridCellIndex(Layout, RampCell);
				const bool bRampNearHole = FMath::Abs(RampCell.X - Floor.ExitCell.X) <= 1
					&& FMath::Abs(RampCell.Y - Floor.ExitCell.Y) <= 1;
				if (OccupiedCells.Contains(RampCellIndex) || bRampNearHole)
				{
					continue;
				}

				// Ascent runs from the ramp cell toward the platform; the high end
				// touches the platform's near edge.
				const FIntPoint AscentSign(-Delta.X, -Delta.Y);
				const FVector2D AscentDir(static_cast<float>(AscentSign.X), static_cast<float>(AscentSign.Y));
				int32 TargetPlatformIndex = INDEX_NONE;
				for (int32 PlatformIndex = 0; PlatformIndex < Floor.BouncePlatforms.Num(); ++PlatformIndex)
				{
					if (Floor.BouncePlatforms[PlatformIndex].Cell == RampTarget.Cell)
					{
						TargetPlatformIndex = PlatformIndex;
						break;
					}
				}

				if (TargetPlatformIndex == INDEX_NONE)
				{
					break;
				}

				const T66TowerMapTerrain::FBouncePlatform& Target = Floor.BouncePlatforms[TargetPlatformIndex];
				const FVector2D TargetCenter = (Target.Bounds.Min + Target.Bounds.Max) * 0.5f;
				const float TargetHalf = (Target.Bounds.Max.X - Target.Bounds.Min.X) * 0.5f;
				const FVector2D HighEnd = TargetCenter - (AscentDir * TargetHalf);
				const FVector2D RampCenter = HighEnd - (AscentDir * (Layout.RampLength * 0.5f));
				const FVector2D AlongHalf = AscentDir * (Layout.RampLength * 0.5f);
				const FVector2D AcrossHalf = FVector2D(FMath::Abs(AscentDir.Y), FMath::Abs(AscentDir.X)) * (Layout.RampWidth * 0.5f);
				const FVector2D HalfExtents(
					FMath::Abs(AlongHalf.X) + FMath::Abs(AcrossHalf.X),
					FMath::Abs(AlongHalf.Y) + FMath::Abs(AcrossHalf.Y));

				T66TowerMapTerrain::FBounceRamp& Ramp = Floor.BounceRamps.AddDefaulted_GetRef();
				Ramp.Bounds = FBox2D(RampCenter - HalfExtents, RampCenter + HalfExtents);
				Ramp.BaseZ = Floor.SurfaceZ;
				Ramp.TopZ = Target.TopZ;
				Ramp.AscentSign = AscentSign;
				OccupiedCells.Add(RampCellIndex);
				++RampsPlaced;
				break;
			}
		}

		// Validate the dry-chain guarantee so regressions surface in logs, not playtests:
		// consecutive dry anchors along the path (chain platforms and ramp wedges)
		// within jump range, chain tops above the lava cap, and the descent hole
		// reachable from the final dry anchor.
		float MaxChainGap = 0.0f;
		const FBox2D* PreviousAnchor = nullptr;
		const FBox2D* LastAnchor = nullptr;
		for (const FIntPoint& PathCell : PathCells)
		{
			const int32 PathCellIndex = T66GetGridCellIndex(Layout, PathCell);
			const FBox2D* Anchor = DryAnchorByCell.Find(PathCellIndex);
			if (!Anchor)
			{
				continue;
			}

			if (PreviousAnchor)
			{
				MaxChainGap = FMath::Max(MaxChainGap, T66BoxEdgeGap2D(*PreviousAnchor, *Anchor));
			}
			PreviousAnchor = Anchor;
			LastAnchor = Anchor;
		}

		const float LavaClearance = Tier2TopZ - (Floor.SurfaceZ + Layout.LavaMaxHeight);
		float HoleReach = 0.0f;
		if (LastAnchor && Floor.bHasDropHole)
		{
			const FBox2D HoleBox(
				FVector2D(Floor.HoleCenter.X - Floor.HoleHalfExtent.X, Floor.HoleCenter.Y - Floor.HoleHalfExtent.Y),
				FVector2D(Floor.HoleCenter.X + Floor.HoleHalfExtent.X, Floor.HoleCenter.Y + Floor.HoleHalfExtent.Y));
			HoleReach = T66BoxEdgeGap2D(*LastAnchor, HoleBox);
		}

		const bool bChainPass = bHasChainPath
			&& (ChainPlatformCount > 0 || PathCells.Num() <= 2)
			&& MaxChainGap <= Layout.PlatformChainMaxGap + 1.0f
			&& HoleReach <= Layout.PlatformChainMaxGap + 1.0f
			&& LavaClearance >= 50.0f;
		if (bChainPass)
		{
			UE_LOG(
				LogT66TowerMapTerrain,
				Log,
				TEXT("[T66Proof][BounceCourseSummary] Floor=%d Result=PASS Platforms=%d Chain=%d Ramps=%d PathCells=%d MaxChainGap=%.0f HoleReach=%.0f LavaClearance=%.0f"),
				Floor.FloorNumber,
				Floor.BouncePlatforms.Num(),
				ChainPlatformCount,
				Floor.BounceRamps.Num(),
				PathCells.Num(),
				MaxChainGap,
				HoleReach,
				LavaClearance);
		}
		else
		{
			UE_LOG(
				LogT66TowerMapTerrain,
				Warning,
				TEXT("[T66Proof][BounceCourseSummary] Floor=%d Result=FAIL Platforms=%d Chain=%d Ramps=%d PathCells=%d MaxChainGap=%.0f HoleReach=%.0f LavaClearance=%.0f"),
				Floor.FloorNumber,
				Floor.BouncePlatforms.Num(),
				ChainPlatformCount,
				Floor.BounceRamps.Num(),
				PathCells.Num(),
				MaxChainGap,
				HoleReach,
				LavaClearance);
		}

		if (!bChainPass)
		{
			// Without a proven dry chain the lava hazard would be unfair; drop the
			// course markers so the hazard manager falls back to legacy coverage.
			Floor.SafeChainCells.Reset();
		}
	}

	static void T66ValidateRoomCompositions(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor)
	{
		int32 ComposedRooms = 0;
		int32 ValidRooms = 0;
		int32 InvalidRooms = 0;
		FString FirstFailure;
		for (const T66TowerMapTerrain::FRoom& Room : Floor.Rooms)
		{
			if (Room.CompositionProfileID.IsNone())
			{
				continue;
			}

			++ComposedRooms;
			const T66MapGeneration::FRoomValidationResult Result =
				T66MapGeneration::ValidateRoomComposition(Layout, Floor, Room);
			if (Result.bValid)
			{
				++ValidRooms;
			}
			else
			{
				++InvalidRooms;
				if (FirstFailure.IsEmpty())
				{
					FirstFailure = FString::Printf(
						TEXT("Room=%d Profile=%s Reason=%s Structures=%d Rewards=%d Hazards=%d Open=%.2f Density=%.2f"),
						Room.RoomId,
						*Room.CompositionProfileID.ToString(),
						*Result.FailureReason,
						Result.StructureCount,
						Result.RewardSlotCount,
						Result.HazardAnchorCount,
						Result.EstimatedCombatOpenAreaRatio,
						Result.StructureDensity);
				}
			}
		}

		if (ComposedRooms <= 0)
		{
			return;
		}

		if (InvalidRooms <= 0)
		{
			UE_LOG(
				LogT66TowerMapTerrain,
				Log,
				TEXT("[T66Proof][RoomCompositionSummary] Floor=%d Result=PASS Rooms=%d Valid=%d Invalid=%d"),
				Floor.FloorNumber,
				ComposedRooms,
				ValidRooms,
				InvalidRooms);
		}
		else
		{
			UE_LOG(
				LogT66TowerMapTerrain,
				Warning,
				TEXT("[T66Proof][RoomCompositionSummary] Floor=%d Result=FAIL Rooms=%d Valid=%d Invalid=%d FirstFailure=\"%s\""),
				Floor.FloorNumber,
				ComposedRooms,
				ValidRooms,
				InvalidRooms,
				*FirstFailure);
		}
	}

	static void T66BuildFloorMazeWalls(
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor,
		FRandomStream& Rng)
	{
		T66ResetFloorMazeMetadata(Floor);
		if (!Floor.bMobFloor
			|| Floor.FloorNumber == Layout.StartFloorNumber
			|| Floor.FloorNumber == Layout.BossFloorNumber)
		{
			return;
		}

		bool bBuiltDungeonRooms = false;
		bool bBuiltGridGraph = false;
		FRandomStream GridRng = Rng;
		bBuiltGridGraph = T66BuildFloorDungeonLoop(Layout, Floor, GridRng);
		bBuiltDungeonRooms = bBuiltGridGraph;
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

		// Tier terrain + bounce platforms only build over the dungeon-room layout:
		// the grid-graph fallback emits interior template walls inside cells that
		// raised terrain could intersect, and legacy lanes have no cell semantics.
		if (bBuiltDungeonRooms)
		{
			T66BuildFloorTierTerrain(Layout, Floor, Rng);
			T66BuildFloorBouncePlatforms(Layout, Floor, Rng);
			T66ValidateRoomCompositions(Layout, Floor);

			// No-softlock proof: every walkable cell must be reachable from arrival
			// over the directed (cell, tier) walk graph; the return path exists by
			// construction (drops reach the connected ground network).
			int32 ReachedCells = 0;
			int32 TotalWalkableCells = 0;
			bool bExitReached = false;
			T66ValidateTierAccess(Layout, Floor, ReachedCells, TotalWalkableCells, bExitReached);
			const bool bTierPass = ReachedCells == TotalWalkableCells && (bExitReached || !Floor.bHasDropHole);
			if (bTierPass)
			{
				UE_LOG(
					LogT66TowerMapTerrain,
					Log,
					TEXT("[T66Proof][TierAccessSummary] Floor=%d Result=PASS Reached=%d/%d ExitReached=%d Mesas=%d TierRamps=%d Lifts=%d"),
					Floor.FloorNumber,
					ReachedCells,
					TotalWalkableCells,
					bExitReached ? 1 : 0,
					Floor.TierMesas.Num(),
					Floor.TierRamps.Num(),
					Floor.TierLifts.Num());
			}
			else
			{
				UE_LOG(
					LogT66TowerMapTerrain,
					Warning,
					TEXT("[T66Proof][TierAccessSummary] Floor=%d Result=FAIL Reached=%d/%d ExitReached=%d Mesas=%d TierRamps=%d Lifts=%d"),
					Floor.FloorNumber,
					ReachedCells,
					TotalWalkableCells,
					bExitReached ? 1 : 0,
					Floor.TierMesas.Num(),
					Floor.TierRamps.Num(),
					Floor.TierLifts.Num());

				// Unverified terrain must not gate the lava hazard or ship raised
				// blocks the player could be boxed in by: flatten the floor.
				Floor.CellTiers.Reset();
				Floor.TierMesas.Reset();
				Floor.TierRamps.Reset();
				Floor.TierLifts.Reset();
				Floor.SafeChainCells.Reset();
			}
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

	static void T66EmitHorizontalWallWithGap(
		TArray<FBox2D>& OutBoxes,
		const float MinX,
		const float MaxX,
		const float CenterY,
		const float HalfThickness,
		const float GapCenterX,
		const float GapHalfWidth)
	{
		T66EmitWallRect(OutBoxes, MinX, CenterY - HalfThickness, GapCenterX - GapHalfWidth, CenterY + HalfThickness);
		T66EmitWallRect(OutBoxes, GapCenterX + GapHalfWidth, CenterY - HalfThickness, MaxX, CenterY + HalfThickness);
	}

	static void T66EmitVerticalWallWithGap(
		TArray<FBox2D>& OutBoxes,
		const float CenterX,
		const float MinY,
		const float MaxY,
		const float HalfThickness,
		const float GapCenterY,
		const float GapHalfWidth)
	{
		T66EmitWallRect(OutBoxes, CenterX - HalfThickness, MinY, CenterX + HalfThickness, GapCenterY - GapHalfWidth);
		T66EmitWallRect(OutBoxes, CenterX - HalfThickness, GapCenterY + GapHalfWidth, CenterX + HalfThickness, MaxY);
	}

	static void T66AddStartGalleryWing(
		T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor,
		const T66TowerMapTerrain::ET66TowerStartGalleryCategory Category,
		const FName CategoryID,
		const FVector& Direction,
		const FVector& SideDirection,
		const FVector2D& HubCenter,
		const float HubHalfExtent,
		const float ConnectorLength,
		const float ConnectorHalfWidth,
		const float WingAcrossHalfExtent,
		const float WingDepthHalfExtent,
		const float WallHalfThickness,
		const float DoorHalfWidth)
	{
		const FVector2D Direction2D(Direction.X, Direction.Y);
		const FVector2D ConnectorStart = HubCenter + Direction2D * HubHalfExtent;
		const FVector2D ConnectorEnd = ConnectorStart + Direction2D * ConnectorLength;
		const FVector2D WingCenter = ConnectorEnd + Direction2D * WingDepthHalfExtent;

		const bool bVerticalWing = FMath::Abs(Direction2D.Y) >= FMath::Abs(Direction2D.X);
		FBox2D ConnectorBox;
		FBox2D WingBox;
		if (bVerticalWing)
		{
			const float ConnectorMinY = FMath::Min(ConnectorStart.Y, ConnectorEnd.Y);
			const float ConnectorMaxY = FMath::Max(ConnectorStart.Y, ConnectorEnd.Y);
			ConnectorBox = FBox2D(
				FVector2D(HubCenter.X - ConnectorHalfWidth, ConnectorMinY),
				FVector2D(HubCenter.X + ConnectorHalfWidth, ConnectorMaxY));
			WingBox = FBox2D(
				FVector2D(WingCenter.X - WingAcrossHalfExtent, WingCenter.Y - WingDepthHalfExtent),
				FVector2D(WingCenter.X + WingAcrossHalfExtent, WingCenter.Y + WingDepthHalfExtent));

			T66EmitWallRect(Floor.MazeWallBoxes, ConnectorBox.Min.X - WallHalfThickness, ConnectorBox.Min.Y, ConnectorBox.Min.X + WallHalfThickness, ConnectorBox.Max.Y);
			T66EmitWallRect(Floor.MazeWallBoxes, ConnectorBox.Max.X - WallHalfThickness, ConnectorBox.Min.Y, ConnectorBox.Max.X + WallHalfThickness, ConnectorBox.Max.Y);

			T66EmitWallRect(Floor.MazeWallBoxes, WingBox.Min.X - WallHalfThickness, WingBox.Min.Y, WingBox.Min.X + WallHalfThickness, WingBox.Max.Y);
			T66EmitWallRect(Floor.MazeWallBoxes, WingBox.Max.X - WallHalfThickness, WingBox.Min.Y, WingBox.Max.X + WallHalfThickness, WingBox.Max.Y);
			const float InwardWallY = Direction2D.Y > 0.0f ? WingBox.Min.Y : WingBox.Max.Y;
			const float FarWallY = Direction2D.Y > 0.0f ? WingBox.Max.Y : WingBox.Min.Y;
			T66EmitHorizontalWallWithGap(Floor.MazeWallBoxes, WingBox.Min.X, WingBox.Max.X, InwardWallY, WallHalfThickness, HubCenter.X, DoorHalfWidth);
			T66EmitWallRect(Floor.MazeWallBoxes, WingBox.Min.X, FarWallY - WallHalfThickness, WingBox.Max.X, FarWallY + WallHalfThickness);
		}
		else
		{
			const float ConnectorMinX = FMath::Min(ConnectorStart.X, ConnectorEnd.X);
			const float ConnectorMaxX = FMath::Max(ConnectorStart.X, ConnectorEnd.X);
			ConnectorBox = FBox2D(
				FVector2D(ConnectorMinX, HubCenter.Y - ConnectorHalfWidth),
				FVector2D(ConnectorMaxX, HubCenter.Y + ConnectorHalfWidth));
			WingBox = FBox2D(
				FVector2D(WingCenter.X - WingDepthHalfExtent, WingCenter.Y - WingAcrossHalfExtent),
				FVector2D(WingCenter.X + WingDepthHalfExtent, WingCenter.Y + WingAcrossHalfExtent));

			T66EmitWallRect(Floor.MazeWallBoxes, ConnectorBox.Min.X, ConnectorBox.Min.Y - WallHalfThickness, ConnectorBox.Max.X, ConnectorBox.Min.Y + WallHalfThickness);
			T66EmitWallRect(Floor.MazeWallBoxes, ConnectorBox.Min.X, ConnectorBox.Max.Y - WallHalfThickness, ConnectorBox.Max.X, ConnectorBox.Max.Y + WallHalfThickness);

			T66EmitWallRect(Floor.MazeWallBoxes, WingBox.Min.X, WingBox.Min.Y - WallHalfThickness, WingBox.Max.X, WingBox.Min.Y + WallHalfThickness);
			T66EmitWallRect(Floor.MazeWallBoxes, WingBox.Min.X, WingBox.Max.Y - WallHalfThickness, WingBox.Max.X, WingBox.Max.Y + WallHalfThickness);
			const float InwardWallX = Direction2D.X > 0.0f ? WingBox.Min.X : WingBox.Max.X;
			const float FarWallX = Direction2D.X > 0.0f ? WingBox.Max.X : WingBox.Min.X;
			T66EmitVerticalWallWithGap(Floor.MazeWallBoxes, InwardWallX, WingBox.Min.Y, WingBox.Max.Y, WallHalfThickness, HubCenter.Y, DoorHalfWidth);
			T66EmitWallRect(Floor.MazeWallBoxes, FarWallX - WallHalfThickness, WingBox.Min.Y, FarWallX + WallHalfThickness, WingBox.Max.Y);
		}

		Floor.WalkableFloorBoxes.Add(ConnectorBox);
		Floor.WalkableFloorBoxes.Add(WingBox);

		T66TowerMapTerrain::FStartGalleryWing& Wing = Layout.StartGalleryWings.AddDefaulted_GetRef();
		Wing.Category = Category;
		Wing.CategoryID = CategoryID;
		Wing.Center = FVector(WingCenter.X, WingCenter.Y, Floor.SurfaceZ);
		Wing.Direction = Direction.GetSafeNormal2D();
		Wing.SideDirection = SideDirection.GetSafeNormal2D();
		Wing.SurfaceZ = Floor.SurfaceZ;
		Wing.AcrossHalfExtent = WingAcrossHalfExtent;
		Wing.DepthHalfExtent = WingDepthHalfExtent;
		Wing.WalkableBox = WingBox;
		Wing.FloorNumber = Floor.FloorNumber;
	}

	static void T66BuildStartFloorRoom(
		T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor,
		const FVector& SpawnLocation,
		const FVector& AltarLocation)
	{
		(void)SpawnLocation;
		T66ResetFloorMazeMetadata(Floor);
		Layout.StartGalleryWings.Reset();

		const float HalfThickness = FMath::Max(Layout.WallThickness * 0.25f, Layout.PlacementCellSize * T66GetMazeWallHalfThicknessScale(Layout));
		const float RoomHalfExtent = FMath::Max(Layout.PlacementCellSize, T66GetTowerTuning().StartRoomSquareSize) * 0.5f;
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

		const FBox2D StartRoomBounds(FVector2D(MinX, MinY), FVector2D(MaxX, MaxY));
		Floor.WalkableFloorBoxes.Add(StartRoomBounds);
		T66AddBoxRoomRecord(Floor, 0, StartRoomBounds, Layout.StartRoomRuleID, TEXT("Start"));
		T66EmitWallRect(Floor.MazeWallBoxes, MinX - HalfThickness, MinY, MinX + HalfThickness, MaxY);
		T66EmitWallRect(Floor.MazeWallBoxes, MaxX - HalfThickness, MinY, MaxX + HalfThickness, MaxY);
		T66EmitWallRect(Floor.MazeWallBoxes, MinX, MinY - HalfThickness, MaxX, MinY + HalfThickness);
		T66EmitWallRect(Floor.MazeWallBoxes, MinX, MaxY - HalfThickness, MaxX, MaxY + HalfThickness);
		T66FinalizeFloorMazeMetadata(Layout, Floor);
	}

	static void T66BuildBossFloorRoom(
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor)
	{
		T66ResetFloorMazeMetadata(Floor);

		const float HalfThickness = FMath::Max(Layout.WallThickness * 0.25f, Layout.PlacementCellSize * T66GetMazeWallHalfThicknessScale(Layout));
		const float RoomBoundsInset = Layout.WallThickness + (Layout.PlacementCellSize * 0.20f);
		const float RoomHalfExtent = FMath::Max(1400.0f, Floor.BoundsHalfExtent - RoomBoundsInset);
		const float MinX = Floor.Center.X - RoomHalfExtent;
		const float MaxX = Floor.Center.X + RoomHalfExtent;
		const float MinY = Floor.Center.Y - RoomHalfExtent;
		const float MaxY = Floor.Center.Y + RoomHalfExtent;

		const FBox2D BossRoomBounds(FVector2D(MinX, MinY), FVector2D(MaxX, MaxY));
		Floor.WalkableFloorBoxes.Add(BossRoomBounds);
		T66AddBoxRoomRecord(Floor, 0, BossRoomBounds, Layout.BossRoomRuleID, TEXT("Boss"));
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
		(void)Layout;

		if (!T66ShouldUseGeneratedDungeonKit()
			|| !World
			|| !Theme.FloorMaterial
			|| Floor.BoundsHalfExtent <= 1.0f)
		{
			return false;
		}

		UStaticMesh* CubeMesh = FT66VisualUtil::GetBasicShapeCube();
		if (!CubeMesh)
		{
			return false;
		}

		TArray<FName> FloorTags = Tags;
		FloorTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit")));
		FloorTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_Floor")));
		FloorTags.AddUnique(FName(*FString::Printf(TEXT("T66_Floor_Tower_GeneratedDungeonKit_Floor_%02d"), Floor.FloorNumber)));

		const float FloorThickness = Layout.FloorThickness;
		int32 VisualRectangleCount = 0;

		auto SpawnVisualRectangleForBox = [&](const FBox2D& SourceBox)
		{
			const FVector2D BoxSize = SourceBox.Max - SourceBox.Min;
			if (BoxSize.X <= 10.0f || BoxSize.Y <= 10.0f)
			{
				return;
			}

			if (T66ShouldUseFloorBaffles()
				&& T66SpawnFloorBaffleTubeVisualsForBox(World, SourceBox, Floor, Theme.FloorMaterial, SpawnParams, FloorTags))
			{
				++VisualRectangleCount;
				return;
			}

			const FVector2D Center = (SourceBox.Min + SourceBox.Max) * 0.5f;
			AStaticMeshActor* FloorVisual = T66SpawnEnvironmentRectangle(
				World,
				CubeMesh,
				Theme.FloorMaterial,
				FVector(Center.X, Center.Y, Floor.SurfaceZ - (FloorThickness * 0.5f)),
				FVector(BoxSize.X * 0.5f, BoxSize.Y * 0.5f, FloorThickness * 0.5f),
				SpawnParams,
				FloorTags,
				true);
			if (FloorVisual)
			{
				++VisualRectangleCount;
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
				FVector(Center.X, Center.Y, Floor.SurfaceZ - (FloorThickness * 0.5f)),
				FRotator::ZeroRotator,
				FVector(BoxSize.X * 0.5f, BoxSize.Y * 0.5f, FloorThickness * 0.5f),
				SpawnParams,
				CollisionTags,
				true);
		};

		auto SpawnSurfaceBox = [&](const FBox2D& SourceBox)
		{
			SpawnVisualRectangleForBox(SourceBox);
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

			// FIRST-PASS: multi-rectangle drop-hole floors may show edge seams; revisit with masked-material or procedural-cutout solution if seams read badly.
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

		return VisualRectangleCount > 0;
	}
	static bool T66SpawnGeneratedDungeonFloorUndersideTiles(
		UWorld* World,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const float CeilingBottomZ,
		const FActorSpawnParameters& SpawnParams,
		const TArray<FName>& Tags,
		const bool bRespectDropHole,
		const bool bSpawnCeilingCollision)
	{
		if (!T66ShouldUseGeneratedDungeonKit()
			|| !World
			|| Floor.BoundsHalfExtent <= 1.0f)
		{
			return false;
		}

		UMaterialInterface* CeilingMaterial = Theme.CeilingMaterial ? Theme.CeilingMaterial : Theme.RoofMaterial;
		if (!CeilingMaterial)
		{
			return false;
		}

		TArray<FName> CeilingTags = Tags;
		CeilingTags.AddUnique(T66TowerMapCeilingTag);
		CeilingTags.AddUnique(T66TowerTerrainNoSurfaceBounceTag);
		CeilingTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit")));
		CeilingTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_FloorUnderside")));
		CeilingTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_Ceiling")));
		CeilingTags.AddUnique(FName(*FString::Printf(TEXT("T66_Floor_Tower_GeneratedDungeonKit_Ceiling_%02d"), Floor.FloorNumber)));

		const float CeilingThickness = Layout.FloorThickness;
		int32 VisualRectangleCount = 0;

		auto SpawnVisualRectangleForBox = [&](const FBox2D& SourceBox)
		{
			const FVector2D BoxSize = SourceBox.Max - SourceBox.Min;
			if (BoxSize.X <= 10.0f || BoxSize.Y <= 10.0f)
			{
				return;
			}

			if (T66ShouldUseCeilingBaffles()
				&& T66SpawnCeilingBaffleTubeVisualsForBox(World, SourceBox, Floor, CeilingBottomZ, CeilingMaterial, SpawnParams, CeilingTags))
			{
				++VisualRectangleCount;
				return;
			}

			UStaticMesh* CubeMesh = FT66VisualUtil::GetBasicShapeCube();
			if (!CubeMesh)
			{
				return;
			}

			const FVector2D Center = (SourceBox.Min + SourceBox.Max) * 0.5f;
			AStaticMeshActor* CeilingVisual = T66SpawnEnvironmentRectangle(
				World,
				CubeMesh,
				CeilingMaterial,
				FVector(Center.X, Center.Y, CeilingBottomZ + (CeilingThickness * 0.5f)),
				FVector(BoxSize.X * 0.5f, BoxSize.Y * 0.5f, CeilingThickness * 0.5f),
				SpawnParams,
				CeilingTags,
				true);
			if (CeilingVisual)
			{
				++VisualRectangleCount;
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
			CollisionTags.AddUnique(T66TowerTerrainNoSurfaceBounceTag);
			CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit")));
			CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_CollisionProxy")));
			CollisionTags.AddUnique(FName(TEXT("T66_Floor_Tower_GeneratedDungeonKit_CeilingCollision")));
			CollisionTags.AddUnique(FName(*FString::Printf(TEXT("T66_Floor_Tower_GeneratedDungeonKit_CeilingCollision_%02d"), Floor.FloorNumber)));

			const FVector2D Center = (SourceBox.Min + SourceBox.Max) * 0.5f;
			T66SpawnHiddenCollisionProxyActor(
				World,
				FVector(Center.X, Center.Y, CeilingBottomZ + (CeilingThickness * 0.5f)),
				FRotator::ZeroRotator,
				FVector(BoxSize.X * 0.5f, BoxSize.Y * 0.5f, CeilingThickness * 0.5f),
				SpawnParams,
				CollisionTags,
				false);
		};

		// Camera blocker (2026-06-10): the hero boom now collision-tests against
		// ECC_Camera, and a camera inside/level with a ceiling slab blacks out the
		// scene. Ceilings get an invisible camera-only proxy so the boom physically
		// stays below them; gameplay channels are untouched.
		auto SpawnCeilingCameraBlockerForBox = [&](const FBox2D& SourceBox)
		{
			if (bSpawnCeilingCollision)
			{
				return; // the full collision proxy already blocks the camera
			}

			const FVector2D BoxSize = SourceBox.Max - SourceBox.Min;
			if (BoxSize.X <= 10.0f || BoxSize.Y <= 10.0f)
			{
				return;
			}

			TArray<FName> BlockerTags = Tags;
			BlockerTags.AddUnique(T66TowerMapCeilingTag);
			BlockerTags.AddUnique(T66TowerTerrainNoSurfaceBounceTag);
			BlockerTags.AddUnique(FName(TEXT("T66_Floor_Tower_CeilingCameraBlocker")));

			const FVector2D Center = (SourceBox.Min + SourceBox.Max) * 0.5f;
			AActor* Proxy = T66SpawnHiddenCollisionProxyActor(
				World,
				FVector(Center.X, Center.Y, CeilingBottomZ + (CeilingThickness * 0.5f)),
				FRotator::ZeroRotator,
				FVector(BoxSize.X * 0.5f, BoxSize.Y * 0.5f, CeilingThickness * 0.5f),
				SpawnParams,
				BlockerTags,
				false);
			T66MakeCollisionProxyCameraOnly(Proxy);
		};

		auto SpawnSurfaceBox = [&](const FBox2D& SourceBox)
		{
			SpawnVisualRectangleForBox(SourceBox);
			SpawnCeilingCollisionForBox(SourceBox);
			SpawnCeilingCameraBlockerForBox(SourceBox);
		};

		auto SpawnBoxWithHole = [&](const FBox2D& SourceBox)
		{
			if (!bRespectDropHole
				|| !Floor.bHasDropHole
				|| Floor.HoleHalfExtent.X <= 1.0f
				|| Floor.HoleHalfExtent.Y <= 1.0f)
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

		return VisualRectangleCount > 0;
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
		TArray<FName> VisualTags = Tags;
		VisualTags.AddUnique(T66TowerMapCameraOccludingWallVisualTag);

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
					VisualTags);
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
			return Floor.SurfaceZ + Layout.StartFloorHeadroom;
		}

		const T66TowerMapTerrain::FFloor& CarrierFloor = Layout.Floors[FloorIndex - 1];
		return CarrierFloor.SurfaceZ - Layout.FloorThickness - Layout.RoofSkinThickness;
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
			OutRoofSurfaceZ = Floor.SurfaceZ + Layout.StartFloorHeadroom + Layout.FloorThickness;
			OutRoofThickness = Layout.FloorThickness;
			bOutEnableCollision = true;
			return true;
		}

		OutRoofGeometryFloor = Layout.Floors[FloorIndex - 1];
		OutRoofSurfaceZ = OutRoofGeometryFloor.SurfaceZ - Layout.FloorThickness;
		OutRoofThickness = Layout.RoofSkinThickness;
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
		const FVector2D& SingleGeneratedVisualNormal = FVector2D::ZeroVector,
		FT66GeneratedDungeonWallBatch* WallBatch = nullptr)
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
		TArray<FName> VisualTags = Tags;
		VisualTags.AddUnique(T66TowerMapCameraOccludingWallVisualTag);

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
					VisualTags,
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
				bIgnoreCameraChannel,
				WallBatch);

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
					VisualTags,
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
					VisualTags,
					bIgnoreCameraChannel);
			}
			return;
		}
	}

	static void T66AppendShellWallBoxSegments(
		TArray<FBox2D>& OutBoxes,
		const FBox2D& WallBox,
		const T66TowerThemeVisuals::FResolvedTheme& Theme,
		const bool bSplitGeneratedWalls)
	{
		const FVector2D WallSize = WallBox.Max - WallBox.Min;
		if (WallSize.X <= 10.0f || WallSize.Y <= 10.0f)
		{
			return;
		}

		const bool bWallRunsAlongX = WallSize.X >= WallSize.Y;
		const float SpanLength = bWallRunsAlongX ? WallSize.X : WallSize.Y;
		int32 SegmentCount = 1;
		if (bSplitGeneratedWalls && Theme.WallMeshes.Num() > 0)
		{
			const float NativeWallUnitLength = T66GetMeshAxisSize(Theme.WallMeshes[0], 1);
			const float PlannedWallUnitLength = FMath::Max(
				1.0f,
				FMath::Min(
					FMath::Max(NativeWallUnitLength, T66GetGeneratedKitWallVisualTargetSegmentLength()),
					SpanLength));
			SegmentCount = T66GetNativeDungeonKitModuleCount(SpanLength, PlannedWallUnitLength);
		}

		SegmentCount = FMath::Max(1, SegmentCount);
		if (SegmentCount <= 1)
		{
			OutBoxes.Add(WallBox);
			return;
		}

		const float SegmentLength = SpanLength / static_cast<float>(SegmentCount);
		const float SpanMin = bWallRunsAlongX ? WallBox.Min.X : WallBox.Min.Y;
		for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
		{
			const float SegmentMin = SpanMin + (SegmentLength * static_cast<float>(SegmentIndex));
			const float SegmentMax = (SegmentIndex == SegmentCount - 1)
				? (bWallRunsAlongX ? WallBox.Max.X : WallBox.Max.Y)
				: (SegmentMin + SegmentLength);
			OutBoxes.Add(bWallRunsAlongX
				? FBox2D(FVector2D(SegmentMin, WallBox.Min.Y), FVector2D(SegmentMax, WallBox.Max.Y))
				: FBox2D(FVector2D(WallBox.Min.X, SegmentMin), FVector2D(WallBox.Max.X, SegmentMax)));
		}
	}

	static void T66SpawnShellWallsForFloor(
		UWorld* World,
		UStaticMesh* CubeMesh,
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::FFloor& Floor,
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
		const bool bBatchGeneratedWalls =
			T66ShouldUseGeneratedDungeonKit()
			&& Theme.WallFamily == T66TowerThemeVisuals::EWallFamily::SplitCollisionVisual
			&& Theme.WallMeshes.Num() > 0;
		FT66GeneratedDungeonWallBatch WallBatch;
		if (bBatchGeneratedWalls)
		{
			WallBatch.Reset(Theme.WallMeshes.Num());
		}

		Floor.OuterShellWallBoxes.Reset();
		const FBox2D EastShellBox(FVector2D(Layout.ShellRadius - WallHalfDepth, -WallHalfSpan), FVector2D(Layout.ShellRadius + WallHalfDepth, WallHalfSpan));
		const FBox2D WestShellBox(FVector2D(-Layout.ShellRadius - WallHalfDepth, -WallHalfSpan), FVector2D(-Layout.ShellRadius + WallHalfDepth, WallHalfSpan));
		const FBox2D NorthShellBox(FVector2D(-WallHalfSpan, Layout.ShellRadius - WallHalfDepth), FVector2D(WallHalfSpan, Layout.ShellRadius + WallHalfDepth));
		const FBox2D SouthShellBox(FVector2D(-WallHalfSpan, -Layout.ShellRadius - WallHalfDepth), FVector2D(WallHalfSpan, -Layout.ShellRadius + WallHalfDepth));
		T66AppendShellWallBoxSegments(Floor.OuterShellWallBoxes, EastShellBox, Theme, bBatchGeneratedWalls);
		T66AppendShellWallBoxSegments(Floor.OuterShellWallBoxes, WestShellBox, Theme, bBatchGeneratedWalls);
		T66AppendShellWallBoxSegments(Floor.OuterShellWallBoxes, NorthShellBox, Theme, bBatchGeneratedWalls);
		T66AppendShellWallBoxSegments(Floor.OuterShellWallBoxes, SouthShellBox, Theme, bBatchGeneratedWalls);

		T66SpawnThemedWallBox(
			World,
			CubeMesh,
			Theme,
			EastShellBox,
			Floor.SurfaceZ,
			WallHeight,
			SpawnParams,
			ShellTags,
			Layout.Preset.Seed + (Floor.FloorNumber * 4101),
			T66ShouldIgnoreTowerWallCameraCollision(),
			true,
			FVector2D(-1.0f, 0.0f),
			bBatchGeneratedWalls ? &WallBatch : nullptr);
		T66SpawnThemedWallBox(
			World,
			CubeMesh,
			Theme,
			WestShellBox,
			Floor.SurfaceZ,
			WallHeight,
			SpawnParams,
			ShellTags,
			Layout.Preset.Seed + (Floor.FloorNumber * 4102),
			T66ShouldIgnoreTowerWallCameraCollision(),
			true,
			FVector2D(1.0f, 0.0f),
			bBatchGeneratedWalls ? &WallBatch : nullptr);
		T66SpawnThemedWallBox(
			World,
			CubeMesh,
			Theme,
			NorthShellBox,
			Floor.SurfaceZ,
			WallHeight,
			SpawnParams,
			ShellTags,
			Layout.Preset.Seed + (Floor.FloorNumber * 4103),
			T66ShouldIgnoreTowerWallCameraCollision(),
			true,
			FVector2D(0.0f, -1.0f),
			bBatchGeneratedWalls ? &WallBatch : nullptr);
		T66SpawnThemedWallBox(
			World,
			CubeMesh,
			Theme,
			SouthShellBox,
			Floor.SurfaceZ,
			WallHeight,
			SpawnParams,
			ShellTags,
			Layout.Preset.Seed + (Floor.FloorNumber * 4104),
			T66ShouldIgnoreTowerWallCameraCollision(),
			true,
			FVector2D(0.0f, 1.0f),
			bBatchGeneratedWalls ? &WallBatch : nullptr);

		if (bBatchGeneratedWalls)
		{
			T66FlushGeneratedDungeonWallBatch(
				World,
				Theme,
				WallBatch,
				SpawnParams,
				ShellTags,
				TEXT("GeneratedShellWall"),
				T66ShouldIgnoreTowerWallCameraCollision());
		}

		UE_LOG(
			LogT66TowerMapTerrain,
			Display,
			TEXT("[ATMOSPHERE] Registered %d outer shell wall box(es) for floor %d."),
			Floor.OuterShellWallBoxes.Num(),
			Floor.FloorNumber);

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
		const bool bBatchGeneratedWalls =
			T66ShouldUseGeneratedDungeonKit()
			&& Theme.WallFamily == T66TowerThemeVisuals::EWallFamily::SplitCollisionVisual
			&& Theme.WallMeshes.Num() > 0;
		FT66GeneratedDungeonWallBatch WallBatch;
		if (bBatchGeneratedWalls)
		{
			WallBatch.Reset(Theme.WallMeshes.Num());
		}

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
				T66ShouldIgnoreTowerWallCameraCollision(),
				false,
				FVector2D::ZeroVector,
				bBatchGeneratedWalls ? &WallBatch : nullptr);
		}

		if (bBatchGeneratedWalls)
		{
			T66FlushGeneratedDungeonWallBatch(
				World,
				Theme,
				WallBatch,
				SpawnParams,
				WallTags,
				TEXT("GeneratedMazeWall"),
				T66ShouldIgnoreTowerWallCameraCollision());
		}

		if (Floor.DoorwayHeaderBoxes.Num() <= 0)
		{
			return;
		}

		const TArray<FName> DoorwayTags = {
			FName(*FString::Printf(TEXT("T66_Floor_Tower_%02d"), Floor.FloorNumber)),
			FName(*FString::Printf(TEXT("T66_Floor_Tower_Doorway_%02d"), Floor.FloorNumber))
		};

		// Inflatable arch tubes replace the flat lintel cube when baffle visuals are
		// active; the legacy lintel remains the fallback when arches cannot spawn.
		if (T66ShouldUseFloorBaffles()
			&& T66SpawnDoorwayArchTubes(World, Theme, Layout, Floor, WallHeight, SpawnParams, DoorwayTags) > 0)
		{
			return;
		}

		const float HeaderHeight = FMath::Clamp(WallHeight * 0.24f, 260.0f, 520.0f);
		if (!CubeMesh)
		{
			return;
		}

		const float HeaderZ = Floor.SurfaceZ + WallHeight - HeaderHeight - 80.0f;
		for (const FBox2D& HeaderBox : Floor.DoorwayHeaderBoxes)
		{
			const FVector2D HeaderCenter = (HeaderBox.Min + HeaderBox.Max) * 0.5f;
			const FVector2D HeaderHalfExtents = (HeaderBox.Max - HeaderBox.Min) * 0.5f;
			if (HeaderHalfExtents.X <= 5.0f || HeaderHalfExtents.Y <= 5.0f)
			{
				continue;
			}

			UMaterialInterface* HeaderMaterial = T66ResolveEnvironmentWallMaterialForBox(Theme, HeaderBox);
			if (!HeaderMaterial)
			{
				continue;
			}

			T66SpawnEnvironmentRectangle(
				World,
				CubeMesh,
				HeaderMaterial,
				FVector(HeaderCenter.X, HeaderCenter.Y, HeaderZ + (HeaderHeight * 0.5f)),
				FVector(HeaderHalfExtents.X, HeaderHalfExtents.Y, HeaderHeight * 0.5f),
				SpawnParams,
				DoorwayTags,
				T66ShouldIgnoreTowerWallCameraCollision());
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
			return FText::Format(
				NSLOCTEXT("T66.Tower", "StartFloor", "Floor {0} - Start"),
				FText::AsNumber(Floor.FloorNumber));
		case ET66TowerFloorRole::Boss:
			return FText::Format(
				NSLOCTEXT("T66.Tower", "BossFloor", "Floor {0} - Boss"),
				FText::AsNumber(Floor.FloorNumber));
		case ET66TowerFloorRole::Mob:
			return FText::Format(
				NSLOCTEXT("T66.Tower", "MobFloor", "Floor {0}"),
				FText::AsNumber(Floor.FloorNumber));
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
			return FText::Format(
				NSLOCTEXT("T66.Tower", "StartFloorByNumber", "Floor {0} - Start"),
				FText::AsNumber(FloorNumber));
		}
		if (FloorNumber == Layout.BossFloorNumber)
		{
			return FText::Format(
				NSLOCTEXT("T66.Tower", "BossFloorByNumber", "Floor {0} - Boss"),
				FText::AsNumber(FloorNumber));
		}

		return FText::Format(
			NSLOCTEXT("T66.Tower", "FloorFallbackByNumber", "Floor {0}"),
			FText::AsNumber(FloorNumber));
	}

	bool BuildLayout(const FT66MapPreset& Preset, FLayout& OutLayout, const bool bBossRushFinaleStage)
	{
		const UT66TowerTuningConfig& TowerTuning = T66GetTowerTuning();

		OutLayout = FLayout{};
		OutLayout.Preset = Preset;
		OutLayout.PlacementCellSize = TowerTuning.PlacementCellSize;
		OutLayout.RoofSkinThickness = TowerTuning.RoofSkinThickness;
		OutLayout.StartFloorHeadroom = TowerTuning.StartFloorHeadroom;
		OutLayout.GeneratedDungeonKitCullDistance = TowerTuning.GeneratedDungeonKitCullDistance;
		if (T66ShouldUseGeneratedDungeonKit())
		{
			OutLayout.FloorThickness = TowerTuning.GeneratedDungeonKitFloorThickness;
			OutLayout.FloorSpacing = TowerTuning.GeneratedDungeonKitWallHeight + OutLayout.FloorThickness;
		}
		else
		{
			OutLayout.FloorThickness = 280.0f;
			OutLayout.FloorSpacing = TowerTuning.StandardFloorHeadroom + OutLayout.FloorThickness + OutLayout.RoofSkinThickness;
		}
		OutLayout.WallThickness = TowerTuning.DungeonKitWallDepth;
		OutLayout.ShellRadius = TowerTuning.ShellRadius;
		OutLayout.MazeMode = T66GetConfiguredTowerMazeMode();
		OutLayout.GridColumns = TowerTuning.GridColumns;
		OutLayout.GridRows = TowerTuning.GridRows;
		OutLayout.GridCellSize = TowerTuning.GridCellSize;
		OutLayout.GridDoorWidth = TowerTuning.GridDoorWidth;
		OutLayout.DungeonMinRooms = TowerTuning.DungeonMinRooms;
		OutLayout.DungeonMaxRooms = TowerTuning.DungeonMaxRooms;
		OutLayout.DungeonMinRoomTiles = TowerTuning.DungeonMinRoomTiles;
		OutLayout.DungeonMaxRoomTiles = TowerTuning.DungeonMaxRoomTiles;
		OutLayout.RoomMaxGapCells = TowerTuning.RoomMaxGapCells;
		OutLayout.StartRoomMinTiles = TowerTuning.StartRoomMinTiles;
		OutLayout.StartRoomMaxTiles = TowerTuning.StartRoomMaxTiles;
		OutLayout.GridBranchChance = TowerTuning.GridBranchChance;
		OutLayout.GridMaxBranchCells = TowerTuning.GridMaxBranchCells;
		OutLayout.bBounceCoursePlatforms = TowerTuning.BounceCoursePlatforms != 0;
		OutLayout.PlatformTier1Height = TowerTuning.PlatformTier1Height;
		OutLayout.PlatformTier2Height = TowerTuning.PlatformTier2Height;
		OutLayout.ChainPlatformFootprint = TowerTuning.ChainPlatformFootprint;
		OutLayout.RoomPlatformFootprintMin = TowerTuning.RoomPlatformFootprintMin;
		OutLayout.RoomPlatformFootprintMax = TowerTuning.RoomPlatformFootprintMax;
		OutLayout.RoomPlatformDensityTiles = TowerTuning.RoomPlatformDensityTiles;
		OutLayout.PlatformChainMaxGap = TowerTuning.PlatformChainMaxGap;
		OutLayout.RoundPlatformChance = TowerTuning.RoundPlatformChance;
		OutLayout.RampWidth = TowerTuning.RampWidth;
		OutLayout.RampLength = TowerTuning.RampLength;
		OutLayout.LavaMaxHeight = TowerTuning.LavaMaxHeight;
		OutLayout.bTierTerrain = TowerTuning.TierTerrain != 0;
		OutLayout.TierHeight = TowerTuning.TierHeight;
		OutLayout.MesaInsetCells = TowerTuning.MesaInsetCells;
		OutLayout.MesaMinSpanCells = TowerTuning.MesaMinSpanCells;
		OutLayout.MesaRampsMin = TowerTuning.MesaRampsMin;
		OutLayout.MesaRampsMax = TowerTuning.MesaRampsMax;
		OutLayout.MesaTopBafflePitch = TowerTuning.MesaTopBafflePitch;
		OutLayout.MesaTopBaffleDiameter = TowerTuning.MesaTopBaffleDiameter;
		OutLayout.RampRollerDiameter = TowerTuning.RampRollerDiameter;
		OutLayout.RingMesaChance = TowerTuning.RingMesaChance;
		OutLayout.bTierLifts = TowerTuning.TierLifts != 0;
		OutLayout.LiftChance = TowerTuning.LiftChance;
		OutLayout.LiftFootprint = TowerTuning.LiftFootprint;
		OutLayout.LiftTravelSeconds = TowerTuning.LiftTravelSeconds;
		OutLayout.LiftDwellSeconds = TowerTuning.LiftDwellSeconds;
		OutLayout.bDoorwayArches = TowerTuning.DoorwayArches != 0;
		OutLayout.ArchSegments = TowerTuning.ArchSegments;
		OutLayout.ArchTubeDiameter = TowerTuning.ArchTubeDiameter;
		OutLayout.DefaultRoomRuleID = TowerTuning.DefaultRoomRuleID;
		OutLayout.StartRoomRuleID = TowerTuning.StartRoomRuleID;
		OutLayout.BossRoomRuleID = TowerTuning.BossRoomRuleID;
		OutLayout.StartFloorNumber = TowerTuning.StartFloorNumber;
		OutLayout.BossFloorNumber = bBossRushFinaleStage ? TowerTuning.BossRushBossFloorNumber : TowerTuning.BossFloorNumber;
		OutLayout.FirstMobFloorNumber = bBossRushFinaleStage ? OutLayout.BossFloorNumber : TowerTuning.FirstMobFloorNumber;
		OutLayout.LastMobFloorNumber = bBossRushFinaleStage ? OutLayout.StartFloorNumber : TowerTuning.LastMobFloorNumber;

		const float TopFloorZ = Preset.BaselineZ + 1600.0f;
		const float FloorSpacing = OutLayout.FloorSpacing;
		const FVector2D HoleHalfExtent(OutLayout.PlacementCellSize * 0.5f, OutLayout.PlacementCellSize * 0.5f);
		const FVector AlignedHoleOffset(0.0f, -OutLayout.PlacementCellSize, 0.0f);
		const float StartRoomHalfExtent = TowerTuning.StartRoomSquareSize * 0.5f;
		const int32 TotalFloorCount = bBossRushFinaleStage ? TowerTuning.GetBossRushTotalFloorCount() : TowerTuning.GetNormalTotalFloorCount();
		const float FloorBottomZ = TopFloorZ - (static_cast<float>(TotalFloorCount - 1) * FloorSpacing) - OutLayout.FloorThickness;

		OutLayout.TraceStartZ = TopFloorZ + 12000.0f;
		OutLayout.TraceEndZ = FloorBottomZ - 12000.0f;

		for (int32 FloorIndex = 0; FloorIndex < TotalFloorCount; ++FloorIndex)
		{
			FFloor& Floor = OutLayout.Floors.AddDefaulted_GetRef();
			Floor.FloorNumber = OutLayout.StartFloorNumber + FloorIndex;
			Floor.FloorRole =
				(Floor.FloorNumber == OutLayout.StartFloorNumber) ? ET66TowerFloorRole::Start :
				(Floor.FloorNumber == OutLayout.BossFloorNumber) ? ET66TowerFloorRole::Boss :
				ET66TowerFloorRole::Mob;
			Floor.bMobFloor = Floor.FloorRole == ET66TowerFloorRole::Mob;
			Floor.GameplayLevelNumber = Floor.bMobFloor
				? (Floor.FloorNumber - OutLayout.FirstMobFloorNumber + 1)
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
				: (Floor.PolygonApothem - OutLayout.PlacementCellSize);
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
			if (Floor.FloorNumber == OutLayout.BossFloorNumber && FloorIndex > 0)
			{
				Floor.Center.X = Floor.ArrivalPoint.X;
				Floor.Center.Y = Floor.ArrivalPoint.Y;
				Floor.HoleCenter = Floor.Center;
				Floor.HoleCenter.Z = Floor.SurfaceZ;
				Floor.ExitPoint = Floor.Center;
				Floor.ExitPoint.Z = Floor.SurfaceZ;
			}
			FRandomStream FloorMazeRng(T66BuildTowerFloorSeed(Preset.Seed, Floor.FloorNumber, Floor.GameplayLevelNumber, Floor.Theme));
			T66BuildFloorMazeWalls(OutLayout, Floor, FloorMazeRng);
			if (Floor.bMobFloor)
			{
				// Range contract (section 1.7): the room-course config asks for a COUNT
				// RANGE (more, smaller rooms with seeded variety), not an exact count.
				const bool bRoomLayoutPass = Floor.Rooms.Num() >= OutLayout.DungeonMinRooms
					&& Floor.Rooms.Num() <= OutLayout.DungeonMaxRooms;
				if (bRoomLayoutPass)
				{
					UE_LOG(
						LogT66TowerMapTerrain,
						Log,
						TEXT("[T66Proof][TowerRoomLayoutSummary] Floor=%d Result=PASS Rooms=%d Expected=%d Grid=%dx%d Tile=%.0f RoomTiles=%d-%d WalkableBoxes=%d MazeWalls=%d"),
						Floor.FloorNumber,
						Floor.Rooms.Num(),
						OutLayout.DungeonMinRooms,
						OutLayout.GridColumns,
						OutLayout.GridRows,
						OutLayout.GridCellSize,
						OutLayout.DungeonMinRoomTiles,
						OutLayout.DungeonMaxRoomTiles,
						Floor.WalkableFloorBoxes.Num(),
						Floor.MazeWallBoxes.Num());
				}
				else
				{
					UE_LOG(
						LogT66TowerMapTerrain,
						Warning,
						TEXT("[T66Proof][TowerRoomLayoutSummary] Floor=%d Result=FAIL Rooms=%d Expected=%d Grid=%dx%d Tile=%.0f RoomTiles=%d-%d WalkableBoxes=%d MazeWalls=%d"),
						Floor.FloorNumber,
						Floor.Rooms.Num(),
						OutLayout.DungeonMinRooms,
						OutLayout.GridColumns,
						OutLayout.GridRows,
						OutLayout.GridCellSize,
						OutLayout.DungeonMinRoomTiles,
						OutLayout.DungeonMaxRoomTiles,
						Floor.WalkableFloorBoxes.Num(),
						Floor.MazeWallBoxes.Num());
				}
			}
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
		TArray<const FFloor*> MobFloors;
		for (const FFloor& Floor : Layout.Floors)
		{
			if (Floor.bMobFloor)
			{
				MobFloors.Add(&Floor);
			}
		}

		if (MobFloors.Num() <= 0)
		{
			return false;
		}

		for (int32 Attempt = 0; Attempt < 36; ++Attempt)
		{
			const FFloor& Floor = *MobFloors[Rng.RandRange(0, MobFloors.Num() - 1)];
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
		auto CollectCandidateCenters = [&](const float TestEdgePadding, const float TestHolePadding, const float TestWallPadding)
		{
			const float BoundsPadding = Floor->WalkableFloorBoxes.Num() > 0 ? 0.0f : TestEdgePadding;
			auto AppendValidSlots = [&](const TArray<FVector>& Slots)
			{
				for (const FVector& Candidate : Slots)
				{
					if (T66IsWalkableTowerLocation(*Floor, Candidate, BoundsPadding, TestHolePadding, TestWallPadding))
					{
						CandidateCenters.Add(Candidate);
					}
				}
			};

			if (Floor->CachedContentSpawnSlots.Num() > 0)
			{
				AppendValidSlots(Floor->CachedContentSpawnSlots);
			}
			if (CandidateCenters.Num() <= 0 && Floor->CachedWalkableSpawnSlots.Num() > 0)
			{
				AppendValidSlots(Floor->CachedWalkableSpawnSlots);
			}
			if (CandidateCenters.Num() > 0 || Floor->CachedContentSpawnSlots.Num() > 0 || Floor->CachedWalkableSpawnSlots.Num() > 0)
			{
				return;
			}

			for (float TileMinY = PolygonMinY; TileMinY < PolygonMaxY - KINDA_SMALL_NUMBER; TileMinY += TileSize)
			{
				const float TileMaxY = FMath::Min(TileMinY + TileSize, PolygonMaxY);
				const float TileCenterY = (TileMinY + TileMaxY) * 0.5f;
				for (float TileMinX = PolygonMinX; TileMinX < PolygonMaxX - KINDA_SMALL_NUMBER; TileMinX += TileSize)
				{
					const float TileMaxX = FMath::Min(TileMinX + TileSize, PolygonMaxX);
					const float TileCenterX = (TileMinX + TileMaxX) * 0.5f;
					const FVector Candidate(TileCenterX, TileCenterY, Floor->SurfaceZ);
					if (!T66IsWalkableTowerLocation(*Floor, Candidate, BoundsPadding, TestHolePadding, TestWallPadding))
					{
						continue;
					}

					CandidateCenters.Add(Candidate);
				}
			}
		};

		CollectCandidateCenters(EffectiveEdgePadding, EffectiveHolePadding, EffectiveWallPadding);
		if (CandidateCenters.Num() <= 0)
		{
			CollectCandidateCenters(EffectiveEdgePadding * 0.65f, EffectiveHolePadding * 0.65f, EffectiveWallPadding * 0.65f);
		}
		if (CandidateCenters.Num() <= 0)
		{
			CollectCandidateCenters(FMath::Min(500.0f, EffectiveEdgePadding), FMath::Min(700.0f, EffectiveHolePadding), FMath::Min(250.0f, EffectiveWallPadding));
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

	bool TryGetRoomSurfaceLocation(
		UWorld* World,
		const FLayout& Layout,
		const FFloor& Floor,
		const FRoom& Room,
		FRandomStream& Rng,
		FVector& OutLocation,
		const float EdgePadding,
		const float HolePadding,
		const float WallPadding)
	{
		if (!World || Floor.FloorNumber != Room.FloorNumber || !Room.Bounds.bIsValid)
		{
			return false;
		}

		const float EffectiveEdgePadding = FMath::Max(0.0f, EdgePadding);
		const float EffectiveHolePadding = FMath::Max(0.0f, HolePadding);
		const float EffectiveWallPadding = FMath::Max(0.0f, WallPadding);
		const float TileSize = FMath::Max(600.0f, Layout.PlacementCellSize);
		const float CandidateMinX = Room.Bounds.Min.X + EffectiveEdgePadding;
		const float CandidateMaxX = Room.Bounds.Max.X - EffectiveEdgePadding;
		const float CandidateMinY = Room.Bounds.Min.Y + EffectiveEdgePadding;
		const float CandidateMaxY = Room.Bounds.Max.Y - EffectiveEdgePadding;

		TArray<FVector, TInlineAllocator<64>> CandidateCenters;
		auto AppendCandidate = [&](const FVector& Candidate)
		{
			if (!T66IsWalkableTowerLocation(Floor, Candidate, 0.0f, EffectiveHolePadding, EffectiveWallPadding))
			{
				return;
			}

			CandidateCenters.Add(Candidate);
		};

		if (CandidateMaxX > CandidateMinX && CandidateMaxY > CandidateMinY)
		{
			for (float TileMinY = CandidateMinY; TileMinY < CandidateMaxY - KINDA_SMALL_NUMBER; TileMinY += TileSize)
			{
				const float TileMaxY = FMath::Min(TileMinY + TileSize, CandidateMaxY);
				const float TileCenterY = (TileMinY + TileMaxY) * 0.5f;
				for (float TileMinX = CandidateMinX; TileMinX < CandidateMaxX - KINDA_SMALL_NUMBER; TileMinX += TileSize)
				{
					const float TileMaxX = FMath::Min(TileMinX + TileSize, CandidateMaxX);
					const float TileCenterX = (TileMinX + TileMaxX) * 0.5f;
					AppendCandidate(FVector(TileCenterX, TileCenterY, Floor.SurfaceZ));
				}
			}

			for (int32 Attempt = 0; Attempt < 12; ++Attempt)
			{
				AppendCandidate(FVector(
					Rng.FRandRange(CandidateMinX, CandidateMaxX),
					Rng.FRandRange(CandidateMinY, CandidateMaxY),
					Floor.SurfaceZ));
			}
		}

		if (CandidateCenters.Num() <= 0)
		{
			const FVector CenterCandidate(
				(Room.Bounds.Min.X + Room.Bounds.Max.X) * 0.5f,
				(Room.Bounds.Min.Y + Room.Bounds.Max.Y) * 0.5f,
				Floor.SurfaceZ);
			if (T66IsWalkableTowerLocation(Floor, CenterCandidate, 0.0f, FMath::Min(EffectiveHolePadding, 500.0f), FMath::Min(EffectiveWallPadding, 220.0f)))
			{
				CandidateCenters.Add(CenterCandidate);
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

	bool TryGetObstacleTrapSpawnLocation(
		UWorld* World,
		const FLayout& Layout,
		const int32 FloorNumber,
		FRandomStream& Rng,
		FVector& OutLocation,
		const float FootprintRadius,
		const float EdgePadding,
		const float HolePadding)
	{
		const float EffectiveFootprint = FMath::Max(0.0f, FootprintRadius);
		const float EffectiveEdgePadding = FMath::Max(EdgePadding, EffectiveFootprint * 1.10f);
		const float EffectiveHolePadding = FMath::Max(HolePadding, EffectiveFootprint * 1.20f);
		const float EffectiveWallPadding = FMath::Max(Layout.PlacementCellSize * 0.35f, EffectiveFootprint * 0.80f);

		// Gate clearance (2026-06-11): the hero LANDS at the arrival point after
		// dropping through the previous floor's gate — obstacles must not affect
		// that area (the hole side is already covered by HolePadding).
		const FFloor* Floor = nullptr;
		const bool bHasFloor = T66TryGetFloor(Layout, FloorNumber, Floor) && Floor;
		const float ArrivalClearance = FMath::Max(2000.0f, EffectiveFootprint + 900.0f);
		auto IsClearOfArrival = [&](const FVector& Candidate)
		{
			return !bHasFloor
				|| FVector::DistSquared2D(Candidate, Floor->ArrivalPoint) >= FMath::Square(ArrivalClearance);
		};

		for (int32 Attempt = 0; Attempt < 10; ++Attempt)
		{
			if (TryGetFloorTileCenterSpawnLocation(
				World,
				Layout,
				FloorNumber,
				Rng,
				OutLocation,
				EffectiveEdgePadding,
				EffectiveHolePadding,
				EffectiveWallPadding)
				&& IsClearOfArrival(OutLocation))
			{
				return true;
			}
		}

		for (int32 Attempt = 0; Attempt < 10; ++Attempt)
		{
			if (TryGetRandomSurfaceLocationOnFloor(
				World,
				Layout,
				FloorNumber,
				Rng,
				OutLocation,
				FMath::Min(EffectiveEdgePadding, 700.0f),
				FMath::Min(EffectiveHolePadding, 900.0f))
				&& IsClearOfArrival(OutLocation))
			{
				return true;
			}
		}

		return false;
	}

	bool IsPointInsideBounceObstacle(const FFloor& Floor, const FVector& Location, const float Padding)
	{
		return T66IsLocationInsideBounceObstacle(Floor, Location, Padding);
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
			const float WalkableWallPadding = 120.0f;
			const float SurfaceOffset = HalfThickness + WalkableWallPadding + 40.f;

			TArray<int32, TInlineAllocator<2>> ValidSideIndices;
			for (int32 SideIndex = 0; SideIndex < UE_ARRAY_COUNT(CandidateNormals); ++SideIndex)
			{
				const FVector Candidate = WallLinePoint + (CandidateNormals[SideIndex] * SurfaceOffset);
				if (T66IsWalkableTowerLocation(*Floor, Candidate, 200.0f, 450.0f, WalkableWallPadding))
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
			const float BoundsPadding = Floor->WalkableFloorBoxes.Num() > 0 ? 0.0f : EffectiveEdgePadding;
			const int32 StartIndex = Rng.RandRange(0, Floor->CachedWalkableSpawnSlots.Num() - 1);
			for (int32 Offset = 0; Offset < Floor->CachedWalkableSpawnSlots.Num(); ++Offset)
			{
				const FVector Candidate = Floor->CachedWalkableSpawnSlots[(StartIndex + Offset) % Floor->CachedWalkableSpawnSlots.Num()];
				if (!T66IsWalkableTowerLocation(*Floor, Candidate, BoundsPadding, EffectiveHolePadding, EffectiveEdgePadding))
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
			const float BoundsPadding = Floor->WalkableFloorBoxes.Num() > 0 ? 0.0f : EffectiveEdgePadding;
			const FVector Candidate(
				Floor->Center.X + Rng.FRandRange(-CandidateHalfExtent, CandidateHalfExtent),
				Floor->Center.Y + Rng.FRandRange(-CandidateHalfExtent, CandidateHalfExtent),
				Floor->SurfaceZ);
			if (!T66IsWalkableTowerLocation(*Floor, Candidate, BoundsPadding, EffectiveHolePadding, EffectiveEdgePadding))
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

		FT66TowerTerrainSpawnStats SpawnStats;
		FT66ScopedTowerTerrainSpawnStats ScopedSpawnStats(SpawnStats);
		const bool bLogTiming = CVarT66TowerTerrainTimingLogs.GetValueOnAnyThread() != 0;
		const double SpawnStartSeconds = FPlatformTime::Seconds();

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
			const double FloorStartSeconds = FPlatformTime::Seconds();
			const FT66TowerTerrainSpawnStats FloorStartStats = SpawnStats;
			FFloor Floor = Layout.Floors[FloorIndex];
			const T66TowerThemeVisuals::FResolvedTheme& Theme = FloorThemes[FloorIndex];
			const bool bUsingGeneratedDungeonKitForTheme =
				T66ShouldUseGeneratedDungeonKit()
				&& Theme.WallFamily == T66TowerThemeVisuals::EWallFamily::SplitCollisionVisual;
			const float ModuleWallHeight = bUsingGeneratedDungeonKitForTheme
				? FMath::Max(600.0f, Layout.FloorSpacing - Layout.FloorThickness)
				: FMath::Max(600.0f, T66ResolveFloorCeilingBottomZ(Layout, FloorThemes, FloorIndex) - Floor.SurfaceZ);
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
			T66SpawnTierTerrainForFloor(World, CubeMesh, Theme, Layout, Floor, SpawnParams);
			T66SpawnBounceCourseForFloor(World, CubeMesh, Theme, Layout, Floor, SpawnParams);

			if (bUsingGeneratedDungeonKitForTheme)
			{
				T66SpawnGeneratedDungeonFloorUndersideTiles(
					World,
					Theme,
					Layout,
					Floor,
					Floor.SurfaceZ + ModuleWallHeight,
					SpawnParams,
					FloorTags,
					false,
					false);
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
							T66TowerTerrainNoSurfaceBounceTag,
							FName(*FString::Printf(TEXT("T66_Floor_Tower_%02d"), Floor.FloorNumber)),
							FName(TEXT("T66_Floor_Tower_Roof")),
							FName(*FString::Printf(TEXT("T66_Floor_Tower_Roof_%02d"), Floor.FloorNumber))
						});
				}
			}

			if (bLogTiming)
			{
				UE_LOG(
					LogT66TowerMapTerrain,
					Log,
					TEXT("[MAP] Tower terrain floor %d spawned in %.1f ms (collision proxies +%d, instanced actors +%d, HISM components +%d, HISM instances +%d, maze walls=%d, walkable boxes=%d, generatedKit=%d)."),
					Floor.FloorNumber,
					(FPlatformTime::Seconds() - FloorStartSeconds) * 1000.0,
					SpawnStats.CollisionProxyActors - FloorStartStats.CollisionProxyActors,
					SpawnStats.InstancedVisualActors - FloorStartStats.InstancedVisualActors,
					SpawnStats.HISMComponents - FloorStartStats.HISMComponents,
					SpawnStats.HISMInstances - FloorStartStats.HISMInstances,
					Floor.MazeWallBoxes.Num(),
					Floor.WalkableFloorBoxes.Num(),
					bUsingGeneratedDungeonKitForTheme ? 1 : 0);
			}
		}

		if (bLogTiming)
		{
			UE_LOG(
				LogT66TowerMapTerrain,
				Log,
				TEXT("[MAP] Tower terrain spawned in %.1f ms across %d floors (collision proxies=%d, instanced actors=%d, HISM components=%d, HISM instances=%d, generatedKit=%d)."),
				(FPlatformTime::Seconds() - SpawnStartSeconds) * 1000.0,
				Layout.Floors.Num(),
				SpawnStats.CollisionProxyActors,
				SpawnStats.InstancedVisualActors,
				SpawnStats.HISMComponents,
				SpawnStats.HISMInstances,
				T66ShouldUseGeneratedDungeonKit() ? 1 : 0);
		}

		bOutCollisionReady = true;
		return true;
	}

	// -----------------------------------------------------------------------
	// Themed-surface parity (Test Room): the same Dungeon-theme visuals the
	// live maze renders, honoring the t66.Tower.*Baffles CVars, with the same
	// themed-cube fallback the maze uses when baffle assets are unavailable.
	// -----------------------------------------------------------------------

	static bool T66ResolveParityTheme(UWorld* World, T66TowerThemeVisuals::FResolvedTheme& OutTheme)
	{
		return T66TowerThemeVisuals::ResolveTheme(
			World,
			ET66TowerGameplayLevelTheme::Dungeon,
			/*bBossFloor*/ false,
			OutTheme);
	}

	static FActorSpawnParameters T66ParitySpawnParams()
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return SpawnParams;
	}

	bool SpawnThemedFloorVisual(UWorld* World, const FBox2D& Box, const float SurfaceZ, const TArray<FName>& Tags)
	{
		T66TowerThemeVisuals::FResolvedTheme Theme;
		if (!World || !T66ResolveParityTheme(World, Theme) || !Theme.FloorMaterial)
		{
			return false;
		}

		const FActorSpawnParameters SpawnParams = T66ParitySpawnParams();
		FFloor ParityFloor;
		ParityFloor.FloorNumber = 0;
		ParityFloor.SurfaceZ = SurfaceZ;
		ParityFloor.Center = FVector((Box.Min.X + Box.Max.X) * 0.5f, (Box.Min.Y + Box.Max.Y) * 0.5f, SurfaceZ);

		if (T66ShouldUseFloorBaffles()
			&& T66SpawnFloorBaffleTubeVisualsForBox(World, Box, ParityFloor, Theme.FloorMaterial, SpawnParams, Tags))
		{
			return true;
		}

		UStaticMesh* CubeMesh = FT66VisualUtil::GetBasicShapeCube();
		if (!CubeMesh)
		{
			return false;
		}
		// Same slab the maze falls back to (GeneratedDungeonKitFloorThickness).
		const float SlabThickness = 24.0f;
		const FVector2D Center = (Box.Min + Box.Max) * 0.5f;
		const FVector2D HalfExtents = (Box.Max - Box.Min) * 0.5f;
		return T66SpawnEnvironmentRectangle(
			World,
			CubeMesh,
			Theme.FloorMaterial,
			FVector(Center.X, Center.Y, SurfaceZ - (SlabThickness * 0.5f)),
			FVector(HalfExtents.X, HalfExtents.Y, SlabThickness * 0.5f),
			SpawnParams,
			Tags,
			/*bIgnoreCameraChannel*/ true) != nullptr;
	}

	bool SpawnThemedCeilingVisual(UWorld* World, const FBox2D& Box, const float CeilingBottomZ, const TArray<FName>& Tags)
	{
		T66TowerThemeVisuals::FResolvedTheme Theme;
		if (!World || !T66ResolveParityTheme(World, Theme))
		{
			return false;
		}
		UMaterialInterface* CeilingMaterial = Theme.CeilingMaterial ? Theme.CeilingMaterial : Theme.RoofMaterial;
		if (!CeilingMaterial)
		{
			return false;
		}

		const FActorSpawnParameters SpawnParams = T66ParitySpawnParams();
		FFloor ParityFloor;
		ParityFloor.FloorNumber = 0;
		ParityFloor.SurfaceZ = 0.0f;
		ParityFloor.Center = FVector((Box.Min.X + Box.Max.X) * 0.5f, (Box.Min.Y + Box.Max.Y) * 0.5f, 0.0f);

		if (T66ShouldUseCeilingBaffles()
			&& T66SpawnCeilingBaffleTubeVisualsForBox(World, Box, ParityFloor, CeilingBottomZ, CeilingMaterial, SpawnParams, Tags))
		{
			return true;
		}

		UStaticMesh* CubeMesh = FT66VisualUtil::GetBasicShapeCube();
		if (!CubeMesh)
		{
			return false;
		}
		const float SlabThickness = 24.0f;
		const FVector2D Center = (Box.Min + Box.Max) * 0.5f;
		const FVector2D HalfExtents = (Box.Max - Box.Min) * 0.5f;
		return T66SpawnEnvironmentRectangle(
			World,
			CubeMesh,
			CeilingMaterial,
			FVector(Center.X, Center.Y, CeilingBottomZ + (SlabThickness * 0.5f)),
			FVector(HalfExtents.X, HalfExtents.Y, SlabThickness * 0.5f),
			SpawnParams,
			Tags,
			/*bIgnoreCameraChannel*/ true) != nullptr;
	}

	bool SpawnThemedWallVisual(UWorld* World, const FBox2D& WallBox, const float BaseZ, const float Height, const TArray<FName>& Tags)
	{
		T66TowerThemeVisuals::FResolvedTheme Theme;
		if (!World || Height <= 10.0f || !T66ResolveParityTheme(World, Theme))
		{
			return false;
		}
		UMaterialInterface* WallMaterial = T66ResolveEnvironmentWallMaterialForBox(Theme, WallBox);
		if (!WallMaterial)
		{
			WallMaterial = Theme.WallMaterial;
		}
		if (!WallMaterial)
		{
			return false;
		}

		const FActorSpawnParameters SpawnParams = T66ParitySpawnParams();
		if (T66ShouldUseWallBaffles()
			&& T66SpawnWallBaffleTubeVisualsForBox(World, WallBox, BaseZ, Height, WallMaterial, SpawnParams, Tags, /*bIgnoreCameraChannel*/ true))
		{
			return true;
		}

		UStaticMesh* CubeMesh = FT66VisualUtil::GetBasicShapeCube();
		if (!CubeMesh)
		{
			return false;
		}
		const FVector2D Center = (WallBox.Min + WallBox.Max) * 0.5f;
		const FVector2D HalfExtents = (WallBox.Max - WallBox.Min) * 0.5f;
		return T66SpawnEnvironmentRectangle(
			World,
			CubeMesh,
			WallMaterial,
			FVector(Center.X, Center.Y, BaseZ + (Height * 0.5f)),
			FVector(HalfExtents.X, HalfExtents.Y, Height * 0.5f),
			SpawnParams,
			Tags,
			/*bIgnoreCameraChannel*/ true) != nullptr;
	}
}
