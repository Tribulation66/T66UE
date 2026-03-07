// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ProceduralLandscapeGenerator.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "T66.h"
#include "Math/RandomStream.h"

namespace
{
	int32 GridIdx(int32 Row, int32 Col, int32 GridSize)
	{
		return Row * GridSize + Col;
	}

	float QuantizeToStep(float Value, const FT66MapPreset& Preset)
	{
		const float Step = FMath::Max(Preset.ElevationStep, 1.f);
		return Preset.BaselineZ + FMath::RoundToFloat((Value - Preset.BaselineZ) / Step) * Step;
	}

	uint64 MakeBoundaryKey(int32 A, int32 B)
	{
		const uint32 Low = static_cast<uint32>(FMath::Min(A, B));
		const uint32 High = static_cast<uint32>(FMath::Max(A, B));
		return (static_cast<uint64>(Low) << 32) | static_cast<uint64>(High);
	}

	FIntPoint TurnLeft(const FIntPoint& Dir)
	{
		return FIntPoint(-Dir.Y, Dir.X);
	}

	FIntPoint TurnRight(const FIntPoint& Dir)
	{
		return FIntPoint(Dir.Y, -Dir.X);
	}

	struct FT66RampFootprint
	{
		FBox2D Box;
	};

	void InitializePlatforms(FT66ProceduralMapResult& Result, const FT66MapPreset& Preset, int32 GridSize, float CellSize)
	{
		Result.Platforms.SetNum(GridSize * GridSize);

		for (int32 R = 0; R < GridSize; ++R)
		{
			for (int32 C = 0; C < GridSize; ++C)
			{
				const int32 Idx = GridIdx(R, C, GridSize);
				FT66PlatformNode& P = Result.Platforms[Idx];
				P.Position = FVector2D(
					-Preset.MapHalfExtent + (C + 0.5f) * CellSize,
					-Preset.MapHalfExtent + (R + 0.5f) * CellSize);
				P.TopZ = Preset.BaselineZ;
				P.SizeX = CellSize;
				P.SizeY = CellSize;
				P.GridRow = R;
				P.GridCol = C;
				P.Connections.Reset();
			}
		}
	}

	int32 PickFeatureLevel(FRandomStream& Rng, const FT66MapPreset& Preset, int32 FeatureIdx)
	{
		const int32 MinLevel = FMath::FloorToInt((Preset.ElevationMin - Preset.BaselineZ) / FMath::Max(Preset.ElevationStep, 1.f));
		const int32 MaxLevel = FMath::CeilToInt((Preset.ElevationMax - Preset.BaselineZ) / FMath::Max(Preset.ElevationStep, 1.f));

		const bool bCanGoNegative = MinLevel < 0;
		const bool bCanGoPositive = MaxLevel > 0;
		if (!bCanGoNegative && !bCanGoPositive)
		{
			return 0;
		}

		int32 Sign = 1;
		switch (Preset.Theme)
		{
		case ET66MapTheme::Farm:
			if (bCanGoNegative && bCanGoPositive)
			{
				Sign = (FeatureIdx % 2 == 0) ? 1 : -1;
			}
			else
			{
				Sign = bCanGoPositive ? 1 : -1;
			}
			break;
		case ET66MapTheme::Ocean:
			Sign = (!bCanGoPositive || Rng.FRand() < 0.90f) ? -1 : 1;
			break;
		case ET66MapTheme::Mountain:
			Sign = (!bCanGoNegative || Rng.FRand() < 0.80f) ? 1 : -1;
			break;
		}

		if (Sign > 0 && !bCanGoPositive)
		{
			Sign = -1;
		}
		if (Sign < 0 && !bCanGoNegative)
		{
			Sign = 1;
		}

		const int32 MaxAbsLevel = (Sign > 0) ? MaxLevel : FMath::Abs(MinLevel);
		if (MaxAbsLevel <= 0)
		{
			return 0;
		}

		int32 Magnitude = 1;
		if (Preset.Theme == ET66MapTheme::Farm)
		{
			Magnitude = Rng.RandRange(1, FMath::Min(MaxAbsLevel, 3));
		}
		else if (Preset.Theme == ET66MapTheme::Ocean)
		{
			Magnitude = Rng.RandRange(1, FMath::Min(MaxAbsLevel, 6));
		}
		else
		{
			Magnitude = Rng.RandRange(1, FMath::Min(MaxAbsLevel, 8));
		}

		return Sign * Magnitude;
	}

