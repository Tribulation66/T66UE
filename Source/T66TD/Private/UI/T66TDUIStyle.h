// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Styling/CoreStyle.h"

namespace T66TDUI
{
	inline const FSlateBrush* WhiteBrush()
	{
		return FCoreStyle::Get().GetBrush("WhiteBrush");
	}

	inline FLinearColor ShellFill()
	{
		return FLinearColor(0.014f, 0.010f, 0.014f, 0.985f);
	}

	inline FLinearColor PanelFill()
	{
		return FLinearColor(0.055f, 0.032f, 0.042f, 0.98f);
	}

	inline FLinearColor CardFill()
	{
		return FLinearColor(0.090f, 0.045f, 0.055f, 1.0f);
	}

	inline FLinearColor RaisedFill()
	{
		return FLinearColor(0.125f, 0.060f, 0.070f, 1.0f);
	}

	inline FLinearColor MutedText()
	{
		return FLinearColor(0.84f, 0.80f, 0.80f, 1.0f);
	}

	inline FLinearColor AccentCrimson()
	{
		return FLinearColor(0.82f, 0.24f, 0.22f, 1.0f);
	}

	inline FLinearColor AccentGold()
	{
		return FLinearColor(0.90f, 0.72f, 0.28f, 1.0f);
	}

	inline FLinearColor AccentAsh()
	{
		return FLinearColor(0.30f, 0.34f, 0.38f, 1.0f);
	}
}
