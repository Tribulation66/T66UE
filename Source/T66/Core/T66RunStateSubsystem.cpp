// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RunStateSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66PowerUpSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66SkillRatingSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Gameplay/T66VendorBoss.h"
#include "Core/T66DamageLogSubsystem.h"

namespace
{
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
}

void UT66RunStateSubsystem::ResetLuckRatingTracking()
{
	LuckQuantityByCategory.Reset();
	LuckQualityByCategory.Reset();
}

void UT66RunStateSubsystem::RecordLuckQuantityRoll(FName Category, int32 RolledValue, int32 MinValue, int32 MaxValue)
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
}

void UT66RunStateSubsystem::RecordLuckQuantityBool(FName Category, bool bSucceeded)
{
	LuckQuantityByCategory.FindOrAdd(Category).Add01(bSucceeded ? 1.f : 0.f);
}

void UT66RunStateSubsystem::RecordLuckQualityRarity(FName Category, ET66Rarity Rarity)
{
	LuckQualityByCategory.FindOrAdd(Category).Add01(T66_RarityTo01(Rarity));
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

const TArray<FName>& UT66RunStateSubsystem::GetAllIdolIDs()
{
	// 24 canonical idol IDs grouped by category:
	//   DOT (6): Curse, Lava, Poison, Decay, Bleed, Acid
	//   Bounce (6): Electric, Ice, Sound, Shadow, Star, Rubber
	//   AOE (6): Fire, Earth, Water, Sand, BlackHole, Storm
	//   Pierce (6): Light, Wind, Steel, Wood, Bone, Glass
	static const TArray<FName> Idols = {
		FName(TEXT("Idol_Curse")),
		FName(TEXT("Idol_Lava")),
		FName(TEXT("Idol_Poison")),
		FName(TEXT("Idol_Decay")),
		FName(TEXT("Idol_Bleed")),
		FName(TEXT("Idol_Acid")),
		FName(TEXT("Idol_Electric")),
		FName(TEXT("Idol_Ice")),
		FName(TEXT("Idol_Sound")),
		FName(TEXT("Idol_Shadow")),
		FName(TEXT("Idol_Star")),
		FName(TEXT("Idol_Rubber")),
		FName(TEXT("Idol_Fire")),
		FName(TEXT("Idol_Earth")),
		FName(TEXT("Idol_Water")),
		FName(TEXT("Idol_Sand")),
		FName(TEXT("Idol_BlackHole")),
		FName(TEXT("Idol_Storm")),
		FName(TEXT("Idol_Light")),
		FName(TEXT("Idol_Wind")),
		FName(TEXT("Idol_Steel")),
		FName(TEXT("Idol_Wood")),
		FName(TEXT("Idol_Bone")),
		FName(TEXT("Idol_Glass")),
	};
	return Idols;
}

FLinearColor UT66RunStateSubsystem::GetIdolColor(FName IdolID)
{
	// Back-compat aliases (legacy → closest new idol).
	if (IdolID == FName(TEXT("Idol_Shock")))     IdolID = FName(TEXT("Idol_Electric"));
	if (IdolID == FName(TEXT("Idol_Silence")))    IdolID = FName(TEXT("Idol_Shadow"));
	if (IdolID == FName(TEXT("Idol_Mark")))       IdolID = FName(TEXT("Idol_Light"));
	if (IdolID == FName(TEXT("Idol_Pierce")))     IdolID = FName(TEXT("Idol_Steel"));
	if (IdolID == FName(TEXT("Idol_Split")))      IdolID = FName(TEXT("Idol_Wind"));
	if (IdolID == FName(TEXT("Idol_Knockback")))  IdolID = FName(TEXT("Idol_Wood"));
	if (IdolID == FName(TEXT("Idol_Ricochet")))   IdolID = FName(TEXT("Idol_Rubber"));
	if (IdolID == FName(TEXT("Idol_Hex")))        IdolID = FName(TEXT("Idol_Curse"));
	if (IdolID == FName(TEXT("Idol_Lifesteal")))  IdolID = FName(TEXT("Idol_Bleed"));
	if (IdolID == FName(TEXT("Idol_Lightning")))  IdolID = FName(TEXT("Idol_Electric"));
	if (IdolID == FName(TEXT("Idol_Darkness")))   IdolID = FName(TEXT("Idol_Shadow"));
	if (IdolID == FName(TEXT("Idol_Metal")))      IdolID = FName(TEXT("Idol_Steel"));
	if (IdolID == FName(TEXT("Idol_Spectral")))   IdolID = FName(TEXT("Idol_Curse"));
	if (IdolID == FName(TEXT("Idol_Frost")))      IdolID = FName(TEXT("Idol_Ice"));
	if (IdolID == FName(TEXT("Idol_Glue")))       IdolID = FName(TEXT("Idol_Decay"));

	// DOT idols
	if (IdolID == FName(TEXT("Idol_Curse")))     return FLinearColor(0.50f, 0.10f, 0.60f, 1.f);
	if (IdolID == FName(TEXT("Idol_Lava")))      return FLinearColor(0.95f, 0.40f, 0.05f, 1.f);
	if (IdolID == FName(TEXT("Idol_Poison")))    return FLinearColor(0.20f, 0.85f, 0.35f, 1.f);
	if (IdolID == FName(TEXT("Idol_Decay")))     return FLinearColor(0.45f, 0.40f, 0.20f, 1.f);
	if (IdolID == FName(TEXT("Idol_Bleed")))     return FLinearColor(0.80f, 0.10f, 0.10f, 1.f);
	if (IdolID == FName(TEXT("Idol_Acid")))      return FLinearColor(0.60f, 0.90f, 0.10f, 1.f);

	// Bounce idols
	if (IdolID == FName(TEXT("Idol_Electric")))  return FLinearColor(0.70f, 0.25f, 0.95f, 1.f);
	if (IdolID == FName(TEXT("Idol_Ice")))       return FLinearColor(0.35f, 0.75f, 1.00f, 1.f);
	if (IdolID == FName(TEXT("Idol_Sound")))     return FLinearColor(0.85f, 0.75f, 0.95f, 1.f);
	if (IdolID == FName(TEXT("Idol_Shadow")))    return FLinearColor(0.10f, 0.10f, 0.12f, 1.f);
	if (IdolID == FName(TEXT("Idol_Star")))      return FLinearColor(0.95f, 0.90f, 0.50f, 1.f);
	if (IdolID == FName(TEXT("Idol_Rubber")))    return FLinearColor(0.90f, 0.55f, 0.30f, 1.f);

	// AOE idols
	if (IdolID == FName(TEXT("Idol_Fire")))      return FLinearColor(0.95f, 0.25f, 0.10f, 1.f);
	if (IdolID == FName(TEXT("Idol_Earth")))     return FLinearColor(0.55f, 0.40f, 0.25f, 1.f);
	if (IdolID == FName(TEXT("Idol_Water")))     return FLinearColor(0.20f, 0.60f, 0.95f, 1.f);
	if (IdolID == FName(TEXT("Idol_Sand")))      return FLinearColor(0.90f, 0.85f, 0.55f, 1.f);
	if (IdolID == FName(TEXT("Idol_BlackHole"))) return FLinearColor(0.15f, 0.05f, 0.25f, 1.f);
	if (IdolID == FName(TEXT("Idol_Storm")))     return FLinearColor(0.40f, 0.50f, 0.70f, 1.f);

	// Pierce idols
	if (IdolID == FName(TEXT("Idol_Light")))     return FLinearColor(0.92f, 0.92f, 0.98f, 1.f);
	if (IdolID == FName(TEXT("Idol_Wind")))      return FLinearColor(0.70f, 0.90f, 0.80f, 1.f);
	if (IdolID == FName(TEXT("Idol_Steel")))     return FLinearColor(0.55f, 0.55f, 0.75f, 1.f);
	if (IdolID == FName(TEXT("Idol_Wood")))      return FLinearColor(0.35f, 0.65f, 0.25f, 1.f);
	if (IdolID == FName(TEXT("Idol_Bone")))      return FLinearColor(0.90f, 0.88f, 0.80f, 1.f);
	if (IdolID == FName(TEXT("Idol_Glass")))     return FLinearColor(0.85f, 0.92f, 0.95f, 1.f);

	return FLinearColor(0.25f, 0.25f, 0.28f, 1.f);
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

	TWeakObjectPtr<AActor> Key(Target);
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

	TArray<TWeakObjectPtr<AActor>> ToRemove;
	for (auto& Pair : ActiveDOTs)
	{
		if (!Pair.Key.IsValid())
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
				DOTDamageApplier(Pair.Key.Get(), Damage, Inst.SourceIdolID);
			}
			Inst.NextTickTime = Now + Inst.TickInterval;
		}
	}
	for (const TWeakObjectPtr<AActor>& K : ToRemove)
	{
		ActiveDOTs.Remove(K);
	}
	if (ActiveDOTs.Num() == 0 && World && DOTTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(DOTTimerHandle);
		DOTTimerHandle.Invalidate();
	}
}

