// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PowerUpScreen.h"
#include "Core/T66AchievementsSubsystem.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "UI/T66DemoModeUIUtils.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66FlatStyle.h"
#include "Engine/GameInstance.h"
#include "Engine/TextureDefines.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Layout/Clipping.h"
#include "Misc/CommandLine.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "Styling/CoreStyle.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr int32 ShopFontDelta = -2;
	constexpr float ShopReferenceWidth = 1920.f;
	constexpr float ShopReferenceHeight = 1080.f;
	constexpr float ShopHeroSelectionEditReferenceHeight = 1080.f;
	constexpr float ShopCardGap = 24.f;
	constexpr float ShopRelicCardWidth = 466.f;
	constexpr float ShopRelicCardHeight = 442.f;
	constexpr int32 ShopRelicStatIncrease = UT66BuffSubsystem::RelicPermanentBonusStatPoints;
	const FLinearColor ShopPermanentCardFill(0.020f, 0.022f, 0.032f, 0.98f);
	const FLinearColor ShopPermanentCardAccent(0.92f, 0.05f, 0.12f, 1.0f);
	TMap<FString, TStrongObjectPtr<UTexture2D>> GShopFileTextureCache;
	TMap<FString, TSharedPtr<FSlateBrush>> GShopGeneratedBrushCache;
	TMap<FString, TSharedPtr<FButtonStyle>> GShopGeneratedButtonStyleCache;

	bool IsShopSlicedButtonPath(const FString& SourceRelativePath);

	int32 AdjustShopFontSize(int32 BaseSize)
	{
		return FMath::Max(8, BaseSize + ShopFontDelta);
	}

	FSlateFontInfo ShopBoldFont(int32 BaseSize)
	{
		return FT66FlatStyle::Tokens::FontBold(AdjustShopFontSize(BaseSize));
	}

	FSlateFontInfo ShopRegularFont(int32 BaseSize)
	{
		return FT66FlatStyle::Tokens::FontRegular(AdjustShopFontSize(BaseSize));
	}

	FLinearColor T66PowerUpPanelFill()
	{
		return FLinearColor(0.018f, 0.021f, 0.030f, 0.98f);
	}

	FLinearColor T66PowerUpInsetFill()
	{
		return FLinearColor(0.046f, 0.018f, 0.020f, 0.98f);
	}

	FLinearColor T66PowerUpButtonFill()
	{
		return FLinearColor(0.92f, 0.05f, 0.12f, 1.0f);
	}

	FLinearColor T66PowerUpButtonDisabledFill()
	{
		return FLinearColor(0.28f, 0.025f, 0.035f, 0.88f);
	}

	FLinearColor T66PowerUpNeutralButtonFill()
	{
		return FLinearColor(0.12f, 0.026f, 0.032f, 0.98f);
	}

	FLinearColor T66PowerUpTabActiveText()
	{
		return FLinearColor(1.0f, 0.96f, 0.88f, 1.0f);
	}

	FLinearColor T66PowerUpTabInactiveText()
	{
		return FLinearColor(0.96f, 0.78f, 0.74f, 1.0f);
	}

	FLinearColor T66PowerUpText()
	{
		return FLinearColor(0.99f, 0.94f, 0.92f, 1.0f);
	}

	FLinearColor T66PowerUpMutedText()
	{
		return FLinearColor(0.84f, 0.62f, 0.58f, 1.0f);
	}

	FLinearColor T66PowerUpTitleText()
	{
		return FLinearColor(1.0f, 0.96f, 0.88f, 1.0f);
	}

	FLinearColor T66PowerUpStrokeTint()
	{
		return FLinearColor::White;
	}

	FString MakePowerUpMainMenuChromePath(const TCHAR* FileName)
	{
		return FT66FlatStyle::GetFlatMainMenuElementAssetPath(FileName);
	}

	FString MakePowerUpMainMenuSquareChromePath(const TCHAR* FileName)
	{
		const FString Name(FileName ? FileName : TEXT(""));
		if (Name.Contains(TEXT("_red_square_variant"), ESearchCase::IgnoreCase)
			|| Name.StartsWith(TEXT("dropdown_field_"), ESearchCase::IgnoreCase)
			|| Name.StartsWith(TEXT("leaderboard_tab_button_"), ESearchCase::IgnoreCase))
		{
			FString State = FPaths::GetBaseFilename(Name).ToLower();
			if (State.StartsWith(TEXT("dropdown_field_"), ESearchCase::IgnoreCase))
			{
				State.RemoveFromStart(TEXT("dropdown_field_"), ESearchCase::IgnoreCase);
			}
			else if (State.StartsWith(TEXT("leaderboard_tab_button_"), ESearchCase::IgnoreCase))
			{
				State.RemoveFromStart(TEXT("leaderboard_tab_button_"), ESearchCase::IgnoreCase);
			}
			else if (State.StartsWith(TEXT("cta_new_game_button_"), ESearchCase::IgnoreCase))
			{
				State.RemoveFromStart(TEXT("cta_new_game_button_"), ESearchCase::IgnoreCase);
				State.RemoveFromEnd(TEXT("_red_square_variant"), ESearchCase::IgnoreCase);
			}
			return FT66FlatStyle::GetFlatRedSquareButtonAssetPath(*State);
		}
		return FT66FlatStyle::GetFlatChromeElementAssetPath(FileName);
	}

	FString MakePowerUpReferencePath(const TCHAR* StateFolder, const TCHAR* Family, const FString& FileName)
	{
		return FString(TEXT("SourceAssets/UI/Reference/Screens/PowerUp"))
			/ FString(StateFolder ? StateFolder : TEXT("Common"))
			/ FString(Family ? Family : TEXT(""))
			/ FileName;
	}

	FString MakePowerUpArchivedReferencePath(const TCHAR* StateFolder, const TCHAR* Family, const FString& FileName)
	{
		return FString(TEXT("SourceAssets/UI/Reference/archive/GeneratedChrome_PreSimpleBoxes_20260505_231624/Screens/PowerUp"))
			/ FString(StateFolder ? StateFolder : TEXT("Common"))
			/ FString(Family ? Family : TEXT(""))
			/ FileName;
	}

	bool DoesShopRelativePathExist(const FString& RelativePath)
	{
		if (RelativePath.IsEmpty())
		{
			return false;
		}

		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
		{
			if (FPaths::FileExists(CandidatePath))
			{
				return true;
			}
		}

		return false;
	}

	FString SelectFirstExistingShopPath(const TArray<FString>& RelativePaths)
	{
		for (const FString& RelativePath : RelativePaths)
		{
			if (DoesShopRelativePathExist(RelativePath))
			{
				return RelativePath;
			}
		}

		return RelativePaths.Num() > 0 ? RelativePaths.Last() : FString();
	}

	FString MakePowerUpMainMenuPanelFallbackPath(const FString& Name)
	{
		if (Name.Contains(TEXT("item_art_well.png")) || Name.Contains(TEXT("art_placeholder.png")))
		{
			return MakePowerUpMainMenuSquareChromePath(TEXT("profile_slot_selected_red_square_variant.png"));
		}
		if (Name.Contains(TEXT("info_strip")))
		{
			return MakePowerUpMainMenuSquareChromePath(TEXT("cta_new_game_button_normal_red_square_variant.png"));
		}
		if (Name.Contains(TEXT("row_shell")))
		{
			return FT66FlatStyle::GetFlatLongPanelAssetPath(TEXT("normal"));
		}

		return MakePowerUpMainMenuSquareChromePath(TEXT("main_panel_normal_square_variant.png"));
	}

	FString MakePowerUpOwnedButtonPath(const TCHAR* State)
	{
		const FString NormalizedState = FString(State ? State : TEXT("normal")).ToLower();
		const FString MainMenuState = NormalizedState.Equals(TEXT("selected"), ESearchCase::IgnoreCase)
			? FString(TEXT("normal_red"))
			: NormalizedState;
		const FString FileName = FString::Printf(TEXT("powerup_buttons_pill_%s.png"), *NormalizedState);
		const FString MainMenuFileName = MainMenuState.EndsWith(TEXT("_red"))
			? FString::Printf(TEXT("cta_new_game_button_%s_square_variant.png"), *MainMenuState)
			: FString::Printf(TEXT("cta_new_game_button_%s_red_square_variant.png"), *MainMenuState);
		return SelectFirstExistingShopPath(TArray<FString>{
			MakePowerUpMainMenuSquareChromePath(*MainMenuFileName),
			MakePowerUpReferencePath(TEXT("Diplomas"), TEXT("Buttons"), FileName),
			MakePowerUpArchivedReferencePath(TEXT("Diplomas"), TEXT("Buttons"), FileName)
		});
	}

	FString MakePowerUpOwnedPanelPath(const TCHAR* FileName)
	{
		const FString Name(FileName ? FileName : TEXT(""));
		return SelectFirstExistingShopPath(TArray<FString>{
			MakePowerUpMainMenuPanelFallbackPath(Name),
			MakePowerUpReferencePath(TEXT("Diplomas"), TEXT("Panels"), Name),
			MakePowerUpArchivedReferencePath(TEXT("Diplomas"), TEXT("Panels"), Name)
		});
	}

	FString MakePowerUpStatePanelPath(const TCHAR* StateFolder, const TCHAR* FileName)
	{
		const FString Name(FileName ? FileName : TEXT(""));
		const TCHAR* SafeStateFolder = StateFolder ? StateFolder : TEXT("Common");
		return SelectFirstExistingShopPath(TArray<FString>{
			MakePowerUpMainMenuPanelFallbackPath(Name),
			MakePowerUpReferencePath(SafeStateFolder, TEXT("Panels"), Name),
			MakePowerUpArchivedReferencePath(SafeStateFolder, TEXT("Panels"), Name)
		});
	}

	FString MakePowerUpCommonButtonPath(const TCHAR* State)
	{
		const FString NormalizedState = FString(State ? State : TEXT("normal")).ToLower();
		const FString FileName = FString::Printf(TEXT("powerup_buttons_pill_%s.png"), *NormalizedState);
		const FString ButtonState = NormalizedState.Equals(TEXT("selected"), ESearchCase::IgnoreCase)
			? FString(TEXT("normal"))
			: NormalizedState;
		const FString MainMenuFileName = FString::Printf(TEXT("cta_new_game_button_%s_red_square_variant.png"), *ButtonState);
		return SelectFirstExistingShopPath(TArray<FString>{
			MakePowerUpMainMenuSquareChromePath(*MainMenuFileName),
			MakePowerUpReferencePath(TEXT("Common"), TEXT("Buttons"), FileName),
			MakePowerUpArchivedReferencePath(TEXT("Common"), TEXT("Buttons"), FileName)
		});
	}

	FString MakePowerUpCommonPath(const TCHAR* Family, const TCHAR* FileName)
	{
		const FString FamilyName(Family ? Family : TEXT(""));
		const FString Name(FileName ? FileName : TEXT(""));
		if (FamilyName.Equals(TEXT("ScreenArt"), ESearchCase::IgnoreCase))
		{
			return FString();
		}
		if (FamilyName.Equals(TEXT("Icons"), ESearchCase::IgnoreCase)
			&& Name.Contains(TEXT("coupon"), ESearchCase::IgnoreCase))
		{
			return SelectFirstExistingShopPath(TArray<FString>{
				MakePowerUpReferencePath(TEXT("Common"), TEXT("Icons"), Name),
				MakePowerUpArchivedReferencePath(TEXT("Common"), TEXT("Icons"), Name),
				MakePowerUpMainMenuChromePath(TEXT("coupon_ticket_icon.png"))
			});
		}

		return SelectFirstExistingShopPath(TArray<FString>{
			MakePowerUpReferencePath(TEXT("Common"), *FamilyName, Name),
			MakePowerUpArchivedReferencePath(TEXT("Common"), *FamilyName, Name)
		});
	}

	FString MakePowerUpDrugsPath(const TCHAR* Family, const TCHAR* FileName)
	{
		const FString FamilyName(Family ? Family : TEXT(""));
		const FString Name(FileName ? FileName : TEXT(""));
		if (FamilyName.Equals(TEXT("Icons"), ESearchCase::IgnoreCase)
			&& Name.Contains(TEXT("coupon"), ESearchCase::IgnoreCase))
		{
			return SelectFirstExistingShopPath(TArray<FString>{
				MakePowerUpReferencePath(TEXT("Drugs"), TEXT("Icons"), Name),
				MakePowerUpArchivedReferencePath(TEXT("Drugs"), TEXT("Icons"), Name),
				MakePowerUpMainMenuChromePath(TEXT("coupon_ticket_icon.png"))
			});
		}

		return SelectFirstExistingShopPath(TArray<FString>{
			MakePowerUpReferencePath(TEXT("Drugs"), *FamilyName, Name),
			MakePowerUpArchivedReferencePath(TEXT("Drugs"), *FamilyName, Name)
		});
	}

	FString MakePowerUpDrugsButtonPath(const TCHAR* State)
	{
		const FString NormalizedState = FString(State ? State : TEXT("normal")).ToLower();
		const FString FileName = FString::Printf(TEXT("powerupdrugs_buttons_pill_%s.png"), *NormalizedState);
		const FString ButtonState = NormalizedState.Equals(TEXT("selected"), ESearchCase::IgnoreCase)
			? FString(TEXT("normal"))
			: NormalizedState;
		const FString MainMenuFileName = FString::Printf(TEXT("cta_new_game_button_%s_red_square_variant.png"), *ButtonState);
		return SelectFirstExistingShopPath(TArray<FString>{
			MakePowerUpMainMenuSquareChromePath(*MainMenuFileName),
			MakePowerUpReferencePath(TEXT("Drugs"), TEXT("Buttons"), FileName),
			MakePowerUpArchivedReferencePath(TEXT("Drugs"), TEXT("Buttons"), FileName)
		});
	}

	FT66ButtonParams FlattenShopButton(FT66ButtonParams Params)
	{
		Params
			.SetBorderVisual(ET66ButtonBorderVisual::None)
			.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
			.SetUseGlow(false);

		if (Params.FontSize > 0)
		{
			Params.SetFontSize(AdjustShopFontSize(static_cast<int32>(Params.FontSize)));
		}

		return Params;
	}

	TSharedRef<SWidget> MakeShopButton(const FT66ButtonParams& Params)
	{
		return FT66FlatStyle::MakeButton(FlattenShopButton(Params));
	}

	TSharedRef<SWidget> MakeShopPanel(const TSharedRef<SWidget>& Content, const FLinearColor& FillColor, const FMargin& Padding, ET66PanelType Type = ET66PanelType::Panel)
	{
		return FT66FlatStyle::MakePanel(
			Content,
			FT66PanelParams(Type)
				.SetBorderVisual(ET66ButtonBorderVisual::None)
				.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
				.SetColor(FillColor)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeShopFittedText(
		const TAttribute<FText>& Text,
		const FSlateFontInfo& Font,
		const TAttribute<FSlateColor>& TextColor,
		ETextJustify::Type Justification = ETextJustify::Center)
	{
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::DownOnly)
				[
					SNew(STextBlock)
					.Text(Text)
					.Font(Font)
					.ColorAndOpacity(TextColor)
					.Justification(Justification)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
			];
	}

	UTexture2D* LoadShopFileTexture(const FString& FilePath)
	{
		if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GShopFileTextureCache.Find(FilePath))
		{
			return CachedTexture->Get();
		}

		const TextureFilter Filter = IsShopSlicedButtonPath(FilePath)
			|| FilePath.Contains(TEXT("SourceAssets/UI/Reference/"))
			|| FilePath.Contains(TEXT("SourceAssets/UI/PowerUp/Diplomas/Generated/"))
			? TextureFilter::TF_Nearest
			: TextureFilter::TF_Trilinear;

		UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
			FilePath,
			Filter,
			false,
			TEXT("ShopTexture"));
		if (!Texture)
		{
			Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
				FilePath,
				Filter,
				TEXT("ShopTexture"));
		}
		if (!Texture)
		{
			return nullptr;
		}

		GShopFileTextureCache.Add(FilePath, TStrongObjectPtr<UTexture2D>(Texture));
		return Texture;
	}

	UTexture2D* LoadShopFlatContentTexture(const FString& FilePath)
	{
		if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GShopFileTextureCache.Find(FilePath))
		{
			return CachedTexture->Get();
		}

		const TextureFilter Filter = FilePath.Contains(TEXT("RuntimeDependencies/T66/UI/PowerUp/Diplomas/Generated/"))
			|| FilePath.Contains(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/"))
			? TextureFilter::TF_Nearest
			: TextureFilter::TF_Trilinear;

		UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
			FilePath,
			Filter,
			false,
			TEXT("ShopFlatContentTexture"));
		if (!Texture)
		{
			Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
				FilePath,
				Filter,
				TEXT("ShopFlatContentTexture"));
		}
		if (!Texture)
		{
			return nullptr;
		}

		GShopFileTextureCache.Add(FilePath, TStrongObjectPtr<UTexture2D>(Texture));
		return Texture;
	}

	FString MakeShopSettingsAssetPath(const TCHAR* FileName)
	{
		const FString Name(FileName);
		const auto BasicButtonPath = [](const TCHAR* State) -> FString
		{
			return MakePowerUpCommonButtonPath(State);
		};
		const auto SelectButtonPath = [](const TCHAR* State) -> FString
		{
			return MakePowerUpCommonButtonPath(State);
		};

		if (Name.StartsWith(TEXT("settings_toggle_on_")))
		{
			return SelectButtonPath(TEXT("selected"));
		}
		if (Name.StartsWith(TEXT("settings_compact_neutral_")) || Name.StartsWith(TEXT("settings_toggle_off_")))
		{
			if (Name.Contains(TEXT("_hover"))) return SelectButtonPath(TEXT("hover"));
			if (Name.Contains(TEXT("_pressed"))) return SelectButtonPath(TEXT("pressed"));
			return SelectButtonPath(TEXT("normal"));
		}
		if (Name.StartsWith(TEXT("settings_toggle_inactive_")))
		{
			return BasicButtonPath(TEXT("disabled"));
		}
		if (Name == TEXT("settings_content_shell_frame.png"))
		{
			return MakePowerUpMainMenuSquareChromePath(TEXT("main_panel_normal_square_variant.png"));
		}
		if (Name == TEXT("settings_row_shell_full.png") || Name == TEXT("settings_row_shell_split.png"))
		{
			return FT66FlatStyle::GetFlatLongPanelAssetPath(TEXT("normal"));
		}
		if (Name == TEXT("settings_dropdown_field.png"))
		{
			return MakePowerUpMainMenuSquareChromePath(TEXT("dropdown_field_normal_square_variant.png"));
		}

		return FString(TEXT("SourceAssets/UI/Reference/Shared")) / Name;
	}

	FMargin GetShopGeneratedBrushMargin(const FString& SourceRelativePath)
	{
		if (SourceRelativePath.Contains(TEXT("main_panel_normal.png")) || SourceRelativePath.Contains(TEXT("main_panel_normal_square_variant.png")))
		{
			return FMargin(0.055f, 0.080f, 0.055f, 0.090f);
		}
		if (SourceRelativePath.Contains(TEXT("leaderboard_row_normal.png")) || SourceRelativePath.Contains(TEXT("leaderboard_row_hover.png")))
		{
			return FMargin(0.070f, 0.180f, 0.070f, 0.180f);
		}
		if (SourceRelativePath.Contains(TEXT("long_panel_normal.png")) || SourceRelativePath.Contains(TEXT("long_panel_hover.png"))
			|| SourceRelativePath.Contains(TEXT("long_panel_disabled.png")))
		{
			return FMargin(0.055f, 0.210f, 0.055f, 0.210f);
		}
		if (SourceRelativePath.Contains(TEXT("player_row_panel_normal.png")) || SourceRelativePath.Contains(TEXT("player_row_panel_hover.png"))
			|| SourceRelativePath.Contains(TEXT("player_row_panel_normal_square_variant.png")) || SourceRelativePath.Contains(TEXT("player_row_panel_hover_square_variant.png")))
		{
			return FMargin(0.080f, 0.160f, 0.080f, 0.160f);
		}
		if (SourceRelativePath.Contains(TEXT("profile_slot_normal.png")) || SourceRelativePath.Contains(TEXT("profile_slot_selected.png"))
			|| SourceRelativePath.Contains(TEXT("profile_slot_normal_square_variant.png")) || SourceRelativePath.Contains(TEXT("profile_slot_selected_square_variant.png"))
			|| SourceRelativePath.Contains(TEXT("profile_slot_selected_red_square_variant.png")))
		{
			return FMargin(0.100f, 0.100f, 0.100f, 0.100f);
		}
		if (SourceRelativePath.Contains(TEXT("leaderboard_tab_button_")))
		{
			return FMargin(0.093f, 0.213f, 0.093f, 0.213f);
		}
		if (SourceRelativePath.Contains(TEXT("cta_new_game_button_")) || SourceRelativePath.Contains(TEXT("cta_load_game_button_")))
		{
			return FMargin(0.083f, 0.231f, 0.083f, 0.231f);
		}
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
		if (SourceRelativePath.Contains(TEXT("upgrade_card_normal.png")))
		{
			return FMargin(0.118f, 0.175f, 0.118f, 0.175f);
		}
		if (SourceRelativePath.Contains(TEXT("item_art_well.png")))
		{
			return FMargin(0.125f, 0.180f, 0.125f, 0.180f);
		}
		if (SourceRelativePath.Contains(TEXT("info_strip.png")))
		{
			return FMargin(0.112f, 0.160f, 0.112f, 0.160f);
		}
		if (SourceRelativePath.Contains(TEXT("dropdown_field_normal.png")) || SourceRelativePath.Contains(TEXT("dropdown_field_normal_square_variant.png")))
		{
			return FMargin(0.06f, 0.34f, 0.06f, 0.34f);
		}
		if (SourceRelativePath.Contains(TEXT("progress_bar_track.png")) || SourceRelativePath.Contains(TEXT("progress_bar_fill_red.png")))
		{
			return FMargin(0.12f, 0.34f, 0.12f, 0.34f);
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

	bool IsZeroShopMargin(const FMargin& Margin)
	{
		return FMath::IsNearlyZero(Margin.Left)
			&& FMath::IsNearlyZero(Margin.Top)
			&& FMath::IsNearlyZero(Margin.Right)
			&& FMath::IsNearlyZero(Margin.Bottom);
	}

	bool IsShopSlicedButtonPath(const FString& SourceRelativePath)
	{
		return FT66FlatStyle::IsFlatChromeButtonAssetPath(SourceRelativePath)
			&& SourceRelativePath.Contains(TEXT("/Buttons/"));
	}

	void EnsureShopRuntimeImageBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize);
	FVector2D ResolveShopImageSize(UTexture2D* Texture, const FVector2D& FallbackImageSize);

	const FSlateBrush* ResolveShopGeneratedBrush(const FString& SourceRelativePath, const FVector2D& ImageSize = FVector2D::ZeroVector)
	{
		const FString BrushKey = FString::Printf(TEXT("%s::%.0fx%.0f"), *SourceRelativePath, ImageSize.X, ImageSize.Y);
		if (const TSharedPtr<FSlateBrush>* CachedBrush = GShopGeneratedBrushCache.Find(BrushKey))
		{
			return CachedBrush->Get();
		}

		UTexture2D* Texture = nullptr;
		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(SourceRelativePath))
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			Texture = LoadShopFileTexture(CandidatePath);
			if (Texture)
			{
				break;
			}
		}

		if (!Texture)
		{
			if (!T66RuntimeUIBrushAccess::ShouldUseSimpleReferenceFallback(SourceRelativePath))
			{
				return nullptr;
			}

			TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
			const FMargin BrushMargin = GetShopGeneratedBrushMargin(SourceRelativePath);
			const bool bSlicedButton = IsShopSlicedButtonPath(SourceRelativePath);
			const FVector2D ResolvedSize = ImageSize.X > 0.f && ImageSize.Y > 0.f ? ImageSize : FVector2D(1.f, 1.f);
			T66RuntimeUIBrushAccess::ConfigureSimpleReferenceFallbackBrush(
				*Brush,
				SourceRelativePath,
				ResolvedSize,
				bSlicedButton ? FMargin(0.f) : BrushMargin,
				bSlicedButton || IsZeroShopMargin(BrushMargin) ? ESlateBrushDrawType::Image : ESlateBrushDrawType::Box);
			GShopGeneratedBrushCache.Add(BrushKey, Brush);
			return Brush.Get();
		}

		const FVector2D ResolvedSize = ResolveShopImageSize(Texture, ImageSize);
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		const FMargin BrushMargin = GetShopGeneratedBrushMargin(SourceRelativePath);
		const bool bSlicedButton = IsShopSlicedButtonPath(SourceRelativePath);
		Brush->DrawAs = bSlicedButton || IsZeroShopMargin(BrushMargin) ? ESlateBrushDrawType::Image : ESlateBrushDrawType::Box;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ResolvedSize;
		Brush->Margin = bSlicedButton ? FMargin(0.f) : BrushMargin;
		Brush->TintColor = FSlateColor(FLinearColor::White);
		Brush->SetResourceObject(Texture);

		GShopGeneratedBrushCache.Add(BrushKey, Brush);
		return Brush.Get();
	}

	const FSlateBrush* ResolveShopFlatContentBrush(const FString& SourceRelativePath, const FVector2D& ImageSize = FVector2D::ZeroVector)
	{
		const FString BrushKey = FString::Printf(TEXT("FlatContent::%s::%.0fx%.0f"), *SourceRelativePath, ImageSize.X, ImageSize.Y);
		if (const TSharedPtr<FSlateBrush>* CachedBrush = GShopGeneratedBrushCache.Find(BrushKey))
		{
			return CachedBrush->Get();
		}

		UTexture2D* Texture = nullptr;
		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(SourceRelativePath))
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			Texture = LoadShopFlatContentTexture(CandidatePath);
			if (Texture)
			{
				break;
			}
		}

		if (!Texture)
		{
			return nullptr;
		}

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ResolveShopImageSize(Texture, ImageSize);
		Brush->TintColor = FSlateColor(FLinearColor::White);
		Brush->SetResourceObject(Texture);

		GShopGeneratedBrushCache.Add(BrushKey, Brush);
		return Brush.Get();
	}

	const FScrollBarStyle* GetShopReferenceScrollBarStyle()
	{
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		static bool bInitialized = false;
		if (!bInitialized)
		{
			bInitialized = true;
			const FSlateBrush* VerticalTrackBrush = ResolveShopGeneratedBrush(MakePowerUpMainMenuChromePath(TEXT("progress_bar_track.png")), FVector2D(18.f, 120.f));
			const FSlateBrush* HorizontalTrackBrush = ResolveShopGeneratedBrush(MakePowerUpMainMenuChromePath(TEXT("progress_bar_track.png")), FVector2D(156.f, 18.f));
			const FSlateBrush* ThumbBrush = ResolveShopGeneratedBrush(MakePowerUpMainMenuChromePath(TEXT("progress_bar_fill_red.png")), FVector2D(156.f, 18.f));
			const FSlateBrush* HoverBrush = ResolveShopGeneratedBrush(MakePowerUpMainMenuChromePath(TEXT("progress_bar_fill_red.png")), FVector2D(156.f, 18.f));

			if (VerticalTrackBrush && HorizontalTrackBrush && ThumbBrush && HoverBrush)
			{
				Style
					.SetVerticalBackgroundImage(*VerticalTrackBrush)
					.SetVerticalTopSlotImage(*VerticalTrackBrush)
					.SetVerticalBottomSlotImage(*VerticalTrackBrush)
					.SetHorizontalBackgroundImage(*HorizontalTrackBrush)
					.SetHorizontalTopSlotImage(*HorizontalTrackBrush)
					.SetHorizontalBottomSlotImage(*HorizontalTrackBrush)
					.SetNormalThumbImage(*ThumbBrush)
					.SetHoveredThumbImage(*HoverBrush)
					.SetDraggedThumbImage(*HoverBrush)
					.SetThickness(18.f);
			}
		}

		return &Style;
	}

	const FScrollBarStyle* GetShopDrugsScrollBarStyle()
	{
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		static bool bInitialized = false;
		if (!bInitialized)
		{
			bInitialized = true;
			const FSlateBrush* TrackBrush = ResolveShopGeneratedBrush(MakePowerUpMainMenuChromePath(TEXT("progress_bar_track.png")), FVector2D(18.f, 120.f));
			const FSlateBrush* ThumbBrush = ResolveShopGeneratedBrush(MakePowerUpMainMenuChromePath(TEXT("progress_bar_fill_red.png")), FVector2D(18.f, 96.f));

			if (TrackBrush && ThumbBrush)
			{
				Style
					.SetVerticalBackgroundImage(*TrackBrush)
					.SetVerticalTopSlotImage(*TrackBrush)
					.SetVerticalBottomSlotImage(*TrackBrush)
					.SetNormalThumbImage(*ThumbBrush)
					.SetHoveredThumbImage(*ThumbBrush)
					.SetDraggedThumbImage(*ThumbBrush)
					.SetThickness(18.f);
			}
		}

		return &Style;
	}

	const FButtonStyle* ResolveShopGeneratedButtonStyle(
		const FString& Key,
		const FString& NormalPath,
		const FString& HoverPath,
		const FString& PressedPath,
		const FString& DisabledPath)
	{
		if (const TSharedPtr<FButtonStyle>* CachedStyle = GShopGeneratedButtonStyleCache.Find(Key))
		{
			return CachedStyle->Get();
		}

		const FButtonStyle& NoBorderStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
		TSharedPtr<FButtonStyle> Style = MakeShared<FButtonStyle>(NoBorderStyle);
		if (const FSlateBrush* NormalBrush = ResolveShopGeneratedBrush(NormalPath))
		{
			Style->SetNormal(*NormalBrush);
		}
		if (const FSlateBrush* HoverBrush = ResolveShopGeneratedBrush(HoverPath))
		{
			Style->SetHovered(*HoverBrush);
		}
		if (const FSlateBrush* PressedBrush = ResolveShopGeneratedBrush(PressedPath))
		{
			Style->SetPressed(*PressedBrush);
		}
		if (const FSlateBrush* DisabledBrush = ResolveShopGeneratedBrush(DisabledPath))
		{
			Style->SetDisabled(*DisabledBrush);
		}
		Style->SetNormalPadding(FMargin(0.f));
		Style->SetPressedPadding(FMargin(0.f));

		GShopGeneratedButtonStyleCache.Add(Key, Style);
		return Style.Get();
	}

	const FButtonStyle* ResolveShopCompactButtonStyle()
	{
		return ResolveShopGeneratedButtonStyle(
			TEXT("PowerUp.CompactButton"),
			MakeShopSettingsAssetPath(TEXT("settings_compact_neutral_normal.png")),
			MakeShopSettingsAssetPath(TEXT("settings_compact_neutral_hover.png")),
			MakeShopSettingsAssetPath(TEXT("settings_compact_neutral_pressed.png")),
			MakeShopSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")));
	}

	const FButtonStyle* ResolveShopDrugsCompactButtonStyle()
	{
		return ResolveShopGeneratedButtonStyle(
			TEXT("PowerUp.Drugs.CompactButton"),
			MakePowerUpOwnedButtonPath(TEXT("normal")),
			MakePowerUpOwnedButtonPath(TEXT("hover")),
			MakePowerUpOwnedButtonPath(TEXT("pressed")),
			MakePowerUpOwnedButtonPath(TEXT("disabled")));
	}

	FString MakeShopDrugsDuoButtonPath(const bool bLeft, const TCHAR* State)
	{
		(void)bLeft;
		return MakePowerUpDrugsButtonPath(State);
	}

	const FButtonStyle* ResolveShopDrugsToggleButtonStyle(const bool bActive, const bool bLeft)
	{
		return bActive
			? ResolveShopGeneratedButtonStyle(
				bLeft ? TEXT("PowerUp.Drugs.ToggleLeftOn") : TEXT("PowerUp.Drugs.ToggleRightOn"),
				MakeShopDrugsDuoButtonPath(bLeft, TEXT("selected")),
				MakeShopDrugsDuoButtonPath(bLeft, TEXT("selected")),
				MakeShopDrugsDuoButtonPath(bLeft, TEXT("pressed")),
				MakePowerUpDrugsButtonPath(TEXT("disabled")))
			: ResolveShopGeneratedButtonStyle(
				bLeft ? TEXT("PowerUp.Drugs.ToggleLeftOff") : TEXT("PowerUp.Drugs.ToggleRightOff"),
				MakeShopDrugsDuoButtonPath(bLeft, TEXT("normal")),
				MakeShopDrugsDuoButtonPath(bLeft, TEXT("hover")),
				MakeShopDrugsDuoButtonPath(bLeft, TEXT("pressed")),
				MakePowerUpDrugsButtonPath(TEXT("disabled")));
	}

	const FButtonStyle* ResolveShopDiplomaCompactButtonStyle()
	{
		return ResolveShopGeneratedButtonStyle(
			TEXT("PowerUp.DiplomaCompactButton"),
			MakePowerUpOwnedButtonPath(TEXT("normal")),
			MakePowerUpOwnedButtonPath(TEXT("hover")),
			MakePowerUpOwnedButtonPath(TEXT("pressed")),
			MakePowerUpOwnedButtonPath(TEXT("disabled")));
	}

	FString MakeShopDuoButtonPath(const bool bLeft, const TCHAR* State)
	{
		(void)bLeft;
		return MakePowerUpCommonButtonPath(State);
	}

	const FButtonStyle* ResolveShopToggleButtonStyle(const bool bActive, const bool bLeft)
	{
		return bActive
			? ResolveShopGeneratedButtonStyle(
				bLeft ? TEXT("PowerUp.ToggleLeftOn") : TEXT("PowerUp.ToggleRightOn"),
				MakeShopDuoButtonPath(bLeft, TEXT("selected")),
				MakeShopDuoButtonPath(bLeft, TEXT("selected")),
				MakeShopDuoButtonPath(bLeft, TEXT("pressed")),
				MakeShopSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")))
			: ResolveShopGeneratedButtonStyle(
				bLeft ? TEXT("PowerUp.ToggleLeftOff") : TEXT("PowerUp.ToggleRightOff"),
				MakeShopDuoButtonPath(bLeft, TEXT("normal")),
				MakeShopDuoButtonPath(bLeft, TEXT("hover")),
				MakeShopDuoButtonPath(bLeft, TEXT("pressed")),
				MakeShopSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")));
	}

	TSharedRef<SWidget> MakeShopGeneratedPanel(
		const FString& SourceRelativePath,
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const FLinearColor& Tint = FLinearColor::White,
		const FLinearColor& FallbackFill = T66PowerUpPanelFill())
	{
		if (const FSlateBrush* Brush = ResolveShopGeneratedBrush(SourceRelativePath))
		{
			return SNew(SBorder)
				.BorderImage(Brush)
				.BorderBackgroundColor(Tint)
				.Padding(Padding)
				[
					Content
				];
		}

		return MakeShopPanel(Content, FallbackFill, Padding);
	}

	TSharedRef<SWidget> MakeShopGeneratedButton(
		const FT66ButtonParams& Params,
		const FButtonStyle* ButtonStyle,
		const FSlateFontInfo& Font,
		const FLinearColor& TextColor,
		const FMargin& ContentPadding)
	{
		const TSharedRef<SWidget> ButtonContent = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: StaticCastSharedRef<SWidget>(
				MakeShopFittedText(
					Params.DynamicLabel.IsBound() ? Params.DynamicLabel : TAttribute<FText>(Params.Label),
					Font,
					TAttribute<FSlateColor>(TextColor)));

		if (!ButtonStyle)
		{
			return FT66FlatStyle::MakeBareButton(
				FT66BareButtonParams(Params.OnClicked, ButtonContent)
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
			ButtonContent,
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

	FString GetShopForbiddenChadPartKey(ET66HeroStatType StatType)
	{
		switch (StatType)
		{
			case ET66HeroStatType::Damage:      return TEXT("left_arm");
			case ET66HeroStatType::AttackSpeed: return TEXT("right_arm");
			case ET66HeroStatType::AttackScale: return TEXT("head");
			case ET66HeroStatType::Accuracy:    return TEXT("head");
			case ET66HeroStatType::Armor:       return TEXT("torso");
			case ET66HeroStatType::Evasion:     return TEXT("left_leg");
			case ET66HeroStatType::Luck:        return TEXT("right_leg");
			case ET66HeroStatType::Speed:       return TEXT("right_leg");
			default:                            return TEXT("unknown");
		}
	}

	void EnsureShopRuntimeImageBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
	}

	FVector2D ResolveShopImageSize(UTexture2D* Texture, const FVector2D& FallbackImageSize)
	{
		if (FallbackImageSize.X > 0.f && FallbackImageSize.Y > 0.f)
		{
			return FallbackImageSize;
		}

		if (Texture)
		{
			const int32 TextureWidth = Texture->GetSizeX();
			const int32 TextureHeight = Texture->GetSizeY();
			if (TextureWidth > 0 && TextureHeight > 0)
			{
				return FVector2D(static_cast<float>(TextureWidth), static_cast<float>(TextureHeight));
			}
		}

		return FVector2D(1.f, 1.f);
	}

	TSharedPtr<FSlateBrush> MakeShopImageBrush(
		TMap<FString, TSharedPtr<FSlateBrush>>& OwnedBrushes,
		UT66UITexturePoolSubsystem* TexPool,
		UObject* Requester,
		const FString& BrushKey,
		const FString& SourceRelativePath,
		const FString& PackagePath,
		const FString& ObjectPath,
		const FVector2D& ImageSize)
	{
		const TArray<FString> CandidateSourcePaths = T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(SourceRelativePath);
		const bool bHasSourceFile = CandidateSourcePaths.ContainsByPredicate([](const FString& CandidatePath)
		{
			return FPaths::FileExists(CandidatePath);
		});
		const bool bHasImportedTexture = TexPool && FPackageName::DoesPackageExist(PackagePath);
		TSharedPtr<FSlateBrush> Brush;

		if (bHasImportedTexture)
		{
			if (UTexture2D* AssetTexture = T66RuntimeUITextureAccess::LoadAssetTexture(*ObjectPath, TextureFilter::TF_Trilinear, TEXT("ShopTexture")))
			{
				Brush = MakeShared<FSlateBrush>();
				EnsureShopRuntimeImageBrush(Brush, ResolveShopImageSize(AssetTexture, ImageSize));
				Brush->SetResourceObject(AssetTexture);
				Brush->TintColor = FSlateColor(FLinearColor::White);
			}
			else
			{
				Brush = MakeShared<FSlateBrush>();
				EnsureShopRuntimeImageBrush(Brush, ResolveShopImageSize(nullptr, ImageSize));
				const TSoftObjectPtr<UTexture2D> Soft{ FSoftObjectPath(ObjectPath) };
				T66SlateTexture::BindSharedBrushAsync(TexPool, Soft, Requester, Brush, FName(*BrushKey), false);
			}
		}
		else if (bHasSourceFile)
		{
			UTexture2D* Texture = nullptr;
			for (const FString& CandidatePath : CandidateSourcePaths)
			{
				if (!FPaths::FileExists(CandidatePath))
				{
					continue;
				}

				Texture = LoadShopFileTexture(CandidatePath);
				if (Texture)
				{
					break;
				}
			}

			if (Texture)
			{
				Brush = MakeShared<FSlateBrush>();
				EnsureShopRuntimeImageBrush(Brush, ResolveShopImageSize(Texture, ImageSize));
				Brush->SetResourceObject(Texture);
				Brush->TintColor = FSlateColor(FLinearColor::White);
			}
			else
			{
				return nullptr;
			}
		}
		else
		{
			return nullptr;
		}

		OwnedBrushes.Add(BrushKey, Brush);
		return Brush;
	}

	TSharedPtr<FSlateBrush> GetShopForbiddenChadPartBrush(
		TMap<FString, TSharedPtr<FSlateBrush>>& OwnedBrushes,
		UT66UITexturePoolSubsystem* TexPool,
		UObject* Requester,
		ET66HeroStatType StatType,
		const FVector2D& ImageSize)
	{
		const FString PartKey = GetShopForbiddenChadPartKey(StatType);
		const FString FileStem = FString::Printf(TEXT("forbidden_chad_%s"), *PartKey);
		const FString BrushKey = FString::Printf(TEXT("forbidden_chad::%s::%.0fx%.0f"), *PartKey, ImageSize.X, ImageSize.Y);
		// Route through the generated source path so runtime dependency remapping also
		// gives us the authoring folder as an editor fallback.
		const FString SourceRelativePath = FString::Printf(TEXT("SourceAssets/UI/PowerUp/Statues/Generated/forbidden_chad/%s.png"), *FileStem);
		const FString PackagePath = FString::Printf(TEXT("/Game/UI/PowerUp/Statues/forbidden_chad/%s"), *FileStem);
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *FileStem);

		return MakeShopImageBrush(OwnedBrushes, TexPool, Requester, BrushKey, SourceRelativePath, PackagePath, ObjectPath, ImageSize);
	}

	FString GetShopSecondaryBuffSlug(ET66SecondaryStatType StatType)
	{
		switch (StatType)
		{
		case ET66SecondaryStatType::AoeDamage:      return TEXT("aoe-damage");
		case ET66SecondaryStatType::BounceDamage:   return TEXT("bounce-damage");
		case ET66SecondaryStatType::PierceDamage:   return TEXT("pierce-damage");
		case ET66SecondaryStatType::DotDamage:      return TEXT("dot-damage");
		case ET66SecondaryStatType::HeadshotChance: return TEXT("headshot");
		case ET66SecondaryStatType::AoeSpeed:       return TEXT("aoe-speed");
		case ET66SecondaryStatType::BounceSpeed:    return TEXT("bounce-speed");
		case ET66SecondaryStatType::PierceSpeed:    return TEXT("pierce-speed");
		case ET66SecondaryStatType::DotSpeed:       return TEXT("dot-speed");
		case ET66SecondaryStatType::CritChance:     return TEXT("crit-chance");
		case ET66SecondaryStatType::AoeScale:       return TEXT("aoe-scale");
		case ET66SecondaryStatType::BounceScale:    return TEXT("bounce-scale");
		case ET66SecondaryStatType::PierceScale:    return TEXT("pierce-scale");
		case ET66SecondaryStatType::DotScale:       return TEXT("dot-scale");
		case ET66SecondaryStatType::AttackRange:    return TEXT("range");
		case ET66SecondaryStatType::Execute:        return TEXT("execute");
		case ET66SecondaryStatType::Taunt:          return TEXT("taunt");
		case ET66SecondaryStatType::DamageReduction:return TEXT("damage-reduction");
		case ET66SecondaryStatType::ReflectDamage:  return TEXT("damage-reflection");
		case ET66SecondaryStatType::HpRegen:        return TEXT("hp-regen");
		case ET66SecondaryStatType::Crush:          return TEXT("crush");
		case ET66SecondaryStatType::EvasionChance:  return TEXT("dodge");
		case ET66SecondaryStatType::CounterAttack:  return TEXT("counter-attack");
		case ET66SecondaryStatType::LifeSteal:      return TEXT("life-steal");
		case ET66SecondaryStatType::Invisibility:   return TEXT("invisibility");
		case ET66SecondaryStatType::Assassinate:    return TEXT("assassinate");
		case ET66SecondaryStatType::TreasureChest:  return TEXT("treasure-chest");
		case ET66SecondaryStatType::Cheating:       return TEXT("cheating");
		case ET66SecondaryStatType::Stealing:       return TEXT("stealing");
		case ET66SecondaryStatType::LootCrate:      return TEXT("loot-crate");
		case ET66SecondaryStatType::LootBag:        return TEXT("loot-bag");
		case ET66SecondaryStatType::LootWheel:      return TEXT("loot-wheel");
		case ET66SecondaryStatType::Alchemy:        return TEXT("alchemy");
		case ET66SecondaryStatType::Accuracy:       return TEXT("accuracy");
		case ET66SecondaryStatType::VendorToken:    return TEXT("vendor-token");
		default:                                    return FString();
		}
	}

	TSharedPtr<FSlateBrush> GetShopSecondaryBuffBrush(
		TMap<FString, TSharedPtr<FSlateBrush>>& OwnedBrushes,
		UT66UITexturePoolSubsystem* TexPool,
		UObject* Requester,
		ET66SecondaryStatType StatType,
		const FVector2D& ImageSize)
	{
		const FString Slug = GetShopSecondaryBuffSlug(StatType);
		if (Slug.IsEmpty())
		{
			return nullptr;
		}

		const FString FileStem = Slug;
		const FString BrushKey = FString::Printf(TEXT("secondary_buff::%s::%.0fx%.0f"), *Slug, ImageSize.X, ImageSize.Y);
		const FString SourceRelativePath = FString::Printf(TEXT("RuntimeDependencies/T66/UI/PowerUp/SecondaryBuffs/%s.png"), *FileStem);
		const FString PackagePath = FString::Printf(TEXT("/Game/UI/PowerUp/SecondaryBuffs/%s"), *FileStem);
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *FileStem);

		return MakeShopImageBrush(OwnedBrushes, TexPool, Requester, BrushKey, SourceRelativePath, PackagePath, ObjectPath, ImageSize);
	}

}

UT66PowerUpScreen::UT66PowerUpScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::PowerUp;
	bIsModal = false;
}

void UT66PowerUpScreen::OnScreenActivated_Implementation()
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		if (FParse::Param(FCommandLine::Get(), TEXT("T66PowerUpHeroSelectionSteroidEdit"))
			|| FParse::Param(FCommandLine::Get(), TEXT("T66PowerUpHeroSelectionDrugEdit")))
		{
			Buffs->BeginHeroSelectionSingleUseBuffEdit(Buffs->GetSelectedSingleUseBuffEditSlotIndex());
		}

		if (Buffs->IsHeroSelectionSingleUseBuffEditActive())
		{
			bShowingSingleUse = true;
		}
	}

	FString RequestedPowerUpTab;
	bool bHasRequestedPowerUpTab = false;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66PowerUpTab="), RequestedPowerUpTab))
	{
		bHasRequestedPowerUpTab = true;
	}
	else
	{
		FString RequestedFrontendScreen;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66FrontendScreen="), RequestedFrontendScreen)
			&& (RequestedFrontendScreen.Equals(TEXT("Relics"), ESearchCase::IgnoreCase)
				|| RequestedFrontendScreen.Equals(TEXT("Steroids"), ESearchCase::IgnoreCase)
				|| RequestedFrontendScreen.Equals(TEXT("Diplomas"), ESearchCase::IgnoreCase)
				|| RequestedFrontendScreen.Equals(TEXT("Drugs"), ESearchCase::IgnoreCase)))
		{
			RequestedPowerUpTab = RequestedFrontendScreen;
			bHasRequestedPowerUpTab = true;
		}
	}

	if (bHasRequestedPowerUpTab)
	{
		bShowingSingleUse =
			RequestedPowerUpTab.Equals(TEXT("SingleUse"), ESearchCase::IgnoreCase)
			|| RequestedPowerUpTab.Equals(TEXT("Single"), ESearchCase::IgnoreCase)
			|| RequestedPowerUpTab.Equals(TEXT("Temporary"), ESearchCase::IgnoreCase)
			|| RequestedPowerUpTab.Equals(TEXT("Steroids"), ESearchCase::IgnoreCase)
			|| RequestedPowerUpTab.Equals(TEXT("Steroid"), ESearchCase::IgnoreCase)
			|| RequestedPowerUpTab.Equals(TEXT("Drugs"), ESearchCase::IgnoreCase)
			|| RequestedPowerUpTab.Equals(TEXT("Drug"), ESearchCase::IgnoreCase);
	}

	if (HasBuiltSlateUI() && !bNeedsWarmActivationRefresh)
	{
		if (bHasRequestedPowerUpTab)
		{
			RequestDeferredSlateRebuild();
		}
		else
		{
			SetShowingSingleUse(bShowingSingleUse);
		}
		return;
	}

	Super::OnScreenActivated_Implementation();
}

