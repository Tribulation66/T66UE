// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "UI/T66TooltipTypes.h"

class UT66LocalizationSubsystem;

namespace T66TooltipResolvers
{
	T66_API FT66TooltipPayload MakeRichTooltip(FName TooltipId, ET66TooltipKind Kind, const FText& Title, const FText& Body, FName SourceTag = NAME_None);
	T66_API FT66TooltipPayload MakeBaseStatTooltip(const UT66LocalizationSubsystem* Loc, int32 StatIndex, const FText& Title, FName SourceTag = NAME_None);
	T66_API FT66TooltipPayload MakeStatTooltip(const UT66LocalizationSubsystem* Loc, ET66StatType StatType, FName SourceTag = NAME_None);
	T66_API FT66TooltipPayload MakeIdolTooltip(const UT66LocalizationSubsystem* Loc, FName IdolID, FName SourceTag = NAME_None);
	T66_API FT66TooltipPayload MakeItemTooltip(const UT66LocalizationSubsystem* Loc, const FItemData& ItemData, const FT66InventorySlot& Slot, int32 StackCount = 0, int32 SellGold = 0, FName SourceTag = NAME_None);
	T66_API FT66TooltipPayload MakeMobLootTooltip(int32 StackCount, int32 SellGold, FName SourceTag = NAME_None);
	T66_API FT66TooltipPayload MakePowerUpTooltip(FName TooltipId, const FText& Title, const FText& Effect, int32 Cost, bool bOwned, bool bEquipped, bool bBlocked, FName SourceTag = NAME_None);
	T66_API FT66TooltipPayload MakeVendorActionTooltip(FName TooltipId, const FText& Title, const FText& Body, int32 GoldValue = 0, bool bDisabled = false, FName SourceTag = NAME_None);
}
