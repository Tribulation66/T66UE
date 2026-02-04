// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ProceduralLandscapeGenerator.h"
#include "T66.h"

#include "Math/UnrealMathUtility.h"

namespace
{
	// Deterministic 2D value noise (hash-based). Same seed + (x,y) => same value in [0,1].
	inline uint32 T66Hash(int32 Seed, int32 X, int32 Y)
	{
		uint32 H = static_cast<uint32>(Seed);
		H = (H * 31) + static_cast<uint32>(X);
		H = (H * 31) + static_cast<uint32>(Y);
		H ^= (H >> 16);
		H *= 0x85ebca6b;
		H ^= (H >> 13);
		H *= 0xc2b2ae35;
		H ^= (H >> 16);
		return H;
	}

	inline float T66ValueNoise(int32 Seed, float X, float Y, float ScaleMeters)
	{
		// Scale so wavelength ~ ScaleMeters; 1 unit in (X,Y) = 1 meter when ScaleMeters=1.
		const float S = FMath::Max(ScaleMeters, 1.f);
		const float Nx = X / S;
		const float Ny = Y / S;
		const int32 I0 = FMath::FloorToInt(Nx);
		const int32 J0 = FMath::FloorToInt(Ny);
		const float Fx = Nx - I0;
		const float Fy = Ny - J0;
		const float FxS = Fx * Fx * (3.f - 2.f * Fx);
		const float FyS = Fy * Fy * (3.f - 2.f * Fy);

		auto V = [Seed](int32 i, int32 j) {
			return (T66Hash(Seed, i, j) % 65536u) / 65535.f;
		};

		const float V00 = V(I0, J0);
		const float V10 = V(I0 + 1, J0);
		const float V01 = V(I0, J0 + 1);
		const float V11 = V(I0 + 1, J0 + 1);
		const float Vx0 = FMath::Lerp(V00, V10, FxS);
		const float Vx1 = FMath::Lerp(V01, V11, FxS);
		return FMath::Lerp(Vx0, Vx1, FyS);
	}

	// Normalize to [-1, 1] for symmetric hills
	inline float T66NoiseSigned(int32 Seed, float WorldX, float WorldY, float ScaleMeters)
	{
		return 2.f * T66ValueNoise(Seed, WorldX, WorldY, ScaleMeters) - 1.f;
	}
}

