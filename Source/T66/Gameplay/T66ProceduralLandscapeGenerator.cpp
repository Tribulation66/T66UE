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
	const bool bUseSmallScale = Params.SmallScaleMeters >= 1.f;
	const bool bOnlyVL = Params.bOnlyVeryLargeHills;

	OutHeights.SetNumUninitialized(SizeX * SizeY);

	const int32 Seed = Params.Seed;
	const float A = Params.HillAmplitude;
	const float VL = Params.VeryLargeScaleMeters;
	const float L = Params.LargeScaleMeters;
	const float M = Params.MediumScaleMeters;
	const float S = bUseSmallScale ? Params.SmallScaleMeters : 0.f;
	const float OriginX = Params.LandscapeOriginX;
	const float OriginY = Params.LandscapeOriginY;

	for (int32 Jy = 0; Jy < SizeY; ++Jy)
	{
		for (int32 Ix = 0; Ix < SizeX; ++Ix)
		{
			const float WorldX = OriginX + Ix * QuadSizeUU;
			const float WorldY = OriginY + Jy * QuadSizeUU;
			// Convert UU to meters (1 m = 100 UU)
			const float WxM = WorldX / 100.f;
			const float WyM = WorldY / 100.f;

			float H;
			if (bOnlyVL)
			{
				H = T66NoiseSigned(Seed, WxM, WyM, VL);  // Single octave: sparse very large hills only
			}
			else
			{
				// Mix: ~3 large hills (VL) + ~4 medium hills (L) inside miasma; same height from amplitude
				H = 0.55f * T66NoiseSigned(Seed, WxM, WyM, VL)
					+ 0.45f * T66NoiseSigned(Seed + 1, WxM, WyM, L);
				// Optional extra detail (disable for clean 3 large + 4 medium)
				// H += 0.14f * T66NoiseSigned(Seed + 2, WxM, WyM, M);
				// if (bUseSmallScale) H += 0.08f * T66NoiseSigned(Seed + 3, WxM, WyM, S);
			}
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

	// Enforce large hills spread: large-hill peaks cannot be right next to each other (within LargeHillMinSpacingUU)
	if (!bOnlyVL && Params.LargeHillMinSpacingUU > 0.f)
	{
		TArray<float> VLOnly;
		VLOnly.SetNumUninitialized(SizeX * SizeY);
		for (int32 Jy = 0; Jy < SizeY; ++Jy)
		{
			for (int32 Ix = 0; Ix < SizeX; ++Ix)
			{
				const float WxM = (OriginX + Ix * QuadSizeUU) / 100.f;
				const float WyM = (OriginY + Jy * QuadSizeUU) / 100.f;
				VLOnly[Jy * SizeX + Ix] = T66NoiseSigned(Seed, WxM, WyM, VL);
			}
		}
		struct FPeak { int32 Idx; float Val; float Wx; float Wy; };
		TArray<FPeak> Peaks;
		for (int32 Jy = 1; Jy < SizeY - 1; ++Jy)
		{
			for (int32 Ix = 1; Ix < SizeX - 1; ++Ix)
			{
				const int32 Idx = Jy * SizeX + Ix;
				const float V = VLOnly[Idx];
				if (V <= VLOnly[Idx - 1] || V <= VLOnly[Idx + 1] || V <= VLOnly[Idx - SizeX] || V <= VLOnly[Idx + SizeX] ||
				    V <= VLOnly[Idx - SizeX - 1] || V <= VLOnly[Idx - SizeX + 1] || V <= VLOnly[Idx + SizeX - 1] || V <= VLOnly[Idx + SizeX + 1])
					continue;
				const float Wx = OriginX + Ix * QuadSizeUU;
				const float Wy = OriginY + Jy * QuadSizeUU;
				Peaks.Add({ Idx, V, Wx, Wy });
			}
		}
		Peaks.Sort([](const FPeak& A, const FPeak& B) { return A.Val > B.Val; });
		const float MinSpacingSq = Params.LargeHillMinSpacingUU * Params.LargeHillMinSpacingUU;
		for (int32 i = 0; i < Peaks.Num(); ++i)
		{
			const FPeak& P = Peaks[i];
			bool bTooCloseToHigher = false;
			for (int32 j = 0; j < i; ++j)
			{
				const float Dx = P.Wx - Peaks[j].Wx;
				const float Dy = P.Wy - Peaks[j].Wy;
				if ((Dx * Dx + Dy * Dy) < MinSpacingSq)
				{
					bTooCloseToHigher = true;
					break;
				}
			}
			if (bTooCloseToHigher)
			{
				OutHeights[P.Idx] *= 0.28f;  // dampen so this is no longer a distinct large-hill peak
			}
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

	// Flat zones at world Z=0 with smooth blend into hills (start area and boss area)
	static constexpr float StartMinX = -17600.f, StartMaxX = -13600.f, StartMinY = -2000.f, StartMaxY = 2000.f;
	static constexpr float BossMinX = 13600.f, BossMaxX = 17600.f, BossMinY = -2000.f, BossMaxY = 2000.f;
	const float BlendRadiusUU = 2500.f;  // UU over which to blend from flat to hills (wider = gentler slope)
	const auto BlendTowardFlat = [BlendRadiusUU](float WorldVal, float ZoneMin, float ZoneMax) -> float
	{
		const float DistOut = FMath::Max(ZoneMin - WorldVal, FMath::Max(WorldVal - ZoneMax, 0.f));
		const float T = FMath::Clamp(1.f - DistOut / BlendRadiusUU, 0.f, 1.f);
		return FMath::SmoothStep(0.f, 1.f, T); // smoother transition at the edge
	};

	for (int32 Jy = 0; Jy < SizeY; ++Jy)
	{
		for (int32 Ix = 0; Ix < SizeX; ++Ix)
		{
			const float WorldX = OriginX + Ix * QuadSizeUU;
			const float WorldY = OriginY + Jy * QuadSizeUU;
			const int32 Idx = Jy * SizeX + Ix;
			float H = OutHeights[Idx];

			const float StartBlendX = BlendTowardFlat(WorldX, StartMinX, StartMaxX);
			const float StartBlendY = BlendTowardFlat(WorldY, StartMinY, StartMaxY);
			const float StartBlend = StartBlendX * StartBlendY;

			const float BossBlendX = BlendTowardFlat(WorldX, BossMinX, BossMaxX);
			const float BossBlendY = BlendTowardFlat(WorldY, BossMinY, BossMaxY);
			const float BossBlend = BossBlendX * BossBlendY;

			const float FlatBlend = FMath::Max(StartBlend, BossBlend);
			if (FlatBlend > 0.f)
			{
				H = FMath::Lerp(H, 0.f, FlatBlend);
			}
			OutHeights[Idx] = H;
		}
	}

	// Constrain hills to inside miasma walls (SafeHalfExtent = 18400). Outside this box, blend height to 0 so no hills extend past the walls.
	static constexpr float MiasmaHalfExtent = 18400.f;
	static constexpr float MiasmaBlendMarginUU = 600.f;  // Over this distance inside the wall, blend down so peaks don't stick out
	for (int32 Jy = 0; Jy < SizeY; ++Jy)
	{
		for (int32 Ix = 0; Ix < SizeX; ++Ix)
		{
			const float WorldX = OriginX + Ix * QuadSizeUU;
			const float WorldY = OriginY + Jy * QuadSizeUU;
			const int32 Idx = Jy * SizeX + Ix;
			const float DistInX = MiasmaHalfExtent - FMath::Abs(WorldX);
			const float DistInY = MiasmaHalfExtent - FMath::Abs(WorldY);
			// Mask = 1 fully inside; 0 outside; smooth blend in the margin band
			const float Tx = FMath::Clamp(DistInX / MiasmaBlendMarginUU, 0.f, 1.f);
			const float Ty = FMath::Clamp(DistInY / MiasmaBlendMarginUU, 0.f, 1.f);
			const float Mask = FMath::Min(Tx, Ty);
			const float SmoothedMask = Mask * Mask * (3.f - 2.f * Mask);
			OutHeights[Idx] *= SmoothedMask;
		}
	}

	// Z=0 is the lowest point: clamp so no geometry below 0 (flat zones stay at 0; prevents spawn under ground)
	for (float& Hi : OutHeights)
	{
		Hi = FMath::Max(0.f, Hi);
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
	// UE Landscape: SectionSize=63, SectionsPerComponent=1 => (Size-1) must be multiple of 63.
	// MainMap: 505x505 (500x500 requested; 505 = 8*63+1 is nearest valid).
	// Small: 505x505. Large: 1009x1009.
	switch (Preset)
	{
	case ET66LandscapeSizePreset::MainMap:
		OutSizeX = 8 * 63 + 1;  // 505 -> 50400 UU
		OutSizeY = 8 * 63 + 1;  // 505
		break;
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
