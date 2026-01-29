// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66SaveSubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Kismet/GameplayStatics.h"

const FString UT66SaveSubsystem::SaveIndexSlotName(TEXT("T66_SaveIndex"));
const FString UT66SaveSubsystem::SlotNamePrefix(TEXT("T66_Slot_"));

FString UT66SaveSubsystem::GetSlotName(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots) return FString();
	return FString::Printf(TEXT("%s%02d"), *SlotNamePrefix, SlotIndex);
}

int32 UT66SaveSubsystem::FindFirstEmptySlot() const
{
	UT66SaveIndex* Index = LoadOrCreateIndex();
	if (!Index) return INDEX_NONE;

	for (int32 i = 0; i < MaxSlots; ++i)
	{
		if (Index->SlotMeta.Num() <= i || !Index->SlotMeta[i].bOccupied)
		{
			// Also verify no file exists for this slot (in case index was lost)
			if (!UGameplayStatics::DoesSaveGameExist(GetSlotName(i), 0))
			{
				return i;
			}
		}
	}
	return INDEX_NONE;
}

int32 UT66SaveSubsystem::FindOldestOccupiedSlot() const
{
	UT66SaveIndex* Index = LoadOrCreateIndex();
	if (!Index) return INDEX_NONE;

	int32 BestSlot = INDEX_NONE;
	FDateTime BestTime = FDateTime::MaxValue();

	for (int32 i = 0; i < MaxSlots; ++i)
	{
		if (!UGameplayStatics::DoesSaveGameExist(GetSlotName(i), 0))
		{
			continue;
		}

		FDateTime Parsed = FDateTime::MinValue();
		if (Index->SlotMeta.Num() > i)
		{
			const FString& Utc = Index->SlotMeta[i].LastPlayedUtc;
			if (!Utc.IsEmpty())
			{
				FDateTime::ParseIso8601(*Utc, Parsed);
			}
		}

		if (BestSlot == INDEX_NONE || Parsed < BestTime)
		{
			BestSlot = i;
			BestTime = Parsed;
		}
	}

	// If index metadata was empty, just fall back to slot 0 if it exists.
	if (BestSlot == INDEX_NONE)
	{
		for (int32 i = 0; i < MaxSlots; ++i)
		{
			if (UGameplayStatics::DoesSaveGameExist(GetSlotName(i), 0))
			{
				return i;
			}
		}
	}
	return BestSlot;
}

bool UT66SaveSubsystem::DoesSlotExist(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots) return false;
	return UGameplayStatics::DoesSaveGameExist(GetSlotName(SlotIndex), 0);
}

bool UT66SaveSubsystem::SaveToSlot(int32 SlotIndex, UT66RunSaveGame* SaveGameObject)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots || !SaveGameObject) return false;

	FString SlotName = GetSlotName(SlotIndex);
	if (!UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, 0))
	{
		return false;
	}

	UpdateIndexOnSave(SlotIndex, SaveGameObject->HeroID.ToString(), SaveGameObject->MapName, SaveGameObject->LastPlayedUtc);
	return true;
}

UT66RunSaveGame* UT66SaveSubsystem::LoadFromSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots) return nullptr;

	FString SlotName = GetSlotName(SlotIndex);
	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
	return Cast<UT66RunSaveGame>(Loaded);
}

void UT66SaveSubsystem::UpdateIndexOnSave(int32 SlotIndex, const FString& HeroDisplayName, const FString& MapName, const FString& LastPlayedUtc)
{
	UT66SaveIndex* Index = LoadOrCreateIndex();
	if (!Index) return;

	while (Index->SlotMeta.Num() <= SlotIndex)
	{
		FT66SaveSlotMeta Meta;
		Index->SlotMeta.Add(Meta);
	}

	Index->SlotMeta[SlotIndex].bOccupied = true;
	Index->SlotMeta[SlotIndex].LastPlayedUtc = LastPlayedUtc;
	Index->SlotMeta[SlotIndex].HeroDisplayName = HeroDisplayName;
	Index->SlotMeta[SlotIndex].MapName = MapName;
	SaveIndex(Index);
}

bool UT66SaveSubsystem::GetSlotMeta(int32 SlotIndex, bool& bOutOccupied, FString& OutLastPlayedUtc, FString& OutHeroDisplayName, FString& OutMapName) const
{
	bOutOccupied = false;
	OutLastPlayedUtc = OutHeroDisplayName = OutMapName = FString();
	if (SlotIndex < 0 || SlotIndex >= MaxSlots) return false;

	UT66SaveIndex* Index = LoadOrCreateIndex();
	if (!Index || Index->SlotMeta.Num() <= SlotIndex) return true;

	const FT66SaveSlotMeta& Meta = Index->SlotMeta[SlotIndex];
	bOutOccupied = Meta.bOccupied && UGameplayStatics::DoesSaveGameExist(GetSlotName(SlotIndex), 0);
	OutLastPlayedUtc = Meta.LastPlayedUtc;
	OutHeroDisplayName = Meta.HeroDisplayName;
	OutMapName = Meta.MapName;
	return true;
}

UT66SaveIndex* UT66SaveSubsystem::LoadOrCreateIndex() const
{
	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SaveIndexSlotName, 0);
	UT66SaveIndex* Index = Cast<UT66SaveIndex>(Loaded);
	if (!Index)
	{
		Index = NewObject<UT66SaveIndex>(const_cast<UT66SaveSubsystem*>(this));
		if (Index)
		{
			Index->SlotMeta.SetNum(MaxSlots);
		}
	}
	return Index;
}

bool UT66SaveSubsystem::SaveIndex(UT66SaveIndex* Index) const
{
	if (!Index) return false;
	return UGameplayStatics::SaveGameToSlot(Index, SaveIndexSlotName, 0);
}
