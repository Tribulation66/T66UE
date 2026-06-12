// Copyright Tribulation 66. All Rights Reserved.

#include "Core/RunState/T66RunStateSubsystem_Private.h"

using namespace T66RunStatePrivate;

namespace
{
	constexpr float T66BaseDamageMultiplierPerPoint = 0.0015f;
	constexpr float T66BaseAttackSpeedMultiplierPerPoint = 0.0012f;
	constexpr float T66BaseScaleMultiplierPerPoint = 0.0008f;
	constexpr float T66BaseAccuracyMultiplierPerPoint = 0.0010f;
	constexpr float T66BaseArmorReductionPerPoint = 0.0008f;
	constexpr float T66BaseEvasionChancePerPoint = 0.0006f;
	constexpr float T66StatChancePerBonusPoint = 0.001f;
	constexpr float T66StatSmallChancePerBonusPoint = 0.0005f;
	constexpr float T66StatRangePerBonusPoint = 2.5f;
	constexpr float T66StatMovementMultiplierPerBonusPoint = 0.002f;
	constexpr float T66StatElementalPowerPerBonusPoint = 0.005f;
	constexpr float T66InteractableLuckQualityTiltPerPoint = 0.005f;
	constexpr float T66StealingLuckChancePerPoint = 0.001f;
	constexpr float T66StealingLuckChanceCap = 0.95f;
	constexpr float T66GamblingLuckRescueRerollPerPoint = 0.001f;
	constexpr float T66GamblingLuckRescueRerollCap = 0.25f;
	constexpr float T66ProcLuckChancePerPoint = 0.0005f;
	constexpr float T66ProcLuckChanceCap = 0.95f;

	// Stats Rework: maps a legacy primary stat onto its unified replacement so all upgrade
	// sources (level-up, surgeries) bump the named stat directly. All eight are migrated; the old
	// primary fields now serve only as each stat's hidden innate base.
	ET66StatType T66MapBaseStatToUnifiedStat(const ET66HeroStatType BaseStatType)
	{
		switch (BaseStatType)
		{
		case ET66HeroStatType::Damage:      return ET66StatType::AllDamage;
		case ET66HeroStatType::AttackSpeed: return ET66StatType::AllAttackSpeed;
		case ET66HeroStatType::AttackScale: return ET66StatType::AllScale;
		case ET66HeroStatType::Armor:       return ET66StatType::DamageReduction;
		case ET66HeroStatType::Evasion:     return ET66StatType::EvasionChance;
		case ET66HeroStatType::Luck:        return ET66StatType::Luck;
		case ET66HeroStatType::Speed:       return ET66StatType::MovementSpeed;
		case ET66HeroStatType::Accuracy:    return ET66StatType::Accuracy;
		default:                            return ET66StatType::None;
		}
	}
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


int32 UT66RunStateSubsystem::GetItemBaseStatTenths(const ET66HeroStatType StatType) const
{
	switch (StatType)
	{
	case ET66HeroStatType::Damage:      return FMath::Max(0, ItemBaseStatBonusesPrecise.DamageTenths);
	case ET66HeroStatType::AttackSpeed: return FMath::Max(0, ItemBaseStatBonusesPrecise.AttackSpeedTenths);
	case ET66HeroStatType::AttackScale: return FMath::Max(0, ItemBaseStatBonusesPrecise.AttackScaleTenths);
	case ET66HeroStatType::Accuracy:    return FMath::Max(0, ItemBaseStatBonusesPrecise.AccuracyTenths);
	case ET66HeroStatType::Armor:       return FMath::Max(0, ItemBaseStatBonusesPrecise.ArmorTenths);
	case ET66HeroStatType::Evasion:     return FMath::Max(0, ItemBaseStatBonusesPrecise.EvasionTenths);
	case ET66HeroStatType::Luck:        return FMath::Max(0, ItemBaseStatBonusesPrecise.LuckTenths);
	case ET66HeroStatType::Speed:       return FMath::Max(0, ItemBaseStatBonusesPrecise.SpeedTenths);
	default:                            return 0;
	}
}


int32 UT66RunStateSubsystem::GetPermanentBaseBuffTenths(const ET66HeroStatType StatType) const
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
	case ET66HeroStatType::Speed:       return WholeStatToTenths(FMath::Max(0, PermanentBuffStatBonuses.Speed));
	default:                            return 0;
	}
}


int32 UT66RunStateSubsystem::GetTemporaryBaseStatAmplifierTenths(const ET66HeroStatType StatType) const
{
	int32 TotalTenths = 0;
	for (const FT66TemporaryBaseStatAmplifier& Amplifier : TemporaryBaseStatAmplifiers)
	{
		if (Amplifier.StatType == StatType && Amplifier.SecondsRemaining > 0.f)
		{
			TotalTenths += FMath::Max(0, Amplifier.BonusTenths);
		}
	}
	return TotalTenths;
}

int32 UT66RunStateSubsystem::GetTemporaryStatAmplifierTenths(const ET66StatType StatType) const
{
	int32 TotalTenths = 0;
	for (const FT66TemporaryStatAmplifier& Amplifier : TemporaryStatAmplifiers)
	{
		if (Amplifier.StatType == StatType && Amplifier.SecondsRemaining > 0.f)
		{
			TotalTenths += FMath::Max(0, Amplifier.BonusTenths);
		}
	}
	return TotalTenths;
}

int32 UT66RunStateSubsystem::GetSaintBlessingBaseStatTenths(const ET66HeroStatType StatType) const
{
	switch (StatType)
	{
	case ET66HeroStatType::Damage:      return FMath::Max(0, SaintBlessingBaseStatBonusesPrecise.DamageTenths);
	case ET66HeroStatType::AttackSpeed: return FMath::Max(0, SaintBlessingBaseStatBonusesPrecise.AttackSpeedTenths);
	case ET66HeroStatType::AttackScale: return FMath::Max(0, SaintBlessingBaseStatBonusesPrecise.AttackScaleTenths);
	case ET66HeroStatType::Accuracy:    return FMath::Max(0, SaintBlessingBaseStatBonusesPrecise.AccuracyTenths);
	case ET66HeroStatType::Armor:       return FMath::Max(0, SaintBlessingBaseStatBonusesPrecise.ArmorTenths);
	case ET66HeroStatType::Evasion:     return FMath::Max(0, SaintBlessingBaseStatBonusesPrecise.EvasionTenths);
	case ET66HeroStatType::Luck:        return FMath::Max(0, SaintBlessingBaseStatBonusesPrecise.LuckTenths);
	case ET66HeroStatType::Speed:       return FMath::Max(0, SaintBlessingBaseStatBonusesPrecise.SpeedTenths);
	default:                            return 0;
	}
}


int32 UT66RunStateSubsystem::GetPreciseBaseStatTenths(const ET66HeroStatType StatType) const
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

	switch (StatType)
	{
	case ET66HeroStatType::Damage:      TotalTenths += FMath::Max(0, NoIdolBaseStatBonusesPrecise.DamageTenths); break;
	case ET66HeroStatType::AttackSpeed: TotalTenths += FMath::Max(0, NoIdolBaseStatBonusesPrecise.AttackSpeedTenths); break;
	case ET66HeroStatType::AttackScale: TotalTenths += FMath::Max(0, NoIdolBaseStatBonusesPrecise.AttackScaleTenths); break;
	default: break;
	}

	TotalTenths += GetPermanentBaseBuffTenths(StatType);
	TotalTenths += GetSaintBlessingBaseStatTenths(StatType);
	TotalTenths += GetTemporaryBaseStatAmplifierTenths(StatType);
	return ClampHeroStatTenths(TotalTenths);
}


