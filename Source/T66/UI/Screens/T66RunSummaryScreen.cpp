// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66CommunityContentSubsystem.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardPacingUtils.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66SaveMigration.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66TemporaryBuffUIUtils.h"
#include "UI/T66StatsPanelSlate.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
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
#include "Widgets/Images/SThrobber.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Styling/CoreStyle.h"
#include "UObject/StrongObjectPtr.h"
#include "HAL/PlatformProcess.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66RunSummary, Log, All);

UT66RunSummaryScreen::UT66RunSummaryScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::RunSummary;
	bIsModal = true;
}

void UT66RunSummaryScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	bLiveRunSubmissionProcessed = false;
	bLiveRunFinalAccountingProcessed = false;
	bDifficultyClearSummaryMode = false;
	bDailyClimbSummaryMode = false;
	bBackendRankDataReceived = false;
	bAwaitingBackendRankData = false;
	BackendScoreRankAllTime = 0;
	BackendScoreRankWeekly = 0;
	BackendSpeedRunRankAllTime = 0;
	BackendSpeedRunRankWeekly = 0;
	BackendDailyScoreRank = 0;
	bLiveRunCheatFlagged = false;
	bChadCouponsResolutionProcessed = false;
	SummaryChadCouponsEarned = 0;
	SummaryChadCouponsSourceLabel.Reset();
	SummaryChadCouponsFailureReason.Reset();
	bChadCouponsPopupDontShowAgainChecked = false;

	// If we were opened from a leaderboard row click, load the saved snapshot first.
	const bool bWasViewingSaved = bViewingSavedLeaderboardRunSummary;
	const TObjectPtr<UT66LeaderboardRunSummarySaveGame> PrevSummary = LoadedSavedSummary;
	const bool bConsumedRequest = LoadSavedRunSummaryIfRequested();
	if (!bConsumedRequest && bViewingSavedLeaderboardRunSummary && HasValidLiveRunSummaryContext())
	{
		UE_LOG(LogT66RunSummary, Log, TEXT("Run Summary: clearing stale cached viewer state before live activation."));
		ResetSavedRunSummaryViewerState();
	}

	// If a new pending request was consumed (or viewer state changed), rebuild immediately.
	// NOTE: Snapshot loading also happens during RebuildWidget() so the first open should already be correct.
	if (bConsumedRequest || (bWasViewingSaved != bViewingSavedLeaderboardRunSummary) || (PrevSummary != LoadedSavedSummary))
	{
		ForceRebuildSlate();
	}

	// Default: banners are only relevant for a freshly-finished run (not for viewing saved leaderboard snapshots).
	bNewPersonalBestScore = false;
	bNewPersonalBestTime = false;

	// Foundation: submit score at run end if allowed by settings (Practice Mode blocks).
	// Important: do NOT submit when we're just viewing a saved run from the leaderboard.
	if (!bViewingSavedLeaderboardRunSummary)
	{
		UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
		UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
		bDailyClimbSummaryMode = T66GI && T66GI->IsDailyClimbRunActive();
		if (UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr)
		{
			Backend->OnSubmitRunDataReady.RemoveAll(this);
			Backend->OnDailyClimbSubmitDataReady.RemoveAll(this);
			if (bDailyClimbSummaryMode)
			{
				Backend->OnDailyClimbSubmitDataReady.AddUObject(this, &UT66RunSummaryScreen::HandleBackendDailyClimbSubmitDataReadyForSummary);
			}
			else
			{
				Backend->OnSubmitRunDataReady.AddUObject(this, &UT66RunSummaryScreen::HandleBackendSubmitRunDataReadyForSummary);
			}
		}
		bDifficultyClearSummaryMode = !bDailyClimbSummaryMode && RunState && RunState->HasPendingDifficultyClearSummary();
		UE_LOG(
			LogT66RunSummary,
			Log,
			TEXT("Run Summary: live activation stage=%d score=%d difficultyClear=%d daily=%d runEnded=%d victory=%d"),
			RunState ? RunState->GetCurrentStage() : -1,
			RunState ? RunState->GetCurrentScore() : -1,
			bDifficultyClearSummaryMode ? 1 : 0,
			bDailyClimbSummaryMode ? 1 : 0,
			RunState && RunState->HasRunEnded() ? 1 : 0,
			RunState && RunState->DidRunEndInVictory() ? 1 : 0);
		if (HasValidLiveRunSummaryContext())
		{
			PrepareChadCouponsPopupForLiveRun();
			if (bDifficultyClearSummaryMode)
			{
				ProcessRunSummaryLeaderboardSubmission(true);
			}
			else
			{
				ProcessLiveRunFinalSubmission();
			}
		}
		else
		{
			UE_LOG(LogT66RunSummary, Log, TEXT("Run Summary: live activation has no finished-run context; skipping leaderboard submission and final accounting."));
		}

		// The UI manager reuses modal widget instances, so the cached Slate tree can
		// still reflect the previous button stack/popup state unless we rebuild after
		// activation has resolved the current live-run mode.
		ForceRebuildSlate();
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
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnSubmitRunDataReady.RemoveAll(this);
			Backend->OnDailyClimbSubmitDataReady.RemoveAll(this);
		}

		if (bDailyClimbSummaryMode)
		{
			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
			{
				T66GI->ClearActiveDailyClimbRun();
			}
		}
	}
	InventoryItemIconBrushes.Reset();
	IdolIconBrushes.Reset();
	TemporaryBuffIconBrushes.Reset();
	PowerCouponSpriteBrush.Reset();
	bLiveRunSubmissionProcessed = false;
	bLiveRunFinalAccountingProcessed = false;
	bDifficultyClearSummaryMode = false;
	bDailyClimbSummaryMode = false;
	bBackendRankDataReceived = false;
	bAwaitingBackendRankData = false;
	BackendScoreRankAllTime = 0;
	BackendScoreRankWeekly = 0;
	BackendSpeedRunRankAllTime = 0;
	BackendSpeedRunRankWeekly = 0;
	BackendDailyScoreRank = 0;
	bLiveRunCheatFlagged = false;
	bChadCouponsResolutionProcessed = false;
	SummaryChadCouponsEarned = 0;
	SummaryChadCouponsSourceLabel.Reset();
	SummaryChadCouponsFailureReason.Reset();
	bChadCouponsPopupDontShowAgainChecked = false;
	bShowPowerCouponsPopup = false;
	Super::OnScreenDeactivated_Implementation();
}

namespace
{
	constexpr int32 T66RunSummaryFontDelta = -8;

	enum class ET66RunSummaryButtonFamily : uint8
	{
		CompactNeutral,
		ToggleOn,
		ToggleOff,
		CtaGreen,
		CtaBlue
	};

	enum class ET66RunSummaryButtonState : uint8
	{
		Normal,
		Hovered,
		Pressed
	};

