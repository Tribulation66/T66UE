// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "T66DataTypes.generated.h"

class AActor;
class USkeletalMesh;
class UStaticMesh;
class UAnimationAsset;
class UTexture2D;

/**
 * Attack category (defines the fundamental behavior of a hero's primary attack and idol sources).
 */
UENUM(BlueprintType)
enum class ET66AttackCategory : uint8
{
	Pierce UMETA(DisplayName = "Pierce"),
	Bounce UMETA(DisplayName = "Bounce"),
	AOE UMETA(DisplayName = "AOE"),
	DOT UMETA(DisplayName = "DOT"),
};

/** Ultimate ability type per hero. */
UENUM(BlueprintType)
enum class ET66UltimateType : uint8
{
	None UMETA(DisplayName = "None"),
	SpearStorm UMETA(DisplayName = "Colossal Sword"),
	MeteorStrike UMETA(DisplayName = "Meteor Strike"),
	ChainLightning UMETA(DisplayName = "Chain Lightning"),
	PlagueCloud UMETA(DisplayName = "Plague Cloud"),
	PrecisionStrike UMETA(DisplayName = "Precision Strike"),
	FanTheHammer UMETA(DisplayName = "Fan the Hammer"),
	Deadeye UMETA(DisplayName = "Deadeye"),
	Discharge UMETA(DisplayName = "Discharge"),
	Juiced UMETA(DisplayName = "Juiced"),
	DeathSpiral UMETA(DisplayName = "Death Spiral"),
	Shockwave UMETA(DisplayName = "Shockwave"),
	TidalWave UMETA(DisplayName = "Tidal Wave"),
	GoldRush UMETA(DisplayName = "Gold Rush"),
	MiasmaBomb UMETA(DisplayName = "Miasma Bomb"),
	RabidFrenzy UMETA(DisplayName = "Rabid Frenzy"),
	Blizzard UMETA(DisplayName = "Blizzard"),
	ScopedSniper UMETA(DisplayName = "Scoped Sniper"),
};

