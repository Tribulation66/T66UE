// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Styling/CoreStyle.h"
#include "UI/Style/T66FlatStyle.h"
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
		return FT66FlatStyle::BackgroundColor();
	}

	inline FLinearColor PanelFill()
	{
		return FT66FlatStyle::DefaultFill();
	}

	inline FLinearColor CardFill()
	{
		return FT66FlatStyle::DefaultFill();
	}

	inline FLinearColor RaisedFill()
	{
		return FT66FlatStyle::SelectedFill();
	}

	inline FLinearColor MutedText()
	{
		return FT66FlatStyle::SecondaryText();
	}

	inline FLinearColor Text()
	{
		return FT66FlatStyle::PrimaryText();
	}

	inline FLinearColor ScreenBackground()
	{
		return FT66FlatStyle::BackgroundColor();
	}

	inline FLinearColor ScreenTint()
	{
		return FLinearColor(0.020f, 0.010f, 0.020f, 0.28f);
	}

	inline FLinearColor PanelOutline()
	{
		return FT66FlatStyle::DefaultBorder();
	}

	inline FLinearColor SelectedFill()
	{
		return FT66FlatStyle::SelectedFill();
	}

	inline FLinearColor SelectedBorder()
	{
		return FT66FlatStyle::SelectedBorder();
	}

	inline FLinearColor AccentGreen()
	{
		return FT66FlatStyle::GoodStandingGreen();
	}

	inline FLinearColor AccentBlue()
	{
		return FT66FlatStyle::DataAccent();
	}

	inline FLinearColor AccentGold()
	{
		return FT66FlatStyle::SelectedText();
	}

	inline FLinearColor AccentPurple()
	{
		return FT66FlatStyle::PurpleAccent();
	}

	inline FLinearColor Danger()
	{
		return FT66FlatStyle::SelectedBorder();
	}

	inline FLinearColor ButtonTextDark()
	{
		return FLinearColor(0.070f, 0.055f, 0.030f, 1.0f);
	}

	inline const TCHAR* MasterBasicPanelPath()
	{
		return TEXT("");
	}

	inline const TCHAR* MasterInnerPanelPath()
	{
		return TEXT("");
	}

	inline const TCHAR* MasterBasicButtonPath()
	{
		return TEXT("");
	}

	inline const TCHAR* MasterSelectedButtonPath()
	{
		return TEXT("");
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
		return WhiteBrush();
	}

	inline const FSlateBrush* RowShellBrush()
	{
		return WhiteBrush();
	}

	inline const FSlateBrush* ButtonPlateBrush(const ET66ButtonType Type)
	{
		return WhiteBrush();
	}

	inline TSharedRef<SWidget> MakeSpritePanel(const TSharedRef<SWidget>& Content, const FMargin& Padding, const bool bRow = false)
	{
		return bRow
			? FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, Padding, Content)
			: FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, Padding, Content);
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
			.SetStateTextShadowColors(
				FLinearColor::Transparent,
				FLinearColor::Transparent,
				FLinearColor::Transparent)
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
