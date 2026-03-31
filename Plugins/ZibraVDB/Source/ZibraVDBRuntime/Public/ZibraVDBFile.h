// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/CustomVersion.h"

#include <array>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

namespace ZibraVDB
{
	enum class EChannelType : int32_t
	{
		Density = 0,
		Temperature = 1,
		Flame = 2,
		Num = 3
	};

	struct FZibraVDBHeader
	{
		uint64_t UUID[2];
		uint32_t FrameCount;
		uint64_t OriginalSize;
		FIntVector AABBSize;
		// Deprecated.
		uint32_t MaxWordCount;
		uint32_t MaxSpatialBlockCount;
		uint32_t MaxChannelBlockCount;
		TArray<FString> ChannelNames;
	};

	struct FZibraVDBMetadata
	{
		float CompressionRate{};
	};

	const int32 CurrentSerializationVersion = 0;

	// Should not be changed. To increment version change ZibraVDBFileVersion
	const FGuid ZibraVDBFileGUID(0x3DC15BB1, 0xE4C9409C, 0x9E540952, 0x9E555017);

	class ZIBRAVDBRUNTIME_API FZibraVDBFile
	{
	public:
		FZibraVDBFile();
		explicit FZibraVDBFile(FString fileName);

		bool ReadFromFile(const FString& FilePath);

		bool HasBinaryData() const noexcept;
		const FString& GetFileName() const noexcept;
		float GetZibraVDBSize() const noexcept;
		float GetOriginalOpenVDBSize() const noexcept;
		const TArray64<uint8>& GetBinaryData() const noexcept;
		const FZibraVDBHeader& GetHeader() const noexcept;
		const FZibraVDBMetadata& GetMetadata() const noexcept;

		void Export(const FString& OutputFilePath) const;

	private:
		FString FileName;
		float ZibraVDBSize;
		float OriginalOpenVDBSize;
		FZibraVDBHeader Header{};
		FZibraVDBMetadata Metadata{};
		TArray64<uint8> BinaryData;

		ZIBRAVDBRUNTIME_API friend FArchive& operator<<(FArchive& Ar, FZibraVDBFile& ZibraVDBFile);
		ZIBRAVDBRUNTIME_API friend FArchive& operator<<(FArchive& Ar, FZibraVDBHeader& ZibraVDBHeader);
		ZIBRAVDBRUNTIME_API friend FArchive& operator<<(FArchive& Ar, FZibraVDBMetadata& ZibraVDBMetadata);
	};
}	 // namespace ZibraVDB
