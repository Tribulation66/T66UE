// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66TooltipTypes.h"

FString T66TooltipKindToString(const ET66TooltipKind Kind)
{
	switch (Kind)
	{
	case ET66TooltipKind::Action:            return TEXT("Action");
	case ET66TooltipKind::Stat:              return TEXT("Stat");
	case ET66TooltipKind::Item:              return TEXT("Item");
	case ET66TooltipKind::Weapon:            return TEXT("Weapon");
	case ET66TooltipKind::Idol:              return TEXT("Idol");
	case ET66TooltipKind::PowerUp:           return TEXT("PowerUp");
	case ET66TooltipKind::TemporaryBuff:     return TEXT("TemporaryBuff");
	case ET66TooltipKind::VendorOffer:       return TEXT("VendorOffer");
	case ET66TooltipKind::Economy:           return TEXT("Economy");
	case ET66TooltipKind::Setting:           return TEXT("Setting");
	case ET66TooltipKind::Leaderboard:       return TEXT("Leaderboard");
	case ET66TooltipKind::Achievement:       return TEXT("Achievement");
	case ET66TooltipKind::Party:             return TEXT("Party");
	case ET66TooltipKind::Status:            return TEXT("Status");
	case ET66TooltipKind::WorldInteractable: return TEXT("WorldInteractable");
	case ET66TooltipKind::Warning:           return TEXT("Warning");
	case ET66TooltipKind::General:
	default:                                 return TEXT("General");
	}
}