/** Hero passive ability type (always-on innate). */
UENUM(BlueprintType)
enum class ET66PassiveType : uint8
{
	None UMETA(DisplayName = "None"),
	IronWill UMETA(DisplayName = "Iron Will"),
	RallyingBlow UMETA(DisplayName = "Rallying Blow"),
	ArcaneAmplification UMETA(DisplayName = "Arcane Amplification"),
	MarksmanFocus UMETA(DisplayName = "Marksman's Focus"),
	ToxinStacking UMETA(DisplayName = "Toxin Stacking"),
	QuickDraw UMETA(DisplayName = "Quick Draw"),
	Headshot UMETA(DisplayName = "Headshot"),
	StaticCharge UMETA(DisplayName = "Static Charge"),
	Overclock UMETA(DisplayName = "Overclock"),
	ChaosTheory UMETA(DisplayName = "Chaos Theory"),
	Endurance UMETA(DisplayName = "Endurance"),
	BrawlersFury UMETA(DisplayName = "Brawler's Fury"),
	Unflinching UMETA(DisplayName = "Unflinching"),
	TreasureHunter UMETA(DisplayName = "Treasure Hunter"),
	Evasive UMETA(DisplayName = "Evasive"),
	Frostbite UMETA(DisplayName = "Frostbite"),
};

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

	/** Hero half portrait texture for UI (Type A; used in selection/default hero UI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> Portrait;

	/** Hero half portrait texture for Type B (used in selection/default hero UI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> PortraitTypeB;

	/** Hero low-health portrait texture for Type A (used at 1 or fewer hearts). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> PortraitLow;

	/** Hero full-health portrait texture for Type A (used at 5 or more hearts). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> PortraitFull;

	/** Hero low-health portrait texture for Type B (used at 1 or fewer hearts). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> PortraitTypeBLow;

	/** Hero full-health portrait texture for Type B (used at 5 or more hearts). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> PortraitTypeBFull;

	/** Hero invincible/blessed portrait texture for Type A (used during the Saint blessing). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> PortraitInvincible;

	/** Hero invincible/blessed portrait texture for Type B (used during the Saint blessing). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> PortraitTypeBInvincible;

	/** Map theme associated with this hero (for Solo runs) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FName MapTheme;

	/** Max movement speed (UU/s). Hero ramps from base speed toward this at AccelerationPercentPerSecond per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Movement", meta = (ClampMin = "100", ClampMax = "3000"))
	float MaxSpeed = 1400.f;

	/** Percent of MaxSpeed gained per second when moving (e.g. 20 = 20% of max per second until capped). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Movement", meta = (ClampMin = "1", ClampMax = "100"))
	float AccelerationPercentPerSecond = 20.f;

	/** Whether this hero is unlocked by default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	bool bUnlockedByDefault = true;

	// ============================================
	// Attack Category
	// ============================================

	/** Primary attack category (Pierce/Bounce/AOE/DOT) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	ET66AttackCategory PrimaryCategory = ET66AttackCategory::Pierce;

	/** Ultimate ability type (None = legacy flat damage to all). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	ET66UltimateType UltimateType = ET66UltimateType::None;

	/** Passive ability type (always-on innate). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	ET66PassiveType PassiveType = ET66PassiveType::None;

	// ============================================
	// Generic Base Stats (leveled up via RNG)
	// ============================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Base")
	int32 BaseDamage = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Base")
	int32 BaseAttackSpeed = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Base")
	int32 BaseAttackScale = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Base")
	int32 BaseAccuracyStat = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Base")
	int32 BaseArmor = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Base")
	int32 BaseEvasion = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Base")
	int32 BaseLuck = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Base")
	int32 BaseSpeed = 2;

	// ============================================
	// Per-Level Gain Ranges (generic stats only)
	// ============================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlDmgMin = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlDmgMax = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlAtkSpdMin = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlAtkSpdMax = 0.4f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlAtkScaleMin = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlAtkScaleMax = 0.4f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlAccuracyMin = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlAccuracyMax = 0.4f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlArmorMin = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlArmorMax = 0.4f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlEvasionMin = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlEvasionMax = 0.4f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlLuckMin = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlLuckMax = 0.4f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlSpeedMin = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|LevelGains")
	float LvlSpeedMax = 0.4f;

	// ============================================
	// Category-Specific Base Stats (all 4 categories; boosted by items)
	// ============================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Pierce")
	int32 BasePierceDmg = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Pierce")
	int32 BasePierceAtkSpd = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Pierce")
	int32 BasePierceAtkScale = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Bounce")
	int32 BaseBounceDmg = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Bounce")
	int32 BaseBounceAtkSpd = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Bounce")
	int32 BaseBounceAtkScale = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|AOE")
	int32 BaseAoeDmg = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|AOE")
	int32 BaseAoeAtkSpd = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|AOE")
	int32 BaseAoeAtkScale = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|DOT")
	int32 BaseDotDmg = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|DOT")
	int32 BaseDotAtkSpd = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|DOT")
	int32 BaseDotAtkScale = 1;

	// ============================================
	// Primary Attack Properties
	// ============================================

	/** Seconds between auto-attacks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float BaseFireInterval = 1.f;

	/** Max targeting range (UU). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float BaseAttackRange = 1000.f;

	/** Instant hit damage before category effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	int32 BaseHitDamage = 20;

	/** Innate count for the hero's primary category effect (e.g., pierce-through count). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	int32 BasePierceCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	int32 BaseBounceCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	int32 BaseAoeCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	int32 BaseDotSources = 0;

	// ============================================
	// Behavior Tuning (primary attack)
	// ============================================

	/** Projectile travel speed (UU/s) for Pierce/Bounce. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tuning")
	float ProjectileSpeed = 2000.f;

	/** Damage reduction per pierce/bounce (0.15 = 15% loss per hit). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tuning")
	float FalloffPerHit = 0.f;

	/** Delay before AOE triggers after initial hit (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tuning")
	float AoeDelay = 0.f;

	/** Base AOE explosion radius (UU). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tuning")
	float AoeRadius = 0.f;

	/** Seconds between DOT ticks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tuning")
	float DotTickInterval = 0.f;

	/** Base DOT duration (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tuning")
	float DotDuration = 0.f;

	// ============================================
	// Secondary Stat Base Values (multiplied by item Line 2 rarity multiplier)
	// ============================================

	/** Base crit damage multiplier (e.g. 1.5 = 50% bonus on crit). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseCritDamage = 1.5f;

	/** Base crit chance (0..1, e.g. 0.05 = 5%). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseCritChance = 0.05f;

	/** Close range damage multiplier (1.0 = no bonus). Close = 0-10% of range. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseCloseRangeDmg = 1.0f;

	/** Long range damage multiplier (1.0 = no bonus). Long = 90-100% of range. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseLongRangeDmg = 1.0f;

	/** Base aggro generation multiplier (1.0 = normal). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseTaunt = 1.0f;

	/** Base reflect chance (0.01 = 1%). When it procs, 50% of damage is reflected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseReflectDmg = 0.01f;

	/** Base HP regen per second (0 = none). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseHpRegen = 0.0f;

	/** Base OHKO chance when reflecting (0.01 = 1%). Requires reflect to fire. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseCrushChance = 0.01f;

	/** Base chance to confuse enemies on hit (0.01 = 1%). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseInvisChance = 0.01f;

	/** Base counter-attack chance on dodge (0.01 = 1%). When it procs, 50% of would-be damage is dealt to attacker. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseCounterAttack = 0.01f;

	/** Base life-steal chance per hit (0.01 = 1%). When it procs, heal 10% of damage dealt. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseLifeSteal = 0.01f;

	/** Base OHKO chance when dodging (0.01 = 1%). Requires dodge to occur. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseAssassinateChance = 0.01f;

	/** Base cheating success chance (0.05 = 5%). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseCheatChance = 0.05f;

	/** Base stealing success chance (0.05 = 5%). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseStealChance = 0.05f;

	/** Base chance for untargeted auto-attacks to prefer enemy head hit zones. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Secondary")
	float BaseAccuracy = 0.15f;

	FHeroData()
		: HeroID(NAME_None)
		, PlaceholderColor(FLinearColor::White)
		, bUnlockedByDefault(true)
	{}
};

/**
 * Core hero stats used by the leveling system.
 * Intended tuning range (early): 1-5 at base, then increases via level ups.
 */
USTRUCT(BlueprintType)
struct T66_API FT66HeroStatBlock
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 Damage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 AttackSpeed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 AttackScale = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 Accuracy = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 Armor = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 Evasion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 Luck = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 Speed = 1;
};

