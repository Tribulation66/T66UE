// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66IdolManagerSubsystem.h"

#include "Core/T66GameInstance.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66RunStateSubsystem.h"

namespace
{
	static int32 T66GetReachableStageCountBeforeDifficulty(const ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Easy: return 0;
		case ET66Difficulty::Medium: return 4;
		case ET66Difficulty::Hard: return 8;
		case ET66Difficulty::VeryHard: return 12;
		case ET66Difficulty::Impossible: return 16;
		default: return 0;
		}
	}
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
		return FMath::Clamp(PlayerExperience->GetDifficultyStartStage(Difficulty), 1, 23);
	}

	switch (Difficulty)
	{
	case ET66Difficulty::Easy: return 1;
	case ET66Difficulty::Medium: return 6;
	case ET66Difficulty::Hard: return 11;
	case ET66Difficulty::VeryHard: return 16;
	case ET66Difficulty::Impossible: return 21;
	default: return 1;
	}
}

int32 UT66IdolManagerSubsystem::GetDifficultyEndStage(const ET66Difficulty Difficulty) const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66PlayerExperienceSubSystem* PlayerExperience = GI ? GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	if (PlayerExperience)
	{
		return FMath::Clamp(PlayerExperience->GetDifficultyEndStage(Difficulty), 1, 23);
	}

	switch (Difficulty)
	{
	case ET66Difficulty::Easy: return 4;
	case ET66Difficulty::Medium: return 9;
	case ET66Difficulty::Hard: return 14;
	case ET66Difficulty::VeryHard: return 19;
	case ET66Difficulty::Impossible: return 23;
	default: return 4;
	}
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
		FName(TEXT("Idol_Light")),
		FName(TEXT("Idol_Steel")),
		FName(TEXT("Idol_Wood")),
		FName(TEXT("Idol_Bone")),
		FName(TEXT("Idol_BlackHole")),
		FName(TEXT("Idol_Earth")),
		FName(TEXT("Idol_Water")),
		FName(TEXT("Idol_Storm")),
		FName(TEXT("Idol_Electric")),
		FName(TEXT("Idol_Ice")),
		FName(TEXT("Idol_Shadow")),
		FName(TEXT("Idol_Star")),
		FName(TEXT("Idol_Lava")),
		FName(TEXT("Idol_Poison")),
		FName(TEXT("Idol_Bleed")),
		FName(TEXT("Idol_Curse")),
	};
	return Idols;
}

FLinearColor UT66IdolManagerSubsystem::GetIdolColor(FName IdolID)
{
	if (IdolID == FName(TEXT("Idol_Shock")))     IdolID = FName(TEXT("Idol_Electric"));
	if (IdolID == FName(TEXT("Idol_Silence")))   IdolID = FName(TEXT("Idol_Shadow"));
	if (IdolID == FName(TEXT("Idol_Mark")))      IdolID = FName(TEXT("Idol_Light"));
	if (IdolID == FName(TEXT("Idol_Pierce")))    IdolID = FName(TEXT("Idol_Steel"));
	if (IdolID == FName(TEXT("Idol_Knockback"))) IdolID = FName(TEXT("Idol_Wood"));
	if (IdolID == FName(TEXT("Idol_Hex")))       IdolID = FName(TEXT("Idol_Curse"));
	if (IdolID == FName(TEXT("Idol_Lifesteal"))) IdolID = FName(TEXT("Idol_Bleed"));
	if (IdolID == FName(TEXT("Idol_Lightning"))) IdolID = FName(TEXT("Idol_Electric"));
	if (IdolID == FName(TEXT("Idol_Darkness")))  IdolID = FName(TEXT("Idol_Shadow"));
	if (IdolID == FName(TEXT("Idol_Metal")))     IdolID = FName(TEXT("Idol_Steel"));
	if (IdolID == FName(TEXT("Idol_Spectral")))  IdolID = FName(TEXT("Idol_Curse"));
	if (IdolID == FName(TEXT("Idol_Frost")))     IdolID = FName(TEXT("Idol_Ice"));

	if (IdolID == FName(TEXT("Idol_Curse")))     return FLinearColor(0.50f, 0.10f, 0.60f, 1.f);
	if (IdolID == FName(TEXT("Idol_Lava")))      return FLinearColor(0.95f, 0.40f, 0.05f, 1.f);
	if (IdolID == FName(TEXT("Idol_Poison")))    return FLinearColor(0.20f, 0.85f, 0.35f, 1.f);
	if (IdolID == FName(TEXT("Idol_Bleed")))     return FLinearColor(0.80f, 0.10f, 0.10f, 1.f);
	if (IdolID == FName(TEXT("Idol_Electric")))  return FLinearColor(0.70f, 0.25f, 0.95f, 1.f);
	if (IdolID == FName(TEXT("Idol_Ice")))       return FLinearColor(0.35f, 0.75f, 1.00f, 1.f);
	if (IdolID == FName(TEXT("Idol_Shadow")))    return FLinearColor(0.10f, 0.10f, 0.12f, 1.f);
	if (IdolID == FName(TEXT("Idol_Star")))      return FLinearColor(0.95f, 0.90f, 0.50f, 1.f);
	if (IdolID == FName(TEXT("Idol_Earth")))     return FLinearColor(0.55f, 0.40f, 0.25f, 1.f);
	if (IdolID == FName(TEXT("Idol_Water")))     return FLinearColor(0.20f, 0.60f, 0.95f, 1.f);
	if (IdolID == FName(TEXT("Idol_BlackHole"))) return FLinearColor(0.15f, 0.05f, 0.25f, 1.f);
	if (IdolID == FName(TEXT("Idol_Storm")))     return FLinearColor(0.40f, 0.50f, 0.70f, 1.f);
	if (IdolID == FName(TEXT("Idol_Light")))     return FLinearColor(0.92f, 0.92f, 0.98f, 1.f);
	if (IdolID == FName(TEXT("Idol_Steel")))     return FLinearColor(0.55f, 0.55f, 0.75f, 1.f);
	if (IdolID == FName(TEXT("Idol_Wood")))      return FLinearColor(0.35f, 0.65f, 0.25f, 1.f);
	if (IdolID == FName(TEXT("Idol_Bone")))      return FLinearColor(0.90f, 0.88f, 0.80f, 1.f);

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
	IdolStockSelected.Empty(IdolStockSlotCount);
	IdolStockStage = GetCurrentStage();

	const TArray<FName>& AllIdols = GetAllIdolIDs();

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

		IdolStockIDs.Add(IdolID);
		IdolStockTierValues.Add(static_cast<uint8>(OwnedTierValue > 0
			? FMath::Clamp(OwnedTierValue + 1, 1, MaxIdolLevel)
			: 1));
		IdolStockSelected.Add(false);
	}

	while (IdolStockIDs.Num() < IdolStockSlotCount)
	{
		IdolStockIDs.Add(NAME_None);
		IdolStockTierValues.Add(0);
		IdolStockSelected.Add(false);
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

bool UT66IdolManagerSubsystem::ApplyStockOfferToEquipped(const int32 SlotIndex)
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
	}

	return bApplied;
}

