// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66ProceduralLandscapeParams.generated.h"

UENUM(BlueprintType)
enum class ET66MapTheme : uint8
{
	Farm     UMETA(DisplayName = "Farm"),
	Ocean    UMETA(DisplayName = "Ocean"),
	Mountain UMETA(DisplayName = "Mountain"),
};

USTRUCT(BlueprintType)
struct FT66MapPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Seed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ET66MapTheme Theme = ET66MapTheme::Farm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElevationMin = -1400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElevationMax = 1400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1", ClampMax = "1"))
	float ElevationBias = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "2", ClampMax = "128"))
	int32 GridSize = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaselineZ = 0.f;

	/** Thickness of the visible top surface slabs (UU). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SurfaceThickness = 120.f;

	/** Extra depth below the local low side for vertical walls (UU). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WallBottomPadding = 80.f;

	/** Quantization step used for plateau heights (UU). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElevationStep = 700.f;

	/** Broad terrain undulation scale in grid cells. Larger values create wider hill groupings. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MacroNoiseScaleCells = 6.0f;

	/** Broad terrain undulation strength in elevation steps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MacroNoiseSteps = 1.5f;

	/** Mid-frequency breakup scale in grid cells. Smaller values create more local hill variation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DetailNoiseScaleCells = 2.5f;

	/** Mid-frequency breakup strength in elevation steps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DetailNoiseSteps = 0.8f;

	/** Ridged noise scale in grid cells used to form sightline-breaking spines. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RidgeNoiseScaleCells = 4.0f;

	/** Ridged noise strength in elevation steps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RidgeNoiseSteps = 1.0f;

	/** Extra lift for mid-map ridges that help block long cross-map views. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OcclusionRidgeSteps = 1.2f;

	/** Variable ramp depth range (UU). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RampDepthMin = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RampDepthMax = 5000.f;

	/** Variable ramp width range as fraction of cell size [0..1]. Narrow = cliff doorways. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RampWidthMinAlpha = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RampWidthMaxAlpha = 0.40f;

	/** Number of region seed cells placed on the grid. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RegionSeedCountMin = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RegionSeedCountMax = 8;

	/** Max height-step difference allowed between adjacent cells (multiples of ElevationStep). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAdjacentSteps = 2;

	/** Ramp run distance per 1uu rise. Larger = shallower/more walkable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RampRunPerRise = 2.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MapHalfExtent = 50000.f;

	/** Unity Megabonk hilliness value [0..1]. Their generator uses hilliness / 5 as slope-raise chance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float FarmHilliness = 0.5f;

	static FT66MapPreset GetDefaultForTheme(ET66MapTheme InTheme)
	{
		FT66MapPreset P;
		P.Theme = InTheme;
		switch (InTheme)
		{
		case ET66MapTheme::Farm:
			P.ElevationMin        = -5000.f;
			P.ElevationMax        = 5000.f;
			P.ElevationBias       = 0.0f;
			P.GridSize            = 20;
			P.BaselineZ           = -500.f;
			P.SurfaceThickness    = 120.f;
			P.WallBottomPadding   = 120.f;
			P.ElevationStep       = 1000.f;
			P.MacroNoiseScaleCells = 8.40f;
			P.MacroNoiseSteps     = 1.30f;
			P.DetailNoiseScaleCells = 3.10f;
			P.DetailNoiseSteps    = 0.40f;
			P.RidgeNoiseScaleCells = 5.10f;
			P.RidgeNoiseSteps     = 0.75f;
			P.OcclusionRidgeSteps = 0.95f;
			P.RampDepthMin        = 2600.f;
			P.RampDepthMax        = 18000.f;
			P.RampWidthMinAlpha   = 0.14f;
			P.RampWidthMaxAlpha   = 0.94f;
			P.RegionSeedCountMin  = 40;
			P.RegionSeedCountMax  = 52;
			P.MaxAdjacentSteps    = 4;
			P.RampRunPerRise      = 7.50f;
			P.MapHalfExtent       = 2000.f;
			P.FarmHilliness       = 0.5f;
			break;

		case ET66MapTheme::Ocean:
			P.ElevationMin        = -2450.f;
			P.ElevationMax        = 1750.f;
			P.ElevationBias       = -0.35f;
			P.GridSize            = 28;
			P.BaselineZ           = 0.f;
			P.SurfaceThickness    = 140.f;
			P.WallBottomPadding   = 120.f;
			P.ElevationStep       = 350.f;
			P.MacroNoiseScaleCells = 7.75f;
			P.MacroNoiseSteps     = 1.70f;
			P.DetailNoiseScaleCells = 2.65f;
			P.DetailNoiseSteps    = 0.80f;
			P.RidgeNoiseScaleCells = 4.60f;
			P.RidgeNoiseSteps     = 1.05f;
			P.OcclusionRidgeSteps = 1.45f;
			P.RampDepthMin        = 1500.f;
			P.RampDepthMax        = 5000.f;
			P.RampWidthMinAlpha   = 0.12f;
			P.RampWidthMaxAlpha   = 0.40f;
			P.RegionSeedCountMin  = 10;
			P.RegionSeedCountMax  = 15;
			P.MaxAdjacentSteps    = 2;
			P.RampRunPerRise      = 4.00f;
			break;

		case ET66MapTheme::Mountain:
			P.ElevationMin        = -1050.f;
			P.ElevationMax        = 3150.f;
			P.ElevationBias       = 0.35f;
			P.GridSize            = 30;
			P.BaselineZ           = 0.f;
			P.SurfaceThickness    = 140.f;
			P.WallBottomPadding   = 120.f;
			P.ElevationStep       = 350.f;
			P.MacroNoiseScaleCells = 6.70f;
			P.MacroNoiseSteps     = 2.05f;
			P.DetailNoiseScaleCells = 2.30f;
			P.DetailNoiseSteps    = 0.90f;
			P.RidgeNoiseScaleCells = 3.95f;
			P.RidgeNoiseSteps     = 1.35f;
			P.OcclusionRidgeSteps = 1.55f;
			P.RampDepthMin        = 1500.f;
			P.RampDepthMax        = 5000.f;
			P.RampWidthMinAlpha   = 0.12f;
			P.RampWidthMaxAlpha   = 0.40f;
			P.RegionSeedCountMin  = 11;
			P.RegionSeedCountMax  = 17;
			P.MaxAdjacentSteps    = 2;
			P.RampRunPerRise      = 4.20f;
			break;
		}
		return P;
	}
};
