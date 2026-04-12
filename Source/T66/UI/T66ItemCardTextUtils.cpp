// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66ItemCardTextUtils.h"

#include "Core/T66LocalizationSubsystem.h"

namespace T66ItemCardTextUtils
{
	FText GetPrimaryStatLabel(const UT66LocalizationSubsystem* Loc, ET66HeroStatType Type)
	{
		if (Loc)
		{
			switch (Type)
			{
			case ET66HeroStatType::Damage:      return Loc->GetText_Stat_Damage();
			case ET66HeroStatType::AttackSpeed: return Loc->GetText_Stat_AttackSpeed();
			case ET66HeroStatType::AttackScale: return Loc->GetText_Stat_AttackScale();
			case ET66HeroStatType::Accuracy:    return Loc->GetText_Stat_Accuracy();
			case ET66HeroStatType::Armor:       return Loc->GetText_Stat_Armor();
			case ET66HeroStatType::Evasion:     return Loc->GetText_Stat_Evasion();
			case ET66HeroStatType::Luck:        return Loc->GetText_Stat_Luck();
			case ET66HeroStatType::Speed:       return Loc->GetText_Stat_Speed();
			default: break;
			}
		}

		switch (Type)
		{
		case ET66HeroStatType::Damage:      return NSLOCTEXT("T66.Stats", "Damage", "Damage");
		case ET66HeroStatType::AttackSpeed: return NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed");
		case ET66HeroStatType::AttackScale: return NSLOCTEXT("T66.Stats", "AttackScale", "Attack Scale");
		case ET66HeroStatType::Accuracy:    return NSLOCTEXT("T66.Stats", "Accuracy", "Accuracy");
		case ET66HeroStatType::Armor:       return NSLOCTEXT("T66.Stats", "Armor", "Armor");
		case ET66HeroStatType::Evasion:     return NSLOCTEXT("T66.Stats", "Evasion", "Evasion");
		case ET66HeroStatType::Luck:        return NSLOCTEXT("T66.Stats", "Luck", "Luck");
		case ET66HeroStatType::Speed:       return NSLOCTEXT("T66.Stats", "Speed", "Speed");
		default:                            return FText::GetEmpty();
		}
	}

	static FText BuildPrimaryStatLine(
		const UT66LocalizationSubsystem* Loc,
		const FItemData& ItemData,
		int32 MainValue,
		float CurrentHeroScaleMultiplier)
	{
		if (MainValue <= 0)
		{
			return FText::GetEmpty();
		}

		const FText Label = GetPrimaryStatLabel(Loc, ItemData.PrimaryStatType);
		if (ItemData.PrimaryStatType == ET66HeroStatType::AttackScale)
		{
			const FNumberFormattingOptions OneDecimal = []()
			{
				FNumberFormattingOptions Options;
				Options.MinimumFractionalDigits = 1;
				Options.MaximumFractionalDigits = 1;
				return Options;
			}();

			return FText::Format(
				NSLOCTEXT("T66.ItemTooltip", "AttackScaleLineFormat", "{0}: +{1} ({2})"),
				Label,
				FText::AsNumber(MainValue),
				FText::AsNumber(CurrentHeroScaleMultiplier, &OneDecimal));
		}

		return FText::Format(
			NSLOCTEXT("T66.ItemTooltip", "MainStatLineFormat", "{0}: +{1}"),
			Label,
			FText::AsNumber(MainValue));
	}

	static FText BuildSecondaryStatLine(
		const UT66LocalizationSubsystem* Loc,
		const FItemData& ItemData,
		ET66ItemRarity ItemRarity,
		int32 MainValue,
		float Line2MultiplierOverride)
	{
		if (ItemData.SecondaryStatType == ET66SecondaryStatType::GamblerToken)
		{
			const int32 TokenLevel = FMath::Clamp(MainValue, 1, 5);
			const int32 SellPercent = 40 + TokenLevel * 10;
			return FText::Format(
				NSLOCTEXT("T66.ItemTooltip", "GamblerTokenLineFormat", "Level {0}: sell items for {1}% of buy value."),
				FText::AsNumber(TokenLevel),
				FText::AsNumber(SellPercent));
		}

		if (!Loc || ItemData.SecondaryStatType == ET66SecondaryStatType::None)
		{
			return FText::GetEmpty();
		}

		const FText Label = Loc->GetText_SecondaryStatName(ItemData.SecondaryStatType);
		const FNumberFormattingOptions PercentFormatting = []()
		{
			FNumberFormattingOptions Options;
			Options.MinimumFractionalDigits = 0;
			Options.MaximumFractionalDigits = 1;
			return Options;
		}();

		const float EffectiveLine2Multiplier = Line2MultiplierOverride > 0.f
			? Line2MultiplierOverride
			: FItemData::GetLine2RarityMultiplier(ItemRarity);
		const float BonusRatio = FMath::Max(0.f, EffectiveLine2Multiplier - 1.f);
		const FText BonusPercentText = FText::Format(
			NSLOCTEXT("T66.ItemTooltip", "SecondaryBonusPercentFormat", "+{0}"),
			FText::AsPercent(BonusRatio, &PercentFormatting));

		const FText StatLineFormat = Loc->GetText_StatLineFormat();
		return FText::Format(StatLineFormat, Label, BonusPercentText);
	}

	FText BuildItemCardDescription(
		const UT66LocalizationSubsystem* Loc,
		const FItemData& ItemData,
		ET66ItemRarity ItemRarity,
		int32 MainValue,
		float CurrentHeroScaleMultiplier,
		float Line2MultiplierOverride)
	{
		if (ItemData.SecondaryStatType == ET66SecondaryStatType::GamblerToken)
		{
			return BuildSecondaryStatLine(Loc, ItemData, ItemRarity, MainValue, Line2MultiplierOverride);
		}

		const FText Line1 = BuildPrimaryStatLine(Loc, ItemData, MainValue, CurrentHeroScaleMultiplier);
		const FText Line2 = BuildSecondaryStatLine(Loc, ItemData, ItemRarity, MainValue, Line2MultiplierOverride);

		if (!Line1.IsEmpty() && !Line2.IsEmpty())
		{
			return FText::Format(NSLOCTEXT("T66.Vendor", "TwoLineDesc", "{0}\n{1}"), Line1, Line2);
		}

		return !Line1.IsEmpty() ? Line1 : Line2;
	}
}