int32 UT66RunStateSubsystem::GetStatBonusTenths(const ET66StatType StatType) const
{
	const int32 ItemTenths = ItemStatBonusTenths.Contains(StatType)
		? FMath::Max(0, ItemStatBonusTenths.FindRef(StatType))
		: 0;
	const int32 PersistentTenths = PersistentStatBonusTenths.Contains(StatType)
		? FMath::Max(0, PersistentStatBonusTenths.FindRef(StatType))
		: 0;
	const int32 PermanentTenths = PermanentStatBonusTenths.Contains(StatType)
		? FMath::Max(0, PermanentStatBonusTenths.FindRef(StatType))
		: 0;
	const int32 SaintTenths = SaintBlessingStatBonusTenths.Contains(StatType)
		? FMath::Max(0, SaintBlessingStatBonusTenths.FindRef(StatType))
		: 0;
	const int32 TemporaryTenths = GetTemporaryStatAmplifierTenths(StatType);
	return FMath::Max(0, ItemTenths + PersistentTenths + PermanentTenths + SaintTenths + TemporaryTenths);
}


float UT66RunStateSubsystem::GetStatBonusValue(const ET66StatType StatType) const
{
	return TenthsToFloatStat(GetStatBonusTenths(StatType));
}

int32 UT66RunStateSubsystem::GetNoIdolBaseBonusTenthsForRarity(const ET66ItemRarity Rarity)
{
	switch (Rarity)
	{
	case ET66ItemRarity::Black:  return 5;
	case ET66ItemRarity::Red:    return 8;
	case ET66ItemRarity::Yellow: return 12;
	case ET66ItemRarity::White:  return 16;
	default:                     return 5;
	}
}

void UT66RunStateSubsystem::ApplyNoIdolSelection(const ET66ItemRarity Rarity)
{
	const int32 BonusTenths = GetNoIdolBaseBonusTenthsForRarity(Rarity);
	const int32 MaxBonusTenths = MaxHeroStatValue * HeroStatTenthsScale;
	NoIdolBaseStatBonusesPrecise.DamageTenths = FMath::Clamp(NoIdolBaseStatBonusesPrecise.DamageTenths + BonusTenths, 0, MaxBonusTenths);
	NoIdolBaseStatBonusesPrecise.AttackSpeedTenths = FMath::Clamp(NoIdolBaseStatBonusesPrecise.AttackSpeedTenths + BonusTenths, 0, MaxBonusTenths);
	NoIdolBaseStatBonusesPrecise.AttackScaleTenths = FMath::Clamp(NoIdolBaseStatBonusesPrecise.AttackScaleTenths + BonusTenths, 0, MaxBonusTenths);
	NoIdolSelectionStacks = FMath::Max(0, NoIdolSelectionStacks + 1);
	SyncLegacyHeroStatsFromPrecise();
	HeroProgressChanged.Broadcast();
}

void UT66RunStateSubsystem::RestoreNoIdolState(const int32 Stacks, const FT66HeroPreciseStatBlock& Bonuses)
{
	NoIdolSelectionStacks = FMath::Max(0, Stacks);
	NoIdolBaseStatBonusesPrecise = FT66HeroPreciseStatBlock{};
	const int32 MaxBonusTenths = MaxHeroStatValue * HeroStatTenthsScale;
	NoIdolBaseStatBonusesPrecise.DamageTenths = FMath::Clamp(Bonuses.DamageTenths, 0, MaxBonusTenths);
	NoIdolBaseStatBonusesPrecise.AttackSpeedTenths = FMath::Clamp(Bonuses.AttackSpeedTenths, 0, MaxBonusTenths);
	NoIdolBaseStatBonusesPrecise.AttackScaleTenths = FMath::Clamp(Bonuses.AttackScaleTenths, 0, MaxBonusTenths);
	SyncLegacyHeroStatsFromPrecise();
}


int32 UT66RunStateSubsystem::GetCategoryBaseStatTenths(const ET66StatType StatType) const
{
	switch (StatType)
	{
	case ET66StatType::AoeDamage:   return WholeStatToTenths(BaseAoeDmg);
	case ET66StatType::BounceDamage:return WholeStatToTenths(BaseBounceDmg);
	case ET66StatType::SummonDamage:return WholeStatToTenths(BaseSummonDmg);
	case ET66StatType::DotDamage:   return WholeStatToTenths(BaseDotDmg);
	case ET66StatType::AoeSpeed:    return WholeStatToTenths(BaseAoeAtkSpd);
	case ET66StatType::BounceSpeed: return WholeStatToTenths(BaseBounceAtkSpd);
	case ET66StatType::SummonSpeed: return WholeStatToTenths(BaseSummonAtkSpd);
	case ET66StatType::DotSpeed:    return WholeStatToTenths(BaseDotAtkSpd);
	case ET66StatType::AoeScale:    return WholeStatToTenths(BaseAoeAtkScale);
	case ET66StatType::BounceScale: return WholeStatToTenths(BaseBounceAtkScale);
	case ET66StatType::SummonScale: return WholeStatToTenths(BaseSummonAtkScale);
	case ET66StatType::DotScale:    return WholeStatToTenths(BaseDotAtkScale);
	default:                                 return 0;
	}
}


