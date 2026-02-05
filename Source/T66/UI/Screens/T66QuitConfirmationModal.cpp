// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66QuitConfirmationModal.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/Style/T66Style.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"

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

	return SNew(SBorder)
		// Opaque background (no transparency).
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f))
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FT66Style::Get().GetBrush("T66.Brush.ObsidianPanel"))
				.Padding(FMargin(40.0f, 30.0f))
				[
					SNew(SVerticalBox)
					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 20.0f)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FT66Style::Tokens::FontBold(36))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					// Message
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 30.0f)
					[
						SNew(STextBlock)
						.Text(MessageText)
						.Font(FT66Style::Tokens::FontRegular(18))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
					]
					// Buttons
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
							.MinDesiredWidth(320.0f)
							.HeightOverride(56.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66QuitConfirmationModal::HandleStayClicked))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Success)
								.ContentPadding(FMargin(18.f, 10.f))
								[
									SNew(STextBlock)
									.Text(StayText)
									.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
									.Justification(ETextJustify::Center)
									.AutoWrapText(false)
								]
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.0f, 0.0f)
						[
							SNew(SBox)
							.MinDesiredWidth(320.0f)
							.HeightOverride(56.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66QuitConfirmationModal::HandleQuitClicked))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Danger"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Danger)
								.ContentPadding(FMargin(18.f, 10.f))
								[
									SNew(STextBlock)
									.Text(QuitText)
									.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
									.Justification(ETextJustify::Center)
									.AutoWrapText(false)
								]
							]
						]
					]
				]
			]
		];
}

FReply UT66QuitConfirmationModal::HandleStayClicked() { OnStayClicked(); return FReply::Handled(); }
FReply UT66QuitConfirmationModal::HandleQuitClicked() { OnQuitClicked(); return FReply::Handled(); }

void UT66QuitConfirmationModal::OnStayClicked() { CloseModal(); }
void UT66QuitConfirmationModal::OnQuitClicked() { UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false); }
