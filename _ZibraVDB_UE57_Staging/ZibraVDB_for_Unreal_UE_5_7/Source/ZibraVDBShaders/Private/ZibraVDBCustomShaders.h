// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "DataDrivenShaderPlatformInfo.h"
#include "GlobalDistanceFieldParameters.h"
#include "MeshMaterialShader.h"
#include "SceneView.h"
#include "VirtualShadowMaps/VirtualShadowMapArray.h"

#define ZIB_MAX_LIGHT_COUNT 16

class ZIBRAVDBSHADERS_API FVolumeDownscale : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FVolumeDownscale)
	SHADER_USE_PARAMETER_STRUCT(FVolumeDownscale, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, ZIBRAVDBSHADERS_API)

	SHADER_PARAMETER(FIntVector4, BlockGridSize)
	SHADER_PARAMETER(uint, DownscaleFactor)
	SHADER_PARAMETER(uint, DownscaledBlockSize)
	SHADER_PARAMETER(int, DensityChannelMask)

	SHADER_PARAMETER_SRV(Buffer<float>, Input)
	SHADER_PARAMETER_SRV(Texture3D<uint>, BlockIndexTexture)
	SHADER_PARAMETER_SRV(ByteAddressBuffer, BlockBuffer)
	SHADER_PARAMETER_UAV(RWTexture3D<float>, Output)

	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6);
	}
	static void ModifyCompilationEnvironment(
		const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("ENGINE_MAJOR_VERSION"), ENGINE_MAJOR_VERSION);
		OutEnvironment.SetDefine(TEXT("ENGINE_MINOR_VERSION"), ENGINE_MINOR_VERSION);
	}
};
class ZIBRAVDBSHADERS_API FVolumeShadowRay : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FVolumeShadowRay)
	SHADER_USE_PARAMETER_STRUCT(FVolumeShadowRay, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, ZIBRAVDBSHADERS_API)
	SHADER_PARAMETER_SRV(Texture3D<float>, Density)
	SHADER_PARAMETER_SAMPLER(SamplerState, TextureSampler)
	SHADER_PARAMETER_SAMPLER(SamplerState, VSMPointSampler)

	SHADER_PARAMETER(int, LightCount)
	SHADER_PARAMETER_ARRAY(FVector4f, Light_Position_AttenuationRadius, [ZIB_MAX_LIGHT_COUNT])
	SHADER_PARAMETER_ARRAY(FVector4f, Light_PositionWS_EnableReceivedShadow, [ZIB_MAX_LIGHT_COUNT])
	SHADER_PARAMETER_ARRAY(FVector4f, Light_Color_EnableRaymarch, [ZIB_MAX_LIGHT_COUNT])
	SHADER_PARAMETER_ARRAY(FVector4f, Light_Direction_RadiusLimit, [ZIB_MAX_LIGHT_COUNT])
	SHADER_PARAMETER_ARRAY(FVector4f, Light_RectData, [ZIB_MAX_LIGHT_COUNT])
	SHADER_PARAMETER_ARRAY(FVector4f, Light_RectTangent, [ZIB_MAX_LIGHT_COUNT])
	SHADER_PARAMETER_SCALAR_ARRAY(float, Light_SourceRadius, [ZIB_MAX_LIGHT_COUNT])
	SHADER_PARAMETER_SCALAR_ARRAY(int, Light_VSM_IDS, [ZIB_MAX_LIGHT_COUNT])
	
	SHADER_PARAMETER(FVector3f, GridSize)

	SHADER_PARAMETER(float, DensityScale)
	SHADER_PARAMETER(float, StepSize)
	SHADER_PARAMETER(int, MaxStepCount)
	SHADER_PARAMETER(float, ShadowIntensity)
	SHADER_PARAMETER(uint32, SmoothReceivedShadows)
	SHADER_PARAMETER(FVector3f, AbsorptionColor)
	SHADER_PARAMETER(FVector3f, CameraPositionWS)
	SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
	SHADER_PARAMETER(FMatrix44f, BoxToWorld)
	SHADER_PARAMETER(FMatrix44f, WorldToBox)

	SHADER_PARAMETER(int, AmbientLightingMode)
	SHADER_PARAMETER(float, AmbientLightIntensity)
	SHADER_PARAMETER(FVector3f, AmbientLightingColor)
	SHADER_PARAMETER(float, AOShadowIntensity)
	SHADER_PARAMETER(float, AORadius)
	SHADER_PARAMETER(int, AORayMarcherStepCount)

	SHADER_PARAMETER_UAV(RWTexture3D<float4>, IlluminationOUT)

	SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FVirtualShadowMapUniformParameters, VirtualShadowMap)

	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6);
	}

	static void ModifyCompilationEnvironment(
		const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("IS_MATERIAL_SHADER"), 0);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE"), ThreadGroupSize);
		OutEnvironment.SetDefine(TEXT("ENGINE_MAJOR_VERSION"), ENGINE_MAJOR_VERSION);
		OutEnvironment.SetDefine(TEXT("ENGINE_MINOR_VERSION"), ENGINE_MINOR_VERSION);

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION < 5
		OutEnvironment.SetDefine(TEXT("VSM_PAGE_SIZE"), FVirtualShadowMap::PageSize);
		OutEnvironment.SetDefine(TEXT("VSM_PAGE_SIZE_MASK"), FVirtualShadowMap::PageSizeMask);
		OutEnvironment.SetDefine(TEXT("VSM_LOG2_PAGE_SIZE"), FVirtualShadowMap::Log2PageSize);
		OutEnvironment.SetDefine(TEXT("VSM_LEVEL0_DIM_PAGES_XY"), FVirtualShadowMap::Level0DimPagesXY);
		OutEnvironment.SetDefine(TEXT("VSM_LOG2_LEVEL0_DIM_PAGES_XY"), FVirtualShadowMap::Log2Level0DimPagesXY);
		OutEnvironment.SetDefine(TEXT("VSM_MAX_MIP_LEVELS"), FVirtualShadowMap::MaxMipLevels);
		OutEnvironment.SetDefine(TEXT("VSM_VIRTUAL_MAX_RESOLUTION_XY"), FVirtualShadowMap::VirtualMaxResolutionXY);
		OutEnvironment.SetDefine(TEXT("VSM_RASTER_WINDOW_PAGES"), FVirtualShadowMap::RasterWindowPages);
		OutEnvironment.SetDefine(TEXT("VSM_PAGE_TABLE_SIZE"), FVirtualShadowMap::PageTableSize);
		OutEnvironment.SetDefine(TEXT("INDEX_NONE"), INDEX_NONE);