int32 UT66RunStateSubsystem::GetCategoryTotalStatTenths(const ET66StatType StatType) const
{
	return ClampHeroStatTenths(GetCategoryBaseStatTenths(StatType) + GetStatBonusTenths(StatType));
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


void UT66RunStateSubsystem::ClearPersistentStatBonuses()
{
	PersistentStatBonusTenths.Reset();
}


void UT66RunStateSubsystem::AddPersistentStatBonusTenths(const ET66StatType StatType, const int32 DeltaTenths)
{
	if (StatType == ET66StatType::None || DeltaTenths <= 0)
	{
		return;
	}

	int32& Accum = PersistentStatBonusTenths.FindOrAdd(StatType);
	Accum = FMath::Clamp(Accum + DeltaTenths, 0, MaxHeroStatValue * HeroStatTenthsScale);
}


#if !UE_BUILD_SHIPPING
void UT66RunStateSubsystem::DebugAddPersistentStatBonusTenths(const ET66StatType StatType, const int32 DeltaTenths)
{
	AddPersistentStatBonusTenths(StatType, DeltaTenths);
	HeroProgressChanged.Broadcast();
}
#endif


void UT66RunStateSubsystem::AddItemStatBonusTenths(const ET66StatType StatType, const int32 DeltaTenths)
{
	if (StatType == ET66StatType::None || DeltaTenths <= 0)
	{
		return;
	}

	int32& Accum = ItemStatBonusTenths.FindOrAdd(StatType);
	Accum = FMath::Clamp(Accum + DeltaTenths, 0, MaxHeroStatValue * HeroStatTenthsScale);
}


int32 UT66RunStateSubsystem::RollHeroBaseGainTenthsBiased(const FT66HeroStatGainRange& Range, const FName Category)
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


// Stats Rework: ApplyPrimaryGainToSecondaryBonuses removed. The primary->secondary fan-out is gone;
// surgeries and level-ups now bump unified stats directly (see ApplyPermanentBaseStat / ApplyOneHeroLevelUp).


void UT66RunStateSubsystem::InitializeHeroStatTuningForSelectedHero()
{
	// Safe defaults
	HeroStats = FT66HeroStatBlock{};
	HeroPreciseStats = FT66HeroPreciseStatBlock{};
	HeroPerLevelGains = FT66HeroPerLevelStatGains{};
	HeroPerLevelGains.Damage.Min = 2.0f;      HeroPerLevelGains.Damage.Max = 2.0f;
	HeroPerLevelGains.AttackSpeed.Min = 2.0f; HeroPerLevelGains.AttackSpeed.Max = 2.0f;
	HeroPerLevelGains.AttackScale.Min = 2.0f; HeroPerLevelGains.AttackScale.Max = 2.0f;
	HeroPerLevelGains.Accuracy.Min = 2.0f;    HeroPerLevelGains.Accuracy.Max = 2.0f;
	HeroPerLevelGains.Armor.Min = 2.0f;       HeroPerLevelGains.Armor.Max = 2.0f;
	HeroPerLevelGains.Evasion.Min = 2.0f;     HeroPerLevelGains.Evasion.Max = 2.0f;
	HeroPerLevelGains.Luck.Min = 2.0f;        HeroPerLevelGains.Luck.Max = 2.0f;
	HeroPerLevelGains.Speed.Min = 2.0f;       HeroPerLevelGains.Speed.Max = 2.0f;

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
			BaseSummonDmg = FMath::Max(1, HD.BaseSummonDmg);
			BaseSummonAtkSpd = FMath::Max(1, HD.BaseSummonAtkSpd);
			BaseSummonAtkScale = FMath::Max(1, HD.BaseSummonAtkScale);
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
			HeroBaseHeadshotChance = FMath::Clamp(HD.BaseHeadshotChance, 0.f, 1.f);
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
			PassiveType = ET66PassiveType::None;
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


void UT66RunStateSubsystem::RefreshPermanentBuffBonusesFromProfile()
{
	PermanentBuffStatBonuses = FT66HeroStatBonuses{};
	PermanentStatBonusTenths.Reset();

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UT66BuffSubsystem* Buffs = GI->GetSubsystem<UT66BuffSubsystem>())
		{
			PermanentBuffStatBonuses = Buffs->GetPermanentBuffStatBonuses();
			const auto ApplyElementSurgery = [this, Buffs](const ET66StatType StatType)
			{
				const int32 Bonus = Buffs->GetSurgeryStatBonus(StatType);
				if (Bonus > 0)
				{
					PermanentStatBonusTenths.FindOrAdd(StatType) += WholeStatToTenths(Bonus);
				}
			};
			ApplyElementSurgery(ET66StatType::FirePower);
			ApplyElementSurgery(ET66StatType::IcePower);
			ApplyElementSurgery(ET66StatType::ElectricityPower);
			ApplyElementSurgery(ET66StatType::NaturePower);
			ApplyElementSurgery(ET66StatType::WindPower);
		}
	}

	auto ApplyPermanentBaseStat = [&](const ET66HeroStatType StatType)
	{
		const int32 GainTenths = GetPermanentBaseBuffTenths(StatType);
		if (GainTenths <= 0)
		{
			return;
		}

		// Stats Rework: a surgery/permanent buff bumps its named unified stat directly as additive
		// percent instead of fanning a primary out across categories.
		const ET66StatType UnifiedType = T66MapBaseStatToUnifiedStat(StatType);
		if (UnifiedType != ET66StatType::None)
		{
			PermanentStatBonusTenths.FindOrAdd(UnifiedType) += GainTenths;
		}
	};

	ApplyPermanentBaseStat(ET66HeroStatType::Damage);
	ApplyPermanentBaseStat(ET66HeroStatType::AttackSpeed);
	ApplyPermanentBaseStat(ET66HeroStatType::AttackScale);
	ApplyPermanentBaseStat(ET66HeroStatType::Accuracy);
	ApplyPermanentBaseStat(ET66HeroStatType::Armor);
	ApplyPermanentBaseStat(ET66HeroStatType::Evasion);
	ApplyPermanentBaseStat(ET66HeroStatType::Luck);
	ApplyPermanentBaseStat(ET66HeroStatType::Speed);
}


void UT66RunStateSubsystem::ApplyOneHeroLevelUp()
{
	if (HeroLevel >= MaxHeroLevel)
	{
		return;
	}

	HeroLevel = FMath::Clamp(HeroLevel + 1, DefaultHeroLevel, MaxHeroLevel);

	auto GetBaseStatCategoryName = [](const ET66HeroStatType StatType) -> FName
	{
		switch (StatType)
		{
		case ET66HeroStatType::Damage:      return FName(TEXT("Damage"));
		case ET66HeroStatType::AttackSpeed: return FName(TEXT("AttackSpeed"));
		case ET66HeroStatType::AttackScale: return FName(TEXT("AttackScale"));
		case ET66HeroStatType::Accuracy:    return FName(TEXT("Accuracy"));
		case ET66HeroStatType::Armor:       return FName(TEXT("Armor"));
		case ET66HeroStatType::Evasion:     return FName(TEXT("Evasion"));
		case ET66HeroStatType::Luck:        return FName(TEXT("Luck"));
		case ET66HeroStatType::Speed:       return FName(TEXT("Speed"));
		default:                            return FName(TEXT("Primary"));
		}
	};

	auto ApplyGain = [&](const ET66HeroStatType StatType, const FT66HeroStatGainRange& Range)
	{
		const int32 GainTenths = RollHeroBaseGainTenthsBiased(Range, GetBaseStatCategoryName(StatType));
		if (GainTenths <= 0)
		{
			return;
		}

		// Stats Rework: leveling grants the gain as additive percent into the unified stat
		// (each tenth = 0.1%).
		AddPersistentStatBonusTenths(T66MapBaseStatToUnifiedStat(StatType), GainTenths);
	};

	ApplyGain(ET66HeroStatType::Damage, HeroPerLevelGains.Damage);
	ApplyGain(ET66HeroStatType::AttackSpeed, HeroPerLevelGains.AttackSpeed);
	ApplyGain(ET66HeroStatType::AttackScale, HeroPerLevelGains.AttackScale);
	ApplyGain(ET66HeroStatType::Accuracy, HeroPerLevelGains.Accuracy);
	ApplyGain(ET66HeroStatType::Armor, HeroPerLevelGains.Armor);
	ApplyGain(ET66HeroStatType::Evasion, HeroPerLevelGains.Evasion);
	ApplyGain(ET66HeroStatType::Luck, HeroPerLevelGains.Luck);
	ApplyGain(ET66HeroStatType::Speed, HeroPerLevelGains.Speed);

	SyncLegacyHeroStatsFromPrecise();
	ResetHeroDamagePercent();
	const int32 WaveKills = ApplyLevelUpWave(GetDataDrivenLevelUpWaveRadiusUU());
	AddStructuredEvent(
		ET66RunEventType::DamageDealt,
		FString::Printf(TEXT("Type=LevelUp,Level=%d,WaveRadius=%.0f,WaveKills=%d"), HeroLevel, GetDataDrivenLevelUpWaveRadiusUU(), WaveKills));
	HeartsChanged.Broadcast();
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
	ClearPersistentStatBonuses();

	HeroLevel = DefaultHeroLevel;
	HeroXP = 0;
	XPToNextLevel = GetDataDrivenLevelUpXPThreshold();
	SyncLegacyHeroStatsFromPrecise();
}


