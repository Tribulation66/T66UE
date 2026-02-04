// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66ProceduralLandscapeParams.generated.h"

/** Preset for procedural landscape world size. */
UENUM(BlueprintType)
enum class ET66LandscapeSizePreset : uint8
{
	/** ~1 km x 1 km (63x63 quads per component, 8x8 components = 505x505 vertices) */
	Small UMETA(DisplayName = "Small (~1 km)"),
	/** ~2–4 km (e.g. 16x16 components) */
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
	float HillAmplitude = 8000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "10"))
	float VeryLargeScaleMeters = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "10"))
	float LargeScaleMeters = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "10"))
	float MediumScaleMeters = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "1", ClampMax = "10"))
	int32 SmoothPasses = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float SmoothStrength = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape")
	bool bCarveRiverValley = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (EditCondition = "bCarveRiverValley", ClampMin = "10"))
	float RiverWidthMeters = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape", meta = (EditCondition = "bCarveRiverValley", ClampMin = "0"))
	float RiverDepthUU = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Landscape")
	ET66LandscapeSizePreset SizePreset = ET66LandscapeSizePreset::Small;
};
