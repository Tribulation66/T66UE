// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66LobbyBackConfirmModal.h"
#include "UI/Screens/T66LobbyScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

UT66LobbyBackConfirmModal::UT66LobbyBackConfirmModal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::LobbyBackConfirm;
	bIsModal = true;
}

TSharedRef<SWidget> UT66LobbyBackConfirmModal::BuildSlateUI()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	FText TitleText = Loc ? Loc->GetText_LobbyLeaveConfirmTitle() : NSLOCTEXT("T66.Lobby", "LeaveConfirmTitle", "Leave lobby?");
	FText MessageText = Loc ? Loc->GetText_LobbyLeaveConfirmMessage() : NSLOCTEXT("T66.Lobby", "LeaveConfirmMessage", "Are you sure you want to leave the lobby?");
	FText StayText = Loc ? Loc->GetText_LobbyLeaveStay() : NSLOCTEXT("T66.Lobby", "LeaveStay", "STAY");
	FText LeaveText = Loc ? Loc->GetText_LobbyLeaveLeave() : NSLOCTEXT("T66.Lobby", "LeaveLeave", "LEAVE");

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
					.Padding(0.0f, 0.0f, 0.0f, 30.0f)
					[
						SNew(STextBlock)
						.Text(MessageText)
						.Font(FT66Style::Tokens::FontRegular(18))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
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
								.OnClicked(FOnClicked::CreateUObject(this, &UT66LobbyBackConfirmModal::HandleStayClicked))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Success)
								.ContentPadding(FMargin(18.f, 10.f))
								[
									SNew(STextBlock)
									.Text(StayText)
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
								.OnClicked(FOnClicked::CreateUObject(this, &UT66LobbyBackConfirmModal::HandleLeaveClicked))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Danger"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Danger)
								.ContentPadding(FMargin(18.f, 10.f))
								[
									SNew(STextBlock)
									.Text(LeaveText)
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

FReply UT66LobbyBackConfirmModal::HandleStayClicked()
{
	CloseModal();
	return FReply::Handled();
}

FReply UT66LobbyBackConfirmModal::HandleLeaveClicked()
{
	// Close modal first so GoBack() will actually navigate (it only closes modal if one is open).
	CloseModal();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->bHeroSelectionFromLobby = false;
	}
	if (UIManager)
	{
		if (UT66LobbyScreen* Lobby = Cast<UT66LobbyScreen>(UIManager->GetCurrentScreen()))
		{
			Lobby->NavigateBack();
		}
	}
	return FReply::Handled();
}

void UT66LobbyBackConfirmModal::OnStayClicked()
{
	CloseModal();
}

void UT66LobbyBackConfirmModal::OnLeaveClicked()
{
	CloseModal();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->bHeroSelectionFromLobby = false;
	}
	if (UIManager)
	{
		if (UT66LobbyScreen* Lobby = Cast<UT66LobbyScreen>(UIManager->GetCurrentScreen()))
		{
			Lobby->NavigateBack();
		}
	}
}
