#include "Core/RunState/T66RunStateSubsystem_Private.h"

using namespace T66RunStatePrivate;

namespace
{
	static void T66AccumulateScoreBucket(TArray<int32>& Buckets, const int32 BucketIndex, const int32 Points)
	{
		if (BucketIndex < 0 || Points <= 0)
		{
			return;
		}

		if (Buckets.Num() <= BucketIndex)
		{
			Buckets.SetNumZeroed(BucketIndex + 1);
		}

		Buckets[BucketIndex] = FMath::Clamp(Buckets[BucketIndex] + Points, 0, MAX_int32);
	}
}

void UT66RunStateSubsystem::ResetScoreBudgetContext()
{
	ScoreBudgetContext = FT66ScoreBudget{};
}

void UT66RunStateSubsystem::RefreshScoreBudgetOverflowState()
{
	if (ScoreBudgetContext.bExceededLegalScore)
	{
		return;
	}

	if (ScoreBudgetContext.GetTotalAwardedScore() <= ScoreBudgetContext.GetTotalLegalScore())
	{
		return;
	}

	ScoreBudgetContext.bExceededLegalScore = true;
	ScoreBudgetContext.FirstExceededAtStage = FMath::Max(1, CurrentStage);
	ScoreBudgetContext.FirstExceededAtRunSeconds = GetCurrentRunElapsedSeconds();
	ScoreBudgetContext.FailureReason = FString::Printf(
		TEXT("score_exceeded_legal_budget:%d>%d"),
		ScoreBudgetContext.GetTotalAwardedScore(),
		ScoreBudgetContext.GetTotalLegalScore());

	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		T66GI->bRunIneligibleForLeaderboard = true;
	}
}

void UT66RunStateSubsystem::RegisterSpawnedEnemyScoreBudget(const int32 Points, const int32 Stage)
{
	if (Points <= 0)
	{
		return;
	}

	ScoreBudgetContext.EnemyScoreBudget = FMath::Clamp(ScoreBudgetContext.EnemyScoreBudget + Points, 0, MAX_int32);
	ScoreBudgetContext.RegisteredEnemySpawns = FMath::Clamp(ScoreBudgetContext.RegisteredEnemySpawns + 1, 0, MAX_int32);
	RefreshScoreBudgetOverflowState();
	static_cast<void>(Stage);
}

void UT66RunStateSubsystem::RegisterSpawnedBossScoreBudget(const int32 Points, const int32 Stage, const FName BossID)
{
	if (Points <= 0)
	{
		return;
	}

	ScoreBudgetContext.BossScoreBudget = FMath::Clamp(ScoreBudgetContext.BossScoreBudget + Points, 0, MAX_int32);
	ScoreBudgetContext.RegisteredBossSpawns = FMath::Clamp(ScoreBudgetContext.RegisteredBossSpawns + 1, 0, MAX_int32);
	RefreshScoreBudgetOverflowState();
	static_cast<void>(Stage);
	static_cast<void>(BossID);
}

void UT66RunStateSubsystem::AddEnemyKillScore(const int32 Points)
{
	if (Points <= 0)
	{
		return;
	}

	AddScore(Points);
	ScoreBudgetContext.EnemyScoreAwarded = FMath::Clamp(ScoreBudgetContext.EnemyScoreAwarded + Points, 0, MAX_int32);
	T66AccumulateScoreBucket(
		ScoreBudgetContext.AwardedScorePerMinute,
		FMath::Max(0, FMath::FloorToInt(GetCurrentRunElapsedSeconds() / 60.0f)),
		Points);
	T66AccumulateScoreBucket(
		ScoreBudgetContext.AwardedScorePerStage,
		FMath::Max(0, CurrentStage - 1),
		Points);
	RefreshScoreBudgetOverflowState();
}

void UT66RunStateSubsystem::AddBossKillScore(const int32 Points, const FName BossID)
{
	if (Points <= 0)
	{
		return;
	}

	AddScore(Points);
	ScoreBudgetContext.BossScoreAwarded = FMath::Clamp(ScoreBudgetContext.BossScoreAwarded + Points, 0, MAX_int32);
	T66AccumulateScoreBucket(
		ScoreBudgetContext.AwardedScorePerMinute,
		FMath::Max(0, FMath::FloorToInt(GetCurrentRunElapsedSeconds() / 60.0f)),
		Points);
	T66AccumulateScoreBucket(
		ScoreBudgetContext.AwardedScorePerStage,
		FMath::Max(0, CurrentStage - 1),
		Points);
	RefreshScoreBudgetOverflowState();
	static_cast<void>(BossID);
}
