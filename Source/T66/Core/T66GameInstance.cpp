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