/** Fixed-point hero stat storage in tenths (e.g. 17 = 1.7 displayed stat points). */
USTRUCT(BlueprintType)
struct T66_API FT66HeroPreciseStatBlock
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 DamageTenths = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 AttackSpeedTenths = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 AttackScaleTenths = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 AccuracyTenths = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 ArmorTenths = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 EvasionTenths = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 LuckTenths = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 SpeedTenths = 0;

	static constexpr int32 TenthsScale = 10;

	static int32 WholeStatToTenths(const int32 WholeValue)
	{
		return FMath::Max(0, WholeValue) * TenthsScale;
	}

	static int32 TenthsToDisplayStat(const int32 TenthsValue)
	{
		if (TenthsValue <= 0)
		{
			return 0;
		}

		return FMath::Max(1, FMath::CeilToInt(static_cast<float>(TenthsValue) / static_cast<float>(TenthsScale)));
	}

	void SetFromWholeStatBlock(const FT66HeroStatBlock& WholeStats)
	{
		DamageTenths = WholeStatToTenths(WholeStats.Damage);
		AttackSpeedTenths = WholeStatToTenths(WholeStats.AttackSpeed);
		AttackScaleTenths = WholeStatToTenths(WholeStats.AttackScale);
		AccuracyTenths = WholeStatToTenths(WholeStats.Accuracy);
		ArmorTenths = WholeStatToTenths(WholeStats.Armor);
		EvasionTenths = WholeStatToTenths(WholeStats.Evasion);
		LuckTenths = WholeStatToTenths(WholeStats.Luck);
		SpeedTenths = WholeStatToTenths(WholeStats.Speed);
	}

	FT66HeroStatBlock ToDisplayStatBlock() const
	{
		FT66HeroStatBlock WholeStats;
		WholeStats.Damage = TenthsToDisplayStat(DamageTenths);
		WholeStats.AttackSpeed = TenthsToDisplayStat(AttackSpeedTenths);
		WholeStats.AttackScale = TenthsToDisplayStat(AttackScaleTenths);
		WholeStats.Accuracy = TenthsToDisplayStat(AccuracyTenths);
		WholeStats.Armor = TenthsToDisplayStat(ArmorTenths);
		WholeStats.Evasion = TenthsToDisplayStat(EvasionTenths);
		WholeStats.Luck = TenthsToDisplayStat(LuckTenths);
		WholeStats.Speed = TenthsToDisplayStat(SpeedTenths);
		return WholeStats;
	}

	bool HasAnyPositiveValue() const
	{
		return DamageTenths > 0
			|| AttackSpeedTenths > 0
			|| AttackScaleTenths > 0
			|| AccuracyTenths > 0
			|| ArmorTenths > 0
			|| EvasionTenths > 0
			|| LuckTenths > 0
			|| SpeedTenths > 0;
	}
};

/** Flat additive stat bonuses (used for items, buffs, etc.). Defaults to 0. */
USTRUCT(BlueprintType)
struct T66_API FT66HeroStatBonuses
{
	GENERATED_BODY()

	// Generic stats (affect all attack categories)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 Damage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 AttackSpeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 AttackScale = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 Accuracy = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 Armor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 Evasion = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	int32 Luck = 0;

	// Category-specific stat bonuses (from items)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats|Pierce")
	int32 PierceDmg = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats|Pierce")
	int32 PierceAtkSpd = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats|Pierce")
	int32 PierceAtkScale = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats|Bounce")
	int32 BounceDmg = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats|Bounce")
	int32 BounceAtkSpd = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats|Bounce")
	int32 BounceAtkScale = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats|AOE")
	int32 AoeDmg = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats|AOE")
	int32 AoeAtkSpd = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats|AOE")
	int32 AoeAtkScale = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats|DOT")
	int32 DotDmg = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats|DOT")
	int32 DotAtkSpd = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats|DOT")
	int32 DotAtkScale = 0;
};

/** Random gain range (inclusive) for a stat on level up. */
USTRUCT(BlueprintType)
struct T66_API FT66HeroStatGainRange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	float Min = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	float Max = 0.f;

	int32 GetMinTenths() const
	{
		return FMath::Max(0, FMath::RoundToInt(FMath::Min(Min, Max) * 10.f));
	}

	int32 GetMaxTenths() const
	{
		return FMath::Max(0, FMath::RoundToInt(FMath::Max(Min, Max) * 10.f));
	}

	int32 RollTenths(FRandomStream& Rng) const
	{
		const int32 A = GetMinTenths();
		const int32 B = GetMaxTenths();
		if (B <= 0) return 0;
		return Rng.RandRange(A, B);
	}
};

/** Per-level gain ranges for the 7 foundational stats. */
USTRUCT(BlueprintType)
struct T66_API FT66HeroPerLevelStatGains
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	FT66HeroStatGainRange Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	FT66HeroStatGainRange AttackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	FT66HeroStatGainRange AttackScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	FT66HeroStatGainRange Accuracy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	FT66HeroStatGainRange Armor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	FT66HeroStatGainRange Evasion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	FT66HeroStatGainRange Luck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero|Stats")
	FT66HeroStatGainRange Speed;
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

	/** Companion portrait texture used by companion-selection UI (optional; falls back to Portrait). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> SelectionPortrait;

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
};

UENUM(BlueprintType)
enum class ET66ItemEffectType : uint8
{
	None UMETA(DisplayName = "None"),
	BonusDamagePct UMETA(DisplayName = "BonusDamagePct"),
	BonusAttackSpeedPct UMETA(DisplayName = "BonusAttackSpeedPct"),
	DashCooldownReductionPct UMETA(DisplayName = "DashCooldownReductionPct"),
	BonusMoveSpeedPct UMETA(DisplayName = "BonusMoveSpeedPct"),
	BonusArmorPctPoints UMETA(DisplayName = "BonusArmorPctPoints"),
	BonusEvasionPctPoints UMETA(DisplayName = "BonusEvasionPctPoints"),
	BonusLuckFlat UMETA(DisplayName = "BonusLuckFlat"),
};

/** Foundational hero stats that items can roll as their primary stat line (Line 1). */
UENUM(BlueprintType)
enum class ET66HeroStatType : uint8
{
	Damage UMETA(DisplayName = "Damage"),
	AttackSpeed UMETA(DisplayName = "AttackSpeed"),
	AttackScale UMETA(DisplayName = "AttackScale"),
	Armor UMETA(DisplayName = "Armor"),
	Evasion UMETA(DisplayName = "Evasion"),
	Luck UMETA(DisplayName = "Luck"),
	Speed UMETA(DisplayName = "Speed"),
	/** Appended to preserve serialized enum values for existing save data and data tables. */
	Accuracy UMETA(DisplayName = "Accuracy"),
};

