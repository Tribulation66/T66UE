// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "HAL/Runnable.h"
#include "ZibraVDBSDKIntegration/Public/ZibraVDBCompressorManager.h"

class ZIBRAVDBEDITOR_API FZibraCompressionWorker : public FRunnable
{
public:
	FZibraCompressionWorker(TSharedPtr<FZibraVDBCompressorManager> CompressorManager, const FString& InputFileMask,
		TFunction<void()> InCompletionCallback = TFunction<void()>([]() {}));
	virtual ~FZibraCompressionWorker() final;

	//~ Begin FRunnable Interface.
	bool Init() final;
	uint32 Run() final;
	void Stop() final;
	//~ End FRunnable Interface.

private:
	TSharedPtr<FZibraVDBCompressorManager> CompressorManager;
	FString InputFileMask;
	FString OutputFile;

	TFunction<void()> CompletionCallback;

	// Thread handle. Control the thread using this, with operators like Kill and Suspend
	FRunnableThread* Thread;
};