UT66LocalizationSubsystem* UT66PowerUpScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

UT66BuffSubsystem* UT66PowerUpScreen::GetBuffSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66BuffSubsystem>();
	}
	return nullptr;
}

void UT66PowerUpScreen::SetShowingSingleUse(bool bInShowingSingleUse)
{
	const bool bChangedTab = bShowingSingleUse != bInShowingSingleUse;
	bShowingSingleUse = bInShowingSingleUse;
	if (PageSwitcher.IsValid())
	{
		PageSwitcher->SetActiveWidgetIndex(bShowingSingleUse ? 1 : 0);
	}
	if (bChangedTab && HasBuiltSlateUI())
	{
		RequestDeferredSlateRebuild();
	}
}

FReply UT66PowerUpScreen::HandleBackClicked()
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		if (Buffs->IsHeroSelectionSingleUseBuffEditActive())
		{
			Buffs->EndHeroSelectionSingleUseBuffEdit();
			NavigateTo(ET66ScreenType::HeroSelection);
			return FReply::Handled();
		}
	}

	NavigateBack();
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandleShowPermanentClicked()
{
	SetShowingSingleUse(false);
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandleShowSingleUseClicked()
{
	SetShowingSingleUse(true);
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandleUnlockClicked(ET66HeroStatType StatType)
{
	UT66BuffSubsystem* Buffs = GetBuffSubsystem();
	if (Buffs && Buffs->UnlockNextFillStep(StatType))
	{
		RefreshScreen();
	}
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandlePurchaseRelicClicked(FName RelicID)
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		if (Buffs->PurchaseRelic(RelicID))
		{
			RefreshScreen();
		}
	}

	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandlePurchaseSingleUseClicked(ET66SecondaryStatType StatType)
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		if (Buffs->PurchaseSingleUseBuff(StatType))
		{
			RefreshScreen();
		}
	}

	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandleHeroSelectionSingleUseClicked(ET66SecondaryStatType StatType)
{
	if (!T66IsLiveSecondaryStatType(StatType))
	{
		return FReply::Handled();
	}

	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		const int32 SlotIndex = Buffs->GetSelectedSingleUseBuffEditSlotIndex();
		const TArray<ET66SecondaryStatType> Slots = Buffs->GetSelectedSingleUseBuffSlots();
		const ET66SecondaryStatType FocusedSlotStat = Slots.IsValidIndex(SlotIndex) ? Slots[SlotIndex] : ET66SecondaryStatType::None;
		const int32 OwnedCount = Buffs->GetOwnedSingleUseBuffCount(StatType);
		const int32 AssignedCount = Buffs->GetSelectedSingleUseBuffSlotAssignedCountForStat(StatType);
		const int32 AssignedOutsideFocused = AssignedCount - (FocusedSlotStat == StatType ? 1 : 0);
		if (FocusedSlotStat == StatType)
		{
			return FReply::Handled();
		}

		if (OwnedCount > AssignedOutsideFocused)
		{
			if (Buffs->SetSelectedSingleUseBuffSlot(SlotIndex, StatType))
			{
				Buffs->EndHeroSelectionSingleUseBuffEdit();
				NavigateTo(ET66ScreenType::HeroSelection);
			}
			return FReply::Handled();
		}

		if (Buffs->PurchaseSingleUseBuff(StatType))
		{
			RefreshScreen();
		}
	}

	return FReply::Handled();
}