bool UT66RunStateSubsystem::EquipIdolInSlot(int32 SlotIndex, FName IdolID)
{
	if (SlotIndex < 0 || SlotIndex >= MaxEquippedIdolSlots) return false;
	if (IdolID.IsNone()) return false;

	if (EquippedIdolIDs.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
	}
	if (EquippedIdolLevels.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolLevels.Init(0, MaxEquippedIdolSlots);
	}

	if (EquippedIdolIDs[SlotIndex] == IdolID) return false;
	EquippedIdolIDs[SlotIndex] = IdolID;
	EquippedIdolLevels[SlotIndex] = 1;
	IdolsChanged.Broadcast();
	return true;
}

bool UT66RunStateSubsystem::EquipIdolFirstEmpty(FName IdolID)
{
	if (IdolID.IsNone()) return false;
	if (EquippedIdolIDs.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
	}
	if (EquippedIdolLevels.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolLevels.Init(0, MaxEquippedIdolSlots);
	}

	for (int32 i = 0; i < EquippedIdolIDs.Num(); ++i)
	{
		if (EquippedIdolIDs[i].IsNone())
		{
			EquippedIdolIDs[i] = IdolID;
			EquippedIdolLevels[i] = 1;
			IdolsChanged.Broadcast();
			return true;
		}
	}
	return false;
}

int32 UT66RunStateSubsystem::GetEquippedIdolLevelInSlot(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxEquippedIdolSlots) return 0;
	if (EquippedIdolIDs.Num() != MaxEquippedIdolSlots) return 0;
	if (SlotIndex >= EquippedIdolIDs.Num()) return 0;
	if (EquippedIdolIDs[SlotIndex].IsNone()) return 0;
	if (EquippedIdolLevels.Num() != MaxEquippedIdolSlots) return 1; // back-compat default
	return FMath::Clamp(static_cast<int32>(EquippedIdolLevels[SlotIndex]), 1, MaxIdolLevel);
}

bool UT66RunStateSubsystem::SelectIdolFromAltar(FName IdolID)
{
	if (IdolID.IsNone()) return false;

	if (EquippedIdolIDs.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
	}
	if (EquippedIdolLevels.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolLevels.Init(0, MaxEquippedIdolSlots);
	}

	// If already equipped, level it up.
	for (int32 i = 0; i < EquippedIdolIDs.Num(); ++i)
	{
		if (EquippedIdolIDs[i] == IdolID)
		{
			const int32 Cur = FMath::Clamp(static_cast<int32>(EquippedIdolLevels[i]), 1, MaxIdolLevel);
			const int32 Next = FMath::Clamp(Cur + 1, 1, MaxIdolLevel);
			if (Next == Cur) return false;
			EquippedIdolLevels[i] = static_cast<uint8>(Next);
			IdolsChanged.Broadcast();
			return true;
		}
	}

	// Otherwise, equip into first empty slot (level 1).
	for (int32 i = 0; i < EquippedIdolIDs.Num(); ++i)
	{
		if (EquippedIdolIDs[i].IsNone())
		{
			EquippedIdolIDs[i] = IdolID;
			EquippedIdolLevels[i] = 1;
			IdolsChanged.Broadcast();
			return true;
		}
	}
	return false;
}

void UT66RunStateSubsystem::ClearEquippedIdols()
{
	if (EquippedIdolIDs.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
		EquippedIdolLevels.Init(0, MaxEquippedIdolSlots);
		IdolsChanged.Broadcast();
		return;
	}
	if (EquippedIdolLevels.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolLevels.Init(0, MaxEquippedIdolSlots);
	}

	bool bAny = false;
	for (FName& N : EquippedIdolIDs)
	{
		if (!N.IsNone())
		{
			N = NAME_None;
			bAny = true;
		}
	}
	if (bAny)
	{
		EquippedIdolLevels.Init(0, MaxEquippedIdolSlots);
		IdolsChanged.Broadcast();
	}
}

void UT66RunStateSubsystem::EnsureIdolStock()
{
	if (IdolStockIDs.Num() == IdolStockSlotCount)
	{
		return; // Already generated.
	}
	RerollIdolStock();
}

void UT66RunStateSubsystem::RerollIdolStock()
{
	IdolStockIDs.Empty(IdolStockSlotCount);
	IdolStockSelected.Init(false, IdolStockSlotCount);

	const TArray<FName>& AllIdols = GetAllIdolIDs();

	// Build pool: exclude idols that are already equipped at max level.
	TArray<FName> Pool;
	Pool.Reserve(AllIdols.Num());
	for (const FName& ID : AllIdols)
	{
		bool bMaxed = false;
		for (int32 i = 0; i < EquippedIdolIDs.Num(); ++i)
		{
			if (EquippedIdolIDs[i] == ID)
			{
				const int32 Lvl = (EquippedIdolLevels.IsValidIndex(i))
					? FMath::Clamp(static_cast<int32>(EquippedIdolLevels[i]), 1, MaxIdolLevel)
					: 1;
				if (Lvl >= MaxIdolLevel)
				{
					bMaxed = true;
				}
				break;
			}
		}
		if (!bMaxed)
		{
			Pool.Add(ID);
		}
	}

	// Shuffle and pick up to IdolStockSlotCount.
	const int32 Count = FMath::Min(IdolStockSlotCount, Pool.Num());
	for (int32 i = Pool.Num() - 1; i > 0; --i)
	{
		const int32 j = FMath::RandRange(0, i);
		Pool.Swap(i, j);
	}
	for (int32 i = 0; i < Count; ++i)
	{
		IdolStockIDs.Add(Pool[i]);
	}

	// Pad with NAME_None if pool was too small.
	while (IdolStockIDs.Num() < IdolStockSlotCount)
	{
		IdolStockIDs.Add(NAME_None);
	}

	IdolsChanged.Broadcast();
}

bool UT66RunStateSubsystem::SelectIdolFromStock(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= IdolStockSlotCount) return false;
	if (!IdolStockIDs.IsValidIndex(SlotIndex)) return false;
	if (IdolStockIDs[SlotIndex].IsNone()) return false;
	if (IdolStockSelected.IsValidIndex(SlotIndex) && IdolStockSelected[SlotIndex]) return false;

	const bool bApplied = SelectIdolFromAltar(IdolStockIDs[SlotIndex]);
	if (bApplied && IdolStockSelected.IsValidIndex(SlotIndex))
	{
		IdolStockSelected[SlotIndex] = true;
	}
	return bApplied;
}

