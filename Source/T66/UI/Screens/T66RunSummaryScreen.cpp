// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66PowerUpSubsystem.h"
#include "Core/T66SkillRatingSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "EngineUtils.h"
#include "Widgets/Images/SImage.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Styling/CoreStyle.h"
#include "HAL/PlatformProcess.h"

UT66RunSummaryScreen::UT66RunSummaryScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::RunSummary;
	bIsModal = true;
}

void UT66RunSummaryScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	// If we were opened from a leaderboard row click, load the saved snapshot first.
	const bool bWasViewingSaved = bViewingSavedLeaderboardRunSummary;
	const TObjectPtr<UT66LeaderboardRunSummarySaveGame> PrevSummary = LoadedSavedSummary;
	const bool bConsumedRequest = LoadSavedRunSummaryIfRequested();

	// If a new pending request was consumed (or viewer state changed), rebuild immediately.
	// NOTE: Snapshot loading also happens during RebuildWidget() so the first open should already be correct.
	if (bConsumedRequest || (bWasViewingSaved != bViewingSavedLeaderboardRunSummary) || (PrevSummary != LoadedSavedSummary))
	{
		ForceRebuildSlate();
	}

	// Default: banners are only relevant for a freshly-finished run (not for viewing saved leaderboard snapshots).
	bNewPersonalHighScore = false;
	bNewPersonalBestTime = false;

	// Foundation: submit score at run end if allowed by settings (Practice Mode blocks).
	// Important: do NOT submit when we're just viewing a saved run from the leaderboard.
	if (!bViewingSavedLeaderboardRunSummary)
	{
		UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
		UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
		if (RunState && LB)
		{
			// High Score submission: subsystem only updates if it's a new personal best (and Practice Mode blocks).
			LB->SubmitRunBounty(RunState->GetCurrentScore());
			bNewPersonalHighScore = LB->WasLastHighScoreNewPersonalBest();
		}

		// Speed Run banner: if any completed stage submission during this run set a new personal best time.
		if (RunState)
		{
			bNewPersonalBestTime = RunState->DidThisRunSetNewPersonalBestTime();
		}
	}

	EnsurePreviewCaptures();
}

void UT66RunSummaryScreen::OnScreenDeactivated_Implementation()
{
	DestroyPreviewCaptures();
	InventoryItemIconBrushes.Reset();
	IdolIconBrushes.Reset();
	Super::OnScreenDeactivated_Implementation();
}

namespace
{
	template <typename TStage>
	static TStage* FindStage(UWorld* World)
	{
		if (!World) return nullptr;
		for (TActorIterator<TStage> It(World); It; ++It)
		{
			return *It;
		}
		return nullptr;
	}
}

void UT66RunSummaryScreen::EnsurePreviewCaptures()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UT66GameInstance* GI = Cast<UT66GameInstance>(World->GetGameInstance());
	if (!GI) return;

	// Reuse the same preview stages used in the frontend screens (spawn them far away so players never see them).
	if (!HeroPreviewStage)
	{
		HeroPreviewStage = FindStage<AT66HeroPreviewStage>(World);
		if (!HeroPreviewStage)
		{
			FActorSpawnParameters P;
			P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			HeroPreviewStage = World->SpawnActor<AT66HeroPreviewStage>(AT66HeroPreviewStage::StaticClass(), FVector(1200000.f, 0.f, 200.f), FRotator::ZeroRotator, P);
		}
	}

	// Drive them from the saved selection so it matches the hero/companion selection UI.
	if (HeroPreviewStage)
	{
		const FName HeroID =
			(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->HeroID :
			(!GI->SelectedHeroID.IsNone() ? GI->SelectedHeroID :
				(GI->GetAllHeroIDs().Num() > 0 ? GI->GetAllHeroIDs()[0] : NAME_None));

		const ET66BodyType BodyType =
			(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->HeroBodyType : GI->SelectedHeroBodyType;
		FName SkinID = GI->SelectedHeroSkinID.IsNone() ? FName(TEXT("Default")) : GI->SelectedHeroSkinID;
		if (!HeroID.IsNone())
		{
			HeroPreviewStage->SetPreviewHero(HeroID, BodyType, SkinID);
		}

		// Capture hero preview to a render target for the Run Summary panel.
		if (!HeroPreviewRT)
		{
			HeroPreviewRT = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
			if (HeroPreviewRT)
			{
				HeroPreviewRT->RenderTargetFormat = RTF_RGBA8;
				HeroPreviewRT->SizeX = 512;
				HeroPreviewRT->SizeY = 512;
				HeroPreviewRT->UpdateResource();
			}
		}
		if (HeroPreviewRT && !HeroCaptureActor)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			HeroCaptureActor = World->SpawnActor<ASceneCapture2D>(HeroPreviewStage->GetIdealCameraLocation(), HeroPreviewStage->GetIdealCameraRotation(), SpawnParams);
			if (HeroCaptureActor)
			{
				USceneCaptureComponent2D* CaptureComp = HeroCaptureActor->GetCaptureComponent2D();
				if (CaptureComp)
				{
					CaptureComp->TextureTarget = HeroPreviewRT;
					CaptureComp->bCaptureEveryFrame = false;
					CaptureComp->CaptureSource = SCS_FinalColorLDR;
					CaptureComp->CaptureScene();
				}
			}
		}
		if (HeroCaptureActor && HeroPreviewRT)
		{
			USceneCaptureComponent2D* CaptureComp = HeroCaptureActor->GetCaptureComponent2D();
			if (CaptureComp && CaptureComp->TextureTarget == HeroPreviewRT)
			{
				HeroCaptureActor->SetActorLocation(HeroPreviewStage->GetIdealCameraLocation());
				HeroCaptureActor->SetActorRotation(HeroPreviewStage->GetIdealCameraRotation());
				CaptureComp->CaptureScene();
			}
		}
		if (HeroPreviewRT)
		{
			if (!HeroPreviewBrush.IsValid())
			{
				HeroPreviewBrush = MakeShared<FSlateBrush>();
				HeroPreviewBrush->DrawAs = ESlateBrushDrawType::Image;
				HeroPreviewBrush->ImageSize = FVector2D(420.f, 420.f);
			}
			HeroPreviewBrush->SetResourceObject(HeroPreviewRT);
		}
	}
}

