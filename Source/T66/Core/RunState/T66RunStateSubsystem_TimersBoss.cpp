// Copyright Tribulation 66. All Rights Reserved.

#include "Core/RunState/T66RunStateSubsystem_Private.h"

using namespace T66RunStatePrivate;

int32 UT66RunStateSubsystem::GetDifficultyScalarTier() const
{
	const int32 Skulls = FMath::Max(0, DifficultySkulls);
	if (Skulls <= 0) return 0;

	int32 SkullColorBandSize = 4;
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UT66PlayerExperienceSubSystem* PlayerExperience = GI->GetSubsystem<UT66PlayerExperienceSubSystem>())
		{
			const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
			const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
			SkullColorBandSize = PlayerExperience->GetDifficultySkullColorBandSize(Difficulty);
		}
	}

	return ((Skulls - 1) / FMath::Max(1, SkullColorBandSize)) + 1;
}


float UT66RunStateSubsystem::GetDifficultyScalar() const
{
	// Per spec update: every skull increments scalar by +0.1.
	// Skull 1 => 1.1x ... Skull 4 => 1.4x, then the HUD color band advances on skull 5.
	const int32 Skulls = FMath::Max(0, DifficultySkulls);
	return FMath::Clamp(1.0f + (0.1f * static_cast<float>(Skulls)), 1.0f, 99.0f);
}


void UT66RunStateSubsystem::IncreaseDifficultyTier()
{
	AddDifficultySkulls(1);
}


void UT66RunStateSubsystem::AddDifficultySkulls(int32 DeltaSkulls)
{
	if (DeltaSkulls == 0) return;
	DifficultySkulls = FMath::Clamp(DifficultySkulls + DeltaSkulls, 0, 9999);

	// Cache integer "tier" for backward-compat call sites (treated as skull count).
	DifficultyTier = FMath::Clamp(DifficultySkulls, 0, 999);
	DifficultyChanged.Broadcast();
}


int32 UT66RunStateSubsystem::RegisterTotemActivated()
{
	TotemsActivatedCount = FMath::Clamp(TotemsActivatedCount + 1, 0, 999);
	return TotemsActivatedCount;
}


void UT66RunStateSubsystem::SetInStageCatchUp(bool bInCatchUp)
{
	if (bInStageCatchUp == bInCatchUp) return;
	bInStageCatchUp = bInCatchUp;
	StageChanged.Broadcast();
}


void UT66RunStateSubsystem::AddOwedBoss(FName BossID)
{
	if (BossID.IsNone()) return;
	OwedBossIDs.Add(BossID);
	AddStructuredEvent(ET66RunEventType::StageExited, FString::Printf(TEXT("OwedBoss=%s"), *BossID.ToString()));
	LogAdded.Broadcast();
}


void UT66RunStateSubsystem::RemoveFirstOwedBoss()
{
	if (OwedBossIDs.Num() <= 0) return;
	OwedBossIDs.RemoveAt(0);
	LogAdded.Broadcast();
}


void UT66RunStateSubsystem::ClearOwedBosses()
{
	if (OwedBossIDs.Num() <= 0) return;
	OwedBossIDs.Empty();
	if (CowardiceGatesTakenCount != 0)
	{
		CowardiceGatesTakenCount = 0;
		CowardiceGatesTakenChanged.Broadcast();
	}
	LogAdded.Broadcast();
}


void UT66RunStateSubsystem::AddCowardiceGateTaken()
{
	CowardiceGatesTakenCount = FMath::Clamp(CowardiceGatesTakenCount + 1, 0, 9999);
	CowardiceGatesTakenChanged.Broadcast();
}


float UT66RunStateSubsystem::GetCurrentRunElapsedSeconds() const
{
	const float CurrentStageSeconds = (bSpeedRunStarted && SpeedRunStartWorldSeconds > 0.f && GetWorld())
		? FMath::Max(0.f, static_cast<float>(GetWorld()->GetTimeSeconds()) - SpeedRunStartWorldSeconds)
		: FMath::Max(0.f, SpeedRunElapsedSeconds);
	return FMath::Max(0.f, CompletedStageActiveSeconds + CurrentStageSeconds);
}


