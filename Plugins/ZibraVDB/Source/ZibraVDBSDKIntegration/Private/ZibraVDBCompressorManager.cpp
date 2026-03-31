// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.
#include "ZibraVDBCompressorManager.h"

#if WITH_EDITOR

#include "ZibraVDBRHI/Public/ZibraUnrealRHI.h"
#include "ZibraVDBSDKIntegration.h"

Zibra::CE::ReturnCode FZibraVDBCompressorManager::InitializeRHI() noexcept
{
	Zibra::RHI::RHIFactory* RHIFactory = nullptr;
	auto ReturnCode = Zibra::RHI::CAPI::CreateRHIFactory(&RHIFactory);
	if (ReturnCode != Zibra::RHI::ZRHI_SUCCESS)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	ReturnCode = RHIFactory->SetGFXAPI(Zibra::RHI::GFXAPI::D3D12);
	if (ReturnCode != Zibra::RHI::ZRHI_SUCCESS)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	ReturnCode = RHIFactory->Create(&RHIRuntime);
	if (ReturnCode != Zibra::RHI::ZRHI_SUCCESS)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	RHIFactory->Release();

	ReturnCode = RHIRuntime->Initialize();
	if (ReturnCode != Zibra::RHI::ZRHI_SUCCESS)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	return Zibra::CE::ZCE_SUCCESS;
}

Zibra::CE::ReturnCode FZibraVDBCompressorManager::InitializeCompressor(
	float DefaultQuality, TArray<TPair<FString, float>> PerChannelCompressionSettings) noexcept
{
	Zibra::CE::Compression::CompressorFactory* CompressorFactory = nullptr;
	CompressorFactory = Zibra::CE::Compression::CAPI::CreateCompressorFactory();
	if (!CompressorFactory)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	auto ReturnCode = CompressorFactory->UseRHI(RHIRuntime);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	ReturnCode = CompressorFactory->SetQuality(DefaultQuality);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	for (const auto& [ChannelName, Quality] : PerChannelCompressionSettings)
	{
		ReturnCode = CompressorFactory->OverrideChannelQuality(TCHAR_TO_UTF8(*ChannelName), Quality);
		if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
		{
			return Zibra::CE::ZCE_ERROR;
		}
	}

	ReturnCode = CompressorFactory->Create(&Compressor);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	CompressorFactory->Release();

	ReturnCode = Compressor->Initialize();
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	return Zibra::CE::ZCE_SUCCESS;
}

Zibra::CE::ReturnCode FZibraVDBCompressorManager::Initialize(float DefaultQuality,
	const Zibra::OpenVDBHelper::SequenceInfo& SequenceInfo, TArray<TPair<FString, float>> PerChannelCompressionSettings) noexcept
{
	if (!FZibraVDBSDKIntegration::IsPluginEnabled())
	{
		return Zibra::CE::ZCE_ERROR;
	}

	auto ReturnCode = InitializeRHI();
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	ReturnCode = InitializeCompressor(DefaultQuality, PerChannelCompressionSettings);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	CachedPerChannelCompressionSettings = PerChannelCompressionSettings;
	CurrSequenceInfo = SequenceInfo;
	bInitialized = true;

	return Zibra::CE::ZCE_SUCCESS;
}

void FZibraVDBCompressorManager::Release() noexcept
{
	if (Compressor)
	{
		Compressor->Release();
	}
	if (RHIRuntime)
	{
		RHIRuntime->Release();
	}
}

Zibra::CE::ReturnCode FZibraVDBCompressorManager::CompressFrameSequence(const FString& InputFileMask) noexcept
{
	if (CurrSequenceInfo.channelCount == 0)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	std::vector<std::filesystem::path> FilePaths = FZibraVDBSDKIntegration::CalculateFileList(InputFileMask);

	auto RHIReturnCode = RHIRuntime->StartRecording();
	if (RHIReturnCode != Zibra::RHI::ZRHI_SUCCESS)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	auto ReturnCode = Compressor->StartSequence();
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		return ReturnCode;
	}

	TArray<FTCHARToUTF8> Converters;
	TArray<const char*> CStrings;
	Converters.Reserve(CachedPerChannelCompressionSettings.Num());
	for (const auto& [Channel, Quality] : CachedPerChannelCompressionSettings)
	{
		Converters.Emplace(*Channel);
		CStrings.Add(Converters.Last().Get());
	}

	bCompressionInProgress = true;
	bAborting = false;
	CurrentSequenceFrameCount = FilePaths.size();

	for (const std::filesystem::path& FilePath : FilePaths)
	{
		Zibra::CE::Compression::CompressFrameDesc compressFrameDesc{};
		compressFrameDesc.channelsCount = CachedPerChannelCompressionSettings.Num();
		compressFrameDesc.channels = CStrings.GetData();

		Zibra::CE::Compression::FrameManager* frameManager = nullptr;

		std::wstring WidePath = FilePath.native();

		Zibra::CE::Compression::SparseFrame* SparseFrame =
			Zibra::OpenVDBHelper::CAPI::ReadFrame(WidePath.c_str(), compressFrameDesc.channels, compressFrameDesc.channelsCount);
		if (!SparseFrame)
		{
			return Zibra::CE::ZCE_ERROR;
		}

		SparseFrame->aabb.maxX -= SparseFrame->aabb.minX;
		SparseFrame->aabb.maxY -= SparseFrame->aabb.minY;
		SparseFrame->aabb.maxZ -= SparseFrame->aabb.minZ;
		SparseFrame->aabb.minX = 0;
		SparseFrame->aabb.minY = 0;
		SparseFrame->aabb.minZ = 0;

		compressFrameDesc.frame = SparseFrame;

		auto status = Compressor->CompressFrame(compressFrameDesc, &frameManager);
		if (status != Zibra::CE::ZCE_SUCCESS)
		{
			return status;
		}

		Zibra::OpenVDBHelper::CAPI::ReleaseSparseFrame(compressFrameDesc.frame);

		ReturnCode = frameManager->Finish();
		if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
		{
			return ReturnCode;
		}

		Counter.Increment();

		if (bAborting)
		{
			break;
		}
	}

	if (!bAborting)
	{
		TUniquePtr<Zibra::Legacy::OStream> OStreamPtr = MakeUnique<FArchiveWrapper>(OutputFile);
		ReturnCode = Compressor->FinishSequence(OStreamPtr.Get());
		if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
		{
			return ReturnCode;
		}
	}

	bCompressionInProgress = false;

	RHIReturnCode = RHIRuntime->StopRecording();
	if (RHIReturnCode != Zibra::RHI::ZRHI_SUCCESS)
	{
		return Zibra::CE::ZCE_ERROR;
	}

	return Zibra::CE::ZCE_SUCCESS;
}

void FZibraVDBCompressorManager::SetOutputFile(FString FilePath) noexcept
{
	OutputFile = FilePath;
}

float FZibraVDBCompressorManager::GetCompressionProgress() const noexcept
{
	return bCompressionInProgress ? (static_cast<float>(Counter.GetValue()) / CurrentSequenceFrameCount) : 0.0f;
}

void FZibraVDBCompressorManager::AbortCompression() noexcept
{
	bAborting = true;
}

#endif
