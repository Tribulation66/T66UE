// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66ReportBugScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66PlayerController.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/EngineVersion.h"
#include "Misc/App.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/SOverlay.h"

UT66ReportBugScreen::UT66ReportBugScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::ReportBug;
	bIsModal = true;
}

TSharedRef<SWidget> UT66ReportBugScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	FText TitleText = Loc ? Loc->GetText_ReportBugTitle() : NSLOCTEXT("T66.ReportBug", "Title", "REPORT BUG");
	FText SubmitText = Loc ? Loc->GetText_ReportBugSubmit() : NSLOCTEXT("T66.ReportBug", "Submit", "SUBMIT");
	FText CancelText = Loc ? Loc->GetText_Cancel() : NSLOCTEXT("T66.Common", "Cancel", "CANCEL");
	FText HintText = Loc ? Loc->GetText_DescribeTheBugHint() : NSLOCTEXT("T66.ReportBug", "Hint", "Describe the bug...");

	BugReportText.Empty();

	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f))
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.15f, 1.0f))
				.Padding(FMargin(40.0f, 30.0f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 16.0f)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FT66Style::Tokens::FontBold(28))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 16.0f)
					[
						SNew(SBox)
						.WidthOverride(400.0f)
						.HeightOverride(120.0f)
						[
							SNew(SMultiLineEditableTextBox)
							.Font(FT66Style::Tokens::FontRegular(12))
							.OnTextChanged_Lambda([this](const FText& T) { BugReportText = T.ToString(); })
							.HintText(HintText)
						]
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
							FT66Style::MakeButton(SubmitText, FOnClicked::CreateUObject(this, &UT66ReportBugScreen::HandleSubmitClicked), ET66ButtonType::Success, 120.f)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.0f, 0.0f)
						[
							FT66Style::MakeButton(CancelText, FOnClicked::CreateUObject(this, &UT66ReportBugScreen::HandleCancelClicked), ET66ButtonType::Neutral, 120.f)
						]
					]
				]
			]
		];
}

FReply UT66ReportBugScreen::HandleSubmitClicked() { OnSubmitClicked(); return FReply::Handled(); }
FReply UT66ReportBugScreen::HandleCancelClicked() { OnCancelClicked(); return FReply::Handled(); }

void UT66ReportBugScreen::OnSubmitClicked()
{
	// Save a local bug report file (text-only) with lightweight context.
	const FString Timestamp = FDateTime::UtcNow().ToIso8601().Replace(TEXT(":"), TEXT("-"));
	const FString MapName = GetWorld() ? UWorld::RemovePIEPrefix(GetWorld()->GetMapName()) : FString(TEXT("UnknownMap"));

	int32 Hearts = -1, MaxHearts = -1, Gold = -1, Debt = -1, Stage = -1, Score = -1;
	float TimerSeconds = -1.f;
	bool bTimerActive = false;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			Hearts = RunState->GetCurrentHearts();
			MaxHearts = RunState->GetMaxHearts();
			Gold = RunState->GetCurrentGold();
			Debt = RunState->GetCurrentDebt();
			Stage = RunState->GetCurrentStage();
			Score = RunState->GetCurrentScore();
			bTimerActive = RunState->GetStageTimerActive();
			TimerSeconds = RunState->GetStageTimerSecondsRemaining();
		}
	}

	FString Report;
	Report += TEXT("TRIBULATION 66 — BUG REPORT\n");
	Report += FString::Printf(TEXT("UTC: %s\n"), *Timestamp);
	Report += FString::Printf(TEXT("Map: %s\n"), *MapName);
	Report += FString::Printf(TEXT("OS: %s\n"), *FPlatformMisc::GetOSVersion());
	Report += FString::Printf(TEXT("CPU: %s\n"), *FPlatformMisc::GetCPUBrand());
	Report += FString::Printf(TEXT("Project: %s\n"), FApp::GetProjectName());
	const FString BuildVersion = FApp::GetBuildVersion();
	Report += FString::Printf(TEXT("BuildVersion: %s\n"), *BuildVersion);
	Report += FString::Printf(TEXT("EngineVersion: %s\n"), *FEngineVersion::Current().ToString());
	Report += TEXT("Sentry: not configured (placeholder)\n");
	Report += TEXT("\n-- Run Context (best effort) --\n");
	Report += FString::Printf(TEXT("Stage: %d\n"), Stage);
	Report += FString::Printf(TEXT("Hearts: %d / %d\n"), Hearts, MaxHearts);
	Report += FString::Printf(TEXT("Gold: %d\n"), Gold);
	Report += FString::Printf(TEXT("Debt: %d\n"), Debt);
	Report += FString::Printf(TEXT("Score: %d\n"), Score);
	Report += FString::Printf(TEXT("StageTimerActive: %s\n"), bTimerActive ? TEXT("true") : TEXT("false"));
	Report += FString::Printf(TEXT("StageTimerSecondsRemaining: %.2f\n"), TimerSeconds);
	Report += TEXT("\n-- Player Text --\n");
	Report += BugReportText;
	Report += TEXT("\n");

	const FString Dir = FPaths::ProjectSavedDir() / TEXT("BugReports");
	IFileManager::Get().MakeDirectory(*Dir, true);
	const FString FilePath = Dir / FString::Printf(TEXT("BugReport_%s.txt"), *Timestamp);
	FFileHelper::SaveStringToFile(Report, *FilePath);

	UE_LOG(LogTemp, Log, TEXT("Report Bug saved: %s"), *FilePath);
	CloseModal();

	// In gameplay, Report Bug is opened from Pause Menu. Our UIManager is single-modal, so opening this
	// replaces Pause Menu. When closing, re-open Pause Menu so the player can resume/unpause.
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->IsGameplayLevel() && PC->IsPaused())
		{
			ShowModal(ET66ScreenType::PauseMenu);
		}
	}
}

void UT66ReportBugScreen::OnCancelClicked()
{
	CloseModal();

	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->IsGameplayLevel() && PC->IsPaused())
		{
			ShowModal(ET66ScreenType::PauseMenu);
		}
	}
}
