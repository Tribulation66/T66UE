// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66HeroCooldownBarWidget.h"
#include "UI/Style/T66Style.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

void UT66HeroCooldownBarWidget::SetProgress(float InPct)
{
	Pct = FMath::Clamp(InPct, 0.f, 1.f);
	if (FillWidthBox)
	{
		FillWidthBox->SetWidthOverride(BarWidth * Pct);
	}
}

void UT66HeroCooldownBarWidget::SetRangeUnits(int32 Units)
{
	RangeUnits = FMath::Max(0, Units);
	if (RangeTextBlock)
	{
		RangeTextBlock->SetText(FText::AsNumber(RangeUnits));
	}
}

TSharedRef<SWidget> UT66HeroCooldownBarWidget::RebuildWidget()
{
	FillWidthBox.Reset();
	RangeTextBlock.Reset();

	const TSharedRef<SWidget> Built =
		SNew(SVerticalBox)

		// Cooldown bar: background with fill that grows 0 -> 1 (left to right)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f)
		[
			SNew(SBox)
			.WidthOverride(BarWidth)
			.HeightOverride(BarHeight)
			[
				SNew(SBorder)
				.Padding(FMargin(1.f))
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.75f))
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SAssignNew(FillWidthBox, SBox)
						.WidthOverride(BarWidth * Pct)
						.HeightOverride(BarHeight)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.2f, 0.75f, 1.f, 1.f))
						]
					]
				]
			]
		]

		// Range units text below the bar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 2.f)
		.HAlign(HAlign_Center)
		[
			SAssignNew(RangeTextBlock, STextBlock)
			.Text(FText::AsNumber(RangeUnits))
			.Font(FT66Style::Tokens::FontBold(12))
			.ColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.9f, 0.6f, 1.f)))
		];

	return Built;
}
