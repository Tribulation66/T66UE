// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66ReportBugScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Gameplay/T66PlayerController.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
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
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/SOverlay.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66ReportBug, Log, All);

namespace
{
	constexpr float ReportBugButtonHeight = 52.f;

	struct FReportBugReferenceButtonBrushSet
	{
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Normal;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Hovered;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Pressed;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Disabled;
	};

	struct FReportBugReferenceButtonStyleEntry
	{
		FButtonStyle Style;
		bool bInitialized = false;
	};

	const FSlateBrush* ResolveReportBugReferenceBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const FString& RelativePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel);
	}

	const TCHAR* GetReportBugButtonPrefix(ET66ButtonType Type)
	{
		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return TEXT("button_success");
		case ET66ButtonType::Danger:
			return TEXT("button_danger");
		default:
			return TEXT("button_neutral");
		}
	}

	FReportBugReferenceButtonBrushSet& GetReportBugButtonBrushSet(ET66ButtonType Type)
	{
		static FReportBugReferenceButtonBrushSet Neutral;
		static FReportBugReferenceButtonBrushSet Success;
		static FReportBugReferenceButtonBrushSet Danger;

		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return Success;
		case ET66ButtonType::Danger:
			return Danger;
		default:
			return Neutral;
		}
	}

	FReportBugReferenceButtonStyleEntry& GetReportBugButtonStyleEntry(ET66ButtonType Type)
	{
		static FReportBugReferenceButtonStyleEntry Neutral;
		static FReportBugReferenceButtonStyleEntry Success;
		static FReportBugReferenceButtonStyleEntry Danger;

		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return Success;
		case ET66ButtonType::Danger:
			return Danger;
		default:
			return Neutral;
		}
	}

	const FSlateBrush* ResolveReportBugButtonBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const TCHAR* Prefix,
		const TCHAR* State,
		const TCHAR* DebugLabel)
	{
		return ResolveReportBugReferenceBrush(
			Entry,
			FString::Printf(TEXT("SourceAssets/UI/Worker2Reference/SheetSlices/Common/%s_%s.png"), Prefix, State),
			FMargin(0.16f, 0.28f, 0.16f, 0.28f),
			DebugLabel);
	}

	const FButtonStyle& GetReportBugButtonStyle(ET66ButtonType Type)
	{
		FReportBugReferenceButtonStyleEntry& StyleEntry = GetReportBugButtonStyleEntry(Type);
		if (!StyleEntry.bInitialized)
		{
			StyleEntry.bInitialized = true;
			StyleEntry.Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			StyleEntry.Style.SetNormalPadding(FMargin(0.f));
			StyleEntry.Style.SetPressedPadding(FMargin(0.f));

			FReportBugReferenceButtonBrushSet& BrushSet = GetReportBugButtonBrushSet(Type);
			const TCHAR* Prefix = GetReportBugButtonPrefix(Type);
			if (const FSlateBrush* Brush = ResolveReportBugButtonBrush(BrushSet.Normal, Prefix, TEXT("normal"), TEXT("ReportBugButtonNormal")))
			{
				StyleEntry.Style.SetNormal(*Brush);
			}
			if (const FSlateBrush* Brush = ResolveReportBugButtonBrush(BrushSet.Hovered, Prefix, TEXT("hover"), TEXT("ReportBugButtonHover")))
			{
				StyleEntry.Style.SetHovered(*Brush);
			}
			if (const FSlateBrush* Brush = ResolveReportBugButtonBrush(BrushSet.Pressed, Prefix, TEXT("pressed"), TEXT("ReportBugButtonPressed")))
			{
				StyleEntry.Style.SetPressed(*Brush);
			}
			if (const FSlateBrush* Brush = ResolveReportBugButtonBrush(BrushSet.Disabled, TEXT("button_neutral"), TEXT("disabled"), TEXT("ReportBugButtonDisabled")))
			{
				StyleEntry.Style.SetDisabled(*Brush);
			}
		}

		return StyleEntry.Style;
	}

	const FSlateBrush* GetReportBugShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveReportBugReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/Worker2Reference/SheetSlices/Common/panel_modal.png"),
			FMargin(0.035f, 0.12f, 0.035f, 0.12f),
			TEXT("ReportBugShell"));
	}

	const FSlateBrush* GetReportBugFieldBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveReportBugReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/Worker2Reference/SheetSlices/Common/dropdown_field.png"),
			FMargin(0.06f, 0.34f, 0.06f, 0.34f),
			TEXT("ReportBugField"));
	}

	TSharedRef<SWidget> MakeReportBugShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		if (const FSlateBrush* ShellBrush = GetReportBugShellBrush())
		{
			return SNew(SBorder)
				.BorderImage(ShellBrush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Panel())
			.Padding(Padding)
			[
				Content
			];
	}

	TSharedRef<SWidget> MakeReportBugField(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		if (const FSlateBrush* FieldBrush = GetReportBugFieldBrush())
		{
			return SNew(SBorder)
				.BorderImage(FieldBrush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return SNew(SBorder)
			.BorderBackgroundColor(FT66Style::PanelInner())
			.Padding(Padding)
			[
				Content
			];
	}

	TSharedRef<SWidget> MakeReportBugButton(const FT66ButtonParams& Params)
	{
		const int32 FontSize = Params.FontSize > 0 ? Params.FontSize : 18;
		const TAttribute<FText> ButtonText = Params.DynamicLabel.IsBound()
			? Params.DynamicLabel
			: TAttribute<FText>(Params.Label);
		const TAttribute<FSlateColor> TextColor = Params.bHasTextColorOverride
			? Params.TextColorOverride
			: TAttribute<FSlateColor>(FSlateColor(FT66Style::Tokens::Text));
		const FMargin ContentPadding = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(20.f, 9.f);

		FSlateFontInfo ButtonFont = FT66Style::MakeFont(*Params.FontWeight, FontSize);
		ButtonFont.LetterSpacing = 0;

		const TSharedRef<SWidget> Content = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: StaticCastSharedRef<SWidget>(
				SNew(STextBlock)
				.Text(ButtonText)
				.Font(ButtonFont)
				.ColorAndOpacity(TextColor)
				.Justification(ETextJustify::Center)
				.ShadowOffset(FVector2D(0.f, 1.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.72f))
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis));

		return SNew(SBox)
			.MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
			.HeightOverride(Params.Height > 0.f ? Params.Height : ReportBugButtonHeight)
			.Visibility(Params.Visibility)
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(Params.OnClicked, Content)
					.SetButtonStyle(&GetReportBugButtonStyle(Params.Type))
					.SetPadding(ContentPadding)
					.SetHAlign(HAlign_Center)
					.SetVAlign(VAlign_Center)
					.SetEnabled(Params.IsEnabled))
			];
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

	return SNew(SBorder)
		.BorderBackgroundColor(FT66Style::Scrim())
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MakeReportBugShell(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 16.0f)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FT66Style::Tokens::FontBold(28))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 16.0f)
					[
							SNew(SBox)
							.WidthOverride(400.0f)
							.HeightOverride(120.0f)
							[
								MakeReportBugField(
									SNew(SMultiLineEditableTextBox)
									.Font(FT66Style::Tokens::FontRegular(12))
									.Text(FText::FromString(BugReportText))
									.OnTextChanged_Lambda([this](const FText& T) { BugReportText = T.ToString(); })
									.ForegroundColor(FT66Style::Tokens::Text)
									.HintText(HintText),
									FMargin(12.f, 10.f))
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
							MakeReportBugButton(
								FT66ButtonParams(SubmitText, FOnClicked::CreateUObject(this, &UT66ReportBugScreen::HandleSubmitClicked), ET66ButtonType::Success)
								.SetMinWidth(140.f)
								.SetHeight(ReportBugButtonHeight)
								.SetEnabled(CanSubmitReport))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.0f, 0.0f)
						[
							MakeReportBugButton(
								FT66ButtonParams(CancelText, FOnClicked::CreateUObject(this, &UT66ReportBugScreen::HandleCancelClicked), ET66ButtonType::Neutral)
								.SetMinWidth(140.f)
								.SetHeight(ReportBugButtonHeight))
						]
					]
				,
				FMargin(40.0f, 30.0f))
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

