// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraReadMetadataWorker.h"

#include "HAL/RunnableThread.h"
#include "ZibraLicensingManager.h"
#include "ZibraVDBSDKIntegration.h"
#include "ZibraVDBSDKIntegration/Public/ZibraVDBCompressorManager.h"

FZibraReadMetadataWorker::FZibraReadMetadataWorker(const std::vector<std::filesystem::path>& InFilePaths,
	TFunction<void(Zibra::OpenVDBHelper::SequenceInfo*)> InCompletionCallback)
	: FilePaths(InFilePaths), CompletionCallback(InCompletionCallback)
{
	// Init all attributes before creating a thread.
	Thread = FRunnableThread::Create(this, TEXT("FZibraReadMetadataWorker"));
}

FZibraReadMetadataWorker::~FZibraReadMetadataWorker()
{
	if (Thread)
	{
		// Kill() is a blocking call, it waits for the thread to finish.
		Thread->Kill();
		delete Thread;
	}
}

bool FZibraReadMetadataWorker::Init()
{
	// Return false if you want to abort the thread
	return true;
}

uint32 FZibraReadMetadataWorker::Run()
{
	std::vector<const wchar_t*> FilePathCStrings;
	FilePathCStrings.reserve(FilePaths.size());
	for (const std::filesystem::path& FilePath : FilePaths)
	{
		FilePathCStrings.push_back(FilePath.native().c_str());
	}

	Zibra::OpenVDBHelper::SequenceInfo* SequenceInfo =
		FZibraVDBSDKIntegration::ReadSequenceInfo(FilePathCStrings.data(), FilePathCStrings.size(), &Progress);
	CompletionCallback(SequenceInfo);

	return 0;
}

void FZibraReadMetadataWorker::Stop()
{
}
