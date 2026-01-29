// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "T66DataTypes.generated.h"

/**
 * Hero data row for the Hero DataTable
 * Each row represents one selectable hero in the game
 * NOTE: Localized display names are managed by UT66LocalizationSubsystem::GetText_HeroName()
 */
USTRUCT(BlueprintType)
struct T66_API FHeroData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identifier for this hero (used for spawning, saving, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName HeroID;

	/** Fallback display name (used if localization not found) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText DisplayName;

	/** Short description of the hero's playstyle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText Description;

	/** Placeholder color for the cylinder representation (until real mesh is added) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor PlaceholderColor = FLinearColor::White;

	/** The actual character Blueprint class to spawn (can be null for placeholder) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSoftClassPtr<APawn> HeroClass;

	/** Hero portrait texture for UI (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> Portrait;

	/** Map theme associated with this hero (for Solo runs) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FName MapTheme;

	/** Whether this hero is unlocked by default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	bool bUnlockedByDefault = true;

	FHeroData()
		: HeroID(NAME_None)
		, PlaceholderColor(FLinearColor::White)
		, bUnlockedByDefault(true)
	{}
};

/**
 * Companion data row for the Companion DataTable
 * Each row represents one selectable companion
 * NOTE: Localized display names are managed by UT66LocalizationSubsystem::GetText_CompanionName()
 */
USTRUCT(BlueprintType)
struct T66_API FCompanionData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identifier for this companion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName CompanionID;

	/** Fallback display name (used if localization not found) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText DisplayName;

	/** Lore text (begins with "She tells me...") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (MultiLine = true))
	FText LoreText;

	/** Placeholder color for visual representation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor PlaceholderColor = FLinearColor::White;

	/** The companion actor class to spawn alongside the hero */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSoftClassPtr<AActor> CompanionClass;

	/** Companion portrait texture for UI (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> Portrait;

	/** Whether this companion is unlocked by default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	bool bUnlockedByDefault = true;

	FCompanionData()
		: CompanionID(NAME_None)
		, PlaceholderColor(FLinearColor::White)
		, bUnlockedByDefault(true)
	{}
};

/**
 * Party size enumeration
 */
UENUM(BlueprintType)
enum class ET66PartySize : uint8
{
	Solo UMETA(DisplayName = "Solo"),
	Duo UMETA(DisplayName = "Duo"),
	Trio UMETA(DisplayName = "Trio")
};

/**
 * Difficulty levels matching leaderboard categories
 */
UENUM(BlueprintType)
enum class ET66Difficulty : uint8
{
	Easy UMETA(DisplayName = "Easy"),
	Medium UMETA(DisplayName = "Medium"),
	Hard UMETA(DisplayName = "Hard"),
	VeryHard UMETA(DisplayName = "Very Hard"),
	Impossible UMETA(DisplayName = "Impossible"),
	Perdition UMETA(DisplayName = "Perdition"),
	Final UMETA(DisplayName = "Final")
};

/**
 * Body type for heroes and companions
 */
UENUM(BlueprintType)
enum class ET66BodyType : uint8
{
	TypeA UMETA(DisplayName = "Type A"),
	TypeB UMETA(DisplayName = "Type B")
};

/**
 * Leaderboard filter type (Global, Friends, Streamers)
 */
UENUM(BlueprintType)
enum class ET66LeaderboardFilter : uint8
{
	Global UMETA(DisplayName = "Global"),
	Friends UMETA(DisplayName = "Friends"),
	Streamers UMETA(DisplayName = "Streamers")
};

/**
 * Leaderboard time filter (Current season vs All Time)
 */
UENUM(BlueprintType)
enum class ET66LeaderboardTime : uint8
{
	Current UMETA(DisplayName = "Current"),
	AllTime UMETA(DisplayName = "All Time")
};

/**
 * Leaderboard type (High Score vs Speed Run)
 */
UENUM(BlueprintType)
enum class ET66LeaderboardType : uint8
{
	HighScore UMETA(DisplayName = "High Score"),
	SpeedRun UMETA(DisplayName = "Speed Run")
};

/**
 * Leaderboard entry row for the Leaderboard DataTable
 * Each row represents one ranked entry
 */
USTRUCT(BlueprintType)
struct T66_API FLeaderboardEntry : public FTableRowBase
{
	GENERATED_BODY()

	/** Rank position (1-10 typically, or 11 for player's own rank) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 Rank = 0;

	/** Player display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString PlayerName;

	/** Total score for this run */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int64 Score = 0;

	/** Time to complete (in seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	float TimeSeconds = 0.0f;

	/** Hero ID used in this run */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FName HeroID;

	/** Party size of this run */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66PartySize PartySize = ET66PartySize::Solo;

	/** Difficulty of this run */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	/** Stage reached (1-66) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 StageReached = 0;

	/** Whether this is the local player's entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	bool bIsLocalPlayer = false;

	FLeaderboardEntry()
		: Rank(0)
		, Score(0)
		, TimeSeconds(0.0f)
		, HeroID(NAME_None)
		, PartySize(ET66PartySize::Solo)
		, Difficulty(ET66Difficulty::Easy)
		, StageReached(0)
		, bIsLocalPlayer(false)
	{}
};

/**
 * Achievement tier (rarity)
 */
UENUM(BlueprintType)
enum class ET66AchievementTier : uint8
{
	Black UMETA(DisplayName = "Black"),
	Red UMETA(DisplayName = "Red"),
	Yellow UMETA(DisplayName = "Yellow"),
	White UMETA(DisplayName = "White")
};

/**
 * Achievement data row for the Achievements DataTable
 */
USTRUCT(BlueprintType)
struct T66_API FAchievementData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identifier for this achievement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName AchievementID;

	/** Display name shown in UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText DisplayName;

	/** Description of what needs to be done */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText Description;

	/** Achievement tier (Black, Red, Yellow, White) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	ET66AchievementTier Tier = ET66AchievementTier::Black;

	/** Achievement Coins reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 RewardCoins = 25;

	/** Requirement count (e.g., kill 10 enemies) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 RequirementCount = 1;

	/** Current progress (runtime only, not stored in DataTable) */
	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	int32 CurrentProgress = 0;

	/** Whether this achievement is unlocked */
	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	bool bIsUnlocked = false;

	FAchievementData()
		: AchievementID(NAME_None)
		, Tier(ET66AchievementTier::Black)
		, RewardCoins(25)
		, RequirementCount(1)
		, CurrentProgress(0)
		, bIsUnlocked(false)
	{}
};

/**
 * Skin data for heroes and companions
 */
USTRUCT(BlueprintType)
struct T66_API FSkinData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identifier for this skin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName SkinID;

	/** Display name shown in UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText DisplayName;

	/** Hero or Companion ID this skin belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName OwnerID;

	/** Cost in coins to purchase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int32 CoinCost = 0;

	/** Whether this skin is owned by the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsOwned = false;

	/** Whether this skin is currently equipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsEquipped = false;

	/** Whether this is the default skin (always owned) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	bool bIsDefault = false;

	FSkinData()
		: SkinID(NAME_None)
		, OwnerID(NAME_None)
		, CoinCost(0)
		, bIsOwned(false)
		, bIsEquipped(false)
		, bIsDefault(false)
	{}
};
