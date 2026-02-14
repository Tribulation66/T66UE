// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiasmaManager.h"
#include "Gameplay/T66MiasmaTile.h"
#include "Core/T66RunStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Algo/RandomShuffle.h"

AT66MiasmaManager::AT66MiasmaManager()
{
	PrimaryActorTick.bCanEverTick = false;
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

void AT66MiasmaManager::BuildGrid()
{
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
	if (bSpawningPaused) return;

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	if (!RunState->GetStageTimerActive())
	{
		return; // frozen: no spawn until start gate
	}

	const float Remaining = RunState->GetStageTimerSecondsRemaining();
	const float Duration = UT66RunStateSubsystem::StageTimerDurationSeconds;
	const float Elapsed = FMath::Clamp(Duration - Remaining, 0.f, Duration);
	const float Alpha = (Duration <= 0.f) ? 1.f : (Elapsed / Duration);

	const int32 Total = TileCenters.Num();
	int32 Desired = FMath::Clamp(FMath::FloorToInt(Alpha * static_cast<float>(Total)), 0, Total);

	// As soon as the timer starts, we want some miasma to be present immediately.
	if (Desired == 0)
	{
		Desired = 1;
	}

	// At timer end, ensure full coverage.
	if (Remaining <= 0.05f)
	{
		Desired = Total;
	}

	EnsureSpawnedCount(Desired);
}

static constexpr int32 MaxTilesToSpawnPerFrame = 8;

void AT66MiasmaManager::EnsureSpawnedCount(int32 DesiredCount)
{
	UWorld* World = GetWorld();
	if (!World) return;

	int32 SpawnedThisCall = 0;
	while (SpawnedTiles.Num() < DesiredCount && SpawnedTiles.Num() < TileCenters.Num() && SpawnedThisCall < MaxTilesToSpawnPerFrame)
	{
		const FVector Loc = TileCenters[SpawnedTiles.Num()];
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66MiasmaTile* Tile = World->SpawnActorDeferred<AT66MiasmaTile>(AT66MiasmaTile::StaticClass(), FTransform(FRotator::ZeroRotator, Loc), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Tile)
		{
			Tile->TileSize = TileSize;
			Tile->FinishSpawning(FTransform(FRotator::ZeroRotator, Loc));
			SpawnedTiles.Add(Tile);
			++SpawnedThisCall;
		}
		else
		{
			break;
		}
	}
}

void AT66MiasmaManager::ClearAllMiasma()
{
	for (TObjectPtr<AT66MiasmaTile>& Tile : SpawnedTiles)
	{
		if (Tile)
		{
			Tile->Destroy();
		}
	}
	SpawnedTiles.Reset();
}