int32 UT66RunStateSubsystem::GetDataDrivenLevelUpXPThreshold() const
{
	ET66Difficulty Difficulty = ET66Difficulty::Easy;
	if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		Difficulty = T66GI->SelectedDifficulty;
	}

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UT66PlayerExperienceSubSystem* PlayerExperience = GI->GetSubsystem<UT66PlayerExperienceSubSystem>())
		{
			return FMath::Max(1, PlayerExperience->GetDifficultyLevelUpXPThreshold(Difficulty));
		}
	}

	return FMath::Max(1, DefaultXPToLevel);
}


float UT66RunStateSubsystem::GetDataDrivenLevelUpWaveRadiusUU() const
{
	ET66Difficulty Difficulty = ET66Difficulty::Easy;
	if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		Difficulty = T66GI->SelectedDifficulty;
	}

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UT66PlayerExperienceSubSystem* PlayerExperience = GI->GetSubsystem<UT66PlayerExperienceSubSystem>())
		{
			return FMath::Max(0.f, PlayerExperience->GetDifficultyLevelUpWaveRadiusUU(Difficulty));
		}
	}

	return 900.f;
}


float UT66RunStateSubsystem::GetDataDrivenHeadshotChancePerBonusPoint() const
{
	ET66Difficulty Difficulty = ET66Difficulty::Easy;
	if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		Difficulty = T66GI->SelectedDifficulty;
	}

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UT66PlayerExperienceSubSystem* PlayerExperience = GI->GetSubsystem<UT66PlayerExperienceSubSystem>())
		{
			return FMath::Clamp(PlayerExperience->GetDifficultyHeadshotChancePerBonusPoint(Difficulty), 0.f, 1.f);
		}
	}

	return T66StatSmallChancePerBonusPoint;
}


float UT66RunStateSubsystem::GetDataDrivenHeadshotStunDurationSeconds() const
{
	ET66Difficulty Difficulty = ET66Difficulty::Easy;
	if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		Difficulty = T66GI->SelectedDifficulty;
	}

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UT66PlayerExperienceSubSystem* PlayerExperience = GI->GetSubsystem<UT66PlayerExperienceSubSystem>())
		{
			return FMath::Max(0.f, PlayerExperience->GetDifficultyHeadshotStunDurationSeconds(Difficulty));
		}
	}

	return 0.75f;
}


// Stats Rework: ApplyLevelUpPrimaryGainTenths removed. Leveling no longer raises primary stat points;
// gains route into unified stats via T66MapBaseStatToUnifiedStat in ApplyOneHeroLevelUp.


int32 UT66RunStateSubsystem::ApplyLevelUpWave(const float RadiusUU)
{
	UWorld* World = GetWorld();
	if (!World || RadiusUU <= 0.f)
	{
		return 0;
	}

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if (!PlayerPawn)
	{
		return 0;
	}

	const FVector Origin = PlayerPawn->GetActorLocation();
	const float RadiusSq = FMath::Square(RadiusUU);
	UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
	if (!Registry)
	{
		return 0;
	}

	TArray<AT66EnemyBase*> EnemiesToKill;
	for (const TWeakObjectPtr<AT66EnemyBase>& WeakEnemy : Registry->GetEnemies())
	{
		AT66EnemyBase* Enemy = WeakEnemy.Get();
		if (Enemy && Enemy->CurrentHP > 0 && FVector::DistSquared2D(Origin, Enemy->GetActorLocation()) <= RadiusSq)
		{
			EnemiesToKill.Add(Enemy);
		}
	}

	TArray<AT66MobBase*> MobsToKill;
	for (const TWeakObjectPtr<AT66MobBase>& WeakMob : Registry->GetActiveMobs())
	{
		AT66MobBase* Mob = WeakMob.Get();
		if (Mob && Mob->IsAliveAndActive() && FVector::DistSquared2D(Origin, Mob->GetActorLocation()) <= RadiusSq)
		{
			MobsToKill.Add(Mob);
		}
	}

	const bool bPreviousSuppressLevelUpWaveXP = bSuppressLevelUpWaveXP;
	bSuppressLevelUpWaveXP = true;

	int32 KillCount = 0;
	for (AT66EnemyBase* Enemy : EnemiesToKill)
	{
		if (Enemy && T66CombatShared::TryApplyNonBossOHKO(Enemy, nullptr, FName(TEXT("LevelUpWave")), UT66FloatingCombatTextSubsystem::EventType_LevelUp))
		{
			++KillCount;
		}
	}

	for (AT66MobBase* Mob : MobsToKill)
	{
		if (Mob && T66CombatShared::TryApplyNonBossOHKO(Mob, nullptr, FName(TEXT("LevelUpWave")), UT66FloatingCombatTextSubsystem::EventType_LevelUp))
		{
			++KillCount;
		}
	}

	bSuppressLevelUpWaveXP = bPreviousSuppressLevelUpWaveXP;
	return KillCount;
}


int32 UT66RunStateSubsystem::GetSpeedStat() const
{
	// Stats Rework: Move Speed = innate Speed base (excl. surgery) + Movement Speed stat upgrades.
	const int32 BaseTenths = FMath::Max(0, GetPreciseBaseStatTenths(ET66HeroStatType::Speed) - GetPermanentBaseBuffTenths(ET66HeroStatType::Speed));
	const int32 InnateDisplay = TenthsToDisplayStat(BaseTenths);
	const int32 UpgradePoints = FMath::RoundToInt(GetStatBonusValue(ET66StatType::MovementSpeed));
	return ClampHeroStatValue(InnateDisplay + UpgradePoints);
}


int32 UT66RunStateSubsystem::GetDamageStat() const
{
	return TenthsToDisplayStat(GetPreciseBaseStatTenths(ET66HeroStatType::Damage));
}


int32 UT66RunStateSubsystem::GetAttackSpeedStat() const
{
	return TenthsToDisplayStat(GetPreciseBaseStatTenths(ET66HeroStatType::AttackSpeed));
}


int32 UT66RunStateSubsystem::GetScaleStat() const
{
	return TenthsToDisplayStat(GetPreciseBaseStatTenths(ET66HeroStatType::AttackScale));
}


int32 UT66RunStateSubsystem::GetAccuracyStat() const
{
	return TenthsToDisplayStat(GetPreciseBaseStatTenths(ET66HeroStatType::Accuracy));
}


int32 UT66RunStateSubsystem::GetArmorStat() const
{
	return TenthsToDisplayStat(GetPreciseBaseStatTenths(ET66HeroStatType::Armor));
}


int32 UT66RunStateSubsystem::GetEvasionStat() const
{
	return TenthsToDisplayStat(GetPreciseBaseStatTenths(ET66HeroStatType::Evasion));
}


