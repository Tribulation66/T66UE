// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.
#include "ZibraVDBDecompressorManager.h"

#include "RenderCommandFence.h"
#include "ZibraVDBRHI/Public/ZibraUnrealRHI.h"
#include "ZibraVDBSDKIntegration.h"

DEFINE_LOG_CATEGORY(LogZibraVDBDecompression);

Zibra::CE::ReturnCode FZibraVDBDecompressorManager::InitializeRHI()
{
	RHIRuntime = &Zibra::RHI::APIUnrealRHI::ZibraUnrealRHI::Get();
	return Zibra::CE::ZCE_SUCCESS;
}

Zibra::CE::ReturnCode FZibraVDBDecompressorManager::InitializeDecompressor(const TArray64<uint8>& ZibraVDBBinaryData)
{
	auto ReturnCode = Zibra::CE::Decompression::CAPI::CreateDecompressorFactory(&DecompressorFactory);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return ReturnCode;
	}

	ReturnCode = DecompressorFactory->UseRHI(RHIRuntime);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return ReturnCode;
	}

	MemoryReader = MakeShared<FMemoryReaderWrapper>(ZibraVDBBinaryData);
	ReturnCode = Zibra::CE::Decompression::CAPI::CreateDecoderFromStream(MemoryReader.Get(), &FileDecoder);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return ReturnCode;
	}

	ReturnCode = DecompressorFactory->UseDecoder(FileDecoder);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return ReturnCode;
	}

	ReturnCode = DecompressorFactory->Create(&Decompressor);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return ReturnCode;
	}

	FormatMapper = Decompressor->GetFormatMapper();
	if (!FormatMapper)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	ENQUEUE_RENDER_COMMAND(InitializeDecompressor)
	([this](FRHICommandListImmediate& RHICmdList) { Decompressor->Initialize(); });

	FRenderCommandFence RenderFence;
	RenderFence.BeginFence();
	RenderFence.Wait();

	return Zibra::CE::ZCE_SUCCESS;
}

Zibra::CE::ReturnCode FZibraVDBDecompressorManager::Initialize(const TArray64<uint8>& ZibraVDBBinaryData) noexcept
{
	if (!FZibraVDBSDKIntegration::IsPluginEnabled())
	{
		return Zibra::CE::ZCE_ERROR;
	}

	auto ReturnCode = InitializeRHI();
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return ReturnCode;
	}

	ReturnCode = InitializeDecompressor(ZibraVDBBinaryData);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return ReturnCode;
	}

	bInitialized = true;
	return Zibra::CE::ZCE_SUCCESS;
}

void FZibraVDBDecompressorManager::Release() noexcept
{
	if (FileDecoder)
	{
		Zibra::CE::Decompression::CAPI::ReleaseDecoder(FileDecoder);
		FileDecoder = nullptr;
	}
	if (FormatMapper)
	{
		FormatMapper->Release();
		FormatMapper = nullptr;
	}
	if (Decompressor)
	{
		Decompressor->Release();
		Decompressor = nullptr;
	}
	if (RHIRuntime)
	{
		RHIRuntime->Release();
		RHIRuntime = nullptr;
	}
}

