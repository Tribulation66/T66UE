// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66GameInstance.h"
#include "Engine/DataTable.h"

UT66GameInstance::UT66GameInstance()
{
	// Default selections
	SelectedPartySize = ET66PartySize::Solo;
	SelectedHeroID = NAME_None;
	SelectedCompanionID = NAME_None;
	SelectedDifficulty = ET66Difficulty::Easy;
	SelectedHeroBodyType = ET66BodyType::TypeA;
	SelectedCompanionBodyType = ET66BodyType::TypeA;
}

void UT66GameInstance::Init()
{
	Super::Init();

	// Pre-load DataTables on init if paths are set
	GetHeroDataTable();
	GetCompanionDataTable();
	GetItemsDataTable();
	GetBossesDataTable();
	GetStagesDataTable();
	GetHouseNPCsDataTable();
	GetLoanSharkDataTable();
}

UDataTable* UT66GameInstance::GetHeroDataTable()
{
	if (!CachedHeroDataTable && !HeroDataTable.IsNull())
	{
		CachedHeroDataTable = HeroDataTable.LoadSynchronous();
	}
	return CachedHeroDataTable;
}

UDataTable* UT66GameInstance::GetCompanionDataTable()
{
	if (!CachedCompanionDataTable && !CompanionDataTable.IsNull())
	{
		CachedCompanionDataTable = CompanionDataTable.LoadSynchronous();
	}
	return CachedCompanionDataTable;
}

bool UT66GameInstance::GetHeroData(FName HeroID, FHeroData& OutHeroData)
{
	UDataTable* DataTable = GetHeroDataTable();
	if (!DataTable)
	{
		return false;
	}

	// Find the row by name
	FHeroData* FoundRow = DataTable->FindRow<FHeroData>(HeroID, TEXT("GetHeroData"));
	if (FoundRow)
	{
		OutHeroData = *FoundRow;
		return true;
	}
	return false;
}

bool UT66GameInstance::GetCompanionData(FName CompanionID, FCompanionData& OutCompanionData)
{
	UDataTable* DataTable = GetCompanionDataTable();
	if (!DataTable)
	{
		return false;
	}

	FCompanionData* FoundRow = DataTable->FindRow<FCompanionData>(CompanionID, TEXT("GetCompanionData"));
	if (FoundRow)
	{
		OutCompanionData = *FoundRow;
		return true;
	}
	return false;
}

UDataTable* UT66GameInstance::GetItemsDataTable()
{
	if (!CachedItemsDataTable && !ItemsDataTable.IsNull())
	{
		CachedItemsDataTable = ItemsDataTable.LoadSynchronous();
	}
	return CachedItemsDataTable;
}

void UT66GameInstance::EnsureCachedItemIDs()
{
	if (bCachedItemIDsInitialized)
	{
		return;
	}

	bCachedItemIDsInitialized = true;
	CachedItemIDs.Reset();

	if (UDataTable* ItemsDT = GetItemsDataTable())
	{
		CachedItemIDs = ItemsDT->GetRowNames();
	}

	// Fallback (keeps game functional even if DT isn't wired yet).
	if (CachedItemIDs.Num() == 0)
	{
		CachedItemIDs.Add(FName(TEXT("Item_01")));
		CachedItemIDs.Add(FName(TEXT("Item_02")));
		CachedItemIDs.Add(FName(TEXT("Item_03")));
		CachedItemIDs.Add(FName(TEXT("Item_04")));
	}
}

