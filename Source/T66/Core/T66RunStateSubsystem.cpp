// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RunStateSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66SkillRatingSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Kismet/GameplayStatics.h"

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
	// Canonical idol IDs (sprites + UI depend on these).
	// Back-compat: GetIdolColor/tooltip code accepts some legacy IDs, but this list is the current altar roster.
	static const TArray<FName> Idols = {
		FName(TEXT("Idol_Darkness")),
		FName(TEXT("Idol_Fire")),
		FName(TEXT("Idol_Frost")),
		FName(TEXT("Idol_Glue")),
		FName(TEXT("Idol_Light")),
		FName(TEXT("Idol_Lightning")),
		FName(TEXT("Idol_Metal")),
		FName(TEXT("Idol_Poison")),
		FName(TEXT("Idol_Spectral")),
		FName(TEXT("Idol_Water")),
		FName(TEXT("Idol_Wind")),
		FName(TEXT("Idol_Wood")),
	};
	return Idols;
}

FLinearColor UT66RunStateSubsystem::GetIdolColor(FName IdolID)
{
	// Back-compat aliases.
	if (IdolID == FName(TEXT("Idol_Shock"))) IdolID = FName(TEXT("Idol_Lightning"));
	if (IdolID == FName(TEXT("Idol_Silence"))) IdolID = FName(TEXT("Idol_Darkness"));
	if (IdolID == FName(TEXT("Idol_Mark"))) IdolID = FName(TEXT("Idol_Light"));
	if (IdolID == FName(TEXT("Idol_Pierce"))) IdolID = FName(TEXT("Idol_Metal"));
	if (IdolID == FName(TEXT("Idol_Split"))) IdolID = FName(TEXT("Idol_Wind"));
	if (IdolID == FName(TEXT("Idol_Knockback"))) IdolID = FName(TEXT("Idol_Wood"));
	if (IdolID == FName(TEXT("Idol_Ricochet"))) IdolID = FName(TEXT("Idol_Water"));
	if (IdolID == FName(TEXT("Idol_Hex"))) IdolID = FName(TEXT("Idol_Spectral"));
	if (IdolID == FName(TEXT("Idol_Lifesteal"))) IdolID = FName(TEXT("Idol_Poison"));

	if (IdolID == FName(TEXT("Idol_Darkness"))) return FLinearColor(0.10f, 0.10f, 0.12f, 1.f);
	if (IdolID == FName(TEXT("Idol_Light"))) return FLinearColor(0.92f, 0.92f, 0.98f, 1.f);
	if (IdolID == FName(TEXT("Idol_Lightning"))) return FLinearColor(0.70f, 0.25f, 0.95f, 1.f);
	if (IdolID == FName(TEXT("Idol_Metal"))) return FLinearColor(0.55f, 0.55f, 0.75f, 1.f);
	if (IdolID == FName(TEXT("Idol_Poison"))) return FLinearColor(0.20f, 0.85f, 0.35f, 1.f);
	if (IdolID == FName(TEXT("Idol_Spectral"))) return FLinearColor(0.60f, 0.25f, 0.90f, 1.f);
	if (IdolID == FName(TEXT("Idol_Water"))) return FLinearColor(0.20f, 0.95f, 0.95f, 1.f);
	if (IdolID == FName(TEXT("Idol_Wind"))) return FLinearColor(0.95f, 0.80f, 0.20f, 1.f);
	if (IdolID == FName(TEXT("Idol_Wood"))) return FLinearColor(0.35f, 0.65f, 0.25f, 1.f);

	if (IdolID == FName(TEXT("Idol_Frost"))) return FLinearColor(0.35f, 0.75f, 1.0f, 1.f);
	if (IdolID == FName(TEXT("Idol_Glue"))) return FLinearColor(0.20f, 0.85f, 0.35f, 1.f);
	if (IdolID == FName(TEXT("Idol_Fire"))) return FLinearColor(0.95f, 0.25f, 0.10f, 1.f);
	return FLinearColor(0.25f, 0.25f, 0.28f, 1.f);
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

void UT66RunStateSubsystem::AddMaxHearts(int32 DeltaHearts)
{
	if (DeltaHearts <= 0) return;
	MaxHearts = FMath::Clamp(MaxHearts + DeltaHearts, 1, 9999);
	CurrentHearts = FMath::Clamp(CurrentHearts + DeltaHearts, 0, MaxHearts);
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
	HeroPerLevelGains.AttackSize.Min = 1;  HeroPerLevelGains.AttackSize.Max = 2;
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
	}

	// Enforce sane minimums for gameplay safety.
	HeroStats.Damage = FMath::Clamp(HeroStats.Damage, 1, 9999);
	HeroStats.AttackSpeed = FMath::Clamp(HeroStats.AttackSpeed, 1, 9999);
	HeroStats.AttackSize = FMath::Clamp(HeroStats.AttackSize, 1, 9999);
	HeroStats.Armor = FMath::Clamp(HeroStats.Armor, 1, 9999);
	HeroStats.Evasion = FMath::Clamp(HeroStats.Evasion, 1, 9999);
	HeroStats.Luck = FMath::Clamp(HeroStats.Luck, 1, 9999);
	HeroStats.Speed = FMath::Clamp(HeroStats.Speed, 1, 9999);
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
	HeroStats.AttackSize = FMath::Clamp(HeroStats.AttackSize + RollGainBiased(HeroPerLevelGains.AttackSize, FName(TEXT("LevelUp_AttackSizeGain"))), 1, 9999);
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
	const int32 Sz = GetScaleStat(); // Attack Size
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
	}
}

