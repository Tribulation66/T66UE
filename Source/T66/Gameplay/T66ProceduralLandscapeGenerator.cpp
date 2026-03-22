// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ProceduralLandscapeGenerator.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "Core/T66GameplayLayout.h"
#include "T66.h"
#include "Math/RandomStream.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
	int32 GridIdx(int32 Row, int32 Col, int32 GridSize)
	{
		return Row * GridSize + Col;
	}

	float QuantizeToStep(float Value, const FT66MapPreset& Preset)
	{
		const float Step = FMath::Max(Preset.ElevationStep, 1.f);
		const float Quantized = Preset.BaselineZ + FMath::RoundToFloat((Value - Preset.BaselineZ) / Step) * Step;
		return FMath::Clamp(Quantized, Preset.ElevationMin, Preset.ElevationMax);
	}

	TArray<float> BuildHeightLevels(const FT66MapPreset& Preset)
	{
		TArray<float> Levels;
		const float Step = FMath::Max(Preset.ElevationStep, 1.f);
		const int32 MinSteps = FMath::FloorToInt((Preset.ElevationMin - Preset.BaselineZ) / Step);
		const int32 MaxSteps = FMath::CeilToInt((Preset.ElevationMax - Preset.BaselineZ) / Step);

		for (int32 StepIndex = MinSteps; StepIndex <= MaxSteps; ++StepIndex)
		{
			const float Level = Preset.BaselineZ + static_cast<float>(StepIndex) * Step;
			if (Level < Preset.ElevationMin - KINDA_SMALL_NUMBER || Level > Preset.ElevationMax + KINDA_SMALL_NUMBER)
			{
				continue;
			}

			Levels.Add(Level);
		}

		if (Levels.Num() == 0)
		{
			Levels.Add(Preset.BaselineZ);
		}

		bool bHasBaseline = false;
		for (const float Level : Levels)
		{
			if (FMath::IsNearlyEqual(Level, Preset.BaselineZ))
			{
				bHasBaseline = true;
				break;
			}
		}
		if (!bHasBaseline)
		{
			Levels.Add(Preset.BaselineZ);
		}

		Levels.Sort();

		for (int32 Index = Levels.Num() - 2; Index >= 0; --Index)
		{
			if (FMath::IsNearlyEqual(Levels[Index], Levels[Index + 1]))
			{
				Levels.RemoveAt(Index + 1);
			}
		}

		return Levels;
	}

	float SampleSeededGridNoise(int32 Seed, float GridX, float GridY, float ScaleCells, int32 Salt)
	{
		const float SafeScale = FMath::Max(ScaleCells, 0.35f);
		const uint32 Hash = HashCombineFast(GetTypeHash(Seed), GetTypeHash(Salt));
		const float OffsetX = static_cast<float>(Hash & 1023u) * 0.173f;
		const float OffsetY = static_cast<float>((Hash >> 10) & 1023u) * 0.197f;
		return FMath::PerlinNoise2D(FVector2D(GridX / SafeScale + OffsetX, GridY / SafeScale + OffsetY));
	}

	float SampleRidgedGridNoise(int32 Seed, float GridX, float GridY, float ScaleCells, int32 Salt)
	{
		return 1.f - FMath::Abs(SampleSeededGridNoise(Seed, GridX, GridY, ScaleCells, Salt));
	}

	float GetLevelSelectionScore(const TArray<float>& HeightLevels, int32 LevelIndex, const FT66MapPreset& Preset, int32 UsageCount, FRandomStream& Rng)
	{
		const float Level = HeightLevels[LevelIndex];
		const float UpperRange = FMath::Max(Preset.ElevationMax - Preset.BaselineZ, Preset.ElevationStep);
		const float LowerRange = FMath::Max(Preset.BaselineZ - Preset.ElevationMin, Preset.ElevationStep);
		const float SignedAlpha = (Level >= Preset.BaselineZ)
			? (Level - Preset.BaselineZ) / UpperRange
			: -((Preset.BaselineZ - Level) / LowerRange);
		const float BiasTarget = FMath::Clamp(Preset.ElevationBias, -1.f, 1.f) * 0.8f;

		float Score = -FMath::Abs(SignedAlpha - BiasTarget) * 2.3f;
		Score -= static_cast<float>(UsageCount) * 0.55f;

		if (FMath::IsNearlyEqual(Level, Preset.BaselineZ))
		{
			Score += 0.18f;
		}

		Score += Rng.FRandRange(0.f, 0.35f);
		return Score;
	}

	uint64 MakeBoundaryKey(int32 A, int32 B)
	{
		const uint32 Low = static_cast<uint32>(FMath::Min(A, B));
		const uint32 High = static_cast<uint32>(FMath::Max(A, B));
		return (static_cast<uint64>(Low) << 32) | static_cast<uint64>(High);
	}

	FBox2D BuildRampFootprint(
		const FT66ProceduralMapResult& Result,
		const FT66RampEdge& Edge,
		float Padding);

	bool DoesRampOverlapOuterRing(
		const FT66ProceduralMapResult& Result,
		const FT66RampEdge& Edge,
		const FT66MapPreset& Preset,
		float CellSize,
		float Padding);

	bool DoesRampOverlapProtectedSpawnZones(
		const FT66ProceduralMapResult& Result,
		const FT66RampEdge& Edge,
		float Padding);

	struct FT66RampFootprint
	{
		FBox2D Box;
	};

	// ========================================================================
	// Platform initialization
	// ========================================================================

	void InitializePlatforms(FT66ProceduralMapResult& Result, const FT66MapPreset& Preset, int32 GridSize, float CellSize)
	{
		Result.Platforms.SetNum(GridSize * GridSize);

		for (int32 R = 0; R < GridSize; ++R)
		{
			for (int32 C = 0; C < GridSize; ++C)
			{
				const int32 Idx = GridIdx(R, C, GridSize);
				FT66PlatformNode& P = Result.Platforms[Idx];
				P.bFarmOccupied = (Preset.Theme != ET66MapTheme::Farm);
				P.Position = FVector2D(
					-Preset.MapHalfExtent + (C + 0.5f) * CellSize,
					-Preset.MapHalfExtent + (R + 0.5f) * CellSize);
				P.TopZ = Preset.BaselineZ;
				P.SurfaceStartZ = Preset.BaselineZ;
				P.SizeX = CellSize;
				P.SizeY = CellSize;
				P.GridRow = R;
				P.GridCol = C;
				P.FarmLevel = 0;
				P.FarmCellShape = ET66FarmCellShape::Flat;
				P.FarmDecoration = ET66FarmCellDecoration::None;
				P.FarmDecorationLocalOffset = FVector::ZeroVector;
				P.FarmDecorationLocalRotation = FRotator::ZeroRotator;
				P.FarmDecorationLocalScale = FVector(1.f, 1.f, 1.f);
				P.Connections.Reset();
			}
		}
	}

	int32 ClampHeightKeyToPreset(int32 HeightKey, const FT66MapPreset& Preset)
	{
		const float Step = FMath::Max(Preset.ElevationStep, 1.f);
		const int32 MinKey = FMath::FloorToInt((Preset.ElevationMin - Preset.BaselineZ) / Step);
		const int32 MaxKey = FMath::CeilToInt((Preset.ElevationMax - Preset.BaselineZ) / Step);
		return FMath::Clamp(HeightKey, MinKey, MaxKey);
	}

	int32 HeightKeyFromValue(float Value, const FT66MapPreset& Preset)
	{
		const float Step = FMath::Max(Preset.ElevationStep, 1.f);
		return ClampHeightKeyToPreset(FMath::RoundToInt((Value - Preset.BaselineZ) / Step), Preset);
	}

	float HeightValueFromKey(int32 HeightKey, const FT66MapPreset& Preset)
	{
		const float RawHeight = Preset.BaselineZ + static_cast<float>(ClampHeightKeyToPreset(HeightKey, Preset)) * FMath::Max(Preset.ElevationStep, 1.f);
		return FMath::Clamp(RawHeight, Preset.ElevationMin, Preset.ElevationMax);
	}

	float GetMaxClimbableRampAngleDegrees()
	{
		constexpr float DefaultWalkableFloorZ = 0.71f;
		constexpr float RampSlopeSafetyMarginDegrees = 4.0f;

		float WalkableFloorAngle = FMath::RadiansToDegrees(FMath::Acos(DefaultWalkableFloorZ));
		if (const AT66HeroBase* HeroCDO = GetDefault<AT66HeroBase>())
		{
			if (const UCharacterMovementComponent* Movement = HeroCDO->GetCharacterMovement())
			{
				WalkableFloorAngle = Movement->GetWalkableFloorAngle();
			}
		}

		return FMath::Clamp(WalkableFloorAngle - RampSlopeSafetyMarginDegrees, 10.0f, 85.0f);
	}

	float GetRequiredRampDepthForHeight(float HeightDiff)
	{
		const float SafeAngleDegrees = GetMaxClimbableRampAngleDegrees();
		const float TanSafeAngle = FMath::Tan(FMath::DegreesToRadians(SafeAngleDegrees));
		if (TanSafeAngle <= KINDA_SMALL_NUMBER)
		{
			return TNumericLimits<float>::Max();
		}

		return HeightDiff / TanSafeAngle;
	}

	float GetRampSlopeAngleDegrees(float HeightDiff, float Depth)
	{
		if (Depth <= KINDA_SMALL_NUMBER)
		{
			return 90.0f;
		}

		return FMath::RadiansToDegrees(FMath::Atan(HeightDiff / Depth));
	}

	bool IsPlatformOnOuterRing(const FT66PlatformNode& Platform, int32 GridSize)
	{
		return Platform.GridRow == 0 || Platform.GridRow == GridSize - 1
			|| Platform.GridCol == 0 || Platform.GridCol == GridSize - 1;
	}

	float GetRampSeamOverlap(const FT66ProceduralMapResult& Result, const FT66RampEdge& Edge)
	{
		const FT66PlatformNode& Lo = Result.Platforms[Edge.LowerIndex];
		const FT66PlatformNode& Hi = Result.Platforms[Edge.HigherIndex];
		const float CellSize = FMath::Min(
			FMath::Min(Lo.SizeX, Lo.SizeY),
			FMath::Min(Hi.SizeX, Hi.SizeY));
		return FMath::Clamp(CellSize * 0.06f, 80.f, 220.f);
	}

	void PaintOrthogonalPlateauBlob(
		TArray<int32>& HeightKeys,
		int32 GridSize,
		int32 CenterRow,
		int32 CenterCol,
		int32 MainRows,
		int32 MainCols,
		int32 HeightKey,
		FRandomStream& Rng)
	{
		auto PaintRect = [&](int32 RowMin, int32 RowMax, int32 ColMin, int32 ColMax)
		{
			for (int32 Row = FMath::Max(1, RowMin); Row <= FMath::Min(GridSize - 2, RowMax); ++Row)
			{
				for (int32 Col = FMath::Max(1, ColMin); Col <= FMath::Min(GridSize - 2, ColMax); ++Col)
				{
					const int32 Idx = GridIdx(Row, Col, GridSize);
					HeightKeys[Idx] = FMath::Max(HeightKeys[Idx], HeightKey);
				}
			}
		};

		const int32 HalfRows = MainRows / 2;
		const int32 HalfCols = MainCols / 2;
		PaintRect(CenterRow - HalfRows, CenterRow + MainRows - HalfRows - 1, CenterCol - HalfCols, CenterCol + MainCols - HalfCols - 1);

		const int32 ArmCount = Rng.RandRange(1, 3);
		for (int32 ArmIndex = 0; ArmIndex < ArmCount; ++ArmIndex)
		{
			const int32 Side = Rng.RandRange(0, 3);
			const int32 ArmRows = FMath::Max(1, MainRows + Rng.RandRange(-1, 2));
			const int32 ArmCols = FMath::Max(1, MainCols + Rng.RandRange(-1, 2));

			switch (Side)
			{
			case 0: // north
				PaintRect(CenterRow - HalfRows - ArmRows + 1, CenterRow - HalfRows, CenterCol - ArmCols / 2, CenterCol + (ArmCols - ArmCols / 2) - 1);
				break;
			case 1: // south
				PaintRect(CenterRow + MainRows - HalfRows - 1, CenterRow + MainRows - HalfRows + ArmRows - 2, CenterCol - ArmCols / 2, CenterCol + (ArmCols - ArmCols / 2) - 1);
				break;
			case 2: // west
				PaintRect(CenterRow - ArmRows / 2, CenterRow + (ArmRows - ArmRows / 2) - 1, CenterCol - HalfCols - ArmCols + 1, CenterCol - HalfCols);
				break;
			default: // east
				PaintRect(CenterRow - ArmRows / 2, CenterRow + (ArmRows - ArmRows / 2) - 1, CenterCol + MainCols - HalfCols - 1, CenterCol + MainCols - HalfCols + ArmCols - 2);
				break;
			}
		}
	}

	void GenerateFarmPlateauHeights(TArray<FT66PlatformNode>& Platforms, const FT66MapPreset& Preset, int32 GridSize)
	{
		FRandomStream Rng(Preset.Seed);
		TArray<int32> HeightKeys;
		HeightKeys.Init(HeightKeyFromValue(Preset.BaselineZ, Preset), Platforms.Num());
		TArray<FIntPoint> SeedCenters;

		auto PickOpenCenter = [&](int32 Margin, const TArray<FIntPoint>& ExistingCenters, int32 MinSpacing) -> FIntPoint
		{
			for (int32 Attempt = 0; Attempt < 80; ++Attempt)
			{
				const FIntPoint Candidate(
					Rng.RandRange(Margin, GridSize - Margin - 1),
					Rng.RandRange(Margin, GridSize - Margin - 1));

				bool bFarEnough = true;
				for (const FIntPoint& Existing : ExistingCenters)
				{
					if (FMath::Abs(Candidate.X - Existing.X) + FMath::Abs(Candidate.Y - Existing.Y) < MinSpacing)
					{
						bFarEnough = false;
						break;
					}
				}

				if (bFarEnough)
				{
					return Candidate;
				}
			}

			return FIntPoint(
				Rng.RandRange(Margin, GridSize - Margin - 1),
				Rng.RandRange(Margin, GridSize - Margin - 1));
		};

		auto PickHeightKey = [&](std::initializer_list<int32> Choices) -> int32
		{
			const int32 ChoiceCount = static_cast<int32>(Choices.size());
			const int32 ChoiceIndex = Rng.RandRange(0, ChoiceCount - 1);
			auto It = Choices.begin();
			for (int32 Index = 0; Index < ChoiceIndex; ++Index)
			{
				++It;
			}
			return *It;
		};

		const int32 StepKey = HeightKeyFromValue(Preset.BaselineZ + Preset.ElevationStep, Preset) - HeightKeyFromValue(Preset.BaselineZ, Preset);
		const int32 BaseKey = HeightKeyFromValue(Preset.BaselineZ, Preset);
		const int32 StepBand = FMath::Max(StepKey, 1);
		const int32 ShallowLowKey = ClampHeightKeyToPreset(BaseKey - 2 * StepBand, Preset);
		const int32 LowKey = ClampHeightKeyToPreset(BaseKey - 5 * StepBand, Preset);
		const int32 DeepLowKey = ClampHeightKeyToPreset(BaseKey - 8 * StepBand, Preset);
		const int32 MidLowKey = ClampHeightKeyToPreset(BaseKey + 2 * StepBand, Preset);
		const int32 MidKey = ClampHeightKeyToPreset(BaseKey + 5 * StepBand, Preset);
		const int32 HighKey = ClampHeightKeyToPreset(BaseKey + 8 * StepBand, Preset);
		const int32 PeakKey = ClampHeightKeyToPreset(BaseKey + 11 * StepBand, Preset);

		struct FFarmSeed
		{
			int32 Row = 0;
			int32 Col = 0;
			int32 HeightKey = 0;
			float AxisWeightX = 1.f;
			float AxisWeightY = 1.f;
			int32 NoiseSalt = 0;
		};

		auto BlendOrthogonalPlateauBlob = [&](int32 CenterRow, int32 CenterCol, int32 MainRows, int32 MainCols, int32 HeightKey, bool bRaise)
		{
			auto BlendRect = [&](int32 RowMin, int32 RowMax, int32 ColMin, int32 ColMax)
			{
				for (int32 Row = FMath::Max(1, RowMin); Row <= FMath::Min(GridSize - 2, RowMax); ++Row)
				{
					for (int32 Col = FMath::Max(1, ColMin); Col <= FMath::Min(GridSize - 2, ColMax); ++Col)
					{
						const int32 Idx = GridIdx(Row, Col, GridSize);
						HeightKeys[Idx] = bRaise
							? FMath::Max(HeightKeys[Idx], HeightKey)
							: FMath::Min(HeightKeys[Idx], HeightKey);
					}
				}
			};

			const int32 HalfRows = MainRows / 2;
			const int32 HalfCols = MainCols / 2;
			BlendRect(CenterRow - HalfRows, CenterRow + MainRows - HalfRows - 1, CenterCol - HalfCols, CenterCol + MainCols - HalfCols - 1);

			const int32 ArmCount = Rng.RandRange(1, 3);
			for (int32 ArmIndex = 0; ArmIndex < ArmCount; ++ArmIndex)
			{
				const int32 Side = Rng.RandRange(0, 3);
				const int32 ArmRows = FMath::Max(1, MainRows + Rng.RandRange(-1, 2));
				const int32 ArmCols = FMath::Max(1, MainCols + Rng.RandRange(-1, 2));

				switch (Side)
				{
				case 0:
					BlendRect(CenterRow - HalfRows - ArmRows + 1, CenterRow - HalfRows, CenterCol - ArmCols / 2, CenterCol + (ArmCols - ArmCols / 2) - 1);
					break;
				case 1:
					BlendRect(CenterRow + MainRows - HalfRows - 1, CenterRow + MainRows - HalfRows + ArmRows - 2, CenterCol - ArmCols / 2, CenterCol + (ArmCols - ArmCols / 2) - 1);
					break;
				case 2:
					BlendRect(CenterRow - ArmRows / 2, CenterRow + (ArmRows - ArmRows / 2) - 1, CenterCol - HalfCols - ArmCols + 1, CenterCol - HalfCols);
					break;
				default:
					BlendRect(CenterRow - ArmRows / 2, CenterRow + (ArmRows - ArmRows / 2) - 1, CenterCol + MainCols - HalfCols - 1, CenterCol + MainCols - HalfCols + ArmCols - 2);
					break;
				}
			}
		};

		const int32 SeedCountMin = FMath::Max(Preset.RegionSeedCountMin, 40);
		const int32 SeedCountMax = FMath::Max(Preset.RegionSeedCountMax, 52);
		const int32 SeedCount = Rng.RandRange(SeedCountMin, SeedCountMax);
		TArray<FFarmSeed> Seeds;
		Seeds.Reserve(SeedCount);

		const TArray<int32> MandatoryHeights = { DeepLowKey, LowKey, ShallowLowKey, MidLowKey, MidKey, HighKey, PeakKey };
		for (int32 SeedIndex = 0; SeedIndex < SeedCount; ++SeedIndex)
		{
			const FIntPoint Center = PickOpenCenter(2, SeedCenters, 3);
			SeedCenters.Add(Center);

			int32 HeightKey = BaseKey;
			if (MandatoryHeights.IsValidIndex(SeedIndex))
			{
				HeightKey = MandatoryHeights[SeedIndex];
			}
			else
			{
				HeightKey = PickHeightKey({ DeepLowKey, DeepLowKey, LowKey, LowKey, ShallowLowKey, ShallowLowKey, MidLowKey, MidLowKey, MidKey, HighKey, PeakKey });
			}

			FFarmSeed Seed;
			Seed.Row = Center.X;
			Seed.Col = Center.Y;
			Seed.HeightKey = HeightKey;
			Seed.AxisWeightX = Rng.FRandRange(0.88f, 1.12f);
			Seed.AxisWeightY = Rng.FRandRange(0.88f, 1.12f);
			Seed.NoiseSalt = 0x460000 + SeedIndex * 97;
			Seeds.Add(Seed);
		}

		for (int32 R = 0; R < GridSize; ++R)
		{
			for (int32 C = 0; C < GridSize; ++C)
			{
				float BestScore = TNumericLimits<float>::Max();
				int32 BestHeightKey = BaseKey;

				for (const FFarmSeed& Seed : Seeds)
				{
					const float DX = FMath::Abs(static_cast<float>(C - Seed.Col));
					const float DY = FMath::Abs(static_cast<float>(R - Seed.Row));
					const float ScaledX = DX * Seed.AxisWeightX;
					const float ScaledY = DY * Seed.AxisWeightY;
					float Score = FMath::Max(ScaledX, ScaledY) + 0.35f * FMath::Min(ScaledX, ScaledY);
					Score += SampleSeededGridNoise(Preset.Seed ^ Seed.NoiseSalt, static_cast<float>(C), static_cast<float>(R), 2.0f, Seed.NoiseSalt) * 0.42f;

					if (Score < BestScore)
					{
						BestScore = Score;
						BestHeightKey = Seed.HeightKey;
					}
				}

				HeightKeys[GridIdx(R, C, GridSize)] = BestHeightKey;
			}
		}

		const int32 RaisedPlateauCount = Rng.RandRange(5, 8);
		for (int32 PlateauIndex = 0; PlateauIndex < RaisedPlateauCount; ++PlateauIndex)
		{
			const FIntPoint Center = PickOpenCenter(3, SeedCenters, 3);
			BlendOrthogonalPlateauBlob(
				Center.X,
				Center.Y,
				Rng.RandRange(2, 4),
				Rng.RandRange(2, 5),
				PickHeightKey({ MidKey, HighKey, HighKey, PeakKey }),
				true);
		}

		const int32 SunkenPocketCount = Rng.RandRange(4, 7);
		for (int32 PocketIndex = 0; PocketIndex < SunkenPocketCount; ++PocketIndex)
		{
			const FIntPoint Center = PickOpenCenter(2, SeedCenters, 2);
			BlendOrthogonalPlateauBlob(
				Center.X,
				Center.Y,
				Rng.RandRange(2, 3),
				Rng.RandRange(2, 3),
				PickHeightKey({ ShallowLowKey, LowKey, LowKey, DeepLowKey, DeepLowKey }),
				false);
		}

		const int32 LedgeCount = Rng.RandRange(4, 6);
		for (int32 LedgeIndex = 0; LedgeIndex < LedgeCount; ++LedgeIndex)
		{
			const FIntPoint Center = PickOpenCenter(2, SeedCenters, 2);
			const bool bLongAlongRows = Rng.FRand() < 0.5f;
			BlendOrthogonalPlateauBlob(
				Center.X,
				Center.Y,
				bLongAlongRows ? Rng.RandRange(3, 5) : Rng.RandRange(1, 3),
				bLongAlongRows ? Rng.RandRange(1, 3) : Rng.RandRange(3, 5),
				PickHeightKey({ MidKey, HighKey, PeakKey }),
				true);
		}

		auto CountNegativeNonReservedCells = [&]() -> int32
		{
			int32 NegativeCount = 0;
			for (int32 Index = 0; Index < HeightKeys.Num(); ++Index)
			{
				const FT66PlatformNode& Cell = Platforms[Index];
				const float HalfX = Cell.SizeX * 0.5f;
				const float HalfY = Cell.SizeY * 0.5f;
				if (T66GameplayLayout::DoesRectOverlapTraversalKeepClearZone2D(
						Cell.Position.X - HalfX,
						Cell.Position.X + HalfX,
						Cell.Position.Y - HalfY,
						Cell.Position.Y + HalfY))
				{
					continue;
				}

				if (HeightKeys[Index] < BaseKey)
				{
					++NegativeCount;
				}
			}
			return NegativeCount;
		};

		int32 TraversableNonReservedCells = 0;
		for (int32 Index = 0; Index < HeightKeys.Num(); ++Index)
		{
			const FT66PlatformNode& Cell = Platforms[Index];
			const float HalfX = Cell.SizeX * 0.5f;
			const float HalfY = Cell.SizeY * 0.5f;
			if (!T66GameplayLayout::DoesRectOverlapTraversalKeepClearZone2D(
					Cell.Position.X - HalfX,
					Cell.Position.X + HalfX,
					Cell.Position.Y - HalfY,
					Cell.Position.Y + HalfY))
			{
				++TraversableNonReservedCells;
			}
		}

		const int32 TargetNegativeCells = FMath::Max(24, FMath::RoundToInt(static_cast<float>(TraversableNonReservedCells) * 0.22f));
		for (int32 NegativeBoostPass = 0; NegativeBoostPass < 4 && CountNegativeNonReservedCells() < TargetNegativeCells; ++NegativeBoostPass)
		{
			const FIntPoint Center = PickOpenCenter(2, SeedCenters, 2);
			BlendOrthogonalPlateauBlob(
				Center.X,
				Center.Y,
				Rng.RandRange(2, 4),
				Rng.RandRange(2, 4),
				PickHeightKey({ LowKey, LowKey, DeepLowKey, DeepLowKey }),
				false);
		}

		auto BreakUpOversizedFarmPlateaus = [&](int32 MaxSpanCells, int32 MaxAreaCells, int32 MaxPasses)
		{
			for (int32 Pass = 0; Pass < MaxPasses; ++Pass)
			{
				TArray<bool> bVisited;
				bVisited.Init(false, HeightKeys.Num());
				bool bChanged = false;

				for (int32 StartIdx = 0; StartIdx < HeightKeys.Num(); ++StartIdx)
				{
					if (bVisited[StartIdx])
					{
						continue;
					}

					TArray<int32> RegionCells;
					TArray<int32> Queue;
					Queue.Add(StartIdx);
					bVisited[StartIdx] = true;

					const int32 RegionKey = HeightKeys[StartIdx];
					int32 MinRow = Platforms[StartIdx].GridRow;
					int32 MaxRow = Platforms[StartIdx].GridRow;
					int32 MinCol = Platforms[StartIdx].GridCol;
					int32 MaxCol = Platforms[StartIdx].GridCol;
					bool bTouchesReservedTraversal = false;
					bool bTouchesOuterEdge = false;

					for (int32 Head = 0; Head < Queue.Num(); ++Head)
					{
						const int32 CurIdx = Queue[Head];
						const FT66PlatformNode& Cell = Platforms[CurIdx];
						const float HalfX = Cell.SizeX * 0.5f;
						const float HalfY = Cell.SizeY * 0.5f;
						RegionCells.Add(CurIdx);
						MinRow = FMath::Min(MinRow, Cell.GridRow);
						MaxRow = FMath::Max(MaxRow, Cell.GridRow);
						MinCol = FMath::Min(MinCol, Cell.GridCol);
						MaxCol = FMath::Max(MaxCol, Cell.GridCol);
						bTouchesReservedTraversal |= T66GameplayLayout::DoesRectOverlapTraversalKeepClearZone2D(
							Cell.Position.X - HalfX,
							Cell.Position.X + HalfX,
							Cell.Position.Y - HalfY,
							Cell.Position.Y + HalfY);
						bTouchesOuterEdge |= (Cell.GridRow == 0 || Cell.GridRow == GridSize - 1 || Cell.GridCol == 0 || Cell.GridCol == GridSize - 1);

						auto VisitNeighbor = [&](int32 NR, int32 NC)
						{
							if (NR < 0 || NR >= GridSize || NC < 0 || NC >= GridSize)
							{
								return;
							}

							const int32 NIdx = GridIdx(NR, NC, GridSize);
							if (bVisited[NIdx] || HeightKeys[NIdx] != RegionKey)
							{
								return;
							}

							bVisited[NIdx] = true;
							Queue.Add(NIdx);
						};

						VisitNeighbor(Cell.GridRow - 1, Cell.GridCol);
						VisitNeighbor(Cell.GridRow + 1, Cell.GridCol);
						VisitNeighbor(Cell.GridRow, Cell.GridCol - 1);
						VisitNeighbor(Cell.GridRow, Cell.GridCol + 1);
					}

					const int32 RowSpan = MaxRow - MinRow + 1;
					const int32 ColSpan = MaxCol - MinCol + 1;
					if (bTouchesReservedTraversal || bTouchesOuterEdge)
					{
						continue;
					}

					if (RowSpan <= MaxSpanCells && ColSpan <= MaxSpanCells && RegionCells.Num() <= MaxAreaCells)
					{
						continue;
					}

					const bool bSplitByRows = (RowSpan >= ColSpan);
					const int32 Pivot = bSplitByRows ? ((MinRow + MaxRow) / 2) : ((MinCol + MaxCol) / 2);

					int32 DeltaKey = (RegionKey <= BaseKey) ? 1 : -1;
					if (RegionKey == BaseKey)
					{
						DeltaKey = (Rng.FRand() < 0.68f) ? 1 : -1;
					}
					else if (RegionKey >= PeakKey - 1)
					{
						DeltaKey = -1;
					}
					else if (RegionKey <= LowKey + 1)
					{
						DeltaKey = 1;
					}

					int32 DeltaMagnitude = 1;
					if (RegionCells.Num() > MaxAreaCells * 2 || RowSpan > MaxSpanCells + 1 || ColSpan > MaxSpanCells + 1)
					{
						DeltaMagnitude = 2;
					}
					if (Rng.FRand() < 0.30f)
					{
						DeltaMagnitude = FMath::Max(DeltaMagnitude, 2);
					}

					const int32 SplitHeightKey = ClampHeightKeyToPreset(RegionKey + DeltaKey * DeltaMagnitude, Preset);
					if (SplitHeightKey == RegionKey)
					{
						continue;
					}

					int32 ChangedCells = 0;
					for (const int32 RegionCellIdx : RegionCells)
					{
						const FT66PlatformNode& Cell = Platforms[RegionCellIdx];
						const bool bOnSplitSide = bSplitByRows ? (Cell.GridRow > Pivot) : (Cell.GridCol > Pivot);
						if (!bOnSplitSide)
						{
							continue;
						}

						HeightKeys[RegionCellIdx] = SplitHeightKey;
						++ChangedCells;
					}

					if (ChangedCells < 2)
					{
						for (const int32 RegionCellIdx : RegionCells)
						{
							const FT66PlatformNode& Cell = Platforms[RegionCellIdx];
							const bool bOnSplitSide = bSplitByRows ? (Cell.GridRow < Pivot) : (Cell.GridCol < Pivot);
							if (!bOnSplitSide)
							{
								continue;
							}

							HeightKeys[RegionCellIdx] = SplitHeightKey;
							++ChangedCells;
						}
					}

					bChanged |= (ChangedCells >= 2);
				}

				if (!bChanged)
				{
					break;
				}
			}
		};

		BreakUpOversizedFarmPlateaus(4, 10, 4);

		for (int32 R = 0; R < GridSize; ++R)
		{
			for (int32 C = 0; C < GridSize; ++C)
			{
				Platforms[GridIdx(R, C, GridSize)].TopZ = HeightValueFromKey(HeightKeys[GridIdx(R, C, GridSize)], Preset);
			}
		}
	}

	FIntPoint WorldToGridCell(const FVector2D& WorldPos, const FT66MapPreset& Preset, float CellSize, int32 GridSize)
	{
		const int32 Col = FMath::Clamp(
			FMath::FloorToInt((WorldPos.X + Preset.MapHalfExtent) / FMath::Max(CellSize, 1.f)),
			0,
			GridSize - 1);
		const int32 Row = FMath::Clamp(
			FMath::FloorToInt((WorldPos.Y + Preset.MapHalfExtent) / FMath::Max(CellSize, 1.f)),
			0,
			GridSize - 1);
		return FIntPoint(Row, Col);
	}

	bool DoesPlatformOverlapFarmGrowthKeepClear(const FT66PlatformNode& Platform)
	{
		const float HalfX = Platform.SizeX * 0.5f;
		const float HalfY = Platform.SizeY * 0.5f;
		return T66GameplayLayout::DoesRectOverlapTraversalKeepClearZone2D(
			Platform.Position.X - HalfX,
			Platform.Position.X + HalfX,
			Platform.Position.Y - HalfY,
			Platform.Position.Y + HalfY);
	}

	void GenerateFarmGrowthMap(FT66ProceduralMapResult& Result, const FT66MapPreset& Preset, int32 GridSize, float CellSize)
	{
		Result.Ramps.Reset();
		const int32 BaseKey = HeightKeyFromValue(Preset.BaselineZ, Preset);
		const int32 MaxKey = HeightKeyFromValue(Preset.ElevationMax, Preset);
		const float StepHeight = Preset.ElevationStep;

		struct FFarmGrowthCell
		{
			bool bOccupied = false;
			bool bSlope = false;
			int32 HeightKey = 0;
			float SurfaceStartZ = 0.f;
			ET66FarmCellShape Shape = ET66FarmCellShape::Flat;
			ET66FarmCellDecoration Decoration = ET66FarmCellDecoration::None;
			FVector DecorationLocalOffset = FVector::ZeroVector;
			FRotator DecorationLocalRotation = FRotator::ZeroRotator;
			FVector DecorationLocalScale = FVector(1.f, 1.f, 1.f);
		};

		auto MakeSlopeShapeFromGrowth = [&](const FIntPoint& GrowthDirection, int32 Delta) -> ET66FarmCellShape
		{
			const FIntPoint RiseDirection = (Delta >= 0)
				? GrowthDirection
				: FIntPoint(-GrowthDirection.X, -GrowthDirection.Y);

			if (RiseDirection.Y > 0) return ET66FarmCellShape::SlopePosX;
			if (RiseDirection.Y < 0) return ET66FarmCellShape::SlopeNegX;
			if (RiseDirection.X > 0) return ET66FarmCellShape::SlopePosY;
			return ET66FarmCellShape::SlopeNegY;
		};

		auto GetNeighborIndex = [&](int32 PlatformIndex, const FIntPoint& Direction) -> int32
		{
			if (!Result.Platforms.IsValidIndex(PlatformIndex))
			{
				return INDEX_NONE;
			}

			const FT66PlatformNode& Platform = Result.Platforms[PlatformIndex];
			const int32 NeighborRow = Platform.GridRow + Direction.X;
			const int32 NeighborCol = Platform.GridCol + Direction.Y;
			if (NeighborRow < 0 || NeighborRow >= GridSize || NeighborCol < 0 || NeighborCol >= GridSize)
			{
				return INDEX_NONE;
			}

			return GridIdx(NeighborRow, NeighborCol, GridSize);
		};

		auto TryGenerate = [&](int32 Seed, TArray<FFarmGrowthCell>& OutCells, int32& OutGeneratedCount) -> bool
		{
			FRandomStream Rng(Seed);
			OutCells.Reset();
			OutCells.SetNum(Result.Platforms.Num());
			OutGeneratedCount = 0;

			auto OccupyFlatCell = [&](int32 PlatformIndex, int32 HeightKey)
			{
				if (!OutCells.IsValidIndex(PlatformIndex))
				{
					return;
				}

				FFarmGrowthCell& Cell = OutCells[PlatformIndex];
				Cell.bOccupied = true;
				Cell.bSlope = false;
				Cell.HeightKey = ClampHeightKeyToPreset(HeightKey, Preset);
				Cell.SurfaceStartZ = HeightValueFromKey(Cell.HeightKey, Preset);
				Cell.Shape = ET66FarmCellShape::Flat;
				Cell.Decoration = ET66FarmCellDecoration::None;
			};

			auto OccupySlopeCell = [&](int32 PlatformIndex, int32 HeightKey, float EntrySurfaceZ, ET66FarmCellShape Shape)
			{
				if (!OutCells.IsValidIndex(PlatformIndex))
				{
					return;
				}

				FFarmGrowthCell& Cell = OutCells[PlatformIndex];
				Cell.bOccupied = true;
				Cell.bSlope = true;
				Cell.HeightKey = ClampHeightKeyToPreset(HeightKey, Preset);
				Cell.SurfaceStartZ = EntrySurfaceZ;
				Cell.Shape = Shape;
				Cell.Decoration = ET66FarmCellDecoration::None;
			};

			auto SpawnRandomDecoration = [&](FFarmGrowthCell& Cell)
			{
				const float ObjectToSpawn = Rng.FRand();
				const float HorizontalScale = CellSize / 20.f;
				const float VerticalScale = StepHeight / 12.f;

				if (ObjectToSpawn > 0.75f)
				{
					switch (Rng.RandRange(0, 2))
					{
					case 0: Cell.Decoration = ET66FarmCellDecoration::Tree1; break;
					case 1: Cell.Decoration = ET66FarmCellDecoration::Tree2; break;
					default: Cell.Decoration = ET66FarmCellDecoration::Tree3; break;
					}

					Cell.DecorationLocalOffset = FVector(
						Rng.FRandRange(-0.25f, 0.25f) * HorizontalScale,
						Rng.FRandRange(-0.25f, 0.25f) * HorizontalScale,
						0.25f * VerticalScale);
					Cell.DecorationLocalRotation = FRotator(90.f, Rng.FRandRange(0.f, 360.f), 0.f);
					Cell.DecorationLocalScale = FVector(0.25f);
					return;
				}

				if (ObjectToSpawn > 0.50f)
				{
					Cell.Decoration = (Rng.FRand() < 0.5f) ? ET66FarmCellDecoration::Rock : ET66FarmCellDecoration::Rocks;
					Cell.DecorationLocalOffset = FVector(
						Rng.FRandRange(-0.25f, 0.25f) * HorizontalScale,
						Rng.FRandRange(-0.25f, 0.25f) * HorizontalScale,
						0.25f * VerticalScale);
					Cell.DecorationLocalScale = FVector(Rng.FRandRange(0.25f, 0.50f));
					Cell.DecorationLocalRotation = FRotator(0.f, Rng.FRandRange(0.f, 360.f), 0.f);
					return;
				}

				if (ObjectToSpawn > 0.40f)
				{
					Cell.Decoration = ET66FarmCellDecoration::Log;
					Cell.DecorationLocalOffset = FVector(
						Rng.FRandRange(-0.25f, 0.25f) * HorizontalScale,
						Rng.FRandRange(-0.25f, 0.25f) * HorizontalScale,
						0.32f * VerticalScale);
					Cell.DecorationLocalScale = FVector(Rng.FRandRange(0.10f, 0.25f));
					Cell.DecorationLocalRotation = FRotator(0.f, Rng.FRandRange(0.f, 360.f), 0.f);
					return;
				}

				Cell.Decoration = ET66FarmCellDecoration::None;
				Cell.DecorationLocalOffset = FVector::ZeroVector;
				Cell.DecorationLocalRotation = FRotator::ZeroRotator;
				Cell.DecorationLocalScale = FVector(1.f, 1.f, 1.f);
			};

			auto HasUngeneratedNeighbor = [&](int32 PlatformIndex, const FIntPoint& Direction) -> bool
			{
				const int32 NeighborIndex = GetNeighborIndex(PlatformIndex, Direction);
				return NeighborIndex != INDEX_NONE && !OutCells[NeighborIndex].bOccupied;
			};

			auto GatherOpenDirections = [&](int32 PlatformIndex) -> TArray<FIntPoint>
			{
				TArray<FIntPoint> Directions;
				for (const FIntPoint& Direction : { FIntPoint(0, 1), FIntPoint(0, -1), FIntPoint(1, 0), FIntPoint(-1, 0) })
				{
					if (HasUngeneratedNeighbor(PlatformIndex, Direction))
					{
						Directions.Add(Direction);
					}
				}
				return Directions;
			};

			auto PickAnchor = [&]() -> int32
			{
				for (int32 Col = 0; Col < GridSize; ++Col)
				{
					for (int32 Row = 0; Row < GridSize; ++Row)
					{
						const int32 PlatformIndex = GridIdx(Row, Col, GridSize);
						if (!OutCells[PlatformIndex].bOccupied || OutCells[PlatformIndex].bSlope)
						{
							continue;
						}

						if (GatherOpenDirections(PlatformIndex).Num() > 0)
						{
							return PlatformIndex;
						}
					}
				}

				return INDEX_NONE;
			};

			auto CanCreateSlopeFrom = [&](int32 NewPlatformIndex, const FIntPoint& Direction) -> bool
			{
				const int32 ForwardIndex = GetNeighborIndex(NewPlatformIndex, Direction);
				return ForwardIndex != INDEX_NONE && !OutCells[ForwardIndex].bOccupied;
			};

			const int32 SeedRow = Rng.RandRange(0, GridSize - 1);
			const int32 SeedCol = Rng.RandRange(0, GridSize - 1);
			const int32 SeedIndex = GridIdx(SeedRow, SeedCol, GridSize);
			if (SeedIndex == INDEX_NONE)
			{
				return false;
			}

			OccupyFlatCell(SeedIndex, BaseKey);
			++OutGeneratedCount;

			int32 CurrentIndex = SeedIndex;
			FIntPoint LockedDirection = FIntPoint::ZeroValue;
			const float RaiseChance = FMath::Clamp(Preset.FarmHilliness / 5.0f, 0.0f, 1.0f);

			while (CurrentIndex != INDEX_NONE)
			{
				FIntPoint GrowthDirection = FIntPoint::ZeroValue;
				if (LockedDirection != FIntPoint::ZeroValue && HasUngeneratedNeighbor(CurrentIndex, LockedDirection))
				{
					GrowthDirection = LockedDirection;
				}
				else
				{
					const TArray<FIntPoint> OpenDirections = GatherOpenDirections(CurrentIndex);
					if (OpenDirections.Num() == 0)
					{
						CurrentIndex = PickAnchor();
						LockedDirection = FIntPoint::ZeroValue;
						continue;
					}

					GrowthDirection = OpenDirections[Rng.RandRange(0, OpenDirections.Num() - 1)];
				}

				if (GrowthDirection == FIntPoint::ZeroValue)
				{
					CurrentIndex = PickAnchor();
					LockedDirection = FIntPoint::ZeroValue;
					continue;
				}

				const int32 PreviousIndex = CurrentIndex;
				const int32 NewIndex = GetNeighborIndex(CurrentIndex, GrowthDirection);
				if (NewIndex == INDEX_NONE || OutCells[NewIndex].bOccupied)
				{
					LockedDirection = FIntPoint::ZeroValue;
					continue;
				}

				OccupyFlatCell(NewIndex, OutCells[PreviousIndex].HeightKey);
				++OutGeneratedCount;

				if (CanCreateSlopeFrom(NewIndex, GrowthDirection)
					&& OutCells[PreviousIndex].HeightKey < MaxKey
					&& Rng.FRand() < RaiseChance)
				{
					const int32 NewHeightKey = ClampHeightKeyToPreset(OutCells[PreviousIndex].HeightKey + 1, Preset);
					if (NewHeightKey > OutCells[PreviousIndex].HeightKey)
					{
						const float EntrySurfaceZ = HeightValueFromKey(OutCells[PreviousIndex].HeightKey, Preset);
						OccupySlopeCell(NewIndex, NewHeightKey, EntrySurfaceZ, MakeSlopeShapeFromGrowth(GrowthDirection, 1));
						LockedDirection = GrowthDirection;
						CurrentIndex = NewIndex;
						continue;
					}
				}

				SpawnRandomDecoration(OutCells[NewIndex]);
				LockedDirection = FIntPoint::ZeroValue;
				CurrentIndex = NewIndex;
			}

			return OutGeneratedCount == Result.Platforms.Num();
		};

		auto CommitCells = [&](const TArray<FFarmGrowthCell>& Cells)
		{
			for (int32 PlatformIndex = 0; PlatformIndex < Result.Platforms.Num(); ++PlatformIndex)
			{
				const FFarmGrowthCell* Cell = Cells.IsValidIndex(PlatformIndex) ? &Cells[PlatformIndex] : nullptr;
				FT66PlatformNode& Platform = Result.Platforms[PlatformIndex];
				if (Cell && Cell->bOccupied)
				{
					Platform.bFarmOccupied = true;
					Platform.FarmLevel = FMath::Max(Cell->HeightKey - BaseKey, 0);
					const float TileCenterZ = Preset.BaselineZ + static_cast<float>(Platform.FarmLevel) * StepHeight;
					Platform.TopZ = TileCenterZ + StepHeight * 0.5f;
					Platform.SurfaceStartZ = Cell->bSlope ? (TileCenterZ - StepHeight * 0.5f) : Platform.TopZ;
					Platform.FarmCellShape = Cell->Shape;
					Platform.FarmDecoration = Cell->Decoration;
					Platform.FarmDecorationLocalOffset = Cell->DecorationLocalOffset;
					Platform.FarmDecorationLocalRotation = Cell->DecorationLocalRotation;
					Platform.FarmDecorationLocalScale = Cell->DecorationLocalScale;
				}
				else
				{
					Platform.bFarmOccupied = false;
					Platform.FarmLevel = 0;
					Platform.TopZ = Preset.BaselineZ + StepHeight * 0.5f;
					Platform.SurfaceStartZ = Platform.TopZ;
					Platform.FarmCellShape = ET66FarmCellShape::Flat;
					Platform.FarmDecoration = ET66FarmCellDecoration::None;
					Platform.FarmDecorationLocalOffset = FVector::ZeroVector;
					Platform.FarmDecorationLocalRotation = FRotator::ZeroRotator;
					Platform.FarmDecorationLocalScale = FVector(1.f, 1.f, 1.f);
				}
			}
		};

		TArray<FFarmGrowthCell> Cells;
		int32 GeneratedCount = 0;
		TryGenerate(Preset.Seed, Cells, GeneratedCount);
		CommitCells(Cells);
		if (GeneratedCount != Result.Platforms.Num())
		{
			UE_LOG(LogT66, Warning,
				TEXT("[MAP] Farm growth produced %d / %d occupied cells for seed %d."),
				GeneratedCount,
				Result.Platforms.Num(),
				Preset.Seed);
		}
	}

	// ========================================================================
	// Region-based height assignment (replaces lane features)
	// ========================================================================

	void GenerateRegionHeights(TArray<FT66PlatformNode>& Platforms, const FT66MapPreset& Preset, int32 GridSize)
	{
		if (Preset.Theme == ET66MapTheme::Farm)
		{
			GenerateFarmPlateauHeights(Platforms, Preset, GridSize);
			return;
		}

		FRandomStream Rng(Preset.Seed);

		const TArray<float> HeightLevels = BuildHeightLevels(Preset);
		const int32 NumHeightLevels = HeightLevels.Num();

		const int32 SeedCount = Rng.RandRange(Preset.RegionSeedCountMin, Preset.RegionSeedCountMax);
		const int32 MaxSeedsPerLevel = 2;
		const int32 RequiredDistinctLevels = FMath::Min(FMath::Min(NumHeightLevels, SeedCount), 5);

		struct FSeedInfo
		{
			int32 Row;
			int32 Col;
			float Height;
		};

		TArray<FSeedInfo> Seeds;
		Seeds.Reserve(SeedCount);

		const float MinSeedSpacing = static_cast<float>(GridSize) * 0.28f;

		for (int32 I = 0; I < SeedCount; ++I)
		{
			FIntPoint Cell(1, 1);
			bool bFound = false;

			for (int32 Attempt = 0; Attempt < 60 && !bFound; ++Attempt)
			{
				const FIntPoint Candidate(
					Rng.RandRange(1, GridSize - 2),
					Rng.RandRange(1, GridSize - 2));

				bool bFarEnough = true;
				for (const FSeedInfo& Existing : Seeds)
				{
					const float Dist = FVector2D::Distance(
						FVector2D(Candidate.X, Candidate.Y),
						FVector2D(Existing.Row, Existing.Col));
					if (Dist < MinSeedSpacing)
					{
						bFarEnough = false;
						break;
					}
				}

				if (bFarEnough)
				{
					Cell = Candidate;
					bFound = true;
				}
			}

			if (!bFound)
			{
				Cell = FIntPoint(Rng.RandRange(1, GridSize - 2), Rng.RandRange(1, GridSize - 2));
			}

			Seeds.Add({ Cell.X, Cell.Y, 0.f });
		}

		// Assign heights: ensure multiple distinct levels while still honoring theme bias.
		TArray<int32> HeightUsage;
		HeightUsage.Init(0, NumHeightLevels);
		TSet<int32> UsedLevelIndices;

		for (int32 I = 0; I < Seeds.Num(); ++I)
		{
			TArray<int32> Available;
			for (int32 L = 0; L < NumHeightLevels; ++L)
			{
				if (HeightUsage[L] < MaxSeedsPerLevel)
				{
					Available.Add(L);
				}
			}

			if (UsedLevelIndices.Num() < RequiredDistinctLevels)
			{
				TArray<int32> Unused;
				for (int32 L : Available)
				{
					if (!UsedLevelIndices.Contains(L))
					{
						Unused.Add(L);
					}
				}
				if (Unused.Num() > 0)
				{
					Available = MoveTemp(Unused);
				}
			}

			if (Available.Num() == 0)
			{
				Available.Add(Rng.RandRange(0, NumHeightLevels - 1));
			}

			int32 ChosenIdx = Available[0];
			float BestScore = TNumericLimits<float>::Lowest();
			for (const int32 CandidateIdx : Available)
			{
				const float Score = GetLevelSelectionScore(HeightLevels, CandidateIdx, Preset, HeightUsage[CandidateIdx], Rng);
				if (Score > BestScore)
				{
					BestScore = Score;
					ChosenIdx = CandidateIdx;
				}
			}

			Seeds[I].Height = HeightLevels[ChosenIdx];
			HeightUsage[ChosenIdx]++;
			UsedLevelIndices.Add(ChosenIdx);
		}

		const FVector2D GridCenter(static_cast<float>(GridSize - 1) * 0.5f, static_cast<float>(GridSize - 1) * 0.5f);
		const float MaxRadius = FMath::Max(GridCenter.Size(), 1.f);

		// Assign every cell to the closest seed, then layer deterministic stepped noise on top so the
		// terrain reads like rolling, broken hills instead of a few giant plateaus.
		for (int32 R = 0; R < GridSize; ++R)
		{
			for (int32 C = 0; C < GridSize; ++C)
			{
				float BestDist = TNumericLimits<float>::Max();
				float BestHeight = Preset.BaselineZ;

				for (const FSeedInfo& S : Seeds)
				{
					const float DR = static_cast<float>(R - S.Row);
					const float DC = static_cast<float>(C - S.Col);
					const float Dist = FMath::Sqrt(DR * DR + DC * DC);
					if (Dist < BestDist)
					{
						BestDist = Dist;
						BestHeight = S.Height;
					}
				}

				const float GridX = static_cast<float>(C);
				const float GridY = static_cast<float>(R);
				const float MacroNoise = SampleSeededGridNoise(Preset.Seed, GridX, GridY, Preset.MacroNoiseScaleCells, 0x4D414352);
				const float DetailNoise = SampleSeededGridNoise(Preset.Seed, GridX, GridY, Preset.DetailNoiseScaleCells, 0x4445544C);
				const float RidgeNoise = SampleRidgedGridNoise(Preset.Seed, GridX, GridY, Preset.RidgeNoiseScaleCells, 0x52494447);
				const float RadiusAlpha = FVector2D::Distance(FVector2D(GridX, GridY), GridCenter) / MaxRadius;
				const float VisibilityBand = FMath::Clamp(1.f - FMath::Abs(RadiusAlpha - 0.58f) / 0.28f, 0.f, 1.f);
				const float OcclusionLift = VisibilityBand * FMath::Clamp((RidgeNoise - 0.18f) / 0.82f, 0.f, 1.f) * Preset.OcclusionRidgeSteps;
				const float NoiseSteps =
					MacroNoise * Preset.MacroNoiseSteps +
					DetailNoise * Preset.DetailNoiseSteps +
					(RidgeNoise * 2.f - 1.f) * Preset.RidgeNoiseSteps +
					OcclusionLift;
				const float NoisyHeight = BestHeight + NoiseSteps * Preset.ElevationStep;

				Platforms[GridIdx(R, C, GridSize)].TopZ =
					QuantizeToStep(FMath::Clamp(NoisyHeight, Preset.ElevationMin, Preset.ElevationMax), Preset);
			}
		}
	}

	// ========================================================================
	// Outer ring: force boundary cells to max height for natural bowl/arena
	// ========================================================================

	void ForceOuterRingToMaxHeight(TArray<FT66PlatformNode>& Platforms, const FT66MapPreset& Preset, int32 GridSize)
	{
		for (int32 R = 0; R < GridSize; ++R)
		{
			for (int32 C = 0; C < GridSize; ++C)
			{
				if (R == 0 || R == GridSize - 1 || C == 0 || C == GridSize - 1)
				{
					Platforms[GridIdx(R, C, GridSize)].TopZ = Preset.ElevationMax;
				}
			}
		}
	}

	int32 FlattenReservedTraversalZones(TArray<FT66PlatformNode>& Platforms, const FT66MapPreset& Preset)
	{
		int32 FlattenedCount = 0;

		for (FT66PlatformNode& Platform : Platforms)
		{
			const float HalfX = Platform.SizeX * 0.5f;
			const float HalfY = Platform.SizeY * 0.5f;
			const float MinX = Platform.Position.X - HalfX;
			const float MaxX = Platform.Position.X + HalfX;
			const float MinY = Platform.Position.Y - HalfY;
			const float MaxY = Platform.Position.Y + HalfY;

			if (!T66GameplayLayout::DoesRectOverlapTraversalKeepClearZone2D(MinX, MaxX, MinY, MaxY))
			{
				continue;
			}

			const float TargetHeight = Preset.BaselineZ;

			if (!FMath::IsNearlyEqual(Platform.TopZ, TargetHeight))
			{
				Platform.TopZ = FMath::Clamp(TargetHeight, Preset.ElevationMin, Preset.ElevationMax);
				++FlattenedCount;
			}
		}

		return FlattenedCount;
	}

	// ========================================================================
	// Adjacency clamping
	// ========================================================================

	void ClampAdjacency(TArray<FT66PlatformNode>& Platforms, const FT66MapPreset& Preset, int32 GridSize, int32 MaxIter = 24)
	{
		const float MaxDelta = FMath::Max(Preset.ElevationStep, 1.f) * FMath::Max(Preset.MaxAdjacentSteps, 1);
		for (int32 Iter = 0; Iter < MaxIter; ++Iter)
		{
			bool bChanged = false;

			for (int32 R = 0; R < GridSize; ++R)
			{
				for (int32 C = 0; C < GridSize; ++C)
				{
					const int32 Idx = GridIdx(R, C, GridSize);
					float NewH = Platforms[Idx].TopZ;

					for (const FIntPoint Off : { FIntPoint(-1, 0), FIntPoint(1, 0), FIntPoint(0, -1), FIntPoint(0, 1) })
					{
						const int32 NR = R + Off.X;
						const int32 NC = C + Off.Y;
						if (NR < 0 || NR >= GridSize || NC < 0 || NC >= GridSize)
						{
							continue;
						}

						const float NeighborH = Platforms[GridIdx(NR, NC, GridSize)].TopZ;
						if (NewH > NeighborH + MaxDelta)
						{
							NewH = NeighborH + MaxDelta;
						}
						else if (NewH < NeighborH - MaxDelta)
						{
							NewH = NeighborH - MaxDelta;
						}
					}

					NewH = QuantizeToStep(FMath::Clamp(NewH, Preset.ElevationMin, Preset.ElevationMax), Preset);
					if (!FMath::IsNearlyEqual(NewH, Platforms[Idx].TopZ))
					{
						Platforms[Idx].TopZ = NewH;
						bChanged = true;
					}
				}
			}

			if (!bChanged)
			{
				break;
			}
		}
	}

	bool DoesPlatformOverlapReservedTraversal(const FT66PlatformNode& Platform)
	{
		const float HalfX = Platform.SizeX * 0.5f;
		const float HalfY = Platform.SizeY * 0.5f;
		return T66GameplayLayout::DoesRectOverlapTraversalKeepClearZone2D(
			Platform.Position.X - HalfX,
			Platform.Position.X + HalfX,
			Platform.Position.Y - HalfY,
			Platform.Position.Y + HalfY);
	}

	bool DoesRectOverlapProtectedSpawnZones(float MinX, float MaxX, float MinY, float MaxY, float Margin = 0.f)
	{
		auto OverlapsRect = [=](float ZoneMinX, float ZoneMaxX, float ZoneMinY, float ZoneMaxY) -> bool
		{
			return T66GameplayLayout::DoAxisRangesOverlap(MinX, MaxX, ZoneMinX - Margin, ZoneMaxX + Margin)
				&& T66GameplayLayout::DoAxisRangesOverlap(MinY, MaxY, ZoneMinY - Margin, ZoneMaxY + Margin);
		};

		return OverlapsRect(T66GameplayLayout::StartPartitionWestX, T66GameplayLayout::StartAreaEastX,
				-T66GameplayLayout::AreaHalfHeightY, T66GameplayLayout::AreaHalfHeightY)
			|| OverlapsRect(T66GameplayLayout::BossAreaWestX, T66GameplayLayout::BossPartitionEastX,
				-T66GameplayLayout::AreaHalfHeightY, T66GameplayLayout::AreaHalfHeightY);
	}

	bool DoesRampOverlapProtectedSpawnZones(
		const FT66ProceduralMapResult& Result,
		const FT66RampEdge& Edge,
		float Padding)
	{
		const FBox2D Footprint = BuildRampFootprint(Result, Edge, Padding);
		if (!Footprint.bIsValid)
		{
			return false;
		}

		return DoesRectOverlapProtectedSpawnZones(
			Footprint.Min.X,
			Footprint.Max.X,
			Footprint.Min.Y,
			Footprint.Max.Y,
			Padding)
			|| T66GameplayLayout::DoesRectOverlapTraversalKeepClearZone2D(
			Footprint.Min.X,
			Footprint.Max.X,
			Footprint.Min.Y,
			Footprint.Max.Y,
			Padding);
	}

	int32 QuantizedHeightKey(float Height, const FT66MapPreset& Preset)
	{
		const float Step = FMath::Max(Preset.ElevationStep, 1.f);
		return FMath::RoundToInt((Height - Preset.BaselineZ) / Step);
	}

	float HeightFromQuantizedKey(int32 Key, const FT66MapPreset& Preset)
	{
		const float RawHeight = Preset.BaselineZ + static_cast<float>(Key) * FMath::Max(Preset.ElevationStep, 1.f);
		return FMath::Clamp(RawHeight, Preset.ElevationMin, Preset.ElevationMax);
	}

	void CollapseTinyRegions(TArray<FT66PlatformNode>& Platforms, const FT66MapPreset& Preset, int32 GridSize, int32 MaxPasses = 3)
	{
		const float MaxDelta = FMath::Max(Preset.ElevationStep, 1.f) * FMath::Max(Preset.MaxAdjacentSteps, 1);
		const int32 MaxCellsPerTinyRegion = (Preset.Theme == ET66MapTheme::Farm) ? 1 : 3;

		for (int32 Pass = 0; Pass < MaxPasses; ++Pass)
		{
			TArray<int32> RegionId;
			RegionId.Init(-1, Platforms.Num());
			bool bChanged = false;
			int32 NextRegionId = 0;

			for (int32 StartIdx = 0; StartIdx < Platforms.Num(); ++StartIdx)
			{
				if (RegionId[StartIdx] >= 0)
				{
					continue;
				}

				const float RegionHeight = Platforms[StartIdx].TopZ;
				TArray<int32> RegionCells;
				TArray<int32> Queue;
				Queue.Add(StartIdx);
				RegionId[StartIdx] = NextRegionId;

				bool bTouchesReservedTraversal = false;
				bool bTouchesOuterEdge = false;
				TMap<int32, int32> NeighborHeightCounts;

				for (int32 Head = 0; Head < Queue.Num(); ++Head)
				{
					const int32 CurIdx = Queue[Head];
					RegionCells.Add(CurIdx);
					const FT66PlatformNode& Cell = Platforms[CurIdx];

					bTouchesReservedTraversal |= DoesPlatformOverlapReservedTraversal(Cell);
					bTouchesOuterEdge |= (Cell.GridRow == 0 || Cell.GridRow == GridSize - 1 || Cell.GridCol == 0 || Cell.GridCol == GridSize - 1);

					auto VisitNeighbor = [&](int32 NR, int32 NC)
					{
						if (NR < 0 || NR >= GridSize || NC < 0 || NC >= GridSize)
						{
							return;
						}

						const int32 NIdx = GridIdx(NR, NC, GridSize);
						if (FMath::IsNearlyEqual(Platforms[NIdx].TopZ, RegionHeight))
						{
							if (RegionId[NIdx] < 0)
							{
								RegionId[NIdx] = NextRegionId;
								Queue.Add(NIdx);
							}
							return;
						}

						const int32 HeightKey = QuantizedHeightKey(Platforms[NIdx].TopZ, Preset);
						NeighborHeightCounts.FindOrAdd(HeightKey)++;
					};

					VisitNeighbor(Cell.GridRow - 1, Cell.GridCol);
					VisitNeighbor(Cell.GridRow + 1, Cell.GridCol);
					VisitNeighbor(Cell.GridRow, Cell.GridCol - 1);
					VisitNeighbor(Cell.GridRow, Cell.GridCol + 1);
				}

				++NextRegionId;

				if (RegionCells.Num() > MaxCellsPerTinyRegion || bTouchesReservedTraversal || bTouchesOuterEdge || NeighborHeightCounts.Num() == 0)
				{
					continue;
				}

				int32 BestHeightKey = 0;
				int32 BestNeighborCount = 0;
				for (const TPair<int32, int32>& Pair : NeighborHeightCounts)
				{
					if (Pair.Value > BestNeighborCount)
					{
						BestHeightKey = Pair.Key;
						BestNeighborCount = Pair.Value;
					}
				}

				if (BestNeighborCount < 2)
				{
					continue;
				}

				const float TargetHeight = HeightFromQuantizedKey(BestHeightKey, Preset);
				if (FMath::Abs(TargetHeight - RegionHeight) > MaxDelta + KINDA_SMALL_NUMBER)
				{
					continue;
				}

				for (const int32 RegionCellIdx : RegionCells)
				{
					Platforms[RegionCellIdx].TopZ = TargetHeight;
				}
				bChanged = true;
			}

			if (!bChanged)
			{
				break;
			}
		}
	}

	// ========================================================================
	// Neighbor connections (grid adjacency)
	// ========================================================================

	void BuildNeighborConnections(FT66ProceduralMapResult& Result, int32 GridSize)
	{
		for (int32 R = 0; R < GridSize; ++R)
		{
			for (int32 C = 0; C < GridSize; ++C)
			{
				const int32 Idx = GridIdx(R, C, GridSize);
				if (!Result.Platforms[Idx].bFarmOccupied)
				{
					continue;
				}
				if (C + 1 < GridSize)
				{
					const int32 Right = GridIdx(R, C + 1, GridSize);
					if (Result.Platforms[Right].bFarmOccupied)
					{
						Result.Platforms[Idx].Connections.AddUnique(Right);
						Result.Platforms[Right].Connections.AddUnique(Idx);
					}
				}
				if (R + 1 < GridSize)
				{
					const int32 Up = GridIdx(R + 1, C, GridSize);
					if (Result.Platforms[Up].bFarmOccupied)
					{
						Result.Platforms[Idx].Connections.AddUnique(Up);
						Result.Platforms[Up].Connections.AddUnique(Idx);
					}
				}
			}
		}
	}

	// ========================================================================
	// Ramp creation helper
	// ========================================================================

	int32 CountContiguousSupportCells(
		const FT66ProceduralMapResult& Result,
		int32 GridSize,
		int32 StartIdx,
		int32 StepRow,
		int32 StepCol,
		float RequiredTopZ)
	{
		if (StepRow == 0 && StepCol == 0)
		{
			return 0;
		}

		int32 Count = 0;
		int32 Row = Result.Platforms[StartIdx].GridRow;
		int32 Col = Result.Platforms[StartIdx].GridCol;
		while (Row >= 0 && Row < GridSize && Col >= 0 && Col < GridSize)
		{
			const FT66PlatformNode& P = Result.Platforms[GridIdx(Row, Col, GridSize)];
			if (!FMath::IsNearlyEqual(P.TopZ, RequiredTopZ))
			{
				break;
			}

			++Count;
			Row += StepRow;
			Col += StepCol;
		}

		return Count;
	}

	float ComputeMaxSupportedRampDepth(
		const FT66ProceduralMapResult& Result,
		int32 GridSize,
		float CellSize,
		const TArray<TPair<int32, int32>>& SpanPairs,
		bool bAlongX)
	{
		float MaxSupportedDepth = TNumericLimits<float>::Max();

		for (const TPair<int32, int32>& Pair : SpanPairs)
		{
			const FT66PlatformNode& A = Result.Platforms[Pair.Key];
			const FT66PlatformNode& B = Result.Platforms[Pair.Value];
			const int32 LowerIdx = (A.TopZ <= B.TopZ) ? Pair.Key : Pair.Value;
			const int32 HigherIdx = (A.TopZ <= B.TopZ) ? Pair.Value : Pair.Key;
			const FT66PlatformNode& Lower = Result.Platforms[LowerIdx];
			const FT66PlatformNode& Higher = Result.Platforms[HigherIdx];

			const int32 StepRow = bAlongX ? 0 : FMath::Clamp(Lower.GridRow - Higher.GridRow, -1, 1);
			const int32 StepCol = bAlongX ? FMath::Clamp(Lower.GridCol - Higher.GridCol, -1, 1) : 0;
			const int32 SupportCells = CountContiguousSupportCells(Result, GridSize, LowerIdx, StepRow, StepCol, Lower.TopZ);
			MaxSupportedDepth = FMath::Min(MaxSupportedDepth, static_cast<float>(SupportCells) * CellSize);
		}

		return MaxSupportedDepth;
	}

	void AddRampForBoundary(
		FT66ProceduralMapResult& Result,
		const FT66MapPreset& Preset,
		int32 GridSize,
		float CellSize,
		int32 AIdx,
		int32 BIdx,
		bool bAlongX,
		int32 BoundaryEdgeCount,
		FRandomStream& Rng,
		TSet<uint64>& Used)
	{
		const uint64 Key = MakeBoundaryKey(AIdx, BIdx);
		if (Used.Contains(Key))
		{
			return;
		}

		const FT66PlatformNode& A = Result.Platforms[AIdx];
		const FT66PlatformNode& B = Result.Platforms[BIdx];
		if (IsPlatformOnOuterRing(A, GridSize) || IsPlatformOnOuterRing(B, GridSize))
		{
			return;
		}

		const float HeightDiff = FMath::Abs(A.TopZ - B.TopZ);
		if (HeightDiff < 50.f)
		{
			return;
		}

		FT66RampEdge Edge;
		Edge.LowerIndex = (A.TopZ <= B.TopZ) ? AIdx : BIdx;
		Edge.HigherIndex = (A.TopZ <= B.TopZ) ? BIdx : AIdx;
		Edge.bAlongX = bAlongX;

		TArray<TPair<int32, int32>> SupportPairs;
		SupportPairs.Add(TPair<int32, int32>(AIdx, BIdx));
		const float SupportInset = CellSize * 0.08f;
		const float MaxSupportedDepth = FMath::Max(CellSize * 0.82f, ComputeMaxSupportedRampDepth(Result, GridSize, CellSize, SupportPairs, bAlongX) - SupportInset);
		if (MaxSupportedDepth < CellSize * 0.78f)
		{
			return;
		}

		const float RequiredSlopeDepth = GetRequiredRampDepthForHeight(HeightDiff);
		if (RequiredSlopeDepth > MaxSupportedDepth + KINDA_SMALL_NUMBER)
		{
			return;
		}

		const float HeightDrivenDepth = HeightDiff * FMath::Max(Preset.RampRunPerRise, 1.f);
		const float BoundarySpanAlpha = FMath::Clamp((static_cast<float>(BoundaryEdgeCount) - 1.f) / 6.f, 0.f, 1.f);
		const float DesiredDepth = FMath::Clamp(
			HeightDrivenDepth * Rng.FRandRange(
				FMath::Lerp(0.90f, 1.00f, BoundarySpanAlpha),
				FMath::Lerp(1.25f, 1.60f, BoundarySpanAlpha)),
			FMath::Max(Preset.RampDepthMin, RequiredSlopeDepth),
			FMath::Lerp(Preset.RampDepthMax, Preset.RampDepthMax * 1.25f, BoundarySpanAlpha));
		Edge.Depth = FMath::Min(DesiredDepth, MaxSupportedDepth);
		if (Edge.Depth < CellSize * 0.78f)
		{
			return;
		}

		if (GetRampSlopeAngleDegrees(HeightDiff, Edge.Depth) > GetMaxClimbableRampAngleDegrees() + 0.25f)
		{
			return;
		}

		const float HeightAlpha = FMath::Clamp(
			HeightDiff / FMath::Max(FMath::Abs(Preset.ElevationMax - Preset.ElevationMin), 1.f),
			0.f,
			1.f);
		const float WidthProfileRoll = Rng.FRand();
		float WidthBlend = 0.f;
		if (WidthProfileRoll < 0.28f)
		{
			WidthBlend = Rng.FRandRange(0.05f, 0.28f);
		}
		else if (WidthProfileRoll < 0.73f)
		{
			WidthBlend = Rng.FRandRange(0.34f, 0.67f);
		}
		else
		{
			WidthBlend = Rng.FRandRange(0.72f, 1.0f);
		}
		WidthBlend = FMath::Clamp(WidthBlend + HeightAlpha * 0.12f + BoundarySpanAlpha * 0.18f + Rng.FRandRange(-0.10f, 0.10f), 0.f, 1.f);
		const float WidthAlpha = FMath::Lerp(Preset.RampWidthMinAlpha, Preset.RampWidthMaxAlpha, WidthBlend);
		Edge.Width = FMath::Clamp(
			CellSize * WidthAlpha * Rng.FRandRange(0.82f, 1.22f),
			CellSize * 0.12f,
			CellSize * 0.96f);

		const float MaxOffset = FMath::Max((CellSize - Edge.Width) * 0.5f, 0.f);
		const bool bAllowOffset = BoundaryEdgeCount >= 3 && Edge.Width >= CellSize * 0.72f;
		Edge.PerpOffset = (bAllowOffset && MaxOffset > KINDA_SMALL_NUMBER)
			? Rng.FRandRange(-MaxOffset * 0.35f, MaxOffset * 0.35f)
			: 0.f;
		Edge.bVisualOwner = true;

		if (DoesRampOverlapProtectedSpawnZones(Result, Edge, CellSize * 0.04f))
		{
			return;
		}

		if (DoesRampOverlapOuterRing(Result, Edge, Preset, CellSize, CellSize * 0.06f))
		{
			return;
		}

		Result.Ramps.Add(Edge);
		Used.Add(Key);
	}

	void AddRampForBoundarySpan(
		FT66ProceduralMapResult& Result,
		const FT66MapPreset& Preset,
		int32 GridSize,
		float CellSize,
		const TArray<TPair<int32, int32>>& SpanPairs,
		bool bAlongX,
		FRandomStream& Rng,
		TSet<uint64>& Used)
	{
		if (SpanPairs.Num() == 0)
		{
			return;
		}

		const int32 OwnerPairIndex = SpanPairs.Num() / 2;
		const int32 OwnerAIdx = SpanPairs[OwnerPairIndex].Key;
		const int32 OwnerBIdx = SpanPairs[OwnerPairIndex].Value;
		const FT66PlatformNode& OwnerA = Result.Platforms[OwnerAIdx];
		const FT66PlatformNode& OwnerB = Result.Platforms[OwnerBIdx];
		const float HeightDiff = FMath::Abs(OwnerA.TopZ - OwnerB.TopZ);
		if (HeightDiff < 50.f)
		{
			return;
		}

		float GlobalPerpCenter = 0.f;
		for (const TPair<int32, int32>& Pair : SpanPairs)
		{
			const FT66PlatformNode& A = Result.Platforms[Pair.Key];
			const FT66PlatformNode& B = Result.Platforms[Pair.Value];
			GlobalPerpCenter += bAlongX
				? (A.Position.Y + B.Position.Y) * 0.5f
				: (A.Position.X + B.Position.X) * 0.5f;
		}
		GlobalPerpCenter /= static_cast<float>(SpanPairs.Num());

		const float SupportInset = CellSize * 0.08f;
		const float MaxSupportedDepth = FMath::Max(CellSize * 0.82f, ComputeMaxSupportedRampDepth(Result, GridSize, CellSize, SpanPairs, bAlongX) - SupportInset);
		if (MaxSupportedDepth < CellSize * 0.78f)
		{
			return;
		}

		const float RequiredSlopeDepth = GetRequiredRampDepthForHeight(HeightDiff);
		if (RequiredSlopeDepth > MaxSupportedDepth + KINDA_SMALL_NUMBER)
		{
			return;
		}

		const float HeightDrivenDepth = HeightDiff * FMath::Max(Preset.RampRunPerRise, 1.f);
		const float SpanAlpha = FMath::Clamp((static_cast<float>(SpanPairs.Num()) - 1.f) / 5.f, 0.f, 1.f);
		const float DesiredDepth = FMath::Clamp(
			HeightDrivenDepth * Rng.FRandRange(
				FMath::Lerp(0.95f, 1.10f, SpanAlpha),
				FMath::Lerp(1.35f, 1.85f, SpanAlpha)),
			FMath::Max(Preset.RampDepthMin, RequiredSlopeDepth),
			FMath::Lerp(Preset.RampDepthMax, Preset.RampDepthMax * 1.35f, SpanAlpha));
		const float Depth = FMath::Min(DesiredDepth, MaxSupportedDepth);
		if (Depth < CellSize * 0.78f)
		{
			return;
		}

		if (GetRampSlopeAngleDegrees(HeightDiff, Depth) > GetMaxClimbableRampAngleDegrees() + 0.25f)
		{
			return;
		}

		const float HeightAlpha = FMath::Clamp(
			HeightDiff / FMath::Max(FMath::Abs(Preset.ElevationMax - Preset.ElevationMin), 1.f),
			0.f,
			1.f);
		const float SpanWidth = CellSize * static_cast<float>(SpanPairs.Num());
		const float WidthAlpha = FMath::Clamp(0.25f + HeightAlpha * 0.18f + SpanAlpha * 0.82f, 0.f, 1.f);
		const float Width = FMath::Clamp(
			SpanWidth - CellSize * 0.08f,
			CellSize * FMath::Lerp(Preset.RampWidthMinAlpha, Preset.RampWidthMaxAlpha, WidthAlpha),
			SpanWidth);

		for (int32 PairIndex = 0; PairIndex < SpanPairs.Num(); ++PairIndex)
		{
			const TPair<int32, int32>& Pair = SpanPairs[PairIndex];
			const uint64 Key = MakeBoundaryKey(Pair.Key, Pair.Value);
			if (Used.Contains(Key))
			{
				return;
			}
		}

		for (int32 PairIndex = 0; PairIndex < SpanPairs.Num(); ++PairIndex)
		{
			const TPair<int32, int32>& Pair = SpanPairs[PairIndex];
			const FT66PlatformNode& A = Result.Platforms[Pair.Key];
			const FT66PlatformNode& B = Result.Platforms[Pair.Value];
			const float LocalPerpCenter = bAlongX
				? (A.Position.Y + B.Position.Y) * 0.5f
				: (A.Position.X + B.Position.X) * 0.5f;

			FT66RampEdge Edge;
			Edge.LowerIndex = (A.TopZ <= B.TopZ) ? Pair.Key : Pair.Value;
			Edge.HigherIndex = (A.TopZ <= B.TopZ) ? Pair.Value : Pair.Key;
			Edge.Width = Width;
			Edge.Depth = Depth;
			Edge.PerpOffset = GlobalPerpCenter - LocalPerpCenter;
			Edge.bVisualOwner = (PairIndex == OwnerPairIndex);
			Edge.bAlongX = bAlongX;
			Result.Ramps.Add(Edge);
			Used.Add(MakeBoundaryKey(Pair.Key, Pair.Value));
		}
	}

	FBox2D BuildRampFootprint(
		const FT66ProceduralMapResult& Result,
		const FT66RampEdge& Edge,
		float Padding)
	{
		const FT66PlatformNode& Lo = Result.Platforms[Edge.LowerIndex];
		const FT66PlatformNode& Hi = Result.Platforms[Edge.HigherIndex];
		const FVector2D Mid = 0.5f * (Lo.Position + Hi.Position);
		const float SeamOverlap = GetRampSeamOverlap(Result, Edge);
		const float HalfDepth = (Edge.Depth + SeamOverlap * 2.f) * 0.5f + Padding;
		const float HalfWidth = Edge.Width * 0.5f + Padding;

		FVector2D Min;
		FVector2D Max;
		if (Edge.bAlongX)
		{
			Min = FVector2D(Mid.X - HalfDepth, Mid.Y + Edge.PerpOffset - HalfWidth);
			Max = FVector2D(Mid.X + HalfDepth, Mid.Y + Edge.PerpOffset + HalfWidth);
		}
		else
		{
			Min = FVector2D(Mid.X + Edge.PerpOffset - HalfWidth, Mid.Y - HalfDepth);
			Max = FVector2D(Mid.X + Edge.PerpOffset + HalfWidth, Mid.Y + HalfDepth);
		}
		return FBox2D(Min, Max);
	}

	bool DoesRampOverlapOuterRing(
		const FT66ProceduralMapResult& Result,
		const FT66RampEdge& Edge,
		const FT66MapPreset& Preset,
		float CellSize,
		float Padding)
	{
		const FBox2D Footprint = BuildRampFootprint(Result, Edge, Padding);
		if (!Footprint.bIsValid)
		{
			return false;
		}

		const float InnerMin = -Preset.MapHalfExtent + CellSize;
		const float InnerMax = Preset.MapHalfExtent - CellSize;
		return Footprint.Min.X < InnerMin
			|| Footprint.Max.X > InnerMax
			|| Footprint.Min.Y < InnerMin
			|| Footprint.Max.Y > InnerMax;
	}

	bool DoesRampOverlapAny(
		const FT66ProceduralMapResult& Result,
		const FT66RampEdge& Candidate,
		const TArray<FT66RampFootprint>& Accepted,
		float Padding)
	{
		const FBox2D CandidateBox = BuildRampFootprint(Result, Candidate, Padding);
		for (const FT66RampFootprint& Existing : Accepted)
		{
			if (CandidateBox.Intersect(Existing.Box))
			{
				return true;
			}
		}
		return false;
	}

	// ========================================================================
	// Region-aware ramp builder with multi-ramp boundaries and connectivity
	// ========================================================================

	void BuildRamps(FT66ProceduralMapResult& Result, const FT66MapPreset& Preset, int32 GridSize, float CellSize)
	{
		FRandomStream Rng(Preset.Seed ^ 0x4D617A65);
		TSet<uint64> Used;
		TArray<FT66RampFootprint> AcceptedFootprints;
		const float RampPadding = CellSize * 0.14f;
		const int32 MaxTotalRamps = 96;
		const auto MakeRegionBoundaryKey = [](int32 RegionA, int32 RegionB, bool bAlongX) -> uint64
		{
			const uint64 Low = static_cast<uint64>(FMath::Min(RegionA, RegionB));
			const uint64 High = static_cast<uint64>(FMath::Max(RegionA, RegionB));
			return (Low << 33) | (High << 1) | (bAlongX ? 1ull : 0ull);
		};

		// --- Phase 1: discover connected components (regions) of same-height cells ---

		TArray<int32> RegionID;
		RegionID.Init(-1, Result.Platforms.Num());
		int32 NumRegions = 0;

		struct FRegionInfo
		{
			TArray<int32> Cells;
			float Height = 0.f;
			bool bTouchesOuterEdge = false;
		};
		TArray<FRegionInfo> Regions;

		for (int32 StartIdx = 0; StartIdx < Result.Platforms.Num(); ++StartIdx)
		{
			if (RegionID[StartIdx] >= 0)
			{
				continue;
			}

			const float Height = Result.Platforms[StartIdx].TopZ;
			const int32 ThisRegion = NumRegions++;

			TArray<int32> Queue;
			Queue.Add(StartIdx);
			RegionID[StartIdx] = ThisRegion;

			FRegionInfo Info;
			Info.Height = Height;

			for (int32 Head = 0; Head < Queue.Num(); ++Head)
			{
				const int32 Cur = Queue[Head];
				Info.Cells.Add(Cur);
				const FT66PlatformNode& P = Result.Platforms[Cur];
				Info.bTouchesOuterEdge |= IsPlatformOnOuterRing(P, GridSize);

				auto TryNeighbor = [&](int32 NR, int32 NC)
				{
					if (NR < 0 || NR >= GridSize || NC < 0 || NC >= GridSize) return;
					const int32 NIdx = GridIdx(NR, NC, GridSize);
					if (RegionID[NIdx] >= 0) return;
					if (FMath::IsNearlyEqual(Result.Platforms[NIdx].TopZ, Height))
					{
						RegionID[NIdx] = ThisRegion;
						Queue.Add(NIdx);
					}
				};

				TryNeighbor(P.GridRow - 1, P.GridCol);
				TryNeighbor(P.GridRow + 1, P.GridCol);
				TryNeighbor(P.GridRow, P.GridCol - 1);
				TryNeighbor(P.GridRow, P.GridCol + 1);
			}

			Regions.Add(MoveTemp(Info));
		}

		// --- Phase 2: for each pair of adjacent regions, count shared boundary edges ---

		struct FRegionBoundary
		{
			int32 RegionA = -1;
			int32 RegionB = -1;
			TArray<TPair<int32, int32>> CellPairs; // (AIdx, BIdx)
			bool bAlongX = true;
		};

		struct FBoundaryRun
		{
			TArray<TPair<int32, int32>> CellPairs;
			int32 LineCoord = 0;
			int32 MinAlongCoord = 0;
			float SortJitter = 0.f;
		};

		struct FSortableBoundaryPair
		{
			TPair<int32, int32> CellPair;
			int32 LineCoord = 0;
			int32 AlongCoord = 0;
		};

		struct FRegionRampAccess
		{
			bool bHasUpRamp = false;
			bool bHasDownRamp = false;
			bool bHasHigherNeighbor = false;
			bool bHasLowerNeighbor = false;
		};

		TMap<uint64, FRegionBoundary> BoundaryMap;

		for (int32 R = 0; R < GridSize; ++R)
		{
			for (int32 C = 0; C < GridSize; ++C)
			{
				const int32 Idx = GridIdx(R, C, GridSize);
				const int32 MyRegion = RegionID[Idx];

				auto CheckNeighbor = [&](int32 NR, int32 NC, bool bAlongX)
				{
					if (NR < 0 || NR >= GridSize || NC < 0 || NC >= GridSize) return;
					const int32 NIdx = GridIdx(NR, NC, GridSize);
					const int32 OtherRegion = RegionID[NIdx];
					if (MyRegion == OtherRegion) return;
					if (FMath::Abs(Result.Platforms[Idx].TopZ - Result.Platforms[NIdx].TopZ) < 50.f) return;

					const uint64 PairKey = MakeRegionBoundaryKey(MyRegion, OtherRegion, bAlongX);
					FRegionBoundary& Boundary = BoundaryMap.FindOrAdd(PairKey);
					if (Boundary.RegionA < 0)
					{
						Boundary.RegionA = FMath::Min(MyRegion, OtherRegion);
						Boundary.RegionB = FMath::Max(MyRegion, OtherRegion);
						Boundary.bAlongX = bAlongX;
					}
					Boundary.CellPairs.Add(TPair<int32, int32>(Idx, NIdx));
				};

				if (C + 1 < GridSize) CheckNeighbor(R, C + 1, true);
				if (R + 1 < GridSize) CheckNeighbor(R + 1, C, false);
			}
		}

		auto BuildRunsForBoundary = [&](const FRegionBoundary& Boundary) -> TArray<FBoundaryRun>
		{
			TArray<FSortableBoundaryPair> SortedPairs;
			SortedPairs.Reserve(Boundary.CellPairs.Num());
			for (const TPair<int32, int32>& CellPair : Boundary.CellPairs)
			{
				const FT66PlatformNode& A = Result.Platforms[CellPair.Key];
				const FT66PlatformNode& B = Result.Platforms[CellPair.Value];
				FSortableBoundaryPair Sortable;
				Sortable.CellPair = CellPair;
				if (Boundary.bAlongX)
				{
					Sortable.LineCoord = FMath::Min(A.GridCol, B.GridCol);
					Sortable.AlongCoord = A.GridRow;
				}
				else
				{
					Sortable.LineCoord = FMath::Min(A.GridRow, B.GridRow);
					Sortable.AlongCoord = A.GridCol;
				}
				SortedPairs.Add(Sortable);
			}

			SortedPairs.Sort([](const FSortableBoundaryPair& A, const FSortableBoundaryPair& B)
			{
				return (A.LineCoord == B.LineCoord) ? (A.AlongCoord < B.AlongCoord) : (A.LineCoord < B.LineCoord);
			});

			TArray<FBoundaryRun> Runs;
			for (const FSortableBoundaryPair& Pair : SortedPairs)
			{
				FBoundaryRun* ActiveRun = (Runs.Num() > 0) ? &Runs.Last() : nullptr;
				if (!ActiveRun || ActiveRun->LineCoord != Pair.LineCoord || Pair.AlongCoord > ActiveRun->MinAlongCoord + ActiveRun->CellPairs.Num())
				{
					FBoundaryRun& NewRun = Runs.AddDefaulted_GetRef();
					NewRun.LineCoord = Pair.LineCoord;
					NewRun.MinAlongCoord = Pair.AlongCoord;
					NewRun.SortJitter = Rng.FRand();
					NewRun.CellPairs.Add(Pair.CellPair);
				}
				else
				{
					ActiveRun->CellPairs.Add(Pair.CellPair);
				}
			}

			Runs.Sort([](const FBoundaryRun& A, const FBoundaryRun& B)
			{
				const int32 SizeDelta = B.CellPairs.Num() - A.CellPairs.Num();
				if (SizeDelta != 0)
				{
					return SizeDelta < 0;
				}
				return A.SortJitter < B.SortJitter;
			});

			return Runs;
		};

		TFunction<bool(const FT66RampEdge&)> DoesRampHaveNaturalLandings;

		auto TryPlaceRampOnBoundary = [&](FRegionBoundary& Boundary, int32 DesiredRamps) -> int32
		{
			const TArray<FBoundaryRun> Runs = BuildRunsForBoundary(Boundary);
			int32 PlacedForBoundary = 0;

			for (const FBoundaryRun& Run : Runs)
			{
				if (PlacedForBoundary >= DesiredRamps) break;
				if (Result.Ramps.Num() >= MaxTotalRamps) break;
				if (Run.CellPairs.Num() == 0) continue;

				const int32 RunLen = Run.CellPairs.Num();
				int32 DesiredRunRamps = 1;
				if (RunLen >= 8)
				{
					DesiredRunRamps = 3;
				}
				else if (RunLen >= 5)
				{
					DesiredRunRamps = 2;
				}
				DesiredRunRamps = FMath::Min(DesiredRunRamps, DesiredRamps - PlacedForBoundary);

				TArray<float> PreferredAnchors;
				if (DesiredRunRamps == 1)
				{
					PreferredAnchors = {
						static_cast<float>(RunLen - 1) * 0.5f,
						static_cast<float>(RunLen - 1) / 3.f,
						static_cast<float>(RunLen - 1) * (2.f / 3.f)
					};
				}
				else if (DesiredRunRamps == 2)
				{
					PreferredAnchors = {
						static_cast<float>(RunLen - 1) / 3.f,
						static_cast<float>(RunLen - 1) * (2.f / 3.f),
						static_cast<float>(RunLen - 1) * 0.5f
					};
				}
				else
				{
					PreferredAnchors = {
						static_cast<float>(RunLen - 1) * 0.25f,
						static_cast<float>(RunLen - 1) * 0.5f,
						static_cast<float>(RunLen - 1) * 0.75f
					};
				}

				TArray<int32> CandidatePairIndices;
				CandidatePairIndices.Reserve(RunLen);
				for (int32 PairIndex = 0; PairIndex < RunLen; ++PairIndex)
				{
					CandidatePairIndices.Add(PairIndex);
				}

				CandidatePairIndices.Sort([&](int32 A, int32 B)
				{
					float BestAnchorDistA = TNumericLimits<float>::Max();
					float BestAnchorDistB = TNumericLimits<float>::Max();
					for (const float Anchor : PreferredAnchors)
					{
						BestAnchorDistA = FMath::Min(BestAnchorDistA, FMath::Abs(static_cast<float>(A) - Anchor));
						BestAnchorDistB = FMath::Min(BestAnchorDistB, FMath::Abs(static_cast<float>(B) - Anchor));
					}

					if (!FMath::IsNearlyEqual(BestAnchorDistA, BestAnchorDistB))
					{
						return BestAnchorDistA < BestAnchorDistB;
					}

					const float CenterBiasA = FMath::Abs(static_cast<float>(A) - static_cast<float>(RunLen - 1) * 0.5f);
					const float CenterBiasB = FMath::Abs(static_cast<float>(B) - static_cast<float>(RunLen - 1) * 0.5f);
					return CenterBiasA < CenterBiasB;
				});

				for (int32 PairIndex : CandidatePairIndices)
				{
					if (PlacedForBoundary >= DesiredRamps) break;
					if (Result.Ramps.Num() >= MaxTotalRamps) break;

					const TPair<int32, int32>& RampPair = Run.CellPairs[PairIndex];
					const uint64 PairKey = MakeBoundaryKey(RampPair.Key, RampPair.Value);
					if (Used.Contains(PairKey))
					{
						continue;
					}

					const int32 PrevNum = Result.Ramps.Num();
					AddRampForBoundary(Result, Preset, GridSize, CellSize, RampPair.Key, RampPair.Value, Boundary.bAlongX, RunLen, Rng, Used);
					if (Result.Ramps.Num() == PrevNum)
					{
						continue;
					}

					const FT66RampEdge& Candidate = Result.Ramps.Last();
					if (!DoesRampHaveNaturalLandings(Candidate))
					{
						Used.Remove(PairKey);
						Result.Ramps.SetNum(PrevNum);
						continue;
					}

					if (DoesRampOverlapAny(Result, Candidate, AcceptedFootprints, RampPadding))
					{
						Used.Remove(PairKey);
						Result.Ramps.SetNum(PrevNum);
						continue;
					}

					AcceptedFootprints.Add({ BuildRampFootprint(Result, Candidate, RampPadding) });
					++PlacedForBoundary;
				}
			}

			return PlacedForBoundary;
		};

		auto BuildRegionRampAccess = [&]() -> TArray<FRegionRampAccess>
		{
			TArray<FRegionRampAccess> Access;
			Access.SetNum(NumRegions);

			for (const FT66RampEdge& Edge : Result.Ramps)
			{
				const int32 LowerRegion = RegionID[Edge.LowerIndex];
				const int32 HigherRegion = RegionID[Edge.HigherIndex];
				if (LowerRegion == HigherRegion)
				{
					continue;
				}

				Access[LowerRegion].bHasUpRamp = true;
				Access[HigherRegion].bHasDownRamp = true;
			}

			for (const TPair<uint64, FRegionBoundary>& Pair : BoundaryMap)
			{
				const FRegionBoundary& Boundary = Pair.Value;
				if (!Regions.IsValidIndex(Boundary.RegionA) || !Regions.IsValidIndex(Boundary.RegionB))
				{
					continue;
				}

				const float HeightA = Regions[Boundary.RegionA].Height;
				const float HeightB = Regions[Boundary.RegionB].Height;
				if (HeightA < HeightB)
				{
					Access[Boundary.RegionA].bHasHigherNeighbor = true;
					Access[Boundary.RegionB].bHasLowerNeighbor = true;
				}
				else if (HeightB < HeightA)
				{
					Access[Boundary.RegionB].bHasHigherNeighbor = true;
					Access[Boundary.RegionA].bHasLowerNeighbor = true;
				}
			}

			return Access;
		};

		auto IsSameRegionHeightCell = [&](int32 Row, int32 Col, int32 RegionIndex, float RequiredHeight) -> bool
		{
			if (Row < 0 || Row >= GridSize || Col < 0 || Col >= GridSize)
			{
				return false;
			}

			const int32 CellIdx = GridIdx(Row, Col, GridSize);
			return RegionID[CellIdx] == RegionIndex
				&& FMath::IsNearlyEqual(Result.Platforms[CellIdx].TopZ, RequiredHeight);
		};

		auto CountLandingPadDepth = [&](int32 StartIdx, int32 RegionIndex, float RequiredHeight, int32 ForwardRow, int32 ForwardCol, int32 SideRow, int32 SideCol, int32 HalfWidthCells) -> int32
		{
			if (!Result.Platforms.IsValidIndex(StartIdx))
			{
				return 0;
			}

			const FT66PlatformNode& StartCell = Result.Platforms[StartIdx];
			int32 Depth = 0;
			for (int32 Layer = 0; Layer < GridSize; ++Layer)
			{
				bool bLayerValid = true;
				for (int32 SideOffset = -HalfWidthCells; SideOffset <= HalfWidthCells; ++SideOffset)
				{
					const int32 Row = StartCell.GridRow + Layer * ForwardRow + SideOffset * SideRow;
					const int32 Col = StartCell.GridCol + Layer * ForwardCol + SideOffset * SideCol;
					if (!IsSameRegionHeightCell(Row, Col, RegionIndex, RequiredHeight))
					{
						bLayerValid = false;
						break;
					}
				}

				if (!bLayerValid)
				{
					break;
				}

				++Depth;
			}

			return Depth;
		};

		auto HasLandingShoulder = [&](int32 StartIdx, int32 RegionIndex, float RequiredHeight, int32 ForwardRow, int32 ForwardCol, int32 SideRow, int32 SideCol) -> bool
		{
			if (!Result.Platforms.IsValidIndex(StartIdx))
			{
				return false;
			}

			const FT66PlatformNode& StartCell = Result.Platforms[StartIdx];
			for (int32 Layer = 0; Layer < 2; ++Layer)
			{
				const int32 BaseRow = StartCell.GridRow + Layer * ForwardRow;
				const int32 BaseCol = StartCell.GridCol + Layer * ForwardCol;
				if (IsSameRegionHeightCell(BaseRow + SideRow, BaseCol + SideCol, RegionIndex, RequiredHeight)
					|| IsSameRegionHeightCell(BaseRow - SideRow, BaseCol - SideCol, RegionIndex, RequiredHeight))
				{
					return true;
				}
			}

			return false;
		};

		DoesRampHaveNaturalLandings = [&](const FT66RampEdge& Edge) -> bool
		{
			if (!Result.Platforms.IsValidIndex(Edge.LowerIndex) || !Result.Platforms.IsValidIndex(Edge.HigherIndex))
			{
				return false;
			}

			const FT66PlatformNode& Lower = Result.Platforms[Edge.LowerIndex];
			const FT66PlatformNode& Higher = Result.Platforms[Edge.HigherIndex];
			const int32 LowerRegion = RegionID[Edge.LowerIndex];
			const int32 HigherRegion = RegionID[Edge.HigherIndex];
			if (LowerRegion < 0 || HigherRegion < 0 || LowerRegion == HigherRegion)
			{
				return false;
			}

			const int32 StepRow = FMath::Clamp(Higher.GridRow - Lower.GridRow, -1, 1);
			const int32 StepCol = FMath::Clamp(Higher.GridCol - Lower.GridCol, -1, 1);
			if (StepRow == 0 && StepCol == 0)
			{
				return false;
			}

			const int32 SideRow = Edge.bAlongX ? 1 : 0;
			const int32 SideCol = Edge.bAlongX ? 0 : 1;
			const int32 HalfWidthCells = (Edge.Width >= CellSize * 0.78f) ? 1 : 0;
			const int32 MinLandingDepthCells = (Preset.Theme == ET66MapTheme::Farm) ? 2 : 1;

			const int32 LowerDepth = CountLandingPadDepth(
				Edge.LowerIndex,
				LowerRegion,
				Lower.TopZ,
				-StepRow,
				-StepCol,
				SideRow,
				SideCol,
				HalfWidthCells);
			const int32 HigherDepth = CountLandingPadDepth(
				Edge.HigherIndex,
				HigherRegion,
				Higher.TopZ,
				StepRow,
				StepCol,
				SideRow,
				SideCol,
				HalfWidthCells);

			if (LowerDepth < MinLandingDepthCells || HigherDepth < MinLandingDepthCells)
			{
				return false;
			}

			if (!HasLandingShoulder(Edge.LowerIndex, LowerRegion, Lower.TopZ, -StepRow, -StepCol, SideRow, SideCol))
			{
				return false;
			}

			if (!HasLandingShoulder(Edge.HigherIndex, HigherRegion, Higher.TopZ, StepRow, StepCol, SideRow, SideCol))
			{
				return false;
			}

			return true;
		};

		auto BuildRegionRampRoots = [&]() -> TArray<TArray<int32>>
		{
			TArray<TArray<int32>> Roots;
			Roots.SetNum(NumRegions);

			for (const FT66RampEdge& Edge : Result.Ramps)
			{
				const int32 LowerRegion = RegionID[Edge.LowerIndex];
				const int32 HigherRegion = RegionID[Edge.HigherIndex];
				if (LowerRegion == HigherRegion)
				{
					continue;
				}

				if (Roots.IsValidIndex(LowerRegion))
				{
					Roots[LowerRegion].AddUnique(Edge.LowerIndex);
				}
				if (Roots.IsValidIndex(HigherRegion))
				{
					Roots[HigherRegion].AddUnique(Edge.HigherIndex);
				}
			}

			return Roots;
		};

		auto MeasureRegionRampCoverage = [&](int32 RegionIndex, const TArray<TArray<int32>>& RampRoots, int32& OutFarthestCellIdx, int32& OutMaxDistance) -> bool
		{
			OutFarthestCellIdx = -1;
			OutMaxDistance = -1;

			if (!Regions.IsValidIndex(RegionIndex) || RegionIndex < 0 || RegionIndex >= NumRegions)
			{
				return false;
			}

			const TArray<int32>& RegionCells = Regions[RegionIndex].Cells;
			if (RegionCells.Num() == 0)
			{
				return false;
			}

			if (!RampRoots.IsValidIndex(RegionIndex) || RampRoots[RegionIndex].Num() == 0)
			{
				OutFarthestCellIdx = RegionCells[0];
				OutMaxDistance = TNumericLimits<int32>::Max() / 8;
				return false;
			}

			TArray<int32> Distance;
			Distance.Init(-1, Result.Platforms.Num());
			TArray<int32> Queue;
			Queue.Reserve(RegionCells.Num());

			for (const int32 RootIdx : RampRoots[RegionIndex])
			{
				if (!RegionID.IsValidIndex(RootIdx) || RegionID[RootIdx] != RegionIndex)
				{
					continue;
				}

				if (Distance[RootIdx] >= 0)
				{
					continue;
				}

				Distance[RootIdx] = 0;
				Queue.Add(RootIdx);
			}

			for (int32 Head = 0; Head < Queue.Num(); ++Head)
			{
				const int32 CurIdx = Queue[Head];
				const FT66PlatformNode& Cell = Result.Platforms[CurIdx];
				const int32 NextDistance = Distance[CurIdx] + 1;

				auto VisitNeighbor = [&](int32 NR, int32 NC)
				{
					if (NR < 0 || NR >= GridSize || NC < 0 || NC >= GridSize)
					{
						return;
					}

					const int32 NIdx = GridIdx(NR, NC, GridSize);
					if (RegionID[NIdx] != RegionIndex || Distance[NIdx] >= 0)
					{
						return;
					}

					Distance[NIdx] = NextDistance;
					Queue.Add(NIdx);
				};

				VisitNeighbor(Cell.GridRow - 1, Cell.GridCol);
				VisitNeighbor(Cell.GridRow + 1, Cell.GridCol);
				VisitNeighbor(Cell.GridRow, Cell.GridCol - 1);
				VisitNeighbor(Cell.GridRow, Cell.GridCol + 1);
			}

			for (const int32 CellIdx : RegionCells)
			{
				const int32 CellDistance = (Distance[CellIdx] >= 0) ? Distance[CellIdx] : (TNumericLimits<int32>::Max() / 8);
				if (CellDistance > OutMaxDistance)
				{
					OutMaxDistance = CellDistance;
					OutFarthestCellIdx = CellIdx;
				}
			}

			return true;
		};

		auto GetBoundaryDistanceToCell = [&](const FRegionBoundary& Boundary, int32 RegionIndex, int32 CellIdx) -> int32
		{
			if (!Result.Platforms.IsValidIndex(CellIdx))
			{
				return TNumericLimits<int32>::Max();
			}

			const FT66PlatformNode& SourceCell = Result.Platforms[CellIdx];
			int32 BestDistance = TNumericLimits<int32>::Max();

			for (const TPair<int32, int32>& CellPair : Boundary.CellPairs)
			{
				const int32 BoundaryCellIdx = (RegionID[CellPair.Key] == RegionIndex) ? CellPair.Key : CellPair.Value;
				if (!Result.Platforms.IsValidIndex(BoundaryCellIdx))
				{
					continue;
				}

				const FT66PlatformNode& BoundaryCell = Result.Platforms[BoundaryCellIdx];
				const int32 GridDistance =
					FMath::Abs(SourceCell.GridRow - BoundaryCell.GridRow) +
					FMath::Abs(SourceCell.GridCol - BoundaryCell.GridCol);
				BestDistance = FMath::Min(BestDistance, GridDistance);
			}

			return BestDistance;
		};

		auto CollapseRegionIntoNeighbor = [&](int32 RegionIndex) -> bool
		{
			if (!Regions.IsValidIndex(RegionIndex) || Regions[RegionIndex].Cells.Num() == 0)
			{
				return false;
			}

			FRegionBoundary* BestBoundary = nullptr;
			float BestHeightDiff = TNumericLimits<float>::Max();
			int32 BestSharedEdges = -1;

			for (TPair<uint64, FRegionBoundary>& Pair : BoundaryMap)
			{
				FRegionBoundary& Boundary = Pair.Value;
				if (Boundary.RegionA != RegionIndex && Boundary.RegionB != RegionIndex)
				{
					continue;
				}

				const int32 OtherRegion = (Boundary.RegionA == RegionIndex) ? Boundary.RegionB : Boundary.RegionA;
				if (!Regions.IsValidIndex(OtherRegion) || Regions[OtherRegion].bTouchesOuterEdge)
				{
					continue;
				}

				const float HeightDiff = FMath::Abs(Regions[RegionIndex].Height - Regions[OtherRegion].Height);
				if (Boundary.CellPairs.Num() > BestSharedEdges
					|| (Boundary.CellPairs.Num() == BestSharedEdges && HeightDiff < BestHeightDiff))
				{
					BestBoundary = &Boundary;
					BestSharedEdges = Boundary.CellPairs.Num();
					BestHeightDiff = HeightDiff;
				}
			}

			if (BestBoundary == nullptr)
			{
				return false;
			}

			const int32 OtherRegion = (BestBoundary->RegionA == RegionIndex) ? BestBoundary->RegionB : BestBoundary->RegionA;
			if (!Regions.IsValidIndex(OtherRegion))
			{
				return false;
			}

			const float TargetHeight = Regions[OtherRegion].Height;
			for (const int32 CellIdx : Regions[RegionIndex].Cells)
			{
				Result.Platforms[CellIdx].TopZ = TargetHeight;
			}

			UE_LOG(LogT66, Warning,
				TEXT("[MAP] Collapsed unresolved pocket region %d (%d cells, height %.0f) into neighbor region %d (height %.0f)."),
				RegionIndex,
				Regions[RegionIndex].Cells.Num(),
				Regions[RegionIndex].Height,
				OtherRegion,
				TargetHeight);
			return true;
		};

		// --- Phase 3: place ramps per boundary span rather than per cell edge ---

		struct FBoundarySortEntry
		{
			uint64 Key;
			float Priority;
		};
		TArray<FBoundarySortEntry> SortedBoundaries;
		SortedBoundaries.Reserve(BoundaryMap.Num());

		for (auto& Pair : BoundaryMap)
		{
			const FRegionBoundary& B = Pair.Value;
			if (Regions[B.RegionA].bTouchesOuterEdge || Regions[B.RegionB].bTouchesOuterEdge)
			{
				continue;
			}

			float Priority = 0.f;
			const float HA = Regions[B.RegionA].Height;
			const float HB = Regions[B.RegionB].Height;
			const float HeightDiff = FMath::Abs(HA - HB);

			// Favor trapped-low regions (below baseline with no easy escape).
			if (HA < 0.f || HB < 0.f) Priority += 50000.f;
			Priority += HeightDiff;
			Priority += static_cast<float>(B.CellPairs.Num()) * 100.f;
			Priority += Rng.FRandRange(0.f, 50.f);

			SortedBoundaries.Add({ Pair.Key, Priority });
		}

		SortedBoundaries.Sort([](const FBoundarySortEntry& A, const FBoundarySortEntry& B)
		{
			return A.Priority > B.Priority;
		});

		for (const FBoundarySortEntry& Entry : SortedBoundaries)
		{
			if (Result.Ramps.Num() >= MaxTotalRamps) break;

			FRegionBoundary& Boundary = BoundaryMap[Entry.Key];
			const int32 SharedEdges = Boundary.CellPairs.Num();
			int32 DesiredRamps = 1;
			if (SharedEdges >= 5) DesiredRamps = 3;
			else if (SharedEdges >= 3) DesiredRamps = 2;
			TryPlaceRampOnBoundary(Boundary, DesiredRamps);
		}

		// --- Phase 4: connectivity check — ensure every region is reachable ---

		auto BuildRegionGraph = [&]() -> TArray<TSet<int32>>
		{
			TArray<TSet<int32>> Adj;
			Adj.SetNum(NumRegions);
			for (const FT66RampEdge& Edge : Result.Ramps)
			{
				const int32 RA = RegionID[Edge.LowerIndex];
				const int32 RB = RegionID[Edge.HigherIndex];
				if (RA != RB)
				{
					Adj[RA].Add(RB);
					Adj[RB].Add(RA);
				}
			}
			// Same-height neighbors are implicitly connected (walkable).
			for (int32 R = 0; R < GridSize; ++R)
			{
				for (int32 C = 0; C < GridSize; ++C)
				{
					const int32 Idx = GridIdx(R, C, GridSize);
					const int32 MyReg = RegionID[Idx];
					auto LinkIfSameHeight = [&](int32 NR, int32 NC)
					{
						if (NR < 0 || NR >= GridSize || NC < 0 || NC >= GridSize) return;
						const int32 NIdx = GridIdx(NR, NC, GridSize);
						const int32 OtherReg = RegionID[NIdx];
						if (MyReg != OtherReg && FMath::Abs(Result.Platforms[Idx].TopZ - Result.Platforms[NIdx].TopZ) < 50.f)
						{
							Adj[MyReg].Add(OtherReg);
							Adj[OtherReg].Add(MyReg);
						}
					};
					if (C + 1 < GridSize) LinkIfSameHeight(R, C + 1);
					if (R + 1 < GridSize) LinkIfSameHeight(R + 1, C);
				}
			}
			return Adj;
		};

		for (int32 ConnPass = 0; ConnPass < 20; ++ConnPass)
		{
			TArray<TSet<int32>> RegionAdj = BuildRegionGraph();

			// BFS from the start-side region to ensure the map is traversable from where the player begins.
			int32 StartRootIndex = 0;
			float BestStartDistSq = TNumericLimits<float>::Max();
			const FVector StartCenter = T66GameplayLayout::GetStartAreaCenter(Preset.BaselineZ);
			const FVector2D StartCenter2D(StartCenter.X, StartCenter.Y);
			for (int32 PlatformIndex = 0; PlatformIndex < Result.Platforms.Num(); ++PlatformIndex)
			{
				const FT66PlatformNode& Platform = Result.Platforms[PlatformIndex];
				const FVector PlatformLocation(Platform.Position.X, Platform.Position.Y, Preset.BaselineZ);
				if (!T66GameplayLayout::IsInsideStartArea2D(PlatformLocation, Platform.SizeX * 0.1f))
				{
					continue;
				}

				const float DistSq = FVector2D::DistSquared(Platform.Position, StartCenter2D);
				if (DistSq < BestStartDistSq)
				{
					BestStartDistSq = DistSq;
					StartRootIndex = PlatformIndex;
				}
			}

			const int32 StartRootRegion = RegionID.IsValidIndex(StartRootIndex) ? RegionID[StartRootIndex] : 0;
			TArray<bool> Reachable;
			Reachable.Init(false, NumRegions);
			TArray<int32> BFS;
			BFS.Add(StartRootRegion);
			Reachable[StartRootRegion] = true;
			for (int32 Head = 0; Head < BFS.Num(); ++Head)
			{
				for (int32 Neighbor : RegionAdj[BFS[Head]])
				{
					if (!Reachable[Neighbor])
					{
						Reachable[Neighbor] = true;
						BFS.Add(Neighbor);
					}
				}
			}

			// Find first unreachable region.
			int32 DisconnectedRegion = -1;
			for (int32 I = 0; I < NumRegions; ++I)
			{
				if (Regions[I].bTouchesOuterEdge)
				{
					continue;
				}

				if (!Reachable[I])
				{
					DisconnectedRegion = I;
					break;
				}
			}

			if (DisconnectedRegion < 0) break;

			// Force a ramp from the disconnected region to any reachable neighbor.
			bool bFixed = false;
			for (int32 CellIdx : Regions[DisconnectedRegion].Cells)
			{
				if (bFixed) break;
				const FT66PlatformNode& P = Result.Platforms[CellIdx];
				auto TryForceRamp = [&](int32 NR, int32 NC)
				{
					if (bFixed) return;
					if (NR < 0 || NR >= GridSize || NC < 0 || NC >= GridSize) return;
					const int32 NIdx = GridIdx(NR, NC, GridSize);
					const int32 NReg = RegionID[NIdx];
					if (NReg == DisconnectedRegion) return;
					if (!Reachable[NReg]) return;

					const bool bAlongX = (P.GridRow == Result.Platforms[NIdx].GridRow);
					const int32 PrevNum = Result.Ramps.Num();
					AddRampForBoundary(Result, Preset, GridSize, CellSize, CellIdx, NIdx, bAlongX, 1, Rng, Used);
					if (Result.Ramps.Num() == PrevNum)
					{
						return;
					}

					const FT66RampEdge& Candidate = Result.Ramps.Last();
					if (!DoesRampHaveNaturalLandings(Candidate))
					{
						Used.Remove(MakeBoundaryKey(CellIdx, NIdx));
						Result.Ramps.SetNum(PrevNum);
						return;
					}

					if (DoesRampOverlapAny(Result, Candidate, AcceptedFootprints, RampPadding))
					{
						Used.Remove(MakeBoundaryKey(CellIdx, NIdx));
						Result.Ramps.SetNum(PrevNum);
						return;
					}

					AcceptedFootprints.Add({ BuildRampFootprint(Result, Candidate, RampPadding) });
					bFixed = true;
				};
				TryForceRamp(P.GridRow - 1, P.GridCol);
				TryForceRamp(P.GridRow + 1, P.GridCol);
				TryForceRamp(P.GridRow, P.GridCol - 1);
				TryForceRamp(P.GridRow, P.GridCol + 1);
			}

			if (!bFixed)
			{
				UE_LOG(LogT66, Warning, TEXT("[MAP] Could not force-connect region %d (height %.0f, %d cells)"),
					DisconnectedRegion, Regions[DisconnectedRegion].Height, Regions[DisconnectedRegion].Cells.Num());
				break;
			}
		}

		// --- Phase 5: local elevation-access guarantee — every interior region needs a way to change elevation. ---
		for (int32 AccessPass = 0; AccessPass < 8; ++AccessPass)
		{
			if (Result.Ramps.Num() >= MaxTotalRamps)
			{
				break;
			}

			TArray<FRegionRampAccess> Access = BuildRegionRampAccess();
			bool bAddedAccessRamp = false;

			for (int32 RegionIndex = 0; RegionIndex < NumRegions; ++RegionIndex)
			{
				if (Regions[RegionIndex].bTouchesOuterEdge)
				{
					continue;
				}

				const bool bNeedsUpRamp = Access[RegionIndex].bHasHigherNeighbor && !Access[RegionIndex].bHasUpRamp;
				const bool bNeedsDownRamp = Access[RegionIndex].bHasLowerNeighbor && !Access[RegionIndex].bHasDownRamp;
				if (!bNeedsUpRamp && !bNeedsDownRamp)
				{
					continue;
				}

				TArray<FRegionBoundary*> CandidateBoundaries;
				for (TPair<uint64, FRegionBoundary>& Pair : BoundaryMap)
				{
					FRegionBoundary& Boundary = Pair.Value;
					if (Boundary.RegionA != RegionIndex && Boundary.RegionB != RegionIndex)
					{
						continue;
					}

					const int32 OtherRegion = (Boundary.RegionA == RegionIndex) ? Boundary.RegionB : Boundary.RegionA;
					if (!Regions.IsValidIndex(OtherRegion) || Regions[OtherRegion].bTouchesOuterEdge)
					{
						continue;
					}

					const bool bBoundaryIsUpward = Regions[OtherRegion].Height > Regions[RegionIndex].Height;
					if ((bNeedsUpRamp && bBoundaryIsUpward) || (bNeedsDownRamp && !bBoundaryIsUpward))
					{
						CandidateBoundaries.Add(&Boundary);
					}
				}

				CandidateBoundaries.Sort([&](const FRegionBoundary& A, const FRegionBoundary& B)
				{
					const float HeightDiffA = FMath::Abs(Regions[A.RegionA].Height - Regions[A.RegionB].Height);
					const float HeightDiffB = FMath::Abs(Regions[B.RegionA].Height - Regions[B.RegionB].Height);
					if (A.CellPairs.Num() != B.CellPairs.Num())
					{
						return A.CellPairs.Num() > B.CellPairs.Num();
					}
					return HeightDiffA < HeightDiffB;
				});

				for (FRegionBoundary* Boundary : CandidateBoundaries)
				{
					if (!Boundary || Result.Ramps.Num() >= MaxTotalRamps)
					{
						break;
					}

					if (TryPlaceRampOnBoundary(*Boundary, 1) > 0)
					{
						bAddedAccessRamp = true;
						Access = BuildRegionRampAccess();
						break;
					}
				}
			}

			if (!bAddedAccessRamp)
			{
				break;
			}
		}

		// --- Phase 6: local ramp coverage guarantee — no terrace should strand the player more than a few cells from an elevation change. ---
		const int32 MaxCellsFromRamp = (Preset.Theme == ET66MapTheme::Farm) ? 4 : 6;
		for (int32 CoveragePass = 0; CoveragePass < 10; ++CoveragePass)
		{
			if (Result.Ramps.Num() >= MaxTotalRamps)
			{
				break;
			}

			const TArray<FRegionRampAccess> Access = BuildRegionRampAccess();
			const TArray<TArray<int32>> RampRoots = BuildRegionRampRoots();
			bool bAddedCoverageRamp = false;

			for (int32 RegionIndex = 0; RegionIndex < NumRegions; ++RegionIndex)
			{
				if (Regions[RegionIndex].bTouchesOuterEdge)
				{
					continue;
				}

				if (!Access[RegionIndex].bHasHigherNeighbor && !Access[RegionIndex].bHasLowerNeighbor)
				{
					continue;
				}

				int32 FarthestCellIdx = -1;
				int32 MaxDistance = -1;
				MeasureRegionRampCoverage(RegionIndex, RampRoots, FarthestCellIdx, MaxDistance);
				if (FarthestCellIdx < 0 || MaxDistance <= MaxCellsFromRamp)
				{
					continue;
				}

				TArray<FRegionBoundary*> CandidateBoundaries;
				for (TPair<uint64, FRegionBoundary>& Pair : BoundaryMap)
				{
					FRegionBoundary& Boundary = Pair.Value;
					if (Boundary.RegionA != RegionIndex && Boundary.RegionB != RegionIndex)
					{
						continue;
					}

					const int32 OtherRegion = (Boundary.RegionA == RegionIndex) ? Boundary.RegionB : Boundary.RegionA;
					if (!Regions.IsValidIndex(OtherRegion) || Regions[OtherRegion].bTouchesOuterEdge)
					{
						continue;
					}

					CandidateBoundaries.Add(&Boundary);
				}

				CandidateBoundaries.Sort([&](const FRegionBoundary& A, const FRegionBoundary& B)
				{
					const int32 DistanceA = GetBoundaryDistanceToCell(A, RegionIndex, FarthestCellIdx);
					const int32 DistanceB = GetBoundaryDistanceToCell(B, RegionIndex, FarthestCellIdx);
					if (DistanceA != DistanceB)
					{
						return DistanceA < DistanceB;
					}

					if (A.CellPairs.Num() != B.CellPairs.Num())
					{
						return A.CellPairs.Num() > B.CellPairs.Num();
					}

					const float HeightDiffA = FMath::Abs(Regions[A.RegionA].Height - Regions[A.RegionB].Height);
					const float HeightDiffB = FMath::Abs(Regions[B.RegionA].Height - Regions[B.RegionB].Height);
					return HeightDiffA < HeightDiffB;
				});

				for (FRegionBoundary* Boundary : CandidateBoundaries)
				{
					if (!Boundary || Result.Ramps.Num() >= MaxTotalRamps)
					{
						break;
					}

					if (TryPlaceRampOnBoundary(*Boundary, 1) > 0)
					{
						bAddedCoverageRamp = true;
						break;
					}
				}

				if (bAddedCoverageRamp)
				{
					break;
				}
			}

			if (!bAddedCoverageRamp)
			{
				break;
			}
		}

		// --- Phase 7: collapse tiny unresolved pockets instead of leaving player traps. ---
		{
			const TArray<FRegionRampAccess> Access = BuildRegionRampAccess();
			const TArray<TArray<int32>> RampRoots = BuildRegionRampRoots();
			const int32 MaxCollapsedPocketCells = (Preset.Theme == ET66MapTheme::Farm) ? 6 : 3;

			for (int32 RegionIndex = 0; RegionIndex < NumRegions; ++RegionIndex)
			{
				if (Regions[RegionIndex].bTouchesOuterEdge || Regions[RegionIndex].Cells.Num() == 0)
				{
					continue;
				}

				if (Regions[RegionIndex].Cells.Num() > MaxCollapsedPocketCells)
				{
					continue;
				}

				const bool bHasElevationNeighbor = Access[RegionIndex].bHasHigherNeighbor || Access[RegionIndex].bHasLowerNeighbor;
				if (!bHasElevationNeighbor)
				{
					continue;
				}

				if (RampRoots.IsValidIndex(RegionIndex) && RampRoots[RegionIndex].Num() > 0)
				{
					continue;
				}

				CollapseRegionIntoNeighbor(RegionIndex);
			}
		}
	}
}

