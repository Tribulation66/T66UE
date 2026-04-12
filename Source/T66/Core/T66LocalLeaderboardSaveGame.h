// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "GameFramework/SaveGame.h"
#include "T66LocalLeaderboardSaveGame.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66LocalScoreRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66PartySize PartySize = ET66PartySize::Solo;

	/** Best score (higher is better). */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int64 BestScore = 0;

	/** Whether the best score was submitted as anonymous. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	bool bSubmittedAnonymous = false;

	/** Saved run summary snapshot associated with this PB (viewer opens this slot). */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString RunSummarySlotName;

	/** UTC timestamp for when the PB was achieved. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FDateTime AchievedAtUtc = FDateTime::MinValue();

	/** Global all-time rank recorded for the current best-score run. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 BestScoreRankAllTime = 0;

	/** Best global all-time rank ever achieved for this difficulty + party. Lower is better. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 BestRankAllTime = 0;

	/** Score associated with BestRankAllTime. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int64 BestRankScore = 0;

	/** Run summary slot associated with BestRankAllTime. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString BestRankRunSummarySlotName;

	/** UTC timestamp for when BestRankAllTime was achieved. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FDateTime BestRankAchievedAtUtc = FDateTime::MinValue();
};

USTRUCT(BlueprintType)
struct T66_API FT66LocalSpeedRunStageRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66PartySize PartySize = ET66PartySize::Solo;

	/** Stage number (1..5 used for the menu leaderboard per difficulty; can store more for future use). */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 Stage = 1;

	/** Best time in seconds (lower is better). 0 means unset. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	float BestSeconds = 0.f;

	/** Whether the best time was submitted as anonymous. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	bool bSubmittedAnonymous = false;
};

USTRUCT(BlueprintType)
struct T66_API FT66LocalCompletedRunTimeRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66PartySize PartySize = ET66PartySize::Solo;

	/** Best completed full-run time in seconds (lower is better). */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	float BestCompletedSeconds = 0.f;

	/** Whether the best completed time was submitted as anonymous. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	bool bSubmittedAnonymous = false;

	/** Saved run summary snapshot associated with this PB (viewer opens this slot). */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString RunSummarySlotName;

	/** UTC timestamp for when the PB was achieved. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FDateTime AchievedAtUtc = FDateTime::MinValue();

	/** Global all-time rank recorded for the current best completed time. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 BestCompletedRankAllTime = 0;

	/** Best global all-time rank ever achieved for this difficulty + party. Lower is better. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 BestRankAllTime = 0;

	/** Completed-run time associated with BestRankAllTime. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	float BestRankCompletedSeconds = 0.f;

	/** Run summary slot associated with BestRankAllTime. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString BestRankRunSummarySlotName;

	/** UTC timestamp for when BestRankAllTime was achieved. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FDateTime BestRankAchievedAtUtc = FDateTime::MinValue();
};

USTRUCT(BlueprintType)
struct T66_API FT66RecentRunRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FGuid RunId;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FDateTime EndedAtUtc = FDateTime::MinValue();

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString RunSummarySlotName;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66PartySize PartySize = ET66PartySize::Solo;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FName HeroID = NAME_None;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FName CompanionID = NAME_None;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 Score = 0;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 StageReached = 1;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	float DurationSeconds = 0.f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	bool bWasFullClear = false;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	bool bWasSpeedRunMode = false;
};

UENUM(BlueprintType)
enum class ET66AccountRestrictionKind : uint8
{
	None UMETA(DisplayName = "None"),
	Suspicion UMETA(DisplayName = "Suspicion"),
	CheatingCertainty UMETA(DisplayName = "Cheating Certainty"),
};

UENUM(BlueprintType)
enum class ET66AppealReviewStatus : uint8
{
	NotSubmitted UMETA(DisplayName = "Not Submitted"),
	UnderReview UMETA(DisplayName = "Under Review"),
	Denied UMETA(DisplayName = "Denied"),
	Approved UMETA(DisplayName = "Approved"),
};

/**
 * Locally cached account-status / appeal record.
 * Refreshed from the backend when online, with a local fallback for offline/dev flows.
 */
USTRUCT(BlueprintType)
struct T66_API FT66AccountRestrictionRecord
{
	GENERATED_BODY()

	/** None = button hidden. Suspicion/Certainty = button visible. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "AccountStatus")
	ET66AccountRestrictionKind Restriction = ET66AccountRestrictionKind::None;

	/** Human-readable reason string (backend/detection authored). */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "AccountStatus")
	FString RestrictionReason;

	/** For viewer-mode run summary (optional). */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "AccountStatus")
	FString RunSummarySlotName;

	/** Current appeal status. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "AccountStatus")
	ET66AppealReviewStatus AppealStatus = ET66AppealReviewStatus::NotSubmitted;

	/** Last submitted appeal message (player-authored, not localized). */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "AccountStatus")
	FString LastAppealMessage;

	/** Last submitted evidence link (player-authored, not localized). */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "AccountStatus")
	FString LastEvidenceUrl;
};

/**
 * Local (offline) leaderboard persistence.
 * Stores the local player's best score and per-stage speedrun times.
 */
UCLASS()
class T66_API UT66LocalLeaderboardSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	/** Bump if fields change in a breaking way. */
	UPROPERTY(SaveGame)
	int32 SchemaVersion = 3;

	UPROPERTY(SaveGame)
	TArray<FT66LocalScoreRecord> ScoreRecords;

	UPROPERTY(SaveGame)
	TArray<FT66LocalSpeedRunStageRecord> SpeedRunStageRecords;

	UPROPERTY(SaveGame)
	TArray<FT66LocalCompletedRunTimeRecord> CompletedRunTimeRecords;

	/** Completed runs, newest first, saved locally with no hard history cap. */
	UPROPERTY(SaveGame)
	TArray<FT66RecentRunRecord> RecentRuns;

	/** Placeholder local account restriction state (for the Main Menu Account Status panel). */
	UPROPERTY(SaveGame)
	FT66AccountRestrictionRecord AccountRestriction;
};

