// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBGPUProfiler.h"

bool FZibraVDBGPUProfiler::FGPUFrameData::CanWrite() const noexcept
{
	return StartQuery.GetQuery() == nullptr;
}

bool FZibraVDBGPUProfiler::FGPUFrameData::CanRead() const noexcept
{
	return StartQuery.GetQuery() != nullptr;
}

FZibraVDBGPUProfiler::FZibraVDBGPUProfiler()
{
	QueryPool = RHICreateRenderQueryPool(RQT_AbsoluteTime);
}

FZibraVDBGPUProfiler::~FZibraVDBGPUProfiler()
{
	for (FGPUFrameData& Frame : GPUFrames)
	{
		Frame.StartQuery.ReleaseQuery();
		Frame.EndQuery.ReleaseQuery();
	}
}

void FZibraVDBGPUProfiler::BeginFrame(FRHICommandListImmediate& RHICmdList)
{
	// Get frame to write into
	ActiveWriteFrame = GetWriteFrame();

	if (ActiveWriteFrame == nullptr)
	{
		return;
	}

	ActiveWriteFrame->StartQuery = QueryPool->AllocateQuery();
	RHICmdList.EndRenderQuery(ActiveWriteFrame->StartQuery.GetQuery());
}
void FZibraVDBGPUProfiler::EndFrame(FRHICommandList& RHICmdList)
{
	if (ActiveWriteFrame == nullptr)
	{
		return;
	}

	ActiveWriteFrame->EndQuery = QueryPool->AllocateQuery();
	RHICmdList.EndRenderQuery(ActiveWriteFrame->EndQuery.GetQuery());

	ActiveWriteFrame = nullptr;
	CurrentWriteFrame = (CurrentWriteFrame + 1) % NumBufferFrames;
}

bool FZibraVDBGPUProfiler::TryGetFrameResult(FRHICommandListImmediate& RHICmdList, uint64& OutDurationMicroseconds)
{
	FGPUFrameData* ReadFrame = GetReadFrame();
	if (!ReadFrame)
	{
		return false;
	}

	if (!ProcessFrame(RHICmdList, *ReadFrame))
	{
		return false;
	}

	CurrentReadFrame = (CurrentReadFrame + 1) % NumBufferFrames;

	OutDurationMicroseconds = DurationMicroseconds;
	DurationMicroseconds = 0;
	return true;
}

FZibraVDBGPUProfiler::FGPUFrameData* FZibraVDBGPUProfiler::GetReadFrame()
{
	check(CurrentReadFrame >= 0 && CurrentReadFrame < UE_ARRAY_COUNT(GPUFrames));
	return GPUFrames[CurrentReadFrame].CanRead() ? &GPUFrames[CurrentReadFrame] : nullptr;
}

FZibraVDBGPUProfiler::FGPUFrameData* FZibraVDBGPUProfiler::GetWriteFrame()
{
	check(CurrentWriteFrame >= 0 && CurrentWriteFrame < UE_ARRAY_COUNT(GPUFrames));
	return GPUFrames[CurrentWriteFrame].CanWrite() ? &GPUFrames[CurrentWriteFrame] : nullptr;
}

bool FZibraVDBGPUProfiler::ProcessFrame(FRHICommandListImmediate& RHICmdList, FGPUFrameData& ReadFrame)
{
	uint64 StartMicroseconds = 0;
	if (!RHIGetRenderQueryResult(ReadFrame.StartQuery.GetQuery(), StartMicroseconds, false))
	{
		return false;
	}

	uint64 EndMicroseconds = 0;
	ensure(RHIGetRenderQueryResult(ReadFrame.EndQuery.GetQuery(), EndMicroseconds, false));
	ReadFrame.StartQuery.ReleaseQuery();
	ReadFrame.EndQuery.ReleaseQuery();

	DurationMicroseconds = EndMicroseconds - StartMicroseconds;

	return true;
}
