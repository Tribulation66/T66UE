// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CowardicePromptWidget.h"
#include "Gameplay/T66CowardiceGate.h"
#include "Gameplay/T66PlayerController.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "UI/Style/T66FlatStyle.h"

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

	const TSharedRef<SWidget> TitleLabel = FT66FlatStyle::AttachMetadata(
		SNew(STextBlock)
		.Text(NSLOCTEXT("T66.Cowardice", "Title", "Take Cowardice Gate?"))
		.Font(FT66FlatStyle::MakeBoldFont(22))
		.ColorAndOpacity(FT66FlatStyle::PrimaryText())
		.Justification(ETextJustify::Center),
		FName(TEXT("CowardicePrompt.Title")),
		TEXT("Label.Header"),
		ET66FlatState::Default,
		TOptional<FLinearColor>(),
		false,
		NAME_None,
		true);

	const TSharedRef<SWidget> StatusLabel = FT66FlatStyle::AttachMetadata(
		SAssignNew(StatusText, STextBlock)
		.Text(FText::GetEmpty())
		.Font(FT66FlatStyle::MakeFont(14))
		.ColorAndOpacity(FT66FlatStyle::SecondaryText())
		.Justification(ETextJustify::Center),
		FName(TEXT("CowardicePrompt.Status")),
		TEXT("Label.Body"),
		ET66FlatState::Default,
		TOptional<FLinearColor>(),
		false,
		NAME_None,
		true);

	const TSharedRef<SWidget> ActionsRow = FT66FlatStyle::AttachMetadata(
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
		[
			FT66FlatStyle::MakeFlatButton(
				ET66FlatState::Selected,
				Loc ? Loc->GetText_Yes() : NSLOCTEXT("T66.Common", "Yes", "YES"),
				FOnClicked::CreateUObject(this, &UT66CowardicePromptWidget::OnYes),
				nullptr,
				nullptr,
				FMargin(14.f, 8.f),
				140.f,
				34.f,
				true,
				18,
				FName(TEXT("CowardicePrompt.YesButton")))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
		[
			FT66FlatStyle::MakeFlatButton(
				ET66FlatState::Default,
				Loc ? Loc->GetText_No() : NSLOCTEXT("T66.Common", "No", "NO"),
				FOnClicked::CreateUObject(this, &UT66CowardicePromptWidget::OnNo),
				nullptr,
				nullptr,
				FMargin(14.f, 8.f),
				140.f,
				34.f,
				true,
				18,
				FName(TEXT("CowardicePrompt.NoButton")))
		],
		FName(TEXT("CowardicePrompt.Actions")),
		TEXT("ActionRow"),
		ET66FlatState::Default);

	const TSharedRef<SWidget> PanelContent =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
		[
			TitleLabel
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
		[
			SNew(SBox)
			.WidthOverride(320.f)
			.HeightOverride(24.f)
			[
				StatusLabel
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			ActionsRow
		];

	const TSharedRef<SWidget> Panel = FT66FlatStyle::MakeFlatPanel(
		ET66FlatState::Default,
		FMargin(22.f),
		PanelContent,
		nullptr,
		FName(TEXT("CowardicePrompt.Panel")));

	return FT66FlatStyle::AttachMetadata(
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			Panel
		],
		FName(TEXT("CowardicePrompt.Root")),
		TEXT("Overlay"),
		ET66FlatState::Default);
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

