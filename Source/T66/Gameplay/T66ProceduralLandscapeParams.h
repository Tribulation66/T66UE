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
	float ElevationMin = -1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElevationMax = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1", ClampMax = "1"))
	float ElevationBias = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "2", ClampMax = "128"))
	int32 GridSize = 100;

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
	float ElevationStep = 400.f;

	/** Variable ramp depth range (UU). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RampDepthMin = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RampDepthMax = 900.f;

	/** Variable ramp width range as fraction of cell size [0..1]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RampWidthMinAlpha = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RampWidthMaxAlpha = 0.9f;

	/** Number of large macro features stamped into the baseline. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FeatureCountMin = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FeatureCountMax = 36;

	/** Stroke length in cells for distributed plateau lanes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FeatureLengthMinCells = 12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FeatureLengthMaxCells = 40;

	/** Stroke half-width in cells for distributed plateau lanes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FeatureWidthMinCells = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FeatureWidthMaxCells = 6;

	/** Random side rooms / plateaus carved off the lanes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RoomChance = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAdjacentSteps = 2;

	/** Ramp run distance per 1uu rise. Larger = shallower/more walkable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RampRunPerRise = 2.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MapHalfExtent = 50000.f;

	static FT66MapPreset GetDefaultForTheme(ET66MapTheme InTheme)
	{
		FT66MapPreset P;
		P.Theme = InTheme;
		switch (InTheme)
		{
		case ET66MapTheme::Farm:
			P.ElevationMin      = -1200.f;
			P.ElevationMax      = 1200.f;
			P.ElevationBias     = 0.f;
			P.GridSize          = 100;
			P.BaselineZ         = 0.f;
			P.SurfaceThickness  = 120.f;
			P.WallBottomPadding = 80.f;
			P.ElevationStep     = 400.f;
			P.RampDepthMin      = 800.f;
			P.RampDepthMax      = 2600.f;
			P.RampWidthMinAlpha = 0.45f;
			P.RampWidthMaxAlpha = 0.75f;
			P.FeatureCountMin   = 16;
			P.FeatureCountMax   = 20;
			P.FeatureLengthMinCells = 6;
			P.FeatureLengthMaxCells = 16;
			P.FeatureWidthMinCells = 2;
			P.FeatureWidthMaxCells = 4;
			P.RoomChance       = 0.10f;
			P.MaxAdjacentSteps = 3;
			P.RampRunPerRise   = 2.8f;
			break;

		case ET66MapTheme::Ocean:
			P.ElevationMin      = -8000.f;
			P.ElevationMax      = 0.f;
			P.ElevationBias     = -0.5f;
			P.GridSize          = 100;
			P.BaselineZ         = 0.f;
			P.SurfaceThickness  = 140.f;
			P.WallBottomPadding = 120.f;
			P.ElevationStep     = 800.f;
			P.RampDepthMin      = 1400.f;
			P.RampDepthMax      = 5200.f;
			P.RampWidthMinAlpha = 0.45f;
			P.RampWidthMaxAlpha = 0.80f;
			P.FeatureCountMin   = 18;
			P.FeatureCountMax   = 22;
			P.FeatureLengthMinCells = 8;
			P.FeatureLengthMaxCells = 20;
			P.FeatureWidthMinCells = 2;
			P.FeatureWidthMaxCells = 4;
			P.RoomChance       = 0.12f;
			P.MaxAdjacentSteps = 3;
			P.RampRunPerRise   = 2.5f;
			break;

		case ET66MapTheme::Mountain:
			P.ElevationMin      = -1600.f;
			P.ElevationMax      = 8000.f;
			P.ElevationBias     = 0.35f;
			P.GridSize          = 100;
			P.BaselineZ         = 0.f;
			P.SurfaceThickness  = 140.f;
			P.WallBottomPadding = 120.f;
			P.ElevationStep     = 800.f;
			P.RampDepthMin      = 1600.f;
			P.RampDepthMax      = 6000.f;
			P.RampWidthMinAlpha = 0.45f;
			P.RampWidthMaxAlpha = 0.80f;
			P.FeatureCountMin   = 18;
			P.FeatureCountMax   = 22;
			P.FeatureLengthMinCells = 8;
			P.FeatureLengthMaxCells = 22;
			P.FeatureWidthMinCells = 2;
			P.FeatureWidthMaxCells = 5;
			P.RoomChance       = 0.12f;
			P.MaxAdjacentSteps = 4;
			P.RampRunPerRise   = 2.4f;
			break;
		}
		return P;
	}
};