/**
 * Secondary stat types (Line 2 of items).
 * Each item template maps to exactly one secondary stat. The multiplier is derived from rarity.
 */
UENUM(BlueprintType)
enum class ET66SecondaryStatType : uint8
{
	None UMETA(DisplayName = "None"),
	// Category Damage (4)
	AoeDamage UMETA(DisplayName = "AOE Damage"),
	BounceDamage UMETA(DisplayName = "Bounce Damage"),
	PierceDamage UMETA(DisplayName = "Pierce Damage"),
	DotDamage UMETA(DisplayName = "DOT Damage"),
	// Category Speed (4)
	AoeSpeed UMETA(DisplayName = "AOE Speed"),
	BounceSpeed UMETA(DisplayName = "Bounce Speed"),
	PierceSpeed UMETA(DisplayName = "Pierce Speed"),
	DotSpeed UMETA(DisplayName = "DOT Speed"),
	// Category Scale (4)
	AoeScale UMETA(DisplayName = "AOE Scale"),
	BounceScale UMETA(DisplayName = "Bounce Scale"),
	PierceScale UMETA(DisplayName = "Pierce Scale"),
	DotScale UMETA(DisplayName = "DOT Scale"),
	// Crit (2)
	CritDamage UMETA(DisplayName = "Crit Damage"),
	CritChance UMETA(DisplayName = "Crit Chance"),
	// Range-conditional (3)
	CloseRangeDamage UMETA(DisplayName = "Close Range Damage"),
	LongRangeDamage UMETA(DisplayName = "Long Range Damage"),
	AttackRange UMETA(DisplayName = "Range"),
	// Armor-defensive (4)
	Taunt UMETA(DisplayName = "Taunt"),
	ReflectDamage UMETA(DisplayName = "Damage Reflection"),
	HpRegen UMETA(DisplayName = "HP Regen"),
	Crush UMETA(DisplayName = "Crush"),
	// Evasion-offensive (4)
	Invisibility UMETA(DisplayName = "Invisibility"),
	CounterAttack UMETA(DisplayName = "Counter Attack"),
	LifeSteal UMETA(DisplayName = "Life Steal"),
	Assassinate UMETA(DisplayName = "Assassinate"),
	// Luck-world live (5) + compatibility (2)
	SpinWheel UMETA(DisplayName = "Spin Wheel"),
	Goblin UMETA(Hidden),
	Leprechaun UMETA(Hidden),
	TreasureChest UMETA(DisplayName = "Treasure Chest"),
	Fountain UMETA(Hidden),
	Cheating UMETA(DisplayName = "Cheating"),
	Stealing UMETA(DisplayName = "Stealing"),
	// Speed (1)
	MovementSpeed UMETA(DisplayName = "Movement Speed"),
	// Luck-world (crate rewards)
	LootCrate UMETA(DisplayName = "Loot Crate"),
	// Defensive item-only bonuses that scale from their parent primary stat.
	DamageReduction UMETA(DisplayName = "Damage Reduction"),
	EvasionChance UMETA(DisplayName = "Dodge"),
	GamblerToken UMETA(DisplayName = "Gambler's Token"),
	Alchemy UMETA(DisplayName = "Alchemy"),
	/** Appended to preserve serialized enum values for existing save data and data tables. */
	Accuracy UMETA(DisplayName = "Accuracy"),
};

FORCEINLINE bool T66IsDeprecatedSecondaryStatType(ET66SecondaryStatType StatType)
{
	return StatType == ET66SecondaryStatType::Goblin
		|| StatType == ET66SecondaryStatType::Leprechaun
		|| StatType == ET66SecondaryStatType::Fountain
		|| StatType == ET66SecondaryStatType::CloseRangeDamage
		|| StatType == ET66SecondaryStatType::LongRangeDamage
		|| StatType == ET66SecondaryStatType::SpinWheel
		|| StatType == ET66SecondaryStatType::MovementSpeed
		|| StatType == ET66SecondaryStatType::GamblerToken
		|| StatType == ET66SecondaryStatType::HpRegen
		|| StatType == ET66SecondaryStatType::LifeSteal
		|| StatType == ET66SecondaryStatType::Alchemy;
}

FORCEINLINE bool T66IsLiveSecondaryStatType(ET66SecondaryStatType StatType)
{
	return StatType != ET66SecondaryStatType::None && !T66IsDeprecatedSecondaryStatType(StatType);
}

FORCEINLINE bool T66IsAccuracyFamilySecondaryStatType(ET66SecondaryStatType StatType)
{
	return StatType == ET66SecondaryStatType::CritDamage
		|| StatType == ET66SecondaryStatType::CritChance
		|| StatType == ET66SecondaryStatType::AttackRange
		|| StatType == ET66SecondaryStatType::Accuracy;
}

FORCEINLINE ET66HeroStatType T66ResolveEffectivePrimaryStatType(ET66HeroStatType PrimaryStatType, ET66SecondaryStatType SecondaryStatType)
{
	return T66IsAccuracyFamilySecondaryStatType(SecondaryStatType)
		? ET66HeroStatType::Accuracy
		: PrimaryStatType;
}

