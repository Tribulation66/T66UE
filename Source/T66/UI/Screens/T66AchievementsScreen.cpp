// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66AchievementsScreen.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66DemoModeUIUtils.h"
#include "UI/T66UIManager.h"
#include "Styling/CoreStyle.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr int32 T66AchievementsFontDelta = 0;
	constexpr int32 T66SecretPlaceholderRowCount = 10;
	constexpr const TCHAR* T66SteamAchievementsLogoPath = TEXT("RuntimeDependencies/T66/UI/Icons/Flat/steam_logo_imagegen.png");
	TMap<FString, TStrongObjectPtr<UTexture2D>> GAchievementsGeneratedTextureCache;
	TMap<FString, TSharedPtr<FSlateBrush>> GAchievementsGeneratedBrushCache;
	TMap<FString, TSharedPtr<FButtonStyle>> GAchievementsGeneratedButtonStyleCache;

	struct FT66AchievementReferenceRowLayout
	{
		float RowY;
		float TextY;
		float ButtonY;
	};

	constexpr FT66AchievementReferenceRowLayout T66AchievementReferenceRows[] = {
		{ 0.497f, 0.509f, 0.498f },
		{ 0.610f, 0.622f, 0.610f },
		{ 0.725f, 0.736f, 0.725f },
		{ 0.838f, 0.850f, 0.838f }
	};

	constexpr float T66AchievementReferenceDividerY[] = {
		0.579f,
		0.693f,
		0.808f
	};

	constexpr float T66AchievementReferenceListPanelX = 0.024f;
	constexpr float T66AchievementReferenceListPanelY = 0.463f;
	constexpr float T66AchievementReferenceListPanelW = 0.951f;
	constexpr float T66AchievementReferenceListPanelH = 0.467f;
	constexpr float T66AchievementReferenceRowX = 0.035f;
	constexpr float T66AchievementReferenceRowW = 0.928f;
	constexpr float T66AchievementReferenceRowH = 0.082f;
	constexpr float T66AchievementReferenceNumberX = 0.059f;
	constexpr float T66AchievementReferenceNumberW = 0.024f;
	constexpr float T66AchievementReferenceNameX = 0.123f;
	constexpr float T66AchievementReferenceNameW = 0.147f;
	constexpr float T66AchievementReferenceDescriptionX = 0.308f;
	constexpr float T66AchievementReferenceDescriptionW = 0.260f;
	constexpr float T66AchievementReferenceProgressX = 0.639f;
	constexpr float T66AchievementReferenceProgressW = 0.036f;
	constexpr float T66AchievementReferenceRewardValueX = 0.722f;
	constexpr float T66AchievementReferenceRewardValueW = 0.014f;
	constexpr float T66AchievementReferenceRewardIconX = 0.745f;
	constexpr float T66AchievementReferenceRewardIconW = 0.027f;
	constexpr float T66AchievementReferenceClaimButtonX = 0.804f;
	constexpr float T66AchievementReferenceClaimButtonW = 0.081f;
	constexpr float T66AchievementReferenceFavoriteButtonX = 0.914f;
	constexpr float T66AchievementReferenceFavoriteButtonW = 0.029f;
	constexpr float T66AchievementReferenceTextH = 0.045f;
	constexpr float T66AchievementReferenceNumberH = 0.040f;
	constexpr float T66AchievementReferenceRewardIconH = 0.040f;
	constexpr float T66AchievementReferenceButtonH = 0.056f;
	constexpr int32 T66AchievementReferenceRowFontSize = 28;

	float GetAchievementReferenceSlotRowY(const int32 RowIndex)
	{
		if (RowIndex <= 0)
		{
			return T66AchievementReferenceListPanelY;
		}

		const int32 LastDividerIndex = static_cast<int32>(UE_ARRAY_COUNT(T66AchievementReferenceDividerY)) - 1;
		const int32 DividerIndex = FMath::Min(RowIndex - 1, LastDividerIndex);
		return T66AchievementReferenceDividerY[DividerIndex];
	}

	float GetAchievementReferenceSlotRowH(const int32 RowIndex)
	{
		const float RowY = GetAchievementReferenceSlotRowY(RowIndex);
		const float RowBottom = RowIndex < UE_ARRAY_COUNT(T66AchievementReferenceDividerY)
			? T66AchievementReferenceDividerY[RowIndex]
			: T66AchievementReferenceListPanelY + T66AchievementReferenceListPanelH;
		return FMath::Max(0.f, RowBottom - RowY);
	}

	FString MakeAchievementsUltrakillElementPath(const TCHAR* FileName)
	{
		return FT66FlatStyle::GetFlatMainMenuElementAssetPath(FileName);
	}

	FString MakeAchievementsUltrakillSquareElementPath(const TCHAR* FileName)
	{
		return FT66FlatStyle::GetFlatChromeElementAssetPath(FileName);
	}

	FString MakeAchievementsAssetPath(const TCHAR* RelativeAssetPath)
	{
		const FString Path(RelativeAssetPath ? RelativeAssetPath : TEXT(""));
		if (Path.Contains(TEXT("progress_fill")))
		{
			return MakeAchievementsUltrakillElementPath(TEXT("progress_bar_fill_red.png"));
		}
		if (Path.Contains(TEXT("progress_track")) || Path.Contains(TEXT("progress_meter")))
		{
			return MakeAchievementsUltrakillElementPath(TEXT("progress_bar_track.png"));
		}
		if (Path.Contains(TEXT("scrollbar_thumb")))
		{
			return MakeAchievementsUltrakillElementPath(TEXT("progress_bar_fill_red.png"));
		}
		if (Path.Contains(TEXT("scrollbar_track")))
		{
			return MakeAchievementsUltrakillElementPath(TEXT("progress_bar_track.png"));
		}
		if (Path.Contains(TEXT("dropdown_field")))
		{
			return MakeAchievementsUltrakillSquareElementPath(TEXT("dropdown_field_normal_square_variant.png"));
		}
		if (Path.Contains(TEXT("/Slots/")) || Path.Contains(TEXT("slot_frame")))
		{
			return MakeAchievementsUltrakillSquareElementPath(TEXT("profile_slot_normal_square_variant.png"));
		}
		if (Path.Contains(TEXT("row_shell")))
		{
			return FT66FlatStyle::GetFlatLongPanelAssetPath(TEXT("normal"));
		}
		if (Path.Contains(TEXT("ScreenArt/")))
		{
			return FString();
		}
		if (Path.Contains(TEXT("progress_panel")))
		{
			return FT66FlatStyle::GetFlatLongPanelAssetPath(TEXT("normal"));
		}
		if (Path.Contains(TEXT("fullscreen_panel")) || Path.Contains(TEXT("inner_panel")))
		{
			return MakeAchievementsUltrakillSquareElementPath(TEXT("main_panel_normal_square_variant.png"));
		}

		return MakeAchievementsUltrakillSquareElementPath(TEXT("main_panel_normal_square_variant.png"));
	}

	FString MakeAchievementsButtonAssetPath(const TCHAR* Family, const TCHAR* State)
	{
		const FString FamilyName(Family ? Family : TEXT("Pill"));
		const FString StateName(State ? State : TEXT("normal"));
		if (FamilyName.Equals(TEXT("SquareIcon"), ESearchCase::IgnoreCase))
		{
			const FString IconState = StateName.Contains(TEXT("selected"), ESearchCase::IgnoreCase) ? FString(TEXT("selected")) : StateName.ToLower();
			return MakeAchievementsUltrakillSquareElementPath(*FString::Printf(TEXT("topbar_icon_button_%s_square_variant.png"), *IconState));
		}
		if (FamilyName.Equals(TEXT("CTA"), ESearchCase::IgnoreCase))
		{
			return FT66FlatStyle::GetFlatRedSquareButtonAssetPath(*StateName);
		}
		return FT66FlatStyle::GetFlatRedSquareButtonAssetPath(*StateName);
	}

	void SetAchievementsActiveStateFolder(const bool bShowingSecret)
	{
		(void)bShowingSecret;
	}

	bool T66IsPausedGameplayWidget(const UUserWidget* Widget)
	{
		const APlayerController* PC = Widget ? Widget->GetOwningPlayer() : nullptr;
		return PC && PC->IsPaused();
	}

	FLinearColor T66AchievementsShellFill()
	{
		return FLinearColor(0.004f, 0.005f, 0.010f, 0.985f);
	}

	FLinearColor T66AchievementsInsetFill()
	{
		return FLinearColor(0.046f, 0.018f, 0.020f, 0.98f);
	}

	FLinearColor T66AchievementsRowFill()
	{
		return FLinearColor(0.018f, 0.020f, 0.030f, 0.98f);
	}

	FLinearColor T66AchievementsUnlockedRowFill()
	{
		return FLinearColor(0.048f, 0.020f, 0.022f, 0.98f);
	}

	FLinearColor T66AchievementsTabActiveFill()
	{
		return FLinearColor(0.92f, 0.05f, 0.12f, 1.0f);
	}

	FLinearColor T66AchievementsTabInactiveFill()
	{
		return FLinearColor(0.10f, 0.11f, 0.15f, 1.0f);
	}

	FLinearColor T66AchievementsTabActiveText()
	{
		return FLinearColor(1.0f, 0.96f, 0.88f, 1.0f);
	}

	FLinearColor T66AchievementsTabInactiveText()
	{
		return FLinearColor(0.96f, 0.78f, 0.74f, 1.0f);
	}

	FLinearColor T66AchievementsParchmentText()
	{
		return FLinearColor(0.99f, 0.94f, 0.92f, 1.0f);
	}

	FLinearColor T66AchievementsParchmentMutedText()
	{
		return FLinearColor(0.84f, 0.62f, 0.58f, 1.0f);
	}

	FLinearColor T66AchievementsSectionText()
	{
		return FLinearColor(1.0f, 0.88f, 0.84f, 1.0f);
	}

	float T66AchievementProgress01(const FAchievementData& Achievement)
	{
		return Achievement.RequirementCount > 0
			? static_cast<float>(FMath::Clamp(Achievement.CurrentProgress, 0, Achievement.RequirementCount))
				/ static_cast<float>(Achievement.RequirementCount)
			: (Achievement.bIsUnlocked ? 1.0f : 0.0f);
	}

	int32 T66AchievementRemainingCount(const FAchievementData& Achievement)
	{
		return FMath::Max(0, Achievement.RequirementCount - FMath::Clamp(Achievement.CurrentProgress, 0, Achievement.RequirementCount));
	}

	int32 T66AchievementOrderKey(const FName AchievementID)
	{
		int32 NumericPart = 0;
		bool bHasDigit = false;
		for (const TCHAR Character : AchievementID.ToString())
		{
			if (FChar::IsDigit(Character))
			{
				NumericPart = (NumericPart * 10) + (Character - TEXT('0'));
				bHasDigit = true;
			}
		}

		return bHasDigit ? NumericPart : MAX_int32;
	}

	int32 AdjustAchievementsFontSize(int32 BaseSize)
	{
		return FMath::Max(8, BaseSize + T66AchievementsFontDelta);
	}

	FSlateFontInfo AchievementsBoldFont(int32 BaseSize)
	{
		return FT66FlatStyle::Tokens::FontBold(AdjustAchievementsFontSize(BaseSize));
	}

	FSlateFontInfo AchievementsRegularFont(int32 BaseSize)
	{
		return FT66FlatStyle::Tokens::FontRegular(AdjustAchievementsFontSize(BaseSize));
	}

	FString MakeSettingsAssetPath(const TCHAR* FileName)
	{
		const FString Name(FileName);
		const auto SelectButtonPath = [](const TCHAR* State) -> FString
		{
			return MakeAchievementsButtonAssetPath(TEXT("Pill"), State);
		};

		if (Name.StartsWith(TEXT("settings_toggle_on_")))
		{
			return SelectButtonPath(TEXT("selected"));
		}
		if (Name.StartsWith(TEXT("settings_compact_neutral_")) || Name.StartsWith(TEXT("settings_toggle_off_")))
		{
			if (Name.StartsWith(TEXT("settings_compact_neutral_")))
			{
				if (Name.Contains(TEXT("_hover"))) return MakeAchievementsButtonAssetPath(TEXT("CTA"), TEXT("hover"));
				if (Name.Contains(TEXT("_pressed"))) return MakeAchievementsButtonAssetPath(TEXT("CTA"), TEXT("pressed"));
				return MakeAchievementsButtonAssetPath(TEXT("CTA"), TEXT("normal"));
			}
			if (Name.Contains(TEXT("_hover"))) return SelectButtonPath(TEXT("hover"));
			if (Name.Contains(TEXT("_pressed"))) return SelectButtonPath(TEXT("pressed"));
			return SelectButtonPath(TEXT("normal"));
		}
		if (Name.StartsWith(TEXT("settings_toggle_inactive_")))
		{
			return MakeAchievementsButtonAssetPath(TEXT("CTA"), TEXT("disabled"));
		}
		if (Name == TEXT("settings_content_shell_frame.png"))
		{
			return MakeAchievementsAssetPath(TEXT("Panels/achievements_panels_reference_progress_panel_v2.png"));
		}
		if (Name == TEXT("settings_row_shell_full.png"))
		{
			return MakeAchievementsAssetPath(TEXT("Panels/achievements_panels_reference_row_shell_v2.png"));
		}
		if (Name == TEXT("settings_row_shell_split.png"))
		{
			return MakeAchievementsAssetPath(TEXT("Panels/achievements_panels_reference_progress_panel_v2.png"));
		}
		if (Name == TEXT("settings_dropdown_field.png"))
		{
			return MakeAchievementsAssetPath(TEXT("Controls/achievements_controls_reference_dropdown_field_normal.png"));
		}

		return MakeAchievementsUltrakillSquareElementPath(TEXT("main_panel_normal_square_variant.png"));
	}

	FMargin GetAchievementsGeneratedBrushMargin(const FString& SourceRelativePath)
	{
		if (SourceRelativePath.Contains(TEXT("inner_panel_normal.png")))
		{
			return FMargin(0.067f, 0.043f, 0.067f, 0.043f);
		}
		if (SourceRelativePath.Contains(TEXT("fullscreen_panel_wide.png")))
		{
			return FMargin(0.060f, 0.090f, 0.060f, 0.105f);
		}
		if (SourceRelativePath.Contains(TEXT("row_shell_quiet.png")))
		{
			return FMargin(0.070f, 0.155f, 0.070f, 0.155f);
		}
		if (SourceRelativePath.Contains(TEXT("reference_progress_panel_v2.png")))
		{
			return FMargin(0.070f, 0.185f, 0.070f, 0.185f);
		}
		if (SourceRelativePath.Contains(TEXT("reference_row_shell_v2.png")))
		{
			return FMargin(0.075f, 0.245f, 0.075f, 0.245f);
		}
		if (SourceRelativePath.Contains(TEXT("main_panel_normal.png")) || SourceRelativePath.Contains(TEXT("main_panel_normal_square_variant.png")))
		{
			return FMargin(0.052f, 0.095f, 0.052f, 0.095f);
		}
		if (SourceRelativePath.Contains(TEXT("player_row_panel_normal.png")) || SourceRelativePath.Contains(TEXT("player_row_panel_hover.png"))
			|| SourceRelativePath.Contains(TEXT("player_row_panel_normal_square_variant.png")) || SourceRelativePath.Contains(TEXT("player_row_panel_hover_square_variant.png")))
		{
			return FMargin(0.070f, 0.245f, 0.070f, 0.245f);
		}
		if (SourceRelativePath.Contains(TEXT("leaderboard_row_normal.png")) || SourceRelativePath.Contains(TEXT("leaderboard_row_hover.png")))
		{
			return FMargin(0.085f, 0.210f, 0.085f, 0.210f);
		}
		if (SourceRelativePath.Contains(TEXT("progress_bar_track.png")) || SourceRelativePath.Contains(TEXT("progress_bar_fill_red.png")))
		{
			return FMargin(0.055f, 0.34f, 0.055f, 0.34f);
		}
		if (SourceRelativePath.Contains(TEXT("search_bar_normal.png")))
		{
			return FMargin(0.060f, 0.340f, 0.060f, 0.340f);
		}
		if (SourceRelativePath.Contains(TEXT("progress_track_v2.png")) || SourceRelativePath.Contains(TEXT("progress_fill_v2.png")))
		{
			return FMargin(0.055f, 0.34f, 0.055f, 0.34f);
		}
		if (SourceRelativePath.Contains(TEXT("scrollbar_track_v2.png")) || SourceRelativePath.Contains(TEXT("scrollbar_thumb_v2.png")))
		{
			return FMargin(0.42f, 0.085f, 0.42f, 0.085f);
		}
		if (SourceRelativePath.Contains(TEXT("dropdown_field_normal.png")) || SourceRelativePath.Contains(TEXT("dropdown_field_normal_square_variant.png")))
		{
			return FMargin(0.06f, 0.34f, 0.06f, 0.34f);
		}
		if (FT66FlatStyle::IsFlatChromePillButtonAssetPath(SourceRelativePath))
		{
			return FMargin(0.093f, 0.213f, 0.093f, 0.213f);
		}
		if (FT66FlatStyle::IsFlatChromeCTAButtonAssetPath(SourceRelativePath))
		{
			return FMargin(0.083f, 0.231f, 0.083f, 0.231f);
		}
		return FMargin(0.f);
	}

	bool IsZeroAchievementsMargin(const FMargin& Margin)
	{
		return FMath::IsNearlyZero(Margin.Left)
			&& FMath::IsNearlyZero(Margin.Top)
			&& FMath::IsNearlyZero(Margin.Right)
			&& FMath::IsNearlyZero(Margin.Bottom);
	}

	bool IsAchievementsSlicedButtonPath(const FString& SourceRelativePath)
	{
		return FT66FlatStyle::IsFlatChromeButtonAssetPath(SourceRelativePath)
			|| SourceRelativePath.Contains(TEXT("/Buttons/Pill/"))
			|| SourceRelativePath.Contains(TEXT("leaderboard_tab_button_"))
			|| SourceRelativePath.Contains(TEXT("cta_new_game_button_"))
			|| SourceRelativePath.Contains(TEXT("topbar_icon_button_"));
	}

	UTexture2D* LoadAchievementsGeneratedTexture(const FString& SourceRelativePath)
	{
		if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GAchievementsGeneratedTextureCache.Find(SourceRelativePath))
		{
			return CachedTexture->Get();
		}

		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(SourceRelativePath))
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			const TextureFilter Filter = IsAchievementsSlicedButtonPath(SourceRelativePath)
				? TextureFilter::TF_Nearest
				: TextureFilter::TF_Trilinear;

			UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
				CandidatePath,
				Filter,
				true,
				TEXT("AchievementsGeneratedUI"));
			if (!Texture)
			{
				Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
					CandidatePath,
					Filter,
					TEXT("AchievementsGeneratedUI"));
			}

			if (Texture)
			{
				GAchievementsGeneratedTextureCache.Add(SourceRelativePath, TStrongObjectPtr<UTexture2D>(Texture));
				return Texture;
			}
		}

		return nullptr;
	}

	const FSlateBrush* ResolveAchievementsGeneratedBrush(const FString& SourceRelativePath, const FVector2D& ImageSize = FVector2D::ZeroVector)
	{
		const FString BrushKey = FString::Printf(TEXT("%s::%.0fx%.0f"), *SourceRelativePath, ImageSize.X, ImageSize.Y);
		if (const TSharedPtr<FSlateBrush>* CachedBrush = GAchievementsGeneratedBrushCache.Find(BrushKey))
		{
			return CachedBrush->Get();
		}

		UTexture2D* Texture = LoadAchievementsGeneratedTexture(SourceRelativePath);
		if (!Texture)
		{
			if (!T66RuntimeUIBrushAccess::ShouldUseSimpleReferenceFallback(SourceRelativePath))
			{
				return nullptr;
			}

			TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
			const FMargin BrushMargin = GetAchievementsGeneratedBrushMargin(SourceRelativePath);
			const bool bSlicedButton = IsAchievementsSlicedButtonPath(SourceRelativePath);
			const FVector2D ResolvedSize = ImageSize.X > 0.f && ImageSize.Y > 0.f ? ImageSize : FVector2D(1.f, 1.f);
			T66RuntimeUIBrushAccess::ConfigureSimpleReferenceFallbackBrush(
				*Brush,
				SourceRelativePath,
				ResolvedSize,
				bSlicedButton ? FMargin(0.f) : BrushMargin,
				bSlicedButton || IsZeroAchievementsMargin(BrushMargin) ? ESlateBrushDrawType::Image : ESlateBrushDrawType::Box);
			GAchievementsGeneratedBrushCache.Add(BrushKey, Brush);
			return Brush.Get();
		}

		const FVector2D ResolvedSize = ImageSize.X > 0.f && ImageSize.Y > 0.f
			? ImageSize
			: FVector2D(static_cast<float>(Texture->GetSizeX()), static_cast<float>(Texture->GetSizeY()));

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		const FMargin BrushMargin = GetAchievementsGeneratedBrushMargin(SourceRelativePath);
		const bool bSlicedButton = IsAchievementsSlicedButtonPath(SourceRelativePath);
		Brush->DrawAs = bSlicedButton || IsZeroAchievementsMargin(BrushMargin) ? ESlateBrushDrawType::Image : ESlateBrushDrawType::Box;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ResolvedSize;
		Brush->Margin = bSlicedButton ? FMargin(0.f) : BrushMargin;
		Brush->TintColor = FSlateColor(FLinearColor::White);
		Brush->SetResourceObject(Texture);

		GAchievementsGeneratedBrushCache.Add(BrushKey, Brush);
		return Brush.Get();
	}

	const FSlateBrush* ResolveAchievementsGeneratedRegionBrush(
		const FString& Key,
		const FString& SourceRelativePath,
		const FBox2f& UVRegion,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		const ESlateBrushDrawType::Type DrawAs,
		const FLinearColor& Tint)
	{
		const FString BrushKey = FString::Printf(TEXT("Region:%s::%s"), *Key, *SourceRelativePath);
		if (const TSharedPtr<FSlateBrush>* CachedBrush = GAchievementsGeneratedBrushCache.Find(BrushKey))
		{
			return CachedBrush->Get();
		}

		UTexture2D* Texture = LoadAchievementsGeneratedTexture(SourceRelativePath);
		if (!Texture)
		{
			if (!T66RuntimeUIBrushAccess::ShouldUseSimpleReferenceFallback(SourceRelativePath))
			{
				return nullptr;
			}

			TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
			T66RuntimeUIBrushAccess::ConfigureSimpleReferenceFallbackBrush(
				*Brush,
				SourceRelativePath,
				ImageSize,
				Margin,
				DrawAs);
			GAchievementsGeneratedBrushCache.Add(BrushKey, Brush);
			return Brush.Get();
		}

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = DrawAs;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
		Brush->Margin = Margin;
		Brush->TintColor = FSlateColor(Tint);
		Brush->SetResourceObject(Texture);
		Brush->SetUVRegion(UVRegion);

		GAchievementsGeneratedBrushCache.Add(BrushKey, Brush);
		return Brush.Get();
	}

	const FScrollBarStyle* GetAchievementsReferenceScrollBarStyle()
	{
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");

		const FSlateBrush* TrackBrush = ResolveAchievementsGeneratedBrush(
			MakeAchievementsAssetPath(TEXT("Controls/achievements_controls_scrollbar_track_v2.png")),
			FVector2D(14.f, 618.f));
		const FSlateBrush* ThumbBrush = ResolveAchievementsGeneratedBrush(
			MakeAchievementsAssetPath(TEXT("Controls/achievements_controls_scrollbar_thumb_v2.png")),
			FVector2D(14.f, 122.f));
		const FSlateBrush* HoverBrush = ResolveAchievementsGeneratedBrush(
			MakeAchievementsAssetPath(TEXT("Controls/achievements_controls_scrollbar_thumb_v2.png")),
			FVector2D(14.f, 122.f));

		if (TrackBrush && ThumbBrush && HoverBrush)
		{
			Style
				.SetVerticalBackgroundImage(*TrackBrush)
				.SetVerticalTopSlotImage(*TrackBrush)
				.SetVerticalBottomSlotImage(*TrackBrush)
				.SetNormalThumbImage(*ThumbBrush)
				.SetHoveredThumbImage(*HoverBrush)
				.SetDraggedThumbImage(*HoverBrush)
				.SetThickness(18.f);
		}

		return &Style;
	}

	const FButtonStyle* ResolveAchievementsGeneratedButtonStyle(
		const FString& Key,
		const FString& NormalPath,
		const FString& HoverPath,
		const FString& PressedPath,
		const FString& DisabledPath)
	{
		if (const TSharedPtr<FButtonStyle>* CachedStyle = GAchievementsGeneratedButtonStyleCache.Find(Key))
		{
			return CachedStyle->Get();
		}

		const FButtonStyle& NoBorderStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
		TSharedPtr<FButtonStyle> Style = MakeShared<FButtonStyle>(NoBorderStyle);
		if (const FSlateBrush* NormalBrush = ResolveAchievementsGeneratedBrush(NormalPath))
		{
			Style->SetNormal(*NormalBrush);
		}
		if (const FSlateBrush* HoverBrush = ResolveAchievementsGeneratedBrush(HoverPath))
		{
			Style->SetHovered(*HoverBrush);
		}
		if (const FSlateBrush* PressedBrush = ResolveAchievementsGeneratedBrush(PressedPath))
		{
			Style->SetPressed(*PressedBrush);
		}
		if (const FSlateBrush* DisabledBrush = ResolveAchievementsGeneratedBrush(DisabledPath))
		{
			Style->SetDisabled(*DisabledBrush);
		}
		Style->SetNormalPadding(FMargin(0.f));
		Style->SetPressedPadding(FMargin(0.f));

		GAchievementsGeneratedButtonStyleCache.Add(Key, Style);
		return Style.Get();
	}

	const FButtonStyle* ResolveAchievementsCompactButtonStyle()
	{
		return ResolveAchievementsGeneratedButtonStyle(
			TEXT("Achievements.CompactButton"),
			MakeSettingsAssetPath(TEXT("settings_compact_neutral_normal.png")),
			MakeSettingsAssetPath(TEXT("settings_compact_neutral_hover.png")),
			MakeSettingsAssetPath(TEXT("settings_compact_neutral_pressed.png")),
			MakeSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")));
	}

	FString MakeAchievementsDuoButtonPath(const bool bLeft, const TCHAR* State)
	{
		(void)bLeft;
		return MakeAchievementsButtonAssetPath(TEXT("Pill"), State);
	}

	const FButtonStyle* ResolveAchievementsToggleButtonStyle(const bool bActive, const bool bLeft)
	{
		return bActive
			? ResolveAchievementsGeneratedButtonStyle(
				bLeft ? TEXT("Achievements.ToggleLeftOn") : TEXT("Achievements.ToggleRightOn"),
				MakeAchievementsDuoButtonPath(bLeft, TEXT("selected")),
				MakeAchievementsDuoButtonPath(bLeft, TEXT("selected")),
				MakeAchievementsDuoButtonPath(bLeft, TEXT("pressed")),
				MakeSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")))
			: ResolveAchievementsGeneratedButtonStyle(
				bLeft ? TEXT("Achievements.ToggleLeftOff") : TEXT("Achievements.ToggleRightOff"),
				MakeAchievementsDuoButtonPath(bLeft, TEXT("normal")),
				MakeAchievementsDuoButtonPath(bLeft, TEXT("hover")),
				MakeAchievementsDuoButtonPath(bLeft, TEXT("pressed")),
				MakeSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")));
	}

	TSharedRef<SWidget> MakeAchievementsGeneratedPanel(
		const FString& SourceRelativePath,
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const FLinearColor& Tint = FLinearColor::White,
		const FLinearColor& FallbackFill = T66AchievementsInsetFill())
	{
		if (const FSlateBrush* Brush = ResolveAchievementsGeneratedBrush(SourceRelativePath))
		{
			return SNew(SBorder)
				.BorderImage(Brush)
				.BorderBackgroundColor(Tint)
				.Padding(Padding)
				[
					Content
				];
		}

		return FT66FlatStyle::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(FallbackFill)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeAchievementsGeneratedButton(
		const FT66ButtonParams& Params,
		const FButtonStyle* ButtonStyle,
		const FSlateFontInfo& Font,
		const FLinearColor& TextColor,
		const FMargin& ContentPadding)
	{
		if (!ButtonStyle)
		{
			return FT66FlatStyle::MakeBareButton(
				FT66BareButtonParams(
					Params.OnClicked,
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					.StretchDirection(EStretchDirection::DownOnly)
					[
						SNew(STextBlock)
						.Text(Params.Label)
						.Font(Font)
						.ColorAndOpacity(TextColor)
						.Justification(ETextJustify::Center)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					])
				.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
				.SetPadding(ContentPadding)
				.SetHAlign(HAlign_Center)
				.SetVAlign(VAlign_Center)
				.SetEnabled(Params.IsEnabled)
				.SetMinWidth(Params.MinWidth)
				.SetHeight(Params.Height)
				.SetVisibility(Params.Visibility));
		}

		return FT66FlatStyle::BuildFlatSlicedPlateButton(
			Params.OnClicked,
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::DownOnly)
			[
				SNew(STextBlock)
				.Text(Params.Label)
				.Font(Font)
				.ColorAndOpacity(TextColor)
				.Justification(ETextJustify::Center)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			],
			&ButtonStyle->Normal,
			&ButtonStyle->Hovered,
			&ButtonStyle->Pressed,
			&ButtonStyle->Disabled,
			Params.MinWidth,
			Params.Height,
			ContentPadding,
			Params.IsEnabled,
			Params.Visibility);
	}

	TSharedRef<SWidget> MakeAchievementsProgressBarSized(const float Percent, const float Width, const float Height)
	{
		const float Pct = FMath::Clamp(Percent, 0.f, 1.f);
		const float BarHeight = FMath::Max(16.f, Height + 5.f);
		const FSlateBrush* TrackBrush = ResolveAchievementsGeneratedBrush(
			MakeAchievementsAssetPath(TEXT("Controls/achievements_controls_progress_track_v2.png")),
			FVector2D(Width, BarHeight));
		const FSlateBrush* FillBrush = ResolveAchievementsGeneratedBrush(
			MakeAchievementsAssetPath(TEXT("Controls/achievements_controls_progress_fill_v2.png")),
			FVector2D(Width, BarHeight));

		if (TrackBrush && FillBrush)
		{
			return SNew(SBox)
				.WidthOverride(Width)
				.HeightOverride(BarHeight)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBorder)
						.BorderImage(TrackBrush)
						.BorderBackgroundColor(FLinearColor::White)
						.Padding(FMargin(0.f))
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					[
						SNew(SBox)
						.WidthOverride(FMath::Max(0.f, Width * Pct))
						.HeightOverride(BarHeight)
						.Visibility(Pct > 0.001f ? EVisibility::Visible : EVisibility::Collapsed)
						[
							SNew(SBorder)
							.BorderImage(FillBrush)
							.BorderBackgroundColor(FLinearColor::White)
							.Padding(FMargin(0.f))
						]
					]
				];
		}

		return FT66FlatStyle::BuildFlatProgressBar(
			Pct,
			FVector2D(Width, BarHeight),
			FLinearColor(0.92f, 0.05f, 0.12f, 1.0f),
			FMargin(4.f, 3.f));
	}

	TSharedRef<SWidget> MakeAchievementsProgressBar(const float Percent, const float Height)
	{
		return MakeAchievementsProgressBarSized(Percent, 1040.f, Height);
	}

	TSharedRef<SWidget> MakeAchievementsFixedImage(const FString& SourceRelativePath, const FVector2D& Size, const FLinearColor& Tint = FLinearColor::White)
	{
		if (const FSlateBrush* Brush = ResolveAchievementsGeneratedBrush(SourceRelativePath, Size))
		{
			return SNew(SBox)
				.WidthOverride(Size.X)
				.HeightOverride(Size.Y)
				[
					SNew(SImage)
					.Image(Brush)
					.ColorAndOpacity(Tint)
				];
		}

		return SNew(SBox)
			.WidthOverride(Size.X)
			.HeightOverride(Size.Y)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.046f, 0.018f, 0.020f, 1.0f))
				.Padding(FMargin(0.f))
			];
	}

	TSharedRef<SWidget> MakeAchievementsFavoritePlate(
		const FOnClicked& OnClicked,
		const FText& Glyph,
		const FLinearColor& GlyphColor,
		const TAttribute<bool>& bEnabled,
		const FText& ToolTipText)
	{
		const FString PlatePath = MakeAchievementsButtonAssetPath(TEXT("SquareIcon"), TEXT("normal"));
		return SNew(SBox)
			.WidthOverride(54.f)
			.HeightOverride(54.f)
			[
				FT66FlatStyle::MakeBareButton(
					FT66BareButtonParams(
						OnClicked,
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							MakeAchievementsFixedImage(PlatePath, FVector2D(54.f, 54.f), FLinearColor::White)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(Glyph)
							.Font(FT66FlatStyle::Tokens::FontRegular(30))
							.ColorAndOpacity(GlyphColor)
							.Justification(ETextJustify::Center)
						])
					.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
					.SetPadding(FMargin(0.f))
					.SetHAlign(HAlign_Fill)
					.SetVAlign(VAlign_Fill)
					.SetEnabled(bEnabled)
					.SetToolTipText(ToolTipText))
			];
	}

}

