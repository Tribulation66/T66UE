// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66IdolManagerSubsystem.h"

#include "Core/T66GameInstance.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66RunStateSubsystem.h"

namespace
{
	struct FIdolStockOffer
	{
		FName IdolID = NAME_None;
		uint8 TierValue = 0;
	};
}

void UT66IdolManagerSubsystem::NormalizeEquippedArrays()
{
	if (EquippedIdolIDs.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
	}
	if (EquippedIdolLevels.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolLevels.Init(0, MaxEquippedIdolSlots);
	}
}

void UT66IdolManagerSubsystem::ClearIdolStock()
{
	IdolStockIDs.Empty();
	IdolStockTierValues.Empty();
	IdolStockSelected.Empty();
	IdolStockStage = INDEX_NONE;
}

void UT66IdolManagerSubsystem::BroadcastIdolStateChanged()
{
	IdolStateChanged.Broadcast();
}

int32 UT66IdolManagerSubsystem::GetCurrentStage() const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	return RunState ? RunState->GetCurrentStage() : 1;
}

int32 UT66IdolManagerSubsystem::GetDifficultyStartStage(const ET66Difficulty Difficulty) const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66PlayerExperienceSubSystem* PlayerExperience = GI ? GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	if (PlayerExperience)
	{
		return FMath::Clamp(PlayerExperience->GetDifficultyStartStage(Difficulty), 1, 33);
	}

	return FMath::Clamp(1 + (static_cast<int32>(Difficulty) * 5), 1, 33);
}

int32 UT66IdolManagerSubsystem::GetDifficultyEndStage(const ET66Difficulty Difficulty) const
{
	return FMath::Clamp(GetDifficultyStartStage(Difficulty) + 4, 1, 33);
}

int32 UT66IdolManagerSubsystem::GetEquippedIdolLevelInSlot(const int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxEquippedIdolSlots) return 0;
	if (!EquippedIdolIDs.IsValidIndex(SlotIndex) || EquippedIdolIDs[SlotIndex].IsNone()) return 0;
	if (!EquippedIdolLevels.IsValidIndex(SlotIndex)) return 1;
	return FMath::Clamp(static_cast<int32>(EquippedIdolLevels[SlotIndex]), 1, MaxIdolLevel);
}

ET66ItemRarity UT66IdolManagerSubsystem::IdolTierValueToRarity(const int32 TierValue)
{
	switch (FMath::Clamp(TierValue, 1, MaxIdolLevel))
	{
	case 1: return ET66ItemRarity::Black;
	case 2: return ET66ItemRarity::Red;
	case 3: return ET66ItemRarity::Yellow;
	case 4: return ET66ItemRarity::White;
	default: return ET66ItemRarity::Black;
	}
}

int32 UT66IdolManagerSubsystem::IdolRarityToTierValue(const ET66ItemRarity Rarity)
{
	switch (Rarity)
	{
	case ET66ItemRarity::Black:  return 1;
	case ET66ItemRarity::Red:    return 2;
	case ET66ItemRarity::Yellow: return 3;
	case ET66ItemRarity::White:  return 4;
	default:                     return 1;
	}
}

ET66ItemRarity UT66IdolManagerSubsystem::GetEquippedIdolRarityInSlot(const int32 SlotIndex) const
{
	return IdolTierValueToRarity(GetEquippedIdolLevelInSlot(SlotIndex));
}

const TArray<FName>& UT66IdolManagerSubsystem::GetAllIdolIDs()
{
	static const TArray<FName> Idols = {
		FName(TEXT("Idol_Curse")),
		FName(TEXT("Idol_Lava")),
		FName(TEXT("Idol_Poison")),
		FName(TEXT("Idol_Decay")),
		FName(TEXT("Idol_Bleed")),
		FName(TEXT("Idol_Acid")),
		FName(TEXT("Idol_Electric")),
		FName(TEXT("Idol_Ice")),
		FName(TEXT("Idol_Sound")),
		FName(TEXT("Idol_Shadow")),
		FName(TEXT("Idol_Star")),
		FName(TEXT("Idol_Rubber")),
		FName(TEXT("Idol_Fire")),
		FName(TEXT("Idol_Earth")),
		FName(TEXT("Idol_Water")),
		FName(TEXT("Idol_Sand")),
		FName(TEXT("Idol_BlackHole")),
		FName(TEXT("Idol_Storm")),
		FName(TEXT("Idol_Light")),
		FName(TEXT("Idol_Wind")),
		FName(TEXT("Idol_Steel")),
		FName(TEXT("Idol_Wood")),
		FName(TEXT("Idol_Bone")),
		FName(TEXT("Idol_Glass")),
	};
	return Idols;
}

