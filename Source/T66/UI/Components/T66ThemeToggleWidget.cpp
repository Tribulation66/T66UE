// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Components/T66ThemeToggleWidget.h"
#include "UI/T66UIManager.h"
#include "UI/T66ScreenBase.h"
#include "UI/Style/T66Style.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"

// ---------------------------------------------------------------------------
TSharedRef<SWidget> UT66ThemeToggleWidget::RebuildWidget()
{
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	// Full-screen overlay so the buttons float in the top-left corner.
	// SelfHitTestInvisible lets clicks pass through to the screen beneath
	// everywhere except the actual buttons.
	return SNew(SOverlay)
		.Visibility(EVisibility::SelfHitTestInvisible)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(30.0f, 30.0f, 0.0f, 0.0f)
		[
			SNew(SHorizontalBox)
			// Dark button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
			FT66Style::MakeButton(
				FT66ButtonParams(Loc ? Loc->GetText_ThemeDark() : NSLOCTEXT("T66.Theme", "Dark", "DARK"),
					FOnClicked::CreateUObject(this, &UT66ThemeToggleWidget::HandleDarkThemeClicked),
					FT66Style::GetTheme() == ET66UITheme::Dark ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
				.SetFontSize(14)
				.SetPadding(FMargin(10.f, 6.f))
				.SetMinWidth(0.f))
			]
			// Light button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
			FT66Style::MakeButton(
				FT66ButtonParams(Loc ? Loc->GetText_ThemeLight() : NSLOCTEXT("T66.Theme", "Light", "LIGHT"),
					FOnClicked::CreateUObject(this, &UT66ThemeToggleWidget::HandleLightThemeClicked),
					FT66Style::GetTheme() == ET66UITheme::Light ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
				.SetFontSize(14)
				.SetPadding(FMargin(10.f, 6.f))
				.SetMinWidth(0.f))
			]
		];
}

// ---------------------------------------------------------------------------
void UT66ThemeToggleWidget::ForceRebuildSlate()
{
	FT66Style::DeferRebuild(this, 50); // Z-order 50: between screens (0) and modals (100)
}

// ---------------------------------------------------------------------------
FReply UT66ThemeToggleWidget::HandleDarkThemeClicked()
{
	if (bThemeChangeInProgress || FT66Style::GetTheme() == ET66UITheme::Dark)
	{
		return FReply::Handled();
	}
	bThemeChangeInProgress = true;
	PendingTheme = ET66UITheme::Dark;
	ApplyPendingTheme();
	return FReply::Handled();
}

FReply UT66ThemeToggleWidget::HandleLightThemeClicked()
{
	if (bThemeChangeInProgress || FT66Style::GetTheme() == ET66UITheme::Light)
	{
		return FReply::Handled();
	}
	bThemeChangeInProgress = true;
	PendingTheme = ET66UITheme::Light;
	ApplyPendingTheme();
	return FReply::Handled();
}

// ---------------------------------------------------------------------------
void UT66ThemeToggleWidget::ApplyPendingTheme()
{
	// 1. Persist & apply theme palette
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->SetLightTheme(PendingTheme == ET66UITheme::Light);
		}
	}

	// 2. Rebuild ALL visible UI (theme toggle, underlying screen, modal) so every
	//    widget gets fresh style references and nothing holds dangling pointers
	//    to evicted old style sets.
	if (UIManager)
	{
		UIManager->RebuildAllVisibleUI();
	}

	bThemeChangeInProgress = false;
}
