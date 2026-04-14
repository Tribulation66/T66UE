// Copyright Tribulation 66. All Rights Reserved.

#include "UI/ST66AnimatedBorderGlow.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

namespace
{
	FPaintGeometry MakePaintGeometry(const FGeometry& AllottedGeometry, const FVector2D& Position, const FVector2D& Size)
	{
		return AllottedGeometry.ToPaintGeometry(
			FVector2f(static_cast<float>(Size.X), static_cast<float>(Size.Y)),
			FSlateLayoutTransform(FVector2f(static_cast<float>(Position.X), static_cast<float>(Position.Y))));
	}
}

ST66AnimatedBorderGlow::~ST66AnimatedBorderGlow()
{
	if (ActiveTimerHandle.IsValid())
	{
		UnRegisterActiveTimer(ActiveTimerHandle.ToSharedRef());
		ActiveTimerHandle.Reset();
	}
}

void ST66AnimatedBorderGlow::Construct(const FArguments& InArgs)
{
	GlowColor = InArgs._GlowColor;
	SweepColor = InArgs._SweepColor;
	BorderInset = FMath::Max(0.f, InArgs._BorderInset);
	InnerThickness = FMath::Max(1.f, InArgs._InnerThickness);
	MidThickness = FMath::Max(InnerThickness, InArgs._MidThickness);
	OuterThickness = FMath::Max(MidThickness, InArgs._OuterThickness);
	SweepLengthFraction = FMath::Clamp(InArgs._SweepLengthFraction, 0.05f, 0.9f);
	SweepSpeed = FMath::Max(0.f, InArgs._SweepSpeed);

	const float BaseOpacityMin = FMath::Clamp(InArgs._BaseOpacityMin, 0.f, 1.f);
	const float BaseOpacityMax = FMath::Max(BaseOpacityMin, FMath::Clamp(InArgs._BaseOpacityMax, 0.f, 1.f));
	BaseOpacityTween = FT66UITween(
		ET66UITweenMode::Sine,
		(BaseOpacityMax - BaseOpacityMin) * 0.5f,
		FMath::Max(0.f, InArgs._BaseOpacityFrequency));
	BaseOpacityBias = (BaseOpacityMin + BaseOpacityMax) * 0.5f;
	CurrentBaseOpacity = BaseOpacityBias;

	ChildSlot
	[
		InArgs._Content.Widget
	];

	TickAnimation(0.f);
	ActiveTimerHandle = RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &ST66AnimatedBorderGlow::HandleActiveTimer));
}

EActiveTimerReturnType ST66AnimatedBorderGlow::HandleActiveTimer(double InCurrentTime, float InDeltaTime)
{
	TickAnimation(InDeltaTime);
	Invalidate(EInvalidateWidgetReason::Paint);
	return EActiveTimerReturnType::Continue;
}

void ST66AnimatedBorderGlow::TickAnimation(float DeltaTime)
{
	CurrentBaseOpacity = FMath::Clamp(BaseOpacityBias + BaseOpacityTween.Tick(DeltaTime), 0.f, 1.f);
	CurrentSweepProgress = FMath::Frac(CurrentSweepProgress + (DeltaTime * SweepSpeed));
}

