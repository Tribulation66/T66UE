// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RunStateSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardPacingUtils.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66SkillRatingSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Gameplay/T66VendorBoss.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Subsystems/SubsystemCollection.h"

namespace
{
	static const FName T66ArthurHeroID(TEXT("Hero_1"));
	static const FName T66GamblersTokenItemID(TEXT("Item_GamblersToken"));
	static const FName T66MaxHeroStatsRunModifierID(TEXT("Mod_MaxHeroStats"));
	static constexpr int32 T66ArthurTestStatBoost = 50;

	static int32 T66_GetDefaultInventoryRollSeed()
	{
		return static_cast<int32>(FPlatformTime::Cycles());
	}

	static void T66SpawnLevelUpBifrost(UWorld* World, const FVector& Location)
	{
		(void)World;
		(void)Location;
		// Disabled for now. This cosmetic Niagara spawn is causing packaged-runtime
		// crash paths on repeated boss clears/stage transitions.
	}

	static void T66KillEnemiesNearPoint(UWorld* World, const FVector& Origin, const float Radius)
	{
		if (!World || Radius <= 0.f)
		{
			return;
		}

		UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
		if (!Registry)
		{
			return;
		}

		const TArray<TWeakObjectPtr<AT66EnemyBase>> Enemies = Registry->GetEnemies();
		const float RadiusSq = FMath::Square(Radius);
		for (const TWeakObjectPtr<AT66EnemyBase>& WeakEnemy : Enemies)
		{
			AT66EnemyBase* Enemy = WeakEnemy.Get();
			if (!Enemy || Enemy->IsPendingKillPending())
			{
				continue;
			}

			if (FVector::DistSquared(Enemy->GetActorLocation(), Origin) > RadiusSq)
			{
				continue;
			}

			Enemy->TakeDamageFromEnvironment(1000000, nullptr, FName(TEXT("LevelUpBurst")));
		}
	}

	static int32 T66_CombineInventorySeed(const FT66InventorySlot& Slot, const int32 SeedSalt)
	{
		uint32 Seed = static_cast<uint32>(Slot.RollSeed != 0 ? Slot.RollSeed : GetTypeHash(Slot.ItemTemplateID));
		Seed = HashCombine(Seed, GetTypeHash(static_cast<uint8>(Slot.Rarity)));
		Seed = HashCombine(Seed, GetTypeHash(Slot.Line1RolledValue));
		Seed = HashCombine(Seed, GetTypeHash(Slot.GetSecondaryStatBonusValue()));
		Seed = HashCombine(Seed, GetTypeHash(SeedSalt));
		return static_cast<int32>(Seed & 0x7fffffff);
	}

	static int32 T66_RollScaledTenthsDeterministic(const int32 BaseGainTenths, const float MinFactor, const float MaxFactor, const int32 Seed)
	{
		if (BaseGainTenths <= 0)
		{
			return 0;
		}

		FRandomStream Stream(Seed == 0 ? 1 : Seed);
		const float Alpha = Stream.GetFraction();
		const float Factor = FMath::Lerp(MinFactor, MaxFactor, Alpha);
		return FMath::Max(1, FMath::CeilToInt(static_cast<float>(BaseGainTenths) * Factor));
	}

	static const TArray<ET66SecondaryStatType>& T66_GetDamageSecondaryTypes()
	{
		static const TArray<ET66SecondaryStatType> Types =
		{
			ET66SecondaryStatType::PierceDamage,
			ET66SecondaryStatType::BounceDamage,
			ET66SecondaryStatType::AoeDamage,
			ET66SecondaryStatType::DotDamage
		};
		return Types;
	}

	static const TArray<ET66SecondaryStatType>& T66_GetAttackSpeedSecondaryTypes()
	{
		static const TArray<ET66SecondaryStatType> Types =
		{
			ET66SecondaryStatType::PierceSpeed,
			ET66SecondaryStatType::BounceSpeed,
			ET66SecondaryStatType::AoeSpeed,
			ET66SecondaryStatType::DotSpeed
		};
		return Types;
	}

	static const TArray<ET66SecondaryStatType>& T66_GetAttackScaleSecondaryTypes()
	{
		static const TArray<ET66SecondaryStatType> Types =
		{
			ET66SecondaryStatType::PierceScale,
			ET66SecondaryStatType::BounceScale,
			ET66SecondaryStatType::AoeScale,
			ET66SecondaryStatType::DotScale
		};
		return Types;
	}

	static const TArray<ET66SecondaryStatType>& T66_GetAccuracySecondaryTypes()
	{
		static const TArray<ET66SecondaryStatType> Types =
		{
			ET66SecondaryStatType::CritDamage,
			ET66SecondaryStatType::CritChance,
			ET66SecondaryStatType::AttackRange,
			ET66SecondaryStatType::Accuracy
		};
		return Types;
	}

	static const TArray<ET66SecondaryStatType>& T66_GetArmorSecondaryTypes()
	{
		static const TArray<ET66SecondaryStatType> Types =
		{
			ET66SecondaryStatType::Taunt,
			ET66SecondaryStatType::ReflectDamage,
			ET66SecondaryStatType::HpRegen,
			ET66SecondaryStatType::Crush,
			ET66SecondaryStatType::DamageReduction
		};
		return Types;
	}

	static const TArray<ET66SecondaryStatType>& T66_GetEvasionSecondaryTypes()
	{
		static const TArray<ET66SecondaryStatType> Types =
		{
			ET66SecondaryStatType::Invisibility,
			ET66SecondaryStatType::CounterAttack,
			ET66SecondaryStatType::LifeSteal,
			ET66SecondaryStatType::Assassinate,
			ET66SecondaryStatType::EvasionChance
		};
		return Types;
	}

	static const TArray<ET66SecondaryStatType>& T66_GetLuckSecondaryTypes()
	{
		static const TArray<ET66SecondaryStatType> Types =
		{
			ET66SecondaryStatType::SpinWheel,
			ET66SecondaryStatType::TreasureChest,
			ET66SecondaryStatType::Cheating,
			ET66SecondaryStatType::Stealing,
			ET66SecondaryStatType::LootCrate,
			ET66SecondaryStatType::Alchemy
		};
		return Types;
	}

	static const TArray<ET66SecondaryStatType>& T66_GetSecondaryTypesForPrimary(const ET66HeroStatType PrimaryStatType)
	{
		switch (PrimaryStatType)
		{
		case ET66HeroStatType::Damage:      return T66_GetDamageSecondaryTypes();
		case ET66HeroStatType::AttackSpeed: return T66_GetAttackSpeedSecondaryTypes();
		case ET66HeroStatType::AttackScale: return T66_GetAttackScaleSecondaryTypes();
		case ET66HeroStatType::Accuracy:    return T66_GetAccuracySecondaryTypes();
		case ET66HeroStatType::Armor:       return T66_GetArmorSecondaryTypes();
		case ET66HeroStatType::Evasion:     return T66_GetEvasionSecondaryTypes();
		case ET66HeroStatType::Luck:        return T66_GetLuckSecondaryTypes();
		default:
		{
			static const TArray<ET66SecondaryStatType> EmptyTypes;
			return EmptyTypes;
		}
		}
	}

	static ET66SecondaryStatType T66_GetHeroMainAttackSecondaryType(const ET66HeroStatType PrimaryStatType, const ET66AttackCategory AttackCategory)
	{
		switch (PrimaryStatType)
		{
		case ET66HeroStatType::Damage:
			switch (AttackCategory)
			{
			case ET66AttackCategory::Pierce: return ET66SecondaryStatType::PierceDamage;
			case ET66AttackCategory::Bounce: return ET66SecondaryStatType::BounceDamage;
			case ET66AttackCategory::AOE:    return ET66SecondaryStatType::AoeDamage;
			case ET66AttackCategory::DOT:    return ET66SecondaryStatType::DotDamage;
			default:                         return ET66SecondaryStatType::PierceDamage;
			}
		case ET66HeroStatType::AttackSpeed:
			switch (AttackCategory)
			{
			case ET66AttackCategory::Pierce: return ET66SecondaryStatType::PierceSpeed;
			case ET66AttackCategory::Bounce: return ET66SecondaryStatType::BounceSpeed;
			case ET66AttackCategory::AOE:    return ET66SecondaryStatType::AoeSpeed;
			case ET66AttackCategory::DOT:    return ET66SecondaryStatType::DotSpeed;
			default:                         return ET66SecondaryStatType::PierceSpeed;
			}
		case ET66HeroStatType::AttackScale:
			switch (AttackCategory)
			{
			case ET66AttackCategory::Pierce: return ET66SecondaryStatType::PierceScale;
			case ET66AttackCategory::Bounce: return ET66SecondaryStatType::BounceScale;
			case ET66AttackCategory::AOE:    return ET66SecondaryStatType::AoeScale;
			case ET66AttackCategory::DOT:    return ET66SecondaryStatType::DotScale;
			default:                         return ET66SecondaryStatType::PierceScale;
			}
		default:
			return ET66SecondaryStatType::None;
		}
	}

	static void T66_AccumulatePressureWindowSummary(
		FT66AntiCheatPressureWindowSummary& Summary,
		int32 HitChecks,
		int32 Dodges,
		int32 DamageApplied,
		float ExpectedDodges)
	{
		if (HitChecks <= 0)
		{
			return;
		}

		Summary.ActiveWindows = FMath::Clamp(Summary.ActiveWindows + 1, 0, 1000000);
		Summary.TotalHitChecks = FMath::Clamp(Summary.TotalHitChecks + HitChecks, 0, 1000000);
		Summary.TotalDodges = FMath::Clamp(Summary.TotalDodges + Dodges, 0, 1000000);
		Summary.TotalDamageApplied = FMath::Clamp(Summary.TotalDamageApplied + DamageApplied, 0, 1000000);
		Summary.TotalExpectedDodges = FMath::Clamp(Summary.TotalExpectedDodges + FMath::Max(0.f, ExpectedDodges), 0.f, 1000000.f);
		Summary.MaxHitChecksInWindow = FMath::Max(Summary.MaxHitChecksInWindow, HitChecks);
		Summary.MaxDodgesInWindow = FMath::Max(Summary.MaxDodgesInWindow, Dodges);
		Summary.MaxDamageAppliedInWindow = FMath::Max(Summary.MaxDamageAppliedInWindow, DamageApplied);
		Summary.MaxExpectedDodgesInWindow = FMath::Max(Summary.MaxExpectedDodgesInWindow, FMath::Max(0.f, ExpectedDodges));

		if (HitChecks >= 4)
		{
			Summary.PressuredWindows4Plus = FMath::Clamp(Summary.PressuredWindows4Plus + 1, 0, 1000000);
			if (DamageApplied <= 0)
			{
				Summary.ZeroDamageWindows4Plus = FMath::Clamp(Summary.ZeroDamageWindows4Plus + 1, 0, 1000000);
			}
		}

		if (HitChecks >= 8)
		{
			Summary.PressuredWindows8Plus = FMath::Clamp(Summary.PressuredWindows8Plus + 1, 0, 1000000);
			if (DamageApplied <= 0)
			{
				Summary.ZeroDamageWindows8Plus = FMath::Clamp(Summary.ZeroDamageWindows8Plus + 1, 0, 1000000);
			}
			if (DamageApplied <= 1)
			{
				Summary.NearPerfectWindows8Plus = FMath::Clamp(Summary.NearPerfectWindows8Plus + 1, 0, 1000000);
			}
		}
	}

	static bool T66_IsGamblersTokenItem(const FName ItemID)
	{
		return ItemID == T66GamblersTokenItemID;
	}

	static int32 T66_ClampGamblersTokenLevel(const int32 Level)
	{
		return FMath::Clamp(Level, 0, UT66RunStateSubsystem::MaxGamblersTokenLevel);
	}

	static bool T66_IsAlchemyEligibleSlot(const FT66InventorySlot& Slot, const UT66GameInstance* GI)
	{
		if (!Slot.IsValid() || Slot.Rarity == ET66ItemRarity::White || T66_IsGamblersTokenItem(Slot.ItemTemplateID))
		{
			return false;
		}

		if (!GI)
		{
			return true;
		}

		FItemData ItemData;
		return const_cast<UT66GameInstance*>(GI)->GetItemData(Slot.ItemTemplateID, ItemData)
			&& ItemData.SecondaryStatType != ET66SecondaryStatType::GamblerToken;
	}

	static bool T66_IsAlchemyMatch(const FT66InventorySlot& A, const FT66InventorySlot& B)
	{
		return A.IsValid() && B.IsValid() && A.ItemTemplateID == B.ItemTemplateID && A.Rarity == B.Rarity;
	}

	static int32 T66_MapBlessingRollToWhiteRange(const FT66InventorySlot& Slot)
	{
		(void)Slot;
		return FItemData::GetFlatSecondaryStatBonus(ET66ItemRarity::White);
	}

	static TArray<int32> T66_GatherAlchemySourceIndices(const TArray<FT66InventorySlot>& InventorySlots, const int32 TargetIndex)
	{
		TArray<int32> SourceIndices;
		if (!InventorySlots.IsValidIndex(TargetIndex))
		{
			return SourceIndices;
		}

		const FT66InventorySlot& TargetSlot = InventorySlots[TargetIndex];
		if (!TargetSlot.IsValid())
		{
			return SourceIndices;
		}

		SourceIndices.Add(TargetIndex);

		TArray<int32> MatchingOthers;
		for (int32 Index = 0; Index < InventorySlots.Num(); ++Index)
		{
			if (Index == TargetIndex || !T66_IsAlchemyMatch(TargetSlot, InventorySlots[Index]))
			{
				continue;
			}

			MatchingOthers.Add(Index);
		}

		MatchingOthers.Sort([&InventorySlots](const int32 A, const int32 B)
		{
			const FT66InventorySlot& SlotA = InventorySlots[A];
			const FT66InventorySlot& SlotB = InventorySlots[B];
			if (SlotA.Line1RolledValue != SlotB.Line1RolledValue)
			{
				return SlotA.Line1RolledValue > SlotB.Line1RolledValue;
			}

			const float Line2A = SlotA.GetLine2Multiplier();
			const float Line2B = SlotB.GetLine2Multiplier();
			if (!FMath::IsNearlyEqual(Line2A, Line2B))
			{
				return Line2A > Line2B;
			}

			return A < B;
		});

		for (const int32 Index : MatchingOthers)
		{
			if (SourceIndices.Num() >= UT66RunStateSubsystem::AlchemyCopiesRequired)
			{
				break;
			}

			SourceIndices.Add(Index);
		}

		return SourceIndices;
	}

	static FT66InventorySlot T66_BuildAlchemyUpgradeSlot(const FT66InventorySlot& TargetSlot, const TArray<FT66InventorySlot>& SourceSlots)
	{
		FT66InventorySlot UpgradedSlot = TargetSlot;
		UpgradedSlot.Rarity = UT66RunStateSubsystem::GetNextItemRarity(TargetSlot.Rarity);

		int32 SourceSeed = GetTypeHash(TargetSlot.ItemTemplateID);
		for (const FT66InventorySlot& SourceSlot : SourceSlots)
		{
			SourceSeed = HashCombine(SourceSeed, GetTypeHash(SourceSlot.RollSeed != 0 ? SourceSlot.RollSeed : T66_GetDefaultInventoryRollSeed()));
		}

		UpgradedSlot.Line1RolledValue = FItemData::GetAlchemyFlatStatBonus(UpgradedSlot.Rarity);
		UpgradedSlot.SecondaryStatBonusOverride = FItemData::GetAlchemyFlatStatBonus(UpgradedSlot.Rarity);
		UpgradedSlot.Line2MultiplierOverride = FItemData::GetLine2RarityMultiplier(UpgradedSlot.Rarity);
		UpgradedSlot.RollSeed = SourceSeed;
		return UpgradedSlot;
	}

	static int32 T66_MapRollToRarityRange(const int32 Value, const ET66ItemRarity SourceRarity, const ET66ItemRarity TargetRarity)
	{
		int32 SourceMin = 1;
		int32 SourceMax = 3;
		FItemData::GetLine1RollRange(SourceRarity, SourceMin, SourceMax);

		int32 TargetMin = 1;
		int32 TargetMax = 3;
		FItemData::GetLine1RollRange(TargetRarity, TargetMin, TargetMax);

		if (SourceMax <= SourceMin)
		{
			return TargetMax;
		}

		const float Alpha = FMath::Clamp(
			static_cast<float>(Value - SourceMin) / static_cast<float>(SourceMax - SourceMin),
			0.f,
			1.f);
		return FMath::RoundToInt(FMath::Lerp(static_cast<float>(TargetMin), static_cast<float>(TargetMax), Alpha));
	}

	static void T66_ApplyLuckyAlchemyBonus(FT66InventorySlot& Slot)
	{
		if (!Slot.IsValid() || Slot.Rarity == ET66ItemRarity::White)
		{
			return;
		}

		const ET66ItemRarity PreviousRarity = Slot.Rarity;
		Slot.Rarity = UT66RunStateSubsystem::GetNextItemRarity(Slot.Rarity);
		Slot.Line1RolledValue = FMath::Max(
			Slot.Line1RolledValue,
			FItemData::GetAlchemyFlatStatBonus(Slot.Rarity));
		Slot.SecondaryStatBonusOverride = FMath::Max(
			Slot.SecondaryStatBonusOverride,
			FItemData::GetAlchemyFlatStatBonus(Slot.Rarity));
		Slot.Line2MultiplierOverride = FItemData::GetLine2RarityMultiplier(Slot.Rarity);
		Slot.RollSeed = HashCombine(GetTypeHash(PreviousRarity), GetTypeHash(Slot.RollSeed != 0 ? Slot.RollSeed : T66_GetDefaultInventoryRollSeed()));
	}

	static float T66_GetSellFractionForTokenLevel(const int32 TokenLevel)
	{
		return FMath::Clamp(0.40f + 0.10f * static_cast<float>(T66_ClampGamblersTokenLevel(TokenLevel)), 0.40f, 1.00f);
	}

	static float T66_RarityTo01(ET66Rarity R)
	{
		// Linear mapping across tiers (Black worst -> White best).
		switch (R)
		{
		case ET66Rarity::Black:  return 0.f;
		case ET66Rarity::Red:    return 1.f / 3.f;
		case ET66Rarity::Yellow: return 2.f / 3.f;
		case ET66Rarity::White:  return 1.f;
		default: return 0.f;
		}
	}

	template<typename TMapType>
	static float T66_AverageCategories01(const TMapType& ByCategory)
	{
		if (ByCategory.Num() <= 0)
		{
			return 0.5f; // neutral fallback if a run had no rolls recorded
		}

		double Sum = 0.0;
		int32 N = 0;
		for (const auto& Kvp : ByCategory)
		{
			const auto& Acc = Kvp.Value;
			if (Acc.Count <= 0) continue;
			Sum += static_cast<double>(Acc.Avg01());
			++N;
		}
		return (N > 0) ? static_cast<float>(Sum / static_cast<double>(N)) : 0.5f;
	}

	static void T66_RecomputeBossAggregate(TArray<FT66BossPartSnapshot>& BossParts, int32& OutBossMaxHP, int32& OutBossCurrentHP)
	{
		OutBossMaxHP = 0;
		OutBossCurrentHP = 0;

		for (FT66BossPartSnapshot& Part : BossParts)
		{
			Part.MaxHP = FMath::Max(1, Part.MaxHP);
			Part.CurrentHP = FMath::Clamp(Part.CurrentHP, 0, Part.MaxHP);
			OutBossMaxHP += Part.MaxHP;
			OutBossCurrentHP += Part.CurrentHP;
		}

		if (OutBossMaxHP <= 0)
		{
			OutBossMaxHP = 100;
			OutBossCurrentHP = 0;
		}
	}
}

void UT66RunStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ResetHeartSlotTiers();
	SyncMaxHPToHeartTiers();
	CurrentHP = FMath::Clamp(CurrentHP, 0.f, MaxHP);

	Collection.InitializeDependency<UT66IdolManagerSubsystem>();
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		IdolManager->IdolStateChanged.RemoveDynamic(this, &UT66RunStateSubsystem::HandleIdolStateChanged);
		IdolManager->IdolStateChanged.AddDynamic(this, &UT66RunStateSubsystem::HandleIdolStateChanged);
	}
}

void UT66RunStateSubsystem::Deinitialize()
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		IdolManager->IdolStateChanged.RemoveDynamic(this, &UT66RunStateSubsystem::HandleIdolStateChanged);
	}

	Super::Deinitialize();
}

void UT66RunStateSubsystem::HandleIdolStateChanged()
{
	IdolsChanged.Broadcast();
}

UT66IdolManagerSubsystem* UT66RunStateSubsystem::GetIdolManager() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr;
}

void UT66RunStateSubsystem::ResetLuckRatingTracking()
{
	LuckQuantityByCategory.Reset();
	LuckQualityByCategory.Reset();
}

void UT66RunStateSubsystem::RecordLuckQuantityRoll(FName Category, int32 RolledValue, int32 MinValue, int32 MaxValue, int32 RunDrawIndex, int32 PreDrawSeed)
{
	const int32 MinV = FMath::Min(MinValue, MaxValue);
	const int32 MaxV = FMath::Max(MinValue, MaxValue);
	float Score01 = 0.5f;
	if (MaxV <= MinV)
	{
		Score01 = 1.f;
	}
	else
	{
		Score01 = FMath::Clamp(static_cast<float>(RolledValue - MinV) / static_cast<float>(MaxV - MinV), 0.f, 1.f);
	}

	LuckQuantityByCategory.FindOrAdd(Category).Add01(Score01);
	RecordAntiCheatLuckEvent(ET66AntiCheatLuckEventType::QuantityRoll, Category, Score01, RolledValue, MinV, MaxV, RunDrawIndex, PreDrawSeed);
}

void UT66RunStateSubsystem::RecordLuckQuantityFloatRollRounded(FName Category, int32 RolledValue, int32 MinValue, int32 MaxValue, float ReplayMinValue, float ReplayMaxValue, int32 RunDrawIndex, int32 PreDrawSeed)
{
	const int32 MinV = FMath::Min(MinValue, MaxValue);
	const int32 MaxV = FMath::Max(MinValue, MaxValue);
	float Score01 = 0.5f;
	if (MaxV <= MinV)
	{
		Score01 = 1.f;
	}
	else
	{
		Score01 = FMath::Clamp(static_cast<float>(RolledValue - MinV) / static_cast<float>(MaxV - MinV), 0.f, 1.f);
	}

	LuckQuantityByCategory.FindOrAdd(Category).Add01(Score01);
	FT66FloatRange ReplayRange;
	ReplayRange.Min = ReplayMinValue;
	ReplayRange.Max = ReplayMaxValue;
	RecordAntiCheatLuckEvent(
		ET66AntiCheatLuckEventType::QuantityRoll,
		Category,
		Score01,
		RolledValue,
		MinV,
		MaxV,
		RunDrawIndex,
		PreDrawSeed,
		-1.f,
		nullptr,
		&ReplayRange);
}

void UT66RunStateSubsystem::RecordLuckQuantityBool(FName Category, bool bSucceeded, float ExpectedChance01, int32 RunDrawIndex, int32 PreDrawSeed)
{
	LuckQuantityByCategory.FindOrAdd(Category).Add01(bSucceeded ? 1.f : 0.f);
	RecordAntiCheatLuckEvent(ET66AntiCheatLuckEventType::QuantityBool, Category, bSucceeded ? 1.f : 0.f, bSucceeded ? 1 : 0, 0, 1, RunDrawIndex, PreDrawSeed, ExpectedChance01);
}

void UT66RunStateSubsystem::RecordLuckQualityRarity(FName Category, ET66Rarity Rarity, int32 RunDrawIndex, int32 PreDrawSeed, const FT66RarityWeights* ReplayWeights)
{
	const float Score01 = T66_RarityTo01(Rarity);
	LuckQualityByCategory.FindOrAdd(Category).Add01(Score01);
	RecordAntiCheatLuckEvent(ET66AntiCheatLuckEventType::QualityRarity, Category, Score01, static_cast<int32>(Rarity), 0, 3, RunDrawIndex, PreDrawSeed, -1.f, ReplayWeights);
}

float UT66RunStateSubsystem::ComputeLuckRatingQuantity01() const
{
	return T66_AverageCategories01(LuckQuantityByCategory);
}

float UT66RunStateSubsystem::ComputeLuckRatingQuality01() const
{
	return T66_AverageCategories01(LuckQualityByCategory);
}

int32 UT66RunStateSubsystem::GetLuckRatingQuantity0To100() const
{
	return FMath::Clamp(FMath::RoundToInt(ComputeLuckRatingQuantity01() * 100.f), 0, 100);
}

int32 UT66RunStateSubsystem::GetLuckRatingQuality0To100() const
{
	return FMath::Clamp(FMath::RoundToInt(ComputeLuckRatingQuality01() * 100.f), 0, 100);
}

int32 UT66RunStateSubsystem::GetLuckRating0To100() const
{
	const bool bHasQuantity = (LuckQuantityByCategory.Num() > 0);
	const bool bHasQuality = (LuckQualityByCategory.Num() > 0);

	float Rating01 = 0.5f;
	if (bHasQuantity && bHasQuality)
	{
		Rating01 = 0.5f * (ComputeLuckRatingQuantity01() + ComputeLuckRatingQuality01());
	}
	else if (bHasQuantity)
	{
		Rating01 = ComputeLuckRatingQuantity01();
	}
	else if (bHasQuality)
	{
		Rating01 = ComputeLuckRatingQuality01();
	}

	return FMath::Clamp(FMath::RoundToInt(Rating01 * 100.f), 0, 100);
}

int32 UT66RunStateSubsystem::GetLuckQuantitySampleCount() const
{
	int32 TotalCount = 0;
	for (const TPair<FName, FT66LuckAccumulator>& Pair : LuckQuantityByCategory)
	{
		TotalCount = FMath::Clamp(TotalCount + FMath::Max(0, Pair.Value.Count), 0, 1000000);
	}
	return TotalCount;
}

int32 UT66RunStateSubsystem::GetLuckQualitySampleCount() const
{
	int32 TotalCount = 0;
	for (const TPair<FName, FT66LuckAccumulator>& Pair : LuckQualityByCategory)
	{
		TotalCount = FMath::Clamp(TotalCount + FMath::Max(0, Pair.Value.Count), 0, 1000000);
	}
	return TotalCount;
}

void UT66RunStateSubsystem::GetLuckQuantityAccumulators(TArray<FT66SavedLuckAccumulator>& OutAccumulators) const
{
	OutAccumulators.Reset();

	TArray<FName> Categories;
	LuckQuantityByCategory.GenerateKeyArray(Categories);
	Categories.Sort([](const FName& A, const FName& B)
	{
		return A.ToString() < B.ToString();
	});

	for (const FName& Category : Categories)
	{
		if (const FT66LuckAccumulator* Accumulator = LuckQuantityByCategory.Find(Category))
		{
			FT66SavedLuckAccumulator& Saved = OutAccumulators.AddDefaulted_GetRef();
			Saved.Category = Category;
			Saved.Sum01 = static_cast<float>(Accumulator->Sum01);
			Saved.Count = FMath::Max(0, Accumulator->Count);
		}
	}
}