bool UT66RunStateSubsystem::IsIdolStockSlotSelected(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= IdolStockSlotCount) return false;
	return IdolStockSelected.IsValidIndex(SlotIndex) && IdolStockSelected[SlotIndex];
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
	if (MaxHP <= 0.f) return 0;
	const float T = FMath::LogX(HeartTierScale, MaxHP / DefaultMaxHP);
	return FMath::Clamp(FMath::RoundToInt(T), 0, 4);
}

float UT66RunStateSubsystem::GetHeartSlotFill(int32 SlotIndex) const
{
	const int32 Tier = GetHeartDisplayTier();
	const float HPPerHeart = HPPerRedHeart * FMath::Pow(HeartTierScale, static_cast<float>(Tier));
	if (HPPerHeart <= 0.f || SlotIndex < 0 || SlotIndex >= 5) return 0.f;
	const float SlotStart = static_cast<float>(SlotIndex) * HPPerHeart;
	const float SlotEnd = SlotStart + HPPerHeart;
	if (CurrentHP <= SlotStart) return 0.f;
	if (CurrentHP >= SlotEnd) return 1.f;
	return (CurrentHP - SlotStart) / HPPerHeart;
}

void UT66RunStateSubsystem::AddMaxHearts(int32 DeltaHearts)
{
	if (DeltaHearts <= 0) return;
	const int32 Tier = GetHeartDisplayTier();
	const float HPPerHeart = HPPerRedHeart * FMath::Pow(HeartTierScale, static_cast<float>(Tier));
	const float Added = HPPerHeart * static_cast<float>(DeltaHearts);
	MaxHP = FMath::Clamp(MaxHP + Added, DefaultMaxHP, 99999.f);
	CurrentHP = FMath::Clamp(CurrentHP + Added, 0.f, MaxHP);
	HeartsChanged.Broadcast();
}

int32 UT66RunStateSubsystem::RegisterTotemActivated()
{
	TotemsActivatedCount = FMath::Clamp(TotemsActivatedCount + 1, 0, 999);
	return TotemsActivatedCount;
}

float UT66RunStateSubsystem::AddGamblerAngerFromBet(int32 BetGold)
{
	const float Delta = FMath::Clamp(static_cast<float>(FMath::Max(0, BetGold)) / 100.f, 0.f, 1.f);
	GamblerAnger01 = FMath::Clamp(GamblerAnger01 + Delta, 0.f, 1.f);
	GamblerAngerChanged.Broadcast();
	return GamblerAnger01;
}

void UT66RunStateSubsystem::ResetGamblerAnger()
{
	if (FMath::IsNearlyZero(GamblerAnger01)) return;
	GamblerAnger01 = 0.f;
	GamblerAngerChanged.Broadcast();
}

void UT66RunStateSubsystem::InitializeHeroStatTuningForSelectedHero()
{
	// Safe defaults
	HeroStats = FT66HeroStatBlock{};
	HeroPerLevelGains = FT66HeroPerLevelStatGains{};
	HeroPerLevelGains.Damage.Min = 1;      HeroPerLevelGains.Damage.Max = 2;
	HeroPerLevelGains.AttackSpeed.Min = 1; HeroPerLevelGains.AttackSpeed.Max = 2;
	HeroPerLevelGains.AttackScale.Min = 1; HeroPerLevelGains.AttackScale.Max = 2;
	HeroPerLevelGains.Armor.Min = 1;       HeroPerLevelGains.Armor.Max = 2;
	HeroPerLevelGains.Evasion.Min = 1;     HeroPerLevelGains.Evasion.Max = 2;
	HeroPerLevelGains.Luck.Min = 1;        HeroPerLevelGains.Luck.Max = 2;

	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		FT66HeroStatBlock Base;
		FT66HeroPerLevelStatGains Gains;
		if (T66GI->GetHeroStatTuning(T66GI->SelectedHeroID, Base, Gains))
		{
			HeroStats = Base;
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
			HeroBaseHpRegen = FMath::Max(0.f, HD.BaseHpRegen);
			HeroBaseCrushChance = FMath::Clamp(HD.BaseCrushChance, 0.f, 1.f);
			HeroBaseInvisChance = FMath::Clamp(HD.BaseInvisChance, 0.f, 1.f);
			HeroBaseCounterAttack = FMath::Max(0.f, HD.BaseCounterAttack);
			HeroBaseLifeSteal = FMath::Clamp(HD.BaseLifeSteal, 0.f, 1.f);
			HeroBaseAssassinateChance = FMath::Clamp(HD.BaseAssassinateChance, 0.f, 1.f);
			HeroBaseCheatChance = FMath::Clamp(HD.BaseCheatChance, 0.f, 1.f);
			HeroBaseStealChance = FMath::Clamp(HD.BaseStealChance, 0.f, 1.f);
			float Range = HD.BaseAttackRange;
			HeroBaseAttackRange = FMath::Max(100.f, Range);
			PassiveType = HD.PassiveType;
		}
	}

	// Enforce sane minimums for gameplay safety.
	HeroStats.Damage = FMath::Clamp(HeroStats.Damage, 1, 9999);
	HeroStats.AttackSpeed = FMath::Clamp(HeroStats.AttackSpeed, 1, 9999);
	HeroStats.AttackScale = FMath::Clamp(HeroStats.AttackScale, 1, 9999);
	HeroStats.Armor = FMath::Clamp(HeroStats.Armor, 1, 9999);
	HeroStats.Evasion = FMath::Clamp(HeroStats.Evasion, 1, 9999);
	HeroStats.Luck = FMath::Clamp(HeroStats.Luck, 1, 9999);
	HeroStats.Speed = FMath::Clamp(HeroStats.Speed, 1, 9999);

	RefreshPowerupBonusesFromProfile();
}

void UT66RunStateSubsystem::AddPowerCrystalsEarnedThisRun(int32 Amount)
{
	if (Amount <= 0) return;
	PowerCrystalsEarnedThisRun = FMath::Clamp(PowerCrystalsEarnedThisRun + Amount, 0, 2000000000);
}

void UT66RunStateSubsystem::RefreshPowerupBonusesFromProfile()
{
	PowerupStatBonuses = FT66HeroStatBonuses{};
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PowerUpSubsystem* PowerUp = GI->GetSubsystem<UT66PowerUpSubsystem>())
		{
			PowerupStatBonuses = PowerUp->GetPowerupStatBonuses();
		}
	}
}