FLinearColor UT66IdolManagerSubsystem::GetIdolColor(FName IdolID)
{
	if (IdolID == FName(TEXT("Idol_Shock")))     IdolID = FName(TEXT("Idol_Electric"));
	if (IdolID == FName(TEXT("Idol_Silence")))   IdolID = FName(TEXT("Idol_Shadow"));
	if (IdolID == FName(TEXT("Idol_Mark")))      IdolID = FName(TEXT("Idol_Light"));
	if (IdolID == FName(TEXT("Idol_Pierce")))    IdolID = FName(TEXT("Idol_Steel"));
	if (IdolID == FName(TEXT("Idol_Split")))     IdolID = FName(TEXT("Idol_Wind"));
	if (IdolID == FName(TEXT("Idol_Knockback"))) IdolID = FName(TEXT("Idol_Wood"));
	if (IdolID == FName(TEXT("Idol_Ricochet")))  IdolID = FName(TEXT("Idol_Rubber"));
	if (IdolID == FName(TEXT("Idol_Hex")))       IdolID = FName(TEXT("Idol_Curse"));
	if (IdolID == FName(TEXT("Idol_Lifesteal"))) IdolID = FName(TEXT("Idol_Bleed"));
	if (IdolID == FName(TEXT("Idol_Lightning"))) IdolID = FName(TEXT("Idol_Electric"));
	if (IdolID == FName(TEXT("Idol_Darkness")))  IdolID = FName(TEXT("Idol_Shadow"));
	if (IdolID == FName(TEXT("Idol_Metal")))     IdolID = FName(TEXT("Idol_Steel"));
	if (IdolID == FName(TEXT("Idol_Spectral")))  IdolID = FName(TEXT("Idol_Curse"));
	if (IdolID == FName(TEXT("Idol_Frost")))     IdolID = FName(TEXT("Idol_Ice"));
	if (IdolID == FName(TEXT("Idol_Glue")))      IdolID = FName(TEXT("Idol_Decay"));

	if (IdolID == FName(TEXT("Idol_Curse")))     return FLinearColor(0.50f, 0.10f, 0.60f, 1.f);
	if (IdolID == FName(TEXT("Idol_Lava")))      return FLinearColor(0.95f, 0.40f, 0.05f, 1.f);
	if (IdolID == FName(TEXT("Idol_Poison")))    return FLinearColor(0.20f, 0.85f, 0.35f, 1.f);
	if (IdolID == FName(TEXT("Idol_Decay")))     return FLinearColor(0.45f, 0.40f, 0.20f, 1.f);
	if (IdolID == FName(TEXT("Idol_Bleed")))     return FLinearColor(0.80f, 0.10f, 0.10f, 1.f);
	if (IdolID == FName(TEXT("Idol_Acid")))      return FLinearColor(0.60f, 0.90f, 0.10f, 1.f);
	if (IdolID == FName(TEXT("Idol_Electric")))  return FLinearColor(0.70f, 0.25f, 0.95f, 1.f);
	if (IdolID == FName(TEXT("Idol_Ice")))       return FLinearColor(0.35f, 0.75f, 1.00f, 1.f);
	if (IdolID == FName(TEXT("Idol_Sound")))     return FLinearColor(0.85f, 0.75f, 0.95f, 1.f);
	if (IdolID == FName(TEXT("Idol_Shadow")))    return FLinearColor(0.10f, 0.10f, 0.12f, 1.f);
	if (IdolID == FName(TEXT("Idol_Star")))      return FLinearColor(0.95f, 0.90f, 0.50f, 1.f);
	if (IdolID == FName(TEXT("Idol_Rubber")))    return FLinearColor(0.90f, 0.55f, 0.30f, 1.f);
	if (IdolID == FName(TEXT("Idol_Fire")))      return FLinearColor(0.95f, 0.25f, 0.10f, 1.f);
	if (IdolID == FName(TEXT("Idol_Earth")))     return FLinearColor(0.55f, 0.40f, 0.25f, 1.f);
	if (IdolID == FName(TEXT("Idol_Water")))     return FLinearColor(0.20f, 0.60f, 0.95f, 1.f);
	if (IdolID == FName(TEXT("Idol_Sand")))      return FLinearColor(0.90f, 0.85f, 0.55f, 1.f);
	if (IdolID == FName(TEXT("Idol_BlackHole"))) return FLinearColor(0.15f, 0.05f, 0.25f, 1.f);
	if (IdolID == FName(TEXT("Idol_Storm")))     return FLinearColor(0.40f, 0.50f, 0.70f, 1.f);
	if (IdolID == FName(TEXT("Idol_Light")))     return FLinearColor(0.92f, 0.92f, 0.98f, 1.f);
	if (IdolID == FName(TEXT("Idol_Wind")))      return FLinearColor(0.70f, 0.90f, 0.80f, 1.f);
	if (IdolID == FName(TEXT("Idol_Steel")))     return FLinearColor(0.55f, 0.55f, 0.75f, 1.f);
	if (IdolID == FName(TEXT("Idol_Wood")))      return FLinearColor(0.35f, 0.65f, 0.25f, 1.f);
	if (IdolID == FName(TEXT("Idol_Bone")))      return FLinearColor(0.90f, 0.88f, 0.80f, 1.f);
	if (IdolID == FName(TEXT("Idol_Glass")))     return FLinearColor(0.85f, 0.92f, 0.95f, 1.f);

	return FLinearColor(0.25f, 0.25f, 0.28f, 1.f);
}

