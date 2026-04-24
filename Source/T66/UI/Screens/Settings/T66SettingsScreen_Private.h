// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "UI/Screens/T66SettingsScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RetroFXSubsystem.h"
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
#include "Misc/DefaultValueHelper.h"
#include "UI/Style/T66ButtonVisuals.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66RuntimeUIFontAccess.h"
#include "UI/Style/T66Style.h"
#include "Engine/Texture2D.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace T66SettingsScreenPrivate
{
	constexpr int32 SettingsFontDelta = -10;
	constexpr int32 SettingsCompactButtonFontSize = 18;
	constexpr float SettingsCompactButtonHeight = 44.f;

	enum class ET66SettingsSpriteFamily : uint8
	{
		CompactNeutral,
		ToggleOn,
		ToggleOff,
		ToggleInactive
	};

	struct FSettingsSpriteBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
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
		const FMargin& Margin = FMargin(0.16f, 0.28f, 0.16f, 0.28f))
	{
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->DrawAs = ESlateBrushDrawType::Box;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
			Entry.Brush->ImageSize = ImageSize;
			Entry.Brush->Margin = Margin;
		}

		if (!Entry.Texture.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					true,
					TEXT("SettingsReferenceSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		return Entry.Texture.IsValid() ? Entry.Brush.Get() : nullptr;
	}

	inline FSettingsSpriteBrushSet& GetSettingsButtonSpriteSet(ET66SettingsSpriteFamily Family)
	{
		static FSettingsSpriteBrushSet CompactNeutral;
		static FSettingsSpriteBrushSet ToggleOn;
		static FSettingsSpriteBrushSet ToggleOff;
		static FSettingsSpriteBrushSet ToggleInactive;

		switch (Family)
		{
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
		const TCHAR* Prefix = TEXT("settings_compact_neutral");
		switch (Family)
		{
		case ET66SettingsSpriteFamily::ToggleOn:
			Prefix = TEXT("settings_toggle_on");
			break;
		case ET66SettingsSpriteFamily::ToggleOff:
			Prefix = TEXT("settings_toggle_off");
			break;
		case ET66SettingsSpriteFamily::ToggleInactive:
			Prefix = TEXT("settings_toggle_inactive");
			break;
		case ET66SettingsSpriteFamily::CompactNeutral:
		default:
			break;
		}

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

		static FString Path;
		Path = FString::Printf(TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/%s_%s.png"), Prefix, Suffix);
		return *Path;
	}

	inline FVector2D GetSettingsButtonSpriteSize(ET66SettingsSpriteFamily Family, ET66ButtonBorderState State)
	{
		if (Family == ET66SettingsSpriteFamily::CompactNeutral)
		{
			return State == ET66ButtonBorderState::Pressed ? FVector2D(186.f, 68.f) : FVector2D(180.f, 68.f);
		}
		if (Family == ET66SettingsSpriteFamily::ToggleOn)
		{
			return State == ET66ButtonBorderState::Pressed ? FVector2D(187.f, 67.f) : FVector2D(180.f, 68.f);
		}
		if (Family == ET66SettingsSpriteFamily::ToggleOff)
		{
			return State == ET66ButtonBorderState::Pressed ? FVector2D(186.f, 68.f) : FVector2D(180.f, 68.f);
		}
		return State == ET66ButtonBorderState::Hovered ? FVector2D(186.f, 69.f) : FVector2D(180.f, 68.f);
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
			GetSettingsButtonSpriteSize(Family, State));
	}

	inline const FSlateBrush* ResolveSettingsDisabledButtonSpriteBrush()
	{
		FSettingsSpriteBrushSet& Set = GetSettingsButtonSpriteSet(ET66SettingsSpriteFamily::ToggleInactive);
		return ResolveSettingsSpriteBrush(
			Set.Disabled,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_toggle_inactive_normal.png"),
			FVector2D(180.f, 69.f));
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
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_content_shell_frame.png"),
			FVector2D(1521.f, 463.f),
			FMargin(0.035f, 0.12f, 0.035f, 0.12f));
	}

	inline const FSlateBrush* GetSettingsRowShellBrush()
	{
		static FSettingsSpriteBrushEntry Entry;
		return ResolveSettingsSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_row_shell_full.png"),
			FVector2D(861.f, 74.f),
			FMargin(0.055f, 0.32f, 0.055f, 0.32f));
	}

	inline const FSlateBrush* GetSettingsDropdownFieldBrush()
	{
		static FSettingsSpriteBrushEntry Entry;
		return ResolveSettingsSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_dropdown_field.png"),
			FVector2D(862.f, 77.f),
			FMargin(0.06f, 0.34f, 0.06f, 0.34f));
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

			TSharedRef<SButton> ButtonRef =
				SAssignNew(Button, SButton)
				.ButtonStyle(&OwnedButtonStyle)
				.ContentPadding(FMargin(0.f))
				.IsEnabled(InArgs._IsEnabled)
				.OnClicked(InArgs._OnClicked)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.Image(this, &ST66SettingsSpriteButton::GetCurrentBrush)
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
					]
				];

			ChildSlot
			[
				SNew(SBox)
				.MinDesiredWidth(InArgs._MinWidth > 0.f ? InArgs._MinWidth : FOptionalSize())
				.HeightOverride(InArgs._Height > 0.f ? InArgs._Height : FOptionalSize())
				.Visibility(InArgs._Visibility)
				[
					ButtonRef
				]
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

	inline FLinearColor T66SettingsShellFill()
	{
		return FT66Style::Background();
	}

	inline FLinearColor T66SettingsPageFill()
	{
		return FT66Style::PanelOuter();
	}

	inline FLinearColor T66SettingsRowFill()
	{
		return FT66Style::PanelInner();
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
		return FT66Style::Text();
	}

	inline FLinearColor GetSettingsPageMuted()
	{
		return FT66Style::TextMuted();
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
		return FT66Style::Text();
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
		(void)FillColor;
		return SNew(SBorder)
			.BorderImage(GetSettingsRowShellBrush())
			.BorderBackgroundColor(FLinearColor::White)
			.Padding(Padding)
			[
				Content
			];
	}

	inline TSharedRef<SWidget> MakeSettingsContentShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		return SNew(SBorder)
			.BorderImage(GetSettingsContentShellBrush())
			.BorderBackgroundColor(FLinearColor::White)
			.Padding(Padding)
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				Content
			];
	}

	inline TSharedRef<SWidget> MakeSettingsSpriteButton(
		const FT66ButtonParams& Params,
		TAttribute<ET66SettingsSpriteFamily> SpriteFamily)
	{
		const int32 FontSize = Params.FontSize > 0 ? Params.FontSize : AdjustSettingsFontSize(SettingsCompactButtonFontSize);
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
			: StaticCastSharedRef<SWidget>(
				SNew(STextBlock)
				.Text(Text)
				.Font(Font)
				.ColorAndOpacity(TextColor)
				.ShadowOffset(FVector2D(1.f, 1.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.65f))
				.Justification(ETextJustify::Center));

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
			FlatParams.SetFontSize(AdjustSettingsFontSize(SettingsCompactButtonFontSize));
		}
		else
		{
			FlatParams.SetFontSize(AdjustSettingsFontSize(static_cast<int32>(FlatParams.FontSize)));
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
		TSharedRef<TFunction<bool()>> IsSelectedRef = MakeShared<TFunction<bool()>>(MoveTemp(IsSelected));
		return MakeSettingsSpriteButton(
			ButtonParams,
			TAttribute<ET66SettingsSpriteFamily>::CreateLambda([IsSelectedRef, SelectedFamily]() -> ET66SettingsSpriteFamily
			{
				return (*IsSelectedRef)() ? SelectedFamily : ET66SettingsSpriteFamily::CompactNeutral;
			}));
	}

	inline TSharedRef<SWidget> MakeSettingsRow(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(12.f, 10.f))
	{
		return SNew(SBorder)
			.BorderImage(GetSettingsRowShellBrush())
			.BorderBackgroundColor(FLinearColor::White)
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
		FSlateColor TextColor = FSlateColor(GetSettingsPageText());
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
					SNew(STextBlock)
					.Text(Label)
					.Font(SettingsRegularFont(22))
					.ColorAndOpacity(GetSettingsPageText())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 18.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Description)
					.Font(SettingsRegularFont(16))
					.ColorAndOpacity(GetSettingsPageMuted())
					.AutoWrapText(true)
				])
			: StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.MinDesiredWidth(Style.LabelMinWidth)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(SettingsRegularFont(22))
					.ColorAndOpacity(GetSettingsPageText())
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
			]);
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

	inline TSharedRef<SWidget> MakeSettingsPercentEntryRow(
		const FText& Label,
		const FText& Description,
		TFunction<float()> GetPercent,
		TFunction<void(float)> SetPercent,
		const FText& HintText,
		const FText& HelpText)
	{
		auto CommitNumericValue = [SetPercent](const FText& NewText)
		{
			const FString TrimmedText = NewText.ToString().TrimStartAndEnd();
			float ParsedValue = 0.0f;
			if (TrimmedText.IsEmpty())
			{
				SetPercent(0.0f);
			}
			else if (FDefaultValueHelper::ParseFloat(TrimmedText, ParsedValue))
			{
				SetPercent(FMath::Clamp(ParsedValue, 0.0f, 100.0f));
			}
		};

		return MakeSettingsRow(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.46f).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(SettingsRegularFont(22))
					.ColorAndOpacity(GetSettingsPageText())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 18.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Description)
					.Font(SettingsRegularFont(16))
					.ColorAndOpacity(GetSettingsPageMuted())
					.AutoWrapText(true)
				]
			]
			+ SHorizontalBox::Slot().FillWidth(0.54f).VAlign(VAlign_Center).Padding(10.f, 0.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBox)
					.WidthOverride(140.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(GetRetroButtonOutline())
						.Padding(1.f)
						[
							SNew(SEditableTextBox)
							.Text(FormatRetroPercent(GetPercent()))
							.Font(SettingsRegularFont(18))
							.ForegroundColor(GetRetroButtonText())
							.BackgroundColor(FLinearColor::White)
							.Justification(ETextJustify::Center)
							.HintText(HintText)
							.OnTextCommitted_Lambda([CommitNumericValue](const FText& NewText, ETextCommit::Type)
							{
								CommitNumericValue(NewText);
							})
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(HelpText)
					.Font(SettingsRegularFont(14))
					.ColorAndOpacity(GetSettingsPageMuted())
					.AutoWrapText(true)
				]
			]);
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
				SNew(SSlider)
				.Value(Params.InitialValue)
				.OnValueChanged_Lambda([Params, ValueText](float Value)
				{
					if (Params.OnValueChanged)
					{
						Params.OnValueChanged(Value);
					}
					if ((*ValueText).IsValid())
					{
						(*ValueText)->SetText(Params.FormatValueText ? Params.FormatValueText(Value) : FText::GetEmpty());
					}
				})
				.OnMouseCaptureEnd_Lambda([Params]()
				{
					if (Params.OnMouseCaptureEnd)
					{
						Params.OnMouseCaptureEnd();
					}
				})
				.OnControllerCaptureEnd_Lambda([Params]()
				{
					if (Params.OnControllerCaptureEnd)
					{
						Params.OnControllerCaptureEnd();
					}
				})
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
