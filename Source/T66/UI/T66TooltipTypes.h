// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"

enum class ET66TooltipKind : uint8
{
	General,
	Action,
	Stat,
	Item,
	Weapon,
	Idol,
	PowerUp,
	TemporaryBuff,
	VendorOffer,
	Economy,
	Setting,
	Leaderboard,
	Achievement,
	Party,
	Status,
	WorldInteractable,
	Warning,
};

struct T66_API FT66TooltipRow
{
	FText Label;
	FText Value;
	FText Description;

	bool IsEmpty() const
	{
		return Label.IsEmpty() && Value.IsEmpty() && Description.IsEmpty();
	}
};

struct T66_API FT66TooltipPayload
{
	FName TooltipId = NAME_None;
	ET66TooltipKind Kind = ET66TooltipKind::General;
	FText Title;
	FText Subtitle;
	FText Body;
	TArray<FT66TooltipRow> Rows;
	TArray<FText> Warnings;
	FName SourceTag = NAME_None;
	FName EntityId = NAME_None;
	ET66ItemRarity Rarity = ET66ItemRarity::Black;
	bool bPlayerSpecific = false;
	bool bDynamic = false;
	float Width = 360.f;
	int32 FontSizeAdjustment = 0;

	bool IsEmpty() const
	{
		if (!Title.IsEmpty() || !Subtitle.IsEmpty() || !Body.IsEmpty())
		{
			return false;
		}

		for (const FT66TooltipRow& Row : Rows)
		{
			if (!Row.IsEmpty())
			{
				return false;
			}
		}

		for (const FText& Warning : Warnings)
		{
			if (!Warning.IsEmpty())
			{
				return false;
			}
		}

		return true;
	}
};

T66_API FString T66TooltipKindToString(ET66TooltipKind Kind);
