// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66LoadingScreenWidget.h"
#include "UI/Style/T66Style.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

void UT66LoadingScreenWidget::SetLoadingText(const FText& InLoadingText)
{
	LoadingTextOverride = InLoadingText;
	if (LoadingTextBlock.IsValid())
	{
		LoadingTextBlock->SetText(LoadingTextOverride);
	}
}

TSharedRef<SWidget> UT66LoadingScreenWidget::RebuildWidget()
{
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	const FText LoadingText = LoadingTextOverride.IsEmpty()
		? (Loc ? Loc->GetText_Loading() : NSLOCTEXT("T66.Loading", "Loading", "LOADING..."))
		: LoadingTextOverride;

	const FLinearColor BgColor = FT66Style::Tokens::Bg;
	const FLinearColor TextColor = FT66Style::Tokens::Text;

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(BgColor)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(LoadingTextBlock, STextBlock)
				.Text(LoadingText)
				.Font(FT66Style::Tokens::FontTitle())
				.ColorAndOpacity(FSlateColor(TextColor))
		];
}
