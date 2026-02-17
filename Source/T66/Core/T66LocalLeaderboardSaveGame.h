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
};

USTRUCT(BlueprintType)
struct T66_API FT66LocalSpeedRunStageRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66PartySize PartySize = ET66PartySize::Solo;

	/** Stage number (1..10 used for the menu leaderboard; can store more for future use). */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 Stage = 1;

	/** Best time in seconds (lower is better). 0 means unset. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	float BestSeconds = 0.f;

	/** Whether the best time was submitted as anonymous. */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	bool bSubmittedAnonymous = false;
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
 * Local placeholder record for Account Status / Appeal.
 * Today this is purely local/offline (Steam moderation backend will replace it later).
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
	int32 SchemaVersion = 1;

	UPROPERTY(SaveGame)
	TArray<FT66LocalScoreRecord> ScoreRecords;

	UPROPERTY(SaveGame)
	TArray<FT66LocalSpeedRunStageRecord> SpeedRunStageRecords;

	/** Placeholder local account restriction state (for the Main Menu Account Status panel). */
	UPROPERTY(SaveGame)
	FT66AccountRestrictionRecord AccountRestriction;
};

