// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "ZibraVDBRuntime/Public/ZibraVDBVolume4.h"

class FZibraVDBImporterCommon
{
public:
	struct FOpenVDBImportedDetails
	{
		FString InputFilePath;
		FString InputFileMask;
		TArray<FString> AvailableChannels;
		float Quality;
		bool UsePerChannelCompressionSettings;
		TArray<FCompressionSettings> PerChannelCompressionSettings;
	};

	static UObject* ImportCompressedFile(const FString& FilePath, const FString& AssetPath, const FString& DestinationName,
		bool bStripDestinationName = true, const TUniquePtr<FOpenVDBImportedDetails>& OpenVDBImportedDetails = nullptr,
		bool NotifyUser = true);

	static void OpenContentDrawerAndFocusOnAsset(const UObject* Asset);

	static void SetCompressionDetails(
		const TArray<UObject*>& ImportedAssets, const TUniquePtr<FOpenVDBImportedDetails>& OpenVDBImportedDetails);
	static FString StripDestinationName(const FString& DestinationName) noexcept;
	static FString StripFrameNumber(const FString& DestinationName) noexcept;

private:
	static bool IsDestinationAssetExists(const FString& DestinationPath, const FString& DestinationName);
};