void UT66RunStateSubsystem::GetLuckQualityAccumulators(TArray<FT66SavedLuckAccumulator>& OutAccumulators) const
{
	OutAccumulators.Reset();

	TArray<FName> Categories;
	LuckQualityByCategory.GenerateKeyArray(Categories);
	Categories.Sort([](const FName& A, const FName& B)
	{
		return A.ToString() < B.ToString();
	});

	for (const FName& Category : Categories)
	{
		if (const FT66LuckAccumulator* Accumulator = LuckQualityByCategory.Find(Category))
		{
			FT66SavedLuckAccumulator& Saved = OutAccumulators.AddDefaulted_GetRef();
			Saved.Category = Category;
			Saved.Sum01 = static_cast<float>(Accumulator->Sum01);
			Saved.Count = FMath::Max(0, Accumulator->Count);
		}
	}
}

void UT66RunStateSubsystem::GetAntiCheatLuckEvents(TArray<FT66AntiCheatLuckEvent>& OutEvents) const
{
	OutEvents = AntiCheatLuckEvents;
}

void UT66RunStateSubsystem::GetAntiCheatHitCheckEvents(TArray<FT66AntiCheatHitCheckEvent>& OutEvents) const
{
	OutEvents = AntiCheatHitCheckEvents;
}

void UT66RunStateSubsystem::GetAntiCheatEvasionBuckets(TArray<FT66AntiCheatEvasionBucketSummary>& OutBuckets) const
{
	OutBuckets = AntiCheatEvasionBuckets;
}

void UT66RunStateSubsystem::GetAntiCheatPressureWindowSummary(FT66AntiCheatPressureWindowSummary& OutSummary) const
{
	OutSummary = BuildAntiCheatPressureWindowSummarySnapshot();
}

void UT66RunStateSubsystem::GetAntiCheatGamblerSummaries(TArray<FT66AntiCheatGamblerGameSummary>& OutSummaries) const
{
	OutSummaries = AntiCheatGamblerSummaries;
}

void UT66RunStateSubsystem::GetAntiCheatGamblerEvents(TArray<FT66AntiCheatGamblerEvent>& OutEvents) const
{
	OutEvents = AntiCheatGamblerEvents;
}

float UT66RunStateSubsystem::GetRunElapsedSecondsForAntiCheatEvent() const
{
	if (bRunEnded)
	{
		return FMath::Max(0.f, FinalRunElapsedSeconds);
	}

	if (bSpeedRunStarted && GetWorld())
	{
		return FMath::Max(0.f, static_cast<float>(GetWorld()->GetTimeSeconds()) - SpeedRunStartWorldSeconds);
	}

	return FMath::Max(0.f, SpeedRunElapsedSeconds);
}

void UT66RunStateSubsystem::InitializeAntiCheatEvasionBuckets()
{
	if (AntiCheatEvasionBuckets.Num() == AntiCheatEvasionBucketCount)
	{
		return;
	}

	AntiCheatEvasionBuckets.Reset();
	for (int32 BucketIndex = 0; BucketIndex < AntiCheatEvasionBucketCount; ++BucketIndex)
	{
		FT66AntiCheatEvasionBucketSummary& Bucket = AntiCheatEvasionBuckets.AddDefaulted_GetRef();
		Bucket.BucketIndex = BucketIndex;
		Bucket.MinChance01 = GetAntiCheatEvasionBucketMinChance01(BucketIndex);
		Bucket.MaxChance01 = GetAntiCheatEvasionBucketMaxChance01(BucketIndex);
	}
}

int32 UT66RunStateSubsystem::GetAntiCheatEvasionBucketIndex(float EvasionChance01)
{
	const float Clamped = FMath::Clamp(EvasionChance01, 0.f, 1.f);
	if (Clamped < 0.15f) return 0;
	if (Clamped < 0.30f) return 1;
	if (Clamped < 0.45f) return 2;
	if (Clamped < 0.60f) return 3;
	return 4;
}

float UT66RunStateSubsystem::GetAntiCheatEvasionBucketMinChance01(int32 BucketIndex)
{
	switch (BucketIndex)
	{
	case 1: return 0.15f;
	case 2: return 0.30f;
	case 3: return 0.45f;
	case 4: return 0.60f;
	default: return 0.f;
	}
}

float UT66RunStateSubsystem::GetAntiCheatEvasionBucketMaxChance01(int32 BucketIndex)
{
	switch (BucketIndex)
	{
	case 0: return 0.15f;
	case 1: return 0.30f;
	case 2: return 0.45f;
	case 3: return 0.60f;
	default: return 1.f;
	}
}

void UT66RunStateSubsystem::ResetAntiCheatPressureTracking()
{
	AntiCheatPressureWindowSummary = FT66AntiCheatPressureWindowSummary{};
	AntiCheatPressureWindowSummary.WindowSeconds = AntiCheatPressureWindowSeconds;
	AntiCheatCurrentPressureWindowIndex = INDEX_NONE;
	AntiCheatCurrentPressureHitChecks = 0;
	AntiCheatCurrentPressureDodges = 0;
	AntiCheatCurrentPressureDamageApplied = 0;
	AntiCheatCurrentPressureExpectedDodges = 0.f;
}

void UT66RunStateSubsystem::FinalizeCurrentAntiCheatPressureWindow()
{
	if (AntiCheatCurrentPressureHitChecks <= 0)
	{
		AntiCheatCurrentPressureWindowIndex = INDEX_NONE;
		AntiCheatCurrentPressureDodges = 0;
		AntiCheatCurrentPressureDamageApplied = 0;
		AntiCheatCurrentPressureExpectedDodges = 0.f;
		return;
	}

	T66_AccumulatePressureWindowSummary(
		AntiCheatPressureWindowSummary,
		AntiCheatCurrentPressureHitChecks,
		AntiCheatCurrentPressureDodges,
		AntiCheatCurrentPressureDamageApplied,
		AntiCheatCurrentPressureExpectedDodges);

	AntiCheatCurrentPressureWindowIndex = INDEX_NONE;
	AntiCheatCurrentPressureHitChecks = 0;
	AntiCheatCurrentPressureDodges = 0;
	AntiCheatCurrentPressureDamageApplied = 0;
	AntiCheatCurrentPressureExpectedDodges = 0.f;
}

void UT66RunStateSubsystem::RecordAntiCheatPressureHitCheck(float RunElapsedSeconds, float EvasionChance01, bool bDodged, bool bDamageApplied)
{
	const float ClampedTime = FMath::Max(0.f, RunElapsedSeconds);
	const int32 WindowIndex = FMath::Max(0, FMath::FloorToInt(ClampedTime / static_cast<float>(AntiCheatPressureWindowSeconds)));
	if (AntiCheatCurrentPressureWindowIndex != WindowIndex)
	{
		FinalizeCurrentAntiCheatPressureWindow();
		AntiCheatCurrentPressureWindowIndex = WindowIndex;
	}

	AntiCheatCurrentPressureHitChecks = FMath::Clamp(AntiCheatCurrentPressureHitChecks + 1, 0, 1000000);
	AntiCheatCurrentPressureExpectedDodges = FMath::Clamp(AntiCheatCurrentPressureExpectedDodges + FMath::Clamp(EvasionChance01, 0.f, 1.f), 0.f, 1000000.f);
	if (bDodged)
	{
		AntiCheatCurrentPressureDodges = FMath::Clamp(AntiCheatCurrentPressureDodges + 1, 0, 1000000);
	}
	if (bDamageApplied)
	{
		AntiCheatCurrentPressureDamageApplied = FMath::Clamp(AntiCheatCurrentPressureDamageApplied + 1, 0, 1000000);
	}
}

FT66AntiCheatPressureWindowSummary UT66RunStateSubsystem::BuildAntiCheatPressureWindowSummarySnapshot() const
{
	FT66AntiCheatPressureWindowSummary Summary = AntiCheatPressureWindowSummary;
	Summary.WindowSeconds = AntiCheatPressureWindowSeconds;
	if (AntiCheatCurrentPressureHitChecks > 0)
	{
		T66_AccumulatePressureWindowSummary(
			Summary,
			AntiCheatCurrentPressureHitChecks,
			AntiCheatCurrentPressureDodges,
			AntiCheatCurrentPressureDamageApplied,
			AntiCheatCurrentPressureExpectedDodges);
	}
	return Summary;
}

void UT66RunStateSubsystem::RecordAntiCheatLuckEvent(ET66AntiCheatLuckEventType EventType, FName Category, float Value01, int32 RawValue, int32 RawMin, int32 RawMax, int32 RunDrawIndex, int32 PreDrawSeed, float ExpectedChance01, const FT66RarityWeights* ReplayWeights, const FT66FloatRange* ReplayFloatRange)
{
	FT66AntiCheatLuckEvent& Event = AntiCheatLuckEvents.AddDefaulted_GetRef();
	Event.EventType = EventType;
	Event.Category = Category;
	Event.TimeSeconds = GetRunElapsedSecondsForAntiCheatEvent();
	Event.Value01 = FMath::Clamp(Value01, 0.f, 1.f);
	Event.RawValue = RawValue;
	Event.RawMin = RawMin;
	Event.RawMax = RawMax;
	Event.RunDrawIndex = RunDrawIndex;
	Event.PreDrawSeed = PreDrawSeed;
	Event.ExpectedChance01 = ExpectedChance01;
	Event.bHasRarityWeights = (ReplayWeights != nullptr);
	if (ReplayWeights)
	{
		Event.RarityWeights = *ReplayWeights;
	}
	Event.bHasFloatReplayRange = (ReplayFloatRange != nullptr);
	if (ReplayFloatRange)
	{
		Event.FloatReplayMin = ReplayFloatRange->Min;
		Event.FloatReplayMax = ReplayFloatRange->Max;
	}

	if (AntiCheatLuckEvents.Num() > MaxAntiCheatLuckEvents)
	{
		const int32 RemoveCount = AntiCheatLuckEvents.Num() - MaxAntiCheatLuckEvents;
		AntiCheatLuckEvents.RemoveAt(0, RemoveCount, EAllowShrinking::No);
		bAntiCheatLuckEventsTruncated = true;
	}
}

void UT66RunStateSubsystem::RecordAntiCheatHitCheckEvent(float EvasionChance01, bool bDodged, bool bDamageApplied)
{
	FT66AntiCheatHitCheckEvent& Event = AntiCheatHitCheckEvents.AddDefaulted_GetRef();
	Event.TimeSeconds = GetRunElapsedSecondsForAntiCheatEvent();
	Event.EvasionChance01 = FMath::Clamp(EvasionChance01, 0.f, 1.f);
	Event.bDodged = bDodged;
	Event.bDamageApplied = bDamageApplied;
	InitializeAntiCheatEvasionBuckets();
	if (AntiCheatEvasionBuckets.Num() == AntiCheatEvasionBucketCount)
	{
		const int32 BucketIndex = GetAntiCheatEvasionBucketIndex(Event.EvasionChance01);
		if (AntiCheatEvasionBuckets.IsValidIndex(BucketIndex))
		{
			FT66AntiCheatEvasionBucketSummary& Bucket = AntiCheatEvasionBuckets[BucketIndex];
			Bucket.HitChecks = FMath::Clamp(Bucket.HitChecks + 1, 0, 1000000);
			Bucket.ExpectedDodges = FMath::Clamp(Bucket.ExpectedDodges + Event.EvasionChance01, 0.f, 1000000.f);
			if (Event.bDodged)
			{
				Bucket.Dodges = FMath::Clamp(Bucket.Dodges + 1, 0, 1000000);
			}
			if (Event.bDamageApplied)
			{
				Bucket.DamageApplied = FMath::Clamp(Bucket.DamageApplied + 1, 0, 1000000);
			}
		}
	}
	RecordAntiCheatPressureHitCheck(GetCurrentRunElapsedSeconds(), Event.EvasionChance01, Event.bDodged, Event.bDamageApplied);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SkillRatingSubsystem* Skill = GI->GetSubsystem<UT66SkillRatingSubsystem>())
		{
			Skill->NotifyHitCheck(Event.EvasionChance01, Event.bDodged, Event.bDamageApplied);
		}
	}

	if (AntiCheatHitCheckEvents.Num() > MaxAntiCheatHitCheckEvents)
	{
		const int32 RemoveCount = AntiCheatHitCheckEvents.Num() - MaxAntiCheatHitCheckEvents;
		AntiCheatHitCheckEvents.RemoveAt(0, RemoveCount, EAllowShrinking::No);
		bAntiCheatHitCheckEventsTruncated = true;
	}
}

FT66AntiCheatGamblerGameSummary& UT66RunStateSubsystem::FindOrAddAntiCheatGamblerSummary(ET66AntiCheatGamblerGameType GameType)
{
	for (FT66AntiCheatGamblerGameSummary& Summary : AntiCheatGamblerSummaries)
	{
		if (Summary.GameType == GameType)
		{
			return Summary;
		}
	}

	FT66AntiCheatGamblerGameSummary& Summary = AntiCheatGamblerSummaries.AddDefaulted_GetRef();
	Summary.GameType = GameType;
	return Summary;
}

void UT66RunStateSubsystem::RecordAntiCheatGamblerRound(
	ET66AntiCheatGamblerGameType GameType,
	int32 BetGold,
	int32 PayoutGold,
	bool bCheatAttempted,
	bool bCheatSucceeded,
	bool bWin,
	bool bDraw,
	int32 PlayerChoice,
	int32 OpponentChoice,
	int32 OutcomeValue,
	int32 OutcomeSecondaryValue,
	int32 SelectedMask,
	int32 ResolvedMask,
	int32 PathBits,
	int32 ShufflePreDrawSeed,
	int32 ShuffleStartDrawIndex,
	int32 OutcomePreDrawSeed,
	int32 OutcomeDrawIndex,
	float OutcomeExpectedChance01,
	const FString& ActionSequence)
{
	FT66AntiCheatGamblerGameSummary& Summary = FindOrAddAntiCheatGamblerSummary(GameType);
	Summary.Rounds = FMath::Clamp(Summary.Rounds + 1, 0, 1000000);
	Summary.TotalBetGold = FMath::Clamp(Summary.TotalBetGold + FMath::Max(0, BetGold), 0, INT32_MAX);
	Summary.TotalPayoutGold = FMath::Clamp(Summary.TotalPayoutGold + FMath::Max(0, PayoutGold), 0, INT32_MAX);
	if (bCheatAttempted)
	{
		Summary.CheatAttempts = FMath::Clamp(Summary.CheatAttempts + 1, 0, 1000000);
		if (bCheatSucceeded)
		{
			Summary.CheatSuccesses = FMath::Clamp(Summary.CheatSuccesses + 1, 0, 1000000);
		}
	}
	if (bDraw)
	{
		Summary.Draws = FMath::Clamp(Summary.Draws + 1, 0, 1000000);
	}
	else if (bWin)
	{
		Summary.Wins = FMath::Clamp(Summary.Wins + 1, 0, 1000000);
	}
	else
	{
		Summary.Losses = FMath::Clamp(Summary.Losses + 1, 0, 1000000);
	}

	FT66AntiCheatGamblerEvent& Event = AntiCheatGamblerEvents.AddDefaulted_GetRef();
	Event.GameType = GameType;
	Event.TimeSeconds = GetRunElapsedSecondsForAntiCheatEvent();
	Event.BetGold = FMath::Max(0, BetGold);
	Event.PayoutGold = FMath::Max(0, PayoutGold);
	Event.bCheatAttempted = bCheatAttempted;
	Event.bCheatSucceeded = bCheatSucceeded;
	Event.bWin = bWin;
	Event.bDraw = bDraw;
	Event.PlayerChoice = PlayerChoice;
	Event.OpponentChoice = OpponentChoice;
	Event.OutcomeValue = OutcomeValue;
	Event.OutcomeSecondaryValue = OutcomeSecondaryValue;
	Event.SelectedMask = SelectedMask;
	Event.ResolvedMask = ResolvedMask;
	Event.PathBits = PathBits;
	Event.ShufflePreDrawSeed = ShufflePreDrawSeed;
	Event.ShuffleStartDrawIndex = ShuffleStartDrawIndex;
	Event.OutcomePreDrawSeed = OutcomePreDrawSeed;
	Event.OutcomeDrawIndex = OutcomeDrawIndex;
	Event.OutcomeExpectedChance01 = OutcomeExpectedChance01;
	Event.ActionSequence = ActionSequence;

	if (AntiCheatGamblerEvents.Num() > MaxAntiCheatGamblerEvents)
	{
		const int32 RemoveCount = AntiCheatGamblerEvents.Num() - MaxAntiCheatGamblerEvents;
		AntiCheatGamblerEvents.RemoveAt(0, RemoveCount, EAllowShrinking::No);
		bAntiCheatGamblerEventsTruncated = true;
	}
}

const TArray<FName>& UT66RunStateSubsystem::GetAllIdolIDs()
{
	return UT66IdolManagerSubsystem::GetAllIdolIDs();
}

FLinearColor UT66RunStateSubsystem::GetIdolColor(FName IdolID)
{
	return UT66IdolManagerSubsystem::GetIdolColor(IdolID);
}

void UT66RunStateSubsystem::SetDOTDamageApplier(TFunction<void(AActor*, int32, FName)> InApplier)
{
	DOTDamageApplier = MoveTemp(InApplier);
}

void UT66RunStateSubsystem::ApplyDOT(AActor* Target, float Duration, float TickInterval, float DamagePerTick, FName SourceIdolID)
{
	if (!Target || Duration <= 0.f || TickInterval <= 0.f || DamagePerTick <= 0.f) return;

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return;

	FT66DotInstance Inst;
	Inst.RemainingDuration = Duration;
	Inst.TickInterval = FMath::Max(0.05f, TickInterval);
	Inst.DamagePerTick = DamagePerTick;
	Inst.NextTickTime = World->GetTimeSeconds() + Inst.TickInterval;
	Inst.SourceIdolID = SourceIdolID;

	FT66DotKey Key;
	Key.Target = TWeakObjectPtr<AActor>(Target);
	Key.SourceIdolID = SourceIdolID;
	ActiveDOTs.FindOrAdd(Key) = Inst;

	// Start timer if not already running.
	if (!DOTTimerHandle.IsValid())
	{
		World->GetTimerManager().SetTimer(DOTTimerHandle, this, &UT66RunStateSubsystem::TickDOT, DOTTickRateSeconds, true);
	}
}

void UT66RunStateSubsystem::TickDOT()
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	const double Now = World ? World->GetTimeSeconds() : 0.0;

	TArray<FT66DotKey> ToRemove;
	for (auto& Pair : ActiveDOTs)
	{
		if (!Pair.Key.Target.IsValid())
		{
			ToRemove.Add(Pair.Key);
			continue;
		}
		FT66DotInstance& Inst = Pair.Value;
		Inst.RemainingDuration -= DOTTickRateSeconds;
		if (Inst.RemainingDuration <= 0.f)
		{
			ToRemove.Add(Pair.Key);
			continue;
		}
		if (Now >= Inst.NextTickTime)
		{
			const int32 Damage = FMath::Max(1, FMath::RoundToInt(Inst.DamagePerTick));
			if (DOTDamageApplier)
			{
				DOTDamageApplier(Pair.Key.Target.Get(), Damage, Inst.SourceIdolID);
			}
			Inst.NextTickTime = Now + Inst.TickInterval;
		}
	}
	for (const FT66DotKey& Key : ToRemove)
	{
		ActiveDOTs.Remove(Key);
	}
	if (ActiveDOTs.Num() == 0 && World && DOTTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(DOTTimerHandle);
		DOTTimerHandle.Invalidate();
	}
}

const TArray<FName>& UT66RunStateSubsystem::GetEquippedIdols() const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->GetEquippedIdols();
	}

	static const TArray<FName> Empty;
	return Empty;
}

const TArray<uint8>& UT66RunStateSubsystem::GetEquippedIdolTierValues() const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->GetEquippedIdolTierValues();
	}

	static const TArray<uint8> Empty;
	return Empty;
}

bool UT66RunStateSubsystem::EquipIdolInSlot(int32 SlotIndex, FName IdolID)
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->EquipIdolInSlot(SlotIndex, IdolID);
	}
	return false;
}

bool UT66RunStateSubsystem::EquipIdolFirstEmpty(FName IdolID)
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->EquipIdolFirstEmpty(IdolID);
	}
	return false;
}

int32 UT66RunStateSubsystem::GetEquippedIdolLevelInSlot(int32 SlotIndex) const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->GetEquippedIdolLevelInSlot(SlotIndex);
	}
	return 0;
}

ET66ItemRarity UT66RunStateSubsystem::IdolTierValueToRarity(int32 TierValue)
{
	return UT66IdolManagerSubsystem::IdolTierValueToRarity(TierValue);
}

int32 UT66RunStateSubsystem::IdolRarityToTierValue(ET66ItemRarity Rarity)
{
	return UT66IdolManagerSubsystem::IdolRarityToTierValue(Rarity);
}

ET66ItemRarity UT66RunStateSubsystem::GetEquippedIdolRarityInSlot(int32 SlotIndex) const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->GetEquippedIdolRarityInSlot(SlotIndex);
	}
	return ET66ItemRarity::Black;
}

bool UT66RunStateSubsystem::SelectIdolFromAltar(FName IdolID)
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->SelectIdolFromAltar(IdolID);
	}
	return false;
}

void UT66RunStateSubsystem::ClearEquippedIdols()
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		IdolManager->ClearEquippedIdols();
	}
}

void UT66RunStateSubsystem::EnsureIdolStock()
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		IdolManager->EnsureIdolStock();
	}
}

void UT66RunStateSubsystem::RerollIdolStock()
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		IdolManager->RerollIdolStock();
	}
}

const TArray<FName>& UT66RunStateSubsystem::GetIdolStockIDs() const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->GetIdolStockIDs();
	}

	static const TArray<FName> Empty;
	return Empty;
}

int32 UT66RunStateSubsystem::GetIdolStockTierValue(int32 SlotIndex) const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->GetIdolStockTierValue(SlotIndex);
	}
	return 0;
}

ET66ItemRarity UT66RunStateSubsystem::GetIdolStockRarityInSlot(int32 SlotIndex) const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->GetIdolStockRarityInSlot(SlotIndex);
	}
	return ET66ItemRarity::Black;
}

bool UT66RunStateSubsystem::SelectIdolFromStock(int32 SlotIndex)
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->SelectIdolFromStock(SlotIndex);
	}
	return false;
}

bool UT66RunStateSubsystem::IsIdolStockSlotSelected(int32 SlotIndex) const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->IsIdolStockSlotSelected(SlotIndex);
	}
	return false;
}

void UT66RunStateSubsystem::TrimLogsIfNeeded()
{
	if (EventLog.Num() > MaxEventLogEntries)
	{
		const int32 RemoveCount = EventLog.Num() - MaxEventLogEntries;
		EventLog.RemoveAt(0, RemoveCount, EAllowShrinking::No);
	}
	if (StructuredEventLog.Num() > MaxStructuredEventLogEntries)
	{
		const int32 RemoveCount = StructuredEventLog.Num() - MaxStructuredEventLogEntries;
		StructuredEventLog.RemoveAt(0, RemoveCount, EAllowShrinking::No);
	}
}

int32 UT66RunStateSubsystem::GetDifficultyScalarTier() const
{
	const int32 Skulls = FMath::Max(0, DifficultySkulls);
	if (Skulls <= 0) return 0;
	// Tier 1 for skulls 1-5, Tier 2 for skulls 6-10, etc.
	return ((Skulls - 1) / 5) + 1;
}

float UT66RunStateSubsystem::GetDifficultyScalar() const
{
	// Per spec update: every skull increments scalar by +0.1.
	// Skull 1 => 1.1x ... Skull 5 => 1.5x, then color tier advances on skull 6.
	const int32 Skulls = FMath::Max(0, DifficultySkulls);
	return FMath::Clamp(1.0f + (0.1f * static_cast<float>(Skulls)), 1.0f, 99.0f);
}

void UT66RunStateSubsystem::IncreaseDifficultyTier()
{
	AddDifficultySkulls(1);
}

void UT66RunStateSubsystem::AddDifficultySkulls(int32 DeltaSkulls)
{
	if (DeltaSkulls == 0) return;
	DifficultySkulls = FMath::Clamp(DifficultySkulls + DeltaSkulls, 0, 9999);

	// Cache integer "tier" for backward-compat call sites (treated as skull count).
	DifficultyTier = FMath::Clamp(DifficultySkulls, 0, 999);
	DifficultyChanged.Broadcast();
}

int32 UT66RunStateSubsystem::GetHeartDisplayTier() const
{
	int32 HighestTier = 0;
	for (int32 SlotIndex = 0; SlotIndex < DefaultMaxHearts; ++SlotIndex)
	{
		HighestTier = FMath::Max(HighestTier, GetHeartSlotTier(SlotIndex));
	}
	return HighestTier;
}

int32 UT66RunStateSubsystem::GetHeartSlotTier(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= DefaultMaxHearts)
	{
		return 0;
	}

	const int32 StoredTier = HeartSlotTiers.IsValidIndex(SlotIndex)
		? static_cast<int32>(HeartSlotTiers[SlotIndex])
		: 0;
	return FMath::Clamp(StoredTier, 0, MaxHeartTier);
}

float UT66RunStateSubsystem::GetHPForHeartTier(const int32 Tier)
{
	return HPPerRedHeart * FMath::Pow(HeartTierScale, static_cast<float>(FMath::Clamp(Tier, 0, MaxHeartTier)));
}

float UT66RunStateSubsystem::GetHeartSlotCapacity(const int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= DefaultMaxHearts)
	{
		return 0.f;
	}

	return GetHPForHeartTier(GetHeartSlotTier(SlotIndex));
}

float UT66RunStateSubsystem::GetTotalHeartCapacity() const
{
	float TotalCapacity = 0.f;
	for (int32 SlotIndex = 0; SlotIndex < DefaultMaxHearts; ++SlotIndex)
	{
		TotalCapacity += GetHeartSlotCapacity(SlotIndex);
	}
	return TotalCapacity;
}

void UT66RunStateSubsystem::ResetHeartSlotTiers()
{
	HeartSlotTiers.Init(0, DefaultMaxHearts);
}

void UT66RunStateSubsystem::SyncMaxHPToHeartTiers()
{
	MaxHP = FMath::Max(DefaultMaxHP, GetTotalHeartCapacity());
}

int32 UT66RunStateSubsystem::FindUpgradeableHeartSlot() const
{
	int32 BestSlot = INDEX_NONE;
	int32 LowestTier = MaxHeartTier + 1;
	for (int32 SlotIndex = 0; SlotIndex < DefaultMaxHearts; ++SlotIndex)
	{
		const int32 Tier = GetHeartSlotTier(SlotIndex);
		if (Tier >= MaxHeartTier)
		{
			continue;
		}

		if (Tier < LowestTier)
		{
			LowestTier = Tier;
			BestSlot = SlotIndex;
		}
	}

	return BestSlot;
}

