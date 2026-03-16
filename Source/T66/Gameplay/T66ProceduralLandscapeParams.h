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

	static FT66MapPreset GetDefaultForTheme(ET66MapTheme InTheme)
	{
		FT66MapPreset P;
		P.Theme = InTheme;
		switch (InTheme)
		{
		case ET66MapTheme::Farm:
			P.ElevationMin        = -1400.f;
			P.ElevationMax        = 1400.f;
			P.ElevationBias       = 0.f;
			P.GridSize            = 20;
			P.BaselineZ           = 0.f;
			P.SurfaceThickness    = 120.f;
			P.WallBottomPadding   = 80.f;
			P.ElevationStep       = 700.f;
			P.RampDepthMin        = 1500.f;
			P.RampDepthMax        = 5000.f;
			P.RampWidthMinAlpha   = 0.12f;
			P.RampWidthMaxAlpha   = 0.40f;
			P.RegionSeedCountMin  = 5;
			P.RegionSeedCountMax  = 8;
			P.MaxAdjacentSteps    = 2;
			P.RampRunPerRise      = 2.8f;
			break;

		case ET66MapTheme::Ocean:
			P.ElevationMin        = -1400.f;
			P.ElevationMax        = 1400.f;
			P.ElevationBias       = -0.3f;
			P.GridSize            = 20;
			P.BaselineZ           = 0.f;
			P.SurfaceThickness    = 140.f;
			P.WallBottomPadding   = 120.f;
			P.ElevationStep       = 700.f;
			P.RampDepthMin        = 1500.f;
			P.RampDepthMax        = 5000.f;
			P.RampWidthMinAlpha   = 0.12f;
			P.RampWidthMaxAlpha   = 0.40f;
			P.RegionSeedCountMin  = 5;
			P.RegionSeedCountMax  = 8;
			P.MaxAdjacentSteps    = 2;
			P.RampRunPerRise      = 2.5f;
			break;

		case ET66MapTheme::Mountain:
			P.ElevationMin        = -1400.f;
			P.ElevationMax        = 1400.f;
			P.ElevationBias       = 0.2f;
			P.GridSize            = 20;
			P.BaselineZ           = 0.f;
			P.SurfaceThickness    = 140.f;
			P.WallBottomPadding   = 120.f;
			P.ElevationStep       = 700.f;
			P.RampDepthMin        = 1500.f;
			P.RampDepthMax        = 5000.f;
			P.RampWidthMinAlpha   = 0.12f;
			P.RampWidthMaxAlpha   = 0.40f;
			P.RegionSeedCountMin  = 5;
			P.RegionSeedCountMax  = 8;
			P.MaxAdjacentSteps    = 2;
			P.RampRunPerRise      = 2.4f;
			break;
		}
		return P;
	}
};