UT66AchievementsScreen::UT66AchievementsScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Achievements;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66AchievementsScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

UT66AchievementsSubsystem* UT66AchievementsScreen::GetAchievementsSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66AchievementsSubsystem>();
	}
	return nullptr;
}

UT66PlayerSettingsSubsystem* UT66AchievementsScreen::GetPlayerSettingsSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66PlayerSettingsSubsystem>();
	}
	return nullptr;
}

void UT66AchievementsScreen::RefreshAchievements()
{
	if (UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem())
	{
		AllAchievements = Achievements->GetAllAchievements();
		AllAchievements.Sort([Achievements](const FAchievementData& Left, const FAchievementData& Right)
		{
			const bool bLeftClaimed = Achievements ? Achievements->IsAchievementClaimed(Left.AchievementID) : false;
			const bool bRightClaimed = Achievements ? Achievements->IsAchievementClaimed(Right.AchievementID) : false;
			if (bLeftClaimed != bRightClaimed)
			{
				return !bLeftClaimed && bRightClaimed;
			}

			const float LeftProgress = T66AchievementProgress01(Left);
			const float RightProgress = T66AchievementProgress01(Right);
			if (!FMath::IsNearlyEqual(LeftProgress, RightProgress))
			{
				return LeftProgress > RightProgress;
			}

			const int32 LeftRemaining = T66AchievementRemainingCount(Left);
			const int32 RightRemaining = T66AchievementRemainingCount(Right);
			if (LeftRemaining != RightRemaining)
			{
				return LeftRemaining < RightRemaining;
			}

			const int32 LeftOrder = T66AchievementOrderKey(Left.AchievementID);
			const int32 RightOrder = T66AchievementOrderKey(Right.AchievementID);
			if (LeftOrder != RightOrder)
			{
				return LeftOrder < RightOrder;
			}
			return Left.AchievementID.LexicalLess(Right.AchievementID);
		});
	}
	else
	{
		AllAchievements.Reset();
	}
}