void UT66GameInstance::EnsureCachedItemIDsByRarity()
{
	if (bCachedItemIDsByRarityInitialized)
	{
		return;
	}
	bCachedItemIDsByRarityInitialized = true;

	CachedItemIDs_Black.Reset();
	CachedItemIDs_Red.Reset();
	CachedItemIDs_Yellow.Reset();
	CachedItemIDs_White.Reset();

	EnsureCachedItemIDs();

	for (const FName& ItemID : CachedItemIDs)
	{
		if (ItemID.IsNone()) continue;

		FItemData D;
		if (GetItemData(ItemID, D))
		{
			switch (D.ItemRarity)
			{
				case ET66ItemRarity::Black: CachedItemIDs_Black.Add(ItemID); break;
				case ET66ItemRarity::Red: CachedItemIDs_Red.Add(ItemID); break;
				case ET66ItemRarity::Yellow: CachedItemIDs_Yellow.Add(ItemID); break;
				case ET66ItemRarity::White: CachedItemIDs_White.Add(ItemID); break;
				default: break;
			}
		}
		else
		{
			// Safe fallback mapping for early project bootstraps.
			if (ItemID == TEXT("Item_01")) CachedItemIDs_Black.Add(ItemID);
			else if (ItemID == TEXT("Item_02")) CachedItemIDs_Red.Add(ItemID);
			else if (ItemID == TEXT("Item_03")) CachedItemIDs_Yellow.Add(ItemID);
			else if (ItemID == TEXT("Item_04")) CachedItemIDs_White.Add(ItemID);
		}
	}

	// If any pool is empty, fall back to "all items" so the game never stalls.
	if (CachedItemIDs_Black.Num() == 0) CachedItemIDs_Black = CachedItemIDs;
	if (CachedItemIDs_Red.Num() == 0) CachedItemIDs_Red = CachedItemIDs;
	if (CachedItemIDs_Yellow.Num() == 0) CachedItemIDs_Yellow = CachedItemIDs;
	if (CachedItemIDs_White.Num() == 0) CachedItemIDs_White = CachedItemIDs;
}

FName UT66GameInstance::GetRandomItemID()
{
	EnsureCachedItemIDs();
	if (CachedItemIDs.Num() <= 0)
	{
		return FName(TEXT("Item_01"));
	}
	return CachedItemIDs[FMath::RandRange(0, CachedItemIDs.Num() - 1)];
}

FName UT66GameInstance::GetRandomItemIDForLootRarity(ET66Rarity LootRarity)
{
	EnsureCachedItemIDsByRarity();
	const TArray<FName>* Pool = &CachedItemIDs;
	switch (LootRarity)
	{
		case ET66Rarity::Black: Pool = &CachedItemIDs_Black; break;
		case ET66Rarity::Red: Pool = &CachedItemIDs_Red; break;
		case ET66Rarity::Yellow: Pool = &CachedItemIDs_Yellow; break;
		case ET66Rarity::White: Pool = &CachedItemIDs_White; break;
		default: break;
	}

	if (!Pool || Pool->Num() <= 0)
	{
		return GetRandomItemID();
	}
	return (*Pool)[FMath::RandRange(0, Pool->Num() - 1)];
}

UDataTable* UT66GameInstance::GetBossesDataTable()
{
	if (!CachedBossesDataTable && !BossesDataTable.IsNull())
	{
		CachedBossesDataTable = BossesDataTable.LoadSynchronous();
	}
	return CachedBossesDataTable;
}

UDataTable* UT66GameInstance::GetStagesDataTable()
{
	if (!CachedStagesDataTable && !StagesDataTable.IsNull())
	{
		CachedStagesDataTable = StagesDataTable.LoadSynchronous();
	}
	return CachedStagesDataTable;
}

UDataTable* UT66GameInstance::GetHouseNPCsDataTable()
{
	if (!CachedHouseNPCsDataTable && !HouseNPCsDataTable.IsNull())
	{
		CachedHouseNPCsDataTable = HouseNPCsDataTable.LoadSynchronous();
	}
	return CachedHouseNPCsDataTable;
}

UDataTable* UT66GameInstance::GetLoanSharkDataTable()
{
	if (!CachedLoanSharkDataTable && !LoanSharkDataTable.IsNull())
	{
		CachedLoanSharkDataTable = LoanSharkDataTable.LoadSynchronous();
	}
	return CachedLoanSharkDataTable;
}

