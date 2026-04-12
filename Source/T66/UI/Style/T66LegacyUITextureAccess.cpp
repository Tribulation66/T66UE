// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66LegacyUITextureAccess.h"

#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "TextureResource.h"
#include "UI/Style/T66LegacyUIBrushAccess.h"
#include "UI/Style/T66RuntimeUIHelpers.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	void AddUniqueAbsolutePath(TArray<FString>& Paths, const FString& Path)
	{
		if (Path.IsEmpty())
		{
			return;
		}

		Paths.AddUnique(FPaths::ConvertRelativePathToFull(Path));
	}

	bool IsRuntimeDependencyRelativePath(const FString& RelativePath)
	{
		return RelativePath.StartsWith(TEXT("RuntimeDependencies/"), ESearchCase::CaseSensitive);
	}

	void ConfigureUITexture(UTexture2D* Texture, const TextureFilter Filter, const bool bClamp)
	{
		if (!Texture)
		{
			return;
		}

		Texture->SRGB = true;
		Texture->Filter = Filter;
		Texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
		Texture->CompressionSettings = TC_EditorIcon;
		Texture->NeverStream = true;
		if (bClamp)
		{
			Texture->AddressX = TextureAddress::TA_Clamp;
			Texture->AddressY = TextureAddress::TA_Clamp;
		}
		Texture->UpdateResource();
	}

	void ConfigureCookedUITexture(UTexture2D* Texture, const TCHAR* DebugLabel)
	{
		if (!Texture)
		{
			return;
		}

		// Cooked textures are immutable runtime assets. Rebuilding their resource after
		// overriding NeverStream / compression / mip policy can trip packaged-only
		// assertions for textures imported with authored mip layouts. Only keep the
		// asset alive for Slate usage; do not mutate platform data here.
		Texture->AddToRoot();

		UE_LOG(
			LogT66RuntimeUI,
			Verbose,
			TEXT("RuntimeUI: using cooked texture '%s' without runtime resource mutation"),
			T66RuntimeUIHelpers::SafeDebugLabel(DebugLabel));
	}

	bool DecodeImageFileToBGRA(const FString& FilePath, TArray<FColor>& OutPixels, int32& OutWidth, int32& OutHeight)
	{
		OutPixels.Reset();
		OutWidth = 0;
		OutHeight = 0;

		TArray64<uint8> CompressedBytes;
		if (!FFileHelper::LoadFileToArray(CompressedBytes, *FilePath) || CompressedBytes.Num() == 0)
		{
			return false;
		}

		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
		const EImageFormat Format = ImageWrapperModule.DetectImageFormat(CompressedBytes.GetData(), CompressedBytes.Num());
		if (Format == EImageFormat::Invalid)
		{
			return false;
		}

		const TSharedPtr<IImageWrapper> Wrapper = ImageWrapperModule.CreateImageWrapper(Format);
		if (!Wrapper.IsValid() || !Wrapper->SetCompressed(CompressedBytes.GetData(), CompressedBytes.Num()))
		{
			return false;
		}

		TArray64<uint8> RawBytes;
		if (!Wrapper->GetRaw(ERGBFormat::BGRA, 8, RawBytes))
		{
			return false;
		}

		OutWidth = Wrapper->GetWidth();
		OutHeight = Wrapper->GetHeight();
		if (OutWidth <= 0 || OutHeight <= 0)
		{
			return false;
		}

		const int64 PixelCount = static_cast<int64>(OutWidth) * static_cast<int64>(OutHeight);
		if (RawBytes.Num() != PixelCount * static_cast<int64>(sizeof(FColor)))
		{
			return false;
		}

		OutPixels.SetNumUninitialized(static_cast<int32>(PixelCount));
		FMemory::Memcpy(OutPixels.GetData(), RawBytes.GetData(), RawBytes.Num());
		return true;
	}

	void CopyPixelsIntoMip(FTexture2DMipMap& Mip, const TArray<FColor>& Pixels)
	{
		const int64 ExpectedBytes = static_cast<int64>(Pixels.Num()) * static_cast<int64>(sizeof(FColor));
		void* DestPixels = Mip.BulkData.Lock(LOCK_READ_WRITE);
		DestPixels = Mip.BulkData.Realloc(ExpectedBytes);
		FMemory::Memcpy(DestPixels, Pixels.GetData(), ExpectedBytes);
		Mip.BulkData.Unlock();
	}

	UTexture2D* CreateTextureWithGeneratedMips(const FString& SourcePath, const TArray<FColor>& BasePixels, const int32 Width, const int32 Height, const TextureFilter Filter)
	{
		if (BasePixels.Num() != Width * Height || Width <= 0 || Height <= 0)
		{
			return nullptr;
		}

		const FName TextureName = MakeUniqueObjectName(
			GetTransientPackage(),
			UTexture2D::StaticClass(),
			FName(*FPaths::GetBaseFilename(SourcePath)));

		UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8, TextureName);
		if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
		{
			return nullptr;
		}

		Texture->bNotOfflineProcessed = true;
		Texture->AddressX = TextureAddress::TA_Clamp;
		Texture->AddressY = TextureAddress::TA_Clamp;
		CopyPixelsIntoMip(Texture->GetPlatformData()->Mips[0], BasePixels);

		TArray<FColor> SourcePixels = BasePixels;
		int32 SourceWidth = Width;
		int32 SourceHeight = Height;

		while (SourceWidth > 1 || SourceHeight > 1)
		{
			const int32 NextWidth = FMath::Max(1, SourceWidth / 2);
			const int32 NextHeight = FMath::Max(1, SourceHeight / 2);

			TArray<FColor> NextPixels;
			FImageUtils::ImageResize(
				SourceWidth,
				SourceHeight,
				SourcePixels,
				NextWidth,
				NextHeight,
				NextPixels,
				true,
				false);

			FTexture2DMipMap* NextMip = new FTexture2DMipMap(NextWidth, NextHeight, 1);
			Texture->GetPlatformData()->Mips.Add(NextMip);
			CopyPixelsIntoMip(*NextMip, NextPixels);

			SourcePixels = MoveTemp(NextPixels);
			SourceWidth = NextWidth;
			SourceHeight = NextHeight;
		}

		ConfigureUITexture(Texture, Filter, true);
		return Texture;
	}
}

