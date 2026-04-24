// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Styling/CoreStyle.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
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
		return FLinearColor(0.020f, 0.024f, 0.018f, 0.965f);
	}

	inline FLinearColor PanelFill()
	{
		return FLinearColor(0.050f, 0.045f, 0.034f, 0.975f);
	}

	inline FLinearColor CardFill()
	{
		return FLinearColor(0.086f, 0.072f, 0.048f, 1.0f);
	}

	inline FLinearColor RaisedFill()
	{
		return FLinearColor(0.120f, 0.104f, 0.064f, 1.0f);
	}

	inline FLinearColor InnerFill()
	{
		return FLinearColor(0.031f, 0.034f, 0.028f, 0.955f);
	}

	inline FLinearColor StoneShadow()
	{
		return FLinearColor(0.006f, 0.008f, 0.007f, 0.64f);
	}

	inline FLinearColor StrokeGold()
	{
		return FLinearColor(0.74f, 0.58f, 0.27f, 1.0f);
	}

	inline FLinearColor StrokeGreen()
	{
		return FLinearColor(0.24f, 0.48f, 0.26f, 1.0f);
	}

	inline FLinearColor StrokePurple()
	{
		return FLinearColor(0.36f, 0.24f, 0.50f, 1.0f);
	}

	inline FLinearColor BrightText()
	{
		return FLinearColor(0.97f, 0.94f, 0.84f, 1.0f);
	}

	inline FLinearColor MutedText()
	{
		return FLinearColor(0.78f, 0.74f, 0.66f, 1.0f);
	}

	inline FLinearColor AccentCrimson()
	{
		return FLinearColor(0.64f, 0.17f, 0.18f, 1.0f);
	}

	inline FLinearColor AccentGold()
	{
		return FLinearColor(0.92f, 0.72f, 0.30f, 1.0f);
	}

	inline FLinearColor AccentGreen()
	{
		return FLinearColor(0.32f, 0.70f, 0.36f, 1.0f);
	}

	inline FLinearColor AccentPurple()
	{
		return FLinearColor(0.58f, 0.39f, 0.78f, 1.0f);
	}

	inline FLinearColor AccentAsh()
	{
		return FLinearColor(0.38f, 0.40f, 0.34f, 1.0f);
	}

	inline FLinearColor SelectionStroke(const bool bIsSelected)
	{
		FLinearColor Color = bIsSelected ? AccentGold() : StrokeGreen();
		Color.A = bIsSelected ? 0.20f : 0.0f;
		return Color;
	}

	inline const FSlateBrush* ButtonPlateBrush(const ET66ButtonType Type)
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush GreenEntry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush BlueEntry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush PurpleEntry;

		T66RuntimeUIBrushAccess::FOptionalTextureBrush* Entry = &BlueEntry;
		const TCHAR* RelativePath = TEXT("SourceAssets/TD/UI/td_main_menu/Components/button_blue_normal.png");
		const TCHAR* DebugLabel = TEXT("TDButtonBlue");

		if (Type == ET66ButtonType::Success || Type == ET66ButtonType::Primary)
		{
			Entry = &GreenEntry;
			RelativePath = TEXT("SourceAssets/TD/UI/td_main_menu/Components/button_green_normal.png");
			DebugLabel = TEXT("TDButtonGreen");
		}
		else if (Type == ET66ButtonType::Danger)
		{
			Entry = &PurpleEntry;
			RelativePath = TEXT("SourceAssets/TD/UI/td_battle/Components/button_purple_normal.png");
			DebugLabel = TEXT("TDButtonPurple");
		}

		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			*Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			FMargin(0.18f, 0.32f, 0.18f, 0.32f),
			DebugLabel);
	}

	inline const FSlateBrush* LeftPanelShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(TEXT("SourceAssets/TD/UI/td_main_menu/Components/panel_left_shell.png")),
			FMargin(0.13f, 0.12f, 0.13f, 0.12f),
			TEXT("TDLeftPanelShell"));
	}

	inline const FSlateBrush* RightPanelShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(TEXT("SourceAssets/TD/UI/td_main_menu/Components/panel_right_shell.png")),
			FMargin(0.13f, 0.12f, 0.13f, 0.12f),
			TEXT("TDRightPanelShell"));
	}

	inline const FSlateBrush* ContentPanelBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(TEXT("SourceAssets/TD/UI/td_main_menu/Components/panel_card_shell.png")),
			FMargin(0.12f, 0.28f, 0.12f, 0.28f),
			TEXT("TDContentPanel"));
	}

	inline const FSlateBrush* CenterFrameBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(TEXT("SourceAssets/TD/UI/td_main_menu/Components/cta_stack_shell.png")),
			FMargin(0.16f, 0.22f, 0.16f, 0.22f),
			TEXT("TDCenterFrame"));
	}

	inline TSharedRef<SWidget> MakeGeneratedPanel(
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* Brush,
		const FLinearColor& FallbackColor,
		const FMargin& Padding)
	{
		return SNew(SBorder)
			.BorderImage(Brush ? Brush : WhiteBrush())
			.BorderBackgroundColor(Brush ? FLinearColor::White : FallbackColor)
			.Padding(Padding)
			[
				Content
			];
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
			.SetUseDotaPlateOverlay(true)
			.SetDotaPlateOverrideBrush(ButtonPlateBrush(ET66ButtonType::Success))
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
			.SetUseDotaPlateOverlay(true)
			.SetDotaPlateOverrideBrush(ButtonPlateBrush(ET66ButtonType::Neutral))
			.SetTextColor(BrightText());
		return Params;
	}
}
