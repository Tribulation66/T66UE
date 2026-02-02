// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66SkillRatingSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
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
		if (!HeroID.IsNone())
		{
			HeroPreviewStage->SetPreviewHero(HeroID, BodyType);
		}
		if (UTextureRenderTarget2D* RT = HeroPreviewStage->GetRenderTarget())
		{
			if (!HeroPreviewBrush.IsValid())
			{
				HeroPreviewBrush = MakeShared<FSlateBrush>();
				HeroPreviewBrush->DrawAs = ESlateBrushDrawType::Image;
			}
			HeroPreviewBrush->SetResourceObject(RT);
			HeroPreviewBrush->ImageSize = FVector2D(640.f, 640.f);
		}
	}
}

void UT66RunSummaryScreen::DestroyPreviewCaptures()
{
	// Keep preview stages around; they are harmless and reused by other screens if present.
	HeroPreviewBrush.Reset();
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

	const int32 AttackSizeStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->AttackSizeStat :
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
		: FText::FromString(FString::Printf(TEXT("Stage Reached: %d  |  High Score: %d"), StageReached, HighScore));

	auto MakePanel = [](const FText& Header, const TSharedRef<SWidget>& Body) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
			.Padding(FMargin(FT66Style::Tokens::Space4))
			[
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
				]
			];
	};

	auto MakeHeroPreview = [](const TSharedPtr<FSlateBrush>& Brush) -> TSharedRef<SWidget>
	{
		return Brush.IsValid()
			? StaticCastSharedRef<SWidget>(SNew(SBorder)
				.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
				.Padding(0.f)
				[
					SNew(SBox)
					.WidthOverride(420.f)
					.HeightOverride(420.f)
					[
						SNew(SImage).Image(Brush.Get())
					]
				])
			: StaticCastSharedRef<SWidget>(SNew(SBorder)
				.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
				.Padding(0.f)
				[
					SNew(SBox)
					.WidthOverride(420.f)
					.HeightOverride(420.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.RunSummary", "NoPreview", "No Preview"))
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
						.Justification(ETextJustify::Center)
					]
				]);
	};

	// Event log (hidden by default; opened via "EVENT LOG" button).
	TSharedRef<SWidget> EventLogPanel =
		MakePanel(
			NSLOCTEXT("T66.RunSummary", "EventLogTitle", "EVENT LOG"),
			SNew(SBorder)
			.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
			.Padding(FMargin(FT66Style::Tokens::Space2))
			[
				LogListView.IsValid() ? StaticCastSharedRef<SWidget>(LogListView.ToSharedRef()) : StaticCastSharedRef<SWidget>(SNew(SSpacer))
			]);

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
				SNew(SBorder)
				.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
				.Padding(FMargin(10.f, 8.f))
				.Visibility_Lambda([this]() { return (bProofOfRunLocked && !ProofOfRunUrl.IsEmpty()) ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					SNew(SHyperlink)
					.Text_Lambda([this]() { return FText::FromString(ProofOfRunUrl); })
					.OnNavigate(FSimpleDelegate::CreateUObject(this, &UT66RunSummaryScreen::HandleProofLinkNavigate))
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Visibility_Lambda([this, bIsOwnerOfViewedRun]() { return (bIsOwnerOfViewedRun && bProofOfRunLocked) ? EVisibility::Visible : EVisibility::Collapsed; })
				.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleProofEditClicked))
				.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
				.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
				.ContentPadding(FMargin(14.f, 8.f))
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.RunSummary", "Edit", "EDIT"))
					.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
				]
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
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleProofConfirmClicked))
				.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary"))
				.ButtonColorAndOpacity(FT66Style::Tokens::Success)
				.ContentPadding(FMargin(14.f, 8.f))
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.RunSummary", "Confirm", "CONFIRM"))
					.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
				]
			]
		];

	TSharedRef<SWidget> ProofOfRunPanel =
		MakePanel(
			NSLOCTEXT("T66.RunSummary", "ProofOfRunHeader", "PROOF OF RUN"),
			ProofBody
		);

	// Cheat report UI (available to everyone).
	TSharedRef<SWidget> ReportCheatButton =
		SNew(SButton)
		.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleReportCheatingClicked))
		.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary"))
		.ButtonColorAndOpacity(FLinearColor(0.75f, 0.20f, 0.20f, 1.f))
		.ContentPadding(FMargin(14.f, 10.f))
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.RunSummary", "TheyreCheating", "THEY'RE CHEATING"))
			.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
		];

	TSharedRef<SWidget> ReportPrompt =
		SNew(SBorder)
		.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
		.Padding(FMargin(FT66Style::Tokens::Space3))
		.Visibility_Lambda([this]() { return bReportPromptVisible ? EVisibility::Visible : EVisibility::Collapsed; })
		[
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
					SNew(SButton)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleReportSubmitClicked))
					.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary"))
					.ButtonColorAndOpacity(FT66Style::Tokens::Accent2)
					.ContentPadding(FMargin(14.f, 8.f))
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.RunSummary", "Submit", "SUBMIT"))
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f)
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleReportCloseClicked))
					.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
					.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
					.ContentPadding(FMargin(14.f, 8.f))
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.RunSummary", "Close", "CLOSE"))
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
					]
				]
			]
		];

	auto MakeEventLogButton = [&](const FText& Text) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.MinDesiredWidth(160.f)
			.HeightOverride(44.f)
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleViewLogClicked))
				.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
				.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
				.ContentPadding(FMargin(14.f, 8.f))
				[
					SNew(STextBlock)
					.Text(Text)
					.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
				]
			];
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
		AddStatLine(Loc ? Loc->GetText_Stat_AttackSize() : NSLOCTEXT("T66.Stats", "AttackSize", "Attack Size"), AttackSizeStat);
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
		const TArray<FName>* InventoryPtr = nullptr;

		if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
		{
			IdolsPtr = &LoadedSavedSummary->EquippedIdols;
			InventoryPtr = &LoadedSavedSummary->Inventory;
		}
		else if (RunState)
		{
			IdolsPtr = &RunState->GetEquippedIdols();
			InventoryPtr = &RunState->GetInventory();
		}

		const TArray<FName> Empty;
		const TArray<FName>& Idols = IdolsPtr ? *IdolsPtr : Empty;
		const TArray<FName>& Inventory = InventoryPtr ? *InventoryPtr : Empty;

		for (int32 IdolSlotIndex = 0; IdolSlotIndex < UT66RunStateSubsystem::MaxEquippedIdolSlots; ++IdolSlotIndex)
		{
			const FName IdolID = (Idols.IsValidIndex(IdolSlotIndex)) ? Idols[IdolSlotIndex] : NAME_None;
			const FLinearColor IdolColor = !IdolID.IsNone() ? UT66RunStateSubsystem::GetIdolColor(IdolID) : FT66Style::Tokens::Stroke;
			FIdolData IdolData;
			const bool bHasIdolData = GI && !IdolID.IsNone() && GI->GetIdolData(IdolID, IdolData);
			UTexture2D* IdolTex = nullptr;
			if (bHasIdolData && !IdolData.Icon.IsNull())
			{
				IdolTex = IdolData.Icon.Get();
				if (!IdolTex)
				{
					IdolTex = IdolData.Icon.LoadSynchronous();
				}
			}
			TSharedPtr<FSlateBrush> IdolBrush;
			if (IdolTex)
			{
				IdolBrush = MakeShared<FSlateBrush>();
				IdolBrush->DrawAs = ESlateBrushDrawType::Image;
				IdolBrush->SetResourceObject(IdolTex);
				IdolBrush->ImageSize = FVector2D(18.f, 18.f);
				IdolIconBrushes.Add(IdolBrush);
			}

			IdolsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBorder)
				.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
				.Padding(FMargin(12.f, 10.f))
				[
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
					]
				]
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
				UTexture2D* ItemTex = nullptr;
				if (bHasItemData && !ItemData.Icon.IsNull())
				{
					ItemTex = ItemData.Icon.Get();
					if (!ItemTex)
					{
						ItemTex = ItemData.Icon.LoadSynchronous();
					}
				}
				TSharedPtr<FSlateBrush> ItemBrush;
				if (ItemTex)
				{
					ItemBrush = MakeShared<FSlateBrush>();
					ItemBrush->DrawAs = ESlateBrushDrawType::Image;
					ItemBrush->SetResourceObject(ItemTex);
					ItemBrush->ImageSize = FVector2D(18.f, 18.f);
					InventoryItemIconBrushes.Add(ItemBrush);
				}

				InvBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(SBorder)
					.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
					.Padding(FMargin(12.f, 10.f))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SBox).WidthOverride(18.f).HeightOverride(18.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(bHasItemData ? ItemData.PlaceholderColor : FT66Style::Tokens::Stroke)
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
						]
					]
				];
			}
		}
	}

	// Buttons row (must be inside the main panel).
	TSharedRef<SWidget> ButtonsRow =
		bViewingSavedLeaderboardRunSummary
		? StaticCastSharedRef<SWidget>(
			SNew(SBox).MinDesiredWidth(200.f).HeightOverride(54.f)
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked))
				.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
				.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
				.ContentPadding(FMargin(18.f, 10.f))
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK"))
					.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
				]
			])
		: StaticCastSharedRef<SWidget>(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SNew(SBox).MinDesiredWidth(180.f).HeightOverride(54.f)
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked))
					.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary"))
					.ButtonColorAndOpacity(FT66Style::Tokens::Success)
					.ContentPadding(FMargin(18.f, 10.f))
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_Restart() : NSLOCTEXT("T66.RunSummary", "Restart", "RESTART"))
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
					]
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SNew(SBox).MinDesiredWidth(200.f).HeightOverride(54.f)
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleMainMenuClicked))
					.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
					.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
					.ContentPadding(FMargin(18.f, 10.f))
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_MainMenu() : NSLOCTEXT("T66.RunSummary", "MainMenu", "MAIN MENU"))
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
					]
				]
			]);

	// Panels in the requested layout.
	TSharedRef<SWidget> BaseStatsPanel = MakePanel(
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

	TSharedRef<SWidget> LuckRatingPanel = MakePanel(
		NSLOCTEXT("T66.RunSummary", "LuckRatingPanel", "LUCK RATING"),
		MakeBigCenteredValue(LuckRatingText)
	);
	TSharedRef<SWidget> SkillRatingPanel = MakePanel(
		NSLOCTEXT("T66.RunSummary", "SkillRatingPanel", "SKILL RATING"),
		MakeBigCenteredValue(SkillRatingText)
	);
	const FText CheatProbLine = FText::Format(
		NSLOCTEXT("T66.RunSummary", "CheatProbabilityLineFormat", "Probability of Cheating: {0}%"),
		FText::AsNumber(0)
	);
	TSharedRef<SWidget> CheatProbabilityPanel = MakePanel(
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

	TSharedRef<SWidget> InventoryPanel = MakePanel(
		NSLOCTEXT("T66.RunSummary", "InventoryPanel", "INVENTORY"),
		SNew(SScrollBox) + SScrollBox::Slot()[InvBox]
	);

	TSharedRef<SWidget> IdolsPanel = MakePanel(
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
				SNew(SBorder)
				.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
				.Padding(FMargin(FT66Style::Tokens::Space6))
				[
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
							return Loc ? Loc->GetText_NewPersonalHighScore() : NSLOCTEXT("T66.RunSummary", "NewPersonalHighScore", "New Personal High Score");
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
					// Buttons row (inside panel)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
					[
						ButtonsRow
					]
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
			UIManager->CloseModal();
			return;
		}
		UGameplayStatics::OpenLevel(this, FName(TEXT("FrontendLevel")));
		return;
	}
	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState) RunState->ResetForNewRun();
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
			UIManager->CloseModal();
			return;
		}
		UGameplayStatics::OpenLevel(this, FName(TEXT("FrontendLevel")));
		return;
	}
	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState) RunState->ResetForNewRun();
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