void UT66PowerUpScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	bNeedsWarmActivationRefresh = false;
	RequestDeferredSlateRebuild();
}

TSharedRef<SWidget> UT66PowerUpScreen::BuildSlateUI()
{
	bNeedsWarmActivationRefresh = false;

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66BuffSubsystem* Buffs = GetBuffSubsystem();
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	OwnedBrushes.Reset();
	if ((FParse::Param(FCommandLine::Get(), TEXT("T66PowerUpHeroSelectionSteroidEdit"))
		|| FParse::Param(FCommandLine::Get(), TEXT("T66PowerUpHeroSelectionDrugEdit"))) && Buffs)
	{
		Buffs->BeginHeroSelectionSingleUseBuffEdit(Buffs->GetSelectedSingleUseBuffEditSlotIndex());
	}
	const bool bHeroSelectionSingleUseEdit = Buffs && Buffs->IsHeroSelectionSingleUseBuffEditActive();
	const bool bDemoDrugPurchasesBlocked = Buffs && !Buffs->AreSingleUseBuffPurchasesAllowed();
	if (bHeroSelectionSingleUseEdit)
	{
		bShowingSingleUse = true;
	}
	else
	{
		FString RequestedPowerUpTab;
		bool bHasRequestedPowerUpTab = false;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66PowerUpTab="), RequestedPowerUpTab))
		{
			bHasRequestedPowerUpTab = true;
		}
		else
		{
			FString RequestedFrontendScreen;
			if (FParse::Value(FCommandLine::Get(), TEXT("T66FrontendScreen="), RequestedFrontendScreen)
				&& (RequestedFrontendScreen.Equals(TEXT("Relics"), ESearchCase::IgnoreCase)
					|| RequestedFrontendScreen.Equals(TEXT("Steroids"), ESearchCase::IgnoreCase)
					|| RequestedFrontendScreen.Equals(TEXT("Diplomas"), ESearchCase::IgnoreCase)
					|| RequestedFrontendScreen.Equals(TEXT("Drugs"), ESearchCase::IgnoreCase)))
			{
				RequestedPowerUpTab = RequestedFrontendScreen;
				bHasRequestedPowerUpTab = true;
			}
		}

		if (bHasRequestedPowerUpTab)
		{
			bShowingSingleUse =
				RequestedPowerUpTab.Equals(TEXT("SingleUse"), ESearchCase::IgnoreCase)
				|| RequestedPowerUpTab.Equals(TEXT("Single"), ESearchCase::IgnoreCase)
				|| RequestedPowerUpTab.Equals(TEXT("Temporary"), ESearchCase::IgnoreCase)
				|| RequestedPowerUpTab.Equals(TEXT("Steroids"), ESearchCase::IgnoreCase)
				|| RequestedPowerUpTab.Equals(TEXT("Steroid"), ESearchCase::IgnoreCase)
				|| RequestedPowerUpTab.Equals(TEXT("Drugs"), ESearchCase::IgnoreCase)
				|| RequestedPowerUpTab.Equals(TEXT("Drug"), ESearchCase::IgnoreCase);
		}
	}

	const int32 SingleUsePercent = FMath::RoundToInt((UT66BuffSubsystem::SingleUseSecondaryBuffMultiplier - 1.f) * 100.f);
	const FText PermanentTabText = NSLOCTEXT("T66.PowerUp", "PermanentTab", "RELICS (PERMANENT)");
	const FText SingleUseTabText = NSLOCTEXT("T66.PowerUp", "SingleUseTab", "STEROIDS (ONE RUN USE)");
	const FText PermanentHintText = FText::Format(
		NSLOCTEXT("T66.PowerUp", "PermanentHint", "Buy relics for permanent +{0} primary-stat upgrades across every run. Solomon's Ring enables boss-pet capture."),
		FText::AsNumber(ShopRelicStatIncrease));
	const FText SingleUseHintText = FText::Format(
		NSLOCTEXT("T66.PowerUp", "SingleUseHint", "Buy steroids for +{0}% secondary-stat boosts. Owned steroids can be equipped from Hero Selection, up to 4 total per run."),
		FText::AsNumber(SingleUsePercent));
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");

	const int32 Balance = Achievements ? Achievements->GetChadCouponBalance() : (Buffs ? Buffs->GetChadCouponBalance() : 0);

	auto GetStatLabel = [Loc](ET66HeroStatType Stat) -> FText
	{
		if (Loc)
		{
			switch (Stat)
			{
				case ET66HeroStatType::Damage:      return Loc->GetText_Stat_Damage();
				case ET66HeroStatType::AttackSpeed: return Loc->GetText_Stat_AttackSpeed();
				case ET66HeroStatType::AttackScale: return Loc->GetText_Stat_AttackScale();
				case ET66HeroStatType::Accuracy:    return Loc->GetText_Stat_Accuracy();
				case ET66HeroStatType::Armor:       return Loc->GetText_Stat_Armor();
				case ET66HeroStatType::Evasion:     return Loc->GetText_Stat_Evasion();
				case ET66HeroStatType::Luck:        return Loc->GetText_Stat_Luck();
				case ET66HeroStatType::Speed:       return Loc->GetText_Stat_Speed();
				default: break;
			}
		}
		return NSLOCTEXT("T66.PowerUp", "StatUnknown", "?");
	};
	auto GetSecondaryLabel = [Loc](ET66SecondaryStatType StatType) -> FText
	{
		return Loc ? Loc->GetText_SecondaryStatName(StatType) : FText::FromString(TEXT("?"));
	};

	auto GetSingleUseSteroidName = [](ET66SecondaryStatType StatType) -> FText
	{
		switch (StatType)
		{
			case ET66SecondaryStatType::AoeDamage:       return NSLOCTEXT("T66.PowerUp", "Drug_AoeDamage", "OXYMETHOLONE");
			case ET66SecondaryStatType::BounceDamage:    return NSLOCTEXT("T66.PowerUp", "Drug_BounceDamage", "METHANDROSTENOLONE");
			case ET66SecondaryStatType::PierceDamage:    return NSLOCTEXT("T66.PowerUp", "Drug_PierceDamage", "FLUOXYMESTERONE");
			case ET66SecondaryStatType::DotDamage:       return NSLOCTEXT("T66.PowerUp", "Drug_DotDamage", "NANDROLONE DECANOATE");
			case ET66SecondaryStatType::AoeSpeed:        return NSLOCTEXT("T66.PowerUp", "Drug_AoeSpeed", "CAFFEINE CITRATE");
			case ET66SecondaryStatType::BounceSpeed:     return NSLOCTEXT("T66.PowerUp", "Drug_BounceSpeed", "MODAFINIL");
			case ET66SecondaryStatType::PierceSpeed:     return NSLOCTEXT("T66.PowerUp", "Drug_PierceSpeed", "EPHEDRINE HCL");
			case ET66SecondaryStatType::DotSpeed:        return NSLOCTEXT("T66.PowerUp", "Drug_DotSpeed", "SALBUTAMOL SULFATE");
			case ET66SecondaryStatType::AoeScale:        return NSLOCTEXT("T66.PowerUp", "Drug_AoeScale", "TESTOSTERONE ENANTHATE");
			case ET66SecondaryStatType::BounceScale:     return NSLOCTEXT("T66.PowerUp", "Drug_BounceScale", "BOLDENONE UNDECYLENATE");
			case ET66SecondaryStatType::PierceScale:     return NSLOCTEXT("T66.PowerUp", "Drug_PierceScale", "DROSTANOLONE PROPIONATE");
			case ET66SecondaryStatType::DotScale:        return NSLOCTEXT("T66.PowerUp", "Drug_DotScale", "METHENOLONE ENANTHATE");
			case ET66SecondaryStatType::HeadshotChance:  return NSLOCTEXT("T66.PowerUp", "Drug_HeadshotChance", "TRENBOLONE ACETATE");
			case ET66SecondaryStatType::CritChance:      return NSLOCTEXT("T66.PowerUp", "Drug_CritChance", "STANOZOLOL");
			case ET66SecondaryStatType::AttackRange:     return NSLOCTEXT("T66.PowerUp", "Drug_AttackRange", "CLENBUTEROL HCL");
			case ET66SecondaryStatType::Execute:         return NSLOCTEXT("T66.PowerUp", "Drug_Execute", "ATOMOXETINE HCL");
			case ET66SecondaryStatType::Accuracy:        return NSLOCTEXT("T66.PowerUp", "Drug_Accuracy", "ATOMOXETINE HCL");
			case ET66SecondaryStatType::Taunt:           return NSLOCTEXT("T66.PowerUp", "Drug_Taunt", "HYDROCORTISONE");
			case ET66SecondaryStatType::DamageReduction: return NSLOCTEXT("T66.PowerUp", "Drug_DamageReduction", "PREDNISONE");
			case ET66SecondaryStatType::ReflectDamage:   return NSLOCTEXT("T66.PowerUp", "Drug_ReflectDamage", "DEXAMETHASONE");
			case ET66SecondaryStatType::Crush:           return NSLOCTEXT("T66.PowerUp", "Drug_Crush", "BETAMETHASONE");
			case ET66SecondaryStatType::EvasionChance:   return NSLOCTEXT("T66.PowerUp", "Drug_EvasionChance", "SCOPOLAMINE HBR");
			case ET66SecondaryStatType::CounterAttack:   return NSLOCTEXT("T66.PowerUp", "Drug_CounterAttack", "LIDOCAINE HCL");
			case ET66SecondaryStatType::Invisibility:    return NSLOCTEXT("T66.PowerUp", "Drug_Invisibility", "DIPHENHYDRAMINE HCL");
			case ET66SecondaryStatType::Assassinate:     return NSLOCTEXT("T66.PowerUp", "Drug_Assassinate", "ATROPINE SULFATE");
			case ET66SecondaryStatType::InteractableLuck:return NSLOCTEXT("T66.PowerUp", "Steroid_InteractableLuck", "NICOTINAMIDE RIBOSIDE");
			case ET66SecondaryStatType::StealingLuck:    return NSLOCTEXT("T66.PowerUp", "Steroid_StealingLuck", "LOPERAMIDE HCL");
			case ET66SecondaryStatType::GamblingLuck:    return NSLOCTEXT("T66.PowerUp", "Steroid_GamblingLuck", "SILDENAFIL CITRATE");
			case ET66SecondaryStatType::ProcLuck:        return NSLOCTEXT("T66.PowerUp", "Steroid_ProcLuck", "THEOBROMINE");
			case ET66SecondaryStatType::VendorToken:     return NSLOCTEXT("T66.PowerUp", "Drug_VendorToken", "VENDOR TOKEN");
			default:                                     return NSLOCTEXT("T66.PowerUp", "Steroid_Unknown", "COMPOUND");
		}
	};

	auto GetSingleUseEffectText = [&](ET66SecondaryStatType StatType) -> FText
	{
		return FText::Format(
			NSLOCTEXT("T66.PowerUp", "SingleUseEffectFormat", "+{0}% {1}"),
			FText::AsNumber(SingleUsePercent),
			GetSecondaryLabel(StatType));
	};

	auto GetPermanentEffectText = [&](ET66HeroStatType StatType) -> FText
	{
		return FText::Format(
			NSLOCTEXT("T66.PowerUp", "PermanentEffectFormat", "+{0} {1}"),
			FText::AsNumber(ShopRelicStatIncrease),
			GetStatLabel(StatType));
	};

	auto GetSteroidRowTitle = [&](ET66HeroStatType StatType) -> FText
	{
		return FText::Format(
			NSLOCTEXT("T66.PowerUp", "SteroidRowTitle", "{0} Steroids"),
			GetStatLabel(StatType));
	};

	struct FSingleUseRowDef
	{
		ET66HeroStatType PrimaryStat = ET66HeroStatType::Damage;
		TArray<ET66SecondaryStatType> SecondaryStats;
	};

	const TArray<FSingleUseRowDef> SingleUseRows = {
		{ ET66HeroStatType::Damage,      { ET66SecondaryStatType::AoeDamage, ET66SecondaryStatType::BounceDamage, ET66SecondaryStatType::PierceDamage, ET66SecondaryStatType::DotDamage } },
		{ ET66HeroStatType::AttackSpeed, { ET66SecondaryStatType::AoeSpeed, ET66SecondaryStatType::BounceSpeed, ET66SecondaryStatType::PierceSpeed, ET66SecondaryStatType::DotSpeed } },
		{ ET66HeroStatType::AttackScale, { ET66SecondaryStatType::AoeScale, ET66SecondaryStatType::BounceScale, ET66SecondaryStatType::PierceScale, ET66SecondaryStatType::DotScale } },
		{ ET66HeroStatType::Accuracy,    { ET66SecondaryStatType::CritChance, ET66SecondaryStatType::HeadshotChance, ET66SecondaryStatType::AttackRange, ET66SecondaryStatType::Execute } },
		{ ET66HeroStatType::Armor,       { ET66SecondaryStatType::DamageReduction, ET66SecondaryStatType::ReflectDamage, ET66SecondaryStatType::Taunt, ET66SecondaryStatType::Crush } },
		{ ET66HeroStatType::Evasion,     { ET66SecondaryStatType::EvasionChance, ET66SecondaryStatType::CounterAttack, ET66SecondaryStatType::Invisibility, ET66SecondaryStatType::Assassinate } },
		{ ET66HeroStatType::Luck,        { ET66SecondaryStatType::InteractableLuck, ET66SecondaryStatType::StealingLuck, ET66SecondaryStatType::GamblingLuck, ET66SecondaryStatType::ProcLuck } },
	};
	const TArray<FT66RelicDefinition> RelicCardOrder = UT66BuffSubsystem::GetAllRelicDefinitions();
	auto GetRelicEffectText = [&](const FT66RelicDefinition& RelicDef) -> FText
	{
		if (RelicDef.bIsSolomonsRing)
		{
			return NSLOCTEXT("T66.PowerUp", "SolomonsRingEffect", "Enables boss-pet capture");
		}
		if (RelicDef.bUsesSecondaryStat)
		{
			return FText::Format(
				NSLOCTEXT("T66.PowerUp", "RelicSecondaryEffectFormat", "+{0} {1}"),
				FText::AsNumber(RelicDef.BonusStatPoints),
				GetSecondaryLabel(RelicDef.SecondaryStatType));
		}
		return FText::Format(
			NSLOCTEXT("T66.PowerUp", "RelicPrimaryEffectFormat", "+{0} {1}"),
			FText::AsNumber(RelicDef.BonusStatPoints),
			GetStatLabel(RelicDef.PrimaryStatType));
	};

	auto MakePermanentStatPanel = [&](const FT66RelicDefinition& RelicDef) -> TSharedRef<SWidget>
	{
		const int32 Cost = Buffs ? Buffs->GetRelicCost(RelicDef.RelicID) : UT66BuffSubsystem::RelicUnlockCostCC;
		const bool bRelicOwned = Buffs && Buffs->IsRelicOwned(RelicDef.RelicID);
		const bool bRelicActionEnabled = !bRelicOwned && Balance >= Cost;
		const FText ButtonText = bRelicOwned
			? NSLOCTEXT("T66.PowerUp", "RelicOwned", "OWNED")
			: NSLOCTEXT("T66.PowerUp", "BuyRelic", "BUY");
		const FText CostText = FText::AsNumber(Cost);
		const FSlateBrush* RelicBrush = ResolveShopGeneratedBrush(MakePowerUpOwnedPanelPath(TEXT("powerup_diplomas_art_placeholder.png")), FVector2D(244.f, 244.f));
		const FSlateBrush* CouponBrush = ResolveShopGeneratedBrush(MakePowerUpCommonPath(TEXT("Icons"), TEXT("powerup_iconsgenerated_icon_07_coupon_ticket_white_v1.png")), FVector2D(30.f, 24.f));
		const FText RelicTitle = RelicDef.DisplayName.IsEmpty() ? FText::FromName(RelicDef.RelicID) : RelicDef.DisplayName;
		const TSharedRef<SWidget> RelicImageWidget = RelicBrush
			? StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(220.f)
				.HeightOverride(156.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					[
						SNew(SImage)
						.Image(RelicBrush)
					]
				])
			: StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(220.f)
				.HeightOverride(156.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.PowerUp", "MissingRelicArt", "RELIC"))
					.Font(ShopBoldFont(13))
					.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]);

		return MakeShopGeneratedPanel(
			MakePowerUpOwnedPanelPath(TEXT("powerup_panels_upgrade_card_normal.png")),
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBox)
				.WidthOverride(350.f)
				.HeightOverride(42.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					.StretchDirection(EStretchDirection::DownOnly)
					[
						SNew(STextBlock)
						.Text(RelicTitle)
						.Font(ShopBoldFont(16))
						.ColorAndOpacity(T66PowerUpTitleText())
						.Justification(ETextJustify::Center)
						.AutoWrapText(true)
						.WrapTextAt(350.f)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(SBox)
				.WidthOverride(258.f)
				.HeightOverride(186.f)
				[
					MakeShopGeneratedPanel(
						MakePowerUpOwnedPanelPath(TEXT("powerup_panels_item_art_well.png")),
						RelicImageWidget,
						FMargin(6.f),
						T66PowerUpStrokeTint(),
						T66PowerUpInsetFill())
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				SNew(STextBlock)
				.Text(GetRelicEffectText(RelicDef))
				.Font(ShopRegularFont(12))
				.ColorAndOpacity(T66PowerUpText())
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.WrapTextAt(304.f)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNullWidget::NullWidget
			]
			+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Bottom).Padding(16.f, 8.f, 16.f, 18.f)
			[
				T66DemoModeUI::WrapWithComingSoonOverlay(
				MakeShopGeneratedButton(
					FT66ButtonParams(ButtonText, FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandlePurchaseRelicClicked, RelicDef.RelicID), ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetHeight(44.f)
					.SetColor(TAttribute<FSlateColor>::CreateLambda([bRelicOwned, bRelicActionEnabled]() -> FSlateColor
					{
						if (bRelicOwned)
						{
							return FSlateColor(T66PowerUpButtonFill());
						}

						return FSlateColor(bRelicActionEnabled ? T66PowerUpButtonFill() : T66PowerUpButtonDisabledFill());
					}))
					.SetEnabled(TAttribute<bool>(bRelicActionEnabled))
					.SetContent(
						SNew(SBox)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFit)
							.StretchDirection(EStretchDirection::DownOnly)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(ButtonText)
									.Font(ShopBoldFont(15))
									.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
									.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
									.Clipping(EWidgetClipping::ClipToBounds)
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 0.f, 0.f, 0.f)
								[
									SNew(STextBlock)
									.Text(CostText)
									.Font(ShopBoldFont(14))
									.ColorAndOpacity(ShopPermanentCardAccent)
									.Visibility(bRelicOwned ? EVisibility::Collapsed : EVisibility::Visible)
									.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
									.Clipping(EWidgetClipping::ClipToBounds)
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
								[
									SNew(SBox)
									.WidthOverride(24.f)
									.HeightOverride(18.f)
									.Visibility(bRelicOwned || !CouponBrush ? EVisibility::Collapsed : EVisibility::Visible)
									[
										SNew(SImage)
										.Image(CouponBrush)
									]
								]
							]
						])
					,
					ResolveShopDiplomaCompactButtonStyle(),
					ShopBoldFont(15),
					FT66FlatStyle::Tokens::Text,
					FMargin(14.f, 7.f, 14.f, 6.f)
				),
				false,
				this,
				NAME_None)
			],
			FMargin(26.f, 30.f, 26.f, 34.f),
			FLinearColor::White,
			T66PowerUpStrokeTint());
	};

	auto MakeSingleUseSecondaryCard = [&](ET66SecondaryStatType StatType) -> TSharedRef<SWidget>
	{
		const int32 Cost = Buffs ? Buffs->GetSingleUseBuffCost() : UT66BuffSubsystem::SingleUseBuffCostCC;
		const FText CostText = FText::AsNumber(Cost);
		const int32 FocusedSlotIndex = Buffs ? Buffs->GetSelectedSingleUseBuffEditSlotIndex() : 0;
		const TArray<ET66SecondaryStatType> SelectedSlots = Buffs ? Buffs->GetSelectedSingleUseBuffSlots() : TArray<ET66SecondaryStatType>();
		const ET66SecondaryStatType FocusedSlotStat = SelectedSlots.IsValidIndex(FocusedSlotIndex) ? SelectedSlots[FocusedSlotIndex] : ET66SecondaryStatType::None;
		const int32 OwnedCount = Buffs ? Buffs->GetOwnedSingleUseBuffCount(StatType) : 0;
		const int32 AssignedCount = Buffs ? Buffs->GetSelectedSingleUseBuffSlotAssignedCountForStat(StatType) : 0;
		const int32 AssignedOutsideFocused = FMath::Max(0, AssignedCount - (FocusedSlotStat == StatType ? 1 : 0));
		const bool bFocusedSlotMatches = bHeroSelectionSingleUseEdit && FocusedSlotStat == StatType;
		const bool bCanEquipOwnedCopy = bHeroSelectionSingleUseEdit && OwnedCount > AssignedOutsideFocused;
		const bool bUseOwnedCopy = bFocusedSlotMatches || bCanEquipOwnedCopy;
		const bool bShowCost = !bHeroSelectionSingleUseEdit || !bUseOwnedCopy;
		const bool bSingleUsePurchaseBlocked = bDemoDrugPurchasesBlocked && !bUseOwnedCopy;
		const bool bShowPurchaseCost = bShowCost && !bSingleUsePurchaseBlocked;
		const bool bSingleUseActionEnabled = bFocusedSlotMatches ? false : (bUseOwnedCopy ? true : (Balance >= Cost && !bSingleUsePurchaseBlocked));
		const FText SingleUseActionText = bFocusedSlotMatches
			? NSLOCTEXT("T66.PowerUp", "SingleUseEquipped", "EQUIPPED")
			: (bUseOwnedCopy
				? NSLOCTEXT("T66.PowerUp", "SingleUseEquip", "EQUIP")
				: NSLOCTEXT("T66.PowerUp", "BuySingleUse", "BUY"));
		const TSharedPtr<FSlateBrush> IconBrush = GetShopSecondaryBuffBrush(OwnedBrushes, TexPool, this, StatType, FVector2D(124.f, 124.f));
		const FSlateBrush* CouponBrush = ResolveShopGeneratedBrush(MakePowerUpDrugsPath(TEXT("Icons"), TEXT("powerupdrugs_icons_coupon_ticket.png")), FVector2D(26.f, 21.f));
		const TSharedRef<SWidget> IconWidget = IconBrush.IsValid()
			? StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(124.f)
				.HeightOverride(110.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					[
						FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(
							SNew(SImage)
							.Image(IconBrush.Get())),
							NAME_None,
							TEXT("Icon"))
					]
				])
			: StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(124.f)
				.HeightOverride(110.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.PowerUp", "MissingSecondaryArt", "ART"))
					.Font(ShopBoldFont(12))
					.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]);

		return SNew(SBox)
			.HeightOverride(300.f)
			.Padding(FMargin(8.f, 0.f))
			[
				SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(238.f)
						.HeightOverride(48.f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFit)
							.StretchDirection(EStretchDirection::DownOnly)
							[
								SNew(STextBlock)
								.Text(GetSingleUseSteroidName(StatType))
								.Font(ShopBoldFont(17))
								.ColorAndOpacity(T66PowerUpTitleText())
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
								.WrapTextAt(238.f)
								.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
								.Clipping(EWidgetClipping::ClipToBounds)
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 8.f)
					[
						SNew(SBox)
							.WidthOverride(150.f)
							.HeightOverride(112.f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
						[
							IconWidget
						]
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 8.f)
					[
						SNew(SBox)
						.WidthOverride(238.f)
						.HeightOverride(34.f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFit)
							.StretchDirection(EStretchDirection::DownOnly)
							[
								SNew(STextBlock)
								.Text(GetSingleUseEffectText(StatType))
								.Font(ShopRegularFont(14))
								.ColorAndOpacity(T66PowerUpText())
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
								.WrapTextAt(238.f)
								.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
								.Clipping(EWidgetClipping::ClipToBounds)
							]
						]
					]
					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						SNew(SSpacer)
					]
					+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 32.f)
					[
						T66DemoModeUI::WrapWithComingSoonOverlay(
						MakeShopGeneratedButton(
							FT66ButtonParams(
								SingleUseActionText,
								bHeroSelectionSingleUseEdit
									? FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleHeroSelectionSingleUseClicked, StatType)
									: FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandlePurchaseSingleUseClicked, StatType),
								ET66ButtonType::Primary)
							.SetMinWidth(0.f)
							.SetHeight(43.f)
							.SetColor(TAttribute<FSlateColor>::CreateLambda([bSingleUseActionEnabled]() -> FSlateColor
							{
								return FSlateColor(bSingleUseActionEnabled ? T66PowerUpButtonFill() : T66PowerUpButtonDisabledFill());
							}))
							.SetEnabled(TAttribute<bool>(bSingleUseActionEnabled))
							.SetContent(
								SNew(SBox)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SScaleBox)
									.Stretch(EStretch::ScaleToFit)
									.StretchDirection(EStretchDirection::DownOnly)
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock)
											.Text(SingleUseActionText)
											.Font(ShopBoldFont(15))
											.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
											.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
											.Clipping(EWidgetClipping::ClipToBounds)
										]
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16.f, 0.f, 0.f, 0.f)
										[
											SNew(SBox)
											.Visibility(bShowPurchaseCost ? EVisibility::Visible : EVisibility::Collapsed)
											[
												SNew(STextBlock)
												.Text(CostText)
												.Font(ShopBoldFont(15))
												.ColorAndOpacity(ShopPermanentCardAccent)
												.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
												.Clipping(EWidgetClipping::ClipToBounds)
											]
										]
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(7.f, 0.f, 0.f, 0.f)
										[
											SNew(SBox)
											.WidthOverride(26.f)
											.HeightOverride(21.f)
											.Visibility((bShowPurchaseCost && CouponBrush) ? EVisibility::Visible : EVisibility::Collapsed)
											[
												SNew(SImage)
												.Image(CouponBrush)
											]
										]
									]
								])
							,
							ResolveShopDrugsCompactButtonStyle(),
							ShopBoldFont(15),
							FT66FlatStyle::Tokens::Text,
							FMargin(10.f, 5.f, 10.f, 5.f)
						),
						bSingleUsePurchaseBlocked,
						this,
						NAME_None)
					]
			];
	};

	TSharedRef<SGridPanel> RelicGrid = SNew(SGridPanel);
	for (int32 RelicIndex = 0; RelicIndex < RelicCardOrder.Num(); ++RelicIndex)
	{
		const int32 Column = RelicIndex % 4;
		const int32 Row = RelicIndex / 4;
		RelicGrid->AddSlot(Column, Row)
			.Padding(Column > 0 ? ShopCardGap : 0.f, Row > 0 ? ShopCardGap : 0.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(ShopRelicCardWidth)
				.HeightOverride(ShopRelicCardHeight)
				[
					MakePermanentStatPanel(RelicCardOrder[RelicIndex])
				]
			];
	}

	TSharedRef<SWidget> PermanentPage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(-12.f, 0.f, -12.f, 8.f)
		[
			SNew(SBox)
			.HeightOverride(54.f)
			[
				MakeShopGeneratedPanel(
					MakePowerUpOwnedPanelPath(TEXT("powerup_panels_info_strip.png")),
					SNew(SBox)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(PermanentHintText)
						.Font(ShopRegularFont(14))
						.ColorAndOpacity(T66PowerUpText())
						.Justification(ETextJustify::Center)
						.AutoWrapText(true)
						.WrapTextAt(1500.f)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					],
					FMargin(18.f, 7.f),
					FLinearColor::White,
					T66PowerUpInsetFill())
			]
		]
		+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Center).VAlign(VAlign_Top)
		[
			SNew(SScrollBox)
			.ScrollBarStyle(GetShopDrugsScrollBarStyle())
			.ScrollBarVisibility(EVisibility::Visible)
			.ScrollBarThickness(FVector2D(28.f, 28.f))
			.ScrollBarPadding(FMargin(14.f, 0.f, 0.f, 0.f))
			+ SScrollBox::Slot()
			[
				RelicGrid
			]
		];

	TSharedRef<SVerticalBox> SingleUseRowsBox = SNew(SVerticalBox);
	SingleUseRowsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
	[
		SNew(SBox)
		.HeightOverride(72.f)
		[
			bHeroSelectionSingleUseEdit
				? StaticCastSharedRef<SWidget>(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.f, 0.f, 14.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(184.f)
						.HeightOverride(72.f)
						[
							MakeShopGeneratedButton(
								FT66ButtonParams(
									BackText,
									FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleBackClicked),
									ET66ButtonType::Neutral)
								.SetMinWidth(184.f)
								.SetHeight(72.f)
								.SetColor(T66PowerUpNeutralButtonFill()),
								ResolveShopDrugsCompactButtonStyle(),
								ShopBoldFont(20),
								T66PowerUpTabInactiveText(),
								FMargin(20.f, 11.f, 20.f, 9.f))
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						MakeShopGeneratedPanel(
							MakePowerUpStatePanelPath(TEXT("Drugs"), TEXT("powerup_panels_info_strip.png")),
							SNew(SBox)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(SingleUseHintText)
								.Font(ShopRegularFont(16))
								.ColorAndOpacity(T66PowerUpText())
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
								.WrapTextAt(1360.f)
								.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
								.Clipping(EWidgetClipping::ClipToBounds)
							],
							FMargin(22.f, 12.f),
							FLinearColor::White,
							T66PowerUpInsetFill())
					])
				: StaticCastSharedRef<SWidget>(
					MakeShopGeneratedPanel(
						MakePowerUpStatePanelPath(TEXT("Drugs"), TEXT("powerup_panels_info_strip.png")),
						SNew(SBox)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(SingleUseHintText)
							.Font(ShopRegularFont(16))
							.ColorAndOpacity(T66PowerUpText())
							.Justification(ETextJustify::Center)
							.AutoWrapText(true)
							.WrapTextAt(1500.f)
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						],
						FMargin(22.f, 12.f),
						FLinearColor::White,
						T66PowerUpInsetFill()))
		]
	];

	for (int32 RowIndex = 0; RowIndex < SingleUseRows.Num(); ++RowIndex)
	{
		const FSingleUseRowDef& RowDef = SingleUseRows[RowIndex];
		const int32 CardCount = RowDef.SecondaryStats.Num();
		TSharedRef<SHorizontalBox> CardsRow = SNew(SHorizontalBox);
		CardsRow->AddSlot()
			.FillWidth(1.f)
			.Padding(0.f, 0.f, ShopCardGap, 0.f)
			[
				SNew(SBox)
				.Padding(FMargin(16.f, 12.f))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(270.f)
					.HeightOverride(110.f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFit)
						.StretchDirection(EStretchDirection::DownOnly)
						[
							SNew(STextBlock)
							.Text(GetSteroidRowTitle(RowDef.PrimaryStat))
							.Font(ShopBoldFont(28))
							.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
							.Justification(ETextJustify::Center)
							.AutoWrapText(true)
							.WrapTextAt(270.f)
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						]
					]
				]
			];
		for (int32 CardIndex = 0; CardIndex < CardCount; ++CardIndex)
		{
			CardsRow->AddSlot()
				.FillWidth(1.f)
				.Padding(CardIndex < CardCount - 1 ? FMargin(0.f, 0.f, ShopCardGap, 0.f) : FMargin(0.f))
				[
					MakeSingleUseSecondaryCard(RowDef.SecondaryStats[CardIndex])
				];
		}

		SingleUseRowsBox->AddSlot()
			.AutoHeight()
			.Padding(0.f, RowIndex > 0 ? ShopCardGap : 0.f, 0.f, 0.f)
			[
				MakeShopGeneratedPanel(
					MakePowerUpStatePanelPath(TEXT("Drugs"), TEXT("powerup_panels_row_shell_quiet.png")),
					CardsRow,
					FMargin(22.f, 28.f, 22.f, 78.f),
					FLinearColor::White,
					T66PowerUpPanelFill())
			];
	}

	TSharedRef<SWidget> SingleUsePage =
		SNew(SScrollBox)
		.ScrollBarStyle(GetShopDrugsScrollBarStyle())
		.ScrollBarVisibility(EVisibility::Visible)
		.ScrollBarThickness(FVector2D(28.f, 28.f))
		.ScrollBarPadding(FMargin(14.f, 0.f, 0.f, 0.f))
		+ SScrollBox::Slot()
		[
			SingleUseRowsBox
		];

	const FT66FlatStyle::FFrontendChromeMetrics& ChromeMetrics = FT66FlatStyle::GetFrontendChromeMetrics();
	const FSlateFontInfo ChromeTabFont = FT66FlatStyle::MakeFrontendChromeTabFont();
	constexpr float PowerUpTabWidth = 540.f;
	constexpr float PowerUpTabHeight = 78.f;
	auto MakePowerUpTabButton = [&](const FText& TabText, FOnClicked OnClicked, bool bActive, bool bLeft) -> TSharedRef<SWidget>
	{
		return MakeShopGeneratedButton(
			FT66ButtonParams(TabText, MoveTemp(OnClicked), bActive ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
			.SetMinWidth(PowerUpTabWidth)
			.SetHeight(PowerUpTabHeight)
			.SetColor(bActive ? T66PowerUpButtonFill() : T66PowerUpNeutralButtonFill()),
			bShowingSingleUse ? ResolveShopDrugsToggleButtonStyle(bActive, bLeft) : ResolveShopToggleButtonStyle(bActive, bLeft),
			ChromeTabFont,
			bActive ? T66PowerUpTabActiveText() : T66PowerUpTabInactiveText(),
			ChromeMetrics.TabPadding);
	};

	const TSharedRef<SWidget> Header =
		SNew(SBox)
		.HeightOverride(bHeroSelectionSingleUseEdit ? 0.f : 92.f)
		.Visibility(bHeroSelectionSingleUseEdit ? EVisibility::Collapsed : EVisibility::Visible)
		[
			SNew(SHorizontalBox)
			.Visibility(EVisibility::SelfHitTestInvisible)
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNullWidget::NullWidget
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 2.f, 24.f, 12.f)
			[
				MakePowerUpTabButton(
					PermanentTabText,
					FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleShowPermanentClicked),
					!bShowingSingleUse,
					true)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 2.f, 0.f, 12.f)
			[
				MakePowerUpTabButton(
					SingleUseTabText,
					FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleShowSingleUseClicked),
					bShowingSingleUse,
					false)
			]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNullWidget::NullWidget
			]
		];

	const TSharedRef<SWidget> RootPanel =
		SNew(SBox)
		.WidthOverride(ShopReferenceWidth)
		.HeightOverride(bHeroSelectionSingleUseEdit ? ShopHeroSelectionEditReferenceHeight : ShopReferenceHeight)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(bHeroSelectionSingleUseEdit ? FLinearColor::Black : FLinearColor::Transparent)
			.Padding(bHeroSelectionSingleUseEdit ? FMargin(34.f, 16.f, 34.f, 18.f) : FMargin(0.f, 6.f, 0.f, 12.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					Header
				]
				+ SVerticalBox::Slot().FillHeight(1.f)
				[
					SNew(SBox)
					[
						SAssignNew(PageSwitcher, SWidgetSwitcher)
						.WidgetIndex(bShowingSingleUse ? 1 : 0)
						+ SWidgetSwitcher::Slot()[ PermanentPage ]
						+ SWidgetSwitcher::Slot()[ SingleUsePage ]
					]
				]
			]
		];
	const TSharedRef<SWidget> Root =
		SNew(SBox)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				RootPanel
			]
		];

	if (bHeroSelectionSingleUseEdit)
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor::Black)
			]
			+ SOverlay::Slot()
			[
				Root
			];
	}

	return FT66FlatStyle::MakeTopBarScreenRoot(
		UIManager,
		Root,
		SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black),
		FLinearColor::Transparent,
		FMargin(0.f, 0.f, 0.f, -4.f));
}