void UT66RunStateSubsystem::RebuildHeartSlotTiersFromMaxHP()
{
	ResetHeartSlotTiers();

	MaxHP = FMath::Max(DefaultMaxHP, MaxHP);
	float RemainingExtraHP = MaxHP - DefaultMaxHP;
	while (RemainingExtraHP > KINDA_SMALL_NUMBER)
	{
		const int32 SlotIndex = FindUpgradeableHeartSlot();
		if (SlotIndex == INDEX_NONE)
		{
			break;
		}

		const int32 CurrentTier = GetHeartSlotTier(SlotIndex);
		const float UpgradeDelta = GetHPForHeartTier(CurrentTier + 1) - GetHPForHeartTier(CurrentTier);
		if (RemainingExtraHP + 0.01f < UpgradeDelta)
		{
			break;
		}

		HeartSlotTiers[SlotIndex] = static_cast<uint8>(CurrentTier + 1);
		RemainingExtraHP -= UpgradeDelta;
	}

	SyncMaxHPToHeartTiers();
	CurrentHP = FMath::Clamp(CurrentHP, 0.f, MaxHP);
}

float UT66RunStateSubsystem::GetHeartSlotFill(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= DefaultMaxHearts)
	{
		return 0.f;
	}

	float SlotStart = 0.f;
	for (int32 Index = 0; Index < SlotIndex; ++Index)
	{
		SlotStart += GetHeartSlotCapacity(Index);
	}

	const float SlotCapacity = GetHeartSlotCapacity(SlotIndex);
	if (SlotCapacity <= 0.f)
	{
		return 0.f;
	}

	const float SlotEnd = SlotStart + SlotCapacity;
	if (CurrentHP <= SlotStart) return 0.f;
	if (CurrentHP >= SlotEnd) return 1.f;
	return (CurrentHP - SlotStart) / SlotCapacity;
}

void UT66RunStateSubsystem::AddMaxHearts(int32 DeltaHearts)
{
	if (DeltaHearts <= 0)
	{
		return;
	}

	if (HeartSlotTiers.Num() != DefaultMaxHearts)
	{
		RebuildHeartSlotTiersFromMaxHP();
	}

	const float PreviousMaxHP = MaxHP;
	bool bUpgraded = false;
	for (int32 UpgradeIndex = 0; UpgradeIndex < DeltaHearts; ++UpgradeIndex)
	{
		const int32 SlotIndex = FindUpgradeableHeartSlot();
		if (SlotIndex == INDEX_NONE)
		{
			break;
		}

		HeartSlotTiers[SlotIndex] = static_cast<uint8>(GetHeartSlotTier(SlotIndex) + 1);
		bUpgraded = true;
	}

	if (!bUpgraded)
	{
		return;
	}

	SyncMaxHPToHeartTiers();
	CurrentHP = FMath::Clamp(CurrentHP + (MaxHP - PreviousMaxHP), 0.f, MaxHP);
	HeartsChanged.Broadcast();
}

int32 UT66RunStateSubsystem::RegisterTotemActivated()
{
	TotemsActivatedCount = FMath::Clamp(TotemsActivatedCount + 1, 0, 999);
	return TotemsActivatedCount;
}

float UT66RunStateSubsystem::AddGamblerAngerFromBet(int32 BetGold)
{
	return AddCasinoAngerFromGold(BetGold);
}

float UT66RunStateSubsystem::AddCasinoAngerFromGold(int32 AngerGold)
{
	const float Delta = FMath::Clamp(static_cast<float>(FMath::Max(0, AngerGold)) / static_cast<float>(VendorAngerThresholdGold), 0.f, 1.f);
	const float NewAnger01 = FMath::Clamp(GamblerAnger01 + Delta, 0.f, 1.f);
	const int32 NewVendorAngerGold = FMath::RoundToInt(NewAnger01 * static_cast<float>(VendorAngerThresholdGold));
	if (FMath::IsNearlyEqual(NewAnger01, GamblerAnger01) && VendorAngerGold == NewVendorAngerGold)
	{
		return GamblerAnger01;
	}

	GamblerAnger01 = NewAnger01;
	VendorAngerGold = NewVendorAngerGold;
	GamblerAngerChanged.Broadcast();
	VendorChanged.Broadcast();
	return GamblerAnger01;
}

void UT66RunStateSubsystem::ResetGamblerAnger()
{
	if (FMath::IsNearlyZero(GamblerAnger01) && VendorAngerGold == 0) return;
	GamblerAnger01 = 0.f;
	VendorAngerGold = 0;
	GamblerAngerChanged.Broadcast();
	VendorChanged.Broadcast();
}

int32 UT66RunStateSubsystem::WholeStatToTenths(const int32 WholeValue)
{
	return FT66HeroPreciseStatBlock::WholeStatToTenths(WholeValue);
}

int32 UT66RunStateSubsystem::TenthsToDisplayStat(const int32 ValueTenths)
{
	return ClampHeroStatValue(FT66HeroPreciseStatBlock::TenthsToDisplayStat(ValueTenths));
}

float UT66RunStateSubsystem::TenthsToFloatStat(const int32 ValueTenths)
{
	return FMath::Max(0.f, static_cast<float>(ValueTenths) / static_cast<float>(HeroStatTenthsScale));
}

int32 UT66RunStateSubsystem::GetItemPrimaryStatTenths(const ET66HeroStatType StatType) const
{
	switch (StatType)
	{
	case ET66HeroStatType::Damage:      return FMath::Max(0, ItemPrimaryStatBonusesPrecise.DamageTenths);
	case ET66HeroStatType::AttackSpeed: return FMath::Max(0, ItemPrimaryStatBonusesPrecise.AttackSpeedTenths);
	case ET66HeroStatType::AttackScale: return FMath::Max(0, ItemPrimaryStatBonusesPrecise.AttackScaleTenths);
	case ET66HeroStatType::Accuracy:    return FMath::Max(0, ItemPrimaryStatBonusesPrecise.AccuracyTenths);
	case ET66HeroStatType::Armor:       return FMath::Max(0, ItemPrimaryStatBonusesPrecise.ArmorTenths);
	case ET66HeroStatType::Evasion:     return FMath::Max(0, ItemPrimaryStatBonusesPrecise.EvasionTenths);
	case ET66HeroStatType::Luck:        return FMath::Max(0, ItemPrimaryStatBonusesPrecise.LuckTenths);
	case ET66HeroStatType::Speed:       return FMath::Max(0, ItemPrimaryStatBonusesPrecise.SpeedTenths);
	default:                            return 0;
	}
}

int32 UT66RunStateSubsystem::GetPermanentPrimaryBuffTenths(const ET66HeroStatType StatType) const
{
	switch (StatType)
	{
	case ET66HeroStatType::Damage:      return WholeStatToTenths(FMath::Max(0, PermanentBuffStatBonuses.Damage));
	case ET66HeroStatType::AttackSpeed: return WholeStatToTenths(FMath::Max(0, PermanentBuffStatBonuses.AttackSpeed));
	case ET66HeroStatType::AttackScale: return WholeStatToTenths(FMath::Max(0, PermanentBuffStatBonuses.AttackScale));
	case ET66HeroStatType::Accuracy:    return WholeStatToTenths(FMath::Max(0, PermanentBuffStatBonuses.Accuracy));
	case ET66HeroStatType::Armor:       return WholeStatToTenths(FMath::Max(0, PermanentBuffStatBonuses.Armor));
	case ET66HeroStatType::Evasion:     return WholeStatToTenths(FMath::Max(0, PermanentBuffStatBonuses.Evasion));
	case ET66HeroStatType::Luck:        return WholeStatToTenths(FMath::Max(0, PermanentBuffStatBonuses.Luck));
	case ET66HeroStatType::Speed:       return 0;
	default:                            return 0;
	}
}

int32 UT66RunStateSubsystem::GetPrecisePrimaryStatTenths(const ET66HeroStatType StatType) const
{
	int32 TotalTenths = 0;
	switch (StatType)
	{
	case ET66HeroStatType::Damage:      TotalTenths = HeroPreciseStats.DamageTenths; break;
	case ET66HeroStatType::AttackSpeed: TotalTenths = HeroPreciseStats.AttackSpeedTenths; break;
	case ET66HeroStatType::AttackScale: TotalTenths = HeroPreciseStats.AttackScaleTenths; break;
	case ET66HeroStatType::Accuracy:    TotalTenths = HeroPreciseStats.AccuracyTenths; break;
	case ET66HeroStatType::Armor:       TotalTenths = HeroPreciseStats.ArmorTenths; break;
	case ET66HeroStatType::Evasion:     TotalTenths = HeroPreciseStats.EvasionTenths; break;
	case ET66HeroStatType::Luck:        TotalTenths = HeroPreciseStats.LuckTenths; break;
	case ET66HeroStatType::Speed:       TotalTenths = HeroPreciseStats.SpeedTenths; break;
	default:                            TotalTenths = WholeStatToTenths(DefaultHeroLevel); break;
	}

	TotalTenths += GetItemPrimaryStatTenths(StatType);
	TotalTenths += GetPermanentPrimaryBuffTenths(StatType);
	return ClampHeroStatTenths(TotalTenths);
}

int32 UT66RunStateSubsystem::GetSecondaryStatBonusTenths(const ET66SecondaryStatType StatType) const
{
	const int32 PersistentTenths = PersistentSecondaryStatBonusTenths.Contains(StatType)
		? FMath::Max(0, PersistentSecondaryStatBonusTenths.FindRef(StatType))
		: 0;
	const int32 ItemTenths = ItemSecondaryStatBonusTenths.Contains(StatType)
		? FMath::Max(0, ItemSecondaryStatBonusTenths.FindRef(StatType))
		: 0;
	return FMath::Max(0, PersistentTenths + ItemTenths);
}

float UT66RunStateSubsystem::GetSecondaryStatBonusValue(const ET66SecondaryStatType StatType) const
{
	return TenthsToFloatStat(GetSecondaryStatBonusTenths(StatType));
}

int32 UT66RunStateSubsystem::GetCategoryBaseStatTenths(const ET66SecondaryStatType StatType) const
{
	switch (StatType)
	{
	case ET66SecondaryStatType::AoeDamage:   return WholeStatToTenths(BaseAoeDmg);
	case ET66SecondaryStatType::BounceDamage:return WholeStatToTenths(BaseBounceDmg);
	case ET66SecondaryStatType::PierceDamage:return WholeStatToTenths(BasePierceDmg);
	case ET66SecondaryStatType::DotDamage:   return WholeStatToTenths(BaseDotDmg);
	case ET66SecondaryStatType::AoeSpeed:    return WholeStatToTenths(BaseAoeAtkSpd);
	case ET66SecondaryStatType::BounceSpeed: return WholeStatToTenths(BaseBounceAtkSpd);
	case ET66SecondaryStatType::PierceSpeed: return WholeStatToTenths(BasePierceAtkSpd);
	case ET66SecondaryStatType::DotSpeed:    return WholeStatToTenths(BaseDotAtkSpd);
	case ET66SecondaryStatType::AoeScale:    return WholeStatToTenths(BaseAoeAtkScale);
	case ET66SecondaryStatType::BounceScale: return WholeStatToTenths(BaseBounceAtkScale);
	case ET66SecondaryStatType::PierceScale: return WholeStatToTenths(BasePierceAtkScale);
	case ET66SecondaryStatType::DotScale:    return WholeStatToTenths(BaseDotAtkScale);
	default:                                 return 0;
	}
}

int32 UT66RunStateSubsystem::GetCategoryTotalStatTenths(const ET66SecondaryStatType StatType) const
{
	return ClampHeroStatTenths(GetCategoryBaseStatTenths(StatType) + GetSecondaryStatBonusTenths(StatType));
}

void UT66RunStateSubsystem::SyncLegacyHeroStatsFromPrecise()
{
	HeroStats = HeroPreciseStats.ToDisplayStatBlock();
	HeroStats.Damage = ClampHeroStatValue(HeroStats.Damage);
	HeroStats.AttackSpeed = ClampHeroStatValue(HeroStats.AttackSpeed);
	HeroStats.AttackScale = ClampHeroStatValue(HeroStats.AttackScale);
	HeroStats.Accuracy = ClampHeroStatValue(HeroStats.Accuracy);
	HeroStats.Armor = ClampHeroStatValue(HeroStats.Armor);
	HeroStats.Evasion = ClampHeroStatValue(HeroStats.Evasion);
	HeroStats.Luck = ClampHeroStatValue(HeroStats.Luck);
	HeroStats.Speed = ClampHeroStatValue(HeroStats.Speed);
}

void UT66RunStateSubsystem::ClearPersistentSecondaryStatBonuses()
{
	PersistentSecondaryStatBonusTenths.Reset();
}

void UT66RunStateSubsystem::AddPersistentSecondaryStatBonusTenths(const ET66SecondaryStatType StatType, const int32 DeltaTenths)
{
	if (StatType == ET66SecondaryStatType::None || DeltaTenths <= 0)
	{
		return;
	}

	int32& Accum = PersistentSecondaryStatBonusTenths.FindOrAdd(StatType);
	Accum = FMath::Clamp(Accum + DeltaTenths, 0, MaxHeroStatValue * HeroStatTenthsScale);
}

void UT66RunStateSubsystem::AddItemSecondaryStatBonusTenths(const ET66SecondaryStatType StatType, const int32 DeltaTenths)
{
	if (StatType == ET66SecondaryStatType::None || DeltaTenths <= 0)
	{
		return;
	}

	int32& Accum = ItemSecondaryStatBonusTenths.FindOrAdd(StatType);
	Accum = FMath::Clamp(Accum + DeltaTenths, 0, MaxHeroStatValue * HeroStatTenthsScale);
}

int32 UT66RunStateSubsystem::RollHeroPrimaryGainTenthsBiased(const FT66HeroStatGainRange& Range, const FName Category)
{
	UT66RngSubsystem* RngSub = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		RngSub = GI->GetSubsystem<UT66RngSubsystem>();
	}

	const int32 MinTenths = Range.GetMinTenths();
	const int32 MaxTenths = Range.GetMaxTenths();
	if (MaxTenths <= 0)
	{
		return 0;
	}
	if (MaxTenths <= MinTenths)
	{
		return MaxTenths;
	}

	const int32 PreDrawSeed = HeroStatRng.GetCurrentSeed();
	float U = HeroStatRng.GetFraction();
	if (RngSub)
	{
		U = RngSub->BiasHigh01(U);
	}

	const int32 Span = MaxTenths - MinTenths;
	const int32 Delta = FMath::Clamp(FMath::FloorToInt(U * static_cast<float>(Span + 1)), 0, Span);
	const int32 RolledTenths = MinTenths + Delta;
	RecordLuckQuantityFloatRollRounded(
		Category,
		RolledTenths,
		MinTenths,
		MaxTenths,
		Range.Min,
		Range.Max,
		INDEX_NONE,
		PreDrawSeed);
	return RolledTenths;
}

void UT66RunStateSubsystem::ApplyPrimaryGainToSecondaryBonuses(
	const ET66HeroStatType PrimaryStatType,
	const int32 PrimaryGainTenths,
	TMap<ET66SecondaryStatType, int32>& TargetBonuses,
	const int32 SeedSalt) const
{
	if (PrimaryGainTenths <= 0)
	{
		return;
	}

	const TArray<ET66SecondaryStatType>& SecondaryTypes = T66_GetSecondaryTypesForPrimary(PrimaryStatType);
	if (SecondaryTypes.Num() == 0)
	{
		return;
	}

	const ET66SecondaryStatType MainAttackSecondaryType = T66_GetHeroMainAttackSecondaryType(PrimaryStatType, HeroPrimaryAttackCategory);
	for (int32 Index = 0; Index < SecondaryTypes.Num(); ++Index)
	{
		const ET66SecondaryStatType SecondaryType = SecondaryTypes[Index];
		float MinFactor = 0.5f;
		float MaxFactor = 1.0f;
		if (PrimaryStatType == ET66HeroStatType::Damage
			|| PrimaryStatType == ET66HeroStatType::AttackSpeed
			|| PrimaryStatType == ET66HeroStatType::AttackScale)
		{
			const bool bIsMainAttackType = (SecondaryType == MainAttackSecondaryType);
			MinFactor = bIsMainAttackType ? 0.70f : 0.10f;
			MaxFactor = bIsMainAttackType ? 1.00f : 0.50f;
		}

		const int32 SecondaryGainTenths = T66_RollScaledTenthsDeterministic(
			PrimaryGainTenths,
			MinFactor,
			MaxFactor,
			HashCombine(GetTypeHash(SecondaryType), GetTypeHash(SeedSalt + Index + 1)));
		if (SecondaryGainTenths <= 0)
		{
			continue;
		}

		int32& Accum = TargetBonuses.FindOrAdd(SecondaryType);
		Accum = FMath::Clamp(Accum + SecondaryGainTenths, 0, MaxHeroStatValue * HeroStatTenthsScale);
	}
}

void UT66RunStateSubsystem::InitializeHeroStatTuningForSelectedHero()
{
	// Safe defaults
	HeroStats = FT66HeroStatBlock{};
	HeroPreciseStats = FT66HeroPreciseStatBlock{};
	HeroPerLevelGains = FT66HeroPerLevelStatGains{};
	HeroPerLevelGains.Damage.Min = 0.5f;      HeroPerLevelGains.Damage.Max = 1.0f;
	HeroPerLevelGains.AttackSpeed.Min = 0.1f; HeroPerLevelGains.AttackSpeed.Max = 1.0f;
	HeroPerLevelGains.AttackScale.Min = 0.1f; HeroPerLevelGains.AttackScale.Max = 1.0f;
	HeroPerLevelGains.Accuracy.Min = 0.1f;    HeroPerLevelGains.Accuracy.Max = 1.0f;
	HeroPerLevelGains.Armor.Min = 0.1f;       HeroPerLevelGains.Armor.Max = 1.0f;
	HeroPerLevelGains.Evasion.Min = 0.1f;     HeroPerLevelGains.Evasion.Max = 1.0f;
	HeroPerLevelGains.Luck.Min = 0.1f;        HeroPerLevelGains.Luck.Max = 1.0f;
	HeroPerLevelGains.Speed.Min = 0.1f;       HeroPerLevelGains.Speed.Max = 1.0f;

	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		FT66HeroStatBlock Base;
		FT66HeroPerLevelStatGains Gains;
		if (T66GI->GetHeroStatTuning(T66GI->SelectedHeroID, Base, Gains))
		{
			HeroStats = Base;
			HeroPreciseStats.SetFromWholeStatBlock(Base);
			HeroPerLevelGains = Gains;
		}

		// Load category-specific base stats and secondary base stats from the hero DataTable.
		FHeroData HD;
		if (T66GI->GetHeroData(T66GI->SelectedHeroID, HD))
		{
			BasePierceDmg = FMath::Max(1, HD.BasePierceDmg);
			BasePierceAtkSpd = FMath::Max(1, HD.BasePierceAtkSpd);
			BasePierceAtkScale = FMath::Max(1, HD.BasePierceAtkScale);
			BaseBounceDmg = FMath::Max(1, HD.BaseBounceDmg);
			BaseBounceAtkSpd = FMath::Max(1, HD.BaseBounceAtkSpd);
			BaseBounceAtkScale = FMath::Max(1, HD.BaseBounceAtkScale);
			BaseAoeDmg = FMath::Max(1, HD.BaseAoeDmg);
			BaseAoeAtkSpd = FMath::Max(1, HD.BaseAoeAtkSpd);
			BaseAoeAtkScale = FMath::Max(1, HD.BaseAoeAtkScale);
			BaseDotDmg = FMath::Max(1, HD.BaseDotDmg);
			BaseDotAtkSpd = FMath::Max(1, HD.BaseDotAtkSpd);
			BaseDotAtkScale = FMath::Max(1, HD.BaseDotAtkScale);

			// Secondary base stats
			HeroBaseCritDamage = FMath::Max(1.f, HD.BaseCritDamage);
			HeroBaseCritChance = FMath::Clamp(HD.BaseCritChance, 0.f, 1.f);
			HeroBaseCloseRangeDmg = FMath::Max(0.f, HD.BaseCloseRangeDmg);
			HeroBaseLongRangeDmg = FMath::Max(0.f, HD.BaseLongRangeDmg);
			HeroBaseTaunt = FMath::Max(0.f, HD.BaseTaunt);
			HeroBaseReflectDmg = FMath::Max(0.f, HD.BaseReflectDmg);
			HeroBaseHpRegen = 0.f;
			HeroBaseCrushChance = FMath::Clamp(HD.BaseCrushChance, 0.f, 1.f);
			HeroBaseInvisChance = FMath::Clamp(HD.BaseInvisChance, 0.f, 1.f);
			HeroBaseCounterAttack = FMath::Max(0.f, HD.BaseCounterAttack);
			HeroBaseLifeSteal = 0.f;
			HeroBaseAssassinateChance = FMath::Clamp(HD.BaseAssassinateChance, 0.f, 1.f);
			HeroBaseCheatChance = FMath::Clamp(HD.BaseCheatChance, 0.f, 1.f);
			HeroBaseStealChance = FMath::Clamp(HD.BaseStealChance, 0.f, 1.f);
			float Range = HD.BaseAttackRange;
			HeroBaseAttackRange = FMath::Max(100.f, Range);
			HeroBaseAccuracy = FMath::Clamp(HD.BaseAccuracy, 0.f, 1.f);
			PassiveType = HD.PassiveType;
			HeroPrimaryAttackCategory = HD.PrimaryCategory;
		}
	}
	else
	{
		HeroPreciseStats.SetFromWholeStatBlock(HeroStats);
	}

	// Enforce sane minimums for gameplay safety.
	HeroPreciseStats.DamageTenths = ClampHeroStatTenths(FMath::Max(HeroPreciseStats.DamageTenths, WholeStatToTenths(HeroStats.Damage)));
	HeroPreciseStats.AttackSpeedTenths = ClampHeroStatTenths(FMath::Max(HeroPreciseStats.AttackSpeedTenths, WholeStatToTenths(HeroStats.AttackSpeed)));
	HeroPreciseStats.AttackScaleTenths = ClampHeroStatTenths(FMath::Max(HeroPreciseStats.AttackScaleTenths, WholeStatToTenths(HeroStats.AttackScale)));
	HeroPreciseStats.AccuracyTenths = ClampHeroStatTenths(FMath::Max(HeroPreciseStats.AccuracyTenths, WholeStatToTenths(HeroStats.Accuracy)));
	HeroPreciseStats.ArmorTenths = ClampHeroStatTenths(FMath::Max(HeroPreciseStats.ArmorTenths, WholeStatToTenths(HeroStats.Armor)));
	HeroPreciseStats.EvasionTenths = ClampHeroStatTenths(FMath::Max(HeroPreciseStats.EvasionTenths, WholeStatToTenths(HeroStats.Evasion)));
	HeroPreciseStats.LuckTenths = ClampHeroStatTenths(FMath::Max(HeroPreciseStats.LuckTenths, WholeStatToTenths(HeroStats.Luck)));
	HeroPreciseStats.SpeedTenths = ClampHeroStatTenths(FMath::Max(HeroPreciseStats.SpeedTenths, WholeStatToTenths(HeroStats.Speed)));
	SyncLegacyHeroStatsFromPrecise();

	RefreshPermanentBuffBonusesFromProfile();
}

void UT66RunStateSubsystem::AddPowerCrystalsEarnedThisRun(int32 Amount)
{
	if (Amount <= 0) return;
	PowerCrystalsEarnedThisRun = FMath::Clamp(PowerCrystalsEarnedThisRun + Amount, 0, 2000000000);
}

void UT66RunStateSubsystem::MarkPendingPowerCrystalsGrantedToWallet()
{
	PowerCrystalsGrantedToWalletThisRun = FMath::Clamp(PowerCrystalsEarnedThisRun, 0, 2000000000);
}

void UT66RunStateSubsystem::ActivatePendingSingleUseBuffsForRunStart()
{
	SingleUseSecondaryMultipliers.Reset();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BuffSubsystem* PowerUp = GI->GetSubsystem<UT66BuffSubsystem>())
		{
			SingleUseSecondaryMultipliers = PowerUp->ConsumePendingSingleUseBuffMultipliers();
		}

		if (UT66RngSubsystem* Rng = GI->GetSubsystem<UT66RngSubsystem>())
		{
			Rng->UpdateLuckStat(GetLuckStat());
		}
	}
}

void UT66RunStateSubsystem::SetPendingDifficultyClearSummary(const bool bPending)
{
	if (bPendingDifficultyClearSummary == bPending)
	{
		return;
	}

	bPendingDifficultyClearSummary = bPending;
	StageChanged.Broadcast();
}

void UT66RunStateSubsystem::SetSaintBlessingActive(const bool bActive)
{
	if (bSaintBlessingActive == bActive)
	{
		return;
	}

	bSaintBlessingActive = bActive;
	SurvivalChanged.Broadcast();
}

void UT66RunStateSubsystem::BeginSaintBlessingEmpowerment()
{
	if (bSaintBlessingLoadoutSnapshotValid)
	{
		return;
	}

	bSaintBlessingLoadoutSnapshotValid = true;
	SaintBlessingInventorySnapshot = InventorySlots;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
		{
			SaintBlessingEquippedIdolsSnapshot = IdolManager->GetEquippedIdols();
			SaintBlessingEquippedIdolTiersSnapshot = IdolManager->GetEquippedIdolTierValues();

			TArray<uint8> BlessedTiers = SaintBlessingEquippedIdolTiersSnapshot;
			const int32 MaxCount = FMath::Min(BlessedTiers.Num(), SaintBlessingEquippedIdolsSnapshot.Num());
			for (int32 Index = 0; Index < MaxCount; ++Index)
			{
				if (!SaintBlessingEquippedIdolsSnapshot[Index].IsNone())
				{
					BlessedTiers[Index] = static_cast<uint8>(UT66IdolManagerSubsystem::MaxIdolLevel);
				}
			}

			IdolManager->RestoreState(
				SaintBlessingEquippedIdolsSnapshot,
				BlessedTiers,
				IdolManager->GetCurrentDifficulty(),
				IdolManager->GetRemainingCatchUpIdolPicks());
		}
	}

	bool bInventoryChanged = false;
	for (FT66InventorySlot& Slot : InventorySlots)
	{
		if (!Slot.IsValid() || T66_IsGamblersTokenItem(Slot.ItemTemplateID))
		{
			continue;
		}

		Slot.Line1RolledValue = T66_MapBlessingRollToWhiteRange(Slot);
		Slot.SecondaryStatBonusOverride = FItemData::GetFlatSecondaryStatBonus(ET66ItemRarity::White);
		Slot.Line2MultiplierOverride = FMath::Max(Slot.GetLine2Multiplier(), FItemData::GetLine2RarityMultiplier(ET66ItemRarity::White));
		Slot.Rarity = ET66ItemRarity::White;
		bInventoryChanged = true;
	}

	if (bInventoryChanged)
	{
		RecomputeItemDerivedStats();
		InventoryChanged.Broadcast();
	}
}

