// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SavePreviewScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/T66UIManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	FName SavePreviewTag(const TCHAR* Tag)
	{
		return FName(Tag);
	}
}

UT66SavePreviewScreen::UT66SavePreviewScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::SavePreview;
	bIsModal = true;
}

UT66LocalizationSubsystem* UT66SavePreviewScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	return nullptr;
}

TSharedRef<SWidget> UT66SavePreviewScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const FText PreviewTitle = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.SavePreview", "PreviewFallback", "PREVIEW");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FText LoadText = NSLOCTEXT("T66.SavePreview", "Load", "LOAD");
	const FText SubtitleText = NSLOCTEXT("T66.SavePreview", "Subtitle", "The run is paused for inspection. Back returns to Save Slots, Load resumes normally.");

	T66ScreenSlateHelpers::FFriendslopStandardModalParams Params;
	Params.TitleText = PreviewTitle;
	Params.BodyText = SubtitleText;
	Params.RootTag = SavePreviewTag(TEXT("SavePreview.Root"));
	Params.ScrimTag = SavePreviewTag(TEXT("SavePreview.Scrim"));
	Params.PanelTag = SavePreviewTag(TEXT("SavePreview.ModalPanel"));
	Params.TitleTag = SavePreviewTag(TEXT("SavePreview.Title"));
	Params.BodyTag = SavePreviewTag(TEXT("SavePreview.Subtitle"));
	Params.StatusTag = SavePreviewTag(TEXT("SavePreview.Status"));
	Params.LeftButton.Label = BackText;
	Params.LeftButton.OnClicked = FOnClicked::CreateUObject(this, &UT66SavePreviewScreen::HandleBackClicked);
	Params.LeftButton.State = T66ScreenSlateHelpers::EFriendslopStandardModalButtonState::Default;
	Params.LeftButton.Chrome = T66ScreenSlateHelpers::EFriendslopStandardModalButtonChrome::Red;
	Params.LeftButton.Tag = SavePreviewTag(TEXT("SavePreview.BackButton"));
	Params.RightButton.Label = LoadText;
	Params.RightButton.OnClicked = FOnClicked::CreateUObject(this, &UT66SavePreviewScreen::HandleLoadClicked);
	Params.RightButton.State = T66ScreenSlateHelpers::EFriendslopStandardModalButtonState::Selected;
	Params.RightButton.Chrome = T66ScreenSlateHelpers::EFriendslopStandardModalButtonChrome::Green;
	Params.RightButton.Tag = SavePreviewTag(TEXT("SavePreview.LoadButton"));

	return T66ScreenSlateHelpers::MakeFriendslopStandardModal(Params);
}

FReply UT66SavePreviewScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66SavePreviewScreen::HandleLoadClicked()
{
	OnLoadClicked();
	return FReply::Handled();
}

void UT66SavePreviewScreen::OnBackClicked()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	AT66PlayerController* PC = GetOwningPlayer<AT66PlayerController>();
	if (PC)
	{
		PC->SetPause(false);
	}

	if (GI)
	{
		GI->bSaveSlotPreviewMode = false;
		GI->PendingFrontendScreen = ET66ScreenType::SaveSlots;
	}

	if (UIManager)
	{
		UIManager->HideAllUI();
	}

	UT66GameInstance::TransitionToFrontendLevel(this);
}

void UT66SavePreviewScreen::OnLoadClicked()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	AT66PlayerController* PC = GetOwningPlayer<AT66PlayerController>();
	if (GI)
	{
		GI->bSaveSlotPreviewMode = false;
		GI->bRestoreSaveSlotsState = false;
		GI->PendingSaveSlotsPage = 0;
	}

	if (UIManager)
	{
		UIManager->CloseModal();
	}

	if (PC)
	{
		PC->SetPause(false);
		PC->RestoreGameplayInputMode();
	}
}
