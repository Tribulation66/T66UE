// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Animation/T66SlateAnimationBindings.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Rendering/SlateRenderTransform.h"
#include "Widgets/SWidget.h"

namespace T66SlateAnimationBindings
{
	TFunction<void(float)> MakeOpacitySetter(TWeakPtr<SWidget> WidgetWeak)
	{
		return [WidgetWeak](const float Value)
		{
			if (TSharedPtr<SWidget> Widget = WidgetWeak.Pin())
			{
				Widget->SetRenderOpacity(Value);
				Widget->Invalidate(EInvalidateWidgetReason::Paint);
			}
		};
	}

	TFunction<void(float)> MakeTranslationSetter(TWeakPtr<SWidget> WidgetWeak, EAxis::Type Axis)
	{
		return [WidgetWeak, Axis](const float Value)
		{
			if (TSharedPtr<SWidget> Widget = WidgetWeak.Pin())
			{
				FVector2D Translation = FVector2D::ZeroVector;
				if (Axis == EAxis::X)
				{
					Translation.X = Value;
				}
				else if (Axis == EAxis::Y)
				{
					Translation.Y = Value;
				}

				Widget->SetRenderTransform(FSlateRenderTransform(Translation));
				Widget->Invalidate(EInvalidateWidgetReason::Paint);
			}
		};
	}

	TFunction<void(float)> MakeScaleSetter(TWeakPtr<SWidget> WidgetWeak)
	{
		return [WidgetWeak](const float Value)
		{
			if (TSharedPtr<SWidget> Widget = WidgetWeak.Pin())
			{
				Widget->SetRenderTransform(FSlateRenderTransform(FTransform2D(FScale2D(Value, Value))));
				Widget->Invalidate(EInvalidateWidgetReason::Paint);
			}
		};
	}

	TFunction<void(float)> MakeMaterialScalarSetter(UMaterialInstanceDynamic* Material, FName ParameterName)
	{
		TWeakObjectPtr<UMaterialInstanceDynamic> MaterialWeak(Material);
		return [MaterialWeak, ParameterName](const float Value)
		{
			if (UMaterialInstanceDynamic* MaterialPtr = MaterialWeak.Get())
			{
				MaterialPtr->SetScalarParameterValue(ParameterName, Value);
			}
		};
	}
}
