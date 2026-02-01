// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CowardicePromptWidget.h"
#include "Gameplay/T66CowardiceGate.h"
#include "Gameplay/T66PlayerController.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

void UT66CowardicePromptWidget::SetGate(AT66CowardiceGate* InGate)
{
	Gate = InGate;
}

TSharedRef<SWidget> UT66CowardicePromptWidget::RebuildWidget()
{
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.75f))
			.Padding(22.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Cowardice", "Title", "Take Cowardice Gate?"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
				[
					SAssignNew(StatusText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
					.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.f))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(140.f)
						.HeightOverride(44.f)
						[
							SNew(SButton)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66CowardicePromptWidget::OnYes))
							.ButtonColorAndOpacity(FLinearColor(0.55f, 0.15f, 0.15f, 1.f))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(Loc ? Loc->GetText_Yes() : NSLOCTEXT("T66.Common", "Yes", "YES"))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
								.ColorAndOpacity(FLinearColor::White)
							]
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(140.f)
						.HeightOverride(44.f)
						[
							SNew(SButton)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66CowardicePromptWidget::OnNo))
							.ButtonColorAndOpacity(FLinearColor(0.2f, 0.2f, 0.28f, 1.f))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(Loc ? Loc->GetText_No() : NSLOCTEXT("T66.Common", "No", "NO"))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
								.ColorAndOpacity(FLinearColor::White)
							]
						]
					]
				]
			]
		];
}

FReply UT66CowardicePromptWidget::OnYes()
{
	if (!Gate.IsValid())
	{
		ClosePrompt();
		return FReply::Handled();
	}
	const bool bOk = Gate->ConfirmCowardice();
	if (!bOk && StatusText.IsValid())
	{
		StatusText->SetText(NSLOCTEXT("T66.Common", "Failed", "Failed."));
	}
	// Travel will happen; close immediately to restore input.
	ClosePrompt();
	return FReply::Handled();
}

FReply UT66CowardicePromptWidget::OnNo()
{
	ClosePrompt();
	return FReply::Handled();
}

void UT66CowardicePromptWidget::ClosePrompt()
{
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
}