	struct FT66RunSummarySpriteBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
	};

	struct FT66RunSummaryButtonBrushSet
	{
		FT66RunSummarySpriteBrushEntry Normal;
		FT66RunSummarySpriteBrushEntry Hover;
		FT66RunSummarySpriteBrushEntry Pressed;
	};

	const FLinearColor RunSummaryFantasyText(0.953f, 0.925f, 0.835f, 1.0f);

	const FSlateBrush* ResolveRunSummarySpriteBrush(
		FT66RunSummarySpriteBrushEntry& Entry,
		const FString& RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		const ESlateBrushDrawType::Type DrawAs)
	{
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->DrawAs = DrawAs;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
			Entry.Brush->ImageSize = ImageSize;
			Entry.Brush->Margin = Margin;
		}

		if (!Entry.Texture.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					true,
					TEXT("RunSummaryReferenceSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		return Entry.Texture.IsValid() ? Entry.Brush.Get() : nullptr;
	}

	const FSlateBrush* GetRunSummaryContentShellBrush()
	{
		static FT66RunSummarySpriteBrushEntry Entry;
		return ResolveRunSummarySpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Panels/runflow_summary_surface_shell.png"),
			FVector2D(1680.f, 860.f),
			FMargin(0.035f, 0.12f, 0.035f, 0.12f),
			ESlateBrushDrawType::Box);
	}

	const FSlateBrush* GetRunSummaryRowShellBrush()
	{
		static FT66RunSummarySpriteBrushEntry Entry;
		return ResolveRunSummarySpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Panels/runflow_summary_row_shell.png"),
			FVector2D(1180.f, 86.f),
			FMargin(0.055f, 0.32f, 0.055f, 0.32f),
			ESlateBrushDrawType::Box);
	}

	FString GetRunSummaryButtonPath(const ET66RunSummaryButtonFamily Family, const ET66RunSummaryButtonState State)
	{
		if (Family == ET66RunSummaryButtonFamily::CtaGreen || Family == ET66RunSummaryButtonFamily::CtaBlue)
		{
			const TCHAR* Prefix = Family == ET66RunSummaryButtonFamily::CtaGreen
				? TEXT("runflow_cta_primary")
				: TEXT("runflow_cta_blue");
			const TCHAR* Suffix = TEXT("normal");
			if (State == ET66RunSummaryButtonState::Hovered)
			{
				Suffix = TEXT("hover");
			}
			else if (State == ET66RunSummaryButtonState::Pressed)
			{
				Suffix = TEXT("pressed");
			}
			return FString::Printf(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/%s_%s.png"), Prefix, Suffix);
		}

		const TCHAR* Prefix = TEXT("runflow_button_neutral");
		if (Family == ET66RunSummaryButtonFamily::ToggleOn)
		{
			Prefix = TEXT("runflow_button_primary");
		}
		else if (Family == ET66RunSummaryButtonFamily::ToggleOff)
		{
			Prefix = TEXT("runflow_button_danger");
		}

		const TCHAR* Suffix = TEXT("normal");
		if (State == ET66RunSummaryButtonState::Hovered)
		{
			Suffix = TEXT("hover");
		}
		else if (State == ET66RunSummaryButtonState::Pressed)
		{
			Suffix = TEXT("pressed");
		}
		return FString::Printf(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/%s_%s.png"), Prefix, Suffix);
	}

	FVector2D GetRunSummaryButtonSize(const ET66RunSummaryButtonFamily Family, const ET66RunSummaryButtonState State)
	{
		if (Family == ET66RunSummaryButtonFamily::CtaGreen)
		{
			return FVector2D(388.f, 100.f);
		}
		if (Family == ET66RunSummaryButtonFamily::CtaBlue)
		{
			return FVector2D(388.f, 97.f);
		}
		if (Family == ET66RunSummaryButtonFamily::ToggleOn)
		{
			return State == ET66RunSummaryButtonState::Pressed ? FVector2D(187.f, 67.f) : FVector2D(180.f, 68.f);
		}
		if (Family == ET66RunSummaryButtonFamily::ToggleOff)
		{
			return State == ET66RunSummaryButtonState::Pressed ? FVector2D(186.f, 68.f) : FVector2D(180.f, 68.f);
		}
		return State == ET66RunSummaryButtonState::Pressed ? FVector2D(186.f, 68.f) : FVector2D(180.f, 68.f);
	}

	FT66RunSummaryButtonBrushSet& GetRunSummaryButtonBrushSet(const ET66RunSummaryButtonFamily Family)
	{
		static FT66RunSummaryButtonBrushSet CompactNeutral;
		static FT66RunSummaryButtonBrushSet ToggleOn;
		static FT66RunSummaryButtonBrushSet ToggleOff;
		static FT66RunSummaryButtonBrushSet CtaGreen;
		static FT66RunSummaryButtonBrushSet CtaBlue;
		switch (Family)
		{
		case ET66RunSummaryButtonFamily::ToggleOn: return ToggleOn;
		case ET66RunSummaryButtonFamily::ToggleOff: return ToggleOff;
		case ET66RunSummaryButtonFamily::CtaGreen: return CtaGreen;
		case ET66RunSummaryButtonFamily::CtaBlue: return CtaBlue;
		case ET66RunSummaryButtonFamily::CompactNeutral:
		default: return CompactNeutral;
		}
	}

	const FSlateBrush* GetRunSummaryButtonBrush(const ET66RunSummaryButtonFamily Family, const ET66RunSummaryButtonState State)
	{
		FT66RunSummaryButtonBrushSet& Set = GetRunSummaryButtonBrushSet(Family);
		FT66RunSummarySpriteBrushEntry* Entry = &Set.Normal;
		if (State == ET66RunSummaryButtonState::Hovered)
		{
			Entry = &Set.Hover;
		}
		else if (State == ET66RunSummaryButtonState::Pressed)
		{
			Entry = &Set.Pressed;
		}
		return ResolveRunSummarySpriteBrush(
			*Entry,
			GetRunSummaryButtonPath(Family, State),
			GetRunSummaryButtonSize(Family, State),
			(Family == ET66RunSummaryButtonFamily::CtaGreen || Family == ET66RunSummaryButtonFamily::CtaBlue)
				? FMargin(0.16f, 0.30f, 0.16f, 0.30f)
				: FMargin(0.14f, 0.30f, 0.14f, 0.30f),
			ESlateBrushDrawType::Box);
	}

	TSharedRef<SWidget> MakeRunSummarySpritePanel(
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* Brush,
		const FMargin& Padding,
		const FLinearColor& FallbackColor = FLinearColor(0.025f, 0.023f, 0.034f, 0.97f))
	{
		return SNew(SBorder)
			.BorderImage(Brush ? Brush : FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(Brush ? FLinearColor::White : FallbackColor)
			.Padding(Padding)
			[
				Content
			];
	}

	TSharedRef<SWidget> MakeRunSummarySpriteButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		const ET66RunSummaryButtonFamily Family,
		const float MinWidth,
		const float Height,
		const int32 FontSize,
		const FMargin& ContentPadding = FMargin(12.f, 7.f, 12.f, 6.f))
	{
		const FSlateBrush* NormalBrush = GetRunSummaryButtonBrush(Family, ET66RunSummaryButtonState::Normal);
		const FSlateBrush* HoverBrush = GetRunSummaryButtonBrush(Family, ET66RunSummaryButtonState::Hovered);
		const FSlateBrush* PressedBrush = GetRunSummaryButtonBrush(Family, ET66RunSummaryButtonState::Pressed);
		if (!NormalBrush)
		{
			return FT66Style::MakeButton(
				FT66ButtonParams(Label, OnClicked, Family == ET66RunSummaryButtonFamily::ToggleOff ? ET66ButtonType::Danger : ET66ButtonType::Primary)
				.SetMinWidth(MinWidth)
				.SetHeight(Height)
				.SetFontSize(FontSize)
				.SetPadding(ContentPadding));
		}

		const TSharedPtr<ET66RunSummaryButtonState> ButtonState = MakeShared<ET66RunSummaryButtonState>(ET66RunSummaryButtonState::Normal);
		const TAttribute<const FSlateBrush*> BrushAttr = TAttribute<const FSlateBrush*>::CreateLambda(
			[ButtonState, NormalBrush, HoverBrush, PressedBrush]() -> const FSlateBrush*
			{
				if (ButtonState.IsValid() && *ButtonState == ET66RunSummaryButtonState::Pressed)
				{
					return PressedBrush ? PressedBrush : NormalBrush;
				}
				if (ButtonState.IsValid() && *ButtonState == ET66RunSummaryButtonState::Hovered)
				{
					return HoverBrush ? HoverBrush : NormalBrush;
				}
				return NormalBrush;
			});

		return SNew(SBox)
			.MinDesiredWidth(MinWidth > 0.f ? MinWidth : FOptionalSize())
			.HeightOverride(Height > 0.f ? Height : FOptionalSize())
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Visibility(EVisibility::HitTestInvisible)
					.Image(BrushAttr)
				]
				+ SOverlay::Slot()
				[
					SNew(SButton)
					.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
					.ButtonColorAndOpacity(FLinearColor::Transparent)
					.ContentPadding(ContentPadding)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked(OnClicked)
					.OnHovered(FSimpleDelegate::CreateLambda([ButtonState]() { *ButtonState = ET66RunSummaryButtonState::Hovered; }))
					.OnUnhovered(FSimpleDelegate::CreateLambda([ButtonState]() { *ButtonState = ET66RunSummaryButtonState::Normal; }))
					.OnPressed(FSimpleDelegate::CreateLambda([ButtonState]() { *ButtonState = ET66RunSummaryButtonState::Pressed; }))
					.OnReleased(FSimpleDelegate::CreateLambda([ButtonState]() { *ButtonState = ET66RunSummaryButtonState::Hovered; }))
					[
						SNew(STextBlock)
						.Text(Label)
						.Font(FT66Style::Tokens::FontBold(FontSize))
						.ColorAndOpacity(RunSummaryFantasyText)
						.Justification(ETextJustify::Center)
						.AutoWrapText(true)
					]
				]
			];
	}

	static FText FormatRunSummaryDurationText(float TotalSeconds)
	{
		const int32 RoundedSeconds = FMath::Max(0, FMath::RoundToInt(TotalSeconds));
		const int32 Hours = RoundedSeconds / 3600;
		const int32 Minutes = (RoundedSeconds % 3600) / 60;
		const int32 Seconds = RoundedSeconds % 60;

		if (Hours > 0)
		{
			return FText::FromString(FString::Printf(TEXT("%d:%02d:%02d"), Hours, Minutes, Seconds));
		}

		return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
	}

	static FText FormatRunSummaryRankText(int32 Rank)
	{
		return (Rank > 0)
			? FText::Format(NSLOCTEXT("T66.RunSummary", "RankFormat", "#{0}"), FText::AsNumber(Rank))
			: NSLOCTEXT("T66.RunSummary", "RankNA", "N/A");
	}

	static int32 GetDisplayedRunSummaryStageNumber(
		const UT66PlayerExperienceSubSystem* PlayerExperience,
		const ET66Difficulty Difficulty,
		const int32 StageReached)
	{
		if (StageReached <= 0)
		{
			return 1;
		}

		const int32 StageCount = (Difficulty == ET66Difficulty::Impossible) ? 3 : 4;
		if (!PlayerExperience)
		{
			return FMath::Clamp(StageReached, 1, StageCount);
		}

		const int32 StartStage = FMath::Max(1, PlayerExperience->GetDifficultyStartStage(Difficulty));
		const int32 StageOffset = FMath::Max(0, StageReached - StartStage);
		return (StageOffset % StageCount) + 1;
	}

	static bool T66GetNextDifficulty(const ET66Difficulty Current, ET66Difficulty& OutNextDifficulty)
	{
		switch (Current)
		{
		case ET66Difficulty::Easy: OutNextDifficulty = ET66Difficulty::Medium; return true;
		case ET66Difficulty::Medium: OutNextDifficulty = ET66Difficulty::Hard; return true;
		case ET66Difficulty::Hard: OutNextDifficulty = ET66Difficulty::VeryHard; return true;
		case ET66Difficulty::VeryHard: OutNextDifficulty = ET66Difficulty::Impossible; return true;
		default: break;
		}

		OutNextDifficulty = Current;
		return false;
	}

	static int32 AdjustRunSummaryFontSize(int32 BaseSize)
	{
		return FMath::Max(BaseSize + T66RunSummaryFontDelta, 8);
	}

	static FSlateFontInfo RunSummaryRegularFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontRegular(AdjustRunSummaryFontSize(BaseSize));
	}

	static FSlateFontInfo RunSummaryBoldFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontBold(AdjustRunSummaryFontSize(BaseSize));
	}

	static FSlateFontInfo RunSummaryHeadingFont()
	{
		FSlateFontInfo Font = FT66Style::Tokens::FontHeading();
		Font.Size = AdjustRunSummaryFontSize(Font.Size);
		return Font;
	}

	static FSlateFontInfo RunSummaryBodyFont()
	{
		FSlateFontInfo Font = FT66Style::Tokens::FontBody();
		Font.Size = AdjustRunSummaryFontSize(Font.Size);
		return Font;
	}

	static FT66ButtonParams FlattenRunSummaryButton(FT66ButtonParams Params)
	{
		const int32 BaseButtonFontSize = Params.FontSize > 0 ? Params.FontSize : FT66Style::Tokens::FontButton().Size;
		Params.SetFontSize(AdjustRunSummaryFontSize(BaseButtonFontSize));
		return Params;
	}

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

bool UT66RunSummaryScreen::HasValidLiveRunSummaryContext() const
{
	if (bViewingSavedLeaderboardRunSummary)
	{
		return false;
	}

	const UGameInstance* GI = GetGameInstance();
	const UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	return RunState && (RunState->HasPendingDifficultyClearSummary() || RunState->HasRunEnded());
}

void UT66RunSummaryScreen::PrepareChadCouponsPopupForLiveRun()
{
	SummaryChadCouponsEarned = 0;
	SummaryChadCouponsSourceLabel.Reset();
	SummaryChadCouponsFailureReason.Reset();
	bShowPowerCouponsPopup = false;
	bChadCouponsPopupDontShowAgainChecked = false;

	if (!HasValidLiveRunSummaryContext())
	{
		return;
	}

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	const UT66CommunityContentSubsystem* Community = GI ? GI->GetSubsystem<UT66CommunityContentSubsystem>() : nullptr;
	const int32 PendingCoupons = RunState->GetPendingPowerCrystalsForWallet();
	const bool bDailyClimbRun = T66GI && T66GI->IsDailyClimbRunActive();

	int32 ChallengeRewardCoupons = 0;
	if (!bDailyClimbRun && Community)
	{
		ChallengeRewardCoupons = Community->GetApprovedRewardForActiveChallenge(
			RunState,
			&SummaryChadCouponsSourceLabel,
			&SummaryChadCouponsFailureReason);
	}

	if (RunState->ShouldSuppressPendingPowerCrystalsForWallet())
	{
		const bool bSelectedRunModifier = T66GI && T66GI->SelectedRunModifierKind != ET66RunModifierKind::None;
		UE_LOG(
			LogT66RunSummary,
			Log,
			TEXT("Run Summary: suppressing %d Chad Coupons because %s."),
			PendingCoupons,
			bSelectedRunModifier ? TEXT("a run modifier was selected") : TEXT("the run was flagged locally"));

		RunState->MarkPendingPowerCrystalsSuppressedForWallet();
		if (!bSelectedRunModifier || bDailyClimbRun)
		{
			bChadCouponsResolutionProcessed = true;
			bLiveRunCheatFlagged = true;
			return;
		}
	}

	if (bDailyClimbRun)
	{
		if (PendingCoupons > 0)
		{
			UE_LOG(
				LogT66RunSummary,
				Log,
				TEXT("Run Summary: suppressing %d standard Chad Coupons because Daily Climb rewards are backend-authoritative."),
				PendingCoupons);
			RunState->MarkPendingPowerCrystalsSuppressedForWallet();
		}

		SummaryChadCouponsSourceLabel = TEXT("Daily Climb reward");
		return;
	}

	SummaryChadCouponsEarned = ChallengeRewardCoupons;
	if (SummaryChadCouponsEarned <= 0)
	{
		if (PendingCoupons <= 0 && SummaryChadCouponsFailureReason.IsEmpty())
		{
			SummaryChadCouponsSourceLabel.Reset();
		}
		return;
	}

	if (SummaryChadCouponsSourceLabel.IsEmpty())
	{
		SummaryChadCouponsSourceLabel = TEXT("Challenge reward");
	}
}