UENUM(BlueprintType)
enum class ET66StageEffectType : uint8
{
	None UMETA(DisplayName = "None"),
	Speed UMETA(DisplayName = "Speed"),
	Launch UMETA(DisplayName = "Launch"),
	Slide UMETA(DisplayName = "Slide"),
};

UENUM(BlueprintType)
enum class ET66HeroStatusEffectType : uint8
{
	None UMETA(DisplayName = "None"),
	Burn UMETA(DisplayName = "Burn"),
	Chill UMETA(DisplayName = "Chill"),
	Curse UMETA(DisplayName = "Curse"),
};

/**
 * Item template data row for the Items DataTable.
 *
 * There are 28 live item templates plus deprecated legacy rows kept for compatibility.
 * Each template defines:
 *   - Line 1: a primary stat type (one of the item-facing foundational stats)
 *   - Line 2: a secondary stat type (one live secondary effect)
 *
 * Rarity and Line 1 rolled value are stored at runtime in FT66InventorySlot.
 * Line 2 multiplier is still preserved for legacy item-card text, but gameplay now prefers
 * fixed flat secondary rewards per rarity plus primary-driven proxy rolls.
 */
USTRUCT(BlueprintType)
struct T66_API FItemData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique template identifier (e.g. "Item_AoeDamage", "Item_CritChance"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName ItemID;

	/** Default/fallback UI icon. Uses the black rarity sprite when rarity-specific icons are available. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Rarity-specific icons (black/red/yellow/white). Falls back to Icon when unset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> BlackIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> RedIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> YellowIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> WhiteIcon;

	/** Line 1: Primary stat this item boosts (additive flat bonus, rolled at drop time). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	ET66HeroStatType PrimaryStatType = ET66HeroStatType::Damage;

	/** Line 2: Secondary effect provided by this item (multiplied by rarity multiplier). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	ET66SecondaryStatType SecondaryStatType = ET66SecondaryStatType::None;

	/** Base buy price in gold (scaled by rarity multiplier at shop time). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int32 BaseBuyGold = 50;

	/** Base sell price in gold (scaled by rarity multiplier at sell time). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int32 BaseSellGold = 25;

	FItemData()
		: ItemID(NAME_None)
	{}

	// ============================================
	// Rarity Helpers (static)
	// ============================================

	/** Line 1 additive flat roll range per rarity. */
	static void GetLine1RollRange(ET66ItemRarity Rarity, int32& OutMin, int32& OutMax)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:  OutMin = 1;  OutMax = 1;  break;
		case ET66ItemRarity::Red:    OutMin = 3;  OutMax = 3;  break;
		case ET66ItemRarity::Yellow: OutMin = 9;  OutMax = 9;  break;
		case ET66ItemRarity::White:  OutMin = 27; OutMax = 27; break;
		default:                     OutMin = 1;  OutMax = 1;  break;
		}
	}

	static int32 GetFlatSecondaryStatBonus(ET66ItemRarity Rarity)
	{
		int32 OutMin = 1;
		int32 OutMax = 1;
		GetLine1RollRange(Rarity, OutMin, OutMax);
		return OutMax;
	}

	static int32 GetAlchemyFlatStatBonus(ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:  return 1;
		case ET66ItemRarity::Red:    return 4;
		case ET66ItemRarity::Yellow: return 10;
		case ET66ItemRarity::White:  return 30;
		default:                     return 1;
		}
	}

	/** Line 2 multiplicative scalar per rarity. */
	static float GetLine2RarityMultiplier(ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:  return 1.1f;
		case ET66ItemRarity::Red:    return 1.2f;
		case ET66ItemRarity::Yellow: return 1.5f;
		case ET66ItemRarity::White:  return 2.0f;
		default:                     return 1.0f;
		}
	}

	/** Buy price scaled by rarity. */
	static float GetRarityPriceMultiplier(ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:  return 1.0f;
		case ET66ItemRarity::Red:    return 2.0f;
		case ET66ItemRarity::Yellow: return 4.0f;
		case ET66ItemRarity::White:  return 8.0f;
		default:                     return 1.0f;
		}
	}

	int32 GetBuyGoldForRarity(ET66ItemRarity Rarity) const
	{
		return FMath::RoundToInt(static_cast<float>(BaseBuyGold) * GetRarityPriceMultiplier(Rarity));
	}

	int32 GetSellGoldForRarity(ET66ItemRarity Rarity) const
	{
		return FMath::RoundToInt(static_cast<float>(BaseSellGold) * GetRarityPriceMultiplier(Rarity));
	}

	TSoftObjectPtr<UTexture2D> GetIconForRarity(ET66ItemRarity Rarity) const
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:
			return !BlackIcon.IsNull() ? BlackIcon : Icon;
		case ET66ItemRarity::Red:
			return !RedIcon.IsNull() ? RedIcon : (!BlackIcon.IsNull() ? BlackIcon : Icon);
		case ET66ItemRarity::Yellow:
			return !YellowIcon.IsNull() ? YellowIcon : (!RedIcon.IsNull() ? RedIcon : (!BlackIcon.IsNull() ? BlackIcon : Icon));
		case ET66ItemRarity::White:
			return !WhiteIcon.IsNull() ? WhiteIcon : (!YellowIcon.IsNull() ? YellowIcon : (!RedIcon.IsNull() ? RedIcon : (!BlackIcon.IsNull() ? BlackIcon : Icon)));
		default:
			return !BlackIcon.IsNull() ? BlackIcon : Icon;
		}
	}

	/** UI border/tint color for an item rarity tier. */
	static FLinearColor GetItemRarityColor(ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:  return FLinearColor(0.10f, 0.10f, 0.12f, 1.f);
		case ET66ItemRarity::Red:    return FLinearColor(0.90f, 0.20f, 0.20f, 1.f);
		case ET66ItemRarity::Yellow: return FLinearColor(0.95f, 0.80f, 0.15f, 1.f);
		case ET66ItemRarity::White:  return FLinearColor(0.92f, 0.92f, 0.96f, 1.f);
		default:                     return FLinearColor(0.10f, 0.10f, 0.12f, 1.f);
		}
	}
};

