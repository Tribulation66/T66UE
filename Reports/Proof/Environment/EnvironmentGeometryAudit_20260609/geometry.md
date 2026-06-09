# Environment Geometry Audit - Dungeon Floor, Walls, Ceiling

Date: 2026-06-09  
Scope: Read-only evidence pass for current UE5.7 tower/dungeon room surfaces. No source, content, config, or asset changes were made.

## Summary

- Current tower/dungeon room surfaces are spawned at runtime by `T66TowerMapTerrain::Spawn(...)` from precomputed `T66TowerMapTerrain::FLayout` / `FFloor` records.
- Floor, wall, and ceiling/roof visuals are currently box-based `AStaticMeshActor` / `UStaticMeshComponent` geometry using `/Engine/BasicShapes/Cube.Cube`, scaled to generated `FBox2D` extents. The active path is not `ProceduralMeshComponent`, `DynamicMesh`, Geometry Script, or authored per-tile modular mesh placement.
- With `T66.Tower.UseGeneratedDungeonKit=1`, visual cubes are paired with hidden `UBoxComponent` collision proxies for floor/wall/ceiling collision. The "generated kit" naming currently means split visual/collision box placement in the live code, not active floor/ceiling static-mesh module tiling.
- Generated kit mesh asset families exist in `Content/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01`, and HISM helper code exists, but the audited active floor/ceiling path does not consume `Theme.FloorMeshes`; the wall HISM batch path is present but the active wall visual function spawns cube rectangles and casts `WallBatch` unused.

## 1. Geometry

### Common Representation

