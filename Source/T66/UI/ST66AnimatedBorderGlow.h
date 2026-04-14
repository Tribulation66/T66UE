// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66UITween.h"
#include "Widgets/SCompoundWidget.h"

class T66_API ST66AnimatedBorderGlow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(ST66AnimatedBorderGlow)
		: _GlowColor(FLinearColor(0.36f, 0.86f, 0.58f, 1.0f))
		, _SweepColor(FLinearColor(0.78f, 1.00f, 0.86f, 1.0f))
		, _BorderInset(2.0f)
		, _InnerThickness(2.0f)
		, _MidThickness(5.0f)
		, _OuterThickness(9.0f)
		, _BaseOpacityMin(0.20f)
		, _BaseOpacityMax(0.30f)
		, _BaseOpacityFrequency(0.24f)
		, _SweepLengthFraction(0.24f)
		, _SweepSpeed(0.11f)
	{}
		SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_ARGUMENT(FLinearColor, GlowColor)
		SLATE_ARGUMENT(FLinearColor, SweepColor)
		SLATE_ARGUMENT(float, BorderInset)
		SLATE_ARGUMENT(float, InnerThickness)
		SLATE_ARGUMENT(float, MidThickness)
		SLATE_ARGUMENT(float, OuterThickness)
		SLATE_ARGUMENT(float, BaseOpacityMin)
		SLATE_ARGUMENT(float, BaseOpacityMax)
		SLATE_ARGUMENT(float, BaseOpacityFrequency)
		SLATE_ARGUMENT(float, SweepLengthFraction)
		SLATE_ARGUMENT(float, SweepSpeed)
	SLATE_END_ARGS()

	~ST66AnimatedBorderGlow();
	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

private:
	EActiveTimerReturnType HandleActiveTimer(double InCurrentTime, float InDeltaTime);
	void TickAnimation(float DeltaTime);

	FLinearColor GlowColor = FLinearColor::White;
	FLinearColor SweepColor = FLinearColor::White;
	float BorderInset = 2.0f;
	float InnerThickness = 2.0f;
	float MidThickness = 5.0f;
	float OuterThickness = 9.0f;
	float SweepLengthFraction = 0.24f;
	float SweepSpeed = 0.11f;
	FT66UITween BaseOpacityTween;
	float BaseOpacityBias = 0.25f;
	float CurrentBaseOpacity = 0.25f;
	float CurrentSweepProgress = 0.0f;
	TSharedPtr<FActiveTimerHandle> ActiveTimerHandle;
};
