// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SettingsScreen.h"
#include "UI/T66UIManager.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"

UT66SettingsScreen::UT66SettingsScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Settings;
	bIsModal = true;
}

TSharedRef<SWidget> UT66SettingsScreen::BuildSlateUI()
{
	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f))
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.WidthOverride(700.0f)
			.HeightOverride(500.0f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.12f, 1.0f))
				.Padding(FMargin(20.0f))
				[
					SNew(SVerticalBox)
					// Title row
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("SETTINGS")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
							.ColorAndOpacity(FLinearColor::White)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.WidthOverride(50.0f)
							.HeightOverride(50.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleCloseClicked))
								.ButtonColorAndOpacity(FLinearColor(0.5f, 0.2f, 0.2f, 1.0f))
								[
									SNew(STextBlock)
									.Text(FText::FromString(TEXT("X")))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
					]
					// Content placeholder
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Settings content placeholder.\n\nTabs: Gameplay | Graphics | Controls | Audio | Crashing")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
						.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
					]
				]
			]
		];
}

FReply UT66SettingsScreen::HandleCloseClicked() { OnCloseClicked(); return FReply::Handled(); }

void UT66SettingsScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	CurrentTab = ET66SettingsTab::Gameplay;
	OnTabChanged(CurrentTab);
}

void UT66SettingsScreen::SwitchToTab(ET66SettingsTab Tab) { CurrentTab = Tab; OnTabChanged(Tab); }
void UT66SettingsScreen::OnApplyGraphicsClicked() { UE_LOG(LogTemp, Log, TEXT("Apply Graphics - not implemented")); }
void UT66SettingsScreen::OnCloseClicked() { CloseModal(); }
