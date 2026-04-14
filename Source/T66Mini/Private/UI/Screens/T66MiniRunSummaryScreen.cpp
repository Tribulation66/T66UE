// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MiniRunSummaryScreen.h"

#include "Core/T66GameInstance.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "UI/T66MiniUIStyle.h"
#include "UI/T66UITypes.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

UT66MiniRunSummaryScreen::UT66MiniRunSummaryScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::MiniRunSummary;
	bIsModal = false;
}

TSharedRef<SWidget> UT66MiniRunSummaryScreen::BuildSlateUI()
{
	const UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	const FT66MiniRunSummary Summary = FrontendState ? FrontendState->GetLastRunSummary() : FT66MiniRunSummary();
	const FString ResultTitle = Summary.bWasVictory ? TEXT("RUN CLEARED") : TEXT("RUN ENDED");
	const FLinearColor ResultColor = Summary.bWasVictory ? T66MiniUI::AccentGreen() : FLinearColor(0.88f, 0.30f, 0.24f, 1.0f);

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(T66MiniUI::ShellFill())
			.Padding(FMargin(28.f, 24.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 8.f, 0.f, 8.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(ResultTitle))
					.Font(T66MiniUI::TitleFont(42))
					.ColorAndOpacity(ResultColor)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 0.f, 0.f, 20.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Summary.ResultLabel))
					.Font(T66MiniUI::BodyFont(20))
					.ColorAndOpacity(T66MiniUI::MutedText())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.Padding(0.f, 0.f, 16.f, 0.f)
					[
						SNew(SBorder)
						.BorderImage(T66MiniUI::WhiteBrush())
						.BorderBackgroundColor(T66MiniUI::PanelFill())
						.Padding(FMargin(18.f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(Summary.HeroDisplayName)).Font(T66MiniUI::BoldFont(24)).ColorAndOpacity(FLinearColor::White)]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Difficulty: %s"), *Summary.DifficultyDisplayName))).Font(T66MiniUI::BodyFont(18)).ColorAndOpacity(T66MiniUI::MutedText())]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Wave reached: %d / 5"), Summary.WaveReached))).Font(T66MiniUI::BodyFont(18)).ColorAndOpacity(FLinearColor::White)]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Materials banked: %d"), Summary.MaterialsCollected))).Font(T66MiniUI::BodyFont(18)).ColorAndOpacity(FLinearColor::White)]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Run time: %.1fs"), Summary.RunSeconds))).Font(T66MiniUI::BodyFont(18)).ColorAndOpacity(FLinearColor::White)]
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SNew(SBorder)
						.BorderImage(T66MiniUI::WhiteBrush())
						.BorderBackgroundColor(T66MiniUI::CardFill())
						.Padding(FMargin(18.f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(TEXT("LOADOUT SNAPSHOT"))).Font(T66MiniUI::BoldFont(22)).ColorAndOpacity(FLinearColor::White)]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Idols equipped: %d"), Summary.EquippedIdolCount))).Font(T66MiniUI::BodyFont(18)).ColorAndOpacity(T66MiniUI::MutedText())]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Items owned: %d"), Summary.OwnedItemCount))).Font(T66MiniUI::BodyFont(18)).ColorAndOpacity(T66MiniUI::MutedText())]
							+ SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Bottom).Padding(0.f, 14.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(TEXT("Use Play Again to jump straight back into idol selection with the same hero and difficulty."))).Font(T66MiniUI::BodyFont(16)).ColorAndOpacity(T66MiniUI::MutedText()).AutoWrapText(true)]
						]
					]
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				.VAlign(VAlign_Bottom)
				.Padding(0.f, 20.f, 0.f, 0.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
					[
						SNew(SButton)
						.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniRunSummaryScreen::HandlePlayAgainClicked))
						.ButtonColorAndOpacity(T66MiniUI::AccentGreen())
						[
							SNew(STextBlock).Text(FText::FromString(TEXT("PLAY AGAIN"))).Font(T66MiniUI::BoldFont(20)).ColorAndOpacity(T66MiniUI::ButtonTextDark()).Justification(ETextJustify::Center)
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
					[
						SNew(SButton)
						.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniRunSummaryScreen::HandleMiniMenuClicked))
						.ButtonColorAndOpacity(T66MiniUI::AccentBlue())
						[
							SNew(STextBlock).Text(FText::FromString(TEXT("MINI MENU"))).Font(T66MiniUI::BoldFont(20)).ColorAndOpacity(FLinearColor::White).Justification(ETextJustify::Center)
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(SButton)
						.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniRunSummaryScreen::HandleMainMenuClicked))
						.ButtonColorAndOpacity(T66MiniUI::RaisedFill())
						[
							SNew(STextBlock).Text(FText::FromString(TEXT("FULL MAIN MENU"))).Font(T66MiniUI::BoldFont(20)).ColorAndOpacity(FLinearColor::White).Justification(ETextJustify::Center)
						]
					]
				]
			]
		];
}

FReply UT66MiniRunSummaryScreen::HandlePlayAgainClicked()
{
	if (UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr)
	{
		const FT66MiniRunSummary Summary = FrontendState->GetLastRunSummary();
		FrontendState->BeginNewRun();
		FrontendState->SelectHero(Summary.HeroID);
		FrontendState->SelectDifficulty(Summary.DifficultyID);
		if (UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr)
		{
			FrontendState->RefreshIdolOffers(DataSubsystem);
		}
	}

	NavigateTo(ET66ScreenType::MiniIdolSelect);
	return FReply::Handled();
}

FReply UT66MiniRunSummaryScreen::HandleMiniMenuClicked()
{
	if (UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->ResetRunSetup();
	}

	NavigateTo(ET66ScreenType::MiniMainMenu);
	return FReply::Handled();
}

FReply UT66MiniRunSummaryScreen::HandleMainMenuClicked()
{
	if (UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->ResetRunSetup();
	}

	NavigateTo(ET66ScreenType::MainMenu);
	return FReply::Handled();
}
