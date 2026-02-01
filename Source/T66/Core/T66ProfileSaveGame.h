// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "T66ProfileSaveGame.generated.h"

/**
 * Persistent achievement state for a single achievement.
 * Stored in profile save (lifetime progression).
 */
USTRUCT(BlueprintType)
struct T66_API FT66AchievementState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Achievements")
	int32 CurrentProgress = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Achievements")
	bool bIsUnlocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Achievements")
	bool bIsClaimed = false;
};

/**
 * Player profile save (not tied to run slots).
 * Stores lifetime meta-progression like Achievement Coins (AC) and achievement progress.
 */
UCLASS(BlueprintType)
class T66_API UT66ProfileSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 SaveVersion = 2;

	/** Achievement Coins (AC) wallet balance. Starts at 0 for new profiles. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Achievements")
	int32 AchievementCoinsBalance = 0;

	/** Lifetime achievements state keyed by AchievementID. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Achievements")
	TMap<FName, FT66AchievementState> AchievementStateByID;

	/** Optional lifetime stat: total enemies killed. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 LifetimeEnemiesKilled = 0;

	/**
	 * Companion Union progression (lifetime / profile).
	 * Key: CompanionID, Value: number of stages cleared while that companion was selected.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Union")
	TMap<FName, int32> CompanionUnionStagesClearedByID;
};