FT66ProceduralMapResult FT66ProceduralMapGenerator::Generate(const FT66MapPreset& Preset)
{
	FT66ProceduralMapResult Result;

	const int32 GridSize = FMath::Clamp(Preset.GridSize, 2, 128);
	const float MapSize = Preset.MapHalfExtent * 2.f;
	const float CellSize = MapSize / static_cast<float>(GridSize);

	InitializePlatforms(Result, Preset, GridSize, CellSize);
	int32 FlattenedTraversalPlatforms = 0;

	if (Preset.Theme == ET66MapTheme::Farm)
	{
		GenerateFarmGrowthMap(Result, Preset, GridSize, CellSize);
		BuildNeighborConnections(Result, GridSize);
	}
	else
	{
		GenerateRegionHeights(Result.Platforms, Preset, GridSize);
		ForceOuterRingToMaxHeight(Result.Platforms, Preset, GridSize);
		ClampAdjacency(Result.Platforms, Preset, GridSize);
		FlattenedTraversalPlatforms = FlattenReservedTraversalZones(Result.Platforms, Preset);
		CollapseTinyRegions(Result.Platforms, Preset, GridSize);
		ClampAdjacency(Result.Platforms, Preset, GridSize, 8);
		FlattenReservedTraversalZones(Result.Platforms, Preset);
		BuildNeighborConnections(Result, GridSize);
		BuildRamps(Result, Preset, GridSize, CellSize);
	}

	float MinZ = TNumericLimits<float>::Max();
	float MaxZ = TNumericLimits<float>::Lowest();
	int32 BaselineCount = 0;
	int32 PositiveCount = 0;
	int32 NegativeCount = 0;
	for (const FT66PlatformNode& P : Result.Platforms)
	{
		MinZ = FMath::Min(MinZ, P.TopZ);
		MaxZ = FMath::Max(MaxZ, P.TopZ);
		if (FMath::IsNearlyEqual(P.TopZ, Preset.BaselineZ))
		{
			++BaselineCount;
		}
		else if (P.TopZ > Preset.BaselineZ)
		{
			++PositiveCount;
		}
		else
		{
			++NegativeCount;
		}
	}

	UE_LOG(LogT66, Log,
		TEXT("[MAP] Generated %d platforms, %d ramps (grid %dx%d, cell %.0f, theme %d, seed %d). Baseline=%d, Pos=%d, Neg=%d, Z range [%.0f, %.0f]"),
		Result.Platforms.Num(),
		Result.Ramps.Num(),
		GridSize,
		GridSize,
		CellSize,
		static_cast<int32>(Preset.Theme),
		Preset.Seed,
		BaselineCount,
		PositiveCount,
		NegativeCount,
		MinZ,
		MaxZ);

	if (FlattenedTraversalPlatforms > 0)
	{
		UE_LOG(LogT66, Log, TEXT("[MAP] Flattened %d reserved traversal platforms to baseline %.0f."),
			FlattenedTraversalPlatforms,
			Preset.BaselineZ);
	}

	Result.bValid = true;
	return Result;
}