#endif
	}

	static const int ThreadGroupSize = 8;
};

class ZIBRAVDBSHADERS_API FMakeBlockList : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FMakeBlockList)
	SHADER_USE_PARAMETER_STRUCT(FMakeBlockList, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, ZIBRAVDBSHADERS_API)

	SHADER_PARAMETER(int, SpatialBlockCount)
	SHADER_PARAMETER(FIntVector3, BlockGridSize)

	SHADER_PARAMETER(int, DensityChannelMask)
	SHADER_PARAMETER(int, TemperatureChannelMask)
	SHADER_PARAMETER(int, FlamesChannelMask)

	SHADER_PARAMETER_SRV(ByteAddressBuffer, BlockInfo)
	SHADER_PARAMETER_UAV(RWByteAddressBuffer, BlockBuffer)
	SHADER_PARAMETER_UAV(RWTexture3D<uint>, BlockIndexTexture)

	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6);
	}
	static void ModifyCompilationEnvironment(
		const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("ENGINE_MAJOR_VERSION"), ENGINE_MAJOR_VERSION);
		OutEnvironment.SetDefine(TEXT("ENGINE_MINOR_VERSION"), ENGINE_MINOR_VERSION);
	}
};

class ZIBRAVDBSHADERS_API FCalculateMaxDensityPerRenderBlock : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCalculateMaxDensityPerRenderBlock)
	SHADER_USE_PARAMETER_STRUCT(FCalculateMaxDensityPerRenderBlock, FGlobalShader)

	class FEnableInterpolation : SHADER_PERMUTATION_BOOL("ENABLE_INTERPOLATION");
	using FPermutationDomain = TShaderPermutationDomain<FEnableInterpolation>;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, ZIBRAVDBSHADERS_API)

	SHADER_PARAMETER(FIntVector4, BlockGridSize)

	SHADER_PARAMETER(uint, SparseBlockGridSize)
	SHADER_PARAMETER(uint, SparseBlockGridSizeMultiply)
	SHADER_PARAMETER(uint, SparseBlockGridSizeShift1)
	SHADER_PARAMETER(uint, SparseBlockGridSizeShift2)

	SHADER_PARAMETER(uint, SpatialBlockCount)
	SHADER_PARAMETER(uint, BlockOffset)

	SHADER_PARAMETER(float, DensityChannelScale)
	SHADER_PARAMETER(float, DensityDenormalizationScale)

	SHADER_PARAMETER_SRV(ByteAddressBuffer, BlockBuffer)
	SHADER_PARAMETER_SRV(Texture3D<uint>, BlockIndexTexture)
	SHADER_PARAMETER_SRV(ByteAddressBuffer, DecompressionPerChannelBlockInfo)
	SHADER_PARAMETER_SRV(Buffer<float>, DecompressionPerChannelBlockData)

	SHADER_PARAMETER_UAV(RWTexture3D<float>, MaxDensityPerBlockTexture)

	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6);
	}
};

