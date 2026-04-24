// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Components/T66MiniFriendsPanel.h"

#include "Styling/CoreStyle.h"
#include "UI/T66MiniUIStyle.h"
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
			.Font(T66MiniUI::TitleFont(24))
			.ColorAndOpacity(T66MiniUI::Text())
		];

	for (const FText& Row : Rows)
	{
		Content->AddSlot()
		.AutoHeight()
		.Padding(0.f, 4.f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(T66MiniUI::CardFill())
			.Padding(FMargin(12.f, 10.f))
			[
				SNew(STextBlock)
				.Text(Row)
				.Font(T66MiniUI::BodyFont(16))
				.ColorAndOpacity(T66MiniUI::Text())
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
		.BorderBackgroundColor(T66MiniUI::PanelFill())
		.Padding(FMargin(12.f, 10.f))
		[
			SNew(STextBlock)
			.Text(InArgs._Footer)
			.Font(T66MiniUI::BodyFont(15))
			.ColorAndOpacity(T66MiniUI::MutedText())
			.AutoWrapText(true)
		]
	];

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(T66MiniUI::ShellFill())
		.Padding(FMargin(18.f))
		[
			Content
		]
	];
}