	void PaintBrush(
		TArray<FT66PlatformNode>& Platforms,
		TArray<float>& Dominance,
		int32 GridSize,
		int32 CenterR,
		int32 CenterC,
		int32 RadiusCells,
		float Height,
		float Priority)
	{
		for (int32 DR = -RadiusCells; DR <= RadiusCells; ++DR)
		{
			for (int32 DC = -RadiusCells; DC <= RadiusCells; ++DC)
			{
				const int32 R = CenterR + DR;
				const int32 C = CenterC + DC;
				if (R < 0 || R >= GridSize || C < 0 || C >= GridSize)
				{
					continue;
				}

				const float Dist = FMath::Sqrt(static_cast<float>(DR * DR + DC * DC));
				if (Dist > static_cast<float>(RadiusCells) + 0.25f)
				{
					continue;
				}

				const int32 Idx = GridIdx(R, C, GridSize);
				const float Strength = Priority - Dist * 0.08f;
				if (Strength > Dominance[Idx])
				{
					Platforms[Idx].TopZ = Height;
					Dominance[Idx] = Strength;
				}
			}
		}
	}

	void GenerateLaneFeatures(TArray<FT66PlatformNode>& Platforms, const FT66MapPreset& Preset, int32 GridSize)
	{
		FRandomStream Rng(Preset.Seed);
		TArray<float> Dominance;
		Dominance.Init(-1000000.f, Platforms.Num());

		const int32 FeatureCount = Rng.RandRange(Preset.FeatureCountMin, Preset.FeatureCountMax);
		TArray<FIntPoint> StartSeeds;
		const float MinSeedSpacing = static_cast<float>(GridSize) / FMath::Clamp(FMath::Sqrt(static_cast<float>(FeatureCount)) * 0.75f, 1.f, 1000.f);

		for (int32 FeatureIdx = 0; FeatureIdx < FeatureCount; ++FeatureIdx)
		{
			const int32 Level = PickFeatureLevel(Rng, Preset, FeatureIdx);
			if (Level == 0)
			{
				continue;
			}

			const float Height = FMath::Clamp(
				QuantizeToStep(Preset.BaselineZ + static_cast<float>(Level) * Preset.ElevationStep, Preset),
				Preset.ElevationMin,
				Preset.ElevationMax);

			FIntPoint Start;
			bool bFoundStart = false;
			for (int32 Attempt = 0; Attempt < 40 && !bFoundStart; ++Attempt)
			{
				const FIntPoint Candidate(
					Rng.RandRange(0, GridSize - 1),
					Rng.RandRange(0, GridSize - 1));

				bool bFarEnough = true;
				for (const FIntPoint& Existing : StartSeeds)
				{
					const float Dist = FVector2D::Distance(FVector2D(Candidate), FVector2D(Existing));
					if (Dist < MinSeedSpacing)
					{
						bFarEnough = false;
						break;
					}
				}

				if (bFarEnough)
				{
					Start = Candidate;
					bFoundStart = true;
				}
			}

			if (!bFoundStart)
			{
				Start = FIntPoint(Rng.RandRange(0, GridSize - 1), Rng.RandRange(0, GridSize - 1));
			}
			StartSeeds.Add(Start);

			int32 CurR = Start.X;
			int32 CurC = Start.Y;
			FIntPoint Dir;
			switch (Rng.RandRange(0, 3))
			{
			case 0: Dir = FIntPoint(1, 0); break;
			case 1: Dir = FIntPoint(-1, 0); break;
			case 2: Dir = FIntPoint(0, 1); break;
			default: Dir = FIntPoint(0, -1); break;
			}

			const int32 Length = Rng.RandRange(Preset.FeatureLengthMinCells, Preset.FeatureLengthMaxCells);
			const int32 Width = Rng.RandRange(Preset.FeatureWidthMinCells, Preset.FeatureWidthMaxCells);
			const float BasePriority = 1000.f + FeatureIdx * 10.f + Rng.FRandRange(0.f, 3.f);

			for (int32 StepIdx = 0; StepIdx < Length; ++StepIdx)
			{
				PaintBrush(Platforms, Dominance, GridSize, CurR, CurC, Width, Height, BasePriority - StepIdx * 0.01f);

				if (Rng.FRand() < Preset.RoomChance)
				{
					PaintBrush(Platforms, Dominance, GridSize, CurR, CurC, Width + Rng.RandRange(1, 3), Height, BasePriority + 0.5f);
				}

				if (Rng.FRand() < 0.28f)
				{
					Dir = (Rng.FRand() < 0.5f) ? TurnLeft(Dir) : TurnRight(Dir);
				}

				if (Rng.FRand() < 0.10f)
				{
					const FIntPoint BranchDir = (Rng.FRand() < 0.5f) ? TurnLeft(Dir) : TurnRight(Dir);
					const int32 BranchLength = FMath::Max(4, Length / 3 + Rng.RandRange(-3, 6));
					int32 BranchR = CurR;
					int32 BranchC = CurC;
					for (int32 B = 0; B < BranchLength; ++B)
					{
						PaintBrush(Platforms, Dominance, GridSize, BranchR, BranchC, FMath::Max(1, Width - 1), Height, BasePriority - 5.f);
						BranchR = FMath::Clamp(BranchR + BranchDir.X, 0, GridSize - 1);
						BranchC = FMath::Clamp(BranchC + BranchDir.Y, 0, GridSize - 1);
					}
				}

				if (Rng.FRand() < 0.08f)
				{
					// Teleport-start a continuation elsewhere to break regional coordination.
					CurR = Rng.RandRange(0, GridSize - 1);
					CurC = Rng.RandRange(0, GridSize - 1);
				}

				int32 NextR = CurR + Dir.X;
				int32 NextC = CurC + Dir.Y;
				if (NextR < 0 || NextR >= GridSize || NextC < 0 || NextC >= GridSize)
				{
					Dir = (Rng.FRand() < 0.5f) ? TurnLeft(Dir) : TurnRight(Dir);
					NextR = FMath::Clamp(CurR + Dir.X, 0, GridSize - 1);
					NextC = FMath::Clamp(CurC + Dir.Y, 0, GridSize - 1);
				}

				CurR = NextR;
				CurC = NextC;
			}
		}
	}

