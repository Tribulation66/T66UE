// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBRenderBuffer.h"

#include "RenderingThread.h"

// --- Begin FZibraVDBNativeResource ---

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
void FZibraVDBNativeResource::InitResource(FRHICommandListBase& RHICmdList)
#else
void FZibraVDBNativeResource::InitResource()
#endif
{
	check(IsInRenderingThread());

	CurrentFrameIndex = -1;

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 503
	FRenderResource::InitResource(RHICmdList);

	DecompressionPerChannelBlockData.InitResource(RHICmdList);
	DecompressionPerSpatialBlockInfo.InitResource(RHICmdList);
	DecompressionPerChannelBlockInfo.InitResource(RHICmdList);
	RenderBlockBuffer.InitResource(RHICmdList);
#else
	FRenderResource::InitResource();

	DecompressionPerChannelBlockData.InitResource();
	DecompressionPerSpatialBlockInfo.InitResource();
	DecompressionSpatialToChannelIndexLookup.InitResource();
	DecompressionPerChannelBlockInfo.InitResource();
	RenderBlockBuffer.InitResource();
#endif
}

void FZibraVDBNativeResource::ReleaseResource()
{
	// Flush the rendering command to be sure there is no command left that can create/modify a rendering resource
	FlushRenderingCommands();
	
	BeginReleaseResource(&DecompressionPerChannelBlockData);
	BeginReleaseResource(&DecompressionPerSpatialBlockInfo);
	BeginReleaseResource(&DecompressionPerChannelBlockInfo);

	BeginReleaseResource(&RenderBlockBuffer);

	// In case of reimport or program termination this UObject gets deleted before resource is released. Force an extra flush.
	FlushRenderingCommands();

	FRenderResource::ReleaseResource();
}

// --- End FZibraVDBNativeResource ---
