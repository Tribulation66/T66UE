// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66LeaderboardScreen.h"

#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/Components/T66LeaderboardPanel.h"
#include "UI/Dota/T66DotaSlate.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"

#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

UT66LeaderboardScreen::UT66LeaderboardScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Leaderboard;
	bIsModal = true;
}

void UT66LeaderboardScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	if (LeaderboardPanel.IsValid())
	{
		LeaderboardPanel->SetUIManager(UIManager);
	}
}

TSharedRef<SWidget> UT66LeaderboardScreen::BuildSlateUI()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;

	const FText TitleText = Loc ? Loc->GetText_Leaderboard() : NSLOCTEXT("T66.Leaderboard", "Title", "LEADERBOARD");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FLinearColor ScrimColor(FT66Style::Tokens::Scrim.R, FT66Style::Tokens::Scrim.G, FT66Style::Tokens::Scrim.B, 0.95f);
	const TSharedRef<SWidget> Content =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66LeaderboardScreen::HandleBackClicked), ET66ButtonType::Neutral)
					.SetMinWidth(120.f))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(TitleText)
				.Font(FT66Style::Tokens::FontBold(36))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(120.f)
			]
		]
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SAssignNew(LeaderboardPanel, ST66LeaderboardPanel)
			.LocalizationSubsystem(Loc)
			.LeaderboardSubsystem(LB)
			.UIManager(UIManager)
		];

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(ScrimColor)
		.Padding(FMargin(40.f))
		[
			FT66Style::IsDotaTheme()
				? FT66DotaSlate::MakeScreenSurface(Content, FMargin(20.f))
				: Content
		];
}

FReply UT66LeaderboardScreen::HandleBackClicked()
{
	if (!UIManager)
	{
		return FReply::Handled();
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (PC->IsPaused())
		{
			UIManager->ShowModal(ET66ScreenType::PauseMenu);
			return FReply::Handled();
		}
	}

	UIManager->CloseModal();
	return FReply::Handled();
}
