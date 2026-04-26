// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Input/SButton.h"

class SBorder;

enum class ET66OverlayChromeBrush : uint8
{
	OverlayModalPanel,
	CasinoShellPanel,
	ContentPanelWide,
	ContentPanelTall,
	InnerPanel,
	HeaderSummaryBar,
	CrateStripFrame,
	SlotNormal,
	SlotHover,
	SlotSelected,
	SlotDisabled,
	OfferCardNormal,
	OfferCardHover,
	OfferCardSelected,
	OfferCardDisabled,
	CrateWinnerMarker,
};

enum class ET66OverlayChromeButtonFamily : uint8
{
	Neutral,
	Primary,
	Danger,
	Tab,
};

struct T66_API FT66OverlayChromeButtonParams
{
	FText Label;
	FOnClicked OnClicked;
	ET66OverlayChromeButtonFamily Family = ET66OverlayChromeButtonFamily::Neutral;
	float MinWidth = 120.f;
	float MinHeight = 44.f;
	int32 FontSize = 14;
	FMargin Padding = FMargin(12.f, 5.f);
	TAttribute<bool> IsEnabled = true;
	TAttribute<bool> IsSelected = false;
	TSharedPtr<SWidget> CustomContent;

	FT66OverlayChromeButtonParams() = default;
	FT66OverlayChromeButtonParams(
		const FText& InLabel,
		FOnClicked InOnClicked,
		const ET66OverlayChromeButtonFamily InFamily = ET66OverlayChromeButtonFamily::Neutral)
		: Label(InLabel)
		, OnClicked(MoveTemp(InOnClicked))
		, Family(InFamily)
	{
	}

	FT66OverlayChromeButtonParams& SetMinWidth(const float InMinWidth) { MinWidth = InMinWidth; return *this; }
	FT66OverlayChromeButtonParams& SetMinHeight(const float InMinHeight) { MinHeight = InMinHeight; return *this; }
	FT66OverlayChromeButtonParams& SetFontSize(const int32 InFontSize) { FontSize = InFontSize; return *this; }
	FT66OverlayChromeButtonParams& SetPadding(const FMargin& InPadding) { Padding = InPadding; return *this; }
	FT66OverlayChromeButtonParams& SetEnabled(const TAttribute<bool>& InIsEnabled) { IsEnabled = InIsEnabled; return *this; }
	FT66OverlayChromeButtonParams& SetSelected(const TAttribute<bool>& InIsSelected) { IsSelected = InIsSelected; return *this; }
	FT66OverlayChromeButtonParams& SetContent(const TSharedRef<SWidget>& InContent) { CustomContent = InContent; return *this; }
};

namespace T66OverlayChromeStyle
{
	T66_API const FSlateBrush* GetBrush(ET66OverlayChromeBrush Brush);

	T66_API FT66OverlayChromeButtonParams MakeButtonParams(
		const FText& Label,
		FOnClicked OnClicked,
		ET66OverlayChromeButtonFamily Family = ET66OverlayChromeButtonFamily::Neutral);

	T66_API TSharedRef<SWidget> MakePanel(
		const TSharedRef<SWidget>& Content,
		ET66OverlayChromeBrush Brush = ET66OverlayChromeBrush::ContentPanelWide,
		const FMargin& Padding = FMargin(16.f),
		TSharedPtr<SBorder>* OutBorder = nullptr);

	T66_API TSharedRef<SWidget> MakeButton(const FT66OverlayChromeButtonParams& Params);

	T66_API TSharedRef<SWidget> MakeSlotPanel(
		const TSharedRef<SWidget>& Content,
		const TAttribute<bool>& IsSelected = false,
		const TAttribute<bool>& IsEnabled = true,
		const FMargin& Padding = FMargin(6.f));
}
