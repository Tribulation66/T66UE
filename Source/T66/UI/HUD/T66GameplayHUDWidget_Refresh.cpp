// Copyright Tribulation 66. All Rights Reserved.

#include "UI/HUD/T66GameplayHUDWidget_Private.h"

void UT66GameplayHUDWidget::RefreshHeroStats()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	if (StatDamageText.IsValid())
		StatDamageText->SetText(MakeGradeStatText(TEXT("Dmg"), RunState->GetDamageStat()));
	if (StatAttackSpeedText.IsValid())
		StatAttackSpeedText->SetText(MakeGradeStatText(TEXT("AS"), RunState->GetAttackSpeedStat()));
	if (StatAttackScaleText.IsValid())
		StatAttackScaleText->SetText(MakeGradeStatText(TEXT("Scale"), RunState->GetScaleStat()));
	if (StatArmorText.IsValid())
		StatArmorText->SetText(MakeGradeStatText(TEXT("Armor"), RunState->GetArmorStat()));
	if (StatEvasionText.IsValid())
		StatEvasionText->SetText(MakeGradeStatText(TEXT("Eva"), RunState->GetEvasionStat()));
	if (StatLuckText.IsValid())
		StatLuckText->SetText(MakeGradeStatText(TEXT("Luck"), RunState->GetLuckStat()));
}


void UT66GameplayHUDWidget::RefreshDPS()
{
	if (!DPSText.IsValid()) return;

	UT66DamageLogSubsystem* DamageLog = GetDamageLog();
	const int32 DisplayDPS = DamageLog ? FMath::RoundToInt(FMath::Max(0.f, DamageLog->GetRollingDPS())) : 0;
	const FLinearColor DPSColor = GetDPSColor(DisplayDPS);
	if (DisplayDPS != LastDisplayedDPS || !LastDisplayedDPSColor.Equals(DPSColor))
	{
		LastDisplayedDPS = DisplayDPS;
		LastDisplayedDPSColor = DPSColor;
		DPSText->SetText(FText::Format(
			NSLOCTEXT("T66.GameplayHUD", "DPSFormat", "DPS {0}"),
			FText::AsNumber(DisplayDPS)));
		DPSText->SetColorAndOpacity(FSlateColor(DPSColor));
	}
}


void UT66GameplayHUDWidget::HandleBackendLeaderboardDataReady(const FString& Key)
{
	static_cast<void>(Key);
	MarkHUDDirty();
}


void UT66GameplayHUDWidget::HandleBackendRunSummaryReady(const FString& EntryId)
{
	static_cast<void>(EntryId);
	MarkHUDDirty();
}


void UT66GameplayHUDWidget::RefreshFPS()
{
	if (!FPSText.IsValid() && !ElevationText.IsValid()) return;
	UWorld* World = GetWorld();
	const float Delta = World ? World->GetDeltaSeconds() : 0.f;
	const int32 FPS = (Delta > 0.f) ? FMath::RoundToInt(1.f / Delta) : 0;
	if (FPSText.IsValid())
	{
		FPSText->SetText(FText::FromString(FString::Printf(TEXT("FPS: %d"), FPS)));
	}

	if (ElevationText.IsValid())
	{
		const APawn* Pawn = GetOwningPlayerPawn();
		if (!Pawn)
		{
			if (const APlayerController* PC = GetOwningPlayer())
			{
				Pawn = PC->GetPawn();
			}
		}

		float GroundElevation = Pawn ? Pawn->GetActorLocation().Z : 0.f;
		if (const AT66HeroBase* Hero = Cast<AT66HeroBase>(Pawn))
		{
			if (const UCapsuleComponent* Capsule = Hero->GetCapsuleComponent())
			{
				GroundElevation -= Capsule->GetScaledCapsuleHalfHeight();
			}
		}

		ElevationText->SetText(FText::FromString(FString::Printf(TEXT("ELV: %d"), FMath::RoundToInt(GroundElevation))));
	}
}


void UT66GameplayHUDWidget::RefreshEconomy()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	// Net Worth
	if (NetWorthText.IsValid())
	{
		const int32 NetWorth = RunState->GetNetWorth();
		NetWorthText->SetText(FText::AsNumber(NetWorth));

		const FLinearColor NetWorthColor = NetWorth > 0
			? FT66Style::Tokens::Success
			: (NetWorth < 0 ? FT66Style::Tokens::Danger : FT66Style::Tokens::Text);
		NetWorthText->SetColorAndOpacity(FSlateColor(NetWorthColor));
	}

	// Gold
	if (GoldText.IsValid())
	{
		GoldText->SetText(FText::AsNumber(RunState->GetCurrentGold()));
	}

	// Owe (Debt) in red
	if (DebtText.IsValid())
	{
		DebtText->SetText(FText::AsNumber(RunState->GetCurrentDebt()));
	}

	// Score
	if (ScoreText.IsValid())
	{
		ScoreText->SetText(FText::AsNumber(RunState->GetCurrentScore()));
	}
	if (ScoreMultiplierText.IsValid())
	{
		ScoreMultiplierText->SetVisibility(EVisibility::Collapsed);
	}
}


void UT66GameplayHUDWidget::RefreshTutorialHint()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	// Tutorial hint (above crosshair)
	if (TutorialHintBorder.IsValid() && TutorialHintLine1Text.IsValid() && TutorialHintLine2Text.IsValid())
	{
		const bool bShow = RunState->IsTutorialHintVisible() && (!RunState->GetTutorialHintLine1().IsEmpty() || !RunState->GetTutorialHintLine2().IsEmpty());
		TutorialHintBorder->SetVisibility(bShow ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
		if (bShow)
		{
			const FText L1 = RunState->GetTutorialHintLine1();
			const FText L2 = RunState->GetTutorialHintLine2();
			TutorialHintLine1Text->SetText(L1);
			TutorialHintLine2Text->SetText(L2);
			TutorialHintLine2Text->SetVisibility(L2.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible);
		}
	}
}


void UT66GameplayHUDWidget::RefreshTutorialSubtitle()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	if (TutorialSubtitleBorder.IsValid() && TutorialSubtitleSpeakerText.IsValid() && TutorialSubtitleBodyText.IsValid())
	{
		const bool bShow = RunState->IsTutorialSubtitleVisible() && !RunState->GetTutorialSubtitleText().IsEmpty();
		TutorialSubtitleBorder->SetVisibility(bShow ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
		if (bShow)
		{
			const FText Speaker = RunState->GetTutorialSubtitleSpeaker();
			TutorialSubtitleSpeakerText->SetText(Speaker);
			TutorialSubtitleSpeakerText->SetVisibility(Speaker.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible);
			TutorialSubtitleBodyText->SetText(RunState->GetTutorialSubtitleText());
		}
	}
}


