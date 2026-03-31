// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "ZibraVDBStreamWrappers.h"

#include <HAL/FileManager.h>

#if WITH_EDITOR

DEFINE_LOG_CATEGORY(LogZibraVDBStream);

FArchiveWrapper::FArchiveWrapper(const FString& FilePath)
{
	Archive = IFileManager::Get().CreateFileWriter(*FilePath);
	if (!Archive)
	{
		UE_LOG(LogZibraVDBStream, Error, TEXT("Failed to open file for writing: %s"), *FilePath);
	}
}

FArchiveWrapper::~FArchiveWrapper()
{
	if (Archive)
	{
		Archive->Close();
		delete Archive;
	}
}

inline void FArchiveWrapper::write(const char* s, size_t count) noexcept
{
	Archive->Serialize(const_cast<char*>(s), count);
}

inline bool FArchiveWrapper::fail() const noexcept
{
	return Archive->IsError();
}

inline size_t FArchiveWrapper::tellp() noexcept
{
	return Archive->Tell();
}

inline Zibra::Legacy::OStream& FArchiveWrapper::seekp(size_t pos) noexcept
{
	Archive->Seek(pos);
	return *this;
}

#endif

FMemoryReaderWrapper::FMemoryReaderWrapper(const TArray64<uint8>& InData)
	: MemoryReader(InData.GetData(), InData.Num()), BytesReadTotal(0), BytesReadLastTime(0)
{
}

inline void FMemoryReaderWrapper::read(char* s, size_t count) noexcept
{
	MemoryReader.Serialize(s, count);
	BytesReadLastTime = count;
	BytesReadTotal += count;
}

inline bool FMemoryReaderWrapper::fail() const noexcept
{
	return MemoryReader.IsError();
}

inline bool FMemoryReaderWrapper::good() const noexcept
{
	return !MemoryReader.IsError();
}

inline bool FMemoryReaderWrapper::bad() const noexcept
{
	return MemoryReader.IsError();
}

bool FMemoryReaderWrapper::eof() const noexcept
{
	return MemoryReader.AtEnd();
}

inline Zibra::Legacy::IStream& FMemoryReaderWrapper::seekg(size_t pos) noexcept
{
	MemoryReader.Seek(pos);
	return *this;
}

inline size_t FMemoryReaderWrapper::tellg() noexcept
{
	return MemoryReader.Tell();
}

[[nodiscard]]
inline size_t FMemoryReaderWrapper::gcount() noexcept
{
	return BytesReadLastTime;
}
