// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66ShelvedFeatureScreen.h"

#include "UI/Style/T66FlatStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

UT66ShelvedFeatureScreen::UT66ShelvedFeatureScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::None;
	bIsModal = false;
}

TSharedRef<SWidget> UT66ShelvedFeatureScreen::BuildSlateUI()
{
	return SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(FMargin(48.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.ShelvedFeature", "Title", "FEATURE SHELVED"))
				.Font(FT66FlatStyle::MakeBoldFont(48))
				.ColorAndOpacity(FT66FlatStyle::PrimaryText())
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 16.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.ShelvedFeature", "Body", "This mode is disabled but preserved for later reintroduction."))
				.Font(FT66FlatStyle::MakeFont(24))
				.ColorAndOpacity(FT66FlatStyle::SecondaryText())
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 32.f, 0.f, 0.f)
			[
				FT66FlatStyle::MakeFlatButton(
					ET66FlatState::Default,
					NSLOCTEXT("T66.ShelvedFeature", "Back", "BACK"),
					FOnClicked::CreateUObject(this, &UT66ShelvedFeatureScreen::HandleBackClicked),
					nullptr,
					nullptr,
					FMargin(22.f, 8.f),
					180.f,
					56.f,
					true,
					22,
					FName(TEXT("ShelvedFeature.BackButton")))
			]
		];
}

FReply UT66ShelvedFeatureScreen::HandleBackClicked()
{
	NavigateTo(ET66ScreenType::MainMenu);
	return FReply::Handled();
}