namespace T66LegacyUITextureAccess
{
	FString MakeProjectDirPath(const FString& RelativePath)
	{
		if (!RelativePath.IsEmpty() && FPaths::IsRelative(RelativePath))
		{
			if (const FString RuntimeDependencyRelativePath = MapLegacySourceRelativePathToRuntimeDependencyRelativePath(RelativePath);
				!RuntimeDependencyRelativePath.IsEmpty())
			{
				return MakeProjectRuntimeDependencyPath(RuntimeDependencyRelativePath);
			}
		}

		return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / RelativePath);
	}

	FString MakeProjectContentPath(const FString& RelativePath)
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / RelativePath);
	}

	FString MakeProjectRuntimeDependencyPath(const FString& RelativePath)
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / RelativePath);
	}

	FString MapLegacySourceRelativePathToRuntimeDependencyRelativePath(const FString& RelativePath)
	{
		const FString NormalizedPath = RelativePath.Replace(TEXT("\\"), TEXT("/"));

		auto RemapPrefix = [&NormalizedPath](const TCHAR* LegacyPrefix, const TCHAR* RuntimePrefix) -> FString
		{
			const FString LegacyPrefixString(LegacyPrefix);
			if (!NormalizedPath.StartsWith(LegacyPrefixString))
			{
				return FString();
			}

			return FString(RuntimePrefix) + NormalizedPath.RightChop(LegacyPrefixString.Len());
		};

		auto RemapExactDirectory = [&NormalizedPath](const TCHAR* LegacyDirectory, const TCHAR* RuntimeDirectory) -> FString
		{
			return NormalizedPath.Equals(LegacyDirectory, ESearchCase::CaseSensitive)
				? FString(RuntimeDirectory)
				: FString();
		};

		if (const FString Remapped = RemapPrefix(TEXT("SourceAssets/UI/MainMenuGenerated/"), TEXT("RuntimeDependencies/T66/UI/MainMenu/")); !Remapped.IsEmpty())
		{
			return Remapped;
		}
		if (const FString Remapped = RemapPrefix(TEXT("SourceAssets/UI/MainMenu/Generated/"), TEXT("RuntimeDependencies/T66/UI/MainMenu/")); !Remapped.IsEmpty())
		{
			return Remapped;
		}
		if (const FString Remapped = RemapPrefix(TEXT("SourceAssets/UI/Currency/"), TEXT("RuntimeDependencies/T66/UI/Currency/")); !Remapped.IsEmpty())
		{
			return Remapped;
		}
		if (const FString Remapped = RemapPrefix(TEXT("SourceAssets/UI/ChestRewards/"), TEXT("RuntimeDependencies/T66/UI/ChestRewards/")); !Remapped.IsEmpty())
		{
			return Remapped;
		}
		if (const FString Remapped = RemapPrefix(TEXT("SourceAssets/UI/PowerUp/Statues/Generated/"), TEXT("RuntimeDependencies/T66/UI/PowerUp/Statues/")); !Remapped.IsEmpty())
		{
			return Remapped;
		}
		if (const FString Remapped = RemapPrefix(
			TEXT("SourceAssets/Shikashi's Fantasy Icons Pack v2/Shikashi's Fantasy Icons Pack v2/"),
			TEXT("RuntimeDependencies/T66/UI/Minimap/")); !Remapped.IsEmpty())
		{
			return Remapped;
		}
		if (const FString Remapped = RemapPrefix(TEXT("SourceAssets/UI/MainMenuBlueTrim/"), TEXT("RuntimeDependencies/T66/UI/MainMenuBlueTrim/")); !Remapped.IsEmpty())
		{
			return Remapped;
		}
		if (const FString Remapped = RemapPrefix(TEXT("SourceAssets/UI/MainMenuBlueWoodFill/"), TEXT("RuntimeDependencies/T66/UI/MainMenuBlueWoodFill/")); !Remapped.IsEmpty())
		{
			return Remapped;
		}
		if (const FString Remapped = RemapPrefix(TEXT("SourceAssets/Archive/UI/MainMenuArcaneFill/"), TEXT("RuntimeDependencies/T66/UI/MainMenuArcaneFill/")); !Remapped.IsEmpty())
		{
			return Remapped;
		}
		if (const FString Remapped = RemapExactDirectory(TEXT("SourceAssets/UI/MainMenuBlueTrim"), TEXT("RuntimeDependencies/T66/UI/MainMenuBlueTrim")); !Remapped.IsEmpty())
		{
			return Remapped;
		}
		if (const FString Remapped = RemapExactDirectory(TEXT("SourceAssets/UI/MainMenuBlueWoodFill"), TEXT("RuntimeDependencies/T66/UI/MainMenuBlueWoodFill")); !Remapped.IsEmpty())
		{
			return Remapped;
		}
		if (const FString Remapped = RemapExactDirectory(TEXT("SourceAssets/Archive/UI/MainMenuArcaneFill"), TEXT("RuntimeDependencies/T66/UI/MainMenuArcaneFill")); !Remapped.IsEmpty())
		{
			return Remapped;
		}

		return FString();
	}

	TArray<FString> BuildLooseTextureCandidatePaths(const FString& RelativePath)
	{
		TArray<FString> CandidatePaths;
		if (RelativePath.IsEmpty())
		{
			return CandidatePaths;
		}

		if (FPaths::IsRelative(RelativePath))
		{
			if (IsRuntimeDependencyRelativePath(RelativePath))
			{
				AddUniqueAbsolutePath(CandidatePaths, MakeProjectRuntimeDependencyPath(RelativePath));
				return CandidatePaths;
			}

			if (const FString RuntimeDependencyRelativePath = MapLegacySourceRelativePathToRuntimeDependencyRelativePath(RelativePath);
				!RuntimeDependencyRelativePath.IsEmpty())
			{
				AddUniqueAbsolutePath(CandidatePaths, MakeProjectRuntimeDependencyPath(RuntimeDependencyRelativePath));
			}

			AddUniqueAbsolutePath(CandidatePaths, FPaths::ProjectContentDir() / RelativePath);
			AddUniqueAbsolutePath(CandidatePaths, MakeProjectDirPath(RelativePath));
			return CandidatePaths;
		}

		AddUniqueAbsolutePath(CandidatePaths, RelativePath);
		return CandidatePaths;
	}

	UTexture2D* LoadAssetTexture(
		const TCHAR* AssetPath,
		const TextureFilter Filter,
		const TCHAR* DebugLabel)
	{
		if (!AssetPath || !*AssetPath)
		{
			return nullptr;
		}

		UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, AssetPath);
		if (!Texture)
		{
			UE_LOG(
				LogT66RuntimeUI,
				Warning,
				TEXT("RuntimeUI: cooked texture '%s' missing at '%s'"),
				T66RuntimeUIHelpers::SafeDebugLabel(DebugLabel),
				AssetPath);
			return nullptr;
		}

		ConfigureCookedUITexture(Texture, DebugLabel);
		UE_LOG(
			LogT66RuntimeUI,
			Verbose,
			TEXT("RuntimeUI: loaded cooked texture '%s' from '%s'"),
			T66RuntimeUIHelpers::SafeDebugLabel(DebugLabel),
			AssetPath);
		return Texture;
	}

	UTexture2D* ImportFileTexture(
		const FString& FilePath,
		TextureFilter Filter,
		const bool bClamp,
		const TCHAR* DebugLabel)
	{
		if (!IFileManager::Get().FileExists(*FilePath))
		{
			UE_LOG(
				LogT66RuntimeUI,
				Verbose,
				TEXT("RuntimeUI: loose texture '%s' missing at '%s'"),
				T66RuntimeUIHelpers::SafeDebugLabel(DebugLabel),
				*FilePath);
			return nullptr;
		}

		UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(FilePath);
		if (!Texture)
		{
			UE_LOG(
				LogT66RuntimeUI,
				Warning,
				TEXT("RuntimeUI: failed to import loose texture '%s' from '%s'"),
				T66RuntimeUIHelpers::SafeDebugLabel(DebugLabel),
				*FilePath);
			return nullptr;
		}

		ConfigureUITexture(Texture, Filter, bClamp);

		UE_LOG(
			LogT66RuntimeUI,
			Verbose,
			TEXT("RuntimeUI: imported loose texture '%s' from '%s'"),
			T66RuntimeUIHelpers::SafeDebugLabel(DebugLabel),
			*FilePath);
		return Texture;
	}

	UTexture2D* ImportFileTextureWithGeneratedMips(
		const FString& FilePath,
		const TextureFilter Filter,
		const TCHAR* DebugLabel)
	{
		if (!IFileManager::Get().FileExists(*FilePath))
		{
			UE_LOG(
				LogT66RuntimeUI,
				Verbose,
				TEXT("RuntimeUI: loose texture '%s' missing at '%s'"),
				T66RuntimeUIHelpers::SafeDebugLabel(DebugLabel),
				*FilePath);
			return nullptr;
		}

		TArray<FColor> DecodedPixels;
		int32 Width = 0;
		int32 Height = 0;
		if (DecodeImageFileToBGRA(FilePath, DecodedPixels, Width, Height))
		{
			if (UTexture2D* Texture = CreateTextureWithGeneratedMips(FilePath, DecodedPixels, Width, Height, Filter))
			{
		UE_LOG(
			LogT66RuntimeUI,
			Verbose,
			TEXT("RuntimeUI: imported loose texture '%s' with generated mips from '%s'"),
			T66RuntimeUIHelpers::SafeDebugLabel(DebugLabel),
			*FilePath);
				return Texture;
			}
		}

		return ImportFileTexture(FilePath, Filter, true, DebugLabel);
	}
}
