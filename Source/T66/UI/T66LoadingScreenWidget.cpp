// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66LoadingScreenWidget.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "UI/Style/T66FlatStyle.h"

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

	const TSharedRef<SWidget> LoadingLabel = FT66FlatStyle::AttachMetadata(
		SNew(SBox)
		.WidthOverride(258.333f)
		.HeightOverride(80.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(LoadingTextBlock, STextBlock)
			.Text(LoadingText)
			.Font(FT66FlatStyle::MakeBoldFont(56))
			.ColorAndOpacity(FT66FlatStyle::PrimaryText())
		],
		FName(TEXT("LoadingScreen.Text")),
		TEXT("Label.Title"),
		ET66FlatState::Default,
		TOptional<FLinearColor>(),
		false,
		NAME_None,
		true);

	return FT66FlatStyle::AttachMetadata(
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66FlatStyle::BackgroundColor())
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			LoadingLabel
		],
		FName(TEXT("LoadingScreen.Background")),
		TEXT("FullscreenOverlay"),
		ET66FlatState::Default,
		FT66FlatStyle::BackgroundColor());
}