void UT66RunSummaryScreen::DestroyPreviewCaptures()
{
	HeroPreviewBrush.Reset();
	if (HeroCaptureActor)
	{
		HeroCaptureActor->Destroy();
		HeroCaptureActor = nullptr;
	}
	HeroPreviewRT = nullptr;
}

bool UT66RunSummaryScreen::LoadSavedRunSummaryIfRequested()
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	if (!LB)
	{
		return false;
	}

	// Prefer in-memory fake snapshot (non-local leaderboard rows), then slot-based request.
	UT66LeaderboardRunSummarySaveGame* FakeSnap = LB->ConsumePendingFakeRunSummarySnapshot();
	if (FakeSnap)
	{
		bViewingSavedLeaderboardRunSummary = false;
		LoadedSavedSummary = nullptr;
		LoadedSavedSummarySlotName.Reset();
		bLogVisible = false;
		bReportPromptVisible = false;
		ReportReasonTextBox.Reset();
		ProofUrlTextBox.Reset();
		ProofOfRunUrl.Reset();
		bProofOfRunLocked = false;

		LoadedSavedSummary = FakeSnap;
		bViewingSavedLeaderboardRunSummary = true;
		if (LoadedSavedSummary->SchemaVersion >= 3)
		{
			ProofOfRunUrl = LoadedSavedSummary->ProofOfRunUrl;
			bProofOfRunLocked = LoadedSavedSummary->bProofOfRunLocked;
		}
		return true;
	}

	FString SlotName;
	if (!LB->ConsumePendingRunSummaryRequest(SlotName))
	{
		// No pending request: do not clobber any existing viewer state.
		return false;
	}

	// We are consuming a request: clear previous snapshot state now.
	bViewingSavedLeaderboardRunSummary = false;
	LoadedSavedSummary = nullptr;
	LoadedSavedSummarySlotName.Reset();
	bLogVisible = false;
	bReportPromptVisible = false;
	ReportReasonTextBox.Reset();
	ProofUrlTextBox.Reset();
	ProofOfRunUrl.Reset();
	bProofOfRunLocked = false;

	if (SlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		return true;
	}

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
	LoadedSavedSummary = Cast<UT66LeaderboardRunSummarySaveGame>(Loaded);
	if (LoadedSavedSummary)
	{
		bViewingSavedLeaderboardRunSummary = true;
		LoadedSavedSummarySlotName = SlotName;

		// Proof-of-run fields are schema-versioned. If not present, default to empty/unlocked.
		if (LoadedSavedSummary->SchemaVersion >= 3)
		{
			ProofOfRunUrl = LoadedSavedSummary->ProofOfRunUrl;
			bProofOfRunLocked = LoadedSavedSummary->bProofOfRunLocked;
		}
	}
	return true;
}

TSharedRef<SWidget> UT66RunSummaryScreen::RebuildWidget()
{
	// Critical: Slate is built before OnScreenActivated() (AddToViewport/TakeWidget).
	// Load any pending leaderboard snapshot here so the first render is correct (no "empty run" flash).
	LoadSavedRunSummaryIfRequested();
	return BuildSlateUI();
}

