// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66LegacyRuntimeTextureAccess.h"

#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"

DEFINE_LOG_CATEGORY(LogT66RuntimeAssets);

namespace
{
	const TCHAR* SafeDebugLabel(const TCHAR* DebugLabel)
	{
		return (DebugLabel && *DebugLabel) ? DebugLabel : TEXT("<unnamed>");
	}
}

namespace T66LegacyRuntimeTextureAccess
{
	UTexture2D* ImportFileTexture(
		const FString& FilePath,
		TextureFilter Filter,
		TextureGroup LODGroup,
		const bool bClamp,
		const TCHAR* DebugLabel)
	{
		if (!IFileManager::Get().FileExists(*FilePath))
		{
			UE_LOG(
				LogT66RuntimeAssets,
				Log,
				TEXT("RuntimeAssets: loose texture '%s' missing at '%s'"),
				SafeDebugLabel(DebugLabel),
				*FilePath);
			return nullptr;
		}

		UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(FilePath);
		if (!Texture)
		{
			UE_LOG(
				LogT66RuntimeAssets,
				Warning,
				TEXT("RuntimeAssets: failed to import loose texture '%s' from '%s'"),
				SafeDebugLabel(DebugLabel),
				*FilePath);
			return nullptr;
		}

		Texture->SRGB = true;
		Texture->Filter = Filter;
		Texture->LODGroup = LODGroup;
		Texture->NeverStream = true;
		if (bClamp)
		{
			Texture->AddressX = TextureAddress::TA_Clamp;
			Texture->AddressY = TextureAddress::TA_Clamp;
		}
		Texture->UpdateResource();

		UE_LOG(
			LogT66RuntimeAssets,
			Log,
			TEXT("RuntimeAssets: imported loose texture '%s' from '%s'"),
			SafeDebugLabel(DebugLabel),
			*FilePath);
		return Texture;
	}
}
