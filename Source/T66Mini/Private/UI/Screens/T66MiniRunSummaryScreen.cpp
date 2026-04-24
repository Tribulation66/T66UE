// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MiniRunSummaryScreen.h"

#include "Core/T66GameInstance.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Styling/SlateBrush.h"
#include "UI/Screens/T66MiniGeneratedScreenChrome.h"
#include "UI/T66MiniUIStyle.h"
#include "UI/T66UITypes.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FSlateBrush* T66MiniSceneBackgroundBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(TEXT("SourceAssets/UI/MainMenuReference/scene_background_purple_imagegen_1920x1080.png")),
			FMargin(0.f),
			TEXT("MiniSceneBackground"));
	}

	TSharedRef<SWidget> T66MiniMakeSceneBackground(const FLinearColor& FallbackColor)
	{
		if (const FSlateBrush* Brush = T66MiniSceneBackgroundBrush())
		{
			return SNew(SImage)
				.Image(Brush)
				.ColorAndOpacity(FLinearColor(0.82f, 0.82f, 0.88f, 1.0f));
		}

		return SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(FallbackColor);
	}
}

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
	const FString HeroDisplayName = Summary.HeroDisplayName.IsEmpty() ? FString(TEXT("Run Details")) : Summary.HeroDisplayName;
	const FString DifficultyDisplayName = Summary.DifficultyDisplayName.IsEmpty() ? FString(TEXT("Not selected")) : Summary.DifficultyDisplayName;

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			T66MiniMakeSceneBackground(T66MiniUI::ShellFill())
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(FLinearColor(0.018f, 0.014f, 0.024f, 0.58f))
			.Padding(FMargin(28.f, 24.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 8.f, 0.f, 8.f)
				[
					T66MiniGeneratedChrome::MakeTitlePlaque(
						FText::FromString(ResultTitle),
						38,
						560.f,
						86.f,
						ResultColor)
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
						T66MiniGeneratedChrome::MakePanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(HeroDisplayName)).Font(T66MiniUI::BoldFont(22)).ColorAndOpacity(FLinearColor::White)]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Difficulty: %s"), *DifficultyDisplayName))).Font(T66MiniUI::BodyFont(17)).ColorAndOpacity(T66MiniUI::MutedText())]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Wave reached: %d / 5"), Summary.WaveReached))).Font(T66MiniUI::BodyFont(17)).ColorAndOpacity(FLinearColor::White)]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Materials banked: %d"), Summary.MaterialsCollected))).Font(T66MiniUI::BodyFont(17)).ColorAndOpacity(FLinearColor::White)]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Run time: %.1fs"), Summary.RunSeconds))).Font(T66MiniUI::BodyFont(17)).ColorAndOpacity(FLinearColor::White)]
						, FMargin(26.f, 22.f, 88.f, 26.f))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						T66MiniGeneratedChrome::MakePanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(TEXT("LOADOUT SNAPSHOT"))).Font(T66MiniUI::BoldFont(21)).ColorAndOpacity(FLinearColor::White)]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Idols equipped: %d"), Summary.EquippedIdolCount))).Font(T66MiniUI::BodyFont(17)).ColorAndOpacity(T66MiniUI::MutedText())]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Items owned: %d"), Summary.OwnedItemCount))).Font(T66MiniUI::BodyFont(17)).ColorAndOpacity(T66MiniUI::MutedText())]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(TEXT("Use Play Again to jump straight back into idol selection with the same hero and difficulty."))).Font(T66MiniUI::BodyFont(15)).ColorAndOpacity(T66MiniUI::MutedText()).AutoWrapText(true)]
						, FMargin(26.f, 22.f, 88.f, 26.f))
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
						T66MiniGeneratedChrome::MakeButton(
							NSLOCTEXT("T66Mini.RunSummary", "PlayAgain", "PLAY AGAIN"),
							FOnClicked::CreateUObject(this, &UT66MiniRunSummaryScreen::HandlePlayAgainClicked),
							ET66ButtonType::Success,
							240.f,
							58.f,
							18)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
					[
						T66MiniGeneratedChrome::MakeButton(
							NSLOCTEXT("T66Mini.RunSummary", "MiniMenu", "MINI MENU"),
							FOnClicked::CreateUObject(this, &UT66MiniRunSummaryScreen::HandleMiniMenuClicked),
							ET66ButtonType::Neutral,
							240.f,
							58.f,
							18)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						T66MiniGeneratedChrome::MakeButton(
							NSLOCTEXT("T66Mini.RunSummary", "FullMainMenu", "FULL MAIN MENU"),
							FOnClicked::CreateUObject(this, &UT66MiniRunSummaryScreen::HandleMainMenuClicked),
							ET66ButtonType::Neutral,
							280.f,
							58.f,
							17)
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
