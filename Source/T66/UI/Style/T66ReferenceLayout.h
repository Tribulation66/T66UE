// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Margin.h"

struct FT66ReferenceRect
{
	constexpr FT66ReferenceRect()
		: X(0.f)
		, Y(0.f)
		, Width(0.f)
		, Height(0.f)
	{
	}

	constexpr FT66ReferenceRect(float InX, float InY, float InWidth, float InHeight)
		: X(InX)
		, Y(InY)
		, Width(InWidth)
		, Height(InHeight)
	{
	}

	constexpr float Right() const
	{
		return X + Width;
	}

	constexpr float Bottom() const
	{
		return Y + Height;
	}

	float X;
	float Y;
	float Width;
	float Height;
};

struct FT66ReferenceTransform
{
	FT66ReferenceTransform()
		: ReferenceSize(FVector2D(1.f, 1.f))
		, TargetSize(FVector2D(1.f, 1.f))
	{
	}

	FT66ReferenceTransform(const FVector2D& InReferenceSize, const FVector2D& InTargetSize)
		: ReferenceSize(InReferenceSize)
		, TargetSize(InTargetSize)
	{
	}

	float ScaleX() const
	{
		return ReferenceSize.X > KINDA_SMALL_NUMBER ? (TargetSize.X / ReferenceSize.X) : 1.f;
	}

	float ScaleY() const
	{
		return ReferenceSize.Y > KINDA_SMALL_NUMBER ? (TargetSize.Y / ReferenceSize.Y) : 1.f;
	}

	float MapX(float Value) const
	{
		return Value * ScaleX();
	}

	float MapY(float Value) const
	{
		return Value * ScaleY();
	}

	FVector2D MapSize(const FT66ReferenceRect& Rect) const
	{
		return FVector2D(MapX(Rect.Width), MapY(Rect.Height));
	}

	FMargin MapMargin(const FT66ReferenceRect& Rect) const
	{
		return FMargin(MapX(Rect.X), MapY(Rect.Y), MapX(Rect.Width), MapY(Rect.Height));
	}

	FMargin MapPadding(float Left, float Top, float Right, float Bottom) const
	{
		return FMargin(MapX(Left), MapY(Top), MapX(Right), MapY(Bottom));
	}

	FVector2D ReferenceSize;
	FVector2D TargetSize;
};

#include "UI/Style/T66MainMenuReferenceLayout.generated.h"