void UT66RunSummaryScreen::ResolveChadCouponsPopupForLiveRun(const bool bAllowGrant)
{
	if (bViewingSavedLeaderboardRunSummary || bChadCouponsResolutionProcessed)
	{
		return;
	}

	bChadCouponsResolutionProcessed = true;
	bShowPowerCouponsPopup = false;
	if (!bAllowGrant || SummaryChadCouponsEarned <= 0)
	{
		if (!bAllowGrant)
		{
			if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
			{
				if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
				{
					RunState->MarkPendingPowerCrystalsSuppressedForWallet();
				}
			}
		}
		return;
	}

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PlayerSettings = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	if (!Achievements)
	{
		UE_LOG(LogT66RunSummary, Warning, TEXT("Run Summary: unable to credit %d Chad Coupons because Achievements subsystem was unavailable."), SummaryChadCouponsEarned);
		return;
	}

	Achievements->AddChadCoupons(SummaryChadCouponsEarned);
	if (RunState->GetPendingPowerCrystalsForWallet() > 0)
	{
		RunState->MarkPendingPowerCrystalsGrantedToWallet();
	}
	bShowPowerCouponsPopup = !PlayerSettings || PlayerSettings->GetShowRunSummaryChadCouponsPopup();
}

void UT66RunSummaryScreen::ProcessRunSummaryLeaderboardSubmission(const bool bTreatAsVictoryForTime)
{
	if (bLiveRunSubmissionProcessed || bViewingSavedLeaderboardRunSummary)
	{
		return;
	}

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	if (!GI || !RunState || !LB)
	{
		UE_LOG(LogT66RunSummary, Warning, TEXT("Run Summary: leaderboard submission skipped because required subsystems were unavailable."));
		ResolveChadCouponsPopupForLiveRun(true);
		return;
	}
	if (!HasValidLiveRunSummaryContext())
	{
		UE_LOG(LogT66RunSummary, Log, TEXT("Run Summary: refusing leaderboard submission because there is no finished-run context."));
		bAwaitingBackendRankData = false;
		return;
	}

	FString SavedRunSummarySlotName;
	const bool bSavedSnapshot = LB->SaveFinishedRunSummarySnapshot(SavedRunSummarySlotName);
	if (bDailyClimbSummaryMode)
	{
		if (!Backend || !T66GI || !T66GI->ActiveDailyClimbChallenge.IsValid() || T66GI->ActiveDailyClimbChallenge.AttemptId.IsEmpty())
		{
			UE_LOG(LogT66RunSummary, Warning, TEXT("Run Summary: Daily Climb submission skipped because the active daily context was incomplete."));
			bAwaitingBackendRankData = false;
			ResolveChadCouponsPopupForLiveRun(false);
			bLiveRunSubmissionProcessed = true;
			return;
		}

		UT66LeaderboardRunSummarySaveGame* Snapshot = nullptr;
		if (bSavedSnapshot && !SavedRunSummarySlotName.IsEmpty())
		{
			if (USaveGame* LoadedSnapshot = UGameplayStatics::LoadGameFromSlot(SavedRunSummarySlotName, 0))
			{
				Snapshot = Cast<UT66LeaderboardRunSummarySaveGame>(LoadedSnapshot);
			}
		}

		if (!Snapshot)
		{
			UE_LOG(LogT66RunSummary, Warning, TEXT("Run Summary: Daily Climb submission skipped because the run summary snapshot could not be loaded."));
			bAwaitingBackendRankData = false;
			ResolveChadCouponsPopupForLiveRun(false);
			bLiveRunSubmissionProcessed = true;
			return;
		}

		const FString DisplayName = !T66GI->CurrentRunOwnerDisplayName.IsEmpty()
			? T66GI->CurrentRunOwnerDisplayName
			: TEXT("Player");
		const FString DailyRequestKey = FString::Printf(TEXT("daily_submit_%s"), *T66GI->ActiveDailyClimbChallenge.ChallengeId);

		Backend->SubmitDailyClimbRun(
			DisplayName,
			Snapshot,
			T66GI->ActiveDailyClimbChallenge.ChallengeId,
			T66GI->ActiveDailyClimbChallenge.AttemptId,
			DailyRequestKey);

		UE_LOG(
			LogT66RunSummary,
			Log,
			TEXT("Run Summary: submitted Daily Climb result snapshotSaved=%d slot=%s challenge=%s attempt=%s stage=%d score=%d"),
			bSavedSnapshot ? 1 : 0,
			SavedRunSummarySlotName.IsEmpty() ? TEXT("<none>") : *SavedRunSummarySlotName,
			*T66GI->ActiveDailyClimbChallenge.ChallengeId,
			*T66GI->ActiveDailyClimbChallenge.AttemptId,
			RunState->GetCurrentStage(),
			RunState->GetCurrentScore());

		bNewPersonalBestScore = false;
		bNewPersonalBestTime = false;
		bAwaitingBackendRankData = true;
		bLiveRunSubmissionProcessed = true;
		return;
	}

	const bool bShouldSubmitTime =
		(RunState->DidRunEndInVictory() || bTreatAsVictoryForTime)
		&& PS
		&& PS->GetSpeedRunMode();
	bool bSubmittedTime = false;
	bool bSubmittedScore = false;
	if (bShouldSubmitTime)
	{
		bSubmittedTime = LB->SubmitDifficultyClearRun(RunState->GetFinalRunElapsedSeconds(), SavedRunSummarySlotName);
		bSubmittedScore = bSubmittedTime;
		bNewPersonalBestScore = LB->WasLastScoreNewPersonalBest();
		bNewPersonalBestTime = LB->WasLastCompletedRunTimeNewPersonalBest();
	}
	else
	{
		bSubmittedScore = LB->SubmitRunScore(RunState->GetCurrentScore(), SavedRunSummarySlotName);
		bNewPersonalBestScore = LB->WasLastScoreNewPersonalBest();
		bNewPersonalBestTime = false;
	}

	UE_LOG(
		LogT66RunSummary,
		Log,
		TEXT("Run Summary: leaderboard submission snapshotSaved=%d scoreSubmitted=%d timeSubmitted=%d slot=%s stage=%d score=%d treatAsVictoryForTime=%d"),
		bSavedSnapshot ? 1 : 0,
		bSubmittedScore ? 1 : 0,
		bSubmittedTime ? 1 : 0,
		SavedRunSummarySlotName.IsEmpty() ? TEXT("<none>") : *SavedRunSummarySlotName,
		RunState->GetCurrentStage(),
		RunState->GetCurrentScore(),
		bTreatAsVictoryForTime ? 1 : 0);

	if (!bSubmittedScore && !bSubmittedTime)
	{
		bAwaitingBackendRankData = false;
		ResolveChadCouponsPopupForLiveRun(true);
	}
	else
	{
		bAwaitingBackendRankData = true;
	}

	bLiveRunSubmissionProcessed = true;
}

void UT66RunSummaryScreen::ProcessLiveRunFinalSubmission()
{
	if (bLiveRunFinalAccountingProcessed || bViewingSavedLeaderboardRunSummary)
	{
		return;
	}

	ProcessRunSummaryLeaderboardSubmission(false);
	ProcessLiveRunFinalAccounting();
}

