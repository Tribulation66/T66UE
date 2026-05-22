// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class SWidget;
class UMaterialInstanceDynamic;

namespace T66SlateAnimationBindings
{
	TFunction<void(float)> MakeOpacitySetter(TWeakPtr<SWidget> WidgetWeak);
	TFunction<void(float)> MakeTranslationSetter(TWeakPtr<SWidget> WidgetWeak, EAxis::Type Axis);
	TFunction<void(float)> MakeScaleSetter(TWeakPtr<SWidget> WidgetWeak);
	TFunction<void(float)> MakeMaterialScalarSetter(UMaterialInstanceDynamic* Material, FName ParameterName);
}
