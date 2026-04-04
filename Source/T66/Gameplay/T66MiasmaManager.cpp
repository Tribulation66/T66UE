// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiasmaManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66MainMapTerrain.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
	static bool T66ShouldUseMainBoardCoverage(const UWorld* World)
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
}

AT66MiasmaManager::AT66MiasmaManager()
{
	PrimaryActorTick.bCanEverTick = true;

	TileInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TileInstances"));
	TileInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TileInstances->SetGenerateOverlapEvents(false);
	TileInstances->SetCanEverAffectNavigation(false);
	RootComponent = TileInstances;

	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		TileInstances->SetStaticMesh(Cube);
	}

	if (UMaterialInterface* FlatMaterial = FT66VisualUtil::GetFlatColorMaterial())
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(FlatMaterial, this))
		{
			FT66VisualUtil::ConfigureFlatColorMaterial(Mat, FLinearColor(0.96f, 0.33f, 0.08f, 1.f));
			TileInstances->SetMaterial(0, Mat);
		}
	}
}

void AT66MiasmaManager::BeginPlay()
{
	Super::BeginPlay();
	if (!bSpawningPaused)
	{
		BuildGrid();
		UpdateFromRunState();
	}
}

void AT66MiasmaManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickDamageOverActiveTiles(DeltaTime);
}

void AT66MiasmaManager::BuildMainMapSubcellGrid()
{
	TileCenters.Reset();

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

	TileSize = Board.Settings.CellSize * 0.5f;
	const float QuarterCellOffset = TileSize * 0.5f;

	for (const T66MainMapTerrain::FCell& Cell : Board.Cells)
	{
		if (!Cell.bOccupied)
		{
			continue;
		}

		FVector CellCenter = FVector::ZeroVector;
		if (!T66MainMapTerrain::TryGetCellLocation(Board, FIntPoint(Cell.X, Cell.Z), 0.f, CellCenter))
		{
			continue;
		}

		for (const float OffsetX : { -QuarterCellOffset, QuarterCellOffset })
		{
			for (const float OffsetY : { -QuarterCellOffset, QuarterCellOffset })
			{
				FVector SampleLocation = CellCenter + FVector(OffsetX, OffsetY, 0.f);
				FHitResult Hit;
				const FVector TraceStart = SampleLocation + FVector(0.f, 0.f, 4000.f);
				const FVector TraceEnd = SampleLocation - FVector(0.f, 0.f, 9000.f);
				if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
				{
					SampleLocation.Z = Hit.ImpactPoint.Z + TileZ;
				}
				else
				{
					SampleLocation.Z = CellCenter.Z + TileZ;
				}

				TileCenters.Add(SampleLocation);
			}
		}
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
}

void AT66MiasmaManager::BuildGrid()
{
	if (T66ShouldUseMainBoardCoverage(GetWorld()))
	{
		BuildMainMapSubcellGrid();
		if (TileCenters.Num() > 0)
		{
			return;
		}
	}

	TileCenters.Reset();

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
	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState || TileCenters.Num() <= 0)
	{
		return;
	}

	if (!RunState->GetStageTimerActive())
	{
		return;
	}

	const float Remaining = RunState->GetStageTimerSecondsRemaining();
	const float Duration = UT66RunStateSubsystem::StageTimerDurationSeconds;
	const float Elapsed = FMath::Clamp(Duration - Remaining, 0.f, Duration);
	const float Alpha = (Duration <= 0.f) ? 1.f : (Elapsed / Duration);

	const int32 Total = TileCenters.Num();
	int32 Desired = FMath::Clamp(FMath::FloorToInt(Alpha * static_cast<float>(Total)), 0, Total);
	if (Desired == 0)
	{
		Desired = 1;
	}
	if (Remaining <= 0.05f)
	{
		Desired = Total;
	}

	EnsureSpawnedCount(Desired);
}

void AT66MiasmaManager::EnsureSpawnedCount(int32 DesiredCount)
{
	if (!TileInstances)
	{
		return;
	}

	const FVector InstanceScale(TileSize / 100.f, TileSize / 100.f, 0.05f);
	while (SpawnedTileCount < DesiredCount && SpawnedTileCount < TileCenters.Num())
	{
		const FVector& Location = TileCenters[SpawnedTileCount];
		TileInstances->AddInstance(FTransform(FRotator::ZeroRotator, Location, InstanceScale));
		++SpawnedTileCount;
	}
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
	for (int32 Index = 0; Index < SpawnedTileCount; ++Index)
	{
		const FVector& TileCenter = TileCenters[Index];
		if (FMath::Abs(HeroLocation.X - TileCenter.X) <= HalfExtent
			&& FMath::Abs(HeroLocation.Y - TileCenter.Y) <= HalfExtent)
		{
			RunState->ApplyDamage(20, this);
			return;
		}
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
}
