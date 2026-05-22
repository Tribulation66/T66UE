// Copyright Tribulation 66. All Rights Reserved.

// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

enum class ET66AnimationCurve : uint8
{
	Linear,
	EaseInQuad,
	EaseOutQuad,
	EaseInOutQuad,
	EaseInCubic,
	EaseOutCubic,
	EaseInOutCubic,
	EaseInExpo,
	EaseOutExpo,
	EaseInOutExpo,
	Anticipation,
	Overshoot,
	Settle,
	Elastic,
	WeightedDecel,
	BounceIn,
	BounceOut,
};

struct T66_API FT66AnimationCurveSpec
{
	ET66AnimationCurve Curve = ET66AnimationCurve::Linear;
	float Strength = 1.f;
	float Amplitude = 1.f;
	float Frequency = 1.f;

	FT66AnimationCurveSpec() = default;
	explicit FT66AnimationCurveSpec(ET66AnimationCurve InCurve);
	FT66AnimationCurveSpec(ET66AnimationCurve InCurve, float InStrength, float InAmplitude = 1.f, float InFrequency = 1.f);
};

namespace T66AnimationCurves
{
	T66_API float EvaluateCurve(const FT66AnimationCurveSpec& Spec, float NormalizedTime);
}
