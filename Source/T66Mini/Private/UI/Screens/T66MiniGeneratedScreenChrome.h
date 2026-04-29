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

	inline const TCHAR* SliceRelativePath(const ESlice Slice)
	{
		switch (Slice)
		{
		case ESlice::TitlePlaque:
		case ESlice::PanelLarge:
		case ESlice::PanelMedium:
			return T66MiniUI::MasterBasicPanelPath();
		case ESlice::PanelSmall:
		case ESlice::RowLong:
		case ESlice::StatChip:
		case ESlice::IdolOfferRow:
		case ESlice::IdolOfferRowAction:
		case ESlice::SummaryRow:
			return T66MiniUI::MasterInnerPanelPath();
		case ESlice::CardNormal:
		case ESlice::CardSelected:
		case ESlice::CardDisabled:
		case ESlice::PortraitFrame:
		case ESlice::BadgeFrame:
			return TEXT("SourceAssets/UI/MasterLibrary/Slices/Slots/basic_slot_normal.png");
		case ESlice::ButtonGreenNormal:
			return T66MiniUI::MasterSelectedButtonPath();
		case ESlice::ButtonBlueNormal:
		case ESlice::ButtonPurpleNormal:
			return T66MiniUI::MasterBasicButtonPath();
		case ESlice::Count:
		default:
			return T66MiniUI::MasterBasicPanelPath();
		}
	}

	inline FMargin SliceMargin(const ESlice Slice)
	{
		switch (Slice)
		{
		case ESlice::CardNormal:
		case ESlice::CardSelected:
		case ESlice::CardDisabled:
		case ESlice::PortraitFrame:
		case ESlice::BadgeFrame:
			return FMargin(0.167f, 0.160f, 0.167f, 0.160f);
		case ESlice::ButtonGreenNormal:
		case ESlice::ButtonBlueNormal:
		case ESlice::ButtonPurpleNormal:
			return T66MiniUI::MasterButtonMargin();
		case ESlice::TitlePlaque:
		case ESlice::RowLong:
		case ESlice::IdolOfferRow:
		case ESlice::IdolOfferRowAction:
		case ESlice::SummaryRow:
		case ESlice::StatChip:
		case ESlice::PanelSmall:
		case ESlice::PanelLarge:
		case ESlice::PanelMedium:
		default:
			return T66MiniUI::MasterPanelMargin();
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

		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			*Entries[Index],
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(SliceRelativePath(Slice)),
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
		FT66ButtonParams ResolvedParams = Params;
		ResolvedParams.SetUseDotaPlateOverlay(false);
		return FT66Style::MakeButton(ResolvedParams);
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