The terrain owner imports static mesh actors, static mesh components, box components, and HISM types, but the common primitive selected for the active runtime surfaces is the engine cube:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:11-18
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "Components/BoxComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
```

```cpp
// Source/T66/Gameplay/T66VisualUtil.cpp:121-128
UStaticMesh* FT66VisualUtil::GetBasicShapeCube()
{
	static TObjectPtr<UStaticMesh> Cached = nullptr;
	if (!Cached)
	{
		Cached = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	}
	return Cached.Get();
}
```

All stretched cube actors are sized through mesh bounds and requested half-extents:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:101-112
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
```

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:201-218
if (UStaticMeshComponent* MeshComponent = Actor->GetStaticMeshComponent())
{
	const bool bCameraOccludingWallVisual = T66IsCameraOccludingTowerWallVisual(ExtraTags);
	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetStaticMesh(Mesh);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetRelativeScale3D(T66ComputeMeshScaleForHalfExtents(Mesh, DesiredHalfExtents));
	...
	MeshComponent->SetCollisionEnabled(bEnableCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
```

Hidden collision proxies are separate actors with `UBoxComponent`, not mesh collision:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:277-300
UBoxComponent* CollisionComponent = NewObject<UBoxComponent>(Actor, TEXT("CollisionProxy"));
...
CollisionComponent->SetBoxExtent(DesiredHalfExtents, false);
CollisionComponent->SetGenerateOverlapEvents(false);
CollisionComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
T66ConfigureTowerCollisionResponses(CollisionComponent, bIgnoreCameraChannel);
CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
...
Actor->SetActorHiddenInGame(true);
Actor->Tags.AddUnique(T66TowerMapTerrainCollisionProxyTag);
```

### Floor

Current floor geometry is generated per walkable rectangle/box. The generated-kit path still uses cube rectangles for visuals, plus hidden box collision proxies:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4026-4044
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
```

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4066-4075
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
```

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4096-4104
const FVector2D Center = (SourceBox.Min + SourceBox.Max) * 0.5f;
T66SpawnHiddenCollisionProxyActor(
	World,
	FVector(Center.X, Center.Y, Floor.SurfaceZ - (FloorThickness * 0.5f)),
	FRotator::ZeroRotator,
	FVector(BoxSize.X * 0.5f, BoxSize.Y * 0.5f, FloorThickness * 0.5f),
	SpawnParams,
	CollisionTags,
	true);
```

Drop holes are represented by splitting the affected floor box into up to four rectangles:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4131-4143
// FIRST-PASS: multi-rectangle drop-hole floors may show edge seams; revisit with masked-material or procedural-cutout solution if seams read badly.
SpawnSurfaceBox(FBox2D(FVector2D(SourceBox.Min.X, SourceBox.Min.Y), FVector2D(HoleMinX, SourceBox.Max.Y)));
SpawnSurfaceBox(FBox2D(FVector2D(HoleMaxX, SourceBox.Min.Y), FVector2D(SourceBox.Max.X, SourceBox.Max.Y)));
SpawnSurfaceBox(FBox2D(FVector2D(HoleMinX, SourceBox.Min.Y), FVector2D(HoleMaxX, HoleMinY)));
SpawnSurfaceBox(FBox2D(FVector2D(HoleMinX, HoleMaxY), FVector2D(HoleMaxX, SourceBox.Max.Y)));
...
for (const FBox2D& WalkableBox : Floor.WalkableFloorBoxes)
{
	SpawnBoxWithHole(WalkableBox);
}
```

Fallback floor geometry when generated-kit mode does not produce a floor is also stretched cube slabs:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:3893-3925
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
	...
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
```

### Walls

Shell walls and maze walls are generated as `FBox2D` spans and passed through `T66SpawnThemedWallBox(...)`.

The shell wall builder derives four perimeter boxes from `ShellRadius` and `WallThickness`:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4637-4671
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
	...
	const FBox2D EastShellBox(FVector2D(Layout.ShellRadius - WallHalfDepth, -WallHalfSpan), FVector2D(Layout.ShellRadius + WallHalfDepth, WallHalfSpan));
	const FBox2D WestShellBox(FVector2D(-Layout.ShellRadius - WallHalfDepth, -WallHalfSpan), FVector2D(-Layout.ShellRadius + WallHalfDepth, WallHalfSpan));
	const FBox2D NorthShellBox(FVector2D(-WallHalfSpan, Layout.ShellRadius - WallHalfDepth), FVector2D(WallHalfSpan, Layout.ShellRadius + WallHalfDepth));
	const FBox2D SouthShellBox(FVector2D(-WallHalfSpan, -Layout.ShellRadius - WallHalfDepth), FVector2D(WallHalfSpan, -Layout.ShellRadius + WallHalfDepth));
	T66AppendShellWallBoxSegments(Floor.OuterShellWallBoxes, EastShellBox, Theme, bBatchGeneratedWalls);
```

Maze wall boxes are iterated directly:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4751-4802
static void T66SpawnMazeWalls(
	UWorld* World,
	UStaticMesh* CubeMesh,
	const T66TowerThemeVisuals::FResolvedTheme& Theme,
	const T66TowerMapTerrain::FLayout& Layout,
	const T66TowerMapTerrain::FFloor& Floor,
	const float WallHeight,
	const FActorSpawnParameters& SpawnParams)
{
	...
	for (const FBox2D& WallBox : Floor.MazeWallBoxes)
	{
		const FVector2D WallCenter = (WallBox.Min + WallBox.Max) * 0.5f;
		const FVector2D WallHalfExtents = (WallBox.Max - WallBox.Min) * 0.5f;
		...
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
```

For `EWallFamily::SplitCollisionVisual`, the current visual function spawns a cube rectangle and a hidden collision proxy:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:713-743
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

	UStaticMesh* CubeMesh = FT66VisualUtil::GetBasicShapeCube();
	UMaterialInterface* WallMaterial = T66ResolveEnvironmentWallMaterialForBox(Theme, WallBox);
```

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:757-785
AStaticMeshActor* WallVisual = T66SpawnEnvironmentRectangle(
	World,
	CubeMesh,
	WallMaterial,
	FVector(WallCenter.X, WallCenter.Y, BaseZ + (DesiredHeight * 0.5f)),
	FVector(WallSize.X * 0.5f, WallSize.Y * 0.5f, DesiredHeight * 0.5f),
	SpawnParams,
	VisualTags,
	bIgnoreCameraChannel);
...
T66SpawnHiddenCollisionProxyActor(
	World,
	FVector(WallCenter.X, WallCenter.Y, BaseZ + (DesiredHeight * 0.5f)),
	FRotator::ZeroRotator,
	FVector(WallSize.X * 0.5f, WallSize.Y * 0.5f, DesiredHeight * 0.5f),
	SpawnParams,
	CollisionTags,
	bIgnoreCameraChannel);
```

The older HISM/module support exists, but the active wall visual function above does not add instances to the batch:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:492-521
struct FT66GeneratedDungeonWallBatch
{
	TArray<TArray<FTransform>> InstanceTransformsByMesh;
	int32 InstanceCount = 0;
	...
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
```

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:524-640
static AActor* T66SpawnGeneratedDungeonInstancedMeshActor(
	UWorld* World,
	const TArray<UStaticMesh*>& Meshes,
	const TArray<TArray<FTransform>>& InstanceTransformsByMesh,
	...
	UHierarchicalInstancedStaticMeshComponent* MeshComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(
		Actor,
		MakeUniqueObjectName(Actor, UHierarchicalInstancedStaticMeshComponent::StaticClass(), FName(*ComponentName)));
	...
	MeshComponent->SetStaticMesh(Mesh);
	...
	MeshComponent->AddInstances(InstanceTransformsByMesh[MeshIndex], false, false, false);
```

### Ceiling / Roof

There are two ceiling-like surfaces:

- `T66SpawnGeneratedDungeonFloorUndersideTiles(...)` creates underside/ceiling visuals at `Floor.SurfaceZ + ModuleWallHeight` in generated-kit mode.
- `T66SpawnFloorRoofSurface(...)` creates roof/ceiling surfaces through `T66SpawnPolygonSurface(...)` in fallback/non-generated-kit mode.

Current generated underside/ceiling visuals are cube rectangles with ceiling tags and no surface bounce:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4154-4192
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
	...
	UMaterialInterface* CeilingMaterial = Theme.CeilingMaterial ? Theme.CeilingMaterial : Theme.RoofMaterial;
	...
	UStaticMesh* CubeMesh = FT66VisualUtil::GetBasicShapeCube();
	...
	CeilingTags.AddUnique(T66TowerMapCeilingTag);
	CeilingTags.AddUnique(T66TowerTerrainNoSurfaceBounceTag);
	...
	const float CeilingThickness = Layout.FloorThickness;
```

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4203-4212
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
```

Optional ceiling collision is a hidden box proxy:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4232-4249
TArray<FName> CollisionTags = Tags;
CollisionTags.AddUnique(T66TowerMapTraversalBarrierTag);
CollisionTags.AddUnique(T66TowerMapCeilingTag);
CollisionTags.AddUnique(T66TowerTerrainNoSurfaceBounceTag);
...
T66SpawnHiddenCollisionProxyActor(
	World,
	FVector(Center.X, Center.Y, CeilingBottomZ + (CeilingThickness * 0.5f)),
	FRotator::ZeroRotator,
	FVector(BoxSize.X * 0.5f, BoxSize.Y * 0.5f, CeilingThickness * 0.5f),
	SpawnParams,
	CollisionTags,
	false);
```

Fallback/non-generated roof surface generation reuses the polygon surface slab path:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4413-4449
static bool T66BuildFloorRoofSurface(
	const T66TowerMapTerrain::FLayout& Layout,
	const TArray<T66TowerThemeVisuals::FResolvedTheme>& FloorThemes,
	const int32 FloorIndex,
	T66TowerMapTerrain::FFloor& OutRoofGeometryFloor,
	float& OutRoofSurfaceZ,
	float& OutRoofThickness,
	bool& bOutEnableCollision)
{
	...
	if (FloorIndex == 0)
	{
		OutRoofGeometryFloor = Floor;
		OutRoofGeometryFloor.bHasDropHole = false;
		...
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
```

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4452-4479
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
	...
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
```

## 2. Generation Rules

### Layout Records

The generator stores rooms, grids, walkable surface boxes, wall boxes, and floor identity on `FFloor`:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.h:106-139
struct FFloor
{
	int32 FloorNumber = 0;
	ET66TowerFloorRole FloorRole = ET66TowerFloorRole::Mob;
	int32 GameplayLevelNumber = INDEX_NONE;
	ET66TowerGameplayLevelTheme Theme = ET66TowerGameplayLevelTheme::Dungeon;
	bool bMobFloor = false;
	bool bHasDropHole = false;
	FVector Center = FVector::ZeroVector;
	float SurfaceZ = 0.0f;
	float WalkableHalfExtent = 9000.0f;
	float PolygonApothem = 9000.0f;
	float BoundsHalfExtent = 9000.0f;
	...
	TArray<FGridCell> GridCells;
	TArray<FIntPoint> MainPathCells;
	TArray<FIntPoint> OptionalCells;
	TArray<FBox2D> WalkableFloorBoxes;
	TArray<FBox2D> MazeWallBoxes;
	TArray<FBox2D> DoorwayHeaderBoxes;
	TArray<FBox2D> TrapEligibleWallBoxes;
	TArray<FBox2D> OuterShellWallBoxes;
	...
	TArray<FRoom> Rooms;
	FName FloorTag = NAME_None;
};
```

`FLayout` owns the global tower scale, grid scale, floor stacking, and room tuning knobs:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.h:155-185
struct FLayout
{
	FT66MapPreset Preset;
	TArray<FFloor> Floors;
	float PlacementCellSize = 1800.0f;
	float FloorThickness = 320.0f;
	float FloorSpacing = 2600.0f;
	float WallThickness = 320.0f;
	float ShellRadius = 12000.0f;
	ET66TowerMazeMode MazeMode = ET66TowerMazeMode::LegacyLanes;
	int32 GridColumns = 6;
	int32 GridRows = 6;
	float GridCellSize = 6500.0f;
	float GridDoorWidth = 2600.0f;
	float TraceStartZ = 16000.0f;
	float TraceEndZ = -32000.0f;
	...
	float RoofSkinThickness = 12.0f;
	float StartFloorHeadroom = 2000.0f;
```

### Current Tuned Values

The live config currently sets 1000 cm layout/grid cells, 70x70 grid, 120 cm wall depth, 24 cm generated-kit floor thickness, 1200 cm generated-kit wall height, and 20,000 cm shell radius:

```ini
; Config/DefaultT66TowerTuning.ini:5-23
[/Script/T66.T66TowerTuningConfig]
RoofSkinThickness=12.0
StartFloorHeadroom=2000.0
StandardFloorHeadroom=1200.0
PlacementCellSize=1000.0
DungeonKitWallDepth=120.0
GeneratedDungeonKitWallHeight=1200.0
GeneratedDungeonKitFloorThickness=24.0
GeneratedDungeonKitCullDistance=30000
ShellRadius=20000.0
StartRoomSquareSize=6500.0
GridColumns=70
GridRows=70
GridCellSize=1000.0
GridDoorWidth=1000.0
DungeonMinRooms=10
DungeonMaxRooms=10
DungeonMinRoomTiles=10
DungeonMaxRoomTiles=20
```

These values are loaded into `FLayout` during `BuildLayout(...)`:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4946-4976
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
	...
	OutLayout.WallThickness = TowerTuning.DungeonKitWallDepth;
	OutLayout.ShellRadius = TowerTuning.ShellRadius;
	...
	OutLayout.GridColumns = TowerTuning.GridColumns;
	OutLayout.GridRows = TowerTuning.GridRows;
	OutLayout.GridCellSize = TowerTuning.GridCellSize;
	OutLayout.GridDoorWidth = TowerTuning.GridDoorWidth;
	OutLayout.DungeonMinRooms = TowerTuning.DungeonMinRooms;
	OutLayout.DungeonMaxRooms = TowerTuning.DungeonMaxRooms;
	OutLayout.DungeonMinRoomTiles = TowerTuning.DungeonMinRoomTiles;
	OutLayout.DungeonMaxRoomTiles = TowerTuning.DungeonMaxRoomTiles;
```

### Floor Stacking / Elevation

Floors are stacked by `FloorSpacing`. In generated-kit mode, `FloorSpacing = 1200 + 24 = 1224` cm from current config values. `SurfaceZ` is the floor center Z for each floor; drop holes are one placement cell square.

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4989-5037
const float TopFloorZ = Preset.BaselineZ + 1600.0f;
const float FloorSpacing = OutLayout.FloorSpacing;
const FVector2D HoleHalfExtent(OutLayout.PlacementCellSize * 0.5f, OutLayout.PlacementCellSize * 0.5f);
const FVector AlignedHoleOffset(0.0f, -OutLayout.PlacementCellSize, 0.0f);
...
for (int32 FloorIndex = 0; FloorIndex < TotalFloorCount; ++FloorIndex)
{
	FFloor& Floor = OutLayout.Floors.AddDefaulted_GetRef();
	Floor.FloorNumber = OutLayout.StartFloorNumber + FloorIndex;
	...
	Floor.Center = FVector(0.0f, 0.0f, TopFloorZ - (static_cast<float>(FloorIndex) * FloorSpacing));
	Floor.SurfaceZ = Floor.Center.Z;
	Floor.PolygonApothem = OutLayout.ShellRadius - (OutLayout.WallThickness * 0.5f) + 20.0f;
	...
	if (Floor.bHasDropHole)
	{
		const FVector HoleOffset = AlignedHoleOffset;
		Floor.HoleCenter = Floor.Center + HoleOffset;
		Floor.HoleCenter.Z = Floor.SurfaceZ;
		Floor.HoleHalfExtent = HoleHalfExtent;
	}
```

### Room / Grid Generation

The active mob-floor dungeon generator is grid/room graph based. Note: the prompt assumed a Voronoi fill; the live tower generator uses no Voronoi fill in current code. A scoped source search for `Voronoi` under `Source`, `Config`, `Gameplay`, and `Content` returned no current tower/map-gen hits; the only hits were combat VFX documentation.

The grid cells are axis-aligned `GridCellSize` boxes:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:3288-3303
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
```

Room sizes and placement are cell-based:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:2792-2832
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
	...
	for (int32 Attempt = 0; Attempt < 420 && OutRooms.Num() < TargetRoomCount; ++Attempt)
	{
		FT66DungeonRoom Candidate;
		if (T66TryFindScatteredDungeonRoom(Layout, OutRooms, Rng, 1, Candidate))
		{
			OutRooms.Add(Candidate);
		}
	}
```

Room graph edges are selected by Manhattan distance plus random jitter, then extra edges are optionally selected:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:2835-2934
static bool T66BuildDungeonRoomGraph(
	const TArray<FT66DungeonRoom>& Rooms,
	FRandomStream& Rng,
	TArray<FT66DungeonGraphEdge>& OutEdges,
	TArray<int32>& OutDegree)
{
	...
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
	...
	FT66DungeonGraphEdge& ChosenEdge = OutEdges[BestEdgeIndex];
	ChosenEdge.bSelected = true;
	...
	const int32 ExtraBudget = FMath::Clamp(RoomCount / 4, 2, 6);
	...
	if (Rng.FRand() > 0.38f)
	{
		continue;
	}

	Edge.bSelected = true;
```

Selected graph edges carve corridors:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:3350-3364
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
```

Walkable floor geometry is not one authored tile per grid cell; the builder compresses horizontal runs of non-solid cells into `WalkableFloorBoxes`:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:3425-3445
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
```

Maze wall geometry is emitted as `FBox2D` rectangles around solid/open cell edges and interior templates:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:3483-3517
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
		...
		if (bNeighborWalkable)
		{
			...
			if (T66IsDungeonDoorwayEdge(CellIndex, NeighborIndex, Tiles, RoomIdByTile))
			{
				T66EmitDungeonTileEdgeWall(Layout, Floor, Cell, DirectionIndex, true, HalfThickness);
			}
			continue;
		}

		T66EmitDungeonTileEdgeWall(Layout, Floor, Cell, DirectionIndex, false, HalfThickness);
	}
}
```

## 3. Tiling / Size

### Floor and Ceiling Piece Size

- Floor and ceiling pieces are sized from `FBox2D` surface boxes and `Layout.FloorThickness`.
- Current generated-kit floor thickness is 24 cm.
- The floor surface is not placed as a uniform grid of authored static mesh modules; it is one stretched cube rectangle per `WalkableFloorBoxes` run, split further around drop holes.
- The ceiling/underside path mirrors the same source boxes and thickness.

### Wall Piece Size

- Shell walls are derived from `ShellRadius` and `WallThickness`.
- Maze walls are derived from cell side boxes and interior template boxes.
- Current generated-kit wall height is `Layout.FloorSpacing - Layout.FloorThickness`; with current config this is 1200 cm.
- `T66TowerDungeonKitUnitSize` is 1300 cm and wall segment cvar defaults to two units, 2600 cm, but that cvar only appears in live code for wall segment planning:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:38-63
static constexpr int32 T66TowerFloorVertexCount = 4;
static constexpr float T66TowerDungeonKitUnitSize = 1300.0f;
...
static TAutoConsoleVariable<int32> CVarT66TowerUseGeneratedDungeonKit(
	TEXT("T66.Tower.UseGeneratedDungeonKit"),
	1,
	TEXT("0 uses legacy material-only dungeon wall cubes, 1 uses generated tower theme visuals with lightweight collision proxies."),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarT66TowerGeneratedKitWallVisualSegmentLength(
	TEXT("T66.Tower.GeneratedKitWallVisualSegmentLength"),
	T66TowerDungeonKitUnitSize * 2.0f,
	TEXT("Target world-space length for generated wall visual meshes. Larger values reduce high-poly HISM instances; collision proxies stay layout-authored."),
	ECVF_Default);
```

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:145-150
static float T66GetGeneratedKitWallVisualTargetSegmentLength()
{
	const UT66TowerTuningConfig& TowerTuning = T66GetTowerTuning();
	const float RequestedLength = CVarT66TowerGeneratedKitWallVisualSegmentLength.GetValueOnAnyThread();
	return FMath::Clamp(RequestedLength, TowerTuning.PlacementCellSize, TowerTuning.PlacementCellSize * 6.0f);
}
```

No live code hit was found for `T66.Tower.GeneratedKitFloorVisualTileSize`; that name appears in `Gameplay/World/MODULAR_DUNGEON_KIT_INSTRUCTIONS.md`, not in the audited C++/config path.

### UV / Texture Tiling

- The terrain generator does not author UVs, create procedural mesh UVs, or set per-surface UV/tiling scalar parameters in `T66TowerMapTerrain.cpp`.
- Visuals are scaled cube meshes with whole material assignment. Texture repetition is material-asset behavior, not generator-authored per-room or per-tile UV layout.
- String scan of the dungeon material assets found `bUseWorldSpaceUVs` and `WorldSpaceTileSize` in the material instance data; that supports material-owned world-space tiling rather than geometry-authored UV subdivision.

Material asset string refs:

- `Content/ToonStyle/Environment/Dungeon/Materials/MI_Dungeon_Floor.uasset`
- `Content/ToonStyle/Environment/Dungeon/Materials/MI_Dungeon_Wall_XZ.uasset`
- `Content/ToonStyle/Environment/Dungeon/Materials/MI_Dungeon_Wall_YZ.uasset`
- `Content/ToonStyle/Environment/Dungeon/Materials/MI_Dungeon_Ceiling.uasset`

## 4. Materials

The theme resolver assigns floor/wall/ceiling/roof materials by surface type:

```cpp
// Source/T66/Gameplay/T66TowerThemeVisuals.h:29-45
struct FResolvedTheme
{
	FName ThemeName = NAME_None;
	UMaterialInterface* FloorMaterial = nullptr;
	UMaterialInterface* WallMaterial = nullptr;
	UMaterialInterface* RoofMaterial = nullptr;
	UMaterialInterface* WallXZMaterial = nullptr;
	UMaterialInterface* WallYZMaterial = nullptr;
	UMaterialInterface* CeilingMaterial = nullptr;
	...
	TArray<UStaticMesh*> WallMeshes;
	TArray<UStaticMesh*> FloorMeshes;
	...
	EWallFamily WallFamily = EWallFamily::SolidMaterial;
	float CeilingOffset = 1600.0f;
	bool bBossFloor = false;
};
```

The active material path format for dungeon surfaces is:

```cpp
// Source/T66/Gameplay/T66TowerThemeVisuals.cpp:258-270
UMaterialInterface* T66TowerThemeVisuals::ResolveEnvironmentSurfaceMaterial(
	UObject* Outer,
	const T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme,
	const EEnvironmentSurfaceType Surface)
{
	const FString MaterialPath = FString::Printf(
		TEXT("/Game/ToonStyle/Environment/%s/Materials/MI_%s_%s.MI_%s_%s"),
		T66ThemeNameForPath(Theme),
		T66ThemeNameForPath(Theme),
		T66SurfaceNameForPath(Surface),
		T66ThemeNameForPath(Theme),
		T66SurfaceNameForPath(Surface));
	return T66LoadThemeMaterial(Outer, *MaterialPath);
}
```

For `Dungeon`, `T66ThemeNameForPath(...)` returns `"Dungeon"` and surface names are `Floor`, `Wall_XZ`, `Wall_YZ`, and `Ceiling`:

```cpp
// Source/T66/Gameplay/T66TowerThemeVisuals.cpp:112-125
static const TCHAR* T66SurfaceNameForPath(const T66TowerThemeVisuals::EEnvironmentSurfaceType Surface)
{
	switch (Surface)
	{
	case T66TowerThemeVisuals::EEnvironmentSurfaceType::WallYZ:
		return TEXT("Wall_YZ");
	case T66TowerThemeVisuals::EEnvironmentSurfaceType::Floor:
		return TEXT("Floor");
	case T66TowerThemeVisuals::EEnvironmentSurfaceType::Ceiling:
		return TEXT("Ceiling");
	case T66TowerThemeVisuals::EEnvironmentSurfaceType::WallXZ:
	default:
		return TEXT("Wall_XZ");
	}
}
```

```cpp
// Source/T66/Gameplay/T66TowerThemeVisuals.cpp:223-239
bool T66TowerThemeVisuals::ResolveTheme(
	UObject* Outer,
	const T66TowerMapTerrain::ET66TowerGameplayLevelTheme ThemeId,
	const bool bBossFloor,
	FResolvedTheme& OutTheme)
{
	OutTheme = FResolvedTheme{};
	OutTheme.bBossFloor = bBossFloor;
	OutTheme.ThemeName = T66ThemeDisplayName(ThemeId);

	OutTheme.WallFamily = EWallFamily::SplitCollisionVisual;
	OutTheme.WallXZMaterial = ResolveEnvironmentSurfaceMaterial(Outer, ThemeId, EEnvironmentSurfaceType::WallXZ);
	OutTheme.WallYZMaterial = ResolveEnvironmentSurfaceMaterial(Outer, ThemeId, EEnvironmentSurfaceType::WallYZ);
	OutTheme.FloorMaterial = ResolveEnvironmentSurfaceMaterial(Outer, ThemeId, EEnvironmentSurfaceType::Floor);
	OutTheme.CeilingMaterial = ResolveEnvironmentSurfaceMaterial(Outer, ThemeId, EEnvironmentSurfaceType::Ceiling);
	OutTheme.WallMaterial = OutTheme.WallXZMaterial ? OutTheme.WallXZMaterial : OutTheme.WallYZMaterial;
	OutTheme.RoofMaterial = OutTheme.CeilingMaterial ? OutTheme.CeilingMaterial : OutTheme.FloorMaterial;
```

Material assignment is done by setting every mesh material slot to the resolved material:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:229-240
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
```

Current dungeon material asset refs found in `Content`:

- `/Game/ToonStyle/Environment/Dungeon/Materials/MI_Dungeon_Floor.MI_Dungeon_Floor`
- `/Game/ToonStyle/Environment/Dungeon/Materials/MI_Dungeon_Wall_XZ.MI_Dungeon_Wall_XZ`
- `/Game/ToonStyle/Environment/Dungeon/Materials/MI_Dungeon_Wall_YZ.MI_Dungeon_Wall_YZ`
- `/Game/ToonStyle/Environment/Dungeon/Materials/MI_Dungeon_Ceiling.MI_Dungeon_Ceiling`
- `/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof.MI_TowerDungeonRoof` and `/Game/World/Terrain/TowerDungeon/T_TowerDungeonRoof.T_TowerDungeonRoof` are still preloaded/fallback-related tower dungeon roof refs:

```cpp
// Source/T66/Core/T66GameInstance.cpp:2095-2100
AddPath(FSoftObjectPath(TEXT("/Game/Materials/M_Environment_Lit.M_Environment_Lit")));
AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof.MI_TowerDungeonRoof")));
AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/TowerDungeon/T_TowerDungeonRoof.T_TowerDungeonRoof")));
AddCoherentThemeKitAssets();
AddPath(FSoftObjectPath(TEXT("/Engine/BasicShapes/Plane.Plane")));
AddAllTowerThemeAssets();
```

Generated kit asset refs also exist and are preloaded:

```cpp
// Source/T66/Core/T66GameInstance.cpp:1979-1989
auto AddCoherentThemeKitAssets = [&AddPath]()
{
	const TCHAR* ModuleIds[] =
	{
		TEXT("DungeonWall_TorchSconce_A"),
		TEXT("DungeonWall_StoneBlocks_A"),
		TEXT("DungeonWall_Chains_A"),
		TEXT("DungeonWall_BonesNiche_A"),
		TEXT("DungeonFloor_StoneSlabs_A"),
		TEXT("DungeonFloor_Drain_A"),
		TEXT("DungeonFloor_Cracked_A"),
		TEXT("DungeonFloor_Bones_A"),
```

```cpp
// Source/T66/Core/T66GameInstance.cpp:2024-2030
for (const TCHAR* ModuleId : ModuleIds)
{
	const FString MeshPath = FString::Printf(
		TEXT("/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/%s_UnrealReady.%s_UnrealReady"),
		ModuleId,
		ModuleId);
	AddPath(FSoftObjectPath(MeshPath));
}
```

Matching asset files include:

- `Content/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonWall_TorchSconce_A_UnrealReady.uasset`
- `Content/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonWall_StoneBlocks_A_UnrealReady.uasset`
- `Content/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonWall_Chains_A_UnrealReady.uasset`
- `Content/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonWall_BonesNiche_A_UnrealReady.uasset`
- `Content/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonFloor_StoneSlabs_A_UnrealReady.uasset`
- `Content/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonFloor_Drain_A_UnrealReady.uasset`
- `Content/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonFloor_Cracked_A_UnrealReady.uasset`
- `Content/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonFloor_Bones_A_UnrealReady.uasset`

## 5. Insertion Point For Ribbed / Inflated Baffle Geometry

Geometry is not baked into a map asset. The procedural layout emits boxes, and the single render/spawn pass chooses how those boxes become visible/collidable surfaces.

The central dispatch point is `T66TowerMapTerrain::Spawn(...)`:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:5831-5899
bool Spawn(UWorld* World, const FLayout& Layout, ET66Difficulty Difficulty, const FActorSpawnParameters& SpawnParams, bool& bOutCollisionReady)
{
	...
	UStaticMesh* CubeMesh = FT66VisualUtil::GetBasicShapeCube();
	...
	for (int32 FloorIndex = 0; FloorIndex < Layout.Floors.Num(); ++FloorIndex)
	{
		...
		T66SpawnShellWallsForFloor(World, CubeMesh, Layout, Floor, Theme, ModuleWallHeight, SpawnParams);
		const bool bSpawnedGeneratedFloor = T66SpawnGeneratedDungeonFloorTiles(World, Theme, Layout, Floor, SpawnParams, FloorTags);
		if (!bSpawnedGeneratedFloor)
		{
			T66SpawnPolygonFloor(World, CubeMesh, Theme.FloorMaterial, Layout, Floor, SpawnParams, FloorTags);
		}
		T66SpawnMazeWalls(World, CubeMesh, Theme, Layout, Floor, ModuleWallHeight, SpawnParams);
		...
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
```

Concrete clean insertion points:

- Floor: replace or augment `T66SpawnGeneratedDungeonFloorTiles(...)`, specifically `SpawnVisualRectangleForBox(...)`, while preserving `SpawnCollisionSlabForBox(...)`.
- Walls: replace or augment `T66SpawnGeneratedDungeonWallVisualsForSide(...)` / `T66SpawnThemedWallBox(...)`, while preserving the hidden `T66SpawnHiddenCollisionProxyActor(...)` collision proxy.
- Ceiling / underside: replace or augment `T66SpawnGeneratedDungeonFloorUndersideTiles(...)`, specifically `SpawnVisualRectangleForBox(...)`, while preserving optional ceiling collision and `T66TowerMapCeilingTag` / `T66_NoSurfaceBounce` tagging.
- Roof fallback: if generated-kit mode is disabled, `T66SpawnFloorRoofSurface(...)` and `T66SpawnPolygonSurface(...)` are the roof/ceiling fallback places.

The floor helper's visual/collision lambdas are named directly in the code:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4058-4082
auto SpawnVisualRectangleForBox = [&](const FBox2D& SourceBox)
{
	...
	AStaticMeshActor* FloorVisual = T66SpawnEnvironmentRectangle(
...
auto SpawnCollisionSlabForBox = [&](const FBox2D& SourceBox)
```

The ceiling/underside helper has the same visual lambda shape:

```cpp
// Source/T66/Gameplay/T66TowerMapTerrain.cpp:4195-4204
auto SpawnVisualRectangleForBox = [&](const FBox2D& SourceBox)
{
	const FVector2D BoxSize = SourceBox.Max - SourceBox.Min;
	...
	const FVector2D Center = (SourceBox.Min + SourceBox.Max) * 0.5f;
	AStaticMeshActor* CeilingVisual = T66SpawnEnvironmentRectangle(
```

The existing process doc already identifies this intended replacement direction:

```md
<!-- Gameplay/World/MODULAR_DUNGEON_KIT_INSTRUCTIONS.md:40-45 -->
First integration principle:

- keep the procedural room layout and spawn logic
- use generated DungeonKit01 wall and floor meshes as visual runtime geometry
- use hidden `UBoxComponent` cube/slab proxies for wall and floor collision
- use old visible cuboid wall/floor geometry only as fallback when the generated kit is unavailable or disabled
```

Therefore, the lowest-churn baffle/ribbed insertion strategy is to keep the existing `FLayout` / `FFloor` / `FBox2D` room generation and collision proxies, and replace the visual rectangle spawning inside the generated floor/wall/ceiling helpers with ribbed modular static mesh visuals or an actually populated HISM module path. The runtime geometry selection is centralized in `T66TowerMapTerrain.cpp`; the room generator does not have to be rewritten to test ribbed/inflated visuals.

## Evidence Commands / Limits

- Read-only source/doc/config inspection only.
- No Unreal editor, build, cook, or runtime launch was performed.
- Scoped searches found no active `Voronoi` tower/map generation source refs, no live `GeneratedKitFloorVisualTileSize` C++/config ref, and no call sites for `T66ChooseGeneratedDungeonFloorMeshIndex(...)` or `T66ConfigureGeneratedThemeKit(...)` beyond their definitions.