class ZIBRAVDBSHADERS_API FComputeRenderBlocks : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FComputeRenderBlocks)
	SHADER_USE_PARAMETER_STRUCT(FComputeRenderBlocks, FGlobalShader)

	class FDensityOnly : SHADER_PERMUTATION_BOOL("DENSITY_ONLY");
	class FEnableInterpolation : SHADER_PERMUTATION_BOOL("ENABLE_INTERPOLATION");
	class FEnableDownscale : SHADER_PERMUTATION_BOOL("ENABLE_DOWNSCALE");
	class FHVEnabled : SHADER_PERMUTATION_BOOL("HV_ENABLED");
	using FPermutationDomain = TShaderPermutationDomain<FDensityOnly, FEnableInterpolation, FEnableDownscale, FHVEnabled>;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, ZIBRAVDBSHADERS_API)

	SHADER_PARAMETER(FIntVector4, BlockGridSize)
	SHADER_PARAMETER(FIntVector4, TextureSize)

	SHADER_PARAMETER(uint, SparseBlockGridSize)
	SHADER_PARAMETER(uint, SparseBlockGridSizeMultiply)
	SHADER_PARAMETER(uint, SparseBlockGridSizeShift1)
	SHADER_PARAMETER(uint, SparseBlockGridSizeShift2)

	SHADER_PARAMETER(float, rb_flameScale)
	SHADER_PARAMETER(float, rb_temperatureScale)
	SHADER_PARAMETER(int, rb_offset)
	SHADER_PARAMETER(uint, SpatialBlockCount)

	SHADER_PARAMETER(uint, DownscaleFactor)
	SHADER_PARAMETER(uint, DownscaledBlockSize)

	SHADER_PARAMETER(float, rb_densityScale)
	SHADER_PARAMETER(float, DensityChannelScale)

	SHADER_PARAMETER(FVector3f, rb_maskSpherePos)
	SHADER_PARAMETER(float, rb_maskSphereRadius)
	SHADER_PARAMETER(float, rb_maskSphereFalloff)
	SHADER_PARAMETER(FMatrix44f, rb_frameTransform)
	SHADER_PARAMETER(int, rb_maskEnabled)

	SHADER_PARAMETER_SRV(Buffer<float>, Input)
	SHADER_PARAMETER_SRV(ByteAddressBuffer, BlockBuffer)
	SHADER_PARAMETER_SRV(Texture3D<uint>, BlockIndexTexture)

	SHADER_PARAMETER_TEXTURE(Texture3D<float4>, Illumination)
	SHADER_PARAMETER_SAMPLER(SamplerState, samplerIllumination)

	SHADER_PARAMETER_UAV(RWTexture3D<float3>, SparseBlocks)
	SHADER_PARAMETER_UAV(RWTexture3D<float>, SparseBlocksDensity)
	SHADER_PARAMETER_UAV(RWTexture3D<float>, MaxDensityPerBlockTexture)
	SHADER_PARAMETER_TEXTURE(Texture2D<float4>, ScatteringColorsTexture)
	SHADER_PARAMETER_TEXTURE(Texture2D<float4>, FlameColorsTexture)
	SHADER_PARAMETER_TEXTURE(Texture2D<float4>, TemperatureColorsTexture)
	SHADER_PARAMETER_SAMPLER(SamplerState, ColorsTextureSampler)

	SHADER_PARAMETER(float, bEnableEmissionMasking)
	SHADER_PARAMETER(float, bEnableFlameEmissionMasking)
	SHADER_PARAMETER(float, bEnableTemperatureEmissionMasking)
	SHADER_PARAMETER(float, EmissionMaskCenter)
	SHADER_PARAMETER(float, EmissionMaskWidth)
	SHADER_PARAMETER(float, EmissionMaskIntensity)
	SHADER_PARAMETER(float, EmissionMaskRamp)

	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6);
	}
	static void ModifyCompilationEnvironment(
		const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("ENGINE_MAJOR_VERSION"), ENGINE_MAJOR_VERSION);
		OutEnvironment.SetDefine(TEXT("ENGINE_MINOR_VERSION"), ENGINE_MINOR_VERSION);
	}
};

