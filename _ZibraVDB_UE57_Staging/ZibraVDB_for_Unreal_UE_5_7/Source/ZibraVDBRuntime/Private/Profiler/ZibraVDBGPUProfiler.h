// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once
#include "HAL/Platform.h"
#include "RHIResources.h"

class FZibraVDBGPUProfiler
{
public:
	static constexpr uint32 NumBufferFrames = 5;

	struct FGPUFrameData
	{
		bool CanWrite() const noexcept;
		bool CanRead() const noexcept;

		FRHIPooledRenderQuery StartQuery;
		FRHIPooledRenderQuery EndQuery;
	};

	FZibraVDBGPUProfiler();
	~FZibraVDBGPUProfiler();

	void BeginFrame(FRHICommandListImmediate& RHICmdList);
	void EndFrame(FRHICommandList& RHICmdList);

	bool TryGetFrameResult(FRHICommandListImmediate& RHICmdList, uint64& OutDurationMicroseconds);

private:
	FGPUFrameData* GetReadFrame();
	FGPUFrameData* GetWriteFrame();

	bool ProcessFrame(FRHICommandListImmediate& RHICmdList, FGPUFrameData& ReadFrame);

	int32 CurrentReadFrame = 0;		// Index of the next frame to read from
	int32 CurrentWriteFrame = 0;	// Index of the next frame to write into
	FGPUFrameData GPUFrames[NumBufferFrames];

	FGPUFrameData* ActiveWriteFrame = nullptr;
	FRenderQueryPoolRHIRef QueryPool;

	uint64 DurationMicroseconds = -1;
};
