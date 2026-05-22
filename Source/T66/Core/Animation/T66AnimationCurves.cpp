// Copyright Tribulation 66. All Rights Reserved.

// Copyright Tribulation 66. All Rights Reserved.

#include "Core/Animation/T66AnimationCurves.h"

namespace
{
	float ClampTime(const float NormalizedTime)
	{
		return FMath::Clamp(NormalizedTime, 0.f, 1.f);
	}

	float EaseOutBounce(const float T)
	{
		// Piecewise quadratic bounce that falls quickly, then rebounds in shorter arcs.
		static constexpr float N1 = 7.5625f;
		static constexpr float D1 = 2.75f;

		if (T < 1.f / D1)
		{
			return N1 * T * T;
		}
		if (T < 2.f / D1)
		{
			const float LocalT = T - (1.5f / D1);
			return N1 * LocalT * LocalT + 0.75f;
		}
		if (T < 2.5f / D1)
		{
			const float LocalT = T - (2.25f / D1);
			return N1 * LocalT * LocalT + 0.9375f;
		}

		const float LocalT = T - (2.625f / D1);
		return N1 * LocalT * LocalT + 0.984375f;
	}
}

FT66AnimationCurveSpec::FT66AnimationCurveSpec(const ET66AnimationCurve InCurve)
	: Curve(InCurve)
{
	switch (Curve)
	{
	case ET66AnimationCurve::Anticipation:
		Strength = 1.2f;
		break;
	case ET66AnimationCurve::Overshoot:
	case ET66AnimationCurve::Settle:
		Strength = 1.7f;
		break;
	case ET66AnimationCurve::Elastic:
		Amplitude = 1.f;
		Frequency = 3.f;
		break;
	default:
		break;
	}
}

FT66AnimationCurveSpec::FT66AnimationCurveSpec(const ET66AnimationCurve InCurve, const float InStrength, const float InAmplitude, const float InFrequency)
	: Curve(InCurve)
	, Strength(InStrength)
	, Amplitude(InAmplitude)
	, Frequency(InFrequency)
{
}

float T66AnimationCurves::EvaluateCurve(const FT66AnimationCurveSpec& Spec, const float NormalizedTime)
{
	const float T = ClampTime(NormalizedTime);
	switch (Spec.Curve)
	{
	case ET66AnimationCurve::EaseInQuad:
		// Slow start, then quadratic acceleration.
		return T * T;

	case ET66AnimationCurve::EaseOutQuad:
		// Fast start, then quadratic deceleration.
		return 1.f - ((1.f - T) * (1.f - T));

	case ET66AnimationCurve::EaseInOutQuad:
		// Symmetric quadratic acceleration then deceleration.
		return T < 0.5f
			? 2.f * T * T
			: 1.f - FMath::Pow(-2.f * T + 2.f, 2.f) * 0.5f;

	case ET66AnimationCurve::EaseInCubic:
		// Stronger slow start with cubic acceleration.
		return T * T * T;

	case ET66AnimationCurve::EaseOutCubic:
		// Stronger deceleration into the target.
		return 1.f - FMath::Pow(1.f - T, 3.f);

	case ET66AnimationCurve::EaseInOutCubic:
		// Symmetric cubic acceleration then deceleration.
		return T < 0.5f
			? 4.f * T * T * T
			: 1.f - FMath::Pow(-2.f * T + 2.f, 3.f) * 0.5f;

	case ET66AnimationCurve::EaseInExpo:
		// Very slow start with exponential ramp.
		return T <= 0.f ? 0.f : FMath::Pow(2.f, 10.f * T - 10.f);

	case ET66AnimationCurve::EaseOutExpo:
		// Strong first movement that settles exponentially.
		return T >= 1.f ? 1.f : 1.f - FMath::Pow(2.f, -10.f * T);

	case ET66AnimationCurve::EaseInOutExpo:
		// Exponential acceleration and deceleration with a stable midpoint.
		if (T <= 0.f)
		{
			return 0.f;
		}
		if (T >= 1.f)
		{
			return 1.f;
		}
		return T < 0.5f
			? FMath::Pow(2.f, 20.f * T - 10.f) * 0.5f
			: (2.f - FMath::Pow(2.f, -20.f * T + 10.f)) * 0.5f;

	case ET66AnimationCurve::Anticipation:
	{
		// Pulls backward before moving forward, useful as the curve portion of an anticipation phase.
		const float Strength = FMath::Max(0.f, Spec.Strength);
		return T * T * ((Strength + 1.f) * T - Strength);
	}

	case ET66AnimationCurve::Overshoot:
	{
		// Moves past the target and returns to 1 at the end.
		const float Strength = FMath::Max(0.f, Spec.Strength);
		const float LocalT = T - 1.f;
		return 1.f + LocalT * LocalT * ((Strength + 1.f) * LocalT + Strength);
	}

	case ET66AnimationCurve::Settle:
	{
		// Dampened overshoot that arrives with less rebound than Elastic.
		const float Strength = FMath::Max(0.f, Spec.Strength);
		const float LocalT = T - 1.f;
		const float Back = 1.f + LocalT * LocalT * ((Strength + 1.f) * LocalT + Strength);
		const float Damping = 1.f - (0.16f * FMath::Sin(T * PI));
		return FMath::Lerp(T, Back, Damping);
	}

	case ET66AnimationCurve::Elastic:
	{
		// Overshoots with a damped sine wave, strongest near the end.
		if (T <= 0.f || T >= 1.f)
		{
			return T;
		}

		const float Amplitude = FMath::Max(0.01f, Spec.Amplitude);
		const float Frequency = FMath::Max(0.01f, Spec.Frequency);
		const float Period = 1.f / Frequency;
		const float S = Period / 4.f;
		return Amplitude * FMath::Pow(2.f, -10.f * T) * FMath::Sin((T - S) * (2.f * PI) / Period) + 1.f;
	}

	case ET66AnimationCurve::WeightedDecel:
		// Heavy deceleration: most travel happens early, with an extended final settle.
		return 1.f - FMath::Pow(1.f - T, 4.f);

	case ET66AnimationCurve::BounceIn:
		// Starts with small bounces, then lands at the target.
		return 1.f - EaseOutBounce(1.f - T);

	case ET66AnimationCurve::BounceOut:
		// Falls quickly, then bounces into the target.
		return EaseOutBounce(T);

	case ET66AnimationCurve::Linear:
	default:
		// Constant-speed interpolation.
		return T;
	}
}
