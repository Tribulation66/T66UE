// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MiniSaveSlotsScreen.h"

#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66MiniRunStateSubsystem.h"
#include "Core/T66MiniRuntimeSubsystem.h"
#include "Data/T66MiniDataTypes.h"
#include "Save/T66MiniRunSaveGame.h"
#include "Save/T66MiniSaveSubsystem.h"
#include "UI/Screens/T66MiniGeneratedScreenChrome.h"
#include "UI/T66MiniUIStyle.h"
#include "UI/T66UIManager.h"
#include "UI/T66UITypes.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FSlateBrush* T66MiniSaveSlotsSceneBackgroundBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(TEXT("SourceAssets/UI/MainMenuReference/scene_background_purple_imagegen_1920x1080.png")),
			FMargin(0.f),
			TEXT("MiniSceneBackground"));
	}

	TSharedRef<SWidget> T66MiniMakeSaveSlotsSceneBackground(const FLinearColor& FallbackColor)
	{
		if (const FSlateBrush* Brush = T66MiniSaveSlotsSceneBackgroundBrush())
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

UT66MiniSaveSlotsScreen::UT66MiniSaveSlotsScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::MiniSaveSlots;
	bIsModal = false;
}

TSharedRef<SWidget> UT66MiniSaveSlotsScreen::BuildSlateUI()
{
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	const TArray<FT66MiniSaveSlotSummary> SlotSummaries = SaveSubsystem ? SaveSubsystem->BuildRunSlotSummaries(DataSubsystem) : TArray<FT66MiniSaveSlotSummary>();

	TSharedRef<SGridPanel> CardGrid = SNew(SGridPanel);
	for (int32 Index = 0; Index < SlotSummaries.Num(); ++Index)
	{
		const FT66MiniSaveSlotSummary& Summary = SlotSummaries[Index];
		const int32 Row = Index / 2;
		const int32 Column = Index % 2;

		CardGrid->AddSlot(Column, Row)
			.Padding(Column == 0 ? FMargin(0.f, 0.f, 18.f, 18.f) : FMargin(0.f, 0.f, 0.f, 18.f))
			[
				SNew(SBox)
				.WidthOverride(520.f)
				.HeightOverride(182.f)
				[
					T66MiniGeneratedChrome::MakePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 14.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(54.f)
								.HeightOverride(54.f)
								[
									T66MiniGeneratedChrome::MakePanel(
										SNew(STextBlock)
										.Text(FText::AsNumber(Summary.SlotIndex + 1))
										.Font(T66MiniUI::BoldFont(20))
										.ColorAndOpacity(T66MiniUI::AccentGold())
										.Justification(ETextJustify::Center),
										FMargin(4.f),
										T66MiniGeneratedChrome::ESlice::BadgeFrame)
								]
							]
							+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Mini Slot %d"), Summary.SlotIndex + 1)))
								.Font(T66MiniUI::BoldFont(22))
								.ColorAndOpacity(FLinearColor::White)
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 10.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(Summary.bOccupied
								? FString::Printf(TEXT("%s  |  %s  |  Wave %d"), *Summary.HeroDisplayName, *Summary.DifficultyDisplayName, Summary.WaveIndex)
								: TEXT("Empty slot")))
							.Font(T66MiniUI::BodyFont(17))
							.ColorAndOpacity(Summary.bOccupied ? FLinearColor::White : T66MiniUI::MutedText())
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 8.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(Summary.bOccupied
								? FString::Printf(TEXT("Last Updated: %s"), *Summary.LastUpdatedUtc)
								: TEXT("No mini run saved yet.")))
							.Font(T66MiniUI::BodyFont(14))
							.ColorAndOpacity(T66MiniUI::MutedText())
							.AutoWrapText(true)
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						.VAlign(VAlign_Bottom)
						[
							Summary.bOccupied
								? StaticCastSharedRef<SWidget>(
									T66MiniGeneratedChrome::MakeButton(
										T66MiniGeneratedChrome::MakeButtonParams(
											NSLOCTEXT("T66Mini.SaveSlots", "Load", "LOAD"),
											FOnClicked::CreateLambda([this, SlotIndex = Summary.SlotIndex]()
											{
												return HandleLoadSlotClicked(SlotIndex);
											}),
											ET66ButtonType::Success,
											128.f,
											42.f,
											16)
										.SetEnabled(true)))
								: StaticCastSharedRef<SWidget>(SNew(SSpacer))
						]
					, FMargin(22.f, 18.f, 52.f, 18.f))
				]
			];
	}

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			T66MiniMakeSaveSlotsSceneBackground(T66MiniUI::ShellFill())
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(FLinearColor(0.018f, 0.014f, 0.024f, 0.58f))
			.Padding(FMargin(28.f, 22.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 0.f, 0.f, 18.f)
				[
					T66MiniGeneratedChrome::MakeTitlePlaque(
						NSLOCTEXT("T66Mini.SaveSlots", "Title", "Mini Save Slots"),
						34,
						560.f,
						86.f)
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					CardGrid
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(172.f, 12.f, 0.f, 0.f)
				[
					T66MiniGeneratedChrome::MakeRowPanel(
						SAssignNew(StatusTextBlock, STextBlock)
						.Text(NSLOCTEXT("T66Mini.SaveSlots", "DefaultStatus", "These slots are mini-specific and separate from the regular game's save pipeline."))
						.Font(T66MiniUI::BodyFont(16))
						.ColorAndOpacity(T66MiniUI::MutedText())
						.AutoWrapText(true)
					, FMargin(14.f))
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(28.f, 0.f, 0.f, 24.f)
		[
			SNew(SBox)
			.WidthOverride(140.f)
			.HeightOverride(48.f)
			[
				T66MiniGeneratedChrome::MakeButton(
					NSLOCTEXT("T66Mini.SaveSlots", "Back", "BACK"),
					FOnClicked::CreateUObject(this, &UT66MiniSaveSlotsScreen::HandleBackClicked),
					ET66ButtonType::Neutral,
					140.f,
					48.f,
					18)
			]
		];
}

FReply UT66MiniSaveSlotsScreen::HandleBackClicked()
{
	NavigateBack();
	return FReply::Handled();
}

FReply UT66MiniSaveSlotsScreen::HandleLoadSlotClicked(const int32 SlotIndex)
{
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniRuntimeSubsystem* RuntimeSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRuntimeSubsystem>() : nullptr;
	if (!SaveSubsystem || !FrontendState || !RunState || !RuntimeSubsystem)
	{
		return FReply::Handled();
	}

	if (UT66MiniRunSaveGame* RunSave = SaveSubsystem->LoadRunFromSlot(SlotIndex))
	{
		FrontendState->SeedFromRunSave(RunSave);
		RunState->LoadRunFromSave(RunSave);

		if (RunSave->bPendingShopIntermission)
		{
			FrontendState->ResumeIntermissionFlow();
			NavigateTo(ET66ScreenType::MiniShop);
			SetStatus(FText::FromString(FString::Printf(TEXT("Loaded mini slot %d into intermission shop flow."), SlotIndex + 1)));
			return FReply::Handled();
		}

		FString FailureReason;
		if (!RuntimeSubsystem->LaunchMiniBattle(this, &FailureReason))
		{
			SetStatus(FText::FromString(FailureReason));
			return FReply::Handled();
		}

		if (UIManager)
		{
			UIManager->HideAllUI();
		}

		SetStatus(FText::FromString(FString::Printf(TEXT("Loaded mini slot %d and launched the isolated mini battle runtime."), SlotIndex + 1)));
	}
	else
	{
		SetStatus(FText::FromString(FString::Printf(TEXT("Mini slot %d could not be loaded."), SlotIndex + 1)));
	}

	return FReply::Handled();
}

void UT66MiniSaveSlotsScreen::SetStatus(const FText& InText)
{
	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(InText);
	}
}
