// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66RetroFXSettings.generated.h"

/**
 * Persistent local settings for the Retro FX experimentation tab.
 * Scalar values are stored as 0..100 strengths so the settings screen can
 * stage them directly. Binary options use dedicated bools where appropriate,
 * while legacy on/off scalar paths still remain 0..100 for compatibility.
 */
USTRUCT(BlueprintType)
struct T66_API FT66RetroFXSettings
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	bool bEnableRetroFXMaster = true;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1BlendPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1DitheringPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1BayerDitheringPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1ColorLUTPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1ColorBoostPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1FogPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1FogDensityPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1FogStartDistancePercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1FogFallOffDistancePercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1SceneDepthFogPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	bool bUseRealLowResolution = true;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float FakeResolutionSwitchSizePercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float FakeResolutionSwitchUVPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float TargetResolutionHeightPercent = 40.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float N64BlurBlendPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float N64BlurStepsPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float N64LowFakeResolutionPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	bool bUseUE5RFXN64BlurReplaceTonemapper = false;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float ChromaticAberrationPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float ChromaticDistortionPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	bool bInvertChromaticDistortion = false;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float T66PixelationPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|World")
	float WorldPixelationPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Characters")
	float CharacterPixelationPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Characters")
	bool bEnableCharacterOutline = false;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI")
	float UIChromeTreatmentPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI")
	float UITextTreatmentPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Chrome")
	float UIChromePixelationPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Chrome")
	float UIChromeDitheringPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Chrome")
	float UIChromeVertexSnapPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Chrome")
	float UIChromeVertexSnapResolutionPercent = 50.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Chrome")
	float UIChromeScanlinePercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Chrome")
	float UIChromeChromaticAberrationPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Text")
	float UITextPixelationPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Text")
	float UITextDitheringPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Text")
	float UITextVertexSnapPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Text")
	float UITextVertexSnapResolutionPercent = 50.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Text")
	float UITextScanlinePercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Text")
	float UITextChromaticAberrationPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Background Image")
	float UIBackgroundImageTreatmentPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Background Image")
	float UIBackgroundImagePixelationPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Background Image")
	float UIBackgroundImageDitheringPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Background Image")
	float UIBackgroundImageVertexSnapPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Background Image")
	float UIBackgroundImageVertexSnapResolutionPercent = 50.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Background Image")
	float UIBackgroundImageScanlinePercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|Background Image")
	float UIBackgroundImageChromaticAberrationPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|CRT")
	bool UIFullScreenCRTEnabled = true;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|CRT", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UICRTScanlineStrength = 0.4f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|CRT", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UICRTPhosphorMaskStrength = 0.3f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|CRT", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UICRTBloomStrength = 0.2f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|CRT", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UICRTChromaticAberrationStrength = 0.25f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|CRT", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UICRTBarrelDistortionStrength = 0.15f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|CRT", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UICRTVignetteStrength = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|CRT", meta = (ClampMin = "4", ClampMax = "8"))
	int32 UICRTColorQuantizationBits = 6;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|UI|CRT", meta = (ClampMin = "120", ClampMax = "4320"))
	int32 UICRTReferenceResolutionHeight = 1080;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	bool bEnableWorldGeometry = false;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float WorldVertexSnapPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float WorldVertexSnapResolutionPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float WorldVertexNoisePercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float WorldAffineBlendPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float WorldAffineDistance1Percent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float WorldAffineDistance2Percent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float WorldAffineDistance3Percent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	bool bEnableCharacterGeometry = false;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float CharacterVertexSnapPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float CharacterVertexSnapResolutionPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float CharacterVertexNoisePercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float CharacterAffineBlendPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float CharacterAffineDistance1Percent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float CharacterAffineDistance2Percent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float CharacterAffineDistance3Percent = 0.0f;
};
