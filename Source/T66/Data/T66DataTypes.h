// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "T66DataTypes.generated.h"

class AActor;

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

UENUM(BlueprintType)
enum class ET66ItemRarity : uint8
{
	Black UMETA(DisplayName = "Black"),
	Red UMETA(DisplayName = "Red"),
	Yellow UMETA(DisplayName = "Yellow"),
	White UMETA(DisplayName = "White"),
	Cursed UMETA(DisplayName = "Cursed"),
};

UENUM(BlueprintType)
enum class ET66ItemEffectType : uint8
{
	None UMETA(DisplayName = "None"),
	BonusDamagePct UMETA(DisplayName = "BonusDamagePct"),
	BonusAttackSpeedPct UMETA(DisplayName = "BonusAttackSpeedPct"),
	DashCooldownReductionPct UMETA(DisplayName = "DashCooldownReductionPct"),
};

/**
 * Item data row for the Items DataTable (v0: 3 placeholder items)
 */
USTRUCT(BlueprintType)
struct T66_API FItemData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rarity")
	ET66ItemRarity ItemRarity = ET66ItemRarity::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor PlaceholderColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int32 BuyValueGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int32 SellValueGold = 0;

	/** Single additive % that increases Damage + Attack Speed + Scale equally. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float PowerGivenPercent = 0.f;

	/** Optional single extra effect (v0 framework). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	ET66ItemEffectType EffectType = ET66ItemEffectType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float EffectMagnitude = 0.f;

	/** Tooltip lines (shown under Power). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FText EffectLine1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FText EffectLine2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FText EffectLine3;

	FItemData()
		: ItemID(NAME_None)
		, ItemRarity(ET66ItemRarity::Black)
		, PlaceholderColor(FLinearColor::White)
		, BuyValueGold(0)
		, SellValueGold(0)
		, PowerGivenPercent(0.f)
		, EffectType(ET66ItemEffectType::None)
		, EffectMagnitude(0.f)
	{}
};

/**
 * Boss data row for the Bosses DataTable (v0: single placeholder boss)
 */
USTRUCT(BlueprintType)
struct T66_API FBossData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identifier for this boss */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName BossID;

	/** Optional override class to spawn (defaults to AT66BossBase if unset) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSoftClassPtr<AActor> BossClass;

	/** Placeholder color for boss sphere */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor PlaceholderColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.f);

	/** Max HP (v0: 100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 MaxHP = 100;

	/** Proximity required to awaken from dormant state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AwakenDistance = 900.f;

	/** Chase speed once awakened */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MoveSpeed = 350.f;

	/** Fire interval once awakened (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float FireIntervalSeconds = 2.0f;

	/** Projectile speed (uu/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ProjectileSpeed = 900.f;

	/** Damage to hero per projectile hit (hearts) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 ProjectileDamageHearts = 1;

	FBossData()
		: BossID(NAME_None)
	{}
};

/**
 * Stage data row for the Stages DataTable (v0: stages are the same, but data-driven)
 *
 * Row name convention recommended: Stage_01, Stage_02, ...
 */
USTRUCT(BlueprintType)
struct T66_API FStageData : public FTableRowBase
{
	GENERATED_BODY()

	/** Stage number (1..66) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	int32 StageNumber = 1;

	/** Boss to spawn for this stage (Bosses DT row name) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	FName BossID;

	/** Boss spawn location for this stage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	FVector BossSpawnLocation = FVector(2500.f, 0.f, 200.f);

	FStageData()
		: StageNumber(1)
		, BossID(NAME_None)
	{}
};

/**
 * House NPC data row (Vendor/Gambler/Saint/Ouroboros).
 */
USTRUCT(BlueprintType)
struct T66_API FHouseNPCData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName NPCID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor NPCColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafeZone")
	float SafeZoneRadius = 650.f;

	/** Only used by Gambler: payout on correct guess. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gambler")
	int32 GamblerWinGold = 10;

	FHouseNPCData()
		: NPCID(NAME_None)
	{}
};

/**
 * Loan Shark data (tuning for debt collector NPC).
 */
USTRUCT(BlueprintType)
struct T66_API FLoanSharkData : public FTableRowBase
{
	GENERATED_BODY()

	/** Row ID (typically "LoanShark"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoanShark")
	FName LoanSharkID = FName(TEXT("LoanShark"));

	/** Base move speed when debt is 0 (should still not spawn at 0). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoanShark")
	float BaseMoveSpeed = 650.f;

	/** Additional move speed per 100 gold of debt (e.g. 50 means +50 speed per 100 debt). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoanShark")
	float MoveSpeedPer100Debt = 50.f;

	/** Base touch damage in hearts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoanShark")
	int32 BaseDamageHearts = 1;

	/** Extra hearts of damage per this many debt (e.g. 200 means +1 heart per 200 debt). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoanShark")
	int32 DebtPerExtraHeart = 200;

	FLoanSharkData() = default;
};

/**
 * Run event type for structured provenance (integrity / Run Summary)
 */
UENUM(BlueprintType)
enum class ET66RunEventType : uint8
{
	StageEntered   UMETA(DisplayName = "Stage Entered"),
	StageExited    UMETA(DisplayName = "Stage Exited"),
	ItemAcquired   UMETA(DisplayName = "Item Acquired"),
	GoldGained     UMETA(DisplayName = "Gold Gained"),
	EnemyKilled    UMETA(DisplayName = "Enemy Killed"),
	DamageDealt    UMETA(DisplayName = "Damage Dealt")
};

/**
 * Single structured run event (timestamp + payload for provenance)
 */
USTRUCT(BlueprintType)
struct T66_API FRunEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunEvent")
	ET66RunEventType EventType = ET66RunEventType::StageEntered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunEvent")
	float Timestamp = 0.f;

	/** Machine- and human-readable payload (e.g. "ItemID=X,Source=LootBag" or "PointValue=10") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunEvent")
	FString Payload;

	FRunEvent() = default;
	FRunEvent(ET66RunEventType InType, float InTime, const FString& InPayload)
		: EventType(InType), Timestamp(InTime), Payload(InPayload) {}
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