void UT66GameplayHUDWidget::RefreshStageAndTimer()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	const UT66PlayerExperienceSubSystem* PlayerExperience = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;

	bool bTowerBloodActive = false;
	if (const UWorld* World = GetWorld())
	{
		if (const AT66GameMode* GameMode = Cast<AT66GameMode>(World->GetAuthGameMode()))
		{
			static constexpr float TowerBloodDelaySeconds = 120.0f;
			const float Elapsed = UT66RunStateSubsystem::StageTimerDurationSeconds - RunState->GetStageTimerSecondsRemaining();
			bTowerBloodActive = GameMode->IsUsingTowerMainMapLayout()
				&& RunState->GetStageTimerActive()
				&& Elapsed >= TowerBloodDelaySeconds;
		}
	}

	if (DifficultyAreaNameText.IsValid())
	{
		DifficultyAreaNameText->SetText(BuildDifficultyAreaNameText(Difficulty));
		DifficultyAreaNameText->SetColorAndOpacity(bTowerBloodActive
			? FSlateColor(FLinearColor(0.95f, 0.18f, 0.20f, 1.0f))
			: FSlateColor(FT66Style::Tokens::Accent2));
	}

	// Stage number
	if (StageText.IsValid())
	{
		StageText->SetText(BuildDisplayedStageText(
			Loc,
			PlayerExperience,
			Difficulty,
			RunState->GetCurrentStage(),
			RunState->IsInStageCatchUp()));

		StageText->SetColorAndOpacity(bTowerBloodActive ? FSlateColor(FLinearColor(0.95f, 0.18f, 0.20f, 1.0f)) : FSlateColor(FT66Style::Tokens::Text));
	}

	// (Central countdown timer removed)
}


