// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RunModifierTypes.h"

void FT66RunModifierSnapshot::Sanitize()
{
	StartRandomItems = FMath::Clamp(StartRandomItems, 0, 20);
	StartBonusGold = FMath::Clamp(StartBonusGold, 0, 10000);
	EnemyHealthMultiplier = FMath::Clamp(EnemyHealthMultiplier, 0.1f, 10.0f);
	EnemyDamageMultiplier = FMath::Clamp(EnemyDamageMultiplier, 0.1f, 10.0f);
	TrapDamageMultiplier = FMath::Clamp(TrapDamageMultiplier, 0.1f, 10.0f);
	HeroHealthMultiplier = FMath::Clamp(HeroHealthMultiplier, 0.1f, 10.0f);
	HeroDamageMultiplier = FMath::Clamp(HeroDamageMultiplier, 0.1f, 10.0f);
	HeroLuckFlat = FMath::Clamp(HeroLuckFlat, -99, 99);
	EnemyLootBagCountMultiplier = FMath::Clamp(EnemyLootBagCountMultiplier, 0.0f, 20.0f);
}

void FT66RunModifierSnapshot::Merge(const FT66RunModifierSnapshot& Other)
{
	StartRandomItems += Other.StartRandomItems;
	StartBonusGold += Other.StartBonusGold;
	EnemyHealthMultiplier *= Other.EnemyHealthMultiplier;
	EnemyDamageMultiplier *= Other.EnemyDamageMultiplier;
	TrapDamageMultiplier *= Other.TrapDamageMultiplier;
	HeroHealthMultiplier *= Other.HeroHealthMultiplier;
	HeroDamageMultiplier *= Other.HeroDamageMultiplier;
	HeroLuckFlat += Other.HeroLuckFlat;
	EnemyLootBagCountMultiplier *= Other.EnemyLootBagCountMultiplier;
	Sanitize();
}

bool FT66RunModifierSnapshot::HasAnyGameplayModifier() const
{
	return StartRandomItems > 0
		|| StartBonusGold > 0
		|| !FMath::IsNearlyEqual(EnemyHealthMultiplier, 1.0f)
		|| !FMath::IsNearlyEqual(EnemyDamageMultiplier, 1.0f)
		|| !FMath::IsNearlyEqual(TrapDamageMultiplier, 1.0f)
		|| !FMath::IsNearlyEqual(HeroHealthMultiplier, 1.0f)
		|| !FMath::IsNearlyEqual(HeroDamageMultiplier, 1.0f)
		|| HeroLuckFlat != 0
		|| !FMath::IsNearlyEqual(EnemyLootBagCountMultiplier, 1.0f);
}

FT66RunModifierSnapshot FT66RunModifierCatalog::FromDailyChallenge(const FT66DailyClimbChallengeData& Challenge)
{
	FT66RunModifierSnapshot Snapshot;
	for (const FT66DailyClimbRule& Rule : Challenge.Rules)
	{
		switch (Rule.Type)
		{
		case ET66DailyClimbRuleType::StartRandomItems:
			Snapshot.StartRandomItems += FMath::Max(0, Rule.IntValue);
			break;
		case ET66DailyClimbRuleType::StartBonusGold:
			Snapshot.StartBonusGold += FMath::Max(0, Rule.IntValue);
			break;
		case ET66DailyClimbRuleType::EnemyHpMultiplier:
			Snapshot.EnemyHealthMultiplier *= Rule.FloatValue > 0.0f ? Rule.FloatValue : 1.0f;
			break;
		case ET66DailyClimbRuleType::EnemyDamageMultiplier:
			Snapshot.EnemyDamageMultiplier *= Rule.FloatValue > 0.0f ? Rule.FloatValue : 1.0f;
			break;
		case ET66DailyClimbRuleType::TrapDamageMultiplier:
			Snapshot.TrapDamageMultiplier *= Rule.FloatValue > 0.0f ? Rule.FloatValue : 1.0f;
			break;
		case ET66DailyClimbRuleType::HeroHealthMultiplier:
			Snapshot.HeroHealthMultiplier *= Rule.FloatValue > 0.0f ? Rule.FloatValue : 1.0f;
			break;
		case ET66DailyClimbRuleType::HeroDamageMultiplier:
			Snapshot.HeroDamageMultiplier *= Rule.FloatValue > 0.0f ? Rule.FloatValue : 1.0f;
			break;
		case ET66DailyClimbRuleType::HeroLuckFlat:
			Snapshot.HeroLuckFlat += Rule.IntValue;
			break;
		case ET66DailyClimbRuleType::EnemyLootBagCountMultiplier:
			Snapshot.EnemyLootBagCountMultiplier *= Rule.FloatValue >= 0.0f ? Rule.FloatValue : 1.0f;
			break;
		default:
			break;
		}
	}
	Snapshot.Sanitize();
	return Snapshot;
}

FT66RunModifierSnapshot FT66RunModifierCatalog::FromCommunityRules(const FT66CommunityRuleSet& Rules)
{
	FT66RunModifierSnapshot Snapshot;
	Snapshot.StartRandomItems = Rules.StartRandomItems;
	Snapshot.StartBonusGold = Rules.StartBonusGold;
	Snapshot.EnemyHealthMultiplier = Rules.EnemyHealthMultiplier;
	Snapshot.EnemyDamageMultiplier = Rules.EnemyDamageMultiplier;
	Snapshot.TrapDamageMultiplier = Rules.TrapDamageMultiplier;
	Snapshot.HeroHealthMultiplier = Rules.HeroHealthMultiplier;
	Snapshot.HeroDamageMultiplier = Rules.HeroDamageMultiplier;
	Snapshot.HeroLuckFlat = Rules.HeroLuckFlat;
	Snapshot.EnemyLootBagCountMultiplier = Rules.EnemyLootBagCountMultiplier;
	Snapshot.Sanitize();
	return Snapshot;
}