int32 UT66RunStateSubsystem::GetLuckStat() const
{
	return TenthsToDisplayStat(GetPreciseBaseStatTenths(ET66HeroStatType::Luck));
}


float UT66RunStateSubsystem::GetSingleUseLuckModifierPercent() const
{
	return 0.f;
}


float UT66RunStateSubsystem::GetTotalLuckModifierPercent() const
{
	// Stats Rework: surgery power flows through the unified Luck stat, so the innate luck base
	// excludes permanent-buff contribution; upgrades add as percent points.
	const int32 BaseTenths = FMath::Max(0, GetPreciseBaseStatTenths(ET66HeroStatType::Luck) - GetPermanentBaseBuffTenths(ET66HeroStatType::Luck));
	const int32 InnateLuckDisplay = TenthsToDisplayStat(BaseTenths);
	const float BaseLuckModifierPercent = static_cast<float>(FMath::Max(0, InnateLuckDisplay - 1)) + GetStatBonusValue(ET66StatType::Luck);
	return FMath::Max(0.f, BaseLuckModifierPercent + GetSingleUseLuckModifierPercent());
}


float UT66RunStateSubsystem::GetEffectiveLuckValue() const
{
	const float SeedLuck = static_cast<float>(GetSeedLuck0To100());
	return FMath::Max(0.f, SeedLuck * (1.f + (GetTotalLuckModifierPercent() / 100.f)));
}


int32 UT66RunStateSubsystem::GetEffectiveLuckBiasStat() const
{
	return FMath::Max(1, FMath::RoundToInt(GetEffectiveLuckValue()));
}


FText UT66RunStateSubsystem::GetSeedLuckAdjectiveText(const int32 InSeedLuck0To100)
{
	const int32 ClampedLuck = FMath::Clamp(InSeedLuck0To100, 0, 100);
	if (ClampedLuck >= 90) return NSLOCTEXT("T66.RunState", "SeedLuck_Mythic", "Mythic");
	if (ClampedLuck >= 75) return NSLOCTEXT("T66.RunState", "SeedLuck_Blessed", "Blessed");
	if (ClampedLuck >= 60) return NSLOCTEXT("T66.RunState", "SeedLuck_Fortunate", "Fortunate");
	if (ClampedLuck >= 40) return NSLOCTEXT("T66.RunState", "SeedLuck_Steady", "Steady");
	if (ClampedLuck >= 20) return NSLOCTEXT("T66.RunState", "SeedLuck_Starved", "Starved");
	return NSLOCTEXT("T66.RunState", "SeedLuck_Cursed", "Cursed");
}


int32 UT66RunStateSubsystem::GetSummonDmgStat() const      { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66StatType::SummonDamage)); }


int32 UT66RunStateSubsystem::GetSummonAtkSpdStat() const   { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66StatType::SummonSpeed)); }


int32 UT66RunStateSubsystem::GetSummonAtkScaleStat() const { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66StatType::SummonScale)); }


int32 UT66RunStateSubsystem::GetBounceDmgStat() const      { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66StatType::BounceDamage)); }


int32 UT66RunStateSubsystem::GetBounceAtkSpdStat() const   { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66StatType::BounceSpeed)); }


int32 UT66RunStateSubsystem::GetBounceAtkScaleStat() const { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66StatType::BounceScale)); }


int32 UT66RunStateSubsystem::GetAoeDmgStat() const         { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66StatType::AoeDamage)); }


int32 UT66RunStateSubsystem::GetAoeAtkSpdStat() const      { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66StatType::AoeSpeed)); }


int32 UT66RunStateSubsystem::GetAoeAtkScaleStat() const    { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66StatType::AoeScale)); }


int32 UT66RunStateSubsystem::GetDotDmgStat() const         { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66StatType::DotDamage)); }


int32 UT66RunStateSubsystem::GetDotAtkSpdStat() const      { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66StatType::DotSpeed)); }


int32 UT66RunStateSubsystem::GetDotAtkScaleStat() const    { return TenthsToDisplayStat(GetCategoryTotalStatTenths(ET66StatType::DotScale)); }