void UT66RunStateSubsystem::EndSaintBlessingEmpowerment()
{
	if (!bSaintBlessingLoadoutSnapshotValid)
	{
		return;
	}

	TArray<FT66InventorySlot> RestoredInventory = SaintBlessingInventorySnapshot;
	for (int32 Index = SaintBlessingInventorySnapshot.Num(); Index < InventorySlots.Num(); ++Index)
	{
		RestoredInventory.Add(InventorySlots[Index]);
	}

	InventorySlots = MoveTemp(RestoredInventory);
	RecomputeItemDerivedStats();
	InventoryChanged.Broadcast();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
		{
			IdolManager->RestoreState(
				SaintBlessingEquippedIdolsSnapshot,
				SaintBlessingEquippedIdolTiersSnapshot,
				IdolManager->GetCurrentDifficulty(),
				IdolManager->GetRemainingCatchUpIdolPicks());
		}
	}

	SaintBlessingInventorySnapshot.Reset();
	SaintBlessingEquippedIdolsSnapshot.Reset();
	SaintBlessingEquippedIdolTiersSnapshot.Reset();
	bSaintBlessingLoadoutSnapshotValid = false;
}

void UT66RunStateSubsystem::SetFinalSurvivalEnemyScalar(const float Scalar)
{
	const float ClampedScalar = FMath::Clamp(Scalar, 1.f, 99.f);
	if (FMath::IsNearlyEqual(FinalSurvivalEnemyScalar, ClampedScalar, 0.05f))
	{
		return;
	}

	FinalSurvivalEnemyScalar = ClampedScalar;
	DifficultyChanged.Broadcast();
}

void UT66RunStateSubsystem::RefreshPermanentBuffBonusesFromProfile()
{
	PermanentBuffStatBonuses = FT66HeroStatBonuses{};
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BuffSubsystem* Buffs = GI->GetSubsystem<UT66BuffSubsystem>())
		{
			PermanentBuffStatBonuses = Buffs->GetPermanentBuffStatBonuses();
		}
	}
}

void UT66RunStateSubsystem::ApplyOneHeroLevelUp()
{
	UT66RngSubsystem* RngSub = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		RngSub = GI->GetSubsystem<UT66RngSubsystem>();
		if (RngSub)
		{
			// Luck influences how often we roll the high end, but the min/max ranges themselves stay fixed.
			RngSub->UpdateLuckStat(GetLuckStat());
		}
	}

	auto ApplyPrimaryGain = [&](const ET66HeroStatType StatType, int32& TargetTenths, const FT66HeroStatGainRange& Range, const FName Category)
	{
		const int32 RolledGainTenths = RollHeroPrimaryGainTenthsBiased(Range, Category);
		if (RolledGainTenths <= 0)
		{
			return;
		}

		const int32 PreviousTenths = TargetTenths;
		TargetTenths = ClampHeroStatTenths(TargetTenths + RolledGainTenths);
		const int32 AppliedGainTenths = FMath::Max(0, TargetTenths - PreviousTenths);
		if (AppliedGainTenths > 0)
		{
			ApplyPrimaryGainToSecondaryBonuses(StatType, AppliedGainTenths, PersistentSecondaryStatBonusTenths, HeroStatRng.GetCurrentSeed());
		}
	};

	// Foundational stats roll within the hero's authored per-level gain ranges.
	ApplyPrimaryGain(ET66HeroStatType::Damage, HeroPreciseStats.DamageTenths, HeroPerLevelGains.Damage, FName(TEXT("LevelUp_DamageGain")));
	ApplyPrimaryGain(ET66HeroStatType::AttackSpeed, HeroPreciseStats.AttackSpeedTenths, HeroPerLevelGains.AttackSpeed, FName(TEXT("LevelUp_AttackSpeedGain")));
	ApplyPrimaryGain(ET66HeroStatType::AttackScale, HeroPreciseStats.AttackScaleTenths, HeroPerLevelGains.AttackScale, FName(TEXT("LevelUp_AttackScaleGain")));
	ApplyPrimaryGain(ET66HeroStatType::Accuracy, HeroPreciseStats.AccuracyTenths, HeroPerLevelGains.Accuracy, FName(TEXT("LevelUp_AccuracyGain")));
	ApplyPrimaryGain(ET66HeroStatType::Armor, HeroPreciseStats.ArmorTenths, HeroPerLevelGains.Armor, FName(TEXT("LevelUp_ArmorGain")));
	ApplyPrimaryGain(ET66HeroStatType::Evasion, HeroPreciseStats.EvasionTenths, HeroPerLevelGains.Evasion, FName(TEXT("LevelUp_EvasionGain")));
	ApplyPrimaryGain(ET66HeroStatType::Luck, HeroPreciseStats.LuckTenths, HeroPerLevelGains.Luck, FName(TEXT("LevelUp_LuckGain")));
	ApplyPrimaryGain(ET66HeroStatType::Speed, HeroPreciseStats.SpeedTenths, HeroPerLevelGains.Speed, FName(TEXT("LevelUp_SpeedGain")));
	SyncLegacyHeroStatsFromPrecise();
}

void UT66RunStateSubsystem::InitializeHeroStatsForNewRun()
{
	// Seed RNG once per run so stage reloads keep the same future stat gain sequence.
	int32 Seed = static_cast<int32>(FPlatformTime::Cycles());
	const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	if (T66GI)
	{
		Seed ^= static_cast<int32>(GetTypeHash(T66GI->SelectedHeroID));
		Seed ^= static_cast<int32>(GetTypeHash(T66GI->SelectedCompanionID));
	}
	Seed ^= (HeroLevel * 1337);
	HeroStatRng.Initialize(Seed);

	InitializeHeroStatTuningForSelectedHero();
	ClearPersistentSecondaryStatBonuses();

	// Apply level-up gains for any non-1 starting level (difficulty boosts start at +10 levels per step).
	const int32 TargetLevel = FMath::Clamp(HeroLevel, DefaultHeroLevel, MaxHeroLevel);
	HeroLevel = TargetLevel;
	for (int32 L = 2; L <= TargetLevel; ++L)
	{
		ApplyOneHeroLevelUp();
	}

	if (T66GI && T66GI->SelectedHeroID == T66ArthurHeroID)
	{
		const int32 ArthurBoostTenths = WholeStatToTenths(T66ArthurTestStatBoost);
		HeroPreciseStats.DamageTenths = ClampHeroStatTenths(HeroPreciseStats.DamageTenths + ArthurBoostTenths);
		HeroPreciseStats.AttackSpeedTenths = ClampHeroStatTenths(HeroPreciseStats.AttackSpeedTenths + ArthurBoostTenths);
		HeroPreciseStats.AttackScaleTenths = ClampHeroStatTenths(HeroPreciseStats.AttackScaleTenths + ArthurBoostTenths);
		HeroPreciseStats.AccuracyTenths = ClampHeroStatTenths(HeroPreciseStats.AccuracyTenths + ArthurBoostTenths);
		HeroPreciseStats.ArmorTenths = ClampHeroStatTenths(HeroPreciseStats.ArmorTenths + ArthurBoostTenths);
		HeroPreciseStats.EvasionTenths = ClampHeroStatTenths(HeroPreciseStats.EvasionTenths + ArthurBoostTenths);
		HeroPreciseStats.LuckTenths = ClampHeroStatTenths(HeroPreciseStats.LuckTenths + ArthurBoostTenths);
		HeroPreciseStats.SpeedTenths = ClampHeroStatTenths(HeroPreciseStats.SpeedTenths + ArthurBoostTenths);

		for (uint8 RawStatType = static_cast<uint8>(ET66SecondaryStatType::None) + 1;
			RawStatType <= static_cast<uint8>(ET66SecondaryStatType::Accuracy);
			++RawStatType)
		{
			const ET66SecondaryStatType StatType = static_cast<ET66SecondaryStatType>(RawStatType);
			if (T66IsLiveSecondaryStatType(StatType))
			{
				AddPersistentSecondaryStatBonusTenths(StatType, ArthurBoostTenths);
			}
		}
	}

	SyncLegacyHeroStatsFromPrecise();
}

// ============================================
// Category-specific stat getters (base from hero DataTable + item bonuses)
// ============================================

int32 UT66RunStateSubsystem::GetSpeedStat() const
{
	return TenthsToDisplayStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::Speed));
}

int32 UT66RunStateSubsystem::GetDamageStat() const
{
	return TenthsToDisplayStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::Damage));
}

int32 UT66RunStateSubsystem::GetAttackSpeedStat() const
{
	return TenthsToDisplayStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::AttackSpeed));
}

int32 UT66RunStateSubsystem::GetScaleStat() const
{
	return TenthsToDisplayStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::AttackScale));
}

int32 UT66RunStateSubsystem::GetAccuracyStat() const
{
	return TenthsToDisplayStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::Accuracy));
}

int32 UT66RunStateSubsystem::GetArmorStat() const
{
	return TenthsToDisplayStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::Armor));
}

int32 UT66RunStateSubsystem::GetEvasionStat() const
{
	return TenthsToDisplayStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::Evasion));
}

int32 UT66RunStateSubsystem::GetLuckStat() const
{
	return TenthsToDisplayStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::Luck));
}

int32 UT66RunStateSubsystem::GetPierceDmgStat() const      { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::PierceDamage)); }
int32 UT66RunStateSubsystem::GetPierceAtkSpdStat() const   { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::PierceSpeed)); }
int32 UT66RunStateSubsystem::GetPierceAtkScaleStat() const { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::PierceScale)); }

int32 UT66RunStateSubsystem::GetBounceDmgStat() const      { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::BounceDamage)); }
int32 UT66RunStateSubsystem::GetBounceAtkSpdStat() const   { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::BounceSpeed)); }
int32 UT66RunStateSubsystem::GetBounceAtkScaleStat() const { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::BounceScale)); }

int32 UT66RunStateSubsystem::GetAoeDmgStat() const         { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::AoeDamage)); }
int32 UT66RunStateSubsystem::GetAoeAtkSpdStat() const      { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::AoeSpeed)); }
int32 UT66RunStateSubsystem::GetAoeAtkScaleStat() const    { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::AoeScale)); }

int32 UT66RunStateSubsystem::GetDotDmgStat() const         { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::DotDamage)); }
int32 UT66RunStateSubsystem::GetDotAtkSpdStat() const      { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::DotSpeed)); }
int32 UT66RunStateSubsystem::GetDotAtkScaleStat() const    { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::DotScale)); }

// ============================================
// Secondary stat getters (hero base * accumulated item Line 2 multipliers)
// ============================================

float UT66RunStateSubsystem::GetSecondaryStatValue(ET66SecondaryStatType StatType) const
{
	float M = 1.f;
	if (const float* Mult = SecondaryMultipliers.Find(StatType); Mult && *Mult > 0.f)
	{
		M *= *Mult;
	}
	if (const float* SingleUseMult = SingleUseSecondaryMultipliers.Find(StatType); SingleUseMult && *SingleUseMult > 0.f)
	{
		M *= *SingleUseMult;
	}
	const float DamageMult = GetHeroDamageMultiplier();
	const float AttackSpeedMult = GetHeroAttackSpeedMultiplier();
	const float ScaleMult = GetHeroScaleMultiplier();
	const float AccuracyMult = GetHeroAccuracyMultiplier();
	const float BonusPoints = GetSecondaryStatBonusValue(StatType);
	const float BaseArmorReduction = FMath::Clamp(static_cast<float>(GetArmorStat() - 1) * 0.008f, 0.f, 0.80f);
	const float BaseEvasionChance = FMath::Clamp(static_cast<float>(GetEvasionStat() - 1) * 0.006f, 0.f, 0.60f);

	switch (StatType)
	{
	case ET66SecondaryStatType::AoeDamage:        return TenthsToFloatStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::AoeDamage)) * M * DamageMult;
	case ET66SecondaryStatType::BounceDamage:     return TenthsToFloatStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::BounceDamage)) * M * DamageMult;
	case ET66SecondaryStatType::PierceDamage:     return TenthsToFloatStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::PierceDamage)) * M * DamageMult;
	case ET66SecondaryStatType::DotDamage:        return TenthsToFloatStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::DotDamage)) * M * DamageMult;
	case ET66SecondaryStatType::CritDamage:       return FMath::Max(1.f, (HeroBaseCritDamage + (BonusPoints * 0.05f)) * M * AccuracyMult);
	case ET66SecondaryStatType::CloseRangeDamage: return 1.f;
	case ET66SecondaryStatType::LongRangeDamage:  return 1.f;
	case ET66SecondaryStatType::AoeSpeed:         return TenthsToFloatStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::AoeSpeed)) * M * AttackSpeedMult;
	case ET66SecondaryStatType::BounceSpeed:      return TenthsToFloatStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::BounceSpeed)) * M * AttackSpeedMult;
	case ET66SecondaryStatType::PierceSpeed:      return TenthsToFloatStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::PierceSpeed)) * M * AttackSpeedMult;
	case ET66SecondaryStatType::DotSpeed:         return TenthsToFloatStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::DotSpeed)) * M * AttackSpeedMult;
	case ET66SecondaryStatType::CritChance:       return FMath::Clamp((HeroBaseCritChance + (BonusPoints * 0.01f)) * M * AccuracyMult, 0.f, 1.f);
	case ET66SecondaryStatType::AoeScale:         return TenthsToFloatStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::AoeScale)) * M * ScaleMult;
	case ET66SecondaryStatType::BounceScale:      return TenthsToFloatStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::BounceScale)) * M * ScaleMult;
	case ET66SecondaryStatType::PierceScale:      return TenthsToFloatStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::PierceScale)) * M * ScaleMult;
	case ET66SecondaryStatType::DotScale:         return TenthsToFloatStat(GetCategoryTotalStatTenths(ET66SecondaryStatType::DotScale)) * M * ScaleMult;
	case ET66SecondaryStatType::AttackRange:      return FMath::Max(100.f, (HeroBaseAttackRange + (BonusPoints * 25.f)) * M * AccuracyMult);
	case ET66SecondaryStatType::Accuracy:         return FMath::Clamp((HeroBaseAccuracy + (BonusPoints * 0.01f)) * M * AccuracyMult, 0.f, 1.f);
	case ET66SecondaryStatType::Taunt:            return FMath::Max(0.f, (HeroBaseTaunt + (BonusPoints * 0.05f)) * M);
	case ET66SecondaryStatType::ReflectDamage:    return FMath::Clamp((HeroBaseReflectDmg + (BonusPoints * 0.01f)) * M, 0.f, 1.f);
	case ET66SecondaryStatType::HpRegen:          return FMath::Max(0.f, (HeroBaseHpRegen + (BonusPoints * 0.10f)) * M);
	case ET66SecondaryStatType::Crush:            return FMath::Clamp((HeroBaseCrushChance + (BonusPoints * 0.005f)) * M, 0.f, 1.f);
	case ET66SecondaryStatType::Invisibility:     return FMath::Clamp((HeroBaseInvisChance + (BonusPoints * 0.005f)) * M, 0.f, 1.f);
	case ET66SecondaryStatType::CounterAttack:    return FMath::Clamp((HeroBaseCounterAttack + (BonusPoints * 0.01f)) * M, 0.f, 1.f);
	case ET66SecondaryStatType::LifeSteal:        return FMath::Clamp((HeroBaseLifeSteal + (BonusPoints * 0.005f)) * M, 0.f, 1.f);
	case ET66SecondaryStatType::Assassinate:      return FMath::Clamp((HeroBaseAssassinateChance + (BonusPoints * 0.005f)) * M, 0.f, 1.f);
	case ET66SecondaryStatType::SpinWheel:        return 1.f;
	case ET66SecondaryStatType::Goblin:           return 1.f * M;
	case ET66SecondaryStatType::Leprechaun:       return 1.f * M;
	case ET66SecondaryStatType::TreasureChest:    return FMath::Max(1.f, (1.f + (BonusPoints * 0.05f)) * M);
	case ET66SecondaryStatType::Fountain:         return 1.f * M;
	case ET66SecondaryStatType::Cheating:         return FMath::Clamp((HeroBaseCheatChance + (BonusPoints * 0.01f)) * M, 0.f, 1.f);
	case ET66SecondaryStatType::Stealing:         return FMath::Clamp((HeroBaseStealChance + (BonusPoints * 0.01f)) * M, 0.f, 1.f);
	case ET66SecondaryStatType::MovementSpeed:    return FMath::Max(1.f, (1.f + (BonusPoints * 0.02f)) * M);
	case ET66SecondaryStatType::LootCrate:        return FMath::Max(1.f, (1.f + (BonusPoints * 0.05f)) * M);
	case ET66SecondaryStatType::DamageReduction:  return FMath::Clamp(BaseArmorReduction + (BonusPoints * 0.005f), 0.f, 0.80f);
	case ET66SecondaryStatType::EvasionChance:    return FMath::Clamp(BaseEvasionChance + (BonusPoints * 0.005f), 0.f, 0.60f);
	case ET66SecondaryStatType::Alchemy:          return FMath::Clamp(BonusPoints * 0.01f * M, 0.f, 1.f);
	default: return 1.f;
	}
}

float UT66RunStateSubsystem::GetSecondaryStatBaselineValue(ET66SecondaryStatType StatType) const
{
	switch (StatType)
	{
	case ET66SecondaryStatType::AoeDamage:       return static_cast<float>(BaseAoeDmg);
	case ET66SecondaryStatType::BounceDamage:    return static_cast<float>(BaseBounceDmg);
	case ET66SecondaryStatType::PierceDamage:    return static_cast<float>(BasePierceDmg);
	case ET66SecondaryStatType::DotDamage:       return static_cast<float>(BaseDotDmg);
	case ET66SecondaryStatType::CritDamage:      return HeroBaseCritDamage;
	case ET66SecondaryStatType::CloseRangeDamage:return 1.f;
	case ET66SecondaryStatType::LongRangeDamage: return 1.f;
	case ET66SecondaryStatType::AoeSpeed:        return static_cast<float>(BaseAoeAtkSpd);
	case ET66SecondaryStatType::BounceSpeed:     return static_cast<float>(BaseBounceAtkSpd);
	case ET66SecondaryStatType::PierceSpeed:     return static_cast<float>(BasePierceAtkSpd);
	case ET66SecondaryStatType::DotSpeed:        return static_cast<float>(BaseDotAtkSpd);
	case ET66SecondaryStatType::CritChance:      return FMath::Clamp(HeroBaseCritChance, 0.f, 1.f);
	case ET66SecondaryStatType::AoeScale:        return static_cast<float>(BaseAoeAtkScale);
	case ET66SecondaryStatType::BounceScale:     return static_cast<float>(BaseBounceAtkScale);
	case ET66SecondaryStatType::PierceScale:     return static_cast<float>(BasePierceAtkScale);
	case ET66SecondaryStatType::DotScale:        return static_cast<float>(BaseDotAtkScale);
	case ET66SecondaryStatType::AttackRange:     return HeroBaseAttackRange;
	case ET66SecondaryStatType::Accuracy:        return FMath::Clamp(HeroBaseAccuracy, 0.f, 1.f);
	case ET66SecondaryStatType::Taunt:           return HeroBaseTaunt;
	case ET66SecondaryStatType::ReflectDamage:   return FMath::Clamp(HeroBaseReflectDmg, 0.f, 1.f);
	case ET66SecondaryStatType::HpRegen:         return 0.f;
	case ET66SecondaryStatType::Crush:           return FMath::Clamp(HeroBaseCrushChance, 0.f, 1.f);
	case ET66SecondaryStatType::Invisibility:    return FMath::Clamp(HeroBaseInvisChance, 0.f, 1.f);
	case ET66SecondaryStatType::CounterAttack:   return FMath::Clamp(HeroBaseCounterAttack, 0.f, 1.f);
	case ET66SecondaryStatType::LifeSteal:       return 0.f;
	case ET66SecondaryStatType::Assassinate:     return FMath::Clamp(HeroBaseAssassinateChance, 0.f, 1.f);
	case ET66SecondaryStatType::SpinWheel:       return 1.f;
	case ET66SecondaryStatType::Goblin:          return 1.f;
	case ET66SecondaryStatType::Leprechaun:      return 1.f;
	case ET66SecondaryStatType::TreasureChest:   return 1.f;
	case ET66SecondaryStatType::Fountain:        return 1.f;
	case ET66SecondaryStatType::Cheating:        return FMath::Clamp(HeroBaseCheatChance, 0.f, 1.f);
	case ET66SecondaryStatType::Stealing:        return FMath::Clamp(HeroBaseStealChance, 0.f, 1.f);
	case ET66SecondaryStatType::MovementSpeed:   return 1.f;
	case ET66SecondaryStatType::LootCrate:       return 1.f;
	case ET66SecondaryStatType::DamageReduction: return 0.f;
	case ET66SecondaryStatType::EvasionChance:   return 0.f;
	case ET66SecondaryStatType::Alchemy:         return 0.f;
	default:                                     return 1.f;
	}
}

float UT66RunStateSubsystem::GetAggroMultiplier() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::Taunt);
}

float UT66RunStateSubsystem::GetHpRegenPerSecond() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::HpRegen);
}

float UT66RunStateSubsystem::GetMovementSpeedSecondaryMultiplier() const
{
	return 1.f;
}

float UT66RunStateSubsystem::GetLootCrateRewardMultiplier() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::LootCrate);
}

float UT66RunStateSubsystem::GetAlchemyLuckyUpgradeChance01() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::Alchemy);
}

float UT66RunStateSubsystem::GetCritChance01() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::CritChance);
}

float UT66RunStateSubsystem::GetCritDamageMultiplier() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::CritDamage);
}

float UT66RunStateSubsystem::GetLifeStealFraction() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::LifeSteal);
}

float UT66RunStateSubsystem::GetReflectDamageFraction() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::ReflectDamage);
}

float UT66RunStateSubsystem::GetCrushChance01() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::Crush);
}

float UT66RunStateSubsystem::GetAssassinateChance01() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::Assassinate);
}

float UT66RunStateSubsystem::GetInvisibilityChance01() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::Invisibility);
}

float UT66RunStateSubsystem::GetCounterAttackFraction() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::CounterAttack);
}

float UT66RunStateSubsystem::GetCloseRangeThreshold() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::AttackRange) * 0.10f;
}

float UT66RunStateSubsystem::GetLongRangeThreshold() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::AttackRange) * 0.90f;
}

float UT66RunStateSubsystem::GetCloseRangeDamageMultiplier() const
{
	return 1.f;
}

float UT66RunStateSubsystem::GetLongRangeDamageMultiplier() const
{
	return 1.f;
}

float UT66RunStateSubsystem::GetHeroMoveSpeedMultiplier() const
{
	// Mapping is driven by the foundational Speed stat (not shown in the HUD stat panel).
	// Keep it modest because Speed gains are +1 per level.
	const float S = TenthsToFloatStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::Speed));
	return 1.f + FMath::Max(0.f, S - 1.f) * 0.01f;
}

float UT66RunStateSubsystem::GetHeroDamageMultiplier() const
{
	const float D = TenthsToFloatStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::Damage));
	return 1.f + FMath::Max(0.f, D - 1.f) * 0.015f;
}

float UT66RunStateSubsystem::GetHeroAttackSpeedMultiplier() const
{
	const float AttackSpeedStat = TenthsToFloatStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::AttackSpeed));
	return 1.f + FMath::Max(0.f, AttackSpeedStat - 1.f) * 0.012f;
}

float UT66RunStateSubsystem::GetHeroScaleMultiplier() const
{
	const float ScaleStat = TenthsToFloatStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::AttackScale));
	return 1.f + FMath::Max(0.f, ScaleStat - 1.f) * 0.008f;
}

float UT66RunStateSubsystem::GetHeroAccuracyMultiplier() const
{
	const float AccuracyStat = TenthsToFloatStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::Accuracy));
	return 1.f + FMath::Max(0.f, AccuracyStat - 1.f) * 0.010f;
}

float UT66RunStateSubsystem::GetArmorReduction01() const
{
	const float ArmorStat = TenthsToFloatStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::Armor));
	const float Base = FMath::Max(0.f, ArmorStat - 1.f) * 0.008f;
	const float Bonus = GetSecondaryStatValue(ET66SecondaryStatType::DamageReduction);
	return FMath::Clamp(Base + Bonus + ItemArmorBonus01, 0.f, 0.80f);
}

float UT66RunStateSubsystem::GetEvasionChance01() const
{
	const float EvasionStat = TenthsToFloatStat(GetPrecisePrimaryStatTenths(ET66HeroStatType::Evasion));
	const float Base = FMath::Max(0.f, EvasionStat - 1.f) * 0.006f;
	const float Bonus = GetSecondaryStatValue(ET66SecondaryStatType::EvasionChance);
	return FMath::Clamp(Base + Bonus + ItemEvasionBonus01, 0.f, 0.60f);
}

float UT66RunStateSubsystem::GetAccuracyChance01() const
{
	const float PassiveBonus = (PassiveType == ET66PassiveType::Headshot) ? 0.20f : 0.f;
	return FMath::Clamp(GetSecondaryStatValue(ET66SecondaryStatType::Accuracy) + PassiveBonus, 0.f, 0.95f);
}

void UT66RunStateSubsystem::NotifyEnemyKilledByHero()
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return;
	const double Now = World->GetTimeSeconds();

	if (PassiveType == ET66PassiveType::RallyingBlow)
	{
		RallyStacks = FMath::Min(3, RallyStacks + 1);
		RallyTimerEndWorldTime = Now + 3.0;
	}

	if (PassiveType == ET66PassiveType::ChaosTheory)
	{
		ChaosTheoryBounceStacks = FMath::Min(3, ChaosTheoryBounceStacks + 1);
		ChaosTheoryTimerEndWorldTime = Now + 5.0;
	}
}

float UT66RunStateSubsystem::GetRallyAttackSpeedMultiplier() const
{
	if (PassiveType != ET66PassiveType::RallyingBlow || RallyStacks <= 0) return 1.f;
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World || World->GetTimeSeconds() >= RallyTimerEndWorldTime) return 1.f;
	return 1.f + 0.15f * static_cast<float>(RallyStacks);
}

bool UT66RunStateSubsystem::HasActiveDOT(AActor* Target) const
{
	if (!Target) return false;
	const TWeakObjectPtr<AActor> Key(Target);
	for (const auto& Pair : ActiveDOTs)
	{
		if (Pair.Key.Target == Key && Pair.Value.RemainingDuration > 0.f)
		{
			return true;
		}
	}
	return false;
}

float UT66RunStateSubsystem::GetToxinStackingDamageMultiplier(AActor* Target) const
{
	if (PassiveType != ET66PassiveType::ToxinStacking || !Target) return 1.f;
	return HasActiveDOT(Target) ? 1.15f : 1.f;
}

