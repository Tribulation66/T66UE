// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SettingsScreen.h"
#include "UI/Screens/Settings/T66SettingsScreen_Private.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

using namespace T66SettingsScreenPrivate;

namespace
{
	bool TryResolveSettingsTabName(const FString& RawName, ET66SettingsTab& OutTab)
	{
		FString Name = RawName.TrimStartAndEnd();
		if (Name.StartsWith(TEXT("Settings"), ESearchCase::IgnoreCase) && !Name.Equals(TEXT("Settings"), ESearchCase::IgnoreCase) && !Name.Equals(TEXT("SettingsScreen"), ESearchCase::IgnoreCase))
		{
			Name = Name.RightChop(8);
		}

		if (Name.Equals(TEXT("Gameplay"), ESearchCase::IgnoreCase))
		{
			OutTab = ET66SettingsTab::Gameplay;
			return true;
		}
		if (Name.Equals(TEXT("Graphics"), ESearchCase::IgnoreCase))
		{
			OutTab = ET66SettingsTab::Graphics;
			return true;
		}
		if (Name.Equals(TEXT("Controls"), ESearchCase::IgnoreCase))
		{
			OutTab = ET66SettingsTab::Controls;
			return true;
		}
		if (Name.Equals(TEXT("HUD"), ESearchCase::IgnoreCase))
		{
			OutTab = ET66SettingsTab::HUD;
			return true;
		}
		if (Name.Equals(TEXT("MediaViewer"), ESearchCase::IgnoreCase) || Name.Equals(TEXT("Media"), ESearchCase::IgnoreCase))
		{
			OutTab = ET66SettingsTab::MediaViewer;
			return true;
		}
		if (Name.Equals(TEXT("Audio"), ESearchCase::IgnoreCase))
		{
			OutTab = ET66SettingsTab::Audio;
			return true;
		}
		if (Name.Equals(TEXT("Crashing"), ESearchCase::IgnoreCase))
		{
			OutTab = ET66SettingsTab::Crashing;
			return true;
		}
		if (Name.Equals(TEXT("RetroFX"), ESearchCase::IgnoreCase))
		{
			OutTab = ET66SettingsTab::RetroFX;
			return true;
		}

		return false;
	}
}

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
	if (bRetroFXPreviewPopup)
	{
		return HandleCloseRetroFXPreviewPopupClicked();
	}

	OnCloseClicked();
	return FReply::Handled();
}

void UT66SettingsScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	CurrentTab = ET66SettingsTab::RetroFX;
	ApplyCommandLineTabOverride();
	OnTabChanged(CurrentTab);
	if (ContentSwitcher.IsValid())
	{
		ContentSwitcher->SetActiveWidgetIndex(static_cast<int32>(CurrentTab));
	}

	SetKeyboardFocus();
}

void UT66SettingsScreen::ApplyCommandLineTabOverride()
{
	FString RequestedSettingsTab;
	if (!FParse::Value(FCommandLine::Get(), TEXT("T66SettingsTab="), RequestedSettingsTab))
	{
		FParse::Value(FCommandLine::Get(), TEXT("T66FrontendScreen="), RequestedSettingsTab);
	}

	ET66SettingsTab RequestedTab = CurrentTab;
	if (!RequestedSettingsTab.IsEmpty() && TryResolveSettingsTabName(RequestedSettingsTab, RequestedTab))
	{
		CurrentTab = RequestedTab;
	}
}

void UT66SettingsScreen::OnScreenDeactivated_Implementation()
{
	CommitPendingRetroFXOnClose();
	Super::OnScreenDeactivated_Implementation();
}

void UT66SettingsScreen::NativeDestruct()
{
	CommitPendingRetroFXOnClose();
	Super::NativeDestruct();
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
	if (bRetroFXPreviewPopup)
	{
		HandleCloseRetroFXPreviewPopupClicked();
		return;
	}

	CommitPendingRetroFXOnClose();

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

bool UT66SettingsScreen::HandleBackAction()
{
	if (bRetroFXPreviewPopup)
	{
		HandleCloseRetroFXPreviewPopupClicked();
		return true;
	}

	if (bWaitingForRebind)
	{
		bWaitingForRebind = false;
		if (RebindStatusText.IsValid())
		{
			if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
			{
				RebindStatusText->SetText(Loc->GetText_RebindCancelled());
			}
		}
		return true;
	}

	OnCloseClicked();
	return true;
}
