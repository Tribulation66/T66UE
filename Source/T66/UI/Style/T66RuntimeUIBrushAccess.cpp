// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66RuntimeUIBrushAccess.h"

#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "UI/Style/T66RuntimeUIHelpers.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY(LogT66RuntimeUI);

namespace T66ScreenSlateHelpers
{
	T66_API FString MakeReferenceChromeElementAssetPath(const TCHAR* FileName);
	T66_API FString MakeReferenceMainMenuElementAssetPath(const TCHAR* FileName);
}

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

	FString MakeUltrakillElementFallbackPath(const TCHAR* FileName);

	FString NormalizeReferenceFallbackPath(FString Path)
	{
		Path.ReplaceInline(TEXT("\\"), TEXT("/"));
		return Path;
	}

	FString MakeUltrakillElementFallbackPath(const TCHAR* FileName)
	{
		FString ResolvedFileName(FileName ? FileName : TEXT(""));
		if (!ResolvedFileName.Contains(TEXT("square_variant"), ESearchCase::IgnoreCase))
		{
			const FString BaseName = FPaths::GetBaseFilename(ResolvedFileName).ToLower();
			auto ResolveButtonState = [](const FString& Name, const TCHAR* Prefix) -> FString
			{
				FString State = Name;
				State.RemoveFromStart(Prefix, ESearchCase::IgnoreCase);
				return State.IsEmpty() ? FString(TEXT("normal")) : State;
			};

			if (BaseName.StartsWith(TEXT("cta_new_game_button_"), ESearchCase::IgnoreCase)
				|| BaseName.StartsWith(TEXT("cta_load_game_button_"), ESearchCase::IgnoreCase))
			{
				const FString State = BaseName.StartsWith(TEXT("cta_load_game_button_"), ESearchCase::IgnoreCase)
					? ResolveButtonState(BaseName, TEXT("cta_load_game_button_"))
					: ResolveButtonState(BaseName, TEXT("cta_new_game_button_"));
				const FString BaseRedState = State.Equals(TEXT("selected"), ESearchCase::IgnoreCase)
					? FString(TEXT("normal"))
					: State;
				const FString RedState = BaseRedState.Contains(TEXT("red"), ESearchCase::IgnoreCase)
					? BaseRedState
					: FString::Printf(TEXT("%s_red"), *BaseRedState);
				ResolvedFileName = FString::Printf(TEXT("SquareVariant/cta_new_game_button_%s_square_variant.png"), *RedState);
			}
			else if (BaseName.StartsWith(TEXT("leaderboard_tab_button_"), ESearchCase::IgnoreCase))
			{
				const FString State = ResolveButtonState(BaseName, TEXT("leaderboard_tab_button_"));
				const FString ButtonState = State.Contains(TEXT("selected"), ESearchCase::IgnoreCase)
					? FString(TEXT("normal"))
					: State;
				ResolvedFileName = FString::Printf(TEXT("SquareVariant/cta_new_game_button_%s_red_square_variant.png"), *ButtonState);
			}
			else if (BaseName.StartsWith(TEXT("topbar_text_button_"), ESearchCase::IgnoreCase))
			{
				const FString State = ResolveButtonState(BaseName, TEXT("topbar_text_button_"));
				const FString ButtonState = State.Contains(TEXT("selected"), ESearchCase::IgnoreCase)
					? FString(TEXT("normal"))
					: State;
				ResolvedFileName = FString::Printf(TEXT("SquareVariant/cta_new_game_button_%s_red_square_variant.png"), *ButtonState);
			}
			else if (BaseName.StartsWith(TEXT("main_panel_normal"), ESearchCase::IgnoreCase))
			{
				ResolvedFileName = TEXT("SquareVariant/main_panel_normal_square_variant.png");
			}
			else if (BaseName.StartsWith(TEXT("player_row_panel_"), ESearchCase::IgnoreCase))
			{
				const FString State = BaseName.Contains(TEXT("hover")) ? FString(TEXT("hover")) : FString(TEXT("normal"));
				ResolvedFileName = FString::Printf(TEXT("SquareVariant/player_row_panel_%s_square_variant.png"), *State);
			}
			else if (BaseName.StartsWith(TEXT("profile_slot_"), ESearchCase::IgnoreCase))
			{
				const FString State = ResolveButtonState(BaseName, TEXT("profile_slot_"));
				ResolvedFileName = FString::Printf(TEXT("SquareVariant/profile_slot_%s_square_variant.png"), *State);
			}
			else if (BaseName.StartsWith(TEXT("dropdown_field_"), ESearchCase::IgnoreCase))
			{
				const FString State = ResolveButtonState(BaseName, TEXT("dropdown_field_"));
				ResolvedFileName = FString::Printf(TEXT("SquareVariant/cta_new_game_button_%s_red_square_variant.png"), *State);
			}
		}

		const FString SquareVariantPrefix = TEXT("SquareVariant/");
		if (ResolvedFileName.StartsWith(SquareVariantPrefix, ESearchCase::IgnoreCase))
		{
			const FString ChromeFileName = ResolvedFileName.RightChop(SquareVariantPrefix.Len());
			if (ChromeFileName.Contains(TEXT("_red_square_variant"), ESearchCase::IgnoreCase))
			{
				return FPaths::ProjectDir()
					/ FString(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant"))
					/ ChromeFileName;
			}
			return FPaths::ProjectDir() / T66ScreenSlateHelpers::MakeReferenceChromeElementAssetPath(*ChromeFileName);
		}

		return FPaths::ProjectDir()
			/ T66ScreenSlateHelpers::MakeReferenceMainMenuElementAssetPath(*ResolvedFileName);
	}

	FString ResolveReferenceStateFromPath(const FString& NormalizedPath)
	{
		const FString BaseName = FPaths::GetBaseFilename(NormalizedPath).ToLower();
		if (BaseName.Contains(TEXT("disabled")))
		{
			return TEXT("disabled");
		}
		if (BaseName.Contains(TEXT("pressed")))
		{
			return TEXT("pressed");
		}
		if (BaseName.Contains(TEXT("hover")))
		{
			return TEXT("hover");
		}
		if (BaseName.Contains(TEXT("selected")) || BaseName.Contains(TEXT("active")) || BaseName.Contains(TEXT("focused")))
		{
			return TEXT("selected");
		}
		return TEXT("normal");
	}

	FString ResolveReferenceUltrakillFallbackPath(const FString& SourcePath)
	{
		const FString NormalizedPath = NormalizeReferenceFallbackPath(SourcePath);
		if (!NormalizedPath.Contains(TEXT("SourceAssets/UI/Reference/"))
			|| NormalizedPath.Contains(TEXT("/Screens/MainMenu/")))
		{
			return FString();
		}

		const FString State = ResolveReferenceStateFromPath(NormalizedPath);
		if (NormalizedPath.Contains(TEXT("/Buttons/")))
		{
			if (NormalizedPath.Contains(TEXT("cta")) || NormalizedPath.Contains(TEXT("primary")) || NormalizedPath.Contains(TEXT("danger")))
			{
				const FString CtaState = State.Equals(TEXT("selected"), ESearchCase::IgnoreCase) ? FString(TEXT("normal")) : State;
				const TCHAR* CtaStem = NormalizedPath.Contains(TEXT("danger")) ? TEXT("cta_load_game_button") : TEXT("cta_new_game_button");
				return MakeUltrakillElementFallbackPath(*FString::Printf(TEXT("%s_%s.png"), CtaStem, *CtaState));
			}

			return MakeUltrakillElementFallbackPath(*FString::Printf(TEXT("leaderboard_tab_button_%s.png"), *State));
		}

		if (NormalizedPath.Contains(TEXT("/Panels/")))
		{
			if (NormalizedPath.Contains(TEXT("row"))
				|| NormalizedPath.Contains(TEXT("strip"))
				|| NormalizedPath.Contains(TEXT("card"))
				|| NormalizedPath.Contains(TEXT("header"))
				|| NormalizedPath.Contains(TEXT("socket"))
				|| NormalizedPath.Contains(TEXT("tag")))
			{
				return MakeUltrakillElementFallbackPath(TEXT("player_row_panel_normal.png"));
			}
			return MakeUltrakillElementFallbackPath(TEXT("main_panel_normal.png"));
		}

		if (NormalizedPath.Contains(TEXT("/Slots/")))
		{
			FString SlotState = State;
			if (SlotState.Equals(TEXT("pressed"), ESearchCase::IgnoreCase))
			{
				SlotState = TEXT("selected");
			}
			if (SlotState.Equals(TEXT("active"), ESearchCase::IgnoreCase))
			{
				SlotState = TEXT("selected");
			}
			return MakeUltrakillElementFallbackPath(*FString::Printf(TEXT("profile_slot_%s.png"), *SlotState));
		}

		if (NormalizedPath.Contains(TEXT("dropdown_field")))
		{
			const FString DropdownState = State.Equals(TEXT("selected"), ESearchCase::IgnoreCase) ? FString(TEXT("hover")) : State;
			return MakeUltrakillElementFallbackPath(*FString::Printf(TEXT("dropdown_field_%s.png"), *DropdownState));
		}

		if (NormalizedPath.Contains(TEXT("/Progress/")) || NormalizedPath.Contains(TEXT("progress_meter")))
		{
			return MakeUltrakillElementFallbackPath(TEXT("progress_bar_track.png"));
		}

		return FString();
	}

	bool ContainsAnyReferenceFallbackCategory(const FString& NormalizedPath)
	{
		static constexpr const TCHAR* Categories[] =
		{
			TEXT("/Buttons/"),
			TEXT("/Panels/"),
			TEXT("/Slots/"),
			TEXT("/Controls/"),
			TEXT("/Fields/"),
			TEXT("/Progress/"),
			TEXT("/Dividers/"),
			TEXT("/TopBar/")
		};

		for (const TCHAR* Category : Categories)
		{
			if (NormalizedPath.Contains(Category))
			{
				return true;
			}
		}

		return false;
	}

	FLinearColor ResolveSimpleReferenceFallbackColor(const FString& NormalizedPath)
	{
		if (NormalizedPath.Contains(TEXT("/Buttons/")))
		{
			return FLinearColor(0.13f, 0.15f, 0.18f, 1.0f);
		}
		if (NormalizedPath.Contains(TEXT("/Panels/")))
		{
			return FLinearColor(0.055f, 0.060f, 0.072f, 0.96f);
		}
		if (NormalizedPath.Contains(TEXT("/Slots/")))
		{
			return FLinearColor(0.095f, 0.110f, 0.130f, 0.96f);
		}
		if (NormalizedPath.Contains(TEXT("/Controls/")) || NormalizedPath.Contains(TEXT("/Fields/")))
		{
			return FLinearColor(0.105f, 0.115f, 0.135f, 0.92f);
		}
		if (NormalizedPath.Contains(TEXT("/Progress/")))
		{
			return FLinearColor(0.060f, 0.070f, 0.085f, 0.95f);
		}
		if (NormalizedPath.Contains(TEXT("/TopBar/")))
		{
			return FLinearColor(0.125f, 0.135f, 0.155f, 0.96f);
		}
		if (NormalizedPath.Contains(TEXT("/Dividers/")))
		{
			return FLinearColor(0.38f, 0.31f, 0.20f, 0.72f);
		}

		return FLinearColor(0.08f, 0.09f, 0.11f, 0.94f);
	}
}