bool UT66IdolManagerSubsystem::EquipIdolInSlot(const int32 SlotIndex, const FName IdolID)
{
	if (SlotIndex < 0 || SlotIndex >= MaxEquippedIdolSlots || IdolID.IsNone()) return false;

	NormalizeEquippedArrays();
	if (EquippedIdolIDs[SlotIndex] == IdolID) return false;

	EquippedIdolIDs[SlotIndex] = IdolID;
	EquippedIdolLevels[SlotIndex] = 1;
	BroadcastIdolStateChanged();
	return true;
}

bool UT66IdolManagerSubsystem::EquipIdolFirstEmpty(const FName IdolID)
{
	if (IdolID.IsNone()) return false;

	NormalizeEquippedArrays();
	for (int32 Index = 0; Index < EquippedIdolIDs.Num(); ++Index)
	{
		if (!EquippedIdolIDs[Index].IsNone()) continue;

		EquippedIdolIDs[Index] = IdolID;
		EquippedIdolLevels[Index] = 1;
		BroadcastIdolStateChanged();
		return true;
	}

	return false;
}

bool UT66IdolManagerSubsystem::SelectIdolFromAltar(const FName IdolID)
{
	if (IdolID.IsNone()) return false;

	NormalizeEquippedArrays();
	for (int32 Index = 0; Index < EquippedIdolIDs.Num(); ++Index)
	{
		if (EquippedIdolIDs[Index] != IdolID) continue;

		const int32 CurrentTier = FMath::Clamp(static_cast<int32>(EquippedIdolLevels[Index]), 1, MaxIdolLevel);
		const int32 NextTier = FMath::Clamp(CurrentTier + 1, 1, MaxIdolLevel);
		if (NextTier == CurrentTier) return false;

		EquippedIdolLevels[Index] = static_cast<uint8>(NextTier);
		BroadcastIdolStateChanged();
		return true;
	}

	for (int32 Index = 0; Index < EquippedIdolIDs.Num(); ++Index)
	{
		if (!EquippedIdolIDs[Index].IsNone()) continue;

		EquippedIdolIDs[Index] = IdolID;
		EquippedIdolLevels[Index] = 1;
		BroadcastIdolStateChanged();
		return true;
	}

	return false;
}