float UT66RunStateSubsystem::GetStatValue(ET66StatType StatType) const
{
	float M = 1.f;
	if (const float* Mult = StatMultipliers.Find(StatType); Mult && *Mult > 0.f)
	{
		M *= *Mult;
	}
	if (const float* SingleUseMult = SingleUseStatMultipliers.Find(StatType); SingleUseMult && *SingleUseMult > 0.f)
	{
		M *= *SingleUseMult;
	}
	const float DamageMult = GetHeroDamageMultiplier();
	const float AttackSpeedMult = GetHeroAttackSpeedMultiplier();
	const float ScaleMult = GetHeroScaleMultiplier();
	const float AccuracyMult = GetHeroAccuracyMultiplier();
	const float BonusPoints = GetStatBonusValue(StatType);
	const float BaseArmorReduction = FMath::Clamp(static_cast<float>(GetArmorStat() - 1) * T66BaseArmorReductionPerPoint, 0.f, 0.80f);
	const float BaseEvasionChance = FMath::Clamp(static_cast<float>(GetEvasionStat() - 1) * T66BaseEvasionChancePerPoint, 0.f, 0.60f);

	// Stats Rework: magnitude stats resolve as hidden base x (1 + accumulated percent) x global
	// category multiplier. Each accumulated stat point is read as one percent.
	auto MagnitudeValue = [this, M](const ET66StatType MagnitudeType, const float CategoryMultiplier) -> float
	{
		return TenthsToFloatStat(GetCategoryBaseStatTenths(MagnitudeType)) * (1.f + GetStatBonusValue(MagnitudeType) * 0.01f) * M * CategoryMultiplier;
	};

	switch (StatType)
	{
	case ET66StatType::AoeDamage:        return MagnitudeValue(ET66StatType::AoeDamage, DamageMult);
	case ET66StatType::BounceDamage:     return MagnitudeValue(ET66StatType::BounceDamage, DamageMult);
	case ET66StatType::SummonDamage:     return MagnitudeValue(ET66StatType::SummonDamage, DamageMult);
	case ET66StatType::DotDamage:        return MagnitudeValue(ET66StatType::DotDamage, DamageMult);
	case ET66StatType::CritDamage:       return 2.0f;
	case ET66StatType::CloseRangeDamage: return 1.f;
	case ET66StatType::LongRangeDamage:  return 1.f;
	case ET66StatType::AoeSpeed:         return MagnitudeValue(ET66StatType::AoeSpeed, AttackSpeedMult);
	case ET66StatType::BounceSpeed:      return MagnitudeValue(ET66StatType::BounceSpeed, AttackSpeedMult);
	case ET66StatType::SummonSpeed:      return MagnitudeValue(ET66StatType::SummonSpeed, AttackSpeedMult);
	case ET66StatType::DotSpeed:         return MagnitudeValue(ET66StatType::DotSpeed, AttackSpeedMult);
	case ET66StatType::CritChance:       return ApplyProcLuckToChance01(FMath::Clamp((HeroBaseCritChance + (BonusPoints * T66StatChancePerBonusPoint)) * M * AccuracyMult, 0.f, 1.f));
	case ET66StatType::AoeScale:         return MagnitudeValue(ET66StatType::AoeScale, ScaleMult);
	case ET66StatType::BounceScale:      return MagnitudeValue(ET66StatType::BounceScale, ScaleMult);
	case ET66StatType::SummonScale:      return MagnitudeValue(ET66StatType::SummonScale, ScaleMult);
	case ET66StatType::DotScale:         return MagnitudeValue(ET66StatType::DotScale, ScaleMult);
	case ET66StatType::AttackRange:      return FMath::Max(100.f, (HeroBaseAttackRange + (BonusPoints * T66StatRangePerBonusPoint)) * M * AccuracyMult);
	case ET66StatType::Accuracy:         return FMath::Clamp((HeroBaseAccuracy + (BonusPoints * T66StatChancePerBonusPoint)) * M * AccuracyMult, 0.f, 1.f);
	case ET66StatType::Execute:          return ApplyProcLuckToChance01(FMath::Clamp((BonusPoints * T66StatSmallChancePerBonusPoint) * M, 0.f, 1.f));
	case ET66StatType::Taunt:            return ApplyProcLuckToChance01(FMath::Clamp((BonusPoints * T66StatChancePerBonusPoint) * M, 0.f, 1.f));
	case ET66StatType::ReflectDamage:    return ApplyProcLuckToChance01(FMath::Clamp((HeroBaseReflectDmg + (BonusPoints * T66StatChancePerBonusPoint)) * M, 0.f, 1.f));
	case ET66StatType::HpRegen:          return FMath::Max(0.f, (HeroBaseHpRegen + (BonusPoints * 0.10f)) * M);
	case ET66StatType::Crush:            return ApplyProcLuckToChance01(FMath::Clamp((HeroBaseCrushChance + (BonusPoints * T66StatSmallChancePerBonusPoint)) * M, 0.f, 1.f));
	case ET66StatType::Invisibility:     return ApplyProcLuckToChance01(FMath::Clamp((HeroBaseInvisChance + (BonusPoints * T66StatSmallChancePerBonusPoint)) * M, 0.f, 1.f));
	case ET66StatType::CounterAttack:    return ApplyProcLuckToChance01(FMath::Clamp((HeroBaseCounterAttack + (BonusPoints * T66StatChancePerBonusPoint)) * M, 0.f, 1.f));
	case ET66StatType::LifeSteal:        return ApplyProcLuckToChance01(FMath::Clamp((HeroBaseLifeSteal + (BonusPoints * T66StatSmallChancePerBonusPoint)) * M, 0.f, 1.f));
	case ET66StatType::Assassinate:      return ApplyProcLuckToChance01(FMath::Clamp((HeroBaseAssassinateChance + (BonusPoints * T66StatSmallChancePerBonusPoint)) * M, 0.f, 1.f));
	case ET66StatType::SpinWheel:        return 1.f;
	case ET66StatType::Goblin:           return 1.f * M;
	case ET66StatType::Leprechaun:       return 1.f * M;
	case ET66StatType::TreasureChest:    return FMath::Max(1.f, (1.f + (BonusPoints * T66InteractableLuckQualityTiltPerPoint)) * M);
	case ET66StatType::Fountain:         return 1.f * M;
	case ET66StatType::Cheating:         return FMath::Clamp((BonusPoints * T66GamblingLuckRescueRerollPerPoint) * M, 0.f, T66GamblingLuckRescueRerollCap);
	case ET66StatType::Stealing:         return FMath::Clamp((BonusPoints * T66StealingLuckChancePerPoint) * M, 0.f, T66StealingLuckChanceCap);
	case ET66StatType::MovementSpeed:    return FMath::Max(1.f, (1.f + (BonusPoints * T66StatMovementMultiplierPerBonusPoint)) * M);
	case ET66StatType::LootCrate:        return FMath::Max(1.f, (1.f + (BonusPoints * T66InteractableLuckQualityTiltPerPoint)) * M);
	case ET66StatType::LootBag:          return FMath::Max(1.f, (1.f + (BonusPoints * T66InteractableLuckQualityTiltPerPoint)) * M);
	case ET66StatType::LootWheel:        return FMath::Max(1.f, (1.f + (BonusPoints * T66InteractableLuckQualityTiltPerPoint)) * M);
	case ET66StatType::DamageReduction:  return FMath::Clamp(BaseArmorReduction + (BonusPoints * T66StatSmallChancePerBonusPoint), 0.f, 0.80f);
	case ET66StatType::EvasionChance:    return FMath::Clamp(BaseEvasionChance + (BonusPoints * T66StatSmallChancePerBonusPoint), 0.f, 0.60f);
	case ET66StatType::Alchemy:          return FMath::Clamp(BonusPoints * T66StatChancePerBonusPoint * M, 0.f, 1.f);
	case ET66StatType::VendorToken:      return 0.f;
	case ET66StatType::HeadshotChance:   return ApplyProcLuckToChance01(FMath::Clamp((HeroBaseHeadshotChance + (BonusPoints * GetDataDrivenHeadshotChancePerBonusPoint())) * M * AccuracyMult, 0.f, 1.f));
	case ET66StatType::FirePower:        return FMath::Max(1.f, (1.f + (BonusPoints * T66StatElementalPowerPerBonusPoint)) * M);
	case ET66StatType::IcePower:         return FMath::Max(1.f, (1.f + (BonusPoints * T66StatElementalPowerPerBonusPoint)) * M);
	case ET66StatType::ElectricityPower: return FMath::Max(1.f, (1.f + (BonusPoints * T66StatElementalPowerPerBonusPoint)) * M);
	case ET66StatType::NaturePower:      return FMath::Max(1.f, (1.f + (BonusPoints * T66StatElementalPowerPerBonusPoint)) * M);
	case ET66StatType::WindPower:        return FMath::Max(1.f, (1.f + (BonusPoints * T66StatElementalPowerPerBonusPoint)) * M);
	case ET66StatType::InteractableLuck: return FMath::Max(1.f, (1.f + (BonusPoints * T66InteractableLuckQualityTiltPerPoint)) * M);
	case ET66StatType::StealingLuck:     return FMath::Clamp((BonusPoints * T66StealingLuckChancePerPoint) * M, 0.f, T66StealingLuckChanceCap);
	case ET66StatType::GamblingLuck:     return FMath::Clamp((BonusPoints * T66GamblingLuckRescueRerollPerPoint) * M, 0.f, T66GamblingLuckRescueRerollCap);
	case ET66StatType::ProcLuck:         return FMath::Clamp((BonusPoints * T66ProcLuckChancePerPoint) * M, 0.f, T66ProcLuckChanceCap);
	default: return 1.f;
	}
}


