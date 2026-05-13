// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Styling/CoreStyle.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66Style.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SWidget.h"

namespace T66TDUI
{
	inline const FSlateBrush* WhiteBrush()
	{
		return FCoreStyle::Get().GetBrush("WhiteBrush");
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

	inline FLinearColor InnerFill()
	{
		return FT66FlatStyle::DisabledFill();
	}

	inline FLinearColor StoneShadow()
	{
		return FLinearColor(0.006f, 0.008f, 0.007f, 0.64f);
	}

	inline FLinearColor StrokeGold()
	{
		return FT66FlatStyle::DefaultBorder();
	}

	inline FLinearColor StrokeGreen()
	{
		return FT66FlatStyle::ReadyBorder();
	}

	inline FLinearColor StrokeRed()
	{
		return FT66FlatStyle::SelectedBorder();
	}

	inline FLinearColor BrightText()
	{
		return FT66FlatStyle::PrimaryText();
	}

	inline FLinearColor MutedText()
	{
		return FT66FlatStyle::SecondaryText();
	}

	inline FLinearColor AccentCrimson()
	{
		return FT66FlatStyle::SelectedBorder();
	}

	inline FLinearColor AccentGold()
	{
		return FT66FlatStyle::SelectedText();
	}

	inline FLinearColor AccentGreen()
	{
		return FT66FlatStyle::GoodStandingGreen();
	}

	inline FLinearColor AccentRed()
	{
		return FT66FlatStyle::SelectedBorder();
	}

	inline FLinearColor AccentAsh()
	{
		return FT66FlatStyle::DisabledBorder();
	}

	inline FLinearColor SelectionStroke(const bool bIsSelected)
	{
		FLinearColor Color = bIsSelected ? AccentGold() : StrokeGreen();
		Color.A = bIsSelected ? 0.20f : 0.0f;
		return Color;
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

	inline const FSlateBrush* ButtonPlateBrush(const ET66ButtonType Type)
	{
		return WhiteBrush();
	}

	inline const FSlateBrush* LeftPanelShellBrush()
	{
		return WhiteBrush();
	}

	inline const FSlateBrush* RightPanelShellBrush()
	{
		return WhiteBrush();
	}

	inline const FSlateBrush* ContentPanelBrush()
	{
		return WhiteBrush();
	}

	inline const FSlateBrush* CenterFrameBrush()
	{
		return WhiteBrush();
	}

	inline TSharedRef<SWidget> MakeGeneratedPanel(
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* Brush,
		const FLinearColor& FallbackColor,
		const FMargin& Padding)
	{
		return FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, Padding, Content);
	}

	inline TSharedRef<SWidget> MakeLeftPanel(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(18.f))
	{
		return MakeGeneratedPanel(Content, LeftPanelShellBrush(), ShellFill(), Padding);
	}

	inline TSharedRef<SWidget> MakeRightPanel(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(18.f))
	{
		return MakeGeneratedPanel(Content, RightPanelShellBrush(), ShellFill(), Padding);
	}

	inline TSharedRef<SWidget> MakeContentPanel(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(14.f))
	{
		return MakeGeneratedPanel(Content, ContentPanelBrush(), CardFill(), Padding);
	}

	inline TSharedRef<SWidget> MakeCenterPanel(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(20.f, 18.f))
	{
		return MakeGeneratedPanel(Content, CenterFrameBrush(), PanelFill(), Padding);
	}

	inline FT66ButtonParams MakePrimaryButtonParams(const FText& Label, const FOnClicked& OnClicked, const float MinWidth, const float Height, const int32 FontSize)
	{
		FT66ButtonParams Params(Label, OnClicked, ET66ButtonType::Success);
		Params
			.SetMinWidth(MinWidth)
			.SetHeight(Height)
			.SetFontSize(FontSize)
			.SetPadding(FMargin(14.f, 8.f, 14.f, 6.f))
			.SetUseGlow(false)
			.SetTextColor(BrightText())
			.SetStateTextShadowColors(
				FLinearColor(0.f, 0.f, 0.f, 0.40f),
				FLinearColor(0.f, 0.f, 0.f, 0.48f),
				FLinearColor(0.f, 0.f, 0.f, 0.32f))
			.SetTextShadowOffset(FVector2D(0.f, 1.f));
		return Params;
	}

	inline FT66ButtonParams MakeUtilityButtonParams(const FText& Label, const FOnClicked& OnClicked, const float MinWidth, const float Height, const int32 FontSize)
	{
		FT66ButtonParams Params(Label, OnClicked, ET66ButtonType::Neutral);
		Params
			.SetMinWidth(MinWidth)
			.SetHeight(Height)
			.SetFontSize(FontSize)
			.SetPadding(FMargin(12.f, 4.f, 12.f, 3.f))
			.SetUseGlow(false)
			.SetTextColor(BrightText());
		return Params;
	}

	inline ET66FlatState StateForButtonType(const ET66ButtonType Type)
	{
		switch (Type)
		{
		case ET66ButtonType::Success:
		case ET66ButtonType::Primary:
		case ET66ButtonType::ToggleActive:
			return ET66FlatState::Selected;
		case ET66ButtonType::Danger:
			return ET66FlatState::Ready;
		case ET66ButtonType::Neutral:
		default:
			return ET66FlatState::Default;
		}
	}

	inline TSharedRef<SWidget> MakeFlatButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		const ET66ButtonType Type,
		const float MinWidth,
		const float Height,
		const int32 FontSize,
		const FName Tag = NAME_None,
		const TAttribute<bool> IsEnabled = true)
	{
		return FT66FlatStyle::MakeFlatButton(
			StateForButtonType(Type),
			Label,
			OnClicked,
			nullptr,
			nullptr,
			FMargin(14.f, 8.f, 14.f, 6.f),
			MinWidth,
			Height,
			IsEnabled,
			FontSize,
			Tag);
	}
}