void UT66GameplayHUDWidget::RefreshBeatTargets()
{
	if (T66_IsHudReviewStateCommandLine())
	{
		if (ScoreTargetText.IsValid())
		{
			ScoreTargetText->SetText(NSLOCTEXT("T66.GameplayHUD", "HudReviewScoreTarget", "Score to Beat 1,590"));
			ScoreTargetText->SetVisibility(EVisibility::Visible);
		}
		if (ScorePacingText.IsValid())
		{
			ScorePacingText->SetText(NSLOCTEXT("T66.GameplayHUD", "HudReviewScorePacing", "Score Pace 900"));
			ScorePacingText->SetVisibility(EVisibility::Visible);
		}
		if (SpeedRunTargetText.IsValid())
		{
			SpeedRunTargetText->SetText(NSLOCTEXT("T66.GameplayHUD", "HudReviewSpeedTarget", "Time to Beat 0:38.08"));
			SpeedRunTargetText->SetVisibility(EVisibility::Visible);
		}
		if (SpeedRunPacingText.IsValid())
		{
			SpeedRunPacingText->SetText(NSLOCTEXT("T66.GameplayHUD", "HudReviewSpeedPacing", "Time Pace 0:21.40"));
			SpeedRunPacingText->SetVisibility(EVisibility::Visible);
		}
		return;
	}

	UT66RunStateSubsystem* RunState = GetRunState();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	UT66PlayerSettingsSubsystem* PS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66BackendSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!RunState || !T66GI || !PS || !LB)
	{
		return;
	}

	auto ResolveTarget = [PS, LB, Backend, T66GI](const ET66LeaderboardType Type, const FT66BeatTargetSelection& Selection, FT66ResolvedBeatTarget& OutTarget)
	{
		OutTarget = FT66ResolvedBeatTarget{};

		switch (Selection.Mode)
		{
		case ET66BeatTargetSelectionMode::PersonalBest:
			if (Type == ET66LeaderboardType::Score)
			{
				FT66LocalScoreRecord Record;
				if (LB->GetLocalBestScoreRecord(T66GI->SelectedDifficulty, T66GI->SelectedPartySize, Record) && Record.BestScore > 0)
				{
					OutTarget.bValid = true;
					OutTarget.Score = Record.BestScore;
					OutTarget.bSupportsPacing = false;
				}
			}
			else
			{
				FT66LocalCompletedRunTimeRecord Record;
				if (LB->GetLocalBestCompletedRunTimeRecord(T66GI->SelectedDifficulty, T66GI->SelectedPartySize, Record) && Record.BestCompletedSeconds > 0.01f)
				{
					OutTarget.bValid = true;
					OutTarget.TimeSeconds = Record.BestCompletedSeconds;
					OutTarget.bSupportsPacing = false;
				}
			}
			return;

		case ET66BeatTargetSelectionMode::FriendsTop:
		case ET66BeatTargetSelectionMode::StreamersTop:
		case ET66BeatTargetSelectionMode::GlobalTop:
		{
			const ET66LeaderboardFilter Filter =
				(Selection.Mode == ET66BeatTargetSelectionMode::FriendsTop) ? ET66LeaderboardFilter::Friends :
				(Selection.Mode == ET66BeatTargetSelectionMode::StreamersTop) ? ET66LeaderboardFilter::Streamers :
				ET66LeaderboardFilter::Global;

			EnsureHudBeatTargetLeaderboardRequested(Backend, Type, T66GI->SelectedPartySize, T66GI->SelectedDifficulty, Filter);
			if (!Backend)
			{
				return;
			}

			const FString CacheKey = MakeHudBeatTargetCacheKey(Type, T66GI->SelectedPartySize, T66GI->SelectedDifficulty, Filter);
			if (!Backend->HasCachedLeaderboard(CacheKey))
			{
				return;
			}

			const TArray<FLeaderboardEntry> Entries = Backend->GetCachedLeaderboard(CacheKey);
			if (Entries.Num() <= 0)
			{
				return;
			}

			const FLeaderboardEntry& Entry = Entries[0];
			OutTarget.bValid = true;
			OutTarget.Score = Entry.Score;
			OutTarget.TimeSeconds = Entry.TimeSeconds;
			OutTarget.EntryId = Entry.EntryId;
			OutTarget.bSupportsPacing = (Filter == ET66LeaderboardFilter::Global) && Entry.bHasRunSummary && !Entry.EntryId.IsEmpty();
			return;
		}

		case ET66BeatTargetSelectionMode::FavoriteRun:
		{
			FT66FavoriteLeaderboardRun Favorite;
			if (!PS->FindFavoriteLeaderboardRun(Selection.FavoriteEntryId, Favorite))
			{
				return;
			}
			if (Favorite.Difficulty != T66GI->SelectedDifficulty
				|| Favorite.PartySize != T66GI->SelectedPartySize)
			{
				return;
			}

			OutTarget.bValid = true;
			OutTarget.Score = Favorite.Score;
			OutTarget.TimeSeconds = Favorite.TimeSeconds;
			OutTarget.EntryId = Favorite.EntryId;
			OutTarget.bSupportsPacing = (Favorite.Filter == ET66LeaderboardFilter::Global) && Favorite.bHasRunSummary && !Favorite.EntryId.IsEmpty();
			return;
		}

		default:
			return;
		}
	};

	auto LoadPacingSummary = [Backend](const FT66ResolvedBeatTarget& Target) -> UT66LeaderboardRunSummarySaveGame*
	{
		if (!Target.bSupportsPacing)
		{
			return nullptr;
		}

		if (!Target.LocalRunSummarySlotName.IsEmpty())
		{
			if (UGameplayStatics::DoesSaveGameExist(Target.LocalRunSummarySlotName, 0))
			{
				return Cast<UT66LeaderboardRunSummarySaveGame>(UGameplayStatics::LoadGameFromSlot(Target.LocalRunSummarySlotName, 0));
			}
			return nullptr;
		}

		if (!Backend || Target.EntryId.IsEmpty())
		{
			return nullptr;
		}

		if (Backend->HasCachedRunSummary(Target.EntryId))
		{
			return Backend->GetCachedRunSummary(Target.EntryId);
		}

		Backend->FetchRunSummary(Target.EntryId);
		return nullptr;
	};

	FT66ResolvedBeatTarget ScoreTarget;
	ResolveTarget(ET66LeaderboardType::Score, PS->GetScoreToBeatSelection(), ScoreTarget);

	if (ScoreTargetText.IsValid())
	{
		if (PS->GetShowScoreToBeat() && ScoreTarget.bValid)
		{
			ScoreTargetText->SetText(FText::Format(
				NSLOCTEXT("T66.GameplayHUD", "ScoreTargetFormat", "Score to Beat {0}"),
				FText::AsNumber(ScoreTarget.Score)));
			ScoreTargetText->SetVisibility(EVisibility::Visible);
		}
		else
		{
			ScoreTargetText->SetVisibility(EVisibility::Collapsed);
		}
	}

	if (ScorePacingText.IsValid())
	{
		UT66LeaderboardRunSummarySaveGame* PacingSummary = PS->GetShowScorePacing() ? LoadPacingSummary(ScoreTarget) : nullptr;
		FT66StagePacingPoint PacingPoint;
		if (PS->GetShowScorePacing()
			&& !RunState->IsInStageCatchUp()
			&& PacingSummary
			&& FindStagePacingPointForStage(PacingSummary, RunState->GetCurrentStage(), PacingPoint)
			&& PacingPoint.Score > 0)
		{
			ScorePacingText->SetText(FText::Format(
				NSLOCTEXT("T66.GameplayHUD", "ScorePacingFormat", "Score Pace {0}"),
				FText::AsNumber(PacingPoint.Score)));
			ScorePacingText->SetVisibility(EVisibility::Visible);
		}
		else
		{
			ScorePacingText->SetVisibility(EVisibility::Collapsed);
		}
	}

	FT66ResolvedBeatTarget TimeTarget;
	ResolveTarget(ET66LeaderboardType::SpeedRun, PS->GetTimeToBeatSelection(), TimeTarget);
	const bool bShowLiveRunTime = PS->GetSpeedRunMode();

	if (SpeedRunTargetText.IsValid())
	{
		if (bShowLiveRunTime && PS->GetShowTimeToBeat() && TimeTarget.bValid && !RunState->IsInStageCatchUp())
		{
			SpeedRunTargetText->SetText(FText::Format(
				NSLOCTEXT("T66.GameplayHUD", "SpeedRunTargetFormat", "Time to Beat {0}"),
				FormatHudTimerValue(TimeTarget.TimeSeconds)));
			SpeedRunTargetText->SetVisibility(EVisibility::Visible);
		}
		else
		{
			SpeedRunTargetText->SetVisibility(EVisibility::Collapsed);
		}
	}

	if (SpeedRunPacingText.IsValid())
	{
		UT66LeaderboardRunSummarySaveGame* PacingSummary = (bShowLiveRunTime && PS->GetShowTimePacing()) ? LoadPacingSummary(TimeTarget) : nullptr;
		FT66StagePacingPoint PacingPoint;
		if (bShowLiveRunTime
			&& PS->GetShowTimePacing()
			&& !RunState->IsInStageCatchUp()
			&& PacingSummary
			&& FindStagePacingPointForStage(PacingSummary, RunState->GetCurrentStage(), PacingPoint)
			&& PacingPoint.ElapsedSeconds > 0.f)
		{
			SpeedRunPacingText->SetText(FText::Format(
				NSLOCTEXT("T66.GameplayHUD", "TimePacingFormat", "Time Pace {0}"),
				FormatHudTimerValue(PacingPoint.ElapsedSeconds)));
			SpeedRunPacingText->SetVisibility(EVisibility::Visible);
		}
		else
		{
			SpeedRunPacingText->SetVisibility(EVisibility::Collapsed);
		}
	}
}


void UT66GameplayHUDWidget::RefreshSpeedRunTimers()
{
	if (T66_IsHudReviewStateCommandLine())
	{
		if (SpeedRunText.IsValid())
		{
			SpeedRunText->SetVisibility(EVisibility::Visible);
			SpeedRunText->SetText(NSLOCTEXT("T66.GameplayHUD", "HudReviewSpeedRunTimer", "Time 0:04.50"));
		}
		LastDisplayedSpeedRunTotalCs = 450;
		return;
	}

	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	UT66PlayerSettingsSubsystem* PS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;

	// Time (speedrun timer): counts up after leaving the start area (visibility toggled by player setting)
	if (SpeedRunText.IsValid())
	{
		const bool bShow = PS ? PS->GetSpeedRunMode() : false;
		const EVisibility DesiredVisibility = bShow ? EVisibility::Visible : EVisibility::Collapsed;
		if (SpeedRunText->GetVisibility() != DesiredVisibility)
		{
			SpeedRunText->SetVisibility(DesiredVisibility);
		}
		if (bShow)
		{
			const float Secs = FMath::Max(0.f, RunState->GetCurrentRunElapsedSeconds());
			// [GOLD] HUD text cache: only reformat when the displayed centisecond value changes (avoids FText::Format every frame).
			const int32 TotalCs = FMath::FloorToInt(Secs * 100.f);
			if (TotalCs != LastDisplayedSpeedRunTotalCs)
			{
				LastDisplayedSpeedRunTotalCs = TotalCs;
				SpeedRunText->SetText(FText::Format(
					NSLOCTEXT("T66.GameplayHUD", "SpeedRunTimerFormat", "Time {0}"),
					FormatHudTimerValue(Secs)));
			}
		}
		else
		{
			LastDisplayedSpeedRunTotalCs = -1;
		}
	}
}


