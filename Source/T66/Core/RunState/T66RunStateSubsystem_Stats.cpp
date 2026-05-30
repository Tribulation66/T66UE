// Copyright Tribulation 66. All Rights Reserved.

#include "Core/RunState/T66RunStateSubsystem_Private.h"

using namespace T66RunStatePrivate;

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
	case ET66HeroStatType::Speed:       return WholeStatToTenths(FMath::Max(0, PermanentBuffStatBonuses.Speed));
	default:                            return 0;
	}
}


int32 UT66RunStateSubsystem::GetTemporaryPrimaryStatAmplifierTenths(const ET66HeroStatType StatType) const
{
	int32 TotalTenths = 0;
	for (const FT66TemporaryPrimaryStatAmplifier& Amplifier : TemporaryPrimaryStatAmplifiers)
	{
		if (Amplifier.StatType == StatType && Amplifier.SecondsRemaining > 0.f)
		{
			TotalTenths += FMath::Max(0, Amplifier.BonusTenths);
		}
	}
	return TotalTenths;
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

	TotalTenths += GetPermanentPrimaryBuffTenths(StatType);
	return ClampHeroStatTenths(TotalTenths);
}


int32 UT66RunStateSubsystem::GetSecondaryStatBonusTenths(const ET66SecondaryStatType StatType) const
{
	const int32 ItemTenths = ItemSecondaryStatBonusTenths.Contains(StatType)
		? FMath::Max(0, ItemSecondaryStatBonusTenths.FindRef(StatType))
		: 0;
	const int32 PersistentTenths = PersistentSecondaryStatBonusTenths.Contains(StatType)
		? FMath::Max(0, PersistentSecondaryStatBonusTenths.FindRef(StatType))
		: 0;
	const int32 PermanentTenths = PermanentSecondaryStatBonusTenths.Contains(StatType)
		? FMath::Max(0, PermanentSecondaryStatBonusTenths.FindRef(StatType))
		: 0;
	return FMath::Max(0, ItemTenths + PersistentTenths + PermanentTenths);
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
	PermanentSecondaryStatBonusTenths.Reset();

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UT66BuffSubsystem* Buffs = GI->GetSubsystem<UT66BuffSubsystem>())
		{
			PermanentBuffStatBonuses = Buffs->GetPermanentBuffStatBonuses();
		}
	}

	constexpr int32 DiplomaSeedSalt = 0x5D1F0A7D;
	auto ApplyPermanentPrimary = [&](const ET66HeroStatType StatType)
	{
		const int32 GainTenths = GetPermanentPrimaryBuffTenths(StatType);
		if (GainTenths <= 0)
		{
			return;
		}

		ApplyPrimaryGainToSecondaryBonuses(
			StatType,
			GainTenths,
			PermanentSecondaryStatBonusTenths,
			HashCombine(static_cast<uint32>(DiplomaSeedSalt), GetTypeHash(static_cast<uint8>(StatType))));
	};

	ApplyPermanentPrimary(ET66HeroStatType::Damage);
	ApplyPermanentPrimary(ET66HeroStatType::AttackSpeed);
	ApplyPermanentPrimary(ET66HeroStatType::AttackScale);
	ApplyPermanentPrimary(ET66HeroStatType::Accuracy);
	ApplyPermanentPrimary(ET66HeroStatType::Armor);
	ApplyPermanentPrimary(ET66HeroStatType::Evasion);
	ApplyPermanentPrimary(ET66HeroStatType::Luck);
	ApplyPermanentPrimary(ET66HeroStatType::Speed);
}