void UT66RunStateSubsystem::ResetVendorForStage()
{
	VendorAngerGold = 0;
	VendorStockStage = 0;
	VendorStockItemIDs.Reset();
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

bool UT66RunStateSubsystem::ApplyTrueDamage(int32 Hearts)
{
	if (Hearts <= 0) return false;
	if (bInLastStand) return false;

	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	const float Now = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
	if (Now - LastDamageTime < InvulnDurationSeconds)
	{
		return false;
	}

	LastDamageTime = Now;
	const int32 Reduced = FMath::Max(1, Hearts);
	CurrentHearts = FMath::Max(0, CurrentHearts - Reduced);

	// Skill Rating tracking: any damage that actually applies counts as a hit event.
	if (UGameInstance* GI3 = GetGameInstance())
	{
		if (UT66SkillRatingSubsystem* Skill = GI3->GetSubsystem<UT66SkillRatingSubsystem>())
		{
			Skill->NotifyDamageTaken();
		}
	}

	// Survival charge fills on real damage taken.
	SurvivalCharge01 = FMath::Clamp(SurvivalCharge01 + (static_cast<float>(Reduced) * SurvivalChargePerHeart), 0.f, 1.f);
	SurvivalChanged.Broadcast();
	HeartsChanged.Broadcast();

	if (CurrentHearts <= 0)
	{
		// Dev Immortality: never end the run.
		if (bDevImmortality)
		{
			return true;
		}

		// If charged, trigger last-stand instead of dying.
		if (SurvivalCharge01 >= 1.f)
		{
			EnterLastStand();
		}
		else
		{
			OnPlayerDied.Broadcast();
		}
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

	// Reset reroll counter when stage changes.
	if (VendorStockRerollStage != Stage)
	{
		VendorStockRerollStage = Stage;
		VendorStockRerollCounter = 0;
	}

	VendorStockStage = Stage;
	VendorStockItemIDs.Reset();
	VendorStockSold.Reset();

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI)
	{
		// Fallback: keep deterministic placeholder behavior.
		VendorStockItemIDs = { FName(TEXT("Item_Black_01")), FName(TEXT("Item_Red_01")), FName(TEXT("Item_Yellow_01")), FName(TEXT("Item_White_01")) };
		VendorStockSold.Init(false, VendorStockItemIDs.Num());
		VendorChanged.Broadcast();
		return;
	}

	UDataTable* ItemsDT = GI->GetItemsDataTable();
	TArray<FName> PoolBlack;
	TArray<FName> PoolRed;
	TArray<FName> PoolYellow;

		if (ItemsDT)
	{
		const TArray<FName> RowNames = ItemsDT->GetRowNames();
		for (const FName& ItemID : RowNames)
		{
			if (ItemID.IsNone()) continue;
			FItemData D;
			if (!GI->GetItemData(ItemID, D)) continue;
			if (D.BuyValueGold <= 0) continue;

				// Canonical pool: items can appear in shop and in loot bags.
			switch (D.ItemRarity)
			{
				case ET66ItemRarity::Black:  PoolBlack.Add(ItemID); break;
				case ET66ItemRarity::Red:    PoolRed.Add(ItemID); break;
				case ET66ItemRarity::Yellow: PoolYellow.Add(ItemID); break;
					case ET66ItemRarity::White:  PoolYellow.Add(ItemID); break; // v0: allow whites in the shop pool by treating them as "rare slot" candidates
				default: break;
			}
		}
	}

	// Fallback if DT missing or pools empty.
	if (PoolBlack.Num() == 0) PoolBlack = { FName(TEXT("Item_Black_01")) };
	if (PoolRed.Num() == 0) PoolRed = { FName(TEXT("Item_Red_01")) };
	if (PoolYellow.Num() == 0) PoolYellow = { FName(TEXT("Item_Yellow_01")) };

	// Seed is stable per stage, but changes per reroll so the stock can refresh.
	FRandomStream Rng(Stage * 777 + 13 + VendorStockRerollCounter * 10007);
	auto PickUnique = [&](const TArray<FName>& Pool) -> FName
	{
		if (Pool.Num() == 0) return NAME_None;
		for (int32 Try = 0; Try < 40; ++Try)
		{
			const FName C = Pool[Rng.RandRange(0, Pool.Num() - 1)];
			if (!C.IsNone() && !VendorStockItemIDs.Contains(C))
			{
				return C;
			}
		}
		return Pool[0];
	};

	// Stock: 4 items (2 black, 1 red, 1 yellow)
	VendorStockItemIDs.Add(PickUnique(PoolBlack));
	VendorStockItemIDs.Add(PickUnique(PoolBlack));
	VendorStockItemIDs.Add(PickUnique(PoolRed));
	VendorStockItemIDs.Add(PickUnique(PoolYellow));

	for (int32 i = VendorStockItemIDs.Num() - 1; i >= 0; --i)
	{
		if (VendorStockItemIDs[i].IsNone())
		{
			VendorStockItemIDs.RemoveAt(i);
		}
	}
	VendorStockSold.Init(false, VendorStockItemIDs.Num());
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
	if (Index < 0 || Index >= VendorStockItemIDs.Num()) return false;
	if (IsVendorStockSlotSold(Index)) return false;
	if (!HasInventorySpace()) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	FItemData D;
	if (!GI || !GI->GetItemData(VendorStockItemIDs[Index], D)) return false;
	if (D.BuyValueGold <= 0) return false;
	if (!TrySpendGold(D.BuyValueGold)) return false;

	AddItem(VendorStockItemIDs[Index]);
	VendorStockSold[Index] = true;
	bVendorBoughtSomethingThisStage = true;
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("VendorPurchase=%s"), *VendorStockItemIDs[Index].ToString()));
	VendorChanged.Broadcast();
	return true;
}

