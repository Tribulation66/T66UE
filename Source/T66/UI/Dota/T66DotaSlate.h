// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Margin.h"
#include "Styling/SlateColor.h"
#include "Templates/SharedPointer.h"

class SWidget;
struct FSlateBrush;

enum class ET66DotaButtonPlateType : uint8
{
	Neutral,
	Primary,
	Danger,
};

class T66_API FT66DotaSlate
{
public:
	static TSharedRef<SWidget> MakeScreenSurface(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(14.f));
	static TSharedRef<SWidget> MakeViewportFrame(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(6.f));
	static TSharedRef<SWidget> MakeViewportCutoutFrame(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(6.f));
	static TSharedRef<SWidget> MakeSlotFrame(const TSharedRef<SWidget>& Content, const TAttribute<FSlateColor>& AccentColor, const FMargin& Padding = FMargin(1.f));
	static TSharedRef<SWidget> MakeSlotFrame(const TSharedRef<SWidget>& Content, const FLinearColor& AccentColor, const FMargin& Padding = FMargin(1.f));
	static TSharedRef<SWidget> MakeHudPanel(const TSharedRef<SWidget>& Content, const FText& Title, const FMargin& Padding = FMargin(12.f, 10.f));
	static TSharedRef<SWidget> MakeHudPanel(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(12.f, 10.f));
	static TSharedRef<SWidget> MakeDivider(float Height = 1.f);
	static TSharedRef<SWidget> MakeMinimapFrame(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(10.f));
	static const FSlateBrush* GetButtonPlateBrush(ET66DotaButtonPlateType Type);
	static const FSlateBrush* GetInventorySlotFrameBrush();
};
