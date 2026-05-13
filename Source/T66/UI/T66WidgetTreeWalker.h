// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class SWidget;

class T66_API FT66WidgetTreeWalker
{
public:
	static bool DumpWidgetTreeToJson(
		const TSharedRef<SWidget>& RootWidget,
		const FString& ScreenName,
		const FVector2D& ViewportSize,
		const FString& OutputPath,
		FString& OutError);
};