void UT66RunSummaryScreen::ProcessLiveRunFinalAccounting()
{
	if (bLiveRunFinalAccountingProcessed || bViewingSavedLeaderboardRunSummary)
	{
		return;
	}
	if (!HasValidLiveRunSummaryContext())
	{
		UE_LOG(LogT66RunSummary, Log, TEXT("Run Summary: skipping final accounting because there is no finished-run context."));
		return;
	}

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	if (!RunState || !Achievements || !T66GI)
	{
		UE_LOG(LogT66RunSummary, Warning, TEXT("Run Summary: final accounting skipped because required subsystems were unavailable."));
		return;
	}

	if (!T66GI->SelectedHeroID.IsNone())
	{
		Achievements->AddHeroGamesPlayed(T66GI->SelectedHeroID, 1);
		Achievements->AddHeroCumulativeScore(T66GI->SelectedHeroID, RunState->GetCurrentScore());
	}
	if (!T66GI->SelectedCompanionID.IsNone())
	{
		Achievements->AddCompanionGamesPlayed(T66GI->SelectedCompanionID, 1);
		Achievements->AddCompanionCumulativeScore(T66GI->SelectedCompanionID, RunState->GetCurrentScore());
		Achievements->AddCompanionTotalHealing(
			T66GI->SelectedCompanionID,
			FMath::RoundToInt(RunState->GetCompanionHealingDoneThisRun()));
	}

	if (RunState->DidRunEndInVictory())
	{
		if (!T66GI->SelectedHeroID.IsNone())
		{
			Achievements->RecordHeroDifficultyClear(T66GI->SelectedHeroID, T66GI->SelectedDifficulty);
		}
		if (!T66GI->SelectedCompanionID.IsNone())
		{
			Achievements->RecordCompanionDifficultyClear(T66GI->SelectedCompanionID, T66GI->SelectedDifficulty);
		}
	}

	bLiveRunFinalAccountingProcessed = true;
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
		if (!bStoredHeroPreviewStageVisibility)
		{
			bHeroPreviewStageWasVisible = HeroPreviewStage->IsStageVisible();
			bStoredHeroPreviewStageVisibility = true;
		}

		HeroPreviewStage->SetStageVisible(true);
		HeroPreviewStage->SetPreviewStageMode(ET66PreviewStageMode::RunSummary);

		const ET66Difficulty PreviewDifficulty =
			(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->Difficulty : GI->SelectedDifficulty;
		const FName HeroID =
			(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->HeroID :
			(!GI->SelectedHeroID.IsNone() ? GI->SelectedHeroID :
				(GI->GetAllHeroIDs().Num() > 0 ? GI->GetAllHeroIDs()[0] : NAME_None));

		const ET66BodyType BodyType =
			(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->HeroBodyType : GI->SelectedHeroBodyType;
		const FName CompanionID =
			(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->CompanionID : GI->SelectedCompanionID;
		FName SkinID = GI->SelectedHeroSkinID.IsNone() ? FName(TEXT("Default")) : GI->SelectedHeroSkinID;
		if (!HeroID.IsNone())
		{
			HeroPreviewStage->SetPreviewDifficulty(PreviewDifficulty);
			HeroPreviewStage->SetPreviewHero(HeroID, BodyType, SkinID, CompanionID);
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
	if (HeroPreviewStage)
	{
		HeroPreviewStage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		if (bStoredHeroPreviewStageVisibility)
		{
			HeroPreviewStage->SetStageVisible(bHeroPreviewStageWasVisible);
		}
	}
	bStoredHeroPreviewStageVisibility = false;
	bHeroPreviewStageWasVisible = true;

	HeroPreviewBrush.Reset();
	if (HeroCaptureActor)
	{
		HeroCaptureActor->Destroy();
		HeroCaptureActor = nullptr;
	}
	HeroPreviewRT = nullptr;
}

void UT66RunSummaryScreen::ResetSavedRunSummaryViewerState()
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
	if (LB->ConsumePendingRunSummaryRequest(SlotName))
	{
		// We are consuming an explicit saved-slot request: clear previous snapshot state now.
		ResetSavedRunSummaryViewerState();

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

	// No explicit slot request; fall back to in-memory fake snapshot (non-local leaderboard rows).
	UT66LeaderboardRunSummarySaveGame* FakeSnap = LB->ConsumePendingFakeRunSummarySnapshot();
	if (FakeSnap)
	{
		ResetSavedRunSummaryViewerState();
		LoadedSavedSummary = FakeSnap;
		bViewingSavedLeaderboardRunSummary = true;
		if (LoadedSavedSummary->SchemaVersion >= 3)
		{
			ProofOfRunUrl = LoadedSavedSummary->ProofOfRunUrl;
			bProofOfRunLocked = LoadedSavedSummary->bProofOfRunLocked;
		}
		return true;
	}

	// No pending request: do not clobber any existing viewer state.
	return false;
}

TSharedRef<SWidget> UT66RunSummaryScreen::RebuildWidget()
{
	// Critical: Slate is built before OnScreenActivated() (AddToViewport/TakeWidget).
	// Load any pending leaderboard snapshot here so the first render is correct (no "empty run" flash).
	LoadSavedRunSummaryIfRequested();
	if (!bViewingSavedLeaderboardRunSummary)
	{
		UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
		UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		bDifficultyClearSummaryMode = RunState && RunState->HasPendingDifficultyClearSummary();
	}
	else
	{
		bDifficultyClearSummaryMode = false;
	}
	return FT66Style::MakeResponsiveRoot(BuildSlateUI());
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
		UWorld* World = GetWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		if (RunState)
		{
			LogPtr = &RunState->GetEventLog();
		}
	}

	const TArray<FString>& Log = LogPtr ? *LogPtr : TArray<FString>();

	LogItems.Reserve(Log.Num());
	for (const FString& Entry : Log)
	{
		if (T66LeaderboardPacing::IsStageMarker(Entry))
		{
			continue;
		}
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
			.Font(RunSummaryRegularFont(12))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		];
}

TSharedRef<SWidget> UT66RunSummaryScreen::BuildSlateUI()
{
	UWorld* World = GetWorld();
	UGameInstance* GIBase = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GIBase ? GIBase->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const UT66GameInstance* T66GIBase = Cast<UT66GameInstance>(GIBase);
	const ET66Difficulty SummaryDifficulty =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->Difficulty :
		(T66GIBase ? T66GIBase->SelectedDifficulty : ET66Difficulty::Easy);
	const ET66PartySize SummaryPartySize =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->PartySize :
		(T66GIBase ? T66GIBase->SelectedPartySize : ET66PartySize::Solo);
	const UT66PlayerExperienceSubSystem* PlayerExperience = GIBase ? GIBase->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	const int32 StageReached =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->StageReached :
		(RunState ? RunState->GetCurrentStage() : 1);
	const int32 DisplayStageReached = GetDisplayedRunSummaryStageNumber(PlayerExperience, SummaryDifficulty, StageReached);

	const int32 DisplayScore =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->Score :
		(RunState ? RunState->GetCurrentScore() : 0);
	const float DisplaySeconds =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->RunDurationSeconds :
		(RunState ? RunState->GetFinalRunElapsedSeconds() : 0.f);

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

	const int32 AccuracyStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->AccuracyStat :
		(RunState ? RunState->GetAccuracyStat() : 1);

	const int32 ArmorStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->ArmorStat :
		(RunState ? RunState->GetArmorStat() : 1);

	const int32 EvasionStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->EvasionStat :
		(RunState ? RunState->GetEvasionStat() : 1);

	const int32 LuckStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->LuckStat :
		(RunState ? RunState->GetLuckStat() : 1);

	UT66LocalizationSubsystem* Loc = GIBase ? GIBase->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66GameInstance* GI = Cast<UT66GameInstance>(GIBase);
	UT66UITexturePoolSubsystem* TexPool = GIBase ? GIBase->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

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

	const FText TitleText = bDailyClimbSummaryMode
		? NSLOCTEXT("T66.RunSummary", "DailyTitle", "DAILY CLIMB SUMMARY")
		: (Loc ? Loc->GetText_RunSummaryTitle() : NSLOCTEXT("T66.RunSummary", "Title", "RUN SUMMARY"));
	const FText StageScoreText = FText::Format(
		NSLOCTEXT("T66.RunSummary", "StageReachedScoreTimeFormat", "Stage Reached: {0}  |  Score: {1}  |  Time: {2}"),
		FText::AsNumber(DisplayStageReached),
		FText::AsNumber(DisplayScore),
		FormatRunSummaryDurationText(DisplaySeconds));

	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	const bool bDotaTheme = FT66Style::IsDotaTheme();
	const FText ScoreRankLabelText = NSLOCTEXT("T66.RunSummary", "ScoreRankLabel", "Score");
	const FText SpeedRunRankLabelText = NSLOCTEXT("T66.RunSummary", "SpeedRunRankLabel", "Speed Run");
	const TWeakObjectPtr<UT66RunSummaryScreen> WeakScreen(this);
	const TWeakObjectPtr<UT66LeaderboardSubsystem> WeakLB(LB);

	auto MakeSectionPanel = [](const FText& Header, const TSharedRef<SWidget>& Body) -> TSharedRef<SWidget>
	{
		return MakeRunSummarySpritePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(STextBlock)
				.Text(Header)
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
				.Font(RunSummaryHeadingFont())
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				Body
			],
			GetRunSummaryRowShellBrush(),
			FT66Style::Tokens::Space4);
	};

	auto ResolveDisplayedRank = [WeakScreen, WeakLB, SummaryDifficulty, SummaryPartySize](bool bScoreRank, bool bWeeklyRank) -> int32
	{
		if (!WeakScreen.IsValid())
		{
			return 0;
		}

		const UT66RunSummaryScreen* Screen = WeakScreen.Get();
		if (Screen->bViewingSavedLeaderboardRunSummary && Screen->LoadedSavedSummary)
		{
			if (bScoreRank)
			{
				return bWeeklyRank ? Screen->LoadedSavedSummary->ScoreRankWeekly : Screen->LoadedSavedSummary->ScoreRankAllTime;
			}

			return bWeeklyRank ? Screen->LoadedSavedSummary->SpeedRunRankWeekly : Screen->LoadedSavedSummary->SpeedRunRankAllTime;
		}

		if (Screen->bBackendRankDataReceived)
		{
			if (bScoreRank)
			{
				return bWeeklyRank ? Screen->BackendScoreRankWeekly : Screen->BackendScoreRankAllTime;
			}

			return bWeeklyRank ? Screen->BackendSpeedRunRankWeekly : Screen->BackendSpeedRunRankAllTime;
		}

		if (!WeakLB.IsValid())
		{
			return 0;
		}

		if (bScoreRank)
		{
			return bWeeklyRank
				? WeakLB->GetLocalScoreRankWeekly(SummaryDifficulty, SummaryPartySize)
				: WeakLB->GetLocalScoreRankAllTime(SummaryDifficulty, SummaryPartySize);
		}

		return bWeeklyRank
			? WeakLB->GetLocalSpeedRunRankWeekly(SummaryDifficulty, SummaryPartySize)
			: WeakLB->GetLocalSpeedRunRankAllTime(SummaryDifficulty, SummaryPartySize);
	};

	auto GetRankLoadingVisibility = [WeakScreen]() -> EVisibility
	{
		if (!WeakScreen.IsValid())
		{
			return EVisibility::Collapsed;
		}

		const UT66RunSummaryScreen* Screen = WeakScreen.Get();
		return (!Screen->bViewingSavedLeaderboardRunSummary && Screen->bAwaitingBackendRankData)
			? EVisibility::Visible
			: EVisibility::Collapsed;
	};

	auto GetRankContentVisibility = [WeakScreen]() -> EVisibility
	{
		if (!WeakScreen.IsValid())
		{
			return EVisibility::Visible;
		}

		const UT66RunSummaryScreen* Screen = WeakScreen.Get();
		return (!Screen->bViewingSavedLeaderboardRunSummary && Screen->bAwaitingBackendRankData)
			? EVisibility::Collapsed
			: EVisibility::Visible;
	};

	auto MakeRankValuePair = [](const FText& Label, TAttribute<FText> RankText) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(RunSummaryRegularFont(14))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(RankText)
				.Font(RunSummaryBoldFont(14))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];
	};

	auto MakeRankSectionPanel = [&](const FText& Header, TAttribute<FText> ScoreRankText, TAttribute<FText> SpeedRunRankText) -> TSharedRef<SWidget>
	{
		return MakeRunSummarySpritePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(STextBlock)
				.Text(Header)
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
				.Font(RunSummaryHeadingFont())
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SHorizontalBox)
					.Visibility_Lambda(GetRankContentVisibility)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 24.f, 0.f)
					[
						MakeRankValuePair(ScoreRankLabelText, ScoreRankText)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakeRankValuePair(SpeedRunRankLabelText, SpeedRunRankText)
					]
				]
				+ SOverlay::Slot()
				[
					SNew(SHorizontalBox)
					.Visibility_Lambda(GetRankLoadingVisibility)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 2.f, 8.f, 2.f)
					[
						SNew(SThrobber)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.RunSummary", "RanksLoading", "Updating ranks..."))
						.Font(RunSummaryRegularFont(14))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					]
				]
			],
			GetRunSummaryRowShellBrush(),
			FT66Style::Tokens::Space4);
	};

	auto MakeRatingSectionPanel = [](const FText& Header, const TSharedRef<SWidget>& Body) -> TSharedRef<SWidget>
	{
		FSlateFontInfo SmallerHeadingFont = RunSummaryHeadingFont();
		SmallerHeadingFont.Size = FMath::Max(SmallerHeadingFont.Size - 6, 8);

		return MakeRunSummarySpritePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(STextBlock)
				.Text(Header)
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
				.Font(SmallerHeadingFont)
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				Body
			],
			GetRunSummaryRowShellBrush(),
			FT66Style::Tokens::Space4);
	};

	TSharedRef<SWidget> WeeklyRankPanel = MakeRankSectionPanel(
		NSLOCTEXT("T66.RunSummary", "WeeklyRankHeader", "Weekly Rank"),
		TAttribute<FText>::CreateLambda([ResolveDisplayedRank]() { return FormatRunSummaryRankText(ResolveDisplayedRank(true, true)); }),
		TAttribute<FText>::CreateLambda([ResolveDisplayedRank]() { return FormatRunSummaryRankText(ResolveDisplayedRank(false, true)); }));
	TSharedRef<SWidget> AllTimeRankPanel = MakeRankSectionPanel(
		NSLOCTEXT("T66.RunSummary", "AllTimeRankHeader", "All Time Rank"),
		TAttribute<FText>::CreateLambda([ResolveDisplayedRank]() { return FormatRunSummaryRankText(ResolveDisplayedRank(true, false)); }),
		TAttribute<FText>::CreateLambda([ResolveDisplayedRank]() { return FormatRunSummaryRankText(ResolveDisplayedRank(false, false)); }));
	TSharedRef<SWidget> DailyRankPanel =
		MakeRunSummarySpritePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.RunSummary", "DailyRankHeader", "Daily Rank"))
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
				.Font(RunSummaryHeadingFont())
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SHorizontalBox)
					.Visibility_Lambda(GetRankContentVisibility)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 24.f, 0.f)
					[
						MakeRankValuePair(
							NSLOCTEXT("T66.RunSummary", "DailyRankLabel", "Score"),
							TAttribute<FText>::CreateLambda([WeakScreen]()
							{
								if (!WeakScreen.IsValid())
								{
									return FormatRunSummaryRankText(0);
								}

								const UT66RunSummaryScreen* Screen = WeakScreen.Get();
								return FormatRunSummaryRankText(Screen->BackendDailyScoreRank);
							}))
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakeRankValuePair(
							NSLOCTEXT("T66.RunSummary", "DailyRewardLabel", "Reward"),
							TAttribute<FText>::CreateLambda([WeakScreen]()
							{
								if (!WeakScreen.IsValid())
								{
									return FText::Format(
										NSLOCTEXT("T66.RunSummary", "DailyRewardValue", "{0} Coupons"),
										FText::AsNumber(0));
								}

								const UT66RunSummaryScreen* Screen = WeakScreen.Get();
								return FText::Format(
									NSLOCTEXT("T66.RunSummary", "DailyRewardValue", "{0} Coupons"),
									FText::AsNumber(FMath::Max(0, Screen->SummaryChadCouponsEarned)));
							}))
					]
				]
				+ SOverlay::Slot()
				[
					SNew(SHorizontalBox)
					.Visibility_Lambda(GetRankLoadingVisibility)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 2.f, 8.f, 2.f)
					[
						SNew(SThrobber)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.RunSummary", "DailyRanksLoading", "Submitting Daily Climb..."))
						.Font(RunSummaryRegularFont(14))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					]
				]
			],
			GetRunSummaryRowShellBrush(),
			FT66Style::Tokens::Space4);

	auto MakeHeroPreview = [bDotaTheme](const TSharedPtr<FSlateBrush>& Brush) -> TSharedRef<SWidget>
	{
		constexpr float PreviewSize = 220.f;
		TSharedRef<SWidget> PreviewContent = Brush.IsValid()
			? StaticCastSharedRef<SWidget>(SNew(SBox)
				.WidthOverride(PreviewSize)
				.HeightOverride(PreviewSize)
				[
					SNew(SImage).Image(Brush.Get())
				])
			: StaticCastSharedRef<SWidget>(SNew(SBox)
				.WidthOverride(PreviewSize)
				.HeightOverride(PreviewSize)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.RunSummary", "NoPreview", "No Preview"))
					.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
					.Font(RunSummaryBodyFont())
					.Justification(ETextJustify::Center)
				]);
		return bDotaTheme
			? StaticCastSharedRef<SWidget>(FT66Style::MakeViewportFrame(PreviewContent, FMargin(8.f)))
			: StaticCastSharedRef<SWidget>(MakeRunSummarySpritePanel(PreviewContent, GetRunSummaryRowShellBrush(), FMargin(0.f)));
	};

	// Event log (hidden by default; opened via "EVENT LOG" button).
	TSharedRef<SWidget> EventLogPanel =
		MakeSectionPanel(
			NSLOCTEXT("T66.RunSummary", "EventLogTitle", "EVENT LOG"),
			MakeRunSummarySpritePanel(
				LogListView.IsValid() ? StaticCastSharedRef<SWidget>(LogListView.ToSharedRef()) : StaticCastSharedRef<SWidget>(SNew(SSpacer)),
				GetRunSummaryRowShellBrush(),
				FT66Style::Tokens::Space2
			)
		);

	// Proof of run UI is only editable by the owner of the viewed backend run
	// or when we are viewing a locally saved snapshot.
	bool bIsOwnerOfViewedRun = !bViewingSavedLeaderboardRunSummary;
	if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
	{
		bIsOwnerOfViewedRun = !LoadedSavedSummarySlotName.IsEmpty();
		if (!bIsOwnerOfViewedRun)
		{
			if (UGameInstance* LocalGI = GetGameInstance())
			{
				if (UT66SteamHelper* SteamHelper = LocalGI->GetSubsystem<UT66SteamHelper>())
				{
					const FString LocalSteamId = SteamHelper->GetLocalSteamId();
					bIsOwnerOfViewedRun = !LocalSteamId.IsEmpty() && LocalSteamId == LoadedSavedSummary->OwnerSteamId;
				}
			}
		}
	}

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
				SNew(SBox)
				.Visibility(TAttribute<EVisibility>::CreateLambda([this, bIsOwnerOfViewedRun]()
				{
					const bool bBackendLockedEntry = LoadedSavedSummary && !LoadedSavedSummary->EntryId.IsEmpty();
					return (bIsOwnerOfViewedRun && bProofOfRunLocked && !bBackendLockedEntry) ? EVisibility::Visible : EVisibility::Collapsed;
				}))
				[
					MakeRunSummarySpriteButton(NSLOCTEXT("T66.RunSummary", "Edit", "EDIT"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleProofEditClicked), ET66RunSummaryButtonFamily::CompactNeutral, 86.f, 34.f, 12, FMargin(14.f, 8.f))
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
				MakeRunSummarySpriteButton(NSLOCTEXT("T66.RunSummary", "Confirm", "CONFIRM"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleProofConfirmClicked), ET66RunSummaryButtonFamily::ToggleOn, 120.f, 34.f, 12, FMargin(14.f, 8.f))
			]
		];

	TSharedRef<SWidget> ProofOfRunPanel =
		MakeSectionPanel(
			NSLOCTEXT("T66.RunSummary", "ProofOfRunHeader", "PROOF OF RUN"),
			ProofBody
		);

	// Cheat report UI (available to everyone).
	TSharedRef<SWidget> ReportCheatButton =
		MakeRunSummarySpriteButton(
			NSLOCTEXT("T66.RunSummary", "TheyreCheating", "THEY'RE CHEATING"),
			FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleReportCheatingClicked),
			ET66RunSummaryButtonFamily::ToggleOff,
			180.f,
			38.f,
			12,
			FMargin(14.f, 10.f));

	TSharedRef<SWidget> ReportPrompt =
		MakeRunSummarySpritePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.RunSummary", "ReportReasonHeader", "REASON"))
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
				.Font(RunSummaryHeadingFont())
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
					MakeRunSummarySpriteButton(NSLOCTEXT("T66.RunSummary", "Submit", "SUBMIT"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleReportSubmitClicked), ET66RunSummaryButtonFamily::ToggleOn, 110.f, 34.f, 12, FMargin(14.f, 8.f))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f)
				[
					MakeRunSummarySpriteButton(NSLOCTEXT("T66.RunSummary", "Close", "CLOSE"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleReportCloseClicked), ET66RunSummaryButtonFamily::CompactNeutral, 110.f, 34.f, 12, FMargin(14.f, 8.f))
				]
			]
		,
		GetRunSummaryRowShellBrush(),
		FT66Style::Tokens::Space3);

	auto MakeEventLogButton = [&](const FText& Text) -> TSharedRef<SWidget>
	{
		return MakeRunSummarySpriteButton(
			Text,
			FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleViewLogClicked),
			ET66RunSummaryButtonFamily::CompactNeutral,
			160.f,
			36.f,
			12,
			FMargin(14.f, 8.f));
	};

	// Stats panel: same width and content as Vendor/Gambler (primary + secondary, scroll). Header "STATS".
	const float StatsPanelWidth = FT66Style::Tokens::NPCVendorStatsPanelWidth;
	TSharedRef<SWidget> BaseStatsPanel = [&]() -> TSharedRef<SWidget>
	{
		if (RunState && !bViewingSavedLeaderboardRunSummary)
		{
			return T66StatsPanelSlate::MakeEssentialStatsPanel(RunState, Loc, StatsPanelWidth, false, T66RunSummaryFontDelta);
		}
		if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary && LoadedSavedSummary->SecondaryStatValues.Num() > 0)
		{
			T66StatsPanelSlate::FT66SnapshotStatsPanelOptions RunSummaryStatsOptions;
			RunSummaryStatsOptions.WidthOverride = StatsPanelWidth;
			RunSummaryStatsOptions.FontSizeAdjustment = T66RunSummaryFontDelta;
			RunSummaryStatsOptions.bExtended = false;
			return T66StatsPanelSlate::MakeEssentialStatsPanelFromSnapshotWithOptions(LoadedSavedSummary, Loc, RunSummaryStatsOptions);
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
				.Font(RunSummaryBodyFont())
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];
		};
		AddStatLine(Loc ? Loc->GetText_Level() : NSLOCTEXT("T66.Common", "Level", "LEVEL"), HeroLevel);
		AddStatLine(Loc ? Loc->GetText_Stat_Damage() : NSLOCTEXT("T66.Stats", "Damage", "Damage"), DamageStat);
		AddStatLine(Loc ? Loc->GetText_Stat_AttackSpeed() : NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed"), AttackSpeedStat);
		AddStatLine(Loc ? Loc->GetText_Stat_AttackScale() : NSLOCTEXT("T66.Stats", "AttackScale", "Attack Scale"), AttackScaleStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Accuracy() : NSLOCTEXT("T66.Stats", "Accuracy", "Accuracy"), AccuracyStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Armor() : NSLOCTEXT("T66.Stats", "Armor", "Armor"), ArmorStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Evasion() : NSLOCTEXT("T66.Stats", "Evasion", "Evasion"), EvasionStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Luck() : NSLOCTEXT("T66.Stats", "Luck", "Luck"), LuckStat);
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
						.Font(RunSummaryHeadingFont())
					]
					+ SVerticalBox::Slot().AutoHeight()[PrimaryContent],
					FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4)
				)
			];
	}();

	// Idols: one row, 6 columns, with simple border under hero.
	const TArray<FName>* IdolsPtr = nullptr;
	const TArray<uint8>* IdolTiersPtr = nullptr;
	TArray<FName> InventoryLocal;
	const TArray<FT66InventorySlot>* InvSlotsPtr = nullptr;
	TArray<ET66SecondaryStatType> TemporaryBuffSlots;
	if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
	{
		IdolsPtr = &LoadedSavedSummary->EquippedIdols;
		IdolTiersPtr = &LoadedSavedSummary->EquippedIdolTiers;
		InventoryLocal = LoadedSavedSummary->Inventory;
		TemporaryBuffSlots = LoadedSavedSummary->TemporaryBuffSlots;
	}
	else if (RunState)
	{
		if (UT66IdolManagerSubsystem* IdolManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr)
		{
			IdolsPtr = &IdolManager->GetEquippedIdols();
			IdolTiersPtr = &IdolManager->GetEquippedIdolTierValues();
		}
		else
		{
			IdolsPtr = &RunState->GetEquippedIdols();
			IdolTiersPtr = &RunState->GetEquippedIdolTierValues();
		}
		InventoryLocal = RunState->GetInventory();
		InvSlotsPtr = &RunState->GetInventorySlots();
		if (UT66BuffSubsystem* Buffs = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BuffSubsystem>() : nullptr)
		{
			TemporaryBuffSlots = Buffs->GetSelectedSingleUseBuffSlots();
		}
	}
	const TArray<FName> Empty;
	const TArray<FName>& Idols = IdolsPtr ? *IdolsPtr : Empty;
	const TArray<uint8> EmptyIdolTiers;
	const TArray<uint8>& IdolTiers = IdolTiersPtr ? *IdolTiersPtr : EmptyIdolTiers;

	static constexpr float IdolSlotPad = 3.f;
	static constexpr float IdolSlotSize = 46.f;
	TSharedRef<SHorizontalBox> IdolSlotsRow = SNew(SHorizontalBox);
	for (int32 i = 0; i < UT66IdolManagerSubsystem::MaxEquippedIdolSlots; ++i)
	{
		const FName IdolID = Idols.IsValidIndex(i) ? Idols[i] : NAME_None;
		const int32 IdolTierValue = IdolTiers.IsValidIndex(i)
			? FMath::Clamp(static_cast<int32>(IdolTiers[i]), 1, UT66IdolManagerSubsystem::MaxIdolLevel)
			: 1;
		const FLinearColor IdolColor = !IdolID.IsNone()
			? FItemData::GetItemRarityColor(UT66IdolManagerSubsystem::IdolTierValueToRarity(IdolTierValue))
			: FLinearColor(0.45f, 0.55f, 0.50f, 0.5f);
		FIdolData IdolData;
		const bool bHasIdolData = GI && !IdolID.IsNone() && GI->GetIdolData(IdolID, IdolData);
		TSharedPtr<FSlateBrush> IdolBrush;
		const TSoftObjectPtr<UTexture2D> IdolIconSoft = bHasIdolData
			? IdolData.GetIconForRarity(UT66IdolManagerSubsystem::IdolTierValueToRarity(IdolTierValue))
			: TSoftObjectPtr<UTexture2D>();
		if (!IdolIconSoft.IsNull())
		{
			IdolBrush = MakeShared<FSlateBrush>();
			IdolBrush->DrawAs = ESlateBrushDrawType::Image;
			IdolBrush->ImageSize = FVector2D(IdolSlotSize - 4.f, IdolSlotSize - 4.f);
			IdolIconBrushes.Add(IdolBrush);
			if (TexPool) T66SlateTexture::BindSharedBrushAsync(TexPool, IdolIconSoft, this, IdolBrush, IdolID, true);
		}
		IdolSlotsRow->AddSlot()
			.AutoWidth()
			.Padding(IdolSlotPad)
			[
				SNew(SBox).WidthOverride(IdolSlotSize).HeightOverride(IdolSlotSize)
				[
					bDotaTheme
						? StaticCastSharedRef<SWidget>(FT66Style::MakeSlotFrame(
							IdolBrush.IsValid()
								? StaticCastSharedRef<SWidget>(SNew(SImage).Image(IdolBrush.Get()))
								: StaticCastSharedRef<SWidget>(SNew(SSpacer)),
							IdolColor,
							FMargin(1.f)))
						: StaticCastSharedRef<SWidget>(SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(IdolColor)
							.Padding(1.f)
							[
								IdolBrush.IsValid()
								? StaticCastSharedRef<SWidget>(SNew(SImage).Image(IdolBrush.Get()))
								: StaticCastSharedRef<SWidget>(SNew(SSpacer))
							])
				]
			];
	}
	TSharedRef<SWidget> IdolsBorderedGrid = bDotaTheme
		? StaticCastSharedRef<SWidget>(FT66Style::MakeScreenSurface(IdolSlotsRow, FMargin(4.f)))
		: StaticCastSharedRef<SWidget>(SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66Style::Tokens::Stroke)
			.Padding(4.f)
			[
				IdolSlotsRow
			]);

	// Inventory: slot grid (2x10), sprites only — larger slots than in-game.
	static constexpr int32 InvCols = 10;
	static constexpr int32 InvRows = 2;
	static constexpr float InvSlotSize = 44.f;
	static constexpr float InvSlotPad = 2.f;
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
		const ET66ItemRarity SlotRarity = (InvSlotsPtr && InvSlotsPtr->IsValidIndex(InvIdx)) ? (*InvSlotsPtr)[InvIdx].Rarity : ET66ItemRarity::Black;
		const bool bHasData = GI && GI->GetItemData(ItemID, ItemData);
		const TSoftObjectPtr<UTexture2D> ItemIconSoft = bHasData ? ItemData.GetIconForRarity(SlotRarity) : TSoftObjectPtr<UTexture2D>();
		if (!ItemIconSoft.IsNull() && TexPool)
		{
			T66SlateTexture::BindSharedBrushAsync(TexPool, ItemIconSoft, this, InventoryItemIconBrushes[InvIdx], ItemID, true);
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
						bDotaTheme
							? StaticCastSharedRef<SWidget>(FT66Style::MakeSlotFrame(
								(SlotBrush.IsValid() && bHasItem)
									? StaticCastSharedRef<SWidget>(SNew(SImage).Image(SlotBrush.Get()))
									: StaticCastSharedRef<SWidget>(SNew(SSpacer)),
								SlotColor,
								FMargin(0.f)))
							: StaticCastSharedRef<SWidget>(SNew(SBorder)
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
								])
					]
				];
		}
		InvGridRef->AddSlot().AutoHeight()[RowBox];
	}
	TSharedRef<SWidget> InventorySlotGrid = bDotaTheme
		? StaticCastSharedRef<SWidget>(FT66Style::MakeScreenSurface(InvGridRef, FMargin(4.f)))
		: StaticCastSharedRef<SWidget>(SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66Style::Tokens::Stroke)
			.Padding(4.f)
			[
				InvGridRef
			]);

	static constexpr float TempBuffSlotSize = 40.f;
	static constexpr float TempBuffSlotPad = 3.f;
	TemporaryBuffIconBrushes.Reset();
	TemporaryBuffIconBrushes.SetNum(UT66BuffSubsystem::MaxSelectedSingleUseBuffs);

	TSharedRef<SHorizontalBox> TemporaryBuffSlotsRow = SNew(SHorizontalBox);
	for (int32 SlotIndex = 0; SlotIndex < UT66BuffSubsystem::MaxSelectedSingleUseBuffs; ++SlotIndex)
	{
		const ET66SecondaryStatType SlotStat = TemporaryBuffSlots.IsValidIndex(SlotIndex)
			? TemporaryBuffSlots[SlotIndex]
			: ET66SecondaryStatType::None;
		const bool bHasTemporaryBuff = T66IsLiveSecondaryStatType(SlotStat);
		TSharedPtr<FSlateBrush> TempBuffBrush = bHasTemporaryBuff
			? T66TemporaryBuffUI::CreateSecondaryBuffBrush(TexPool, this, SlotStat, FVector2D(TempBuffSlotSize - 6.f, TempBuffSlotSize - 6.f))
			: nullptr;
		TemporaryBuffIconBrushes[SlotIndex] = TempBuffBrush;
		const FText SlotTooltip = bHasTemporaryBuff
			? (Loc ? Loc->GetText_SecondaryStatName(SlotStat) : FText::FromString(TEXT("Temporary Buff")))
			: NSLOCTEXT("T66.RunSummary", "EmptyTemporaryBuffSlot", "Empty temporary buff slot");

		TemporaryBuffSlotsRow->AddSlot()
			.AutoWidth()
			.Padding(TempBuffSlotPad)
			[
				SNew(SBox)
				.WidthOverride(TempBuffSlotSize)
				.HeightOverride(TempBuffSlotSize)
				.ToolTipText(SlotTooltip)
				[
					bDotaTheme
						? StaticCastSharedRef<SWidget>(FT66Style::MakeSlotFrame(
							(bHasTemporaryBuff && TempBuffBrush.IsValid())
								? StaticCastSharedRef<SWidget>(SNew(SImage).Image(TempBuffBrush.Get()))
								: StaticCastSharedRef<SWidget>(SNew(SSpacer)),
							bHasTemporaryBuff ? FLinearColor(0.36f, 0.72f, 0.46f, 1.f) : FLinearColor(0.45f, 0.55f, 0.50f, 0.5f),
							FMargin(1.f)))
						: StaticCastSharedRef<SWidget>(SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(bHasTemporaryBuff ? FLinearColor(0.36f, 0.72f, 0.46f, 1.f) : FLinearColor(0.45f, 0.55f, 0.50f, 0.5f))
							.Padding(1.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.25f))
								.Padding(1.f)
								[
									(bHasTemporaryBuff && TempBuffBrush.IsValid())
										? StaticCastSharedRef<SWidget>(SNew(SImage).Image(TempBuffBrush.Get()))
										: StaticCastSharedRef<SWidget>(SNew(SSpacer))
								]
							])
				]
			];
	}

	TSharedRef<SWidget> TemporaryBuffsPanel =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.RunSummary", "TemporaryBuffsTitle", "TEMP BUFFS"))
			.Font(RunSummaryBoldFont(14))
			.ColorAndOpacity(FT66Style::Tokens::Text)
			.Justification(ETextJustify::Center)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			bDotaTheme
				? StaticCastSharedRef<SWidget>(FT66Style::MakeScreenSurface(TemporaryBuffSlotsRow, FMargin(4.f)))
				: StaticCastSharedRef<SWidget>(SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FT66Style::Tokens::Stroke)
					.Padding(4.f)
					[
						TemporaryBuffSlotsRow
					])
		];

	// Back button (when viewing saved run) — shown in overlay bottom-left; Restart + Main Menu stay in panel.
	TSharedRef<SWidget> BackButton =
		MakeRunSummarySpriteButton(
			Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK"),
			FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked),
			ET66RunSummaryButtonFamily::CompactNeutral,
			120.f,
			38.f,
			12,
			FMargin(18.f, 10.f));

	TSharedRef<SWidget> ButtonsStack = [&]() -> TSharedRef<SWidget>
	{
		if (bDailyClimbSummaryMode)
		{
			return StaticCastSharedRef<SWidget>(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeRunSummarySpriteButton(Loc ? Loc->GetText_MainMenu() : NSLOCTEXT("T66.RunSummary", "MainMenu", "MAIN MENU"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleMainMenuClicked), ET66RunSummaryButtonFamily::CtaBlue, 220.f, 44.f, 12, FMargin(18.f, 10.f))
				]);
		}

		if (bDifficultyClearSummaryMode)
		{
			TSharedRef<SVerticalBox> DifficultyClearButtons = SNew(SVerticalBox);

			if (UGameInstance* LocalGI = GetGameInstance())
			{
				if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(LocalGI))
				{
					ET66Difficulty NextDifficulty = T66GI->SelectedDifficulty;
					if (T66GetNextDifficulty(T66GI->SelectedDifficulty, NextDifficulty))
					{
						DifficultyClearButtons->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
						[
							MakeRunSummarySpriteButton(NSLOCTEXT("T66.RunSummary", "Continue", "CONTINUE"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleContinueDifficultyClicked), ET66RunSummaryButtonFamily::CtaGreen, 220.f, 44.f, 12, FMargin(18.f, 10.f))
						];
					}
				}
			}

			DifficultyClearButtons->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				MakeRunSummarySpriteButton(NSLOCTEXT("T66.RunSummary", "SaveGame", "SAVE GAME"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleSaveAndQuitClicked), ET66RunSummaryButtonFamily::CompactNeutral, 220.f, 42.f, 12, FMargin(18.f, 10.f))
			];

			DifficultyClearButtons->AddSlot().AutoHeight()
			[
				MakeRunSummarySpriteButton(NSLOCTEXT("T66.RunSummary", "QuitRun", "QUIT"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleQuitToMainMenuClicked), ET66RunSummaryButtonFamily::ToggleOff, 220.f, 42.f, 12, FMargin(18.f, 10.f))
			];

			return StaticCastSharedRef<SWidget>(DifficultyClearButtons);
		}

		return StaticCastSharedRef<SWidget>(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				MakeRunSummarySpriteButton(NSLOCTEXT("T66.RunSummary", "GoAgain", "GO AGAIN!"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked), ET66RunSummaryButtonFamily::CtaGreen, 180.f, 44.f, 12, FMargin(18.f, 10.f))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				MakeRunSummarySpriteButton(Loc ? Loc->GetText_MainMenu() : NSLOCTEXT("T66.RunSummary", "MainMenu", "MAIN MENU"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleMainMenuClicked), ET66RunSummaryButtonFamily::CtaBlue, 200.f, 44.f, 12, FMargin(18.f, 10.f))
			]);
	}();

	auto MakeBigCenteredValue = [](const FText& ValueText, const int32 FontSize = 34) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(ValueText)
				.Font(RunSummaryBoldFont(FontSize))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			];
	};

	// Seed Luck is fixed per run. Older saved snapshots will not have it.
	const bool bHasSavedSeedLuck = bViewingSavedLeaderboardRunSummary
		&& LoadedSavedSummary
		&& LoadedSavedSummary->SchemaVersion >= 19
		&& LoadedSavedSummary->SeedLuck0To100 >= 0;
	const int32 SeedLuck0To100 =
		bHasSavedSeedLuck
		? FMath::Clamp(LoadedSavedSummary->SeedLuck0To100, 0, 100)
		: (RunState ? RunState->GetSeedLuck0To100() : -1);
	const FText SeedLuckText =
		(SeedLuck0To100 >= 0)
		? FText::Format(
			NSLOCTEXT("T66.RunSummary", "SeedLuckOutOf100Format", "{0} / 100 ({1})"),
			FText::AsNumber(SeedLuck0To100),
			UT66RunStateSubsystem::GetSeedLuckAdjectiveText(SeedLuck0To100))
		: NSLOCTEXT("T66.RunSummary", "RatingNA", "N/A");

	FString BackendSubmitStatus;
	if (!bViewingSavedLeaderboardRunSummary)
	{
		if (UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr)
		{
			BackendSubmitStatus = Backend->GetLastSubmitRunStatus();
		}
	}

	const bool bSavedSummaryFlagged =
		bViewingSavedLeaderboardRunSummary
		&& LoadedSavedSummary
		&& !LoadedSavedSummary->IntegrityContext.ShouldAllowRankedSubmission();
	const bool bIntegrityPending = !bViewingSavedLeaderboardRunSummary && bAwaitingBackendRankData;
	const bool bFlaggedForCheating =
		bLiveRunCheatFlagged
		|| BackendSubmitStatus == TEXT("flagged")
		|| BackendSubmitStatus == TEXT("banned")
		|| BackendSubmitStatus == TEXT("suspended")
		|| bSavedSummaryFlagged;
	const FText IntegrityStatusText = bIntegrityPending
		? NSLOCTEXT("T66.RunSummary", "IntegrityPending", "Checking integrity...")
		: (bFlaggedForCheating
			? NSLOCTEXT("T66.RunSummary", "IntegrityFlagged", "Run flagged for cheating.")
			: NSLOCTEXT("T66.RunSummary", "IntegrityClear", "No cheating detected."));

	TSharedRef<SWidget> SeedLuckPanel = MakeRatingSectionPanel(
		NSLOCTEXT("T66.RunSummary", "SeedLuckPanel", "SEED LUCK"),
		MakeBigCenteredValue(SeedLuckText, 28)
	);
	TSharedRef<SWidget> IntegrityPanel = MakeSectionPanel(
		NSLOCTEXT("T66.RunSummary", "IntegrityPanel", "INTEGRITY"),
		SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(IntegrityStatusText)
			.Font(RunSummaryBoldFont(18))
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
		FName(TEXT("Idol_Shadow")),
		FName(TEXT("Idol_Lava")),
		FName(TEXT("Idol_Electric")),
		FName(TEXT("Idol_BlackHole"))
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
			[SNew(STextBlock).Text(RankHeader).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::TextMuted).Font(RunSummaryBoldFont(12))]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 80.f, 0.f)
			[SNew(STextBlock).Text(SourceHeader).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::TextMuted).Font(RunSummaryBoldFont(12))]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[SNew(STextBlock).Text(DamageHeader).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::TextMuted).Font(RunSummaryBoldFont(12))]
		];
	for (int32 Rank = 0; Rank < TableRows.Num(); ++Rank)
	{
		const FDamageLogEntry& Entry = TableRows[Rank];
		DamageBySourceBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
				[SNew(STextBlock).Text(FText::AsNumber(Rank + 1)).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::Text).Font(RunSummaryBodyFont())]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 80.f, 0.f)
				[SNew(STextBlock).Text(GetDamageSourceDisplayName(Entry.SourceID)).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::Text).Font(RunSummaryBodyFont())]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[SNew(STextBlock).Text(FText::AsNumber(Entry.TotalDamage)).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::Text).Font(RunSummaryBodyFont())]
			];
	}
	TSharedRef<SWidget> DamageBySourcePanel = MakeSectionPanel(
		NSLOCTEXT("T66.RunSummary", "DamageBySourcePanel", "DAMAGE BY SOURCE"),
		DamageBySourceBox
	);

	const TAttribute<FMargin> SafeContentInsets = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66Style::GetSafeFrameInsets();
	});

	const TAttribute<FMargin> SafeBackPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66Style::GetSafePadding(FMargin(20.f, 0.f, 0.f, 20.f));
	});

	const TAttribute<FMargin> SafeDrawerPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66Style::GetSafePadding(FMargin(0.f, 70.f, 0.f, 0.f));
	});

	const TAttribute<FOptionalSize> EventLogWidthAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		const FVector2D SafeFrame = FT66Style::GetSafeFrameSize();
		return FOptionalSize(FMath::Clamp(SafeFrame.X * 0.42f, 420.f, 520.f));
	});

	const TAttribute<FOptionalSize> EventLogHeightAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		const FVector2D SafeFrame = FT66Style::GetSafeFrameSize();
		return FOptionalSize(FMath::Clamp(SafeFrame.Y - 140.f, 420.f, 620.f));
	});

	const TAttribute<FOptionalSize> MainPanelWidthAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		const FVector2D SafeFrame = FT66Style::GetSafeFrameSize();
		return FOptionalSize(FMath::Clamp(SafeFrame.X - 96.f, 1320.f, 1720.f));
	});

	const TAttribute<FOptionalSize> MainPanelHeightAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		const FVector2D SafeFrame = FT66Style::GetSafeFrameSize();
		return FOptionalSize(FMath::Clamp(SafeFrame.Y - 56.f, 760.f, 990.f));
	});

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(bDotaTheme ? FT66Style::ScreenBackground() : FT66Style::Tokens::Bg)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.Visibility(bDotaTheme ? EVisibility::Visible : EVisibility::Collapsed)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66Style::Scrim())
			]
			// Main full-screen panel
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.Padding(SafeContentInsets)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(MainPanelWidthAttr)
					.HeightOverride(MainPanelHeightAttr)
					[
						MakeRunSummarySpritePanel(
							SNew(SScrollBox)
							.Orientation(Orient_Vertical)
							.ScrollBarVisibility(EVisibility::Visible)
							.ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
							+ SScrollBox::Slot()
							.Padding(0.f, 0.f, 10.f, 48.f)
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
								.Font(RunSummaryBoldFont(36))
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
							.Text(StageScoreText)
							.Font(RunSummaryBoldFont(18))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
						// Rank panels for the freshly finished run only (Daily Climb uses a dedicated single panel).
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
						[
							SNew(SBox)
							.Visibility_Lambda([this]() { return bViewingSavedLeaderboardRunSummary ? EVisibility::Collapsed : EVisibility::Visible; })
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, bDailyClimbSummaryMode ? 0.f : 10.f)
								[
									bDailyClimbSummaryMode ? DailyRankPanel : WeeklyRankPanel
								]
								+ SVerticalBox::Slot().AutoHeight()
								[
									bDailyClimbSummaryMode ? StaticCastSharedRef<SWidget>(SNew(SSpacer).Size(FVector2D(1.f, 1.f))) : AllTimeRankPanel
								]
							]
						]
						// Personal best banners (only for newly-finished runs)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, -10.f, 0.f, 6.f)
						[
							SNew(STextBlock)
							.Visibility_Lambda([this]() { return (bNewPersonalBestScore && !bViewingSavedLeaderboardRunSummary) ? EVisibility::Visible : EVisibility::Collapsed; })
							.Text_Lambda([this, Loc]()
							{
								return Loc ? Loc->GetText_NewPersonalBestScore() : NSLOCTEXT("T66.RunSummary", "NewPersonalBestScore", "New Personal Score");
							})
							.Font(RunSummaryBoldFont(16))
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
							.Font(RunSummaryBoldFont(16))
							.ColorAndOpacity(FT66Style::Tokens::Success)
						]
						// Main content: Left = Seed Luck, Integrity, buttons/proof. Center = Hero (middle), idols (1x6), inventory. Right = Stats, Damage.
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 26.f)
						[
							SNew(SHorizontalBox)
							// Left side: Seed Luck, Integrity, buttons/proof.
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 24.f, 0.f)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[SeedLuckPanel]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[IntegrityPanel]
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
							// Center: Hero preview (middle), then idols (1 row x 6), inventory, then temp buffs
							+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
								[MakeHeroPreview(HeroPreviewBrush)]
								+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
								[IdolsBorderedGrid]
								+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
								[InventorySlotGrid]
								+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
								[TemporaryBuffsPanel]
							]
							// Right: Stats, Damage by source
							+ SHorizontalBox::Slot().AutoWidth().Padding(24.f, 0.f, 0.f, 0.f)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[BaseStatsPanel]
								+ SVerticalBox::Slot().FillHeight(1.f)[DamageBySourcePanel]
							]
						]
						],
							GetRunSummaryContentShellBrush(),
							FT66Style::Tokens::Space6
						)
					]
				]
			]
			// Back button (bottom-left) — when viewing a saved leaderboard run
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(SafeBackPadding)
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
			.Padding(SafeDrawerPadding)
			[
				SNew(SBox)
				.WidthOverride(EventLogWidthAttr)
				.HeightOverride(EventLogHeightAttr)
				.Visibility_Lambda([this]() { return bLogVisible ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					EventLogPanel
				]
			]
			// Power Coupons earned popup (only when earned >= 1 this run, not when viewing saved).
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.Visibility_Lambda([this]() { return bShowPowerCouponsPopup ? EVisibility::Visible : EVisibility::Collapsed; })
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.68f))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					MakeRunSummarySpritePanel(
						SNew(SBox)
						.WidthOverride(320.f)
						.Padding(24.f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.RunSummary", "PowerCouponsEarnedTitle", "Chad Coupons earned"))
								.Font(RunSummaryBoldFont(18))
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
										return FText::AsNumber(FMath::Max(0, SummaryChadCouponsEarned));
									})
									.Font(RunSummaryBoldFont(24))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text_Lambda([this]()
								{
									return SummaryChadCouponsSourceLabel.IsEmpty()
										? NSLOCTEXT("T66.RunSummary", "PowerCouponsSourceFallback", "Approved challenge reward")
										: FText::FromString(SummaryChadCouponsSourceLabel);
								})
								.Font(RunSummaryRegularFont(12))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(SCheckBox)
									.IsChecked_Lambda([this]()
									{
										return bChadCouponsPopupDontShowAgainChecked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
									})
									.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
									{
										bChadCouponsPopupDontShowAgainChecked = (NewState == ECheckBoxState::Checked);
									})
								]
								+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
								[
									SNew(STextBlock)
									.Text(NSLOCTEXT("T66.RunSummary", "PowerCouponsDontShowAgain", "don't show this again"))
									.Font(RunSummaryRegularFont(13))
									.ColorAndOpacity(FT66Style::Tokens::TextMuted)
									.AutoWrapText(true)
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
							[
								MakeRunSummarySpriteButton(
									NSLOCTEXT("T66.RunSummary", "PowerCouponsThankYou", "Nice!"),
									FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandlePowerCouponsThankYouClicked),
									ET66RunSummaryButtonFamily::ToggleOn,
									140.f,
									38.f,
									12)
							]
						]
						,
						GetRunSummaryRowShellBrush(),
						FMargin(0.f),
						bDotaTheme ? FT66Style::Scrim() : FLinearColor(0.04f, 0.04f, 0.07f, 0.96f))
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

	UGameInstance* GI = GetGameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;

	// Persist into the snapshot immediately (viewer mode) and push to backend when this is
	// an online leaderboard run.
	if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
	{
		LoadedSavedSummary->SchemaVersion = FMath::Max(LoadedSavedSummary->SchemaVersion, 3);
		LoadedSavedSummary->ProofOfRunUrl = ProofOfRunUrl;
		LoadedSavedSummary->bProofOfRunLocked = bProofOfRunLocked;

		if (!LoadedSavedSummary->EntryId.IsEmpty() && Backend && Backend->IsBackendConfigured() && Backend->HasSteamTicket())
		{
			Backend->UpdateProofOfRun(LoadedSavedSummary->EntryId, ProofOfRunUrl);
		}
		else if (!LoadedSavedSummarySlotName.IsEmpty())
		{
			UGameplayStatics::SaveGameToSlot(LoadedSavedSummary, LoadedSavedSummarySlotName, 0);
		}
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
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary && !LoadedSavedSummary->EntryId.IsEmpty()
				&& Backend->IsBackendConfigured() && Backend->HasSteamTicket())
			{
				Backend->SubmitRunReport(LoadedSavedSummary->EntryId, Reason);
				UE_LOG(LogT66RunSummary, Log, TEXT("Run Summary: submitted backend run report for entry=%s"), *LoadedSavedSummary->EntryId);
			}
		}
	}

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
	if (bChadCouponsPopupDontShowAgainChecked)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66PlayerSettingsSubsystem* Settings = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
			{
				Settings->SetShowRunSummaryChadCouponsPopup(false);
			}
		}
	}

	bShowPowerCouponsPopup = false;
	bChadCouponsPopupDontShowAgainChecked = false;
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

