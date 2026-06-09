// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66ItemCardTextUtils.h"

#include "Core/T66LocalizationSubsystem.h"

namespace T66ItemCardTextUtils
{
	FText GetBaseStatLabel(const UT66LocalizationSubsystem* Loc, ET66HeroStatType Type)
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
			case ET66HeroStatType::Special:     return NSLOCTEXT("T66.Stats", "Special", "Special");
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
		case ET66HeroStatType::Special:     return NSLOCTEXT("T66.Stats", "Special", "Special");
		default:                            return FText::GetEmpty();
		}
	}

	static FText BuildStatLine(
		const UT66LocalizationSubsystem* Loc,
		const FItemData& ItemData,
		ET66ItemRarity ItemRarity,
		int32 MainValue,
		float Line2MultiplierOverride)
	{
		if (ItemData.StatType == ET66StatType::VendorToken)
		{
			const int32 TokenStacks = FMath::Clamp(MainValue, 1, 16);
			const int32 SellPercent = FMath::Min(100, 70 + FMath::RoundToInt(static_cast<float>(TokenStacks) * 2.5f));
			const int32 BuyDiscountTenths = TokenStacks * 25;
			const FNumberFormattingOptions BuyDiscountFormatting = []()
			{
				FNumberFormattingOptions Options;
				Options.MinimumFractionalDigits = 0;
				Options.MaximumFractionalDigits = 1;
				return Options;
			}();
			return FText::Format(
				NSLOCTEXT("T66.ItemTooltip", "VendorTokenStackLineFormat", "{0} stacks: sell items for {1}% of buy value, buy discount +{2}%."),
				FText::AsNumber(TokenStacks),
				FText::AsNumber(SellPercent),
				FText::AsNumber(static_cast<float>(BuyDiscountTenths) / 10.f, &BuyDiscountFormatting));
		}

		if (!Loc || ItemData.StatType == ET66StatType::None)
		{
			return FText::GetEmpty();
		}

		const FText Label = Loc->GetText_StatName(ItemData.StatType);
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
		static_cast<void>(CurrentHeroScaleMultiplier);

		if (ItemData.StatType == ET66StatType::VendorToken)
		{
			return BuildStatLine(Loc, ItemData, ItemRarity, MainValue, Line2MultiplierOverride);
		}

		if (ItemData.BaseStatType == ET66HeroStatType::Special)
		{
			const FText CategoryLine = GetBaseStatLabel(Loc, ItemData.BaseStatType);
			const FText Line2 = BuildStatLine(Loc, ItemData, ItemRarity, MainValue, Line2MultiplierOverride);
			return !Line2.IsEmpty()
				? FText::Format(NSLOCTEXT("T66.Shop", "SpecialTwoLineDesc", "{0}\n{1}"), CategoryLine, Line2)
			: CategoryLine;
		}

		return BuildStatLine(Loc, ItemData, ItemRarity, MainValue, Line2MultiplierOverride);
	}
}
