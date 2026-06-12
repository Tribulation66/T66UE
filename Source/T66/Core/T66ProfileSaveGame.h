// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "GameFramework/SaveGame.h"
#include "Core/T66SaveMigration.h"
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

/** Permanent local custom hero build stored in the player profile, independent of run saves. */
USTRUCT(BlueprintType)
struct T66_API FT66SavedCustomHeroBuild
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Hero")
	bool bConfigured = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Hero")
	FName WeaponSourceHeroID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Hero")
	FName VisualSourceHeroID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Hero")
	ET66BodyType BodyType = ET66BodyType::Chad;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Hero")
	FT66HeroStatBlock Stats;
};

UENUM(BlueprintType)
enum class ET66AccountMedalTier : uint8
{
	None UMETA(DisplayName = "None"),
	Bronze UMETA(DisplayName = "Bronze"),
	Silver UMETA(DisplayName = "Silver"),
	Gold UMETA(DisplayName = "Gold"),
	Platinum UMETA(DisplayName = "Platinum"),
	Diamond UMETA(DisplayName = "Diamond"),
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
	int32 SaveVersion = T66CurrentProfileSaveVersion;

	/** Item IDs ever obtained (any run type) — used to show only unlocked items in The Lab. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lab")
	TArray<FName> LabUnlockedItemIDs;

	/** Enemy/Boss IDs ever killed (any run type) — used to show only unlocked entities in The Lab. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lab")
	TArray<FName> LabUnlockedEnemyIDs;

	/** Unified progression wallet. New profiles start with 10 Chad Coupons (CC). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 ChadCouponsBalance = 10;

	/** @deprecated Replaced by ChadCouponsBalance in SaveVersion 12. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Achievements", meta = (DeprecatedProperty))
	int32 AchievementCoinsBalance = 10000;

	/** Per-hero owned skin IDs (e.g. Hero_1 -> [DemoSkin]). Default is always considered owned. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skins")
	TMap<FName, FT66OwnedSkinsList> OwnedHeroSkinsByHero;

	/** Per-hero equipped skin ID (e.g. Hero_1 -> DemoSkin). Missing/Default means Default. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skins")
	TMap<FName, FName> EquippedHeroSkinIDByHero;

	/** @deprecated Migrated to OwnedHeroSkinsByHero in SaveVersion 5. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skins")
	TArray<FName> OwnedHeroSkinIDs;

	/** @deprecated Migrated to EquippedHeroSkinIDByHero in SaveVersion 5. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skins")
	FName EquippedHeroSkinID = FName(TEXT("Default"));

	/** Per-companion owned skin IDs. Default is always considered owned. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skins")
	TMap<FName, FT66OwnedSkinsList> OwnedCompanionSkinsByCompanion;

	/** Per-companion equipped skin ID. Missing/Default means Default. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skins")
	TMap<FName, FName> EquippedCompanionSkinIDByCompanion;

	/** Lifetime achievements state keyed by AchievementID. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Achievements")
	TMap<FName, FT66AchievementState> AchievementStateByID;

	/** Optional lifetime stat: total enemies killed. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 LifetimeEnemiesKilled = 0;

	/** Lifetime bosses defeated (stage or Coliseum). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 LifetimeBossesKilled = 0;

	/** Lifetime stage clears (any companion). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 LifetimeStagesCleared = 0;

	/** Lifetime runs completed (reached Run Summary). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 LifetimeRunsCompleted = 0;

	/** Lifetime shop purchases. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 LifetimeShopPurchases = 0;

	/** Lifetime gambler wins. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 LifetimeGamblerWins = 0;

	/** Highest unlocked Vendor Token rank (0 = locked). Runtime run-state token pickups convert to stacks. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 VendorTokenUnlockedLevel = 0;

	/** First-time onboarding: set true once the player completes the tutorial area. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial")
	bool bHasCompletedTutorial = false;

	/**
	 * Companion Union progression (lifetime / profile).
	 * Key: CompanionID, Value: number of stages cleared while that companion was selected.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Union")
	TMap<FName, int32> CompanionUnionStagesClearedByID;

	/**
	 * Hero Unity progression (lifetime / profile).
	 * Key: HeroID, Value: number of stages cleared while that hero was selected.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Union")
	TMap<FName, int32> HeroUnityStagesClearedByID;

	/** Lifetime games played per hero (increment once when a run reaches Run Summary). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	TMap<FName, int32> HeroGamesPlayedByID;

	/** Highest medal earned per hero from difficulty clears. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	TMap<FName, ET66AccountMedalTier> HeroHighestMedalByID;

	/** Lifetime cumulative score earned per hero across all completed runs. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	TMap<FName, int32> HeroCumulativeScoreByID;

	/** Simple hero mastery XP. Awarded once when a run reaches its summary flow. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	TMap<FName, int32> HeroMasteryXPByID;

	/** Lifetime games played per companion (increment once when a run reaches Run Summary). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	TMap<FName, int32> CompanionGamesPlayedByID;

	/** Highest medal earned per companion from difficulty clears. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	TMap<FName, ET66AccountMedalTier> CompanionHighestMedalByID;

	/** Lifetime cumulative score earned per companion across all completed runs. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	TMap<FName, int32> CompanionCumulativeScoreByID;

	/** Lifetime healing done by each companion across all completed runs. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	TMap<FName, int32> CompanionTotalHealingByID;

	/** Last hero selected/played in the main Tribulation frontend. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Selection")
	FName LastSelectedHeroID = NAME_None;

	/** Last companion selected/used in the main Tribulation frontend. NAME_None means no companion. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Selection")
	FName LastSelectedCompanionID = NAME_None;

	/** Permanent locally saved custom hero build. This is profile data, not run/load-state data. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom Hero")
	FT66SavedCustomHeroBuild CustomHeroBuild;

	/** Captured permanent pet IDs. IDs are keyed to the source stage boss. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pets")
	TArray<FName> CapturedPetIDs;

	/** Active pet selection for new runs. NAME_None means no active pet. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pets")
	FName ActivePetID = NAME_None;

	/** Most recently selected pet in the frontend. Kept separately for future UI resume behavior. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pets")
	FName LastSelectedPetID = NAME_None;

	/** Pet bond progression. Key: PetID, Value: stages cleared while active. Affects movement speed only. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pets")
	TMap<FName, int32> PetBondStagesClearedByID;

	/** Per-pet owned skin IDs. Default is always considered owned. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pets")
	TMap<FName, FT66OwnedSkinsList> OwnedPetSkinsByPet;

	/** Per-pet equipped skin ID. Missing/Default means Default. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pets")
	TMap<FName, FName> EquippedPetSkinIDByPet;
};

