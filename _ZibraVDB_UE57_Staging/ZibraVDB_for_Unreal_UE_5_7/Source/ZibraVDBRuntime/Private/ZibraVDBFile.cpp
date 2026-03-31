// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBFile.h"

#include "Misc/FileHelper.h"
#include "ZibraVDBDecompressorManager.h"
#include "ZibraVDBNotifications.h"
#include "ZibraVDBSDKIntegration.h"

DEFINE_LOG_CATEGORY_STATIC(LogZibraVDBSerialization, Log, All);

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

namespace ZibraVDB
{
	static const FCustomVersionRegistration ZibraVDBFileVersionRegistration(
		ZibraVDBFileGUID, CurrentSerializationVersion, TEXT("ZibraVDBFileVersion"));

	FZibraVDBFile::FZibraVDBFile() = default;

	FZibraVDBFile::FZibraVDBFile(FString FileName) : FileName(MoveTemp(FileName))
	{
	}

	bool FZibraVDBFile::HasBinaryData() const noexcept
	{
		return !BinaryData.IsEmpty();
	}
	const FString& FZibraVDBFile::GetFileName() const noexcept
	{
		return FileName;
	}
	float FZibraVDBFile::GetZibraVDBSize() const noexcept
	{
		return ZibraVDBSize;
	}
	float FZibraVDBFile::GetOriginalOpenVDBSize() const noexcept
	{
		return OriginalOpenVDBSize;
	}
	const TArray64<uint8>& FZibraVDBFile::GetBinaryData() const noexcept
	{
		return BinaryData;
	}
	const FZibraVDBHeader& FZibraVDBFile::GetHeader() const noexcept
	{
		return Header;
	}
	const FZibraVDBMetadata& FZibraVDBFile::GetMetadata() const noexcept
	{
		return Metadata;
	}

	bool FZibraVDBFile::ReadFromFile(const FString& FilePath)
	{
		if (!IFileManager::Get().FileExists(*FilePath))
		{
			FZibraVDBNotifications::AddNotification(LOCTEXT("FileDoesntExist", "File does not exist."));
			return false;
		}

		const bool bLoadSuccessful = FFileHelper::LoadFileToArray(BinaryData, *FilePath);

		if (!bLoadSuccessful)
		{
			FZibraVDBNotifications::AddNotification(LOCTEXT("LoadFileToArrayFailed", "Failed to load data from file."));
			return false;
		}

		FZibraVDBDecompressorManager DecompressorManager;
		auto ReturnCode = DecompressorManager.Initialize(BinaryData);
		if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
		{
			FZibraVDBNotifications::AddNotification(
				FText::Format(LOCTEXT("DecompressorInitializationFailure", "Failed to initialize decompressor: {0}"),
					FText::FromString(FZibraVDBSDKIntegration::ErrorToString(ReturnCode))));
			return false;
		}
		Zibra::CE::Decompression::SequenceInfo SequenceInfo = DecompressorManager.GetSequenceInfo();
		Zibra::CE::Decompression::PlaybackInfo PlaybackInfo = DecompressorManager.GetPlaybackInfo();
		Zibra::CE::Decompression::FrameRange FrameRange = DecompressorManager.GetFrameRange();

		uint32 MaxSpatialBlockCount = 0;
		uint32 MaxChannelBlockCount = 0;
		for (int FrameIndex = FrameRange.start; FrameIndex <= FrameRange.end; FrameIndex++)
		{
			Zibra::CE::Decompression::FrameInfo FrameInfo = DecompressorManager.GetFrameInfo(FrameIndex);
			MaxSpatialBlockCount = FMath::Max(MaxSpatialBlockCount, FrameInfo.spatialBlockCount);
			MaxChannelBlockCount = FMath::Max(MaxChannelBlockCount, FrameInfo.channelBlockCount);
		}

		ZibraVDBSize = float(IFileManager::Get().FileSize(*FilePath));
		FileName = FilePath;
		OriginalOpenVDBSize = SequenceInfo.originalSize;

		static_assert(sizeof(Header.UUID) == sizeof(SequenceInfo.fileUUID));
		memcpy(Header.UUID, SequenceInfo.fileUUID, sizeof(Header.UUID));
		Header.FrameCount = PlaybackInfo.frameCount;
		Header.OriginalSize = SequenceInfo.originalSize;
		Header.AABBSize = {int(SequenceInfo.maxAABBSize.x), int(SequenceInfo.maxAABBSize.y), int(SequenceInfo.maxAABBSize.z)};
		Header.MaxSpatialBlockCount = MaxSpatialBlockCount;
		Header.MaxChannelBlockCount = MaxChannelBlockCount;
		Header.ChannelNames.SetNum(SequenceInfo.channelCount);
		for (uint32_t i = 0; i < SequenceInfo.channelCount; ++i)
		{
			Header.ChannelNames[i] = SequenceInfo.channels[i];
		}

		Metadata.CompressionRate = OriginalOpenVDBSize / ZibraVDBSize;
		// TODO Decompression additional metadata

		DecompressorManager.Release();

		return true;
	}

	void ZibraVDB::FZibraVDBFile::Export(const FString& OutputFilePath) const
	{
		FFileHelper::SaveArrayToFile(BinaryData, *OutputFilePath);
	}

	// Begin operator overloads.
	ZIBRAVDBRUNTIME_API FArchive& operator<<(FArchive& Ar, FZibraVDBHeader& ZibraVDBHeader)
	{
		Ar << ZibraVDBHeader.UUID[0] << ZibraVDBHeader.UUID[1] << ZibraVDBHeader.FrameCount << ZibraVDBHeader.OriginalSize
		   << ZibraVDBHeader.AABBSize << ZibraVDBHeader.MaxWordCount << ZibraVDBHeader.MaxSpatialBlockCount
		   << ZibraVDBHeader.MaxChannelBlockCount << ZibraVDBHeader.ChannelNames;
		return Ar;
	}
	ZIBRAVDBRUNTIME_API FArchive& operator<<(FArchive& Ar, FZibraVDBMetadata& ZibraVDBMetadata)
	{
		// TODO Decompression additional metadata
		Ar << ZibraVDBMetadata.CompressionRate;
		return Ar;
	}
	ZIBRAVDBRUNTIME_API FArchive& ZibraVDB::operator<<(FArchive& Ar, FZibraVDBFile& ZibraVDBFile)
	{
		Ar.UsingCustomVersion(ZibraVDBFileGUID);
		int32 Version = Ar.CustomVer(ZibraVDBFileGUID);

		Ar << ZibraVDBFile.FileName << ZibraVDBFile.Header << ZibraVDBFile.Metadata;

		if (Ar.IsLoading())
		{
			// Version is -1 if asset was saved before implementing versioning
			if (Version == -1)
			{
				TArray<uint8> LegacyBinaryData;
				Ar << LegacyBinaryData;
				ZibraVDBFile.BinaryData.Append(LegacyBinaryData.GetData(), LegacyBinaryData.Num());
			}
			else if (Version == 0)
			{
				Ar << ZibraVDBFile.BinaryData;
			}
			else
			{
				UE_LOG(LogZibraVDBSerialization, Error, TEXT("Unsupported file version: %d"), Version);
				return Ar;
			}
		}
		else if (Ar.IsSaving())
		{
			Ar << ZibraVDBFile.BinaryData;
		}
		return Ar;
	}
	// End operator overloads.
}	 // namespace ZibraVDB

#undef LOCTEXT_NAMESPACE
