// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66TooltipSlate.h"

#include "Types/SlateEnums.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66FlatWidgetMetadata.h"
#include "UI/Style/T66FriendslopStyle.h"
#include "UI/Style/T66RuntimeUIFontAccess.h"
#include "Widgets/SToolTip.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	int32 AdjustTooltipFontSize(const int32 BaseSize, const int32 Adjustment)
	{
		return FMath::Max(8, BaseSize + Adjustment);
	}

	void ApplyTooltipMetadata(const TSharedRef<SWidget>& Widget, const FT66TooltipPayload& Payload, const bool bHasTooltip, const bool bTooltipRequired)
	{
		if (const TSharedPtr<FT66FlatWidgetMetadata> Metadata = Widget->GetMetaData<FT66FlatWidgetMetadata>())
		{
			Metadata->TooltipId = Payload.TooltipId;
			Metadata->TooltipKind = T66TooltipKindToString(Payload.Kind);
			Metadata->bHasTooltip = bHasTooltip;
			Metadata->bTooltipRequired = bTooltipRequired;
		}
	}

	void AddTextBlock(
		const TSharedRef<SVerticalBox>& Box,
		const FText& Text,
		const FSlateFontInfo& Font,
		const FSlateColor& Color,
		const float WrapWidth,
		const FMargin& Padding = FMargin())
	{
		if (Text.IsEmpty())
		{
			return;
		}

		Box->AddSlot().AutoHeight().Padding(Padding)
		[
			SNew(STextBlock)
			.Text(Text)
			.Font(Font)
			.ColorAndOpacity(Color)
			.AutoWrapText(true)
			.WrapTextAt(WrapWidth)
			.Justification(ETextJustify::Left)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		];
	}

	void AddRow(const TSharedRef<SVerticalBox>& Box, const FT66TooltipRow& Row, const FT66TooltipPayload& Payload)
	{
		if (Row.IsEmpty())
		{
			return;
		}

		const float WrapWidth = FMath::Max(180.f, Payload.Width - 32.f);
		TSharedRef<SVerticalBox> RowBox = SNew(SVerticalBox);

		if (!Row.Label.IsEmpty() || !Row.Value.IsEmpty())
		{
			RowBox->AddSlot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Row.Label)
					.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(AdjustTooltipFontSize(13, Payload.FontSizeAdjustment), true))
					.ColorAndOpacity(FT66FlatStyle::PrimaryText())
					.AutoWrapText(true)
					.WrapTextAt(WrapWidth * 0.55f)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Row.Value)
					.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(AdjustTooltipFontSize(13, Payload.FontSizeAdjustment), false))
					.ColorAndOpacity(FT66FlatStyle::SelectedText())
					.AutoWrapText(true)
					.WrapTextAt(WrapWidth * 0.45f)
				]
			];
		}

		AddTextBlock(
			RowBox,
			Row.Description,
			T66RuntimeUIFontAccess::MakeFriendslopFont(AdjustTooltipFontSize(12, Payload.FontSizeAdjustment), false),
			FT66FlatStyle::SecondaryText(),
			WrapWidth,
			FMargin(0.f, 3.f, 0.f, 0.f));

		Box->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			RowBox
		];
	}
}