bool UT66IdolManagerSubsystem::SelectIdolFromStock(const int32 SlotIndex)
{
	if (!ApplyStockOfferToEquipped(SlotIndex))
	{
		return false;
	}

	BroadcastIdolStateChanged();
	return true;
}

bool UT66IdolManagerSubsystem::IsIdolStockSlotSelected(const int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= IdolStockSlotCount) return false;
	return IdolStockSelected.IsValidIndex(SlotIndex) && IdolStockSelected[SlotIndex];
}

bool UT66IdolManagerSubsystem::SellEquippedIdolInSlot(const int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxEquippedIdolSlots)
	{
		return false;
	}

	NormalizeEquippedArrays();
	if (!EquippedIdolIDs.IsValidIndex(SlotIndex) || EquippedIdolIDs[SlotIndex].IsNone())
	{
		return false;
	}

	const FName IdolID = EquippedIdolIDs[SlotIndex];
	const int32 TierValue = EquippedIdolLevels.IsValidIndex(SlotIndex)
		? FMath::Clamp(static_cast<int32>(EquippedIdolLevels[SlotIndex]), 1, MaxIdolLevel)
		: 1;

	bool bRevertedCurrentOffer = false;
	for (int32 StockIndex = 0; StockIndex < IdolStockIDs.Num(); ++StockIndex)
	{
		if (!IdolStockIDs.IsValidIndex(StockIndex)
			|| IdolStockIDs[StockIndex] != IdolID
			|| !IdolStockSelected.IsValidIndex(StockIndex)
			|| !IdolStockSelected[StockIndex])
		{
			continue;
		}

		const int32 OfferedTierValue = IdolStockTierValues.IsValidIndex(StockIndex)
			? FMath::Clamp(static_cast<int32>(IdolStockTierValues[StockIndex]), 1, MaxIdolLevel)
			: 1;

		IdolStockSelected[StockIndex] = false;
		if (TierValue == OfferedTierValue)
		{
			if (OfferedTierValue > 1)
			{
				EquippedIdolLevels[SlotIndex] = static_cast<uint8>(OfferedTierValue - 1);
			}
			else
			{
				EquippedIdolIDs[SlotIndex] = NAME_None;
				EquippedIdolLevels[SlotIndex] = 0;
			}

			bRevertedCurrentOffer = true;
		}
		break;
	}

	if (!bRevertedCurrentOffer)
	{
		EquippedIdolIDs[SlotIndex] = NAME_None;
		EquippedIdolLevels[SlotIndex] = 0;
	}

	BroadcastIdolStateChanged();
	return true;
}

void UT66IdolManagerSubsystem::RestoreState(
	const TArray<FName>& InEquippedIdols,
	const TArray<uint8>& InEquippedIdolTiers,
	const ET66Difficulty Difficulty,
	const int32 InRemainingCatchUpIdolPicks)
{
	CurrentDifficulty = Difficulty;
	EquippedIdolIDs = InEquippedIdols;
	EquippedIdolLevels = InEquippedIdolTiers;
	NormalizeEquippedArrays();
	ClearIdolStock();
	RemainingCatchUpIdolPicks = FMath::Max(0, InRemainingCatchUpIdolPicks);
	BroadcastIdolStateChanged();
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

int32 UT66IdolManagerSubsystem::GetCatchUpIdolPickCountForDifficulty(const ET66Difficulty Difficulty) const
{
	return T66GetReachableStageCountBeforeDifficulty(Difficulty);
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

void UT66IdolManagerSubsystem::SetRemainingCatchUpIdolPicks(const int32 NewRemainingCatchUpIdolPicks)
{
	const int32 MaxCatchUpIdolPicks = GetCatchUpIdolPickCountForDifficulty(CurrentDifficulty);
	RemainingCatchUpIdolPicks = FMath::Clamp(NewRemainingCatchUpIdolPicks, 0, MaxCatchUpIdolPicks);
}