void UT66IdolManagerSubsystem::ClearEquippedIdols()
{
	NormalizeEquippedArrays();

	bool bChanged = false;
	for (int32 Index = 0; Index < EquippedIdolIDs.Num(); ++Index)
	{
		if (EquippedIdolIDs[Index].IsNone() && EquippedIdolLevels[Index] == 0) continue;
		EquippedIdolIDs[Index] = NAME_None;
		EquippedIdolLevels[Index] = 0;
		bChanged = true;
	}

	if (bChanged)
	{
		BroadcastIdolStateChanged();
	}
}

void UT66IdolManagerSubsystem::EnsureIdolStock()
{
	const int32 CurrentStage = GetCurrentStage();
	if (IdolStockStage == CurrentStage
		&& IdolStockIDs.Num() == IdolStockSlotCount
		&& IdolStockTierValues.Num() == IdolStockSlotCount)
	{
		return;
	}

	RerollIdolStock();
}

void UT66IdolManagerSubsystem::RerollIdolStock()
{
	NormalizeEquippedArrays();

	IdolStockIDs.Empty(IdolStockSlotCount);
	IdolStockTierValues.Empty(IdolStockSlotCount);
	IdolStockSelected.Init(false, IdolStockSlotCount);
	IdolStockStage = GetCurrentStage();

	const TArray<FName>& AllIdols = GetAllIdolIDs();
	const bool bHasEmptySlot = EquippedIdolIDs.Contains(NAME_None);

	TArray<FIdolStockOffer> Pool;
	Pool.Reserve(AllIdols.Num());

	for (const FName& IdolID : AllIdols)
	{
		int32 OwnedTierValue = 0;
		for (int32 SlotIndex = 0; SlotIndex < EquippedIdolIDs.Num(); ++SlotIndex)
		{
			if (EquippedIdolIDs[SlotIndex] != IdolID) continue;

			OwnedTierValue = EquippedIdolLevels.IsValidIndex(SlotIndex)
				? FMath::Clamp(static_cast<int32>(EquippedIdolLevels[SlotIndex]), 1, MaxIdolLevel)
				: 1;
			break;
		}

		if (OwnedTierValue > 0)
		{
			if (OwnedTierValue < MaxIdolLevel)
			{
				FIdolStockOffer& Offer = Pool.AddDefaulted_GetRef();
				Offer.IdolID = IdolID;
				Offer.TierValue = static_cast<uint8>(OwnedTierValue + 1);
			}
			continue;
		}

		if (bHasEmptySlot)
		{
			FIdolStockOffer& Offer = Pool.AddDefaulted_GetRef();
			Offer.IdolID = IdolID;
			Offer.TierValue = 1;
		}
	}

	const int32 Count = FMath::Min(IdolStockSlotCount, Pool.Num());
	for (int32 Index = Pool.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = FMath::RandRange(0, Index);
		Pool.Swap(Index, SwapIndex);
	}

	for (int32 Index = 0; Index < Count; ++Index)
	{
		IdolStockIDs.Add(Pool[Index].IdolID);
		IdolStockTierValues.Add(Pool[Index].TierValue);
	}

	while (IdolStockIDs.Num() < IdolStockSlotCount)
	{
		IdolStockIDs.Add(NAME_None);
		IdolStockTierValues.Add(0);
	}

	BroadcastIdolStateChanged();
}

int32 UT66IdolManagerSubsystem::GetIdolStockTierValue(const int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= IdolStockSlotCount) return 0;
	if (!IdolStockTierValues.IsValidIndex(SlotIndex)) return 0;
	return FMath::Clamp(static_cast<int32>(IdolStockTierValues[SlotIndex]), 0, MaxIdolLevel);
}

ET66ItemRarity UT66IdolManagerSubsystem::GetIdolStockRarityInSlot(const int32 SlotIndex) const
{
	return IdolTierValueToRarity(FMath::Max(1, GetIdolStockTierValue(SlotIndex)));
}