bool UT66RunStateSubsystem::ResolveVendorStealAttempt(int32 Index, bool bTimingHit, bool bRngSuccess)
{
	EnsureVendorStockForCurrentStage();
	if (Index < 0 || Index >= VendorStockItemIDs.Num()) return false;
	if (IsVendorStockSlotSold(Index)) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	FItemData D;
	if (!GI || !GI->GetItemData(VendorStockItemIDs[Index], D)) return false;
	if (D.BuyValueGold <= 0) return false;

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
		AddItem(VendorStockItemIDs[Index]);
		VendorStockSold[Index] = true;
		AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("VendorSteal=%s"), *VendorStockItemIDs[Index].ToString()));
		bGranted = true;
		// Success: no anger increase.
	}
	else
	{
		// Failure: anger increases and the item is not granted.
		VendorAngerGold = FMath::Clamp(VendorAngerGold + D.BuyValueGold, 0, 9999999);
	}

	// Luck Rating tracking (quantity): vendor steal success means item granted with no anger increase.
	RecordLuckQuantityBool(FName(TEXT("VendorStealSuccess")), (LastVendorStealOutcome == ET66VendorStealOutcome::Success));

	VendorChanged.Broadcast();
	return bGranted;
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
				ApplyTrueDamage(1);
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

	// Hearts stay at 0, but the run continues.
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
		CurrentHearts = 0;
		HeartsChanged.Broadcast();
		return;
	}

	// Die as normal now.
	CurrentHearts = 0;
	HeartsChanged.Broadcast();
	OnPlayerDied.Broadcast();
}

