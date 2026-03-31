// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include <Zibra/CE/Decompression.h>
#include <Zibra/RHI.h>
#include <ZibraVDBStreamWrappers.h>

DECLARE_LOG_CATEGORY_EXTERN(LogZibraVDBDecompression, Log, All);

class ZIBRAVDBSDKINTEGRATION_API FZibraVDBDecompressorManager
{
public:
	Zibra::CE::ReturnCode Initialize(const TArray64<uint8>& ZibraVDBBinaryData) noexcept;
	void Release() noexcept;
	Zibra::CE::ReturnCode Decompress(int FrameIndex) const noexcept;

	Zibra::CE::Decompression::FormatMapper* GetFormatMapper() const noexcept;

	Zibra::CE::Decompression::SequenceInfo GetSequenceInfo() const noexcept;
	Zibra::CE::Decompression::PlaybackInfo GetPlaybackInfo() const noexcept;
	Zibra::CE::Decompression::FrameRange GetFrameRange() const noexcept;
	Zibra::CE::Decompression::FrameInfo GetFrameInfo(uint32 FrameIndex) const noexcept;
	Zibra::CE::Decompression::DecompressorResourcesRequirements GetDecompressorResourcesRequirements() const noexcept;
	Zibra::CE::Decompression::MaxDimensionsPerSubmit GetMaxDimensionsPerSubmit() const noexcept;

	Zibra::CE::ReturnCode RegisterResources(const Zibra::CE::Decompression::DecompressorResources& Resources) const noexcept;

	bool bInitialized = false;
	mutable bool bBuffersBound = false;

private:
	Zibra::CE::ReturnCode InitializeRHI();
	Zibra::CE::ReturnCode InitializeDecompressor(const TArray64<uint8>& ZibraVDBBinaryData);

	Zibra::RHI::RHIRuntime* RHIRuntime = nullptr;
	Zibra::CE::Decompression::DecompressorFactory* DecompressorFactory = nullptr;
	Zibra::CE::ZibraVDB::FileDecoder* FileDecoder = nullptr;
	Zibra::CE::Decompression::Decompressor* Decompressor = nullptr;
	Zibra::CE::Decompression::FormatMapper* FormatMapper = nullptr;

	TSharedPtr<FMemoryReaderWrapper> MemoryReader;
};
