// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66TooltipResolvers.h"

#include "Core/T66LocalizationSubsystem.h"
#include "UI/T66ItemCardTextUtils.h"

namespace
{
	FText BaseStatName(const UT66LocalizationSubsystem* Loc, const ET66HeroStatType StatType)
	{
		return T66ItemCardTextUtils::GetBaseStatLabel(Loc, StatType);
	}

	FText BaseStatDescription(const UT66LocalizationSubsystem* Loc, const ET66HeroStatType StatType)
	{
		if (!Loc)
		{
			return FText::GetEmpty();
		}

		switch (StatType)
		{
		case ET66HeroStatType::Damage:      return Loc->GetText_BaseStatDescription(1);
		case ET66HeroStatType::AttackSpeed: return Loc->GetText_BaseStatDescription(2);
		case ET66HeroStatType::AttackScale: return Loc->GetText_BaseStatDescription(3);
		case ET66HeroStatType::Accuracy:    return Loc->GetText_BaseStatDescription(4);
		case ET66HeroStatType::Armor:       return Loc->GetText_BaseStatDescription(5);
		case ET66HeroStatType::Evasion:     return Loc->GetText_BaseStatDescription(6);
		case ET66HeroStatType::Luck:        return Loc->GetText_BaseStatDescription(7);
		case ET66HeroStatType::Speed:       return Loc->GetText_BaseStatDescription(8);
		case ET66HeroStatType::Special:
		default:                            return FText::GetEmpty();
		}
	}

	FName MakeGeneratedTooltipId(const TCHAR* Prefix, const FName Id)
	{
		return Id.IsNone()
			? FName(Prefix)
			: FName(*(FString(Prefix) + TEXT(".") + Id.ToString()));
	}

	void AddRow(TArray<FT66TooltipRow>& Rows, const FText& Label, const FText& Value, const FText& Description = FText::GetEmpty())
	{
		FT66TooltipRow Row;
		Row.Label = Label;
		Row.Value = Value;
		Row.Description = Description;
		Rows.Add(Row);
	}
}

FT66TooltipPayload T66TooltipResolvers::MakeRichTooltip(const FName TooltipId, const ET66TooltipKind Kind, const FText& Title, const FText& Body, const FName SourceTag)
{
	FT66TooltipPayload Payload;
	Payload.TooltipId = TooltipId;
	Payload.Kind = Kind;
	Payload.Title = Title;
	Payload.Body = Body;
	Payload.SourceTag = SourceTag;
	return Payload;
}

FT66TooltipPayload T66TooltipResolvers::MakeBaseStatTooltip(const UT66LocalizationSubsystem* Loc, const int32 StatIndex, const FText& Title, const FName SourceTag)
{
	FT66TooltipPayload Payload;
	Payload.TooltipId = FName(*FString::Printf(TEXT("Stat.Primary.%d"), StatIndex));
	Payload.Kind = ET66TooltipKind::Stat;
	Payload.Title = Title;
	Payload.Body = Loc ? Loc->GetText_BaseStatDescription(StatIndex) : FText::GetEmpty();
	Payload.SourceTag = SourceTag;
	return Payload;
}

FT66TooltipPayload T66TooltipResolvers::MakeStatTooltip(const UT66LocalizationSubsystem* Loc, const ET66StatType StatType, const FName SourceTag)
{
	FT66TooltipPayload Payload;
	Payload.TooltipId = FName(*FString::Printf(TEXT("Stat.Secondary.%d"), static_cast<int32>(StatType)));
	Payload.Kind = ET66TooltipKind::Stat;
	Payload.Title = Loc ? Loc->GetText_StatName(StatType) : StaticEnum<ET66StatType>()->GetDisplayNameTextByValue(static_cast<int64>(StatType));
	Payload.Body = Loc ? Loc->GetText_StatDescription(StatType) : FText::GetEmpty();
	Payload.EntityId = FName(*FString::Printf(TEXT("%d"), static_cast<int32>(StatType)));
	Payload.SourceTag = SourceTag;
	return Payload;
}

FT66TooltipPayload T66TooltipResolvers::MakeIdolTooltip(const UT66LocalizationSubsystem* Loc, const FName IdolID, const FName SourceTag)
{
	FT66TooltipPayload Payload;
	Payload.TooltipId = MakeGeneratedTooltipId(TEXT("Idol"), IdolID);
	Payload.Kind = ET66TooltipKind::Idol;
	Payload.Title = Loc ? Loc->GetText_IdolDisplayName(IdolID) : FText::FromName(IdolID);
	Payload.Body = Loc ? Loc->GetText_IdolTooltip(IdolID) : FText::GetEmpty();
	Payload.EntityId = IdolID;
	Payload.SourceTag = SourceTag;
	return Payload;
}

