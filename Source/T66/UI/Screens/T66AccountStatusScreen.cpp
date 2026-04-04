// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66AccountStatusScreen.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"

#include "Kismet/GameplayStatics.h"

#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

namespace
{
	FLinearColor T66AccountStatusShellFill()
	{
		return FT66Style::Background();
	}

	FLinearColor T66AccountStatusPanelFill()
	{
		return FT66Style::PanelOuter();
	}

	FLinearColor T66AccountStatusNeutralButtonFill()
	{
		return FT66Style::ButtonNeutral();
	}

	TSharedRef<SWidget> MakeAccountStatusPanel(const TSharedRef<SWidget>& Content, ET66PanelType Type, const FLinearColor& FillColor, const FMargin& Padding)
	{
		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(Type)
				.SetBorderVisual(ET66ButtonBorderVisual::None)
				.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
				.SetColor(FillColor)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeAccountStatusButton(const FT66ButtonParams& Params)
	{
		FT66ButtonParams FlatParams = Params;
		FlatParams
			.SetBorderVisual(ET66ButtonBorderVisual::None)
			.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
			.SetUseGlow(false);

		return FT66Style::MakeButton(FlatParams);
	}
}

UT66AccountStatusScreen::UT66AccountStatusScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::AccountStatus;
	bIsModal = false;
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
	const bool bModalPresentation = (UIManager && UIManager->GetCurrentModalType() == ScreenType) || (!UIManager && GetOwningPlayer() && GetOwningPlayer()->IsPaused());
	const float TopInset = bModalPresentation ? 0.f : (UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f);
	const bool bShowBackButton = bModalPresentation;

	TSharedRef<SWidget> Content =
		SNew(SBox)
		.MaxDesiredWidth(760.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
			[
				SNew(STextBlock)
				.Text(TitleText)
				.Font(FT66Style::Tokens::FontBold(34))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(STextBlock)
				.Text(StatusHeadlineText)
				.Font(FT66Style::Tokens::FontBold(24))
				.ColorAndOpacity(StatusColor)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, bShowReason ? 10.f : 20.f)
			[
				SNew(SBox)
				.MaxDesiredWidth(620.f)
				[
					SNew(STextBlock)
					.Text(StatusBodyText)
					.Font(FT66Style::Tokens::FontRegular(18))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.AutoWrapText(true)
					.Justification(ETextJustify::Center)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 20.f)
			[
				SNew(SBox)
				.Visibility(bShowReason ? EVisibility::Visible : EVisibility::Collapsed)
				[
					FT66Style::MakePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
						[
							SNew(STextBlock)
							.Text(Loc ? Loc->GetText_AccountStatus_ReasonLabel() : NSLOCTEXT("T66.AccountStatus", "ReasonLabel", "Reason"))
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(FText::FromString(Record.RestrictionReason))
							.Font(FT66Style::Tokens::FontRegular(16))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.AutoWrapText(true)
						],
						FT66PanelParams(ET66PanelType::Panel)
							.SetBorderVisual(ET66ButtonBorderVisual::None)
							.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
							.SetColor(T66AccountStatusPanelFill())
							.SetPadding(FMargin(16.f, 14.f))
					)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
			[
				SNew(SBox)
				.Visibility(bShowBackButton ? EVisibility::Visible : EVisibility::Collapsed)
				[
					MakeAccountStatusButton(
						FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleBackClicked))
						.SetFontSize(18)
						.SetPadding(FMargin(16.f, 8.f))
						.SetColor(T66AccountStatusNeutralButtonFill()))
				]
			]
		];

	const TSharedRef<SWidget> Root =
		bModalPresentation
		? StaticCastSharedRef<SWidget>(
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66Style::Scrim())
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(FMargin(48.f))
			[
				MakeAccountStatusPanel(
					SNew(SBox)
					.WidthOverride(700.f)
					[
						MakeAccountStatusPanel(
							SNew(SBox)
							.HAlign(HAlign_Center)
							.Padding(FMargin(28.f, 24.f, 28.f, 24.f))
							[
								Content
							],
							ET66PanelType::Panel2,
							T66AccountStatusPanelFill(),
							FMargin(16.f))
					],
					ET66PanelType::Panel,
					T66AccountStatusShellFill(),
					FMargin(24.f)
				)
			])
		: StaticCastSharedRef<SWidget>(
			SNew(SBox)
			.Padding(FMargin(0.f, TopInset, 0.f, 0.f))
			[
				MakeAccountStatusPanel(
					MakeAccountStatusPanel(
						SNew(SBox)
						.HAlign(HAlign_Center)
						.Padding(FMargin(28.f, 24.f, 28.f, 24.f))
						[
							Content
						],
						ET66PanelType::Panel2,
						T66AccountStatusPanelFill(),
						FMargin(16.f)),
					ET66PanelType::Panel,
					T66AccountStatusShellFill(),
					FMargin(24.f))
			]);

	return Root;
}

FReply UT66AccountStatusScreen::HandleBackClicked()
{
	if (UIManager)
	{
		if (UIManager->GetCurrentModalType() == ScreenType)
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
			return FReply::Handled();
		}

		UIManager->GoBack();
	}
	return FReply::Handled();
}
