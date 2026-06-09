// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66QuitConfirmationModal.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/Shutdown/T66ShutdownSubsystem.h"
#include "Engine/GameInstance.h"
#include "UI/Style/T66FlatStyle.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66QuitConfirmation, Log, All);

namespace
{
	FName QuitConfirmationTag(const TCHAR* Name)
	{
		return FName(Name);
	}
}

UT66QuitConfirmationModal::UT66QuitConfirmationModal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::QuitConfirmation;
	bIsModal = true;
}

TSharedRef<SWidget> UT66QuitConfirmationModal::BuildSlateUI()
{
	// Get localization subsystem
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	FText TitleText = Loc ? Loc->GetText_QuitConfirmTitle() : NSLOCTEXT("T66.QuitConfirm", "Title", "QUIT GAME?");
	FText MessageText = Loc ? Loc->GetText_QuitConfirmMessage() : NSLOCTEXT("T66.QuitConfirm", "Message", "Are you sure you want to quit?");
	FText StayText = Loc ? Loc->GetText_NoStay() : NSLOCTEXT("T66.QuitConfirm", "Stay", "NO, I WANT TO STAY");
	FText QuitText = Loc ? Loc->GetText_YesQuit() : NSLOCTEXT("T66.QuitConfirm", "Quit", "YES, I WANT TO QUIT");

	T66ScreenSlateHelpers::FFriendslopStandardModalParams Params;
	Params.TitleText = TitleText;
	Params.BodyText = MessageText;
	Params.RootTag = QuitConfirmationTag(TEXT("QuitConfirmation.Root"));
	Params.ScrimTag = QuitConfirmationTag(TEXT("QuitConfirmation.Scrim"));
	Params.PanelTag = QuitConfirmationTag(TEXT("QuitConfirmation.ModalPanel"));
	Params.TitleTag = QuitConfirmationTag(TEXT("QuitConfirmation.Title"));
	Params.BodyTag = QuitConfirmationTag(TEXT("QuitConfirmation.Message"));
	Params.StatusTag = QuitConfirmationTag(TEXT("QuitConfirmation.Status"));
	Params.LeftButton.Label = StayText;
	Params.LeftButton.OnClicked = FOnClicked::CreateUObject(this, &UT66QuitConfirmationModal::HandleStayClicked);
	Params.LeftButton.State = T66ScreenSlateHelpers::EFriendslopStandardModalButtonState::Ready;
	Params.LeftButton.Chrome = T66ScreenSlateHelpers::EFriendslopStandardModalButtonChrome::Green;
	Params.LeftButton.Tag = QuitConfirmationTag(TEXT("QuitConfirmation.StayButton"));
	Params.RightButton.Label = QuitText;
	Params.RightButton.OnClicked = FOnClicked::CreateUObject(this, &UT66QuitConfirmationModal::HandleQuitClicked);
	Params.RightButton.State = T66ScreenSlateHelpers::EFriendslopStandardModalButtonState::Selected;
	Params.RightButton.Chrome = T66ScreenSlateHelpers::EFriendslopStandardModalButtonChrome::Red;
	Params.RightButton.Tag = QuitConfirmationTag(TEXT("QuitConfirmation.QuitButton"));

	if (FParse::Param(FCommandLine::Get(), TEXT("T66FriendslopPreviewDoNotAskAgain")))
	{
		if (!bDoNotAskAgainPreviewInitialized)
		{
			bDoNotAskAgainPreviewChecked = FParse::Param(FCommandLine::Get(), TEXT("T66FriendslopPreviewDoNotAskAgainChecked"));
			bDoNotAskAgainPreviewInitialized = true;
		}

		Params.bShowCheckboxRow = true;
		Params.CheckboxRow.Label = FParse::Param(FCommandLine::Get(), TEXT("T66FriendslopPreviewDoNotAskAgainLongLabel"))
			? NSLOCTEXT("T66.SharedModal", "DoNotAskAgainLongPreview", "Do Not Ask Again For This Prompt Type")
			: NSLOCTEXT("T66.SharedModal", "DoNotAskAgain", "Do Not Ask Again");
		Params.CheckboxRow.OnClicked = FOnClicked::CreateUObject(this, &UT66QuitConfirmationModal::HandleDoNotAskAgainClicked);
		Params.CheckboxRow.IsChecked = TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateUObject(this, &UT66QuitConfirmationModal::IsDoNotAskAgainPreviewChecked));
		Params.CheckboxRow.RowTag = QuitConfirmationTag(TEXT("QuitConfirmation.DoNotAskAgain"));
		Params.CheckboxRow.CheckboxTag = QuitConfirmationTag(TEXT("QuitConfirmation.DoNotAskAgain.Checkbox"));
		Params.CheckboxRow.LabelTag = QuitConfirmationTag(TEXT("QuitConfirmation.DoNotAskAgain.Label"));
	}

	return T66ScreenSlateHelpers::MakeFriendslopStandardModal(Params);
}

FReply UT66QuitConfirmationModal::HandleStayClicked() { OnStayClicked(); return FReply::Handled(); }
FReply UT66QuitConfirmationModal::HandleQuitClicked() { OnQuitClicked(); return FReply::Handled(); }
FReply UT66QuitConfirmationModal::HandleDoNotAskAgainClicked()
{
	bDoNotAskAgainPreviewChecked = !bDoNotAskAgainPreviewChecked;
	return FReply::Handled();
}

bool UT66QuitConfirmationModal::IsDoNotAskAgainPreviewChecked() const
{
	return bDoNotAskAgainPreviewChecked;
}

void UT66QuitConfirmationModal::OnStayClicked() { CloseModal(); }
void UT66QuitConfirmationModal::OnQuitClicked()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66ShutdownSubsystem* Shutdown = GI->GetSubsystem<UT66ShutdownSubsystem>())
		{
			Shutdown->RequestQuitGame(ET66ShutdownReason::UserQuit, 0);
			return;
		}
	}

	UE_LOG(LogT66QuitConfirmation, Warning, TEXT("[Shutdown] Quit confirmation fallback to UKismetSystemLibrary::QuitGame because UT66ShutdownSubsystem is unavailable."));
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}

