// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66CommunityContentSubsystem.h"
#include "Core/T66DifficultyTuningSubsystem.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66WeaponManagerSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardPacingUtils.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66TemporaryBuffUIUtils.h"
#include "UI/T66FrontendVideoCatalog.h"
#include "UI/T66FrontendVideoPlayer.h"
#include "UI/T66StatsPanelSlate.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "Engine/TextureDefines.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66Style.h"
#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
#include "UObject/SoftObjectPath.h"
#include "GameFramework/Pawn.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SCanvas.h"
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
#include "HAL/PlatformApplicationMisc.h"
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
	SummaryAchievementsUnlocked = 0;
	SummarySecretAchievementsUnlocked = 0;
	bStatsExpanded = false;

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
		RequestDeferredSlateRebuild();
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
		bDailyClimbSummaryMode = T66GI && T66GI->IsDailyClimbRun();
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
		UE_LOG(
			LogT66RunSummary,
			Log,
			TEXT("Run Summary: live activation stage=%d score=%d daily=%d runEnded=%d victory=%d"),
			RunState ? RunState->GetCurrentStage() : -1,
			RunState ? RunState->GetCurrentScore() : -1,
			bDailyClimbSummaryMode ? 1 : 0,
			RunState && RunState->HasRunEnded() ? 1 : 0,
			RunState && RunState->DidRunEndInVictory() ? 1 : 0);
		if (HasValidLiveRunSummaryContext())
		{
			PrepareChadCouponsPopupForLiveRun();
			ProcessLiveRunFinalSubmission();
		}
		else
		{
			UE_LOG(LogT66RunSummary, Log, TEXT("Run Summary: live activation has no finished-run context; skipping leaderboard submission and final accounting."));
		}
		RefreshRunAchievementSummaryCounters();

		// The UI manager reuses modal widget instances, so the cached Slate tree can
		// still reflect the previous button stack/popup state unless we rebuild after
		// activation has resolved the current live-run mode.
		RequestDeferredSlateRebuild();
	}
	else
	{
		bShowPowerCouponsPopup = false;
	}

	OpenRunSummaryPreviewVideo();
}

void UT66RunSummaryScreen::OnScreenDeactivated_Implementation()
{
	CloseRunSummaryPreviewVideo();
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
	SummaryAchievementsUnlocked = 0;
	SummarySecretAchievementsUnlocked = 0;
	bStatsExpanded = false;
	bShowPowerCouponsPopup = false;
	Super::OnScreenDeactivated_Implementation();
}

