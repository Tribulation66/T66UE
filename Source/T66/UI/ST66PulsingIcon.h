// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66UITween.h"
#include "Widgets/SCompoundWidget.h"

class SImage;
struct FSlateBrush;

class T66_API ST66PulsingIcon : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(ST66PulsingIcon) {}
		SLATE_ARGUMENT(FSlateBrush*, IconBrush)
		SLATE_ARGUMENT(FSlateBrush*, GlowBrush)
		SLATE_ARGUMENT(FVector2D, IconSize)
		SLATE_ARGUMENT(FVector2D, GlowSize)
		SLATE_ARGUMENT(float, GlowOpacityMin)
		SLATE_ARGUMENT(float, GlowOpacityMax)
		SLATE_ARGUMENT(float, GlowOpacityFrequency)
		SLATE_ARGUMENT(float, GlowScaleAmplitude)
		SLATE_ARGUMENT(float, GlowScaleFrequency)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	EActiveTimerReturnType HandleActiveTimer(double InCurrentTime, float InDeltaTime);
	void TickGlow(float DeltaTime);

	TSharedPtr<SImage> GlowImageWidget;
	TSharedPtr<SImage> IconImageWidget;

	FT66UITween GlowOpacityTween;
	FT66UITween GlowScaleTween;
	float GlowOpacityBias = 1.f;
	float CurrentGlowOpacity = 1.f;
	float CurrentGlowScale = 1.f;
};
