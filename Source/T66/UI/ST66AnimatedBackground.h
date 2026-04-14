// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66UITween.h"
#include "Widgets/SCompoundWidget.h"

class SImage;
struct FSlateBrush;

struct FT66AnimatedBackgroundLayer
{
	FSlateBrush* Brush = nullptr;
	FVector2D BaseOffset = FVector2D::ZeroVector;
	FVector2D SwayAmplitude = FVector2D::ZeroVector;
	float SwayFrequency = 0.f;
	float ScalePulseAmplitude = 0.f;
	float ScalePulseFrequency = 0.f;
	float OpacityMin = 1.f;
	float OpacityMax = 1.f;
	float OpacityFrequency = 0.f;
	float PhaseOffset = 0.f;
};

class T66_API ST66AnimatedBackground : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(ST66AnimatedBackground) {}
		SLATE_ARGUMENT(TArray<FT66AnimatedBackgroundLayer>, Layers)
	SLATE_END_ARGS()

	~ST66AnimatedBackground();
	void Construct(const FArguments& InArgs);

private:
	struct FRuntimeLayer
	{
		FT66AnimatedBackgroundLayer Definition;
		TSharedPtr<SImage> ImageWidget;
		FT66UITween SwayXTween;
		FT66UITween SwayYTween;
		FT66UITween ScaleTween;
		FT66UITween OpacityTween;
		float OpacityBias = 1.f;
		FVector2D CurrentOffset = FVector2D::ZeroVector;
		float CurrentScale = 1.f;
		float CurrentOpacity = 1.f;
	};

	EActiveTimerReturnType HandleActiveTimer(double InCurrentTime, float InDeltaTime);
	void TickLayer(FRuntimeLayer& Layer, float DeltaTime);

	TArray<FRuntimeLayer> RuntimeLayers;
	TSharedPtr<FActiveTimerHandle> ActiveTimerHandle;
};
