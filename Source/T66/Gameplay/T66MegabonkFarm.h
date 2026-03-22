// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66ProceduralLandscapeGenerator.h"

class UWorld;
struct FActorSpawnParameters;
struct FT66MapPreset;

namespace T66MegabonkFarm
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
		int32 BoardSize = 40;
		float BoardScale = 10.0f;
		float CellSize = 2000.0f;
		float StepHeight = 1000.0f;
		float HalfExtent = 40000.0f;
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
		ET66FarmCellShape Shape = ET66FarmCellShape::Flat;
		ET66FarmCellDecoration Decoration = ET66FarmCellDecoration::None;
		ECellRegion Region = ECellRegion::MainBoard;
		FVector DecorationLocalOffset = FVector::ZeroVector;
		FRotator DecorationLocalRotation = FRotator::ZeroRotator;
		FVector DecorationLocalScale = FVector(1.0f, 1.0f, 1.0f);
	};

	struct FBoard
	{
		FSettings Settings;
		int32 OccupiedCount = 0;
		TArray<FCell> Cells;
		TArray<FCell> ExtraCells;
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

	FSettings MakeSettings(const FT66MapPreset& Preset);
	FVector GetBoardOrigin(const FT66MapPreset& Preset);
	FVector GetCellCenter(const FT66MapPreset& Preset, int32 Row, int32 Col, float Z);
	FVector GetSpawnLocation(const FT66MapPreset& Preset, float Z);
	FVector GetPreferredSpawnLocation(const FT66MapPreset& Preset, float HeightOffset);
	float GetTraceZ(const FT66MapPreset& Preset);
	float GetLowestCollisionBottomZ(const FT66MapPreset& Preset);
	bool Generate(const FT66MapPreset& Preset, FBoard& OutBoard);
	bool Spawn(UWorld* World, const FBoard& Board, const FT66MapPreset& Preset, const FActorSpawnParameters& SpawnParams, bool& bOutCollisionReady);
}