bool UT66RunSummaryScreen::SaveCurrentRunToSlot(const bool bFromDifficultyClearSummary)
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	APlayerController* PC = GetOwningPlayer();
	if (!GI || !PC)
	{
		return false;
	}

	UT66SaveSubsystem* SaveSub = GI->GetSubsystem<UT66SaveSubsystem>();
	if (!SaveSub)
	{
		return false;
	}

	int32 SlotIndex = GI->CurrentSaveSlotIndex;
	if (SlotIndex < 0 || SlotIndex >= UT66SaveSubsystem::MaxSlots)
	{
		SlotIndex = SaveSub->FindFirstEmptySlot();
		if (SlotIndex == INDEX_NONE)
		{
			SlotIndex = SaveSub->FindOldestOccupiedSlot();
			if (SlotIndex == INDEX_NONE)
			{
				return false;
			}
		}
		GI->CurrentSaveSlotIndex = SlotIndex;
	}

	UT66RunSaveGame* SaveObj = NewObject<UT66RunSaveGame>(this);
	if (!SaveObj)
	{
		return false;
	}

	SaveObj->HeroID = GI->SelectedHeroID;
	SaveObj->HeroBodyType = GI->SelectedHeroBodyType;
	SaveObj->CompanionID = GI->SelectedCompanionID;
	SaveObj->Difficulty = GI->SelectedDifficulty;
	SaveObj->PartySize = GI->SelectedPartySize;
	SaveObj->MapName = GetWorld() ? UWorld::RemovePIEPrefix(GetWorld()->GetMapName()) : FString();
	SaveObj->LastPlayedUtc = FDateTime::UtcNow().ToIso8601();
	SaveObj->RunSeed = GI->RunSeed;
	SaveObj->MainMapLayoutVariant = GI->CurrentMainMapLayoutVariant;
	SaveObj->bRunIneligibleForLeaderboard = GI->bRunIneligibleForLeaderboard;

	if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
	{
		SaveObj->StageReached = RunState->GetCurrentStage();
		RunState->ExportSavedRunSnapshot(SaveObj->RunSnapshot);
		if (bFromDifficultyClearSummary)
		{
			SaveObj->RunSnapshot.bPendingDifficultyClearSummary = true;
		}

		if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
		{
			SaveObj->EquippedIdols = IdolManager->GetEquippedIdols();
			SaveObj->EquippedIdolTiers = IdolManager->GetEquippedIdolTierValues();
		}
		else
		{
			SaveObj->EquippedIdols = RunState->GetEquippedIdols();
			SaveObj->EquippedIdolTiers = RunState->GetEquippedIdolTierValues();
		}
	}

	if (APawn* Pawn = PC->GetPawn())
	{
		SaveObj->PlayerTransform = Pawn->GetActorTransform();
	}

	const UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>();
	SaveObj->OwnerPlayerId = !GI->CurrentRunOwnerPlayerId.IsEmpty()
		? GI->CurrentRunOwnerPlayerId
		: (PartySubsystem ? PartySubsystem->GetLocalPlayerId() : TEXT("local_player"));
	SaveObj->OwnerDisplayName = !GI->CurrentRunOwnerDisplayName.IsEmpty()
		? GI->CurrentRunOwnerDisplayName
		: (PartySubsystem ? PartySubsystem->GetLocalDisplayName() : TEXT("You"));
	SaveObj->PartyMemberIds = GI->CurrentRunPartyMemberIds.Num() > 0
		? GI->CurrentRunPartyMemberIds
		: (PartySubsystem ? PartySubsystem->GetCurrentPartyMemberIds() : TArray<FString>());
	SaveObj->PartyMemberDisplayNames = GI->CurrentRunPartyMemberDisplayNames.Num() > 0
		? GI->CurrentRunPartyMemberDisplayNames
		: (PartySubsystem ? PartySubsystem->GetCurrentPartyMemberDisplayNames() : TArray<FString>());
	if (SaveObj->PartyMemberIds.Num() == 0 && !SaveObj->OwnerPlayerId.IsEmpty())
	{
		SaveObj->PartyMemberIds.Add(SaveObj->OwnerPlayerId);
	}
	if (SaveObj->PartyMemberDisplayNames.Num() == 0 && !SaveObj->OwnerDisplayName.IsEmpty())
	{
		SaveObj->PartyMemberDisplayNames.Add(SaveObj->OwnerDisplayName);
	}

	return SaveSub->SaveToSlot(SlotIndex, SaveObj);
}

