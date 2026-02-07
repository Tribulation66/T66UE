// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"

/**
 * Deterministic procedural rolling-hills heightfield generator.
 * Used by both editor tool and runtime (GameMode) to produce the same terrain from a seed.
 */
class T66_API FT66ProceduralLandscapeGenerator
{
public:
	/**
	 * Generate a heightfield (floats, world Z in UU) into OutHeights.
	 * Grid is SizeX x SizeY (vertex count). World position at (Ix,Jy) is (LandscapeOriginX + Ix*QuadSizeUU, LandscapeOriginY + Jy*QuadSizeUU).
	 * Applies flat zones (start area, boss area) at Z=0 with smooth blend. Returns false if params or dimensions are invalid.
	 */
	static bool GenerateHeightfield(
		const FT66ProceduralLandscapeParams& Params,
		int32 SizeX,
		int32 SizeY,
		float QuadSizeUU,
		TArray<float>& OutHeights);

	/**
	 * Convert float heights (world Z) to Landscape uint16 height data.
	 * Mid = 32768, ScaleZ is the landscape scale Z (e.g. 100).
	 */
	static void FloatsToLandscapeHeightData(const TArray<float>& Heights, float ScaleZ, TArray<uint16>& OutHeightData);

	/**
	 * Get grid dimensions from preset. Returns (SizeX, SizeY) vertex count.
	 * Small: 505x505, Large: 1009x1009 (or similar; must match Landscape component layout).
	 */
	static void GetDimensionsForPreset(ET66LandscapeSizePreset Preset, int32& OutSizeX, int32& OutSizeY);

	/** Default quad size in UU per vertex step (1 m = 100 UU typical). */
	static constexpr float DefaultQuadSizeUU = 100.f;
};
