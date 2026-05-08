// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66ButtonVisuals.h"
#include "UI/Style/T66Style.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66ButtonVisuals, Log, All);

TSharedPtr<FT66ButtonBorderBrushSet> FT66ButtonVisuals::CreateBorderBrushSet(ET66ButtonBorderVisual Visual)
{
	(void)Visual;
	return nullptr;
}

TSharedPtr<FT66ButtonFillBrushSet> FT66ButtonVisuals::CreateFillBrushSet(ET66ButtonBackgroundVisual Visual)
{
	(void)Visual;
	return nullptr;
}