class ZIBRAVDBSHADERS_API FSetPaddingVoxel : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSetPaddingVoxel)
	SHADER_USE_PARAMETER_STRUCT(FSetPaddingVoxel, FGlobalShader)

	class FDensityOnly : SHADER_PERMUTATION_BOOL("DENSITY_ONLY");
	class FEnableInterpolation : SHADER_PERMUTATION_BOOL("ENABLE_INTERPOLATION");
	class FEnableDownscale : SHADER_PERMUTATION_BOOL("ENABLE_DOWNSCALE");
	class FHVEnabled : SHADER_PERMUTATION_BOOL("HV_ENABLED");

	using FPermutationDomain = TShaderPermutationDomain<FDensityOnly, FEnableInterpolation, FEnableDownscale, FHVEnabled>;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, ZIBRAVDBSHADERS_API)

	SHADER_PARAMETER(FIntVector4, BlockGridSize)
	SHADER_PARAMETER(FIntVector4, TextureSize)

	SHADER_PARAMETER(uint, SparseBlockGridSize)
	SHADER_PARAMETER(uint, SparseBlockGridSizeMultiply)
	SHADER_PARAMETER(uint, SparseBlockGridSizeShift1)
	SHADER_PARAMETER(uint, SparseBlockGridSizeShift2)

	SHADER_PARAMETER(float, rb_flameScale)
	SHADER_PARAMETER(float, rb_temperatureScale)
	SHADER_PARAMETER(int, rb_offset)
	SHADER_PARAMETER(uint, SpatialBlockCount)

	SHADER_PARAMETER(uint, DownscaleFactor)
	SHADER_PARAMETER(uint, DownscaledBlockSize)

	SHADER_PARAMETER(float, rb_densityScale)
	SHADER_PARAMETER(float, DensityChannelScale)
	SHADER_PARAMETER(float, DensityDenormalizationScale)

	SHADER_PARAMETER_SRV(Buffer<float>, Input)
	SHADER_PARAMETER_SRV(StructuredBuffer<int>, BlockBuffer)
	SHADER_PARAMETER_SRV(Texture3D<uint>, BlockIndexTexture)

	SHADER_PARAMETER_UAV(RWTexture3D<float3>, SparseBlocks)
	SHADER_PARAMETER_UAV(RWTexture3D<float>, SparseBlocksDensity)
	SHADER_PARAMETER_UAV(RWTexture3D<float>, MaxDensityPerBlockTexture)

	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6);
	}
	static void ModifyCompilationEnvironment(
		const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("ENGINE_MAJOR_VERSION"), ENGINE_MAJOR_VERSION);
		OutEnvironment.SetDefine(TEXT("ENGINE_MINOR_VERSION"), ENGINE_MINOR_VERSION);
		//OutEnvironment.SetDefine(TEXT("ENABLE_INTERPOLATION"), true);
	}
};

class ZIBRAVDBSHADERS_API FComposeReflectionsVolume : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FComposeReflectionsVolume)
	SHADER_USE_PARAMETER_STRUCT(FComposeReflectionsVolume, FGlobalShader)

	class FEnableInterpolation : SHADER_PERMUTATION_BOOL("ENABLE_INTERPOLATION");
	class FEnableDownscale : SHADER_PERMUTATION_BOOL("ENABLE_DOWNSCALE");
	using FPermutationDomain = TShaderPermutationDomain<FEnableInterpolation, FEnableDownscale>;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, ZIBRAVDBSHADERS_API)

	SHADER_PARAMETER_SRV(Texture3D<float>, MaxDensityPerBlockTexture)
	SHADER_PARAMETER_SRV(Texture3D<uint>, BlockIndexTexture)

	SHADER_PARAMETER(uint, SparseBlockGridSize)
	SHADER_PARAMETER(uint, SparseBlockGridSizeMultiply)
	SHADER_PARAMETER(uint, SparseBlockGridSizeShift1)
	SHADER_PARAMETER(uint, SparseBlockGridSizeShift2)

	SHADER_PARAMETER(FIntVector3, rb_IlluminationSize)
	SHADER_PARAMETER(int, rb_IlluminationDownscaleFactor)

	SHADER_PARAMETER_SRV(Texture3D<float3>, SparseBlocks)
	SHADER_PARAMETER_SRV(Texture3D<float>, SparseBlocksDensity)
	SHADER_PARAMETER_UAV(RWTexture3D<float4>, IlluminatedVolume)

	SHADER_PARAMETER(uint, DownscaleFactor)
	SHADER_PARAMETER(uint, DownscaledBlockSize)

	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6);
	}
	static void ModifyCompilationEnvironment(
		const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("ENGINE_MAJOR_VERSION"), ENGINE_MAJOR_VERSION);
		OutEnvironment.SetDefine(TEXT("ENGINE_MINOR_VERSION"), ENGINE_MINOR_VERSION);
	}
};

