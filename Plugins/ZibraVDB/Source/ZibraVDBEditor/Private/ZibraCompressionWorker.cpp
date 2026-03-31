// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraCompressionWorker.h"

#include "HAL/RunnableThread.h"
#include "ZibraLicensingManager.h"

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

FZibraCompressionWorker::FZibraCompressionWorker(TSharedPtr<FZibraVDBCompressorManager> InCompressorManager,
	const FString& InputFileMask, TFunction<void()> InCompletionCallback)
	: CompressorManager(InCompressorManager), InputFileMask(InputFileMask), CompletionCallback(InCompletionCallback)
{
	// Init all attributes before creating a thread.
	Thread = FRunnableThread::Create(this, TEXT("FZibraCompressionWorker"));
}

FZibraCompressionWorker::~FZibraCompressionWorker()
{
	if (Thread)
	{
		// Kill() is a blocking call, it waits for the thread to finish.
		Thread->Kill();
		delete Thread;
	}
}

bool FZibraCompressionWorker::Init()
{
	return true;
}

uint32 FZibraCompressionWorker::Run()
{
	if (!FZibraLicensingManager::GetInstance().IsLicenseVerified(FZibraLicensingManager::Product::Compression))
	{
		FString ErrorString;
		static const FText ErrorTitle = LOCTEXT("ZibraVDBLicenseErrorTitle", "ZibraVDB License Error");
		if (FZibraLicensingManager::GetInstance().IsAnyLicenseVerified())
		{
			ErrorString = TEXT(
				"Your license does not include Compression functionality. You can change your license via Project Settings. You can purchase new ZibraVDB license on https://zibra.ai");
		}
		else
		{
			ErrorString = TEXT("Compression requires active license. You can enter your license details via Project Settings. You can purchase ZibraVDB license on https://zibra.ai");
		}
		UE_LOG(LogZibraVDBLicensing, Error, TEXT("%s"), *ErrorString);
	}

	auto ReturnCode = CompressorManager->CompressFrameSequence(InputFileMask);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		UE_LOG(LogZibraVDBCompression, Error, TEXT("Failed to compress a sequence."))
		return 0;
	}

	CompletionCallback();

	return 0;
}

void FZibraCompressionWorker::Stop()
{
	CompressorManager->AbortCompression();
}

#undef LOCTEXT_NAMESPACE
