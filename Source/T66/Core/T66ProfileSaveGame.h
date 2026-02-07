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

/** Wrapper for TMap value: list of owned skin IDs per hero (Unreal does not support TArray as TMap value in UPROPERTY). */
USTRUCT(BlueprintType)
struct T66_API FT66OwnedSkinsList
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skins")
	TArray<FName> SkinIDs;
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
	int32 SaveVersion = 7;

	/** Item IDs ever obtained (any run type) — used to show only unlocked items in The Lab. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lab")
	TArray<FName> LabUnlockedItemIDs;

	/** Enemy/Boss IDs ever killed (any run type) — used to show only unlocked entities in The Lab. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lab")
	TArray<FName> LabUnlockedEnemyIDs;

	/** Achievement Coins (AC) wallet balance. New profiles start with 10000 AC. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Achievements")
	int32 AchievementCoinsBalance = 10000;

	/** Per-hero owned skin IDs (e.g. Hero_1 -> [Beachgoer]). Default is always considered owned. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skins")
	TMap<FName, FT66OwnedSkinsList> OwnedHeroSkinsByHero;

	/** Per-hero equipped skin ID (e.g. Hero_1 -> Beachgoer). Missing/Default means Default. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skins")
	TMap<FName, FName> EquippedHeroSkinIDByHero;

	/** @deprecated Migrated to OwnedHeroSkinsByHero in SaveVersion 5. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skins")
	TArray<FName> OwnedHeroSkinIDs;

	/** @deprecated Migrated to EquippedHeroSkinIDByHero in SaveVersion 5. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skins")
	FName EquippedHeroSkinID = FName(TEXT("Default"));

	/** Per-companion owned skin IDs (e.g. Companion_01 -> [Beachgoer]). Default is always considered owned. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skins")
	TMap<FName, FT66OwnedSkinsList> OwnedCompanionSkinsByCompanion;

	/** Per-companion equipped skin ID (e.g. Companion_01 -> Beachgoer). Missing/Default means Default. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skins")
	TMap<FName, FName> EquippedCompanionSkinIDByCompanion;

	/** Lifetime achievements state keyed by AchievementID. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Achievements")
	TMap<FName, FT66AchievementState> AchievementStateByID;

	/** Optional lifetime stat: total enemies killed. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 LifetimeEnemiesKilled = 0;

	/** First-time onboarding: set true once the player completes the tutorial area. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial")
	bool bHasCompletedTutorial = false;

	/**
	 * Companion Union progression (lifetime / profile).
	 * Key: CompanionID, Value: number of stages cleared while that companion was selected.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Union")
	TMap<FName, int32> CompanionUnionStagesClearedByID;
};

