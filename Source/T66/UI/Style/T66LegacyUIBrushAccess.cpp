// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66LegacyUIBrushAccess.h"

#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "Styling/SlateBrush.h"
#include "UI/Style/T66RuntimeUIHelpers.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY(LogT66RuntimeUI);

namespace
{
	FSlateBrush MakeNineSliceBrush(UTexture2D* Texture, const FMargin& Margin)
	{
		FSlateBrush Brush;
		const bool bUseImageDraw =
			FMath::IsNearlyZero(Margin.Left) &&
			FMath::IsNearlyZero(Margin.Top) &&
			FMath::IsNearlyZero(Margin.Right) &&
			FMath::IsNearlyZero(Margin.Bottom);
		Brush.DrawAs = bUseImageDraw ? ESlateBrushDrawType::Image : ESlateBrushDrawType::Box;
		Brush.Tiling = ESlateBrushTileType::NoTile;
		Brush.SetResourceObject(Texture);
		Brush.TintColor = FSlateColor(FLinearColor::White);
		Brush.ImageSize = FVector2D(1.f, 1.f);
		Brush.Margin = Margin;
		return Brush;
	}

	T66LegacyUIBrushAccess::FOptionalTextureBrush& GetDotaButtonPlateEntry(T66LegacyUIBrushAccess::ET66DotaPlateBrushKind Kind)
	{
		static T66LegacyUIBrushAccess::FOptionalTextureBrush Neutral;
		static T66LegacyUIBrushAccess::FOptionalTextureBrush Primary;
		static T66LegacyUIBrushAccess::FOptionalTextureBrush Danger;

		switch (Kind)
		{
		case T66LegacyUIBrushAccess::ET66DotaPlateBrushKind::Primary:
			return Primary;
		case T66LegacyUIBrushAccess::ET66DotaPlateBrushKind::Danger:
			return Danger;
		case T66LegacyUIBrushAccess::ET66DotaPlateBrushKind::Neutral:
		default:
			return Neutral;
		}
	}

	const TCHAR* GetDotaButtonPlateAssetPath(T66LegacyUIBrushAccess::ET66DotaPlateBrushKind Kind)
	{
		switch (Kind)
		{
		case T66LegacyUIBrushAccess::ET66DotaPlateBrushKind::Primary:
			return TEXT("/Game/UI/Assets/T_UI_DotaPrimaryButtonPlate.T_UI_DotaPrimaryButtonPlate");
		case T66LegacyUIBrushAccess::ET66DotaPlateBrushKind::Danger:
			return TEXT("/Game/UI/Assets/T_UI_DotaDangerButtonPlate.T_UI_DotaDangerButtonPlate");
		case T66LegacyUIBrushAccess::ET66DotaPlateBrushKind::Neutral:
		default:
			return TEXT("/Game/UI/Assets/T_UI_DotaNeutralButtonPlate.T_UI_DotaNeutralButtonPlate");
		}
	}

	FString GetDotaButtonPlateFallbackPath(T66LegacyUIBrushAccess::ET66DotaPlateBrushKind Kind)
	{
		switch (Kind)
		{
		case T66LegacyUIBrushAccess::ET66DotaPlateBrushKind::Primary:
			return T66LegacyUIBrushAccess::GetDotaGeneratedSourceDir() / TEXT("dota_primary_button_plate.png");
		case T66LegacyUIBrushAccess::ET66DotaPlateBrushKind::Danger:
			return T66LegacyUIBrushAccess::GetDotaGeneratedSourceDir() / TEXT("dota_danger_button_plate.png");
		case T66LegacyUIBrushAccess::ET66DotaPlateBrushKind::Neutral:
		default:
			return T66LegacyUIBrushAccess::GetDotaGeneratedSourceDir() / TEXT("dota_neutral_button_plate.png");
		}
	}

	FMargin GetDotaButtonPlateMargin(T66LegacyUIBrushAccess::ET66DotaPlateBrushKind Kind)
	{
		return (Kind == T66LegacyUIBrushAccess::ET66DotaPlateBrushKind::Primary)
			? FMargin(0.f)
			: FMargin(0.22f, 0.32f, 0.22f, 0.32f);
	}
}

