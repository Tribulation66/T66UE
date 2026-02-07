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
		.Padding(15.0f, 15.0f, 0.0f, 0.0f)
		[
			SNew(SHorizontalBox)
			// Dark button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SBox)
				.HeightOverride(40.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked(FT66Style::DebounceClick(FOnClicked::CreateUObject(this, &UT66ThemeToggleWidget::HandleDarkThemeClicked)))
					.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>(
						FT66Style::GetTheme() == ET66UITheme::Dark ? "T66.Button.ToggleActive" : "T66.Button.Neutral"))
					.ContentPadding(FMargin(10.0f, 6.0f))
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_ThemeDark() : NSLOCTEXT("T66.Theme", "Dark", "DARK"))
						.Font(FT66Style::Tokens::FontBold(14))
						.ColorAndOpacity(FT66Style::GetTheme() == ET66UITheme::Dark ? FT66Style::Tokens::Panel : FT66Style::Tokens::Text)
					]
				]
			]
			// Light button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.HeightOverride(40.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked(FT66Style::DebounceClick(FOnClicked::CreateUObject(this, &UT66ThemeToggleWidget::HandleLightThemeClicked)))
					.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>(
						FT66Style::GetTheme() == ET66UITheme::Light ? "T66.Button.ToggleActive" : "T66.Button.Neutral"))
					.ContentPadding(FMargin(10.0f, 6.0f))
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_ThemeLight() : NSLOCTEXT("T66.Theme", "Light", "LIGHT"))
						.Font(FT66Style::Tokens::FontBold(14))
						.ColorAndOpacity(FT66Style::GetTheme() == ET66UITheme::Light ? FT66Style::Tokens::Panel : FT66Style::Tokens::Text)
					]
				]
			]
		];
}

// ---------------------------------------------------------------------------
void UT66ThemeToggleWidget::ForceRebuildSlate()
{
	const bool bWasInViewport = IsInViewport();
	constexpr int32 ZOrder = 50; // between screens (0) and modals (100)

	if (bWasInViewport)
	{
		RemoveFromParent();
	}

	ReleaseSlateResources(true);

	if (bWasInViewport)
	{
		AddToViewport(ZOrder);
	}
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
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UT66ThemeToggleWidget::ApplyPendingTheme));
	}
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
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UT66ThemeToggleWidget::ApplyPendingTheme));
	}
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

	// 2. Rebuild our own buttons so the active/inactive look updates
	ForceRebuildSlate();

	// 3. Force-rebuild the current screen so ALL widgets are recreated with new styles.
	//    RefreshScreen() alone is not enough because some screens (e.g. CompanionSelection,
	//    HeroSelection) override it to only update data, leaving old widgets alive with
	//    dangling pointers to the previous FSlateStyleSet.
	//    ForceRebuildSlate + OnScreenActivated ensures a full tear-down + re-init.
	if (UIManager)
	{
		if (UT66ScreenBase* Screen = UIManager->GetCurrentScreen())
		{
			Screen->ForceRebuildSlate();
			Screen->OnScreenActivated();
		}
	}

	bThemeChangeInProgress = false;
}
