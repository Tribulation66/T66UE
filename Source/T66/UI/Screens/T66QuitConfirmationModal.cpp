// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66QuitConfirmationModal.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
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

	FText TitleText = Loc ? Loc->GetText_QuitConfirmTitle() : FText::FromString(TEXT("QUIT GAME?"));
	FText MessageText = Loc ? Loc->GetText_QuitConfirmMessage() : FText::FromString(TEXT("Are you sure you want to quit?"));
	FText StayText = Loc ? Loc->GetText_NoStay() : FText::FromString(TEXT("NO, I WANT TO STAY"));
	FText QuitText = Loc ? Loc->GetText_YesQuit() : FText::FromString(TEXT("YES, I WANT TO QUIT"));

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
					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 20.0f)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
						.ColorAndOpacity(FLinearColor::White)
					]
					// Message
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 30.0f)
					[
						SNew(STextBlock)
						.Text(MessageText)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
						.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
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
							.WidthOverride(180.0f)
							.HeightOverride(50.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66QuitConfirmationModal::HandleStayClicked))
								.ButtonColorAndOpacity(FLinearColor(0.2f, 0.5f, 0.2f, 1.0f))
								[
									SNew(STextBlock)
									.Text(StayText)
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.0f, 0.0f)
						[
							SNew(SBox)
							.WidthOverride(180.0f)
							.HeightOverride(50.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66QuitConfirmationModal::HandleQuitClicked))
								.ButtonColorAndOpacity(FLinearColor(0.5f, 0.2f, 0.2f, 1.0f))
								[
									SNew(STextBlock)
									.Text(QuitText)
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
									.ColorAndOpacity(FLinearColor::White)
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