void UT66RunStateSubsystem::ApplyOneHeroLevelUp()
{
	// Speed is a foundational stat, but it behaves differently:
	// user request: always +1 Speed per level.
	HeroStats.Speed = FMath::Clamp(HeroStats.Speed + 1, 1, 9999);

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

	auto RollGainBiased = [&](const FT66HeroStatGainRange& Range, FName Category) -> int32
	{
		const int32 A0 = FMath::Min(Range.Min, Range.Max);
		const int32 B0 = FMath::Max(Range.Min, Range.Max);
		const int32 A = FMath::Max(0, A0);
		const int32 B = FMath::Max(0, B0);
		if (B <= 0) return 0;
		if (B <= A) return B;

		float U = HeroStatRng.GetFraction(); // 0..1
		if (RngSub)
		{
			U = RngSub->BiasHigh01(U);
		}
		const int32 Span = (B - A);
		// Inclusive integer roll biased toward B.
		const int32 Delta = FMath::Clamp(FMath::FloorToInt(U * static_cast<float>(Span + 1)), 0, Span);
		const int32 Rolled = A + Delta;
		RecordLuckQuantityRoll(Category, Rolled, A, B);
		return Rolled;
	};

	// Other foundational stats roll within the hero's per-level gain ranges.
	HeroStats.Damage = FMath::Clamp(HeroStats.Damage + RollGainBiased(HeroPerLevelGains.Damage, FName(TEXT("LevelUp_DamageGain"))), 1, 9999);
	HeroStats.AttackSpeed = FMath::Clamp(HeroStats.AttackSpeed + RollGainBiased(HeroPerLevelGains.AttackSpeed, FName(TEXT("LevelUp_AttackSpeedGain"))), 1, 9999);
	HeroStats.AttackScale = FMath::Clamp(HeroStats.AttackScale + RollGainBiased(HeroPerLevelGains.AttackScale, FName(TEXT("LevelUp_AttackScaleGain"))), 1, 9999);
	HeroStats.Armor = FMath::Clamp(HeroStats.Armor + RollGainBiased(HeroPerLevelGains.Armor, FName(TEXT("LevelUp_ArmorGain"))), 1, 9999);
	HeroStats.Evasion = FMath::Clamp(HeroStats.Evasion + RollGainBiased(HeroPerLevelGains.Evasion, FName(TEXT("LevelUp_EvasionGain"))), 1, 9999);
	HeroStats.Luck = FMath::Clamp(HeroStats.Luck + RollGainBiased(HeroPerLevelGains.Luck, FName(TEXT("LevelUp_LuckGain"))), 1, 9999);
}

void UT66RunStateSubsystem::InitializeHeroStatsForNewRun()
{
	// Seed RNG once per run so stage reloads keep the same future stat gain sequence.
	int32 Seed = static_cast<int32>(FPlatformTime::Cycles());
	if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		Seed ^= static_cast<int32>(GetTypeHash(T66GI->SelectedHeroID));
		Seed ^= static_cast<int32>(GetTypeHash(T66GI->SelectedCompanionID));
	}
	Seed ^= (HeroLevel * 1337);
	HeroStatRng.Initialize(Seed);

	InitializeHeroStatTuningForSelectedHero();

	// Apply level-up gains for any non-1 starting level (difficulty boosts start at +10 levels per step).
	const int32 TargetLevel = FMath::Max(1, HeroLevel);
	for (int32 L = 2; L <= TargetLevel; ++L)
	{
		ApplyOneHeroLevelUp();
	}
}

// ============================================
// Category-specific stat getters (base from hero DataTable + item bonuses)
// ============================================

int32 UT66RunStateSubsystem::GetPierceDmgStat() const      { return FMath::Max(1, BasePierceDmg + FMath::Max(0, ItemStatBonuses.PierceDmg)); }
int32 UT66RunStateSubsystem::GetPierceAtkSpdStat() const   { return FMath::Max(1, BasePierceAtkSpd + FMath::Max(0, ItemStatBonuses.PierceAtkSpd)); }
int32 UT66RunStateSubsystem::GetPierceAtkScaleStat() const  { return FMath::Max(1, BasePierceAtkScale + FMath::Max(0, ItemStatBonuses.PierceAtkScale)); }

int32 UT66RunStateSubsystem::GetBounceDmgStat() const      { return FMath::Max(1, BaseBounceDmg + FMath::Max(0, ItemStatBonuses.BounceDmg)); }
int32 UT66RunStateSubsystem::GetBounceAtkSpdStat() const   { return FMath::Max(1, BaseBounceAtkSpd + FMath::Max(0, ItemStatBonuses.BounceAtkSpd)); }
int32 UT66RunStateSubsystem::GetBounceAtkScaleStat() const  { return FMath::Max(1, BaseBounceAtkScale + FMath::Max(0, ItemStatBonuses.BounceAtkScale)); }

int32 UT66RunStateSubsystem::GetAoeDmgStat() const         { return FMath::Max(1, BaseAoeDmg + FMath::Max(0, ItemStatBonuses.AoeDmg)); }
int32 UT66RunStateSubsystem::GetAoeAtkSpdStat() const      { return FMath::Max(1, BaseAoeAtkSpd + FMath::Max(0, ItemStatBonuses.AoeAtkSpd)); }
int32 UT66RunStateSubsystem::GetAoeAtkScaleStat() const     { return FMath::Max(1, BaseAoeAtkScale + FMath::Max(0, ItemStatBonuses.AoeAtkScale)); }

int32 UT66RunStateSubsystem::GetDotDmgStat() const         { return FMath::Max(1, BaseDotDmg + FMath::Max(0, ItemStatBonuses.DotDmg)); }
int32 UT66RunStateSubsystem::GetDotAtkSpdStat() const      { return FMath::Max(1, BaseDotAtkSpd + FMath::Max(0, ItemStatBonuses.DotAtkSpd)); }
int32 UT66RunStateSubsystem::GetDotAtkScaleStat() const     { return FMath::Max(1, BaseDotAtkScale + FMath::Max(0, ItemStatBonuses.DotAtkScale)); }

// ============================================
// Secondary stat getters (hero base * accumulated item Line 2 multipliers)
// ============================================

float UT66RunStateSubsystem::GetSecondaryStatValue(ET66SecondaryStatType StatType) const
{
	const float* Mult = SecondaryMultipliers.Find(StatType);
	const float M = (Mult && *Mult > 0.f) ? *Mult : 1.f;
	const float DamageMult = GetHeroDamageMultiplier();
	const float AttackSpeedMult = GetHeroAttackSpeedMultiplier();
	const float ScaleMult = GetHeroScaleMultiplier();

	switch (StatType)
	{
	// Damage-primary-affected secondaries (base * item M * primary Damage mult)
	case ET66SecondaryStatType::AoeDamage:       return static_cast<float>(BaseAoeDmg) * M * DamageMult;
	case ET66SecondaryStatType::BounceDamage:     return static_cast<float>(BaseBounceDmg) * M * DamageMult;
	case ET66SecondaryStatType::PierceDamage:     return static_cast<float>(BasePierceDmg) * M * DamageMult;
	case ET66SecondaryStatType::DotDamage:        return static_cast<float>(BaseDotDmg) * M * DamageMult;
	case ET66SecondaryStatType::CritDamage:       return HeroBaseCritDamage * M * DamageMult;
	case ET66SecondaryStatType::CloseRangeDamage: return HeroBaseCloseRangeDmg * M * DamageMult;
	case ET66SecondaryStatType::LongRangeDamage:  return HeroBaseLongRangeDmg * M * DamageMult;
	// Attack-speed-affected (Speed secondaries + Crit Chance)
	case ET66SecondaryStatType::AoeSpeed:         return static_cast<float>(BaseAoeAtkSpd) * M * AttackSpeedMult;
	case ET66SecondaryStatType::BounceSpeed:      return static_cast<float>(BaseBounceAtkSpd) * M * AttackSpeedMult;
	case ET66SecondaryStatType::PierceSpeed:     return static_cast<float>(BasePierceAtkSpd) * M * AttackSpeedMult;
	case ET66SecondaryStatType::DotSpeed:         return static_cast<float>(BaseDotAtkSpd) * M * AttackSpeedMult;
	case ET66SecondaryStatType::CritChance:      return FMath::Clamp(HeroBaseCritChance * M * AttackSpeedMult, 0.f, 1.f);
	// Scale-affected (Scale secondaries + Range)
	case ET66SecondaryStatType::AoeScale:         return static_cast<float>(BaseAoeAtkScale) * M * ScaleMult;
	case ET66SecondaryStatType::BounceScale:      return static_cast<float>(BaseBounceAtkScale) * M * ScaleMult;
	case ET66SecondaryStatType::PierceScale:     return static_cast<float>(BasePierceAtkScale) * M * ScaleMult;
	case ET66SecondaryStatType::DotScale:         return static_cast<float>(BaseDotAtkScale) * M * ScaleMult;
	case ET66SecondaryStatType::AttackRange:      return HeroBaseAttackRange * M * ScaleMult;
	case ET66SecondaryStatType::Taunt:            return HeroBaseTaunt * M;
	case ET66SecondaryStatType::ReflectDamage:    return HeroBaseReflectDmg * M;
	case ET66SecondaryStatType::HpRegen:          return FMath::Max(1.f, HeroBaseHpRegen) * M;
	case ET66SecondaryStatType::Crush:            return FMath::Clamp(HeroBaseCrushChance * M, 0.f, 1.f);
	case ET66SecondaryStatType::Invisibility:     return FMath::Clamp(HeroBaseInvisChance * M, 0.f, 1.f);
	case ET66SecondaryStatType::CounterAttack:    return HeroBaseCounterAttack * M;
	case ET66SecondaryStatType::LifeSteal:        return FMath::Clamp(HeroBaseLifeSteal * M, 0.f, 1.f);
	case ET66SecondaryStatType::Assassinate:      return FMath::Clamp(HeroBaseAssassinateChance * M, 0.f, 1.f);
	case ET66SecondaryStatType::SpinWheel:        return 1.f * M;
	case ET66SecondaryStatType::Goblin:           return 1.f * M;
	case ET66SecondaryStatType::Leprechaun:       return 1.f * M;
	case ET66SecondaryStatType::TreasureChest:    return 1.f * M;
	case ET66SecondaryStatType::Fountain:         return 1.f * M;
	case ET66SecondaryStatType::Cheating:         return FMath::Clamp(HeroBaseCheatChance * M, 0.f, 1.f);
	case ET66SecondaryStatType::Stealing:         return FMath::Clamp(HeroBaseStealChance * M, 0.f, 1.f);
	case ET66SecondaryStatType::MovementSpeed:    return 1.f * M;
	default: return 1.f;
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
	return GetSecondaryStatValue(ET66SecondaryStatType::MovementSpeed);
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
	return GetSecondaryStatValue(ET66SecondaryStatType::CloseRangeDamage);
}

float UT66RunStateSubsystem::GetLongRangeDamageMultiplier() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::LongRangeDamage);
}