int32 UT66AchievementsScreen::GetUnlockedAchievementCount() const
{
	int32 UnlockedCount = 0;
	for (const FAchievementData& Achievement : AllAchievements)
	{
		if (Achievement.bIsUnlocked)
		{
			++UnlockedCount;
		}
	}
	return UnlockedCount;
}

int32 UT66AchievementsScreen::GetUnlockedAchievementCountForCategory(const ET66AchievementCategory Category) const
{
	int32 UnlockedCount = 0;
	for (const FAchievementData& Achievement : AllAchievements)
	{
		if (Achievement.Category == Category && Achievement.bIsUnlocked)
		{
			++UnlockedCount;
		}
	}
	return UnlockedCount;
}

TArray<FAchievementData> UT66AchievementsScreen::GetAchievementsForCategory(const ET66AchievementCategory Category) const
{
	TArray<FAchievementData> FilteredAchievements;
	for (const FAchievementData& Achievement : AllAchievements)
	{
		if (Achievement.Category == Category)
		{
			FilteredAchievements.Add(Achievement);
		}
	}
	return FilteredAchievements;
}

FText UT66AchievementsScreen::BuildAchievementRewardText(const FAchievementData& Achievement) const
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	if (Achievement.RewardType == ET66AchievementRewardType::SkinUnlock)
	{
		const FText SkinName = Loc
			? Loc->GetText_SkinName(Achievement.RewardSkinID)
			: FText::FromName(Achievement.RewardSkinID);

		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			if (Achievement.RewardEntityType == ET66AchievementRewardEntityType::Hero)
			{
				FHeroData HeroData;
				if (T66GI->GetHeroData(Achievement.RewardEntityID, HeroData))
				{
					const FText HeroName = Loc
						? Loc->GetHeroDisplayName(HeroData)
						: (!HeroData.DisplayName.IsEmpty() ? HeroData.DisplayName : FText::FromName(Achievement.RewardEntityID));
					return FText::Format(
						NSLOCTEXT("T66.Achievements", "SkinRewardHeroFormat", "{0} Outfit: {1}"),
						SkinName,
						HeroName);
				}
			}
			else if (Achievement.RewardEntityType == ET66AchievementRewardEntityType::Companion)
			{
				FCompanionData CompanionData;
				if (T66GI->GetCompanionData(Achievement.RewardEntityID, CompanionData))
				{
					const FText CompanionName = Loc
						? Loc->GetCompanionDisplayName(CompanionData)
						: (!CompanionData.DisplayName.IsEmpty() ? CompanionData.DisplayName : FText::FromName(Achievement.RewardEntityID));
					return FText::Format(
						NSLOCTEXT("T66.Achievements", "SkinRewardCompanionFormat", "{0} Outfit: {1}"),
						SkinName,
						CompanionName);
				}
			}
		}

		return FText::Format(
			NSLOCTEXT("T66.Achievements", "SkinRewardFormat", "{0} Outfit"),
			SkinName);
	}

	if (Loc)
	{
		return FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(Achievement.RewardCoins));
	}

	return FText::Format(NSLOCTEXT("T66.Achievements", "CoinsFormat", "{0} CC"), FText::AsNumber(Achievement.RewardCoins));
}

