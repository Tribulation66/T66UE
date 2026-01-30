// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

UT66RunSummaryScreen::UT66RunSummaryScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::RunSummary;
	bIsModal = true;
}

void UT66RunSummaryScreen::RebuildLogItems()
{
	LogItems.Reset();

	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const TArray<FString>& Log = RunState ? RunState->GetEventLog() : TArray<FString>();

	LogItems.Reserve(Log.Num());
	for (const FString& Entry : Log)
	{
		LogItems.Add(MakeShared<FString>(Entry));
	}
}

TSharedRef<ITableRow> UT66RunSummaryScreen::GenerateLogRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FString Line = (Item.IsValid()) ? *Item : FString();

	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		.Padding(FMargin(4.f, 2.f))
		[
			SNew(STextBlock)
			.Text(FText::FromString(Line))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
			.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.f))
		];
}

TSharedRef<SWidget> UT66RunSummaryScreen::BuildSlateUI()
{
	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const int32 StageReached = RunState ? RunState->GetCurrentStage() : 1;
	const int32 Bounty = RunState ? RunState->GetCurrentScore() : 0;
	UT66LocalizationSubsystem* Loc = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	RebuildLogItems();
	TSharedRef<SListView<TSharedPtr<FString>>> LogList =
		SNew(SListView<TSharedPtr<FString>>)
		.ListItemsSource(&LogItems)
		.OnGenerateRow(SListView<TSharedPtr<FString>>::FOnGenerateRow::CreateUObject(this, &UT66RunSummaryScreen::GenerateLogRow))
		.SelectionMode(ESelectionMode::None);

	return SNew(SBorder)
		// Full-screen, opaque (no transparency)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 1.f))
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(40.f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.12f, 1.f))
				.Padding(40.f, 30.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 0.f, 0.f, 20.f)
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_RunSummaryTitle() : FText::FromString(TEXT("RUN SUMMARY")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 32))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 0.f, 0.f, 8.f)
					[
						SNew(STextBlock)
						.Text(Loc ? FText::Format(Loc->GetText_RunSummaryStageReachedBountyFormat(), FText::AsNumber(StageReached), FText::AsNumber(Bounty))
							: FText::FromString(FString::Printf(TEXT("Stage Reached: %d  |  Bounty: %d"), StageReached, Bounty)))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 0.f, 0.f, 12.f)
					[
						SNew(SBox)
						.WidthOverride(200.f)
						.HeightOverride(120.f)
						[
							SNew(SBorder)
							.BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.2f, 1.f))
							[
								SNew(STextBlock)
								.Text(Loc ? Loc->GetText_RunSummaryPreviewPlaceholder() : FText::FromString(TEXT("3D Preview")))
								.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
								.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.f))
								.Justification(ETextJustify::Center)
							]
						]
					]
					+ SVerticalBox::Slot()
					.MaxHeight(200.f)
					.Padding(0.f, 0.f, 0.f, 20.f)
					[
						SNew(SBox)
						.Visibility(bLogVisible ? EVisibility::Visible : EVisibility::Collapsed)
						[
							LogList
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(12.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(140.f)
							.HeightOverride(44.f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked))
								.ButtonColorAndOpacity(FLinearColor(0.2f, 0.5f, 0.2f, 1.f))
								[
									SNew(STextBlock)
									.Text(Loc ? Loc->GetText_Restart() : FText::FromString(TEXT("RESTART")))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(12.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(140.f)
							.HeightOverride(44.f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleMainMenuClicked))
								.ButtonColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.35f, 1.f))
								[
									SNew(STextBlock)
									.Text(Loc ? Loc->GetText_MainMenu() : FText::FromString(TEXT("MAIN MENU")))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
					]
				]
			]
		];
}

FReply UT66RunSummaryScreen::HandleRestartClicked() { OnRestartClicked(); return FReply::Handled(); }
FReply UT66RunSummaryScreen::HandleMainMenuClicked() { OnMainMenuClicked(); return FReply::Handled(); }
FReply UT66RunSummaryScreen::HandleViewLogClicked() { OnViewLogClicked(); return FReply::Handled(); }

void UT66RunSummaryScreen::OnRestartClicked()
{
	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState) RunState->ResetForNewRun();
	APlayerController* PC = GetOwningPlayer();
	if (PC) PC->SetPause(false);
	UGameplayStatics::OpenLevel(this, FName(TEXT("GameplayLevel")));
}

void UT66RunSummaryScreen::OnMainMenuClicked()
{
	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState) RunState->ResetForNewRun();
	APlayerController* PC = GetOwningPlayer();
	if (PC) PC->SetPause(false);
	UGameplayStatics::OpenLevel(this, FName(TEXT("FrontendLevel")));
}

void UT66RunSummaryScreen::OnViewLogClicked()
{
	bLogVisible = !bLogVisible;
	RefreshScreen();
}
