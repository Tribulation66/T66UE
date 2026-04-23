// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Core/T66RunStateSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Core/T66CommunityContentSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardPacingUtils.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunIntegritySubsystem.h"
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

namespace T66RunStatePrivate
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
