// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66AccountStatusScreen.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/Dota/T66DotaTheme.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"

#include "Kismet/GameplayStatics.h"

#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

UT66AccountStatusScreen::UT66AccountStatusScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::AccountStatus;
	bIsModal = true;
}

void UT66AccountStatusScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	// Ensure we display the latest account restriction state.
	RefreshScreen();
}

void UT66AccountStatusScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	// Rebuild Slate to refresh visibility/state and to reflect any changed localization.
	ForceRebuildSlate();
}

TSharedRef<SWidget> UT66AccountStatusScreen::BuildSlateUI()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	const FT66AccountRestrictionRecord Record = LB ? LB->GetAccountRestrictionRecord() : FT66AccountRestrictionRecord();
	const bool bAccountGood = LB ? LB->IsAccountEligibleForLeaderboard() : true;
	const bool bShowReason = !bAccountGood && !Record.RestrictionReason.IsEmpty();

	const FText TitleText = Loc ? Loc->GetText_AccountStatus() : NSLOCTEXT("T66.AccountStatus", "Title", "ACCOUNT STATUS");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FText StatusHeadlineText = bAccountGood
		? NSLOCTEXT("T66.AccountStatus", "GoodHeadline", "ACCOUNT STATUS: GOOD")
		: NSLOCTEXT("T66.AccountStatus", "BadHeadline", "ACCOUNT STATUS: BAD");
	const FText StatusBodyText = bAccountGood
		? NSLOCTEXT("T66.AccountStatus", "GoodBody", "Your scores go to the leaderboard.")
		: NSLOCTEXT("T66.AccountStatus", "BadBody", "Your scores do not go to the leaderboard.");
	const FLinearColor StatusColor = bAccountGood
		? FLinearColor(0.58f, 0.76f, 0.54f, 1.0f)
		: FLinearColor(0.85f, 0.22f, 0.18f, 1.0f);

	TSharedRef<SWidget> Content =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
		[
			SNew(STextBlock)
			.Text(TitleText)
			.Font(FT66Style::Tokens::FontBold(34))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
		[
			SNew(STextBlock)
			.Text(StatusHeadlineText)
			.Font(FT66Style::Tokens::FontBold(28))
			.ColorAndOpacity(StatusColor)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, bShowReason ? 12.f : 26.f)
		[
			SNew(STextBlock)
			.Text(StatusBodyText)
			.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
			.ColorAndOpacity(FT66Style::Tokens::Text)
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 26.f)
		[
			SNew(SBox)
			.Visibility(bShowReason ? EVisibility::Visible : EVisibility::Collapsed)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_AccountStatus_ReasonLabel() : NSLOCTEXT("T66.AccountStatus", "ReasonLabel", "Reason"))
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Record.RestrictionReason))
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.AutoWrapText(true)
					],
					FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space3)
				)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
		[
			FT66Style::MakeButton(FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleBackClicked))
				.SetPadding(FMargin(18.f, 10.f)))
		];

	// Modal overlay: dim background + centered two-column panel.
	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			// Dim overlay to keep main menu visible underneath.
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66DotaTheme::Scrim())
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(FMargin(60.f))
		[
			FT66Style::MakePanel(
				SNew(SBox)
				.WidthOverride(720.f)
				[
					Content
				],
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space5)
			)
		];
}

FReply UT66AccountStatusScreen::HandleBackClicked()
{
	if (UIManager)
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (PC->IsPaused())
			{
				UIManager->ShowModal(ET66ScreenType::Leaderboard);
				return FReply::Handled();
			}
		}

		UIManager->CloseModal();
	}
	return FReply::Handled();
}
