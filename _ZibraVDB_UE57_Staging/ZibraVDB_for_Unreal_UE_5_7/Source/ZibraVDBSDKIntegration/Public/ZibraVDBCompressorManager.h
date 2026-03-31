// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#if WITH_EDITOR

#include <Zibra/CE/Compression.h>
#include <Zibra/OpenVDBHelper.h>
#include <Zibra/RHI.h>
#include <ZibraVDBStreamWrappers.h>

DEFINE_LOG_CATEGORY_STATIC(LogZibraVDBCompression, Log, All);

class ZIBRAVDBSDKINTEGRATION_API FZibraVDBCompressorManager
{
public:
	Zibra::CE::ReturnCode Initialize(float DefaultQuality, const Zibra::OpenVDBHelper::SequenceInfo& SequenceInfo,
		TArray<TPair<FString, float>> PerChannelCompressionSettings) noexcept;
	void Release() noexcept;

	Zibra::CE::ReturnCode CompressFrameSequence(const FString& InputFileMask) noexcept;

	void SetOutputFile(FString FilePath) noexcept;

	float GetCompressionProgress() const noexcept;

	void AbortCompression() noexcept;

	static Zibra::OpenVDBHelper::SequenceInfo ReadSequenceInfo(const TArray<FString>& FileList, float* Progress) noexcept;

	bool bInitialized = false;

private:
	Zibra::CE::ReturnCode InitializeRHI() noexcept;
	Zibra::CE::ReturnCode InitializeCompressor(
		float DefaultQuality, TArray<TPair<FString, float>> PerChannelCompressionSettings) noexcept;

	Zibra::RHI::RHIRuntime* RHIRuntime = nullptr;
	Zibra::CE::Compression::Compressor* Compressor = nullptr;

	TArray<TPair<FString, float>> CachedPerChannelCompressionSettings;

	Zibra::OpenVDBHelper::SequenceInfo CurrSequenceInfo;

	bool bAborting = false;
	bool bCompressionInProgress = false;
	int32 CurrentSequenceFrameCount = 0;
	FThreadSafeCounter Counter = 0;
	FString OutputFile;
};

#endif
