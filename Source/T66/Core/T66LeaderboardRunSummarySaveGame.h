// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "GameFramework/SaveGame.h"
#include "T66LeaderboardRunSummarySaveGame.generated.h"

/**
 * Snapshot of a run intended to be opened from a leaderboard entry.
 * This is the "beginning of the system" for per-leaderboard-run summary files.
 *
 * v0 scope: local best Bounty run only (offline leaderboard).
 */
UCLASS()
class T66_API UT66LeaderboardRunSummarySaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** Bump if fields change in a breaking way. */
	UPROPERTY(SaveGame)
	int32 SchemaVersion = 4;

	/** What kind of leaderboard this run belongs to (currently HighScore only). */
	UPROPERTY(SaveGame)
	ET66LeaderboardType LeaderboardType = ET66LeaderboardType::HighScore;

	UPROPERTY(SaveGame)
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(SaveGame)
	ET66PartySize PartySize = ET66PartySize::Solo;

	/** UTC timestamp for provenance/debug. */
	UPROPERTY(SaveGame)
	FDateTime SavedAtUtc = FDateTime::MinValue();

	// ===== Core run summary =====

	UPROPERTY(SaveGame)
	int32 StageReached = 1;

	UPROPERTY(SaveGame)
	int32 Bounty = 0;

	// ===== Identity / selection =====

	UPROPERTY(SaveGame)
	FName HeroID = NAME_None;

	UPROPERTY(SaveGame)
	ET66BodyType HeroBodyType = ET66BodyType::TypeA;

	UPROPERTY(SaveGame)
	FName CompanionID = NAME_None;

	UPROPERTY(SaveGame)
	ET66BodyType CompanionBodyType = ET66BodyType::TypeA;

	// ===== Level / stats =====

	UPROPERTY(SaveGame)
	int32 HeroLevel = 1;

	UPROPERTY(SaveGame)
	int32 DamageStat = 1;

	UPROPERTY(SaveGame)
	int32 AttackSpeedStat = 1;

	UPROPERTY(SaveGame)
	int32 AttackSizeStat = 1;

	UPROPERTY(SaveGame)
	int32 ArmorStat = 1;

	UPROPERTY(SaveGame)
	int32 EvasionStat = 1;

	UPROPERTY(SaveGame)
	int32 LuckStat = 1;

	UPROPERTY(SaveGame)
	int32 SpeedStat = 1;

	// ===== Ratings (post-run) =====

	/** Luck Rating (0..100). SchemaVersion>=2. */
	UPROPERTY(SaveGame)
	int32 LuckRating0To100 = -1;

	/** Quantity component (0..100). SchemaVersion>=2. */
	UPROPERTY(SaveGame)
	int32 LuckRatingQuantity0To100 = -1;

	/** Quality component (0..100). SchemaVersion>=2. */
	UPROPERTY(SaveGame)
	int32 LuckRatingQuality0To100 = -1;

	/** Skill Rating (0..100). SchemaVersion>=4. */
	UPROPERTY(SaveGame)
	int32 SkillRating0To100 = -1;

	// ===== Proof of Run =====

	/** Proof-of-run URL (e.g. YouTube). SchemaVersion>=3. */
	UPROPERTY(SaveGame)
	FString ProofOfRunUrl;

	/** True if ProofOfRunUrl is considered "confirmed" (locks edit by default). SchemaVersion>=3. */
	UPROPERTY(SaveGame)
	bool bProofOfRunLocked = false;

	// ===== Inventory / idols / log =====

	UPROPERTY(SaveGame)
	TArray<FName> EquippedIdols;

	UPROPERTY(SaveGame)
	TArray<FName> Inventory;

	UPROPERTY(SaveGame)
	TArray<FString> EventLog;
};