void UT66RunStateSubsystem::MarkRunEnded(bool bWasFullClear)
{
	if (bRunEnded)
	{
		if (bWasFullClear)
		{
			bRunEndedAsVictory = true;
		}
		return;
	}

	if (bWasFullClear)
	{
		RecordStagePacingPoint(CurrentStage, GetCurrentRunElapsedSeconds());
	}

	FinalRunElapsedSeconds = GetCurrentRunElapsedSeconds();
	bRunEnded = true;
	bRunEndedAsVictory = bWasFullClear;
}


bool UT66RunStateSubsystem::HasStagePacingPoint(const int32 Stage) const
{
	return StagePacingPoints.ContainsByPredicate([Stage](const FT66StagePacingPoint& Point)
	{
		return Point.Stage == Stage;
	});
}


void UT66RunStateSubsystem::RecordStagePacingPoint(const int32 Stage, const float CumulativeElapsedSeconds)
{
	const int32 ClampedStage = FMath::Clamp(Stage, 1, 23);
	if (ClampedStage <= 0)
	{
		return;
	}

	FT66StagePacingPoint Point;
	Point.Stage = ClampedStage;
	Point.Score = FMath::Max(0, CurrentScore);
	Point.ElapsedSeconds = FMath::Max(0.f, CumulativeElapsedSeconds);

	const int32 ExistingIndex = StagePacingPoints.IndexOfByPredicate([ClampedStage](const FT66StagePacingPoint& ExistingPoint)
	{
		return ExistingPoint.Stage == ClampedStage;
	});

	if (ExistingIndex != INDEX_NONE)
	{
		StagePacingPoints[ExistingIndex] = Point;
		return;
	}

	StagePacingPoints.Add(Point);
	StagePacingPoints.Sort([](const FT66StagePacingPoint& A, const FT66StagePacingPoint& B)
	{
		return A.Stage < B.Stage;
	});

	AddStructuredEvent(ET66RunEventType::StageExited, T66LeaderboardPacing::MakeStageMarker(Point.Stage, Point.Score, Point.ElapsedSeconds));
}


void UT66RunStateSubsystem::SetCurrentStage(int32 Stage)
{
	const int32 NewStage = FMath::Clamp(Stage, 1, 23);
	if (CurrentStage == NewStage) return;

	// If Speed Run Mode is enabled, record the stage completion time for the stage we're leaving.
	// This is used for the main menu Speed Run leaderboard (stage 1..5 per difficulty).
	{
		UGameInstance* GI = GetGameInstance();
		UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
		const bool bSpeedRunMode = PS ? PS->GetSpeedRunMode() : false;
		UWorld* World = GetWorld();
		const float Now = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
		const float Elapsed = (bSpeedRunStarted && SpeedRunStartWorldSeconds > 0.f)
			? FMath::Max(0.f, Now - SpeedRunStartWorldSeconds)
			: FMath::Max(0.f, SpeedRunElapsedSeconds);
		CompletedStageActiveSeconds += Elapsed;
		RecordStagePacingPoint(CurrentStage, CompletedStageActiveSeconds);
		static_cast<void>(bSpeedRunMode);
	}

	CurrentStage = NewStage;

	// New stage: clear transient movement/status effects so the Start Area is clean.
	StageMoveSpeedMultiplier = 1.f;
	StageMoveSpeedSecondsRemaining = 0.f;
	StatusBurnSecondsRemaining = 0.f;
	StatusBurnDamagePerSecond = 0.f;
	StatusBurnAccumDamage = 0.f;
	StatusChillSecondsRemaining = 0.f;
	StatusChillMoveSpeedMultiplier = 1.f;
	StatusCurseSecondsRemaining = 0.f;
	StatusEffectsChanged.Broadcast();
	HeroProgressChanged.Broadcast();
	// Bible: gambler anger resets at end of every stage.
	ResetGamblerAnger();
	ResetVendorForStage();
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		IdolManager->HandleStageChanged(NewStage);
	}
	StageChanged.Broadcast();
}