class ZIBRAVDBSHADERS_API FClearTex3DFloat4 : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FClearTex3DFloat4)
	SHADER_USE_PARAMETER_STRUCT(FClearTex3DFloat4, FGlobalShader)
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, ZIBRAVDBSHADERS_API)
	SHADER_PARAMETER_UAV(RWTexture3D<FVector4>, Tex3DFloat4UAV)
	SHADER_PARAMETER(FVector4f, Tex3DFloat4ClearValue)
	END_SHADER_PARAMETER_STRUCT()
};

class ZIBRAVDBSHADERS_API FClearTex3DUint4 : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FClearTex3DUint4)
	SHADER_USE_PARAMETER_STRUCT(FClearTex3DUint4, FGlobalShader)
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, ZIBRAVDBSHADERS_API)
	SHADER_PARAMETER_UAV(RWTexture3D<FUintVector4>, Tex3DUint4UAV)
	SHADER_PARAMETER(FUintVector4, Tex3DUint4ClearValue)
	END_SHADER_PARAMETER_STRUCT()

public:
	static void ModifyCompilationEnvironment(
		const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("ENGINE_MAJOR_VERSION"), ENGINE_MAJOR_VERSION);
		OutEnvironment.SetDefine(TEXT("ENGINE_MINOR_VERSION"), ENGINE_MINOR_VERSION);
	}
};

class ZIBRAVDBSHADERS_API FClearTex3DFloat : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FClearTex3DFloat)
	SHADER_USE_PARAMETER_STRUCT(FClearTex3DFloat, FGlobalShader)
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, ZIBRAVDBSHADERS_API)
	SHADER_PARAMETER_UAV(RWTexture3D<float>, Tex3DFloatUAV)
	SHADER_PARAMETER(float, Tex3DFloatClearValue)
	END_SHADER_PARAMETER_STRUCT()

public:
	static void ModifyCompilationEnvironment(
		const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("ENGINE_MAJOR_VERSION"), ENGINE_MAJOR_VERSION);
		OutEnvironment.SetDefine(TEXT("ENGINE_MINOR_VERSION"), ENGINE_MINOR_VERSION);
	}
};

class ZIBRAVDBSHADERS_API FClearTex3DUint : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FClearTex3DUint)
	SHADER_USE_PARAMETER_STRUCT(FClearTex3DUint, FGlobalShader)
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, ZIBRAVDBSHADERS_API)
	SHADER_PARAMETER_UAV(RWTexture3D<uint>, Tex3DUintUAV)
	SHADER_PARAMETER(uint, Tex3DUintClearValue)
	END_SHADER_PARAMETER_STRUCT()

public:
	static void ModifyCompilationEnvironment(
		const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("ENGINE_MAJOR_VERSION"), ENGINE_MAJOR_VERSION);
		OutEnvironment.SetDefine(TEXT("ENGINE_MINOR_VERSION"), ENGINE_MINOR_VERSION);
	}
};

class ZIBRAVDBSHADERS_API FClearEmissionAndDensity : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FClearEmissionAndDensity)
	SHADER_USE_PARAMETER_STRUCT(FClearEmissionAndDensity, FGlobalShader)
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, ZIBRAVDBSHADERS_API)
	SHADER_PARAMETER_UAV(RWTexture3D<FVector3f>, SparseEmission)
	SHADER_PARAMETER_UAV(RWTexture3D<float>, SparseDensity)
	END_SHADER_PARAMETER_STRUCT()
};
