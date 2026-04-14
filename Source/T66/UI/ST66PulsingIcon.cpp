// Copyright Tribulation 66. All Rights Reserved.

#include "UI/ST66PulsingIcon.h"

#include "Math/TransformCalculus2D.h"
#include "Rendering/SlateRenderTransform.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SOverlay.h"

namespace
{
	FSlateRenderTransform MakeGlowTransform(float Scale)
	{
		return TransformCast<FSlateRenderTransform>(FScale2D(Scale, Scale));
	}
}

ST66PulsingIcon::~ST66PulsingIcon()
{
	if (ActiveTimerHandle.IsValid())
	{
		UnRegisterActiveTimer(ActiveTimerHandle.ToSharedRef());
		ActiveTimerHandle.Reset();
	}
}

void ST66PulsingIcon::Construct(const FArguments& InArgs)
{
	const FVector2D IconSize = InArgs._IconSize.IsNearlyZero() ? FVector2D(28.f, 28.f) : InArgs._IconSize;
	const FVector2D GlowSize = InArgs._GlowSize.IsNearlyZero() ? FVector2D(50.f, 50.f) : InArgs._GlowSize;
	const float GlowOpacityMin = FMath::Clamp(InArgs._GlowOpacityMin, 0.f, 1.f);
	const float GlowOpacityMax = FMath::Max(GlowOpacityMin, FMath::Clamp(InArgs._GlowOpacityMax, 0.f, 1.f));

	GlowOpacityTween = FT66UITween(
		ET66UITweenMode::Sine,
		(GlowOpacityMax - GlowOpacityMin) * 0.5f,
		FMath::Max(0.f, InArgs._GlowOpacityFrequency));
	GlowScaleTween = FT66UITween(
		ET66UITweenMode::Sine,
		FMath::Max(0.f, InArgs._GlowScaleAmplitude),
		FMath::Max(0.f, InArgs._GlowScaleFrequency),
		0.27f);
	GlowOpacityBias = (GlowOpacityMin + GlowOpacityMax) * 0.5f;

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(GlowImageWidget, SImage)
			.Image(InArgs._GlowBrush)
			.Visibility(InArgs._GlowBrush ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
			.DesiredSizeOverride(GlowSize)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(IconImageWidget, SImage)
			.Image(InArgs._IconBrush)
			.DesiredSizeOverride(IconSize)
		]
	];

	if (GlowImageWidget.IsValid())
	{
		GlowImageWidget->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	}

	CurrentGlowOpacity = GlowOpacityBias;
	CurrentGlowScale = 1.f;
	TickGlow(0.f);

	if (InArgs._GlowBrush)
	{
		ActiveTimerHandle = RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &ST66PulsingIcon::HandleActiveTimer));
	}
}

EActiveTimerReturnType ST66PulsingIcon::HandleActiveTimer(double InCurrentTime, float InDeltaTime)
{
	TickGlow(InDeltaTime);
	Invalidate(EInvalidateWidgetReason::Paint);
	return EActiveTimerReturnType::Continue;
}

void ST66PulsingIcon::TickGlow(float DeltaTime)
{
	CurrentGlowOpacity = FMath::Clamp(GlowOpacityBias + GlowOpacityTween.Tick(DeltaTime), 0.f, 1.f);
	CurrentGlowScale = 1.f + GlowScaleTween.Tick(DeltaTime);

	if (GlowImageWidget.IsValid())
	{
		GlowImageWidget->SetRenderTransform(MakeGlowTransform(CurrentGlowScale));
		GlowImageWidget->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, CurrentGlowOpacity));
	}
}