bool UT66GameplayHUDWidget::DoesBossPartBarTopologyMatch(const TArray<FT66BossPartSnapshot>& BossParts) const
{
	if (BossPartBarRows.Num() != BossParts.Num())
	{
		return false;
	}

	for (int32 Index = 0; Index < BossParts.Num(); ++Index)
	{
		const FT66BossPartBarRow& ExistingRow = BossPartBarRows[Index];
		const FT66BossPartSnapshot& Part = BossParts[Index];
		if (ExistingRow.PartID != Part.PartID || ExistingRow.HitZoneType != Part.HitZoneType)
		{
			return false;
		}
	}

	return true;
}


void UT66GameplayHUDWidget::RebuildBossPartBars(
	const TArray<FT66BossPartSnapshot>& BossParts,
	const FLinearColor& BossBarBackgroundColor)
{
	if (!BossPartBarsBox.IsValid())
	{
		BossPartBarRows.Reset();
		return;
	}

	BossPartBarsBox->ClearChildren();
	BossPartBarRows.Reset();
	BossPartBarRows.Reserve(BossParts.Num());

	static constexpr float BossBarWidth = 560.f;

	for (const FT66BossPartSnapshot& Part : BossParts)
	{
		FT66BossPartBarRow& Row = BossPartBarRows.AddDefaulted_GetRef();
		Row.PartID = Part.PartID;
		Row.HitZoneType = Part.HitZoneType;

		BossPartBarsBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BossBarBackgroundColor)
				.Padding(0.f)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			[
				SAssignNew(Row.FillBox, SBox)
				.HeightOverride(18.f)
				.WidthOverride(BossBarWidth)
				[
					SAssignNew(Row.FillBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(GetBossPartFillColor(Part.HitZoneType, Part.IsAlive()))
					.Padding(0.f)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(Row.Text, STextBlock)
				.Font(FT66Style::Tokens::FontBold(11))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
		];
	}
}


void UT66GameplayHUDWidget::RefreshBossBar()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	static constexpr float BossBarWidth = 560.f;
	const FLinearColor BossBarBackgroundColor = FT66Style::IsDotaTheme()
		? FT66Style::BossBarBackground()
		: FLinearColor(0.08f, 0.08f, 0.08f, 0.9f);

	// Boss health bar: visible only when boss awakened
	const bool bBossActive = RunState->GetBossActive();
	if (BossBarContainerBox.IsValid())
	{
		const EVisibility DesiredVisibility = bBossActive ? EVisibility::Visible : EVisibility::Collapsed;
		if (BossBarContainerBox->GetVisibility() != DesiredVisibility)
		{
			BossBarContainerBox->SetVisibility(DesiredVisibility);
		}
	}

	if (bBossActive)
	{
		const int32 BossHP = RunState->GetBossCurrentHP();
		const int32 BossMax = FMath::Max(1, RunState->GetBossMaxHP());
		if (!bLastBossBarVisible || BossHP != LastDisplayedBossCurrentHP || BossMax != LastDisplayedBossMaxHP)
		{
			const float Pct = static_cast<float>(BossHP) / static_cast<float>(BossMax);
			if (BossBarFillBox.IsValid())
			{
				BossBarFillBox->SetWidthOverride(FMath::Clamp(BossBarWidth * Pct, 0.f, BossBarWidth));
			}
			if (BossBarText.IsValid())
			{
				BossBarText->SetText(FText::Format(
					NSLOCTEXT("T66.Common", "Fraction", "{0}/{1}"),
					FText::AsNumber(BossHP),
					FText::AsNumber(BossMax)));
			}

			LastDisplayedBossCurrentHP = BossHP;
			LastDisplayedBossMaxHP = BossMax;
		}
		bLastBossBarVisible = true;

		if (BossPartBarsBox.IsValid())
		{
			const TArray<FT66BossPartSnapshot>& BossParts = RunState->GetBossPartSnapshots();
			const bool bShowPartBars = BossParts.Num() > 1;
			const EVisibility DesiredPartVisibility = bShowPartBars ? EVisibility::Visible : EVisibility::Collapsed;
			if (BossPartBarsBox->GetVisibility() != DesiredPartVisibility)
			{
				BossPartBarsBox->SetVisibility(DesiredPartVisibility);
			}

			if (bShowPartBars)
			{
				if (!DoesBossPartBarTopologyMatch(BossParts))
				{
					// Rebuild only when the boss part layout changes; damage updates should reuse the existing rows.
					RebuildBossPartBars(BossParts, BossBarBackgroundColor);
				}

				for (int32 Index = 0; Index < BossParts.Num() && Index < BossPartBarRows.Num(); ++Index)
				{
					const FT66BossPartSnapshot& Part = BossParts[Index];
					FT66BossPartBarRow& Row = BossPartBarRows[Index];
					const int32 PartMaxHP = FMath::Max(1, Part.MaxHP);
					const int32 PartCurrentHP = FMath::Clamp(Part.CurrentHP, 0, PartMaxHP);
					const float PartPct = static_cast<float>(PartCurrentHP) / static_cast<float>(PartMaxHP);
					const bool bPartAlive = Part.IsAlive();

					if (Row.FillBox.IsValid() && (Row.LastCurrentHP != PartCurrentHP || Row.LastMaxHP != PartMaxHP))
					{
						Row.FillBox->SetWidthOverride(FMath::Clamp(BossBarWidth * PartPct, 0.f, BossBarWidth));
					}

					if (Row.FillBorder.IsValid() && (Row.bLastAlive != bPartAlive || Row.LastCurrentHP == INDEX_NONE))
					{
						Row.FillBorder->SetBorderBackgroundColor(GetBossPartFillColor(Part.HitZoneType, bPartAlive));
					}

					if (Row.Text.IsValid() && (Row.LastCurrentHP != PartCurrentHP || Row.LastMaxHP != PartMaxHP))
					{
						Row.Text->SetText(FText::Format(
							NSLOCTEXT("T66.GameplayHUD", "BossPartFraction", "{0} {1}/{2}"),
							GetBossPartDisplayName(Part),
							FText::AsNumber(PartCurrentHP),
							FText::AsNumber(PartMaxHP)));
					}

					Row.LastCurrentHP = PartCurrentHP;
					Row.LastMaxHP = PartMaxHP;
					Row.bLastAlive = bPartAlive;
				}
			}
		}
	}
	else
	{
		if (BossPartBarsBox.IsValid())
		{
			BossPartBarsBox->SetVisibility(EVisibility::Collapsed);
		}
		LastDisplayedBossCurrentHP = INDEX_NONE;
		LastDisplayedBossMaxHP = INDEX_NONE;
		bLastBossBarVisible = false;
	}
}


