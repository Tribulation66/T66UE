// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiasmaBoundary.h"
#include "Gameplay/T66LavaShared.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/World.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66MainMapTerrain.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	struct FT66BoundaryCellInfo
	{
		FIntPoint Coordinate = FIntPoint::ZeroValue;
		float TopSurfaceZ = 0.f;
	};

	static bool T66UsesMainMapPerimeterWalls(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		return !MapName.Contains(TEXT("Coliseum"))
			&& !MapName.Contains(TEXT("Tutorial"))
			&& !MapName.Contains(TEXT("Lab"));
	}

	static UStaticMesh* T66LoadBoundaryPlaneMesh()
	{
		static TObjectPtr<UStaticMesh> CachedMesh = nullptr;
		if (!CachedMesh)
		{
			CachedMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
		}

		return CachedMesh.Get();
	}

	static UMaterialInterface* T66CreateBoundaryLavaMaterial(UObject* Outer)
	{
		UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
		if (!BaseMaterial)
		{
			return nullptr;
		}

		UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Outer);
		if (!Material)
		{
			return nullptr;
		}

		if (UTexture* WhiteTexture = LoadObject<UTexture>(nullptr, T66LavaShared::DefaultTexturePath))
		{
			Material->SetTextureParameterValue(TEXT("DiffuseColorMap"), WhiteTexture);
			Material->SetTextureParameterValue(TEXT("BaseColorTexture"), WhiteTexture);
		}

		const FLinearColor LavaTint(1.00f, 0.34f, 0.05f, 1.f);
		Material->SetVectorParameterValue(TEXT("Tint"), LavaTint);
		Material->SetVectorParameterValue(TEXT("BaseColor"), LavaTint);
		Material->SetScalarParameterValue(TEXT("Brightness"), 2.6f);
		return Material;
	}

	static float T66GetCellTopSurfaceZ(const T66MainMapTerrain::FBoard& Board, int32 Level)
	{
		return Board.Settings.BaselineZ
			+ static_cast<float>(Level) * Board.Settings.StepHeight
			+ Board.Settings.StepHeight * 0.5f;
	}

	static FVector T66GetCellCenter(const T66MainMapTerrain::FBoard& Board, const FIntPoint& Coordinate, float Z)
	{
		return FVector(
			-Board.Settings.HalfExtent + (static_cast<float>(Coordinate.X) + 0.5f) * Board.Settings.CellSize,
			-Board.Settings.HalfExtent + (static_cast<float>(Coordinate.Y) + 0.5f) * Board.Settings.CellSize,
			Z);
	}

	static void T66CollectBoundaryCellInfo(
		const T66MainMapTerrain::FBoard& Board,
		const T66MainMapTerrain::FCell& Cell,
		TMap<FIntPoint, FT66BoundaryCellInfo>& OutCellInfo)
	{
		if (!Cell.bOccupied)
		{
			return;
		}

		const FIntPoint Coordinate(Cell.X, Cell.Z);
		FT66BoundaryCellInfo& CellInfo = OutCellInfo.FindOrAdd(Coordinate);
		CellInfo.Coordinate = Coordinate;
		CellInfo.TopSurfaceZ = T66GetCellTopSurfaceZ(Board, Cell.Level);
	}

	static void T66AddBoundaryLavaInstance(
		UInstancedStaticMeshComponent* LavaInstances,
		const FVector& Center,
		float CellSize)
	{
		if (!LavaInstances)
		{
			return;
		}

		const FVector Scale(
			FMath::Max(CellSize / 100.f, 0.1f),
			FMath::Max(CellSize / 100.f, 0.1f),
			1.0f);
		LavaInstances->AddInstance(FTransform(FRotator::ZeroRotator, Center, Scale));
	}
}

AT66MiasmaBoundary::AT66MiasmaBoundary()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SceneRoot->SetMobility(EComponentMobility::Static);
	SetRootComponent(SceneRoot);

	BoundaryWallInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BoundaryWallInstances"));
	BoundaryWallInstances->SetupAttachment(SceneRoot);
	BoundaryWallInstances->SetMobility(EComponentMobility::Static);
	BoundaryWallInstances->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoundaryWallInstances->SetCollisionResponseToAllChannels(ECR_Block);
	BoundaryWallInstances->SetGenerateOverlapEvents(false);
	BoundaryWallInstances->SetCanEverAffectNavigation(false);
	if (UStaticMesh* PlaneMesh = T66LoadBoundaryPlaneMesh())
	{
		BoundaryWallInstances->SetStaticMesh(PlaneMesh);
	}
}

void AT66MiasmaBoundary::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->RegisterMiasmaBoundary(this);
		}
	}

	BuildBoundaryWalls();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DamageTimerHandle, this, &AT66MiasmaBoundary::ApplyBoundaryDamageTick, DamageIntervalSeconds, true, DamageIntervalSeconds);
	}
}

