// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66ReportBugScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66FlatStyle.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Gameplay/T66PlayerController.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/EngineVersion.h"
#include "Misc/App.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66ReportBug, Log, All);

namespace
{
	FName ReportBugTag(const TCHAR* Name)
	{
		return FName(Name);
	}

	const FEditableTextBoxStyle& GetReportBugTextBoxStyle()
	{
		static FEditableTextBoxStyle Style;
		static bool bInitialized = false;
		if (!bInitialized)
		{
			bInitialized = true;
			Style = FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox");
			const FSlateBrush* NoBrush = FCoreStyle::Get().GetBrush("NoBrush");
			Style.SetBackgroundImageNormal(*NoBrush);
			Style.SetBackgroundImageHovered(*NoBrush);
			Style.SetBackgroundImageFocused(*NoBrush);
			Style.SetBackgroundImageReadOnly(*NoBrush);
			Style.SetPadding(FMargin(0.0f));
		}
		return Style;
	}

	TSharedRef<SWidget> MakeReportBugScrim(const FName Tag)
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.56f))
			.Padding(0.f),
			Tag,
			TEXT("Scrim"),
			ET66FlatState::Default);
	}

	TSharedRef<SWidget> MakeReportBugButton(
		const FText& Text,
		FOnClicked OnClicked,
		TAttribute<bool> IsEnabled,
		const FName Tag)
	{
		return FT66FlatStyle::MakeFlatButton(
			ET66FlatState::Default,
			Text,
			MoveTemp(OnClicked),
			nullptr,
			nullptr,
			FMargin(20.f, 10.f),
			403.f,
			109.f,
			MoveTemp(IsEnabled),
			26,
			Tag);
	}
}

UT66ReportBugScreen::UT66ReportBugScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::ReportBug;
	bIsModal = true;
}

void UT66ReportBugScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	BugReportText.Reset();
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
	const TAttribute<bool> CanSubmitReport = TAttribute<bool>::CreateLambda([this]()
	{
		FString TrimmedText = BugReportText;
		TrimmedText.TrimStartAndEndInline();
		return !TrimmedText.IsEmpty();
	});

	const TSharedRef<SWidget> TextInput =
		FT66FlatStyle::AttachMetadata(
			SNew(SMultiLineEditableTextBox)
			.Style(&GetReportBugTextBoxStyle())
			.Font(FT66FlatStyle::MakeFont(22))
			.Text(FText::FromString(BugReportText))
			.OnTextChanged_Lambda([this](const FText& T) { BugReportText = T.ToString(); })
			.ForegroundColor(FSlateColor(FT66FlatStyle::PrimaryText()))
			.HintText(HintText),
			ReportBugTag(TEXT("ReportBug.TextInput")),
			TEXT("MultilineTextInput"),
			ET66FlatState::Default);

	const TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	auto AddSlot = [Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X, Y, W, H))
			[
				Widget
			];
	};

	AddSlot(0.f, 0.f, 1920.f, 1080.f, MakeReportBugScrim(ReportBugTag(TEXT("ReportBug.Scrim"))));
	AddSlot(452.f, 145.f, 1015.f, 749.f,
		FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(0.f),
			SNullWidget::NullWidget,
			nullptr,
			ReportBugTag(TEXT("ReportBug.ModalPanel"))));
	AddSlot(799.f, 217.f, 322.f, 87.f,
		FT66FlatStyle::MakeFlatLabel(
			TitleText,
			ET66FlatLabelRole::Title,
			ETextJustify::Center,
			ReportBugTag(TEXT("ReportBug.Title"))));
	AddSlot(532.f, 304.f, 857.f, 400.f,
		FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(43.f, 35.f),
			TextInput,
			nullptr,
			ReportBugTag(TEXT("ReportBug.TextFieldPanel"))));
	AddSlot(546.f, 704.f, 403.f, 109.f,
		MakeReportBugButton(
			SubmitText,
			FOnClicked::CreateUObject(this, &UT66ReportBugScreen::HandleSubmitClicked),
			CanSubmitReport,
			ReportBugTag(TEXT("ReportBug.SubmitButton"))));
	AddSlot(978.f, 704.f, 403.f, 109.f,
		MakeReportBugButton(
			CancelText,
			FOnClicked::CreateUObject(this, &UT66ReportBugScreen::HandleCancelClicked),
			true,
			ReportBugTag(TEXT("ReportBug.CancelButton"))));

	const TSharedRef<SWidget> RootContent = SNew(SBox)
		.WidthOverride(1920.f)
		.HeightOverride(1080.f)
		[
			Canvas
		];

	return FT66FlatStyle::AttachMetadata(
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				RootContent
			]
		],
		ReportBugTag(TEXT("ReportBug.Root")),
		TEXT("Root"),
		ET66FlatState::Default);
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
	FString DifficultyKey;
	FString PartyKey;
	FString HeroIdKey;
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

		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
		{
			switch (T66GI->SelectedDifficulty)
			{
			case ET66Difficulty::Medium: DifficultyKey = TEXT("medium"); break;
			case ET66Difficulty::Hard: DifficultyKey = TEXT("hard"); break;
			case ET66Difficulty::VeryHard: DifficultyKey = TEXT("veryhard"); break;
			case ET66Difficulty::Impossible: DifficultyKey = TEXT("impossible"); break;
			case ET66Difficulty::Easy:
			default: DifficultyKey = TEXT("easy"); break;
			}

			switch (T66GI->SelectedPartySize)
			{
			case ET66PartySize::Duo: PartyKey = TEXT("duo"); break;
			case ET66PartySize::Trio: PartyKey = TEXT("trio"); break;
			case ET66PartySize::Quad: PartyKey = TEXT("quad"); break;
			case ET66PartySize::Solo:
			default: PartyKey = TEXT("solo"); break;
			}

			HeroIdKey = T66GI->SelectedHeroID.ToString();
		}
	}

	FString Report;
	Report += TEXT("TRIBULATION 66 — BUG REPORT\n");
	Report += FString::Printf(TEXT("UTC: %s\n"), *Timestamp);
	Report += FString::Printf(TEXT("Map: %s\n"), *MapName);
	Report += FString::Printf(TEXT("OS: %s\n"), *FPlatformMisc::GetOSVersion());
	Report += FString::Printf(TEXT("CPU: %s\n"), *FPlatformMisc::GetCPUBrand());
	Report += FString::Printf(TEXT("Project: %s\n"), FApp::GetProjectName());
	FString BuildVersion = FApp::GetBuildVersion();
	if (BuildVersion.IsEmpty())
	{
		GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), BuildVersion, GGameIni);
	}
	Report += FString::Printf(TEXT("BuildVersion: %s\n"), *BuildVersion);
	Report += FString::Printf(TEXT("EngineVersion: %s\n"), *FEngineVersion::Current().ToString());
	Report += TEXT("Telemetry: backend submission + local backup\n");
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

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			if (Backend->IsBackendConfigured() && Backend->HasSteamTicket())
			{
				Backend->SubmitBugReport(BugReportText, Stage, DifficultyKey, PartyKey, HeroIdKey);
			}
		}
	}

	UE_LOG(LogT66ReportBug, Log, TEXT("Report Bug saved: %s"), *FilePath);
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

