// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66AccountStatusScreen.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"

#include "Kismet/GameplayStatics.h"

#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
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
	const bool bIsRestricted = (Record.Restriction != ET66AccountRestrictionKind::None);
	const bool bIsCertainty = (Record.Restriction == ET66AccountRestrictionKind::CheatingCertainty);
	const bool bCanAppeal = bIsRestricted && !bIsCertainty;

	// Cache the latest saved drafts.
	DraftAppealMessage = Record.LastAppealMessage;
	DraftEvidenceUrl = Record.LastEvidenceUrl;

	const FText TitleText = Loc ? Loc->GetText_AccountStatus() : NSLOCTEXT("T66.AccountStatus", "Title", "ACCOUNT STATUS");

	const FText HeadlineText =
		bIsCertainty
		? (Loc ? Loc->GetText_AccountStatus_CertaintyHeadline() : NSLOCTEXT("T66.AccountStatus", "CertaintyHeadline", "FORBIDDEN EVENT DETECTED"))
		: (Loc ? Loc->GetText_AccountStatus_SuspicionHeadline() : NSLOCTEXT("T66.AccountStatus", "SuspicionHeadline", "RUN UNDER SUSPICION OF CHEATING"));

	const FText ReasonLabelText = Loc ? Loc->GetText_AccountStatus_ReasonLabel() : NSLOCTEXT("T66.AccountStatus", "ReasonLabel", "Reason");
	const FText RunInQuestionText = Loc ? Loc->GetText_AccountStatus_RunInQuestion() : NSLOCTEXT("T66.AccountStatus", "RunInQuestion", "RUN IN QUESTION");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");

	const FText AppealTitleText = Loc ? Loc->GetText_AccountStatus_AppealTitle() : NSLOCTEXT("T66.AccountStatus", "AppealTitle", "APPEAL");
	const FText AppealHintText = Loc ? Loc->GetText_AccountStatus_AppealHint() : NSLOCTEXT("T66.AccountStatus", "AppealHint", "Write your appeal message here...");
	const FText EvidenceHintText = Loc ? Loc->GetText_AccountStatus_EvidenceHint() : NSLOCTEXT("T66.AccountStatus", "EvidenceHint", "Evidence link (optional)...");
	const FText SubmitAppealText = Loc ? Loc->GetText_AccountStatus_SubmitAppeal() : NSLOCTEXT("T66.AccountStatus", "SubmitAppeal", "SUBMIT APPEAL");

	const FText NoAppealText = Loc ? Loc->GetText_AccountStatus_NoAppealAvailable() : NSLOCTEXT("T66.AccountStatus", "NoAppealAvailable", "NO APPEAL AVAILABLE.");

	auto AppealStatusToText = [Loc](ET66AppealReviewStatus Status) -> FText
	{
		switch (Status)
		{
		case ET66AppealReviewStatus::NotSubmitted:
			return Loc ? Loc->GetText_AccountStatus_AppealNotSubmitted() : NSLOCTEXT("T66.AccountStatus", "AppealNotSubmitted", "Appeal not submitted");
		case ET66AppealReviewStatus::UnderReview:
			return Loc ? Loc->GetText_AccountStatus_AppealUnderReview() : NSLOCTEXT("T66.AccountStatus", "AppealUnderReview", "Appeal under review");
		case ET66AppealReviewStatus::Denied:
			return Loc ? Loc->GetText_AccountStatus_AppealDenied() : NSLOCTEXT("T66.AccountStatus", "AppealDenied", "Appeal denied");
		case ET66AppealReviewStatus::Approved:
			return Loc ? Loc->GetText_AccountStatus_AppealApproved() : NSLOCTEXT("T66.AccountStatus", "AppealApproved", "Appeal approved");
		default:
			return NSLOCTEXT("T66.Common", "Unknown", "UNKNOWN");
		}
	};

	const FText StatusLine = AppealStatusToText(Record.AppealStatus);

	// Left column: warning + reasons + run-in-question + back.
	TSharedRef<SWidget> LeftColumn =
		SNew(SVerticalBox)
		// Header
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(HeadlineText)
			.Font(FT66Style::Tokens::FontBold(26))
			.ColorAndOpacity(FLinearColor(0.85f, 0.15f, 0.15f, 1.f))
		]
		// Reason label
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(ReasonLabelText)
			.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		]
		// Reason content
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
		[
			FT66Style::MakePanel(
				SNew(STextBlock)
				.Text_Lambda([this, Record, bIsRestricted]()
				{
					if (!bIsRestricted)
					{
						return NSLOCTEXT("T66.AccountStatus", "NotRestrictedBody", "Your account is in good standing.");
					}
					// Reason strings originate from runtime detection / backend; they are not localized.
					const FString Reason = Record.RestrictionReason.IsEmpty() ? FString(TEXT("Unknown")) : Record.RestrictionReason;
					return FText::FromString(Reason);
				})
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
				.AutoWrapText(true),
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space3)
			)
		]
		// Run In Question
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			FT66Style::MakeButton(FT66ButtonParams(RunInQuestionText, FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleRunInQuestionClicked))
				.SetPadding(FMargin(18.f, 10.f))
				.SetEnabled(TAttribute<bool>::CreateLambda([this, LB]() { return LB && LB->HasAccountRestrictionRunSummary(); })))
		]
		// Spacer to push Back to bottom
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SNew(SSpacer)
		]
		// Back
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)
		[
			FT66Style::MakeButton(FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleBackClicked))
				.SetPadding(FMargin(18.f, 10.f)))
		];

	// Right column: appeal box or locked message.
	TSharedRef<SWidget> RightColumn =
		bCanAppeal
		? StaticCastSharedRef<SWidget>(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(STextBlock)
				.Text(AppealTitleText)
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(STextBlock)
				.Text(StatusLine)
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
			+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 0.f, 0.f, 10.f)
			[
				SAssignNew(AppealTextBox, SMultiLineEditableTextBox)
				.Text(FText::FromString(DraftAppealMessage))
				.OnTextChanged_Lambda([this](const FText& NewText)
				{
					DraftAppealMessage = NewText.ToString();
				})
				.HintText(AppealHintText)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SAssignNew(EvidenceUrlTextBox, SEditableTextBox)
				.Text(FText::FromString(DraftEvidenceUrl))
				.OnTextChanged_Lambda([this](const FText& NewText)
				{
					DraftEvidenceUrl = NewText.ToString();
				})
				.HintText(EvidenceHintText)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
			[
				FT66Style::MakeButton(FT66ButtonParams(SubmitAppealText, FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleSubmitAppealClicked), ET66ButtonType::Primary)
					.SetPadding(FMargin(18.f, 10.f))
					.SetEnabled(TAttribute<bool>::CreateLambda([this, LB]() { return LB && LB->CanSubmitAccountAppeal(); })))
			])
		: StaticCastSharedRef<SWidget>(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(STextBlock)
				.Text(AppealTitleText)
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(STextBlock)
				.Text(StatusLine)
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				FT66Style::MakePanel(
					SNew(STextBlock)
					.Text(NoAppealText)
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Center),
					FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space3)
				)
			]);

	// Modal overlay: dim background + centered two-column panel.
	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			// Dim overlay to keep main menu visible underneath.
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.65f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(FMargin(60.f))
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				// Title
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
				[
					SNew(STextBlock)
					.Text(TitleText)
					.Font(FT66Style::Tokens::FontBold(34))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				// Two columns
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(0.5f).Padding(0.f, 0.f, 18.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(560.f)
						.HeightOverride(520.f)
						[
							LeftColumn
						]
					]
					+ SHorizontalBox::Slot().FillWidth(0.5f)
					[
						SNew(SBox)
						.WidthOverride(560.f)
						.HeightOverride(520.f)
						[
							RightColumn
						]
					]
				],
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space5)
			)
		];
}

FReply UT66AccountStatusScreen::HandleBackClicked()
{
	if (UIManager)
	{
		UIManager->CloseModal();
	}
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleRunInQuestionClicked()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	if (LB && UIManager)
	{
		// Ask Run Summary to return to Account Status when it closes.
		if (LB->RequestOpenAccountRestrictionRunSummary())
		{
			UIManager->ShowModal(ET66ScreenType::RunSummary);
		}
	}
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleSubmitAppealClicked()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	if (!LB)
	{
		return FReply::Handled();
	}

	const FString Msg = AppealTextBox.IsValid() ? AppealTextBox->GetText().ToString() : DraftAppealMessage;
	const FString Url = EvidenceUrlTextBox.IsValid() ? EvidenceUrlTextBox->GetText().ToString() : DraftEvidenceUrl;
	LB->SubmitAccountAppeal(Msg, Url);

	// Refresh to reflect new appeal status.
	InvalidateLayoutAndVolatility();
	RefreshScreen();
	return FReply::Handled();
}

