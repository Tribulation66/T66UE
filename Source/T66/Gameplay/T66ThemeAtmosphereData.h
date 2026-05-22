// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureCube.h"
#include "Gameplay/T66TowerMapTerrain.h"
#include "T66ThemeAtmosphereData.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66ThemeCelAtmosphere
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LightDirection = FVector(-0.4f, 0.6f, -0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RampStep1 = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RampStep2 = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ShadeColor = FLinearColor(0.35f, 0.38f, 0.50f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor MidtoneColor = FLinearColor(0.7f, 0.7f, 0.72f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LitColor = FLinearColor(0.85f, 0.85f, 0.85f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor RimColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RimPower = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RimStrength = 0.21f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor OutlineColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OutlineWidth = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OutlineReferenceDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OutlineReferenceFOVTanHalf = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OutlineDepthOffsetScalar = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor EnvShadeColor = FLinearColor(0.3f, 0.32f, 0.42f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor EnvMidtoneColor = FLinearColor(0.55f, 0.58f, 0.62f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor EnvLitColor = FLinearColor(0.85f, 0.85f, 0.90f, 1.0f);
};

USTRUCT()
struct T66_API FT66ThemeAtmosphereSpec
{
	GENERATED_BODY()

	UPROPERTY()
	FLinearColor SkyLightColor = FLinearColor::White;

	UPROPERTY()
	float SkyLightIntensity = 0.5f;

	UPROPERTY()
	float FogDensity = 0.04f;

	UPROPERTY()
	float FogHeightFalloff = 0.2f;

	UPROPERTY()
	FLinearColor FogInscatteringColor = FLinearColor(0.12f, 0.18f, 0.32f, 1.0f);

	UPROPERTY()
	float FogStartDistance = 400.0f;

	UPROPERTY()
	float FogCutoffDistance = 20000.0f;

	// Ambient cubemap (primary ambient light source for indoor stylized scenes)
	UPROPERTY()
	TSoftObjectPtr<UTextureCube> AmbientCubemap;

	UPROPERTY()
	float AmbientCubemapIntensity = 10.0f;

	UPROPERTY()
	FLinearColor AmbientCubemapTint = FLinearColor(0.55f, 0.7f, 0.95f, 1.0f);

	UPROPERTY()
	FVector4 ColorGradeShadowsTint = FVector4(0.7f, 0.85f, 1.1f, 1.0f);

	UPROPERTY()
	FVector4 ColorGradeMidtonesTint = FVector4(0.95f, 0.97f, 1.02f, 1.0f);

	UPROPERTY()
	FVector4 ColorGradeHighlightsTint = FVector4(1.0f, 1.0f, 1.0f, 1.0f);

	UPROPERTY()
	FVector4 ColorGradeSaturation = FVector4(0.85f, 0.85f, 0.85f, 1.0f);

	UPROPERTY()
	FVector4 ColorGradeContrast = FVector4(1.1f, 1.1f, 1.1f, 1.0f);

	UPROPERTY()
	FVector4 ColorGradeGain = FVector4(0.9f, 0.9f, 0.9f, 1.0f);

	UPROPERTY()
	float TorchIntensity = 1400.0f;

	UPROPERTY()
	float TorchAttenuationRadius = 800.0f;

	UPROPERTY()
	FLinearColor TorchColor = FLinearColor(1.0f, 0.55f, 0.22f);

	UPROPERTY()
	float TorchFalloffExponent = 2.0f;

	// Torch placement (procedural)
	UPROPERTY()
	float TorchVerticalOffset = 450.0f;

	UPROPERTY()
	float TorchSpacingAlongWall = 1800.0f;

	UPROPERTY()
	float TorchMinSeparation = 1400.0f;

	UPROPERTY()
	int32 TorchMaxPerFloor = 60;

	UPROPERTY()
	float CarryLightIntensity = 550.0f;

	UPROPERTY()
	float CarryLightAttenuationRadius = 450.0f;

	UPROPERTY()
	FLinearColor CarryLightColor = FLinearColor(1.0f, 0.7f, 0.4f);

	UPROPERTY()
	float CarryLightFalloffExponent = 1.5f;

	UPROPERTY()
	float CarryLightVerticalOffset = 60.0f;

	UPROPERTY()
	FT66ThemeCelAtmosphere CelAtmosphere;
};

namespace T66ThemeAtmosphereData
{
	T66_API const FT66ThemeAtmosphereSpec& GetSpecForTheme(T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme);
}
