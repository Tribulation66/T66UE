#pragma once

#include "CoreMinimal.h"
#include "T66PlayerExperienceTypes.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66TotemRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Totems", meta = (ClampMin = "1"))
	int32 UsesPerTotem = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Totems", meta = (ClampMin = "1"))
	int32 TotemsPerGameplayStage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Totems", meta = (ClampMin = "1"))
	int32 GameplayStagesWithTotems = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Totems", meta = (ClampMin = "1"))
	int32 SkullColorBandSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Totems")
	bool bDifficultyPersistsAcrossStages = false;

	int32 GetMaxInteractionsPerRun() const
	{
		return FMath::Max(1, UsesPerTotem) * FMath::Max(1, TotemsPerGameplayStage) * FMath::Max(1, GameplayStagesWithTotems);
	}
};

USTRUCT(BlueprintType)
struct T66_API FT66EnemyScoreTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring", meta = (ClampMin = "0.0"))
	float BaseMultiplier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	bool bScaleWithDifficultyScalar = false;
};

USTRUCT(BlueprintType)
struct T66_API FT66BossScoreTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring", meta = (ClampMin = "0.0"))
	float BaseMultiplier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	bool bScaleWithDifficultyScalar = false;
};

USTRUCT(BlueprintType)
struct T66_API FT66SpawnBudget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Spawning", meta = (ClampMin = "0"))
	int32 GameplayFloorsPerStage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Spawning", meta = (ClampMin = "0"))
	int32 InitialEnemiesPerGameplayFloor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Spawning", meta = (ClampMin = "0"))
	int32 TotalInitialEnemiesPerStage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Spawning", meta = (ClampMin = "1"))
	int32 RuntimeEnemiesPerWave = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Spawning", meta = (ClampMin = "1"))
	int32 RuntimeMaxAliveEnemies = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Spawning", meta = (ClampMin = "0.1"))
	float RuntimeSpawnIntervalSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Spawning", meta = (ClampMin = "0"))
	int32 EstimatedRuntimeWavesPerStage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Spawning", meta = (ClampMin = "0"))
	int32 EstimatedRuntimeEnemiesPerStage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Spawning", meta = (ClampMin = "0"))
	int32 EstimatedTotalEnemiesPerStage = 0;
};

USTRUCT(BlueprintType)
struct T66_API FT66ScoreBudget
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	int32 EnemyScoreBudget = 0;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	int32 BossScoreBudget = 0;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	int32 EnemyScoreAwarded = 0;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	int32 BossScoreAwarded = 0;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	int32 RegisteredEnemySpawns = 0;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	int32 RegisteredBossSpawns = 0;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	TArray<int32> AwardedScorePerMinute;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	TArray<int32> AwardedScorePerStage;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	bool bExceededLegalScore = false;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	int32 FirstExceededAtStage = 0;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	float FirstExceededAtRunSeconds = 0.0f;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "PlayerExperience|Scoring")
	FString FailureReason;

	int32 GetTotalLegalScore() const
	{
		return EnemyScoreBudget + BossScoreBudget;
	}

	int32 GetTotalAwardedScore() const
	{
		return EnemyScoreAwarded + BossScoreAwarded;
	}

	int32 GetPeakAwardedScorePerMinute() const
	{
		int32 PeakScore = 0;
		for (const int32 Score : AwardedScorePerMinute)
		{
			PeakScore = FMath::Max(PeakScore, Score);
		}

		return PeakScore;
	}
};