FT66TooltipPayload T66TooltipResolvers::MakeItemTooltip(
	const UT66LocalizationSubsystem* Loc,
	const FItemData& ItemData,
	const FT66InventorySlot& Slot,
	const int32 StackCount,
	const int32 SellGold,
	const FName SourceTag)
{
	FT66TooltipPayload Payload;
	Payload.TooltipId = MakeGeneratedTooltipId(TEXT("Item"), Slot.ItemTemplateID.IsNone() ? ItemData.ItemID : Slot.ItemTemplateID);
	Payload.Kind = ET66TooltipKind::Item;
	Payload.EntityId = Slot.ItemTemplateID.IsNone() ? ItemData.ItemID : Slot.ItemTemplateID;
	Payload.Rarity = Slot.Rarity;
	Payload.SourceTag = SourceTag;
	Payload.bDynamic = true;
	Payload.Title = Loc
		? Loc->GetText_ItemDisplayNameForRarity(Payload.EntityId, Slot.Rarity)
		: FText::FromName(Payload.EntityId);
	Payload.Subtitle = Loc ? Loc->GetText_ItemRarityName(Slot.Rarity) : StaticEnum<ET66ItemRarity>()->GetDisplayNameTextByValue(static_cast<int64>(Slot.Rarity));

	const FText PrimaryLabel = BaseStatName(Loc, ItemData.BaseStatType);
	if (ItemData.BaseStatType != ET66HeroStatType::Special)
	{
		AddRow(
			Payload.Rows,
			NSLOCTEXT("T66.Tooltip", "ItemPrimaryLine", "Primary"),
			FText::Format(NSLOCTEXT("T66.Tooltip", "ItemPrimaryValue", "+{0} {1}"), FText::AsNumber(Slot.Line1RolledValue), PrimaryLabel),
			BaseStatDescription(Loc, ItemData.BaseStatType));
	}

	const FText CardDescription = T66ItemCardTextUtils::BuildItemCardDescription(Loc, ItemData, Slot.Rarity, Slot.Line1RolledValue, 1.f, Slot.GetLine2Multiplier());
	if (!CardDescription.IsEmpty())
	{
		AddRow(
			Payload.Rows,
			NSLOCTEXT("T66.Tooltip", "ItemSecondaryLine", "Secondary"),
			CardDescription,
			Loc ? Loc->GetText_StatDescription(ItemData.StatType) : FText::GetEmpty());
	}

	if (StackCount > 1)
	{
		AddRow(Payload.Rows, NSLOCTEXT("T66.Tooltip", "ItemStack", "Stack"), FText::AsNumber(StackCount));
	}

	if (SellGold > 0)
	{
		AddRow(
			Payload.Rows,
			NSLOCTEXT("T66.Tooltip", "ItemSellValue", "Sell"),
			FText::Format(NSLOCTEXT("T66.Tooltip", "GoldValueFormat", "{0} gold"), FText::AsNumber(SellGold)));
	}

	return Payload;
}

FT66TooltipPayload T66TooltipResolvers::MakeMobLootTooltip(const int32 StackCount, const int32 SellGold, const FName SourceTag)
{
	FT66TooltipPayload Payload;
	Payload.TooltipId = FName(TEXT("Item.MobLoot"));
	Payload.Kind = ET66TooltipKind::Item;
	Payload.Title = NSLOCTEXT("T66.ItemTooltip", "MobLootTitle", "Mob Loot");
	Payload.Body = NSLOCTEXT("T66.ItemTooltip", "MobLootBody", "Sell-only monster loot collected during the run.");
	Payload.SourceTag = SourceTag;
	Payload.bDynamic = true;
	AddRow(Payload.Rows, NSLOCTEXT("T66.Tooltip", "Stack", "Stack"), FText::AsNumber(FMath::Max(0, StackCount)));
	AddRow(Payload.Rows, NSLOCTEXT("T66.Tooltip", "Sell", "Sell"), FText::Format(NSLOCTEXT("T66.Tooltip", "GoldValueFormat", "{0} gold"), FText::AsNumber(FMath::Max(0, SellGold))));
	return Payload;
}

FT66TooltipPayload T66TooltipResolvers::MakePowerUpTooltip(
	const FName TooltipId,
	const FText& Title,
	const FText& Effect,
	const int32 Cost,
	const bool bOwned,
	const bool bEquipped,
	const bool bBlocked,
	const FName SourceTag)
{
	FT66TooltipPayload Payload;
	Payload.TooltipId = TooltipId;
	Payload.Kind = bEquipped ? ET66TooltipKind::TemporaryBuff : ET66TooltipKind::PowerUp;
	Payload.Title = Title;
	Payload.Body = Effect;
	Payload.SourceTag = SourceTag;
	Payload.bDynamic = true;

	if (Cost > 0 && !bOwned && !bEquipped)
	{
		AddRow(Payload.Rows, NSLOCTEXT("T66.Tooltip", "PowerUpCost", "Cost"), FText::AsNumber(Cost));
	}

	AddRow(
		Payload.Rows,
		NSLOCTEXT("T66.Tooltip", "PowerUpState", "State"),
		bEquipped
			? NSLOCTEXT("T66.Tooltip", "StateEquipped", "Equipped")
			: (bOwned ? NSLOCTEXT("T66.Tooltip", "StateOwned", "Owned") : NSLOCTEXT("T66.Tooltip", "StateAvailable", "Available")));

	if (bBlocked)
	{
		Payload.Warnings.Add(NSLOCTEXT("T66.Tooltip", "PowerUpBlocked", "This option is currently unavailable."));
	}

	return Payload;
}

FT66TooltipPayload T66TooltipResolvers::MakeVendorActionTooltip(
	const FName TooltipId,
	const FText& Title,
	const FText& Body,
	const int32 GoldValue,
	const bool bDisabled,
	const FName SourceTag)
{
	FT66TooltipPayload Payload;
	Payload.TooltipId = TooltipId;
	Payload.Kind = ET66TooltipKind::VendorOffer;
	Payload.Title = Title;
	Payload.Body = Body;
	Payload.SourceTag = SourceTag;
	Payload.bDynamic = true;
	if (GoldValue > 0)
	{
		AddRow(Payload.Rows, NSLOCTEXT("T66.Tooltip", "VendorGold", "Gold"), FText::AsNumber(GoldValue));
	}
	if (bDisabled)
	{
		Payload.Warnings.Add(NSLOCTEXT("T66.Tooltip", "VendorUnavailable", "Unavailable in the current shop state."));
	}
	return Payload;
}