float UT66RunStateSubsystem::GetHeroMoveSpeedMultiplier() const
{
	// Mapping is driven by the foundational Speed stat (not shown in the HUD stat panel).
	// Keep it modest because Speed gains are +1 per level.
	const int32 S = GetSpeedStat();
	return 1.f + (static_cast<float>(S - 1) * 0.01f);
}

float UT66RunStateSubsystem::GetHeroDamageMultiplier() const
{
	const int32 D = GetDamageStat();
	return 1.f + (static_cast<float>(D - 1) * 0.015f);
}

float UT66RunStateSubsystem::GetHeroAttackSpeedMultiplier() const
{
	const int32 AS = GetAttackSpeedStat();
	return 1.f + (static_cast<float>(AS - 1) * 0.012f);
}

float UT66RunStateSubsystem::GetHeroScaleMultiplier() const
{
	const int32 Sz = GetScaleStat();
	return 1.f + (static_cast<float>(Sz - 1) * 0.008f);
}

float UT66RunStateSubsystem::GetArmorReduction01() const
{
	// Damage reduction derived from Armor stat, clamped.
	const int32 A = GetArmorStat();
	const float Base = static_cast<float>(A - 1) * 0.008f;
	return FMath::Clamp(Base + ItemArmorBonus01, 0.f, 0.80f);
}

float UT66RunStateSubsystem::GetEvasionChance01() const
{
	// Dodge chance derived from Evasion stat, clamped.
	const int32 E = GetEvasionStat();
	const float Base = static_cast<float>(E - 1) * 0.006f;
	return FMath::Clamp(Base + ItemEvasionBonus01, 0.f, 0.60f);
}

void UT66RunStateSubsystem::NotifyEnemyKilledByHero()
{
	if (PassiveType != ET66PassiveType::RallyingBlow) return;
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return;
	const double Now = World->GetTimeSeconds();
	RallyStacks = FMath::Min(3, RallyStacks + 1);
	RallyTimerEndWorldTime = Now + 3.0;
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
	TWeakObjectPtr<AActor> Key(Target);
	const FT66DotInstance* Inst = ActiveDOTs.Find(Key);
	return Inst && Inst->RemainingDuration > 0.f;
}

float UT66RunStateSubsystem::GetToxinStackingDamageMultiplier(AActor* Target) const
{
	if (PassiveType != ET66PassiveType::ToxinStacking || !Target) return 1.f;
	return HasActiveDOT(Target) ? 1.15f : 1.f;
}