void UT66RunStateSubsystem::ApplyOneHeroLevelUp()
{
	if (HeroLevel >= MaxHeroLevel)
	{
		return;
	}

	HeroLevel = FMath::Clamp(HeroLevel + 1, DefaultHeroLevel, MaxHeroLevel);

	auto GetPrimaryCategoryName = [](const ET66HeroStatType StatType) -> FName
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
		const int32 GainTenths = RollHeroPrimaryGainTenthsBiased(Range, GetPrimaryCategoryName(StatType));
		if (GainTenths <= 0)
		{
			return;
		}

		ApplyLevelUpPrimaryGainTenths(StatType, GainTenths);
		ApplyPrimaryGainToSecondaryBonuses(
			StatType,
			GainTenths,
			PersistentSecondaryStatBonusTenths,
			HashCombine(static_cast<uint32>(HeroLevel), GetTypeHash(static_cast<uint8>(StatType))));
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
	CurrentHP = MaxHP;
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
	ClearPersistentSecondaryStatBonuses();

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


void UT66RunStateSubsystem::ApplyLevelUpPrimaryGainTenths(const ET66HeroStatType StatType, const int32 GainTenths)
{
	const int32 ClampedGain = FMath::Max(0, GainTenths);
	if (ClampedGain <= 0)
	{
		return;
	}

	switch (StatType)
	{
	case ET66HeroStatType::Damage:      HeroPreciseStats.DamageTenths = ClampHeroStatTenths(HeroPreciseStats.DamageTenths + ClampedGain); break;
	case ET66HeroStatType::AttackSpeed: HeroPreciseStats.AttackSpeedTenths = ClampHeroStatTenths(HeroPreciseStats.AttackSpeedTenths + ClampedGain); break;
	case ET66HeroStatType::AttackScale: HeroPreciseStats.AttackScaleTenths = ClampHeroStatTenths(HeroPreciseStats.AttackScaleTenths + ClampedGain); break;
	case ET66HeroStatType::Accuracy:    HeroPreciseStats.AccuracyTenths = ClampHeroStatTenths(HeroPreciseStats.AccuracyTenths + ClampedGain); break;
	case ET66HeroStatType::Armor:       HeroPreciseStats.ArmorTenths = ClampHeroStatTenths(HeroPreciseStats.ArmorTenths + ClampedGain); break;
	case ET66HeroStatType::Evasion:     HeroPreciseStats.EvasionTenths = ClampHeroStatTenths(HeroPreciseStats.EvasionTenths + ClampedGain); break;
	case ET66HeroStatType::Luck:        HeroPreciseStats.LuckTenths = ClampHeroStatTenths(HeroPreciseStats.LuckTenths + ClampedGain); break;
	case ET66HeroStatType::Speed:       HeroPreciseStats.SpeedTenths = ClampHeroStatTenths(HeroPreciseStats.SpeedTenths + ClampedGain); break;
	default:                            break;
	}
}


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


float UT66RunStateSubsystem::GetSingleUseLuckModifierPercent() const
{
	return 0.f;
}


float UT66RunStateSubsystem::GetTotalLuckModifierPercent() const
{
	const float PrimaryLuckModifierPercent = static_cast<float>(FMath::Max(0, GetLuckStat() - 1));
	return FMath::Max(0.f, PrimaryLuckModifierPercent + GetSingleUseLuckModifierPercent());
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
	case ET66SecondaryStatType::Execute:          return FMath::Clamp((BonusPoints * 0.005f) * M, 0.f, 1.f);
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
	case ET66SecondaryStatType::LootBag:          return FMath::Max(1.f, (1.f + (BonusPoints * 0.05f)) * M);
	case ET66SecondaryStatType::LootWheel:        return FMath::Max(1.f, (1.f + (BonusPoints * 0.05f)) * M);
	case ET66SecondaryStatType::DamageReduction:  return FMath::Clamp(BaseArmorReduction + (BonusPoints * 0.005f), 0.f, 0.80f);
	case ET66SecondaryStatType::EvasionChance:    return FMath::Clamp(BaseEvasionChance + (BonusPoints * 0.005f), 0.f, 0.60f);
	case ET66SecondaryStatType::Alchemy:          return FMath::Clamp(BonusPoints * 0.01f * M, 0.f, 1.f);
	case ET66SecondaryStatType::VendorToken:      return 0.f;
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
	case ET66SecondaryStatType::Execute:         return 0.f;
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
	case ET66SecondaryStatType::LootBag:         return 1.f;
	case ET66SecondaryStatType::LootWheel:       return 1.f;
	case ET66SecondaryStatType::DamageReduction: return 0.f;
	case ET66SecondaryStatType::EvasionChance:   return 0.f;
	case ET66SecondaryStatType::Alchemy:         return 0.f;
	case ET66SecondaryStatType::VendorToken:     return 0.f;
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


float UT66RunStateSubsystem::GetLootCrateRewardMultiplier() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::LootCrate);
}


float UT66RunStateSubsystem::GetLootChestRewardMultiplier() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::TreasureChest);
}


float UT66RunStateSubsystem::GetLootBagRewardMultiplier() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::LootBag);
}


float UT66RunStateSubsystem::GetLootWheelRewardMultiplier() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::LootWheel);
}


float UT66RunStateSubsystem::GetExecuteChance01() const
{
	return GetSecondaryStatValue(ET66SecondaryStatType::Execute);
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