void UT66RunStateSubsystem::StartSpeedRunTimer(const bool bResetElapsed)
{
	UWorld* World = GetWorld();
	const float ExistingElapsed = bResetElapsed ? 0.f : FMath::Max(0.f, SpeedRunElapsedSeconds);
	SpeedRunElapsedSeconds = ExistingElapsed;
	bSpeedRunStarted = true;
	SpeedRunStartWorldSeconds = World ? (static_cast<float>(World->GetTimeSeconds()) - ExistingElapsed) : 0.f;
	LastBroadcastSpeedRunSecond = bResetElapsed ? -1 : FMath::FloorToInt(ExistingElapsed);
	SpeedRunTimerChanged.Broadcast();
}


void UT66RunStateSubsystem::StopSpeedRunTimer(const bool bKeepElapsed)
{
	if (!bSpeedRunStarted)
	{
		if (!bKeepElapsed && !FMath::IsNearlyZero(SpeedRunElapsedSeconds))
		{
			SpeedRunElapsedSeconds = 0.f;
			LastBroadcastSpeedRunSecond = -1;
			SpeedRunTimerChanged.Broadcast();
		}
		return;
	}

	if (bKeepElapsed && SpeedRunStartWorldSeconds > 0.f && GetWorld())
	{
		SpeedRunElapsedSeconds = FMath::Max(0.f, static_cast<float>(GetWorld()->GetTimeSeconds()) - SpeedRunStartWorldSeconds);
	}
	else if (!bKeepElapsed)
	{
		SpeedRunElapsedSeconds = 0.f;
	}

	SpeedRunStartWorldSeconds = 0.f;
	bSpeedRunStarted = false;
	LastBroadcastSpeedRunSecond = bKeepElapsed ? FMath::FloorToInt(SpeedRunElapsedSeconds) : -1;
	SpeedRunTimerChanged.Broadcast();
}


void UT66RunStateSubsystem::ResetSpeedRunTimer()
{
	SpeedRunElapsedSeconds = 0.f;
	SpeedRunStartWorldSeconds = 0.f;
	bSpeedRunStarted = false;
	LastBroadcastSpeedRunSecond = -1;
	SpeedRunTimerChanged.Broadcast();
}


void UT66RunStateSubsystem::SetStageTimerActive(bool bActive)
{
	if (bStageTimerActive == bActive) return;
	bStageTimerActive = bActive;

	StageTimerChanged.Broadcast();
}


void UT66RunStateSubsystem::TickStageTimer(float DeltaTime)
{
	if (!bStageTimerActive || StageTimerSecondsRemaining <= 0.f) return;
	StageTimerSecondsRemaining = FMath::Max(0.f, StageTimerSecondsRemaining - DeltaTime);
	const int32 CurrentSecond = FMath::FloorToInt(StageTimerSecondsRemaining);
	if (CurrentSecond != LastBroadcastStageTimerSecond)
	{
		LastBroadcastStageTimerSecond = CurrentSecond;
		StageTimerChanged.Broadcast();
	}
}


void UT66RunStateSubsystem::TickSpeedRunTimer(float DeltaTime)
{
	(void)DeltaTime;
	if (!bSpeedRunStarted) return;
	UWorld* World = GetWorld();
	if (!World) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	SpeedRunElapsedSeconds = FMath::Max(0.f, Now - SpeedRunStartWorldSeconds);

	const int32 CurrentSecond = FMath::FloorToInt(SpeedRunElapsedSeconds);
	if (CurrentSecond != LastBroadcastSpeedRunSecond)
	{
		LastBroadcastSpeedRunSecond = CurrentSecond;
		SpeedRunTimerChanged.Broadcast();
	}
}


