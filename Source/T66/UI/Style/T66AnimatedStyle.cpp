// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66AnimatedStyle.h"

#include "UI/Style/T66FlatStyle.h"

TSharedRef<SWidget> FT66AnimatedStyle::AttachMetadata(
	const TSharedRef<SWidget>& Widget,
	const FName Tag,
	const FString& AnimatedRole)
{
	const FString Role = AnimatedRole.IsEmpty()
		? TEXT("Animated")
		: FString::Printf(TEXT("Animated.%s"), *AnimatedRole);
	return FT66FlatStyle::AttachMetadata(Widget, Tag, Role, ET66FlatState::Default);
}