void UT66GameplayHUDWidget::RefreshLootPrompt()
{
	// No longer show accept/reject prompt; item is added immediately on interact, then item card popup is shown.
	if (LootPromptBox.IsValid())
	{
		LootPromptBox->SetVisibility(EVisibility::Collapsed);
	}
}


void UT66GameplayHUDWidget::RefreshHearts()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	// Hearts: split each gameplay slot into two visual segments so the HUD shows ten hearts total.
	for (int32 i = 0; i < HeartImages.Num(); ++i)
	{
		if (!HeartImages[i].IsValid()) continue;
		const int32 SlotIndex = i / 2;
		const float SlotFill = RunState->GetHeartSlotFill(SlotIndex);
		const float SegmentStart = (i % 2 == 0) ? 0.f : 0.5f;
		const float SegmentFill = FMath::Clamp((SlotFill - SegmentStart) / 0.5f, 0.f, 1.f);
		const int32 HeartTier = RunState->GetHeartSlotTier(SlotIndex);
		HeartImages[i]->SetImage(ResolveHeartBrushForDisplay(
			HeartTierBrushes,
			HeartBlessingBrush,
			HeartBrush,
			RunState->IsSaintBlessingActive(),
			HeartTier));
		HeartImages[i]->SetColorAndOpacity(FLinearColor::White);
		if (HeartFillBoxes.IsValidIndex(i) && HeartFillBoxes[i].IsValid())
		{
			HeartFillBoxes[i]->SetWidthOverride(GT66DisplayedHeartWidth * SegmentFill);
			HeartFillBoxes[i]->SetVisibility(SegmentFill > 0.01f ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}
}


void UT66GameplayHUDWidget::RefreshQuickReviveState()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState)
	{
		return;
	}

	if (QuickReviveIconRowBox.IsValid())
	{
		QuickReviveIconRowBox->SetVisibility(RunState->HasQuickReviveCharge() ? EVisibility::Visible : EVisibility::Collapsed);
	}

	if (QuickReviveDownedOverlayBorder.IsValid())
	{
		const bool bDowned = RunState->IsInQuickReviveDownedState();
		QuickReviveDownedOverlayBorder->SetVisibility(bDowned ? EVisibility::Visible : EVisibility::Collapsed);
		if (QuickReviveDownedText.IsValid())
		{
			const int32 SecondsRemaining = FMath::Max(1, FMath::CeilToInt(RunState->GetQuickReviveDownedSecondsRemaining()));
			QuickReviveDownedText->SetText(FText::Format(
				NSLOCTEXT("T66.GameplayHUD", "QuickReviveDownedCountdown", "REVIVING IN {0}"),
				FText::AsNumber(SecondsRemaining)));
		}
	}
}


void UT66GameplayHUDWidget::RefreshStatusEffects()
{
}


