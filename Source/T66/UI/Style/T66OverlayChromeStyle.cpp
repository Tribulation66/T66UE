// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66OverlayChromeStyle.h"

#include "UI/Style/T66FlatStyle.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"

namespace
{
	ET66FlatState StateForBrush(const ET66OverlayChromeBrush Brush)
	{
		switch (Brush)
		{
		case ET66OverlayChromeBrush::SlotSelected:
		case ET66OverlayChromeBrush::OfferCardSelected:
		case ET66OverlayChromeBrush::CrateWinnerMarker:
			return ET66FlatState::Selected;
		case ET66OverlayChromeBrush::SlotDisabled:
		case ET66OverlayChromeBrush::OfferCardDisabled:
			return ET66FlatState::Disabled;
		case ET66OverlayChromeBrush::SlotHover:
		case ET66OverlayChromeBrush::OfferCardHover:
			return ET66FlatState::Ready;
		default:
			return ET66FlatState::Default;
		}
	}

	ET66FlatState StateForButton(
		const ET66OverlayChromeButtonFamily Family,
		const bool bSelected,
		const bool bEnabled)
	{
		if (!bEnabled)
		{
			return ET66FlatState::Disabled;
		}
		if (bSelected || Family == ET66OverlayChromeButtonFamily::Danger)
		{
			return ET66FlatState::Selected;
		}
		if (Family == ET66OverlayChromeButtonFamily::Primary
			|| Family == ET66OverlayChromeButtonFamily::Central
			|| Family == ET66OverlayChromeButtonFamily::Select)
		{
			return ET66FlatState::Ready;
		}
		return ET66FlatState::Default;
	}
}

namespace T66OverlayChromeStyle
{
	const FSlateBrush* GetBrush(const ET66OverlayChromeBrush Brush)
	{
		return FCoreStyle::Get().GetBrush("WhiteBrush");
	}

	FT66OverlayChromeButtonParams MakeButtonParams(
		const FText& Label,
		FOnClicked OnClicked,
		const ET66OverlayChromeButtonFamily Family)
	{
		return FT66OverlayChromeButtonParams(Label, MoveTemp(OnClicked), Family);
	}

	TSharedRef<SWidget> MakePanel(
		const TSharedRef<SWidget>& Content,
		const ET66OverlayChromeBrush Brush,
		const FMargin& Padding,
		TSharedPtr<SBorder>* OutBorder)
	{
		return FT66FlatStyle::MakeFlatPanel(StateForBrush(Brush), Padding, Content, OutBorder);
	}

	TSharedRef<SWidget> MakeButton(const FT66OverlayChromeButtonParams& Params)
	{
		const ET66FlatState State = StateForButton(
			Params.Family,
			Params.IsSelected.Get(false),
			Params.IsEnabled.Get(true));

		if (Params.CustomContent.IsValid())
		{
			return FT66FlatStyle::MakeFlatToggleGroupButton(
				State,
				Params.CustomContent.ToSharedRef(),
				Params.OnClicked,
				Params.Padding,
				Params.MinWidth,
				Params.MinHeight,
				Params.IsEnabled);
		}

		return FT66FlatStyle::MakeFlatButton(
			State,
			Params.Label,
			Params.OnClicked,
			nullptr,
			nullptr,
			Params.Padding,
			Params.MinWidth,
			Params.MinHeight,
			Params.IsEnabled,
			Params.FontSize);
	}

	TSharedRef<SWidget> MakeSlotPanel(
		const TSharedRef<SWidget>& Content,
		const TAttribute<bool>& IsSelected,
		const TAttribute<bool>& IsEnabled,
		const FMargin& Padding)
	{
		const ET66FlatState State = !IsEnabled.Get(true)
			? ET66FlatState::Disabled
			: (IsSelected.Get(false) ? ET66FlatState::Selected : ET66FlatState::Default);

		return FT66FlatStyle::MakeFlatPanel(State, Padding, Content);
	}
}
