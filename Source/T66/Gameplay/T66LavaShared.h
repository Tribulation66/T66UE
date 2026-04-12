#pragma once

#include "CoreMinimal.h"

namespace T66LavaShared
{
	inline constexpr float TwoPi = 6.28318530718f;
	inline constexpr const TCHAR* BaseMaterialPath = TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit");
	inline constexpr const TCHAR* DefaultTexturePath = TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture");

	inline float Frac(float Value)
	{
		return Value - FMath::FloorToFloat(Value);
	}

	inline float Saturate(float Value)
	{
		return FMath::Clamp(Value, 0.f, 1.f);
	}

	inline float SmoothStep(float Edge0, float Edge1, float Value)
	{
		if (FMath::IsNearlyEqual(Edge0, Edge1))
		{
			return Value < Edge0 ? 0.f : 1.f;
		}

		const float T = Saturate((Value - Edge0) / (Edge1 - Edge0));
		return T * T * (3.f - 2.f * T);
	}

	inline FVector2D Hash2(const FVector2D& P)
	{
		return FVector2D(
			Frac(FMath::Sin(FVector2D::DotProduct(P, FVector2D(127.1f, 311.7f))) * 43758.5453123f),
			Frac(FMath::Sin(FVector2D::DotProduct(P, FVector2D(269.5f, 183.3f))) * 43758.5453123f));
	}

	struct FCellSample
	{
		float Closest = BIG_NUMBER;
		float SecondClosest = BIG_NUMBER;
	};

	inline FCellSample SampleCells(const FVector2D& UV, float Density, float Phase)
	{
		FCellSample Result;

		const FVector2D P = UV * Density;
		const FVector2D Cell(FMath::FloorToFloat(P.X), FMath::FloorToFloat(P.Y));
		const FVector2D Local = P - Cell;

		for (int32 Y = -1; Y <= 1; ++Y)
		{
			for (int32 X = -1; X <= 1; ++X)
			{
				const FVector2D NeighborCell = Cell + FVector2D(static_cast<float>(X), static_cast<float>(Y));
				const FVector2D Jitter = Hash2(NeighborCell);
				const FVector2D Drift(
					FMath::Sin(Phase * (0.65f + Jitter.X * 0.25f) + NeighborCell.Y * 1.73f + Jitter.Y * 4.0f),
					FMath::Cos(Phase * (0.50f + Jitter.Y * 0.25f) + NeighborCell.X * 1.37f + Jitter.X * 4.0f));
				const FVector2D FeaturePoint =
					FVector2D(static_cast<float>(X), static_cast<float>(Y)) + Jitter + Drift * 0.16f;

				const float DistSq = (Local - FeaturePoint).SizeSquared();
				if (DistSq < Result.Closest)
				{
					Result.SecondClosest = Result.Closest;
					Result.Closest = DistSq;
				}
				else if (DistSq < Result.SecondClosest)
				{
					Result.SecondClosest = DistSq;
				}
			}
		}

		Result.Closest = FMath::Sqrt(Result.Closest);
		Result.SecondClosest = FMath::Sqrt(Result.SecondClosest);
		return Result;
	}
}