	void RemoveTinyIslands(TArray<FT66PlatformNode>& Platforms, const FT66MapPreset& Preset, int32 GridSize)
	{
		TArray<float> NewHeights;
		NewHeights.SetNum(Platforms.Num());

		for (int32 R = 0; R < GridSize; ++R)
		{
			for (int32 C = 0; C < GridSize; ++C)
			{
				const int32 Idx = GridIdx(R, C, GridSize);
				const float H = Platforms[Idx].TopZ;
				NewHeights[Idx] = H;

				int32 SameCount = 0;
				int32 BaselineCount = 0;
				for (const FIntPoint Off : { FIntPoint(-1, 0), FIntPoint(1, 0), FIntPoint(0, -1), FIntPoint(0, 1) })
				{
					const int32 NR = R + Off.X;
					const int32 NC = C + Off.Y;
					if (NR < 0 || NR >= GridSize || NC < 0 || NC >= GridSize)
					{
						continue;
					}

					const float NH = Platforms[GridIdx(NR, NC, GridSize)].TopZ;
					if (FMath::IsNearlyEqual(NH, H))
					{
						++SameCount;
					}
					if (FMath::IsNearlyEqual(NH, Preset.BaselineZ))
					{
						++BaselineCount;
					}
				}

				if (!FMath::IsNearlyEqual(H, Preset.BaselineZ) && SameCount == 0 && BaselineCount >= 2)
				{
					NewHeights[Idx] = Preset.BaselineZ;
				}
			}
		}

		for (int32 I = 0; I < Platforms.Num(); ++I)
		{
			Platforms[I].TopZ = NewHeights[I];
		}
	}

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

	void BuildNeighborConnections(FT66ProceduralMapResult& Result, int32 GridSize)
	{
		for (int32 R = 0; R < GridSize; ++R)
		{
			for (int32 C = 0; C < GridSize; ++C)
			{
				const int32 Idx = GridIdx(R, C, GridSize);
				if (C + 1 < GridSize)
				{
					const int32 Right = GridIdx(R, C + 1, GridSize);
					Result.Platforms[Idx].Connections.AddUnique(Right);
					Result.Platforms[Right].Connections.AddUnique(Idx);
				}
				if (R + 1 < GridSize)
				{
					const int32 Up = GridIdx(R + 1, C, GridSize);
					Result.Platforms[Idx].Connections.AddUnique(Up);
					Result.Platforms[Up].Connections.AddUnique(Idx);
				}
			}
		}
	}

