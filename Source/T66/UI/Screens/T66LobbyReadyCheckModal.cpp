// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66LobbyReadyCheckModal.h"
#include "UI/Screens/T66LobbyScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

UT66LobbyReadyCheckModal::UT66LobbyReadyCheckModal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::LobbyReadyCheck;
	bIsModal = true;
}

TSharedRef<SWidget> UT66LobbyReadyCheckModal::BuildSlateUI()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	FText TitleText = Loc ? Loc->GetText_LobbyReadyToStart() : NSLOCTEXT("T66.Lobby", "ReadyToStart", "Ready to start?");
	FText ReadyText = Loc ? Loc->GetText_LobbyReady() : NSLOCTEXT("T66.Lobby", "Ready", "READY");
	FText NotReadyText = Loc ? Loc->GetText_LobbyNotReady() : NSLOCTEXT("T66.Lobby", "NotReady", "NOT READY");

	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f))
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
				.Padding(FMargin(40.0f, 30.0f))
				[
					SNew(SVerticalBox)
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
							.MinDesiredWidth(200.0f)
							.HeightOverride(56.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66LobbyReadyCheckModal::HandleReadyClicked))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Success)
								.ContentPadding(FMargin(18.f, 10.f))
								[
									SNew(STextBlock)
									.Text(ReadyText)
									.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
									.Justification(ETextJustify::Center)
								]
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.0f, 0.0f)
						[
							SNew(SBox)
							.MinDesiredWidth(200.0f)
							.HeightOverride(56.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66LobbyReadyCheckModal::HandleNotReadyClicked))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Danger"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Danger)
								.ContentPadding(FMargin(18.f, 10.f))
								[
									SNew(STextBlock)
									.Text(NotReadyText)
									.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
									.Justification(ETextJustify::Center)
								]
							]
						]
					]
				]
			]
		];
}

FReply UT66LobbyReadyCheckModal::HandleReadyClicked()
{
	if (UIManager)
	{
		if (UT66LobbyScreen* Lobby = Cast<UT66LobbyScreen>(UIManager->GetCurrentScreen()))
		{
			Lobby->SetReadyCheckConfirmed(true);
		}
		CloseModal();
	}
	return FReply::Handled();
}

FReply UT66LobbyReadyCheckModal::HandleNotReadyClicked()
{
	CloseModal();
	return FReply::Handled();
}

void UT66LobbyReadyCheckModal::OnReadyClicked()
{
	if (UIManager)
	{
		if (UT66LobbyScreen* Lobby = Cast<UT66LobbyScreen>(UIManager->GetCurrentScreen()))
		{
			Lobby->SetReadyCheckConfirmed(true);
		}
		CloseModal();
	}
}

void UT66LobbyReadyCheckModal::OnNotReadyClicked()
{
	CloseModal();
}
