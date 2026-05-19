// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"

/**
 * Classification namespace for UI that changes over time or renders gameplay
 * readability content. This intentionally does not define an animated visual
 * treatment yet; future helpers belong here once that treatment is approved.
 */
class T66_API FT66AnimatedStyle
{
public:
	static TSharedRef<SWidget> AttachMetadata(
		const TSharedRef<SWidget>& Widget,
		FName Tag = NAME_None,
		const FString& AnimatedRole = TEXT("Animated"));
};