bool FT66ProceduralLandscapeGenerator::GenerateHeightfield(
	const FT66ProceduralLandscapeParams& Params,
	int32 SizeX,
	int32 SizeY,
	float QuadSizeUU,
	TArray<float>& OutHeights)
{
	if (SizeX < 2 || SizeY < 2)
	{
		UE_LOG(LogT66, Error, TEXT("[MAP] GenerateHeightfield: invalid dimensions SizeX=%d SizeY=%d"), SizeX, SizeY);
		return false;
	}
	if (Params.HillAmplitude <= 0.f || Params.VeryLargeScaleMeters < 10.f || Params.LargeScaleMeters < 10.f || Params.MediumScaleMeters < 10.f)
	{
		UE_LOG(LogT66, Error, TEXT("[MAP] GenerateHeightfield: invalid params (amplitude or scales)"));
		return false;
	}

	OutHeights.SetNumUninitialized(SizeX * SizeY);

	const int32 Seed = Params.Seed;
	const float A = Params.HillAmplitude;
	const float VL = Params.VeryLargeScaleMeters;
	const float L = Params.LargeScaleMeters;
	const float M = Params.MediumScaleMeters;

	for (int32 Jy = 0; Jy < SizeY; ++Jy)
	{
		for (int32 Ix = 0; Ix < SizeX; ++Ix)
		{
			const float WorldX = Ix * QuadSizeUU;
			const float WorldY = Jy * QuadSizeUU;
			// Convert UU to meters (1 m = 100 UU)
			const float WxM = WorldX / 100.f;
			const float WyM = WorldY / 100.f;

			float H = 0.65f * T66NoiseSigned(Seed, WxM, WyM, VL)
				+ 0.25f * T66NoiseSigned(Seed + 1, WxM, WyM, L)
				+ 0.10f * T66NoiseSigned(Seed + 2, WxM, WyM, M);
			H *= A;

			if (Params.bCarveRiverValley)
			{
				// Gentle valley along X (center Y): smooth trench
				const float HalfSizeY = (SizeY - 1) * QuadSizeUU * 0.5f;
				const float DistFromCenterY = FMath::Abs(WorldY - HalfSizeY);
				const float RiverHalfWidthUU = Params.RiverWidthMeters * 100.f * 0.5f;
				if (RiverHalfWidthUU > 0.f)
				{
					const float T = FMath::Clamp(DistFromCenterY / RiverHalfWidthUU, 0.f, 1.f);
					const float Falloff = 1.f - FMath::SmoothStep(0.f, 1.f, T);
					H -= Params.RiverDepthUU * Falloff;
				}
			}

			OutHeights[Jy * SizeX + Ix] = H;
		}
	}

	// Smoothing passes (box blur 3x3, blend by SmoothStrength)
	const int32 Passes = FMath::Clamp(Params.SmoothPasses, 1, 10);
	const float Strength = FMath::Clamp(Params.SmoothStrength, 0.1f, 1.f);
	TArray<float> Temp;
	Temp.SetNumUninitialized(SizeX * SizeY);

	for (int32 P = 0; P < Passes; ++P)
	{
		for (int32 Jy = 0; Jy < SizeY; ++Jy)
		{
			for (int32 Ix = 0; Ix < SizeX; ++Ix)
			{
				float Sum = 0.f;
				int32 Count = 0;
				for (int32 dy = -1; dy <= 1; ++dy)
				{
					for (int32 dx = -1; dx <= 1; ++dx)
					{
						const int32 Nx = FMath::Clamp(Ix + dx, 0, SizeX - 1);
						const int32 Ny = FMath::Clamp(Jy + dy, 0, SizeY - 1);
						Sum += OutHeights[Ny * SizeX + Nx];
						++Count;
					}
				}
				const float Avg = Sum / Count;
				const float Current = OutHeights[Jy * SizeX + Ix];
				Temp[Jy * SizeX + Ix] = FMath::Lerp(Current, Avg, Strength);
			}
		}
		Swap(OutHeights, Temp);
	}

	return true;
}

void FT66ProceduralLandscapeGenerator::FloatsToLandscapeHeightData(const TArray<float>& Heights, float ScaleZ, TArray<uint16>& OutHeightData)
{
	OutHeightData.SetNumUninitialized(Heights.Num());
	if (ScaleZ < 1.f) ScaleZ = 100.f;
	for (int32 I = 0; I < Heights.Num(); ++I)
	{
		// Landscape: WorldZ = (height - 32768) * ScaleZ / 128  =>  height = 32768 + WorldZ * 128 / ScaleZ
		const float WorldZ = Heights[I];
		const int32 H = 32768 + FMath::RoundToInt(WorldZ * 128.f / ScaleZ);
		OutHeightData[I] = static_cast<uint16>(FMath::Clamp(H, 0, 65535));
	}
}

void FT66ProceduralLandscapeGenerator::GetDimensionsForPreset(ET66LandscapeSizePreset Preset, int32& OutSizeX, int32& OutSizeY)
{
	// UE Landscape: SectionSize=63, SectionsPerComponent=1 => 63 quads per component, 64 vertices per component.
	// Small: 8x8 components => 64*8+1 = 513? Actually 8*63+1 = 505. So 505 x 505.
	// Large: 16x16 => 16*63+1 = 1009. 1009 x 1009.
	switch (Preset)
	{
	case ET66LandscapeSizePreset::Small:
		OutSizeX = 8 * 63 + 1;  // 505
		OutSizeY = 8 * 63 + 1;
		break;
	case ET66LandscapeSizePreset::Large:
		OutSizeX = 16 * 63 + 1; // 1009
		OutSizeY = 16 * 63 + 1;
		break;
	default:
		OutSizeX = 505;
		OutSizeY = 505;
		break;
	}
}
