// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "HAL/Runnable.h"

#include <Zibra/OpenVDBHelper.h>

class ZIBRAVDBEDITOR_API FZibraReadMetadataWorker : public FRunnable
{
public:
	FZibraReadMetadataWorker(const std::vector<std::filesystem::path>& InFilePaths,
		TFunction<void(Zibra::OpenVDBHelper::SequenceInfo*)> InCompletionCallback);
	virtual ~FZibraReadMetadataWorker() final;

	//~ Begin FRunnable Interface.
	bool Init() final;
	uint32 Run() final;
	void Stop() final;
	//~ End FRunnable Interface.

	float Progress = 0.0f;

private:
	// Thread handle. Control the thread using this, with operators like Kill and Suspend
	FRunnableThread* Thread;

	std::vector<std::filesystem::path> FilePaths;

	TFunction<void(Zibra::OpenVDBHelper::SequenceInfo*)> CompletionCallback;
};