FReply UT66RunSummaryScreen::HandleContinueDifficultyClicked()
{
	DestroyPreviewCaptures();
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66PlayerExperienceSubSystem* PlayerExperience = GI ? GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	if (!T66GI || !RunState || !PlayerExperience)
	{
		return FReply::Handled();
	}

	ET66Difficulty NextDifficulty = T66GI->SelectedDifficulty;
	if (!T66GetNextDifficulty(T66GI->SelectedDifficulty, NextDifficulty))
	{
		return FReply::Handled();
	}

	RunState->SetPendingDifficultyClearSummary(false);
	RunState->SetSaintBlessingActive(false);
	RunState->SetFinalSurvivalEnemyScalar(1.f);
	RunState->SetStageTimerActive(false);
	RunState->ResetBossState();

	T66GI->SelectedDifficulty = NextDifficulty;
	T66GI->bIsStageTransition = true;

	const int32 NextStage = PlayerExperience->GetDifficultyStartStage(NextDifficulty);
	RunState->SetCurrentStage(NextStage);
	RunState->ResetDifficultyPacing();
	RunState->ResetStageTimerToFull();

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->SetPause(false);
	}
	UGameplayStatics::OpenLevel(this, UT66GameInstance::GetGameplayLevelName());
	return FReply::Handled();
}

