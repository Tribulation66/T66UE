// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66ProceduralLandscapeParams.generated.h"

/** Preset for procedural landscape world size. */
UENUM(BlueprintType)
enum class ET66LandscapeSizePreset : uint8
{
	/** Main run map: 505x505 vertices (~50.5k x 50.5k UU), square. */
	MainMap UMETA(DisplayName = "Main Map (500x500)"),
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
	float HillAmplitude = 3500.f;  // World Z range in UU (100 UU = 1 m). ~35 m tall hills; was 25000 (too large).

	/** Large hills: wavelength in meters. ~165 gives ~3 large hills, spread apart (min spacing enforced in code). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "10"))
	float VeryLargeScaleMeters = 165.f;

	/** Medium hills: wavelength in meters. ~52 gives ~7 medium hills inside miasma. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "10"))
	float LargeScaleMeters = 52.f;

	/** Min distance (UU) between large-hill peaks; peaks closer than this are suppressed so large hills stay spread. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "1000"))
	float LargeHillMinSpacingUU = 7000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "10"))
	float MediumScaleMeters = 22.f;

	/** Set 0 to disable small bumps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "0"))
	float SmallScaleMeters = 0.f;

	/** If true, only very-large-scale noise (no medium). If false, mix large + medium for ~3 large + ~4 medium hills. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape")
	bool bOnlyVeryLargeHills = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "1", ClampMax = "10"))
	int32 SmoothPasses = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float SmoothStrength = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape")
	bool bCarveRiverValley = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (EditCondition = "bCarveRiverValley", ClampMin = "10"))
	float RiverWidthMeters = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (EditCondition = "bCarveRiverValley", ClampMin = "0"))
	float RiverDepthUU = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape")
	ET66LandscapeSizePreset SizePreset = ET66LandscapeSizePreset::MainMap;

	/** World X of landscape vertex (0,0). 505x505 @ 100 UU/quad: use -25200 for map centered at 0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape")
	float LandscapeOriginX = -25200.f;

	/** World Y of landscape vertex (0,0). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape")
	float LandscapeOriginY = -25200.f;
};
