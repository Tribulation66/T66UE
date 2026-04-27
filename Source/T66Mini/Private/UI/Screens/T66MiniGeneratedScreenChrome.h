// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/CoreStyle.h"
#include "UI/T66MiniUIStyle.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
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

	inline const TCHAR* SliceFileName(const ESlice Slice)
	{
		switch (Slice)
		{
		case ESlice::TitlePlaque:
			return TEXT("title_plaque_heavy_wide.png");
		case ESlice::PanelLarge:
			return TEXT("panel_large_alpha.png");
		case ESlice::PanelMedium:
			return TEXT("panel_medium_alpha.png");
		case ESlice::PanelSmall:
			return TEXT("metric_chip_heavy.png");
		case ESlice::RowLong:
			return TEXT("footer_bar_heavy.png");
		case ESlice::StatChip:
			return TEXT("metric_chip_heavy.png");
		case ESlice::CardNormal:
			return TEXT("card_normal_alpha.png");
		case ESlice::CardSelected:
			return TEXT("card_selected_alpha.png");
		case ESlice::CardDisabled:
			return TEXT("card_disabled_alpha.png");
		case ESlice::PortraitFrame:
			return TEXT("portrait_frame_square_alpha.png");
		case ESlice::IdolOfferRow:
			return TEXT("idol_offer_row_alpha.png");
		case ESlice::IdolOfferRowAction:
			return TEXT("idol_offer_row_action_alpha.png");
		case ESlice::SummaryRow:
			return TEXT("summary_row_heavy.png");
		case ESlice::BadgeFrame:
			return TEXT("badge_frame_alpha.png");
		case ESlice::ButtonGreenNormal:
			return TEXT("button_green_heavy.png");
		case ESlice::ButtonBlueNormal:
			return TEXT("button_blue_heavy.png");
		case ESlice::ButtonPurpleNormal:
			return TEXT("button_purple_heavy.png");
		case ESlice::Count:
		default:
			return TEXT("panel_large_alpha.png");
		}
	}

	inline FMargin SliceMargin(const ESlice Slice)
	{
		switch (Slice)
		{
		case ESlice::TitlePlaque:
			return FMargin(0.22f, 0.34f, 0.22f, 0.34f);
		case ESlice::RowLong:
		case ESlice::IdolOfferRow:
		case ESlice::IdolOfferRowAction:
		case ESlice::SummaryRow:
			return FMargin(0.14f, 0.34f, 0.14f, 0.34f);
		case ESlice::StatChip:
		case ESlice::PanelSmall:
			return FMargin(0.18f, 0.32f, 0.18f, 0.32f);
		case ESlice::CardNormal:
		case ESlice::CardSelected:
		case ESlice::CardDisabled:
		case ESlice::PortraitFrame:
		case ESlice::BadgeFrame:
			return FMargin(0.18f, 0.16f, 0.18f, 0.16f);
		case ESlice::ButtonGreenNormal:
		case ESlice::ButtonBlueNormal:
		case ESlice::ButtonPurpleNormal:
			return FMargin(0.20f, 0.34f, 0.20f, 0.34f);
		case ESlice::PanelLarge:
		case ESlice::PanelMedium:
		default:
			return FMargin(0.18f, 0.20f, 0.18f, 0.20f);
		}
	}

	inline const FSlateBrush* SliceBrush(const ESlice Slice)
	{
		static TSharedPtr<T66RuntimeUIBrushAccess::FOptionalTextureBrush> Entries[static_cast<int32>(ESlice::Count)];
		const int32 Index = static_cast<int32>(Slice);
		if (Index < 0 || Index >= static_cast<int32>(ESlice::Count))
		{
			return nullptr;
		}

		if (!Entries[Index].IsValid())
		{
			Entries[Index] = MakeShared<T66RuntimeUIBrushAccess::FOptionalTextureBrush>();
		}

		const FString RelativePath = FString::Printf(
			TEXT("SourceAssets/UI/SettingsReference/MiniScreens/Common/slices/%s"),
			SliceFileName(Slice));
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			*Entries[Index],
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			SliceMargin(Slice),
			TEXT("MiniGeneratedChrome"));
	}

	inline TSharedRef<SWidget> MakePanel(
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const ESlice Slice = ESlice::PanelLarge)
	{
		const FSlateBrush* Brush = SliceBrush(Slice);
		return SNew(SBorder)
			.BorderImage(Brush ? Brush : T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(Brush ? FLinearColor::White : T66MiniUI::PanelFill())
			.Padding(Padding)
			[
				Content
			];
	}

	inline TSharedRef<SWidget> MakeRowPanel(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		return MakePanel(Content, Padding, ESlice::RowLong);
	}

	inline TSharedRef<SWidget> MakeTitlePlaque(
		const FText& Label,
		const int32 FontSize,
		const float Width,
		const float Height,
		const FLinearColor& Color = FLinearColor::White)
	{
		return SNew(SBox)
			.WidthOverride(Width)
			.HeightOverride(Height)
			[
				MakePanel(
					SNew(STextBlock)
					.Text(Label)
					.Font(T66MiniUI::TitleFont(FontSize))
					.ColorAndOpacity(Color)
					.Justification(ETextJustify::Center)
					.ShadowOffset(FVector2D(2.f, 2.f))
					.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.95f)),
					FMargin(58.f, 12.f, 58.f, 10.f),
					ESlice::TitlePlaque)
			];
	}

	inline const FSlateBrush* ButtonPlateBrush(const ET66ButtonType Type)
	{
		if (Type == ET66ButtonType::Success || Type == ET66ButtonType::ToggleActive)
		{
			return SliceBrush(ESlice::ButtonGreenNormal);
		}

		if (Type == ET66ButtonType::Primary || Type == ET66ButtonType::Danger)
		{
			return SliceBrush(ESlice::ButtonPurpleNormal);
		}

		return SliceBrush(ESlice::ButtonBlueNormal);
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
		Params.SetDotaPlateOverrideBrush(ButtonPlateBrush(Type));
		return Params;
	}

	inline TSharedRef<SWidget> MakeButton(
		const FT66ButtonParams& Params)
	{
		const FSlateBrush* PlateBrush = ButtonPlateBrush(Params.Type);
		const FMargin ContentPad = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(14.f, 8.f, 14.f, 6.f);
		const TAttribute<FText> TextAttr = Params.DynamicLabel.IsBound()
			? Params.DynamicLabel
			: TAttribute<FText>(Params.Label);
		const TAttribute<FSlateColor> TextColor = Params.bHasTextColorOverride
			? Params.TextColorOverride
			: TAttribute<FSlateColor>(FSlateColor(T66MiniUI::Text()));

		return SNew(SBox)
			.MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
			.HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize())
			.Visibility(Params.Visibility)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Visibility(PlateBrush ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
					.Image(PlateBrush)
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SOverlay::Slot()
				[
					SNew(SButton)
					.Cursor(EMouseCursor::Hand)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked(Params.OnClicked)
					.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
					.ButtonColorAndOpacity(FLinearColor::Transparent)
					.ContentPadding(ContentPad)
					.IsEnabled(Params.IsEnabled)
					[
						SNew(STextBlock)
						.Text(TextAttr)
						.Font(T66MiniUI::BoldFont(Params.FontSize > 0 ? Params.FontSize : 16))
						.ColorAndOpacity(TextColor)
						.Justification(ETextJustify::Center)
						.ShadowOffset(FVector2D(1.f, 1.f))
						.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.72f))
					]
				]
			];
	}

	inline TSharedRef<SWidget> MakeButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		const ET66ButtonType Type,
		const float MinWidth,
		const float Height,
		const int32 FontSize)
	{
		return MakeButton(MakeButtonParams(Label, OnClicked, Type, MinWidth, Height, FontSize));
	}
}
