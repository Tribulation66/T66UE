// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66MainMapTerrainTypes.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"

class UWorld;
enum class ET66Difficulty : uint8;
struct FActorSpawnParameters;

namespace T66MainMapTerrain
{
	enum class ECellRegion : uint8
	{
		MainBoard,
		StartPath,
		StartArea,
		BossPath,
		BossArea,
	};

	struct FSettings
	{
		int32 BoardSize = 81;
		float BoardScale = 10.0f;
		float CellSize = 2000.0f;
		float StepHeight = 1000.0f;
		float HalfExtent = 81000.0f;
		float BaselineZ = -500.0f;
		float Hilliness = 0.5f;
	};

	struct FCell
	{
		bool bOccupied = false;
		bool bSlope = false;
		int32 X = 0;
		int32 Z = 0;
		int32 Level = 0;
		ET66MapCellShape Shape = ET66MapCellShape::Flat;
		ET66MapCellDecoration Decoration = ET66MapCellDecoration::None;
		ET66MapCellSurfaceFeature SurfaceFeature = ET66MapCellSurfaceFeature::None;
		FIntPoint SurfaceFeatureOrigin = FIntPoint(INDEX_NONE, INDEX_NONE);
		ECellRegion Region = ECellRegion::MainBoard;
		FVector DecorationLocalOffset = FVector::ZeroVector;
		FRotator DecorationLocalRotation = FRotator::ZeroRotator;
		FVector DecorationLocalScale = FVector(1.0f, 1.0f, 1.0f);
	};

	struct FLavaRiver
	{
		FIntPoint PuddleCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		FIntPoint Direction = FIntPoint::ZeroValue;
		int32 LengthCells = 0;
	};

	struct FBoard
	{
		FSettings Settings;
		int32 OccupiedCount = 0;
		TArray<FCell> Cells;
		TArray<FCell> ExtraCells;
		TArray<FLavaRiver> LavaRivers;
		FIntPoint StartAnchor = FIntPoint(INDEX_NONE, INDEX_NONE);
		FIntPoint BossAnchor = FIntPoint(INDEX_NONE, INDEX_NONE);
		FIntPoint StartOutwardDirection = FIntPoint::ZeroValue;
		FIntPoint BossOutwardDirection = FIntPoint::ZeroValue;
		FIntPoint StartPathCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		FIntPoint BossPathCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		FIntPoint StartSpawnCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		FIntPoint BossSpawnCell = FIntPoint(INDEX_NONE, INDEX_NONE);

		int32 Index(int32 X, int32 Z) const
		{
			return Z * Settings.BoardSize + X;
		}

		FCell* GetCell(int32 X, int32 Z)
		{
			return (X >= 0 && X < Settings.BoardSize && Z >= 0 && Z < Settings.BoardSize)
				? &Cells[Index(X, Z)]
				: nullptr;
		}

		const FCell* GetCell(int32 X, int32 Z) const
		{
			return (X >= 0 && X < Settings.BoardSize && Z >= 0 && Z < Settings.BoardSize)
				? &Cells[Index(X, Z)]
				: nullptr;
		}

		FCell* FindExtraCell(int32 X, int32 Z)
		{
			for (FCell& Cell : ExtraCells)
			{
				if (Cell.X == X && Cell.Z == Z)
				{
					return &Cell;
				}
			}
			return nullptr;
		}

		const FCell* FindExtraCell(int32 X, int32 Z) const
		{
			for (const FCell& Cell : ExtraCells)
			{
				if (Cell.X == X && Cell.Z == Z)
				{
					return &Cell;
				}
			}
			return nullptr;
		}
	};

	FT66MapPreset BuildPresetForDifficulty(ET66Difficulty Difficulty, int32 Seed = 0);
	FSettings MakeSettings(const FT66MapPreset& Preset);
	FVector GetBoardOrigin(const FT66MapPreset& Preset);
	FVector GetCellCenter(const FT66MapPreset& Preset, int32 Row, int32 Col, float Z);
	bool TryGetCellLocation(const FBoard& Board, const FIntPoint& Coordinate, float HeightOffset, FVector& OutLocation);
	bool TryGetRegionCenterLocation(const FBoard& Board, ECellRegion Region, float HeightOffset, FVector& OutLocation);
	FVector GetSpawnLocation(const FBoard& Board, float Z);
	FVector GetSpawnLocation(const FT66MapPreset& Preset, float Z);
	FVector GetPreferredSpawnLocation(const FBoard& Board, float HeightOffset);
	FVector GetPreferredSpawnLocation(const FT66MapPreset& Preset, float HeightOffset);
	float GetSurfaceFeatureLavaCoverHeight(const FCell& Cell);
	float GetTraceZ(const FT66MapPreset& Preset);
	float GetLowestCollisionBottomZ(const FT66MapPreset& Preset);
	bool Generate(const FT66MapPreset& Preset, FBoard& OutBoard);
	bool Spawn(UWorld* World, const FBoard& Board, const FT66MapPreset& Preset, const FActorSpawnParameters& SpawnParams, bool& bOutCollisionReady);
}