void UT66RunSummaryScreen::RebuildLogItems()
{
	LogItems.Reset();

	const TArray<FString>* LogPtr = nullptr;
	if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
	{
		LogPtr = &LoadedSavedSummary->EventLog;
	}
	else
	{
		UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		if (RunState)
		{
			LogPtr = &RunState->GetEventLog();
		}
	}

	const TArray<FString>& Log = LogPtr ? *LogPtr : TArray<FString>();

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
	const int32 StageReached =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->StageReached :
		(RunState ? RunState->GetCurrentStage() : 1);

	const int32 HighScore =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->Bounty :
		(RunState ? RunState->GetCurrentScore() : 0);

	const int32 HeroLevel =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->HeroLevel :
		(RunState ? RunState->GetHeroLevel() : 1);

	const int32 DamageStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->DamageStat :
		(RunState ? RunState->GetDamageStat() : 1);

	const int32 AttackSpeedStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->AttackSpeedStat :
		(RunState ? RunState->GetAttackSpeedStat() : 1);

	const int32 AttackScaleStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->AttackScaleStat :
		(RunState ? RunState->GetScaleStat() : 1);

	const int32 ArmorStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->ArmorStat :
		(RunState ? RunState->GetArmorStat() : 1);

	const int32 EvasionStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->EvasionStat :
		(RunState ? RunState->GetEvasionStat() : 1);

	const int32 LuckStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->LuckStat :
		(RunState ? RunState->GetLuckStat() : 1);

	const int32 SpeedStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->SpeedStat :
		(RunState ? RunState->GetSpeedStat() : 1);
	UT66LocalizationSubsystem* Loc = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66GameInstance* GI = GetWorld() ? Cast<UT66GameInstance>(GetWorld()->GetGameInstance()) : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	EnsurePreviewCaptures();
	InventoryItemIconBrushes.Reset();
	IdolIconBrushes.Reset();

	RebuildLogItems();
	SAssignNew(LogListView, SListView<TSharedPtr<FString>>)
		.ListItemsSource(&LogItems)
		.OnGenerateRow(SListView<TSharedPtr<FString>>::FOnGenerateRow::CreateUObject(this, &UT66RunSummaryScreen::GenerateLogRow))
		.SelectionMode(ESelectionMode::None);

	const FText TitleText = Loc ? Loc->GetText_RunSummaryTitle() : NSLOCTEXT("T66.RunSummary", "Title", "RUN SUMMARY");
	const FText StageHighScoreText = Loc
		? FText::Format(Loc->GetText_RunSummaryStageReachedBountyFormat(), FText::AsNumber(StageReached), FText::AsNumber(HighScore))
		: FText::FromString(FString::Printf(TEXT("Stage Reached: %d  |  Score: %d"), StageReached, HighScore));

	auto MakeSectionPanel = [](const FText& Header, const TSharedRef<SWidget>& Body) -> TSharedRef<SWidget>
	{
		return FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(STextBlock)
				.Text(Header)
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				Body
			],
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4)
		);
	};

	auto MakeHeroPreview = [](const TSharedPtr<FSlateBrush>& Brush) -> TSharedRef<SWidget>
	{
		return Brush.IsValid()
			? StaticCastSharedRef<SWidget>(FT66Style::MakePanel(
				SNew(SBox)
				.WidthOverride(420.f)
				.HeightOverride(420.f)
				[
					SNew(SImage).Image(Brush.Get())
				],
				FT66PanelParams(ET66PanelType::Panel).SetPadding(0.f)
			))
			: StaticCastSharedRef<SWidget>(FT66Style::MakePanel(
				SNew(SBox)
				.WidthOverride(420.f)
				.HeightOverride(420.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.RunSummary", "NoPreview", "No Preview"))
					.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
					.Justification(ETextJustify::Center)
				],
				FT66PanelParams(ET66PanelType::Panel).SetPadding(0.f)
			));
	};

	// Event log (hidden by default; opened via "EVENT LOG" button).
	TSharedRef<SWidget> EventLogPanel =
		MakeSectionPanel(
			NSLOCTEXT("T66.RunSummary", "EventLogTitle", "EVENT LOG"),
			FT66Style::MakePanel(
				LogListView.IsValid() ? StaticCastSharedRef<SWidget>(LogListView.ToSharedRef()) : StaticCastSharedRef<SWidget>(SNew(SSpacer)),
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space2)
			)
		);

	// Proof of run UI.
	// NOTE: Today this is local-only, so we treat the player as the owner of viewed runs.
	// When runs can be viewed cross-account, we'll gate edit/confirm by real ownership.
	const bool bIsOwnerOfViewedRun = true;

	TSharedRef<SWidget> ProofBody =
		SNew(SVerticalBox)
		// Link view (clickable) + Edit button
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				FT66Style::MakePanel(
					SNew(SHyperlink)
					.Text_Lambda([this]() { return FText::FromString(ProofOfRunUrl); })
					.OnNavigate(FSimpleDelegate::CreateUObject(this, &UT66RunSummaryScreen::HandleProofLinkNavigate)),
					FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(10.f, 8.f))
						.SetVisibility(TAttribute<EVisibility>::CreateLambda([this]() { return (bProofOfRunLocked && !ProofOfRunUrl.IsEmpty()) ? EVisibility::Visible : EVisibility::Collapsed; }))
				)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
			[
				FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66.RunSummary", "Edit", "EDIT"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleProofEditClicked))
					.SetPadding(FMargin(14.f, 8.f))
					.SetVisibility(TAttribute<EVisibility>::CreateLambda([this, bIsOwnerOfViewedRun]() { return (bIsOwnerOfViewedRun && bProofOfRunLocked) ? EVisibility::Visible : EVisibility::Collapsed; })))
			]
		]
		// Edit view (textbox) + Confirm button
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			.Visibility_Lambda([this, bIsOwnerOfViewedRun]() { return (bIsOwnerOfViewedRun && !bProofOfRunLocked) ? EVisibility::Visible : EVisibility::Collapsed; })
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SAssignNew(ProofUrlTextBox, SEditableTextBox)
				.Text(FText::FromString(ProofOfRunUrl))
				.OnTextChanged_Lambda([this](const FText& NewText)
				{
					// Keep the draft in sync with what the player typed.
					ProofOfRunUrl = NewText.ToString();
				})
				.HintText(NSLOCTEXT("T66.RunSummary", "ProofHint", "Paste YouTube link here..."))
				.MinDesiredWidth(420.f)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
			[
				FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66.RunSummary", "Confirm", "CONFIRM"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleProofConfirmClicked), ET66ButtonType::Success)
					.SetPadding(FMargin(14.f, 8.f)))
			]
		];

	TSharedRef<SWidget> ProofOfRunPanel =
		MakeSectionPanel(
			NSLOCTEXT("T66.RunSummary", "ProofOfRunHeader", "PROOF OF RUN"),
			ProofBody
		);

	// Cheat report UI (available to everyone).
	TSharedRef<SWidget> ReportCheatButton =
		FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66.RunSummary", "TheyreCheating", "THEY'RE CHEATING"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleReportCheatingClicked), ET66ButtonType::Danger)
			.SetPadding(FMargin(14.f, 10.f)));

	TSharedRef<SWidget> ReportPrompt =
		FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.RunSummary", "ReportReasonHeader", "REASON"))
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SAssignNew(ReportReasonTextBox, SMultiLineEditableTextBox)
				.HintText(NSLOCTEXT("T66.RunSummary", "ReportReasonHint", "Describe why you believe they're cheating..."))
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f)
				[
					FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66.RunSummary", "Submit", "SUBMIT"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleReportSubmitClicked), ET66ButtonType::Primary)
						.SetPadding(FMargin(14.f, 8.f)))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f)
				[
					FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66.RunSummary", "Close", "CLOSE"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleReportCloseClicked))
						.SetPadding(FMargin(14.f, 8.f)))
				]
			]
		,
		FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space3));

	auto MakeEventLogButton = [&](const FText& Text) -> TSharedRef<SWidget>
	{
		return FT66Style::MakeButton(FT66ButtonParams(Text, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleViewLogClicked))
			.SetMinWidth(160.f).SetPadding(FMargin(14.f, 8.f)));
	};

	// Level + stats (requested for Run Summary and saved leaderboard snapshots).
	TSharedRef<SVerticalBox> StatsBox = SNew(SVerticalBox);
	{
		const FText StatFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.Stats", "StatLineFormat", "{0}: {1}");

		auto AddStatLine = [&](const FText& Label, int32 Value)
		{
			StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(FText::Format(StatFmt, Label, FText::AsNumber(Value)))
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];
		};

		AddStatLine(Loc ? Loc->GetText_Level() : NSLOCTEXT("T66.Common", "Level", "LEVEL"), HeroLevel);
		AddStatLine(Loc ? Loc->GetText_Stat_Damage() : NSLOCTEXT("T66.Stats", "Damage", "Damage"), DamageStat);
		AddStatLine(Loc ? Loc->GetText_Stat_AttackSpeed() : NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed"), AttackSpeedStat);
		AddStatLine(Loc ? Loc->GetText_Stat_AttackScale() : NSLOCTEXT("T66.Stats", "AttackScale", "Attack Scale"), AttackScaleStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Armor() : NSLOCTEXT("T66.Stats", "Armor", "Armor"), ArmorStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Evasion() : NSLOCTEXT("T66.Stats", "Evasion", "Evasion"), EvasionStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Luck() : NSLOCTEXT("T66.Stats", "Luck", "Luck"), LuckStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Speed() : NSLOCTEXT("T66.Stats", "Speed", "Speed"), SpeedStat);
	}

	// Build idols + inventory lists.
	TSharedRef<SVerticalBox> IdolsBox = SNew(SVerticalBox);
	TSharedRef<SVerticalBox> InvBox = SNew(SVerticalBox);
	{
		const TArray<FName>* IdolsPtr = nullptr;
		TArray<FName> InventoryLocal; // GetInventory() returns by value; store locally.

		if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
		{
			IdolsPtr = &LoadedSavedSummary->EquippedIdols;
			InventoryLocal = LoadedSavedSummary->Inventory;
		}
		else if (RunState)
		{
			IdolsPtr = &RunState->GetEquippedIdols();
			InventoryLocal = RunState->GetInventory();
		}

		const TArray<FName> Empty;
		const TArray<FName>& Idols = IdolsPtr ? *IdolsPtr : Empty;
		const TArray<FName>& Inventory = InventoryLocal;

		for (int32 IdolSlotIndex = 0; IdolSlotIndex < UT66RunStateSubsystem::MaxEquippedIdolSlots; ++IdolSlotIndex)
		{
			const FName IdolID = (Idols.IsValidIndex(IdolSlotIndex)) ? Idols[IdolSlotIndex] : NAME_None;
			const FLinearColor IdolColor = !IdolID.IsNone() ? UT66RunStateSubsystem::GetIdolColor(IdolID) : FT66Style::Tokens::Stroke;
			FIdolData IdolData;
			const bool bHasIdolData = GI && !IdolID.IsNone() && GI->GetIdolData(IdolID, IdolData);
			TSharedPtr<FSlateBrush> IdolBrush;
			if (bHasIdolData && !IdolData.Icon.IsNull())
			{
				IdolBrush = MakeShared<FSlateBrush>();
				IdolBrush->DrawAs = ESlateBrushDrawType::Image;
				IdolBrush->ImageSize = FVector2D(18.f, 18.f);
				IdolIconBrushes.Add(IdolBrush);
				if (TexPool)
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, IdolData.Icon, this, IdolBrush, IdolID, /*bClearWhileLoading*/ true);
				}
			}

			IdolsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
			[
				FT66Style::MakePanel(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(SBox).WidthOverride(18.f).HeightOverride(18.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(IdolColor)
							.Padding(0.f)
							[
								IdolBrush.IsValid()
								? StaticCastSharedRef<SWidget>(SNew(SImage).Image(IdolBrush.Get()))
								: StaticCastSharedRef<SWidget>(SNew(SSpacer))
							]
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(10.f, 0.f)
					[
						SNew(STextBlock)
						.Text(IdolID.IsNone()
							? NSLOCTEXT("T66.Common", "Empty", "EMPTY")
							: FText::FromName(IdolID))
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					],
					FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(12.f, 10.f))
				)
			];
		}

		if (Inventory.Num() <= 0)
		{
			InvBox->AddSlot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.RunSummary", "NoItems", "No items."))
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
			];
		}
		else
		{
			for (const FName& ItemID : Inventory)
			{
				if (ItemID.IsNone()) continue;
				FItemData ItemData;
				const bool bHasItemData = GI && GI->GetItemData(ItemID, ItemData);
				TSharedPtr<FSlateBrush> ItemBrush;
				if (bHasItemData && !ItemData.Icon.IsNull())
				{
					ItemBrush = MakeShared<FSlateBrush>();
					ItemBrush->DrawAs = ESlateBrushDrawType::Image;
					ItemBrush->ImageSize = FVector2D(18.f, 18.f);
					InventoryItemIconBrushes.Add(ItemBrush);
					if (TexPool)
					{
						T66SlateTexture::BindSharedBrushAsync(TexPool, ItemData.Icon, this, ItemBrush, ItemID, /*bClearWhileLoading*/ true);
					}
				}

				InvBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
				[
					FT66Style::MakePanel(
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SBox).WidthOverride(18.f).HeightOverride(18.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FT66Style::Tokens::Stroke)
								.Padding(0.f)
								[
									ItemBrush.IsValid()
									? StaticCastSharedRef<SWidget>(SNew(SImage).Image(ItemBrush.Get()))
									: StaticCastSharedRef<SWidget>(SNew(SSpacer))
								]
							]
						]
						+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(10.f, 0.f)
						[
							SNew(STextBlock)
							.Text(Loc ? Loc->GetText_ItemDisplayName(ItemID) : FText::FromName(ItemID))
							.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						],
						FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(12.f, 10.f))
					)
				];
			}
		}
	}

	// Back button (when viewing saved run) — shown in overlay bottom-left; Restart + Main Menu stay in panel.
	TSharedRef<SWidget> BackButton =
		FT66Style::MakeButton(FT66ButtonParams(Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked))
			.SetMinWidth(120.f).SetPadding(FMargin(18.f, 10.f)));

	// Buttons row (Restart + Main Menu) when not viewing a saved run — kept inside the main panel.
	TSharedRef<SWidget> ButtonsRow =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
		[
			FT66Style::MakeButton(FT66ButtonParams(Loc ? Loc->GetText_Restart() : NSLOCTEXT("T66.RunSummary", "Restart", "RESTART"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked), ET66ButtonType::Success)
				.SetMinWidth(180.f).SetPadding(FMargin(18.f, 10.f)))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
		[
			FT66Style::MakeButton(FT66ButtonParams(Loc ? Loc->GetText_MainMenu() : NSLOCTEXT("T66.RunSummary", "MainMenu", "MAIN MENU"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleMainMenuClicked))
				.SetMinWidth(200.f).SetPadding(FMargin(18.f, 10.f)))
		];

	// Panels in the requested layout.
	TSharedRef<SWidget> BaseStatsPanel = MakeSectionPanel(
		Loc ? Loc->GetText_BaseStatsHeader() : NSLOCTEXT("T66.HeroSelection", "BaseStatsHeader", "Base Stats"),
		StatsBox
	);

	// Placeholder panels (wiring for calculation will come later).
	auto MakeBigCenteredValue = [](const FText& ValueText) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(ValueText)
				.Font(FT66Style::Tokens::FontBold(34))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			];
	};

	// Canonical names:
	// - Dodge Rating -> Skill Rating (computed from no-damage windows)
	// - Luck Rating (computed from recorded RNG outcomes)
	// - Probability of Cheating (wiring TBD)
	const int32 LuckRating0To100 =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
		? ((LoadedSavedSummary->SchemaVersion >= 2) ? LoadedSavedSummary->LuckRating0To100 : -1)
		: (RunState ? RunState->GetLuckRating0To100() : -1);

	UT66SkillRatingSubsystem* Skill = nullptr;
	if (!bViewingSavedLeaderboardRunSummary)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			Skill = GameInstance->GetSubsystem<UT66SkillRatingSubsystem>();
		}
	}
	const int32 SkillRating0To100 =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
		? ((LoadedSavedSummary->SchemaVersion >= 4) ? LoadedSavedSummary->SkillRating0To100 : -1)
		: (Skill ? Skill->GetSkillRating0To100() : -1);

	const FText LuckRatingText =
		(LuckRating0To100 >= 0)
		? FText::Format(NSLOCTEXT("T66.RunSummary", "RatingOutOf100Format", "{0} / 100"), FText::AsNumber(LuckRating0To100))
		: NSLOCTEXT("T66.RunSummary", "RatingNA", "N/A");

	const FText SkillRatingText =
		(SkillRating0To100 >= 0)
		? FText::Format(NSLOCTEXT("T66.RunSummary", "RatingOutOf100Format", "{0} / 100"), FText::AsNumber(SkillRating0To100))
		: NSLOCTEXT("T66.RunSummary", "RatingNA", "N/A");

	TSharedRef<SWidget> LuckRatingPanel = MakeSectionPanel(
		NSLOCTEXT("T66.RunSummary", "LuckRatingPanel", "LUCK RATING"),
		MakeBigCenteredValue(LuckRatingText)
	);
	TSharedRef<SWidget> SkillRatingPanel = MakeSectionPanel(
		NSLOCTEXT("T66.RunSummary", "SkillRatingPanel", "SKILL RATING"),
		MakeBigCenteredValue(SkillRatingText)
	);
	const FText CheatProbLine = FText::Format(
		NSLOCTEXT("T66.RunSummary", "CheatProbabilityLineFormat", "Probability of Cheating: {0}%"),
		FText::AsNumber(0)
	);
	TSharedRef<SWidget> CheatProbabilityPanel = MakeSectionPanel(
		NSLOCTEXT("T66.RunSummary", "IntegrityPanel", "INTEGRITY"),
		SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(CheatProbLine)
			.Font(FT66Style::Tokens::FontBold(18))
			.ColorAndOpacity(FT66Style::Tokens::Text)
			.Justification(ETextJustify::Center)
		]
	);

	// Power Crystals: earned this run + total balance (from PowerUpSubsystem).
	UT66PowerUpSubsystem* PowerUpSub = GI ? GI->GetSubsystem<UT66PowerUpSubsystem>() : nullptr;
	const int32 PowerCrystalsEarned = RunState ? RunState->GetPowerCrystalsEarnedThisRun() : 0;
	const int32 PowerCrystalsTotal = PowerUpSub ? PowerUpSub->GetPowerCrystalBalance() : 0;
	const FText PowerCrystalsEarnedLine = FText::Format(
		NSLOCTEXT("T66.RunSummary", "PowerCrystalsEarnedFormat", "Earned this run: {0}"),
		FText::AsNumber(PowerCrystalsEarned)
	);
	const FText PowerCrystalsTotalLine = FText::Format(
		NSLOCTEXT("T66.RunSummary", "PowerCrystalsTotalFormat", "Total: {0}"),
		FText::AsNumber(PowerCrystalsTotal)
	);
	TSharedRef<SWidget> PowerCrystalsPanel = MakeSectionPanel(
		NSLOCTEXT("T66.RunSummary", "PowerCrystalsPanel", "POWER CRYSTALS"),
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(PowerCrystalsEarnedLine)
			.Font(FT66Style::Tokens::FontRegular(14))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(PowerCrystalsTotalLine)
			.Font(FT66Style::Tokens::FontBold(14))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
	);

	// Damage by source: from DamageLogSubsystem (live) or snapshot (SchemaVersion >= 5).
	TArray<FDamageLogEntry> DamageBySourceSorted;
	if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary && LoadedSavedSummary->SchemaVersion >= 5)
	{
		for (const auto& Pair : LoadedSavedSummary->DamageBySource)
		{
			FDamageLogEntry E;
			E.SourceID = Pair.Key;
			E.TotalDamage = Pair.Value;
			DamageBySourceSorted.Add(E);
		}
		DamageBySourceSorted.Sort([](const FDamageLogEntry& A, const FDamageLogEntry& B) { return A.TotalDamage > B.TotalDamage; });
	}
	else
	{
		UT66DamageLogSubsystem* DamageLog = GI ? GI->GetSubsystem<UT66DamageLogSubsystem>() : nullptr;
		if (DamageLog)
		{
			DamageBySourceSorted = DamageLog->GetDamageBySourceSorted();
		}
	}
	auto GetDamageSourceDisplayName = [](FName SourceID) -> FText
	{
		if (SourceID == UT66DamageLogSubsystem::SourceID_AutoAttack) return NSLOCTEXT("T66.RunSummary", "DamageSource_AutoAttack", "Auto Attack");
		if (SourceID == UT66DamageLogSubsystem::SourceID_Ultimate) return NSLOCTEXT("T66.RunSummary", "DamageSource_Ultimate", "Ultimate");
		return FText::FromName(SourceID);
	};
	TSharedRef<SVerticalBox> DamageBySourceBox = SNew(SVerticalBox);
	for (const FDamageLogEntry& Entry : DamageBySourceSorted)
	{
		DamageBySourceBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(FText::Format(
				NSLOCTEXT("T66.RunSummary", "DamageBySourceLineFormat", "{0}: {1}"),
				GetDamageSourceDisplayName(Entry.SourceID),
				FText::AsNumber(Entry.TotalDamage)))
			.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		];
	}
	if (DamageBySourceSorted.Num() == 0)
	{
		DamageBySourceBox->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.RunSummary", "NoDamageSources", "No damage data."))
			.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		];
	}
	TSharedRef<SWidget> DamageBySourcePanel = MakeSectionPanel(
		NSLOCTEXT("T66.RunSummary", "DamageBySourcePanel", "DAMAGE BY SOURCE"),
		DamageBySourceBox
	);

	// Achievement Coins: stubs (0 earned, 10,000 total).
	const FText ACEarnedLine = FText::Format(
		NSLOCTEXT("T66.RunSummary", "ACEarnedFormat", "Earned this run: {0}"),
		FText::AsNumber(0)
	);
	const FText ACTotalLine = FText::Format(
		NSLOCTEXT("T66.RunSummary", "ACTotalFormat", "Total: {0}"),
		FText::AsNumber(10000)
	);
	TSharedRef<SWidget> ACPanel = MakeSectionPanel(
		NSLOCTEXT("T66.RunSummary", "AchievementCoinsPanel", "ACHIEVEMENT COINS"),
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(ACEarnedLine)
			.Font(FT66Style::Tokens::FontRegular(14))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(ACTotalLine)
			.Font(FT66Style::Tokens::FontBold(14))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
	);

	TSharedRef<SWidget> InventoryPanel = MakeSectionPanel(
		NSLOCTEXT("T66.RunSummary", "InventoryPanel", "INVENTORY"),
		SNew(SScrollBox) + SScrollBox::Slot()[InvBox]
	);

	TSharedRef<SWidget> IdolsPanel = MakeSectionPanel(
		NSLOCTEXT("T66.RunSummary", "IdolsPanel", "IDOLS"),
		SNew(SScrollBox) + SScrollBox::Slot()[IdolsBox]
	);

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::Tokens::Bg)
		[
			SNew(SOverlay)
			// Main full-screen panel
			+ SOverlay::Slot()
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					// Header row: title left, Event Log button right
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(TitleText)
							.Font(FT66Style::Tokens::FontBold(40))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							MakeEventLogButton(NSLOCTEXT("T66.RunSummary", "EventLogTitle", "EVENT LOG"))
						]
					]
					// Stage reached / high score line
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f)
					[
						SNew(STextBlock)
						.Text(StageHighScoreText)
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					]
					// Personal best banners (only for newly-finished runs)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, -10.f, 0.f, 6.f)
					[
						SNew(STextBlock)
						.Visibility_Lambda([this]() { return (bNewPersonalHighScore && !bViewingSavedLeaderboardRunSummary) ? EVisibility::Visible : EVisibility::Collapsed; })
						.Text_Lambda([this, Loc]()
						{
							return Loc ? Loc->GetText_NewPersonalHighScore() : NSLOCTEXT("T66.RunSummary", "NewPersonalHighScore", "New Personal Score");
						})
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FT66Style::Tokens::Success)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f)
					[
						SNew(STextBlock)
						.Visibility_Lambda([this]() { return (bNewPersonalBestTime && !bViewingSavedLeaderboardRunSummary) ? EVisibility::Visible : EVisibility::Collapsed; })
						.Text_Lambda([this, Loc]()
						{
							return Loc ? Loc->GetText_NewPersonalBestTime() : NSLOCTEXT("T66.RunSummary", "NewPersonalBestTime", "New Personal Best Time");
						})
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FT66Style::Tokens::Success)
					]
					// Main content: Hero preview top-left, stats to the right, items to the right of stats
					+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 0.f, 0.f, 18.f)
					[
						SNew(SVerticalBox)
						// Top half: Hero preview + stats + ratings
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f)
						[
							SNew(SHorizontalBox)
							// Left: Hero preview
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 18.f, 0.f)
							[
								MakeHeroPreview(HeroPreviewBrush)
							]
							// Middle: Base stats (fixed width so ratings sit immediately next to it)
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 18.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(420.f)
								[
									BaseStatsPanel
								]
							]
							// Right: Ratings (kept tight next to Base Stats)
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
								[
									LuckRatingPanel
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
								[
									SkillRatingPanel
								]
								+ SVerticalBox::Slot().FillHeight(1.f)
								[
									CheatProbabilityPanel
								]
							]
							// Power Crystals + Damage by source + Achievement Coins (right column)
							+ SHorizontalBox::Slot().AutoWidth().Padding(18.f, 0.f, 0.f, 0.f)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
								[
									PowerCrystalsPanel
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
								[
									DamageBySourcePanel
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
								[
									ACPanel
								]
								+ SVerticalBox::Slot().FillHeight(1.f)
								[
									SNew(SSpacer)
								]
							]
							// Spacer so the block stays left (leaves room for right-side log drawer).
							+ SHorizontalBox::Slot().FillWidth(1.f)
							[
								SNew(SSpacer)
							]
						]
						// Bottom half: Idols + Inventory (short + tight side-by-side, leaving room below)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 18.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(520.f)
								.HeightOverride(250.f)
								[
									IdolsPanel
								]
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SBox)
								.WidthOverride(620.f)
								.HeightOverride(250.f)
								[
									InventoryPanel
								]
							]
							+ SHorizontalBox::Slot().FillWidth(1.f)
							[
								SNew(SSpacer)
							]
						]
					]
					// Buttons row (Restart + Main Menu) — only when not viewing a saved run; Back is in overlay bottom-left
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
					[
						SNew(SBox)
						.Visibility_Lambda([this]() { return bViewingSavedLeaderboardRunSummary ? EVisibility::Collapsed : EVisibility::Visible; })
						[
							ButtonsRow
						]
					],
					FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6)
				)
			]
			// Back button (bottom-left) — when viewing a saved leaderboard run
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(20.f, 0.f, 0.f, 20.f)
			[
				SNew(SBox)
				.Visibility_Lambda([this]() { return bViewingSavedLeaderboardRunSummary ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					BackButton
				]
			]
			// Right-side drawer: event log only (toggle with button).
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.Padding(0.f, 70.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(520.f)
				.HeightOverride(620.f)
				.Visibility_Lambda([this]() { return bLogVisible ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					EventLogPanel
				]
			]
			// Right-side: proof + report block (compact, shown under Event Log while open).
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.Padding(0.f, 70.f + 620.f + 12.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(520.f)
				// Always present in this right-side space (not tied to the Event Log toggle).
				.Visibility_Lambda([this]() { return bViewingSavedLeaderboardRunSummary ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						ProofOfRunPanel
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
					[
						SNew(SBox)
						.HeightOverride(44.f)
						[
							ReportCheatButton
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
					[
						SNew(SBox)
						.HeightOverride(150.f)
						[
							ReportPrompt
						]
					]
				]
			]
		];
}

FReply UT66RunSummaryScreen::HandleRestartClicked() { OnRestartClicked(); return FReply::Handled(); }
FReply UT66RunSummaryScreen::HandleMainMenuClicked() { OnMainMenuClicked(); return FReply::Handled(); }
FReply UT66RunSummaryScreen::HandleViewLogClicked() { OnViewLogClicked(); return FReply::Handled(); }
FReply UT66RunSummaryScreen::HandleProofConfirmClicked()
{
	const FString Url = ProofUrlTextBox.IsValid() ? ProofUrlTextBox->GetText().ToString() : ProofOfRunUrl;
	ProofOfRunUrl = Url;
	ProofOfRunUrl.TrimStartAndEndInline();
	bProofOfRunLocked = !ProofOfRunUrl.IsEmpty();

	// Avoid the "textbox disappears and nothing shows" state.
	if (!bProofOfRunLocked && ProofUrlTextBox.IsValid())
	{
		ProofUrlTextBox->SetText(FText::FromString(ProofOfRunUrl));
	}

	// Persist into the snapshot immediately (only when we're viewing a saved run summary).
	if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary && !LoadedSavedSummarySlotName.IsEmpty())
	{
		LoadedSavedSummary->SchemaVersion = FMath::Max(LoadedSavedSummary->SchemaVersion, 3);
		LoadedSavedSummary->ProofOfRunUrl = ProofOfRunUrl;
		LoadedSavedSummary->bProofOfRunLocked = bProofOfRunLocked;
		UGameplayStatics::SaveGameToSlot(LoadedSavedSummary, LoadedSavedSummarySlotName, 0);
	}

	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

FReply UT66RunSummaryScreen::HandleProofEditClicked()
{
	bProofOfRunLocked = false;
	if (ProofUrlTextBox.IsValid())
	{
		ProofUrlTextBox->SetText(FText::FromString(ProofOfRunUrl));
	}
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

void UT66RunSummaryScreen::HandleProofLinkNavigate() const
{
	if (ProofOfRunUrl.IsEmpty()) return;
	FPlatformProcess::LaunchURL(*ProofOfRunUrl, nullptr, nullptr);
}

FReply UT66RunSummaryScreen::HandleReportCheatingClicked()
{
	bReportPromptVisible = true;
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

FReply UT66RunSummaryScreen::HandleReportSubmitClicked()
{
	const FString Reason = ReportReasonTextBox.IsValid() ? ReportReasonTextBox->GetText().ToString() : FString();
	const int32 StageReached = (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->StageReached : 0;
	const int32 HighScore = (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->Bounty : 0;
	const FName HeroID = (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->HeroID : NAME_None;

	UE_LOG(LogTemp, Warning, TEXT("[CHEAT REPORT] Stage=%d HighScore=%d Hero=%s Reason=%s"),
		StageReached,
		HighScore,
		*HeroID.ToString(),
		*Reason);

	bReportPromptVisible = false;
	if (ReportReasonTextBox.IsValid())
	{
		ReportReasonTextBox->SetText(FText::GetEmpty());
	}
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

FReply UT66RunSummaryScreen::HandleReportCloseClicked()
{
	bReportPromptVisible = false;
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

void UT66RunSummaryScreen::OnRestartClicked()
{
	DestroyPreviewCaptures();
	if (bViewingSavedLeaderboardRunSummary)
	{
		// Viewer-mode (opened from leaderboard): just close the modal.
		if (UIManager)
		{
			// Single-modal UI: if a caller requested returning to a modal after viewing, honor it.
			if (UGameInstance* GI = GetGameInstance())
			{
				if (UT66LeaderboardSubsystem* LB = GI->GetSubsystem<UT66LeaderboardSubsystem>())
				{
					const ET66ScreenType ReturnModal = LB->ConsumePendingReturnModalAfterViewerRunSummary();
					if (ReturnModal != ET66ScreenType::None)
					{
						UIManager->ShowModal(ReturnModal);
						return;
					}
				}
			}
			UIManager->CloseModal();
			return;
		}
		UGameplayStatics::OpenLevel(this, FName(TEXT("FrontendLevel")));
		return;
	}
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState) RunState->ResetForNewRun();
	if (UT66DamageLogSubsystem* DamageLog = GI ? GI->GetSubsystem<UT66DamageLogSubsystem>() : nullptr)
	{
		DamageLog->ResetForNewRun();
	}
	APlayerController* PC = GetOwningPlayer();
	if (PC) PC->SetPause(false);
	UGameplayStatics::OpenLevel(this, FName(TEXT("GameplayLevel")));
}

void UT66RunSummaryScreen::OnMainMenuClicked()
{
	DestroyPreviewCaptures();
	if (bViewingSavedLeaderboardRunSummary)
	{
		// Viewer-mode (opened from leaderboard): the main menu is already underneath.
		if (UIManager)
		{
			// Single-modal UI: if a caller requested returning to a modal after viewing, honor it.
			if (UGameInstance* GI = GetGameInstance())
			{
				if (UT66LeaderboardSubsystem* LB = GI->GetSubsystem<UT66LeaderboardSubsystem>())
				{
					const ET66ScreenType ReturnModal = LB->ConsumePendingReturnModalAfterViewerRunSummary();
					if (ReturnModal != ET66ScreenType::None)
					{
						UIManager->ShowModal(ReturnModal);
						return;
					}
				}
			}
			UIManager->CloseModal();
			return;
		}
		UGameplayStatics::OpenLevel(this, FName(TEXT("FrontendLevel")));
		return;
	}
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState) RunState->ResetForNewRun();
	if (UT66DamageLogSubsystem* DamageLog = GI ? GI->GetSubsystem<UT66DamageLogSubsystem>() : nullptr)
	{
		DamageLog->ResetForNewRun();
	}
	APlayerController* PC = GetOwningPlayer();
	if (PC) PC->SetPause(false);
	// Use full map path for robustness in packaged builds.
	UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/FrontendLevel")));
}

void UT66RunSummaryScreen::OnViewLogClicked()
{
	bLogVisible = !bLogVisible;
	if (bLogVisible)
	{
		RebuildLogItems();
		if (LogListView.IsValid())
		{
			LogListView->RequestListRefresh();
		}
	}
	InvalidateLayoutAndVolatility();
}