void UT66GameplayHUDWidget::RefreshHUD()
{
	FLagScopedScope LagScope(GetWorld(), TEXT("GameplayHUD::RefreshHUD"));

	bHUDDirty = false;
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66GameInstance* GIAsT66 = Cast<UT66GameInstance>(GetGameInstance());
	const UT66GameInstance* T66GI = GIAsT66;
	const UT66PlayerExperienceSubSystem* PlayerExperience = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;

	RefreshEconomy();
	RefreshStageAndTimer();
	RefreshBeatTargets();
	RefreshBossBar();
	RefreshHeroStats();

	// Portrait frame stays neutral; heart tier is already conveyed by the heart row and other HUD accents.
	if (PortraitBorder.IsValid())
	{
		PortraitBorder->SetBorderBackgroundColor(RunState->IsSaintBlessingActive()
			? FLinearColor(0.92f, 0.92f, 0.98f, 1.f)
			: (FT66Style::IsDotaTheme()
				? FT66Style::PanelInner()
				: FLinearColor(0.12f, 0.12f, 0.14f, 1.f)));
	}
	const FName DesiredPortraitHeroID = GIAsT66 ? GIAsT66->SelectedHeroID : NAME_None;
	const ET66BodyType DesiredPortraitBodyType = GIAsT66 ? GIAsT66->SelectedHeroBodyType : ET66BodyType::TypeA;
	ET66HeroPortraitVariant DesiredPortraitVariant = RunState->IsSaintBlessingActive()
		? ET66HeroPortraitVariant::Invincible
		: ET66HeroPortraitVariant::Half;
	const int32 HeartsRemaining = RunState->GetCurrentHearts();
	if (!RunState->IsSaintBlessingActive() && HeartsRemaining <= 1)
	{
		DesiredPortraitVariant = ET66HeroPortraitVariant::Low;
	}
	else if (!RunState->IsSaintBlessingActive() && HeartsRemaining >= 5)
	{
		DesiredPortraitVariant = ET66HeroPortraitVariant::Full;
	}

	const bool bPortraitStateChanged = !bPortraitStateInitialized
		|| LastPortraitHeroID != DesiredPortraitHeroID
		|| LastPortraitBodyType != DesiredPortraitBodyType
		|| LastPortraitVariant != DesiredPortraitVariant;

	if (bPortraitStateChanged)
	{
		bPortraitStateInitialized = true;
		LastPortraitHeroID = DesiredPortraitHeroID;
		LastPortraitBodyType = DesiredPortraitBodyType;
		LastPortraitVariant = DesiredPortraitVariant;

		TSoftObjectPtr<UTexture2D> PortraitSoft;
		if (GIAsT66 && !DesiredPortraitHeroID.IsNone())
		{
			FHeroData HeroData;
			if (GIAsT66->GetHeroData(DesiredPortraitHeroID, HeroData))
			{
				PortraitSoft = GIAsT66->ResolveHeroPortrait(HeroData, DesiredPortraitBodyType, DesiredPortraitVariant);
			}
		}

		bLastPortraitHasRef = !PortraitSoft.IsNull();
		if (PortraitBrush.IsValid())
		{
			UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
			if (PortraitSoft.IsNull() || !TexPool)
			{
				PortraitBrush->SetResourceObject(nullptr);
			}
			else
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, PortraitBrush, FName(TEXT("HUDPortrait")), /*bClearWhileLoading*/ true);
			}
		}
	}
	if (PortraitImage.IsValid())
	{
		const EVisibility DesiredVisibility = bLastPortraitHasRef ? EVisibility::Visible : EVisibility::Collapsed;
		if (PortraitImage->GetVisibility() != DesiredVisibility)
		{
			PortraitImage->SetVisibility(DesiredVisibility);
		}
	}
	if (PortraitPlaceholderText.IsValid())
	{
		const EVisibility DesiredVisibility = bLastPortraitHasRef ? EVisibility::Collapsed : EVisibility::Visible;
		if (PortraitPlaceholderText->GetVisibility() != DesiredVisibility)
		{
			PortraitPlaceholderText->SetVisibility(DesiredVisibility);
		}
	}

	FHeroData SelectedHeroData;
	const bool bHasSelectedHeroData = GIAsT66 && GIAsT66->GetSelectedHeroData(SelectedHeroData);
	const FName DesiredAbilityHeroID = bHasSelectedHeroData ? SelectedHeroData.HeroID : NAME_None;
	ET66UltimateType DesiredUltimateType = bHasSelectedHeroData ? SelectedHeroData.UltimateType : ET66UltimateType::None;
	ET66PassiveType DesiredPassiveType = RunState->GetPassiveType();
	FWeaponData DesiredWeaponData;
	bool bHasDesiredWeaponData = false;
	FName DesiredWeaponID = NAME_None;
	if (GIAsT66)
	{
		if (const UT66CommunityContentSubsystem* Community = GIAsT66->GetSubsystem<UT66CommunityContentSubsystem>())
		{
			const ET66UltimateType OverrideUltimateType = Community->GetActiveUltimateOverride();
			if (OverrideUltimateType != ET66UltimateType::None)
			{
				DesiredUltimateType = OverrideUltimateType;
			}

			const ET66PassiveType OverridePassiveType = Community->GetActivePassiveOverride();
			if (OverridePassiveType != ET66PassiveType::None)
			{
				DesiredPassiveType = OverridePassiveType;
			}
		}

		if (UT66WeaponManagerSubsystem* WeaponManager = GIAsT66->GetSubsystem<UT66WeaponManagerSubsystem>())
		{
			bHasDesiredWeaponData = WeaponManager->GetEquippedWeaponData(DesiredWeaponData);
			DesiredWeaponID = bHasDesiredWeaponData ? DesiredWeaponData.WeaponID : NAME_None;
		}
	}
	const bool bAbilityStateChanged = !bAbilityStateInitialized
		|| LastAbilityHeroID != DesiredAbilityHeroID
		|| LastWeaponID != DesiredWeaponID
		|| LastUltimateType != DesiredUltimateType
		|| LastPassiveType != DesiredPassiveType;

	if (bAbilityStateChanged)
	{
		bAbilityStateInitialized = true;
		LastAbilityHeroID = DesiredAbilityHeroID;
		LastWeaponID = DesiredWeaponID;
		LastUltimateType = DesiredUltimateType;
		LastPassiveType = DesiredPassiveType;

		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (UltimateBrush.IsValid())
		{
			const TSoftObjectPtr<UTexture2D> UltimateSoft = ResolveGameplayUltimateIcon(DesiredAbilityHeroID, DesiredUltimateType);
			if (TexPool && !UltimateSoft.IsNull())
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, UltimateSoft, this, UltimateBrush, FName(TEXT("HUDUltimate")), false);
			}
			else
			{
				UltimateBrush->SetResourceObject(nullptr);
			}
		}

		if (PassiveBrush.IsValid())
		{
			const TSoftObjectPtr<UTexture2D> WeaponSoft = bHasDesiredWeaponData ? DesiredWeaponData.Icon : TSoftObjectPtr<UTexture2D>();
			if (TexPool && !WeaponSoft.IsNull())
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, WeaponSoft, this, PassiveBrush, FName(TEXT("HUDWeapon")), false);
			}
			else
			{
				PassiveBrush->SetResourceObject(nullptr);
			}
		}
		if (PassiveBorder.IsValid())
		{
			if (bHasDesiredWeaponData)
			{
				PassiveBorder->SetToolTip(CreateRichTooltip(DesiredWeaponData.DisplayName, DesiredWeaponData.Description));
			}
			else
			{
				PassiveBorder->SetToolTip(nullptr);
			}
		}
	}

	if (UltimateInputHintText.IsValid())
	{
		UltimateInputHintText->SetText(ResolveGameplayUltimateInputHint(DesiredUltimateType));
	}

	// Hero level + XP progress ring
	if (LevelRingWidget.IsValid())
	{
		LevelRingWidget->SetPercent(RunState->GetHeroXP01());
	}
	if (LevelText.IsValid())
	{
		LevelText->SetText(FText::Format(
			NSLOCTEXT("T66.HUD", "LevelOutOf99", "{0}/99"),
			FText::AsNumber(FMath::Clamp(RunState->GetHeroLevel(), 0, UT66RunStateSubsystem::MaxHeroLevel))));
	}

	// Ultimate (R) � show cooldown overlay with countdown when on cooldown, hide when ready
	{
		const bool bReady = RunState->IsUltimateReady();
		if (UltimateCooldownOverlay.IsValid())
		{
			UltimateCooldownOverlay->SetVisibility(bReady ? EVisibility::Collapsed : EVisibility::Visible);
		}
		if (UltimateText.IsValid() && !bReady)
		{
			const int32 Sec = FMath::CeilToInt(RunState->GetUltimateCooldownRemainingSeconds());
			UltimateText->SetText(FText::AsNumber(FMath::Max(0, Sec)));
		}
		if (UltimateBorder.IsValid())
		{
			// Subtle glow tint when ready, neutral border otherwise
			UltimateBorder->SetBorderBackgroundColor(bReady ? FLinearColor(0.08f, 0.08f, 0.10f, 1.f) : FLinearColor(0.08f, 0.08f, 0.10f, 1.f));
		}
	}

	// The lower ability slot is now the equipped weapon; legacy passive stack UI stays hidden.
	{
		if (PassiveStackBadgeBox.IsValid())
		{
			PassiveStackBadgeBox->SetVisibility(EVisibility::Collapsed);
		}
		if (PassiveStackText.IsValid())
		{
			PassiveStackText->SetText(FText::AsNumber(0));
		}
	}

	// Difficulty (Skulls): 5-slot compression with tier colors (no half-skulls).
	{
		const int32 Skulls = FMath::Max(0, RunState->GetDifficultySkulls());
		const int32 SkullColorBandSize = PlayerExperience
			? PlayerExperience->GetDifficultySkullColorBandSize(T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy)
			: 4;

		// Color tier changes every 4 skulls, but filling within a tier is 1..4.
		// Skull 1-4 => Tier 0, Within 1..4; Skull 5 => Tier 1, Within 1, etc.
		int32 Tier = 0;
		int32 Within = 0;
		if (Skulls > 0)
		{
			Tier = (Skulls - 1) / FMath::Max(1, SkullColorBandSize);
			Within = ((Skulls - 1) % FMath::Max(1, SkullColorBandSize)) + 1;
		}
		const FLinearColor TierC = FT66RarityUtil::GetTierColor(Tier);
		for (int32 i = 0; i < DifficultyImages.Num(); ++i)
		{
			if (!DifficultyImages[i].IsValid()) continue;
			const bool bFilled = (i < Within);
			// Show skull only if slot should be filled; hide empty slots so skulls appear one-by-one.
			DifficultyImages[i]->SetVisibility(bFilled ? EVisibility::Visible : EVisibility::Collapsed);
			if (bFilled)
			{
				// Tier 0 = white skulls; higher tiers get the tier color.
				DifficultyImages[i]->SetColorAndOpacity(Tier == 0 ? FLinearColor::White : TierC);
			}
		}
		if (DifficultyRowBox.IsValid())
		{
			DifficultyRowBox->SetVisibility(Skulls > 0 ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}

	// Cowardice (clowns): show N clowns for gates taken this difficulty segment.
	{
		const int32 Clowns = FMath::Max(0, RunState->GetCowardiceGatesTaken());
		for (int32 i = 0; i < ClownImages.Num(); ++i)
		{
			if (!ClownImages[i].IsValid()) continue;
			const bool bFilled = (i < Clowns);
			ClownImages[i]->SetVisibility(bFilled ? EVisibility::Visible : EVisibility::Collapsed);
			if (bFilled)
			{
				ClownImages[i]->SetColorAndOpacity(FLinearColor::White);
			}
		}
		if (CowardiceRowBox.IsValid())
		{
			CowardiceRowBox->SetVisibility(Clowns > 0 ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}

	// Score multiplier color: theme for initial/tier 0, tier color for higher tiers
	if (ScoreMultiplierText.IsValid())
	{
		ScoreMultiplierText->SetColorAndOpacity(FT66Style::Tokens::Text);
	}

	// Dev toggles (immortality / power)
	if (ImmortalityButtonText.IsValid())
	{
		const bool bOn = RunState->GetDevImmortalityEnabled();
		ImmortalityButtonText->SetText(bOn
			? NSLOCTEXT("T66.Dev", "ImmortalityOn", "IMMORTALITY: ON")
			: NSLOCTEXT("T66.Dev", "ImmortalityOff", "IMMORTALITY: OFF"));
		ImmortalityButtonText->SetColorAndOpacity(bOn ? FLinearColor(0.20f, 0.85f, 0.35f, 1.f) : FT66Style::Tokens::Text);
	}
	if (PowerButtonText.IsValid())
	{
		const bool bOn = RunState->GetDevPowerEnabled();
		PowerButtonText->SetText(bOn
			? NSLOCTEXT("T66.Dev", "PowerOn", "POWER: ON")
			: NSLOCTEXT("T66.Dev", "PowerOff", "POWER: OFF"));
		PowerButtonText->SetColorAndOpacity(bOn ? FLinearColor(0.95f, 0.80f, 0.20f, 1.f) : FT66Style::Tokens::Text);
	}

	// Idol slots: rarity-colored when equipped, dark teal when empty.
	UT66IdolManagerSubsystem* IdolManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr;
	const TArray<FName>& EquippedIdols = IdolManager ? IdolManager->GetEquippedIdols() : RunState->GetEquippedIdols();
	const TArray<FName>& Idols = EquippedIdols;
	for (int32 i = 0; i < IdolSlotBorders.Num(); ++i)
	{
		if (!IdolSlotBorders[i].IsValid()) continue;
		FLinearColor C = FLinearColor(0.08f, 0.14f, 0.12f, 0.92f);
		TSoftObjectPtr<UTexture2D> IdolIconSoft;
		TSharedPtr<IToolTip> IdolTooltipWidget;
		if (i < Idols.Num() && !Idols[i].IsNone())
		{
			const ET66ItemRarity IdolRarity = IdolManager ? IdolManager->GetEquippedIdolRarityInSlot(i) : RunState->GetEquippedIdolRarityInSlot(i);
			C = FItemData::GetItemRarityColor(IdolRarity);
			if (GIAsT66)
			{
				FIdolData IdolData;
				if (GIAsT66->GetIdolData(Idols[i], IdolData))
				{
					IdolIconSoft = IdolData.GetIconForRarity(IdolRarity);
					if (Loc)
					{
						IdolTooltipWidget = CreateRichTooltip(
							Loc->GetText_IdolDisplayName(Idols[i]),
							Loc->GetText_IdolTooltip(Idols[i]));
					}
					else
					{
						IdolTooltipWidget = CreateCustomTooltip(FText::FromName(Idols[i]));
					}
				}
			}
		}
		IdolSlotBorders[i]->SetBorderBackgroundColor(C);
		if (IdolSlotContainers.IsValidIndex(i) && IdolSlotContainers[i].IsValid())
		{
			IdolSlotContainers[i]->SetToolTip(IdolTooltipWidget);
		}

		if (IdolSlotBrushes.IsValidIndex(i) && IdolSlotBrushes[i].IsValid())
		{
			UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
			if (IdolIconSoft.IsNull() || !TexPool)
			{
				IdolSlotBrushes[i]->SetResourceObject(nullptr);
			}
			else
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, IdolIconSoft, this, IdolSlotBrushes[i], FName(TEXT("HUDIdol"), i + 1), /*bClearWhileLoading*/ true);
			}
		}
		if (IdolSlotImages.IsValidIndex(i) && IdolSlotImages[i].IsValid())
		{
			IdolSlotImages[i]->SetVisibility(!IdolIconSoft.IsNull() ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}

	// Inventory slots: item color + hover tooltip, grey when empty
	UT66GameInstance* InventoryGI = Cast<UT66GameInstance>(GetGameInstance());
	const TArray<FName>& EquippedInventory = RunState->GetInventory();
	const TArray<FT66InventorySlot>& EquippedInventorySlots = RunState->GetInventorySlots();
	const TArray<FName>& Inv = EquippedInventory;
	const TArray<FT66InventorySlot>& InvSlots = EquippedInventorySlots;
	if (InvSlots.Num() > InventorySlotBorders.Num())
	{
		FT66Style::DeferRebuild(this);
		return;
	}
	for (int32 i = 0; i < InventorySlotBorders.Num(); ++i)
	{
		if (!InventorySlotBorders[i].IsValid()) continue;

		FLinearColor SlotColor = FLinearColor(0.f, 0.f, 0.f, 0.25f);
		FText Tooltip = FText::GetEmpty();
		TSoftObjectPtr<UTexture2D> SlotIconSoft;
		if (i < Inv.Num() && !Inv[i].IsNone())
		{
			const FName ItemID = Inv[i];
			FItemData D;
			if (InventoryGI && InventoryGI->GetItemData(ItemID, D))
			{
				SlotColor = InvSlots.IsValidIndex(i) ? FItemData::GetItemRarityColor(InvSlots[i].Rarity) : FT66Style::Tokens::Panel2;
				TArray<FText> TipLines;
				TipLines.Reserve(8);
				const ET66ItemRarity SlotRarity = InvSlots.IsValidIndex(i) ? InvSlots[i].Rarity : ET66ItemRarity::Black;
				TipLines.Add(Loc ? Loc->GetText_ItemDisplayNameForRarity(ItemID, SlotRarity) : FText::FromName(ItemID));

				// Icon (optional). Do NOT sync-load in gameplay UI; request via the UI texture pool.
				SlotIconSoft = D.GetIconForRarity(SlotRarity);

				int32 MainValue = 0;
				if (InvSlots.IsValidIndex(i))
				{
					MainValue = InvSlots[i].Line1RolledValue;
				}
				const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f;
				const FText CardDesc = T66ItemCardTextUtils::BuildItemCardDescription(Loc, D, SlotRarity, MainValue, ScaleMult, InvSlots.IsValidIndex(i) ? InvSlots[i].GetLine2Multiplier() : 0.f);
				if (!CardDesc.IsEmpty())
				{
					TipLines.Add(CardDesc);
				}
				{
					int32 SellValue = 0;
					if (RunState && i >= 0 && i < InvSlots.Num())
					{
						SellValue = RunState->GetSellGoldForInventorySlot(InvSlots[i]);
					}
					if (SellValue > 0)
					{
						TipLines.Add(FText::Format(
							NSLOCTEXT("T66.ItemTooltip", "SellValueGold", "Sell: {0} gold"),
							FText::AsNumber(SellValue)));
					}
				}

				Tooltip = TipLines.Num() > 0 ? FText::Join(NSLOCTEXT("T66.Common", "NewLine", "\n"), TipLines) : FText::GetEmpty();
			}
			else
			{
				SlotColor = FLinearColor(0.95f, 0.15f, 0.15f, 1.f);
				Tooltip = FText::FromName(ItemID);
			}
		}
		InventorySlotBorders[i]->SetBorderBackgroundColor(SlotColor);
		const FName CurrentItemID = (i < Inv.Num()) ? Inv[i] : NAME_None;
		if (!CachedInventorySlotIDs.IsValidIndex(i) || CachedInventorySlotIDs[i] != CurrentItemID)
		{
			if (CachedInventorySlotIDs.IsValidIndex(i)) CachedInventorySlotIDs[i] = CurrentItemID;
			if (InventorySlotContainers.IsValidIndex(i) && InventorySlotContainers[i].IsValid())
			{
				InventorySlotContainers[i]->SetToolTip(CreateCustomTooltip(Tooltip));
			}
		}

		if (InventorySlotBrushes.IsValidIndex(i) && InventorySlotBrushes[i].IsValid())
		{
			UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
			if (SlotIconSoft.IsNull() || !TexPool)
			{
				InventorySlotBrushes[i]->SetResourceObject(nullptr);
			}
			else
			{
					T66SlateTexture::BindSharedBrushAsync(TexPool, SlotIconSoft, this, InventorySlotBrushes[i], FName(TEXT("HUDInv"), i + 1), /*bClearWhileLoading*/ true);
			}
		}
		if (InventorySlotImages.IsValidIndex(i) && InventorySlotImages[i].IsValid())
		{
			InventorySlotImages[i]->SetVisibility(!SlotIconSoft.IsNull() ? EVisibility::Visible : EVisibility::Hidden);
		}
	}

	// Panel visibility: each element follows HUD toggle only if enabled in Settings (HUD tab).
	UGameInstance* GIHud = GetGameInstance();
	UT66PlayerSettingsSubsystem* HUDPS = GIHud ? GIHud->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	const bool bPanelsVisible = RunState->GetHUDPanelsVisible();
	auto ElemVis = [HUDPS, bPanelsVisible](bool bAffects) -> EVisibility
	{
		// If this element is not in the toggle set, always visible. Otherwise follow global panels state.
		if (!HUDPS || !bAffects) return EVisibility::Visible;
		return bPanelsVisible ? EVisibility::Visible : EVisibility::Collapsed;
	};
	if (InventoryPanelBox.IsValid()) InventoryPanelBox->SetVisibility(ElemVis(HUDPS ? HUDPS->GetHudToggleAffectsInventory() : true));
	if (MinimapPanelBox.IsValid()) MinimapPanelBox->SetVisibility(ElemVis(HUDPS ? HUDPS->GetHudToggleAffectsMinimap() : true));
	if (IdolSlotsPanelBox.IsValid()) IdolSlotsPanelBox->SetVisibility(ElemVis(HUDPS ? HUDPS->GetHudToggleAffectsIdolSlots() : true));
	if (PortraitStatPanelBox.IsValid()) PortraitStatPanelBox->SetVisibility(ElemVis(HUDPS ? HUDPS->GetHudToggleAffectsPortraitStats() : true));

	// Wheel spin panel: hide when all toggled panels would be collapsed (any one visible is enough to show wheel in its slot).
	const bool bAnyPanelVisible = (!HUDPS || HUDPS->GetHudToggleAffectsInventory() || HUDPS->GetHudToggleAffectsMinimap() || HUDPS->GetHudToggleAffectsIdolSlots() || HUDPS->GetHudToggleAffectsPortraitStats())
		? bPanelsVisible
		: true;
	RefreshPausePresentation();
	UpdateTikTokVisibility();
	if (WheelSpinBox.IsValid())
	{
		if (!bAnyPanelVisible)
		{
			WheelSpinBox->SetVisibility(EVisibility::Collapsed);
		}
		else
		{
			WheelSpinBox->SetVisibility(GetPresentationController().IsWheelPanelOpen() ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}
}