namespace T66LegacyUIBrushAccess
{
	UTexture2D* FOptionalTextureBrush::GetTexture() const
	{
		if (ImportedTexture.IsValid())
		{
			return ImportedTexture.Get();
		}

		return FileTexture.IsValid() ? FileTexture.Get() : nullptr;
	}

	const FString& GetDotaGeneratedSourceDir()
	{
		static const FString Dir = FPaths::ProjectContentDir() / TEXT("SourceAssets/UI/Dota/Generated");
		return Dir;
	}

	const FSlateBrush* ResolveDotaButtonPlateBrush(const ET66DotaPlateBrushKind Kind)
	{
		return ResolveOptionalTextureBrush(
			GetDotaButtonPlateEntry(Kind),
			GetDotaButtonPlateAssetPath(Kind),
			GetDotaButtonPlateFallbackPath(Kind),
			GetDotaButtonPlateMargin(Kind),
			TEXT("DotaButtonPlate"));
	}

	UTexture2D* LoadOptionalTexture(
		FOptionalTextureBrush& Entry,
		const TCHAR* ImportedAssetPath,
		const FString& FallbackFilePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel,
		TextureFilter Filter)
	{
		UTexture2D* Texture = Entry.GetTexture();
		if (Entry.Brush.IsValid())
		{
			if (Texture)
			{
				if (Entry.Brush->GetResourceObject() != Texture)
				{
					Entry.Brush->SetResourceObject(Texture);
				}
				return Texture;
			}

			Entry.Brush.Reset();
		}

		if (!Entry.bChecked)
		{
			Entry.bChecked = true;
			if (ImportedAssetPath && *ImportedAssetPath)
			{
				Entry.ImportedTexture.Reset(LoadObject<UTexture2D>(nullptr, ImportedAssetPath));
				if (Entry.ImportedTexture.IsValid())
				{
					Entry.ImportedTexture->Filter = Filter;
					Entry.ImportedTexture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
					Entry.ImportedTexture->NeverStream = true;
					UE_LOG(
						LogT66RuntimeUI,
						Log,
						TEXT("RuntimeUI: optional texture '%s' loaded from cooked asset '%s'"),
						T66RuntimeUIHelpers::SafeDebugLabel(DebugLabel),
						ImportedAssetPath);
				}
			}

			Texture = Entry.GetTexture();
			if (!Texture && !FallbackFilePath.IsEmpty() && IFileManager::Get().FileExists(*FallbackFilePath))
			{
				Texture = FImageUtils::ImportFileAsTexture2D(FallbackFilePath);
				if (Texture)
				{
					Texture->SRGB = true;
					Texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
					Texture->NeverStream = true;
					Texture->Filter = Filter;
					Texture->CompressionSettings = TC_EditorIcon;
					Texture->UpdateResource();
					Entry.FileTexture.Reset(Texture);
					UE_LOG(
						LogT66RuntimeUI,
						Log,
						TEXT("RuntimeUI: optional texture '%s' loaded from loose file '%s'"),
						T66RuntimeUIHelpers::SafeDebugLabel(DebugLabel),
						*FallbackFilePath);
				}
			}

			Texture = Entry.GetTexture();
			if (Texture)
			{
				Entry.Brush = MakeShared<FSlateBrush>(MakeNineSliceBrush(Texture, Margin));
			}
			else
			{
				UE_LOG(
					LogT66RuntimeUI,
					Warning,
					TEXT("RuntimeUI: optional texture '%s' missing cooked asset '%s' and loose file '%s'"),
					T66RuntimeUIHelpers::SafeDebugLabel(DebugLabel),
					ImportedAssetPath ? ImportedAssetPath : TEXT("<null>"),
					FallbackFilePath.IsEmpty() ? TEXT("<null>") : *FallbackFilePath);
			}
		}

		return Entry.GetTexture();
	}

	const FSlateBrush* ResolveOptionalTextureBrush(
		FOptionalTextureBrush& Entry,
		const TCHAR* ImportedAssetPath,
		const FString& FallbackFilePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel,
		TextureFilter Filter)
	{
		LoadOptionalTexture(Entry, ImportedAssetPath, FallbackFilePath, Margin, DebugLabel, Filter);
		return Entry.Brush.IsValid() ? Entry.Brush.Get() : nullptr;
	}
}
