// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "UI/Style/T66Style.h"

class SGridPanel;
class SWidget;
class UT66UIManager;

namespace T66ScreenSlateHelpers
{
	struct FFrontendChromeMetrics
	{
		int32 TitleFontSize = 54;
		int32 TabFontSize = 28;
		float TabMinWidth = 224.0f;
		float TabHeight = 62.0f;
		float TopBarOverlapPx = 8.0f;
		FMargin HeaderPadding = FMargin(0.0f, 0.0f, 0.0f, 8.0f);
		FMargin TabPadding = FMargin(18.0f, 8.0f, 18.0f, 6.0f);
	};

	struct FResponsiveGridModalMetrics
	{
		int32 Columns = 1, Rows = 1;
		float ModalWidth = 0.0f, ModalHeight = 0.0f;
		float ModalPaddingX = 30.0f, ModalPaddingY = 25.0f;
		float TitleSectionHeight = 58.0f, FooterSectionHeight = 78.0f;
		float TileGap = 6.0f, TileSize = 64.0f;
		float GridWidth = 64.0f, GridHeight = 64.0f;
	};

	const FFrontendChromeMetrics& GetFrontendChromeMetrics();
	float GetFrontendChromeTopInset(const UT66UIManager* UIManager);
	FSlateFontInfo MakeFrontendChromeTitleFont();
	FSlateFontInfo MakeFrontendChromeTabFont();
	TSharedPtr<FSlateBrush> MakeSlateBrush(const FVector2D& ImageSize = FVector2D::ZeroVector, ESlateBrushDrawType::Type DrawAs = ESlateBrushDrawType::Image);
	FResponsiveGridModalMetrics MakeResponsiveGridModalMetrics(int32 ItemCount, const FVector2D& SafeFrameSize);
	void AddUniformGridPaddingSlots(SGridPanel& GridPanel, int32 FilledSlotCount, const FResponsiveGridModalMetrics& Metrics);
	TSharedRef<SWidget> MakeFilledButtonText(
		const FT66ButtonParams& Params,
		float ButtonHeight,
		const TAttribute<FSlateColor>& DefaultTextColor,
		const TAttribute<FLinearColor>& DefaultShadowColor);
	TSharedRef<SWidget> MakeResponsiveGridTile(const FT66ButtonParams& ButtonParams, const FLinearColor& BackgroundColor, const TSharedRef<SWidget>& Content, const FResponsiveGridModalMetrics& Metrics);
	TSharedRef<SWidget> MakeResponsiveGridModal(const FText& TitleText, const TSharedRef<SWidget>& GridContent, const TSharedRef<SWidget>& FooterContent, const FResponsiveGridModalMetrics& Metrics);
	TSharedRef<SWidget> MakeCenteredScrimModal(const TSharedRef<SWidget>& Content, const FMargin& OuterPadding = FMargin(0.0f), float WidthOverride = 0.0f, float HeightOverride = 0.0f, bool bUseWhiteBrush = false);
	TSharedRef<SWidget> MakeTwoButtonRow(const TSharedRef<SWidget>& LeftButton, const TSharedRef<SWidget>& RightButton, const FMargin& LeftPadding = FMargin(10.0f, 0.0f), const FMargin& RightPadding = FMargin(10.0f, 0.0f), EVisibility Visibility = EVisibility::Visible);
}
