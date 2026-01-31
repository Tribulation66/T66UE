// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "GameFramework/SaveGame.h"
#include "T66LocalLeaderboardSaveGame.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66LocalBountyRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66PartySize PartySize = ET66PartySize::Solo;

	/** Best bounty (higher is better). */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int64 BestBounty = 0;

	/** Whether the best bounty was submitted as anonymous. */
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

/**
 * Local (offline) leaderboard persistence.
 * Stores the local player's best bounty and per-stage speedrun times.
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
	TArray<FT66LocalBountyRecord> BountyRecords;

	UPROPERTY(SaveGame)
	TArray<FT66LocalSpeedRunStageRecord> SpeedRunStageRecords;
};