FText UT66AchievementsScreen::GetAchievementActionText(const FAchievementData& Achievement) const
{
	if (Achievement.RewardType == ET66AchievementRewardType::SkinUnlock)
	{
		return NSLOCTEXT("T66.Achievements", "UnlockReward", "UNLOCK");
	}

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		return Loc->GetText_Claim();
	}

	return NSLOCTEXT("T66.Achievements", "Claim", "CLAIM");
}

TSharedRef<SWidget> UT66AchievementsScreen::BuildSlateUI()
{
	FString RequestedAchievementsTab;
	if (!FParse::Value(FCommandLine::Get(), TEXT("T66AchievementsTab="), RequestedAchievementsTab))
	{
		FString RequestedFrontendScreen;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66FrontendScreen="), RequestedFrontendScreen)
			&& (RequestedFrontendScreen.Equals(TEXT("SteamAchievements"), ESearchCase::IgnoreCase)
				|| RequestedFrontendScreen.Equals(TEXT("Steam"), ESearchCase::IgnoreCase)))
		{
			RequestedAchievementsTab = RequestedFrontendScreen;
		}
	}

	if (!RequestedAchievementsTab.IsEmpty())
	{
		if (RequestedAchievementsTab.Equals(TEXT("Secret"), ESearchCase::IgnoreCase))
		{
			ActiveTab = EAchievementTab::Secret;
		}
		else if (
			RequestedAchievementsTab.Equals(TEXT("Achievements"), ESearchCase::IgnoreCase)
			|| RequestedAchievementsTab.Equals(TEXT("Achievement"), ESearchCase::IgnoreCase)
			|| RequestedAchievementsTab.Equals(TEXT("Steam"), ESearchCase::IgnoreCase)
			|| RequestedAchievementsTab.Equals(TEXT("Standard"), ESearchCase::IgnoreCase)
			|| RequestedAchievementsTab.Equals(TEXT("Normal"), ESearchCase::IgnoreCase))
		{
			ActiveTab = EAchievementTab::Achievements;
		}
	}

	const FText SteamText = NSLOCTEXT("T66.Achievements", "SteamTab", "STEAM");
	const FText SecretText = NSLOCTEXT("T66.Achievements", "SecretTab", "SECRET");

	RefreshAchievements();
	const TArray<FAchievementData> StandardAchievements = GetAchievementsForCategory(ET66AchievementCategory::Standard);
	const int32 UnlockedStandardAchievements = GetUnlockedAchievementCountForCategory(ET66AchievementCategory::Standard);
	const float StandardProgress = StandardAchievements.Num() > 0
		? static_cast<float>(UnlockedStandardAchievements) / static_cast<float>(StandardAchievements.Num())
		: 0.0f;
	const bool bShowingSecret = ActiveTab == EAchievementTab::Secret;
	const FText ActiveTabInfoText = bShowingSecret
		? NSLOCTEXT("T66.Achievements", "SecretTabInfo", "Reveal hidden achievements by discovering secret conditions in runs.")
		: NSLOCTEXT("T66.Achievements", "SteamTabInfo", "Track Steam achievements, rewards, and completion progress.");
	const bool bDemoAchievementRowsLocked = false;
	SetAchievementsActiveStateFolder(bShowingSecret);

	if (!bShowingSecret)
	{
		constexpr float SteamCanvasW = 1920.f;
		constexpr float SteamCanvasH = 1080.f;
		const FButtonStyle& NoBorderButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("NoBorder"));
		TSharedRef<SConstraintCanvas> SteamCanvas = SNew(SConstraintCanvas);

		auto STag = [](const TCHAR* Tag) -> FName
		{
			return FName(Tag);
		};

		auto AddCanvas = [&SteamCanvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
		{
			SteamCanvas->AddSlot()
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X, Y, W, H))
			[
				Widget
			];
		};

		auto AddN = [&AddCanvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
		{
			AddCanvas(X * SteamCanvasW, Y * SteamCanvasH, W * SteamCanvasW, H * SteamCanvasH, Widget);
		};

		auto MakeMetadataRegion = [](const FName Tag, const FString& Role, const ET66FlatState State = ET66FlatState::Default) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(SNew(SSpacer), Tag, Role, State);
		};

		auto MakePanelSurface = [](const FName Tag, const ET66FlatState State = ET66FlatState::Default) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatPanel(
				State,
				FMargin(0.f),
				SNew(SSpacer),
				nullptr,
				Tag);
		};

		auto PlainText = [](
			const FText& Text,
			const int32 FontSize,
			const FLinearColor& Color,
			const bool bBold = true,
			const ETextJustify::Type Justification = ETextJustify::Center) -> TSharedRef<SWidget>
		{
			return SNew(STextBlock)
				.Visibility(EVisibility::HitTestInvisible)
				.Text(Text)
				.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
				.ColorAndOpacity(Color)
				.Justification(Justification)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds);
		};

		auto TaggedText = [](
			const FName Tag,
			const FText& Text,
			const int32 FontSize,
			const FLinearColor& Color,
			const bool bBold = true,
			const ETextJustify::Type Justification = ETextJustify::Center) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(STextBlock)
					.Visibility(EVisibility::HitTestInvisible)
					.Text(Text)
					.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
					.ColorAndOpacity(Color)
					.Justification(Justification)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				],
				Tag,
				TEXT("Label"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true);
		};

		auto MakeIcon = [&PlainText](
			const FName Tag,
			const FSlateBrush* Brush,
			const FVector2D& SizeHint,
			const FText& FallbackText,
			const FLinearColor& Tint = FLinearColor::White) -> TSharedRef<SWidget>
		{
			const TSharedRef<SWidget> IconContent = Brush
				? StaticCastSharedRef<SWidget>(
					SNew(SImage)
					.Visibility(EVisibility::HitTestInvisible)
					.Image(Brush)
					.ColorAndOpacity(Tint))
				: StaticCastSharedRef<SWidget>(PlainText(FallbackText, 20, Tint, true, ETextJustify::Center));

			return FT66FlatStyle::AttachMetadata(
				SNew(SBox)
				.Visibility(EVisibility::HitTestInvisible)
				.WidthOverride(SizeHint.X)
				.HeightOverride(SizeHint.Y)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					IconContent
				],
				Tag,
				TEXT("Icon"),
				ET66FlatState::Default);
		};

		auto MakeFlatTab = [&PlainText](
			const FName Tag,
			const FText& Text,
			FOnClicked OnClicked,
			const ET66FlatState State,
			const float Width,
			const float Height) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatToggleGroupButton(
				State,
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					PlainText(Text, 28, State == ET66FlatState::Selected ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText(), true, ETextJustify::Center)
				],
				MoveTemp(OnClicked),
				FMargin(0.f),
				Width,
				Height,
				true,
				Tag,
				FName(TEXT("AchievementTabs")));
		};

		auto MakeClaimButton = [](const FName Tag, FOnClicked OnClicked, const float Width, const float Height) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatButton(
				ET66FlatState::Default,
				NSLOCTEXT("T66.Achievements", "FlatClaim", "CLAIM"),
				MoveTemp(OnClicked),
				nullptr,
				nullptr,
				FMargin(0.f),
				Width,
				Height,
				true,
				24,
				Tag);
		};

		auto MakeFavoriteButton = [](const FName Tag, const int32 RowIndex) -> TSharedRef<SWidget>
		{
			const FSlateBrush* FavoriteBrush = ResolveAchievementsGeneratedBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/favorite_star_outline.png"), FVector2D(48.f, 53.f));
			return FT66FlatStyle::MakeFlatIconButton(
				ET66FlatState::Default,
				FavoriteBrush,
				FOnClicked::CreateLambda([RowIndex]()
				{
					UE_LOG(LogTemp, Display, TEXT("Steam Achievements favorite placeholder toggled for reference row %d."), RowIndex + 1);
					return FReply::Handled();
				}),
				FVector2D(48.f, 53.f),
				Tag);
		};

		const FSlateBrush* InfoBrush = ResolveAchievementsGeneratedBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/info.png"), FVector2D(32.f, 32.f));
		const FSlateBrush* SteamBrush = ResolveAchievementsGeneratedBrush(T66SteamAchievementsLogoPath, FVector2D(128.f, 128.f));
		const FSlateBrush* TicketBrush = ResolveAchievementsGeneratedBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/ticket.png"), FVector2D(42.f, 32.f));

		struct FFlatSteamAchievementRow
		{
			FText Number;
			FText Name;
			FText Description;
		};

		const TArray<FFlatSteamAchievementRow> Rows = {
			{ FText::FromString(TEXT("01")), NSLOCTEXT("T66.Achievements", "FlatSteamCollector1", "Collector 1"), NSLOCTEXT("T66.Achievements", "FlatSteamCollector1Desc", "[Discover 1 items]") },
			{ FText::FromString(TEXT("02")), NSLOCTEXT("T66.Achievements", "FlatSteamFieldNotes1", "Field Notes 1"), NSLOCTEXT("T66.Achievements", "FlatSteamFieldNotes1Desc", "[Discover 1 enemies]") },
			{ FText::FromString(TEXT("03")), NSLOCTEXT("T66.Achievements", "FlatSteamTokenRank1", "Token Rank 1"), NSLOCTEXT("T66.Achievements", "FlatSteamTokenRank1Desc", "[Unlock Vendor Token level 1]") },
			{ FText::FromString(TEXT("04")), NSLOCTEXT("T66.Achievements", "FlatSteamFirstWin1", "First Win 1"), NSLOCTEXT("T66.Achievements", "FlatSteamFirstWin1Desc", "[Win 1 match in any mode]") }
		};

		AddN(0.024f, 0.125f, 0.951f, 0.804f, MakeMetadataRegion(STag(TEXT("SteamAchievements.Root")), TEXT("Root")));
		if (T66IsPausedGameplayWidget(this))
		{
			AddN(0.024f, 0.123f, 0.100f, 0.060f,
				FT66FlatStyle::MakeFlatButton(
					ET66FlatState::Default,
					NSLOCTEXT("T66.Common", "Back", "BACK"),
					FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleBackClicked),
					nullptr,
					nullptr,
					FMargin(0.f),
					0.100f * SteamCanvasW,
					0.060f * SteamCanvasH,
					true,
					22,
					STag(TEXT("SteamAchievements.BackButton"))));
		}
		AddN(0.148f, 0.123f, 0.690f, 0.060f, MakeMetadataRegion(STag(TEXT("SteamAchievements.SubTabs")), TEXT("ToggleGroup.AchievementTabs")));
		AddN(0.148f, 0.123f, 0.320f, 0.060f, MakeFlatTab(
			STag(TEXT("SteamAchievements.SubTabs.SteamButton")),
			SteamText,
			FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleAchievementsTabClicked),
			ET66FlatState::Selected,
			0.320f * SteamCanvasW,
			0.060f * SteamCanvasH));
		AddN(0.498f, 0.123f, 0.340f, 0.060f, MakeFlatTab(
			STag(TEXT("SteamAchievements.SubTabs.SecretButton")),
			SecretText,
			FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleSecretTabClicked),
			ET66FlatState::Default,
			0.340f * SteamCanvasW,
			0.060f * SteamCanvasH));

		AddN(0.438f, 0.140f, 0.016f, 0.030f, FT66FlatStyle::MakeFlatTooltipIcon(ET66FlatState::Selected, InfoBrush, NSLOCTEXT("T66.Achievements", "SteamTabInfoFlatSelected", "Track Steam achievements, rewards, and completion progress."), FVector2D(31.f, 31.f), STag(TEXT("SteamAchievements.SubTabs.SteamInfoIcon")), FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleAchievementsTabClicked)));
		AddN(0.806f, 0.140f, 0.016f, 0.030f, FT66FlatStyle::MakeFlatTooltipIcon(ET66FlatState::Default, InfoBrush, NSLOCTEXT("T66.Achievements", "SecretTabInfoFlat", "Reveal hidden achievements by discovering secret conditions in runs."), FVector2D(31.f, 31.f), STag(TEXT("SteamAchievements.SubTabs.SecretInfoIcon")), FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleSecretTabClicked)));

		AddN(0.024f, 0.225f, 0.951f, 0.213f, MakePanelSurface(STag(TEXT("SteamAchievements.SummaryPanel")), ET66FlatState::Selected));
		AddN(0.060f, 0.239f, 0.114f, 0.185f, MakeIcon(STag(TEXT("SteamAchievements.Summary.SteamLogo")), SteamBrush, FVector2D(128.f, 128.f), FText::FromString(TEXT("S")), FLinearColor::White));
		AddN(0.217f, 0.274f, 0.413f, 0.053f, TaggedText(
			STag(TEXT("SteamAchievements.Summary.Header")),
			NSLOCTEXT("T66.Achievements", "FlatSteamAchievementsHeader", "STEAM ACHIEVEMENTS"),
			40,
			FT66FlatStyle::SelectedText(),
			true,
			ETextJustify::Left));
		AddN(0.607f, 0.274f, 0.102f, 0.053f, TaggedText(
			STag(TEXT("SteamAchievements.Summary.Count")),
			FText::FromString(TEXT("0/100")),
			40,
			FT66FlatStyle::SelectedText(),
			true,
			ETextJustify::Left));
		AddN(0.217f, 0.357f, 0.719f, 0.038f, FT66FlatStyle::MakeFlatProgressBar(
			TAttribute<float>(0.0f),
			TOptional<FLinearColor>(FT66FlatStyle::SelectedBorder()),
			STag(TEXT("SteamAchievements.Summary.ProgressBar"))));

		AddN(T66AchievementReferenceListPanelX, T66AchievementReferenceListPanelY, T66AchievementReferenceListPanelW, T66AchievementReferenceListPanelH, MakePanelSurface(STag(TEXT("SteamAchievements.ListPanel")), ET66FlatState::Default));
		for (int32 DividerIndex = 0; DividerIndex < UE_ARRAY_COUNT(T66AchievementReferenceDividerY); ++DividerIndex)
		{
			AddN(T66AchievementReferenceListPanelX, T66AchievementReferenceDividerY[DividerIndex], T66AchievementReferenceListPanelW, 0.003f, FT66FlatStyle::MakeFlatDivider(
				Orient_Horizontal,
				3.f,
				TOptional<FLinearColor>(),
				FName(*FString::Printf(TEXT("SteamAchievements.Row%02d.Divider"), DividerIndex + 1))));
		}

		for (int32 RowIndex = 0; RowIndex < Rows.Num(); ++RowIndex)
		{
			const FString Prefix = FString::Printf(TEXT("SteamAchievements.Row%02d"), RowIndex + 1);
			const FT66AchievementReferenceRowLayout& RowLayout = T66AchievementReferenceRows[RowIndex];
			AddN(T66AchievementReferenceListPanelX, GetAchievementReferenceSlotRowY(RowIndex), T66AchievementReferenceListPanelW, GetAchievementReferenceSlotRowH(RowIndex), MakeMetadataRegion(FName(*Prefix), TEXT("AchievementRow")));
			AddN(T66AchievementReferenceNumberX, RowLayout.TextY, T66AchievementReferenceNumberW, T66AchievementReferenceNumberH, TaggedText(FName(*(Prefix + TEXT(".Number"))), Rows[RowIndex].Number, T66AchievementReferenceRowFontSize, FT66FlatStyle::PrimaryText(), true, ETextJustify::Center));
			AddN(T66AchievementReferenceNameX, RowLayout.TextY, T66AchievementReferenceNameW, T66AchievementReferenceTextH, TaggedText(FName(*(Prefix + TEXT(".Name"))), Rows[RowIndex].Name, T66AchievementReferenceRowFontSize, FT66FlatStyle::PrimaryText(), true, ETextJustify::Left));
			AddN(T66AchievementReferenceDescriptionX, RowLayout.TextY, T66AchievementReferenceDescriptionW, T66AchievementReferenceTextH, TaggedText(FName(*(Prefix + TEXT(".Description"))), Rows[RowIndex].Description, T66AchievementReferenceRowFontSize, FT66FlatStyle::PrimaryText(), true, ETextJustify::Left));
			AddN(T66AchievementReferenceProgressX, RowLayout.TextY, T66AchievementReferenceProgressW, T66AchievementReferenceTextH, TaggedText(FName(*(Prefix + TEXT(".Progress"))), FText::FromString(TEXT("0/1")), T66AchievementReferenceRowFontSize, FT66FlatStyle::PrimaryText(), true, ETextJustify::Center));
			AddN(T66AchievementReferenceRewardValueX, RowLayout.TextY, T66AchievementReferenceRewardValueW, T66AchievementReferenceTextH, TaggedText(FName(*(Prefix + TEXT(".RewardValue"))), FText::FromString(TEXT("5")), T66AchievementReferenceRowFontSize, FT66FlatStyle::PrimaryText(), true, ETextJustify::Center));
			AddN(T66AchievementReferenceRewardIconX, RowLayout.TextY - 0.002f, T66AchievementReferenceRewardIconW, T66AchievementReferenceRewardIconH, MakeIcon(FName(*(Prefix + TEXT(".RewardIcon"))), TicketBrush, FVector2D(45.f, 38.f), FText::FromString(TEXT("[]")), FLinearColor::White));
			AddN(T66AchievementReferenceClaimButtonX, RowLayout.ButtonY, T66AchievementReferenceClaimButtonW, T66AchievementReferenceButtonH, MakeClaimButton(
				FName(*(Prefix + TEXT(".ClaimButton"))),
				FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleClaimClicked, FName()),
				T66AchievementReferenceClaimButtonW * SteamCanvasW,
				T66AchievementReferenceButtonH * SteamCanvasH));
			AddN(T66AchievementReferenceFavoriteButtonX, RowLayout.ButtonY, T66AchievementReferenceFavoriteButtonW, T66AchievementReferenceButtonH, MakeFavoriteButton(FName(*(Prefix + TEXT(".FavoriteButton"))), RowIndex));
		}
		if (bDemoAchievementRowsLocked)
		{
			for (int32 RowIndex = 0; RowIndex < Rows.Num(); ++RowIndex)
			{
				const FString Prefix = FString::Printf(TEXT("SteamAchievements.Row%02d"), RowIndex + 1);
				AddN(T66AchievementReferenceListPanelX, GetAchievementReferenceSlotRowY(RowIndex), T66AchievementReferenceListPanelW, GetAchievementReferenceSlotRowH(RowIndex),
					T66DemoModeUI::WrapWithComingSoonOverlay(
						MakeMetadataRegion(FName(*(Prefix + TEXT(".DemoOverlaySurface"))), TEXT("AchievementRowUnavailableSurface"), ET66FlatState::Disabled),
						true,
						this,
						FName(*(Prefix + TEXT(".DemoOverlay")))));
			}
		}

		TSharedRef<SWidget> SteamContent = SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
				.BorderBackgroundColor(FT66FlatStyle::BackgroundColor())
			]
			+ SOverlay::Slot()
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				[
					SNew(SBox)
					.WidthOverride(SteamCanvasW)
					.HeightOverride(SteamCanvasH)
					[
						SteamCanvas
					]
				]
			];

		return SteamContent;
	}

	if (bShowingSecret)
	{
		constexpr float SecretCanvasW = 1920.f;
		constexpr float SecretCanvasH = 1080.f;
		TSharedRef<SConstraintCanvas> SecretCanvas = SNew(SConstraintCanvas);

		auto STag = [](const TCHAR* Tag) -> FName
		{
			return FName(Tag);
		};

		auto AddCanvas = [&SecretCanvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
		{
			SecretCanvas->AddSlot()
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X, Y, W, H))
			[
				Widget
			];
		};

		auto AddN = [&AddCanvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
		{
			AddCanvas(X * SecretCanvasW, Y * SecretCanvasH, W * SecretCanvasW, H * SecretCanvasH, Widget);
		};

		auto MakeMetadataRegion = [](const FName Tag, const FString& Role, const ET66FlatState State = ET66FlatState::Default) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(SNew(SSpacer), Tag, Role, State);
		};

		auto MakePanelSurface = [](const FName Tag, const ET66FlatState State = ET66FlatState::Default) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatPanel(
				State,
				FMargin(0.f),
				SNew(SSpacer),
				nullptr,
				Tag);
		};

		auto PlainText = [](
			const FText& Text,
			const int32 FontSize,
			const FLinearColor& Color,
			const bool bBold = true,
			const ETextJustify::Type Justification = ETextJustify::Center) -> TSharedRef<SWidget>
		{
			return SNew(STextBlock)
				.Visibility(EVisibility::HitTestInvisible)
				.Text(Text)
				.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
				.ColorAndOpacity(Color)
				.Justification(Justification)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds);
		};

		auto TaggedText = [](
			const FName Tag,
			const FText& Text,
			const int32 FontSize,
			const FLinearColor& Color,
			const bool bBold = true,
			const ETextJustify::Type Justification = ETextJustify::Center,
			const bool bAutoWrap = false) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(STextBlock)
					.Visibility(EVisibility::HitTestInvisible)
					.Text(Text)
					.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
					.ColorAndOpacity(Color)
					.Justification(Justification)
					.AutoWrapText(bAutoWrap)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				],
				Tag,
				TEXT("Label"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true);
		};

		auto MakeIcon = [](
			const FName Tag,
			const FSlateBrush* Brush,
			const FVector2D& Size,
			const FString& Role = TEXT("Icon")) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				SNew(SBox)
				.WidthOverride(Size.X)
				.HeightOverride(Size.Y)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Visibility(EVisibility::HitTestInvisible)
					.Image(Brush)
					.ColorAndOpacity(FLinearColor::White)
				],
				Tag,
				Role,
				ET66FlatState::Default);
		};

		auto MakeFlatTab = [&PlainText](
			const FName Tag,
			const FText& Text,
			FOnClicked OnClicked,
			const ET66FlatState State,
			const float Width,
			const float Height) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatToggleGroupButton(
				State,
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					PlainText(Text, 28, State == ET66FlatState::Selected ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText(), true, ETextJustify::Center)
				],
				MoveTemp(OnClicked),
				FMargin(0.f),
				Width,
				Height,
				true,
				Tag,
				FName(TEXT("AchievementTabs")));
		};

		auto MakeClaimButton = [](const FName Tag, const float Width, const float Height) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatButton(
				ET66FlatState::Default,
				NSLOCTEXT("T66.Achievements", "SecretClaimButtonFlat", "CLAIM"),
				FOnClicked::CreateLambda([]()
				{
					return FReply::Handled();
				}),
				nullptr,
				nullptr,
				FMargin(0.f),
				Width,
				Height,
				true,
				22,
				Tag);
		};

		const FText MaskedText = NSLOCTEXT("T66.Achievements", "SecretMaskedTextFlat", "???");
		const FSlateBrush* InfoIconBrush = ResolveAchievementsGeneratedBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/info.png"), FVector2D(31.f, 31.f));
		const FSlateBrush* SecretLogoBrush = ResolveAchievementsGeneratedBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/secret_occult_logo.png"), FVector2D(190.f, 174.f));
		const FSlateBrush* TicketIconBrush = ResolveAchievementsGeneratedBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/ticket.png"), FVector2D(45.f, 38.f));
		const FSlateBrush* FavoriteIconBrush = ResolveAchievementsGeneratedBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/favorite_star_outline.png"), FVector2D(48.f, 53.f));

		auto MakeSecretIcon = [](
			const FName Tag,
			const FSlateBrush* Brush,
			const FVector2D& Size,
			const FString& Role = TEXT("Icon")) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				SNew(SBox)
				.WidthOverride(Size.X)
				.HeightOverride(Size.Y)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Visibility(EVisibility::HitTestInvisible)
					.Image(Brush)
					.ColorAndOpacity(FLinearColor::White)
				],
				Tag,
				Role,
				ET66FlatState::Default);
		};

		AddN(0.024f, 0.125f, 0.951f, 0.804f, MakeMetadataRegion(STag(TEXT("SecretAchievements.Root")), TEXT("Root")));
		if (T66IsPausedGameplayWidget(this))
		{
			AddN(0.024f, 0.123f, 0.100f, 0.060f,
				FT66FlatStyle::MakeFlatButton(
					ET66FlatState::Default,
					NSLOCTEXT("T66.Common", "Back", "BACK"),
					FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleBackClicked),
					nullptr,
					nullptr,
					FMargin(0.f),
					0.100f * SecretCanvasW,
					0.060f * SecretCanvasH,
					true,
					22,
					STag(TEXT("SecretAchievements.BackButton"))));
		}
		AddN(0.148f, 0.123f, 0.690f, 0.060f, MakeMetadataRegion(STag(TEXT("SecretAchievements.SubTabs")), TEXT("ToggleGroup.AchievementTabs")));
		AddN(0.148f, 0.123f, 0.320f, 0.060f, MakeFlatTab(
			STag(TEXT("SecretAchievements.SubTabs.SteamButton")),
			SteamText,
			FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleAchievementsTabClicked),
			ET66FlatState::Default,
			0.320f * SecretCanvasW,
			0.060f * SecretCanvasH));
		AddN(0.438f, 0.140f, 0.016f, 0.030f, FT66FlatStyle::MakeFlatTooltipIcon(
			ET66FlatState::Default,
			InfoIconBrush,
			NSLOCTEXT("T66.Achievements", "SteamTabInfoFlatDefault", "Track Steam achievements, rewards, and completion progress."),
			FVector2D(31.f, 31.f),
			STag(TEXT("SecretAchievements.SubTabs.SteamInfoIcon")),
			FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleAchievementsTabClicked)));
		AddN(0.498f, 0.123f, 0.340f, 0.060f, MakeFlatTab(
			STag(TEXT("SecretAchievements.SubTabs.SecretButton")),
			SecretText,
			FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleSecretTabClicked),
			ET66FlatState::Selected,
			0.340f * SecretCanvasW,
			0.060f * SecretCanvasH));
		AddN(0.806f, 0.140f, 0.016f, 0.030f, FT66FlatStyle::MakeFlatTooltipIcon(
			ET66FlatState::Selected,
			InfoIconBrush,
			NSLOCTEXT("T66.Achievements", "SecretTabInfoFlatSelected", "Reveal hidden achievements by discovering secret conditions in runs."),
			FVector2D(31.f, 31.f),
			STag(TEXT("SecretAchievements.SubTabs.SecretInfoIcon")),
			FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleSecretTabClicked)));

		AddN(0.024f, 0.225f, 0.951f, 0.213f, MakePanelSurface(STag(TEXT("SecretAchievements.SummaryPanel")), ET66FlatState::Selected));
		AddN(0.060f, 0.239f, 0.114f, 0.185f, MakeSecretIcon(
			STag(TEXT("SecretAchievements.Summary.SecretLogo")),
			SecretLogoBrush,
			FVector2D(190.f, 174.f)));
		AddN(0.217f, 0.274f, 0.413f, 0.053f, TaggedText(
			STag(TEXT("SecretAchievements.Summary.Header")),
			NSLOCTEXT("T66.Achievements", "SecretAchievementsSummaryHeaderFlat", "SECRET ACHIEVEMENTS"),
			40,
			FT66FlatStyle::SelectedText(),
			true,
			ETextJustify::Left));
		AddN(0.607f, 0.274f, 0.102f, 0.053f, TaggedText(
			STag(TEXT("SecretAchievements.Summary.Count")),
			NSLOCTEXT("T66.Achievements", "SecretAchievementsSummaryCountFlat", "0/???"),
			40,
			FT66FlatStyle::SelectedText(),
			true,
			ETextJustify::Left));
		AddN(0.217f, 0.357f, 0.719f, 0.038f, FT66FlatStyle::MakeFlatProgressBar(
			TAttribute<float>(0.0f),
			TOptional<FLinearColor>(FT66FlatStyle::SelectedBorder()),
			STag(TEXT("SecretAchievements.Summary.ProgressBar"))));

		AddN(T66AchievementReferenceListPanelX, T66AchievementReferenceListPanelY, T66AchievementReferenceListPanelW, T66AchievementReferenceListPanelH, MakePanelSurface(STag(TEXT("SecretAchievements.ListPanel")), ET66FlatState::Default));

		const FText SecretRowNames[] = {
			NSLOCTEXT("T66.Achievements", "SecretRow01NameFlat", "?? #???$ ??#"),
			NSLOCTEXT("T66.Achievements", "SecretRow02NameFlat", "??$# ???# $$?"),
			NSLOCTEXT("T66.Achievements", "SecretRow03NameFlat", "?? ##$ ??$?"),
			NSLOCTEXT("T66.Achievements", "SecretRow04NameFlat", "??# ??$# ?#?")
		};
		const FText SecretRowDescriptions[] = {
			NSLOCTEXT("T66.Achievements", "SecretRow01DescriptionFlat", "LT\"? ?? ##$#]"),
			NSLOCTEXT("T66.Achievements", "SecretRow02DescriptionFlat", "LT\". ?## ????##]"),
			NSLOCTEXT("T66.Achievements", "SecretRow03DescriptionFlat", "L??\"? ?$# ?#??$]"),
			NSLOCTEXT("T66.Achievements", "SecretRow04DescriptionFlat", "L?\"? #??? $#?]")
		};

		for (int32 RowIndex = 0; RowIndex < UE_ARRAY_COUNT(T66AchievementReferenceRows); ++RowIndex)
		{
			const FString Prefix = FString::Printf(TEXT("SecretAchievements.Row%02d"), RowIndex + 1);
			const FT66AchievementReferenceRowLayout& RowLayout = T66AchievementReferenceRows[RowIndex];
			AddN(T66AchievementReferenceListPanelX, GetAchievementReferenceSlotRowY(RowIndex), T66AchievementReferenceListPanelW, GetAchievementReferenceSlotRowH(RowIndex), MakeMetadataRegion(FName(*Prefix), TEXT("AchievementRow")));
			AddN(T66AchievementReferenceNumberX, RowLayout.TextY, T66AchievementReferenceNumberW, T66AchievementReferenceNumberH, TaggedText(
				FName(*(Prefix + TEXT(".Number"))),
				FText::FromString(FString::Printf(TEXT("%02d"), RowIndex + 1)),
				T66AchievementReferenceRowFontSize,
				FT66FlatStyle::PrimaryText(),
				true,
				ETextJustify::Center));
			AddN(T66AchievementReferenceNameX, RowLayout.TextY, T66AchievementReferenceNameW, T66AchievementReferenceTextH, TaggedText(
				FName(*(Prefix + TEXT(".Name"))),
				SecretRowNames[RowIndex],
				T66AchievementReferenceRowFontSize,
				FT66FlatStyle::PrimaryText(),
				true,
				ETextJustify::Left));
			AddN(T66AchievementReferenceDescriptionX, RowLayout.TextY, T66AchievementReferenceDescriptionW, T66AchievementReferenceTextH, TaggedText(
				FName(*(Prefix + TEXT(".Description"))),
				SecretRowDescriptions[RowIndex],
				T66AchievementReferenceRowFontSize,
				FT66FlatStyle::PrimaryText(),
				true,
				ETextJustify::Left));
			AddN(T66AchievementReferenceProgressX, RowLayout.TextY, T66AchievementReferenceProgressW, T66AchievementReferenceTextH, TaggedText(
				FName(*(Prefix + TEXT(".Progress"))),
				MaskedText,
				T66AchievementReferenceRowFontSize,
				FT66FlatStyle::PrimaryText(),
				true,
				ETextJustify::Center));
			AddN(T66AchievementReferenceRewardValueX, RowLayout.TextY, T66AchievementReferenceRewardValueW, T66AchievementReferenceTextH, TaggedText(
				FName(*(Prefix + TEXT(".RewardValue"))),
				NSLOCTEXT("T66.Achievements", "SecretAchievementRewardValueFlat", "5"),
				T66AchievementReferenceRowFontSize,
				FT66FlatStyle::PrimaryText(),
				true,
				ETextJustify::Center));
			AddN(T66AchievementReferenceRewardIconX, RowLayout.TextY - 0.002f, T66AchievementReferenceRewardIconW, T66AchievementReferenceRewardIconH, MakeSecretIcon(
				FName(*(Prefix + TEXT(".RewardIcon"))),
				TicketIconBrush,
				FVector2D(45.f, 38.f)));
			AddN(T66AchievementReferenceClaimButtonX, RowLayout.ButtonY, T66AchievementReferenceClaimButtonW, T66AchievementReferenceButtonH, MakeClaimButton(
				FName(*(Prefix + TEXT(".ClaimButton"))),
				T66AchievementReferenceClaimButtonW * SecretCanvasW,
				T66AchievementReferenceButtonH * SecretCanvasH));
			AddN(T66AchievementReferenceFavoriteButtonX, RowLayout.ButtonY, T66AchievementReferenceFavoriteButtonW, T66AchievementReferenceButtonH, FT66FlatStyle::MakeFlatIconButton(
				ET66FlatState::Default,
				FavoriteIconBrush,
				FOnClicked::CreateLambda([]()
				{
					UE_LOG(LogTemp, Verbose, TEXT("Secret Achievements favorite placeholder clicked."));
					return FReply::Handled();
				}),
				FVector2D(48.f, 53.f),
				FName(*(Prefix + TEXT(".FavoriteButton")))));
		}

		for (int32 DividerIndex = 0; DividerIndex < UE_ARRAY_COUNT(T66AchievementReferenceDividerY); ++DividerIndex)
		{
			AddN(T66AchievementReferenceListPanelX, T66AchievementReferenceDividerY[DividerIndex], T66AchievementReferenceListPanelW, 0.003f, FT66FlatStyle::MakeFlatDivider(
				Orient_Horizontal,
				3.f,
				TOptional<FLinearColor>(),
				FName(*FString::Printf(TEXT("SecretAchievements.Row%02d.Divider"), DividerIndex + 1))));
		}
		if (bDemoAchievementRowsLocked)
		{
			for (int32 RowIndex = 0; RowIndex < UE_ARRAY_COUNT(T66AchievementReferenceRows); ++RowIndex)
			{
				const FString Prefix = FString::Printf(TEXT("SecretAchievements.Row%02d"), RowIndex + 1);
				AddN(T66AchievementReferenceListPanelX, GetAchievementReferenceSlotRowY(RowIndex), T66AchievementReferenceListPanelW, GetAchievementReferenceSlotRowH(RowIndex),
					T66DemoModeUI::WrapWithComingSoonOverlay(
						MakeMetadataRegion(FName(*(Prefix + TEXT(".DemoOverlaySurface"))), TEXT("AchievementRowUnavailableSurface"), ET66FlatState::Disabled),
						true,
						this,
						FName(*(Prefix + TEXT(".DemoOverlay")))));
			}
		}

		TSharedRef<SWidget> SecretContent = SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
				.BorderBackgroundColor(FT66FlatStyle::BackgroundColor())
			]
			+ SOverlay::Slot()
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				[
					SNew(SBox)
					.WidthOverride(SecretCanvasW)
					.HeightOverride(SecretCanvasH)
					[
						SecretCanvas
					]
				]
			];

		return SecretContent;
	}

	const FT66FlatStyle::FFrontendChromeMetrics& ChromeMetrics = FT66FlatStyle::GetFrontendChromeMetrics();
	const FSlateFontInfo ChromeTabFont = FT66FlatStyle::MakeFrontendChromeTabFont();

	auto MakeTabButton = [this, &ChromeMetrics, ChromeTabFont](const FText& Label, bool bActive, bool bLeft, FReply(UT66AchievementsScreen::*Handler)()) -> TSharedRef<SWidget>
	{
		return MakeAchievementsGeneratedButton(
			FT66ButtonParams(Label, FOnClicked::CreateUObject(this, Handler), bActive ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
			.SetMinWidth(ChromeMetrics.TabMinWidth)
			.SetHeight(ChromeMetrics.TabHeight),
			ResolveAchievementsToggleButtonStyle(bActive, bLeft),
			ChromeTabFont,
			bActive ? T66AchievementsTabActiveText() : T66AchievementsTabInactiveText(),
			ChromeMetrics.TabPadding);
	};

	const TSharedRef<SWidget> Root =
		SNew(SBox)
		.Padding(FMargin(-14.f, 0.f, -14.f, 8.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 0.f, 0.f, 12.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.f, 0.f, 20.f, 0.f)
				[
					MakeTabButton(
						SteamText,
						ActiveTab == EAchievementTab::Achievements,
						true,
						&UT66AchievementsScreen::HandleAchievementsTabClicked)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					MakeTabButton(
						SecretText,
						ActiveTab == EAchievementTab::Secret,
						false,
						&UT66AchievementsScreen::HandleSecretTabClicked)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(SBox)
				.HeightOverride(72.f)
				[
					MakeAchievementsGeneratedPanel(
						FT66FlatStyle::GetFlatRedSquareButtonAssetPath(TEXT("normal")),
						SNew(SBox)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(ActiveTabInfoText)
							.Font(AchievementsRegularFont(16))
							.ColorAndOpacity(T66AchievementsParchmentText())
							.Justification(ETextJustify::Center)
							.AutoWrapText(true)
						],
						FMargin(22.f, 12.f),
						FLinearColor::White,
						FLinearColor::White)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(SBox)
				.HeightOverride(126.f)
				[
					MakeAchievementsGeneratedPanel(
						MakeSettingsAssetPath(TEXT("settings_row_shell_split.png")),
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 0.f, 0.f, 8.f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(310.f)
								[
									SNew(STextBlock)
									.Text(NSLOCTEXT("T66.Achievements", "TotalProgressLabel", "TOTAL PROGRESS"))
									.Font(AchievementsBoldFont(24))
									.ColorAndOpacity(T66AchievementsParchmentText())
									.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
								]
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.f)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							[
								SNew(STextBlock)
								.Text_Lambda([bShowingSecret, UnlockedStandardAchievements, StandardAchievements]() -> FText
								{
									if (bShowingSecret)
									{
										return NSLOCTEXT("T66.Achievements", "SecretProgressMaskedLabel", "???");
									}

									return FText::Format(
										NSLOCTEXT("T66.Achievements", "CompletionLabel", "{0} / {1} Achievements"),
										FText::AsNumber(UnlockedStandardAchievements),
										FText::AsNumber(StandardAchievements.Num()));
								})
								.Font(AchievementsBoldFont(22))
								.ColorAndOpacity(T66AchievementsParchmentText())
								.Justification(ETextJustify::Center)
								.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(120.f)
								[
									SNew(STextBlock)
									.Text_Lambda([bShowingSecret, StandardProgress]() -> FText
									{
										return bShowingSecret
											? NSLOCTEXT("T66.Achievements", "SecretProgressMaskedPercent", "???")
											: FText::AsPercent(StandardProgress);
									})
									.Font(AchievementsBoldFont(24))
									.ColorAndOpacity(T66AchievementsParchmentText())
									.Justification(ETextJustify::Right)
									.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
								]
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						[
							MakeAchievementsProgressBar(bShowingSecret ? 0.0f : StandardProgress, 8.f)
						],
						FMargin(40.f, 24.f, 40.f, 20.f),
						FLinearColor::White,
						FLinearColor::White)
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			.Padding(0.f)
			[
				SNew(SScrollBox)
				.ScrollBarStyle(GetAchievementsReferenceScrollBarStyle())
				.ScrollBarVisibility(EVisibility::Visible)
				.ScrollBarThickness(FVector2D(18.f, 18.f))
				.ScrollBarPadding(FMargin(12.f, 0.f, 0.f, 0.f))
				+ SScrollBox::Slot()
				[
					SAssignNew(AchievementListBox, SVerticalBox)
				]
			]
		];

	RebuildAchievementList();
	return FT66FlatStyle::MakeTopBarScreenRoot(
		UIManager,
		Root,
		SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black),
		FLinearColor::Transparent,
		FMargin(0.f, 0.f, 0.f, -4.f));
}

void UT66AchievementsScreen::RebuildAchievementList()
{
	if (!AchievementListBox.IsValid())
	{
		return;
	}

	AchievementListBox->ClearChildren();
	RefreshAchievements();
	SetAchievementsActiveStateFolder(ActiveTab == EAchievementTab::Secret);

	UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem();
	UT66PlayerSettingsSubsystem* PlayerSettings = GetPlayerSettingsSubsystem();
	const FText FavoriteAchievementTooltip = NSLOCTEXT("T66.Achievements", "FavoriteAchievementTooltip", "Favorite achievement");
	const FText UnfavoriteAchievementTooltip = NSLOCTEXT("T66.Achievements", "UnfavoriteAchievementTooltip", "Remove favorite");

	auto AddAchievementSection = [this, Achievements, PlayerSettings, &FavoriteAchievementTooltip, &UnfavoriteAchievementTooltip](const FText& SectionTitle, const TArray<FAchievementData>& SectionAchievements)
	{
		if (!AchievementListBox.IsValid())
		{
			return;
		}

		int32 UnlockedInSection = 0;
		for (const FAchievementData& Achievement : SectionAchievements)
		{
			if (Achievement.bIsUnlocked)
			{
				++UnlockedInSection;
			}
		}

		AchievementListBox->AddSlot()
		.AutoHeight()
		.Padding(28.f, 0.f, 0.f, 7.f)
		[
			SNew(STextBlock)
			.Text(FText::Format(
				NSLOCTEXT("T66.Achievements", "SectionTitleFormat", "{0}  {1} / {2}"),
				SectionTitle,
				FText::AsNumber(UnlockedInSection),
				FText::AsNumber(SectionAchievements.Num())))
			.Font(AchievementsBoldFont(26))
			.ColorAndOpacity(T66AchievementsSectionText())
			.ShadowOffset(FVector2D(0.f, 2.f))
			.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.70f))
		];

		for (const FAchievementData& Achievement : SectionAchievements)
		{
			const FLinearColor RowBackground = Achievement.bIsUnlocked
				? T66AchievementsUnlockedRowFill()
				: T66AchievementsRowFill();
			const FString ProgressString = FString::Printf(
				TEXT("%d/%d"),
				FMath::Min(Achievement.CurrentProgress, Achievement.RequirementCount),
				FMath::Max(1, Achievement.RequirementCount));
			const bool bClaimed = Achievements ? Achievements->IsAchievementClaimed(Achievement.AchievementID) : false;
			const bool bCanClaim = Achievement.bIsUnlocked && !bClaimed;
			const bool bIsFavorited = PlayerSettings && PlayerSettings->IsFavoriteAchievement(Achievement.AchievementID);
			const FText RewardText = BuildAchievementRewardText(Achievement);
			const FText ActionText = GetAchievementActionText(Achievement);
			const FText FavoriteGlyph = FText::FromString(bIsFavorited ? TEXT("\u2605") : TEXT("\u2606"));
			const FLinearColor FavoriteGlyphColor = bIsFavorited
				? FLinearColor(0.92f, 0.05f, 0.12f, 1.0f)
				: T66AchievementsParchmentMutedText();

			AchievementListBox->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 7.f)
			[
				SNew(SBox)
				.HeightOverride(96.f)
				[
					MakeAchievementsGeneratedPanel(
						MakeSettingsAssetPath(TEXT("settings_row_shell_full.png")),
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.VAlign(VAlign_Center)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Text(Achievement.DisplayName)
								.Font(AchievementsBoldFont(18))
								.ColorAndOpacity(T66AchievementsParchmentText())
								.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 2.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(Achievement.Description)
								.Font(AchievementsRegularFont(14))
								.ColorAndOpacity(T66AchievementsParchmentMutedText())
								.AutoWrapText(true)
								.WrapTextAt(900.f)
								.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
								.Clipping(EWidgetClipping::ClipToBounds)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 6.f, 18.f, 0.f)
							[
								MakeAchievementsProgressBarSized(T66AchievementProgress01(Achievement), 760.f, 6.f)
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(170.f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(ProgressString))
								.Font(AchievementsBoldFont(18))
								.ColorAndOpacity(Achievement.bIsUnlocked ? FLinearColor(0.92f, 0.05f, 0.12f, 1.0f) : T66AchievementsParchmentText())
								.Justification(ETextJustify::Center)
								.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(190.f)
							[
								SNew(STextBlock)
								.Text(bClaimed
									? (GetLocSubsystem() ? GetLocSubsystem()->GetText_Claimed() : NSLOCTEXT("T66.Achievements", "Claimed", "CLAIMED"))
									: RewardText)
								.Font(AchievementsBoldFont(18))
								.ColorAndOpacity(T66AchievementsParchmentText())
								.Justification(ETextJustify::Center)
								.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(8.f, 0.f, 0.f, 0.f)
						[
							MakeAchievementsFavoritePlate(
								FOnClicked::CreateLambda([this, PlayerSettings, AchievementID = Achievement.AchievementID]()
								{
									if (PlayerSettings)
									{
										PlayerSettings->SetFavoriteAchievement(AchievementID, !PlayerSettings->IsFavoriteAchievement(AchievementID));
										ForceRebuildSlate();
									}
									return FReply::Handled();
								}),
								FavoriteGlyph,
								FavoriteGlyphColor,
								TAttribute<bool>(PlayerSettings != nullptr),
								bIsFavorited ? UnfavoriteAchievementTooltip : FavoriteAchievementTooltip)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(8.f, 0.f, 0.f, 0.f)
						[
							MakeAchievementsGeneratedButton(
								FT66ButtonParams(
									ActionText,
									FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleClaimClicked, Achievement.AchievementID),
									ET66ButtonType::Primary)
								.SetMinWidth(128.f)
								.SetHeight(40.f)
								.SetEnabled(bCanClaim)
								.SetVisibility(bCanClaim ? EVisibility::Visible : EVisibility::Collapsed),
								ResolveAchievementsCompactButtonStyle(),
								AchievementsBoldFont(18),
								T66AchievementsParchmentText(),
								FMargin(16.f, 7.f, 16.f, 6.f))
						],
						FMargin(28.f, 10.f, 24.f, 10.f),
						FLinearColor::White,
						RowBackground)
				]
			];
		}
	};

	if (ActiveTab == EAchievementTab::Achievements)
	{
		AddAchievementSection(
			NSLOCTEXT("T66.Achievements", "AchievementsHeader", "ACHIEVEMENTS"),
			GetAchievementsForCategory(ET66AchievementCategory::Standard));
		return;
	}

	const FText MaskedText = NSLOCTEXT("T66.Achievements", "SecretMaskedText", "???");
	AchievementListBox->AddSlot()
	.AutoHeight()
	.Padding(28.f, 0.f, 0.f, 8.f)
	[
		SNew(STextBlock)
		.Text(NSLOCTEXT("T66.Achievements", "SecretAchievementsHeader", "SECRET ACHIEVEMENTS"))
		.Font(AchievementsBoldFont(26))
		.ColorAndOpacity(T66AchievementsSectionText())
		.ShadowOffset(FVector2D(0.f, 2.f))
		.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.70f))
	];

	for (int32 RowIndex = 0; RowIndex < T66SecretPlaceholderRowCount; ++RowIndex)
	{
		AchievementListBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 5.f)
		[
			MakeAchievementsGeneratedPanel(
				MakeSettingsAssetPath(TEXT("settings_row_shell_full.png")),
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(MaskedText)
						.Font(AchievementsBoldFont(19))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 3.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(MaskedText)
						.Font(AchievementsRegularFont(16))
						.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
						.AutoWrapText(true)
						.WrapTextAt(900.f)
						.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 6.f, 18.f, 0.f)
					[
						MakeAchievementsProgressBarSized(0.0f, 760.f, 6.f)
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(170.f)
					[
						SNew(STextBlock)
						.Text(MaskedText)
						.Font(AchievementsBoldFont(19))
						.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
						.Justification(ETextJustify::Center)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(190.f)
					[
						SNew(STextBlock)
						.Text(MaskedText)
						.Font(AchievementsBoldFont(19))
						.ColorAndOpacity(T66AchievementsParchmentText())
						.Justification(ETextJustify::Center)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(8.f, 0.f, 0.f, 0.f)
				[
					MakeAchievementsGeneratedButton(
						FT66ButtonParams(
							MaskedText,
							FOnClicked::CreateLambda([]()
							{
								return FReply::Handled();
							}),
							ET66ButtonType::Primary)
						.SetMinWidth(128.f)
						.SetHeight(40.f)
						.SetEnabled(false),
						ResolveAchievementsCompactButtonStyle(),
						AchievementsBoldFont(18),
						FT66FlatStyle::Tokens::TextMuted,
						FMargin(16.f, 7.f, 16.f, 6.f))
					]
				,
				FMargin(28.f, 7.f, 22.f, 7.f),
				FLinearColor::White,
				RowIndex % 2 == 0 ? T66AchievementsRowFill() : T66AchievementsUnlockedRowFill())
		];
	}
}

