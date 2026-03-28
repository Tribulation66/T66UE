// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

enum class ET66UITweenMode : uint8
{
	Sine,
	PingPong,
	Linear,
};

struct FT66UITween
{
	ET66UITweenMode Mode = ET66UITweenMode::Sine;
	float Amplitude = 0.f;
	float Frequency = 0.f;
	float PhaseOffset = 0.f;
	float ElapsedTime = 0.f;

	FT66UITween() = default;

	FT66UITween(ET66UITweenMode InMode, float InAmplitude, float InFrequency, float InPhaseOffset = 0.f)
		: Mode(InMode)
		, Amplitude(InAmplitude)
		, Frequency(InFrequency)
		, PhaseOffset(InPhaseOffset)
	{
	}

	void Reset()
	{
		ElapsedTime = 0.f;
	}

	float Tick(float DeltaTime)
	{
		ElapsedTime += DeltaTime;

		const float SafeFrequency = FMath::Max(0.f, Frequency);
		const float Phase = (ElapsedTime * SafeFrequency) + PhaseOffset;

		switch (Mode)
		{
		case ET66UITweenMode::PingPong:
		{
			const float Alpha = FMath::Frac(Phase);
			const float Triangle = (Alpha <= 0.5f)
				? (Alpha * 2.f)
				: (2.f - (Alpha * 2.f));
			return FMath::Lerp(-Amplitude, Amplitude, Triangle);
		}

		case ET66UITweenMode::Linear:
		{
			const float Alpha = FMath::Frac(Phase);
			return FMath::Lerp(-Amplitude, Amplitude, Alpha);
		}

		case ET66UITweenMode::Sine:
		default:
			return Amplitude * FMath::Sin(Phase * 2.f * PI);
		}
	}
};
