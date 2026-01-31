// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "UI/Style/T66Style.h"
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

void UT66RunSummaryScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	// Foundation: submit score at run end if allowed by settings (Practice Mode blocks).
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	if (RunState && LB)
	{
		LB->SubmitRunBounty(RunState->GetCurrentScore());
	}
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
			.Font(FT66Style::Tokens::FontRegular(12))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
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
		.BorderBackgroundColor(FT66Style::Tokens::Bg)
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(40.f)
			[
				SNew(SBorder)
				.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
				.Padding(FMargin(FT66Style::Tokens::Space8, FT66Style::Tokens::Space6))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 0.f, 0.f, 20.f)
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_RunSummaryTitle() : FText::FromString(TEXT("RUN SUMMARY")))
						.Font(FT66Style::Tokens::FontBold(32))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 0.f, 0.f, 8.f)
					[
						SNew(STextBlock)
						.Text(Loc ? FText::Format(Loc->GetText_RunSummaryStageReachedBountyFormat(), FText::AsNumber(StageReached), FText::AsNumber(Bounty))
							: FText::FromString(FString::Printf(TEXT("Stage Reached: %d  |  Bounty: %d"), StageReached, Bounty)))
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FT66Style::Tokens::Text)
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
							.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
							[
								SNew(STextBlock)
								.Text(Loc ? Loc->GetText_RunSummaryPreviewPlaceholder() : FText::FromString(TEXT("3D Preview")))
								.Font(FT66Style::Tokens::FontRegular(14))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
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
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Success)
								.ContentPadding(FMargin(16.f, 10.f))
								[
									SNew(STextBlock)
									.Text(Loc ? Loc->GetText_Restart() : FText::FromString(TEXT("RESTART")))
									.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
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
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								.ContentPadding(FMargin(16.f, 10.f))
								[
									SNew(STextBlock)
									.Text(Loc ? Loc->GetText_MainMenu() : FText::FromString(TEXT("MAIN MENU")))
									.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
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