/**
 * Runtime inventory slot: stores a specific item instance with its rarity and rolled primary bonus.
 * The item template (FItemData) is looked up by ItemTemplateID.
 */
USTRUCT(BlueprintType)
struct T66_API FT66InventorySlot
{
	GENERATED_BODY()

	/** Maps to a row in the Items DataTable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FName ItemTemplateID;

	/** Rarity tier of this specific instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	ET66ItemRarity Rarity = ET66ItemRarity::Black;

	/** Rolled Line 1 additive bonus (randomized at drop/shop time). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Line1RolledValue = 1;

	/** Optional runtime override for the flat secondary stat reward on this slot. <= 0 uses the rarity default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 SecondaryStatBonusOverride = 0;

	/** Optional runtime override for Line 2 multiplier. <= 0 means use the default rarity multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float Line2MultiplierOverride = 0.f;

	/** Stable per-item seed used to derive deterministic proxy-secondary rolls. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 RollSeed = 0;

	FT66InventorySlot()
		: ItemTemplateID(NAME_None)
		, Rarity(ET66ItemRarity::Black)
		, Line1RolledValue(1)
		, SecondaryStatBonusOverride(0)
		, Line2MultiplierOverride(0.f)
		, RollSeed(0)
	{}

	FT66InventorySlot(FName InTemplateID, ET66ItemRarity InRarity, int32 InRolledValue, float InLine2MultiplierOverride = 0.f, int32 InSecondaryStatBonusOverride = 0, int32 InRollSeed = 0)
		: ItemTemplateID(InTemplateID)
		, Rarity(InRarity)
		, Line1RolledValue(InRolledValue)
		, SecondaryStatBonusOverride(InSecondaryStatBonusOverride)
		, Line2MultiplierOverride(InLine2MultiplierOverride)
		, RollSeed(InRollSeed)
	{}

	bool IsValid() const { return !ItemTemplateID.IsNone(); }

	int32 GetSecondaryStatBonusValue() const
	{
		return SecondaryStatBonusOverride > 0 ? SecondaryStatBonusOverride : FItemData::GetFlatSecondaryStatBonus(Rarity);
	}

	/** Get the Line 2 multiplier for this slot's rarity. */
	float GetLine2Multiplier() const { return Line2MultiplierOverride > 0.f ? Line2MultiplierOverride : FItemData::GetLine2RarityMultiplier(Rarity); }
};

/**
 * Boss data row for the Bosses DataTable (v0: single placeholder boss)
 */
UENUM(BlueprintType)
enum class ET66BossPartProfile : uint8
{
	UseActorDefault UMETA(DisplayName = "Use Actor Default"),
	HumanoidBalanced UMETA(DisplayName = "Humanoid Balanced"),
	Sharpshooter UMETA(DisplayName = "Sharpshooter"),
	Juggernaut UMETA(DisplayName = "Juggernaut"),
	Duelist UMETA(DisplayName = "Duelist"),
};

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

	/** Score awarded for defeating this boss (before difficulty scalar). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 PointValue = 0;

	/** Generic multipart layout profile for bosses that use AT66BossBase directly. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	ET66BossPartProfile BossPartProfile = ET66BossPartProfile::UseActorDefault;

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

	/** Stage number (1..23) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	int32 StageNumber = 1;

	/** Boss to spawn for this stage (Bosses DT row name) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	FName BossID;

	/** Boss spawn location for this stage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	FVector BossSpawnLocation = FVector(2500.f, 0.f, 200.f);

	/** Stage effect applied by ground tiles (helpful movement effect). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Effects")
	ET66StageEffectType StageEffectType = ET66StageEffectType::None;

	/** Visual color for stage effect tiles. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Effects")
	FLinearColor StageEffectColor = FLinearColor(0.20f, 0.90f, 0.35f, 1.f);

	/** Strength scalar for stage effect tuning (1.0 = default). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Effects")
	float StageEffectStrength = 1.f;

	/** Mob roster for this stage (exactly 3). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Enemies")
	FName EnemyA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Enemies")
	FName EnemyB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Enemies")
	FName EnemyC;

	FStageData()
		: StageNumber(1)
		, BossID(NAME_None)
	{}
};

/**
 * Idol data row for the Idols DataTable.
 *
 * Each idol is an independent attack source (fires alongside the hero's basic attack).
 * Category determines the type of effect (Pierce/Bounce/AOE/DOT).
 * Rarity determines the strength: black/red/yellow/white map directly to tiers 1..4.
 * Damage scales linearly, and the category property (bounce count, pierce count,
 * AOE radius, DOT duration) scales linearly.
 *
 * Formula: ValueAtTier(T) = Base + (T - 1) * PerTier
 */