FReply UT66RunSummaryScreen::HandleSaveAndQuitClicked()
{
	if (!SaveCurrentRunToSlot(true))
	{
		return FReply::Handled();
	}

	ProcessLiveRunFinalAccounting();

	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		T66GI->PendingFrontendScreen = ET66ScreenType::MainMenu;
	}

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->SetPause(false);
	}
	UGameplayStatics::OpenLevel(this, UT66GameInstance::GetFrontendLevelName());
	return FReply::Handled();
}

FReply UT66RunSummaryScreen::HandleQuitToMainMenuClicked()
{
	DestroyPreviewCaptures();
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!GI || !RunState)
	{
		return FReply::Handled();
	}

	RunState->SetPendingDifficultyClearSummary(false);
	RunState->SetSaintBlessingActive(false);
	RunState->SetFinalSurvivalEnemyScalar(1.f);
	RunState->MarkRunEnded(true);

	bDifficultyClearSummaryMode = false;
	ProcessLiveRunFinalAccounting();

	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
	{
		if (bDailyClimbSummaryMode)
		{
			T66GI->ClearActiveDailyClimbRun();
		}
		T66GI->PendingFrontendScreen = ET66ScreenType::MainMenu;
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetPause(false);
	}
	UGameplayStatics::OpenLevel(this, UT66GameInstance::GetFrontendLevelName());
	return FReply::Handled();
}