float UT66RunStateSubsystem::GetStatBaselineValue(ET66StatType StatType) const
{
	switch (StatType)
	{
	case ET66StatType::AoeDamage:       return static_cast<float>(BaseAoeDmg);
	case ET66StatType::BounceDamage:    return static_cast<float>(BaseBounceDmg);
	case ET66StatType::SummonDamage:    return static_cast<float>(BaseSummonDmg);
	case ET66StatType::DotDamage:       return static_cast<float>(BaseDotDmg);
	case ET66StatType::CritDamage:      return 2.0f;
	case ET66StatType::CloseRangeDamage:return 1.f;
	case ET66StatType::LongRangeDamage: return 1.f;
	case ET66StatType::AoeSpeed:        return static_cast<float>(BaseAoeAtkSpd);
	case ET66StatType::BounceSpeed:     return static_cast<float>(BaseBounceAtkSpd);
	case ET66StatType::SummonSpeed:     return static_cast<float>(BaseSummonAtkSpd);
	case ET66StatType::DotSpeed:        return static_cast<float>(BaseDotAtkSpd);
	case ET66StatType::CritChance:      return FMath::Clamp(HeroBaseCritChance, 0.f, 1.f);
	case ET66StatType::AoeScale:        return static_cast<float>(BaseAoeAtkScale);
	case ET66StatType::BounceScale:     return static_cast<float>(BaseBounceAtkScale);
	case ET66StatType::SummonScale:     return static_cast<float>(BaseSummonAtkScale);
	case ET66StatType::DotScale:        return static_cast<float>(BaseDotAtkScale);
	case ET66StatType::AttackRange:     return HeroBaseAttackRange;
	case ET66StatType::Accuracy:        return FMath::Clamp(HeroBaseAccuracy, 0.f, 1.f);
	case ET66StatType::Execute:         return 0.f;
	case ET66StatType::Taunt:           return HeroBaseTaunt;
	case ET66StatType::ReflectDamage:   return FMath::Clamp(HeroBaseReflectDmg, 0.f, 1.f);
	case ET66StatType::HpRegen:         return 0.f;
	case ET66StatType::Crush:           return FMath::Clamp(HeroBaseCrushChance, 0.f, 1.f);
	case ET66StatType::Invisibility:    return FMath::Clamp(HeroBaseInvisChance, 0.f, 1.f);
	case ET66StatType::CounterAttack:   return FMath::Clamp(HeroBaseCounterAttack, 0.f, 1.f);
	case ET66StatType::LifeSteal:       return 0.f;
	case ET66StatType::Assassinate:     return FMath::Clamp(HeroBaseAssassinateChance, 0.f, 1.f);
	case ET66StatType::SpinWheel:       return 1.f;
	case ET66StatType::Goblin:          return 1.f;
	case ET66StatType::Leprechaun:      return 1.f;
	case ET66StatType::TreasureChest:   return 1.f;
	case ET66StatType::Fountain:        return 1.f;
	case ET66StatType::Cheating:        return FMath::Clamp(HeroBaseCheatChance, 0.f, 1.f);
	case ET66StatType::Stealing:        return FMath::Clamp(HeroBaseStealChance, 0.f, 1.f);
	case ET66StatType::MovementSpeed:   return 1.f;
	case ET66StatType::LootCrate:       return 1.f;
	case ET66StatType::LootBag:         return 1.f;
	case ET66StatType::LootWheel:       return 1.f;
	case ET66StatType::DamageReduction: return 0.f;
	case ET66StatType::EvasionChance:   return 0.f;
	case ET66StatType::Alchemy:         return 0.f;
	case ET66StatType::VendorToken:     return 0.f;
	case ET66StatType::HeadshotChance:  return FMath::Clamp(HeroBaseHeadshotChance, 0.f, 1.f);
	case ET66StatType::FirePower:       return 1.f;
	case ET66StatType::IcePower:        return 1.f;
	case ET66StatType::ElectricityPower:return 1.f;
	case ET66StatType::NaturePower:     return 1.f;
	case ET66StatType::WindPower:       return 1.f;
	case ET66StatType::InteractableLuck:return 1.f;
	case ET66StatType::StealingLuck:    return 0.f;
	case ET66StatType::GamblingLuck:    return 0.f;
	case ET66StatType::ProcLuck:        return 0.f;
	default:                                     return 1.f;
	}
}


float UT66RunStateSubsystem::GetAggroMultiplier() const
{
	return GetStatValue(ET66StatType::Taunt);
}


float UT66RunStateSubsystem::GetHpRegenPerSecond() const
{
	return GetStatValue(ET66StatType::HpRegen);
}


float UT66RunStateSubsystem::GetLootCrateRewardMultiplier() const
{
	return GetInteractableLuckRewardMultiplier();
}


float UT66RunStateSubsystem::GetLootChestRewardMultiplier() const
{
	return GetInteractableLuckRewardMultiplier();
}


float UT66RunStateSubsystem::GetLootBagRewardMultiplier() const
{
	return GetInteractableLuckRewardMultiplier();
}


float UT66RunStateSubsystem::GetLootWheelRewardMultiplier() const
{
	return GetInteractableLuckRewardMultiplier();
}

float UT66RunStateSubsystem::GetInteractableLuckRewardMultiplier() const
{
	return FMath::Max(
		GetStatValue(ET66StatType::InteractableLuck),
		FMath::Max(
			GetStatValue(ET66StatType::LootCrate),
			FMath::Max(
				GetStatValue(ET66StatType::TreasureChest),
				FMath::Max(GetStatValue(ET66StatType::LootBag), GetStatValue(ET66StatType::LootWheel)))));
}

float UT66RunStateSubsystem::GetStealingLuckChanceBonus01() const
{
	return FMath::Clamp(
		FMath::Max(GetStatValue(ET66StatType::StealingLuck), GetStatValue(ET66StatType::Stealing)),
		0.f,
		T66StealingLuckChanceCap);
}

float UT66RunStateSubsystem::GetGamblingLuckRescueRerollChance01() const
{
	return FMath::Clamp(
		FMath::Max(GetStatValue(ET66StatType::GamblingLuck), GetStatValue(ET66StatType::Cheating)),
		0.f,
		T66GamblingLuckRescueRerollCap);
}

float UT66RunStateSubsystem::GetProcLuckChanceBonus01() const
{
	return GetStatValue(ET66StatType::ProcLuck);
}

float UT66RunStateSubsystem::ApplyProcLuckToChance01(const float BaseChance01) const
{
	const float Base = FMath::Clamp(BaseChance01, 0.f, T66ProcLuckChanceCap);
	if (Base <= 0.f)
	{
		return 0.f;
	}
	return FMath::Clamp(Base + GetProcLuckChanceBonus01(), 0.f, T66ProcLuckChanceCap);
}


float UT66RunStateSubsystem::GetExecuteChance01() const
{
	return GetStatValue(ET66StatType::Execute);
}


float UT66RunStateSubsystem::GetAlchemyLuckyUpgradeChance01() const
{
	return GetStatValue(ET66StatType::Alchemy);
}


float UT66RunStateSubsystem::GetCritChance01() const
{
	return GetStatValue(ET66StatType::CritChance);
}


float UT66RunStateSubsystem::GetCritDamageMultiplier() const
{
	return 2.0f;
}


float UT66RunStateSubsystem::GetHeadshotChance01() const
{
	return GetStatValue(ET66StatType::HeadshotChance);
}


float UT66RunStateSubsystem::GetHeadshotStunDurationSeconds() const
{
	return GetDataDrivenHeadshotStunDurationSeconds();
}


float UT66RunStateSubsystem::GetLifeStealFraction() const
{
	return GetStatValue(ET66StatType::LifeSteal);
}


float UT66RunStateSubsystem::GetReflectDamageFraction() const
{
	return GetStatValue(ET66StatType::ReflectDamage);
}


float UT66RunStateSubsystem::GetCrushChance01() const
{
	return GetStatValue(ET66StatType::Crush);
}