// ---------------------------------------------------------------------------
// New passive implementations
// ---------------------------------------------------------------------------

float UT66RunStateSubsystem::GetQuickDrawDamageMultiplier() const
{
	if (PassiveType != ET66PassiveType::QuickDraw) return 1.f;
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return 1.f;
	const double Now = World->GetTimeSeconds();
	return (Now - LastAttackFireWorldTime >= 2.0) ? 2.f : 1.f;
}

void UT66RunStateSubsystem::NotifyAttackFired()
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return;
	LastAttackFireWorldTime = World->GetTimeSeconds();
	if (PassiveType == ET66PassiveType::Overclock)
		OverclockAttackCounter++;
}

bool UT66RunStateSubsystem::ShouldOverclockDouble() const
{
	if (PassiveType != ET66PassiveType::Overclock) return false;
	return (OverclockAttackCounter % 8) == 0 && OverclockAttackCounter > 0;
}

int32 UT66RunStateSubsystem::GetChaosTheoryBonusBounceCount() const
{
	if (PassiveType != ET66PassiveType::ChaosTheory || ChaosTheoryBounceStacks <= 0) return 0;
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World || World->GetTimeSeconds() >= ChaosTheoryTimerEndWorldTime) return 0;
	return ChaosTheoryBounceStacks;
}

float UT66RunStateSubsystem::GetEnduranceAttackSpeedMultiplier() const
{
	if (PassiveType != ET66PassiveType::Endurance) return 1.f;
	return (CurrentHP > 0.f && CurrentHP <= MaxHP * 0.3f) ? 2.f : 1.f;
}

float UT66RunStateSubsystem::GetEnduranceDamageMultiplier() const
{
	if (PassiveType != ET66PassiveType::Endurance) return 1.f;
	return (CurrentHP > 0.f && CurrentHP <= MaxHP * 0.3f) ? 1.25f : 1.f;
}

float UT66RunStateSubsystem::GetBrawlersFuryDamageMultiplier() const
{
	if (PassiveType != ET66PassiveType::BrawlersFury) return 1.f;
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return 1.f;
	return (World->GetTimeSeconds() < BrawlersFuryEndWorldTime) ? 1.3f : 1.f;
}

float UT66RunStateSubsystem::GetTreasureHunterGoldMultiplier() const
{
	return (PassiveType == ET66PassiveType::TreasureHunter) ? 1.25f : 1.f;
}

void UT66RunStateSubsystem::NotifyEvasionProc()
{
	if (PassiveType == ET66PassiveType::Evasive)
		bEvasiveNextAttackBonusDOT = true;
}

bool UT66RunStateSubsystem::ConsumeEvasiveBonusDOT()
{
	if (PassiveType != ET66PassiveType::Evasive || !bEvasiveNextAttackBonusDOT) return false;
	bEvasiveNextAttackBonusDOT = false;
	return true;
}

void UT66RunStateSubsystem::AddHeroXP(int32 Amount)
{
	if (Amount <= 0) return;
	if (HeroLevel <= 0) HeroLevel = DefaultHeroLevel;
	if (XPToNextLevel <= 0) XPToNextLevel = DefaultXPToLevel;
	HeroLevel = FMath::Clamp(HeroLevel, DefaultHeroLevel, MaxHeroLevel);
	if (HeroLevel >= MaxHeroLevel)
	{
		HeroXP = 0;
		HeroProgressChanged.Broadcast();
		return;
	}

	HeroXP = FMath::Clamp(HeroXP + Amount, 0, 1000000000);
	bool bLeveled = false;
	int32 LevelsGained = 0;
	while (HeroXP >= XPToNextLevel && XPToNextLevel > 0 && HeroLevel < MaxHeroLevel)
	{
		HeroXP -= XPToNextLevel;
		HeroLevel = FMath::Clamp(HeroLevel + 1, DefaultHeroLevel, MaxHeroLevel);
		ApplyOneHeroLevelUp();
		bLeveled = true;
		++LevelsGained;
	}
	if (HeroLevel >= MaxHeroLevel)
	{
		HeroXP = 0;
	}
	if (bLeveled)
	{
		HealToFull();
	}
	HeroProgressChanged.Broadcast();
	if (bLeveled)
	{
		LogAdded.Broadcast();
		UGameInstance* GI = GetGameInstance();
		UWorld* World = GI ? GI->GetWorld() : nullptr;
		APawn* HeroPawn = World && World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
		if (HeroPawn && World)
		{
			T66SpawnLevelUpBifrost(World, HeroPawn->GetActorLocation());
			T66KillEnemiesNearPoint(World, HeroPawn->GetActorLocation(), 375.f);
		}
		if (HeroPawn && GI)
		{
			if (UT66FloatingCombatTextSubsystem* FCT = GI->GetSubsystem<UT66FloatingCombatTextSubsystem>())
			{
				for (int32 LevelUpIndex = 0; LevelUpIndex < FMath::Max(1, LevelsGained); ++LevelUpIndex)
				{
					FCT->ShowStatusEvent(HeroPawn, UT66FloatingCombatTextSubsystem::EventType_LevelUp);
				}
			}
		}
	}
}

void UT66RunStateSubsystem::ResetVendorForStage()
{
	VendorAngerGold = 0;
	VendorStockStage = 0;
	VendorStockItemIDs.Reset();
	VendorStockSlots.Reset();
	VendorStockSold.Reset();
	bVendorBoughtSomethingThisStage = false;
	VendorChanged.Broadcast();
}

void UT66RunStateSubsystem::ResetVendorAnger()
{
	ResetGamblerAnger();
}

void UT66RunStateSubsystem::SetInStageCatchUp(bool bInCatchUp)
{
	if (bInStageCatchUp == bInCatchUp) return;
	bInStageCatchUp = bInCatchUp;
	StageChanged.Broadcast();
}

void UT66RunStateSubsystem::ApplyStageSpeedBoost(float MoveSpeedMultiplier, float DurationSeconds)
{
	const float Mult = FMath::Clamp(MoveSpeedMultiplier, 0.25f, 5.f);
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 30.f);
	if (Dur <= 0.f) return;

	StageMoveSpeedMultiplier = FMath::Max(StageMoveSpeedMultiplier, Mult);
	StageMoveSpeedSecondsRemaining = FMath::Max(StageMoveSpeedSecondsRemaining, Dur);
	HeroProgressChanged.Broadcast(); // movement uses derived stat refresh
}

bool UT66RunStateSubsystem::ApplyTrueDamage(int32 /*DamageHP*/)
{
	return false;
}

void UT66RunStateSubsystem::ApplyStatusBurn(float /*DurationSeconds*/, float /*DamagePerSecond*/) {}
void UT66RunStateSubsystem::ApplyStatusChill(float /*DurationSeconds*/, float /*MoveSpeedMultiplier*/) {}
void UT66RunStateSubsystem::ApplyStatusCurse(float /*DurationSeconds*/) {}

void UT66RunStateSubsystem::EnsureVendorStockForCurrentStage()
{
	const int32 Stage = FMath::Clamp(CurrentStage, 1, 23);
	if (VendorStockStage == Stage && VendorStockItemIDs.Num() > 0 && VendorStockSold.Num() == VendorStockItemIDs.Num())
	{
		return;
	}

	// Reset reroll counter and seen-counts when stage changes.
	if (VendorStockRerollStage != Stage)
	{
		VendorStockRerollStage = Stage;
		VendorStockRerollCounter = 0;
		VendorSeenCounts.Reset();
	}

	VendorStockStage = Stage;
	VendorStockItemIDs.Reset();
	VendorStockSold.Reset();
	VendorStockSlots.Reset();

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI)
	{
		// Fallback: keep deterministic placeholder behavior with the live slot count.
		const FT66InventorySlot FallbackStock[] =
		{
			FT66InventorySlot(FName(TEXT("Item_AoeDamage")), ET66ItemRarity::Black, 2),
			FT66InventorySlot(FName(TEXT("Item_BounceScale")), ET66ItemRarity::Black, 2),
			FT66InventorySlot(FName(TEXT("Item_DamageReduction")), ET66ItemRarity::Black, 2),
			FT66InventorySlot(FName(TEXT("Item_CritDamage")), ET66ItemRarity::Red, 5),
			FT66InventorySlot(FName(TEXT("Item_Accuracy")), ET66ItemRarity::Yellow, 8),
		};
		for (const FT66InventorySlot& Slot : FallbackStock)
		{
			VendorStockSlots.Add(Slot);
			VendorStockItemIDs.Add(Slot.ItemTemplateID);
		}
		VendorStockSold.Init(false, VendorStockSlots.Num());
		VendorChanged.Broadcast();
		return;
	}

	// Build pool of all template IDs from the Items DataTable.
	UDataTable* ItemsDT = GI->GetItemsDataTable();
	TArray<FName> TemplatePool;
	if (ItemsDT)
	{
		for (const FName ItemID : ItemsDT->GetRowNames())
		{
			FItemData ItemData;
			if (!GI->GetItemData(ItemID, ItemData))
			{
				continue;
			}

			if (!T66_IsGamblersTokenItem(ItemID) && T66IsLiveSecondaryStatType(ItemData.SecondaryStatType))
			{
				TemplatePool.Add(ItemID);
			}
		}
	}

	{
		FItemData AccuracyItemData;
		const FName AccuracyItemID(TEXT("Item_Accuracy"));
		if (GI->GetItemData(AccuracyItemID, AccuracyItemData)
			&& T66IsLiveSecondaryStatType(AccuracyItemData.SecondaryStatType)
			&& !TemplatePool.Contains(AccuracyItemID))
		{
			TemplatePool.Add(AccuracyItemID);
		}
	}

	if (TemplatePool.Num() == 0)
	{
		TemplatePool =
		{
			FName(TEXT("Item_AoeDamage")),
			FName(TEXT("Item_BounceScale")),
			FName(TEXT("Item_DamageReduction")),
			FName(TEXT("Item_CritDamage")),
			FName(TEXT("Item_Accuracy"))
		};
	}

	// Seed: per-stage and per-reroll, plus run seed so the first shop display is randomized each run.
	int32 Seed = Stage * 777 + 13 + VendorStockRerollCounter * 10007;
	if (UT66RngSubsystem* RngSub = GI->GetSubsystem<UT66RngSubsystem>())
	{
		Seed ^= RngSub->GetRunSeed();
	}
	FRandomStream Rng(Seed);

	// Smart reroll: weight = 1/(1 + SeenCount*Decay), floor 0.05. Build weights and pick unique cards.
	constexpr float DecayFactor = 2.0f;
	constexpr float WeightFloor = 0.05f;
	TArray<float> Weights;
	Weights.SetNumUninitialized(TemplatePool.Num());
	for (int32 p = 0; p < TemplatePool.Num(); ++p)
	{
		const int32 Seen = VendorSeenCounts.FindRef(TemplatePool[p]);
		Weights[p] = FMath::Max(WeightFloor, 1.0f / (1.0f + static_cast<float>(Seen) * DecayFactor));
	}

	const ET66ItemRarity SlotRarities[VendorDisplaySlotCount] =
	{
		ET66ItemRarity::Black,
		ET66ItemRarity::Black,
		ET66ItemRarity::Black,
		ET66ItemRarity::Red,
		ET66ItemRarity::Yellow
	};

	for (int32 i = 0; i < VendorDisplaySlotCount; ++i)
	{
		float TotalWeight = 0.f;
		for (int32 p = 0; p < TemplatePool.Num(); ++p)
		{
			if (!VendorStockItemIDs.Contains(TemplatePool[p]))
				TotalWeight += Weights[p];
		}
		if (TotalWeight <= 0.f) TotalWeight = 1.f;

		FName Chosen = NAME_None;
		float Roll = Rng.FRand() * TotalWeight;
		for (int32 p = 0; p < TemplatePool.Num(); ++p)
		{
			if (VendorStockItemIDs.Contains(TemplatePool[p])) continue;
			Roll -= Weights[p];
			if (Roll <= 0.f) { Chosen = TemplatePool[p]; break; }
		}
		if (Chosen.IsNone())
		{
			for (int32 p = 0; p < TemplatePool.Num(); ++p)
				if (!VendorStockItemIDs.Contains(TemplatePool[p])) { Chosen = TemplatePool[p]; break; }
		}
		if (Chosen.IsNone()) Chosen = TemplatePool[0];

		VendorSeenCounts.FindOrAdd(Chosen)++;

		const ET66ItemRarity Rar = SlotRarities[i];
		int32 RollMin = 1, RollMax = 3;
		FItemData::GetLine1RollRange(Rar, RollMin, RollMax);
		const int32 Rolled = Rng.RandRange(RollMin, RollMax);

		VendorStockSlots.Add(FT66InventorySlot(Chosen, Rar, Rolled));
		VendorStockItemIDs.Add(Chosen);
	}

	VendorStockSold.Init(false, VendorStockSlots.Num());
	VendorChanged.Broadcast();
}

void UT66RunStateSubsystem::RerollVendorStockForCurrentStage()
{
	const int32 Stage = FMath::Clamp(CurrentStage, 1, 23);
	if (VendorStockRerollStage != Stage)
	{
		VendorStockRerollStage = Stage;
		VendorStockRerollCounter = 0;
	}
	VendorStockRerollCounter = FMath::Clamp(VendorStockRerollCounter + 1, 0, 9999);

	// Force regeneration even if the stage didn't change.
	VendorStockStage = 0;
	VendorStockItemIDs.Reset();
	VendorStockSold.Reset();
	EnsureVendorStockForCurrentStage();
	// EnsureVendorStockForCurrentStage broadcasts VendorChanged.
}

bool UT66RunStateSubsystem::IsVendorStockSlotSold(int32 Index) const
{
	if (Index < 0 || Index >= VendorStockSold.Num()) return true;
	return VendorStockSold[Index];
}

bool UT66RunStateSubsystem::TryBuyVendorStockSlot(int32 Index)
{
	EnsureVendorStockForCurrentStage();
	if (Index < 0 || Index >= VendorStockSlots.Num()) return false;
	if (IsVendorStockSlotSold(Index)) return false;
	if (!HasInventorySpace()) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	FItemData D;
	const FT66InventorySlot& Slot = VendorStockSlots[Index];
	if (!GI || !GI->GetItemData(Slot.ItemTemplateID, D)) return false;
	const int32 BuyPrice = D.GetBuyGoldForRarity(Slot.Rarity);
	if (BuyPrice <= 0) return false;
	if (!TrySpendGold(BuyPrice)) return false;

	AddItemSlot(Slot);
	VendorStockSold[Index] = true;
	bVendorBoughtSomethingThisStage = true;
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("VendorPurchase=%s"), *Slot.ItemTemplateID.ToString()));
	VendorChanged.Broadcast();
	return true;
}

bool UT66RunStateSubsystem::ResolveVendorStealAttempt(int32 Index, bool bTimingHit, bool bRngSuccess)
{
	(void)bRngSuccess;
	EnsureVendorStockForCurrentStage();
	if (Index < 0 || Index >= VendorStockSlots.Num()) return false;
	if (IsVendorStockSlotSold(Index)) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	FItemData D;
	const FT66InventorySlot& StealSlot = VendorStockSlots[Index];
	if (!GI || !GI->GetItemData(StealSlot.ItemTemplateID, D)) return false;
	const int32 BuyPrice = D.GetBuyGoldForRarity(StealSlot.Rarity);
	if (BuyPrice <= 0) return false;

	// Determine success via player-experience tuning and central luck bias.
	UT66PlayerExperienceSubSystem* PlayerExperience = nullptr;
	UT66RngSubsystem* RngSub = nullptr;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		PlayerExperience = GameInstance->GetSubsystem<UT66PlayerExperienceSubSystem>();
		RngSub = GameInstance->GetSubsystem<UT66RngSubsystem>();
		if (RngSub)
		{
			RngSub->UpdateLuckStat(GetLuckStat());
		}
	}

	float BaseChance = 0.f;
	if (bTimingHit)
	{
		const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
		const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
		BaseChance = PlayerExperience
			? PlayerExperience->GetDifficultyVendorStealSuccessChanceOnTimingHitBase(Difficulty)
			: 0.65f;
	}
	BaseChance = FMath::Clamp(BaseChance, 0.f, 1.f);

	bool bSuccess = false;
	float AppliedChance = 0.f;
	int32 DrawIndex = INDEX_NONE;
	int32 PreDrawSeed = 0;
	if (bTimingHit && BaseChance > 0.f)
	{
		const float Chance = RngSub ? RngSub->BiasChance01(BaseChance) : BaseChance;
		AppliedChance = Chance;
		FRandomStream Local(FPlatformTime::Cycles());
		FRandomStream& Stream = RngSub ? RngSub->GetRunStream() : Local;
		if (RngSub)
		{
			bSuccess = RngSub->RollChance01(Chance);
			DrawIndex = RngSub->GetLastRunDrawIndex();
			PreDrawSeed = RngSub->GetLastRunPreDrawSeed();
		}
		else
		{
			bSuccess = (Stream.GetFraction() < Chance);
		}
	}

	LastVendorStealOutcome = ET66VendorStealOutcome::None;
	if (!bTimingHit)
	{
		LastVendorStealOutcome = ET66VendorStealOutcome::Miss;
	}
	else if (!bSuccess)
	{
		LastVendorStealOutcome = ET66VendorStealOutcome::Failed;
	}
	else if (!HasInventorySpace())
	{
		LastVendorStealOutcome = ET66VendorStealOutcome::InventoryFull;
	}
	else
	{
		LastVendorStealOutcome = ET66VendorStealOutcome::Success;
	}

	bool bGranted = false;
	if (LastVendorStealOutcome == ET66VendorStealOutcome::Success)
	{
		AddItemSlot(StealSlot);
		VendorStockSold[Index] = true;
		AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("VendorSteal=%s"), *StealSlot.ItemTemplateID.ToString()));
		bGranted = true;
		// Success: no anger increase.
	}
	else
	{
		// Failure: anger increases and the item is not granted.
		AddCasinoAngerFromGold(BuyPrice);
	}

	// Luck Rating tracking (quantity): vendor steal success means item granted with no anger increase.
	RecordLuckQuantityBool(
		FName(TEXT("VendorStealSuccess")),
		(LastVendorStealOutcome == ET66VendorStealOutcome::Success),
		AppliedChance,
		DrawIndex,
		PreDrawSeed);

	if (LastVendorStealOutcome == ET66VendorStealOutcome::Success)
	{
		VendorChanged.Broadcast();
	}
	return bGranted;
}

void UT66RunStateSubsystem::GenerateBuybackDisplay()
{
	BuybackDisplaySlots.SetNum(BuybackDisplaySlotCount);
	const int32 PoolNum = BuybackPool.Num();
	const int32 MaxPage = PoolNum > 0 ? FMath::Max(0, (PoolNum + BuybackDisplaySlotCount - 1) / BuybackDisplaySlotCount - 1) : 0;
	BuybackDisplayPage = FMath::Clamp(BuybackDisplayPage, 0, MaxPage);
	const int32 Start = BuybackDisplayPage * BuybackDisplaySlotCount;
	for (int32 i = 0; i < BuybackDisplaySlotCount; ++i)
	{
		const int32 Idx = Start + i;
		if (Idx < PoolNum)
		{
			BuybackDisplaySlots[i] = BuybackPool[Idx];
		}
		else
		{
			BuybackDisplaySlots[i] = FT66InventorySlot();
		}
	}
	BuybackChanged.Broadcast();
}

void UT66RunStateSubsystem::RerollBuybackDisplay()
{
	const int32 PoolNum = BuybackPool.Num();
	const int32 MaxPage = PoolNum > 0 ? FMath::Max(0, (PoolNum + BuybackDisplaySlotCount - 1) / BuybackDisplaySlotCount - 1) : 0;
	if (MaxPage > 0)
	{
		BuybackDisplayPage = (BuybackDisplayPage + 1) % (MaxPage + 1);
	}
	GenerateBuybackDisplay();
}

bool UT66RunStateSubsystem::TryBuybackSlot(int32 DisplayIndex)
{
	if (DisplayIndex < 0 || DisplayIndex >= BuybackDisplaySlotCount) return false;
	if (!HasInventorySpace()) return false;

	const int32 PoolIndex = BuybackDisplayPage * BuybackDisplaySlotCount + DisplayIndex;
	if (PoolIndex < 0 || PoolIndex >= BuybackPool.Num()) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI) return false;

	const FT66InventorySlot Slot = BuybackPool[PoolIndex];
	if (!Slot.IsValid()) return false;

	FItemData ItemData;
	int32 BuyPrice = 0;
	if (GI->GetItemData(Slot.ItemTemplateID, ItemData))
	{
		BuyPrice = GetSellGoldForInventorySlot(Slot);
	}
	if (BuyPrice <= 0) BuyPrice = 1;
	if (CurrentGold < BuyPrice) return false;

	CurrentGold -= BuyPrice;
	BuybackPool.RemoveAt(PoolIndex);
	AddItemSlot(Slot);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=-%d,Source=Buyback,ItemID=%s"), BuyPrice, *Slot.ItemTemplateID.ToString()));
	GoldChanged.Broadcast();
	InventoryChanged.Broadcast();
	GenerateBuybackDisplay();
	return true;
}

bool UT66RunStateSubsystem::TryActivateUltimate()
{
	if (UltimateCooldownRemainingSeconds > 0.f) return false;
	UltimateCooldownRemainingSeconds = UltimateCooldownSeconds;
	LastBroadcastUltimateSecond = FMath::CeilToInt(UltimateCooldownRemainingSeconds);
	UltimateChanged.Broadcast();
	return true;
}

void UT66RunStateSubsystem::TickHeroTimers(float DeltaTime)
{
	// HP regen (numerical)
	ApplyHpRegen(DeltaTime);

	// Ultimate cooldown
	if (UltimateCooldownRemainingSeconds > 0.f)
	{
		UltimateCooldownRemainingSeconds = FMath::Max(0.f, UltimateCooldownRemainingSeconds - DeltaTime);
		const int32 Cur = FMath::CeilToInt(UltimateCooldownRemainingSeconds);
		if (Cur != LastBroadcastUltimateSecond)
		{
			LastBroadcastUltimateSecond = Cur;
			UltimateChanged.Broadcast();
		}
	}

	// Last stand timer
	if (bInLastStand && LastStandSecondsRemaining > 0.f)
	{
		LastStandSecondsRemaining = FMath::Max(0.f, LastStandSecondsRemaining - DeltaTime);
		const int32 Cur = FMath::CeilToInt(LastStandSecondsRemaining);
		if (Cur != LastBroadcastLastStandSecond)
		{
			LastBroadcastLastStandSecond = Cur;
			SurvivalChanged.Broadcast();
		}
		if (LastStandSecondsRemaining <= 0.f)
		{
			EndLastStandAndDie();
		}
	}

	if (bInQuickReviveDowned && QuickReviveDownedSecondsRemaining > 0.f)
	{
		QuickReviveDownedSecondsRemaining = FMath::Max(0.f, QuickReviveDownedSecondsRemaining - DeltaTime);
		const int32 Cur = FMath::CeilToInt(QuickReviveDownedSecondsRemaining);
		if (Cur != LastBroadcastQuickReviveSecond)
		{
			LastBroadcastQuickReviveSecond = Cur;
			QuickReviveChanged.Broadcast();
		}
		if (QuickReviveDownedSecondsRemaining <= 0.f)
		{
			EndQuickReviveDownedAndRevive();
		}
	}

	// Stage speed boost timer
	if (StageMoveSpeedSecondsRemaining > 0.f)
	{
		StageMoveSpeedSecondsRemaining = FMath::Max(0.f, StageMoveSpeedSecondsRemaining - DeltaTime);
		if (StageMoveSpeedSecondsRemaining <= 0.f)
		{
			StageMoveSpeedMultiplier = 1.f;
			HeroProgressChanged.Broadcast();
		}
	}

	// Status effects removed — enemies no longer apply Burn/Chill/Curse.
}

bool UT66RunStateSubsystem::GrantQuickReviveCharge()
{
	if (bQuickReviveChargeReady || bInQuickReviveDowned)
	{
		return false;
	}

	bQuickReviveChargeReady = true;
	QuickReviveChanged.Broadcast();
	return true;
}

void UT66RunStateSubsystem::ClearQuickReviveCharge()
{
	if (!bQuickReviveChargeReady)
	{
		return;
	}

	bQuickReviveChargeReady = false;
	QuickReviveChanged.Broadcast();
}

bool UT66RunStateSubsystem::IsBossDamageSource(const AActor* Attacker)
{
	return Cast<AT66BossBase>(Attacker) != nullptr;
}

void UT66RunStateSubsystem::HandleLethalDamage(AActor* Attacker)
{
	if (bInQuickReviveDowned)
	{
		EndQuickReviveDownedAndDie();
		return;
	}

	if (bInLastStand)
	{
		EndLastStandAndDie();
		return;
	}

	// Dev Immortality: never end the run.
	if (bDevImmortality)
	{
		CurrentHP = 0.f;
		LastDamageTime = -9999.f;
		HeartsChanged.Broadcast();
		return;
	}

	if (bQuickReviveChargeReady)
	{
		EnterQuickReviveDowned();
		return;
	}

	// If charged, trigger last-stand instead of dying.
	if (SurvivalCharge01 >= 1.f)
	{
		EnterLastStand();
		return;
	}

	CurrentHP = 0.f;
	LastDamageTime = -9999.f;
	HeartsChanged.Broadcast();
	OnPlayerDied.Broadcast();
}

void UT66RunStateSubsystem::EnterLastStand()
{
	bInLastStand = true;
	LastStandSecondsRemaining = LastStandDurationSeconds;
	LastBroadcastLastStandSecond = FMath::CeilToInt(LastStandSecondsRemaining);

	// Consume the charge.
	SurvivalCharge01 = 0.f;

	// HP stays at 0, but the run continues.
	HeartsChanged.Broadcast();
	SurvivalChanged.Broadcast();
}

void UT66RunStateSubsystem::EnterQuickReviveDowned()
{
	bQuickReviveChargeReady = false;
	bInQuickReviveDowned = true;
	QuickReviveDownedSecondsRemaining = QuickReviveDownedDurationSeconds;
	LastBroadcastQuickReviveSecond = FMath::CeilToInt(QuickReviveDownedSecondsRemaining);
	CurrentHP = 0.f;
	LastDamageTime = -9999.f;

	if (UWorld* World = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			for (const TWeakObjectPtr<AT66EnemyBase>& WeakEnemy : Registry->GetEnemies())
			{
				if (AT66EnemyBase* Enemy = WeakEnemy.Get())
				{
					Enemy->ApplyForcedRunAway(QuickReviveDownedDurationSeconds);
				}
			}
		}
	}

	HeartsChanged.Broadcast();
	QuickReviveChanged.Broadcast();
}

