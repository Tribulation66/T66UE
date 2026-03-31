// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Components/HeterogeneousVolumeComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Curves/CurveEvaluation.h"
#include "Curves/CurveLinearColor.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "Render/ZibraVDBMaterialSceneProxy.h"
#include "ZibraVDBCommon.h"
#include "ZibraVDBDecompressorManager.h"
#include "ZibraVDBRHI/Public/ZibraUnrealRHITypes.h"
#include "ZibraVDBRenderingResourcesSubsystem.h"
#include "ZibraVDBSDKIntegration.h"

#include "ZibraVDBMaterialComponent.generated.h"

#define GRADIENT_TEXTURE_SIZE 64

class UZibraVDBAssetComponent;
class UTextureRenderTarget2D;
class UMaterialInterface;

UENUM(BlueprintType)
enum class ColorMode : uint8
{
	SolidColor UMETA(DisplayName = "Solid Color"),
	Gradient UMETA(DisplayName = "Gradient")
};

UENUM(BlueprintType)
enum class ERayMarchingFiltering : uint8
{
	None,
	Trilinear
};

UENUM(BlueprintType)
enum class EAmbientLightingMode : uint8
{
	Disabled = 0,
	SkyAtmosphere = 1,
	Constant = 2
};

/** First person primitive type - mirrors EFirstPersonPrimitiveType from UE 5.6+ */
UENUM(BlueprintType)
enum class EZibraFirstPersonPrimitiveType : uint8
{
	None = 0,
	FirstPerson = 1
};

UCLASS(Blueprintable, HideCategories = (Activation, Collision, Cooking, Tags, AssetUserData, Lighting, LOD, HLOD, Mobile,
						  RayTracing, PathTracing, Physics, TextureStreaming, Navigation, Rendering, Materials))
