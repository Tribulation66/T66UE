// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBCustomShaders.h"

IMPLEMENT_GLOBAL_SHADER(FVolumeDownscale, "/Plugin/ZibraVDB/ZibraVDBVolumeDownscale.usf", "MainCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FVolumeShadowRay, "/Plugin/ZibraVDB/ZibraVDBShadows.usf", "MainCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FMakeBlockList, "/Plugin/ZibraVDB/ZibraVDBMakeBlockList.usf", "MainCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(
	FCalculateMaxDensityPerRenderBlock, "/Plugin/ZibraVDB/ZibraVDBCalculateMaxDensityPerRenderBlock.usf", "MainCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FComputeRenderBlocks, "/Plugin/ZibraVDB/ZibraVDBComputeRenderBlocks.usf", "MainCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FSetPaddingVoxel, "/Plugin/ZibraVDB/ZibraVDBComputeRenderBlocks.usf", "SetPaddingVoxelCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FComposeReflectionsVolume, "/Plugin/ZibraVDB/ZibraVDBComposeReflectionsVolume.usf", "MainCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FClearTex3DFloat4, "/Plugin/ZibraVDB/ZibraVDBClearTexture.usf", "ClearTex3DFloat4", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FClearTex3DUint4, "/Plugin/ZibraVDB/ZibraVDBClearTexture.usf", "ClearTex3DUint4", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FClearTex3DFloat, "/Plugin/ZibraVDB/ZibraVDBClearTexture.usf", "ClearTex3DFloat", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FClearTex3DUint, "/Plugin/ZibraVDB/ZibraVDBClearTexture.usf", "ClearTex3DUint", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(
	FClearEmissionAndDensity, "/Plugin/ZibraVDB/ZibraVDBClearTexture.usf", "ClearEmissionAndDensity", SF_Compute);

#if WITH_EDITOR
IMPLEMENT_STATIC_UNIFORM_BUFFER_STRUCT(FVirtualShadowMapUniformParameters, "ZibraVirtualShadowMap", VirtualShadowMapUbSlot)
#endif