	void AddRampForBoundary(
		FT66ProceduralMapResult& Result,
		const FT66MapPreset& Preset,
		float CellSize,
		int32 AIdx,
		int32 BIdx,
		bool bAlongX,
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
		const float HeightDiff = FMath::Abs(A.TopZ - B.TopZ);
		if (HeightDiff < 50.f)
		{
			return;
		}

		FT66RampEdge Edge;
		Edge.LowerIndex = (A.TopZ <= B.TopZ) ? AIdx : BIdx;
		Edge.HigherIndex = (A.TopZ <= B.TopZ) ? BIdx : AIdx;
		Edge.bAlongX = bAlongX;

		const float HeightDrivenDepth = HeightDiff * FMath::Max(Preset.RampRunPerRise, 1.f);
		Edge.Depth = FMath::Clamp(
			HeightDrivenDepth * Rng.FRandRange(0.95f, 1.20f),
			Preset.RampDepthMin,
			Preset.RampDepthMax);

		const float HeightAlpha = FMath::Clamp(
			HeightDiff / FMath::Max(FMath::Abs(Preset.ElevationMax - Preset.ElevationMin), 1.f),
			0.f,
			1.f);
		const float WidthAlpha = FMath::Lerp(Preset.RampWidthMinAlpha, Preset.RampWidthMaxAlpha, 0.35f + HeightAlpha * 0.65f);
		Edge.Width = FMath::Clamp(
			CellSize * WidthAlpha * Rng.FRandRange(0.90f, 1.05f),
			CellSize * 0.35f,
			CellSize * 1.00f);

		const float MaxOffset = FMath::Max((CellSize - Edge.Width) * 0.5f, 0.f);
		Edge.PerpOffset = (MaxOffset > KINDA_SMALL_NUMBER)
			? Rng.FRandRange(-MaxOffset * 0.9f, MaxOffset * 0.9f)
			: 0.f;

		Result.Ramps.Add(Edge);
		Used.Add(Key);
	}

