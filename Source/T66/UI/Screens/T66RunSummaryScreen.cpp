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
#include "Core/T66SaveMigration.h"
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
#include "UI/Style/T66FriendslopStyle.h"
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

namespace
{
	int32 T66RunSummaryInventoryRarityRank(const ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::White:
			return 0;
		case ET66ItemRarity::Yellow:
			return 1;
		case ET66ItemRarity::Red:
			return 2;
		case ET66ItemRarity::Black:
		default:
			return 3;
		}
	}

	void T66SortInventorySlotsByRarityForSummary(TArray<FT66InventorySlot>& Slots)
	{
		Slots.StableSort([](const FT66InventorySlot& A, const FT66InventorySlot& B)
		{
			const int32 RankA = T66RunSummaryInventoryRarityRank(A.Rarity);
			const int32 RankB = T66RunSummaryInventoryRarityRank(B.Rarity);
			if (RankA != RankB)
			{
				return RankA < RankB;
			}

			return A.ItemTemplateID.ToString() < B.ItemTemplateID.ToString();
		});
	}

	void T66CopyInventorySlotIDsForSummary(const TArray<FT66InventorySlot>& Slots, TArray<FName>& OutItemIDs)
	{
		OutItemIDs.Reset(Slots.Num());
		for (const FT66InventorySlot& Slot : Slots)
		{
			OutItemIDs.Add(Slot.ItemTemplateID);
		}
	}
}

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
	if (!bConsumedRequest && bViewingSavedLeaderboardRunSummary)
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

	bool T66TryResolveKnownTrapDamageSourceName(const FName SourceID, FText& OutName)
	{
		if (SourceID == FName(TEXT("DungeonWallArrow")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_DungeonWallArrow", "Dungeon Wall Arrow Trap");
			return true;
		}
		if (SourceID == FName(TEXT("DungeonFloorFlame")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_DungeonFloorFlame", "Dungeon Floor Flame Trap");
			return true;
		}
		if (SourceID == FName(TEXT("DungeonFloorSpikePatch")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_DungeonFloorSpikePatch", "Dungeon Floor Spike Trap");
			return true;
		}
		if (SourceID == FName(TEXT("ForestThornVolley")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_ForestThornVolley", "Forest Thorn Volley Trap");
			return true;
		}
		if (SourceID == FName(TEXT("ForestSporeBurst")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_ForestSporeBurst", "Forest Spore Burst Trap");
			return true;
		}
		if (SourceID == FName(TEXT("ForestBramblePatch")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_ForestBramblePatch", "Forest Bramble Patch Trap");
			return true;
		}
		if (SourceID == FName(TEXT("OceanHarpoonVolley")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_OceanHarpoonVolley", "Ocean Harpoon Volley Trap");
			return true;
		}
		if (SourceID == FName(TEXT("OceanSteamBurst")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_OceanSteamBurst", "Ocean Steam Burst Trap");
			return true;
		}
		if (SourceID == FName(TEXT("OceanUrchinPatch")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_OceanUrchinPatch", "Ocean Urchin Patch Trap");
			return true;
		}
		if (SourceID == FName(TEXT("MartianShardVolley")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_MartianShardVolley", "Martian Shard Volley Trap");
			return true;
		}
		if (SourceID == FName(TEXT("MartianPlasmaBurst")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_MartianPlasmaBurst", "Martian Plasma Burst Trap");
			return true;
		}
		if (SourceID == FName(TEXT("MartianCrystalPatch")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_MartianCrystalPatch", "Martian Crystal Patch Trap");
			return true;
		}
		if (SourceID == FName(TEXT("HellSoulBoltVolley")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_HellSoulBoltVolley", "Hell Soul Bolt Volley Trap");
			return true;
		}
		if (SourceID == FName(TEXT("HellEmberBurst")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_HellEmberBurst", "Hell Ember Burst Trap");
			return true;
		}
		if (SourceID == FName(TEXT("HellBrimstonePatch")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_HellBrimstonePatch", "Hell Brimstone Patch Trap");
			return true;
		}
		if (SourceID == FName(TEXT("WallArrow")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_WallArrow", "Wall Arrow Trap");
			return true;
		}
		if (SourceID == FName(TEXT("FloorFlame")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_FloorFlame", "Floor Flame Trap");
			return true;
		}
		if (SourceID == FName(TEXT("FloorSpikePatch")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_FloorSpikePatch", "Floor Spike Patch Trap");
			return true;
		}
		if (SourceID == FName(TEXT("WallProjectile")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_WallProjectile", "Wall Projectile Trap");
			return true;
		}
		if (SourceID == FName(TEXT("FloorBurst")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_FloorBurst", "Floor Burst Trap");
			return true;
		}
		if (SourceID == FName(TEXT("AreaControl")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_AreaControl", "Area Control Trap");
			return true;
		}
		if (SourceID == FName(TEXT("Trap")))
		{
			OutName = NSLOCTEXT("T66.RunSummary", "DamageSource_Trap", "Trap");
			return true;
		}

		return false;
	}

	TArray<FDamageLogEntry> T66BuildRunSummaryDamageRowsFromMap(const TMap<FName, int32>& DamageMap)
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
	}

	void T66CollectRunSummaryDamageRows(
		UGameInstance* GI,
		const UT66LeaderboardRunSummarySaveGame* LoadedSavedSummary,
		const bool bViewingSavedLeaderboardRunSummary,
		TArray<FDamageLogEntry>& OutDamageDealtRows,
		TArray<FDamageLogEntry>& OutDamageReceivedRows)
	{
		OutDamageDealtRows.Reset();
		OutDamageReceivedRows.Reset();

		if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
		{
			OutDamageDealtRows = T66BuildRunSummaryDamageRowsFromMap(LoadedSavedSummary->DamageBySource);
			OutDamageReceivedRows = T66BuildRunSummaryDamageRowsFromMap(LoadedSavedSummary->DamageReceivedBySource);
			return;
		}

		if (UT66DamageLogSubsystem* DamageLog = GI ? GI->GetSubsystem<UT66DamageLogSubsystem>() : nullptr)
		{
			OutDamageDealtRows = DamageLog->GetDamageBySourceSorted();
			OutDamageReceivedRows = DamageLog->GetDamageReceivedBySourceSorted();
		}
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

		FText KnownTrapName;
		if (T66TryResolveKnownTrapDamageSourceName(SourceID, KnownTrapName))
		{
			return KnownTrapName;
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

	const bool bHighScoreMode = PS ? PS->GetHighScoreMode() : true;
	const bool bSpeedRunMode = PS ? PS->GetSpeedRunMode() : true;
	const bool bShouldSubmitTime =
		(RunState->DidRunEndInVictory() || bTreatAsVictoryForTime)
		&& PS
		&& bSpeedRunMode;
	bool bSubmittedTime = false;
	bool bSubmittedScore = false;
	if (bHighScoreMode && bShouldSubmitTime)
	{
		bSubmittedTime = LB->SubmitDifficultyClearRun(RunState->GetFinalRunElapsedSeconds(), SavedRunSummarySlotName);
		bSubmittedScore = bSubmittedTime;
		bNewPersonalBestScore = LB->WasLastScoreNewPersonalBest();
		bNewPersonalBestTime = LB->WasLastCompletedRunTimeNewPersonalBest();
	}
	else if (bShouldSubmitTime)
	{
		bSubmittedTime = LB->SubmitCompletedRunTime(RunState->GetFinalRunElapsedSeconds(), SavedRunSummarySlotName);
		bNewPersonalBestScore = false;
		bNewPersonalBestTime = LB->WasLastCompletedRunTimeNewPersonalBest();
	}
	else if (bHighScoreMode)
	{
		bSubmittedScore = LB->SubmitRunScore(RunState->GetCurrentScore(), SavedRunSummarySlotName);
		bNewPersonalBestScore = LB->WasLastScoreNewPersonalBest();
		bNewPersonalBestTime = false;
	}
	else
	{
		UE_LOG(LogT66RunSummary, Log, TEXT("Run Summary: leaderboard submission skipped because no leaderboard submission mode is enabled."));
		bNewPersonalBestScore = false;
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
	// Capture automation: -T66RunSummarySavedFixture renders the saved/leaderboard
	// variant with a deterministic fixture snapshot (no leaderboard interaction needed).
	if (FParse::Param(FCommandLine::Get(), TEXT("T66RunSummarySavedFixture")))
	{
		if (!bViewingSavedLeaderboardRunSummary || !LoadedSavedSummary)
		{
			ResetSavedRunSummaryViewerState();
			UT66LeaderboardRunSummarySaveGame* Fixture = NewObject<UT66LeaderboardRunSummarySaveGame>(this);
			Fixture->SchemaVersion = 3;
			Fixture->ProofOfRunUrl = TEXT("youtube.com/watch?v=run-proof-001");
			Fixture->bProofOfRunLocked = false;
			Fixture->OwnerDisplayName = TEXT("Solobro");
			LoadedSavedSummary = Fixture;
			bViewingSavedLeaderboardRunSummary = true;
		}
		return true;
	}

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
			T66MigrateLocalRunSummarySaveFields(
				LoadedSavedSummary->SchemaVersion,
				LoadedSavedSummary->HeroID,
				LoadedSavedSummary->EquippedIdols,
				LoadedSavedSummary->EquippedIdolTiers);
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
		// Hellfire palette (approved RunSummary v2): the old purple accent becomes flame
		// gold at every call site; dividers dim ember.
		const FLinearColor Purple = FLinearColor(0.98f, 0.76f, 0.22f, 1.f);
		const FLinearColor Red = FT66FlatStyle::SelectedText();
		const FLinearColor White = FLinearColor(0.99f, 0.94f, 0.88f, 1.f);
		const FLinearColor DimLine(0.55f, 0.12f, 0.07f, 0.5f);
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
		const bool bShowFlatProofActions = bViewingSavedLeaderboardRunSummary;
		const FText FlatScoreRankLabelText = NSLOCTEXT("T66.RunSummary", "ScoreRankLabel", "Score");
		const FText FlatSpeedRunRankLabelText = NSLOCTEXT("T66.RunSummary", "SpeedRunRankLabel", "Speed Run");
		FString AutoDumpPathForReferenceFixture;
		const bool bFlatReferenceCapture = FParse::Value(FCommandLine::Get(), TEXT("T66AutoDumpScreen="), AutoDumpPathForReferenceFixture);
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
		TArray<FDamageLogEntry> FlatDamageDealtRows;
		TArray<FDamageLogEntry> FlatDamageReceivedRows;
		T66CollectRunSummaryDamageRows(
			GI,
			(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary.Get() : nullptr,
			bViewingSavedLeaderboardRunSummary,
			FlatDamageDealtRows,
			FlatDamageReceivedRows);

		const TArray<FName>* FlatIdolsPtr = nullptr;
		TArray<FName> FlatInventoryLocal;
		const TArray<FT66InventorySlot>* FlatInventorySlotsPtr = nullptr;
		TArray<FT66InventorySlot> FlatInventorySlotsLocal;
		if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
		{
			FlatIdolsPtr = &LoadedSavedSummary->EquippedIdols;
			if (LoadedSavedSummary->InventorySlots.Num() > 0)
			{
				FlatInventorySlotsLocal = LoadedSavedSummary->InventorySlots;
				T66SortInventorySlotsByRarityForSummary(FlatInventorySlotsLocal);
				FlatInventorySlotsPtr = &FlatInventorySlotsLocal;
				T66CopyInventorySlotIDsForSummary(FlatInventorySlotsLocal, FlatInventoryLocal);
			}
			else
			{
				FlatInventoryLocal = LoadedSavedSummary->Inventory;
			}
		}
		else if (RunState)
		{
			if (UT66IdolManagerSubsystem* IdolManager = GIBase ? GIBase->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr)
			{
				FlatIdolsPtr = &IdolManager->GetEquippedIdols();
			}
			else
			{
				FlatIdolsPtr = &RunState->GetEquippedIdols();
			}
			RunState->GetInventorySlotsSortedByRarity(FlatInventorySlotsLocal);
			FlatInventorySlotsPtr = &FlatInventorySlotsLocal;
			T66CopyInventorySlotIDsForSummary(FlatInventorySlotsLocal, FlatInventoryLocal);
		}
		const TArray<FName> EmptyFlatIdols;
		const TArray<FName>& FlatIdols = FlatIdolsPtr ? *FlatIdolsPtr : EmptyFlatIdols;

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
		auto AddNTab = [&Canvas, this](const int32 TabIndex, const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
		{
			Canvas->AddSlot()
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X * CanvasW, Y * CanvasH, W * CanvasW, H * CanvasH))
			[
				SNew(SBox)
				.Visibility_Lambda([this, TabIndex]()
				{
					return FlatRunSummaryStatTabIndex == TabIndex ? EVisibility::Visible : EVisibility::Collapsed;
				})
				[
					Widget
				]
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
		// Hellfire transplant chokepoint (approved RunSummary v2 pair): every panel and
		// button shell on this screen resolves its plate here via a per-tag map.
		const FString HellfireRunSummaryDir = TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/RunSummary/");
		auto GetHellfirePlateBrush = [&HellfireRunSummaryDir](const TCHAR* File, const FMargin& BoxMargin, const ESlateBrushDrawType::Type DrawAs, const FVector2D& Size) -> const FSlateBrush*
		{
			return FT66FriendslopStyle::GetCustomBrush(HellfireRunSummaryDir + File, BoxMargin, DrawAs, Size);
		};
		auto MakePanel = [&GetHellfirePlateBrush](const ET66FlatState State, const FName Tag) -> TSharedRef<SWidget>
		{
			(void)State;
			const FString TagString = Tag.ToString();
			// Skull strip backing panel is gone in the reference (wells sit on the backdrop);
			// small wells get their own image plates instead of the 9-sliced big panel.
			if (TagString.Contains(TEXT(".SkullProgressPanel")) && !TagString.Contains(TEXT(".Skull0")))
			{
				return FT66FlatStyle::AttachMetadata(SNew(SBox), Tag, TEXT("Panel"), ET66FlatState::Default);
			}
			const FSlateBrush* PlateBrush = nullptr;
			if (TagString.Contains(TEXT(".Skull0")))
			{
				// Skull wells reuse the idol well plate (same look in the reference; the
				// dedicated skull extraction lost its red ring twice — superseded).
				PlateBrush = GetHellfirePlateBrush(TEXT("rs_well_idol.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(102.f, 92.f));
			}
			else if (TagString.Contains(TEXT(".IdolsPanel.Idol")))
			{
				PlateBrush = GetHellfirePlateBrush(TEXT("rs_well_idol.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(108.f, 102.f));
			}
			else if (TagString.Contains(TEXT(".InventorySlot")))
			{
				PlateBrush = GetHellfirePlateBrush(TEXT("rs_well_inv.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(74.f, 68.f));
			}
			else if (TagString.Contains(TEXT(".ProofUrlField")))
			{
				PlateBrush = GetHellfirePlateBrush(TEXT("rs_field.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(478.f, 58.f));
			}
			else
			{
				// All large panels share one 9-sliced plate (corner caps ~16% of the
				// authored 536x228; legal for every panel here, min height ~130).
				PlateBrush = GetHellfirePlateBrush(TEXT("rs_panel.png"), FMargin(0.075f, 0.165f, 0.075f, 0.165f), ESlateBrushDrawType::Box, FVector2D(536.f, 228.f));
			}
			if (!PlateBrush)
			{
				return FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(0.f), SNullWidget::NullWidget, nullptr, Tag);
			}
			return FT66FlatStyle::AttachMetadata(
				SNew(SBorder)
				.BorderImage(PlateBrush)
				.Padding(FMargin(0.f))
				.Visibility(EVisibility::HitTestInvisible)
				[
					SNullWidget::NullWidget
				],
				Tag,
				TEXT("Panel"),
				ET66FlatState::Default);
		};
		auto MakeButtonShell = [&GetHellfirePlateBrush](
			const ET66FlatState State,
			FOnClicked OnClicked,
			const FName Tag,
			const FName ToggleGroup = NAME_None) -> TSharedRef<SWidget>
		{
			(void)ToggleGroup;
			const FString TagString = Tag.ToString();
			const TCHAR* PlateFile = TEXT("rs_pill.png");
			FVector2D PlateSize(600.f, 118.f);
			if (TagString.Contains(TEXT(".GoAgainButton")))
			{
				PlateFile = TEXT("rs_cta_lava.png");
				PlateSize = FVector2D(594.f, 124.f);
			}
			else if (TagString.Contains(TEXT(".StatTabs")))
			{
				const bool bSelectedShell = TagString.Contains(TEXT(".SelectedShell"));
				PlateFile = bSelectedShell ? TEXT("rs_tab_on.png") : TEXT("rs_tab_off.png");
				PlateSize = FVector2D(168.f, 64.f);
			}
			else if (TagString.Contains(TEXT(".CopyButton")))
			{
				PlateFile = TEXT("rs_btn_copy.png");
				PlateSize = FVector2D(82.f, 76.f);
			}
			else if (TagString.Contains(TEXT(".BackButton")))
			{
				PlateFile = TEXT("rs_back.png");
				PlateSize = FVector2D(223.f, 80.f);
			}
			const FSlateBrush* PlateBrush = GetHellfirePlateBrush(PlateFile, FMargin(0.f), ESlateBrushDrawType::Image, PlateSize);
			if (!PlateBrush)
			{
				return FT66FlatStyle::MakeFlatToggleGroupButton(State, SNullWidget::NullWidget, MoveTemp(OnClicked), FMargin(0.f), 0.f, 0.f, true, Tag, NAME_None);
			}
			TSharedPtr<SButton> ButtonPtr;
			TSharedRef<SButton> Button = SAssignNew(ButtonPtr, SButton)
				.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("NoBorder")))
				.ContentPadding(FMargin(0.f))
				.ClickMethod(EButtonClickMethod::MouseDown)
				.OnClicked(MoveTemp(OnClicked))
				[
					SNew(SImage)
					.Image(PlateBrush)
				];
			// Canonical juice: scale pop only (hover 1.03 / press 0.97, center pivot).
			TWeakPtr<SButton> WeakButton = ButtonPtr;
			Button->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
			Button->SetRenderTransform(TAttribute<TOptional<FSlateRenderTransform>>::CreateLambda(
				[WeakButton]() -> TOptional<FSlateRenderTransform>
				{
					const TSharedPtr<SButton> Pinned = WeakButton.Pin();
					const float Scale = (Pinned.IsValid() && Pinned->IsPressed())
						? 0.97f
						: (Pinned.IsValid() && Pinned->IsHovered()) ? 1.03f : 1.f;
					return FSlateRenderTransform(FScale2D(Scale));
				}));
			return FT66FlatStyle::AttachMetadata(
				Button,
				Tag,
				TEXT("Button"),
				State,
				TOptional<FLinearColor>(),
				true,
				NAME_None,
				false,
				true);
		};
		auto MakeStatTabButton = [this, &MakeButtonShell, StatTabsGroup](
			const FName Tag,
			const int32 TabIndex) -> TSharedRef<SWidget>
		{
			const FString TagString = Tag.ToString();
			TSharedRef<SOverlay> ButtonOverlay = SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBox)
					.Visibility_Lambda([this, TabIndex]()
					{
						return FlatRunSummaryStatTabIndex == TabIndex ? EVisibility::Visible : EVisibility::Collapsed;
					})
					[
						MakeButtonShell(
							ET66FlatState::Selected,
							FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleFlatRunSummaryStatTabClicked, TabIndex),
							FName(*(TagString + TEXT(".SelectedShell"))),
							StatTabsGroup)
					]
				]
				+ SOverlay::Slot()
				[
					SNew(SBox)
					.Visibility_Lambda([this, TabIndex]()
					{
						return FlatRunSummaryStatTabIndex == TabIndex ? EVisibility::Collapsed : EVisibility::Visible;
					})
					[
						MakeButtonShell(
							ET66FlatState::Default,
							FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleFlatRunSummaryStatTabClicked, TabIndex),
							FName(*(TagString + TEXT(".DefaultShell"))),
							StatTabsGroup)
					]
				];

			return FT66FlatStyle::AttachMetadata(
				ButtonOverlay,
				Tag,
				TEXT("ToggleButton"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				true,
				StatTabsGroup,
				false,
				true);
		};
		auto MakeStatTabLabel = [this, Red, Purple](
			const FName Tag,
			const FText& Text,
			const int32 FontSize,
			const int32 TabIndex) -> TSharedRef<SWidget>
		{
			TSharedRef<STextBlock> Label = SNew(STextBlock)
				.Text(Text)
				.Font(FT66FlatStyle::MakeBoldFont(FontSize))
				.ColorAndOpacity_Lambda([this, TabIndex, Red, Purple]()
				{
					return FlatRunSummaryStatTabIndex == TabIndex ? Red : Purple;
				})
				.Justification(ETextJustify::Center)
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
		auto MakeTaggedBox = [](const FName Tag, const FString& Role = TEXT("Region")) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(SNew(SBox), Tag, Role, ET66FlatState::Default);
		};
		auto MakeIcon = [&MakeLabel, &HellfireRunSummaryDir](const FName Tag, const TCHAR* Path, const FVector2D& Size, const FLinearColor& Tint, const FText& Fallback) -> TSharedRef<SWidget>
		{
			// The old flat icons are purple-authored; prefer the white-masked hellfire
			// copy (tintable) when one exists at Hellfire/RunSummary/icons/<name>.png.
			FString ResolvedPath(Path);
			const FString WhiteCopy = HellfireRunSummaryDir + TEXT("icons/") + FPaths::GetCleanFilename(ResolvedPath);
			for (const FString& Candidate : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(WhiteCopy))
			{
				if (FPaths::FileExists(Candidate))
				{
					ResolvedPath = WhiteCopy;
					break;
				}
			}
			static TMap<FString, FT66RunSummarySpriteBrushEntry> FlatIconEntries;
			FT66RunSummarySpriteBrushEntry& Entry = FlatIconEntries.FindOrAdd(ResolvedPath);
			const FSlateBrush* Brush = ResolveRunSummarySpriteBrush(
				Entry,
				ResolvedPath,
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
		auto AddDividerTab = [&](const int32 TabIndex, const float X, const float Y, const float W)
		{
			AddNTab(TabIndex, X, Y, W, 0.002f, MakeRect(DimLine, NAME_None, TEXT("Divider")));
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
		auto AddRightStatLineTab = [&](const int32 TabIndex, const float Y, const TCHAR* Tag, const FText& Text)
		{
			AddNTab(TabIndex, 0.688f, Y, 0.260f, 0.036f, MakeLabel(DTag(Tag), Text, 28, White, false));
			AddDividerTab(TabIndex, 0.684f, Y + 0.043f, 0.286f);
		};
		auto AddTopStatPanel = [&](const float X, const TCHAR* PanelTag, const TCHAR* IconPath, const FText& Label, const FText& Value, const FText& Fallback)
		{
			const FString Prefix(PanelTag);
			AddN(X, 0.017f, 0.135f, 0.080f, MakePanel(ET66FlatState::Default, FName(*Prefix)));
			AddN(X + 0.014f, 0.034f, 0.032f, 0.048f, MakeIcon(FName(*(Prefix + TEXT(".Icon"))), IconPath, FVector2D(56.f, 56.f), Purple, Fallback));
			AddN(X + 0.048f, 0.031f, 0.075f, 0.024f, MakeLabel(FName(*(Prefix + TEXT(".Label"))), Label, 16, Purple, true, ETextJustify::Center));
			AddN(X + 0.065f, 0.055f, 0.030f, 0.032f, MakeLabel(FName(*(Prefix + TEXT(".Value"))), Value, 28, White, true, ETextJustify::Center));
		};

		// Hellfire lava-crackle backdrop (shared plate); black fallback when absent.
		if (const FSlateBrush* BackdropBrush = FT66FriendslopStyle::GetCustomBrush(
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Diplomas/diplomas_backdrop.png"),
			FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(1920.f, 1080.f)))
		{
			AddN(0.f, 0.f, 1.f, 1.f, FT66FlatStyle::AttachMetadata(
				SNew(SImage).Image(BackdropBrush).Visibility(EVisibility::HitTestInvisible),
				DTag(TEXT("RunSummary.Background")), TEXT("Background"), ET66FlatState::Default));
		}
		else
		{
			AddN(0.f, 0.f, 1.f, 1.f, MakeRect(FT66FlatStyle::BackgroundColor(), DTag(TEXT("RunSummary.Background")), TEXT("Background")));
		}
		AddN(0.018f, 0.017f, 0.964f, 0.925f, MakeTaggedBox(DTag(TEXT("RunSummary.Root")), TEXT("ScreenRoot")));
		// Approved v2: title centered at top.
		AddN(0.300f, 0.016f, 0.400f, 0.052f, MakeLabel(DTag(TEXT("RunSummary.Title")), TitleText, 48, White, true, ETextJustify::Center));
		// Approved v2 (saved variant): BACK pill at top-left returns to the main menu
		// (where the leaderboard lives).
		if (bShowFlatProofActions)
		{
			AddN(0.0188f, 0.0352f, 0.1161f, 0.0741f, MakeButtonShell(ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleMainMenuClicked), DTag(TEXT("RunSummary.BackButton"))));
			AddN(0.0290f, 0.0500f, 0.0220f, 0.0440f, MakeIcon(DTag(TEXT("RunSummary.BackButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/back_chevron.png"), FVector2D(44.f, 44.f), Purple, FText::FromString(TEXT("<"))));
			AddN(0.0560f, 0.0520f, 0.0660f, 0.0400f, MakeLabel(DTag(TEXT("RunSummary.BackButton.Label")), Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK"), 30, White, true, ETextJustify::Center));
		}

		AddN(0.7344f, 0.0259f, 0.2417f, 0.0778f, MakeButtonShell(ET66FlatState::Selected, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleViewLogClicked), DTag(TEXT("RunSummary.EventLogButton"))));
		AddN(0.7560f, 0.0410f, 0.0290f, 0.0480f, MakeIcon(DTag(TEXT("RunSummary.EventLogButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/log_clipboard.png"), FVector2D(58.f, 58.f), Purple, FText::FromString(TEXT("L"))));
		AddN(0.8000f, 0.0450f, 0.1300f, 0.0400f, MakeLabel(DTag(TEXT("RunSummary.EventLogButton.Label")), NSLOCTEXT("T66.RunSummary", "FlatEventLog", "EVENT LOG"), 34, White, true, ETextJustify::Center));

		// Left column rects measured from the approved live v2 reference (canonical).
		AddN(0.0271f, 0.1167f, 0.2729f, 0.1537f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Left.ProfilePanel"))));
		AddN(0.0440f, 0.1390f, 0.0440f, 0.0780f, MakeIcon(DTag(TEXT("RunSummary.Left.ProfilePanel.SteamIcon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/steam_placeholder.png"), FVector2D(64.f, 64.f), Purple, FText::FromString(TEXT("S"))));
		AddN(0.0980f, 0.1370f, 0.1900f, 0.0320f, MakeLabel(DTag(TEXT("RunSummary.Left.ProfilePanel.PlayerName")), FlatPlayerNameText, 26, White, true));
		AddN(0.0980f, 0.1700f, 0.1900f, 0.0280f, MakeLabel(DTag(TEXT("RunSummary.Left.ProfilePanel.HeroName")), FlatHeroNameText, 21, White, false));
		AddN(0.0980f, 0.2030f, 0.0900f, 0.0280f, MakeLabel(DTag(TEXT("RunSummary.Left.ProfilePanel.Rank")), FlatRankText, 19, Purple, true));
		AddN(0.1700f, 0.2030f, 0.1180f, 0.0280f, MakeLabel(DTag(TEXT("RunSummary.Left.ProfilePanel.Mastery")), FlatMasteryText, 19, Purple, true, ETextJustify::Right));

		AddN(0.0271f, 0.2870f, 0.2729f, 0.1907f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Left.RunOutcomePanel"))));
		AddN(0.0420f, 0.3060f, 0.0230f, 0.0390f, MakeFlagIcon(DTag(TEXT("RunSummary.Left.RunOutcomePanel.Icon"))));
		AddN(0.0680f, 0.3100f, 0.1400f, 0.0330f, MakeLabel(DTag(TEXT("RunSummary.Left.RunOutcomePanel.Header")), NSLOCTEXT("T66.RunSummary", "RunOutcomeHeader", "RUN OUTCOME"), 26, White, true));
		AddDivider(0.0420f, 0.3400f, 0.2430f);
		AddStatRow(0.3500f, TEXT("RunSummary.Left.RunOutcomePanel.StageRow"), NSLOCTEXT("T66.RunSummary", "OutcomeStageReached", "Stage Reached"), StageReachedValueText);
		AddDivider(0.0420f, 0.3790f, 0.2430f);
		AddStatRow(0.3890f, TEXT("RunSummary.Left.RunOutcomePanel.ScoreRow"), NSLOCTEXT("T66.RunSummary", "OutcomeScore", "Score"), ScoreValueText);
		AddDivider(0.0420f, 0.4180f, 0.2430f);
		AddStatRow(0.4280f, TEXT("RunSummary.Left.RunOutcomePanel.TimeRow"), NSLOCTEXT("T66.RunSummary", "OutcomeTime", "Time"), TimeValueText);

		AddN(0.0271f, 0.4810f, 0.2729f, 0.0930f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Left.SkullProgressPanel"))));
		for (int32 SkullIndex = 0; SkullIndex < 5; ++SkullIndex)
		{
			const FString SkullTag = FString::Printf(TEXT("RunSummary.Left.SkullProgressPanel.Skull%02d"), SkullIndex + 1);
			AddN(0.0310f + SkullIndex * 0.0550f, 0.4926f, 0.0460f, 0.0815f, MakePanel(ET66FlatState::Default, FName(*SkullTag)));
			AddN(0.0420f + SkullIndex * 0.0550f, 0.5070f, 0.0240f, 0.0500f, MakeIcon(NAME_None, TEXT("RuntimeDependencies/T66/UI/Icons/Flat/skull.png"), FVector2D(48.f, 48.f), Red, FText::FromString(TEXT("S"))));
		}

		AddN(0.0271f, 0.5926f, 0.2729f, 0.2000f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Left.RankPanel"))));
		AddN(0.0480f, 0.6180f, 0.1050f, 0.0360f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.WeeklyHeader")), NSLOCTEXT("T66.RunSummary", "WeeklyRankHeaderCaps", "WEEKLY RANK"), 23, Purple, true, ETextJustify::Center));
		AddN(0.1800f, 0.6180f, 0.1180f, 0.0360f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.AllTimeHeader")), NSLOCTEXT("T66.RunSummary", "AllTimeRankHeaderCaps", "ALL TIME RANK"), 23, Purple, true, ETextJustify::Center));
		AddN(0.1620f, 0.6140f, 0.0020f, 0.1500f, MakeRect(DimLine, NAME_None, TEXT("Divider")));
		AddN(0.0420f, 0.6660f, 0.0600f, 0.0280f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.WeeklyScoreLabel")), FlatScoreRankLabelText, 18, White, false));
		AddN(0.1280f, 0.6660f, 0.0350f, 0.0280f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.WeeklyScoreValue")), NSLOCTEXT("T66.RunSummary", "RankNAFlat1", "N/A"), 18, White, false));
		AddN(0.1800f, 0.6660f, 0.0600f, 0.0280f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.AllTimeScoreLabel")), FlatScoreRankLabelText, 18, White, false));
		AddN(0.2560f, 0.6660f, 0.0400f, 0.0280f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.AllTimeScoreValue")), NSLOCTEXT("T66.RunSummary", "RankNAFlat2", "N/A"), 18, White, false, ETextJustify::Right));
		AddN(0.0420f, 0.7160f, 0.0800f, 0.0280f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.WeeklySpeedLabel")), FlatSpeedRunRankLabelText, 18, White, false));
		AddN(0.1280f, 0.7160f, 0.0350f, 0.0280f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.WeeklySpeedValue")), NSLOCTEXT("T66.RunSummary", "RankNAFlat3", "N/A"), 18, White, false));
		AddN(0.1800f, 0.7160f, 0.0800f, 0.0280f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.AllTimeSpeedLabel")), FlatSpeedRunRankLabelText, 18, White, false));
		AddN(0.2560f, 0.7160f, 0.0400f, 0.0280f, MakeLabel(DTag(TEXT("RunSummary.Left.RankPanel.AllTimeSpeedValue")), NSLOCTEXT("T66.RunSummary", "RankNAFlat4", "N/A"), 18, White, false, ETextJustify::Right));

		AddN(0.0271f, 0.8111f, 0.2729f, 0.1278f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Left.SeedLuckPanel"))));
		AddN(0.0420f, 0.8330f, 0.0260f, 0.0460f, MakeIcon(DTag(TEXT("RunSummary.Left.SeedLuckPanel.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/clover.png"), FVector2D(48.f, 48.f), Purple, FText::FromString(TEXT("+"))));
		AddN(0.0700f, 0.8380f, 0.1200f, 0.0330f, MakeLabel(DTag(TEXT("RunSummary.Left.SeedLuckPanel.Header")), NSLOCTEXT("T66.RunSummary", "SeedLuckPanel", "SEED LUCK"), 26, White, true));
		AddDivider(0.0420f, 0.8780f, 0.2430f);
		AddN(0.0890f, 0.8900f, 0.1500f, 0.0360f, MakeLabel(DTag(TEXT("RunSummary.Left.SeedLuckPanel.Value")), FlatSeedLuckText, 24, White, true, ETextJustify::Center));

		AddN(0.3240f, 0.1148f, 0.3187f, 0.4093f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Middle.CharacterPreviewPanel"))));
		AddN(0.3380f, 0.1390f, 0.2900f, 0.3610f,
			GetRunSummaryPreviewVideoBrush()
				? FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(SNew(SImage).Image_UObject(this, &UT66RunSummaryScreen::GetRunSummaryPreviewVideoBrush)), DTag(TEXT("RunSummary.Middle.CharacterPreviewPanel.Preview")), TEXT("ContentArt"), ET66FlatState::Default)
				: MakeLabel(DTag(TEXT("RunSummary.Middle.CharacterPreviewPanel.Preview")), NSLOCTEXT("T66.RunSummary", "NoPreview", "No Preview"), 22, White, false, ETextJustify::Center));

		AddN(0.3240f, 0.5389f, 0.3177f, 0.1352f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Middle.IdolsPanel"))));
		AddN(0.3360f, 0.5500f, 0.0240f, 0.0420f, MakeIcon(DTag(TEXT("RunSummary.Middle.IdolsPanel.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/favorite_star_outline.png"), FVector2D(44.f, 44.f), Purple, FText::FromString(TEXT("*"))));
		AddN(0.3600f, 0.5540f, 0.0700f, 0.0340f, MakeLabel(DTag(TEXT("RunSummary.Middle.IdolsPanel.Header")), NSLOCTEXT("T66.RunSummary", "IdolsHeader", "IDOLS"), 28, White, true));
		constexpr int32 FlatIdolSlotCount = UT66IdolManagerSubsystem::MaxEquippedIdolSlots;
		constexpr float FlatIdolIconSize = 90.f;
		for (int32 IdolIndex = 0; IdolIndex < FlatIdolSlotCount; ++IdolIndex)
		{
			const FString IdolTag = FString::Printf(TEXT("RunSummary.Middle.IdolsPanel.Idol%02d"), IdolIndex + 1);
			// Approved v2: 3 wells (= MaxEquippedIdolSlots) spread across the panel.
			AddN(0.344f + IdolIndex * 0.120f, 0.5926f, 0.0420f, 0.0694f, MakePanel(ET66FlatState::Default, FName(*IdolTag)));
			const FName IdolID = FlatIdols.IsValidIndex(IdolIndex) ? FlatIdols[IdolIndex] : NAME_None;
			if (!IdolID.IsNone() && GI && TexPool)
			{
				FIdolData IdolData;
				if (GI->GetIdolData(IdolID, IdolData))
				{
					const TSoftObjectPtr<UTexture2D> IdolIconSoft = IdolData.Icon;
					if (!IdolIconSoft.IsNull())
					{
						TSharedPtr<FSlateBrush> IdolBrush = MakeShared<FSlateBrush>();
						IdolBrush->DrawAs = ESlateBrushDrawType::Image;
						IdolBrush->ImageSize = FVector2D(FlatIdolIconSize, FlatIdolIconSize);
						IdolBrush->SetResourceObject(nullptr);
						IdolIconBrushes.Add(IdolBrush);
						T66SlateTexture::BindSharedBrushAsync(TexPool, IdolIconSoft, this, IdolBrush, IdolID, true);

						const FName IdolIconTag = FName(*(IdolTag + TEXT(".Icon")));
						AddN(0.3490f + IdolIndex * 0.120f, 0.6010f, 0.0320f, 0.0540f,
							FT66FlatStyle::AttachMetadata(
								SNew(SImage)
								.Image(IdolBrush.Get())
								.ColorAndOpacity(FLinearColor::White)
								.Visibility(EVisibility::HitTestInvisible),
								IdolIconTag,
								TEXT("Icon"),
								ET66FlatState::Default));
					}
				}
			}
		}

		AddN(0.3240f, 0.6870f, 0.3177f, 0.2519f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Middle.InventoryPanel"))));
		AddN(0.330f, 0.702f, 0.024f, 0.042f, MakeIcon(DTag(TEXT("RunSummary.Middle.InventoryPanel.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/cube_box.png"), FVector2D(44.f, 44.f), Purple, FText::FromString(TEXT("B"))));
		AddN(0.354f, 0.706f, 0.135f, 0.034f, MakeLabel(DTag(TEXT("RunSummary.Middle.InventoryPanel.Header")), NSLOCTEXT("T66.RunSummary", "InventoryHeader", "INVENTORY"), 28, White, true));
		AddN(0.486f, 0.704f, 0.050f, 0.024f, MakeLabel(DTag(TEXT("RunSummary.Middle.InventoryPanel.GoldLabel")), NSLOCTEXT("T66.RunSummary", "GoldLabel", "GOLD"), 16, Purple, true, ETextJustify::Center));
		AddN(0.478f, 0.727f, 0.065f, 0.034f, MakeLabel(DTag(TEXT("RunSummary.Middle.InventoryPanel.GoldValue")), bHasFlatLiveContext ? FormatIntWithCommas(FlatGold) : FText::FromString(TEXT("1,275")), 24, White, true, ETextJustify::Center));
		AddN(0.550f, 0.708f, 0.002f, 0.052f, MakeRect(DimLine, NAME_None, TEXT("Divider")));
		AddN(0.552f, 0.704f, 0.050f, 0.024f, MakeLabel(DTag(TEXT("RunSummary.Middle.InventoryPanel.DebtLabel")), NSLOCTEXT("T66.RunSummary", "DebtLabel", "DEBT"), 16, Purple, true, ETextJustify::Center));
		AddN(0.546f, 0.727f, 0.060f, 0.034f, MakeLabel(DTag(TEXT("RunSummary.Middle.InventoryPanel.DebtValue")), bHasFlatLiveContext ? FormatIntWithCommas(FlatDebt) : FText::FromString(TEXT("320")), 24, White, true, ETextJustify::Center));
		AddN(0.589f, 0.708f, 0.002f, 0.052f, MakeRect(DimLine, NAME_None, TEXT("Divider")));
		AddN(0.596f, 0.704f, 0.075f, 0.024f, MakeLabel(DTag(TEXT("RunSummary.Middle.InventoryPanel.NetWorthLabel")), NSLOCTEXT("T66.RunSummary", "NetWorthLabel", "NET WORTH"), 16, Purple, true, ETextJustify::Center));
		AddN(0.607f, 0.727f, 0.050f, 0.034f, MakeLabel(DTag(TEXT("RunSummary.Middle.InventoryPanel.NetWorthValue")), bHasFlatLiveContext ? FormatIntWithCommas(FlatNetWorth) : FText::FromString(TEXT("955")), 24, White, true, ETextJustify::Center));
		// Approved v2: SCROLLABLE inventory grid (8 columns, simple flat wells, hellfire
		// scrollbar) replacing the fixed absolute-positioned 16 wells.
		{
			const FSlateBrush* InvWellBrush = GetHellfirePlateBrush(TEXT("rs_well_inv.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(74.f, 68.f));
			static FScrollBarStyle RunSummaryInvScrollStyle = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>(TEXT("ScrollBar"));
			static bool bRunSummaryInvScrollStyleInit = false;
			if (!bRunSummaryInvScrollStyleInit)
			{
				bRunSummaryInvScrollStyleInit = true;
				const FSlateBrush* ScrollTrack = FT66FriendslopStyle::GetCustomBrush(TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Diplomas/diplomas_scroll_track.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(24.f, 280.f));
				const FSlateBrush* ScrollThumb = FT66FriendslopStyle::GetCustomBrush(TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Diplomas/diplomas_scroll_thumb.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(18.f, 110.f));
				if (ScrollTrack && ScrollThumb)
				{
					RunSummaryInvScrollStyle
						.SetVerticalBackgroundImage(*ScrollTrack)
						.SetVerticalTopSlotImage(*ScrollTrack)
						.SetVerticalBottomSlotImage(*ScrollTrack)
						.SetNormalThumbImage(*ScrollThumb)
						.SetHoveredThumbImage(*ScrollThumb)
						.SetDraggedThumbImage(*ScrollThumb)
						.SetThickness(22.f);
				}
			}
			constexpr int32 FlatInvCols = 8;
			const int32 FlatInventorySlotCount = FMath::Max(16, FMath::DivideAndRoundUp(FMath::Max(FlatInventoryLocal.Num(), 1), FlatInvCols) * FlatInvCols);
			const int32 FlatInvRows = FlatInventorySlotCount / FlatInvCols;
			InventoryItemIconBrushes.SetNum(FlatInventorySlotCount);
			TSharedRef<SVerticalBox> InvRowsBox = SNew(SVerticalBox);
			for (int32 Row = 0; Row < FlatInvRows; ++Row)
			{
				TSharedRef<SHorizontalBox> RowBox = SNew(SHorizontalBox);
				for (int32 Col = 0; Col < FlatInvCols; ++Col)
				{
					const int32 SlotIndex = Row * FlatInvCols + Col;
					const FString SlotTag = FString::Printf(TEXT("RunSummary.Middle.InventorySlot%02d"), SlotIndex + 1);
					TSharedRef<SOverlay> Cell = SNew(SOverlay);
					if (InvWellBrush)
					{
						Cell->AddSlot()
						[
							SNew(SImage).Image(InvWellBrush).Visibility(EVisibility::HitTestInvisible)
						];
					}
					const FName ItemID = FlatInventoryLocal.IsValidIndex(SlotIndex) ? FlatInventoryLocal[SlotIndex] : NAME_None;
					if (!ItemID.IsNone() && GI && TexPool)
					{
						FItemData ItemData;
						const ET66ItemRarity SlotRarity = (FlatInventorySlotsPtr && FlatInventorySlotsPtr->IsValidIndex(SlotIndex))
							? (*FlatInventorySlotsPtr)[SlotIndex].Rarity
							: ET66ItemRarity::Black;
						if (GI->GetItemData(ItemID, ItemData))
						{
							const TSoftObjectPtr<UTexture2D> ItemIconSoft = ItemData.GetIconForRarity(SlotRarity);
							if (!ItemIconSoft.IsNull())
							{
								if (!InventoryItemIconBrushes[SlotIndex].IsValid())
								{
									InventoryItemIconBrushes[SlotIndex] = MakeShared<FSlateBrush>();
								}
								InventoryItemIconBrushes[SlotIndex]->DrawAs = ESlateBrushDrawType::Image;
								InventoryItemIconBrushes[SlotIndex]->ImageSize = FVector2D(52.f, 52.f);
								InventoryItemIconBrushes[SlotIndex]->SetResourceObject(nullptr);
								T66SlateTexture::BindSharedBrushAsync(TexPool, ItemIconSoft, this, InventoryItemIconBrushes[SlotIndex], ItemID, true);
								Cell->AddSlot()
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SImage)
									.Image(InventoryItemIconBrushes[SlotIndex].Get())
									.ColorAndOpacity(FLinearColor::White)
									.Visibility(EVisibility::HitTestInvisible)
								];
							}
						}
					}
					RowBox->AddSlot()
						.AutoWidth()
						.Padding(FMargin(Col > 0 ? 6.f : 0.f, 0.f, 0.f, 0.f))
						[
							FT66FlatStyle::AttachMetadata(
								SNew(SBox).WidthOverride(66.f).HeightOverride(60.f)[Cell],
								FName(*SlotTag),
								TEXT("Panel"),
								ET66FlatState::Default)
						];
				}
				InvRowsBox->AddSlot().AutoHeight().Padding(0.f, Row > 0 ? 6.f : 0.f, 0.f, 0.f)[RowBox];
			}
			AddN(0.327f, 0.758f, 0.322f, 0.146f,
				FT66FlatStyle::AttachMetadata(
					SNew(SScrollBox)
					.Orientation(Orient_Vertical)
					.ScrollBarStyle(&RunSummaryInvScrollStyle)
					.ScrollBarThickness(FVector2D(22.f, 22.f))
					.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
					.ScrollBarVisibility(EVisibility::Visible)
					+ SScrollBox::Slot()[InvRowsBox],
					DTag(TEXT("RunSummary.Middle.InventorySlotGrid")),
					TEXT("InventoryGrid"),
					ET66FlatState::Default));
		}

		AddN(0.6625f, 0.1204f, 0.0833f, 0.0537f, MakeStatTabButton(DTag(TEXT("RunSummary.Right.StatTabs.StatsButton")), 0));
		AddN(0.6750f, 0.1330f, 0.0580f, 0.0290f, MakeStatTabLabel(DTag(TEXT("RunSummary.Right.StatTabs.StatsButton.Label")), NSLOCTEXT("T66.RunSummary", "StatsTab", "STATS"), 22, 0));
		AddN(0.7521f, 0.1204f, 0.0875f, 0.0537f, MakeStatTabButton(DTag(TEXT("RunSummary.Right.StatTabs.DamageDealtButton")), 1));
		AddN(0.7540f, 0.1340f, 0.0840f, 0.0280f, MakeStatTabLabel(DTag(TEXT("RunSummary.Right.StatTabs.DamageDealtButton.Label")), NSLOCTEXT("T66.RunSummary", "DamageDealtTab", "DAMAGE DEALT"), 15, 1));
		AddN(0.8458f, 0.1204f, 0.0875f, 0.0537f, MakeStatTabButton(DTag(TEXT("RunSummary.Right.StatTabs.DamageReceivedButton")), 2));
		AddN(0.8460f, 0.1340f, 0.0880f, 0.0280f, MakeStatTabLabel(DTag(TEXT("RunSummary.Right.StatTabs.DamageReceivedButton.Label")), NSLOCTEXT("T66.RunSummary", "DamageReceivedTab", "DAMAGE RECEIVED"), 13, 2));

		AddN(0.6615f, 0.1870f, 0.3125f, 0.4926f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Right.StatsPanel"))));
		const FText StatFmt = NSLOCTEXT("T66.RunSummary", "FlatStatLineFormat", "{0}: {1}");
		AddRightStatLineTab(0, 0.210f, TEXT("RunSummary.Right.StatsPanel.DamageRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "Damage", "Damage"), FText::AsNumber(FlatDamageStat)));
		AddRightStatLineTab(0, 0.258f, TEXT("RunSummary.Right.StatsPanel.AttackSpeedRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed"), FText::AsNumber(FlatAttackSpeedStat)));
		AddRightStatLineTab(0, 0.306f, TEXT("RunSummary.Right.StatsPanel.AttackScaleRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "AttackScale", "Attack Scale"), FText::AsNumber(FlatAttackScaleStat)));
		AddRightStatLineTab(0, 0.354f, TEXT("RunSummary.Right.StatsPanel.AccuracyRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "Accuracy", "Accuracy"), FText::AsNumber(FlatAccuracyStat)));
		AddRightStatLineTab(0, 0.402f, TEXT("RunSummary.Right.StatsPanel.ArmorRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "Armor", "Armor"), FText::AsNumber(FlatArmorStat)));
		AddRightStatLineTab(0, 0.450f, TEXT("RunSummary.Right.StatsPanel.EvasionRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "Evasion", "Evasion"), FText::AsNumber(FlatEvasionStat)));
		AddRightStatLineTab(0, 0.498f, TEXT("RunSummary.Right.StatsPanel.LuckRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "Luck", "Luck"), FText::AsNumber(FlatLuckStat)));
		AddRightStatLineTab(0, 0.546f, TEXT("RunSummary.Right.StatsPanel.SpeedRow"), FText::Format(StatFmt, NSLOCTEXT("T66.Stats", "Speed", "Speed"), FText::AsNumber(FlatSpeedStat)));

		auto AddDamageRowsForTab = [&](
			const int32 TabIndex,
			const FString& DamageTagPrefix,
			const TArray<FDamageLogEntry>& FlatDamageRows,
			const FText& DamagePlaceholder)
		{
			AddNTab(TabIndex, 0.690f, 0.216f, 0.170f, 0.026f, MakeLabel(FName(*(DamageTagPrefix + TEXT(".SourceHeader"))), NSLOCTEXT("T66.RunSummary", "DamageTableSourceFlat", "SOURCE"), 16, Purple, true));
			AddNTab(TabIndex, 0.882f, 0.216f, 0.070f, 0.026f, MakeLabel(FName(*(DamageTagPrefix + TEXT(".ValueHeader"))), NSLOCTEXT("T66.RunSummary", "DamageTableDamageFlat", "DAMAGE"), 16, Purple, true, ETextJustify::Right));
			AddDividerTab(TabIndex, 0.684f, 0.248f, 0.286f);

			if (FlatDamageRows.Num() <= 0)
			{
				AddNTab(TabIndex, 0.700f, 0.390f, 0.220f, 0.040f, MakeLabel(FName(*(DamageTagPrefix + TEXT(".Placeholder"))), DamagePlaceholder, 24, White, false, ETextJustify::Center));
			}
			else
			{
				constexpr int32 MaxFlatDamageRows = 8;
				const int32 RowsToDisplay = FMath::Min(MaxFlatDamageRows, FlatDamageRows.Num());
				for (int32 RowIndex = 0; RowIndex < RowsToDisplay; ++RowIndex)
				{
					const FDamageLogEntry& Entry = FlatDamageRows[RowIndex];
					const float RowY = 0.262f + static_cast<float>(RowIndex) * 0.047f;
					const FString RowPrefix = FString::Printf(TEXT("%s.Row%02d"), *DamageTagPrefix, RowIndex + 1);
					AddNTab(TabIndex, 0.690f, RowY, 0.180f, 0.032f, MakeLabel(FName(*(RowPrefix + TEXT(".Source"))), T66ResolveRunSummaryDamageSourceName(GI, Entry.SourceID), 22, White, false));
					AddNTab(TabIndex, 0.882f, RowY, 0.070f, 0.032f, MakeLabel(FName(*(RowPrefix + TEXT(".Value"))), FText::AsNumber(Entry.TotalDamage), 22, White, false, ETextJustify::Right));
					AddDividerTab(TabIndex, 0.684f, RowY + 0.039f, 0.286f);
				}

				if (FlatDamageRows.Num() > RowsToDisplay)
				{
					AddNTab(
						TabIndex,
						0.690f,
						0.640f,
						0.260f,
						0.028f,
						MakeLabel(
							FName(*(DamageTagPrefix + TEXT(".MoreRows"))),
							FText::Format(NSLOCTEXT("T66.RunSummary", "DamageMoreRows", "+ {0} more"), FText::AsNumber(FlatDamageRows.Num() - RowsToDisplay)),
							18,
							Purple,
							true,
							ETextJustify::Center));
				}
			}
		};

		AddDamageRowsForTab(
			1,
			FString(TEXT("RunSummary.Right.StatsPanel.DamageDealt")),
			FlatDamageDealtRows,
			NSLOCTEXT("T66.RunSummary", "NoDamageDealt", "No damage dealt."));
		AddDamageRowsForTab(
			2,
			FString(TEXT("RunSummary.Right.StatsPanel.DamageReceived")),
			FlatDamageReceivedRows,
			NSLOCTEXT("T66.RunSummary", "NoDamageReceived", "No damage received."));

		if (bShowFlatProofActions)
		{
			// Saved-variant block rects from the approved saved v2 reference.
			AddN(0.6615f, 0.6907f, 0.3125f, 0.1231f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Right.ProofPanel"))));
			AddN(0.6760f, 0.7060f, 0.0260f, 0.0440f, MakeIcon(DTag(TEXT("RunSummary.Right.ProofPanel.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/link_chain.png"), FVector2D(46.f, 46.f), Purple, FText::FromString(TEXT("C"))));
			AddN(0.7040f, 0.7100f, 0.1400f, 0.0320f, MakeLabel(DTag(TEXT("RunSummary.Right.ProofPanel.Header")), NSLOCTEXT("T66.RunSummary", "ProofOfRunHeader", "PROOF OF RUN"), 27, White, true));
			AddN(0.6850f, 0.7300f, 0.2460f, 0.0590f, MakePanel(ET66FlatState::Default, DTag(TEXT("RunSummary.Right.ProofUrlField.Panel"))));
			AddN(0.6960f, 0.7470f, 0.2240f, 0.0300f, MakeLabel(DTag(TEXT("RunSummary.Right.ProofUrlField")), FText::FromString(FlatProofUrl), 19, White, false));
			AddN(0.9354f, 0.7300f, 0.0344f, 0.0590f, MakeButtonShell(ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleProofCopyClicked), DTag(TEXT("RunSummary.Right.CopyButton"))));
			AddN(0.9430f, 0.7400f, 0.0200f, 0.0390f, MakeIcon(DTag(TEXT("RunSummary.Right.CopyButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/copy_clipboard.png"), FVector2D(46.f, 46.f), Purple, FText::FromString(TEXT("C"))));
			AddN(0.6615f, 0.8241f, 0.3125f, 0.1148f, MakeButtonShell(ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleReportCheatingClicked), DTag(TEXT("RunSummary.Right.SubmitCheatingButton"))));
			AddN(0.6840f, 0.8550f, 0.0300f, 0.0500f, MakeIcon(DTag(TEXT("RunSummary.Right.SubmitCheatingButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/warning_triangle.png"), FVector2D(52.f, 52.f), Purple, FText::FromString(TEXT("!"))));
			AddN(0.7080f, 0.8650f, 0.2520f, 0.0380f, MakeLabel(DTag(TEXT("RunSummary.Right.SubmitCheatingButton.Label")), NSLOCTEXT("T66.RunSummary", "SubmitSuspicion", "SUBMIT SUSPICION OF CHEATING"), 21, White, true, ETextJustify::Center));
		}
		else
		{
			AddN(0.6625f, 0.7037f, 0.3094f, 0.1148f, MakeButtonShell(ET66FlatState::Selected, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked), DTag(TEXT("RunSummary.Right.GoAgainButton"))));
			AddN(0.7080f, 0.7380f, 0.0320f, 0.0440f, MakeIcon(DTag(TEXT("RunSummary.Right.GoAgainButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/refresh.png"), FVector2D(52.f, 52.f), Purple, FText::FromString(TEXT("R"))));
			AddN(0.7480f, 0.7430f, 0.1600f, 0.0400f, MakeLabel(DTag(TEXT("RunSummary.Right.GoAgainButton.Label")), NSLOCTEXT("T66.RunSummary", "GoAgain", "GO AGAIN!"), 32, White, true, ETextJustify::Center));
			AddN(0.6625f, 0.8352f, 0.3104f, 0.1056f, MakeButtonShell(ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleMainMenuClicked), DTag(TEXT("RunSummary.Right.MainMenuButton"))));
			AddN(0.7080f, 0.8660f, 0.0320f, 0.0440f, MakeIcon(DTag(TEXT("RunSummary.Right.MainMenuButton.Icon")), TEXT("RuntimeDependencies/T66/UI/Icons/Flat/home.png"), FVector2D(52.f, 52.f), Purple, FText::FromString(TEXT("H"))));
			AddN(0.7480f, 0.8710f, 0.1600f, 0.0400f, MakeLabel(DTag(TEXT("RunSummary.Right.MainMenuButton.Label")), NSLOCTEXT("T66.RunSummary", "MainMenu", "MAIN MENU"), 32, White, true, ETextJustify::Center));
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
			auto FormatCompactName = [](FName Value) -> FText
			{
				return Value.IsNone() ? NSLOCTEXT("T66.RunSummary", "NoneValue", "None") : FText::FromName(Value);
			};
			auto FormatAttackCategory = [](ET66AttackCategory Category) -> FText
			{
				switch (Category)
				{
				case ET66AttackCategory::AOE: return NSLOCTEXT("T66.RunSummary", "AttackCategoryAOE", "AOE");
				case ET66AttackCategory::Bounce: return NSLOCTEXT("T66.RunSummary", "AttackCategoryBounce", "Bounce");
				case ET66AttackCategory::DOT: return NSLOCTEXT("T66.RunSummary", "AttackCategoryDOT", "DOT");
				case ET66AttackCategory::SingleTarget: return NSLOCTEXT("T66.RunSummary", "AttackCategorySingleTarget", "Single Target");
				case ET66AttackCategory::Summon: return NSLOCTEXT("T66.RunSummary", "AttackCategorySummon", "Summon");
				default:
					return NSLOCTEXT("T66.RunSummary", "AttackCategoryAOEDefault", "AOE");
				}
			};
			auto AddStatValue = [&](ET66StatType StatType, float Value)
			{
				if (!T66IsLiveStatType(StatType))
				{
					return;
				}
				const FText Label = Loc ? Loc->GetText_StatName(StatType) : StaticEnum<ET66StatType>()->GetDisplayNameTextByValue(static_cast<int64>(StatType));
				AddStatLineText(Label, FormatFloat(Value, 1));
			};
			// Stats Rework: saved snapshot StatValues store accumulated bonus points — render with the
			// unified "+X%" language (same rounding as the shared stats panel).
			auto AddStatBonusValue = [&](ET66StatType StatType, float BonusPoints)
			{
				if (!T66IsLiveStatType(StatType))
				{
					return;
				}
				const FText Label = Loc ? Loc->GetText_StatName(StatType) : StaticEnum<ET66StatType>()->GetDisplayNameTextByValue(static_cast<int64>(StatType));
				AddStatLineText(Label, FText::Format(
					NSLOCTEXT("T66.RunSummary", "StatBonusPercent", "+{0}%"),
					FText::AsNumber(FMath::Max(0, FMath::RoundToInt(BonusPoints)))));
			};
			auto AddSavedEnrichedLines = [&]()
			{
				if (!LoadedSavedSummary)
				{
					return;
				}

				AddStatLine(NSLOCTEXT("T66.RunSummary", "NoIdolStacks", "No Idol Stacks"), LoadedSavedSummary->NoIdolSelectionStacks);
				AddStatBonusValue(ET66StatType::FirePower, LoadedSavedSummary->StatValues.FindRef(ET66StatType::FirePower));
				AddStatBonusValue(ET66StatType::IcePower, LoadedSavedSummary->StatValues.FindRef(ET66StatType::IcePower));
				AddStatBonusValue(ET66StatType::ElectricityPower, LoadedSavedSummary->StatValues.FindRef(ET66StatType::ElectricityPower));
				AddStatBonusValue(ET66StatType::NaturePower, LoadedSavedSummary->StatValues.FindRef(ET66StatType::NaturePower));
				AddStatBonusValue(ET66StatType::WindPower, LoadedSavedSummary->StatValues.FindRef(ET66StatType::WindPower));
				AddStatLineText(
					NSLOCTEXT("T66.RunSummary", "MobLoot", "Mob Loot"),
					FText::Format(
						NSLOCTEXT("T66.RunSummary", "MobLootSummaryValue", "{0} collected / {1} sold / {2} left"),
						FText::AsNumber(LoadedSavedSummary->MobLootQuantityCollectedThisRun),
						FText::AsNumber(LoadedSavedSummary->MobLootQuantitySoldThisRun),
						FText::AsNumber(LoadedSavedSummary->MobLootRemainingStack)));
				int32 SavedGamblerBetTotal = 0;
				int32 SavedGamblerPayoutTotal = 0;
				for (const FT66AntiCheatGamblerGameSummary& Summary : LoadedSavedSummary->GamblerOutcomeSummaries)
				{
					SavedGamblerBetTotal += Summary.TotalBetGold;
					SavedGamblerPayoutTotal += Summary.TotalPayoutGold;
				}
				AddStatLineText(
					NSLOCTEXT("T66.RunSummary", "GamblerResults", "Gambler"),
					FText::Format(
						NSLOCTEXT("T66.RunSummary", "GamblerSummaryValue", "{0} games / {1} bet / {2} paid"),
						FText::AsNumber(LoadedSavedSummary->GamblerOutcomeSummaries.Num()),
						FText::AsNumber(SavedGamblerBetTotal),
						FText::AsNumber(SavedGamblerPayoutTotal)));
				AddStatLineText(
					NSLOCTEXT("T66.RunSummary", "Weapon", "Weapon"),
					FText::Format(
						NSLOCTEXT("T66.RunSummary", "WeaponSummaryValue", "{0} / {1} / {2} shots / {3} deg"),
						FormatCompactName(LoadedSavedSummary->EquippedWeaponID),
						FormatAttackCategory(LoadedSavedSummary->EquippedWeaponBranch),
						FText::AsNumber(LoadedSavedSummary->EquippedWeaponProjectileCount),
						FormatFloat(LoadedSavedSummary->EquippedWeaponSpreadAngleDegrees, 1)));
				AddStatLineText(
					NSLOCTEXT("T66.RunSummary", "Pet", "Pet"),
					FText::Format(
						NSLOCTEXT("T66.RunSummary", "PetSummaryValue", "{0} / skin {1} / bond {2} / loot {3}"),
						FormatCompactName(LoadedSavedSummary->ActivePetID),
						FormatCompactName(LoadedSavedSummary->ActivePetSkinID),
						FText::AsNumber(LoadedSavedSummary->ActivePetBondStagesCleared),
						FText::AsNumber(LoadedSavedSummary->PetMobLootQuantityCollectedThisRun)));
				AddStatLineText(
					NSLOCTEXT("T66.RunSummary", "VendorEconomy", "Vendor"),
					FText::Format(
						NSLOCTEXT("T66.RunSummary", "VendorSummaryValue", "{0} gold / {1} debt / {2} inventory value"),
						FText::AsNumber(LoadedSavedSummary->CurrentGold),
						FText::AsNumber(LoadedSavedSummary->CurrentDebt),
						FText::AsNumber(LoadedSavedSummary->InventorySellValueTotal)));
				AddStatLineText(
					NSLOCTEXT("T66.RunSummary", "Boss", "Boss"),
					FText::Format(
						NSLOCTEXT("T66.RunSummary", "BossSummaryValue", "{0} / HP {1}/{2} / parts {3}"),
						FormatCompactName(LoadedSavedSummary->ActiveBossID),
						FText::AsNumber(LoadedSavedSummary->BossCurrentHP),
						FText::AsNumber(LoadedSavedSummary->BossMaxHP),
						FText::AsNumber(LoadedSavedSummary->BossParts.Num())));
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
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "HeadshotChance", "Headshot Chance"), FormatPercent(RunState->GetHeadshotChance01()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "LifeSteal", "Life Steal"), FormatPercent(RunState->GetLifeStealFraction()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "ReflectDamage", "Reflect Damage"), FormatPercent(RunState->GetReflectDamageFraction()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "CrushChance", "Crush Chance"), FormatPercent(RunState->GetCrushChance01()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "AssassinateChance", "Assassinate Chance"), FormatPercent(RunState->GetAssassinateChance01()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "LeapCooldown", "Leap Cooldown"), FormatMultiplier(RunState->GetDashCooldownMultiplier()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "AttackRange", "Attack Range"), FormatFloat(RunState->GetHeroBaseAttackRange(), 0));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "CloseRange", "Close Range"), FormatFloat(RunState->GetCloseRangeThreshold(), 0));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "LongRange", "Long Range"), FormatFloat(RunState->GetLongRangeThreshold(), 0));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "CloseRangeDamage", "Close Damage"), FormatMultiplier(RunState->GetCloseRangeDamageMultiplier()));
				AddStatLineText(NSLOCTEXT("T66.RunSummary", "LongRangeDamage", "Long Damage"), FormatMultiplier(RunState->GetLongRangeDamageMultiplier()));
				AddStatValue(ET66StatType::FirePower, RunState->GetStatValue(ET66StatType::FirePower));
				AddStatValue(ET66StatType::IcePower, RunState->GetStatValue(ET66StatType::IcePower));
				AddStatValue(ET66StatType::ElectricityPower, RunState->GetStatValue(ET66StatType::ElectricityPower));
				AddStatValue(ET66StatType::NaturePower, RunState->GetStatValue(ET66StatType::NaturePower));
				AddStatValue(ET66StatType::WindPower, RunState->GetStatValue(ET66StatType::WindPower));
				AddStatLine(NSLOCTEXT("T66.RunSummary", "NoIdolStacks", "No Idol Stacks"), RunState->GetNoIdolSelectionStacks());
				AddStatLineText(
					NSLOCTEXT("T66.RunSummary", "MobLoot", "Mob Loot"),
					FText::Format(
						NSLOCTEXT("T66.RunSummary", "MobLootSummaryValue", "{0} collected / {1} sold / {2} left"),
						FText::AsNumber(RunState->GetMobLootQuantityCollectedThisRun()),
						FText::AsNumber(RunState->GetMobLootQuantitySoldThisRun()),
						FText::AsNumber(RunState->GetCollectedMobLootStack())));
				{
					TArray<FT66AntiCheatGamblerGameSummary> LiveGamblerSummaries;
					RunState->GetAntiCheatGamblerSummaries(LiveGamblerSummaries);
					int32 LiveGamblerBetTotal = 0;
					int32 LiveGamblerPayoutTotal = 0;
					for (const FT66AntiCheatGamblerGameSummary& Summary : LiveGamblerSummaries)
					{
						LiveGamblerBetTotal += Summary.TotalBetGold;
						LiveGamblerPayoutTotal += Summary.TotalPayoutGold;
					}
					AddStatLineText(
						NSLOCTEXT("T66.RunSummary", "GamblerResults", "Gambler"),
						FText::Format(
							NSLOCTEXT("T66.RunSummary", "GamblerSummaryValue", "{0} games / {1} bet / {2} paid"),
							FText::AsNumber(LiveGamblerSummaries.Num()),
							FText::AsNumber(LiveGamblerBetTotal),
							FText::AsNumber(LiveGamblerPayoutTotal)));
				}
				if (const UT66WeaponManagerSubsystem* WeaponManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66WeaponManagerSubsystem>() : nullptr)
				{
					FWeaponData WeaponData;
					const bool bHasWeaponData = WeaponManager->GetEquippedWeaponData(WeaponData);
					AddStatLineText(
						NSLOCTEXT("T66.RunSummary", "Weapon", "Weapon"),
						FText::Format(
							NSLOCTEXT("T66.RunSummary", "WeaponSummaryValue", "{0} / {1} / {2} shots / {3} deg"),
							FormatCompactName(WeaponManager->GetEquippedWeaponID()),
							FormatAttackCategory(bHasWeaponData ? WeaponData.Branch : ET66AttackCategory::AOE),
							FText::AsNumber(bHasWeaponData ? WeaponData.ProjectileCount : 0),
							FormatFloat(bHasWeaponData ? WeaponData.SpreadAngleDegrees : 0.f, 1)));
				}
				if (const UT66AchievementsSubsystem* Achievements = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66AchievementsSubsystem>() : nullptr)
				{
					const FName ActivePetID = Achievements->GetActivePetID();
					AddStatLineText(
						NSLOCTEXT("T66.RunSummary", "Pet", "Pet"),
						FText::Format(
							NSLOCTEXT("T66.RunSummary", "PetSummaryValue", "{0} / skin {1} / bond {2} / loot {3}"),
							FormatCompactName(ActivePetID),
							FormatCompactName(!ActivePetID.IsNone() ? Achievements->GetEquippedPetSkinID(ActivePetID) : NAME_None),
							FText::AsNumber(!ActivePetID.IsNone() ? Achievements->GetPetBondStagesCleared(ActivePetID) : 0),
							FText::AsNumber(RunState->GetMobLootQuantityCollectedByPetThisRun())));
				}
				AddStatLineText(
					NSLOCTEXT("T66.RunSummary", "VendorEconomy", "Vendor"),
					FText::Format(
						NSLOCTEXT("T66.RunSummary", "VendorSummaryValue", "{0} gold / {1} debt / {2} inventory value"),
						FText::AsNumber(RunState->GetCurrentGold()),
						FText::AsNumber(RunState->GetCurrentDebt()),
						FText::AsNumber(RunState->GetInventorySellValueTotal())));
				AddStatLineText(
					NSLOCTEXT("T66.RunSummary", "Boss", "Boss"),
					FText::Format(
						NSLOCTEXT("T66.RunSummary", "BossSummaryValue", "{0} / HP {1}/{2} / parts {3}"),
						FormatCompactName(RunState->GetActiveBossID()),
						FText::AsNumber(RunState->GetBossCurrentHP()),
						FText::AsNumber(RunState->GetBossMaxHP()),
						FText::AsNumber(RunState->GetBossPartSnapshots().Num())));
			}
			else if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
			{
				TArray<TPair<ET66StatType, float>> SavedStats;
				for (const TPair<ET66StatType, float>& Pair : LoadedSavedSummary->StatValues)
				{
					SavedStats.Add(Pair);
				}
				SavedStats.Sort([](const TPair<ET66StatType, float>& A, const TPair<ET66StatType, float>& B)
				{
					return static_cast<uint8>(A.Key) < static_cast<uint8>(B.Key);
				});
				for (const TPair<ET66StatType, float>& Pair : SavedStats)
				{
					AddStatBonusValue(Pair.Key, Pair.Value);
				}
				AddSavedEnrichedLines();
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
	TArray<FName> InventoryLocal;
	const TArray<FT66InventorySlot>* InvSlotsPtr = nullptr;
	TArray<FT66InventorySlot> InventorySlotsLocal;
	TArray<ET66StatType> TemporaryBuffSlots;
	if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
	{
		IdolsPtr = &LoadedSavedSummary->EquippedIdols;
		if (LoadedSavedSummary->InventorySlots.Num() > 0)
		{
			InventorySlotsLocal = LoadedSavedSummary->InventorySlots;
			T66SortInventorySlotsByRarityForSummary(InventorySlotsLocal);
			InvSlotsPtr = &InventorySlotsLocal;
			T66CopyInventorySlotIDsForSummary(InventorySlotsLocal, InventoryLocal);
		}
		else
		{
			InventoryLocal = LoadedSavedSummary->Inventory;
		}
		TemporaryBuffSlots = LoadedSavedSummary->TemporaryBuffSlots;
	}
	else if (RunState)
	{
		if (UT66IdolManagerSubsystem* IdolManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr)
		{
			IdolsPtr = &IdolManager->GetEquippedIdols();
		}
		else
		{
			IdolsPtr = &RunState->GetEquippedIdols();
		}
		RunState->GetInventorySlotsSortedByRarity(InventorySlotsLocal);
		InvSlotsPtr = &InventorySlotsLocal;
		T66CopyInventorySlotIDsForSummary(InventorySlotsLocal, InventoryLocal);
		if (UT66BuffSubsystem* Buffs = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BuffSubsystem>() : nullptr)
		{
			TemporaryBuffSlots = Buffs->GetSelectedSingleUseBuffSlots();
		}
	}
	const TArray<FName> Empty;
	const TArray<FName>& Idols = IdolsPtr ? *IdolsPtr : Empty;

	static constexpr float IdolSlotPad = 10.f;
	static constexpr float IdolSlotSize = 100.f;
	TSharedRef<SHorizontalBox> IdolSlotsRow = SNew(SHorizontalBox);
	for (int32 i = 0; i < UT66IdolManagerSubsystem::MaxEquippedIdolSlots; ++i)
	{
		const FName IdolID = Idols.IsValidIndex(i) ? Idols[i] : NAME_None;
		const FLinearColor IdolColor = !IdolID.IsNone()
			? UT66IdolManagerSubsystem::GetIdolColor(IdolID)
			: FLinearColor(0.45f, 0.55f, 0.50f, 0.5f);
		FIdolData IdolData;
		const bool bHasIdolData = GI && !IdolID.IsNone() && GI->GetIdolData(IdolID, IdolData);
		TSharedPtr<FSlateBrush> IdolBrush;
		const TSoftObjectPtr<UTexture2D> IdolIconSoft = bHasIdolData
			? IdolData.Icon
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
		const ET66StatType SlotStat = TemporaryBuffSlots.IsValidIndex(SlotIndex)
			? TemporaryBuffSlots[SlotIndex]
			: ET66StatType::None;
		const bool bHasTemporaryBuff = T66IsLiveStatType(SlotStat);
		bHasAnyTemporaryBuff |= bHasTemporaryBuff;
		TSharedPtr<FSlateBrush> TempBuffBrush = bHasTemporaryBuff
			? T66TemporaryBuffUI::CreateSecondaryBuffBrush(TexPool, this, SlotStat, FVector2D(TempBuffSlotSize - 6.f, TempBuffSlotSize - 6.f))
			: nullptr;
		TemporaryBuffIconBrushes[SlotIndex] = TempBuffBrush;
		const FText SlotTooltip = bHasTemporaryBuff
			? (Loc ? Loc->GetText_StatName(SlotStat) : FText::FromString(TEXT("Temporary Buff")))
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
	const int32 NewTabIndex = FMath::Clamp(TabIndex, 0, 2);
	if (FlatRunSummaryStatTabIndex == NewTabIndex)
	{
		return FReply::Handled();
	}

	FlatRunSummaryStatTabIndex = NewTabIndex;
	InvalidateLayoutAndVolatility();
	if (TSharedPtr<SWidget> CachedWidget = GetCachedWidget())
	{
		CachedWidget->Invalidate(EInvalidateWidgetReason::Layout);
	}
	if (UIManager)
	{
		UIManager->RequestFrontendRootPaintRefresh();
	}
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
		UT66GameInstance::TransitionToFrontendLevel(this);
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

		UT66GameInstance::TransitionToFrontendLevel(this);
		return;
	}

	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState) RunState->BeginNewRun();
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
		UT66GameInstance::TransitionToFrontendLevel(this);
		return;
	}
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState) RunState->BeginNewRun();
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

	UT66GameInstance::TransitionToFrontendLevel(this);
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