void UT66AchievementsScreen::OnScreenActivated_Implementation()
{
	FString RequestedAchievementsTab;
	bool bShouldRebuildForRequestedTab = false;
	if (!FParse::Value(FCommandLine::Get(), TEXT("T66AchievementsTab="), RequestedAchievementsTab))
	{
		FString RequestedFrontendScreen;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66FrontendScreen="), RequestedFrontendScreen)
			&& (RequestedFrontendScreen.Equals(TEXT("SteamAchievements"), ESearchCase::IgnoreCase)
				|| RequestedFrontendScreen.Equals(TEXT("Steam"), ESearchCase::IgnoreCase)))
		{
			RequestedAchievementsTab = RequestedFrontendScreen;
		}
	}
	if (!RequestedAchievementsTab.IsEmpty())
	{
		EAchievementTab RequestedTab = ActiveTab;
		bool bHasValidRequestedTab = true;
		if (RequestedAchievementsTab.Equals(TEXT("Secret"), ESearchCase::IgnoreCase))
		{
			RequestedTab = EAchievementTab::Secret;
		}
		else if (
			RequestedAchievementsTab.Equals(TEXT("Achievements"), ESearchCase::IgnoreCase)
			|| RequestedAchievementsTab.Equals(TEXT("Achievement"), ESearchCase::IgnoreCase)
			|| RequestedAchievementsTab.Equals(TEXT("Steam"), ESearchCase::IgnoreCase)
			|| RequestedAchievementsTab.Equals(TEXT("Standard"), ESearchCase::IgnoreCase)
			|| RequestedAchievementsTab.Equals(TEXT("Normal"), ESearchCase::IgnoreCase))
		{
			RequestedTab = EAchievementTab::Achievements;
		}
		else
		{
			bHasValidRequestedTab = false;
		}

		if (bHasValidRequestedTab && ActiveTab != RequestedTab)
		{
			ActiveTab = RequestedTab;
			bShouldRebuildForRequestedTab = HasBuiltSlateUI();
		}
	}

	Super::OnScreenActivated_Implementation();
	if (AchievementListBox.IsValid())
	{
		RebuildAchievementList();
	}

	if (bShouldRebuildForRequestedTab)
	{
		ForceRebuildSlate();
	}

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66AchievementsScreen::HandleLanguageChanged);
	}

	if (UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem())
	{
		Achievements->AchievementsStateChanged.AddUniqueDynamic(this, &UT66AchievementsScreen::HandleAchievementsStateChanged);
	}
}