namespace T66RuntimeUIBrushAccess
{
	UTexture2D* FOptionalTextureBrush::GetTexture() const
	{
		if (ImportedTexture.IsValid())
		{
			return ImportedTexture.Get();
		}

		return FileTexture.IsValid() ? FileTexture.Get() : nullptr;
	}

	bool ShouldUseSimpleReferenceFallback(const FString& SourcePath)
	{
		const FString NormalizedPath = NormalizeReferenceFallbackPath(SourcePath);
		return NormalizedPath.Contains(TEXT("SourceAssets/UI/Reference/"))
			&& !NormalizedPath.Contains(TEXT("/Screens/MainMenu/"))
			&& ContainsAnyReferenceFallbackCategory(NormalizedPath);
	}

	void ConfigureSimpleReferenceFallbackBrush(
		FSlateBrush& Brush,
		const FString& SourcePath,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		const ESlateBrushDrawType::Type DrawAs)
	{
		if (const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush"))
		{
			Brush = *WhiteBrush;
		}
		else
		{
			Brush = FSlateBrush();
		}

		const FString NormalizedPath = NormalizeReferenceFallbackPath(SourcePath);
		Brush.DrawAs = DrawAs;
		Brush.Tiling = ESlateBrushTileType::NoTile;
		Brush.TintColor = FSlateColor(ResolveSimpleReferenceFallbackColor(NormalizedPath));
		Brush.ImageSize = ImageSize.IsNearlyZero() ? FVector2D(1.f, 1.f) : ImageSize;
		Brush.Margin = Margin;
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

			if (Entry.bSimpleReferenceFallback)
			{
				return nullptr;
			}

			Entry.Brush.Reset();
		}

		if (!Entry.bChecked)
		{
			Entry.bChecked = true;
			Entry.bSimpleReferenceFallback = false;
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
			if (!Texture)
			{
				const FString ReferenceFallbackFilePath = ResolveReferenceUltrakillFallbackPath(FallbackFilePath);
				if (!ReferenceFallbackFilePath.IsEmpty()
					&& !ReferenceFallbackFilePath.Equals(FallbackFilePath, ESearchCase::IgnoreCase)
					&& IFileManager::Get().FileExists(*ReferenceFallbackFilePath))
				{
					Texture = FImageUtils::ImportFileAsTexture2D(ReferenceFallbackFilePath);
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
							TEXT("RuntimeUI: optional texture '%s' remapped missing reference chrome '%s' to '%s'"),
							T66RuntimeUIHelpers::SafeDebugLabel(DebugLabel),
							FallbackFilePath.IsEmpty() ? TEXT("<null>") : *FallbackFilePath,
							*ReferenceFallbackFilePath);
					}
				}
			}

			Texture = Entry.GetTexture();
			if (Texture)
			{
				Entry.Brush = MakeShared<FSlateBrush>(MakeNineSliceBrush(Texture, Margin));
			}
			else if (ShouldUseSimpleReferenceFallback(FallbackFilePath))
			{
				Entry.Brush = MakeShared<FSlateBrush>();
				Entry.bSimpleReferenceFallback = true;
				ConfigureSimpleReferenceFallbackBrush(
					*Entry.Brush,
					FallbackFilePath,
					FVector2D(1.f, 1.f),
					Margin,
					ESlateBrushDrawType::Box);
				UE_LOG(
					LogT66RuntimeUI,
					Log,
					TEXT("RuntimeUI: optional texture '%s' missing reference chrome '%s'; using simple box fallback"),
					T66RuntimeUIHelpers::SafeDebugLabel(DebugLabel),
					FallbackFilePath.IsEmpty() ? TEXT("<null>") : *FallbackFilePath);
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
