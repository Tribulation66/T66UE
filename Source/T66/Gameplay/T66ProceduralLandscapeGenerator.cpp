// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ProceduralLandscapeGenerator.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "Core/T66GameplayLayout.h"
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

	// ========================================================================
	// Region-based height assignment (replaces lane features)
	// ========================================================================

	void GenerateRegionHeights(TArray<FT66PlatformNode>& Platforms, const FT66MapPreset& Preset, int32 GridSize)
	{
		FRandomStream Rng(Preset.Seed);

		const TArray<float> HeightLevels = { -1400.f, -700.f, 0.f, 700.f, 1400.f };
		const int32 NumHeightLevels = HeightLevels.Num();

		const int32 SeedCount = Rng.RandRange(Preset.RegionSeedCountMin, Preset.RegionSeedCountMax);

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
			FIntPoint Cell;
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

		// Assign heights: ensure >= 3 distinct levels, no more than 2 seeds share a height.
		TArray<int32> HeightUsage;
		HeightUsage.Init(0, NumHeightLevels);
		TSet<int32> UsedLevelIndices;

		// First pass: assign heights trying to maximize variety.
		for (int32 I = 0; I < Seeds.Num(); ++I)
		{
			TArray<int32> Available;
			for (int32 L = 0; L < NumHeightLevels; ++L)
			{
				if (HeightUsage[L] < 2)
				{
					Available.Add(L);
				}
			}

			// If we haven't hit 3 distinct levels yet and can pick a new one, prefer unused levels.
			if (UsedLevelIndices.Num() < 3)
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

			const int32 ChosenIdx = Available[Rng.RandRange(0, Available.Num() - 1)];
			Seeds[I].Height = HeightLevels[ChosenIdx];
			HeightUsage[ChosenIdx]++;
			UsedLevelIndices.Add(ChosenIdx);
		}

		// Flood-fill: assign every cell to the closest seed (Euclidean distance).
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

				Platforms[GridIdx(R, C, GridSize)].TopZ = BestHeight;
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
			if (!T66GameplayLayout::DoesRectOverlapReservedTraversalZone2D(
				Platform.Position.X - HalfX,
				Platform.Position.X + HalfX,
				Platform.Position.Y - HalfY,
				Platform.Position.Y + HalfY))
			{
				continue;
			}

			if (!FMath::IsNearlyEqual(Platform.TopZ, Preset.BaselineZ))
			{
				Platform.TopZ = Preset.BaselineZ;
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

	// ========================================================================
	// Ramp creation helper
	// ========================================================================

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
		CellSize * 0.10f,
		CellSize * 0.45f);

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

	// ========================================================================
	// Region-aware ramp builder with multi-ramp boundaries and connectivity
	// ========================================================================

	void BuildRamps(FT66ProceduralMapResult& Result, const FT66MapPreset& Preset, int32 GridSize, float CellSize)
	{
		FRandomStream Rng(Preset.Seed ^ 0x4D617A65);
		TSet<uint64> Used;
		TArray<FT66RampFootprint> AcceptedFootprints;
		const float RampPadding = CellSize * 0.05f;
		const int32 MaxTotalRamps = 100;

		// --- Phase 1: discover connected components (regions) of same-height cells ---

		TArray<int32> RegionID;
		RegionID.Init(-1, Result.Platforms.Num());
		int32 NumRegions = 0;

		struct FRegionInfo
		{
			TArray<int32> Cells;
			float Height = 0.f;
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

					const uint64 PairKey = MakeBoundaryKey(MyRegion, OtherRegion);
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

		// --- Phase 3: place ramps per boundary (1 for 1-2 edges, 2 for 3-4, 3 for 5+) ---

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

			// Shuffle cell pairs for variety.
			for (int32 I = Boundary.CellPairs.Num() - 1; I > 0; --I)
			{
				const int32 J = Rng.RandRange(0, I);
				Boundary.CellPairs.Swap(I, J);
			}

			int32 PlacedForBoundary = 0;
			for (const auto& CellPair : Boundary.CellPairs)
			{
				if (PlacedForBoundary >= DesiredRamps) break;
				if (Result.Ramps.Num() >= MaxTotalRamps) break;

				const int32 AIdx = CellPair.Key;
				const int32 BIdx = CellPair.Value;
				const FT66PlatformNode& PA = Result.Platforms[AIdx];
				const FT66PlatformNode& PB = Result.Platforms[BIdx];
				const bool bAlongX = (PA.GridRow == PB.GridRow);

				const int32 PrevNum = Result.Ramps.Num();
				AddRampForBoundary(Result, Preset, CellSize, AIdx, BIdx, bAlongX, Rng, Used);
				if (Result.Ramps.Num() == PrevNum) continue;

				const FT66RampEdge& Candidate = Result.Ramps.Last();
				if (DoesRampOverlapAny(Result, Candidate, AcceptedFootprints, RampPadding))
				{
					Result.Ramps.Pop();
					Used.Remove(MakeBoundaryKey(AIdx, BIdx));
					continue;
				}

				AcceptedFootprints.Add({ BuildRampFootprint(Result, Candidate, RampPadding) });
				++PlacedForBoundary;
			}
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

			// BFS from region 0 to find all reachable regions.
			TArray<bool> Reachable;
			Reachable.Init(false, NumRegions);
			TArray<int32> BFS;
			BFS.Add(0);
			Reachable[0] = true;
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
					AddRampForBoundary(Result, Preset, CellSize, CellIdx, NIdx, bAlongX, Rng, Used);
					if (Result.Ramps.Num() > 0)
					{
						AcceptedFootprints.Add({ BuildRampFootprint(Result, Result.Ramps.Last(), RampPadding) });
						bFixed = true;
					}
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
	}
}

FT66ProceduralMapResult FT66ProceduralMapGenerator::Generate(const FT66MapPreset& Preset)
{
	FT66ProceduralMapResult Result;

	const int32 GridSize = FMath::Clamp(Preset.GridSize, 2, 128);
	const float MapSize = Preset.MapHalfExtent * 2.f;
	const float CellSize = MapSize / static_cast<float>(GridSize);

	InitializePlatforms(Result, Preset, GridSize, CellSize);
	GenerateRegionHeights(Result.Platforms, Preset, GridSize);
	ForceOuterRingToMaxHeight(Result.Platforms, Preset, GridSize);
	ClampAdjacency(Result.Platforms, Preset, GridSize);
	const int32 FlattenedTraversalPlatforms = FlattenReservedTraversalZones(Result.Platforms, Preset);
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

	if (FlattenedTraversalPlatforms > 0)
	{
		UE_LOG(LogT66, Log, TEXT("[MAP] Flattened %d reserved traversal platforms to baseline %.0f."),
			FlattenedTraversalPlatforms,
			Preset.BaselineZ);
	}

	Result.bValid = true;
	return Result;
}