void UT66AchievementsScreen::OnScreenDeactivated_Implementation()
{
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.RemoveDynamic(this, &UT66AchievementsScreen::HandleLanguageChanged);
	}

	if (UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem())
	{
		Achievements->AchievementsStateChanged.RemoveDynamic(this, &UT66AchievementsScreen::HandleAchievementsStateChanged);
	}

	Super::OnScreenDeactivated_Implementation();
}

void UT66AchievementsScreen::OnBackClicked()
{
	if (T66IsPausedGameplayWidget(this) && UIManager)
	{
		ShowModal(ET66ScreenType::PauseMenu);
		return;
	}

	NavigateBack();
}

FReply UT66AchievementsScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66AchievementsScreen::HandleClaimClicked(FName AchievementID)
{
	if (UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem())
	{
		Achievements->TryClaimAchievement(AchievementID);
	}
	return FReply::Handled();
}

FReply UT66AchievementsScreen::HandleAchievementsTabClicked()
{
	if (ActiveTab == EAchievementTab::Achievements)
	{
		return FReply::Handled();
	}

	ActiveTab = EAchievementTab::Achievements;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66AchievementsScreen::HandleSecretTabClicked()
{
	if (ActiveTab == EAchievementTab::Secret)
	{
		return FReply::Handled();
	}

	ActiveTab = EAchievementTab::Secret;
	ForceRebuildSlate();
	return FReply::Handled();
}

void UT66AchievementsScreen::HandleLanguageChanged(ET66Language NewLanguage)
{
	RequestDeferredSlateRebuild();
}

void UT66AchievementsScreen::HandleAchievementsStateChanged()
{
	if (!HasBuiltSlateUI() || !IsVisible())
	{
		return;
	}

	if (AchievementListBox.IsValid())
	{
		RebuildAchievementList();
	}
	else
	{
		ForceRebuildSlate();
	}
}