float UT66RunStateSubsystem::GetAssassinateChance01() const
{
	return GetStatValue(ET66StatType::Assassinate);
}


float UT66RunStateSubsystem::GetInvisibilityChance01() const
{
	return GetStatValue(ET66StatType::Invisibility);
}


float UT66RunStateSubsystem::GetCounterAttackFraction() const
{
	return GetStatValue(ET66StatType::CounterAttack);
}


float UT66RunStateSubsystem::GetCloseRangeThreshold() const
{
	return GetStatValue(ET66StatType::AttackRange) * 0.10f;
}


float UT66RunStateSubsystem::GetLongRangeThreshold() const
{
	return GetStatValue(ET66StatType::AttackRange) * 0.90f;
}


float UT66RunStateSubsystem::GetCloseRangeDamageMultiplier() const
{
	return 1.f;
}


float UT66RunStateSubsystem::GetLongRangeDamageMultiplier() const
{
	return 1.f;
}


float UT66RunStateSubsystem::GetHeroDamageMultiplier() const
{
	// Stats Rework: surgery power now flows through the unified "All Damage" stat, so the innate
	// base excludes permanent-buff contribution; upgrades apply as additive percent (point = 1%).
	const int32 BaseTenths = FMath::Max(0, GetPreciseBaseStatTenths(ET66HeroStatType::Damage) - GetPermanentBaseBuffTenths(ET66HeroStatType::Damage));
	const float D = TenthsToFloatStat(BaseTenths);
	const float BaseMult = 1.f + FMath::Max(0.f, D - 1.f) * T66BaseDamageMultiplierPerPoint;
	return BaseMult * (1.f + GetStatBonusValue(ET66StatType::AllDamage) * 0.01f);
}


float UT66RunStateSubsystem::GetHeroAttackSpeedMultiplier() const
{
	// Stats Rework: upgrades flow through the unified "All Attack Speed" stat; innate base
	// excludes permanent-buff contribution to avoid double counting.
	const int32 BaseTenths = FMath::Max(0, GetPreciseBaseStatTenths(ET66HeroStatType::AttackSpeed) - GetPermanentBaseBuffTenths(ET66HeroStatType::AttackSpeed));
	const float AttackSpeedStat = TenthsToFloatStat(BaseTenths);
	const float BaseMult = 1.f + FMath::Max(0.f, AttackSpeedStat - 1.f) * T66BaseAttackSpeedMultiplierPerPoint;
	return BaseMult * (1.f + GetStatBonusValue(ET66StatType::AllAttackSpeed) * 0.01f);
}


float UT66RunStateSubsystem::GetHeroScaleMultiplier() const
{
	// Stats Rework: upgrades flow through the unified "All Scale" stat; innate base excludes
	// permanent-buff contribution to avoid double counting.
	const int32 BaseTenths = FMath::Max(0, GetPreciseBaseStatTenths(ET66HeroStatType::AttackScale) - GetPermanentBaseBuffTenths(ET66HeroStatType::AttackScale));
	const float ScaleStat = TenthsToFloatStat(BaseTenths);
	const float BaseMult = 1.f + FMath::Max(0.f, ScaleStat - 1.f) * T66BaseScaleMultiplierPerPoint;
	return BaseMult * (1.f + GetStatBonusValue(ET66StatType::AllScale) * 0.01f);
}


float UT66RunStateSubsystem::GetHeroAccuracyMultiplier() const
{
	// Stats Rework: surgery Accuracy now flows through the unified Accuracy (to-hit) stat, so the
	// innate accuracy multiplier excludes permanent-buff contribution to avoid double counting.
	const int32 BaseTenths = FMath::Max(0, GetPreciseBaseStatTenths(ET66HeroStatType::Accuracy) - GetPermanentBaseBuffTenths(ET66HeroStatType::Accuracy));
	const float AccuracyStat = TenthsToFloatStat(BaseTenths);
	return 1.f + FMath::Max(0.f, AccuracyStat - 1.f) * T66BaseAccuracyMultiplierPerPoint;
}


float UT66RunStateSubsystem::GetArmorReduction01() const
{
	// Stats Rework: surgery power flows through the unified Damage Reduction stat, so the innate
	// armor base excludes permanent-buff contribution; upgrades add as percent points.
	const int32 BaseTenths = FMath::Max(0, GetPreciseBaseStatTenths(ET66HeroStatType::Armor) - GetPermanentBaseBuffTenths(ET66HeroStatType::Armor));
	const float ArmorStat = TenthsToFloatStat(BaseTenths);
	const float Base = FMath::Max(0.f, ArmorStat - 1.f) * T66BaseArmorReductionPerPoint;
	const float Upgrades = GetStatBonusValue(ET66StatType::DamageReduction) * T66StatSmallChancePerBonusPoint;
	return FMath::Clamp(Base + Upgrades + ItemArmorBonus01, 0.f, 0.80f);
}


float UT66RunStateSubsystem::GetEvasionChance01() const
{
	// Stats Rework: surgery power flows through the unified Dodge (EvasionChance) stat, so the
	// innate evasion base excludes permanent-buff contribution; upgrades add as percent points.
	const int32 BaseTenths = FMath::Max(0, GetPreciseBaseStatTenths(ET66HeroStatType::Evasion) - GetPermanentBaseBuffTenths(ET66HeroStatType::Evasion));
	const float EvasionStat = TenthsToFloatStat(BaseTenths);
	const float Base = FMath::Max(0.f, EvasionStat - 1.f) * T66BaseEvasionChancePerPoint;
	const float Upgrades = GetStatBonusValue(ET66StatType::EvasionChance) * T66StatSmallChancePerBonusPoint;
	return FMath::Clamp(Base + Upgrades + ItemEvasionBonus01, 0.f, 0.60f);
}


float UT66RunStateSubsystem::GetAccuracyChance01() const
{
	const float PassiveBonus = (PassiveType == ET66PassiveType::Headshot) ? 0.20f : 0.f;
	return FMath::Clamp(GetStatValue(ET66StatType::Accuracy) + PassiveBonus, 0.f, 0.95f);
}


void UT66RunStateSubsystem::AddHeroXP(int32 Amount)
{
	if (Amount <= 0 || bSuppressLevelUpWaveXP || HeroLevel >= MaxHeroLevel)
	{
		return;
	}

	HeroXP = FMath::Clamp(HeroXP + Amount, 0, 2000000000);
	XPToNextLevel = (XPToNextLevel > 0) ? XPToNextLevel : GetDataDrivenLevelUpXPThreshold();

	if (bProcessingHeroLevelUps)
	{
		HeroProgressChanged.Broadcast();
		return;
	}

	bProcessingHeroLevelUps = true;
	while (HeroLevel < MaxHeroLevel && XPToNextLevel > 0 && HeroXP >= XPToNextLevel)
	{
		HeroXP = FMath::Max(0, HeroXP - XPToNextLevel);
		ApplyOneHeroLevelUp();
		XPToNextLevel = GetDataDrivenLevelUpXPThreshold();
	}
	bProcessingHeroLevelUps = false;

	if (HeroLevel >= MaxHeroLevel)
	{
		HeroXP = 0;
		XPToNextLevel = GetDataDrivenLevelUpXPThreshold();
	}

	HeroProgressChanged.Broadcast();
}
