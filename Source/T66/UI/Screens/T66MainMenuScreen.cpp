// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MainMenuScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Components/T66LeaderboardPanel.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
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

UT66LocalizationSubsystem* UT66MainMenuScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

TSharedRef<SWidget> UT66MainMenuScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	// Get localized text
	FText TitleText = Loc ? Loc->GetText_GameTitle() : FText::FromString(TEXT("TRIBULATION 66"));
	FText NewGameText = Loc ? Loc->GetText_NewGame() : FText::FromString(TEXT("NEW GAME"));
	FText LoadGameText = Loc ? Loc->GetText_LoadGame() : FText::FromString(TEXT("LOAD GAME"));
	FText SettingsText = Loc ? Loc->GetText_Settings() : FText::FromString(TEXT("SETTINGS"));
	FText AchievementsText = Loc ? Loc->GetText_Achievements() : FText::FromString(TEXT("ACHIEVEMENTS"));
	FText QuitText = Loc ? Loc->GetText_Quit() : FText::FromString(TEXT("QUIT"));

	// Button style lambda
	auto MakeMenuButton = [this](const FText& Text, FReply (UT66MainMenuScreen::*ClickFunc)(), const FLinearColor& BgColor = FLinearColor(0.1f, 0.1f, 0.15f, 1.0f)) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(280.0f)
			.HeightOverride(55.0f)
			.Padding(FMargin(0.0f, 4.0f))
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
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
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
			// Main two-panel layout
			+ SOverlay::Slot()
			[
				SNew(SHorizontalBox)
				// Left Panel - Navigation
				+ SHorizontalBox::Slot()
				.FillWidth(0.4f)
				.Padding(40.0f, 60.0f, 20.0f, 60.0f)
				[
					SNew(SVerticalBox)
					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					.Padding(0.0f, 0.0f, 0.0f, 50.0f)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 52))
						.ColorAndOpacity(FLinearColor::White)
					]
					// Button stack
					+ SVerticalBox::Slot()
					.AutoHeight()
					.VAlign(VAlign_Top)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							MakeMenuButton(NewGameText, &UT66MainMenuScreen::HandleNewGameClicked, FLinearColor(0.2f, 0.4f, 0.8f, 1.0f))
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							MakeMenuButton(LoadGameText, &UT66MainMenuScreen::HandleLoadGameClicked, FLinearColor(0.15f, 0.35f, 0.7f, 1.0f))
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							MakeMenuButton(SettingsText, &UT66MainMenuScreen::HandleSettingsClicked)
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							MakeMenuButton(AchievementsText, &UT66MainMenuScreen::HandleAchievementsClicked)
						]
					]
					// Spacer
					+ SVerticalBox::Slot().FillHeight(1.0f)
				]
				// Right Panel - Leaderboard
				+ SHorizontalBox::Slot()
				.FillWidth(0.6f)
				.Padding(20.0f, 60.0f, 40.0f, 60.0f)
				[
					SAssignNew(LeaderboardPanel, ST66LeaderboardPanel)
					.LocalizationSubsystem(Loc)
				]
			]
			// Quit button (top-right)
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.Padding(0.0f, 15.0f, 15.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(100.0f)
				.HeightOverride(40.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66MainMenuScreen::HandleQuitClicked))
					.ButtonColorAndOpacity(FLinearColor(0.5f, 0.15f, 0.15f, 1.0f))
					[
						SNew(STextBlock)
						.Text(QuitText)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(FLinearColor::White)
					]
				]
			]
			// Language button (bottom-left) - Globe icon placeholder
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(15.0f, 0.0f, 0.0f, 15.0f)
			[
				SNew(SBox)
				.WidthOverride(50.0f)
				.HeightOverride(50.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66MainMenuScreen::HandleLanguageClicked))
					.ButtonColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.15f, 1.0f))
					[
						// Globe icon placeholder - using a stylized "L" for Language
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("\xF0\x9F\x8C\x90"))) // Globe emoji as placeholder
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
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

	// Subscribe to language changes
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66MainMenuScreen::OnLanguageChanged);
	}
}

void UT66MainMenuScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	// Force rebuild UI with current language
	TakeWidget();
}

void UT66MainMenuScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	// Rebuild UI when language changes
	TakeWidget();
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
	NavigateTo(ET66ScreenType::Achievements);
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