class UZibraVDBMaterialComponent final : public UPrimitiveComponent
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(Transient, TextExportTransient)
	TObjectPtr<UStaticMeshComponent> BBoxMeshComponent;

	UPROPERTY(Transient, TextExportTransient)
	TObjectPtr<UHeterogeneousVolumeComponent> VolumeComponent;

	bool bShowWarningDifferentVoxelSize = false;

	UPROPERTY(EditAnywhere, Category = Volume)
	bool bDifferentVoxelSizeWarning;	// Placeholder

	UPROPERTY(EditAnywhere, Category = Volume, meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	TObjectPtr<UMaterialInterface> Material;

	UPROPERTY(EditAnywhere, Category = Volume, meta = (EditCondition = "bUseHeterogeneousVolume", EditConditionHides))
	TObjectPtr<UMaterialInterface> VolumeMaterial;

	/// ----- General ----- ///
	UPROPERTY(EditAnywhere, Category = Volume, Interp)
	bool bUseHeterogeneousVolume = false;

	/// ----- Density ----- ///

	/** A multiplier for density values */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Density", Interp, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DensityScale = 1.0;

	/** Determines how the ScatteringColor color is defined, whether as a color or a gradient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Density", Interp)
	ColorMode ScatteringColorMode = ColorMode::SolidColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Density", Interp,
		meta = (EditCondition = "ScatteringColorMode == ColorMode::Gradient", EditConditionHides))
	UCurveLinearColor* ScatteringColorGradient;

	/** The color of the light that can scatter inside the smoke */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Density", Interp,
		meta = (EditCondition = "ScatteringColorMode == ColorMode::SolidColor", EditConditionHides))
	FLinearColor ScatteringColor = FLinearColor(1.0, 1.0, 1.0);

	/** The color of the light that can pass through the smoke */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Density", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	FLinearColor AbsorptionColor = FLinearColor(1.0, 1.0, 1.0);

	/// ----- Flame ----- ///

	/** A multiplier for flame values */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Flame", Interp, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float FlameScale = 1.0;

	/** Determines how the flame color is defined, whether as a color or a gradient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Flame", Interp)
	ColorMode FlameColorMode = ColorMode::SolidColor;

	/** The color of the flames */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Flame", Interp,
		meta = (EditCondition = "FlameColorMode == ColorMode::SolidColor", EditConditionHides))
	FLinearColor FlameColor = FLinearColor(0.99, 0.13, 0.011);

	/** Gradient mapped onto the flame in the 0-1 range. The gradient should be represented as a "Color Curve" asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Flame", Interp,
		meta = (EditCondition = "FlameColorMode == ColorMode::Gradient", EditConditionHides))
	UCurveLinearColor* FlameColorGradient;

	/// ----- Temperature ----- ///

	/** A multiplier for temperature values */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Temperature", Interp, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float TemperatureScale = 1.0;

	/** Determines how the temperature color is defined, whether as a color or a gradient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Temperature", Interp)
	ColorMode TemperatureColorMode = ColorMode::SolidColor;

	/** Gradient mapped onto the temperature in the minTemperature-maxTemperature range. The gradient should be represented as a
	 * "Color Curve" asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Temperature", Interp,
		meta = (EditCondition = "TemperatureColorMode == ColorMode::Gradient", EditConditionHides))
	UCurveLinearColor* TemperatureColorGradient;

	/** The color of the temperature */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Temperature", Interp,
		meta = (EditCondition = "TemperatureColorMode == ColorMode::SolidColor", EditConditionHides))
	FLinearColor TemperatureColor = FLinearColor(0.99, 0.13, 0.011);

	/// ----- Illumination ----- ///

	static constexpr float DirectionalLightBrightnessScaleFactor = 1.0;
	static constexpr float PointLightBrightnessScaleFactor = 16.0;
	static constexpr float SpotLightBrightnessScaleFactor = 16.0;
	static constexpr float RectLightBrightnessScaleFactor = 16.0;

	/** The intensity of the volume illumination by the directional light */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Illumination", Interp,
		meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	float DirectionalLightBrightness = 1.0;

	/** The intensity of the volume illumination by point lights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Illumination", Interp,
		meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	float PointLightBrightness = 1.0;

	/** The intensity of the volume illumination by spot lights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Illumination", Interp,
		meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	float SpotLightBrightness = 1.0;

	/** The intensity of the volume illumination by rect lights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Illumination", Interp,
		meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	float RectLightBrightness = 1.0;

	UPROPERTY(meta = (HideInEditor))
	float IlluminatedVolumeResolutionScale = 0.25;
	/** Quality of the illumination shadows */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Illumination",
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	ResolutionDownscaleFactors IlluminationResolution = ResolutionDownscaleFactors::Quarter;

	/** Calculate lighting only on initialization or frame change */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Illumination", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool bUseStaticIllumination = false;

	/** Used for the volume rendering process. Smaller values correspond to more accurate rendering but higher performance costs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Illumination", Interp,
		meta = (ClampMin = "0.1", UIMin = "0.1", DisplayName = "Main Step Size", EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	float RayMarchingMainStepSize = 2.0;

	/** Specifies the maximum number of steps taken during the main volume rendering process */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Illumination",
		meta = (ClampMin = "32", UIMin = "32", ClampMax = "1024", UIMax = "1024", DisplayName = "Max Main Steps",
			EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	int32 RayMarchingMaxMainSteps = 256;

	/**
	 * Channels that this component should be in.  Lights with matching channels will affect the component.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Volume|Illumination", meta = (DisplayName = "Lighting Channels"), Interp)
	FLightingChannels VDBLightingChannels;

	/// ----- Illumination|Ambient ----- ///

	/** Placeholder */
	UPROPERTY(EditAnywhere, Category = "Volume|Illumination|Ambient",
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool AmbientLightingWarning;

	/** Determines whether ambient color is taken from the Sky Atmosphere, specified manually, or disabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Illumination|Ambient",
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	EAmbientLightingMode AmbientLightingMode = EAmbientLightingMode::SkyAtmosphere;

#if WITH_EDITORONLY_DATA
	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Replaced with AmbientLightingMode."))
	bool EnableAmbientLighting_DEPRECATED = true;
#endif

	/** Static color for ambient lighting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Illumination|Ambient", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume && AmbientLightingMode == EAmbientLightingMode::Constant",
			EditConditionHides))
	FLinearColor AmbientLightingColor = FLinearColor(0.0757, 0.1203, 0.2000);
	
	/** Strength of the ambient lighting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Illumination|Ambient", Interp,
		meta = (ClampMin = "0", UIMin = "0",
			EditCondition = "!bUseHeterogeneousVolume && AmbientLightingMode != EAmbientLightingMode::Disabled",
			EditConditionHides))
	float AmbientLightIntensity = 1.0;

	/** Strength of the ambient occlusion shadowing. It can be set to a negative value to brighten occluded areas */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Illumination|Ambient", Interp,
		meta = (DisplayName = "AO Intensity", EditCondition = "!bUseHeterogeneousVolume",
			EditConditionHides))
	float AOShadowIntensity = 1.0;

	/** Ambient occlusion sampling radius in normalized volume space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Illumination|Ambient", Interp,
		meta = (ClampMin = "0.001", UIMin = "0.001", ClampMax = "0.2", UIMax = "0.2", DisplayName = "AO Radius", EditCondition = "!bUseHeterogeneousVolume",
			EditConditionHides))
	float AORadius = 0.04f;

	/** Number of ambient occlusion steps in one direction. Sampling happens in 6 different directions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Illumination|Ambient", Interp,
		meta = (ClampMin = "1", UIMin = "1", ClampMax = "16", UIMax = "16", DisplayName = "AO Steps Count",
			EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	int32 AORayMarcherStepCount = 2;

	/// ----- Shadows ----- ///

	/** Strength of the shadowing effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows", Interp,
		meta = (ClampMin = "0.0005", UIMin = "0.00005", EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	float ShadowIntensity = 1.2;

	/** A toggle to enable or disable volume self-shadowing from directional lights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool EnableDirectionalLightShadows = true;

	/** A toggle to enable or disable volume self-shadowing from point lights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool EnablePointLightShadows = true;

	/** A toggle to enable or disable volume self-shadowing from spot lights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool EnableSpotLightShadows = true;

	/** A toggle to enable or disable volume self-shadowing from rect lights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool EnableRectLightShadows = true;

	/** Determines the step size used specifically for shadow rendering within the smoke or volume */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows", Interp,
		meta = (ClampMin = "0.1", UIMin = "0.1", DisplayName = "Self Shadow Step Size", EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	float RayMarchingSelfShadowStepSize = 1.5;

	/** Specifies the maximum number of steps taken during the shadow rendering process */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows",
		meta = (ClampMin = "8", UIMin = "8", ClampMax = "1024", UIMax = "1024", DisplayName = "Max Self Shadow Steps",
			EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	int32 RayMarchingMaxSelfShadowSteps = 128;

	UPROPERTY(EditAnywhere, Category = "Volume|Shadows|Received Shadows",
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool ReceivedShadowsWarning;	// Placeholder

	/// ----- Shadows|Received Shadows ----- ///

	/** A toggle to enable or disable received shadows from directional lights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows|Received Shadows", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool EnableDirectionalLightReceivedShadows = true;

	/** A toggle to enable or disable received shadows from point lights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows|Received Shadows", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool EnablePointLightReceivedShadows = true;

	/** A toggle to enable or disable received shadows from spot lights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows|Received Shadows", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool EnableSpotLightReceivedShadows = true;

	/** A toggle to enable or disable received shadows from rect lights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows|Received Shadows", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool EnableRectLightReceivedShadows = true;

	/** Enable smooth received shadows (samples at neighboring voxel centers and averages) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows|Received Shadows", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool SmoothReceivedShadows = true;

	/// ----- Shadows|Projected Shadows ----- ///

	UPROPERTY(EditAnywhere, Category = "Volume|Shadows|Projected Shadows",
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool ProjectedShadowsWarning;	 // Placeholder

	/** Strength of the projected shadow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows|Projected Shadows", Interp,
		meta = (ClampMin = "0.0005", UIMin = "0.00005", EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	float ProjectedShadowsIntensity = 1.0;

	/** A toggle to enable or disable projected shadows from directional lights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows|Projected Shadows", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool EnableDirectionalLightProjectedShadows = true;

	/** A toggle to enable or disable received shadows from point and spot lights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows|Projected Shadows", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool EnablePointAndSpotLightProjectedShadows = true;

	/** Specifies the maximum number of steps taken during the projected shadow rendering process */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Shadows|Projected Shadows",
		meta = (ClampMin = "8", UIMin = "8", ClampMax = "1024", UIMax = "1024", DisplayName = "Max Projected Shadow Steps",
			EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	int32 RayMarchingMaxProjectedShadowSteps = 96;

	/// ----- Decompression ----- ///

	/** Skip decompression if the ZibraVDB frame hasn't changed. Enabling this parameter can
	 * increase performance by skipping unneeded decompression step if frame hasn't changed but
	 * may also cause stuttering. For single frame effects such behavior is always enabled regardless of this option. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Decompression")
	bool AllowSkipDecompression = true;

	/// ----- Render ----- ////

	UPROPERTY()
	bool bRayMarchingFilteringInitialized = false;

	/** Select different ray marching interpolation options.
	 * Trilinear filtering offers better visual, but it lowers the performance and uses more VRAM.
	 * No filtering offers better performance and lower VRAM usage, should be used for effects with large voxel per pixel density.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render")
	ERayMarchingFiltering RayMarchingFiltering = ERayMarchingFiltering::Trilinear;

	/** Quality of the emmission textures. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render")
	ResolutionDownscaleFactors VolumeResolution = ResolutionDownscaleFactors::Original;

	/// ----- Render| Emission Masking ----- ///

	// TODO: Find out how to hide the entire rendering section if bUseHeterogeneousVolume is true. Now it behaves wrong, probably
	// due to because of the way the properties are grouped.
	/** Enables emission masking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Emission Masking",
		DisplayName = "Enable Emission Masking", Interp)
	bool bEnableEmissionMasking = false;

	/** Enables emission masking for flame channel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Emission Masking", DisplayName = "Enable Flame Masking", Interp)
	bool bEnableFlameMasking = true;

	/** Enables emission masking for temperature channel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Emission Masking",
		DisplayName = "Enable Temperature Masking", Interp)
	bool bEnableTemperatureMasking = true;

	/**  Defines the center reference density of the mask, density values close to this will be used as a mask. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Emission Masking",
		meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Interp)
	float MaskCenter = 0.1f;

	/** Defines the referense density width of the emission mask's effect. Higher values will give a softer mask. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Emission Masking",
		meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Interp)
	float MaskWidth = 0.1f;

	/** Defines the intensity of the mask. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Emission Masking",
		meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Interp)
	float MaskIntensity = 1.f;

	/** Adjusts the ramp of the emission mask curve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Emission Masking",
		meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Interp)
	float MaskRamp = 1.f;

	/// ----- Sphere Masking ----- ////

	/** Toggles sphere masking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Sphere Masking", Interp)
	bool bEnableSphereMasking = false;

	/** Radius of the masking sphere in world units */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Sphere Masking", Interp)
	float MaskingSphereRadius = 100.0f;

	/** Mask position in world space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Sphere Masking", Interp)
	FVector3f MaskingSpherePosition;

	/** Width of smoothstep gradient on masking sphere edge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Sphere Masking", Interp)
	float MaskingSphereFalloff;

	/// ----- Depth Fade ----- ////

	/** Toggles depth fade for smooth intersection with geometry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Depth Fade", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	bool bEnableDepthFade = false;

	/** The size of the fade transition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Depth Fade", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	float DepthFadeDistance = 0.0f;

	/** The opacity of the fade transition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Depth Fade", Interp,
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	float DepthFadeOpacity = 1.0f;

	/// ----- First Person Rendering ----- ////

	/** When set to FirstPerson, the camera's FirstPersonFieldOfView will be applied to this volume to avoid clipping with scene geometry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render",
		meta = (DisplayName = "First Person Primitive Type", EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	EZibraFirstPersonPrimitiveType VDBFirstPersonPrimitiveType = EZibraFirstPersonPrimitiveType::None;

	/// ----- Translucency Sorting ----- ////

	/** Translucent objects with a lower sort priority draw behind objects with a higher priority */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Translucency Sorting",
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	int32 VolumeTranslucencySortPriority;

	/** Modified sort distance offset for translucent objects in world units. A positive number will move
	 * the sort distance further and a negative number will move the distance closer
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Render|Translucency Sorting",
		meta = (EditCondition = "!bUseHeterogeneousVolume", EditConditionHides))
	float VolumeTranslucencySortDistanceOffset = 0.0f;

	/// ----- HV lighting params ----- ///

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Heterogeneous Volume",
		meta = (EditCondition = "bUseHeterogeneousVolume", EditConditionHides, DisplayName = "Step Factor", ClampMin = "0.0",
			UIMin = "0.0"))
	float HVStepFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Heterogeneous Volume",
		meta = (EditCondition = "bUseHeterogeneousVolume", EditConditionHides, DisplayName = "Shadow Step Factor", ClampMin = "0.0",
			UIMin = "0.0"))
	float HVShadowStepFactor = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Heterogeneous Volume",
		meta = (EditCondition = "bUseHeterogeneousVolume", EditConditionHides, DisplayName = "Shadow Bias Factor", ClampMin = "0.0",
			UIMin = "0.0"))
	float HVShadowBiasFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Heterogeneous Volume",
		meta = (EditCondition = "bUseHeterogeneousVolume", EditConditionHides, DisplayName = "Lighting Downsample Factor",
			ClampMin = "0.0", UIMin = "0.0"))
	float HVLightingDownsampleFactor = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Heterogeneous Volume",
		meta = (EditCondition = "bUseHeterogeneousVolume", EditConditionHides, DisplayName = "Cast Shadows"))
	bool HVCastShadows = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume|Heterogeneous Volume",
		meta = (EditCondition = "bUseHeterogeneousVolume", EditConditionHides, DisplayName = "Emissive Light Source"))
	bool HVEmissiveLightSource = false;

	/// ----- Culling ----- ////

	/** Whether to stop decompression when volume is not in camera frustum */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Volume|Render|Culling")
	bool bAllowFrustumCulling = true;

	/** Whether to always cull everything. Will affect reflections and projected shadows */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Volume|Render|Culling",
		meta = (EditCondition = "bAllowFrustumCulling", EditConditionHides))
	bool bFullFrustumCulling = false;

	/** Bounding box scale that is only used for culling */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Volume|Render|Culling",
		meta = (DisplayName = "Bounds Scale", ClampMin = "1.0", UIMin = "1.0", ClampMax = "10.0", UIMax = "10.0"))
	float ZibraVDBBoundsScale = 1.0f;

	/** Distance used for volume distance cullling. When culled, rendering and decompression is disabled.
	 * 0 means distance culling is off
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Volume|Render|Culling")
	float MaxDrawDistance = 0.0f;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set ZibraVdbAssetComponent"), Category = Volume)
	void SetZibraVDBAssetComponent(UZibraVDBAssetComponent* InZibraVDBAssetComponent);

	UFUNCTION(CallInEditor)
	void SetDensityScale(float NewDensityScale);

	UFUNCTION(CallInEditor)
	void SetAbsorptionColor(FLinearColor NewAbsorptionColor);

	//~ Begin UObject Interface.
	void PostInitProperties() final;
	void PostReinitProperties() final;
	void PostDuplicate(bool bDuplicateForPIE) final;
#if WITH_EDITORONLY_DATA
	void PostLoad() final;
#endif
	//~ End UObject Interface.

	//~ Begin UPrimitiveComponent Interface.
	bool SupportsStaticLighting() const final;
	int32 GetNumMaterials() const final;
	void SetMaterial(int32 ElementIndex, UMaterialInterface* Material) final;
	UMaterialInterface* GetMaterial(int32 Index) const final;
	void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const final;
	FPrimitiveSceneProxy* CreateSceneProxy() final;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) final;
	void OnRegister() final;
	void OnUnregister() final;
	void OnComponentDestroyed(bool bDestroyingHierarchy) final;
	//~ End UPrimitiveComponent Interface.

	void UpdateFrameIndex(const uint32 FrameIndex) noexcept;
	void UpdateResetDecompressedFrameIndex() noexcept;
	void UpdateSceneProxyRenderParams() noexcept;
	void UpdateRenderTexturesUAV() noexcept;
	void UpdateChannelTextures() noexcept;
	void OnVolumeChanged() noexcept;

	TSharedPtr<FZibraVDBRenderingResources>& GetRenderingResources() noexcept;
	const TSharedPtr<FZibraVDBRenderingResources>& GetRenderingResources() const noexcept;

	TSharedPtr<FZibraVDBNativeResource> GetNativeResources() noexcept;
	const TSharedPtr<FZibraVDBNativeResource> GetNativeResources() const noexcept;
	float GetDensityScaleInternal() const noexcept;
	float GetFlameScaleInternal() const noexcept;
	float GetTemperatureScaleInternal() const noexcept;
	float GetShadowIntensityInternal() const noexcept;

	FPrimitiveComponentId GetBBoxMeshPrimitiveComponentId() const noexcept;

	TObjectPtr<UTextureRenderTargetVolume> GetSparseBlocksRGB() const noexcept;
	TObjectPtr<UTextureRenderTargetVolume> GetSparseBlocksDensity() const noexcept;
	TObjectPtr<UTextureRenderTargetVolume> GetMaxDensityPerBlockTexture() const noexcept;
	TObjectPtr<UTextureRenderTargetVolume> GetBlockIndexTexture() const noexcept;
	TObjectPtr<UTextureRenderTargetVolume> GetTransformBuffer() const noexcept;
	TObjectPtr<UTextureRenderTargetVolume> GetIlluminatedVolumeTexture() const noexcept;
	void UpdateVisibility(bool bIsVisible);

	bool VisibleInReflections() const noexcept;

	FZibraTexture2DRHIRef GetScatteringColorTexture() const noexcept;
	FZibraTexture2DRHIRef GetTemperatureColorTexture() const noexcept;
	FZibraTexture2DRHIRef GetFlameColorTexture() const noexcept;

	const FZibraVDBDecompressorManager& GetDecompressorManager() const noexcept;

private:
	void UpdateRelativeLocation();
	void UpdateRayMarchMaterialParams();
	void UpdateBBoxMeshProperties();
	UMaterialInstanceDynamic* UpdateMaterialInstance(bool isDirty = false);
	UMaterialInstanceDynamic* UpdateHVMaterialInstance(bool isDirty = false);
	void UpdateHV();
	void UpdateTranslucentSort();
	void UpdateRenderingTextures();
	FString ConstructRenderingResourcesMapKey() const noexcept;

	bool IsVolumeValid();
	bool IsReadyForRender();
	void UpdateGlobalSet();

	void InitResources();
	void ReleaseResources();
	void ApplyHVParams();

	FString RenderingResourcesMapKey;
	bool bIsRegistered = false;
	bool bIsResourcesValid = false;

	bool bLastFrameHVStatus = false;

	UPROPERTY(Transient, DuplicateTransient, TextExportTransient)
	TObjectPtr<UTexture2D> ScatteringColorTexture;
	UPROPERTY(Transient, DuplicateTransient, TextExportTransient)
	TObjectPtr<UTexture2D> FlameColorTexture;
	UPROPERTY(Transient, DuplicateTransient, TextExportTransient)
	TObjectPtr<UTexture2D> TemperatureColorTexture;

	TSharedPtr<FZibraVDBRenderingResources> ZibraVDBRenderingResources;
	TObjectPtr<UZibraVDBAssetComponent> ZibraVDBAssetComponent;
	FZibraVDBDecompressorManager DecompressorManager;

	friend class UZibraVDBAssetComponent;
	friend class UZibraVDBRenderingResourcesSubsystem;
	friend class FZibraVDBRenderingResources;
	friend class AZibraVDBActor;
};