void AT66MiasmaBoundary::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->UnregisterMiasmaBoundary(this);
		}

		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66MiasmaBoundary::BuildBoundaryWalls()
{
	UWorld* World = GetWorld();
	if (!World || !BoundaryWallInstances)
	{
		return;
	}

	BoundaryWallInstances->ClearInstances();
	PlayableCellCoordinates.Reset();
	PlayableCellSize = 2000.f;
	PlayableHalfExtent = SafeHalfExtent;

	if (!BoundaryWallInstances->GetStaticMesh())
	{
		return;
	}

	if (UMaterialInterface* LavaMaterial = T66CreateBoundaryLavaMaterial(this))
	{
		BoundaryWallInstances->SetMaterial(0, LavaMaterial);
	}

	if (!T66UsesMainMapPerimeterWalls(World))
	{
		return;
	}

	const UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance());
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const int32 RunSeed = T66GI ? T66GI->RunSeed : 0;

	const FT66MapPreset Preset = T66MainMapTerrain::BuildPresetForDifficulty(Difficulty, RunSeed);
	T66MainMapTerrain::FBoard Board;
	if (!T66MainMapTerrain::Generate(Preset, Board))
	{
		return;
	}

	CachePlayableCells(Board);

	TMap<FIntPoint, FT66BoundaryCellInfo> CellInfoByCoordinate;
	for (const T66MainMapTerrain::FCell& Cell : Board.Cells)
	{
		T66CollectBoundaryCellInfo(Board, Cell, CellInfoByCoordinate);
	}
	for (const T66MainMapTerrain::FCell& Cell : Board.ExtraCells)
	{
		T66CollectBoundaryCellInfo(Board, Cell, CellInfoByCoordinate);
	}

	const int32 CenterCoordinate = Board.Settings.BoardSize / 2;
	const int32 FootprintSize = FMath::Max(TotalFootprintSizeCells, Board.Settings.BoardSize);
	const int32 MinCoordinate = CenterCoordinate - (FootprintSize / 2);
	const int32 MaxCoordinate = MinCoordinate + FootprintSize - 1;
	int32 BoundaryLavaCount = 0;

	for (int32 GridY = MinCoordinate; GridY <= MaxCoordinate; ++GridY)
	{
		for (int32 GridX = MinCoordinate; GridX <= MaxCoordinate; ++GridX)
		{
			const FIntPoint Coordinate(GridX, GridY);
			if (CellInfoByCoordinate.Contains(Coordinate))
			{
				continue;
			}

			const int32 SampleX = FMath::Clamp(GridX, 0, Board.Settings.BoardSize - 1);
			const int32 SampleY = FMath::Clamp(GridY, 0, Board.Settings.BoardSize - 1);
			const T66MainMapTerrain::FCell* SampleCell = Board.GetCell(SampleX, SampleY);
			const float SampleTopSurfaceZ = SampleCell
				? T66GetCellTopSurfaceZ(Board, SampleCell->Level)
				: (Board.Settings.BaselineZ + Board.Settings.StepHeight * 0.5f);
			const FVector CellCenter = T66GetCellCenter(Board, Coordinate, SampleTopSurfaceZ + BoundaryLavaTileZ);
			T66AddBoundaryLavaInstance(BoundaryWallInstances, CellCenter, Board.Settings.CellSize);
			++BoundaryLavaCount;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[LAVA] Built boundary lava apron with %d cells (footprint=%dx%d)."), BoundaryLavaCount, FootprintSize, FootprintSize);
}

void AT66MiasmaBoundary::CachePlayableCells(const T66MainMapTerrain::FBoard& Board)
{
	PlayableCellCoordinates.Reset();
	PlayableCellSize = Board.Settings.CellSize;
	PlayableHalfExtent = Board.Settings.HalfExtent;
	SafeHalfExtent = PlayableHalfExtent;

	auto CacheCell = [&](const T66MainMapTerrain::FCell& Cell)
	{
		if (!Cell.bOccupied)
		{
			return;
		}

		PlayableCellCoordinates.Add(FIntPoint(Cell.X, Cell.Z));
	};

	for (const T66MainMapTerrain::FCell& Cell : Board.Cells)
	{
		CacheCell(Cell);
	}
	for (const T66MainMapTerrain::FCell& Cell : Board.ExtraCells)
	{
		CacheCell(Cell);
	}
}

bool AT66MiasmaBoundary::IsLocationInsidePlayableSpace(const FVector& Location) const
{
	if (PlayableCellCoordinates.Num() <= 0 || PlayableCellSize <= KINDA_SMALL_NUMBER)
	{
		return FMath::Abs(Location.X) <= SafeHalfExtent
			&& FMath::Abs(Location.Y) <= SafeHalfExtent;
	}

	const int32 GridX = FMath::FloorToInt((Location.X + PlayableHalfExtent) / PlayableCellSize);
	const int32 GridY = FMath::FloorToInt((Location.Y + PlayableHalfExtent) / PlayableCellSize);
	return PlayableCellCoordinates.Contains(FIntPoint(GridX, GridY));
}

void AT66MiasmaBoundary::ApplyBoundaryDamageTick()
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	AT66HeroBase* Hero = Cast<AT66HeroBase>(Pawn);
	if (!Hero)
	{
		return;
	}

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	if (!IsLocationInsidePlayableSpace(Hero->GetActorLocation()))
	{
		RunState->ApplyDamage(20);
	}
}