namespace
{
	constexpr int32 T66RunSummaryFontDelta = -4;

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
		Pressed,
		Disabled
	};

	struct FT66RunSummarySpriteBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
		bool bSimpleFallback = false;
	};

	struct FT66RunSummaryButtonBrushSet
	{
		FT66RunSummarySpriteBrushEntry Normal;
		FT66RunSummarySpriteBrushEntry Hover;
		FT66RunSummarySpriteBrushEntry Pressed;
		FT66RunSummarySpriteBrushEntry Disabled;
	};

	const FLinearColor RunSummaryFantasyText(0.953f, 0.925f, 0.835f, 1.0f);
	constexpr float RunSummaryReferenceWidth = 1920.f;
	constexpr float RunSummaryReferenceHeight = 1080.f;

	FString T66MakeReadableDamageSourceName(const FName SourceID)
	{
		FString Raw = SourceID.ToString();
		Raw.RemoveFromStart(TEXT("BP_"));
		Raw.RemoveFromEnd(TEXT("_C"));
		Raw.ReplaceInline(TEXT("_"), TEXT(" "));

		FString Out;
		Out.Reserve(Raw.Len() + 8);
		for (int32 Index = 0; Index < Raw.Len(); ++Index)
		{
			const TCHAR Ch = Raw[Index];
			if (Index > 0 && Ch != TCHAR(' ') && FChar::IsUpper(Ch))
			{
				const TCHAR Prev = Raw[Index - 1];
				const bool bPreviousWordChar = FChar::IsLower(Prev) || FChar::IsDigit(Prev);
				const bool bAcronymBoundary = FChar::IsUpper(Prev)
					&& (Index + 1) < Raw.Len()
					&& FChar::IsLower(Raw[Index + 1]);
				if (Prev != TCHAR(' ') && (bPreviousWordChar || bAcronymBoundary))
				{
					Out.AppendChar(TCHAR(' '));
				}
			}
			Out.AppendChar(Ch);
		}

		return Out.IsEmpty() ? FString(TEXT("Unknown")) : Out;
	}

	FText T66ResolveRunSummaryDamageSourceName(UGameInstance* GI, const FName SourceID)
	{
		if (SourceID.IsNone())
		{
			return NSLOCTEXT("T66.RunSummary", "DamageSource_Unknown", "Unknown");
		}

		if (SourceID == UT66DamageLogSubsystem::SourceID_AutoAttack)
		{
			return NSLOCTEXT("T66.RunSummary", "DamageSource_AutoAttack", "Auto Attack");
		}

		if (SourceID == UT66DamageLogSubsystem::SourceID_Ultimate)
		{
			return NSLOCTEXT("T66.RunSummary", "DamageSource_Ultimate", "Ultimate");
		}

		if (SourceID == FName(TEXT("PrimaryAttack")))
		{
			return NSLOCTEXT("T66.RunSummary", "DamageSource_PrimaryAttack", "Primary Attack");
		}

		if (SourceID == UT66DamageLogSubsystem::SourceID_Environment)
		{
			return NSLOCTEXT("T66.RunSummary", "DamageSource_Environment", "Environment");
		}

		const FString SourceString = SourceID.ToString();
		if (SourceString.StartsWith(TEXT("Idol_")))
		{
			if (UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr)
			{
				return Loc->GetText_IdolDisplayName(SourceID);
			}
		}

		if (UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr)
		{
			FT66EnemyData EnemyData;
			if (T66GI->GetEnemyData(SourceID, EnemyData) && !EnemyData.DisplayName.IsEmpty())
			{
				return EnemyData.DisplayName;
			}

			FBossData BossData;
			if (T66GI->GetBossData(SourceID, BossData) && !BossData.DisplayName.IsEmpty())
			{
				return BossData.DisplayName;
			}
		}

		return FText::FromString(T66MakeReadableDamageSourceName(SourceID));
	}

	FString GetRunSummaryReferencePanelPath()
	{
		return FT66FlatStyle::GetFlatSharedAssetPath(TEXT("Panels/Modal/modal_shell_medium.png"));
	}

	FString GetRunSummaryReferenceElementPath(const TCHAR* FileName)
	{
		return FString::Printf(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/%s"), FileName ? FileName : TEXT(""));
	}

	const FEditableTextBoxStyle& GetRunSummaryTextBoxStyle()
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
			Style.SetPadding(FMargin(0.f));
		}
		return Style;
	}

	const FSlateBrush* ResolveRunSummarySpriteBrush(
		FT66RunSummarySpriteBrushEntry& Entry,
		const FString& RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		const ESlateBrushDrawType::Type DrawAs,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
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

		if (!Entry.Texture.IsValid() && !Entry.bSimpleFallback)
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					Filter,
					true,
					TEXT("RunSummaryReferenceSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		if (Entry.Texture.IsValid())
		{
			Entry.bSimpleFallback = false;
			Entry.Brush->SetResourceObject(Entry.Texture.Get());
			return Entry.Brush.Get();
		}

		if (T66RuntimeUIBrushAccess::ShouldUseSimpleReferenceFallback(RelativePath))
		{
			Entry.bSimpleFallback = true;
			T66RuntimeUIBrushAccess::ConfigureSimpleReferenceFallbackBrush(
				*Entry.Brush,
				RelativePath,
				ImageSize,
				Margin,
				DrawAs);
			return Entry.Brush.Get();
		}

		Entry.bSimpleFallback = false;
		Entry.Brush->SetResourceObject(nullptr);
		return nullptr;
	}

	const FSlateBrush* GetRunSummaryContentShellBrush()
	{
		static FT66RunSummarySpriteBrushEntry Entry;
		return ResolveRunSummarySpriteBrush(
			Entry,
			GetRunSummaryReferencePanelPath(),
			FVector2D(1055.f, 645.f),
			FMargin(0.035f, 0.055f, 0.035f, 0.055f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetRunSummaryRowShellBrush()
	{
		static FT66RunSummarySpriteBrushEntry Entry;
		return ResolveRunSummarySpriteBrush(
			Entry,
			FT66FlatStyle::GetFlatLongPanelAssetPath(TEXT("normal")),
			FVector2D(620.f, 175.f),
			FMargin(0.055f, 0.210f, 0.055f, 0.210f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetRunSummaryGeneratedRankPanelBrush()
	{
		static FT66RunSummarySpriteBrushEntry Entry;
		return ResolveRunSummarySpriteBrush(
			Entry,
			GetRunSummaryReferencePanelPath(),
			FVector2D(620.f, 175.f),
			FMargin(0.055f, 0.175f, 0.055f, 0.175f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetRunSummaryGeneratedMetricCardBrush()
	{
		static FT66RunSummarySpriteBrushEntry Entry;
		return ResolveRunSummarySpriteBrush(
			Entry,
			GetRunSummaryReferencePanelPath(),
			FVector2D(520.f, 115.f),
			FMargin(0.070f, 0.230f, 0.070f, 0.230f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetRunSummaryGeneratedDamagePanelBrush()
	{
		static FT66RunSummarySpriteBrushEntry Entry;
		return ResolveRunSummarySpriteBrush(
			Entry,
			GetRunSummaryReferencePanelPath(),
			FVector2D(535.f, 236.f),
			FMargin(0.070f, 0.130f, 0.070f, 0.130f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetRunSummaryStatsPanelBrush()
	{
		static FT66RunSummarySpriteBrushEntry Entry;
		return ResolveRunSummarySpriteBrush(
			Entry,
			GetRunSummaryReferencePanelPath(),
			FVector2D(486.f, 209.f),
			FMargin(0.070f, 0.130f, 0.070f, 0.130f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetRunSummaryPreviewFrameBrush()
	{
		static FT66RunSummarySpriteBrushEntry Entry;
		return ResolveRunSummarySpriteBrush(
			Entry,
			GetRunSummaryReferencePanelPath(),
			FVector2D(386.f, 336.f),
			FMargin(0.085f, 0.085f, 0.085f, 0.085f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetRunSummarySlotFrameBrush()
	{
		static FT66RunSummarySpriteBrushEntry Entry;
		return ResolveRunSummarySpriteBrush(
			Entry,
			FT66FlatStyle::GetFlatChromeButtonAssetPath(TEXT("SquareIcon"), TEXT("normal")),
			FVector2D(151.f, 151.f),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetRunSummaryFieldBrush()
	{
		static FT66RunSummarySpriteBrushEntry Entry;
		return ResolveRunSummarySpriteBrush(
			Entry,
			GetRunSummaryReferenceElementPath(TEXT("SquareVariant/dropdown_field_normal_square_variant.png")),
			FVector2D(580.f, 72.f),
			FMargin(0.075f, 0.240f, 0.075f, 0.240f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FScrollBarStyle* GetRunSummaryReferenceScrollBarStyle()
	{
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		static FT66RunSummarySpriteBrushEntry TrackEntry;
		static FT66RunSummarySpriteBrushEntry ThumbEntry;
		static FT66RunSummarySpriteBrushEntry HoverEntry;

		const FSlateBrush* TrackBrush = ResolveRunSummarySpriteBrush(
			TrackEntry,
			GetRunSummaryReferenceElementPath(TEXT("progress_bar_track.png")),
			FVector2D(34.f, 707.f),
			FMargin(0.38f, 0.055f, 0.38f, 0.055f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
		const FSlateBrush* ThumbBrush = ResolveRunSummarySpriteBrush(
			ThumbEntry,
			GetRunSummaryReferenceElementPath(TEXT("progress_bar_fill_cyan.png")),
			FVector2D(36.f, 260.f),
			FMargin(0.38f, 0.085f, 0.38f, 0.085f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
		const FSlateBrush* HoverBrush = ResolveRunSummarySpriteBrush(
			HoverEntry,
			GetRunSummaryReferenceElementPath(TEXT("progress_bar_fill_cyan.png")),
			FVector2D(36.f, 260.f),
			FMargin(0.38f, 0.085f, 0.38f, 0.085f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);

		if (TrackBrush && ThumbBrush && HoverBrush)
		{
			Style
				.SetVerticalBackgroundImage(*TrackBrush)
				.SetVerticalTopSlotImage(*TrackBrush)
				.SetVerticalBottomSlotImage(*TrackBrush)
				.SetNormalThumbImage(*ThumbBrush)
				.SetHoveredThumbImage(*HoverBrush)
				.SetDraggedThumbImage(*HoverBrush)
				.SetThickness(18.f);
		}

		return &Style;
	}

	TSharedRef<SWidget> MakeRunSummaryReferenceSlot(
		const TSharedRef<SWidget>& Content,
		const float Size,
		const FLinearColor& AccentColor,
		const FMargin& ContentPadding = FMargin(4.f))
	{
		const FSlateBrush* SlotFrameBrush = GetRunSummarySlotFrameBrush();
		return SNew(SBox)
			.WidthOverride(Size)
			.HeightOverride(Size)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.Padding(ContentPadding)
				[
					Content
				]
				+ SOverlay::Slot()
				[
					SlotFrameBrush
						? StaticCastSharedRef<SWidget>(SNew(SImage).Image(SlotFrameBrush))
						: StaticCastSharedRef<SWidget>(SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(AccentColor)
							.Padding(1.f)
							[
								SNew(SSpacer)
							])
				]
			];
	}

	FString GetRunSummaryButtonPath(const ET66RunSummaryButtonFamily Family, const ET66RunSummaryButtonState State)
	{
		const TCHAR* Suffix = TEXT("normal");
		if (Family == ET66RunSummaryButtonFamily::ToggleOn && State == ET66RunSummaryButtonState::Normal)
		{
			Suffix = TEXT("selected");
		}
		else if (State == ET66RunSummaryButtonState::Hovered)
		{
			Suffix = TEXT("hover");
		}
		else if (State == ET66RunSummaryButtonState::Pressed)
		{
			Suffix = TEXT("pressed");
		}
		else if (State == ET66RunSummaryButtonState::Disabled)
		{
			Suffix = TEXT("disabled");
		}
		const bool bCta = Family == ET66RunSummaryButtonFamily::CtaGreen || Family == ET66RunSummaryButtonFamily::CtaBlue;
		if (bCta)
		{
			return FT66FlatStyle::GetFlatChromeButtonAssetPath(TEXT("CTA"), Suffix);
		}
		return FT66FlatStyle::GetFlatChromeButtonAssetPath(TEXT("Pill"), Suffix);
	}

	FVector2D GetRunSummaryButtonSize(const ET66RunSummaryButtonFamily Family, const ET66RunSummaryButtonState State)
	{
		if (Family == ET66RunSummaryButtonFamily::CtaGreen)
		{
			return FVector2D(390.f, 124.f);
		}
		if (Family == ET66RunSummaryButtonFamily::CtaBlue)
		{
			return FVector2D(390.f, 124.f);
		}
		if (Family == ET66RunSummaryButtonFamily::ToggleOn)
		{
			return FVector2D(390.f, 124.f);
		}
		if (Family == ET66RunSummaryButtonFamily::ToggleOff)
		{
			return FVector2D(390.f, 124.f);
		}
		return FVector2D(390.f, 124.f);
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
		else if (State == ET66RunSummaryButtonState::Disabled)
		{
			Entry = &Set.Disabled;
		}
		return ResolveRunSummarySpriteBrush(
			*Entry,
			GetRunSummaryButtonPath(Family, State),
			GetRunSummaryButtonSize(Family, State),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
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
		const FSlateBrush* DisabledBrush = GetRunSummaryButtonBrush(Family, ET66RunSummaryButtonState::Disabled);
		if (!NormalBrush)
		{
			return FT66FlatStyle::MakeButton(
				FT66ButtonParams(Label, OnClicked, Family == ET66RunSummaryButtonFamily::ToggleOff ? ET66ButtonType::Danger : ET66ButtonType::Primary)
				.SetMinWidth(MinWidth)
				.SetHeight(Height)
				.SetFontSize(FontSize)
				.SetPadding(ContentPadding));
		}

		return FT66FlatStyle::BuildFlatSlicedPlateButton(
			OnClicked,
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::DownOnly)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FT66FlatStyle::Tokens::FontBold(FontSize))
					.ColorAndOpacity(RunSummaryFantasyText)
					.Justification(ETextJustify::Center)
					.AutoWrapText(false)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				]
			],
			NormalBrush,
			HoverBrush,
			PressedBrush,
			DisabledBrush,
			MinWidth,
			Height,
			ContentPadding);
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

	static FText MakeRunSummarySingleLinePreview(const FString& Value, const int32 MaxChars)
	{
		if (Value.Len() <= MaxChars)
		{
			return FText::FromString(Value);
		}

		return FText::FromString(Value.Left(FMath::Max(0, MaxChars - 3)) + TEXT("..."));
	}

	static FText FormatRunSummaryRankText(int32 Rank)
	{
		return (Rank > 0)
			? FText::Format(NSLOCTEXT("T66.RunSummary", "RankFormat", "#{0}"), FText::AsNumber(Rank))
			: NSLOCTEXT("T66.RunSummary", "RankNA", "N/A");
	}

	static int32 GetDisplayedRunSummaryStageNumber(
		const UT66DifficultyTuningSubsystem* DifficultyTuning,
		const ET66Difficulty Difficulty,
		const int32 StageReached)
	{
		if (StageReached <= 0)
		{
			return 1;
		}

		const int32 StageCount = 4;
		if (!DifficultyTuning)
		{
			return FMath::Clamp(StageReached, 1, StageCount);
		}

		const int32 StartStage = FMath::Max(1, DifficultyTuning->GetDifficultyStartStage(Difficulty));
		const int32 StageOffset = FMath::Max(0, StageReached - StartStage);
		return (StageOffset % StageCount) + 1;
	}

	static int32 AdjustRunSummaryFontSize(int32 BaseSize)
	{
		return FMath::Max(BaseSize + T66RunSummaryFontDelta, 8);
	}

	static FSlateFontInfo RunSummaryRegularFont(int32 BaseSize)
	{
		return FT66FlatStyle::Tokens::FontRegular(AdjustRunSummaryFontSize(BaseSize));
	}

	static FSlateFontInfo RunSummaryBoldFont(int32 BaseSize)
	{
		return FT66FlatStyle::Tokens::FontBold(AdjustRunSummaryFontSize(BaseSize));
	}

	static FSlateFontInfo RunSummaryHeadingFont()
	{
		FSlateFontInfo Font = FT66FlatStyle::Tokens::FontHeading();
		Font.Size = AdjustRunSummaryFontSize(Font.Size);
		return Font;
	}

	static FSlateFontInfo RunSummaryBodyFont()
	{
		FSlateFontInfo Font = FT66FlatStyle::Tokens::FontBody();
		Font.Size = AdjustRunSummaryFontSize(Font.Size);
		return Font;
	}

	static FT66ButtonParams FlattenRunSummaryButton(FT66ButtonParams Params)
	{
		const int32 BaseButtonFontSize = Params.FontSize > 0 ? Params.FontSize : FT66FlatStyle::Tokens::FontButton().Size;
		Params.SetFontSize(AdjustRunSummaryFontSize(BaseButtonFontSize));
		return Params;
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
	return RunState && RunState->HasRunEnded();
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
	const bool bDailyClimbRun = T66GI && T66GI->IsDailyClimbRun();

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
				TEXT("Run Summary: suppressing %d standard Chad Coupons because Daily Descent rewards are backend-authoritative."),
				PendingCoupons);
			RunState->MarkPendingPowerCrystalsSuppressedForWallet();
		}

		SummaryChadCouponsSourceLabel = TEXT("Daily Descent reward");
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

void UT66RunSummaryScreen::RefreshRunAchievementSummaryCounters()
{
	SummaryAchievementsUnlocked = 0;
	SummarySecretAchievementsUnlocked = 0;

	if (bViewingSavedLeaderboardRunSummary)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	if (!Achievements)
	{
		return;
	}

	const TArray<FName> UnlockedThisRun = Achievements->GetCurrentRunUnlockedAchievementIDs();
	if (UnlockedThisRun.Num() <= 0)
	{
		return;
	}

	const TArray<FAchievementData> AllAchievements = Achievements->GetAllAchievements();
	for (const FName AchievementID : UnlockedThisRun)
	{
		const FAchievementData* Definition = AllAchievements.FindByPredicate(
			[AchievementID](const FAchievementData& Achievement)
			{
				return Achievement.AchievementID == AchievementID;
			});

		if (Definition && Definition->Category == ET66AchievementCategory::Special)
		{
			++SummarySecretAchievementsUnlocked;
		}
		else
		{
			++SummaryAchievementsUnlocked;
		}
	}
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
			UE_LOG(LogT66RunSummary, Warning, TEXT("Run Summary: Daily Descent submission skipped because the active daily context was incomplete."));
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
			UE_LOG(LogT66RunSummary, Warning, TEXT("Run Summary: Daily Descent submission skipped because the run summary snapshot could not be loaded."));
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
			TEXT("Run Summary: submitted Daily Descent result snapshotSaved=%d slot=%s challenge=%s attempt=%s stage=%d score=%d"),
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
		Achievements->AddHeroMasteryXP(T66GI->SelectedHeroID, 100);
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
	RefreshRunAchievementSummaryCounters();
}

void UT66RunSummaryScreen::OpenRunSummaryPreviewVideo()
{
	UWorld* World = GetWorld();
	UGameInstance* GIBase = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* GI = Cast<UT66GameInstance>(GIBase);
	if (!GI)
	{
		CloseRunSummaryPreviewVideo();
		return;
	}

	const FName HeroID =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->HeroID :
		(!GI->SelectedHeroID.IsNone() ? GI->SelectedHeroID :
			(GI->GetAllHeroIDs().Num() > 0 ? GI->GetAllHeroIDs()[0] : NAME_None));
	if (HeroID.IsNone())
	{
		CloseRunSummaryPreviewVideo();
		return;
	}

	const ET66BodyType BodyType =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->HeroBodyType : GI->SelectedHeroBodyType;
	const FName SkinID =
		(!bViewingSavedLeaderboardRunSummary && !GI->SelectedHeroSkinID.IsNone()) ? GI->SelectedHeroSkinID : FName(TEXT("Default"));

	FT66FrontendVideoAsset VideoAsset;
	if (!T66FrontendVideoCatalog::ResolveHeroSelection(HeroID, SkinID, BodyType, VideoAsset))
	{
		CloseRunSummaryPreviewVideo();
		return;
	}

	if (!RunSummaryPreviewVideoPlayer)
	{
		RunSummaryPreviewVideoPlayer = NewObject<UT66FrontendVideoPlayer>(this);
	}
	if (!RunSummaryPreviewVideoPlayer
		|| !RunSummaryPreviewVideoPlayer->OpenVideo(VideoAsset, FVector2D(520.f, 520.f), FName(TEXT("RunSummaryHeroPreview"))))
	{
		CloseRunSummaryPreviewVideo();
	}
}

void UT66RunSummaryScreen::CloseRunSummaryPreviewVideo()
{
	if (RunSummaryPreviewVideoPlayer)
	{
		RunSummaryPreviewVideoPlayer->CloseVideo();
	}
}

const FSlateBrush* UT66RunSummaryScreen::GetRunSummaryPreviewVideoBrush() const
{
	return RunSummaryPreviewVideoPlayer
		? RunSummaryPreviewVideoPlayer->GetVideoBrush()
		: nullptr;
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
			const int32 LoadedSummarySchemaVersion = LoadedSavedSummary->SchemaVersion;
			if (LoadedSummarySchemaVersion < T66SparseActiveHeroIdRunSummarySchemaVersion)
			{
				LoadedSavedSummary->HeroID = T66MigrateSparseActiveHeroID(LoadedSavedSummary->HeroID);
				LoadedSavedSummary->SchemaVersion = T66SparseActiveHeroIdRunSummarySchemaVersion;
			}
			bViewingSavedLeaderboardRunSummary = true;
			LoadedSavedSummarySlotName = SlotName;

			// Proof-of-run fields are schema-versioned. If not present, default to empty/unlocked.
			if (LoadedSummarySchemaVersion >= 3)
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
		// Live run state is resolved in BuildSlateUI; no transition summary mode remains.
	}
	MarkSlateUIBuilt();
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
			.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
			.AutoWrapText(true)
			.WrapTextAt(430.f)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)
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
	const UT66DifficultyTuningSubsystem* DifficultyTuning = GIBase ? GIBase->GetSubsystem<UT66DifficultyTuningSubsystem>() : nullptr;
	const int32 StageReached =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->StageReached :
		(RunState ? RunState->GetCurrentStage() : 1);
	const int32 DisplayStageReached = GetDisplayedRunSummaryStageNumber(DifficultyTuning, SummaryDifficulty, StageReached);

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

	const int32 SpeedStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->SpeedStat :
		(RunState ? RunState->GetSpeedStat() : 1);

	UT66LocalizationSubsystem* Loc = GIBase ? GIBase->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66GameInstance* GI = Cast<UT66GameInstance>(GIBase);
	UT66UITexturePoolSubsystem* TexPool = GIBase ? GIBase->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	OpenRunSummaryPreviewVideo();
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
		? NSLOCTEXT("T66.RunSummary", "DailyTitle", "DAILY DESCENT SUMMARY")
		: (Loc ? Loc->GetText_RunSummaryTitle() : NSLOCTEXT("T66.RunSummary", "Title", "RUN SUMMARY"));
	const FText StageReachedValueText = FText::AsNumber(DisplayStageReached);
	const FText ScoreValueText = FText::AsNumber(DisplayScore);
	const FText TimeValueText = FormatRunSummaryDurationText(DisplaySeconds);

	{
		FlatRunSummaryStatTabIndex = FMath::Clamp(FlatRunSummaryStatTabIndex, 0, 2);

		RebuildLogItems();
		SAssignNew(LogListView, SListView<TSharedPtr<FString>>)
			.ListItemsSource(&LogItems)
			.OnGenerateRow(SListView<TSharedPtr<FString>>::FOnGenerateRow::CreateUObject(this, &UT66RunSummaryScreen::GenerateLogRow))
			.SelectionMode(ESelectionMode::None);

		constexpr float CanvasW = 1920.f;
		constexpr float CanvasH = 1080.f;
		const FName StatTabsGroup(TEXT("RunSummaryStatTabs"));
		const FLinearColor Purple = FT66FlatStyle::PurpleAccent();
		const FLinearColor Red = FT66FlatStyle::SelectedText();
		const FLinearColor White = FT66FlatStyle::PrimaryText();
		const FLinearColor DimLine(Purple.R, Purple.G, Purple.B, 0.45f);
		const bool bHasFlatLiveContext = bViewingSavedLeaderboardRunSummary || HasValidLiveRunSummaryContext();
		const int32 FlatGold = (bHasFlatLiveContext && RunState) ? RunState->GetCurrentGold() : 1275;
		const int32 FlatDebt = (bHasFlatLiveContext && RunState) ? RunState->GetCurrentDebt() : 320;
		const int32 FlatNetWorth = (bHasFlatLiveContext && RunState) ? RunState->GetNetWorth() : 955;
		const int32 FlatSeedLuck = (bHasFlatLiveContext && RunState) ? FMath::Clamp(RunState->GetSeedLuck0To100(), 0, 100) : 65;
		const FText FlatSeedLuckText = FText::Format(
			NSLOCTEXT("T66.RunSummary", "FlatSeedLuckFormat", "{0} / 100 ({1})"),
			FText::AsNumber(FlatSeedLuck),
			UT66RunStateSubsystem::GetSeedLuckAdjectiveText(FlatSeedLuck));
		const FString FlatProofUrl = ProofOfRunUrl.IsEmpty()
			? FString(TEXT("youtube.com/watch?v=run-proof-001"))
			: ProofOfRunUrl;
		const bool bShowFlatProofActions = bViewingSavedLeaderboardRunSummary
			|| (!bDailyClimbSummaryMode && (bAwaitingBackendRankData || bBackendRankDataReceived));
		const FText FlatScoreRankLabelText = NSLOCTEXT("T66.RunSummary", "ScoreRankLabel", "Score");
		const FText FlatSpeedRunRankLabelText = NSLOCTEXT("T66.RunSummary", "SpeedRunRankLabel", "Speed Run");
		FString AutoDumpPathForReferenceFixture;
		const bool bFlatReferenceCapture = FParse::Value(FCommandLine::Get(), TEXT("T66AutoDumpScreen="), AutoDumpPathForReferenceFixture);
		const int32 FlatHeroLevel = bFlatReferenceCapture ? 1 : HeroLevel;
		const int32 FlatDamageStat = bFlatReferenceCapture ? 1 : DamageStat;
		const int32 FlatAttackSpeedStat = bFlatReferenceCapture ? 1 : AttackSpeedStat;
		const int32 FlatAttackScaleStat = bFlatReferenceCapture ? 1 : AttackScaleStat;
		const int32 FlatAccuracyStat = bFlatReferenceCapture ? 1 : AccuracyStat;
		const int32 FlatArmorStat = bFlatReferenceCapture ? 1 : ArmorStat;
		const int32 FlatEvasionStat = bFlatReferenceCapture ? 1 : EvasionStat;
		const int32 FlatLuckStat = bFlatReferenceCapture ? 1 : LuckStat;
		const int32 FlatSpeedStat = bFlatReferenceCapture ? 1 : SpeedStat;
		const FName FlatHeroID =
			(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->HeroID :
			(GI ? GI->SelectedHeroID : NAME_None);
		FText FlatHeroNameText = FlatHeroID.IsNone()
			? NSLOCTEXT("T66.RunSummary", "UnknownHero", "Unknown Hero")
			: FText::FromName(FlatHeroID);
		if (GI && !FlatHeroID.IsNone())
		{
			FHeroData HeroData;
			if (GI->GetHeroData(FlatHeroID, HeroData))
			{
				FlatHeroNameText = Loc ? Loc->GetHeroDisplayName(HeroData) : HeroData.DisplayName;
			}
		}
		FString FlatPlayerNameString;
		if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
		{
			FlatPlayerNameString = !LoadedSavedSummary->OwnerDisplayName.IsEmpty()
				? LoadedSavedSummary->OwnerDisplayName
				: LoadedSavedSummary->DisplayName;
		}
		else if (GI && !GI->CurrentRunOwnerDisplayName.IsEmpty())
		{
			FlatPlayerNameString = GI->CurrentRunOwnerDisplayName;
		}
		else if (UT66SteamHelper* SteamHelper = GIBase ? GIBase->GetSubsystem<UT66SteamHelper>() : nullptr)
		{
			FlatPlayerNameString = SteamHelper->GetLocalDisplayName();
		}
		if (FlatPlayerNameString.IsEmpty())
		{
			FlatPlayerNameString = TEXT("Player");
		}
		const FText FlatPlayerNameText = FText::FromString(FlatPlayerNameString);
		const UT66AchievementsSubsystem* Achievements = GIBase ? GIBase->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
		const int32 FlatHeroMasteryLevel =
			(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->HeroMasteryLevel :
			(Achievements ? Achievements->GetHeroMasteryLevel(FlatHeroID) : 1);
		const int32 FlatHeroMasteryXP =
			(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->HeroMasteryXP :
			(Achievements ? Achievements->GetHeroMasteryXP(FlatHeroID) : 0);
		const int32 FlatDisplayedRank =
			(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->ScoreRankAllTime :
			(bBackendRankDataReceived ? BackendScoreRankAllTime : 0);
		const FText FlatMasteryText = FText::Format(
			NSLOCTEXT("T66.RunSummary", "ProfileMasteryFormat", "LV {0}  {1}/100 XP"),
			FText::AsNumber(FMath::Max(1, FlatHeroMasteryLevel)),
			FText::AsNumber(FMath::Max(0, FlatHeroMasteryXP) % 100));
		const FText FlatRankText = FormatRunSummaryRankText(FlatDisplayedRank);

		auto DTag = [](const TCHAR* Name) -> FName
		{
			return FName(Name);
		};
		auto FormatIntWithCommas = [](const int32 Value) -> FText
		{
			FString Digits = FString::FromInt(FMath::Abs(Value));
			for (int32 InsertAt = Digits.Len() - 3; InsertAt > 0; InsertAt -= 3)
			{
				Digits.InsertAt(InsertAt, TEXT(","));
			}
			if (Value < 0)
			{
				Digits.InsertAt(0, TEXT("-"));
			}
			return FText::FromString(Digits);
		};

		TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
		auto AddN = [&Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
		{
			Canvas->AddSlot()
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X * CanvasW, Y * CanvasH, W * CanvasW, H * CanvasH))
			[
				Widget
			];
		};
		auto MakeLabel = [](
			const FName Tag,
			const FText& Text,
			const int32 FontSize,
			const FLinearColor& Color,
			const bool bBold = true,
			const ETextJustify::Type Justification = ETextJustify::Left) -> TSharedRef<SWidget>
		{
			TSharedRef<STextBlock> Label = SNew(STextBlock)
				.Text(Text)
				.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
				.ColorAndOpacity(Color)
				.Justification(Justification)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
				.Visibility(EVisibility::HitTestInvisible);
			return FT66FlatStyle::AttachMetadata(
				Label,
				Tag,
				TEXT("Label"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true);
		};
		auto MakeRect = [](const FLinearColor& Color, const FName Tag, const FString& Role = TEXT("Rect")) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
				.BorderBackgroundColor(Color)
				.Visibility(EVisibility::HitTestInvisible),
				Tag,
				Role,
				ET66FlatState::Default);
		};
		auto MakePanel = [](const ET66FlatState State, const FName Tag) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatPanel(State, FMargin(0.f), SNullWidget::NullWidget, nullptr, Tag);
		};
		auto MakeButtonShell = [](
			const ET66FlatState State,
			FOnClicked OnClicked,
			const FName Tag,
			const FName ToggleGroup = NAME_None) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatToggleGroupButton(
				State,
				SNullWidget::NullWidget,
				MoveTemp(OnClicked),
				FMargin(0.f),
				0.f,
				0.f,
				true,
				Tag,
				ToggleGroup);
		};
		auto MakeTaggedBox = [](const FName Tag, const FString& Role = TEXT("Region")) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(SNew(SBox), Tag, Role, ET66FlatState::Default);
		};
		auto MakeIcon = [&MakeLabel](const FName Tag, const TCHAR* Path, const FVector2D& Size, const FLinearColor& Tint, const FText& Fallback) -> TSharedRef<SWidget>
		{
			static TMap<FString, FT66RunSummarySpriteBrushEntry> FlatIconEntries;
			FT66RunSummarySpriteBrushEntry& Entry = FlatIconEntries.FindOrAdd(FString(Path));
			const FSlateBrush* Brush = ResolveRunSummarySpriteBrush(
				Entry,
				FString(Path),
				Size,
				FMargin(0.f),
				ESlateBrushDrawType::Image,
				TextureFilter::TF_Trilinear);
			if (!Brush)
			{
				return MakeLabel(Tag, Fallback, 22, Tint, true, ETextJustify::Center);
			}
			TSharedRef<SImage> Image = SNew(SImage)
				.Image(Brush)
				.ColorAndOpacity(Tint)
				.Visibility(EVisibility::HitTestInvisible);
			return FT66FlatStyle::AttachMetadata(Image, Tag, TEXT("Icon"), ET66FlatState::Default);
		};
		auto MakeFlagIcon = [Purple](const FName Tag) -> TSharedRef<SWidget>
		{
			TSharedRef<SConstraintCanvas> FlagCanvas = SNew(SConstraintCanvas);
			auto AddFlagRect = [&FlagCanvas, Purple](const float X, const float Y, const float W, const float H)
			{
				FlagCanvas->AddSlot()
				.Anchors(FAnchors(0.f, 0.f))
				.Alignment(FVector2D(0.f, 0.f))
				.Offset(FMargin(X, Y, W, H))
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
					.BorderBackgroundColor(Purple)
					.Visibility(EVisibility::HitTestInvisible)
				];
			};
			AddFlagRect(0.f, 0.f, 4.f, 42.f);
			AddFlagRect(6.f, 4.f, 22.f, 13.f);
			AddFlagRect(6.f, 17.f, 14.f, 9.f);
			return FT66FlatStyle::AttachMetadata(FlagCanvas, Tag, TEXT("Icon"), ET66FlatState::Default);
		};
		auto AddDivider = [&](const float X, const float Y, const float W)
		{
			AddN(X, Y, W, 0.002f, MakeRect(DimLine, NAME_None, TEXT("Divider")));
		};
		auto AddStatRow = [&](const float Y, const TCHAR* RowTag, const FText& Label, const FText& Value)
		{
			const FString Prefix(RowTag);
			AddN(0.031f, Y, 0.260f, 0.030f, MakeTaggedBox(FName(*Prefix), TEXT("StatRow")));
			AddN(0.031f, Y, 0.170f, 0.030f, MakeLabel(FName(*(Prefix + TEXT(".Label"))), Label, 22, White, false));
			AddN(0.224f, Y, 0.065f, 0.030f, MakeLabel(FName(*(Prefix + TEXT(".Value"))), Value, 22, White, false, ETextJustify::Right));
		};
		auto AddRightStatLine = [&](const float Y, const TCHAR* Tag, const FText& Text)
		{
			AddN(0.688f, Y, 0.260f, 0.036f, MakeLabel(DTag(Tag), Text, 28, White, false));
			AddDivider(0.684f, Y + 0.043f, 0.286f);
		};
		auto AddTopStatPanel = [&](const float X, const TCHAR* PanelTag, const TCHAR* IconPath, const FText& Label, const FText& Value, const FText& Fallback)
		{
			const FString Prefix(PanelTag);
			AddN(X, 0.017f, 0.135f, 0.080f, MakePanel(ET66FlatState::Default, FName(*Prefix)));
			AddN(X + 0.014f, 0.034f, 0.032f, 0.048f, MakeIcon(FName(*(Prefix + TEXT(".Icon"))), IconPath, FVector2D(56.f, 56.f), Purple, Fallback));
			AddN(X + 0.048f, 0.031f, 0.075f, 0.024f, MakeLabel(FName(*(Prefix + TEXT(".Label"))), Label, 16, Purple, true, ETextJustify::Center));
			AddN(X + 0.065f, 0.055f, 0.030f, 0.032f, MakeLabel(FName(*(Prefix + TEXT(".Value"))), Value, 28, White, true, ETextJustify::Center));
		};

		AddN(0.f, 0.f, 1.f, 1.f, MakeRect(FT66FlatStyle::BackgroundColor(), DTag(TEXT("RunSummary.Background")), TEXT("Background")));
		AddN(0.018f, 0.017f, 0.964f, 0.925f, MakeTaggedBox(DTag(TEXT("RunSummary.Root")), TEXT("ScreenRoot")));
		AddN(0.025f, 0.036f, 0.166f, 0.049f, MakeLabel(DTag(TEXT("RunSummary.Title")), TitleText, 48, White, true));

		AddN(0.723f, 0.017f, 0.256f, 0.080f, MakeButtonShell(ET66FlatState::Selected, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleViewLogClicked), DTag(TEXT("RunSummary.EventLogButton"))));
		AddN(0.741f, 0.033f, 0.031f, 0.050f, MakeIcon(DTag(TEXT("RunSummary.EventLogButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/log_clipboard.png"), FVector2D(58.f, 58.f), Red, FText::FromString(TEXT("L"))));
		AddN(0.817f, 0.039f, 0.110f, 0.038f, MakeLabel(DTag(TEXT("RunSummary.EventLogButton.Label")), NSLOCTEXT("T66.RunSummary", "FlatEventLog", "EVENT LOG"), 34, Red, true, ETextJustify::Center));

		AddN(0.018f, 0.110f, 0.286f, 0.120f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Left.ProfilePanel"))));
		AddN(0.032f, 0.128f, 0.040f, 0.070f, MakeIcon(DTag(TEXT("RunSummary.Left.ProfilePanel.SteamIcon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/steam_placeholder.png"), FVector2D(64.f, 64.f), Purple, FText::FromString(TEXT("S"))));
		AddN(0.082f, 0.123f, 0.205f, 0.030f, MakeLabel(DTag(TEXT("RunSummary.Left.ProfilePanel.PlayerName")), FlatPlayerNameText, 24, White, true));
		AddN(0.082f, 0.154f, 0.205f, 0.026f, MakeLabel(DTag(TEXT("RunSummary.Left.ProfilePanel.HeroName")), FlatHeroNameText, 20, White, false));
		AddN(0.082f, 0.183f, 0.085f, 0.026f, MakeLabel(DTag(TEXT("RunSummary.Left.ProfilePanel.Rank")), FlatRankText, 18, Purple, true));
		AddN(0.168f, 0.183f, 0.120f, 0.026f, MakeLabel(DTag(TEXT("RunSummary.Left.ProfilePanel.Mastery")), FlatMasteryText, 18, Purple, true, ETextJustify::Right));

		AddN(0.018f, 0.246f, 0.286f, 0.150f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Left.RunOutcomePanel"))));
		AddN(0.031f, 0.258f, 0.023f, 0.039f, MakeFlagIcon(DTag(TEXT("RunSummary.Left.RunOutcomePanel.Icon"))));
		AddN(0.056f, 0.263f, 0.135f, 0.032f, MakeLabel(DTag(TEXT("RunSummary.Left.RunOutcomePanel.Header")), NSLOCTEXT("T66.RunSummary", "RunOutcomeHeader", "RUN OUTCOME"), 26, White, true));
		AddDivider(0.031f, 0.296f, 0.264f);
		AddStatRow(0.306f, TEXT("RunSummary.Left.RunOutcomePanel.StageRow"), NSLOCTEXT("T66.RunSummary", "OutcomeStageReached", "Stage Reached"), StageReachedValueText);
		AddDivider(0.031f, 0.337f, 0.264f);
		AddStatRow(0.346f, TEXT("RunSummary.Left.RunOutcomePanel.ScoreRow"), NSLOCTEXT("T66.RunSummary", "OutcomeScore", "Score"), ScoreValueText);
		AddDivider(0.031f, 0.373f, 0.264f);
		AddStatRow(0.380f, TEXT("RunSummary.Left.RunOutcomePanel.TimeRow"), NSLOCTEXT("T66.RunSummary", "OutcomeTime", "Time"), TimeValueText);

		AddN(0.018f, 0.412f, 0.286f, 0.075f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Left.SkullProgressPanel"))));
		for (int32 SkullIndex = 0; SkullIndex < 5; ++SkullIndex)
		{
			const FString SkullTag = FString::Printf(TEXT("RunSummary.Left.SkullProgressPanel.Skull%02d"), SkullIndex + 1);
			AddN(0.033f + SkullIndex * 0.052f, 0.421f, 0.039f, 0.055f, MakePanel(ET66FlatState::Default, FName(*SkullTag)));
			AddN(0.041f + SkullIndex * 0.052f, 0.428f, 0.023f, 0.040f, MakeIcon(NAME_None, TEXT("RuntimeDependencies/T66/UI/Icons/Flat/skull.png"), FVector2D(48.f, 48.f), Purple, FText::FromString(TEXT("S"))));
		}

		AddN(0.018f, 0.497f, 0.286f, 0.152f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Left.RankPanel"))));
		AddN(0.048f, 0.514f, 0.100f, 0.036f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.WeeklyHeader")), NSLOCTEXT("T66.RunSummary", "WeeklyRankHeaderCaps", "WEEKLY RANK"), 23, Purple, true, ETextJustify::Center));
		AddN(0.187f, 0.514f, 0.115f, 0.036f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.AllTimeHeader")), NSLOCTEXT("T66.RunSummary", "AllTimeRankHeaderCaps", "ALL TIME RANK"), 23, Purple, true, ETextJustify::Center));
		AddN(0.156f, 0.510f, 0.002f, 0.127f, MakeRect(DimLine, NAME_None, TEXT("Divider")));
		AddN(0.031f, 0.558f, 0.055f, 0.027f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.WeeklyScoreLabel")), FlatScoreRankLabelText, 18, White, false));
		AddN(0.124f, 0.558f, 0.035f, 0.027f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.WeeklyScoreValue")), NSLOCTEXT("T66.RunSummary", "RankNAFlat1", "N/A"), 18, White, false));
		AddN(0.174f, 0.558f, 0.055f, 0.027f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.AllTimeScoreLabel")), FlatScoreRankLabelText, 18, White, false));
		AddN(0.258f, 0.558f, 0.040f, 0.027f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.AllTimeScoreValue")), NSLOCTEXT("T66.RunSummary", "RankNAFlat2", "N/A"), 18, White, false, ETextJustify::Right));
		AddN(0.031f, 0.606f, 0.075f, 0.027f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.WeeklySpeedLabel")), FlatSpeedRunRankLabelText, 18, White, false));
		AddN(0.124f, 0.606f, 0.035f, 0.027f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.WeeklySpeedValue")), NSLOCTEXT("T66.RunSummary", "RankNAFlat3", "N/A"), 18, White, false));
		AddN(0.174f, 0.606f, 0.075f, 0.027f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.AllTimeSpeedLabel")), FlatSpeedRunRankLabelText, 18, White, false));
		AddN(0.258f, 0.606f, 0.040f, 0.027f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.AllTimeSpeedValue")), NSLOCTEXT("T66.RunSummary", "RankNAFlat4", "N/A"), 18, White, false, ETextJustify::Right));

		AddN(0.018f, 0.662f, 0.286f, 0.100f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Left.SeedLuckPanel"))));
		AddN(0.030f, 0.675f, 0.025f, 0.044f, MakeIcon(DTag(TEXT("RunSummary.Left.SeedLuckPanel.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/clover.png"), FVector2D(48.f, 48.f), Purple, FText::FromString(TEXT("+"))));
		AddN(0.057f, 0.681f, 0.115f, 0.032f, MakeLabel(DTag(TEXT("RunSummary.Left.SeedLuckPanel.Header")), NSLOCTEXT("T66.RunSummary", "SeedLuckPanel", "SEED LUCK"), 26, White, true));
		AddDivider(0.031f, 0.712f, 0.264f);
		AddN(0.091f, 0.726f, 0.150f, 0.036f, MakeLabel(DTag(TEXT("RunSummary.Left.SeedLuckPanel.Value")), FlatSeedLuckText, 24, White, true, ETextJustify::Center));

		AddN(0.018f, 0.755f, 0.286f, 0.082f, MakeButtonShell(ET66FlatState::Selected, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked), DTag(TEXT("RunSummary.Actions.GoAgainButton"))));
		AddN(0.052f, 0.779f, 0.033f, 0.044f, MakeIcon(DTag(TEXT("RunSummary.Actions.GoAgainButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/refresh.png"), FVector2D(52.f, 52.f), Red, FText::FromString(TEXT("R"))));
		AddN(0.095f, 0.785f, 0.130f, 0.034f, MakeLabel(DTag(TEXT("RunSummary.Actions.GoAgainButton.Label")), NSLOCTEXT("T66.RunSummary", "GoAgain", "GO AGAIN!"), 28, Red, true, ETextJustify::Center));
		AddN(0.018f, 0.849f, 0.286f, 0.085f, MakeButtonShell(ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleMainMenuClicked), DTag(TEXT("RunSummary.Actions.MainMenuButton"))));
		AddN(0.052f, 0.874f, 0.033f, 0.044f, MakeIcon(DTag(TEXT("RunSummary.Actions.MainMenuButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/home.png"), FVector2D(52.f, 52.f), Purple, FText::FromString(TEXT("H"))));
		AddN(0.095f, 0.879f, 0.130f, 0.034f, MakeLabel(DTag(TEXT("RunSummary.Actions.MainMenuButton.Label")), NSLOCTEXT("T66.RunSummary", "MainMenu", "MAIN MENU"), 27, Purple, true, ETextJustify::Center));

		AddN(0.319f, 0.113f, 0.338f, 0.368f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Middle.CharacterPreviewPanel"))));
		AddN(0.335f, 0.140f, 0.306f, 0.320f,
			GetRunSummaryPreviewVideoBrush()
				? FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(SNew(SImage).Image_UObject(this, &UT66RunSummaryScreen::GetRunSummaryPreviewVideoBrush)), DTag(TEXT("RunSummary.Middle.CharacterPreviewPanel.Preview")), TEXT("ContentArt"), ET66FlatState::Default)
				: MakeLabel(DTag(TEXT("RunSummary.Middle.CharacterPreviewPanel.Preview")), NSLOCTEXT("T66.RunSummary", "NoPreview", "No Preview"), 22, White, false, ETextJustify::Center));

		AddN(0.319f, 0.498f, 0.338f, 0.176f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Middle.IdolsPanel"))));
		AddN(0.331f, 0.508f, 0.024f, 0.042f, MakeIcon(DTag(TEXT("RunSummary.Middle.IdolsPanel.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/favorite_star_outline.png"), FVector2D(44.f, 44.f), Purple, FText::FromString(TEXT("*"))));
		AddN(0.354f, 0.512f, 0.070f, 0.034f, MakeLabel(DTag(TEXT("RunSummary.Middle.IdolsPanel.Header")), NSLOCTEXT("T66.RunSummary", "IdolsHeader", "IDOLS"), 28, White, true));
		const TCHAR* IdolIconPaths[4] =
		{
			TEXT("RuntimeDependencies/T66/UI/Icons/Flat/horned_skull.png"),
			TEXT("RuntimeDependencies/T66/UI/Icons/Flat/diamond_spiral_idol.png"),
			TEXT("RuntimeDependencies/T66/UI/Icons/Flat/eye_wings.png"),
			TEXT("RuntimeDependencies/T66/UI/Icons/Flat/chalice_grail.png"),
		};
		for (int32 IdolIndex = 0; IdolIndex < 4; ++IdolIndex)
		{
			const FString IdolTag = FString::Printf(TEXT("RunSummary.Middle.IdolsPanel.Idol%02d"), IdolIndex + 1);
			AddN(0.330f + IdolIndex * 0.083f, 0.542f, 0.070f, 0.115f, MakePanel(ET66FlatState::Default, FName(*IdolTag)));
			AddN(0.341f + IdolIndex * 0.083f, 0.553f, 0.048f, 0.090f, MakeIcon(NAME_None, IdolIconPaths[IdolIndex], FVector2D(90.f, 90.f), Purple, FText::FromString(TEXT("*"))));
		}

		AddN(0.319f, 0.687f, 0.338f, 0.255f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Middle.InventoryPanel"))));
		AddN(0.330f, 0.702f, 0.024f, 0.042f, MakeIcon(DTag(TEXT("RunSummary.Middle.InventoryPanel.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/cube_box.png"), FVector2D(44.f, 44.f), Purple, FText::FromString(TEXT("B"))));
		AddN(0.354f, 0.706f, 0.100f, 0.034f, MakeLabel(DTag(TEXT("RunSummary.Middle.InventoryPanel.Header")), NSLOCTEXT("T66.RunSummary", "InventoryHeader", "INVENTORY"), 28, White, true));
		AddN(0.486f, 0.704f, 0.050f, 0.024f, MakeLabel(DTag(TEXT("RunSummary.Middle.InventoryPanel.GoldLabel")), NSLOCTEXT("T66.RunSummary", "GoldLabel", "GOLD"), 16, Purple, true, ETextJustify::Center));
		AddN(0.478f, 0.727f, 0.065f, 0.034f, MakeLabel(DTag(TEXT("RunSummary.Middle.InventoryPanel.GoldValue")), bHasFlatLiveContext ? FormatIntWithCommas(FlatGold) : FText::FromString(TEXT("1,275")), 24, White, true, ETextJustify::Center));
		AddN(0.550f, 0.708f, 0.002f, 0.052f, MakeRect(DimLine, NAME_None, TEXT("Divider")));
		AddN(0.552f, 0.704f, 0.050f, 0.024f, MakeLabel(DTag(TEXT("RunSummary.Middle.InventoryPanel.DebtLabel")), NSLOCTEXT("T66.RunSummary", "DebtLabel", "DEBT"), 16, Purple, true, ETextJustify::Center));
		AddN(0.546f, 0.727f, 0.060f, 0.034f, MakeLabel(DTag(TEXT("RunSummary.Middle.InventoryPanel.DebtValue")), bHasFlatLiveContext ? FormatIntWithCommas(FlatDebt) : FText::FromString(TEXT("320")), 24, White, true, ETextJustify::Center));
		AddN(0.589f, 0.708f, 0.002f, 0.052f, MakeRect(DimLine, NAME_None, TEXT("Divider")));
		AddN(0.604f, 0.704f, 0.060f, 0.024f, MakeLabel(DTag(TEXT("RunSummary.Middle.InventoryPanel.NetWorthLabel")), NSLOCTEXT("T66.RunSummary", "NetWorthLabel", "NET WORTH"), 16, Purple, true, ETextJustify::Center));
		AddN(0.607f, 0.727f, 0.050f, 0.034f, MakeLabel(DTag(TEXT("RunSummary.Middle.InventoryPanel.NetWorthValue")), bHasFlatLiveContext ? FormatIntWithCommas(FlatNetWorth) : FText::FromString(TEXT("955")), 24, White, true, ETextJustify::Center));
		AddN(0.327f, 0.760f, 0.322f, 0.139f, MakeTaggedBox(DTag(TEXT("RunSummary.Middle.InventorySlotGrid")), TEXT("InventoryGrid")));
		for (int32 SlotIndex = 0; SlotIndex < 16; ++SlotIndex)
		{
			const int32 Row = SlotIndex / 8;
			const int32 Col = SlotIndex % 8;
			const FString SlotTag = FString::Printf(TEXT("RunSummary.Middle.InventorySlot%02d"), SlotIndex + 1);
			AddN(0.327f + Col * 0.041f, 0.760f + Row * 0.080f, 0.034f, 0.059f, MakePanel(ET66FlatState::Default, FName(*SlotTag)));
		}

		const ET66FlatState StatsState = FlatRunSummaryStatTabIndex == 0 ? ET66FlatState::Selected : ET66FlatState::Default;
		const ET66FlatState DamageDealtState = FlatRunSummaryStatTabIndex == 1 ? ET66FlatState::Selected : ET66FlatState::Default;
		const ET66FlatState DamageReceivedState = FlatRunSummaryStatTabIndex == 2 ? ET66FlatState::Selected : ET66FlatState::Default;
		AddN(0.678f, 0.124f, 0.089f, 0.049f, MakeButtonShell(StatsState, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleFlatRunSummaryStatTabClicked, 0), DTag(TEXT("RunSummary.Right.StatTabs.StatsButton")), StatTabsGroup));
		AddN(0.704f, 0.135f, 0.040f, 0.028f, MakeLabel(DTag(TEXT("RunSummary.Right.StatTabs.StatsButton.Label")), NSLOCTEXT("T66.RunSummary", "StatsTab", "STATS"), 22, StatsState == ET66FlatState::Selected ? Red : Purple, true, ETextJustify::Center));
		AddN(0.774f, 0.124f, 0.098f, 0.049f, MakeButtonShell(DamageDealtState, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleFlatRunSummaryStatTabClicked, 1), DTag(TEXT("RunSummary.Right.StatTabs.DamageDealtButton")), StatTabsGroup));
		AddN(0.786f, 0.135f, 0.074f, 0.028f, MakeLabel(DTag(TEXT("RunSummary.Right.StatTabs.DamageDealtButton.Label")), NSLOCTEXT("T66.RunSummary", "DamageDealtTab", "DAMAGE DEALT"), 18, DamageDealtState == ET66FlatState::Selected ? Red : Purple, true, ETextJustify::Center));
		AddN(0.879f, 0.124f, 0.098f, 0.049f, MakeButtonShell(DamageReceivedState, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleFlatRunSummaryStatTabClicked, 2), DTag(TEXT("RunSummary.Right.StatTabs.DamageReceivedButton")), StatTabsGroup));
		AddN(0.887f, 0.135f, 0.086f, 0.028f, MakeLabel(DTag(TEXT("RunSummary.Right.StatTabs.DamageReceivedButton.Label")), NSLOCTEXT("T66.RunSummary", "DamageReceivedTab", "DAMAGE RECEIVED"), 16, DamageReceivedState == ET66FlatState::Selected ? Red : Purple, true, ETextJustify::Center));

		AddN(0.672f, 0.189f, 0.310f, 0.483f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Right.StatsPanel"))));
		if (FlatRunSummaryStatTabIndex == 0)
		{
			const FText StatFmt = NSLOCTEXT("T66.RunSummary", "FlatStatLineFormat", "{0}: {1}");
			AddRightStatLine(0.210f, TEXT("RunSummary.Right.StatsPanel.LevelRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Common", "Level", "LEVEL"), FText::AsNumber(FlatHeroLevel)));
			AddRightStatLine(0.258f, TEXT("RunSummary.Right.StatsPanel.DamageRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "Damage", "Damage"), FText::AsNumber(FlatDamageStat)));
			AddRightStatLine(0.306f, TEXT("RunSummary.Right.StatsPanel.AttackSpeedRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed"), FText::AsNumber(FlatAttackSpeedStat)));
			AddRightStatLine(0.354f, TEXT("RunSummary.Right.StatsPanel.AttackScaleRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "AttackScale", "Attack Scale"), FText::AsNumber(FlatAttackScaleStat)));
			AddRightStatLine(0.402f, TEXT("RunSummary.Right.StatsPanel.AccuracyRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "Accuracy", "Accuracy"), FText::AsNumber(FlatAccuracyStat)));
			AddRightStatLine(0.450f, TEXT("RunSummary.Right.StatsPanel.ArmorRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "Armor", "Armor"), FText::AsNumber(FlatArmorStat)));
			AddRightStatLine(0.498f, TEXT("RunSummary.Right.StatsPanel.EvasionRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "Evasion", "Evasion"), FText::AsNumber(FlatEvasionStat)));
			AddRightStatLine(0.546f, TEXT("RunSummary.Right.StatsPanel.LuckRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "Luck", "Luck"), FText::AsNumber(FlatLuckStat)));
			AddRightStatLine(0.594f, TEXT("RunSummary.Right.StatsPanel.SpeedRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "Speed", "Speed"), FText::AsNumber(FlatSpeedStat)));
		}
		else
		{
			const FText DamagePlaceholder = FlatRunSummaryStatTabIndex == 1
				? NSLOCTEXT("T66.RunSummary", "NoDamageDealt", "No damage dealt.")
				: NSLOCTEXT("T66.RunSummary", "NoDamageReceived", "No damage received.");
			AddN(0.700f, 0.390f, 0.220f, 0.040f, MakeLabel(DTag(TEXT("RunSummary.Right.StatsPanel.DamagePlaceholder")), DamagePlaceholder, 24, White, false, ETextJustify::Center));
		}

		if (bShowFlatProofActions)
		{
			AddN(0.672f, 0.688f, 0.311f, 0.158f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Right.ProofPanel"))));
			AddN(0.689f, 0.705f, 0.027f, 0.046f, MakeIcon(DTag(TEXT("RunSummary.Right.ProofPanel.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/link_chain.png"), FVector2D(46.f, 46.f), Purple, FText::FromString(TEXT("C"))));
			AddN(0.712f, 0.713f, 0.130f, 0.030f, MakeLabel(DTag(TEXT("RunSummary.Right.ProofPanel.Header")), NSLOCTEXT("T66.RunSummary", "ProofOfRunHeader", "PROOF OF RUN"), 27, White, true));
			AddN(0.681f, 0.755f, 0.241f, 0.071f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Right.ProofUrlField.Panel"))));
			AddN(0.699f, 0.784f, 0.200f, 0.030f, MakeLabel(DTag(TEXT("RunSummary.Right.ProofUrlField")), FText::FromString(FlatProofUrl), 20, White, false));
			AddN(0.931f, 0.755f, 0.043f, 0.071f, MakeButtonShell(ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleProofCopyClicked), DTag(TEXT("RunSummary.Right.CopyButton"))));
			AddN(0.943f, 0.779f, 0.024f, 0.041f, MakeIcon(DTag(TEXT("RunSummary.Right.CopyButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/copy_clipboard.png"), FVector2D(46.f, 46.f), Purple, FText::FromString(TEXT("C"))));
			AddN(0.672f, 0.861f, 0.311f, 0.081f, MakeButtonShell(ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleReportCheatingClicked), DTag(TEXT("RunSummary.Right.SubmitCheatingButton"))));
			AddN(0.688f, 0.878f, 0.032f, 0.048f, MakeIcon(DTag(TEXT("RunSummary.Right.SubmitCheatingButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/warning_triangle.png"), FVector2D(52.f, 52.f), Purple, FText::FromString(TEXT("!"))));
			AddN(0.727f, 0.884f, 0.220f, 0.038f, MakeLabel(DTag(TEXT("RunSummary.Right.SubmitCheatingButton.Label")), NSLOCTEXT("T66.RunSummary", "SubmitSuspicion", "SUBMIT SUSPICION OF CHEATING"), 28, Purple, true, ETextJustify::Center));
		}
		else
		{
			AddN(0.672f, 0.688f, 0.311f, 0.113f, MakeButtonShell(ET66FlatState::Selected, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked), DTag(TEXT("RunSummary.Right.GoAgainButton"))));
			AddN(0.704f, 0.721f, 0.033f, 0.044f, MakeIcon(DTag(TEXT("RunSummary.Right.GoAgainButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/refresh.png"), FVector2D(52.f, 52.f), Red, FText::FromString(TEXT("R"))));
			AddN(0.747f, 0.727f, 0.160f, 0.038f, MakeLabel(DTag(TEXT("RunSummary.Right.GoAgainButton.Label")), NSLOCTEXT("T66.RunSummary", "GoAgain", "GO AGAIN!"), 30, Red, true, ETextJustify::Center));
			AddN(0.672f, 0.829f, 0.311f, 0.113f, MakeButtonShell(ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleMainMenuClicked), DTag(TEXT("RunSummary.Right.MainMenuButton"))));
			AddN(0.704f, 0.862f, 0.033f, 0.044f, MakeIcon(DTag(TEXT("RunSummary.Right.MainMenuButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/home.png"), FVector2D(52.f, 52.f), Purple, FText::FromString(TEXT("H"))));
			AddN(0.747f, 0.868f, 0.160f, 0.038f, MakeLabel(DTag(TEXT("RunSummary.Right.MainMenuButton.Label")), NSLOCTEXT("T66.RunSummary", "MainMenu", "MAIN MENU"), 30, Purple, true, ETextJustify::Center));
		}

		TSharedRef<SOverlay> RootOverlay = SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
				.BorderBackgroundColor(FT66FlatStyle::BackgroundColor())
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::Both)
				[
					SNew(SBox)
					.WidthOverride(CanvasW)
					.HeightOverride(CanvasH)
					[
						Canvas
					]
				]
			];

		RootOverlay->AddSlot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.f, 112.f, 40.f, 0.f))
		[
			SNew(SBox)
			.WidthOverride(520.f)
			.HeightOverride(620.f)
			.Visibility_Lambda([this]() { return bLogVisible ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Default,
					FMargin(18.f),
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
					[
						FT66FlatStyle::MakeFlatLabel(NSLOCTEXT("T66.RunSummary", "FlatEventLogDrawerTitle", "EVENT LOG"), ET66FlatLabelRole::Header)
					]
					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						LogListView.IsValid() ? StaticCastSharedRef<SWidget>(LogListView.ToSharedRef()) : StaticCastSharedRef<SWidget>(SNew(SSpacer))
					],
					nullptr,
					DTag(TEXT("RunSummary.EventLogDrawer")))
			]
		];

		RootOverlay->AddSlot()
		[
			SNew(SBorder)
			.Visibility_Lambda([this]() { return bShowPowerCouponsPopup ? EVisibility::Visible : EVisibility::Collapsed; })
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.68f))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Selected,
					FMargin(26.f),
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[
						FT66FlatStyle::MakeFlatLabel(NSLOCTEXT("T66.RunSummary", "PowerCouponsEarnedTitle", "Chad Coupons earned"), ET66FlatLabelRole::Header)
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 12.f, 0.f, 0.f)
					[
						FT66FlatStyle::MakeFlatLabel(FText::AsNumber(FMath::Max(0, SummaryChadCouponsEarned)), ET66FlatLabelRole::StatValue)
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f, 0.f, 0.f)
					[
						FT66FlatStyle::MakeFlatButton(ET66FlatState::Selected, NSLOCTEXT("T66.RunSummary", "PowerCouponsThankYou", "Nice!"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandlePowerCouponsThankYouClicked))
					],
					nullptr,
					DTag(TEXT("RunSummary.PowerCouponsPopup")))
			]
		];

		RootOverlay->AddSlot()
		[
			SNew(SBorder)
			.Visibility_Lambda([this]() { return bReportPromptVisible ? EVisibility::Visible : EVisibility::Collapsed; })
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.62f))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Default,
					FMargin(24.f),
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						FT66FlatStyle::MakeFlatLabel(NSLOCTEXT("T66.RunSummary", "ReportReasonHeader", "REASON"), ET66FlatLabelRole::Header)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(520.f)
						.HeightOverride(180.f)
						[
							SAssignNew(ReportReasonTextBox, SMultiLineEditableTextBox)
							.HintText(NSLOCTEXT("T66.RunSummary", "ReportReasonHint", "Describe why you believe they're cheating..."))
							.Font(RunSummaryRegularFont(14))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
					[
						FT66FlatStyle::MakeFlatActionRow(
							{
								FT66FlatStyle::MakeFlatButton(ET66FlatState::Selected, NSLOCTEXT("T66.RunSummary", "Submit", "SUBMIT"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleReportSubmitClicked)),
								FT66FlatStyle::MakeFlatButton(ET66FlatState::Default, NSLOCTEXT("T66.RunSummary", "Close", "CLOSE"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleReportCloseClicked))
							})
					],
					nullptr,
					DTag(TEXT("RunSummary.ReportPrompt")))
			]
		];

		return FT66FlatStyle::WrapWithoutRetainer(RootOverlay, DTag(TEXT("RunSummary.ViewportRoot")));
	}

	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
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
				.TextStyle(&FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading")))
				.Font(RunSummaryHeadingFont())
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				Body
			],
			GetRunSummaryGeneratedRankPanelBrush(),
			FT66FlatStyle::Tokens::Space4);
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
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(RankText)
				.Font(RunSummaryBoldFont(14))
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
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
				.TextStyle(&FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading")))
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
						.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
					]
				]
			],
			GetRunSummaryGeneratedMetricCardBrush(),
			FT66FlatStyle::Tokens::Space4);
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
				.TextStyle(&FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading")))
				.Font(SmallerHeadingFont)
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				Body
			],
			GetRunSummaryGeneratedRankPanelBrush(),
			FT66FlatStyle::Tokens::Space4);
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
				.TextStyle(&FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading")))
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
						.Text(NSLOCTEXT("T66.RunSummary", "DailyRanksLoading", "Submitting Daily Descent..."))
						.Font(RunSummaryRegularFont(14))
						.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
					]
				]
			],
			GetRunSummaryRowShellBrush(),
			FT66FlatStyle::Tokens::Space4);

	auto MakeOutcomeRow = [](const FText& Label, const FText& Value) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(RunSummaryRegularFont(15))
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 0.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(Value)
				.Font(RunSummaryBoldFont(17))
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
				.Justification(ETextJustify::Right)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			];
	};

	TSharedRef<SVerticalBox> OutcomeRows = SNew(SVerticalBox);
	auto AddOutcomeRow = [&OutcomeRows, &MakeOutcomeRow](const FText& Label, const FText& Value)
	{
		OutcomeRows->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeOutcomeRow(Label, Value)
		];
	};
	AddOutcomeRow(NSLOCTEXT("T66.RunSummary", "OutcomeStageReached", "Stage Reached"), StageReachedValueText);
	AddOutcomeRow(NSLOCTEXT("T66.RunSummary", "OutcomeScore", "Score"), ScoreValueText);
	AddOutcomeRow(NSLOCTEXT("T66.RunSummary", "OutcomeTime", "Time"), TimeValueText);

	TSharedRef<SWidget> RunOutcomePanel = MakeSectionPanel(
		NSLOCTEXT("T66.RunSummary", "RunOutcomeHeader", "RUN OUTCOME"),
		OutcomeRows);

	auto MakeHeroPreview = [this]() -> TSharedRef<SWidget>
	{
		constexpr float PreviewWidth = 462.f;
		constexpr float PreviewHeight = 380.f;
		const FSlateBrush* PreviewBrush = GetRunSummaryPreviewVideoBrush();
		TSharedRef<SWidget> PreviewContent = PreviewBrush
			? StaticCastSharedRef<SWidget>(SNew(SBox)
				.WidthOverride(PreviewWidth)
				.HeightOverride(PreviewHeight)
				[
					SNew(SImage).Image_UObject(this, &UT66RunSummaryScreen::GetRunSummaryPreviewVideoBrush)
				])
			: StaticCastSharedRef<SWidget>(SNew(SBox)
				.WidthOverride(PreviewWidth)
				.HeightOverride(PreviewHeight)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.RunSummary", "NoPreview", "No Preview"))
					.TextStyle(&FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body")))
					.Font(RunSummaryBodyFont())
					.Justification(ETextJustify::Center)
				]);
		return MakeRunSummarySpritePanel(
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				PreviewContent
			],
			GetRunSummaryPreviewFrameBrush(),
			FMargin(18.f, 28.f, 18.f, 44.f));
	};

	// Event log drawer, hidden by default and collapsible from both the header button and drawer.
	TSharedRef<SWidget> EventLogPanel =
		MakeRunSummarySpritePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.RunSummary", "EventLogTitle", "EVENT LOG"))
					.TextStyle(&FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading")))
					.Font(RunSummaryHeadingFont())
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
				[
					MakeRunSummarySpriteButton(
						NSLOCTEXT("T66.RunSummary", "EventLogCollapse", "HIDE"),
						FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleViewLogClicked),
						ET66RunSummaryButtonFamily::CompactNeutral,
						96.f,
						34.f,
						12,
						FMargin(14.f, 8.f))
				]
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				MakeRunSummarySpritePanel(
					LogListView.IsValid() ? StaticCastSharedRef<SWidget>(LogListView.ToSharedRef()) : StaticCastSharedRef<SWidget>(SNew(SSpacer)),
					GetRunSummaryRowShellBrush(),
					FT66FlatStyle::Tokens::Space2
				)
			],
			GetRunSummaryGeneratedDamagePanelBrush(),
			FT66FlatStyle::Tokens::Space4);

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
				SNew(SBox)
				.Visibility(TAttribute<EVisibility>::CreateLambda([this]() { return (bProofOfRunLocked && !ProofOfRunUrl.IsEmpty()) ? EVisibility::Visible : EVisibility::Collapsed; }))
				[
					MakeRunSummarySpritePanel(
						SNew(SHyperlink)
						.Text_Lambda([this]() { return MakeRunSummarySingleLinePreview(ProofOfRunUrl, 76); })
						.OnNavigate(FSimpleDelegate::CreateUObject(this, &UT66RunSummaryScreen::HandleProofLinkNavigate)),
						GetRunSummaryFieldBrush(),
						FMargin(14.f, 8.f),
						FLinearColor(0.035f, 0.038f, 0.048f, 0.96f))
				]
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
				MakeRunSummarySpritePanel(
					SAssignNew(ProofUrlTextBox, SEditableTextBox)
					.Style(&GetRunSummaryTextBoxStyle())
					.Text(FText::FromString(ProofOfRunUrl))
					.OnTextChanged_Lambda([this](const FText& NewText)
					{
						// Keep the draft in sync with what the player typed.
						ProofOfRunUrl = NewText.ToString();
					})
					.HintText(NSLOCTEXT("T66.RunSummary", "ProofHint", "Paste YouTube link here..."))
					.Font(RunSummaryRegularFont(14))
					.ForegroundColor(FT66FlatStyle::Tokens::Text)
					.MinDesiredWidth(420.f),
					GetRunSummaryFieldBrush(),
					FMargin(14.f, 8.f),
					FLinearColor(0.035f, 0.038f, 0.048f, 0.96f))
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
			300.f,
			58.f,
			14,
			FMargin(20.f, 12.f));

	TSharedRef<SWidget> ReportPrompt =
		MakeRunSummarySpritePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.RunSummary", "ReportReasonHeader", "REASON"))
				.TextStyle(&FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading")))
				.Font(RunSummaryHeadingFont())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(SBox)
				.HeightOverride(120.f)
				[
					MakeRunSummarySpritePanel(
						SAssignNew(ReportReasonTextBox, SMultiLineEditableTextBox)
						.Style(&GetRunSummaryTextBoxStyle())
						.HintText(NSLOCTEXT("T66.RunSummary", "ReportReasonHint", "Describe why you believe they're cheating..."))
						.Font(RunSummaryRegularFont(14))
						.ForegroundColor(FT66FlatStyle::Tokens::Text)
						.AutoWrapText(true),
						GetRunSummaryFieldBrush(),
						FMargin(16.f, 12.f),
						FLinearColor(0.035f, 0.038f, 0.048f, 0.96f))
				]
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
		FT66FlatStyle::Tokens::Space3);

	auto MakeEventLogButton = [&](const FText& Text) -> TSharedRef<SWidget>
	{
		return MakeRunSummarySpriteButton(
			Text,
			FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleViewLogClicked),
			ET66RunSummaryButtonFamily::CompactNeutral,
			274.f,
			65.f,
			16,
			FMargin(20.f, 12.f));
	};

	auto MakeTopSummaryBadge = [](const FText& Label, const FText& Value, TAttribute<EVisibility> BadgeVisibility) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(184.f)
			.HeightOverride(62.f)
			.Visibility(BadgeVisibility)
			[
				MakeRunSummarySpritePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(Label)
						.Font(RunSummaryBoldFont(11))
						.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
						.Justification(ETextJustify::Center)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					]
					+ SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(Value)
						.Font(RunSummaryBoldFont(20))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
						.Justification(ETextJustify::Center)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					],
					GetRunSummaryGeneratedMetricCardBrush(),
					FMargin(10.f, 7.f))
			];
	};

	TSharedRef<SHorizontalBox> TopSummaryBadgesRow = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
		[
			MakeTopSummaryBadge(
				NSLOCTEXT("T66.RunSummary", "NewBestScoreBadge", "NEW BEST SCORE"),
				NSLOCTEXT("T66.RunSummary", "NewBestBadgeYes", "YES"),
				TAttribute<EVisibility>::CreateLambda([this]()
				{
					return (bNewPersonalBestScore && !bViewingSavedLeaderboardRunSummary) ? EVisibility::Visible : EVisibility::Collapsed;
				}))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
		[
			MakeTopSummaryBadge(
				NSLOCTEXT("T66.RunSummary", "NewBestTimeBadge", "NEW BEST TIME"),
				NSLOCTEXT("T66.RunSummary", "NewBestTimeBadgeYes", "YES"),
				TAttribute<EVisibility>::CreateLambda([this]()
				{
					return (bNewPersonalBestTime && !bViewingSavedLeaderboardRunSummary) ? EVisibility::Visible : EVisibility::Collapsed;
				}))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
		[
			MakeTopSummaryBadge(
				NSLOCTEXT("T66.RunSummary", "ChadCouponsGainedBadge", "CHAD COUPONS"),
				FText::AsNumber(FMath::Max(0, SummaryChadCouponsEarned)),
				EVisibility::Visible)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
		[
			MakeTopSummaryBadge(
				NSLOCTEXT("T66.RunSummary", "AchievementsUnlockedBadge", "ACHIEVEMENTS"),
				FText::AsNumber(FMath::Max(0, SummaryAchievementsUnlocked)),
				EVisibility::Visible)
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			MakeTopSummaryBadge(
				NSLOCTEXT("T66.RunSummary", "SecretAchievementsUnlockedBadge", "SECRET ACH."),
				FText::AsNumber(FMath::Max(0, SummarySecretAchievementsUnlocked)),
				EVisibility::Visible)
		];

	// Stats panel: same width and content as Shop/Gambler (primary + secondary, scroll). Header "STATS".
	TSharedRef<SWidget> BaseStatsPanel = [&]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> StatsRowsBox = SNew(SVerticalBox);
		const FText StatFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.Stats", "StatLineFormat", "{0}: {1}");
		auto AddStatLineText = [&](const FText& Label, const FText& Value)
		{
			StatsRowsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(FText::Format(StatFmt, Label, Value))
				.TextStyle(&FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body")))
				.Font(RunSummaryBodyFont())
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			];
		};
		auto AddStatLine = [&AddStatLineText](const FText& Label, int32 Value)
		{
			AddStatLineText(Label, FText::AsNumber(Value));
		};
		AddStatLine(Loc ? Loc->GetText_Level() : NSLOCTEXT("T66.Common", "Level", "LEVEL"), HeroLevel);
		AddStatLine(Loc ? Loc->GetText_Stat_Damage() : NSLOCTEXT("T66.Stats", "Damage", "Damage"), DamageStat);
		AddStatLine(Loc ? Loc->GetText_Stat_AttackSpeed() : NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed"), AttackSpeedStat);
		AddStatLine(Loc ? Loc->GetText_Stat_AttackScale() : NSLOCTEXT("T66.Stats", "AttackScale", "Attack Scale"), AttackScaleStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Accuracy() : NSLOCTEXT("T66.Stats", "Accuracy", "Accuracy"), AccuracyStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Armor() : NSLOCTEXT("T66.Stats", "Armor", "Armor"), ArmorStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Evasion() : NSLOCTEXT("T66.Stats", "Evasion", "Evasion"), EvasionStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Luck() : NSLOCTEXT("T66.Stats", "Luck", "Luck"), LuckStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Speed() : NSLOCTEXT("T66.Stats", "Speed", "Speed"), SpeedStat);

		if (bStatsExpanded)
		{
			auto FormatFloat = [](const float Value, const int32 MaxFractionalDigits = 1) -> FText
			{
				FNumberFormattingOptions Options;
				Options.MinimumFractionalDigits = MaxFractionalDigits > 0 ? 1 : 0;
				Options.MaximumFractionalDigits = FMath::Max(0, MaxFractionalDigits);
				return FText::AsNumber(Value, &Options);
			};
			auto FormatPercent = [&FormatFloat](const float Value01) -> FText
			{
				return FText::Format(NSLOCTEXT("T66.RunSummary", "PercentValue", "{0}%"), FormatFloat(Value01 * 100.f, 1));
			};
			auto FormatMultiplier = [&FormatFloat](const float Value) -> FText
			{
				return FText::Format(NSLOCTEXT("T66.RunSummary", "MultiplierValue", "{0}x"), FormatFloat(Value, 2));
			};

			StatsRowsBox->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 8.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.RunSummary", "ExtendedStatsHeader", "EXTENDED"))
				.Font(RunSummaryBoldFont(14))
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			];

			if (RunState)
			{
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "CritChance", "Crit Chance"), FormatPercent(RunState->GetCritChance01()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "CritDamage", "Crit Damage"), FormatMultiplier(RunState->GetCritDamageMultiplier()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "LifeSteal", "Life Steal"), FormatPercent(RunState->GetLifeStealFraction()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "ReflectDamage", "Reflect Damage"), FormatPercent(RunState->GetReflectDamageFraction()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "CrushChance", "Crush Chance"), FormatPercent(RunState->GetCrushChance01()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "AssassinateChance", "Assassinate Chance"), FormatPercent(RunState->GetAssassinateChance01()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "MoveSpeedMultiplier", "Move Speed Mult"), FormatMultiplier(RunState->GetMovementSpeedSecondaryMultiplier()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "RollCooldown", "Roll Cooldown"), FormatMultiplier(RunState->GetDashCooldownMultiplier()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "AttackRange", "Attack Range"), FormatFloat(RunState->GetHeroBaseAttackRange(), 0));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "CloseRange", "Close Range"), FormatFloat(RunState->GetCloseRangeThreshold(), 0));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "LongRange", "Long Range"), FormatFloat(RunState->GetLongRangeThreshold(), 0));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "CloseRangeDamage", "Close Damage"), FormatMultiplier(RunState->GetCloseRangeDamageMultiplier()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "LongRangeDamage", "Long Damage"), FormatMultiplier(RunState->GetLongRangeDamageMultiplier()));
			}
			else
			{
				StatsRowsBox->AddSlot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.RunSummary", "NoExtendedStatsSnapshot", "No extended stat snapshot saved."))
					.Font(RunSummaryRegularFont(13))
					.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
					.AutoWrapText(true)
				];
			}
		}

		// Header must be "STATS" (same as Shop/Gambler), not "Base Stats", for leaderboard/saved run summaries.
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.StatsPanel", "Header", "STATS"))
					.TextStyle(&FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading")))
					.Font(RunSummaryHeadingFont())
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					MakeRunSummarySpriteButton(
						bStatsExpanded
							? NSLOCTEXT("T66.RunSummary", "CollapseExtendedStats", "-")
							: NSLOCTEXT("T66.RunSummary", "ExpandExtendedStats", "+"),
						FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleStatsExpandClicked),
						ET66RunSummaryButtonFamily::CompactNeutral,
						42.f,
						34.f,
						18,
						FMargin(8.f, 4.f))
				]
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SScrollBox)
				.ScrollBarStyle(GetRunSummaryReferenceScrollBarStyle())
				.ScrollBarThickness(FVector2D(14.f, 14.f))
				.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
				.ScrollBarVisibility(EVisibility::Visible)
				+ SScrollBox::Slot()[StatsRowsBox]
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

	static constexpr float IdolSlotPad = 10.f;
	static constexpr float IdolSlotSize = 100.f;
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
				MakeRunSummaryReferenceSlot(
					IdolBrush.IsValid()
						? StaticCastSharedRef<SWidget>(SNew(SImage).Image(IdolBrush.Get()))
						: StaticCastSharedRef<SWidget>(SNew(SSpacer)),
					IdolSlotSize,
					IdolColor,
					FMargin(4.f))
			];
	}
	TSharedRef<SWidget> IdolsBorderedGrid = MakeRunSummarySpritePanel(
		IdolSlotsRow,
		nullptr,
		FMargin(7.f, 5.f),
		FLinearColor::Transparent);

	// Inventory: 10 columns, scrolls vertically for any run that exceeds the old 20-slot view.
	static constexpr int32 InvCols = 10;
	static constexpr float InvSlotSize = 82.f;
	static constexpr float InvSlotPad = 3.f;
	const FLinearColor InvSlotBorderColor(0.45f, 0.55f, 0.50f, 0.5f);
	const int32 InventorySlotCount = FMath::Max(InventoryLocal.Num(), UT66RunStateSubsystem::MaxInventorySlots);
	const int32 InventoryRows = FMath::Max(1, FMath::DivideAndRoundUp(InventorySlotCount, InvCols));
	InventoryItemIconBrushes.SetNum(InventorySlotCount);
	for (int32 i = 0; i < InventorySlotCount; ++i)
	{
		if (!InventoryItemIconBrushes[i].IsValid()) InventoryItemIconBrushes[i] = MakeShared<FSlateBrush>();
		InventoryItemIconBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		InventoryItemIconBrushes[i]->ImageSize = FVector2D(InvSlotSize - 4.f, InvSlotSize - 4.f);
		InventoryItemIconBrushes[i]->SetResourceObject(nullptr);
	}
	for (int32 InvIdx = 0; InvIdx < InventoryLocal.Num(); ++InvIdx)
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
	for (int32 Row = 0; Row < InventoryRows; ++Row)
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
					MakeRunSummaryReferenceSlot(
						(SlotBrush.IsValid() && bHasItem)
							? StaticCastSharedRef<SWidget>(SNew(SImage).Image(SlotBrush.Get()))
							: StaticCastSharedRef<SWidget>(SNew(SSpacer)),
						InvSlotSize,
						SlotColor,
						FMargin(4.f))
				];
		}
		InvGridRef->AddSlot().AutoHeight()[RowBox];
	}
	TSharedRef<SWidget> InventorySlotGrid = MakeRunSummarySpritePanel(
		SNew(SScrollBox)
		.Orientation(Orient_Vertical)
		.ScrollBarStyle(GetRunSummaryReferenceScrollBarStyle())
		.ScrollBarThickness(FVector2D(14.f, 14.f))
		.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
		.ScrollBarVisibility(InventoryRows > 2 ? EVisibility::Visible : EVisibility::Collapsed)
		+ SScrollBox::Slot()[InvGridRef],
		nullptr,
		FMargin(8.f, 6.f),
		FLinearColor::Transparent);

	static constexpr float TempBuffSlotSize = 40.f;
	static constexpr float TempBuffSlotPad = 3.f;
	TemporaryBuffIconBrushes.Reset();
	TemporaryBuffIconBrushes.SetNum(UT66BuffSubsystem::MaxSelectedSingleUseBuffs);

	TSharedRef<SHorizontalBox> TemporaryBuffSlotsRow = SNew(SHorizontalBox);
	bool bHasAnyTemporaryBuff = false;
	for (int32 SlotIndex = 0; SlotIndex < UT66BuffSubsystem::MaxSelectedSingleUseBuffs; ++SlotIndex)
	{
		const ET66SecondaryStatType SlotStat = TemporaryBuffSlots.IsValidIndex(SlotIndex)
			? TemporaryBuffSlots[SlotIndex]
			: ET66SecondaryStatType::None;
		const bool bHasTemporaryBuff = T66IsLiveSecondaryStatType(SlotStat);
		bHasAnyTemporaryBuff |= bHasTemporaryBuff;
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
				.ToolTipText(SlotTooltip)
				[
					MakeRunSummaryReferenceSlot(
						(bHasTemporaryBuff && TempBuffBrush.IsValid())
							? StaticCastSharedRef<SWidget>(SNew(SImage).Image(TempBuffBrush.Get()))
							: StaticCastSharedRef<SWidget>(SNew(SSpacer)),
						TempBuffSlotSize,
						bHasTemporaryBuff ? FLinearColor(0.36f, 0.72f, 0.46f, 1.f) : FLinearColor(0.45f, 0.55f, 0.50f, 0.5f),
						FMargin(5.f))
				]
			];
	}

	TSharedRef<SWidget> TemporaryBuffsPanel =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.RunSummary", "DrugsUsedTitle", "DRUGS USED"))
			.Font(RunSummaryBoldFont(14))
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			.Justification(ETextJustify::Center)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			MakeRunSummarySpritePanel(
				TemporaryBuffSlotsRow,
				GetRunSummaryRowShellBrush(),
				FMargin(7.f, 5.f))
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
					MakeRunSummarySpriteButton(Loc ? Loc->GetText_MainMenu() : NSLOCTEXT("T66.RunSummary", "MainMenu", "MAIN MENU"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleMainMenuClicked), ET66RunSummaryButtonFamily::CtaBlue, 342.f, 85.f, 18, FMargin(18.f, 10.f))
				]);
		}

		return StaticCastSharedRef<SWidget>(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
			[
				MakeRunSummarySpriteButton(NSLOCTEXT("T66.RunSummary", "GoAgain", "GO AGAIN!"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked), ET66RunSummaryButtonFamily::CtaGreen, 342.f, 85.f, 18, FMargin(22.f, 14.f))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				MakeRunSummarySpriteButton(Loc ? Loc->GetText_MainMenu() : NSLOCTEXT("T66.RunSummary", "MainMenu", "MAIN MENU"), FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleMainMenuClicked), ET66RunSummaryButtonFamily::CtaBlue, 342.f, 85.f, 18, FMargin(22.f, 14.f))
			]);
	}();

	auto MakeBigCenteredValue = [](const FText& ValueText, const int32 FontSize = 34) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::DownOnly)
				[
					SNew(STextBlock)
					.Text(ValueText)
					.Font(RunSummaryBoldFont(FontSize))
					.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
					.Justification(ETextJustify::Center)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				]
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
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			.Justification(ETextJustify::Center)
		]
	);

	// Damage by source: use actual run maps only. The received map is captured after armor and invulnerability.
	auto BuildDamageRowsFromMap = [](const TMap<FName, int32>& DamageMap) -> TArray<FDamageLogEntry>
	{
		TArray<FDamageLogEntry> Rows;
		Rows.Reserve(DamageMap.Num());
		for (const auto& Pair : DamageMap)
		{
			if (Pair.Key.IsNone() || Pair.Value <= 0)
			{
				continue;
			}

			FDamageLogEntry Entry;
			Entry.SourceID = Pair.Key;
			Entry.TotalDamage = Pair.Value;
			Rows.Add(Entry);
		}
		Rows.Sort([](const FDamageLogEntry& A, const FDamageLogEntry& B)
		{
			if (A.TotalDamage != B.TotalDamage)
			{
				return A.TotalDamage > B.TotalDamage;
			}
			return A.SourceID.LexicalLess(B.SourceID);
		});
		return Rows;
	};

	TArray<FDamageLogEntry> DamageDealtRows;
	TArray<FDamageLogEntry> DamageReceivedRows;
	if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
	{
		DamageDealtRows = BuildDamageRowsFromMap(LoadedSavedSummary->DamageBySource);
		DamageReceivedRows = BuildDamageRowsFromMap(LoadedSavedSummary->DamageReceivedBySource);
	}
	else if (UT66DamageLogSubsystem* DamageLog = GI ? GI->GetSubsystem<UT66DamageLogSubsystem>() : nullptr)
	{
		DamageDealtRows = DamageLog->GetDamageBySourceSorted();
		DamageReceivedRows = DamageLog->GetDamageReceivedBySourceSorted();
	}

	const FTextBlockStyle& BodyStyle = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body"));
	const FText RankHeader = NSLOCTEXT("T66.RunSummary", "DamageTableRank", "Rank");
	const FText SourceHeader = NSLOCTEXT("T66.RunSummary", "DamageTableSource", "Source");
	const FText DamageHeader = NSLOCTEXT("T66.RunSummary", "DamageTableDamage", "Damage");
	constexpr float DamageRankColumnWidth = 38.f;
	constexpr float DamageSourceColumnWidth = 172.f;
	constexpr float DamageValueColumnWidth = 92.f;
	auto MakeDamageCellText = [&BodyStyle](const FText& Text, const FLinearColor& Color, const FSlateFontInfo& Font, const ETextJustify::Type Justification = ETextJustify::Left) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(Text)
			.TextStyle(&BodyStyle)
			.ColorAndOpacity(Color)
			.Font(Font)
			.Justification(Justification)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds);
	};

	auto MakeDamageTable = [GI, &MakeDamageCellText, RankHeader, SourceHeader, DamageHeader](
		const TArray<FDamageLogEntry>& TableRows,
		const FText& EmptyText) -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> TableBox = SNew(SVerticalBox);
		TableBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)
				[SNew(SBox).WidthOverride(DamageRankColumnWidth)[MakeDamageCellText(RankHeader, FT66FlatStyle::Tokens::TextMuted, RunSummaryBoldFont(12))]]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)
				[SNew(SBox).WidthOverride(DamageSourceColumnWidth)[MakeDamageCellText(SourceHeader, FT66FlatStyle::Tokens::TextMuted, RunSummaryBoldFont(12))]]
				+ SHorizontalBox::Slot().AutoWidth()
				[SNew(SBox).WidthOverride(DamageValueColumnWidth)[MakeDamageCellText(DamageHeader, FT66FlatStyle::Tokens::TextMuted, RunSummaryBoldFont(12), ETextJustify::Right)]]
			];

		if (TableRows.Num() <= 0)
		{
			TableBox->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(EmptyText)
					.TextStyle(&FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body")))
					.Font(RunSummaryBodyFont())
					.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
					.Justification(ETextJustify::Center)
				];
			return TableBox;
		}

		for (int32 Rank = 0; Rank < TableRows.Num(); ++Rank)
		{
			const FDamageLogEntry& Entry = TableRows[Rank];
			TableBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)
					[SNew(SBox).WidthOverride(DamageRankColumnWidth)[MakeDamageCellText(FText::AsNumber(Rank + 1), FT66FlatStyle::Tokens::Text, RunSummaryBodyFont())]]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)
					[SNew(SBox).WidthOverride(DamageSourceColumnWidth)[MakeDamageCellText(T66ResolveRunSummaryDamageSourceName(GI, Entry.SourceID), FT66FlatStyle::Tokens::Text, RunSummaryBodyFont())]]
					+ SHorizontalBox::Slot().AutoWidth()
					[SNew(SBox).WidthOverride(DamageValueColumnWidth)[MakeDamageCellText(FText::AsNumber(Entry.TotalDamage), FT66FlatStyle::Tokens::Text, RunSummaryBodyFont(), ETextJustify::Right)]]
				];
		}

		return TableBox;
	};

	TSharedRef<bool> bShowingDamageReceived = MakeShared<bool>(false);
	TSharedRef<SWidget> DamageDealtTable = MakeDamageTable(DamageDealtRows, NSLOCTEXT("T66.RunSummary", "NoDamageDealt", "No damage dealt."));
	TSharedRef<SWidget> DamageReceivedTable = MakeDamageTable(DamageReceivedRows, NSLOCTEXT("T66.RunSummary", "NoDamageReceived", "No damage received."));
	TSharedRef<SVerticalBox> DamageBySourceBox = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				MakeRunSummarySpriteButton(
					NSLOCTEXT("T66.RunSummary", "DamageDealtButton", "Damage Dealt"),
					FOnClicked::CreateLambda([bShowingDamageReceived]()
					{
						*bShowingDamageReceived = false;
						return FReply::Handled();
					}),
					ET66RunSummaryButtonFamily::CompactNeutral,
					150.f,
					28.f,
					10,
					FMargin(8.f, 4.f, 8.f, 4.f))
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeRunSummarySpriteButton(
					NSLOCTEXT("T66.RunSummary", "DamageReceivedButton", "Damage Received"),
					FOnClicked::CreateLambda([bShowingDamageReceived]()
					{
						*bShowingDamageReceived = true;
						return FReply::Handled();
					}),
					ET66RunSummaryButtonFamily::CompactNeutral,
					166.f,
					28.f,
					10,
					FMargin(8.f, 4.f, 8.f, 4.f))
			]
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBox)
			.Visibility(TAttribute<EVisibility>::CreateLambda([bShowingDamageReceived]()
			{
				return *bShowingDamageReceived ? EVisibility::Collapsed : EVisibility::Visible;
			}))
			[
				DamageDealtTable
			]
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBox)
			.Visibility(TAttribute<EVisibility>::CreateLambda([bShowingDamageReceived]()
			{
				return *bShowingDamageReceived ? EVisibility::Visible : EVisibility::Collapsed;
			}))
			[
				DamageReceivedTable
			]
		];

	TSharedRef<SWidget> DamageBySourcePanel = MakeRunSummarySpritePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.RunSummary", "DamageBySourcePanel", "DAMAGE BY SOURCE"))
			.TextStyle(&FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading")))
			.Font(RunSummaryHeadingFont())
		]
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			DamageBySourceBox
		],
		GetRunSummaryGeneratedDamagePanelBrush(),
		FT66FlatStyle::Tokens::Space4);

	const TAttribute<FMargin> SafeBackPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66FlatStyle::GetSafePadding(FMargin(20.f, 0.f, 0.f, 20.f));
	});

	const TAttribute<FMargin> SafeDrawerPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66FlatStyle::GetSafePadding(FMargin(0.f, 70.f, 0.f, 0.f));
	});

	const TAttribute<FOptionalSize> EventLogWidthAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		const FVector2D SafeFrame = FT66FlatStyle::GetSafeFrameSize();
		return FOptionalSize(FMath::Clamp(SafeFrame.X * 0.42f, 420.f, 520.f));
	});

	const TAttribute<FOptionalSize> EventLogHeightAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		const FVector2D SafeFrame = FT66FlatStyle::GetSafeFrameSize();
		return FOptionalSize(FMath::Clamp(SafeFrame.Y - 140.f, 420.f, 620.f));
	});

	constexpr float DossierLeftX = 60.f;
	constexpr float DossierMidX = 510.f;
	constexpr float DossierRightX = 1378.f;
	constexpr float DossierTopY = 135.f;

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66FlatStyle::Tokens::Bg)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.Visibility(EVisibility::Collapsed)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66FlatStyle::Scrim())
			]
			// Dossier-style Run Summary canvas: outcome/actions, hero/loadout, stats/damage.
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
					SNew(SBox)
					.WidthOverride(RunSummaryReferenceWidth)
					.HeightOverride(RunSummaryReferenceHeight)
					[
						SNew(SCanvas)
						+ SCanvas::Slot().Position(FVector2D(0.f, 0.f)).Size(FVector2D(RunSummaryReferenceWidth, RunSummaryReferenceHeight))
						[
							MakeRunSummarySpritePanel(SNew(SSpacer), GetRunSummaryContentShellBrush(), FMargin(0.f))
						]
							+ SCanvas::Slot().Position(FVector2D(DossierLeftX, 45.f)).Size(FVector2D(420.f, 55.f))
							[
								SNew(STextBlock)
								.Text(TitleText)
								.Font(RunSummaryBoldFont(36))
								.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
								.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							]
							+ SCanvas::Slot().Position(FVector2D(DossierMidX + 10.f, 43.f)).Size(FVector2D(1010.f, 62.f))
							[
								TopSummaryBadgesRow
							]
							+ SCanvas::Slot().Position(FVector2D(DossierRightX + 186.f, 42.f)).Size(FVector2D(274.f, 65.f))
							[
								MakeEventLogButton(NSLOCTEXT("T66.RunSummary", "EventLogTitle", "EVENT LOG"))
							]
							+ SCanvas::Slot().Position(FVector2D(DossierLeftX, DossierTopY)).Size(FVector2D(420.f, 165.f))
							[
								RunOutcomePanel
							]
							+ SCanvas::Slot().Position(FVector2D(DossierLeftX, 320.f)).Size(FVector2D(420.f, 112.f))
							[
								SNew(SBox)
								.Visibility_Lambda([this]() { return bViewingSavedLeaderboardRunSummary ? EVisibility::Collapsed : EVisibility::Visible; })
								[ bDailyClimbSummaryMode ? DailyRankPanel : WeeklyRankPanel ]
							]
							+ SCanvas::Slot().Position(FVector2D(DossierLeftX, 452.f)).Size(FVector2D(420.f, 112.f))
							[
								SNew(SBox)
								.Visibility_Lambda([this]() { return (!bViewingSavedLeaderboardRunSummary && !bDailyClimbSummaryMode) ? EVisibility::Visible : EVisibility::Collapsed; })
								[ AllTimeRankPanel ]
							]
							+ SCanvas::Slot().Position(FVector2D(DossierLeftX, 590.f)).Size(FVector2D(420.f, 120.f))
							[
								SeedLuckPanel
							]
							+ SCanvas::Slot().Position(FVector2D(DossierLeftX, 728.f)).Size(FVector2D(420.f, 120.f))
							[
								IntegrityPanel
							]
							+ SCanvas::Slot().Position(FVector2D(DossierLeftX + 39.f, 872.f)).Size(FVector2D(342.f, 190.f))
							[
								SNew(SBox)
								.Visibility_Lambda([this]() { return bViewingSavedLeaderboardRunSummary ? EVisibility::Collapsed : EVisibility::Visible; })
								[ ButtonsStack ]
							]
							+ SCanvas::Slot().Position(FVector2D(DossierLeftX, 320.f)).Size(FVector2D(578.f, 185.f))
							[
								SNew(SBox)
								.Visibility_Lambda([this]() { return bViewingSavedLeaderboardRunSummary ? EVisibility::Visible : EVisibility::Collapsed; })
								[ ProofOfRunPanel ]
							]
							+ SCanvas::Slot().Position(FVector2D(DossierLeftX + 39.f, 526.f)).Size(FVector2D(342.f, 65.f))
							[
								SNew(SBox)
								.Visibility_Lambda([this]() { return bViewingSavedLeaderboardRunSummary ? EVisibility::Visible : EVisibility::Collapsed; })
								[ ReportCheatButton ]
							]
							+ SCanvas::Slot().Position(FVector2D(DossierLeftX, 610.f)).Size(FVector2D(420.f, 260.f))
							[
								SNew(SBox)
								.Visibility_Lambda([this]() { return (bViewingSavedLeaderboardRunSummary && bReportPromptVisible) ? EVisibility::Visible : EVisibility::Collapsed; })
								[ ReportPrompt ]
							]
							+ SCanvas::Slot().Position(FVector2D(DossierMidX + 190.f, DossierTopY)).Size(FVector2D(498.f, 452.f))
							[
								MakeHeroPreview()
							]
							+ SCanvas::Slot().Position(FVector2D(DossierMidX + 90.f, 600.f)).Size(FVector2D(720.f, 106.f))
							[
								IdolsBorderedGrid
							]
							+ SCanvas::Slot().Position(FVector2D(DossierMidX, 758.f)).Size(FVector2D(884.f, 195.f))
							[
								InventorySlotGrid
							]
							+ SCanvas::Slot().Position(FVector2D(DossierMidX + 704.f, 600.f)).Size(FVector2D(150.f, 112.f))
							[
								SNew(SBox)
								.Visibility(bHasAnyTemporaryBuff ? EVisibility::Visible : EVisibility::Collapsed)
								[ TemporaryBuffsPanel ]
							]
							+ SCanvas::Slot().Position(FVector2D(DossierRightX, DossierTopY)).Size(FVector2D(460.f, 365.f))
							[
								MakeRunSummarySpritePanel(BaseStatsPanel, GetRunSummaryStatsPanelBrush(), FMargin(28.f, 24.f, 24.f, 22.f))
							]
							+ SCanvas::Slot().Position(FVector2D(DossierRightX, 528.f)).Size(FVector2D(460.f, 410.f))
							[
								DamageBySourcePanel
							]
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
								.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
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
									.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
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
								.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
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
									.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
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
						FLinearColor(0.04f, 0.04f, 0.07f, 0.96f))
				]
			]
		];
}

