// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MainMenuScreen.h"
#include "UI/T66UIManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"

UT66MainMenuScreen::UT66MainMenuScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::MainMenu;
	bIsModal = false;
}

TSharedRef<SWidget> UT66MainMenuScreen::BuildSlateUI()
{
	// Button style - dark with padding
	auto MakeMenuButton = [this](const FText& Text, FReply (UT66MainMenuScreen::*ClickFunc)(), const FLinearColor& BgColor = FLinearColor(0.1f, 0.1f, 0.15f, 1.0f)) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(300.0f)
			.HeightOverride(60.0f)
			.Padding(FMargin(0.0f, 5.0f))
			[
				SNew(SBorder)
				.BorderBackgroundColor(BgColor)
				.Padding(FMargin(0.0f))
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, ClickFunc))
					.ButtonColorAndOpacity(BgColor)
					[
						SNew(STextBlock)
						.Text(Text)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
						.ColorAndOpacity(FLinearColor::White)
						.Justification(ETextJustify::Center)
					]
				]
			];
	};

	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 1.0f))
		[
			SNew(SOverlay)
			// Main content
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				// Title
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 80.0f, 0.0f, 60.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("TRIBULATION 66")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 72))
					.ColorAndOpacity(FLinearColor::White)
				]
				// Button container
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeMenuButton(FText::FromString(TEXT("NEW GAME")), &UT66MainMenuScreen::HandleNewGameClicked, FLinearColor(0.2f, 0.4f, 0.8f, 1.0f))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeMenuButton(FText::FromString(TEXT("LOAD GAME")), &UT66MainMenuScreen::HandleLoadGameClicked, FLinearColor(0.15f, 0.35f, 0.7f, 1.0f))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeMenuButton(FText::FromString(TEXT("SETTINGS")), &UT66MainMenuScreen::HandleSettingsClicked)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeMenuButton(FText::FromString(TEXT("ACHIEVEMENTS")), &UT66MainMenuScreen::HandleAchievementsClicked)
					]
				]
			]
			// Quit button (top-right)
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.Padding(0.0f, 20.0f, 20.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(120.0f)
				.HeightOverride(50.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66MainMenuScreen::HandleQuitClicked))
					.ButtonColorAndOpacity(FLinearColor(0.6f, 0.1f, 0.1f, 1.0f))
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("QUIT")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
						.ColorAndOpacity(FLinearColor::White)
					]
				]
			]
			// Language button (bottom-left)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(20.0f, 0.0f, 0.0f, 20.0f)
			[
				SNew(SBox)
				.WidthOverride(150.0f)
				.HeightOverride(50.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66MainMenuScreen::HandleLanguageClicked))
					.ButtonColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.15f, 1.0f))
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("LANGUAGE")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(FLinearColor::White)
					]
				]
			]
		];
}

void UT66MainMenuScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	UE_LOG(LogTemp, Log, TEXT("MainMenuScreen activated!"));
}

// Slate click handlers (return FReply)
FReply UT66MainMenuScreen::HandleNewGameClicked()
{
	OnNewGameClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleLoadGameClicked()
{
	OnLoadGameClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleSettingsClicked()
{
	OnSettingsClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleAchievementsClicked()
{
	OnAchievementsClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleLanguageClicked()
{
	OnLanguageClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleQuitClicked()
{
	OnQuitClicked();
	return FReply::Handled();
}

// UFUNCTION handlers (call navigation)
void UT66MainMenuScreen::OnNewGameClicked()
{
	NavigateTo(ET66ScreenType::PartySizePicker);
}

void UT66MainMenuScreen::OnLoadGameClicked()
{
	NavigateTo(ET66ScreenType::PartySizePicker);
}

void UT66MainMenuScreen::OnSettingsClicked()
{
	ShowModal(ET66ScreenType::Settings);
}

void UT66MainMenuScreen::OnAchievementsClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Achievements clicked - not yet implemented"));
}

void UT66MainMenuScreen::OnLanguageClicked()
{
	ShowModal(ET66ScreenType::LanguageSelect);
}

void UT66MainMenuScreen::OnQuitClicked()
{
	ShowModal(ET66ScreenType::QuitConfirmation);
}

void UT66MainMenuScreen::OnAccountStatusClicked()
{
	ShowModal(ET66ScreenType::AccountStatus);
}

bool UT66MainMenuScreen::ShouldShowAccountStatus() const
{
	return false;
}