bool UT66IdolManagerSubsystem::SelectIdolFromStock(const int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= IdolStockSlotCount) return false;

	NormalizeEquippedArrays();

	if (!IdolStockIDs.IsValidIndex(SlotIndex) || IdolStockIDs[SlotIndex].IsNone()) return false;
	if (!IdolStockTierValues.IsValidIndex(SlotIndex) || IdolStockTierValues[SlotIndex] <= 0) return false;
	if (IdolStockSelected.IsValidIndex(SlotIndex) && IdolStockSelected[SlotIndex]) return false;

	const FName OfferedIdolID = IdolStockIDs[SlotIndex];
	const int32 OfferedTierValue = FMath::Clamp(static_cast<int32>(IdolStockTierValues[SlotIndex]), 1, MaxIdolLevel);

	bool bApplied = false;
	for (int32 Index = 0; Index < EquippedIdolIDs.Num(); ++Index)
	{
		if (EquippedIdolIDs[Index] != OfferedIdolID) continue;

		const int32 CurrentTierValue = EquippedIdolLevels.IsValidIndex(Index)
			? FMath::Clamp(static_cast<int32>(EquippedIdolLevels[Index]), 1, MaxIdolLevel)
			: 1;
		if (OfferedTierValue != CurrentTierValue + 1)
		{
			return false;
		}

		EquippedIdolLevels[Index] = static_cast<uint8>(OfferedTierValue);
		bApplied = true;
		break;
	}

	if (!bApplied)
	{
		if (OfferedTierValue != 1)
		{
			return false;
		}

		for (int32 Index = 0; Index < EquippedIdolIDs.Num(); ++Index)
		{
			if (!EquippedIdolIDs[Index].IsNone()) continue;

			EquippedIdolIDs[Index] = OfferedIdolID;
			EquippedIdolLevels[Index] = 1;
			bApplied = true;
			break;
		}
	}

	if (bApplied && IdolStockSelected.IsValidIndex(SlotIndex))
	{
		IdolStockSelected[SlotIndex] = true;
		BroadcastIdolStateChanged();
	}

	return bApplied;
}

bool UT66IdolManagerSubsystem::IsIdolStockSlotSelected(const int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= IdolStockSlotCount) return false;
	return IdolStockSelected.IsValidIndex(SlotIndex) && IdolStockSelected[SlotIndex];
}

void UT66IdolManagerSubsystem::ResetForNewRun(const ET66Difficulty Difficulty)
{
	CurrentDifficulty = Difficulty;
	RemainingCatchUpIdolPicks = GetCatchUpIdolPickCountForDifficulty(Difficulty);
	NormalizeEquippedArrays();
	for (int32 Index = 0; Index < EquippedIdolIDs.Num(); ++Index)
	{
		EquippedIdolIDs[Index] = NAME_None;
		EquippedIdolLevels[Index] = 0;
	}
	ClearIdolStock();
	BroadcastIdolStateChanged();
}

void UT66IdolManagerSubsystem::HandleStageChanged(const int32 /*NewStage*/)
{
	ClearIdolStock();
	BroadcastIdolStateChanged();
}

bool UT66IdolManagerSubsystem::ShouldSpawnBossClearAltarForClearedStage(const int32 ClearedStage) const
{
	const bool bIsCheckpointStage = (ClearedStage > 0) && ((ClearedStage % 5) == 0);
	const bool bIsFinalSegmentStage = (ClearedStage >= 31);
	return ClearedStage > 0 && !bIsCheckpointStage && !bIsFinalSegmentStage;
}

int32 UT66IdolManagerSubsystem::GetDifficultySegmentIdolPickCount(const ET66Difficulty Difficulty) const
{
	const int32 StartStage = GetDifficultyStartStage(Difficulty);
	const int32 EndStage = GetDifficultyEndStage(Difficulty);
	return FMath::Max(0, EndStage - StartStage);
}

int32 UT66IdolManagerSubsystem::GetCatchUpIdolPickCountForDifficulty(const ET66Difficulty Difficulty) const
{
	const int32 StartStage = GetDifficultyStartStage(Difficulty);

	int32 Count = 0;
	for (int32 Stage = 1; Stage < StartStage; ++Stage)
	{
		if (ShouldSpawnBossClearAltarForClearedStage(Stage))
		{
			++Count;
		}
	}

	return Count;
}

bool UT66IdolManagerSubsystem::ConsumeCatchUpIdolPick()
{
	if (RemainingCatchUpIdolPicks <= 0)
	{
		return false;
	}

	--RemainingCatchUpIdolPicks;
	BroadcastIdolStateChanged();
	return true;
}
