// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66RetroFXSettings.generated.h"

/**
 * Persistent local settings for the Retro FX experimentation tab.
 * Values are stored as 0..100 strengths so the settings screen can stage
 * them directly without additional UI-side conversions.
 */
USTRUCT(BlueprintType)
struct T66_API FT66RetroFXSettings
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1BlendPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1DitheringPercent = 100.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1BayerDitheringPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1ColorLUTPercent = 100.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1ColorBoostPercent = 25.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1FogPercent = 100.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1FogDensityPercent = 35.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1FogStartDistancePercent = 50.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1FogFallOffDistancePercent = 50.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float PS1SceneDepthFogPercent = 100.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float FakeResolutionSwitchSizePercent = 100.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float FakeResolutionSwitchUVPercent = 100.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float TargetResolutionHeightPercent = 60.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float N64BlurBlendPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float N64BlurStepsPercent = 35.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float N64LowFakeResolutionPercent = 100.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	bool bUseUE5RFXN64BlurReplaceTonemapper = false;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float CRTBlendPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX")
	float T66PixelationPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float WorldVertexSnapPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float WorldVertexSnapResolutionPercent = 50.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float WorldVertexNoisePercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float WorldAffineBlendPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float WorldAffineDistance1Percent = 50.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float WorldAffineDistance2Percent = 50.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float WorldAffineDistance3Percent = 50.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float CharacterVertexSnapPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float CharacterVertexSnapResolutionPercent = 50.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float CharacterVertexNoisePercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float CharacterAffineBlendPercent = 0.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float CharacterAffineDistance1Percent = 50.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float CharacterAffineDistance2Percent = 50.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Retro FX|Geometry")
	float CharacterAffineDistance3Percent = 50.0f;
};
