// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66RuntimeUIFontAccess.h"

#include "Fonts/CompositeFont.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"

namespace T66RuntimeUIFontAccess
{
	namespace
	{
		FString ResolveFirstExistingProjectPath(std::initializer_list<const TCHAR*> RelativePaths)
		{
			for (const TCHAR* RelativePath : RelativePaths)
			{
				if (!RelativePath || !*RelativePath)
				{
					continue;
				}

				const FString CandidatePath = FPaths::ProjectDir() / RelativePath;
				if (IFileManager::Get().FileExists(*CandidatePath))
				{
					return CandidatePath;
				}
			}

			return FString();
		}

		FString ResolveEngineSlateFontPath(const TCHAR* RelativePath)
		{
			if (!RelativePath || !*RelativePath)
			{
				return FString();
			}

			const FString CandidatePath = FPaths::EngineContentDir() / TEXT("Slate/Fonts") / RelativePath;
			return IFileManager::Get().FileExists(*CandidatePath) ? CandidatePath : FString();
		}

		TSharedPtr<const FCompositeFont> MakeLocalizedCompositeFont(const FString& DefaultFontPath)
		{
			if (!IFileManager::Get().FileExists(*DefaultFontPath))
			{
				return nullptr;
			}

			TSharedRef<FStandaloneCompositeFont> CompositeFont = MakeShared<FStandaloneCompositeFont>();
			CompositeFont->DefaultTypeface.AppendFont(
				TEXT("Default"),
				DefaultFontPath,
				EFontHinting::Default,
				EFontLoadingPolicy::LazyLoad);

			if (const FString GenericFallbackPath = ResolveEngineSlateFontPath(TEXT("DroidSansFallback.ttf")); !GenericFallbackPath.IsEmpty())
			{
				CompositeFont->FallbackTypeface.Typeface.AppendFont(
					TEXT("Fallback"),
					GenericFallbackPath,
					EFontHinting::Default,
					EFontLoadingPolicy::LazyLoad);
			}

			if (const FString ThaiFallbackPath = ResolveEngineSlateFontPath(TEXT("NotoSansThai-Regular.ttf")); !ThaiFallbackPath.IsEmpty())
			{
				FCompositeSubFont& ThaiSubFont = CompositeFont->SubTypefaces.AddDefaulted_GetRef();
				ThaiSubFont.Typeface.AppendFont(
					TEXT("Thai"),
					ThaiFallbackPath,
					EFontHinting::Default,
					EFontLoadingPolicy::LazyLoad);
				ThaiSubFont.CharacterRanges.Add(FInt32Range(
					FInt32Range::BoundsType::Inclusive(0x0E00),
					FInt32Range::BoundsType::Inclusive(0x0E7F)));
			}

			if (const FString ArabicFallbackPath = ResolveEngineSlateFontPath(TEXT("NotoNaskhArabicUI-Regular.ttf")); !ArabicFallbackPath.IsEmpty())
			{
				FCompositeSubFont& ArabicSubFont = CompositeFont->SubTypefaces.AddDefaulted_GetRef();
				ArabicSubFont.Typeface.AppendFont(
					TEXT("Arabic"),
					ArabicFallbackPath,
					EFontHinting::Default,
					EFontLoadingPolicy::LazyLoad);
				ArabicSubFont.CharacterRanges.Add(FInt32Range(
					FInt32Range::BoundsType::Inclusive(0x0600),
					FInt32Range::BoundsType::Inclusive(0x06FF)));
				ArabicSubFont.CharacterRanges.Add(FInt32Range(
					FInt32Range::BoundsType::Inclusive(0x0750),
					FInt32Range::BoundsType::Inclusive(0x077F)));
				ArabicSubFont.CharacterRanges.Add(FInt32Range(
					FInt32Range::BoundsType::Inclusive(0x08A0),
					FInt32Range::BoundsType::Inclusive(0x08FF)));
				ArabicSubFont.CharacterRanges.Add(FInt32Range(
					FInt32Range::BoundsType::Inclusive(0xFB50),
					FInt32Range::BoundsType::Inclusive(0xFDFF)));
				ArabicSubFont.CharacterRanges.Add(FInt32Range(
					FInt32Range::BoundsType::Inclusive(0xFE70),
					FInt32Range::BoundsType::Inclusive(0xFEFF)));
			}

			return CompositeFont;
		}
	}

	FString ResolveRadianceFontPath()
	{
		return ResolveFirstExistingProjectPath({
			TEXT("RuntimeDependencies/T66/Fonts/radiance.ttf")
		});
	}

	FString ResolveReaverBoldFontPath()
	{
		return ResolveFirstExistingProjectPath({
			TEXT("RuntimeDependencies/T66/Fonts/Reaver-Bold.woff"),
			TEXT("RuntimeDependencies/T66/Fonts/Reaver-Bold.ttf")
		});
	}

	bool IsBoldWeight(const TCHAR* Weight)
	{
		if (Weight == nullptr)
		{
			return false;
		}

		return FCString::Stricmp(Weight, TEXT("Bold")) == 0
			|| FCString::Stricmp(Weight, TEXT("Black")) == 0
			|| FCString::Stricmp(Weight, TEXT("Semibold")) == 0
			|| FCString::Stricmp(Weight, TEXT("SemiBold")) == 0;
	}

	FSlateFontInfo MakeFontFromAbsoluteFile(const FString& Path, int32 Size)
	{
		if (const TSharedPtr<const FCompositeFont> CompositeFont = MakeLocalizedCompositeFont(Path))
		{
			return FSlateFontInfo(CompositeFont, static_cast<float>(Size));
		}

		return FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), Size);
	}

	FSlateFontInfo MakeLocalizedEngineFont(int32 Size, bool bBold)
	{
		const FString DefaultPath = ResolveEngineSlateFontPath(bBold ? TEXT("Roboto-Bold.ttf") : TEXT("Roboto-Regular.ttf"));
		if (const TSharedPtr<const FCompositeFont> CompositeFont = MakeLocalizedCompositeFont(DefaultPath))
		{
			return FSlateFontInfo(CompositeFont, static_cast<float>(Size));
		}

		return FCoreStyle::GetDefaultFontStyle(bBold ? TEXT("Bold") : TEXT("Regular"), Size);
	}
}
