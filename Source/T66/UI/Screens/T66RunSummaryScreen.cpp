// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66SkillRatingSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66SaveMigration.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66StatsPanelSlate.h"
#include "UI/Style/T66Style.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "UObject/SoftObjectPath.h"
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

		// Power Coupons earned popup: show only when player earned at least 1 this run (beat at least one boss).
		bShowPowerCouponsPopup = (RunState && RunState->GetPowerCrystalsEarnedThisRun() >= 1);
	}
	else
	{
		bShowPowerCouponsPopup = false;
	}

	EnsurePreviewCaptures();
}

void UT66RunSummaryScreen::OnScreenDeactivated_Implementation()
{
	DestroyPreviewCaptures();
	InventoryItemIconBrushes.Reset();
	IdolIconBrushes.Reset();
	PowerCouponSpriteBrush.Reset();
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
				HeroPreviewBrush->ImageSize = FVector2D(520.f, 520.f);
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
		LoadedSavedSummary->HeroID = T66MigrateHeroIDFromSave(LoadedSavedSummary->HeroID);
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

	// Power Coupons popup sprite (Content/UI/Sprites/PowerUp).
	if (bShowPowerCouponsPopup && TexPool)
	{
		if (!PowerCouponSpriteBrush.IsValid())
		{
			PowerCouponSpriteBrush = MakeShared<FSlateBrush>();
			PowerCouponSpriteBrush->DrawAs = ESlateBrushDrawType::Image;
			PowerCouponSpriteBrush->ImageSize = FVector2D(48.f, 48.f);
		}
		const TSoftObjectPtr<UTexture2D> PowerUpTex(FSoftObjectPath(TEXT("/Game/UI/Sprites/PowerUp/PowerUp.PowerUp")));
		T66SlateTexture::BindSharedBrushAsync(TexPool, PowerUpTex, this, PowerCouponSpriteBrush, FName(TEXT("PowerCouponSprite")), /*bClearWhileLoading*/ true);
	}

	RebuildLogItems();
	SAssignNew(LogListView, SListView<TSharedPtr<FString>>)
		.ListItemsSource(&LogItems)
		.OnGenerateRow(SListView<TSharedPtr<FString>>::FOnGenerateRow::CreateUObject(this, &UT66RunSummaryScreen::GenerateLogRow))
		.SelectionMode(ESelectionMode::None);

	const FText TitleText = Loc ? Loc->GetText_RunSummaryTitle() : NSLOCTEXT("T66.RunSummary", "Title", "RUN SUMMARY");
	const FText StageHighScoreText = Loc
		? FText::Format(Loc->GetText_RunSummaryStageReachedBountyFormat(), FText::AsNumber(StageReached), FText::AsNumber(HighScore))
		: FText::FromString(FString::Printf(TEXT("Stage Reached: %d  |  Score: %d"), StageReached, HighScore));

	// Global ranking (high score always; speed run when mode on — N/A if no time for that stage).
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PS = GetWorld() && GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	const int32 BountyRank = (GI && LB && !bViewingSavedLeaderboardRunSummary) ? LB->GetLocalBountyRank(GI->SelectedDifficulty, GI->SelectedPartySize) : 0;
	const bool bSpeedRunMode = PS ? PS->GetSpeedRunMode() : false;
	const int32 SpeedRunStageForRank = FMath::Max(1, StageReached - 1);
	const int32 SpeedRunRank = (bSpeedRunMode && GI && LB && !bViewingSavedLeaderboardRunSummary) ? LB->GetLocalSpeedRunRank(GI->SelectedDifficulty, GI->SelectedPartySize, SpeedRunStageForRank) : 0;
	const FText HighScoreRankLabelText = NSLOCTEXT("T66.RunSummary", "HighScoreRankLabel", "High Score Rank:");
	const FText HighScoreRankValueText = (BountyRank > 0) ? FText::Format(NSLOCTEXT("T66.RunSummary", "RankFormat", "#{0}"), FText::AsNumber(BountyRank)) : NSLOCTEXT("T66.RunSummary", "RankNA", "N/A");
	const FText SpeedRunRankLabelText = FText::Format(NSLOCTEXT("T66.RunSummary", "SpeedRunRankLabelFormat", "Speed Run Rank (Stage {0}):"), FText::AsNumber(SpeedRunStageForRank));
	const FText SpeedRunRankValueText = (SpeedRunRank > 0) ? FText::Format(NSLOCTEXT("T66.RunSummary", "RankFormat", "#{0}"), FText::AsNumber(SpeedRunRank)) : NSLOCTEXT("T66.RunSummary", "RankNA", "N/A");

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
		constexpr float PreviewSize = 520.f;
		return Brush.IsValid()
			? StaticCastSharedRef<SWidget>(FT66Style::MakePanel(
				SNew(SBox)
				.WidthOverride(PreviewSize)
				.HeightOverride(PreviewSize)
				[
					SNew(SImage).Image(Brush.Get())
				],
				FT66PanelParams(ET66PanelType::Panel).SetPadding(0.f)
			))
			: StaticCastSharedRef<SWidget>(FT66Style::MakePanel(
				SNew(SBox)
				.WidthOverride(PreviewSize)
				.HeightOverride(PreviewSize)
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

	// Stats panel: same width and content as Vendor/Gambler (primary + secondary, scroll). Header "STATS".
	const float StatsPanelWidth = FT66Style::Tokens::NPCVendorStatsPanelWidth;
	TSharedRef<SWidget> BaseStatsPanel = [&]() -> TSharedRef<SWidget>
	{
		if (RunState && !bViewingSavedLeaderboardRunSummary)
		{
			return T66StatsPanelSlate::MakeEssentialStatsPanel(RunState, Loc, StatsPanelWidth, true);
		}
		if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary && LoadedSavedSummary->SecondaryStatValues.Num() > 0)
		{
			return T66StatsPanelSlate::MakeEssentialStatsPanelFromSnapshot(LoadedSavedSummary, Loc, StatsPanelWidth);
		}
		TSharedRef<SVerticalBox> PrimaryStatsBox = SNew(SVerticalBox);
		const FText StatFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.Stats", "StatLineFormat", "{0}: {1}");
		auto AddStatLine = [&](const FText& Label, int32 Value)
		{
			PrimaryStatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
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
		TSharedRef<SWidget> PrimaryContent = SNew(SBox)
			.HeightOverride(FT66Style::Tokens::NPCStatsPanelContentHeight)
			[
				SNew(SScrollBox)
				.ScrollBarVisibility(EVisibility::Visible)
				+ SScrollBox::Slot()[PrimaryStatsBox]
			];
		// Header must be "STATS" (same as Vendor/Gambler), not "Base Stats", for leaderboard/saved run summaries.
		return SNew(SBox)
			.WidthOverride(StatsPanelWidth)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.StatsPanel", "Header", "STATS"))
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
					]
					+ SVerticalBox::Slot().AutoHeight()[PrimaryContent],
					FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4)
				)
			];
	}();

	// Idols: one row, 6 columns, with simple border under hero.
	const TArray<FName>* IdolsPtr = nullptr;
	TArray<FName> InventoryLocal;
	const TArray<FT66InventorySlot>* InvSlotsPtr = nullptr;
	if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
	{
		IdolsPtr = &LoadedSavedSummary->EquippedIdols;
		InventoryLocal = LoadedSavedSummary->Inventory;
	}
	else if (RunState)
	{
		IdolsPtr = &RunState->GetEquippedIdols();
		InventoryLocal = RunState->GetInventory();
		InvSlotsPtr = &RunState->GetInventorySlots();
	}
	const TArray<FName> Empty;
	const TArray<FName>& Idols = IdolsPtr ? *IdolsPtr : Empty;

	static constexpr float IdolSlotPad = 4.f;
	static constexpr float IdolSlotSize = 52.f;
	TSharedRef<SHorizontalBox> IdolSlotsRow = SNew(SHorizontalBox);
	for (int32 i = 0; i < UT66RunStateSubsystem::MaxEquippedIdolSlots; ++i)
	{
		const FName IdolID = Idols.IsValidIndex(i) ? Idols[i] : NAME_None;
		const FLinearColor IdolColor = !IdolID.IsNone() ? UT66RunStateSubsystem::GetIdolColor(IdolID) : FLinearColor(0.45f, 0.55f, 0.50f, 0.5f);
		FIdolData IdolData;
		const bool bHasIdolData = GI && !IdolID.IsNone() && GI->GetIdolData(IdolID, IdolData);
		TSharedPtr<FSlateBrush> IdolBrush;
		if (bHasIdolData && !IdolData.Icon.IsNull())
		{
			IdolBrush = MakeShared<FSlateBrush>();
			IdolBrush->DrawAs = ESlateBrushDrawType::Image;
			IdolBrush->ImageSize = FVector2D(IdolSlotSize - 4.f, IdolSlotSize - 4.f);
			IdolIconBrushes.Add(IdolBrush);
			if (TexPool) T66SlateTexture::BindSharedBrushAsync(TexPool, IdolData.Icon, this, IdolBrush, IdolID, true);
		}
		IdolSlotsRow->AddSlot()
			.AutoWidth()
			.Padding(IdolSlotPad)
			[
				SNew(SBox).WidthOverride(IdolSlotSize).HeightOverride(IdolSlotSize)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(IdolColor)
					.Padding(1.f)
					[
						IdolBrush.IsValid()
						? StaticCastSharedRef<SWidget>(SNew(SImage).Image(IdolBrush.Get()))
						: StaticCastSharedRef<SWidget>(SNew(SSpacer))
					]
				]
			];
	}
	TSharedRef<SWidget> IdolsBorderedGrid = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::Tokens::Stroke)
		.Padding(4.f)
		[
			IdolSlotsRow
		];

	// Inventory: slot grid (2x10), sprites only — larger slots than in-game.
	static constexpr int32 InvCols = 10;
	static constexpr int32 InvRows = 2;
	static constexpr float InvSlotSize = 56.f;
	static constexpr float InvSlotPad = 3.f;
	const FLinearColor InvSlotBorderColor(0.45f, 0.55f, 0.50f, 0.5f);
	InventoryItemIconBrushes.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	for (int32 i = 0; i < UT66RunStateSubsystem::MaxInventorySlots; ++i)
	{
		if (!InventoryItemIconBrushes[i].IsValid()) InventoryItemIconBrushes[i] = MakeShared<FSlateBrush>();
		InventoryItemIconBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		InventoryItemIconBrushes[i]->ImageSize = FVector2D(InvSlotSize - 4.f, InvSlotSize - 4.f);
		InventoryItemIconBrushes[i]->SetResourceObject(nullptr);
	}
	for (int32 InvIdx = 0; InvIdx < UT66RunStateSubsystem::MaxInventorySlots && InvIdx < InventoryLocal.Num(); ++InvIdx)
	{
		const FName ItemID = InventoryLocal[InvIdx];
		if (ItemID.IsNone()) continue;
		FItemData ItemData;
		if (GI && GI->GetItemData(ItemID, ItemData) && !ItemData.Icon.IsNull() && TexPool)
		{
			T66SlateTexture::BindSharedBrushAsync(TexPool, ItemData.Icon, this, InventoryItemIconBrushes[InvIdx], ItemID, true);
		}
	}
	TSharedRef<SVerticalBox> InvGridRef = SNew(SVerticalBox);
	for (int32 Row = 0; Row < InvRows; ++Row)
	{
		TSharedRef<SHorizontalBox> RowBox = SNew(SHorizontalBox);
		for (int32 Col = 0; Col < InvCols; ++Col)
		{
			const int32 SlotIndex = Row * InvCols + Col;
			const FLinearColor SlotColor = (InvSlotsPtr && InvSlotsPtr->IsValidIndex(SlotIndex))
				? FItemData::GetItemRarityColor((*InvSlotsPtr)[SlotIndex].Rarity)
				: InvSlotBorderColor;
			const bool bHasItem = InventoryLocal.IsValidIndex(SlotIndex) && !InventoryLocal[SlotIndex].IsNone();
			TSharedPtr<FSlateBrush> SlotBrush = InventoryItemIconBrushes.IsValidIndex(SlotIndex) ? InventoryItemIconBrushes[SlotIndex] : nullptr;
			RowBox->AddSlot()
				.AutoWidth()
				.Padding(InvSlotPad)
				[
					SNew(SBox).WidthOverride(InvSlotSize).HeightOverride(InvSlotSize)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(SlotColor)
						.Padding(1.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.25f))
							.Padding(0.f)
							[
								(SlotBrush.IsValid() && bHasItem)
								? StaticCastSharedRef<SWidget>(SNew(SImage).Image(SlotBrush.Get()))
								: StaticCastSharedRef<SWidget>(SNew(SSpacer))
							]
						]
					]
				];
		}
		InvGridRef->AddSlot().AutoHeight()[RowBox];
	}
	TSharedRef<SWidget> InventorySlotGrid = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::Tokens::Stroke)
		.Padding(4.f)
		[
			InvGridRef
		];

	// Back button (when viewing saved run) — shown in overlay bottom-left; Restart + Main Menu stay in panel.
	TSharedRef<SWidget> BackButton =
		FT66Style::MakeButton(FT66ButtonParams(Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked))
			.SetMinWidth(120.f).SetPadding(FMargin(18.f, 10.f)));

	// Retry level disclaimer and button (run not eligible for leaderboard).
	const FText RetryIneligibleText = NSLOCTEXT("T66.RunSummary", "RetryIneligibleForLeaderboard", "This run will not be eligible for leaderboard.");
	const FText RetryLevelButtonText = NSLOCTEXT("T66.RunSummary", "RetryLevel", "RETRY LEVEL");
	TSharedRef<SWidget> RetryLevelBlock =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(RetryIneligibleText)
			.Font(FT66Style::Tokens::FontRegular(12))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			.Justification(ETextJustify::Center)
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			FT66Style::MakeButton(FT66ButtonParams(RetryLevelButtonText, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRetryLevelClicked), ET66ButtonType::Primary)
				.SetMinWidth(160.f).SetPadding(FMargin(18.f, 10.f)))
		];

	// Buttons stacked vertically (Restart, Main Menu, Retry Level) — shown below Integrity when not viewing a saved run.
	TSharedRef<SWidget> ButtonsStack =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			FT66Style::MakeButton(FT66ButtonParams(Loc ? Loc->GetText_Restart() : NSLOCTEXT("T66.RunSummary", "Restart", "RESTART"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked), ET66ButtonType::Success)
				.SetMinWidth(180.f).SetPadding(FMargin(18.f, 10.f)))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			FT66Style::MakeButton(FT66ButtonParams(Loc ? Loc->GetText_MainMenu() : NSLOCTEXT("T66.RunSummary", "MainMenu", "MAIN MENU"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleMainMenuClicked))
				.SetMinWidth(200.f).SetPadding(FMargin(18.f, 10.f)))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			RetryLevelBlock
		];

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

	// Damage by source: ranked table of 7 sources (Auto Attack + 6 idols). Use real data when present, else dummy.
	static const FName DamageSourceIds[7] =
	{
		UT66DamageLogSubsystem::SourceID_AutoAttack,
		FName(TEXT("Idol_Star")),
		FName(TEXT("Idol_Curse")),
		FName(TEXT("Idol_Rubber")),
		FName(TEXT("Idol_Lava")),
		FName(TEXT("Idol_Electric")),
		FName(TEXT("Idol_Fire"))
	};
	static const int32 DummyDamage[7] = { 520, 380, 290, 210, 150, 85, 42 };
	TMap<FName, int32> DamageMap;
	if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary && LoadedSavedSummary->SchemaVersion >= 5)
	{
		DamageMap = LoadedSavedSummary->DamageBySource;
	}
	else if (UT66DamageLogSubsystem* DamageLog = GI ? GI->GetSubsystem<UT66DamageLogSubsystem>() : nullptr)
	{
		for (const FDamageLogEntry& E : DamageLog->GetDamageBySourceSorted())
		{
			DamageMap.FindOrAdd(E.SourceID) = E.TotalDamage;
		}
	}
	auto GetDamageSourceDisplayName = [](FName SourceID) -> FText
	{
		if (SourceID == UT66DamageLogSubsystem::SourceID_AutoAttack) return NSLOCTEXT("T66.RunSummary", "DamageSource_AutoAttack", "Auto Attack");
		if (SourceID == UT66DamageLogSubsystem::SourceID_Ultimate) return NSLOCTEXT("T66.RunSummary", "DamageSource_Ultimate", "Ultimate");
		return FText::FromName(SourceID);
	};
	TArray<FDamageLogEntry> TableRows;
	for (int32 i = 0; i < 7; ++i)
	{
		FDamageLogEntry E;
		E.SourceID = DamageSourceIds[i];
		E.TotalDamage = DamageMap.Contains(E.SourceID) ? DamageMap[E.SourceID] : DummyDamage[i];
		TableRows.Add(E);
	}
	TableRows.Sort([](const FDamageLogEntry& A, const FDamageLogEntry& B) { return A.TotalDamage > B.TotalDamage; });
	const FTextBlockStyle& BodyStyle = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body");
	const FText RankHeader = NSLOCTEXT("T66.RunSummary", "DamageTableRank", "Rank");
	const FText SourceHeader = NSLOCTEXT("T66.RunSummary", "DamageTableSource", "Source");
	const FText DamageHeader = NSLOCTEXT("T66.RunSummary", "DamageTableDamage", "Damage");
	TSharedRef<SVerticalBox> DamageBySourceBox = SNew(SVerticalBox);
	DamageBySourceBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
			[SNew(STextBlock).Text(RankHeader).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::TextMuted).Font(FT66Style::Tokens::FontBold(12))]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 80.f, 0.f)
			[SNew(STextBlock).Text(SourceHeader).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::TextMuted).Font(FT66Style::Tokens::FontBold(12))]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[SNew(STextBlock).Text(DamageHeader).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::TextMuted).Font(FT66Style::Tokens::FontBold(12))]
		];
	for (int32 Rank = 0; Rank < TableRows.Num(); ++Rank)
	{
		const FDamageLogEntry& Entry = TableRows[Rank];
		DamageBySourceBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
				[SNew(STextBlock).Text(FText::AsNumber(Rank + 1)).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::Text)]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 80.f, 0.f)
				[SNew(STextBlock).Text(GetDamageSourceDisplayName(Entry.SourceID)).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::Text)]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[SNew(STextBlock).Text(FText::AsNumber(Entry.TotalDamage)).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::Text)]
			];
	}
	TSharedRef<SWidget> DamageBySourcePanel = MakeSectionPanel(
		NSLOCTEXT("T66.RunSummary", "DamageBySourcePanel", "DAMAGE BY SOURCE"),
		DamageBySourceBox
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
					// Global ranking (high score + speed run when mode on) — only for current run, not when viewing saved.
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
					[
						SNew(SBox)
						.Visibility_Lambda([this]() { return bViewingSavedLeaderboardRunSummary ? EVisibility::Collapsed : EVisibility::Visible; })
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 24.f, 0.f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth()
								[
									SNew(STextBlock)
									.Text(HighScoreRankLabelText)
									.Font(FT66Style::Tokens::FontRegular(14))
									.ColorAndOpacity(FT66Style::Tokens::TextMuted)
								]
								+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f, 0.f, 0.f)
								[
									SNew(STextBlock)
									.Text(HighScoreRankValueText)
									.Font(FT66Style::Tokens::FontBold(14))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SHorizontalBox)
								.Visibility_Lambda([bSpeedRunMode]() { return bSpeedRunMode ? EVisibility::Visible : EVisibility::Collapsed; })
								+ SHorizontalBox::Slot().AutoWidth()
								[
									SNew(STextBlock)
									.Text(SpeedRunRankLabelText)
									.Font(FT66Style::Tokens::FontRegular(14))
									.ColorAndOpacity(FT66Style::Tokens::TextMuted)
								]
								+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f, 0.f, 0.f)
								[
									SNew(STextBlock)
									.Text(SpeedRunRankValueText)
									.Font(FT66Style::Tokens::FontBold(14))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
						]
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
					// Main content: Left = Luck, Skill, Integrity, Proof of Run, They're cheating. Center = Hero (middle), idols (1x6), inventory. Right = Stats, Damage.
					+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 0.f, 0.f, 18.f)
					[
						SNew(SHorizontalBox)
						// Left side: Luck Rating, Skill Rating, Probability of Cheating, Proof of Run, They're cheating
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 24.f, 0.f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[LuckRatingPanel]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[SkillRatingPanel]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[CheatProbabilityPanel]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f)
							[
								SNew(SBox).Visibility_Lambda([this]() { return bViewingSavedLeaderboardRunSummary ? EVisibility::Collapsed : EVisibility::Visible; })
								[ButtonsStack]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
							[
								SNew(SBox).Visibility_Lambda([this]() { return bViewingSavedLeaderboardRunSummary ? EVisibility::Visible : EVisibility::Collapsed; })[ProofOfRunPanel]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
							[
								SNew(SBox).Visibility_Lambda([this]() { return bViewingSavedLeaderboardRunSummary ? EVisibility::Visible : EVisibility::Collapsed; })
								[ReportCheatButton]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
							[
								SNew(SBox).Visibility_Lambda([this]() { return bReportPromptVisible ? EVisibility::Visible : EVisibility::Collapsed; })
								[ReportPrompt]
							]
							+ SVerticalBox::Slot().FillHeight(1.f)[SNew(SSpacer)]
						]
						// Center: Hero preview (middle), then idols (1 row x 6), then inventory
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
							[MakeHeroPreview(HeroPreviewBrush)]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
							[IdolsBorderedGrid]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[InventorySlotGrid]
						]
						// Right: Stats, Damage by source
						+ SHorizontalBox::Slot().AutoWidth().Padding(24.f, 0.f, 0.f, 0.f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[BaseStatsPanel]
							+ SVerticalBox::Slot().FillHeight(1.f)[DamageBySourcePanel]
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
			// Power Coupons earned popup (only when earned >= 1 this run, not when viewing saved).
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.Visibility_Lambda([this]() { return bShowPowerCouponsPopup ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.7f))
					.Padding(0.f)
					[
						SNew(SBox)
						.WidthOverride(320.f)
						.Padding(24.f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.RunSummary", "PowerCouponsEarnedTitle", "Power Coupons earned"))
								.Font(FT66Style::Tokens::FontBold(18))
								.ColorAndOpacity(FT66Style::Tokens::Text)
								.Justification(ETextJustify::Center)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
								[
									SNew(SBox)
									.WidthOverride(48.f)
									.HeightOverride(48.f)
									[
										SNew(SImage)
										.Image_Lambda([this]() { return PowerCouponSpriteBrush.IsValid() ? PowerCouponSpriteBrush.Get() : FCoreStyle::Get().GetDefaultBrush(); })
									]
								]
								+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text_Lambda([this]()
									{
										UT66RunStateSubsystem* RS = GetWorld() && GetWorld()->GetGameInstance()
											? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
										const int32 Earned = RS ? RS->GetPowerCrystalsEarnedThisRun() : 0;
										return FText::AsNumber(Earned);
									})
									.Font(FT66Style::Tokens::FontBold(24))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f)
							[
								FT66Style::MakeButton(FT66ButtonParams(
									NSLOCTEXT("T66.RunSummary", "PowerCouponsThankYou", "Thank You"),
									FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandlePowerCouponsThankYouClicked)))
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
	bReportPromptVisible = !bReportPromptVisible;
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

FReply UT66RunSummaryScreen::HandlePowerCouponsThankYouClicked()
{
	bShowPowerCouponsPopup = false;
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

FReply UT66RunSummaryScreen::HandleRetryLevelClicked()
{
	DestroyPreviewCaptures();
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	if (T66GI)
	{
		T66GI->bRunIneligibleForLeaderboard = true;
		T66GI->bIsStageTransition = true;
	}
	APlayerController* PC = GetOwningPlayer();
	if (PC) PC->SetPause(false);
	UGameplayStatics::OpenLevel(this, FName(TEXT("GameplayLevel")));
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
