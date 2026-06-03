// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66IdolManagerSubsystem.h"

#include "Core/T66GameInstance.h"
#include "Core/T66DifficultyTuningSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66SaveMigration.h"

void UT66IdolManagerSubsystem::NormalizeEquippedArrays()
{
	if (EquippedIdolIDs.Num() < MaxEquippedIdolSlots)
	{
		const int32 OldNum = EquippedIdolIDs.Num();
		EquippedIdolIDs.SetNum(MaxEquippedIdolSlots);
		for (int32 Index = OldNum; Index < EquippedIdolIDs.Num(); ++Index)
		{
			EquippedIdolIDs[Index] = NAME_None;
		}
	}
	else if (EquippedIdolIDs.Num() > MaxEquippedIdolSlots)
	{
		EquippedIdolIDs.SetNum(MaxEquippedIdolSlots);
	}

	if (EquippedIdolLevels.Num() < MaxEquippedIdolSlots)
	{
		const int32 OldNum = EquippedIdolLevels.Num();
		EquippedIdolLevels.SetNum(MaxEquippedIdolSlots);
		for (int32 Index = OldNum; Index < EquippedIdolLevels.Num(); ++Index)
		{
			EquippedIdolLevels[Index] = 0;
		}
	}
	else if (EquippedIdolLevels.Num() > MaxEquippedIdolSlots)
	{
		EquippedIdolLevels.SetNum(MaxEquippedIdolSlots);
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
	const UT66DifficultyTuningSubsystem* DifficultyTuning = GI ? GI->GetSubsystem<UT66DifficultyTuningSubsystem>() : nullptr;
	if (DifficultyTuning)
	{
		return FMath::Clamp(DifficultyTuning->GetDifficultyStartStage(Difficulty), 1, 20);
	}

	switch (Difficulty)
	{
	case ET66Difficulty::Easy: return 1;
	case ET66Difficulty::Medium: return 5;
	case ET66Difficulty::Hard: return 9;
	case ET66Difficulty::VeryHard: return 13;
	case ET66Difficulty::Impossible: return 17;
	default: return 1;
	}
}

int32 UT66IdolManagerSubsystem::GetDifficultyEndStage(const ET66Difficulty Difficulty) const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66DifficultyTuningSubsystem* DifficultyTuning = GI ? GI->GetSubsystem<UT66DifficultyTuningSubsystem>() : nullptr;
	if (DifficultyTuning)
	{
		return FMath::Clamp(DifficultyTuning->GetDifficultyEndStage(Difficulty), 1, 20);
	}

	switch (Difficulty)
	{
	case ET66Difficulty::Easy: return 4;
	case ET66Difficulty::Medium: return 8;
	case ET66Difficulty::Hard: return 12;
	case ET66Difficulty::VeryHard: return 16;
	case ET66Difficulty::Impossible: return 20;
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

FName UT66IdolManagerSubsystem::NormalizeLegacyIdolID(FName IdolID)
{
	return T66MigrateLegacyIdolID(IdolID);
}

ET66ItemRarity UT66IdolManagerSubsystem::GetEquippedIdolRarityInSlot(const int32 SlotIndex) const
{
	return IdolTierValueToRarity(GetEquippedIdolLevelInSlot(SlotIndex));
}

const TArray<FName>& UT66IdolManagerSubsystem::GetAllIdolIDs()
{
	static const TArray<FName> Idols = {
		FName(TEXT("Idol_Fire_DOT")),
		FName(TEXT("Idol_Fire_AOE")),
		FName(TEXT("Idol_Fire_Pierce")),
		FName(TEXT("Idol_Fire_Bounce")),
		FName(TEXT("Idol_Ice_DOT")),
		FName(TEXT("Idol_Ice_AOE")),
		FName(TEXT("Idol_Ice_Pierce")),
		FName(TEXT("Idol_Ice_Bounce")),
		FName(TEXT("Idol_Electricity_DOT")),
		FName(TEXT("Idol_Electricity_AOE")),
		FName(TEXT("Idol_Electricity_Pierce")),
		FName(TEXT("Idol_Electricity_Bounce")),
		FName(TEXT("Idol_Nature_DOT")),
		FName(TEXT("Idol_Nature_AOE")),
		FName(TEXT("Idol_Nature_Pierce")),
		FName(TEXT("Idol_Nature_Bounce")),
	};
	return Idols;
}

FLinearColor UT66IdolManagerSubsystem::GetIdolColor(FName IdolID)
{
	IdolID = NormalizeLegacyIdolID(IdolID);

	const FString IdolString = IdolID.ToString();
	if (IdolString.StartsWith(TEXT("Idol_Fire_")))        return FLinearColor(0.95f, 0.28f, 0.08f, 1.f);
	if (IdolString.StartsWith(TEXT("Idol_Ice_")))         return FLinearColor(0.28f, 0.70f, 1.00f, 1.f);
	if (IdolString.StartsWith(TEXT("Idol_Electricity_"))) return FLinearColor(0.62f, 0.20f, 1.00f, 1.f);
	if (IdolString.StartsWith(TEXT("Idol_Nature_")))      return FLinearColor(0.20f, 0.78f, 0.32f, 1.f);

	return FLinearColor(0.25f, 0.25f, 0.28f, 1.f);
}

bool UT66IdolManagerSubsystem::EquipIdolInSlot(const int32 SlotIndex, const FName IdolID)
{
	const FName NormalizedIdolID = NormalizeLegacyIdolID(IdolID);
	if (SlotIndex < 0 || SlotIndex >= MaxEquippedIdolSlots || NormalizedIdolID.IsNone()) return false;

	NormalizeEquippedArrays();
	if (EquippedIdolIDs[SlotIndex] == NormalizedIdolID) return false;

	EquippedIdolIDs[SlotIndex] = NormalizedIdolID;
	EquippedIdolLevels[SlotIndex] = 1;
	BroadcastIdolStateChanged();
	return true;
}

bool UT66IdolManagerSubsystem::EquipIdolFirstEmpty(const FName IdolID)
{
	const FName NormalizedIdolID = NormalizeLegacyIdolID(IdolID);
	if (NormalizedIdolID.IsNone()) return false;

	NormalizeEquippedArrays();
	for (int32 Index = 0; Index < EquippedIdolIDs.Num(); ++Index)
	{
		if (!EquippedIdolIDs[Index].IsNone()) continue;

		EquippedIdolIDs[Index] = NormalizedIdolID;
		EquippedIdolLevels[Index] = 1;
		BroadcastIdolStateChanged();
		return true;
	}

	return false;
}

bool UT66IdolManagerSubsystem::SelectIdolFromAltar(const FName IdolID)
{
	const FName NormalizedIdolID = NormalizeLegacyIdolID(IdolID);
	if (NormalizedIdolID.IsNone()) return false;

	NormalizeEquippedArrays();
	for (int32 Index = 0; Index < EquippedIdolIDs.Num(); ++Index)
	{
		if (EquippedIdolIDs[Index] != NormalizedIdolID) continue;
		return false;
	}

	for (int32 Index = 0; Index < EquippedIdolIDs.Num(); ++Index)
	{
		if (!EquippedIdolIDs[Index].IsNone()) continue;

		EquippedIdolIDs[Index] = NormalizedIdolID;
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
	const UGameInstance* GI = GetGameInstance();
	const UT66DifficultyTuningSubsystem* DifficultyTuning = GI ? GI->GetSubsystem<UT66DifficultyTuningSubsystem>() : nullptr;
	const int32 LocalStageNumber = DifficultyTuning
		? DifficultyTuning->GetDifficultyLocalStage(CurrentDifficulty, IdolStockStage)
		: FMath::Clamp(IdolStockStage - GetDifficultyStartStage(CurrentDifficulty) + 1, 1, 4);
	const ET66ItemRarity BaseRarity = DifficultyTuning
		? DifficultyTuning->GetLocalStageIdolRarity(LocalStageNumber)
		: ET66ItemRarity::Black;
	const int32 BaseTierValue = IdolRarityToTierValue(BaseRarity);

	for (const FName& IdolID : AllIdols)
	{
		bool bOwned = false;
		int32 OwnedTierValue = 0;
		for (int32 SlotIndex = 0; SlotIndex < EquippedIdolIDs.Num(); ++SlotIndex)
		{
			if (EquippedIdolIDs[SlotIndex] != IdolID) continue;

			bOwned = true;
			OwnedTierValue = EquippedIdolLevels.IsValidIndex(SlotIndex)
				? FMath::Clamp(static_cast<int32>(EquippedIdolLevels[SlotIndex]), 0, MaxIdolLevel)
				: 0;
			break;
		}

		const int32 OfferTierValue = bOwned
			? FMath::Clamp(OwnedTierValue + 1, 1, MaxIdolLevel)
			: BaseTierValue;
		IdolStockIDs.Add(IdolID);
		IdolStockTierValues.Add(static_cast<uint8>(OfferTierValue));
		IdolStockSelected.Add(bOwned && OwnedTierValue >= OfferTierValue);
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

	const FName OfferedIdolID = NormalizeLegacyIdolID(IdolStockIDs[SlotIndex]);
	const int32 OfferedTierValue = FMath::Clamp(static_cast<int32>(IdolStockTierValues[SlotIndex]), 1, MaxIdolLevel);

	bool bApplied = false;
	for (int32 Index = 0; Index < EquippedIdolIDs.Num(); ++Index)
	{
		if (EquippedIdolIDs[Index] != OfferedIdolID) continue;
		const int32 CurrentTierValue = EquippedIdolLevels.IsValidIndex(Index)
			? FMath::Clamp(static_cast<int32>(EquippedIdolLevels[Index]), 0, MaxIdolLevel)
			: 0;
		if (CurrentTierValue >= OfferedTierValue)
		{
			return false;
		}

		EquippedIdolLevels[Index] = static_cast<uint8>(OfferedTierValue);
		bApplied = true;
		break;
	}

	if (!bApplied)
	{
		for (int32 Index = 0; Index < EquippedIdolIDs.Num(); ++Index)
		{
			if (!EquippedIdolIDs[Index].IsNone()) continue;

			EquippedIdolIDs[Index] = OfferedIdolID;
			EquippedIdolLevels[Index] = static_cast<uint8>(OfferedTierValue);
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

bool UT66IdolManagerSubsystem::SelectNoIdolFromAltar(const ET66ItemRarity Rarity)
{
	if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		RunState->ApplyNoIdolSelection(Rarity);
		BroadcastIdolStateChanged();
		return true;
	}

	return false;
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
			EquippedIdolIDs[SlotIndex] = NAME_None;
			EquippedIdolLevels[SlotIndex] = 0;

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
	const ET66Difficulty Difficulty)
{
	CurrentDifficulty = Difficulty;
	EquippedIdolIDs = InEquippedIdols;
	EquippedIdolLevels = InEquippedIdolTiers;
	NormalizeEquippedArrays();
	const TArray<FName>& LiveIdolIDs = GetAllIdolIDs();
	TSet<FName> SeenIds;
	for (int32 Index = 0; Index < EquippedIdolIDs.Num(); ++Index)
	{
		FName& IdolID = EquippedIdolIDs[Index];
		IdolID = NormalizeLegacyIdolID(IdolID);
		if (IdolID.IsNone() || !LiveIdolIDs.Contains(IdolID) || SeenIds.Contains(IdolID))
		{
			IdolID = NAME_None;
			EquippedIdolLevels[Index] = 0;
			continue;
		}

		SeenIds.Add(IdolID);
		EquippedIdolLevels[Index] = static_cast<uint8>(
			FMath::Clamp(static_cast<int32>(EquippedIdolLevels[Index]), 1, MaxIdolLevel));
	}
	ClearIdolStock();
	BroadcastIdolStateChanged();
}

void UT66IdolManagerSubsystem::ResetForNewRun(const ET66Difficulty Difficulty)
{
	CurrentDifficulty = Difficulty;
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