USTRUCT(BlueprintType)
struct T66_API FIdolData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName IdolID;

	/** Attack category this idol source belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	ET66AttackCategory Category = ET66AttackCategory::Pierce;

	/** UI icon (sprite) for this idol. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Rarity-specific icons (black/red/yellow/white). Falls back to Icon when unset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> BlackIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> RedIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> YellowIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> WhiteIcon;

	/** Deprecated legacy field. Idol progression now uses the four live rarity tiers directly. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	int32 MaxLevel = 4;

	// ============================================
	// Damage scaling (linear: Base + (Tier-1) * PerTier)
	// ============================================

	/** Base damage at black rarity (before hero stat multipliers). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float BaseDamage = 8.f;

	/** Additional damage per rarity tier above black. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float DamagePerLevel = 4.f;

	// ============================================
	// Category property scaling (meaning varies by category)
	//   Pierce: enemies pierced   |  Bounce: bounce count
	//   AOE: explosion radius (UU)|  DOT: duration (seconds)
	// ============================================

	/** Base property value at black rarity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float BaseProperty = 1.f;

	/** Additional property per rarity tier above black. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float PropertyPerLevel = 1.f;

	// ============================================
	// Behavior tuning (per-idol source; mirrors hero attack tuning)
	// ============================================

	/** Projectile speed (UU/s) for Pierce/Bounce idol sources. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float ProjectileSpeed = 2000.f;

	/** Damage falloff per pierce/bounce (0.15 = 15% loss). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float FalloffPerHit = 0.f;

	/** Delay before AOE triggers (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float AoeDelay = 0.15f;

	/** AOE explosion radius (UU). Only used by AOE category. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float AoeRadius = 300.f;

	/** DOT tick interval (seconds). Only used by DOT category. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float DotTickInterval = 0.5f;

	/** DOT base duration (seconds). Only used by DOT category. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float DotDuration = 3.f;

	static int32 GetRarityStepIndex(ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:  return 0;
		case ET66ItemRarity::Red:    return 1;
		case ET66ItemRarity::Yellow: return 2;
		case ET66ItemRarity::White:  return 3;
		default:                     return 0;
		}
	}

	TSoftObjectPtr<UTexture2D> GetIconForRarity(ET66ItemRarity Rarity) const
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:
			return !BlackIcon.IsNull() ? BlackIcon : Icon;
		case ET66ItemRarity::Red:
			return !RedIcon.IsNull() ? RedIcon : (!BlackIcon.IsNull() ? BlackIcon : Icon);
		case ET66ItemRarity::Yellow:
			return !YellowIcon.IsNull() ? YellowIcon : (!RedIcon.IsNull() ? RedIcon : (!BlackIcon.IsNull() ? BlackIcon : Icon));
		case ET66ItemRarity::White:
			return !WhiteIcon.IsNull() ? WhiteIcon : (!YellowIcon.IsNull() ? YellowIcon : (!RedIcon.IsNull() ? RedIcon : (!BlackIcon.IsNull() ? BlackIcon : Icon)));
		default:
			return !BlackIcon.IsNull() ? BlackIcon : Icon;
		}
	}

	/** Get damage at a live rarity tier (1=black .. 4=white). */
	float GetDamageAtTier(int32 Tier) const
	{
		return BaseDamage + FMath::Max(0, FMath::Clamp(Tier, 1, 4) - 1) * DamagePerLevel;
	}

	/** Get category property value at a live rarity tier (1=black .. 4=white). */
	float GetPropertyAtTier(int32 Tier) const
	{
		return BaseProperty + FMath::Max(0, FMath::Clamp(Tier, 1, 4) - 1) * PropertyPerLevel;
	}

	float GetDamageAtRarity(ET66ItemRarity Rarity) const
	{
		return GetDamageAtTier(GetRarityStepIndex(Rarity) + 1);
	}

	float GetPropertyAtRarity(ET66ItemRarity Rarity) const
	{
		return GetPropertyAtTier(GetRarityStepIndex(Rarity) + 1);
	}

	FIdolData()
		: IdolID(NAME_None)
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
 * Character visual mapping row.
 * Maps a stable gameplay ID (HeroID / CompanionID / NPCID / BossID / EnemyVisualID) to imported skeletal assets.
 *
 * - SkeletalMesh: the mesh to assign to a USkeletalMeshComponent
 * - LoopingAnimation: walk animation (used when moving slowly)
 * - AlertAnimation: alert/stand animation (e.g. hero/companion selection preview)
 * - RunAnimation: run animation (used when moving fast); if unset, walk is used for all movement
 * - MeshRelative*: applied directly to the target component
 */
USTRUCT(BlueprintType)
struct T66_API FT66CharacterVisualRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Primary mesh to display. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	/** Walk animation (looping). Used when moving below run threshold. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UAnimationAsset> LoopingAnimation;

	/** Alert/stand animation (e.g. hero/companion selection preview). If set, used in preview instead of walk. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UAnimationAsset> AlertAnimation;

	/** Run animation (looping). Used when moving above run threshold. If unset, walk is used for all movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UAnimationAsset> RunAnimation;

	/** Relative location applied to the target mesh component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FVector MeshRelativeLocation = FVector(0.f, 0.f, -88.f);

	/** Relative rotation applied to the target mesh component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FRotator MeshRelativeRotation = FRotator(0.f, -90.f, 0.f);

	/** Relative scale applied to the target mesh component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FVector MeshRelativeScale = FVector(1.f, 1.f, 1.f);

	/** If true and LoopingAnimation is set, play it in a loop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	bool bLoopAnimation = true;

	/**
	 * If true, the system will auto-adjust the target component's Z so the mesh bounds bottom sits at the actor origin.
	 * Intended for non-capsule actors (companions, house NPCs) where the actor origin is treated as "ground contact".
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	bool bAutoGroundToActorOrigin = false;

	FT66CharacterVisualRow() = default;
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
 * Cumulative score/time checkpoint captured at the end of a stage.
 * Used by HUD pacing comparisons and persisted through run summaries.
 */
