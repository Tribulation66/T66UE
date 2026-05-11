// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "UI/Screens/T66SettingsScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RetroFXSubsystem.h"
#include "Core/T66RuntimePlatformSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66PlayerSettingsSaveGame.h"
#include "Gameplay/T66PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/GameUserSettings.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericApplication.h"
#include "Engine/TextureDefines.h"
#include "Engine/Texture2D.h"
#include "Rendering/DrawElements.h"
#include "Styling/SlateTypes.h"
#include "UI/Style/T66ButtonVisuals.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66RuntimeUIFontAccess.h"
#include "UI/Style/T66Style.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace T66SettingsScreenPrivate
{
	constexpr int32 SettingsFontDelta = -2;
	constexpr int32 SettingsCompactButtonFontSize = 19;
	constexpr float SettingsCompactButtonHeight = 46.f;
	enum class ET66SettingsSpriteFamily : uint8
	{
		CompactNeutral,
		TabSelected,
		ToggleOn,
		ToggleOff,
		ToggleInactive
	};

	struct FSettingsSpriteBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
		bool bSimpleFallback = false;
	};

	struct FSettingsSpriteBrushSet
	{
		FSettingsSpriteBrushEntry Normal;
		FSettingsSpriteBrushEntry Hover;
		FSettingsSpriteBrushEntry Pressed;
		FSettingsSpriteBrushEntry Disabled;
	};

	inline const FSlateBrush* ResolveSettingsSpriteBrush(
		FSettingsSpriteBrushEntry& Entry,
		const TCHAR* RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin = FMargin(0.093f, 0.213f, 0.093f, 0.213f),
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		const FString SourceRelativePath(RelativePath ? RelativePath : TEXT(""));
		const bool bImageDraw =
			FMath::IsNearlyZero(Margin.Left)
			&& FMath::IsNearlyZero(Margin.Top)
			&& FMath::IsNearlyZero(Margin.Right)
			&& FMath::IsNearlyZero(Margin.Bottom);
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->DrawAs = bImageDraw ? ESlateBrushDrawType::Image : ESlateBrushDrawType::Box;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
			Entry.Brush->ImageSize = ImageSize;
			Entry.Brush->Margin = Margin;
		}

		if (!Entry.Texture.IsValid() && !Entry.bSimpleFallback)
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(SourceRelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					Filter,
					true,
					TEXT("SettingsMasterLibrarySprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		if (Entry.Texture.IsValid())
		{
			Entry.bSimpleFallback = false;
			Entry.Brush->SetResourceObject(Entry.Texture.Get());
			return Entry.Brush.Get();
		}

		if (T66RuntimeUIBrushAccess::ShouldUseSimpleReferenceFallback(SourceRelativePath))
		{
			Entry.bSimpleFallback = true;
			T66RuntimeUIBrushAccess::ConfigureSimpleReferenceFallbackBrush(
				*Entry.Brush,
				SourceRelativePath,
				ImageSize,
				Margin,
				bImageDraw ? ESlateBrushDrawType::Image : ESlateBrushDrawType::Box);
			return Entry.Brush.Get();
		}

		Entry.bSimpleFallback = false;
		Entry.Brush->SetResourceObject(nullptr);
		return nullptr;
	}

	inline const FSlateBrush* ResolveSettingsRegionSpriteBrush(
		FSettingsSpriteBrushEntry& Entry,
		const TCHAR* RelativePath,
		const FVector2D& ImageSize,
		const FBox2f& UVRegion,
		const FMargin& Margin,
		const ESlateBrushDrawType::Type DrawAs,
		const FLinearColor& Tint,
		const TextureFilter Filter = TextureFilter::TF_Nearest)
	{
		const FSlateBrush* Brush = ResolveSettingsSpriteBrush(Entry, RelativePath, ImageSize, Margin, Filter);
		if (!Brush || !Entry.Brush.IsValid())
		{
			return nullptr;
		}

		if (!Entry.Texture.IsValid() && Entry.bSimpleFallback)
		{
			return Entry.Brush.Get();
		}

		Entry.Brush->DrawAs = DrawAs;
		Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
		Entry.Brush->ImageSize = ImageSize;
		Entry.Brush->Margin = Margin;
		Entry.Brush->TintColor = FSlateColor(Tint);
		Entry.Brush->SetUVRegion(UVRegion);
		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		return Entry.Brush.Get();
	}

	inline FSettingsSpriteBrushSet& GetSettingsButtonSpriteSet(ET66SettingsSpriteFamily Family)
	{
		static FSettingsSpriteBrushSet CompactNeutral;
		static FSettingsSpriteBrushSet TabSelected;
		static FSettingsSpriteBrushSet ToggleOn;
		static FSettingsSpriteBrushSet ToggleOff;
		static FSettingsSpriteBrushSet ToggleInactive;

		switch (Family)
		{
		case ET66SettingsSpriteFamily::TabSelected:
			return TabSelected;
		case ET66SettingsSpriteFamily::ToggleOn:
			return ToggleOn;
		case ET66SettingsSpriteFamily::ToggleOff:
			return ToggleOff;
		case ET66SettingsSpriteFamily::ToggleInactive:
			return ToggleInactive;
		case ET66SettingsSpriteFamily::CompactNeutral:
		default:
			return CompactNeutral;
		}
	}

	inline const TCHAR* GetSettingsButtonSpritePath(ET66SettingsSpriteFamily Family, ET66ButtonBorderState State)
	{
		const TCHAR* Suffix = TEXT("normal");
		switch (State)
		{
		case ET66ButtonBorderState::Hovered:
			Suffix = TEXT("hover");
			break;
		case ET66ButtonBorderState::Pressed:
			Suffix = TEXT("pressed");
			break;
		case ET66ButtonBorderState::Normal:
		default:
			break;
		}
		if (Family == ET66SettingsSpriteFamily::ToggleOn)
		{
			Suffix = TEXT("selected");
		}
		else if (Family == ET66SettingsSpriteFamily::ToggleOff)
		{
			Suffix = State == ET66ButtonBorderState::Hovered ? TEXT("hover") : Suffix;
		}

		static FString Path;
		if (Family == ET66SettingsSpriteFamily::TabSelected)
		{
			Path = TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_normal_red_square_variant.png");
			return *Path;
		}
		if (Family == ET66SettingsSpriteFamily::ToggleOn)
		{
			Path = TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_normal_red_square_variant.png");
			return *Path;
		}
		if (Family == ET66SettingsSpriteFamily::ToggleOff)
		{
			Path = FString::Printf(
				TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_%s_red_square_variant.png"),
				Suffix);
			return *Path;
		}
		Path = FString::Printf(
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_%s_red_square_variant.png"),
			Suffix);
		return *Path;
	}

	inline FVector2D GetSettingsButtonSpriteSize(ET66SettingsSpriteFamily Family, ET66ButtonBorderState State)
	{
		if (Family == ET66SettingsSpriteFamily::CompactNeutral)
		{
			return FVector2D(213.f, 83.f);
		}
		if (Family == ET66SettingsSpriteFamily::TabSelected)
		{
			return FVector2D(221.f, 91.f);
		}
		if (Family == ET66SettingsSpriteFamily::ToggleOn)
		{
			return FVector2D(148.f, 86.f);
		}
		if (Family == ET66SettingsSpriteFamily::ToggleOff)
		{
			return FVector2D(186.f, 91.f);
		}
		return FVector2D(213.f, 83.f);
	}

	inline const FSlateBrush* ResolveSettingsButtonSpriteBrush(ET66SettingsSpriteFamily Family, ET66ButtonBorderState State)
	{
		FSettingsSpriteBrushSet& Set = GetSettingsButtonSpriteSet(Family);
		FSettingsSpriteBrushEntry* Entry = &Set.Normal;
		if (State == ET66ButtonBorderState::Hovered)
		{
			Entry = &Set.Hover;
		}
		else if (State == ET66ButtonBorderState::Pressed)
		{
			Entry = &Set.Pressed;
		}

		return ResolveSettingsSpriteBrush(
			*Entry,
			GetSettingsButtonSpritePath(Family, State),
			GetSettingsButtonSpriteSize(Family, State),
			FMargin(0.f),
			TextureFilter::TF_Nearest);
	}

	inline const FSlateBrush* ResolveSettingsDisabledButtonSpriteBrush()
	{
		FSettingsSpriteBrushSet& Set = GetSettingsButtonSpriteSet(ET66SettingsSpriteFamily::ToggleInactive);
		return ResolveSettingsSpriteBrush(
			Set.Disabled,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_disabled_red_square_variant.png"),
			FVector2D(166.f, 91.f),
			FMargin(0.f),
			TextureFilter::TF_Nearest);
	}

	inline ET66SettingsSpriteFamily GetDefaultSettingsButtonFamily(ET66ButtonType Type)
	{
		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return ET66SettingsSpriteFamily::ToggleOn;
		case ET66ButtonType::Danger:
			return ET66SettingsSpriteFamily::ToggleOff;
		case ET66ButtonType::Neutral:
		case ET66ButtonType::Row:
		default:
			return ET66SettingsSpriteFamily::CompactNeutral;
		}
	}

	inline const FSlateBrush* GetSettingsContentShellBrush()
	{
		static FSettingsSpriteBrushEntry Entry;
		return ResolveSettingsSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/main_panel_normal_square_variant.png"),
			FVector2D(567.f, 379.f),
			FMargin(0.070f, 0.120f, 0.070f, 0.120f),
			TextureFilter::TF_Nearest);
	}

	inline const FSlateBrush* GetSettingsRowShellBrush()
	{
		static FSettingsSpriteBrushEntry Entry;
		return ResolveSettingsSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/main_panel_normal_square_variant.png"),
			FVector2D(550.f, 83.f),
			FMargin(0.075f, 0.275f, 0.075f, 0.275f),
			TextureFilter::TF_Nearest);
	}

	inline const FSlateBrush* GetSettingsDropdownFieldBrush()
	{
		static FSettingsSpriteBrushEntry Entry;
		return ResolveSettingsSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/dropdown_field_normal_square_variant.png"),
			FVector2D(570.f, 71.f),
			FMargin(0.055f, 0.34f, 0.055f, 0.34f),
			TextureFilter::TF_Nearest);
	}

	inline const FSlateBrush* GetSettingsSceneBackgroundBrush()
	{
		return nullptr;
	}

	class ST66SettingsSpriteButton : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66SettingsSpriteButton)
			: _SpriteFamily(ET66SettingsSpriteFamily::CompactNeutral)
			, _MinWidth(0.f)
			, _Height(0.f)
			, _ContentPadding(FMargin(0.f))
			, _IsEnabled(true)
			, _Visibility(EVisibility::Visible)
		{}
			SLATE_ATTRIBUTE(ET66SettingsSpriteFamily, SpriteFamily)
			SLATE_ARGUMENT(float, MinWidth)
			SLATE_ARGUMENT(float, Height)
			SLATE_ARGUMENT(FMargin, ContentPadding)
			SLATE_ARGUMENT(TAttribute<bool>, IsEnabled)
			SLATE_ARGUMENT(TAttribute<EVisibility>, Visibility)
			SLATE_EVENT(FOnClicked, OnClicked)
			SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			SpriteFamily = InArgs._SpriteFamily;
			ContentPadding = InArgs._ContentPadding;
			FButtonStyle ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			ButtonStyle.SetNormalPadding(FMargin(0.f));
			ButtonStyle.SetPressedPadding(FMargin(0.f));
			OwnedButtonStyle = ButtonStyle;

			ChildSlot
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						InArgs._OnClicked,
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							T66ScreenSlateHelpers::MakeReferenceHorizontalSlicedImage(
								TAttribute<const FSlateBrush*>::CreateLambda([this]() -> const FSlateBrush*
								{
									return GetCurrentBrush();
								}),
								FVector2D(1.f, 1.f),
								0.105f)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Padding(this, &ST66SettingsSpriteButton::GetContentPadding)
							[
								InArgs._Content.Widget
							]
						])
					.SetButtonStyle(&OwnedButtonStyle)
					.SetPadding(FMargin(0.f))
					.SetEnabled(InArgs._IsEnabled)
					.SetMinWidth(T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth(InArgs._MinWidth, InArgs._Height))
					.SetHeight(InArgs._Height)
					.SetVisibility(InArgs._Visibility),
					&Button)
			];
		}

	private:
		const FSlateBrush* GetCurrentBrush() const
		{
			if (!Button.IsValid() || !Button->IsEnabled())
			{
				return ResolveSettingsDisabledButtonSpriteBrush();
			}

			const ET66SettingsSpriteFamily Family = SpriteFamily.Get(ET66SettingsSpriteFamily::CompactNeutral);
			if (Button->IsPressed())
			{
				return ResolveSettingsButtonSpriteBrush(Family, ET66ButtonBorderState::Pressed);
			}
			if (Button->IsHovered())
			{
				return ResolveSettingsButtonSpriteBrush(Family, ET66ButtonBorderState::Hovered);
			}
			return ResolveSettingsButtonSpriteBrush(Family, ET66ButtonBorderState::Normal);
		}

		FMargin GetContentPadding() const
		{
			if (Button.IsValid() && Button->IsPressed())
			{
				return FMargin(ContentPadding.Left, ContentPadding.Top + 1.f, ContentPadding.Right, FMath::Max(0.f, ContentPadding.Bottom - 1.f));
			}
			return ContentPadding;
		}

		TAttribute<ET66SettingsSpriteFamily> SpriteFamily;
		FMargin ContentPadding = FMargin(0.f);
		FButtonStyle OwnedButtonStyle;
		TSharedPtr<SButton> Button;
	};

	inline int32 AdjustSettingsFontSize(int32 BaseSize)
	{
		return FMath::Max(8, BaseSize + SettingsFontDelta);
	}

	inline FSlateFontInfo SettingsRegularFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontRegular(AdjustSettingsFontSize(BaseSize));
	}

	inline FSlateFontInfo SettingsBoldFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontBold(AdjustSettingsFontSize(BaseSize));
	}

	inline bool IsBloodyRetroSettingsPreset()
	{
		return T66ScreenSlateHelpers::GetReferenceChromePreset() == T66ScreenSlateHelpers::ET66ReferenceChromePreset::BloodyRetro;
	}

	inline FLinearColor T66SettingsShellFill()
	{
		return FLinearColor(0.018f, 0.014f, 0.010f, 1.0f);
	}

	inline FLinearColor T66SettingsPageFill()
	{
		if (IsBloodyRetroSettingsPreset())
		{
			return FLinearColor(0.018f, 0.012f, 0.010f, 0.98f);
		}
		return FLinearColor::White;
	}

	inline FLinearColor T66SettingsRowFill()
	{
		if (IsBloodyRetroSettingsPreset())
		{
			return FLinearColor(0.030f, 0.020f, 0.016f, 0.94f);
		}
		return FLinearColor(0.94f, 0.94f, 0.91f, 1.0f);
	}

	inline FLinearColor T66SettingsButtonNeutralFill()
	{
		return FT66Style::ButtonNeutral();
	}

	inline FLinearColor T66SettingsButtonOutline()
	{
		return FT66Style::Border();
	}

	inline FLinearColor GetSettingsPageBackground()
	{
		return T66SettingsPageFill();
	}

	inline FLinearColor GetSettingsPageText()
	{
		if (IsBloodyRetroSettingsPreset())
		{
			return FLinearColor(0.94f, 0.89f, 0.78f, 1.0f);
		}
		return FLinearColor(0.045f, 0.040f, 0.034f, 1.0f);
	}

	inline FLinearColor GetSettingsPageMuted()
	{
		if (IsBloodyRetroSettingsPreset())
		{
			return FLinearColor(0.66f, 0.58f, 0.48f, 1.0f);
		}
		return FLinearColor(0.30f, 0.29f, 0.26f, 1.0f);
	}

	inline FLinearColor GetSettingsButtonText()
	{
		if (IsBloodyRetroSettingsPreset())
		{
			return FLinearColor(0.98f, 0.94f, 0.86f, 1.0f);
		}
		return FLinearColor(0.98f, 0.84f, 0.48f, 1.0f);
	}

	inline FLinearColor GetRetroButtonBackground()
	{
		return T66SettingsButtonNeutralFill();
	}

	inline FLinearColor GetRetroButtonSelectedBackground()
	{
		return FT66Style::ButtonPrimary();
	}

	inline FLinearColor GetRetroButtonOutline()
	{
		return T66SettingsButtonOutline();
	}

	inline FLinearColor GetRetroButtonText()
	{
		return GetSettingsButtonText();
	}

	inline FText FormatRetroPercent(float Value)
	{
		const float ClampedValue = FMath::Clamp(Value, 0.0f, 100.0f);
		if (FMath::IsNearlyEqual(ClampedValue, FMath::RoundToFloat(ClampedValue)))
		{
			return FText::AsNumber(FMath::RoundToInt(ClampedValue));
		}
		return FText::FromString(FString::Printf(TEXT("%.2f"), ClampedValue));
	}

	inline FText GetSettingsOnText(UT66LocalizationSubsystem* Loc)
	{
		return Loc ? Loc->GetText_On() : NSLOCTEXT("T66.Settings", "On", "ON");
	}

	inline FText GetSettingsOffText(UT66LocalizationSubsystem* Loc)
	{
		return Loc ? Loc->GetText_Off() : NSLOCTEXT("T66.Settings", "Off", "OFF");
	}

	inline TAttribute<FSlateColor> MakeSelectedButtonColor(
		TFunction<bool()> IsSelected,
		const FSlateColor& SelectedColor = FSlateColor(FT66Style::ButtonPrimary()),
		const FSlateColor& UnselectedColor = FSlateColor(T66SettingsButtonNeutralFill()))
	{
		return TAttribute<FSlateColor>::CreateLambda([IsSelected, SelectedColor, UnselectedColor]() -> FSlateColor
		{
			return IsSelected() ? SelectedColor : UnselectedColor;
		});
	}

	inline TSharedRef<SWidget> MakeSettingsPanel(const TSharedRef<SWidget>& Content, ET66PanelType Type, const FLinearColor& FillColor, const FMargin& Padding)
	{
		(void)Type;
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				FT66Style::MakeRetroUIChromeSurface(
					StaticCastSharedRef<SWidget>(SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FillColor)))
			]
			+ SOverlay::Slot()
			.Padding(Padding)
			[
				Content
			];
	}

	inline TSharedRef<SWidget> MakeSettingsContentShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		return SNew(SOverlay)
			.Clipping(EWidgetClipping::ClipToBounds)
			+ SOverlay::Slot()
			[
				FT66Style::MakeRetroUIChromeSurface(
					StaticCastSharedRef<SWidget>(SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(GetSettingsPageBackground())))
			]
			+ SOverlay::Slot()
			.Padding(Padding)
			[
				Content
			];
	}

	inline TSharedRef<SWidget> MakeSettingsSpriteButton(
		const FT66ButtonParams& Params,
		TAttribute<ET66SettingsSpriteFamily> SpriteFamily)
	{
		const int32 FontSize = Params.FontSize > 0 ? static_cast<int32>(Params.FontSize) : SettingsCompactButtonFontSize;
		const FSlateFontInfo Font = T66RuntimeUIFontAccess::IsBoldWeight(*Params.FontWeight)
			? SettingsBoldFont(FontSize)
			: SettingsRegularFont(FontSize);
		const TAttribute<FText> Text = Params.DynamicLabel.IsBound()
			? Params.DynamicLabel
			: TAttribute<FText>(Params.Label);
		const TAttribute<FSlateColor> TextColor = Params.bHasTextColorOverride
			? Params.TextColorOverride
			: TAttribute<FSlateColor>(FSlateColor(GetSettingsPageText()));
		const FMargin ContentPadding = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(12.f, 6.f);

		const TSharedRef<SWidget> Content = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(
				SNew(STextBlock)
				.Text(Text)
				.Font(Font)
				.ColorAndOpacity(TextColor)
				.ShadowOffset(FVector2D(1.f, 1.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.65f))
				.Justification(ETextJustify::Center)));

		return SNew(ST66SettingsSpriteButton)
			.SpriteFamily(SpriteFamily)
			.MinWidth(Params.MinWidth)
			.Height(Params.Height > 0.f ? Params.Height : SettingsCompactButtonHeight)
			.ContentPadding(ContentPadding)
			.IsEnabled(Params.IsEnabled)
			.Visibility(Params.Visibility)
			.OnClicked(Params.OnClicked)
			[
				Content
			];
	}

	inline TSharedRef<SWidget> MakeSettingsButton(const FT66ButtonParams& Params)
	{
		FT66ButtonParams FlatParams = Params;
		FlatParams
			.SetBorderVisual(ET66ButtonBorderVisual::None)
			.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
			.SetUseGlow(false);

		if (FlatParams.FontSize <= 0)
		{
			FlatParams.SetFontSize(SettingsCompactButtonFontSize);
		}
		else
		{
			FlatParams.SetFontSize(static_cast<int32>(FlatParams.FontSize));
		}
		if (FlatParams.Height <= 0.f)
		{
			FlatParams.SetHeight(SettingsCompactButtonHeight);
		}
		if (FlatParams.Padding.Left < 0.f)
		{
			FlatParams.SetPadding(FMargin(10.f, 5.f));
		}

		if (!FlatParams.bHasColorOverride && FlatParams.Type == ET66ButtonType::Neutral)
		{
			FlatParams.SetColor(T66SettingsButtonNeutralFill());
		}

		return MakeSettingsSpriteButton(
			FlatParams,
			TAttribute<ET66SettingsSpriteFamily>(GetDefaultSettingsButtonFamily(FlatParams.Type)));
	}

	inline TSharedRef<SWidget> MakeSettingsButton(const FText& Label, FOnClicked OnClicked, ET66ButtonType Type = ET66ButtonType::Neutral, float MinWidth = 120.f)
	{
		return MakeSettingsButton(FT66ButtonParams(Label, MoveTemp(OnClicked), Type).SetMinWidth(MinWidth));
	}

	inline TSharedRef<SWidget> MakeSelectableSettingsButton(
		const FT66ButtonParams& Params,
		TFunction<bool()> IsSelected,
		const FSlateColor& SelectedColor = FSlateColor(FT66Style::ButtonPrimary()),
		const FSlateColor& UnselectedColor = FSlateColor(T66SettingsButtonNeutralFill()))
	{
		(void)SelectedColor;
		(void)UnselectedColor;
		FT66ButtonParams ButtonParams = Params;
		const ET66SettingsSpriteFamily SelectedFamily = GetDefaultSettingsButtonFamily(ButtonParams.Type == ET66ButtonType::Neutral ? ET66ButtonType::Primary : ButtonParams.Type);
		const bool bUseSelectedTabArt = ButtonParams.Height >= 58.f;
		TSharedRef<TFunction<bool()>> IsSelectedRef = MakeShared<TFunction<bool()>>(MoveTemp(IsSelected));
		return MakeSettingsSpriteButton(
			ButtonParams,
			TAttribute<ET66SettingsSpriteFamily>::CreateLambda([IsSelectedRef, SelectedFamily, bUseSelectedTabArt]() -> ET66SettingsSpriteFamily
			{
				if (!(*IsSelectedRef)())
				{
					return ET66SettingsSpriteFamily::CompactNeutral;
				}
				return bUseSelectedTabArt ? ET66SettingsSpriteFamily::TabSelected : SelectedFamily;
			}));
	}

	inline TSharedRef<SWidget> MakeSettingsRow(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(26.f, 24.f, 18.f, 24.f))
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				FT66Style::MakeRetroUIChromeSurface(
					StaticCastSharedRef<SWidget>(SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(T66SettingsRowFill())))
			]
			+ SOverlay::Slot()
			.Padding(Padding)
			[
				Content
			];
	}

	struct FSettingsBoolToggleStyle
	{
		float LabelMinWidth = 200.f;
		float ButtonMinWidth = 88.f;
		int32 ButtonFontSize = 0;
		FMargin ButtonPadding = FMargin(10.f, 5.f);
		float RightPadding = 10.f;
		FSlateColor OnSelectedColor = FSlateColor(FT66Style::Tokens::Success);
		FSlateColor OffSelectedColor = FSlateColor(FT66Style::Tokens::Danger);
		FSlateColor UnselectedColor = FSlateColor(T66SettingsButtonNeutralFill());
		FSlateColor TextColor = FSlateColor(GetSettingsButtonText());
	};

	inline TSharedRef<SWidget> MakeSettingsToggleButtons(
		UT66LocalizationSubsystem* Loc,
		TFunction<bool()> GetValue,
		TFunction<void(bool)> SetValue,
		const FSettingsBoolToggleStyle& Style = FSettingsBoolToggleStyle())
	{
		FT66ButtonParams OnButtonParams(GetSettingsOnText(Loc), FOnClicked::CreateLambda([SetValue]()
		{
			SetValue(true);
			return FReply::Handled();
		}), ET66ButtonType::Neutral);
		OnButtonParams
			.SetMinWidth(Style.ButtonMinWidth)
			.SetPadding(Style.ButtonPadding)
			.SetTextColor(TAttribute<FSlateColor>::CreateLambda([Color = Style.TextColor]() -> FSlateColor
			{
				return Color;
			}));
		if (Style.ButtonFontSize > 0)
		{
			OnButtonParams.SetFontSize(Style.ButtonFontSize);
		}

		FT66ButtonParams OffButtonParams(GetSettingsOffText(Loc), FOnClicked::CreateLambda([SetValue]()
		{
			SetValue(false);
			return FReply::Handled();
		}), ET66ButtonType::Neutral);
		OffButtonParams
			.SetMinWidth(Style.ButtonMinWidth)
			.SetPadding(Style.ButtonPadding)
			.SetTextColor(TAttribute<FSlateColor>::CreateLambda([Color = Style.TextColor]() -> FSlateColor
			{
				return Color;
			}));
		if (Style.ButtonFontSize > 0)
		{
			OffButtonParams.SetFontSize(Style.ButtonFontSize);
		}

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeSettingsSpriteButton(
					OnButtonParams,
					TAttribute<ET66SettingsSpriteFamily>::CreateLambda([GetValue]() -> ET66SettingsSpriteFamily
					{
						return GetValue() ? ET66SettingsSpriteFamily::ToggleOn : ET66SettingsSpriteFamily::ToggleInactive;
					}))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f, 0.f, 0.f)
			[
				MakeSettingsSpriteButton(
					OffButtonParams,
					TAttribute<ET66SettingsSpriteFamily>::CreateLambda([GetValue]() -> ET66SettingsSpriteFamily
					{
						return GetValue() ? ET66SettingsSpriteFamily::ToggleInactive : ET66SettingsSpriteFamily::ToggleOff;
					}))
			];
	}

	inline TSharedRef<SWidget> MakeSettingsToggleRow(
		UT66LocalizationSubsystem* Loc,
		const FText& Label,
		TFunction<bool()> GetValue,
		TFunction<void(bool)> SetValue,
		const FText& Description = FText(),
		const FSettingsBoolToggleStyle& Style = FSettingsBoolToggleStyle())
	{
		const bool bHasDescription = !Description.IsEmpty();
		const TSharedRef<SWidget> LabelContent = bHasDescription
			? StaticCastSharedRef<SWidget>(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
					.Text(Label)
					.Font(SettingsRegularFont(22))
					.ColorAndOpacity(GetSettingsPageText())))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 18.f, 0.f)
				[
					FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
					.Text(Description)
					.Font(SettingsRegularFont(16))
					.ColorAndOpacity(GetSettingsPageMuted())
					.AutoWrapText(true)))
				])
			: StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.MinDesiredWidth(Style.LabelMinWidth)
				[
					FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
					.Text(Label)
					.Font(SettingsRegularFont(22))
					.ColorAndOpacity(GetSettingsPageText())))
				]);

		return MakeSettingsRow(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				LabelContent
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(Style.RightPadding, 0.f, 0.f, 0.f)
			[
				MakeSettingsToggleButtons(Loc, MoveTemp(GetValue), MoveTemp(SetValue), Style)
			],
			FMargin(24.f, 14.f, 18.f, 12.f));
	}

	inline const FSlateBrush* GetSettingsSectionDividerBrush()
	{
		static FSettingsSpriteBrushEntry Entry;
		return ResolveSettingsSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/Settings/Dividers/settings_dividers_section_heading_rule.png"),
			FVector2D(1003.f, 126.f),
			FMargin(0.f),
			TextureFilter::TF_Nearest);
	}

	inline TSharedRef<SWidget> MakeSettingsSectionHeader(const FText& Text, const int32 FontSize = 22)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
				.Text(Text)
				.Font(SettingsBoldFont(FontSize))
				.ColorAndOpacity(GetSettingsPageText())))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 1.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(145.f)
				.HeightOverride(18.f)
				[
					FT66Style::MakeRetroUIChromeSurface(StaticCastSharedRef<SWidget>(
						SNew(SImage)
						.Image(GetSettingsSectionDividerBrush())))
				]
			];
	}

	inline TSharedRef<SWidget> MakeDynamicOnOffButton(
		UT66LocalizationSubsystem* Loc,
		TFunction<bool()> GetValue,
		FOnClicked OnClicked)
	{
		FT66ButtonParams ButtonParams(FText::GetEmpty(), MoveTemp(OnClicked), ET66ButtonType::Neutral);
		ButtonParams
			.SetMinWidth(0.f)
			.SetDynamicLabel(TAttribute<FText>::CreateLambda([Loc, GetValue]() -> FText
			{
				return GetValue() ? GetSettingsOnText(Loc) : GetSettingsOffText(Loc);
			}));

		return MakeSettingsSpriteButton(
			ButtonParams,
			TAttribute<ET66SettingsSpriteFamily>::CreateLambda([GetValue]() -> ET66SettingsSpriteFamily
			{
				return GetValue() ? ET66SettingsSpriteFamily::ToggleOn : ET66SettingsSpriteFamily::ToggleOff;
			}));
	}

	inline const FSlateBrush* GetSettingsReferenceSliderThumbBrush()
	{
		static FSettingsSpriteBrushEntry Entry;
		return ResolveSettingsRegionSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/Settings/Controls/settings_controls_controls_sheet.png"),
			FVector2D(37.f, 37.f),
			FBox2f(
				FVector2f(24.f / 1536.f, 849.f / 1024.f),
				FVector2f(61.f / 1536.f, 886.f / 1024.f)),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			FLinearColor::White,
			TextureFilter::TF_Nearest);
	}

	inline FVector2D GetSettingsBrushSourceSize(const FSlateBrush* SourceBrush)
	{
		if (!SourceBrush)
		{
			return FVector2D(1.f, 1.f);
		}

		if (const UTexture2D* SourceTexture = Cast<UTexture2D>(SourceBrush->GetResourceObject()))
		{
			return FVector2D(
				FMath::Max(1, SourceTexture->GetSizeX()),
				FMath::Max(1, SourceTexture->GetSizeY()));
		}

		return FVector2D(
			FMath::Max(1.f, SourceBrush->ImageSize.X),
			FMath::Max(1.f, SourceBrush->ImageSize.Y));
	}

	inline void PaintSettingsImageRegion(
		const FSlateBrush* SourceBrush,
		const FBox2f& SourceUV,
		const FVector2D& Position,
		const FVector2D& Size,
		const FGeometry& AllottedGeometry,
		FSlateWindowElementList& OutDrawElements,
		const int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		const FLinearColor& Tint)
	{
		if (!SourceBrush || Size.X <= 0.5f || Size.Y <= 0.5f)
		{
			return;
		}

		FSlateBrush LocalBrush = *SourceBrush;
		LocalBrush.DrawAs = ESlateBrushDrawType::Image;
		LocalBrush.Tiling = ESlateBrushTileType::NoTile;
		LocalBrush.Margin = FMargin(0.f);
		LocalBrush.SetUVRegion(SourceUV);

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(
				FVector2f(Size),
				FSlateLayoutTransform(FVector2f(Position))),
			&LocalBrush,
			ESlateDrawEffect::None,
			InWidgetStyle.GetColorAndOpacityTint() * Tint);
	}

	inline void PaintSettingsHorizontalSlicedRegion(
		const FSlateBrush* SourceBrush,
		const FBox2f& SourceUV,
		const FVector2D& Position,
		const FVector2D& Size,
		const float SourceCapFraction,
		const FGeometry& AllottedGeometry,
		FSlateWindowElementList& OutDrawElements,
		const int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		const FLinearColor& Tint)
	{
		if (!SourceBrush || Size.X <= 1.f || Size.Y <= 1.f)
		{
			return;
		}

		const FVector2D FullSourceSize = GetSettingsBrushSourceSize(SourceBrush);
		const FVector2D RegionSourceSize(
			FMath::Max(1.f, static_cast<float>(SourceUV.GetSize().X) * FullSourceSize.X),
			FMath::Max(1.f, static_cast<float>(SourceUV.GetSize().Y) * FullSourceSize.Y));
		const float CapU = FMath::Clamp(SourceCapFraction, 0.02f, 0.45f);
		const float HeightScale = Size.Y / RegionSourceSize.Y;
		const float SourceCapWidth = RegionSourceSize.X * CapU;
		const float DestCapWidth = FMath::Clamp(SourceCapWidth * HeightScale, 1.f, Size.X * 0.42f);
		const float DestCenterWidth = FMath::Max(0.f, Size.X - (DestCapWidth * 2.f));

		auto DrawSlice = [&](const FVector2D& SlicePos, const FVector2D& SliceSize, const float LocalU0, const float LocalU1)
		{
			if (SliceSize.X <= 0.5f || SliceSize.Y <= 0.5f || LocalU1 <= LocalU0)
			{
				return;
			}

			const float U0 = FMath::Lerp(SourceUV.Min.X, SourceUV.Max.X, LocalU0);
			const float U1 = FMath::Lerp(SourceUV.Min.X, SourceUV.Max.X, LocalU1);
			PaintSettingsImageRegion(
				SourceBrush,
				FBox2f(FVector2f(U0, SourceUV.Min.Y), FVector2f(U1, SourceUV.Max.Y)),
				Position + SlicePos,
				SliceSize,
				AllottedGeometry,
				OutDrawElements,
				LayerId,
				InWidgetStyle,
				Tint);
		};

		DrawSlice(FVector2D(0.f, 0.f), FVector2D(DestCapWidth, Size.Y), 0.f, CapU);
		DrawSlice(FVector2D(DestCapWidth, 0.f), FVector2D(DestCenterWidth, Size.Y), CapU, 1.f - CapU);
		DrawSlice(FVector2D(Size.X - DestCapWidth, 0.f), FVector2D(DestCapWidth, Size.Y), 1.f - CapU, 1.f);
	}

	inline const FSlateBrush* GetSettingsReferenceProgressSheetBrush()
	{
		static FSettingsSpriteBrushEntry Entry;
		const FSlateBrush* Brush = ResolveSettingsSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/Settings/Controls/settings_controls_reference_progress_meter_sheet.png"),
			FVector2D(1536.f, 1024.f),
			FMargin(0.f),
			TextureFilter::TF_Nearest);
		return Entry.bSimpleFallback ? nullptr : Brush;
	}

	class ST66SettingsReferenceProgressBar : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66SettingsReferenceProgressBar)
			: _Percent(TOptional<float>(0.f))
			, _DesiredSize(FVector2D(180.f, 18.f))
			, _FallbackFill(FLinearColor(0.10f, 0.64f, 0.96f, 1.f))
			, _Padding(FMargin(4.f, 3.f))
		{}
			SLATE_ATTRIBUTE(TOptional<float>, Percent)
			SLATE_ARGUMENT(FVector2D, DesiredSize)
			SLATE_ARGUMENT(FLinearColor, FallbackFill)
			SLATE_ARGUMENT(FMargin, Padding)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			Percent = InArgs._Percent;
			DesiredSize = InArgs._DesiredSize;
			FallbackFill = InArgs._FallbackFill;
			Padding = InArgs._Padding;
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return DesiredSize;
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FVector2D Size = AllottedGeometry.GetLocalSize();
			if (Size.X <= 1.f || Size.Y <= 1.f)
			{
				return LayerId;
			}

			const FSlateBrush* ProgressSheetBrush = GetSettingsReferenceProgressSheetBrush();
			const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
			const FBox2f TrackUV(FVector2f(24.f / 1536.f, 849.f / 1024.f), FVector2f(501.f / 1536.f, 885.f / 1024.f));
			const FBox2f FillUV(FVector2f(24.f / 1536.f, 890.f / 1024.f), FVector2f(501.f / 1536.f, 945.f / 1024.f));

			if (ProgressSheetBrush)
			{
				PaintSettingsHorizontalSlicedRegion(
					ProgressSheetBrush,
					TrackUV,
					FVector2D::ZeroVector,
					Size,
					0.058f,
					AllottedGeometry,
					OutDrawElements,
					LayerId,
					InWidgetStyle,
					FLinearColor::White);
			}
			else
			{
				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId,
					AllottedGeometry.ToPaintGeometry(FVector2f(Size), FSlateLayoutTransform()),
					WhiteBrush,
					ESlateDrawEffect::None,
					InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(0.02f, 0.02f, 0.04f, 1.f));
			}

			const float Pct = FMath::Clamp(Percent.Get(TOptional<float>(0.f)).Get(0.f), 0.f, 1.f);
			if (Pct <= 0.001f)
			{
				return LayerId + 1;
			}

			const FVector2D InnerPos(Padding.Left, Padding.Top);
			const FVector2D InnerSize(
				FMath::Max(0.f, Size.X - Padding.Left - Padding.Right),
				FMath::Max(0.f, Size.Y - Padding.Top - Padding.Bottom));
			const FVector2D FillSize(FMath::Max(1.f, InnerSize.X * Pct), InnerSize.Y);
			if (FillSize.X <= 0.f || FillSize.Y <= 0.f)
			{
				return LayerId + 1;
			}

			if (ProgressSheetBrush)
			{
				const float FillUMax = FMath::Lerp(FillUV.Min.X, FillUV.Max.X, Pct);
				PaintSettingsImageRegion(
					ProgressSheetBrush,
					FBox2f(FillUV.Min, FVector2f(FillUMax, FillUV.Max.Y)),
					InnerPos,
					FillSize,
					AllottedGeometry,
					OutDrawElements,
					LayerId + 1,
					InWidgetStyle,
					FLinearColor::White);
			}
			else
			{
				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 1,
					AllottedGeometry.ToPaintGeometry(FVector2f(FillSize), FSlateLayoutTransform(FVector2f(InnerPos))),
					WhiteBrush,
					ESlateDrawEffect::None,
					InWidgetStyle.GetColorAndOpacityTint() * FallbackFill);
			}

			return LayerId + 2;
		}

	private:
		TAttribute<TOptional<float>> Percent;
		FVector2D DesiredSize = FVector2D(180.f, 18.f);
		FLinearColor FallbackFill = FLinearColor(0.10f, 0.64f, 0.96f, 1.f);
		FMargin Padding = FMargin(4.f, 3.f);
	};

	inline TSharedRef<SWidget> MakeSettingsReferenceProgressBar(
		TAttribute<TOptional<float>> Percent,
		const FVector2D& DesiredSize,
		const FLinearColor& FallbackFill = FLinearColor(0.10f, 0.64f, 0.96f, 1.f),
		const FMargin& Padding = FMargin(4.f, 3.f))
	{
		return SNew(ST66SettingsReferenceProgressBar)
			.Percent(Percent)
			.DesiredSize(DesiredSize)
			.FallbackFill(FallbackFill)
			.Padding(Padding);
	}

	inline const FScrollBarStyle* GetSettingsReferenceScrollBarStyle()
	{
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		static FSettingsSpriteBrushEntry TrackEntry;
		static FSettingsSpriteBrushEntry ThumbEntry;
		static FSettingsSpriteBrushEntry HoverEntry;
		static FSettingsSpriteBrushEntry UpEntry;
		static FSettingsSpriteBrushEntry DownEntry;

		const FSlateBrush* TrackBrush = ResolveSettingsSpriteBrush(
			TrackEntry,
			TEXT("SourceAssets/UI/Reference/Screens/Settings/Controls/settings_controls_scrollbar_track_generated.png"),
			FVector2D(33.f, 445.f),
			FMargin(0.34f, 0.11f, 0.34f, 0.11f),
			TextureFilter::TF_Nearest);
		const FSlateBrush* ThumbBrush = ResolveSettingsSpriteBrush(
			ThumbEntry,
			TEXT("SourceAssets/UI/Reference/Screens/Settings/Controls/settings_controls_scrollbar_thumb_generated.png"),
			FVector2D(37.f, 444.f),
			FMargin(0.34f, 0.16f, 0.34f, 0.16f),
			TextureFilter::TF_Nearest);
		const FSlateBrush* HoverBrush = ResolveSettingsSpriteBrush(
			HoverEntry,
			TEXT("SourceAssets/UI/Reference/Screens/Settings/Controls/settings_controls_scrollbar_thumb_generated.png"),
			FVector2D(37.f, 444.f),
			FMargin(0.34f, 0.16f, 0.34f, 0.16f),
			TextureFilter::TF_Nearest);
		const FSlateBrush* UpBrush = ResolveSettingsSpriteBrush(
			UpEntry,
			TEXT("SourceAssets/UI/Reference/Screens/Settings/Controls/settings_controls_scrollbar_arrow_up.png"),
			FVector2D(71.f, 71.f),
			FMargin(0.f),
			TextureFilter::TF_Nearest);
		const FSlateBrush* DownBrush = ResolveSettingsSpriteBrush(
			DownEntry,
			TEXT("SourceAssets/UI/Reference/Screens/Settings/Controls/settings_controls_scrollbar_arrow_down.png"),
			FVector2D(71.f, 71.f),
			FMargin(0.f),
			TextureFilter::TF_Nearest);

		if (TrackBrush && ThumbBrush && HoverBrush && UpBrush && DownBrush
			&& !TrackEntry.bSimpleFallback
			&& !ThumbEntry.bSimpleFallback
			&& !HoverEntry.bSimpleFallback
			&& !UpEntry.bSimpleFallback
			&& !DownEntry.bSimpleFallback)
		{
			Style
				.SetVerticalBackgroundImage(*TrackBrush)
				.SetVerticalTopSlotImage(*UpBrush)
				.SetVerticalBottomSlotImage(*DownBrush)
				.SetNormalThumbImage(*ThumbBrush)
				.SetHoveredThumbImage(*HoverBrush)
				.SetDraggedThumbImage(*HoverBrush)
				.SetThickness(18.f);
		}

		return &Style;
	}

	inline const FSliderStyle& GetSettingsReferenceSliderHitTargetStyle()
	{
		static FSliderStyle Style;
		static bool bInitialized = false;
		if (!bInitialized)
		{
			const FSlateBrush& NoBrush = *FCoreStyle::Get().GetBrush("NoBrush");
			const FSlateBrush* ThumbBrush = GetSettingsReferenceSliderThumbBrush();
			Style
				.SetNormalBarImage(NoBrush)
				.SetHoveredBarImage(NoBrush)
				.SetDisabledBarImage(NoBrush)
				.SetNormalThumbImage(ThumbBrush ? *ThumbBrush : NoBrush)
				.SetHoveredThumbImage(ThumbBrush ? *ThumbBrush : NoBrush)
				.SetDisabledThumbImage(ThumbBrush ? *ThumbBrush : NoBrush)
				.SetBarThickness(0.f);
			bInitialized = true;
		}
		return Style;
	}

	inline TSharedRef<SWidget> MakeSettingsReferenceSlider(
		TAttribute<float> Value,
		float StepSize,
		FOnFloatValueChanged OnValueChanged,
		FSimpleDelegate OnMouseCaptureEnd = FSimpleDelegate(),
		FSimpleDelegate OnControllerCaptureEnd = FSimpleDelegate(),
		bool bMouseUsesStep = false)
	{
		const TAttribute<TOptional<float>> ProgressPercent = TAttribute<TOptional<float>>::CreateLambda([Value]() -> TOptional<float>
		{
			return TOptional<float>(FMath::Clamp(Value.Get(0.f), 0.f, 1.f));
		});

		return SNew(SBox)
			.HeightOverride(34.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					MakeSettingsReferenceProgressBar(
						ProgressPercent,
						FVector2D(260.f, 22.f),
						FLinearColor(0.10f, 0.64f, 0.96f, 1.0f),
						FMargin(5.f, 4.f))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SSlider)
					.Style(&GetSettingsReferenceSliderHitTargetStyle())
					.Value(Value)
					.StepSize(StepSize)
					.IndentHandle(false)
					.MouseUsesStep(bMouseUsesStep)
					.SliderBarColor(FLinearColor::Transparent)
					.SliderHandleColor(FLinearColor::White)
					.OnValueChanged(OnValueChanged)
					.OnMouseCaptureEnd(OnMouseCaptureEnd)
					.OnControllerCaptureEnd(OnControllerCaptureEnd)
				]
			];
	}

	inline TSharedRef<SWidget> MakeSettingsPercentSliderRow(
		const FText& Label,
		const FText& Description,
		TFunction<float()> GetPercent,
		TFunction<void(float)> SetPercent,
		const FText& HelpText)
	{
		auto GetSnappedPercent = [GetPercent]()
		{
			return FMath::Clamp(FMath::RoundToInt(GetPercent()), 0, 100);
		};

		auto CommitSliderValue = [SetPercent](float NormalizedValue)
		{
			const int32 SnappedPercent = FMath::Clamp(FMath::RoundToInt(NormalizedValue * 100.0f), 0, 100);
			SetPercent(static_cast<float>(SnappedPercent));
		};

		return SNew(SBox)
			.HeightOverride(104.f)
			[
				MakeSettingsRow(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(0.46f).VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
							.Text(Label)
							.Font(SettingsBoldFont(20))
							.ColorAndOpacity(GetSettingsPageText())))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 18.f, 0.f)
						[
							FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
							.Text(Description)
							.Font(SettingsRegularFont(14))
							.ColorAndOpacity(GetSettingsPageMuted())
							.AutoWrapText(true)))
						]
					]
					+ SHorizontalBox::Slot().FillWidth(0.54f).VAlign(VAlign_Center).Padding(10.f, 0.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
							.Text_Lambda([GetSnappedPercent]()
							{
								return FText::AsNumber(GetSnappedPercent());
							})
							.Font(SettingsBoldFont(22))
							.ColorAndOpacity(GetSettingsPageText())
							.Justification(ETextJustify::Center)))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
						[
							MakeSettingsReferenceSlider(
								TAttribute<float>::CreateLambda([GetSnappedPercent]() -> float
								{
									return static_cast<float>(GetSnappedPercent()) / 100.0f;
								}),
								0.01f,
								FOnFloatValueChanged::CreateLambda([CommitSliderValue](float Value)
								{
									CommitSliderValue(Value);
								}))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
						[
							FT66Style::MakeRetroUIText(StaticCastSharedRef<SWidget>(SNew(STextBlock)
							.Text(HelpText)
							.Font(SettingsRegularFont(12))
							.ColorAndOpacity(GetSettingsPageMuted())
							.AutoWrapText(true)))
						]
					],
					FMargin(22.f, 8.f, 16.f, 8.f))
			];
	}

	inline TSharedRef<SWidget> MakeSettingsDropdown(const FT66DropdownParams& Params);

	inline TSharedRef<SWidget> MakeSettingsDropdownRow(
		const FText& Label,
		TFunction<FText()> GetCurrentValue,
		TFunction<TSharedRef<SWidget>(const TSharedPtr<STextBlock>&)> MakeMenuContent,
		float LabelFillWidth = 0.4f,
		float ValueFillWidth = 0.6f,
		bool bUseEllipsis = false)
	{
		const TSharedRef<STextBlock> CurrentValueWidget = SNew(STextBlock)
			.Text(GetCurrentValue())
			.Font(SettingsRegularFont(18))
			.ColorAndOpacity(GetSettingsPageText());
		if (bUseEllipsis)
		{
			CurrentValueWidget->SetOverflowPolicy(ETextOverflowPolicy::Ellipsis);
		}

		const TSharedPtr<STextBlock> CurrentValueText = CurrentValueWidget;
		const TSharedRef<SWidget> TriggerContent =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(FMargin(10.f, 4.f))
			[
				CurrentValueWidget
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(4.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(26.f)
				[
					SNew(SSpacer)
				]
			];

		return MakeSettingsRow(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(LabelFillWidth).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(SettingsRegularFont(22))
				.ColorAndOpacity(GetSettingsPageText())
			]
			+ SHorizontalBox::Slot().FillWidth(ValueFillWidth)
			[
				MakeSettingsDropdown(FT66DropdownParams(
					TriggerContent,
					[MakeMenuContent, CurrentValueText]()
					{
						return MakeMenuContent(CurrentValueText);
					}).SetHeight(0.f))
			]);
	}

	struct FSettingsSliderRowParams
	{
		FText Label;
		float InitialValue = 0.f;
		TFunction<FText(float)> FormatValueText;
		TFunction<void(float)> OnValueChanged;
		TFunction<void()> OnMouseCaptureEnd;
		TFunction<void()> OnControllerCaptureEnd;
		FText HelpText;
		float LabelFillWidth = 0.3f;
		float SliderFillWidth = 0.55f;
		float ValueFillWidth = 0.15f;
		int32 LabelFontSize = 22;
		int32 ValueFontSize = 26;
		int32 HelpFontSize = 16;
	};

	inline TSharedRef<SWidget> MakeSettingsSliderRow(const FSettingsSliderRowParams& Params)
	{
		const TSharedRef<TSharedPtr<STextBlock>> ValueText = MakeShared<TSharedPtr<STextBlock>>();
		const TSharedRef<float> SliderValue = MakeShared<float>(FMath::Clamp(Params.InitialValue, 0.f, 1.f));
		TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);

		Content->AddSlot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(Params.LabelFillWidth).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Params.Label)
				.Font(SettingsRegularFont(Params.LabelFontSize))
				.ColorAndOpacity(GetSettingsPageText())
			]
			+ SHorizontalBox::Slot().FillWidth(Params.SliderFillWidth).VAlign(VAlign_Center).Padding(10.f, 0.f)
			[
				MakeSettingsReferenceSlider(
					TAttribute<float>::CreateLambda([SliderValue]() -> float
					{
						return FMath::Clamp(*SliderValue, 0.f, 1.f);
					}),
					0.01f,
					FOnFloatValueChanged::CreateLambda([Params, ValueText, SliderValue](float Value)
					{
						*SliderValue = FMath::Clamp(Value, 0.f, 1.f);
						if (Params.OnValueChanged)
						{
							Params.OnValueChanged(Value);
						}
						if ((*ValueText).IsValid())
						{
							(*ValueText)->SetText(Params.FormatValueText ? Params.FormatValueText(Value) : FText::GetEmpty());
						}
					}),
					FSimpleDelegate::CreateLambda([Params]()
					{
						if (Params.OnMouseCaptureEnd)
						{
							Params.OnMouseCaptureEnd();
						}
					}),
					FSimpleDelegate::CreateLambda([Params]()
					{
						if (Params.OnControllerCaptureEnd)
						{
							Params.OnControllerCaptureEnd();
						}
					}))
			]
			+ SHorizontalBox::Slot().FillWidth(Params.ValueFillWidth).VAlign(VAlign_Center)
			[
				SAssignNew(*ValueText, STextBlock)
				.Text(Params.FormatValueText ? Params.FormatValueText(Params.InitialValue) : FText::GetEmpty())
				.Font(SettingsRegularFont(Params.ValueFontSize))
				.ColorAndOpacity(GetSettingsPageText())
				.Justification(ETextJustify::Right)
			]
		];

		if (!Params.HelpText.IsEmpty())
		{
			Content->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(Params.HelpText)
				.Font(SettingsRegularFont(Params.HelpFontSize))
				.ColorAndOpacity(GetSettingsPageMuted())
				.AutoWrapText(true)
			];
		}

		return MakeSettingsRow(Content);
	}

	inline TSharedRef<SWidget> MakeSettingsDropdown(const FT66DropdownParams& Params)
	{
		static FComboButtonStyle FlatComboStyle = []()
		{
			FComboButtonStyle Style = FT66Style::GetDropdownComboButtonStyle();
			Style.ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			return Style;
		}();

		TSharedRef<SComboButton> Combo = SNew(SComboButton)
			.ComboButtonStyle(&FlatComboStyle)
			.OnGetMenuContent_Lambda([OnGet = Params.OnGetMenuContent]()
			{
				return MakeSettingsPanel(OnGet(), ET66PanelType::Panel2, T66SettingsRowFill(), FMargin(0.f));
			})
			.ContentPadding(Params.Padding)
			.ButtonContent()
			[
				Params.Content
			];

		return SNew(SBox)
			.MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
			.HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize())
			.Visibility(Params.Visibility)
			[
				SNew(SBorder)
				.BorderImage(GetSettingsDropdownFieldBrush())
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(FMargin(4.f, 2.f))
				[
					Combo
				]
			];
	}
}
