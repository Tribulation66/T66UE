// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Styling/CoreStyle.h"

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
		return FLinearColor(0.010f, 0.012f, 0.020f, 0.985f);
	}

	inline FLinearColor PanelFill()
	{
		return FLinearColor(0.030f, 0.034f, 0.050f, 0.98f);
	}

	inline FLinearColor CardFill()
	{
		return FLinearColor(0.045f, 0.050f, 0.070f, 1.0f);
	}

	inline FLinearColor RaisedFill()
	{
		return FLinearColor(0.070f, 0.075f, 0.100f, 1.0f);
	}

	inline FLinearColor MutedText()
	{
		return FLinearColor(0.74f, 0.78f, 0.86f, 1.0f);
	}

	inline FLinearColor AccentGreen()
	{
		return FLinearColor(0.68f, 0.80f, 0.54f, 1.0f);
	}

	inline FLinearColor AccentBlue()
	{
		return FLinearColor(0.28f, 0.34f, 0.46f, 1.0f);
	}

	inline FLinearColor AccentGold()
	{
		return FLinearColor(0.82f, 0.72f, 0.30f, 1.0f);
	}

	inline FLinearColor ButtonTextDark()
	{
		return FLinearColor(0.06f, 0.07f, 0.06f, 1.0f);
	}
}