TSharedRef<SWidget> T66TooltipSlate::MakeTooltipContent(const FT66TooltipPayload& Payload)
{
	const float Width = 560.f;
	const float WrapWidth = 460.f;
	const float MinHeight = 310.f;
	FT66TooltipPayload LayoutPayload = Payload;
	LayoutPayload.Width = WrapWidth;
	TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);

	AddTextBlock(
		Content,
		Payload.Title,
		T66RuntimeUIFontAccess::MakeFriendslopFont(AdjustTooltipFontSize(16, Payload.FontSizeAdjustment), true),
		FT66FlatStyle::PrimaryText(),
		WrapWidth);

	AddTextBlock(
		Content,
		Payload.Subtitle,
		T66RuntimeUIFontAccess::MakeFriendslopFont(AdjustTooltipFontSize(12, Payload.FontSizeAdjustment), false),
		FT66FlatStyle::SelectedText(),
		WrapWidth,
		FMargin(0.f, 3.f, 0.f, 0.f));

	AddTextBlock(
		Content,
		Payload.Body,
		T66RuntimeUIFontAccess::MakeFriendslopFont(AdjustTooltipFontSize(13, Payload.FontSizeAdjustment), false),
		FT66FlatStyle::SecondaryText(),
		WrapWidth,
		FMargin(0.f, 6.f, 0.f, 0.f));

	for (const FT66TooltipRow& Row : Payload.Rows)
	{
		AddRow(Content, Row, LayoutPayload);
	}

	for (const FText& Warning : Payload.Warnings)
	{
		AddTextBlock(
			Content,
			Warning,
			T66RuntimeUIFontAccess::MakeFriendslopFont(AdjustTooltipFontSize(12, Payload.FontSizeAdjustment), true),
			FSlateColor(FLinearColor(0.95f, 0.62f, 0.20f, 1.f)),
			WrapWidth,
			FMargin(0.f, 8.f, 0.f, 0.f));
	}

	const FString TooltipAssetPath(TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_tooltip_panel_textless.png"));
	return SNew(SBox)
		.WidthOverride(Width)
		.MinDesiredHeight(MinHeight)
		[
			FT66FriendslopStyle::MakeCustomPanel(
				TooltipAssetPath,
				FMargin(0.12f, 0.20f, 0.12f, 0.26f),
				FVector2D(560.f, 260.f),
				ET66FlatState::Default,
				FMargin(50.f, 40.f, 50.f, 80.f),
				Content,
				nullptr,
				Payload.TooltipId.IsNone() ? NAME_None : FName(*(Payload.TooltipId.ToString() + TEXT(".Content"))),
				FLinearColor(0.035f, 0.038f, 0.047f, 1.f))
		];
}

TSharedPtr<IToolTip> T66TooltipSlate::MakeTooltip(const FT66TooltipPayload& Payload)
{
	if (Payload.IsEmpty())
	{
		return nullptr;
	}

	return SNew(SToolTip)
	[
		MakeTooltipContent(Payload)
	];
}

TSharedPtr<IToolTip> T66TooltipSlate::MakeRichTooltip(const FText& Title, const FText& Description, const int32 FontSizeAdjustment)
{
	FT66TooltipPayload Payload;
	Payload.TooltipId = FName(TEXT("Tooltip.Rich"));
	Payload.Kind = ET66TooltipKind::General;
	Payload.Title = Title;
	Payload.Body = Description;
	Payload.FontSizeAdjustment = FontSizeAdjustment;
	return MakeTooltip(Payload);
}

TSharedPtr<IToolTip> T66TooltipSlate::MakeTextTooltip(const FText& Text, const int32 FontSizeAdjustment)
{
	FT66TooltipPayload Payload;
	Payload.TooltipId = FName(TEXT("Tooltip.Text"));
	Payload.Kind = ET66TooltipKind::General;
	Payload.Body = Text;
	Payload.FontSizeAdjustment = FontSizeAdjustment;
	Payload.Width = 320.f;
	return MakeTooltip(Payload);
}

void T66TooltipSlate::SetTooltip(const TSharedPtr<SWidget>& Widget, const FT66TooltipPayload& Payload, const bool bTooltipRequired)
{
	if (!Widget.IsValid())
	{
		return;
	}

	SetTooltip(Widget.ToSharedRef(), Payload, bTooltipRequired);
}

void T66TooltipSlate::SetTooltip(const TSharedRef<SWidget>& Widget, const FT66TooltipPayload& Payload, const bool bTooltipRequired)
{
	const TSharedPtr<IToolTip> Tooltip = MakeTooltip(Payload);
	Widget->SetToolTip(Tooltip);
	ApplyTooltipMetadata(Widget, Payload, Tooltip.IsValid(), bTooltipRequired);
}

TSharedRef<SWidget> T66TooltipSlate::WithTooltip(const TSharedRef<SWidget>& Widget, const FT66TooltipPayload& Payload, const bool bTooltipRequired)
{
	SetTooltip(Widget, Payload, bTooltipRequired);
	return Widget;
}