void UT66RunStateSubsystem::ResetStageTimerToFull()
{
	bStageTimerActive = false;
	StageTimerSecondsRemaining = StageTimerDurationSeconds;
	LastBroadcastStageTimerSecond = static_cast<int32>(StageTimerDurationSeconds);
	StageTimerChanged.Broadcast();
}


void UT66RunStateSubsystem::ResetDifficultyPacing()
{
	StagePacingPoints.Reset();
	CompletedStageActiveSeconds = 0.f;
	FinalRunElapsedSeconds = 0.f;
	ResetSpeedRunTimer();
	bThisRunSetNewPersonalBestSpeedRunTime = false;
}


void UT66RunStateSubsystem::ResetDifficultyScore()
{
	if (CurrentScore == 0 && ScoreBudgetContext.GetTotalLegalScore() == 0 && ScoreBudgetContext.GetTotalAwardedScore() == 0)
	{
		return;
	}

	CurrentScore = 0;
	ResetScoreBudgetContext();
	ScoreChanged.Broadcast();
}


void UT66RunStateSubsystem::SetBossActive(int32 InMaxHP)
{
	SetBossActiveWithId(NAME_None, InMaxHP);
}


void UT66RunStateSubsystem::SetBossActiveWithId(FName InBossID, int32 InMaxHP)
{
	bBossActive = true;
	ActiveBossID = InBossID;
	BossMaxHP = FMath::Max(1, InMaxHP);
	BossCurrentHP = BossMaxHP;
	BossPartSnapshots.Reset();
	BossPartSnapshots.Shrink();
	BossPartSnapshots.AddDefaulted();
	BossPartSnapshots[0].PartID = FName(TEXT("Core"));
	BossPartSnapshots[0].HitZoneType = ET66HitZoneType::Core;
	BossPartSnapshots[0].MaxHP = BossMaxHP;
	BossPartSnapshots[0].CurrentHP = BossCurrentHP;
	BossChanged.Broadcast();
}


void UT66RunStateSubsystem::SetBossActiveWithParts(FName InBossID, const TArray<FT66BossPartSnapshot>& InBossParts)
{
	bBossActive = true;
	ActiveBossID = InBossID;
	BossPartSnapshots = InBossParts;
	T66_RecomputeBossAggregate(BossPartSnapshots, BossMaxHP, BossCurrentHP);
	BossChanged.Broadcast();
}


void UT66RunStateSubsystem::RescaleBossMaxHPPreservePercent(int32 NewMaxHP)
{
	if (!bBossActive) return;

	if (BossPartSnapshots.Num() > 0)
	{
		const int32 TargetMaxHP = FMath::Max(1, NewMaxHP);
		const int32 OldTotalMaxHP = FMath::Max(1, BossMaxHP);
		int32 RemainingHP = TargetMaxHP;

		for (int32 Index = 0; Index < BossPartSnapshots.Num(); ++Index)
		{
			FT66BossPartSnapshot& Part = BossPartSnapshots[Index];
			const int32 OldPartMaxHP = FMath::Max(1, Part.MaxHP);
			const int32 OldPartCurrentHP = FMath::Clamp(Part.CurrentHP, 0, OldPartMaxHP);
			const float PartPct = FMath::Clamp(static_cast<float>(OldPartCurrentHP) / static_cast<float>(OldPartMaxHP), 0.f, 1.f);
			const int32 RemainingParts = BossPartSnapshots.Num() - Index;

			int32 NewPartMaxHP = 1;
			if (Index == BossPartSnapshots.Num() - 1)
			{
				NewPartMaxHP = FMath::Max(1, RemainingHP);
			}
			else
			{
				NewPartMaxHP = FMath::Clamp(
					FMath::RoundToInt(static_cast<float>(TargetMaxHP) * static_cast<float>(OldPartMaxHP) / static_cast<float>(OldTotalMaxHP)),
					1,
					FMath::Max(1, RemainingHP - (RemainingParts - 1)));
			}

			Part.MaxHP = NewPartMaxHP;
			Part.CurrentHP = FMath::Clamp(FMath::RoundToInt(static_cast<float>(NewPartMaxHP) * PartPct), 0, NewPartMaxHP);
			RemainingHP = FMath::Max(0, RemainingHP - NewPartMaxHP);
		}

		T66_RecomputeBossAggregate(BossPartSnapshots, BossMaxHP, BossCurrentHP);
		BossChanged.Broadcast();
		return;
	}

	const int32 PrevMax = FMath::Max(1, BossMaxHP);
	const int32 PrevCur = FMath::Clamp(BossCurrentHP, 0, PrevMax);
	const float Pct = FMath::Clamp(static_cast<float>(PrevCur) / static_cast<float>(PrevMax), 0.f, 1.f);

	BossMaxHP = FMath::Max(1, NewMaxHP);
	BossCurrentHP = FMath::Clamp(FMath::RoundToInt(static_cast<float>(BossMaxHP) * Pct), 0, BossMaxHP);
	BossChanged.Broadcast();
}


