// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BounceCourseAuditCommandlet.h"

#include "Data/T66DataTypes.h"
#include "Gameplay/T66MainMapTerrain.h"
#include "Gameplay/T66TowerMapTerrain.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66BounceCourseAudit, Log, All);

UT66BounceCourseAuditCommandlet::UT66BounceCourseAuditCommandlet()
{
	IsClient = false;
	IsEditor = false;
	IsServer = false;
	LogToConsole = true;
}

int32 UT66BounceCourseAuditCommandlet::Main(const FString& Params)
{
	int32 SeedCount = 50;
	FParse::Value(*Params, TEXT("seeds="), SeedCount);
	SeedCount = FMath::Clamp(SeedCount, 1, 1000);

	const ET66Difficulty Difficulties[] = {
		ET66Difficulty::Easy,
		ET66Difficulty::Medium,
		ET66Difficulty::Hard,
		ET66Difficulty::VeryHard,
		ET66Difficulty::Impossible
	};

	int32 LayoutCount = 0;
	int32 FloorCount = 0;
	int32 FailedFloorCount = 0;
	int32 FailedLayoutCount = 0;
	float WorstChainGap = 0.0f;
	float WorstHoleReach = 0.0f;
	int32 TotalPlatforms = 0;
	int32 TotalRamps = 0;
	int32 TotalMesas = 0;
	int32 TotalTierRamps = 0;
	int32 TotalTierLifts = 0;
	int32 TotalRingMesas = 0;
	int32 FloorsWithMesas = 0;
	int32 FloorsWithLifts = 0;
	int32 MesaAccessFailCount = 0;

	for (const ET66Difficulty Difficulty : Difficulties)
	{
		for (int32 SeedIndex = 0; SeedIndex < SeedCount; ++SeedIndex)
		{
			const int32 Seed = 1000 + (SeedIndex * 37);
			const FT66MapPreset Preset = T66MainMapTerrain::BuildPresetForDifficulty(Difficulty, Seed);
			T66TowerMapTerrain::FLayout Layout;
			if (!T66TowerMapTerrain::BuildLayout(Preset, Layout))
			{
				++FailedLayoutCount;
				UE_LOG(
					LogT66BounceCourseAudit,
					Error,
					TEXT("[T66Proof][BounceCourseAudit] BuildLayout FAILED difficulty=%d seed=%d"),
					static_cast<int32>(Difficulty),
					Seed);
				continue;
			}

			++LayoutCount;
			bool bLayoutFailed = false;
			for (const T66TowerMapTerrain::FFloor& Floor : Layout.Floors)
			{
				if (!Floor.bMobFloor)
				{
					continue;
				}

				++FloorCount;
				TotalPlatforms += Floor.BouncePlatforms.Num();
				TotalRamps += Floor.BounceRamps.Num();
				TotalMesas += Floor.TierMesas.Num();
				TotalTierRamps += Floor.TierRamps.Num();
				TotalTierLifts += Floor.TierLifts.Num();
				if (Floor.TierMesas.Num() > 0)
				{
					++FloorsWithMesas;
				}
				if (Floor.TierLifts.Num() > 0)
				{
					++FloorsWithLifts;
				}

				// Mesa access invariant: every mesa keeps >= MesaRampsMin always-walkable
				// ramps AND >= 2 total routes (lift edges count as up-edges). A lift may
				// only ever replace a SURPLUS ramp candidate.
				for (const T66TowerMapTerrain::FTierMesa& Mesa : Floor.TierMesas)
				{
					if (Mesa.HasHole())
					{
						++TotalRingMesas;
					}
					auto AscendsIntoMesa = [&Mesa](const FIntPoint& Cell, const FIntPoint& AscentSign)
					{
						const FIntPoint Target = Cell + AscentSign;
						return Target.X >= Mesa.MinCell.X && Target.X < Mesa.MaxCellExclusive.X
							&& Target.Y >= Mesa.MinCell.Y && Target.Y < Mesa.MaxCellExclusive.Y;
					};

					int32 RampRoutes = 0;
					for (const T66TowerMapTerrain::FTierRamp& TierRamp : Floor.TierRamps)
					{
						if (AscendsIntoMesa(TierRamp.Cell, TierRamp.AscentSign))
						{
							++RampRoutes;
						}
					}
					int32 LiftRoutes = 0;
					for (const T66TowerMapTerrain::FTierLift& TierLift : Floor.TierLifts)
					{
						if (AscendsIntoMesa(TierLift.Cell, TierLift.AscentSign))
						{
							++LiftRoutes;
						}
					}

					const int32 RequiredRamps = FMath::Max(2, Layout.MesaRampsMin);
					if (RampRoutes < RequiredRamps || RampRoutes + LiftRoutes < 2)
					{
						++MesaAccessFailCount;
						bLayoutFailed = true;
						UE_LOG(
							LogT66BounceCourseAudit,
							Error,
							TEXT("[T66Proof][BounceCourseAudit] Mesa access FAIL difficulty=%d seed=%d floor=%d roomId=%d ramps=%d lifts=%d requiredRamps=%d"),
							static_cast<int32>(Difficulty),
							Seed,
							Floor.FloorNumber,
							Mesa.RoomId,
							RampRoutes,
							LiftRoutes,
							RequiredRamps);
					}
				}

				// The generator clears SafeChainCells when its own validation fails,
				// so a populated chain is the pass marker; recompute the gap stats
				// here for the aggregate report.
				const bool bFloorPass = Floor.SafeChainCells.Num() > 0 && Floor.BouncePlatforms.Num() > 0;
				if (!bFloorPass)
				{
					++FailedFloorCount;
					bLayoutFailed = true;
					UE_LOG(
						LogT66BounceCourseAudit,
						Error,
						TEXT("[T66Proof][BounceCourseAudit] Floor FAIL difficulty=%d seed=%d floor=%d chainCells=%d platforms=%d"),
						static_cast<int32>(Difficulty),
						Seed,
						Floor.FloorNumber,
						Floor.SafeChainCells.Num(),
						Floor.BouncePlatforms.Num());
					continue;
				}

				// Dry anchors along the path mirror the in-generator validation: chain
				// platforms AND tier-ramp wedges both count as dry surfaces.
				auto BoxGap = [](const FBox2D& A, const FBox2D& B)
				{
					const float GapX = FMath::Max(FMath::Max(A.Min.X - B.Max.X, B.Min.X - A.Max.X), 0.0f);
					const float GapY = FMath::Max(FMath::Max(A.Min.Y - B.Max.Y, B.Min.Y - A.Max.Y), 0.0f);
					return FMath::Max(GapX, GapY);
				};

				const FBox2D* PreviousAnchor = nullptr;
				const FBox2D* LastAnchor = nullptr;
				TArray<FBox2D> AnchorStorage;
				AnchorStorage.Reserve(Floor.SafeChainCells.Num());
				for (const FIntPoint& PathCell : Floor.SafeChainCells)
				{
					const FBox2D* Anchor = nullptr;
					for (const T66TowerMapTerrain::FBouncePlatform& Platform : Floor.BouncePlatforms)
					{
						if (Platform.bSafeChain && Platform.Cell == PathCell)
						{
							Anchor = &Platform.Bounds;
							break;
						}
					}
					if (!Anchor)
					{
						for (const T66TowerMapTerrain::FTierRamp& TierRamp : Floor.TierRamps)
						{
							if (TierRamp.Cell == PathCell)
							{
								Anchor = &TierRamp.Bounds;
								break;
							}
						}
					}
					if (!Anchor)
					{
						continue;
					}

					AnchorStorage.Add(*Anchor);
					if (PreviousAnchor)
					{
						WorstChainGap = FMath::Max(WorstChainGap, BoxGap(AnchorStorage.Last(1), AnchorStorage.Last()));
					}
					PreviousAnchor = &AnchorStorage.Last();
					LastAnchor = PreviousAnchor;
				}

				if (LastAnchor && Floor.bHasDropHole)
				{
					const FBox2D HoleBox(
						FVector2D(Floor.HoleCenter.X - Floor.HoleHalfExtent.X, Floor.HoleCenter.Y - Floor.HoleHalfExtent.Y),
						FVector2D(Floor.HoleCenter.X + Floor.HoleHalfExtent.X, Floor.HoleCenter.Y + Floor.HoleHalfExtent.Y));
					WorstHoleReach = FMath::Max(WorstHoleReach, BoxGap(*LastAnchor, HoleBox));
				}
			}

			if (bLayoutFailed)
			{
				++FailedLayoutCount;
			}
		}
	}

	// Tier-access failures flatten the floor and clear its chain markers inside
	// BuildLayout, so the chain check above already fails those floors; the tier
	// totals confirm the mesas/ramps actually generate.
	const bool bAllPassed = FailedFloorCount == 0 && FailedLayoutCount == 0 && MesaAccessFailCount == 0 && LayoutCount > 0;
	UE_LOG(
		LogT66BounceCourseAudit,
		Display,
		TEXT("[T66Proof][BounceCourseAudit] Result=%s layouts=%d mobFloors=%d failedFloors=%d failedLayouts=%d mesaAccessFails=%d worstChainGap=%.0f worstHoleReach=%.0f totalPlatforms=%d totalRamps=%d totalMesas=%d totalRingMesas=%d totalTierRamps=%d totalTierLifts=%d floorsWithMesas=%d floorsWithLifts=%d seedsPerDifficulty=%d"),
		bAllPassed ? TEXT("PASS") : TEXT("FAIL"),
		LayoutCount,
		FloorCount,
		FailedFloorCount,
		FailedLayoutCount,
		MesaAccessFailCount,
		WorstChainGap,
		WorstHoleReach,
		TotalPlatforms,
		TotalRamps,
		TotalMesas,
		TotalRingMesas,
		TotalTierRamps,
		TotalTierLifts,
		FloorsWithMesas,
		FloorsWithLifts,
		SeedCount);

	return bAllPassed ? 0 : 1;
}