void UT66RunSummaryScreen::HandleBackendSubmitRunDataReadyForSummary(
	const FString& RequestKey,
	bool bSuccess,
	int32 ScoreRankAlltime,
	int32 ScoreRankWeekly,
	int32 SpeedRunRankAlltime,
	int32 SpeedRunRankWeekly,
	bool bNewScorePersonalBest,
	bool bNewSpeedRunPersonalBest)
{
	static_cast<void>(RequestKey);
	static_cast<void>(bNewScorePersonalBest);
	static_cast<void>(bNewSpeedRunPersonalBest);

	if (bViewingSavedLeaderboardRunSummary)
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			const FString SubmitStatus = Backend->GetLastSubmitRunStatus();
			bLiveRunCheatFlagged =
				SubmitStatus == TEXT("flagged")
				|| SubmitStatus == TEXT("banned")
				|| SubmitStatus == TEXT("suspended");
			ResolveChadCouponsPopupForLiveRun(!bLiveRunCheatFlagged);
		}
	}

	bBackendRankDataReceived = bSuccess;
	bAwaitingBackendRankData = false;
	if (bSuccess)
	{
		BackendScoreRankAllTime = FMath::Max(0, ScoreRankAlltime);
		BackendScoreRankWeekly = FMath::Max(0, ScoreRankWeekly);
		BackendSpeedRunRankAllTime = FMath::Max(0, SpeedRunRankAlltime);
		BackendSpeedRunRankWeekly = FMath::Max(0, SpeedRunRankWeekly);
	}
	else
	{
		BackendScoreRankAllTime = 0;
		BackendScoreRankWeekly = 0;
		BackendSpeedRunRankAllTime = 0;
		BackendSpeedRunRankWeekly = 0;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66LeaderboardSubsystem* LB = GI->GetSubsystem<UT66LeaderboardSubsystem>())
		{
			bNewPersonalBestScore = LB->WasLastScoreNewPersonalBest();
			bNewPersonalBestTime = LB->WasLastCompletedRunTimeNewPersonalBest();
		}
	}

	ForceRebuildSlate();
}

void UT66RunSummaryScreen::HandleBackendDailyClimbSubmitDataReadyForSummary(
	const FString& RequestKey,
	bool bSuccess,
	const FString& Status,
	int32 DailyRank,
	int32 CouponsAwarded)
{
	static_cast<void>(RequestKey);

	if (bViewingSavedLeaderboardRunSummary)
	{
		return;
	}

	bBackendRankDataReceived = bSuccess;
	bAwaitingBackendRankData = false;
	BackendDailyScoreRank = bSuccess ? FMath::Max(0, DailyRank) : 0;
	bNewPersonalBestScore = false;
	bNewPersonalBestTime = false;
	bLiveRunCheatFlagged =
		Status == TEXT("flagged")
		|| Status == TEXT("banned")
		|| Status == TEXT("suspended");

	if (Status == TEXT("accepted"))
	{
		SummaryChadCouponsEarned = FMath::Max(0, CouponsAwarded);
		if (SummaryChadCouponsSourceLabel.IsEmpty())
		{
			SummaryChadCouponsSourceLabel = TEXT("Daily Climb reward");
		}
	}
	else
	{
		SummaryChadCouponsEarned = 0;
	}

	ResolveChadCouponsPopupForLiveRun(Status == TEXT("accepted") && !bLiveRunCheatFlagged);
	ForceRebuildSlate();
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
		UGameplayStatics::OpenLevel(this, UT66GameInstance::GetFrontendLevelName());
		return;
	}
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (bDailyClimbSummaryMode)
	{
		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
		{
			T66GI->ClearActiveDailyClimbRun();
			T66GI->PendingFrontendScreen = ET66ScreenType::MainMenu;
		}

		if (APlayerController* PC = GetOwningPlayer())
		{
			PC->SetPause(false);
		}

		UGameplayStatics::OpenLevel(this, UT66GameInstance::GetFrontendLevelName());
		return;
	}

	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState) RunState->ResetForNewRun();
	if (UT66DamageLogSubsystem* DamageLog = GI ? GI->GetSubsystem<UT66DamageLogSubsystem>() : nullptr)
	{
		DamageLog->ResetForNewRun();
	}
	APlayerController* PC = GetOwningPlayer();
	if (PC) PC->SetPause(false);
	UGameplayStatics::OpenLevel(this, UT66GameInstance::GetTribulationEntryLevelName());
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
		UGameplayStatics::OpenLevel(this, UT66GameInstance::GetFrontendLevelName());
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
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
	{
		if (bDailyClimbSummaryMode)
		{
			T66GI->ClearActiveDailyClimbRun();
		}
		T66GI->PendingFrontendScreen = ET66ScreenType::MainMenu;
	}

	UGameplayStatics::OpenLevel(this, UT66GameInstance::GetFrontendLevelName());
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