void UT66RunStateSubsystem::SetBossInactive()
{
	bBossActive = false;
	ActiveBossID = NAME_None;
	BossPartSnapshots.Reset();
	BossCurrentHP = 0;
	BossChanged.Broadcast();
}


bool UT66RunStateSubsystem::ApplyBossDamage(int32 Damage)
{
	if (!bBossActive || Damage <= 0 || BossCurrentHP <= 0) return false;

	if (BossPartSnapshots.Num() > 0)
	{
		for (FT66BossPartSnapshot& Part : BossPartSnapshots)
		{
			if (Part.CurrentHP <= 0)
			{
				continue;
			}

			Part.CurrentHP = FMath::Max(0, Part.CurrentHP - Damage);
			T66_RecomputeBossAggregate(BossPartSnapshots, BossMaxHP, BossCurrentHP);
			BossChanged.Broadcast();
			return BossCurrentHP <= 0;
		}
	}

	BossCurrentHP = FMath::Max(0, BossCurrentHP - Damage);
	if (BossPartSnapshots.Num() == 1)
	{
		BossPartSnapshots[0].MaxHP = BossMaxHP;
		BossPartSnapshots[0].CurrentHP = BossCurrentHP;
	}
	BossChanged.Broadcast();
	return BossCurrentHP <= 0;
}


void UT66RunStateSubsystem::ResetBossState()
{
	bBossActive = false;
	ActiveBossID = NAME_None;
	BossMaxHP = 100;
	BossCurrentHP = 0;
	BossPartSnapshots.Reset();
	BossChanged.Broadcast();
}


void UT66RunStateSubsystem::AddScore(int32 Points)
{
	if (Points <= 0) return;
	CurrentScore += Points;
	ScoreChanged.Broadcast();
}


void UT66RunStateSubsystem::AddStructuredEvent(ET66RunEventType EventType, const FString& Payload)
{
	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	const float Timestamp = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
	StructuredEventLog.Add(FRunEvent(EventType, Timestamp, Payload));
	// Append human-readable line to EventLog for Run Summary scroll
	FString Short = Payload;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	switch (EventType)
	{
		case ET66RunEventType::StageEntered:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_StageFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Stage %s"), *Payload);
			break;
		case ET66RunEventType::ItemAcquired:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_PickedUpFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Picked up %s"), *Payload);
			break;
		case ET66RunEventType::GoldGained:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_GoldFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Gold: %s"), *Payload);
			break;
		case ET66RunEventType::EnemyKilled:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_KillFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Kill +%s"), *Payload);
			break;
		default:
			break;
	}
	EventLog.Add(Short);
	TrimLogsIfNeeded();
}
