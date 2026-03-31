// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Serialization/StaticMemoryReader.h"

#include <Zibra/Foundation.h>

#if WITH_EDITOR

#include "Serialization/Archive.h"

DECLARE_LOG_CATEGORY_EXTERN(LogZibraVDBStream, Log, All);

class FArchiveWrapper : public Zibra::Legacy::OStream
{
public:
	FArchiveWrapper(const FString& FilePath);
	~FArchiveWrapper();

	void write(const char* s, size_t count) noexcept final;

	[[nodiscard]] bool fail() const noexcept final;

	[[nodiscard]] size_t tellp() noexcept final;

	Zibra::Legacy::OStream& seekp(size_t pos) noexcept final;

private:
	FArchive* Archive = nullptr;
};

#endif

class FMemoryReaderWrapper : public Zibra::Legacy::IStream
{
public:
	FMemoryReaderWrapper(const TArray64<uint8>& InData);

	void read(char* s, size_t count) noexcept final;

	bool fail() const noexcept final;

	bool good() const noexcept final;

	bool bad() const noexcept final;

	bool eof() const noexcept final;

	Zibra::Legacy::IStream& seekg(size_t pos) noexcept final;

	size_t tellg() noexcept final;

	[[nodiscard]] size_t gcount() noexcept final;

private:
	mutable FStaticMemoryReader MemoryReader;
	int64 BytesReadTotal;
	int64 BytesReadLastTime;
};
