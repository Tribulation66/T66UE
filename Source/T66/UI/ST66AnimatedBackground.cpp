// Copyright Tribulation 66. All Rights Reserved.

#include "UI/ST66AnimatedBackground.h"

#include "Math/TransformCalculus2D.h"
#include "Rendering/SlateRenderTransform.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SOverlay.h"

namespace
{
	FSlateRenderTransform MakeLayerTransform(float Scale, const FVector2D& Offset)
	{
		return TransformCast<FSlateRenderTransform>(Concatenate(FScale2D(Scale, Scale), Offset));
	}
}

void ST66AnimatedBackground::Construct(const FArguments& InArgs)
{
	RuntimeLayers.Reset();
	RuntimeLayers.Reserve(InArgs._Layers.Num());

	TSharedRef<SOverlay> RootOverlay = SNew(SOverlay);

	for (const FT66AnimatedBackgroundLayer& LayerDefinition : InArgs._Layers)
	{
		FRuntimeLayer& RuntimeLayer = RuntimeLayers.AddDefaulted_GetRef();
		RuntimeLayer.Definition = LayerDefinition;
		RuntimeLayer.SwayXTween = FT66UITween(ET66UITweenMode::Sine, LayerDefinition.SwayAmplitude.X, LayerDefinition.SwayFrequency, LayerDefinition.PhaseOffset);
		RuntimeLayer.SwayYTween = FT66UITween(ET66UITweenMode::Sine, LayerDefinition.SwayAmplitude.Y, LayerDefinition.SwayFrequency, LayerDefinition.PhaseOffset + 0.17f);
		RuntimeLayer.ScaleTween = FT66UITween(ET66UITweenMode::Sine, LayerDefinition.ScalePulseAmplitude, LayerDefinition.ScalePulseFrequency, LayerDefinition.PhaseOffset + 0.33f);
		RuntimeLayer.OpacityTween = FT66UITween(
			ET66UITweenMode::Sine,
			(LayerDefinition.OpacityMax - LayerDefinition.OpacityMin) * 0.5f,
			LayerDefinition.OpacityFrequency,
			LayerDefinition.PhaseOffset + 0.49f);
		RuntimeLayer.OpacityBias = (LayerDefinition.OpacityMin + LayerDefinition.OpacityMax) * 0.5f;

		RootOverlay->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(RuntimeLayer.ImageWidget, SImage)
			.Image(LayerDefinition.Brush)
		];

		if (RuntimeLayer.ImageWidget.IsValid())
		{
			RuntimeLayer.ImageWidget->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		}

		TickLayer(RuntimeLayer, 0.f);
	}

	ChildSlot
	[
		RootOverlay
	];

	RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &ST66AnimatedBackground::HandleActiveTimer));
}

EActiveTimerReturnType ST66AnimatedBackground::HandleActiveTimer(double InCurrentTime, float InDeltaTime)
{
	for (FRuntimeLayer& Layer : RuntimeLayers)
	{
		TickLayer(Layer, InDeltaTime);
	}

	Invalidate(EInvalidateWidgetReason::Paint);
	return EActiveTimerReturnType::Continue;
}

void ST66AnimatedBackground::TickLayer(FRuntimeLayer& Layer, float DeltaTime)
{
	Layer.CurrentOffset = FVector2D(Layer.SwayXTween.Tick(DeltaTime), Layer.SwayYTween.Tick(DeltaTime));
	Layer.CurrentScale = 1.f + Layer.ScaleTween.Tick(DeltaTime);
	Layer.CurrentOpacity = FMath::Clamp(
		Layer.OpacityBias + Layer.OpacityTween.Tick(DeltaTime),
		0.f,
		1.f);

	if (Layer.ImageWidget.IsValid())
	{
		Layer.ImageWidget->SetRenderTransform(MakeLayerTransform(Layer.CurrentScale, Layer.CurrentOffset));
		Layer.ImageWidget->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Layer.CurrentOpacity));
	}
}