void UT66RunStateSubsystem::EndQuickReviveDownedAndRevive()
{
	if (!bInQuickReviveDowned)
	{
		return;
	}

	bInQuickReviveDowned = false;
	QuickReviveDownedSecondsRemaining = 0.f;
	LastBroadcastQuickReviveSecond = 0;

	if (HeartSlotTiers.Num() != DefaultMaxHearts)
	{
		RebuildHeartSlotTiersFromMaxHP();
	}

	CurrentHP = FMath::Clamp(GetHeartSlotCapacity(0), 1.f, MaxHP);

	if (UWorld* World = GetWorld())
	{
		LastDamageTime = static_cast<float>(World->GetTimeSeconds());
	}
	else
	{
		LastDamageTime = 0.f;
	}

	HeartsChanged.Broadcast();
	QuickReviveChanged.Broadcast();
}

void UT66RunStateSubsystem::EndQuickReviveDownedAndDie()
{
	if (!bInQuickReviveDowned)
	{
		return;
	}

	bInQuickReviveDowned = false;
	QuickReviveDownedSecondsRemaining = 0.f;
	LastBroadcastQuickReviveSecond = 0;
	QuickReviveChanged.Broadcast();

	if (bDevImmortality)
	{
		CurrentHP = 0.f;
		HeartsChanged.Broadcast();
		return;
	}

	CurrentHP = 0.f;
	LastDamageTime = -9999.f;
	HeartsChanged.Broadcast();
	OnPlayerDied.Broadcast();
}

void UT66RunStateSubsystem::EndLastStandAndDie()
{
	bInLastStand = false;
	LastStandSecondsRemaining = 0.f;
	LastBroadcastLastStandSecond = 0;
	SurvivalChanged.Broadcast();

	// Dev Immortality: never end the run.
	if (bDevImmortality)
	{
		CurrentHP = 0.f;
		HeartsChanged.Broadcast();
		return;
	}

	// Die as normal now.
	CurrentHP = 0.f;
	HeartsChanged.Broadcast();
	OnPlayerDied.Broadcast();
}

bool UT66RunStateSubsystem::ApplyDamage(int32 DamageHP, AActor* Attacker)
{
	if (DamageHP <= 0) return false;

	if (bSaintBlessingActive)
	{
		return false;
	}

	// If we're in last-stand, we ignore damage (invincible).
	if (bInLastStand) return false;

	// Quick Revive downed state only ends if a boss lands the finishing hit.
	if (bInQuickReviveDowned)
	{
		if (IsBossDamageSource(Attacker))
		{
			EndQuickReviveDownedAndDie();
			return true;
		}
		return false;
	}

	// Iron Will: flat damage reduction before armor.
	if (PassiveType == ET66PassiveType::IronWill)
	{
		const int32 FlatReduction = GetArmorStat() * 2;
		DamageHP = FMath::Max(1, DamageHP - FlatReduction);
	}

	// Unflinching: permanent 15% damage reduction.
	if (PassiveType == ET66PassiveType::Unflinching)
	{
		DamageHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(DamageHP) * 0.85f));
	}

	// Evasion: dodge the entire hit. On dodge: Assassinate (OHKO) and CounterAttack (deal fraction of would-be damage to attacker).
	UT66RngSubsystem* RngSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RngSubsystem>() : nullptr;
	const float Evade = GetEvasionChance01();
	AntiCheatIncomingHitChecks = FMath::Clamp(AntiCheatIncomingHitChecks + 1, 0, 1000000);
	AntiCheatTotalEvasionChance = FMath::Clamp(AntiCheatTotalEvasionChance + FMath::Clamp(Evade, 0.f, 1.f), 0.f, 1000000.f);
	const bool bDodged = Evade > 0.f && (RngSub ? (RngSub->GetRunStream().GetFraction() < Evade) : (FMath::FRand() < Evade));
	if (bDodged)
	{
		RecordAntiCheatHitCheckEvent(Evade, true, false);
		AntiCheatDodgeCount = FMath::Clamp(AntiCheatDodgeCount + 1, 0, 1000000);
		AntiCheatCurrentConsecutiveDodges = FMath::Clamp(AntiCheatCurrentConsecutiveDodges + 1, 0, 1000000);
		AntiCheatMaxConsecutiveDodges = FMath::Max(AntiCheatMaxConsecutiveDodges, AntiCheatCurrentConsecutiveDodges);
		UWorld* DodgeWorld = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
		if (DodgeWorld)
		{
			if (APawn* HeroPawn = DodgeWorld->GetFirstPlayerController() ? DodgeWorld->GetFirstPlayerController()->GetPawn() : nullptr)
			{
				if (UT66FloatingCombatTextSubsystem* FloatingText = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66FloatingCombatTextSubsystem>() : nullptr)
				{
					FloatingText->ShowStatusEvent(HeroPawn, UT66FloatingCombatTextSubsystem::EventType_Dodge);
				}
			}
		}
		if (Attacker)
		{
			const float AssassinateChance = GetAssassinateChance01();
			if (AssassinateChance > 0.f && (RngSub ? (RngSub->GetRunStream().GetFraction() < AssassinateChance) : (FMath::FRand() < AssassinateChance)))
			{
				if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Attacker)) { E->TakeDamageFromHero(99999, FName(TEXT("Assassinate")), NAME_None); }
				else if (AT66BossBase* B = Cast<AT66BossBase>(Attacker)) { B->TakeDamageFromHeroHit(99999, FName(TEXT("Assassinate")), NAME_None); }
			}
			const float CounterChance = FMath::Clamp(GetCounterAttackFraction(), 0.f, 1.f);
			if (CounterChance > 0.f && DamageHP > 0 && (RngSub ? (RngSub->GetRunStream().GetFraction() < CounterChance) : (FMath::FRand() < CounterChance)))
			{
				const int32 CounterDmg = FMath::Max(1, FMath::RoundToInt(static_cast<float>(DamageHP) * 0.5f));
				if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Attacker)) { if (E->CurrentHP > 0) E->TakeDamageFromHero(CounterDmg, FName(TEXT("CounterAttack")), NAME_None); }
				else if (AT66GamblerBoss* GB = Cast<AT66GamblerBoss>(Attacker)) { if (GB->CurrentHP > 0) GB->TakeDamageFromHeroHit(CounterDmg, FName(TEXT("CounterAttack")), NAME_None); }
				else if (AT66VendorBoss* VB = Cast<AT66VendorBoss>(Attacker)) { if (VB->CurrentHP > 0) VB->TakeDamageFromHeroHit(CounterDmg, FName(TEXT("CounterAttack")), NAME_None); }
				else if (AT66BossBase* B = Cast<AT66BossBase>(Attacker)) { if (B->IsAwakened() && B->IsAlive()) B->TakeDamageFromHeroHit(CounterDmg, FName(TEXT("CounterAttack")), NAME_None); }
				if (UT66FloatingCombatTextSubsystem* FloatingText = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66FloatingCombatTextSubsystem>() : nullptr)
				{
					FloatingText->ShowStatusEvent(Attacker, UT66FloatingCombatTextSubsystem::EventType_CounterAttack);
				}
			}
		}
		NotifyEvasionProc();
		return false;
	}
	// Armor: reduce the hit (still at least 1 HP if hit > 0).
	const float Armor = GetArmorReduction01();
	const float Reduced = static_cast<float>(FMath::Max(1, FMath::CeilToInt(static_cast<float>(DamageHP) * (1.f - Armor))));

	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	const float Now = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
	if (Now - LastDamageTime < InvulnDurationSeconds)
	{
		RecordAntiCheatHitCheckEvent(Evade, false, false);
		return false;
	}

	// Reflect: % chance to reflect; when it procs, 50% of reduced damage back to the attacker. Crush: chance to OHKO when reflect fires.
	if (Attacker && Reduced > 0.f)
	{
		const float ReflectChance = FMath::Clamp(GetReflectDamageFraction(), 0.f, 1.f);
		if (ReflectChance > 0.f && (RngSub ? (RngSub->GetRunStream().GetFraction() < ReflectChance) : (FMath::FRand() < ReflectChance)))
		{
			const int32 ReflectedAmount = FMath::Max(1, FMath::RoundToInt(Reduced * 0.5f));
			if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Attacker))
			{
				if (E->CurrentHP > 0) E->TakeDamageFromHero(ReflectedAmount, FName(TEXT("Reflect")), NAME_None);
			}
			else if (AT66GamblerBoss* GB = Cast<AT66GamblerBoss>(Attacker))
			{
				if (GB->CurrentHP > 0) GB->TakeDamageFromHeroHit(ReflectedAmount, FName(TEXT("Reflect")), NAME_None);
			}
			else if (AT66VendorBoss* VB = Cast<AT66VendorBoss>(Attacker))
			{
				if (VB->CurrentHP > 0) VB->TakeDamageFromHeroHit(ReflectedAmount, FName(TEXT("Reflect")), NAME_None);
			}
			else if (AT66BossBase* B = Cast<AT66BossBase>(Attacker))
			{
				if (B->IsAwakened() && B->IsAlive()) B->TakeDamageFromHeroHit(ReflectedAmount, FName(TEXT("Reflect")), NAME_None);
			}
			if (UGameInstance* ReflectGI = GetGameInstance())
			{
				if (UT66FloatingCombatTextSubsystem* FloatingText = ReflectGI->GetSubsystem<UT66FloatingCombatTextSubsystem>())
				{
					FloatingText->ShowStatusEvent(Attacker, UT66FloatingCombatTextSubsystem::EventType_Reflect);
				}
			}

			const float CrushChance = GetCrushChance01();
			if (CrushChance > 0.f && (RngSub ? (RngSub->GetRunStream().GetFraction() < CrushChance) : (FMath::FRand() < CrushChance)))
			{
				if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Attacker)) { E->TakeDamageFromHero(99999, FName(TEXT("Crush")), NAME_None); }
				else if (AT66BossBase* B = Cast<AT66BossBase>(Attacker)) { B->TakeDamageFromHeroHit(99999, FName(TEXT("Crush")), NAME_None); }
				if (UGameInstance* CrushGI = GetGameInstance())
				{
						if (UT66FloatingCombatTextSubsystem* CrushFloating = CrushGI->GetSubsystem<UT66FloatingCombatTextSubsystem>())
					{
						CrushFloating->ShowStatusEvent(Attacker, UT66FloatingCombatTextSubsystem::EventType_Crush);
					}
				}
			}
		}
	}

	LastDamageTime = Now;
	CurrentHP = FMath::Max(0.f, CurrentHP - Reduced);
	AntiCheatDamageTakenHitCount = FMath::Clamp(AntiCheatDamageTakenHitCount + 1, 0, 1000000);
	AntiCheatCurrentConsecutiveDodges = 0;
	RecordAntiCheatHitCheckEvent(Evade, false, true);

	// BrawlersFury: taking damage triggers +30% damage dealt for 3s.
	if (PassiveType == ET66PassiveType::BrawlersFury && World)
	{
		BrawlersFuryEndWorldTime = World->GetTimeSeconds() + 3.0;
	}

	if (World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APawn* HeroPawn = PC->GetPawn())
			{
				if (UT66FloatingCombatTextSubsystem* FCT = GI ? GI->GetSubsystem<UT66FloatingCombatTextSubsystem>() : nullptr)
				{
					FCT->ShowDamageTaken(HeroPawn, FMath::RoundToInt(Reduced));
				}
			}
		}
	}

	// Survival charge: 100 HP damage = full (5 hearts * 20 HP). So 1 HP = 0.01 charge.
	SurvivalCharge01 = FMath::Clamp(SurvivalCharge01 + (Reduced / DefaultMaxHP), 0.f, 1.f);
	SurvivalChanged.Broadcast();
	HeartsChanged.Broadcast();

	if (CurrentHP <= 0.f)
	{
		HandleLethalDamage(Attacker);
	}
	return true;
}

void UT66RunStateSubsystem::AddGold(int32 Amount)
{
	if (Amount == 0) return;
	CurrentGold = FMath::Max(0, CurrentGold + Amount);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Gambler"), Amount));
	GoldChanged.Broadcast();
	LogAdded.Broadcast();
}

bool UT66RunStateSubsystem::TrySpendGold(int32 Amount)
{
	if (Amount <= 0) return true;
	if (CurrentGold < Amount) return false;

	CurrentGold = FMath::Max(0, CurrentGold - Amount);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=-%d,Source=Gambler"), Amount));
	GoldChanged.Broadcast();
	LogAdded.Broadcast();
	return true;
}

void UT66RunStateSubsystem::AddOwedBoss(FName BossID)
{
	if (BossID.IsNone()) return;
	OwedBossIDs.Add(BossID);
	AddStructuredEvent(ET66RunEventType::StageExited, FString::Printf(TEXT("OwedBoss=%s"), *BossID.ToString()));
	LogAdded.Broadcast();
}

void UT66RunStateSubsystem::RemoveFirstOwedBoss()
{
	if (OwedBossIDs.Num() <= 0) return;
	OwedBossIDs.RemoveAt(0);
	LogAdded.Broadcast();
}

void UT66RunStateSubsystem::ClearOwedBosses()
{
	if (OwedBossIDs.Num() <= 0) return;
	OwedBossIDs.Empty();
	if (CowardiceGatesTakenCount != 0)
	{
		CowardiceGatesTakenCount = 0;
		CowardiceGatesTakenChanged.Broadcast();
	}
	LogAdded.Broadcast();
}

void UT66RunStateSubsystem::AddCowardiceGateTaken()
{
	CowardiceGatesTakenCount = FMath::Clamp(CowardiceGatesTakenCount + 1, 0, 9999);
	CowardiceGatesTakenChanged.Broadcast();
}

int32 UT66RunStateSubsystem::GetInventorySellValueTotal() const
{
	int32 TotalSellValue = 0;
	for (const FT66InventorySlot& Slot : InventorySlots)
	{
		TotalSellValue += GetSellGoldForInventorySlot(Slot);
	}

	return TotalSellValue;
}

int32 UT66RunStateSubsystem::GetNetWorth() const
{
	return CurrentGold + GetInventorySellValueTotal() - CurrentDebt;
}

int32 UT66RunStateSubsystem::GetRemainingBorrowCapacity() const
{
	return FMath::Max(0, GetNetWorth() - CurrentDebt);
}

bool UT66RunStateSubsystem::CanBorrowGold(int32 Amount) const
{
	return Amount > 0 && Amount <= GetRemainingBorrowCapacity();
}

bool UT66RunStateSubsystem::BorrowGold(int32 Amount)
{
	if (!CanBorrowGold(Amount))
	{
		return false;
	}

	CurrentGold = FMath::Max(0, CurrentGold + Amount);
	CurrentDebt = FMath::Max(0, CurrentDebt + Amount);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Borrow"), Amount));
	GoldChanged.Broadcast();
	DebtChanged.Broadcast();
	LogAdded.Broadcast();
	return true;
}

int32 UT66RunStateSubsystem::PayDebt(int32 Amount)
{
	if (Amount <= 0 || CurrentDebt <= 0 || CurrentGold <= 0) return 0;
	const int32 Pay = FMath::Clamp(Amount, 0, FMath::Min(CurrentDebt, CurrentGold));
	if (Pay <= 0) return 0;

	CurrentGold = FMath::Max(0, CurrentGold - Pay);
	CurrentDebt = FMath::Max(0, CurrentDebt - Pay);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=-%d,Source=PayDebt"), Pay));
	GoldChanged.Broadcast();
	DebtChanged.Broadcast();
	LogAdded.Broadcast();

	// If debt is cleared, make sure a pending loan shark won't spawn.
	if (CurrentDebt <= 0)
	{
		bLoanSharkPending = false;
	}
	return Pay;
}

TArray<FName> UT66RunStateSubsystem::GetInventory() const
{
	TArray<FName> Result;
	Result.Reserve(InventorySlots.Num());
	for (const FT66InventorySlot& Slot : InventorySlots)
	{
		Result.Add(Slot.ItemTemplateID);
	}
	return Result;
}

void UT66RunStateSubsystem::AddItem(FName ItemID)
{
	if (T66_IsGamblersTokenItem(ItemID))
	{
		ApplyGamblersTokenPickup(1);
		return;
	}

	AddItemWithRarity(ItemID, ET66ItemRarity::Black);
}

void UT66RunStateSubsystem::AddItemWithRarity(FName ItemID, ET66ItemRarity Rarity)
{
	if (ItemID.IsNone()) return;
	if (T66_IsGamblersTokenItem(ItemID))
	{
		ApplyGamblersTokenPickup(1);
		return;
	}

	// Auto-generate the Line 1 roll for the provided rarity.
	int32 RolledMin = 1, RolledMax = 3;
	FItemData::GetLine1RollRange(Rarity, RolledMin, RolledMax);
	UT66RngSubsystem* RngSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RngSubsystem>() : nullptr;
	FRandomStream Local(FPlatformTime::Cycles());
	FRandomStream& Stream = RngSub ? RngSub->GetRunStream() : Local;
	const int32 RolledValue = Stream.RandRange(RolledMin, RolledMax);
	const int32 RollSeed = Stream.RandRange(1, MAX_int32 / 4);

	FT66InventorySlot Slot(ItemID, Rarity, RolledValue, 0.f, 0, RollSeed);
	AddItemSlot(Slot);
}

void UT66RunStateSubsystem::AddItemSlot(const FT66InventorySlot& Slot)
{
	if (!Slot.IsValid()) return;
	if (T66_IsGamblersTokenItem(Slot.ItemTemplateID))
	{
		ApplyGamblersTokenPickup(Slot.Line1RolledValue);
		return;
	}

	FT66InventorySlot NormalizedSlot = Slot;
	if (NormalizedSlot.Line1RolledValue <= 0)
	{
		int32 RolledMin = 1;
		int32 RolledMax = 1;
		FItemData::GetLine1RollRange(NormalizedSlot.Rarity, RolledMin, RolledMax);
		NormalizedSlot.Line1RolledValue = RolledMax;
	}
	if (NormalizedSlot.RollSeed == 0)
	{
		NormalizedSlot.RollSeed = T66_GetDefaultInventoryRollSeed();
	}

	InventorySlots.Add(NormalizedSlot);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("ItemID=%s,Source=LootBag"), *NormalizedSlot.ItemTemplateID.ToString()));
	// Lab unlock: mark item as unlocked for The Lab (any run type including Lab).
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achieve->AddLabUnlockedItem(NormalizedSlot.ItemTemplateID);
		}
	}
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
}

void UT66RunStateSubsystem::ApplyGamblersTokenPickup(int32 TokenLevel)
{
	const int32 ClampedLevel = FMath::Clamp(TokenLevel, 1, MaxGamblersTokenLevel);
	if (ClampedLevel <= 0 || ActiveGamblersTokenLevel >= ClampedLevel)
	{
		return;
	}

	ActiveGamblersTokenLevel = ClampedLevel;
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("ItemID=%s,Source=GamblerToken,Level=%d"), *T66GamblersTokenItemID.ToString(), ActiveGamblersTokenLevel));

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achieve->AddLabUnlockedItem(T66GamblersTokenItemID);
		}
	}

	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
}

float UT66RunStateSubsystem::GetCurrentSellFraction() const
{
	return T66_GetSellFractionForTokenLevel(ActiveGamblersTokenLevel);
}

int32 UT66RunStateSubsystem::GetSellGoldForInventorySlot(const FT66InventorySlot& Slot) const
{
	if (!Slot.IsValid() || T66_IsGamblersTokenItem(Slot.ItemTemplateID))
	{
		return 0;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI)
	{
		return 0;
	}

	FItemData ItemData;
	if (!GI->GetItemData(Slot.ItemTemplateID, ItemData))
	{
		return 0;
	}

	const int32 BuyGold = ItemData.GetBuyGoldForRarity(Slot.Rarity);
	return FMath::Max(0, FMath::RoundToInt(static_cast<float>(BuyGold) * GetCurrentSellFraction()));
}

void UT66RunStateSubsystem::ClearInventory()
{
	InventorySlots.Empty();
	ActiveGamblersTokenLevel = 0;
	RecomputeItemDerivedStats();
	InventoryChanged.Broadcast();
}

void UT66RunStateSubsystem::HealToFull()
{
	if (bInLastStand || bInQuickReviveDowned) return;
	CurrentHP = MaxHP;
	HeartsChanged.Broadcast();
}

void UT66RunStateSubsystem::HealHP(float Amount)
{
	if (bInLastStand || bInQuickReviveDowned) return;
	if (Amount <= 0.f) return;

	const float NewHP = FMath::Clamp(CurrentHP + Amount, 0.f, MaxHP);
	if (FMath::IsNearlyEqual(NewHP, CurrentHP)) return;
	CurrentHP = NewHP;
	HeartsChanged.Broadcast();
}

void UT66RunStateSubsystem::HealHPFromCompanion(float Amount)
{
	if (bInLastStand || bInQuickReviveDowned) return;
	if (Amount <= 0.f) return;

	const float PreviousHP = CurrentHP;
	HealHP(Amount);
	const float AppliedHealing = FMath::Max(0.f, CurrentHP - PreviousHP);
	if (AppliedHealing > 0.f)
	{
		CompanionHealingDoneThisRun += AppliedHealing;
	}
}

void UT66RunStateSubsystem::HealHearts(int32 Hearts)
{
	if (Hearts <= 0) return;
	HealHP(static_cast<float>(Hearts) * HPPerRedHeart);
}

void UT66RunStateSubsystem::ApplyHpRegen(float DeltaTime)
{
	if (bInLastStand || bInQuickReviveDowned) return;
	const float Rate = GetHpRegenPerSecond();
	if (Rate <= 0.f || DeltaTime <= 0.f) return;

	const float Healed = Rate * DeltaTime;
	HealHP(Healed);
}

void UT66RunStateSubsystem::KillPlayer()
{
	CurrentHP = 0.f;
	HandleLethalDamage(nullptr);
}

bool UT66RunStateSubsystem::SellFirstItem()
{
	if (InventorySlots.Num() == 0) return false;
	return SellInventoryItemAt(0);
}

bool UT66RunStateSubsystem::SellInventoryItemAt(int32 InventoryIndex)
{
	if (InventoryIndex < 0 || InventoryIndex >= InventorySlots.Num()) return false;

	const FT66InventorySlot Slot = InventorySlots[InventoryIndex];
	const int32 SellGold = GetSellGoldForInventorySlot(Slot);

	CurrentGold += SellGold;
	BuybackPool.Add(Slot);
	InventorySlots.RemoveAt(InventoryIndex);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Vendor,ItemID=%s"), SellGold, *Slot.ItemTemplateID.ToString()));
	GoldChanged.Broadcast();
	InventoryChanged.Broadcast();
	BuybackChanged.Broadcast();
	LogAdded.Broadcast();
	return true;
}

ET66ItemRarity UT66RunStateSubsystem::GetNextItemRarity(ET66ItemRarity Rarity)
{
	switch (Rarity)
	{
	case ET66ItemRarity::Black:  return ET66ItemRarity::Red;
	case ET66ItemRarity::Red:    return ET66ItemRarity::Yellow;
	case ET66ItemRarity::Yellow: return ET66ItemRarity::White;
	case ET66ItemRarity::White:  return ET66ItemRarity::White;
	default:                     return ET66ItemRarity::Black;
	}
}

bool UT66RunStateSubsystem::CanAlchemyUpgradeInventoryItemAt(const int32 InventoryIndex) const
{
	FT66InventorySlot PreviewSlot;
	int32 MatchingCount = 0;
	return GetAlchemyUpgradePreviewAt(InventoryIndex, PreviewSlot, MatchingCount);
}

int32 UT66RunStateSubsystem::GetAlchemyMatchingInventoryCount(const int32 InventoryIndex) const
{
	if (!InventorySlots.IsValidIndex(InventoryIndex))
	{
		return 0;
	}

	const UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	const FT66InventorySlot& TargetSlot = InventorySlots[InventoryIndex];
	if (!T66_IsAlchemyEligibleSlot(TargetSlot, GI))
	{
		return 0;
	}

	int32 MatchingCount = 0;
	for (const FT66InventorySlot& Slot : InventorySlots)
	{
		if (T66_IsAlchemyMatch(TargetSlot, Slot))
		{
			++MatchingCount;
		}
	}

	return MatchingCount;
}

bool UT66RunStateSubsystem::GetAlchemyUpgradePreviewAt(const int32 InventoryIndex, FT66InventorySlot& OutUpgradedSlot, int32& OutMatchingCount) const
{
	OutUpgradedSlot = FT66InventorySlot();
	OutMatchingCount = GetAlchemyMatchingInventoryCount(InventoryIndex);
	if (!InventorySlots.IsValidIndex(InventoryIndex))
	{
		return false;
	}

	const UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	const FT66InventorySlot& TargetSlot = InventorySlots[InventoryIndex];
	if (!T66_IsAlchemyEligibleSlot(TargetSlot, GI) || OutMatchingCount < AlchemyCopiesRequired)
	{
		return false;
	}

	const TArray<int32> SourceIndices = T66_GatherAlchemySourceIndices(InventorySlots, InventoryIndex);
	if (SourceIndices.Num() < AlchemyCopiesRequired)
	{
		return false;
	}

	TArray<FT66InventorySlot> SourceSlots;
	SourceSlots.Reserve(SourceIndices.Num());
	for (const int32 SourceIndex : SourceIndices)
	{
		SourceSlots.Add(InventorySlots[SourceIndex]);
	}

	OutUpgradedSlot = T66_BuildAlchemyUpgradeSlot(TargetSlot, SourceSlots);
	return true;
}