USTRUCT(BlueprintType)
struct T66_API FT66StagePacingPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunEvent")
	int32 Stage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunEvent")
	int32 Score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunEvent")
	float ElapsedSeconds = 0.f;
};

/**
 * Party size enumeration
 */
UENUM(BlueprintType)
enum class ET66PartySize : uint8
{
	Solo UMETA(DisplayName = "Solo"),
	Duo UMETA(DisplayName = "Duo"),
	Trio UMETA(DisplayName = "Trio"),
	Quad UMETA(DisplayName = "Quad")
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
	Impossible UMETA(DisplayName = "Impossible")
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
 * Leaderboard type (Score vs Speed Run)
 */
UENUM(BlueprintType)
enum class ET66LeaderboardType : uint8
{
	Score UMETA(DisplayName = "Score"),
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

	/** Player display name (solo; also used when reading from DataTable). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString PlayerName;

	/** For duos: 2 names; for trios: 3 names; for solo: 1 name. Empty means use PlayerName. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	TArray<FString> PlayerNames;

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

	/** Stage reached (1-23) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 StageReached = 0;

	/** Whether this is the local player's entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	bool bIsLocalPlayer = false;

	/** Backend entry UUID (used to fetch run summary from backend). Empty for local-only entries. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString EntryId;

	/** True if the backend reports a run summary exists for this entry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	bool bHasRunSummary = false;

	/** Steam avatar URL from the backend (full-size 184x184). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString AvatarUrl;

	FLeaderboardEntry()
		: Rank(0)
		, Score(0)
		, TimeSeconds(0.0f)
		, HeroID(NAME_None)
		, PartySize(ET66PartySize::Solo)
		, Difficulty(ET66Difficulty::Easy)
		, StageReached(0)
		, bIsLocalPlayer(false)
		, bHasRunSummary(false)
	{}
};

/**
 * Leaderboard tuning row: 10th-place Score target.
 * Import `Leaderboard_ScoreTargets.csv` into a DataTable using this row struct.
 */
USTRUCT(BlueprintType)
struct T66_API FT66LeaderboardScoreTargetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66PartySize PartySize = ET66PartySize::Solo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int64 TargetScore10 = 0;

	FT66LeaderboardScoreTargetRow()
		: Difficulty(ET66Difficulty::Easy)
		, PartySize(ET66PartySize::Solo)
		, TargetScore10(0)
	{}
};

/**
 * Leaderboard tuning row: 10th-place "Speed Run" target time per stage.
 * Import `Leaderboard_SpeedrunTargets.csv` into a DataTable using this row struct.
 */
USTRUCT(BlueprintType)
struct T66_API FT66LeaderboardSpeedRunTargetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 Stage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	float TargetTime10Seconds = 0.f;

	FT66LeaderboardSpeedRunTargetRow()
		: Difficulty(ET66Difficulty::Easy)
		, Stage(1)
		, TargetTime10Seconds(0.f)
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

UENUM(BlueprintType)
enum class ET66AchievementCategory : uint8
{
	Standard UMETA(DisplayName = "Standard"),
	Special UMETA(DisplayName = "Special"),
};

UENUM(BlueprintType)
enum class ET66AchievementRewardType : uint8
{
	ChadCoupons UMETA(DisplayName = "Chad Coupons"),
	SkinUnlock UMETA(DisplayName = "Skin Unlock"),
};

UENUM(BlueprintType)
enum class ET66AchievementRewardEntityType : uint8
{
	None UMETA(DisplayName = "None"),
	Hero UMETA(DisplayName = "Hero"),
	Companion UMETA(DisplayName = "Companion"),
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

	/** Achievement category in the browser. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	ET66AchievementCategory Category = ET66AchievementCategory::Standard;

	/** Reward kind for this achievement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	ET66AchievementRewardType RewardType = ET66AchievementRewardType::ChadCoupons;

	/** Chad Coupons reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 RewardCoins = 5;

	/** Reward target entity type when RewardType == SkinUnlock. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	ET66AchievementRewardEntityType RewardEntityType = ET66AchievementRewardEntityType::None;

	/** Reward target entity ID when RewardType == SkinUnlock. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName RewardEntityID = NAME_None;

	/** Reward skin ID when RewardType == SkinUnlock. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName RewardSkinID = NAME_None;

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
		, Category(ET66AchievementCategory::Standard)
		, RewardType(ET66AchievementRewardType::ChadCoupons)
		, RewardCoins(5)
		, RewardEntityType(ET66AchievementRewardEntityType::None)
		, RewardEntityID(NAME_None)
		, RewardSkinID(NAME_None)
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

	/** Cost in Chad Coupons to purchase */
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

	/** Optional portrait override for this skin. Falls back to the owning entity portrait when unset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> Portrait;

	/** Optional selection/info portrait override for this skin. Falls back to Portrait or the owning entity selection portrait when unset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> SelectionPortrait;

	FSkinData()
		: SkinID(NAME_None)
		, OwnerID(NAME_None)
		, CoinCost(0)
		, bIsOwned(false)
		, bIsEquipped(false)
		, bIsDefault(false)
	{}
};

/** Row for DT_Props: defines one scatter-able prop type. */
USTRUCT(BlueprintType)
struct T66_API FT66PropRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
	int32 CountMin = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
	int32 CountMax = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
	FVector ScaleMin = FVector(0.8f, 0.8f, 0.8f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
	FVector ScaleMax = FVector(1.2f, 1.2f, 1.2f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
	bool bRandomYawRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
	float MinDistanceBetween = 600.f;

	/** Optional world-space placement adjustment applied after ground snapping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
	float PlacementZOffset = 0.f;
};