	FBox2D BuildRampFootprint(
		const FT66ProceduralMapResult& Result,
		const FT66RampEdge& Edge,
		float Padding)
	{
		const FT66PlatformNode& Lo = Result.Platforms[Edge.LowerIndex];
		const FT66PlatformNode& Hi = Result.Platforms[Edge.HigherIndex];
		const FVector2D Mid = 0.5f * (Lo.Position + Hi.Position);
		const float HalfDepth = Edge.Depth * 0.5f + Padding;
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

	void BuildRamps(FT66ProceduralMapResult& Result, const FT66MapPreset& Preset, int32 GridSize, float CellSize)
	{
		FRandomStream Rng(Preset.Seed ^ 0x4D617A65);
		TSet<uint64> Used;
		TArray<FT66RampFootprint> AcceptedFootprints;
		const float RampPadding = CellSize * 0.18f;
		const int32 MaxTotalRamps = 20;
		TArray<bool> Visited;
		Visited.Init(false, Result.Platforms.Num());
		const float MaxJumpEscapeHeight = FMath::Max(250.f, Preset.ElevationStep * 0.75f);

		struct FBoundaryCandidate
		{
			int32 AIdx = -1;
			int32 BIdx = -1;
			bool bAlongX = true;
			float Score = 0.f;
		};

		struct FComponentInfo
		{
			TArray<int32> Cells;
			TArray<FBoundaryCandidate> Candidates;
			float Height = 0.f;
			bool bTrappedLow = false;
			float Priority = 0.f;
		};

		TArray<FComponentInfo> Components;

		for (int32 StartIdx = 0; StartIdx < Result.Platforms.Num(); ++StartIdx)
		{
			if (Visited[StartIdx])
			{
				continue;
			}

			const float Height = Result.Platforms[StartIdx].TopZ;
			if (FMath::IsNearlyEqual(Height, Preset.BaselineZ))
			{
				Visited[StartIdx] = true;
				continue;
			}

			TArray<int32> Queue;
			TArray<int32> Component;
			Queue.Add(StartIdx);
			Visited[StartIdx] = true;

			for (int32 Head = 0; Head < Queue.Num(); ++Head)
			{
				const int32 Cur = Queue[Head];
				Component.Add(Cur);
				const FT66PlatformNode& P = Result.Platforms[Cur];

				auto TryNeighbor = [&](int32 NR, int32 NC)
				{
					if (NR < 0 || NR >= GridSize || NC < 0 || NC >= GridSize)
					{
						return;
					}

					const int32 NIdx = GridIdx(NR, NC, GridSize);
					if (Visited[NIdx])
					{
						return;
					}

					if (FMath::IsNearlyEqual(Result.Platforms[NIdx].TopZ, Height))
					{
						Visited[NIdx] = true;
						Queue.Add(NIdx);
					}
				};

				TryNeighbor(P.GridRow - 1, P.GridCol);
				TryNeighbor(P.GridRow + 1, P.GridCol);
				TryNeighbor(P.GridRow, P.GridCol - 1);
				TryNeighbor(P.GridRow, P.GridCol + 1);
			}

			TArray<FBoundaryCandidate> Candidates;
			TSet<uint64> SeenBoundaries;
			bool bHasJumpableHigherExit = false;

			for (int32 Idx : Component)
			{
				const FT66PlatformNode& P = Result.Platforms[Idx];

				auto AddCandidate = [&](int32 NR, int32 NC, bool bAlongX)
				{
					if (NR < 0 || NR >= GridSize || NC < 0 || NC >= GridSize)
					{
						return;
					}

					const int32 NIdx = GridIdx(NR, NC, GridSize);
					if (FMath::IsNearlyEqual(Result.Platforms[NIdx].TopZ, Height))
					{
						return;
					}

					const uint64 Key = MakeBoundaryKey(Idx, NIdx);
					if (SeenBoundaries.Contains(Key) || Used.Contains(Key))
					{
						return;
					}
					SeenBoundaries.Add(Key);

					const float NeighborH = Result.Platforms[NIdx].TopZ;
					const float HeightDiff = FMath::Abs(Height - NeighborH);
					if (NeighborH > Height && HeightDiff <= MaxJumpEscapeHeight)
					{
						bHasJumpableHigherExit = true;
					}
					float Score = HeightDiff;
					if (FMath::IsNearlyEqual(NeighborH, Preset.BaselineZ))
					{
						Score += 100000.f;
					}
					if (NeighborH > Height)
					{
						Score += 25000.f;
					}
					if (Height < Preset.BaselineZ && NeighborH > Height)
					{
						Score += 15000.f;
					}

					FBoundaryCandidate Candidate;
					Candidate.AIdx = Idx;
					Candidate.BIdx = NIdx;
					Candidate.bAlongX = bAlongX;
					Candidate.Score = Score + Rng.FRandRange(0.f, 100.f);
					Candidates.Add(Candidate);
				};

				AddCandidate(P.GridRow, P.GridCol - 1, true);
				AddCandidate(P.GridRow, P.GridCol + 1, true);
				AddCandidate(P.GridRow - 1, P.GridCol, false);
				AddCandidate(P.GridRow + 1, P.GridCol, false);
			}

			Candidates.Sort([](const FBoundaryCandidate& A, const FBoundaryCandidate& B)
			{
				return A.Score > B.Score;
			});

			FComponentInfo Info;
			Info.Cells = MoveTemp(Component);
			Info.Candidates = MoveTemp(Candidates);
			Info.Height = Height;
			Info.bTrappedLow = (Height < Preset.BaselineZ && !bHasJumpableHigherExit);
			Info.Priority = 0.f;
			if (Info.bTrappedLow)
			{
				Info.Priority += 1000000.f;
			}
			if (Height > Preset.BaselineZ)
			{
				Info.Priority += 500000.f;
			}
			Info.Priority += FMath::Abs(Height - Preset.BaselineZ);
			Info.Priority += static_cast<float>(Info.Cells.Num()) * 10.f;
			Components.Add(MoveTemp(Info));
		}

		Components.Sort([](const FComponentInfo& A, const FComponentInfo& B)
		{
			return A.Priority > B.Priority;
		});

		for (const FComponentInfo& ComponentInfo : Components)
		{
			if (Result.Ramps.Num() >= MaxTotalRamps)
			{
				break;
			}

			for (const FBoundaryCandidate& CandidateInfo : ComponentInfo.Candidates)
			{
				const int32 PrevNum = Result.Ramps.Num();
				AddRampForBoundary(Result, Preset, CellSize, CandidateInfo.AIdx, CandidateInfo.BIdx, CandidateInfo.bAlongX, Rng, Used);
				if (Result.Ramps.Num() == PrevNum)
				{
					continue;
				}

				const FT66RampEdge Candidate = Result.Ramps.Last();
				if (DoesRampOverlapAny(Result, Candidate, AcceptedFootprints, RampPadding))
				{
					Result.Ramps.Pop();
					Used.Remove(MakeBoundaryKey(CandidateInfo.AIdx, CandidateInfo.BIdx));
					continue;
				}

				AcceptedFootprints.Add({ BuildRampFootprint(Result, Candidate, RampPadding) });
				break;
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
	GenerateLaneFeatures(Result.Platforms, Preset, GridSize);
	RemoveTinyIslands(Result.Platforms, Preset, GridSize);
	ClampAdjacency(Result.Platforms, Preset, GridSize);
	BuildNeighborConnections(Result, GridSize);
	BuildRamps(Result, Preset, GridSize, CellSize);

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

	Result.bValid = true;
	return Result;
}
