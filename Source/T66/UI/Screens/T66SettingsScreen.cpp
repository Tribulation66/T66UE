// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SettingsScreen.h"
#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

using namespace T66SettingsScreenPrivate;
UT66SettingsScreen::UT66SettingsScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Settings;
	bIsModal = false;
	SetIsFocusable(true);
}

UT66LocalizationSubsystem* UT66SettingsScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

UT66PlayerSettingsSubsystem* UT66SettingsScreen::GetPlayerSettings() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66PlayerSettingsSubsystem>();
	}
	return nullptr;
}

FReply UT66SettingsScreen::HandleCloseClicked()
{
	OnCloseClicked();
	return FReply::Handled();
}

void UT66SettingsScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	CurrentTab = ET66SettingsTab::RetroFX;
	OnTabChanged(CurrentTab);
	if (ContentSwitcher.IsValid())
	{
		ContentSwitcher->SetActiveWidgetIndex(static_cast<int32>(CurrentTab));
	}

	SetKeyboardFocus();
}

void UT66SettingsScreen::SwitchToTab(ET66SettingsTab Tab)
{
	CurrentTab = Tab;
	OnTabChanged(Tab);
	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PS->SetLastSettingsTabIndex(static_cast<int32>(Tab));
	}
	// Switch content via the widget switcher (buttons update via lambdas)
	if (ContentSwitcher.IsValid())
	{
		ContentSwitcher->SetActiveWidgetIndex(static_cast<int32>(Tab));
	}
}
void UT66SettingsScreen::OnCloseClicked()
{
	// If a video-mode confirm is active, closing should not keep the new settings.
	if (bVideoModeConfirmActive)
	{
		EndVideoModeConfirmPrompt(false);
	}

	const bool bModalPresentation = UIManager && UIManager->GetCurrentModalType() == ScreenType;

	if (bModalPresentation)
	{
		CloseModal();
	}
	else if (UIManager)
	{
		UIManager->GoBack();
	}

	// In gameplay, Settings is opened from Pause Menu. Our UIManager is single-modal, so showing Settings
	// replaces Pause Menu. When Settings closes, re-open Pause Menu so the player can resume/unpause.
	if (bModalPresentation)
	{
		if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
		{
			if (PC->IsGameplayLevel() && PC->IsPaused())
			{
				ShowModal(ET66ScreenType::PauseMenu);
			}
		}
	}
}