void UT66RunStateSubsystem::AddHeroXP(int32 Amount)
{
	if (Amount <= 0) return;
	if (HeroLevel <= 0) HeroLevel = DefaultHeroLevel;
	if (XPToNextLevel <= 0) XPToNextLevel = DefaultXPToLevel;

	HeroXP = FMath::Clamp(HeroXP + Amount, 0, 1000000000);
	bool bLeveled = false;
	while (HeroXP >= XPToNextLevel && XPToNextLevel > 0)
	{
		HeroXP -= XPToNextLevel;
		HeroLevel = FMath::Clamp(HeroLevel + 1, 1, 9999);
		ApplyOneHeroLevelUp();
		bLeveled = true;
	}
	HeroProgressChanged.Broadcast();
	if (bLeveled)
	{
		LogAdded.Broadcast();
		UGameInstance* GI = GetGameInstance();
		UWorld* World = GI ? GI->GetWorld() : nullptr;
		APawn* HeroPawn = World && World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
		if (HeroPawn && GI)
		{
			if (UT66FloatingCombatTextSubsystem* FCT = GI->GetSubsystem<UT66FloatingCombatTextSubsystem>())
			{
				FCT->ShowStatusEvent(HeroPawn, UT66FloatingCombatTextSubsystem::EventType_LevelUp);
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
	if (VendorAngerGold == 0) return;
	VendorAngerGold = 0;
	VendorChanged.Broadcast();
}

void UT66RunStateSubsystem::SetInStageBoost(bool bInBoost)
{
	if (bInStageBoost == bInBoost) return;
	bInStageBoost = bInBoost;
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

bool UT66RunStateSubsystem::ApplyTrueDamage(int32 DamageHP)
{
	if (DamageHP <= 0) return false;
	if (bInLastStand) return false;

	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	const float Now = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
	if (Now - LastDamageTime < InvulnDurationSeconds)
	{
		return false;
	}

	LastDamageTime = Now;
	const float Reduced = static_cast<float>(FMath::Max(1, DamageHP));
	CurrentHP = FMath::Max(0.f, CurrentHP - Reduced);

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

	if (UGameInstance* GI3 = GetGameInstance())
	{
		if (UT66SkillRatingSubsystem* Skill = GI3->GetSubsystem<UT66SkillRatingSubsystem>())
		{
			Skill->NotifyDamageTaken();
		}
	}

	SurvivalCharge01 = FMath::Clamp(SurvivalCharge01 + (Reduced / DefaultMaxHP), 0.f, 1.f);
	SurvivalChanged.Broadcast();
	HeartsChanged.Broadcast();

	if (CurrentHP <= 0.f)
	{
		if (bDevImmortality)
		{
			return true;
		}
		OnPlayerDied.Broadcast();
	}
	return true;
}

void UT66RunStateSubsystem::ApplyStatusBurn(float DurationSeconds, float DamagePerSecond)
{
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 30.f);
	if (Dur <= 0.f) return;
	StatusBurnSecondsRemaining = FMath::Max(StatusBurnSecondsRemaining, Dur);
	StatusBurnDamagePerSecond = FMath::Max(0.f, DamagePerSecond);
	StatusEffectsChanged.Broadcast();
}

void UT66RunStateSubsystem::ApplyStatusChill(float DurationSeconds, float MoveSpeedMultiplier)
{
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 30.f);
	if (Dur <= 0.f) return;
	StatusChillSecondsRemaining = FMath::Max(StatusChillSecondsRemaining, Dur);
	StatusChillMoveSpeedMultiplier = FMath::Clamp(MoveSpeedMultiplier, 0.1f, 1.f);
	StatusEffectsChanged.Broadcast();
	HeroProgressChanged.Broadcast(); // movement updates
}

void UT66RunStateSubsystem::ApplyStatusCurse(float DurationSeconds)
{
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 30.f);
	if (Dur <= 0.f) return;
	StatusCurseSecondsRemaining = FMath::Max(StatusCurseSecondsRemaining, Dur);
	StatusEffectsChanged.Broadcast();
}

void UT66RunStateSubsystem::EnsureVendorStockForCurrentStage()
{
	const int32 Stage = FMath::Clamp(CurrentStage, 1, 66);
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
		// Fallback: keep deterministic placeholder behavior (3 slots).
		VendorStockSlots.Add(FT66InventorySlot(FName(TEXT("Item_AoeDamage")), ET66ItemRarity::Black, 2));
		VendorStockSlots.Add(FT66InventorySlot(FName(TEXT("Item_CritDamage")), ET66ItemRarity::Red, 5));
		VendorStockSlots.Add(FT66InventorySlot(FName(TEXT("Item_LifeSteal")), ET66ItemRarity::Yellow, 8));
		for (const FT66InventorySlot& S : VendorStockSlots) VendorStockItemIDs.Add(S.ItemTemplateID);
		VendorStockSold.Init(false, VendorStockSlots.Num());
		VendorChanged.Broadcast();
		return;
	}

	// Build pool of all template IDs from the Items DataTable.
	UDataTable* ItemsDT = GI->GetItemsDataTable();
	TArray<FName> TemplatePool;
	if (ItemsDT)
	{
		TemplatePool = ItemsDT->GetRowNames();
	}
	if (TemplatePool.Num() == 0)
	{
		TemplatePool = { FName(TEXT("Item_AoeDamage")), FName(TEXT("Item_CritDamage")), FName(TEXT("Item_LifeSteal")) };
	}

	// Seed: per-stage and per-reroll, plus run seed so the first shop display is randomized each run.
	int32 Seed = Stage * 777 + 13 + VendorStockRerollCounter * 10007;
	if (UT66RngSubsystem* RngSub = GI->GetSubsystem<UT66RngSubsystem>())
	{
		Seed ^= RngSub->GetRunSeed();
	}
	FRandomStream Rng(Seed);

	// Smart reroll: weight = 1/(1 + SeenCount*Decay), floor 0.05. Build weights and pick 3 unique.
	constexpr float DecayFactor = 2.0f;
	constexpr float WeightFloor = 0.05f;
	TArray<float> Weights;
	Weights.SetNumUninitialized(TemplatePool.Num());
	for (int32 p = 0; p < TemplatePool.Num(); ++p)
	{
		const int32 Seen = VendorSeenCounts.FindRef(TemplatePool[p]);
		Weights[p] = FMath::Max(WeightFloor, 1.0f / (1.0f + static_cast<float>(Seen) * DecayFactor));
	}

	const ET66ItemRarity SlotRarities[] = { ET66ItemRarity::Black, ET66ItemRarity::Black, ET66ItemRarity::Red };

	for (int32 i = 0; i < 3; ++i)
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
	const int32 Stage = FMath::Clamp(CurrentStage, 1, 66);
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
	EnsureVendorStockForCurrentStage();
	if (Index < 0 || Index >= VendorStockSlots.Num()) return false;
	if (IsVendorStockSlotSold(Index)) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	FItemData D;
	const FT66InventorySlot& StealSlot = VendorStockSlots[Index];
	if (!GI || !GI->GetItemData(StealSlot.ItemTemplateID, D)) return false;
	const int32 BuyPrice = D.GetBuyGoldForRarity(StealSlot.Rarity);
	if (BuyPrice <= 0) return false;

	// Determine success via central RNG. Timing window improves odds but does not guarantee success.
	const UT66RngTuningConfig* Tuning = nullptr;
	UT66RngSubsystem* RngSub = nullptr;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		RngSub = GameInstance->GetSubsystem<UT66RngSubsystem>();
		if (RngSub)
		{
			RngSub->UpdateLuckStat(GetLuckStat());
			Tuning = RngSub->GetTuning();
		}
	}

	float BaseChance = 0.f;
	if (bTimingHit)
	{
		BaseChance = Tuning ? Tuning->VendorStealSuccessChanceOnTimingHitBase : 0.65f;
	}
	BaseChance = FMath::Clamp(BaseChance, 0.f, 1.f);

	bool bSuccess = false;
	if (bTimingHit && BaseChance > 0.f)
	{
		const float Chance = RngSub ? RngSub->BiasChance01(BaseChance) : BaseChance;
		FRandomStream Local(FPlatformTime::Cycles());
		FRandomStream& Stream = RngSub ? RngSub->GetRunStream() : Local;
		bSuccess = (Stream.GetFraction() < Chance);
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
		VendorAngerGold = FMath::Clamp(VendorAngerGold + BuyPrice, 0, 9999999);
	}

	// Luck Rating tracking (quantity): vendor steal success means item granted with no anger increase.
	RecordLuckQuantityBool(FName(TEXT("VendorStealSuccess")), (LastVendorStealOutcome == ET66VendorStealOutcome::Success));

	VendorChanged.Broadcast();
	return bGranted;
}

void UT66RunStateSubsystem::GenerateBuybackDisplay()
{
	BuybackDisplaySlots.SetNum(3);
	const int32 PoolNum = BuybackPool.Num();
	const int32 MaxPage = PoolNum > 0 ? FMath::Max(0, (PoolNum + 2) / 3 - 1) : 0;
	BuybackDisplayPage = FMath::Clamp(BuybackDisplayPage, 0, MaxPage);
	const int32 Start = BuybackDisplayPage * 3;
	for (int32 i = 0; i < 3; ++i)
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
	const int32 MaxPage = PoolNum > 0 ? FMath::Max(0, (PoolNum + 2) / 3 - 1) : 0;
	if (MaxPage > 0)
	{
		BuybackDisplayPage = (BuybackDisplayPage + 1) % (MaxPage + 1);
	}
	GenerateBuybackDisplay();
}

