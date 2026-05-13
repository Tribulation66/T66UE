// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/CoreStyle.h"
#include "UI/T66MiniUIStyle.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66Style.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace T66MiniGeneratedChrome
{
	enum class ESlice : uint8
	{
		TitlePlaque,
		PanelLarge,
		PanelMedium,
		PanelSmall,
		RowLong,
		StatChip,
		CardNormal,
		CardSelected,
		CardDisabled,
		PortraitFrame,
		IdolOfferRow,
		IdolOfferRowAction,
		SummaryRow,
		BadgeFrame,
		ButtonGreenNormal,
		ButtonBlueNormal,
		ButtonPurpleNormal,
		Count
	};

	inline ET66FlatState StateForSlice(const ESlice Slice)
	{
		switch (Slice)
		{
		case ESlice::CardSelected:
		case ESlice::ButtonGreenNormal:
			return ET66FlatState::Selected;
		case ESlice::CardDisabled:
			return ET66FlatState::Disabled;
		case ESlice::Count:
		default:
			return ET66FlatState::Default;
		}
	}

	inline ET66FlatState StateForButtonType(const ET66ButtonType Type)
	{
		switch (Type)
		{
		case ET66ButtonType::Success:
		case ET66ButtonType::Primary:
		case ET66ButtonType::ToggleActive:
			return ET66FlatState::Selected;
		case ET66ButtonType::Danger:
			return ET66FlatState::Selected;
		case ET66ButtonType::Neutral:
		default:
			return ET66FlatState::Default;
		}
	}

	inline FString SanitizeTagSegment(const FText& Label)
	{
		FString Segment = Label.ToString();
		const TCHAR* Removals[] = { TEXT(" "), TEXT("'"), TEXT("\""), TEXT("-"), TEXT(":"), TEXT("/"), TEXT("."), TEXT(",") };
		for (const TCHAR* Removal : Removals)
		{
			Segment.ReplaceInline(Removal, TEXT(""));
		}
		if (Segment.IsEmpty())
		{
			Segment = TEXT("Unnamed");
		}
		return Segment;
	}

	inline FMargin ContentSafePadding(const ESlice Slice, const FMargin& Padding)
	{
		switch (Slice)
		{
		case ESlice::PanelLarge:
		case ESlice::PanelMedium:
			return Padding + FMargin(10.f, 4.f, 12.f, 4.f);
		case ESlice::PanelSmall:
		case ESlice::RowLong:
		case ESlice::StatChip:
		case ESlice::IdolOfferRow:
		case ESlice::IdolOfferRowAction:
		case ESlice::SummaryRow:
			return Padding + FMargin(8.f, 3.f, 10.f, 3.f);
		default:
			return Padding;
		}
	}

	inline TSharedRef<SWidget> MakePanel(
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const ESlice Slice = ESlice::PanelLarge,
		const FName Tag = NAME_None)
	{
		const ET66FlatState State = StateForSlice(Slice);
		const FMargin ResolvedPadding = ContentSafePadding(Slice, Padding);
		switch (Slice)
		{
		case ESlice::PanelSmall:
		case ESlice::RowLong:
		case ESlice::StatChip:
		case ESlice::CardNormal:
		case ESlice::CardSelected:
		case ESlice::CardDisabled:
		case ESlice::PortraitFrame:
		case ESlice::IdolOfferRow:
		case ESlice::IdolOfferRowAction:
		case ESlice::SummaryRow:
		case ESlice::BadgeFrame:
			return FT66FlatStyle::MakeFlatSubPanel(State, ResolvedPadding, Content, nullptr, Tag);
		case ESlice::TitlePlaque:
		case ESlice::PanelLarge:
		case ESlice::PanelMedium:
		default:
			return FT66FlatStyle::MakeFlatPanel(State, ResolvedPadding, Content, nullptr, Tag);
		}
	}

	inline TSharedRef<SWidget> MakeRowPanel(const TSharedRef<SWidget>& Content, const FMargin& Padding, const FName Tag = NAME_None)
	{
		return MakePanel(Content, Padding, ESlice::RowLong, Tag);
	}

	inline TSharedRef<SWidget> MakeTitlePlaque(
		const FText& Label,
		const int32 FontSize,
		const float Width,
		const float Height,
		const FLinearColor& Color = FLinearColor::White,
		const FName Tag = NAME_None)
	{
		const FName ResolvedTag = Tag.IsNone()
			? FName(*FString::Printf(TEXT("MiniGenerated.Title.%s"), *SanitizeTagSegment(Label)))
			: Tag;
		return SNew(SBox)
			.WidthOverride(Width)
			.HeightOverride(Height)
			[
				MakePanel(
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					.StretchDirection(EStretchDirection::DownOnly)
					[
						SNew(STextBlock)
						.Text(Label)
						.Font(FT66FlatStyle::MakeBoldFont(FontSize))
						.ColorAndOpacity(Color)
						.Justification(ETextJustify::Center)
					],
					FMargin(58.f, 12.f, 58.f, 10.f),
					ESlice::TitlePlaque,
					ResolvedTag)
			];
	}

	inline FT66ButtonParams MakeButtonParams(
		const FText& Label,
		const FOnClicked& OnClicked,
		const ET66ButtonType Type,
		const float MinWidth,
		const float Height,
		const int32 FontSize)
	{
		FT66ButtonParams Params = T66MiniUI::MakeButtonParams(Label, OnClicked, Type, MinWidth, Height, FontSize);
		return Params;
	}

	inline TSharedRef<SWidget> MakeButton(
		const FT66ButtonParams& Params,
		const FName Tag = NAME_None,
		const FName ToggleGroup = NAME_None)
	{
		FT66FlatButtonParams FlatParams;
		FlatParams.State = StateForButtonType(Params.Type);
		FlatParams.Label = Params.DynamicLabel.IsSet() ? Params.DynamicLabel : TAttribute<FText>(Params.Label);
		FlatParams.OnClicked = Params.OnClicked;
		FlatParams.Padding = Params.Padding.Left < 0.f ? FMargin(14.f, 8.f) : Params.Padding;
		FlatParams.MinWidth = Params.MinWidth;
		FlatParams.Height = Params.Height;
		FlatParams.IsEnabled = Params.IsEnabled;
		FlatParams.FontSize = Params.FontSize > 0 ? Params.FontSize : 18;
		FlatParams.Tag = Tag.IsNone()
			? FName(*FString::Printf(TEXT("MiniGenerated.Button.%s"), *SanitizeTagSegment(Params.Label)))
			: Tag;
		FlatParams.ToggleGroup = ToggleGroup;
		return FT66FlatStyle::MakeFlatButton(FlatParams);
	}

	inline TSharedRef<SWidget> MakeButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		const ET66ButtonType Type,
		const float MinWidth,
		const float Height,
		const int32 FontSize,
		const FName Tag = NAME_None,
		const FName ToggleGroup = NAME_None)
	{
		return MakeButton(MakeButtonParams(Label, OnClicked, Type, MinWidth, Height, FontSize), Tag, ToggleGroup);
	}
}