FReply UT66RunSummaryScreen::HandleRestartClicked() { OnRestartClicked(); return FReply::Handled(); }
FReply UT66RunSummaryScreen::HandleMainMenuClicked() { OnMainMenuClicked(); return FReply::Handled(); }
FReply UT66RunSummaryScreen::HandleViewLogClicked() { OnViewLogClicked(); return FReply::Handled(); }
FReply UT66RunSummaryScreen::HandleFlatRunSummaryStatTabClicked(const int32 TabIndex)
{
	FlatRunSummaryStatTabIndex = FMath::Clamp(TabIndex, 0, 2);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66RunSummaryScreen::HandleProofCopyClicked()
{
	const FString UrlToCopy = ProofOfRunUrl.IsEmpty()
		? FString(TEXT("youtube.com/watch?v=run-proof-001"))
		: ProofOfRunUrl;
	FPlatformApplicationMisc::ClipboardCopy(*UrlToCopy);
	return FReply::Handled();
}

FReply UT66RunSummaryScreen::HandleStatsExpandClicked()
{
	bStatsExpanded = !bStatsExpanded;
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

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

	RequestDeferredSlateRebuild();
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
			SummaryChadCouponsSourceLabel = TEXT("Daily Descent reward");
		}
	}
	else
	{
		SummaryChadCouponsEarned = 0;
	}

	ResolveChadCouponsPopupForLiveRun(Status == TEXT("accepted") && !bLiveRunCheatFlagged);
	RequestDeferredSlateRebuild();
}

void UT66RunSummaryScreen::OnRestartClicked()
{
	CloseRunSummaryPreviewVideo();
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
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
	{
		T66GI->TransitionToGameplayLevel();
		return;
	}

	UGameplayStatics::OpenLevel(this, UT66GameInstance::GetGameplayLevelName());
}

void UT66RunSummaryScreen::OnMainMenuClicked()
{
	CloseRunSummaryPreviewVideo();
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

