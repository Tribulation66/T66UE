// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66ProceduralLandscapeParams.generated.h"

/** Preset for procedural landscape world size. */
UENUM(BlueprintType)
enum class ET66LandscapeSizePreset : uint8
{
	/** Main run map: 1009x1009 vertices (~100.8k x 100.8k UU), square. */
	MainMap UMETA(DisplayName = "Main Map (100k)"),
	/** ~1 km x 1 km (8x8 components = 505x505 vertices) */
	Small UMETA(DisplayName = "Small (~1 km)"),
	/** ~2–4 km (16x16 components) */
	Large UMETA(DisplayName = "Large (~2–4 km)")
};

/** Parameters for procedural rolling-hills landscape generation. All editable in code. */
USTRUCT(BlueprintType)
struct FT66ProceduralLandscapeParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape")
	int32 Seed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "1"))
	float HillAmplitude = 3465.f;  // ~10% taller than 3150; max hill height in UU. Slope limiter keeps climbable.

	/** Large hills: wavelength in meters. Larger = wider hills. ~220 gives ~3 very wide large hills. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "10"))
	float VeryLargeScaleMeters = 220.f;

	/** Medium hills: wavelength in meters. ~72 gives ~7 medium hills, all wide for climbability. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "10"))
	float LargeScaleMeters = 72.f;

	/** Min distance (UU) between large-hill peaks; only peaks closer than this are suppressed (avoid one merged blob). ~2800 = adjacent only. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "1000"))
	float LargeHillMinSpacingUU = 2800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "10"))
	float MediumScaleMeters = 22.f;

	/** Small hills: wavelength in meters. ~18 gives a few small hills; set 0 to disable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "0"))
	float SmallScaleMeters = 18.f;

	/** If true, only very-large-scale noise (no medium). If false, mix large + medium for ~3 large + ~4 medium hills. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape")
	bool bOnlyVeryLargeHills = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "1", ClampMax = "10"))
	int32 SmoothPasses = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float SmoothStrength = 0.2f;

	/** Max slope angle in degrees. Hills steeper than this are flattened so all terrain is climbable. 34° = conservative for 45° walkable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "10", ClampMax = "60"))
	float MaxSlopeDegrees = 34.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape")
	bool bCarveRiverValley = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (EditCondition = "bCarveRiverValley", ClampMin = "10"))
	float RiverWidthMeters = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (EditCondition = "bCarveRiverValley", ClampMin = "0"))
	float RiverDepthUU = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape")
	ET66LandscapeSizePreset SizePreset = ET66LandscapeSizePreset::MainMap;

	/** World X of landscape vertex (0,0). 1009x1009 @ 100 UU/quad: use -50400 for map centered at 0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape")
	float LandscapeOriginX = -50400.f;

	/** World Y of landscape vertex (0,0). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape")
	float LandscapeOriginY = -50400.f;
};
