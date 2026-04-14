// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Components/T66MiniFriendsPanel.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void ST66MiniFriendsPanel::Construct(const FArguments& InArgs)
{
	const TArray<FText> Rows = InArgs._Rows;

	TSharedRef<SVerticalBox> Content = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 12.f)
		[
			SNew(STextBlock)
			.Text(InArgs._Title)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
			.ColorAndOpacity(FLinearColor::White)
		];

	for (const FText& Row : Rows)
	{
		Content->AddSlot()
		.AutoHeight()
		.Padding(0.f, 4.f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.06f, 0.07f, 0.10f, 1.0f))
			.Padding(FMargin(12.f, 10.f))
			[
				SNew(STextBlock)
				.Text(Row)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
				.ColorAndOpacity(FLinearColor::White)
				.AutoWrapText(true)
			]
		];
	}

	Content->AddSlot()
	.FillHeight(1.f)
	.Padding(0.f, 8.f, 0.f, 0.f)
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.04f, 0.05f, 0.08f, 1.0f))
		.Padding(FMargin(12.f, 10.f))
		[
			SNew(STextBlock)
			.Text(InArgs._Footer)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 15))
			.ColorAndOpacity(FLinearColor(0.74f, 0.78f, 0.86f, 1.0f))
			.AutoWrapText(true)
		]
	];

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.03f, 0.04f, 0.06f, 0.98f))
		.Padding(FMargin(18.f))
		[
			Content
		]
	];
}
