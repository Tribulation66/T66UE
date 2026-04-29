// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Styling/CoreStyle.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"

namespace T66MiniUI
{
	inline const FSlateBrush* WhiteBrush()
	{
		return FCoreStyle::Get().GetBrush("WhiteBrush");
	}

	inline FSlateFontInfo TitleFont(const int32 Size)
	{
		return FCoreStyle::GetDefaultFontStyle("Bold", Size);
	}

	inline FSlateFontInfo BoldFont(const int32 Size)
	{
		return FCoreStyle::GetDefaultFontStyle("Bold", Size);
	}

	inline FSlateFontInfo BodyFont(const int32 Size)
	{
		return FCoreStyle::GetDefaultFontStyle("Regular", Size);
	}

	inline FLinearColor ShellFill()
	{
		return FLinearColor(0.025f, 0.021f, 0.024f, 0.985f);
	}

	inline FLinearColor PanelFill()
	{
		return FLinearColor(0.058f, 0.052f, 0.044f, 0.980f);
	}

	inline FLinearColor CardFill()
	{
		return FLinearColor(0.085f, 0.075f, 0.055f, 1.0f);
	}

	inline FLinearColor RaisedFill()
	{
		return FLinearColor(0.130f, 0.105f, 0.070f, 1.0f);
	}

	inline FLinearColor MutedText()
	{
		return FLinearColor(0.765f, 0.735f, 0.655f, 1.0f);
	}

	inline FLinearColor Text()
	{
		return FLinearColor(0.955f, 0.925f, 0.835f, 1.0f);
	}

	inline FLinearColor ScreenBackground()
	{
		return FLinearColor(0.050f, 0.043f, 0.050f, 1.0f);
	}

	inline FLinearColor ScreenTint()
	{
		return FLinearColor(0.055f, 0.025f, 0.075f, 0.24f);
	}

	inline FLinearColor PanelOutline()
	{
		return FLinearColor(0.510f, 0.405f, 0.205f, 0.94f);
	}

	inline FLinearColor SelectedFill()
	{
		return FLinearColor(0.245f, 0.220f, 0.105f, 0.96f);
	}

	inline FLinearColor SelectedBorder()
	{
		return FLinearColor(0.860f, 0.670f, 0.247f, 1.0f);
	}

	inline FLinearColor AccentGreen()
	{
		return FLinearColor(0.475f, 0.700f, 0.245f, 1.0f);
	}

	inline FLinearColor AccentBlue()
	{
		return FLinearColor(0.250f, 0.185f, 0.365f, 1.0f);
	}

	inline FLinearColor AccentGold()
	{
		return FLinearColor(0.860f, 0.670f, 0.247f, 1.0f);
	}

	inline FLinearColor AccentPurple()
	{
		return FLinearColor(0.490f, 0.315f, 0.650f, 1.0f);
	}

	inline FLinearColor Danger()
	{
		return FLinearColor(0.600f, 0.185f, 0.205f, 1.0f);
	}

	inline FLinearColor ButtonTextDark()
	{
		return FLinearColor(0.070f, 0.055f, 0.030f, 1.0f);
	}

	inline const TCHAR* MasterBasicPanelPath()
	{
		return TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/basic_panel_normal.png");
	}

	inline const TCHAR* MasterInnerPanelPath()
	{
		return TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/inner_panel_normal.png");
	}

	inline const TCHAR* MasterBasicButtonPath()
	{
		return TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/basic_button_normal.png");
	}

	inline const TCHAR* MasterSelectedButtonPath()
	{
		return TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/select_button_selected.png");
	}

	inline const FMargin& MasterPanelMargin()
	{
		static const FMargin Margin(0.067f, 0.043f, 0.067f, 0.043f);
		return Margin;
	}

	inline const FMargin& MasterButtonMargin()
	{
		static const FMargin Margin(0.104f, 0.250f, 0.104f, 0.250f);
		return Margin;
	}

	inline const FSlateBrush* ContentShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(MasterBasicPanelPath()),
			MasterPanelMargin(),
			TEXT("MiniMasterContentShell"));
	}

	inline const FSlateBrush* RowShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(MasterInnerPanelPath()),
			MasterPanelMargin(),
			TEXT("MiniMasterRowShell"));
	}

	inline const FSlateBrush* ButtonPlateBrush(const ET66ButtonType Type)
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush BasicEntry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush SelectedEntry;

		const bool bUseSelectedPlate = Type == ET66ButtonType::Success
			|| Type == ET66ButtonType::Primary
			|| Type == ET66ButtonType::ToggleActive;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry = bUseSelectedPlate ? SelectedEntry : BasicEntry;

		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(bUseSelectedPlate ? MasterSelectedButtonPath() : MasterBasicButtonPath()),
			MasterButtonMargin(),
			bUseSelectedPlate ? TEXT("MiniMasterSelectedButton") : TEXT("MiniMasterBasicButton"));
	}

	inline TSharedRef<SWidget> MakeSpritePanel(const TSharedRef<SWidget>& Content, const FMargin& Padding, const bool bRow = false)
	{
		const FSlateBrush* Brush = bRow ? RowShellBrush() : ContentShellBrush();
		return SNew(SBorder)
			.BorderImage(Brush ? Brush : WhiteBrush())
			.BorderBackgroundColor(Brush ? FLinearColor::White : PanelFill())
			.Padding(Padding)
			[
				Content
			];
	}

	inline FT66ButtonParams MakeButtonParams(
		const FText& Label,
		const FOnClicked& OnClicked,
		const ET66ButtonType Type,
		const float MinWidth,
		const float Height,
		const int32 FontSize)
	{
		FT66ButtonParams Params(Label, OnClicked, Type);
		Params
			.SetMinWidth(MinWidth)
			.SetHeight(Height)
			.SetFontSize(FontSize)
			.SetPadding(FMargin(14.f, 8.f, 14.f, 6.f))
			.SetUseGlow(false)
			.SetUseDotaPlateOverlay(false)
			.SetDotaPlateOverrideBrush(ButtonPlateBrush(Type))
			.SetStateTextShadowColors(
				FLinearColor(0.f, 0.f, 0.f, 0.44f),
				FLinearColor(0.f, 0.f, 0.f, 0.50f),
				FLinearColor(0.f, 0.f, 0.f, 0.34f))
			.SetTextShadowOffset(FVector2D(0.f, 1.f))
			.SetTextColor(Text());
		return Params;
	}

	inline TSharedRef<SWidget> MakeButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		const ET66ButtonType Type,
		const float MinWidth,
		const float Height,
		const int32 FontSize)
	{
		return FT66Style::MakeButton(MakeButtonParams(Label, OnClicked, Type, MinWidth, Height, FontSize));
	}
}