bool UT66GameInstance::GetItemData(FName ItemID, FItemData& OutItemData)
{
	UDataTable* DataTable = GetItemsDataTable();
	if (!DataTable)
	{
		return false;
	}
	FItemData* FoundRow = DataTable->FindRow<FItemData>(ItemID, TEXT("GetItemData"));
	if (FoundRow)
	{
		OutItemData = *FoundRow;
		return true;
	}
	return false;
}

bool UT66GameInstance::GetBossData(FName BossID, FBossData& OutBossData)
{
	UDataTable* DataTable = GetBossesDataTable();
	if (!DataTable)
	{
		return false;
	}
	FBossData* FoundRow = DataTable->FindRow<FBossData>(BossID, TEXT("GetBossData"));
	if (FoundRow)
	{
		OutBossData = *FoundRow;
		return true;
	}
	return false;
}

bool UT66GameInstance::GetStageData(int32 StageNumber, FStageData& OutStageData)
{
	UDataTable* DataTable = GetStagesDataTable();
	if (!DataTable)
	{
		return false;
	}
	const FName RowName(*FString::Printf(TEXT("Stage_%02d"), StageNumber));
	FStageData* FoundRow = DataTable->FindRow<FStageData>(RowName, TEXT("GetStageData"));
	if (FoundRow)
	{
		OutStageData = *FoundRow;
		return true;
	}
	return false;
}

bool UT66GameInstance::GetHouseNPCData(FName NPCID, FHouseNPCData& OutNPCData)
{
	if (NPCID.IsNone()) return false;
	UDataTable* DataTable = GetHouseNPCsDataTable();
	if (!DataTable)
	{
		return false;
	}
	FHouseNPCData* FoundRow = DataTable->FindRow<FHouseNPCData>(NPCID, TEXT("GetHouseNPCData"));
	if (FoundRow)
	{
		OutNPCData = *FoundRow;
		return true;
	}
	return false;
}

bool UT66GameInstance::GetLoanSharkData(FName LoanSharkID, FLoanSharkData& OutData)
{
	if (LoanSharkID.IsNone()) return false;
	UDataTable* DataTable = GetLoanSharkDataTable();
	if (!DataTable)
	{
		return false;
	}
	FLoanSharkData* FoundRow = DataTable->FindRow<FLoanSharkData>(LoanSharkID, TEXT("GetLoanSharkData"));
	if (FoundRow)
	{
		OutData = *FoundRow;
		return true;
	}
	return false;
}

TArray<FName> UT66GameInstance::GetAllHeroIDs()
{
	TArray<FName> HeroIDs;
	UDataTable* DataTable = GetHeroDataTable();
	if (DataTable)
	{
		HeroIDs = DataTable->GetRowNames();
	}
	return HeroIDs;
}

TArray<FName> UT66GameInstance::GetAllCompanionIDs()
{
	TArray<FName> CompanionIDs;
	UDataTable* DataTable = GetCompanionDataTable();
	if (DataTable)
	{
		CompanionIDs = DataTable->GetRowNames();
	}
	return CompanionIDs;
}

bool UT66GameInstance::GetSelectedHeroData(FHeroData& OutHeroData)
{
	if (SelectedHeroID.IsNone())
	{
		return false;
	}
	return GetHeroData(SelectedHeroID, OutHeroData);
}

bool UT66GameInstance::GetSelectedCompanionData(FCompanionData& OutCompanionData)
{
	if (SelectedCompanionID.IsNone())
	{
		return false;
	}
	return GetCompanionData(SelectedCompanionID, OutCompanionData);
}

void UT66GameInstance::ClearSelections()
{
	SelectedPartySize = ET66PartySize::Solo;
	SelectedHeroID = NAME_None;
	SelectedCompanionID = NAME_None;
	SelectedDifficulty = ET66Difficulty::Easy;
	SelectedHeroBodyType = ET66BodyType::TypeA;
	SelectedCompanionBodyType = ET66BodyType::TypeA;
}