bool UT66RunStateSubsystem::TryAlchemyUpgradeInventoryItems(int32 TargetIndex, int32 SacrificeIndex, FT66InventorySlot& OutUpgradedSlot)
{
	OutUpgradedSlot = FT66InventorySlot();
	(void)SacrificeIndex;

	if (TargetIndex < 0 || TargetIndex >= InventorySlots.Num())
	{
		return false;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	const FT66InventorySlot OriginalTargetSlot = InventorySlots[TargetIndex];
	if (!OriginalTargetSlot.IsValid())
	{
		return false;
	}

	int32 MatchingCount = 0;
	FT66InventorySlot UpgradedSlot;
	if (!GetAlchemyUpgradePreviewAt(TargetIndex, UpgradedSlot, MatchingCount))
	{
		return false;
	}

	const TArray<int32> SourceIndices = T66_GatherAlchemySourceIndices(InventorySlots, TargetIndex);
	if (SourceIndices.Num() < AlchemyCopiesRequired)
	{
		return false;
	}

	const float LuckyAlchemyChance = GetAlchemyLuckyUpgradeChance01();
	if (LuckyAlchemyChance > 0.f)
	{
		UT66RngSubsystem* RngSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RngSubsystem>() : nullptr;
		const bool bLuckyAlchemy = RngSub
			? RngSub->RollChance01(LuckyAlchemyChance)
			: (FMath::FRand() < LuckyAlchemyChance);
		RecordLuckQuantityBool(
			FName(TEXT("LuckyAlchemySuccess")),
			bLuckyAlchemy,
			LuckyAlchemyChance,
			RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE,
			RngSub ? RngSub->GetLastRunPreDrawSeed() : 0);
		if (bLuckyAlchemy)
		{
			T66_ApplyLuckyAlchemyBonus(UpgradedSlot);
		}
	}

	int32 AngerGold = 0;
	if (GI)
	{
		for (const int32 SourceIndex : SourceIndices)
		{
			if (!InventorySlots.IsValidIndex(SourceIndex))
			{
				continue;
			}

			FItemData SourceItemData;
			if (GI->GetItemData(InventorySlots[SourceIndex].ItemTemplateID, SourceItemData))
			{
				AngerGold += FMath::Max(1, SourceItemData.GetBuyGoldForRarity(InventorySlots[SourceIndex].Rarity));
			}
		}
	}

	int32 InsertIndex = TargetIndex;
	for (const int32 SourceIndex : SourceIndices)
	{
		if (SourceIndex < TargetIndex)
		{
			--InsertIndex;
		}
	}

	TArray<int32> RemovalIndices = SourceIndices;
	RemovalIndices.Sort([](const int32 A, const int32 B) { return A > B; });
	for (const int32 RemovalIndex : RemovalIndices)
	{
		InventorySlots.RemoveAt(RemovalIndex);
	}
	InsertIndex = FMath::Clamp(InsertIndex, 0, InventorySlots.Num());
	InventorySlots.Insert(UpgradedSlot, InsertIndex);

	RecomputeItemDerivedStats();
	AddCasinoAngerFromGold(AngerGold);
	AddStructuredEvent(
		ET66RunEventType::ItemAcquired,
		FString::Printf(
			TEXT("ItemID=%s,Source=Alchemy,Copies=%d,FromRarity=%d,ToRarity=%d"),
			*UpgradedSlot.ItemTemplateID.ToString(),
			AlchemyCopiesRequired,
			static_cast<int32>(OriginalTargetSlot.Rarity),
			static_cast<int32>(UpgradedSlot.Rarity)));
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();

	OutUpgradedSlot = UpgradedSlot;
	return true;
}

void UT66RunStateSubsystem::RecomputeItemDerivedStats()
{
	// Reset all accumulators.
	ItemStatBonuses = FT66HeroStatBonuses{};
	ItemPrimaryStatBonusesPrecise = FT66HeroPreciseStatBlock{};
	ItemPowerGivenPercent = 0.f;
	BonusDamagePercent = 0.f;
	BonusAttackSpeedPercent = 0.f;
	DashCooldownReductionPercent = 0.f;
	ItemDamageMultiplier = 1.f;
	ItemAttackSpeedMultiplier = 1.f;
	DashCooldownMultiplier = 1.f;
	ItemScaleMultiplier = 1.f;
	ItemMoveSpeedMultiplier = 1.f;
	ItemArmorBonus01 = 0.f;
	ItemEvasionBonus01 = 0.f;
	ItemBonusLuckFlat = 0;
	SecondaryMultipliers.Reset();
	ItemSecondaryStatBonusTenths.Reset();

	auto AddPrimaryBonusTenths = [&](ET66HeroStatType Type, int32 DeltaTenths)
	{
		const int32 V = FMath::Clamp(DeltaTenths, 0, MaxHeroStatValue * HeroStatTenthsScale);
		if (V <= 0)
		{
			return;
		}

		switch (Type)
		{
		case ET66HeroStatType::Damage:      ItemPrimaryStatBonusesPrecise.DamageTenths += V; break;
		case ET66HeroStatType::AttackSpeed: ItemPrimaryStatBonusesPrecise.AttackSpeedTenths += V; break;
		case ET66HeroStatType::AttackScale: ItemPrimaryStatBonusesPrecise.AttackScaleTenths += V; break;
		case ET66HeroStatType::Accuracy:    ItemPrimaryStatBonusesPrecise.AccuracyTenths += V; break;
		case ET66HeroStatType::Armor:       ItemPrimaryStatBonusesPrecise.ArmorTenths += V; break;
		case ET66HeroStatType::Evasion:     ItemPrimaryStatBonusesPrecise.EvasionTenths += V; break;
		case ET66HeroStatType::Luck:        ItemPrimaryStatBonusesPrecise.LuckTenths += V; break;
		case ET66HeroStatType::Speed:       ItemPrimaryStatBonusesPrecise.SpeedTenths += V; break;
		default: break;
		}
	};

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	for (const FT66InventorySlot& Slot : InventorySlots)
	{
		if (!Slot.IsValid()) continue;

		FItemData D;
		const bool bHasRow = (GI && GI->GetItemData(Slot.ItemTemplateID, D));
		if (!bHasRow) continue;
		if (T66_IsGamblersTokenItem(Slot.ItemTemplateID) || D.SecondaryStatType == ET66SecondaryStatType::GamblerToken)
		{
			continue;
		}

		// Line 1: Additive flat bonus to primary stat.
		const int32 PrimaryGainTenths = WholeStatToTenths(FMath::Max(0, Slot.Line1RolledValue));
		AddPrimaryBonusTenths(D.PrimaryStatType, PrimaryGainTenths);
		ApplyPrimaryGainToSecondaryBonuses(
			D.PrimaryStatType,
			PrimaryGainTenths,
			ItemSecondaryStatBonusTenths,
			T66_CombineInventorySeed(Slot, static_cast<int32>(D.PrimaryStatType)));

		// Line 2: flat secondary stat bonus on the item's tagged secondary.
		if (D.SecondaryStatType != ET66SecondaryStatType::None)
		{
			AddItemSecondaryStatBonusTenths(D.SecondaryStatType, WholeStatToTenths(Slot.GetSecondaryStatBonusValue()));
		}
	}

	ItemStatBonuses.Damage = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemPrimaryStatBonusesPrecise.DamageTenths);
	ItemStatBonuses.AttackSpeed = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemPrimaryStatBonusesPrecise.AttackSpeedTenths);
	ItemStatBonuses.AttackScale = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemPrimaryStatBonusesPrecise.AttackScaleTenths);
	ItemStatBonuses.Accuracy = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemPrimaryStatBonusesPrecise.AccuracyTenths);
	ItemStatBonuses.Armor = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemPrimaryStatBonusesPrecise.ArmorTenths);
	ItemStatBonuses.Evasion = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemPrimaryStatBonusesPrecise.EvasionTenths);
	ItemStatBonuses.Luck = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemPrimaryStatBonusesPrecise.LuckTenths);

	ItemStatBonuses.PierceDmg = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::PierceDamage));
	ItemStatBonuses.PierceAtkSpd = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::PierceSpeed));
	ItemStatBonuses.PierceAtkScale = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::PierceScale));
	ItemStatBonuses.BounceDmg = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::BounceDamage));
	ItemStatBonuses.BounceAtkSpd = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::BounceSpeed));
	ItemStatBonuses.BounceAtkScale = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::BounceScale));
	ItemStatBonuses.AoeDmg = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::AoeDamage));
	ItemStatBonuses.AoeAtkSpd = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::AoeSpeed));
	ItemStatBonuses.AoeAtkScale = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::AoeScale));
	ItemStatBonuses.DotDmg = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::DotDamage));
	ItemStatBonuses.DotAtkSpd = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::DotSpeed));
	ItemStatBonuses.DotAtkScale = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::DotScale));
}

void UT66RunStateSubsystem::SetTutorialHint(const FText& InLine1, const FText& InLine2)
{
	bTutorialHintVisible = true;
	TutorialHintLine1 = InLine1;
	TutorialHintLine2 = InLine2;
	TutorialHintChanged.Broadcast();
}

void UT66RunStateSubsystem::ClearTutorialHint()
{
	if (!bTutorialHintVisible && TutorialHintLine1.IsEmpty() && TutorialHintLine2.IsEmpty())
	{
		return;
	}
	bTutorialHintVisible = false;
	TutorialHintLine1 = FText::GetEmpty();
	TutorialHintLine2 = FText::GetEmpty();
	TutorialHintChanged.Broadcast();
}

void UT66RunStateSubsystem::SetTutorialSubtitle(const FText& InSpeaker, const FText& InText)
{
	bTutorialSubtitleVisible = true;
	TutorialSubtitleSpeaker = InSpeaker;
	TutorialSubtitleText = InText;
	TutorialSubtitleChanged.Broadcast();
}

void UT66RunStateSubsystem::ClearTutorialSubtitle()
{
	if (!bTutorialSubtitleVisible && TutorialSubtitleSpeaker.IsEmpty() && TutorialSubtitleText.IsEmpty())
	{
		return;
	}

	bTutorialSubtitleVisible = false;
	TutorialSubtitleSpeaker = FText::GetEmpty();
	TutorialSubtitleText = FText::GetEmpty();
	TutorialSubtitleChanged.Broadcast();
}

void UT66RunStateSubsystem::NotifyTutorialMoveInput()
{
	if (bTutorialMoveInputSeen) return;
	bTutorialMoveInputSeen = true;
	TutorialInputChanged.Broadcast();
}

void UT66RunStateSubsystem::NotifyTutorialJumpInput()
{
	if (bTutorialJumpInputSeen) return;
	bTutorialJumpInputSeen = true;
	TutorialInputChanged.Broadcast();
}

void UT66RunStateSubsystem::NotifyTutorialLookInput()
{
	if (bTutorialLookInputSeen) return;
	bTutorialLookInputSeen = true;
	TutorialInputChanged.Broadcast();
}

void UT66RunStateSubsystem::NotifyTutorialAttackLockInput()
{
	if (bTutorialAttackLockInputSeen) return;
	bTutorialAttackLockInputSeen = true;
	TutorialInputChanged.Broadcast();
}

void UT66RunStateSubsystem::NotifyTutorialUltimateUsed()
{
	if (bTutorialUltimateUsedSeen) return;
	bTutorialUltimateUsedSeen = true;
	TutorialInputChanged.Broadcast();
}

void UT66RunStateSubsystem::ResetTutorialInputFlags()
{
	const bool bWasMove = bTutorialMoveInputSeen;
	const bool bWasJump = bTutorialJumpInputSeen;
	const bool bWasLook = bTutorialLookInputSeen;
	const bool bWasLock = bTutorialAttackLockInputSeen;
	const bool bWasUltimate = bTutorialUltimateUsedSeen;
	bTutorialMoveInputSeen = false;
	bTutorialJumpInputSeen = false;
	bTutorialLookInputSeen = false;
	bTutorialAttackLockInputSeen = false;
	bTutorialUltimateUsedSeen = false;
	if (bWasMove || bWasJump || bWasLook || bWasLock || bWasUltimate)
	{
		TutorialInputChanged.Broadcast();
	}
}

void UT66RunStateSubsystem::ToggleDevImmortality()
{
	bDevImmortality = !bDevImmortality;
	DevCheatsChanged.Broadcast();
}

void UT66RunStateSubsystem::ToggleDevPower()
{
	bDevPower = !bDevPower;
	DevCheatsChanged.Broadcast();
}

void UT66RunStateSubsystem::ResetForNewRun()
{
	ResetHeartSlotTiers();
	SyncMaxHPToHeartTiers();
	CurrentHP = MaxHP;
	CurrentGold = DefaultStartGold;
	CurrentDebt = 0;
	bLoanSharkPending = false;
	DifficultyTier = 0;
	DifficultySkulls = 0;
	TotemsActivatedCount = 0;
	GamblerAnger01 = 0.f;
	ResetVendorForStage();
	OwedBossIDs.Empty();
	CowardiceGatesTakenCount = 0;
	InventorySlots.Empty();
	ActiveGamblersTokenLevel = 0;
	BuybackPool.Empty();
	BuybackDisplaySlots.Empty();
	BuybackDisplayPage = 0;
	RecomputeItemDerivedStats();
	ActiveDOTs.Empty();
	RallyStacks = 0;
	RallyTimerEndWorldTime = 0.0;
	if (UWorld* W = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
	{
		if (DOTTimerHandle.IsValid())
		{
			W->GetTimerManager().ClearTimer(DOTTimerHandle);
			DOTTimerHandle.Invalidate();
		}
	}
	EventLog.Empty();
	StructuredEventLog.Empty();
	AntiCheatLuckEvents.Empty();
	AntiCheatHitCheckEvents.Empty();
	AntiCheatGamblerSummaries.Empty();
	AntiCheatGamblerEvents.Empty();
	bAntiCheatLuckEventsTruncated = false;
	bAntiCheatHitCheckEventsTruncated = false;
	bAntiCheatGamblerEventsTruncated = false;
	StagePacingPoints.Empty();
	ResetLuckRatingTracking();
	AntiCheatIncomingHitChecks = 0;
	AntiCheatDamageTakenHitCount = 0;
	AntiCheatDodgeCount = 0;
	AntiCheatCurrentConsecutiveDodges = 0;
	AntiCheatMaxConsecutiveDodges = 0;
	AntiCheatTotalEvasionChance = 0.f;
	InitializeAntiCheatEvasionBuckets();
	ResetAntiCheatPressureTracking();
	CurrentStage = 1;
	bStageTimerActive = false;
	StageTimerSecondsRemaining = StageTimerDurationSeconds;
	LastBroadcastStageTimerSecond = static_cast<int32>(StageTimerDurationSeconds);
	ResetSpeedRunTimer();
	bThisRunSetNewPersonalBestSpeedRunTime = false;
	CompletedStageActiveSeconds = 0.f;
	FinalRunElapsedSeconds = 0.f;
	bRunEnded = false;
	bRunEndedAsVictory = false;
	bPendingDifficultyClearSummary = false;
	bSaintBlessingActive = false;
	SaintBlessingInventorySnapshot.Reset();
	SaintBlessingEquippedIdolsSnapshot.Reset();
	SaintBlessingEquippedIdolTiersSnapshot.Reset();
	bSaintBlessingLoadoutSnapshotValid = false;
	FinalSurvivalEnemyScalar = 1.f;
	CurrentScore = 0;
	LastDamageTime = -9999.f;
	PowerCrystalsEarnedThisRun = 0;
	PowerCrystalsGrantedToWalletThisRun = 0;
	CompanionHealingDoneThisRun = 0.f;
	SingleUseSecondaryMultipliers.Reset();

	// Skill Rating: reset per brand new run.
	if (UGameInstance* GI3 = GetGameInstance())
	{
		if (UT66SkillRatingSubsystem* Skill = GI3->GetSubsystem<UT66SkillRatingSubsystem>())
		{
			Skill->ResetForNewRun();
		}
	}

	bHUDPanelsVisible = true;
	ClearTutorialHint();
	ClearTutorialSubtitle();
	ResetTutorialInputFlags();

	// Clear transient stage/status effects at run start.
	StageMoveSpeedMultiplier = 1.f;
	StageMoveSpeedSecondsRemaining = 0.f;
	StatusBurnSecondsRemaining = 0.f;
	StatusBurnDamagePerSecond = 0.f;
	StatusBurnAccumDamage = 0.f;
	StatusChillSecondsRemaining = 0.f;
	StatusChillMoveSpeedMultiplier = 1.f;
	StatusCurseSecondsRemaining = 0.f;
	HeroLevel = DefaultHeroLevel;
	HeroPreciseStats = FT66HeroPreciseStatBlock{};
	ItemPrimaryStatBonusesPrecise = FT66HeroPreciseStatBlock{};
	ClearPersistentSecondaryStatBonuses();
	ItemSecondaryStatBonusTenths.Reset();
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
		{
			IdolManager->ResetForNewRun(T66GI->SelectedDifficulty);
		}
		if (UT66PlayerExperienceSubSystem* PlayerExperience = T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>())
		{
			const int32 BonusLevels = PlayerExperience->GetDifficultyStartHeroBonusLevels(T66GI->SelectedDifficulty);
			HeroLevel = FMath::Clamp(DefaultHeroLevel + BonusLevels, DefaultHeroLevel, MaxHeroLevel);
		}
		bInStageCatchUp = T66GI->bStageCatchUpPending;
	}
	else if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		IdolManager->ResetForNewRun(ET66Difficulty::Easy);
	}
	InitializeHeroStatsForNewRun();

	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		if (T66GI->HasSelectedRunMod() && T66GI->SelectedRunModifierID == T66MaxHeroStatsRunModifierID)
		{
			HeroLevel = MaxHeroLevel;

			FT66HeroStatBlock MaxHeroStats;
			MaxHeroStats.Damage = MaxHeroLevel;
			MaxHeroStats.AttackSpeed = MaxHeroLevel;
			MaxHeroStats.AttackScale = MaxHeroLevel;
			MaxHeroStats.Accuracy = MaxHeroLevel;
			MaxHeroStats.Armor = MaxHeroLevel;
			MaxHeroStats.Evasion = MaxHeroLevel;
			MaxHeroStats.Luck = MaxHeroLevel;
			MaxHeroStats.Speed = MaxHeroLevel;
			HeroPreciseStats.SetFromWholeStatBlock(MaxHeroStats);
			SyncLegacyHeroStatsFromPrecise();

			ClearPersistentSecondaryStatBonuses();
			const int32 MaxStatTenths = WholeStatToTenths(MaxHeroLevel);
			for (uint8 RawStatType = static_cast<uint8>(ET66SecondaryStatType::None) + 1;
				RawStatType <= static_cast<uint8>(ET66SecondaryStatType::Accuracy);
				++RawStatType)
			{
				const ET66SecondaryStatType StatType = static_cast<ET66SecondaryStatType>(RawStatType);
				if (T66IsLiveSecondaryStatType(StatType))
				{
					AddPersistentSecondaryStatBonusTenths(StatType, MaxStatTenths);
				}
			}
		}
	}

	// Central RNG: seed a new run stream and set the initial effective Luck stat for biasing.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RngSubsystem* Rng = GI->GetSubsystem<UT66RngSubsystem>())
		{
			Rng->BeginRun(GetLuckStat());
		}
	}

	HeroXP = 0;
	XPToNextLevel = DefaultXPToLevel;
	UltimateCooldownRemainingSeconds = 0.f;
	LastBroadcastUltimateSecond = 0;
	SurvivalCharge01 = 0.f;
	bInLastStand = false;
	LastStandSecondsRemaining = 0.f;
	LastBroadcastLastStandSecond = 0;
	bQuickReviveChargeReady = false;
	bInQuickReviveDowned = false;
	QuickReviveDownedSecondsRemaining = 0.f;
	LastBroadcastQuickReviveSecond = 0;
	ResetBossState();
	HeartsChanged.Broadcast();
	GoldChanged.Broadcast();
	DebtChanged.Broadcast();
	DifficultyChanged.Broadcast();
	GamblerAngerChanged.Broadcast();
	VendorChanged.Broadcast();
	InventoryChanged.Broadcast();
	IdolsChanged.Broadcast();
	PanelVisibilityChanged.Broadcast();
	ScoreChanged.Broadcast();
	StageTimerChanged.Broadcast();
	SpeedRunTimerChanged.Broadcast();
	BossChanged.Broadcast();
	HeroProgressChanged.Broadcast();
	UltimateChanged.Broadcast();
	SurvivalChanged.Broadcast();
	QuickReviveChanged.Broadcast();
	StatusEffectsChanged.Broadcast();
}

float UT66RunStateSubsystem::GetCurrentRunElapsedSeconds() const
{
	const float CurrentStageSeconds = (bSpeedRunStarted && SpeedRunStartWorldSeconds > 0.f && GetWorld())
		? FMath::Max(0.f, static_cast<float>(GetWorld()->GetTimeSeconds()) - SpeedRunStartWorldSeconds)
		: FMath::Max(0.f, SpeedRunElapsedSeconds);
	return FMath::Max(0.f, CompletedStageActiveSeconds + CurrentStageSeconds);
}

void UT66RunStateSubsystem::MarkRunEnded(bool bWasFullClear)
{
	if (bRunEnded)
	{
		if (bWasFullClear)
		{
			bRunEndedAsVictory = true;
		}
		return;
	}

	if (bWasFullClear)
	{
		RecordStagePacingPoint(CurrentStage, GetCurrentRunElapsedSeconds());
	}

	FinalRunElapsedSeconds = GetCurrentRunElapsedSeconds();
	bRunEnded = true;
	bRunEndedAsVictory = bWasFullClear;
}

void UT66RunStateSubsystem::ExportSavedRunSnapshot(FT66SavedRunSnapshot& OutSnapshot) const
{
	OutSnapshot = FT66SavedRunSnapshot{};
	OutSnapshot.bValid = true;
	OutSnapshot.CurrentStage = CurrentStage;
	OutSnapshot.CurrentHP = CurrentHP;
	OutSnapshot.MaxHP = MaxHP;
	OutSnapshot.HeartSlotTiers.Reset();
	for (int32 SlotIndex = 0; SlotIndex < DefaultMaxHearts; ++SlotIndex)
	{
		OutSnapshot.HeartSlotTiers.Add(static_cast<uint8>(GetHeartSlotTier(SlotIndex)));
	}
	OutSnapshot.CurrentGold = CurrentGold;
	OutSnapshot.CurrentDebt = CurrentDebt;
	OutSnapshot.DifficultyTier = DifficultyTier;
	OutSnapshot.DifficultySkulls = DifficultySkulls;
	OutSnapshot.TotemsActivatedCount = TotemsActivatedCount;
	OutSnapshot.GamblerAnger01 = GamblerAnger01;
	OutSnapshot.OwedBossIDs = OwedBossIDs;
	OutSnapshot.CowardiceGatesTakenCount = CowardiceGatesTakenCount;
	OutSnapshot.InventorySlots = InventorySlots;
	OutSnapshot.ActiveGamblersTokenLevel = ActiveGamblersTokenLevel;
	OutSnapshot.EventLog = EventLog;
	OutSnapshot.StructuredEventLog = StructuredEventLog;
	OutSnapshot.StagePacingPoints = StagePacingPoints;
	OutSnapshot.bStageTimerActive = bStageTimerActive;
	OutSnapshot.StageTimerSecondsRemaining = StageTimerSecondsRemaining;
	OutSnapshot.SpeedRunElapsedSeconds = bSpeedRunStarted && GetWorld()
		? FMath::Max(0.f, static_cast<float>(GetWorld()->GetTimeSeconds()) - SpeedRunStartWorldSeconds)
		: SpeedRunElapsedSeconds;
	OutSnapshot.bSpeedRunStarted = bSpeedRunStarted;
	OutSnapshot.CompletedStageActiveSeconds = CompletedStageActiveSeconds;
	OutSnapshot.FinalRunElapsedSeconds = FinalRunElapsedSeconds;
	OutSnapshot.bRunEnded = bRunEnded;
	OutSnapshot.bRunEndedAsVictory = bRunEndedAsVictory;
	OutSnapshot.CurrentScore = CurrentScore;
	OutSnapshot.HeroLevel = HeroLevel;
	OutSnapshot.HeroXP = HeroXP;
	OutSnapshot.XPToNextLevel = XPToNextLevel;
	OutSnapshot.HeroPreciseStats = HeroPreciseStats;
	OutSnapshot.HeroStatRngCurrentSeed = HeroStatRng.GetCurrentSeed();
	OutSnapshot.PersistentSecondaryStatBonusEntries.Reset();
	for (const TPair<ET66SecondaryStatType, int32>& Pair : PersistentSecondaryStatBonusTenths)
	{
		if (Pair.Key == ET66SecondaryStatType::None || Pair.Value <= 0)
		{
			continue;
		}

		FT66SavedSecondaryStatBonusEntry& SavedBonus = OutSnapshot.PersistentSecondaryStatBonusEntries.AddDefaulted_GetRef();
		SavedBonus.StatType = Pair.Key;
		SavedBonus.BonusTenths = Pair.Value;
	}
	OutSnapshot.HeroStats = HeroPreciseStats.ToDisplayStatBlock();
	OutSnapshot.PowerCrystalsEarnedThisRun = PowerCrystalsEarnedThisRun;
	OutSnapshot.PowerCrystalsGrantedToWalletThisRun = PowerCrystalsGrantedToWalletThisRun;
	OutSnapshot.CompanionHealingDoneThisRun = CompanionHealingDoneThisRun;
	OutSnapshot.bBossActive = bBossActive;
	OutSnapshot.ActiveBossID = ActiveBossID;
	OutSnapshot.BossMaxHP = BossMaxHP;
	OutSnapshot.BossCurrentHP = BossCurrentHP;
	OutSnapshot.BossParts = BossPartSnapshots;
	OutSnapshot.bPendingDifficultyClearSummary = bPendingDifficultyClearSummary;
	OutSnapshot.bSaintBlessingActive = bSaintBlessingActive;
	OutSnapshot.FinalSurvivalEnemyScalar = FinalSurvivalEnemyScalar;
	OutSnapshot.AntiCheatIncomingHitChecks = AntiCheatIncomingHitChecks;
	OutSnapshot.AntiCheatDamageTakenHitCount = AntiCheatDamageTakenHitCount;
	OutSnapshot.AntiCheatDodgeCount = AntiCheatDodgeCount;
	OutSnapshot.AntiCheatCurrentConsecutiveDodges = AntiCheatCurrentConsecutiveDodges;
	OutSnapshot.AntiCheatMaxConsecutiveDodges = AntiCheatMaxConsecutiveDodges;
	OutSnapshot.AntiCheatTotalEvasionChance = AntiCheatTotalEvasionChance;
	OutSnapshot.AntiCheatEvasionBuckets = AntiCheatEvasionBuckets;
	OutSnapshot.AntiCheatPressureWindowSummary = AntiCheatPressureWindowSummary;
	OutSnapshot.AntiCheatGamblerSummaries = AntiCheatGamblerSummaries;
	OutSnapshot.AntiCheatGamblerEvents = AntiCheatGamblerEvents;
	OutSnapshot.bAntiCheatGamblerEventsTruncated = bAntiCheatGamblerEventsTruncated;
	OutSnapshot.AntiCheatCurrentPressureWindowIndex = AntiCheatCurrentPressureWindowIndex;
	OutSnapshot.AntiCheatCurrentPressureHitChecks = AntiCheatCurrentPressureHitChecks;
	OutSnapshot.AntiCheatCurrentPressureDodges = AntiCheatCurrentPressureDodges;
	OutSnapshot.AntiCheatCurrentPressureDamageApplied = AntiCheatCurrentPressureDamageApplied;
	OutSnapshot.AntiCheatCurrentPressureExpectedDodges = AntiCheatCurrentPressureExpectedDodges;

	if (const UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		OutSnapshot.EquippedIdols = IdolManager->GetEquippedIdols();
		OutSnapshot.EquippedIdolTiers = IdolManager->GetEquippedIdolTierValues();
		OutSnapshot.RemainingCatchUpIdolPicks = IdolManager->GetRemainingCatchUpIdolPicks();
	}
	else
	{
		OutSnapshot.EquippedIdols = GetEquippedIdols();
		OutSnapshot.EquippedIdolTiers = GetEquippedIdolTierValues();
	}

	for (const TPair<FName, FT66LuckAccumulator>& Pair : LuckQuantityByCategory)
	{
		FT66SavedLuckAccumulator& Saved = OutSnapshot.LuckQuantityAccumulators.AddDefaulted_GetRef();
		Saved.Category = Pair.Key;
		Saved.Sum01 = static_cast<float>(Pair.Value.Sum01);
		Saved.Count = Pair.Value.Count;
	}

	for (const TPair<FName, FT66LuckAccumulator>& Pair : LuckQualityByCategory)
	{
		FT66SavedLuckAccumulator& Saved = OutSnapshot.LuckQualityAccumulators.AddDefaulted_GetRef();
		Saved.Category = Pair.Key;
		Saved.Sum01 = static_cast<float>(Pair.Value.Sum01);
		Saved.Count = Pair.Value.Count;
	}

	OutSnapshot.AntiCheatLuckEvents = AntiCheatLuckEvents;
	OutSnapshot.bAntiCheatLuckEventsTruncated = bAntiCheatLuckEventsTruncated;
	OutSnapshot.AntiCheatHitCheckEvents = AntiCheatHitCheckEvents;
	OutSnapshot.bAntiCheatHitCheckEventsTruncated = bAntiCheatHitCheckEventsTruncated;
}

