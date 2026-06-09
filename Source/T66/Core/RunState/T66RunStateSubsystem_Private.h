// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Core/T66RunStateSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Core/T66CommunityContentSubsystem.h"
#include "Core/T66DifficultyTuningSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66WeaponManagerSubsystem.h"
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
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66VendorBoss.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Subsystems/SubsystemCollection.h"
#include "Gameplay/T66CombatShared.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66MobBase.h"

namespace T66RunStatePrivate
{
	static const FName T66VendorTokenItemID(TEXT("Item_VendorToken"));
	static int32 T66_GetDefaultInventoryRollSeed()
	{
		return static_cast<int32>(FPlatformTime::Cycles());
	}

	static int32 T66_CombineInventorySeed(const FT66InventorySlot& Slot, const int32 SeedSalt)
	{
		uint32 Seed = static_cast<uint32>(Slot.RollSeed != 0 ? Slot.RollSeed : GetTypeHash(Slot.ItemTemplateID));
		Seed = HashCombine(Seed, GetTypeHash(static_cast<uint8>(Slot.Rarity)));
		Seed = HashCombine(Seed, GetTypeHash(Slot.Line1RolledValue));
		Seed = HashCombine(Seed, GetTypeHash(Slot.GetStatBonusValue()));
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

	static bool T66_IsVendorTokenItem(const FName ItemID)
	{
		return ItemID == T66VendorTokenItemID;
	}

	static bool T66_IsRetiredRemovedItemID(const FName ItemID)
	{
		return ItemID == FName(TEXT("Item_Accuracy"))
			|| ItemID == FName(TEXT("Item_CritDamage"))
			|| ItemID == FName(TEXT("Item_Cheating"))
			|| ItemID == FName(TEXT("Item_Stealing"))
			|| ItemID == FName(TEXT("Item_LootCrate"))
			|| ItemID == FName(TEXT("Item_TreasureChest"))
			|| ItemID == FName(TEXT("Item_LootBag"))
			|| ItemID == FName(TEXT("Item_LootWheel"))
			|| ItemID == FName(TEXT("Item_HpRegen"))
			|| ItemID == FName(TEXT("Item_LifeSteal"));
	}

	static bool T66_IsBackroomsQuickReviveItem(const FName ItemID)
	{
		return ItemID == UT66RunStateSubsystem::BackroomsQuickReviveItemID;
	}

	static bool T66_IsKromerItem(const FName ItemID)
	{
		return ItemID == UT66RunStateSubsystem::KromerItemID;
	}

	static bool T66_IsMobLootItem(const FName ItemID)
	{
		return ItemID == UT66RunStateSubsystem::MobLootItemID;
	}

	static bool T66_IsRewardOnlySpecialItem(const FName ItemID)
	{
		return T66_IsVendorTokenItem(ItemID) || T66_IsBackroomsQuickReviveItem(ItemID) || T66_IsKromerItem(ItemID) || T66_IsMobLootItem(ItemID);
	}

	static int32 T66_ClampVendorTokenStackCount(const int32 StackCount)
	{
		return FMath::Clamp(StackCount, 0, UT66RunStateSubsystem::MaxVendorTokenStacks);
	}

	static bool T66_IsAlchemyEligibleSlot(const FT66InventorySlot& Slot, const UT66GameInstance* GI)
	{
		if (!Slot.IsValid() || Slot.Rarity == ET66ItemRarity::White || T66_IsRewardOnlySpecialItem(Slot.ItemTemplateID))
		{
			return false;
		}

		if (!GI)
		{
			return true;
		}

		FItemData ItemData;
		return const_cast<UT66GameInstance*>(GI)->GetItemData(Slot.ItemTemplateID, ItemData)
			&& ItemData.StatType != ET66StatType::VendorToken;
	}

	static bool T66_IsAlchemyMatch(const FT66InventorySlot& A, const FT66InventorySlot& B)
	{
		return A.IsValid() && B.IsValid() && A.ItemTemplateID == B.ItemTemplateID && A.Rarity == B.Rarity;
	}

	static int32 T66_MapBlessingRollToWhiteRange(const FT66InventorySlot& Slot)
	{
		(void)Slot;
		return FItemData::GetFlatStatBonus(ET66ItemRarity::White);
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
		UpgradedSlot.StatBonusOverride = FItemData::GetAlchemyFlatStatBonus(UpgradedSlot.Rarity);
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
		Slot.StatBonusOverride = FMath::Max(
			Slot.StatBonusOverride,
			FItemData::GetAlchemyFlatStatBonus(Slot.Rarity));
		Slot.Line2MultiplierOverride = FItemData::GetLine2RarityMultiplier(Slot.Rarity);
		Slot.RollSeed = HashCombine(GetTypeHash(PreviousRarity), GetTypeHash(Slot.RollSeed != 0 ? Slot.RollSeed : T66_GetDefaultInventoryRollSeed()));
	}

	static float T66_GetSellFractionForVendorTokenStacks(const int32 StackCount)
	{
		return FMath::Clamp(0.70f + 0.025f * static_cast<float>(T66_ClampVendorTokenStackCount(StackCount)), 0.70f, 1.00f);
	}

	static float T66_GetBuyDiscountFractionForVendorTokenStacks(const int32 StackCount)
	{
		return 0.025f * static_cast<float>(T66_ClampVendorTokenStackCount(StackCount));
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
