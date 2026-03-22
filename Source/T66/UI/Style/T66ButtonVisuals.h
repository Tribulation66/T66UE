// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"

enum class ET66ButtonBorderVisual : uint8;
enum class ET66ButtonBackgroundVisual : uint8;
enum class ET66ButtonBorderState : uint8
{
	Normal,
	Hovered,
	Pressed,
};

struct FT66ButtonBorderBrushSet
{
	TSharedPtr<FSlateBrush> Normal;
	TSharedPtr<FSlateBrush> Hovered;
	TSharedPtr<FSlateBrush> Pressed;
	TSharedPtr<FSlateBrush> HorizontalNormal;
	TSharedPtr<FSlateBrush> HorizontalHovered;
	TSharedPtr<FSlateBrush> HorizontalPressed;
	TSharedPtr<FSlateBrush> VerticalNormal;
	TSharedPtr<FSlateBrush> VerticalHovered;
	TSharedPtr<FSlateBrush> VerticalPressed;
	float Thickness = 0.f;

	bool IsValid() const
	{
		return Normal.IsValid() && Hovered.IsValid() && Pressed.IsValid();
	}

	const FSlateBrush* GetBrush(ET66ButtonBorderState State) const
	{
		switch (State)
		{
		case ET66ButtonBorderState::Hovered:
			return Hovered.Get();
		case ET66ButtonBorderState::Pressed:
			return Pressed.Get();
		case ET66ButtonBorderState::Normal:
		default:
			return Normal.Get();
		}
	}

	const FSlateBrush* GetHorizontalBrush(ET66ButtonBorderState State) const
	{
		switch (State)
		{
		case ET66ButtonBorderState::Hovered:
			return HorizontalHovered.IsValid() ? HorizontalHovered.Get() : GetBrush(State);
		case ET66ButtonBorderState::Pressed:
			return HorizontalPressed.IsValid() ? HorizontalPressed.Get() : GetBrush(State);
		case ET66ButtonBorderState::Normal:
		default:
			return HorizontalNormal.IsValid() ? HorizontalNormal.Get() : GetBrush(State);
		}
	}

	const FSlateBrush* GetVerticalBrush(ET66ButtonBorderState State) const
	{
		switch (State)
		{
		case ET66ButtonBorderState::Hovered:
			return VerticalHovered.IsValid() ? VerticalHovered.Get() : GetBrush(State);
		case ET66ButtonBorderState::Pressed:
			return VerticalPressed.IsValid() ? VerticalPressed.Get() : GetBrush(State);
		case ET66ButtonBorderState::Normal:
		default:
			return VerticalNormal.IsValid() ? VerticalNormal.Get() : GetBrush(State);
		}
	}
};

struct FT66ButtonFillBrushSet
{
	TSharedPtr<FSlateBrush> Normal;
	TSharedPtr<FSlateBrush> Hovered;
	TSharedPtr<FSlateBrush> Pressed;

	bool IsValid() const
	{
		return Normal.IsValid() && Hovered.IsValid() && Pressed.IsValid();
	}

	const FSlateBrush* GetBrush(ET66ButtonBorderState State) const
	{
		switch (State)
		{
		case ET66ButtonBorderState::Hovered:
			return Hovered.Get();
		case ET66ButtonBorderState::Pressed:
			return Pressed.Get();
		case ET66ButtonBorderState::Normal:
		default:
			return Normal.Get();
		}
	}
};

/**
 * Shared decorative button border styles that sit on top of the base T66 button system.
 * Keep this separate from FT66Style so experimental chrome treatments don't bloat the core token file.
 */
class FT66ButtonVisuals
{
public:
	static TSharedPtr<FT66ButtonBorderBrushSet> CreateBorderBrushSet(ET66ButtonBorderVisual Visual);
	static TSharedPtr<FT66ButtonFillBrushSet> CreateFillBrushSet(ET66ButtonBackgroundVisual Visual);
};