bool UT66RunStateSubsystem::ApplyDamage(int32 Hearts)
{
	if (Hearts <= 0) return false;

	// If we're in last-stand, we ignore damage (invincible).
	if (bInLastStand) return false;

	// Evasion: dodge the entire hit.
	const float Evade = GetEvasionChance01();
	if (Evade > 0.f && FMath::FRand() < Evade)
	{
		return false;
	}

	// Armor: reduce the hit size (still at least 1 if hit > 0).
	const float Armor = GetArmorReduction01();
	const int32 Reduced = FMath::Max(1, FMath::CeilToInt(static_cast<float>(Hearts) * (1.f - Armor)));

	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	const float Now = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
	if (Now - LastDamageTime < InvulnDurationSeconds)
	{
		return false;
	}

	LastDamageTime = Now;
	CurrentHearts = FMath::Max(0, CurrentHearts - Reduced);

	// Skill Rating tracking: any damage that actually applies counts as a hit event.
	if (UGameInstance* GI3 = GetGameInstance())
	{
		if (UT66SkillRatingSubsystem* Skill = GI3->GetSubsystem<UT66SkillRatingSubsystem>())
		{
			Skill->NotifyDamageTaken();
		}
	}

	// Survival charge fills on real damage taken.
	SurvivalCharge01 = FMath::Clamp(SurvivalCharge01 + (static_cast<float>(Reduced) * SurvivalChargePerHeart), 0.f, 1.f);
	SurvivalChanged.Broadcast();
	HeartsChanged.Broadcast();

	if (CurrentHearts <= 0)
	{
		// Dev Immortality: never end the run.
		if (bDevImmortality)
		{
			return true;
		}

		// If charged, trigger last-stand instead of dying.
		if (SurvivalCharge01 >= 1.f)
		{
			EnterLastStand();
		}
		else
		{
			OnPlayerDied.Broadcast();
		}
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
	LogAdded.Broadcast();
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

void UT66RunStateSubsystem::AddItem(FName ItemID)
{
	if (ItemID.IsNone()) return;
	if (Inventory.Num() >= MaxInventorySlots)
	{
		AddLogEntry(TEXT("Inventory full."));
		LogAdded.Broadcast();
		return;
	}
	Inventory.Add(ItemID);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("ItemID=%s,Source=LootBag"), *ItemID.ToString()));
	// Lab unlock: mark item as unlocked for The Lab (any run type including Lab).
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achieve->AddLabUnlockedItem(ItemID);
		}
	}
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
}

void UT66RunStateSubsystem::ClearInventory()
{
	Inventory.Empty();
	RecomputeItemDerivedStats();
	InventoryChanged.Broadcast();
}

void UT66RunStateSubsystem::HealToFull()
{
	if (bInLastStand) return;
	CurrentHearts = MaxHearts;
	HeartsChanged.Broadcast();
}

void UT66RunStateSubsystem::HealHearts(int32 Hearts)
{
	if (bInLastStand) return;
	if (Hearts <= 0) return;

	const int32 NewHearts = FMath::Clamp(CurrentHearts + Hearts, 0, MaxHearts);
	if (NewHearts == CurrentHearts) return;
	CurrentHearts = NewHearts;
	HeartsChanged.Broadcast();
}

void UT66RunStateSubsystem::KillPlayer()
{
	// Dev Immortality: never end the run.
	if (bDevImmortality)
	{
		CurrentHearts = 0;
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

	CurrentHearts = 0;
	LastDamageTime = -9999.f;
	HeartsChanged.Broadcast();
	OnPlayerDied.Broadcast();
}

bool UT66RunStateSubsystem::SellFirstItem()
{
	if (Inventory.Num() == 0) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI) return false;

	FName ItemID = Inventory[0];
	FItemData ItemData;
	if (!GI->GetItemData(ItemID, ItemData))
	{
		Inventory.RemoveAt(0);
		RecomputeItemDerivedStats();
		InventoryChanged.Broadcast();
		AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=unknown,Source=Vendor,ItemID=%s"), *ItemID.ToString()));
		LogAdded.Broadcast();
		return true;
	}

	CurrentGold += ItemData.SellValueGold;
	Inventory.RemoveAt(0);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Vendor,ItemID=%s"), ItemData.SellValueGold, *ItemID.ToString()));
	GoldChanged.Broadcast();
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
	return true;
}

bool UT66RunStateSubsystem::SellInventoryItemAt(int32 InventoryIndex)
{
	if (InventoryIndex < 0 || InventoryIndex >= Inventory.Num()) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI) return false;

	const FName ItemID = Inventory[InventoryIndex];
	FItemData ItemData;
	if (!GI->GetItemData(ItemID, ItemData))
	{
		Inventory.RemoveAt(InventoryIndex);
		RecomputeItemDerivedStats();
		InventoryChanged.Broadcast();
		AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=unknown,Source=Vendor,ItemID=%s"), *ItemID.ToString()));
		LogAdded.Broadcast();
		return true;
	}

	CurrentGold += ItemData.SellValueGold;
	Inventory.RemoveAt(InventoryIndex);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Vendor,ItemID=%s"), ItemData.SellValueGold, *ItemID.ToString()));
	GoldChanged.Broadcast();
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
	return true;
}

