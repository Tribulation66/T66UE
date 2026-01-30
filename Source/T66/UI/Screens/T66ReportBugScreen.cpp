// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66ReportBugScreen.h"
#include "UI/T66UIManager.h"
#include "Gameplay/T66PlayerController.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/SOverlay.h"

UT66ReportBugScreen::UT66ReportBugScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::ReportBug;
	bIsModal = true;
}

TSharedRef<SWidget> UT66ReportBugScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	FText TitleText = Loc ? Loc->GetText_ReportBugTitle() : FText::FromString(TEXT("REPORT BUG"));
	FText SubmitText = Loc ? Loc->GetText_ReportBugSubmit() : FText::FromString(TEXT("SUBMIT"));
	FText CancelText = Loc ? Loc->GetText_Cancel() : FText::FromString(TEXT("CANCEL"));

	BugReportText.Empty();

	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f))
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.15f, 1.0f))
				.Padding(FMargin(40.0f, 30.0f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 16.0f)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 16.0f)
					[
						SNew(SBox)
						.WidthOverride(400.0f)
						.HeightOverride(120.0f)
						[
							SNew(SMultiLineEditableTextBox)
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
							.OnTextChanged_Lambda([this](const FText& T) { BugReportText = T.ToString(); })
							.HintText(FText::FromString(TEXT("Describe the bug...")))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.0f, 0.0f)
						[
							SNew(SBox)
							.WidthOverride(120.0f)
							.HeightOverride(44.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66ReportBugScreen::HandleSubmitClicked))
								.ButtonColorAndOpacity(FLinearColor(0.2f, 0.45f, 0.2f, 1.0f))
								[
									SNew(STextBlock)
									.Text(SubmitText)
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.0f, 0.0f)
						[
							SNew(SBox)
							.WidthOverride(120.0f)
							.HeightOverride(44.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66ReportBugScreen::HandleCancelClicked))
								.ButtonColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.3f, 1.0f))
								[
									SNew(STextBlock)
									.Text(CancelText)
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
					]
				]
			]
		];
}

FReply UT66ReportBugScreen::HandleSubmitClicked() { OnSubmitClicked(); return FReply::Handled(); }
FReply UT66ReportBugScreen::HandleCancelClicked() { OnCancelClicked(); return FReply::Handled(); }

void UT66ReportBugScreen::OnSubmitClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Report Bug: %s"), *BugReportText);
	CloseModal();

	// In gameplay, Report Bug is opened from Pause Menu. Our UIManager is single-modal, so opening this
	// replaces Pause Menu. When closing, re-open Pause Menu so the player can resume/unpause.
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->IsGameplayLevel() && PC->IsPaused())
		{
			ShowModal(ET66ScreenType::PauseMenu);
		}
	}
}

void UT66ReportBugScreen::OnCancelClicked()
{
	CloseModal();

	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->IsGameplayLevel() && PC->IsPaused())
		{
			ShowModal(ET66ScreenType::PauseMenu);
		}
	}
}