bool UT66RunStateSubsystem::TryBuybackSlot(int32 DisplayIndex)
{
	if (DisplayIndex < 0 || DisplayIndex >= 3) return false;
	if (!HasInventorySpace()) return false;

	const int32 PoolIndex = BuybackDisplayPage * 3 + DisplayIndex;
	if (PoolIndex < 0 || PoolIndex >= BuybackPool.Num()) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI) return false;

	const FT66InventorySlot Slot = BuybackPool[PoolIndex];
	if (!Slot.IsValid()) return false;

	FItemData ItemData;
	int32 BuyPrice = 0;
	if (GI->GetItemData(Slot.ItemTemplateID, ItemData))
	{
		BuyPrice = ItemData.GetSellGoldForRarity(Slot.Rarity);
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

	// Status effects timers
	bool bStatusChanged = false;

	if (StatusBurnSecondsRemaining > 0.f)
	{
		StatusBurnSecondsRemaining = FMath::Max(0.f, StatusBurnSecondsRemaining - DeltaTime);
		// DoT tick
		if (StatusBurnDamagePerSecond > 0.f)
		{
			StatusBurnAccumDamage += StatusBurnDamagePerSecond * DeltaTime;
			while (StatusBurnAccumDamage >= 1.f)
			{
				StatusBurnAccumDamage -= 1.f;
				ApplyTrueDamage(5);
			}
		}
		if (StatusBurnSecondsRemaining <= 0.f)
		{
			StatusBurnDamagePerSecond = 0.f;
			StatusBurnAccumDamage = 0.f;
			bStatusChanged = true;
		}
	}

	if (StatusChillSecondsRemaining > 0.f)
	{
		StatusChillSecondsRemaining = FMath::Max(0.f, StatusChillSecondsRemaining - DeltaTime);
		if (StatusChillSecondsRemaining <= 0.f)
		{
			StatusChillMoveSpeedMultiplier = 1.f;
			HeroProgressChanged.Broadcast();
			bStatusChanged = true;
		}
	}

	if (StatusCurseSecondsRemaining > 0.f)
	{
		StatusCurseSecondsRemaining = FMath::Max(0.f, StatusCurseSecondsRemaining - DeltaTime);
		if (StatusCurseSecondsRemaining <= 0.f)
		{
			bStatusChanged = true;
		}
	}

	if (bStatusChanged)
	{
		StatusEffectsChanged.Broadcast();
	}
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

	// If we're in last-stand, we ignore damage (invincible).
	if (bInLastStand) return false;

	// Iron Will (New Chad): flat damage reduction before armor.
	if (PassiveType == ET66PassiveType::IronWill)
	{
		const int32 FlatReduction = GetArmorStat() * 2;
		DamageHP = FMath::Max(1, DamageHP - FlatReduction);
	}

	// Evasion: dodge the entire hit. On dodge: Assassinate (OHKO) and CounterAttack (deal fraction of would-be damage to attacker).
	const float Evade = GetEvasionChance01();
	if (Evade > 0.f && FMath::FRand() < Evade)
	{
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
			if (AssassinateChance > 0.f && FMath::FRand() < AssassinateChance)
			{
				if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Attacker)) { E->TakeDamageFromHero(99999, FName(TEXT("Assassinate")), NAME_None); }
				else if (AT66BossBase* B = Cast<AT66BossBase>(Attacker)) { B->TakeDamageFromHeroHit(99999, FName(TEXT("Assassinate")), NAME_None); }
			}
			const float CounterChance = FMath::Clamp(GetCounterAttackFraction(), 0.f, 1.f);
			if (CounterChance > 0.f && DamageHP > 0 && FMath::FRand() < CounterChance)
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
		return false;
	}

	// Reflect: % chance to reflect; when it procs, 50% of reduced damage back to the attacker. Crush: chance to OHKO when reflect fires.
	if (Attacker && Reduced > 0.f)
	{
		const float ReflectChance = FMath::Clamp(GetReflectDamageFraction(), 0.f, 1.f);
		if (ReflectChance > 0.f && FMath::FRand() < ReflectChance)
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
			if (CrushChance > 0.f && FMath::FRand() < CrushChance)
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

	// Skill Rating tracking: any damage that actually applies counts as a hit event.
	if (UGameInstance* GI3 = GetGameInstance())
	{
		if (UT66SkillRatingSubsystem* Skill = GI3->GetSubsystem<UT66SkillRatingSubsystem>())
		{
			Skill->NotifyDamageTaken();
		}
	}

	// Survival charge: 100 HP damage = full (5 hearts * 20 HP). So 1 HP = 0.01 charge.
	SurvivalCharge01 = FMath::Clamp(SurvivalCharge01 + (Reduced / DefaultMaxHP), 0.f, 1.f);
	SurvivalChanged.Broadcast();
	HeartsChanged.Broadcast();

	if (CurrentHP <= 0.f)
	{
		// Dev Immortality: never end the run.
		if (bDevImmortality)
		{
			return true;
		}
		OnPlayerDied.Broadcast();
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

void UT66RunStateSubsystem::BorrowGold(int32 Amount)
{
	if (Amount <= 0) return;
	CurrentGold = FMath::Max(0, CurrentGold + Amount);
	CurrentDebt = FMath::Max(0, CurrentDebt + Amount);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Borrow"), Amount));
	GoldChanged.Broadcast();
	DebtChanged.Broadcast();
	LogAdded.Broadcast();
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
	if (ItemID.IsNone()) return;
	if (InventorySlots.Num() >= MaxInventorySlots)
	{
		AddLogEntry(TEXT("Inventory full."));
		LogAdded.Broadcast();
		return;
	}

	// Auto-generate rarity and rolled value (default: Black rarity for legacy compat).
	ET66ItemRarity Rarity = ET66ItemRarity::Black;
	int32 RolledMin = 1, RolledMax = 3;
	FItemData::GetLine1RollRange(Rarity, RolledMin, RolledMax);
	FRandomStream Local(FPlatformTime::Cycles());
	const int32 RolledValue = Local.RandRange(RolledMin, RolledMax);

	FT66InventorySlot Slot(ItemID, Rarity, RolledValue);
	AddItemSlot(Slot);
}

void UT66RunStateSubsystem::AddItemSlot(const FT66InventorySlot& Slot)
{
	if (!Slot.IsValid()) return;
	if (InventorySlots.Num() >= MaxInventorySlots)
	{
		AddLogEntry(TEXT("Inventory full."));
		LogAdded.Broadcast();
		return;
	}
	InventorySlots.Add(Slot);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("ItemID=%s,Source=LootBag"), *Slot.ItemTemplateID.ToString()));
	// Lab unlock: mark item as unlocked for The Lab (any run type including Lab).
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achieve->AddLabUnlockedItem(Slot.ItemTemplateID);
		}
	}
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
}

void UT66RunStateSubsystem::ClearInventory()
{
	InventorySlots.Empty();
	RecomputeItemDerivedStats();
	InventoryChanged.Broadcast();
}

void UT66RunStateSubsystem::HealToFull()
{
	if (bInLastStand) return;
	CurrentHP = MaxHP;
	HeartsChanged.Broadcast();
}

void UT66RunStateSubsystem::HealHP(float Amount)
{
	if (bInLastStand) return;
	if (Amount <= 0.f) return;

	const float NewHP = FMath::Clamp(CurrentHP + Amount, 0.f, MaxHP);
	if (FMath::IsNearlyEqual(NewHP, CurrentHP)) return;
	CurrentHP = NewHP;
	HeartsChanged.Broadcast();
}

void UT66RunStateSubsystem::HealHearts(int32 Hearts)
{
	if (Hearts <= 0) return;
	HealHP(static_cast<float>(Hearts) * HPPerRedHeart);
}

void UT66RunStateSubsystem::ApplyHpRegen(float DeltaTime)
{
	if (bInLastStand) return;
	const float Rate = GetHpRegenPerSecond();
	if (Rate <= 0.f || DeltaTime <= 0.f) return;

	const float Healed = Rate * DeltaTime;
	HealHP(Healed);
}

void UT66RunStateSubsystem::KillPlayer()
{
	// Dev Immortality: never end the run.
	if (bDevImmortality)
	{
		CurrentHP = 0.f;
		LastDamageTime = -9999.f;
		HeartsChanged.Broadcast();
		return;
	}

	// If charged, trigger last-stand instead of dying.
	if (!bInLastStand && SurvivalCharge01 >= 1.f)
	{
		EnterLastStand();
		return;
	}

	CurrentHP = 0.f;
	LastDamageTime = -9999.f;
	HeartsChanged.Broadcast();
	OnPlayerDied.Broadcast();
}