Zibra::CE::ReturnCode FZibraVDBDecompressorManager::Decompress(int FrameIndex) const noexcept
{
	Zibra::CE::ReturnCode ReturnCode;
	Zibra::RHI::ReturnCode RHIReturnCode;
	Zibra::CE::Decompression::CompressedFrameContainer* FrameContainer = nullptr;
	ReturnCode = FormatMapper->FetchFrame(FrameIndex, &FrameContainer);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return ReturnCode;
	}

	const Zibra::CE::Decompression::FrameInfo FrameInfo = FrameContainer->GetInfo();
	const Zibra::CE::Decompression::MaxDimensionsPerSubmit MaxDimensionsPerSubmit = Decompressor->GetMaxDimensionsPerSubmit();
	const uint32 MaxChunkSize = MaxDimensionsPerSubmit.maxSpatialBlocks;
	const uint32 ChunksCount = (FrameInfo.spatialBlockCount + MaxChunkSize - 1) / MaxChunkSize;

	if (FrameInfo.spatialBlockCount <= MaxChunkSize)
	{
		Zibra::CE::Decompression::DecompressFrameDesc DecompressDesc{};
		DecompressDesc.frameContainer = FrameContainer;
		DecompressDesc.firstSpatialBlockIndex = 0;
		DecompressDesc.spatialBlocksCount = FrameInfo.spatialBlockCount;
		DecompressDesc.decompressionPerChannelBlockDataOffset = 0;
		DecompressDesc.decompressionPerChannelBlockInfoOffset = 0;
		DecompressDesc.decompressionPerSpatialBlockInfoOffset = 0;

		Zibra::CE::Decompression::DecompressedFrameFeedback FrameFeedback{};

		Zibra::CE::Decompression::ReturnCode status = Decompressor->DecompressFrame(DecompressDesc, &FrameFeedback);
		if (status != Zibra::CE::ZCE_SUCCESS)
		{
			return status;
		}

		RHIReturnCode = RHIRuntime->GarbageCollect();
		if (RHIReturnCode != Zibra::RHI::ZRHI_SUCCESS)
		{
			return Zibra::CE::ZCE_ERROR;
		}
		RHIReturnCode = RHIRuntime->StopRecording();
		if (RHIReturnCode != Zibra::RHI::ZRHI_SUCCESS)
		{
			return Zibra::CE::ZCE_ERROR;
		}
	}
	else
	{
		UE_LOG(LogZibraVDBDecompression, Warning, TEXT("VDB frame is too big to be decompressed."));
	}

	FrameContainer->Release();

	return Zibra::CE::ZCE_SUCCESS;
}

Zibra::CE::Decompression::FormatMapper* FZibraVDBDecompressorManager::GetFormatMapper() const noexcept
{
	return FormatMapper;
}

Zibra::CE::Decompression::SequenceInfo FZibraVDBDecompressorManager::GetSequenceInfo() const noexcept
{
	if (!FormatMapper)
	{
		return Zibra::CE::Decompression::SequenceInfo{};
	}

	return FormatMapper->GetSequenceInfo();
}

Zibra::CE::Decompression::PlaybackInfo FZibraVDBDecompressorManager::GetPlaybackInfo() const noexcept
{
	if (!FormatMapper)
	{
		return Zibra::CE::Decompression::PlaybackInfo{};
	}

	return FormatMapper->GetPlaybackInfo();
}

Zibra::CE::Decompression::FrameRange FZibraVDBDecompressorManager::GetFrameRange() const noexcept
{
	if (!FormatMapper)
	{
		return Zibra::CE::Decompression::FrameRange{};
	}

	return FormatMapper->GetFrameRange();
}

Zibra::CE::Decompression::FrameInfo FZibraVDBDecompressorManager::GetFrameInfo(uint32 FrameIndex) const noexcept
{
	if (!FormatMapper)
	{
		return Zibra::CE::Decompression::FrameInfo{};
	}

	Zibra::CE::Decompression::FrameInfo FrameInfo{};
	Zibra::CE::ReturnCode ReturnCode = FormatMapper->FetchFrameInfo(FrameIndex, &FrameInfo);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		// TODO: Add notification
		return FrameInfo;
	}
	return FrameInfo;
}

Zibra::CE::Decompression::DecompressorResourcesRequirements FZibraVDBDecompressorManager::GetDecompressorResourcesRequirements()
	const noexcept
{
	return Decompressor ? Decompressor->GetResourcesRequirements() : Zibra::CE::Decompression::DecompressorResourcesRequirements{};
}

Zibra::CE::Decompression::MaxDimensionsPerSubmit FZibraVDBDecompressorManager::GetMaxDimensionsPerSubmit() const noexcept
{
	return Decompressor ? Decompressor->GetMaxDimensionsPerSubmit() : Zibra::CE::Decompression::MaxDimensionsPerSubmit{};
}

Zibra::CE::ReturnCode FZibraVDBDecompressorManager::RegisterResources(
	const Zibra::CE::Decompression::DecompressorResources& Resources) const noexcept
{
	if (!Decompressor || !RHIRuntime)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	bBuffersBound = true;

	return Decompressor->RegisterResources(Resources);
}