void UT66RunStateSubsystem::ImportSavedRunSnapshot(const FT66SavedRunSnapshot& Snapshot)
{
	if (!Snapshot.bValid)
	{
		return;
	}

	CurrentStage = FMath::Clamp(Snapshot.CurrentStage, 1, 23);
	CurrentHP = FMath::Max(0.f, Snapshot.CurrentHP);
	MaxHP = FMath::Max(1.f, Snapshot.MaxHP);
	if (Snapshot.HeartSlotTiers.Num() >= DefaultMaxHearts)
	{
		HeartSlotTiers = Snapshot.HeartSlotTiers;
		HeartSlotTiers.SetNum(DefaultMaxHearts);
		for (uint8& TierValue : HeartSlotTiers)
		{
			TierValue = static_cast<uint8>(FMath::Clamp(static_cast<int32>(TierValue), 0, MaxHeartTier));
		}
		SyncMaxHPToHeartTiers();
		CurrentHP = FMath::Clamp(CurrentHP, 0.f, MaxHP);
	}
	else
	{
		RebuildHeartSlotTiersFromMaxHP();
	}
	CurrentGold = FMath::Max(0, Snapshot.CurrentGold);
	CurrentDebt = FMath::Max(0, Snapshot.CurrentDebt);
	DifficultyTier = FMath::Max(0, Snapshot.DifficultyTier);
	DifficultySkulls = FMath::Max(0, Snapshot.DifficultySkulls);
	TotemsActivatedCount = FMath::Max(0, Snapshot.TotemsActivatedCount);
	GamblerAnger01 = FMath::Clamp(Snapshot.GamblerAnger01, 0.f, 1.f);
	OwedBossIDs = Snapshot.OwedBossIDs;
	CowardiceGatesTakenCount = FMath::Max(0, Snapshot.CowardiceGatesTakenCount);
	InventorySlots = Snapshot.InventorySlots;
	ActiveGamblersTokenLevel = T66_ClampGamblersTokenLevel(Snapshot.ActiveGamblersTokenLevel);
	EventLog = Snapshot.EventLog;
	StructuredEventLog = Snapshot.StructuredEventLog;
	StagePacingPoints = Snapshot.StagePacingPoints;
	bStageTimerActive = Snapshot.bStageTimerActive;
	StageTimerSecondsRemaining = FMath::Clamp(Snapshot.StageTimerSecondsRemaining, 0.f, StageTimerDurationSeconds);
	LastBroadcastStageTimerSecond = FMath::FloorToInt(StageTimerSecondsRemaining);
	SpeedRunElapsedSeconds = FMath::Max(0.f, Snapshot.SpeedRunElapsedSeconds);
	bSpeedRunStarted = Snapshot.bSpeedRunStarted;
	if (bSpeedRunStarted && GetWorld())
	{
		SpeedRunStartWorldSeconds = static_cast<float>(GetWorld()->GetTimeSeconds()) - SpeedRunElapsedSeconds;
	}
	else
	{
		SpeedRunStartWorldSeconds = 0.f;
	}
	LastBroadcastSpeedRunSecond = FMath::FloorToInt(SpeedRunElapsedSeconds);
	CompletedStageActiveSeconds = FMath::Max(0.f, Snapshot.CompletedStageActiveSeconds);
	FinalRunElapsedSeconds = FMath::Max(0.f, Snapshot.FinalRunElapsedSeconds);
	bRunEnded = Snapshot.bRunEnded;
	bRunEndedAsVictory = Snapshot.bRunEndedAsVictory;
	bPendingDifficultyClearSummary = Snapshot.bPendingDifficultyClearSummary;
	bSaintBlessingActive = Snapshot.bSaintBlessingActive;
	FinalSurvivalEnemyScalar = FMath::Clamp(Snapshot.FinalSurvivalEnemyScalar, 1.f, 99.f);
	AntiCheatIncomingHitChecks = FMath::Max(0, Snapshot.AntiCheatIncomingHitChecks);
	AntiCheatDamageTakenHitCount = FMath::Max(0, Snapshot.AntiCheatDamageTakenHitCount);
	AntiCheatDodgeCount = FMath::Max(0, Snapshot.AntiCheatDodgeCount);
	AntiCheatCurrentConsecutiveDodges = FMath::Max(0, Snapshot.AntiCheatCurrentConsecutiveDodges);
	AntiCheatMaxConsecutiveDodges = FMath::Max(AntiCheatCurrentConsecutiveDodges, Snapshot.AntiCheatMaxConsecutiveDodges);
	AntiCheatTotalEvasionChance = FMath::Clamp(Snapshot.AntiCheatTotalEvasionChance, 0.f, 1000000.f);
	AntiCheatEvasionBuckets = Snapshot.AntiCheatEvasionBuckets;
	InitializeAntiCheatEvasionBuckets();
	AntiCheatPressureWindowSummary = Snapshot.AntiCheatPressureWindowSummary;
	AntiCheatPressureWindowSummary.WindowSeconds = AntiCheatPressureWindowSeconds;
	AntiCheatGamblerSummaries = Snapshot.AntiCheatGamblerSummaries;
	AntiCheatGamblerEvents = Snapshot.AntiCheatGamblerEvents;
	bAntiCheatGamblerEventsTruncated = Snapshot.bAntiCheatGamblerEventsTruncated;
	AntiCheatCurrentPressureWindowIndex = Snapshot.AntiCheatCurrentPressureWindowIndex;
	AntiCheatCurrentPressureHitChecks = FMath::Max(0, Snapshot.AntiCheatCurrentPressureHitChecks);
	AntiCheatCurrentPressureDodges = FMath::Max(0, Snapshot.AntiCheatCurrentPressureDodges);
	AntiCheatCurrentPressureDamageApplied = FMath::Max(0, Snapshot.AntiCheatCurrentPressureDamageApplied);
	AntiCheatCurrentPressureExpectedDodges = FMath::Clamp(Snapshot.AntiCheatCurrentPressureExpectedDodges, 0.f, 1000000.f);
	CurrentScore = FMath::Max(0, Snapshot.CurrentScore);
	HeroLevel = FMath::Clamp(Snapshot.HeroLevel, DefaultHeroLevel, MaxHeroLevel);
	HeroXP = FMath::Max(0, Snapshot.HeroXP);
	XPToNextLevel = FMath::Max(1, Snapshot.XPToNextLevel);
	HeroStats = Snapshot.HeroStats;
	if (Snapshot.HeroPreciseStats.HasAnyPositiveValue())
	{
		HeroPreciseStats = Snapshot.HeroPreciseStats;
	}
	else
	{
		HeroPreciseStats.SetFromWholeStatBlock(HeroStats);
	}
	HeroPreciseStats.DamageTenths = ClampHeroStatTenths(HeroPreciseStats.DamageTenths);
	HeroPreciseStats.AttackSpeedTenths = ClampHeroStatTenths(HeroPreciseStats.AttackSpeedTenths);
	HeroPreciseStats.AttackScaleTenths = ClampHeroStatTenths(HeroPreciseStats.AttackScaleTenths);
	HeroPreciseStats.AccuracyTenths = ClampHeroStatTenths(HeroPreciseStats.AccuracyTenths);
	HeroPreciseStats.ArmorTenths = ClampHeroStatTenths(HeroPreciseStats.ArmorTenths);
	HeroPreciseStats.EvasionTenths = ClampHeroStatTenths(HeroPreciseStats.EvasionTenths);
	HeroPreciseStats.LuckTenths = ClampHeroStatTenths(HeroPreciseStats.LuckTenths);
	HeroPreciseStats.SpeedTenths = ClampHeroStatTenths(HeroPreciseStats.SpeedTenths);
	HeroStatRng.Initialize(Snapshot.HeroStatRngCurrentSeed != 0 ? Snapshot.HeroStatRngCurrentSeed : static_cast<int32>(FPlatformTime::Cycles()));
	ClearPersistentSecondaryStatBonuses();
	for (const FT66SavedSecondaryStatBonusEntry& SavedBonus : Snapshot.PersistentSecondaryStatBonusEntries)
	{
		AddPersistentSecondaryStatBonusTenths(SavedBonus.StatType, SavedBonus.BonusTenths);
	}
	SyncLegacyHeroStatsFromPrecise();
	PowerCrystalsEarnedThisRun = FMath::Max(0, Snapshot.PowerCrystalsEarnedThisRun);
	PowerCrystalsGrantedToWalletThisRun = FMath::Clamp(Snapshot.PowerCrystalsGrantedToWalletThisRun, 0, PowerCrystalsEarnedThisRun);
	CompanionHealingDoneThisRun = FMath::Max(0.f, Snapshot.CompanionHealingDoneThisRun);
	bBossActive = Snapshot.bBossActive;
	ActiveBossID = Snapshot.ActiveBossID;
	BossMaxHP = FMath::Max(1, Snapshot.BossMaxHP);
	BossCurrentHP = FMath::Clamp(Snapshot.BossCurrentHP, 0, BossMaxHP);
	BossPartSnapshots = Snapshot.BossParts;
	if (BossPartSnapshots.Num() > 0)
	{
		T66_RecomputeBossAggregate(BossPartSnapshots, BossMaxHP, BossCurrentHP);
	}

	LuckQuantityByCategory.Reset();
	for (const FT66SavedLuckAccumulator& Saved : Snapshot.LuckQuantityAccumulators)
	{
		FT66LuckAccumulator Accumulator;
		Accumulator.Sum01 = static_cast<double>(Saved.Sum01);
		Accumulator.Count = FMath::Max(0, Saved.Count);
		LuckQuantityByCategory.Add(Saved.Category, Accumulator);
	}

	LuckQualityByCategory.Reset();
	for (const FT66SavedLuckAccumulator& Saved : Snapshot.LuckQualityAccumulators)
	{
		FT66LuckAccumulator Accumulator;
		Accumulator.Sum01 = static_cast<double>(Saved.Sum01);
		Accumulator.Count = FMath::Max(0, Saved.Count);
		LuckQualityByCategory.Add(Saved.Category, Accumulator);
	}

	AntiCheatLuckEvents = Snapshot.AntiCheatLuckEvents;
	bAntiCheatLuckEventsTruncated = Snapshot.bAntiCheatLuckEventsTruncated;
	AntiCheatHitCheckEvents = Snapshot.AntiCheatHitCheckEvents;
	bAntiCheatHitCheckEventsTruncated = Snapshot.bAntiCheatHitCheckEventsTruncated;
	if (AntiCheatGamblerEvents.Num() > MaxAntiCheatGamblerEvents)
	{
		const int32 RemoveCount = AntiCheatGamblerEvents.Num() - MaxAntiCheatGamblerEvents;
		AntiCheatGamblerEvents.RemoveAt(0, RemoveCount, EAllowShrinking::No);
		bAntiCheatGamblerEventsTruncated = true;
	}

	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		ET66Difficulty Difficulty = ET66Difficulty::Easy;
		if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
		{
			Difficulty = T66GI->SelectedDifficulty;
		}
		IdolManager->RestoreState(Snapshot.EquippedIdols, Snapshot.EquippedIdolTiers, Difficulty, Snapshot.RemainingCatchUpIdolPicks);
	}

	RecomputeItemDerivedStats();
	TrimLogsIfNeeded();

	HeartsChanged.Broadcast();
	GoldChanged.Broadcast();
	DebtChanged.Broadcast();
	DifficultyChanged.Broadcast();
	GamblerAngerChanged.Broadcast();
	InventoryChanged.Broadcast();
	IdolsChanged.Broadcast();
	ScoreChanged.Broadcast();
	StageChanged.Broadcast();
	StageTimerChanged.Broadcast();
	SpeedRunTimerChanged.Broadcast();
	BossChanged.Broadcast();
	HeroProgressChanged.Broadcast();
	SurvivalChanged.Broadcast();
	VendorChanged.Broadcast();
	StatusEffectsChanged.Broadcast();
}

bool UT66RunStateSubsystem::HasStagePacingPoint(const int32 Stage) const
{
	return StagePacingPoints.ContainsByPredicate([Stage](const FT66StagePacingPoint& Point)
	{
		return Point.Stage == Stage;
	});
}

void UT66RunStateSubsystem::RecordStagePacingPoint(const int32 Stage, const float CumulativeElapsedSeconds)
{
	const int32 ClampedStage = FMath::Clamp(Stage, 1, 23);
	if (ClampedStage <= 0)
	{
		return;
	}

	FT66StagePacingPoint Point;
	Point.Stage = ClampedStage;
	Point.Score = FMath::Max(0, CurrentScore);
	Point.ElapsedSeconds = FMath::Max(0.f, CumulativeElapsedSeconds);

	const int32 ExistingIndex = StagePacingPoints.IndexOfByPredicate([ClampedStage](const FT66StagePacingPoint& ExistingPoint)
	{
		return ExistingPoint.Stage == ClampedStage;
	});

	if (ExistingIndex != INDEX_NONE)
	{
		StagePacingPoints[ExistingIndex] = Point;
		return;
	}

	StagePacingPoints.Add(Point);
	StagePacingPoints.Sort([](const FT66StagePacingPoint& A, const FT66StagePacingPoint& B)
	{
		return A.Stage < B.Stage;
	});

	AddStructuredEvent(ET66RunEventType::StageExited, T66LeaderboardPacing::MakeStageMarker(Point.Stage, Point.Score, Point.ElapsedSeconds));
}

void UT66RunStateSubsystem::SetCurrentStage(int32 Stage)
{
	const int32 NewStage = FMath::Clamp(Stage, 1, 23);
	if (CurrentStage == NewStage) return;

	// If Speed Run Mode is enabled, record the stage completion time for the stage we're leaving.
	// This is used for the main menu Speed Run leaderboard (stage 1..5 per difficulty).
	{
		UGameInstance* GI = GetGameInstance();
		UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
		const bool bSpeedRunMode = PS ? PS->GetSpeedRunMode() : false;
		UWorld* World = GetWorld();
		const float Now = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
		const float Elapsed = (bSpeedRunStarted && SpeedRunStartWorldSeconds > 0.f)
			? FMath::Max(0.f, Now - SpeedRunStartWorldSeconds)
			: FMath::Max(0.f, SpeedRunElapsedSeconds);
		CompletedStageActiveSeconds += Elapsed;
		RecordStagePacingPoint(CurrentStage, CompletedStageActiveSeconds);
		static_cast<void>(bSpeedRunMode);
	}

	CurrentStage = NewStage;

	// New stage: clear transient movement/status effects so the Start Area is clean.
	StageMoveSpeedMultiplier = 1.f;
	StageMoveSpeedSecondsRemaining = 0.f;
	StatusBurnSecondsRemaining = 0.f;
	StatusBurnDamagePerSecond = 0.f;
	StatusBurnAccumDamage = 0.f;
	StatusChillSecondsRemaining = 0.f;
	StatusChillMoveSpeedMultiplier = 1.f;
	StatusCurseSecondsRemaining = 0.f;
	StatusEffectsChanged.Broadcast();
	HeroProgressChanged.Broadcast();
	// Bible: gambler anger resets at end of every stage.
	ResetGamblerAnger();
	ResetVendorForStage();
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		IdolManager->HandleStageChanged(NewStage);
	}
	StageChanged.Broadcast();
}

void UT66RunStateSubsystem::StartSpeedRunTimer(const bool bResetElapsed)
{
	UWorld* World = GetWorld();
	const float ExistingElapsed = bResetElapsed ? 0.f : FMath::Max(0.f, SpeedRunElapsedSeconds);
	SpeedRunElapsedSeconds = ExistingElapsed;
	bSpeedRunStarted = true;
	SpeedRunStartWorldSeconds = World ? (static_cast<float>(World->GetTimeSeconds()) - ExistingElapsed) : 0.f;
	LastBroadcastSpeedRunSecond = bResetElapsed ? -1 : FMath::FloorToInt(ExistingElapsed);
	SpeedRunTimerChanged.Broadcast();
}

void UT66RunStateSubsystem::StopSpeedRunTimer(const bool bKeepElapsed)
{
	if (!bSpeedRunStarted)
	{
		if (!bKeepElapsed && !FMath::IsNearlyZero(SpeedRunElapsedSeconds))
		{
			SpeedRunElapsedSeconds = 0.f;
			LastBroadcastSpeedRunSecond = -1;
			SpeedRunTimerChanged.Broadcast();
		}
		return;
	}

	if (bKeepElapsed && SpeedRunStartWorldSeconds > 0.f && GetWorld())
	{
		SpeedRunElapsedSeconds = FMath::Max(0.f, static_cast<float>(GetWorld()->GetTimeSeconds()) - SpeedRunStartWorldSeconds);
	}
	else if (!bKeepElapsed)
	{
		SpeedRunElapsedSeconds = 0.f;
	}

	SpeedRunStartWorldSeconds = 0.f;
	bSpeedRunStarted = false;
	LastBroadcastSpeedRunSecond = bKeepElapsed ? FMath::FloorToInt(SpeedRunElapsedSeconds) : -1;
	SpeedRunTimerChanged.Broadcast();
}

void UT66RunStateSubsystem::ResetSpeedRunTimer()
{
	SpeedRunElapsedSeconds = 0.f;
	SpeedRunStartWorldSeconds = 0.f;
	bSpeedRunStarted = false;
	LastBroadcastSpeedRunSecond = -1;
	SpeedRunTimerChanged.Broadcast();
}

void UT66RunStateSubsystem::SetStageTimerActive(bool bActive)
{
	if (bStageTimerActive == bActive) return;
	bStageTimerActive = bActive;

	StageTimerChanged.Broadcast();
}

void UT66RunStateSubsystem::TickStageTimer(float DeltaTime)
{
	if (!bStageTimerActive || StageTimerSecondsRemaining <= 0.f) return;
	StageTimerSecondsRemaining = FMath::Max(0.f, StageTimerSecondsRemaining - DeltaTime);
	const int32 CurrentSecond = FMath::FloorToInt(StageTimerSecondsRemaining);
	if (CurrentSecond != LastBroadcastStageTimerSecond)
	{
		LastBroadcastStageTimerSecond = CurrentSecond;
		StageTimerChanged.Broadcast();
	}
}

void UT66RunStateSubsystem::TickSpeedRunTimer(float DeltaTime)
{
	(void)DeltaTime;
	if (!bSpeedRunStarted) return;
	UWorld* World = GetWorld();
	if (!World) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	SpeedRunElapsedSeconds = FMath::Max(0.f, Now - SpeedRunStartWorldSeconds);

	const int32 CurrentSecond = FMath::FloorToInt(SpeedRunElapsedSeconds);
	if (CurrentSecond != LastBroadcastSpeedRunSecond)
	{
		LastBroadcastSpeedRunSecond = CurrentSecond;
		SpeedRunTimerChanged.Broadcast();
	}
}

void UT66RunStateSubsystem::ResetStageTimerToFull()
{
	bStageTimerActive = false;
	StageTimerSecondsRemaining = StageTimerDurationSeconds;
	LastBroadcastStageTimerSecond = static_cast<int32>(StageTimerDurationSeconds);
	StageTimerChanged.Broadcast();
}

void UT66RunStateSubsystem::ResetDifficultyPacing()
{
	StagePacingPoints.Reset();
	CompletedStageActiveSeconds = 0.f;
	FinalRunElapsedSeconds = 0.f;
	ResetSpeedRunTimer();
	bThisRunSetNewPersonalBestSpeedRunTime = false;
}

void UT66RunStateSubsystem::SetBossActive(int32 InMaxHP)
{
	SetBossActiveWithId(NAME_None, InMaxHP);
}

void UT66RunStateSubsystem::SetBossActiveWithId(FName InBossID, int32 InMaxHP)
{
	bBossActive = true;
	ActiveBossID = InBossID;
	BossMaxHP = FMath::Max(1, InMaxHP);
	BossCurrentHP = BossMaxHP;
	BossPartSnapshots.Reset();
	BossPartSnapshots.Shrink();
	BossPartSnapshots.AddDefaulted();
	BossPartSnapshots[0].PartID = FName(TEXT("Core"));
	BossPartSnapshots[0].HitZoneType = ET66HitZoneType::Core;
	BossPartSnapshots[0].MaxHP = BossMaxHP;
	BossPartSnapshots[0].CurrentHP = BossCurrentHP;
	BossChanged.Broadcast();
}

void UT66RunStateSubsystem::SetBossActiveWithParts(FName InBossID, const TArray<FT66BossPartSnapshot>& InBossParts)
{
	bBossActive = true;
	ActiveBossID = InBossID;
	BossPartSnapshots = InBossParts;
	T66_RecomputeBossAggregate(BossPartSnapshots, BossMaxHP, BossCurrentHP);
	BossChanged.Broadcast();
}

void UT66RunStateSubsystem::RescaleBossMaxHPPreservePercent(int32 NewMaxHP)
{
	if (!bBossActive) return;

	if (BossPartSnapshots.Num() > 0)
	{
		const int32 TargetMaxHP = FMath::Max(1, NewMaxHP);
		const int32 OldTotalMaxHP = FMath::Max(1, BossMaxHP);
		int32 RemainingHP = TargetMaxHP;

		for (int32 Index = 0; Index < BossPartSnapshots.Num(); ++Index)
		{
			FT66BossPartSnapshot& Part = BossPartSnapshots[Index];
			const int32 OldPartMaxHP = FMath::Max(1, Part.MaxHP);
			const int32 OldPartCurrentHP = FMath::Clamp(Part.CurrentHP, 0, OldPartMaxHP);
			const float PartPct = FMath::Clamp(static_cast<float>(OldPartCurrentHP) / static_cast<float>(OldPartMaxHP), 0.f, 1.f);
			const int32 RemainingParts = BossPartSnapshots.Num() - Index;

			int32 NewPartMaxHP = 1;
			if (Index == BossPartSnapshots.Num() - 1)
			{
				NewPartMaxHP = FMath::Max(1, RemainingHP);
			}
			else
			{
				NewPartMaxHP = FMath::Clamp(
					FMath::RoundToInt(static_cast<float>(TargetMaxHP) * static_cast<float>(OldPartMaxHP) / static_cast<float>(OldTotalMaxHP)),
					1,
					FMath::Max(1, RemainingHP - (RemainingParts - 1)));
			}

			Part.MaxHP = NewPartMaxHP;
			Part.CurrentHP = FMath::Clamp(FMath::RoundToInt(static_cast<float>(NewPartMaxHP) * PartPct), 0, NewPartMaxHP);
			RemainingHP = FMath::Max(0, RemainingHP - NewPartMaxHP);
		}

		T66_RecomputeBossAggregate(BossPartSnapshots, BossMaxHP, BossCurrentHP);
		BossChanged.Broadcast();
		return;
	}

	const int32 PrevMax = FMath::Max(1, BossMaxHP);
	const int32 PrevCur = FMath::Clamp(BossCurrentHP, 0, PrevMax);
	const float Pct = FMath::Clamp(static_cast<float>(PrevCur) / static_cast<float>(PrevMax), 0.f, 1.f);

	BossMaxHP = FMath::Max(1, NewMaxHP);
	BossCurrentHP = FMath::Clamp(FMath::RoundToInt(static_cast<float>(BossMaxHP) * Pct), 0, BossMaxHP);
	BossChanged.Broadcast();
}

void UT66RunStateSubsystem::SetBossInactive()
{
	bBossActive = false;
	ActiveBossID = NAME_None;
	BossPartSnapshots.Reset();
	BossCurrentHP = 0;
	BossChanged.Broadcast();
}

bool UT66RunStateSubsystem::ApplyBossDamage(int32 Damage)
{
	if (!bBossActive || Damage <= 0 || BossCurrentHP <= 0) return false;

	if (BossPartSnapshots.Num() > 0)
	{
		for (FT66BossPartSnapshot& Part : BossPartSnapshots)
		{
			if (Part.CurrentHP <= 0)
			{
				continue;
			}

			Part.CurrentHP = FMath::Max(0, Part.CurrentHP - Damage);
			T66_RecomputeBossAggregate(BossPartSnapshots, BossMaxHP, BossCurrentHP);
			BossChanged.Broadcast();
			return BossCurrentHP <= 0;
		}
	}

	BossCurrentHP = FMath::Max(0, BossCurrentHP - Damage);
	if (BossPartSnapshots.Num() == 1)
	{
		BossPartSnapshots[0].MaxHP = BossMaxHP;
		BossPartSnapshots[0].CurrentHP = BossCurrentHP;
	}
	BossChanged.Broadcast();
	return BossCurrentHP <= 0;
}

void UT66RunStateSubsystem::ResetBossState()
{
	bBossActive = false;
	ActiveBossID = NAME_None;
	BossMaxHP = 100;
	BossCurrentHP = 0;
	BossPartSnapshots.Reset();
	BossChanged.Broadcast();
}

void UT66RunStateSubsystem::AddScore(int32 Points)
{
	if (Points <= 0) return;
	CurrentScore += Points;
	ScoreChanged.Broadcast();
}

void UT66RunStateSubsystem::AddStructuredEvent(ET66RunEventType EventType, const FString& Payload)
{
	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	const float Timestamp = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
	StructuredEventLog.Add(FRunEvent(EventType, Timestamp, Payload));
	// Append human-readable line to EventLog for Run Summary scroll
	FString Short = Payload;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	switch (EventType)
	{
		case ET66RunEventType::StageEntered:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_StageFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Stage %s"), *Payload);
			break;
		case ET66RunEventType::ItemAcquired:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_PickedUpFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Picked up %s"), *Payload);
			break;
		case ET66RunEventType::GoldGained:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_GoldFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Gold: %s"), *Payload);
			break;
		case ET66RunEventType::EnemyKilled:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_KillFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Kill +%s"), *Payload);
			break;
		default:
			break;
	}
	EventLog.Add(Short);
	TrimLogsIfNeeded();
}

void UT66RunStateSubsystem::ToggleHUDPanels()
{
	bHUDPanelsVisible = !bHUDPanelsVisible;
	PanelVisibilityChanged.Broadcast();
}

void UT66RunStateSubsystem::AddLogEntry(const FString& Entry)
{
	EventLog.Add(Entry);
	TrimLogsIfNeeded();
}