int32 ST66AnimatedBorderGlow::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const int32 ContentLayer = SCompoundWidget::OnPaint(
		Args,
		AllottedGeometry,
		MyCullingRect,
		OutDrawElements,
		LayerId,
		InWidgetStyle,
		bParentEnabled);

	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
	if (LocalSize.X <= (BorderInset * 2.f) || LocalSize.Y <= (BorderInset * 2.f))
	{
		return ContentLayer;
	}

	const float Left = BorderInset;
	const float Top = BorderInset;
	const float Right = LocalSize.X - BorderInset;
	const float Bottom = LocalSize.Y - BorderInset;
	const float Width = FMath::Max(0.f, Right - Left);
	const float Height = FMath::Max(0.f, Bottom - Top);
	const float Perimeter = (Width + Height) * 2.f;
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	const FLinearColor StyleTint = InWidgetStyle.GetColorAndOpacityTint();

	if (Width <= KINDA_SMALL_NUMBER || Height <= KINDA_SMALL_NUMBER || Perimeter <= KINDA_SMALL_NUMBER || WhiteBrush == nullptr)
	{
		return ContentLayer;
	}

	auto DrawRect = [&](int32 TargetLayer, const FVector2D& Position, const FVector2D& Size, const FLinearColor& Color)
	{
		if (Size.X <= KINDA_SMALL_NUMBER || Size.Y <= KINDA_SMALL_NUMBER || Color.A <= KINDA_SMALL_NUMBER)
		{
			return;
		}

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			TargetLayer,
			MakePaintGeometry(AllottedGeometry, Position, Size),
			WhiteBrush,
			ESlateDrawEffect::None,
			Color * StyleTint);
	};

	auto DrawFullBorder = [&](int32 TargetLayer, float Thickness, const FLinearColor& Color)
	{
		const float ClampedThickness = FMath::Clamp(Thickness, 1.f, FMath::Min(Width, Height) * 0.5f);
		DrawRect(TargetLayer, FVector2D(Left, Top), FVector2D(Width, ClampedThickness), Color);
		DrawRect(TargetLayer, FVector2D(Right - ClampedThickness, Top), FVector2D(ClampedThickness, Height), Color);
		DrawRect(TargetLayer, FVector2D(Left, Bottom - ClampedThickness), FVector2D(Width, ClampedThickness), Color);
		DrawRect(TargetLayer, FVector2D(Left, Top), FVector2D(ClampedThickness, Height), Color);
	};

	auto DrawPerimeterSegment = [&](int32 TargetLayer, float StartDistance, float SegmentLength, float Thickness, const FLinearColor& Color)
	{
		if (SegmentLength <= KINDA_SMALL_NUMBER || Thickness <= KINDA_SMALL_NUMBER)
		{
			return;
		}

		float Distance = FMath::Fmod(StartDistance, Perimeter);
		if (Distance < 0.f)
		{
			Distance += Perimeter;
		}

		float Remaining = SegmentLength;
		const float ClampedThickness = FMath::Clamp(Thickness, 1.f, FMath::Min(Width, Height) * 0.5f);
		while (Remaining > KINDA_SMALL_NUMBER)
		{
			int32 EdgeIndex = 0;
			float EdgeStart = Distance;
			float EdgeLength = Width;

			if (Distance < Width)
			{
				EdgeIndex = 0;
				EdgeStart = Distance;
				EdgeLength = Width;
			}
			else if (Distance < Width + Height)
			{
				EdgeIndex = 1;
				EdgeStart = Distance - Width;
				EdgeLength = Height;
			}
			else if (Distance < (Width * 2.f) + Height)
			{
				EdgeIndex = 2;
				EdgeStart = Distance - (Width + Height);
				EdgeLength = Width;
			}
			else
			{
				EdgeIndex = 3;
				EdgeStart = Distance - ((Width * 2.f) + Height);
				EdgeLength = Height;
			}

			const float DrawLength = FMath::Min(Remaining, EdgeLength - EdgeStart);
			switch (EdgeIndex)
			{
			case 0:
				DrawRect(TargetLayer, FVector2D(Left + EdgeStart, Top), FVector2D(DrawLength, ClampedThickness), Color);
				break;
			case 1:
				DrawRect(TargetLayer, FVector2D(Right - ClampedThickness, Top + EdgeStart), FVector2D(ClampedThickness, DrawLength), Color);
				break;
			case 2:
				DrawRect(TargetLayer, FVector2D(Right - EdgeStart - DrawLength, Bottom - ClampedThickness), FVector2D(DrawLength, ClampedThickness), Color);
				break;
			case 3:
			default:
				DrawRect(TargetLayer, FVector2D(Left, Bottom - EdgeStart - DrawLength), FVector2D(ClampedThickness, DrawLength), Color);
				break;
			}

			Remaining -= DrawLength;
			Distance = FMath::Fmod(Distance + DrawLength, Perimeter);
		}
	};

	const int32 GlowLayer = ContentLayer + 1;
	DrawFullBorder(GlowLayer, OuterThickness, GlowColor.CopyWithNewOpacity(CurrentBaseOpacity * 0.10f));
	DrawFullBorder(GlowLayer + 1, MidThickness, GlowColor.CopyWithNewOpacity(CurrentBaseOpacity * 0.17f));
	DrawFullBorder(GlowLayer + 2, InnerThickness, SweepColor.CopyWithNewOpacity(CurrentBaseOpacity * 0.42f));

	const float SweepLength = FMath::Clamp(Perimeter * SweepLengthFraction, 64.f, Perimeter * 0.55f);
	const float SweepHeadDistance = CurrentSweepProgress * Perimeter;
	DrawPerimeterSegment(GlowLayer + 1, SweepHeadDistance - (SweepLength * 0.88f), SweepLength, OuterThickness, GlowColor.CopyWithNewOpacity(0.12f));
	DrawPerimeterSegment(GlowLayer + 2, SweepHeadDistance - (SweepLength * 0.50f), SweepLength * 0.62f, MidThickness, GlowColor.CopyWithNewOpacity(0.20f));
	DrawPerimeterSegment(GlowLayer + 3, SweepHeadDistance - (SweepLength * 0.12f), SweepLength * 0.24f, InnerThickness + 1.f, SweepColor.CopyWithNewOpacity(0.58f));

	return GlowLayer + 3;
}