bool UT66RunStateSubsystem::SellFirstItem()
{
	if (InventorySlots.Num() == 0) return false;
	return SellInventoryItemAt(0);
}

bool UT66RunStateSubsystem::SellInventoryItemAt(int32 InventoryIndex)
{
	if (InventoryIndex < 0 || InventoryIndex >= InventorySlots.Num()) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI) return false;

	const FT66InventorySlot Slot = InventorySlots[InventoryIndex];
	FItemData ItemData;
	int32 SellGold = 0;
	if (GI->GetItemData(Slot.ItemTemplateID, ItemData))
	{
		SellGold = ItemData.GetSellGoldForRarity(Slot.Rarity);
	}

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

void UT66RunStateSubsystem::RecomputeItemDerivedStats()
{
	// Reset all accumulators.
	ItemStatBonuses = FT66HeroStatBonuses{};
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

	auto AddPrimaryBonus = [&](ET66HeroStatType Type, int32 Delta)
	{
		const int32 V = FMath::Clamp(Delta, 0, 9999);
		if (V <= 0) return;
		switch (Type)
		{
			case ET66HeroStatType::Damage:      ItemStatBonuses.Damage += V; break;
			case ET66HeroStatType::AttackSpeed:  ItemStatBonuses.AttackSpeed += V; break;
			case ET66HeroStatType::AttackScale:  ItemStatBonuses.AttackScale += V; break;
			case ET66HeroStatType::Armor:        ItemStatBonuses.Armor += V; break;
			case ET66HeroStatType::Evasion:      ItemStatBonuses.Evasion += V; break;
			case ET66HeroStatType::Luck:         ItemStatBonuses.Luck += V; break;
			case ET66HeroStatType::Speed:        HeroStats.Speed = FMath::Clamp(HeroStats.Speed + V, 1, 9999); break;
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

		// Line 1: Additive flat bonus to primary stat.
		AddPrimaryBonus(D.PrimaryStatType, Slot.Line1RolledValue);

		// Line 2: Multiplicative secondary stat bonus.
		// Multiple items with the same secondary type stack multiplicatively.
		if (D.SecondaryStatType != ET66SecondaryStatType::None)
		{
			const float Mult = Slot.GetLine2Multiplier();
			float& Accum = SecondaryMultipliers.FindOrAdd(D.SecondaryStatType);
			if (Accum == 0.f)
			{
				Accum = Mult; // First item of this type
			}
			else
			{
				Accum *= Mult; // Stack multiplicatively
			}
		}
	}
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

void UT66RunStateSubsystem::ResetTutorialInputFlags()
{
	const bool bWasMove = bTutorialMoveInputSeen;
	const bool bWasJump = bTutorialJumpInputSeen;
	bTutorialMoveInputSeen = false;
	bTutorialJumpInputSeen = false;
	if (bWasMove || bWasJump)
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
	MaxHP = DefaultMaxHP;
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
	BuybackPool.Empty();
	BuybackDisplaySlots.Empty();
	BuybackDisplayPage = 0;
	RecomputeItemDerivedStats();
	EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
	EquippedIdolLevels.Init(0, MaxEquippedIdolSlots);
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
	ResetLuckRatingTracking();
	CurrentStage = 1;
	bStageTimerActive = false;
	StageTimerSecondsRemaining = StageTimerDurationSeconds;
	LastBroadcastStageTimerSecond = static_cast<int32>(StageTimerDurationSeconds);
	SpeedRunElapsedSeconds = 0.f;
	SpeedRunStartWorldSeconds = 0.f;
	bSpeedRunStarted = false;
	LastBroadcastSpeedRunSecond = -1;
	bThisRunSetNewPersonalBestSpeedRunTime = false;
	CurrentScore = 0;
	LastDamageTime = -9999.f;
	PowerCrystalsEarnedThisRun = 0;

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
	// Difficulty start boosts: each difficulty step grants +10 levels.
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		const int32 DiffIndex = static_cast<int32>(T66GI->SelectedDifficulty);
		const int32 Steps = FMath::Clamp(DiffIndex, 0, 999);
		HeroLevel = FMath::Clamp(DefaultHeroLevel + (Steps * 10), 1, 9999);
		bInStageBoost = T66GI->bStageBoostPending;
	}
	InitializeHeroStatsForNewRun();

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
	StatusEffectsChanged.Broadcast();
}

void UT66RunStateSubsystem::SetCurrentStage(int32 Stage)
{
	const int32 NewStage = FMath::Clamp(Stage, 1, 66);
	if (CurrentStage == NewStage) return;

	// If Speed Run Mode is enabled, record the stage completion time for the stage we're leaving.
	// This is used for the main menu Speed Run leaderboard (stage 1..10).
	{
		UGameInstance* GI = GetGameInstance();
		UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
		const bool bSpeedRunMode = PS ? PS->GetSpeedRunMode() : false;
		if (bSpeedRunMode)
		{
			const int32 CompletedStage = CurrentStage;
			if (CompletedStage >= 1 && CompletedStage <= 10)
			{
				UWorld* World = GetWorld();
				const float Now = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
				const float Elapsed = (bSpeedRunStarted && SpeedRunStartWorldSeconds > 0.f) ? FMath::Max(0.f, Now - SpeedRunStartWorldSeconds) : SpeedRunElapsedSeconds;
				if (UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr)
				{
					LB->SubmitStageSpeedRunTime(CompletedStage, Elapsed);
					if (LB->WasLastSpeedRunNewPersonalBest() && LB->GetLastSpeedRunSubmittedStage() == CompletedStage)
					{
						bThisRunSetNewPersonalBestSpeedRunTime = true;
					}
				}
			}
		}
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
	StageChanged.Broadcast();
}

void UT66RunStateSubsystem::SetStageTimerActive(bool bActive)
{
	if (bStageTimerActive == bActive) return;
	bStageTimerActive = bActive;

	// Start (or stop) the speedrun timer alongside the stage timer.
	// User-requested behavior: speedrun time starts after leaving the start area (start gate), not at spawn.
	if (bStageTimerActive)
	{
		if (UWorld* World = GetWorld())
		{
			SpeedRunStartWorldSeconds = static_cast<float>(World->GetTimeSeconds());
			bSpeedRunStarted = true;
			SpeedRunElapsedSeconds = 0.f;
			LastBroadcastSpeedRunSecond = -1;
			SpeedRunTimerChanged.Broadcast();
		}
	}
	else
	{
		bSpeedRunStarted = false;
	}

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
	// Speedrun time begins when the stage timer becomes active (start gate) and resets each stage.
	if (!bStageTimerActive || !bSpeedRunStarted) return;
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

	// New stage start area: reset speedrun timer to 0 until the next start gate triggers it again.
	SpeedRunElapsedSeconds = 0.f;
	SpeedRunStartWorldSeconds = 0.f;
	bSpeedRunStarted = false;
	LastBroadcastSpeedRunSecond = -1;
	SpeedRunTimerChanged.Broadcast();
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
	BossChanged.Broadcast();
}

void UT66RunStateSubsystem::RescaleBossMaxHPPreservePercent(int32 NewMaxHP)
{
	if (!bBossActive) return;
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
	BossCurrentHP = 0;
	BossChanged.Broadcast();
}

bool UT66RunStateSubsystem::ApplyBossDamage(int32 Damage)
{
	if (!bBossActive || Damage <= 0 || BossCurrentHP <= 0) return false;
	BossCurrentHP = FMath::Max(0, BossCurrentHP - Damage);
	BossChanged.Broadcast();
	return BossCurrentHP <= 0;
}

void UT66RunStateSubsystem::ResetBossState()
{
	bBossActive = false;
	ActiveBossID = NAME_None;
	BossMaxHP = 100;
	BossCurrentHP = 0;
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