void UT66RunStateSubsystem::RecomputeItemDerivedStats()
{
	// v1 items: a single "main stat line" that adds flat points to one foundational stat (excluding Speed).
	// (Secondary/tertiary lines will be added later.)

	ItemStatBonuses = FT66HeroStatBonuses{};

	// Legacy v0 item tuning is now ignored for gameplay. Keep values stable/safe.
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

	auto AddBonus = [&](ET66HeroStatType Type, int32 Delta)
	{
		const int32 V = FMath::Clamp(Delta, 0, 9999);
		if (V <= 0) return;
		switch (Type)
		{
			case ET66HeroStatType::Damage:      ItemStatBonuses.Damage += V; break;
			case ET66HeroStatType::AttackSpeed: ItemStatBonuses.AttackSpeed += V; break;
			case ET66HeroStatType::AttackSize:  ItemStatBonuses.AttackSize += V; break;
			case ET66HeroStatType::Armor:       ItemStatBonuses.Armor += V; break;
			case ET66HeroStatType::Evasion:     ItemStatBonuses.Evasion += V; break;
			case ET66HeroStatType::Luck:        ItemStatBonuses.Luck += V; break;
			default: break;
		}
	};

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	for (const FName& ItemID : Inventory)
	{
		if (ItemID.IsNone()) continue;

		FItemData D;
		const bool bHasRow = (GI && GI->GetItemData(ItemID, D));

		ET66HeroStatType StatType = bHasRow ? D.MainStatType : ET66HeroStatType::Damage;
		int32 StatValue = bHasRow ? D.MainStatValue : 0;

		// If the DT row hasn't been updated yet, derive a reasonable single-stat bonus from the old effect fields.
		if (bHasRow && StatValue == 0)
		{
			switch (D.EffectType)
			{
				case ET66ItemEffectType::BonusDamagePct:
					StatType = ET66HeroStatType::Damage;
					StatValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f);
					break;
				case ET66ItemEffectType::BonusAttackSpeedPct:
					StatType = ET66HeroStatType::AttackSpeed;
					StatValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f);
					break;
				case ET66ItemEffectType::BonusArmorPctPoints:
					StatType = ET66HeroStatType::Armor;
					StatValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 4.f);
					break;
				case ET66ItemEffectType::BonusEvasionPctPoints:
					StatType = ET66HeroStatType::Evasion;
					StatValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 4.f);
					break;
				case ET66ItemEffectType::BonusLuckFlat:
					StatType = ET66HeroStatType::Luck;
					StatValue = FMath::RoundToInt(FMath::Max(0.f, D.EffectMagnitude));
					break;
				// Speed is not an item stat; map mobility effects to Evasion as a temporary stand-in.
				case ET66ItemEffectType::BonusMoveSpeedPct:
				case ET66ItemEffectType::DashCooldownReductionPct:
					StatType = ET66HeroStatType::Evasion;
					StatValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f);
					break;
				default:
					break;
			}
		}

		// Safe fallback if row is missing entirely.
		if (!bHasRow && StatValue == 0)
		{
			if (ItemID == TEXT("Item_Black_01")) { StatType = ET66HeroStatType::Damage; StatValue = 2; }
			else if (ItemID == TEXT("Item_Red_01")) { StatType = ET66HeroStatType::AttackSpeed; StatValue = 2; }
			else if (ItemID == TEXT("Item_Yellow_01")) { StatType = ET66HeroStatType::Evasion; StatValue = 2; }
			else if (ItemID == TEXT("Item_White_01")) { StatType = ET66HeroStatType::Luck; StatValue = 2; }
		}

		AddBonus(StatType, StatValue);
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
	MaxHearts = DefaultMaxHearts;
	CurrentHearts = MaxHearts;
	CurrentGold = DefaultStartGold;
	CurrentDebt = 0;
	bLoanSharkPending = false;
	DifficultyTier = 0;
	DifficultySkulls = 0;
	TotemsActivatedCount = 0;
	GamblerAnger01 = 0.f;
	ResetVendorForStage();
	OwedBossIDs.Empty();
	Inventory.Empty();
	RecomputeItemDerivedStats();
	EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
	EquippedIdolLevels.Init(0, MaxEquippedIdolSlots);
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
