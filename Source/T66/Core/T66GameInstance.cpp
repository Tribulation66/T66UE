// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66GameInstance.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/IConsoleManager.h"

namespace
{
	// Goal: remove "soft/blurry" presentation caused by resolution scaling / dynamic res.
	// Do this once on boot (no per-frame work).
	void ApplyCrispRenderingDefaults()
	{
		if (GEngine)
		{
			if (UGameUserSettings* GUS = GEngine->GetGameUserSettings())
			{
				// Ensure we're rendering at native resolution (no upscaling blur).
				GUS->SetDynamicResolutionEnabled(false);
				GUS->SetResolutionScaleValueEx(100.f);
				GUS->ApplySettings(false);
			}
		}

		auto SetCVarFloat = [](const TCHAR* Name, float Value)
		{
			if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(Name))
			{
				CVar->Set(Value, ECVF_SetByGameSetting);
			}
		};
		auto SetCVarInt = [](const TCHAR* Name, int32 Value)
		{
			if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(Name))
			{
				CVar->Set(Value, ECVF_SetByGameSetting);
			}
		};

		SetCVarFloat(TEXT("r.ScreenPercentage"), 100.f);
		SetCVarFloat(TEXT("r.SecondaryScreenPercentage.GameViewport"), 100.f);
		SetCVarInt(TEXT("r.DynamicRes.OperationMode"), 0);
		SetCVarInt(TEXT("r.TemporalAA.Upsampling"), 0);
	}
}

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

	ApplyCrispRenderingDefaults();

	// Pre-load DataTables on init if paths are set
	GetHeroDataTable();
	GetCompanionDataTable();
	GetItemsDataTable();
	GetBossesDataTable();
	GetStagesDataTable();
	GetHouseNPCsDataTable();
	GetLoanSharkDataTable();
	GetCharacterVisualsDataTable();
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
			// Canonical item list: items that exist in the game should be both lootable and purchasable.
			// v0 rule: BuyValueGold > 0 indicates it is part of the canonical pool.
			if (D.BuyValueGold <= 0)
			{
				continue;
			}
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

UDataTable* UT66GameInstance::GetCharacterVisualsDataTable()
{
	if (!CachedCharacterVisualsDataTable && !CharacterVisualsDataTable.IsNull())
	{
		CachedCharacterVisualsDataTable = CharacterVisualsDataTable.LoadSynchronous();
	}
	return CachedCharacterVisualsDataTable;
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

bool UT66GameInstance::GetHeroStatTuning(FName HeroID, FT66HeroStatBlock& OutBaseStats, FT66HeroPerLevelStatGains& OutPerLevelGains) const
{
	auto Range = [](int32 Min, int32 Max) -> FT66HeroStatGainRange
	{
		FT66HeroStatGainRange R;
		R.Min = Min;
		R.Max = Max;
		return R;
	};

	// Default: balanced-ish, safe fallbacks.
	OutBaseStats = FT66HeroStatBlock{};
	OutBaseStats.Damage = 2;
	OutBaseStats.AttackSpeed = 2;
	OutBaseStats.AttackSize = 2;
	OutBaseStats.Armor = 2;
	OutBaseStats.Evasion = 2;
	OutBaseStats.Luck = 2;
	OutBaseStats.Speed = 2;

	OutPerLevelGains = FT66HeroPerLevelStatGains{};
	OutPerLevelGains.Damage = Range(1, 2);
	OutPerLevelGains.AttackSpeed = Range(1, 2);
	OutPerLevelGains.AttackSize = Range(1, 2);
	OutPerLevelGains.Armor = Range(1, 2);
	OutPerLevelGains.Evasion = Range(1, 2);
	OutPerLevelGains.Luck = Range(1, 2);

	const FName H = HeroID;
	if (H.IsNone())
	{
		return false;
	}

	// NOTE: These are initial tuning values (1-5 base). They can be moved to a DataTable later,
	// but for now they are centralized and deterministic by HeroID.
	if (H == FName(TEXT("Hero_AliceInWonderlandRabbit")))
	{
		OutBaseStats = { 2, 4, 2, 1, 5, 2, 4 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(2, 3);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(1, 2);
		OutPerLevelGains.Evasion = Range(2, 4);
		OutPerLevelGains.Luck = Range(1, 3);
		return true;
	}
	if (H == FName(TEXT("Hero_LuBu")))
	{
		OutBaseStats = { 5, 2, 3, 5, 1, 1, 2 };
		OutPerLevelGains.Damage = Range(2, 4);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(2, 4);
		OutPerLevelGains.Evasion = Range(1, 2);
		OutPerLevelGains.Luck = Range(1, 2);
		return true;
	}
	if (H == FName(TEXT("Hero_LeonardoDaVinci")))
	{
		OutBaseStats = { 2, 2, 3, 2, 2, 5, 2 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 3);
		OutPerLevelGains.Armor = Range(1, 2);
		OutPerLevelGains.Evasion = Range(1, 2);
		OutPerLevelGains.Luck = Range(2, 4);
		return true;
	}
	if (H == FName(TEXT("Hero_Yakub")))
	{
		OutBaseStats = { 3, 2, 2, 3, 3, 3, 2 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(1, 3);
		OutPerLevelGains.Evasion = Range(1, 3);
		OutPerLevelGains.Luck = Range(1, 3);
		return true;
	}
	if (H == FName(TEXT("Hero_KingArthur")))
	{
		OutBaseStats = { 4, 2, 2, 5, 1, 2, 2 };
		OutPerLevelGains.Damage = Range(2, 3);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(2, 4);
		OutPerLevelGains.Evasion = Range(1, 2);
		OutPerLevelGains.Luck = Range(1, 3);
		return true;
	}
	if (H == FName(TEXT("Hero_MiyamotoMusashi")))
	{
		OutBaseStats = { 4, 5, 2, 2, 3, 1, 4 };
		OutPerLevelGains.Damage = Range(2, 3);
		OutPerLevelGains.AttackSpeed = Range(2, 4);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(1, 2);
		OutPerLevelGains.Evasion = Range(1, 3);
		OutPerLevelGains.Luck = Range(1, 2);
		return true;
	}
	if (H == FName(TEXT("Hero_CaptainJackSparrow")))
	{
		OutBaseStats = { 2, 3, 2, 1, 4, 5, 4 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(1, 3);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(1, 2);
		OutPerLevelGains.Evasion = Range(2, 4);
		OutPerLevelGains.Luck = Range(2, 4);
		return true;
	}
	if (H == FName(TEXT("Hero_SoloLeveler")))
	{
		OutBaseStats = { 3, 3, 3, 3, 3, 3, 3 };
		OutPerLevelGains.Damage = Range(2, 4);
		OutPerLevelGains.AttackSpeed = Range(2, 4);
		OutPerLevelGains.AttackSize = Range(2, 4);
		OutPerLevelGains.Armor = Range(2, 4);
		OutPerLevelGains.Evasion = Range(2, 4);
		OutPerLevelGains.Luck = Range(2, 4);
		return true;
	}
	if (H == FName(TEXT("Hero_Saitama")))
	{
		OutBaseStats = { 5, 2, 2, 4, 1, 1, 3 };
		OutPerLevelGains.Damage = Range(3, 4);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(2, 3);
		OutPerLevelGains.Evasion = Range(1, 2);
		OutPerLevelGains.Luck = Range(1, 2);
		return true;
	}
	if (H == FName(TEXT("Hero_RoboGoon")))
	{
		OutBaseStats = { 3, 2, 3, 5, 1, 1, 2 };
		OutPerLevelGains.Damage = Range(1, 3);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(2, 4);
		OutPerLevelGains.Evasion = Range(1, 2);
		OutPerLevelGains.Luck = Range(1, 2);
		return true;
	}
	if (H == FName(TEXT("Hero_GeorgeWashington")))
	{
		OutBaseStats = { 3, 2, 2, 4, 2, 3, 2 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(2, 3);
		OutPerLevelGains.Evasion = Range(1, 2);
		OutPerLevelGains.Luck = Range(1, 3);
		return true;
	}
	if (H == FName(TEXT("Hero_Cain")))
	{
		OutBaseStats = { 2, 2, 2, 4, 3, 2, 2 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(2, 3);
		OutPerLevelGains.Evasion = Range(1, 3);
		OutPerLevelGains.Luck = Range(1, 2);
		return true;
	}
	if (H == FName(TEXT("Hero_BillyTheKid")))
	{
		OutBaseStats = { 2, 5, 1, 1, 4, 3, 5 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(2, 4);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(1, 2);
		OutPerLevelGains.Evasion = Range(2, 3);
		OutPerLevelGains.Luck = Range(1, 3);
		return true;
	}
	if (H == FName(TEXT("Hero_RoachKing")))
	{
		OutBaseStats = { 2, 2, 3, 4, 3, 2, 2 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(2, 3);
		OutPerLevelGains.Evasion = Range(1, 3);
		OutPerLevelGains.Luck = Range(1, 2);
		return true;
	}
	if (H == FName(TEXT("Hero_Goblino")))
	{
		OutBaseStats = { 2, 2, 2, 2, 3, 4, 3 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(1, 2);
		OutPerLevelGains.Evasion = Range(1, 3);
		OutPerLevelGains.Luck = Range(2, 4);
		return true;
	}
	if (H == FName(TEXT("Hero_BulkBite")))
	{
		OutBaseStats = { 4, 1, 5, 4, 1, 1, 2 };
		OutPerLevelGains.Damage = Range(2, 4);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(2, 4);
		OutPerLevelGains.Armor = Range(2, 3);
		OutPerLevelGains.Evasion = Range(1, 2);
		OutPerLevelGains.Luck = Range(1, 2);
		return true;
	}
	if (H == FName(TEXT("Hero_HoboWanderer")))
	{
		OutBaseStats = { 2, 2, 2, 3, 2, 3, 2 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(1, 3);
		OutPerLevelGains.Evasion = Range(1, 2);
		OutPerLevelGains.Luck = Range(1, 3);
		return true;
	}
	if (H == FName(TEXT("Hero_DryHumor")))
	{
		OutBaseStats = { 3, 3, 2, 1, 4, 2, 3 };
		OutPerLevelGains.Damage = Range(1, 3);
		OutPerLevelGains.AttackSpeed = Range(1, 3);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(1, 2);
		OutPerLevelGains.Evasion = Range(2, 4);
		OutPerLevelGains.Luck = Range(1, 2);
		return true;
	}
	if (H == FName(TEXT("Hero_LyricVoi")))
	{
		OutBaseStats = { 2, 3, 2, 1, 3, 4, 3 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(1, 3);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(1, 2);
		OutPerLevelGains.Evasion = Range(1, 3);
		OutPerLevelGains.Luck = Range(2, 4);
		return true;
	}
	if (H == FName(TEXT("Hero_Jesterma")))
	{
		OutBaseStats = { 3, 2, 2, 1, 4, 3, 4 };
		OutPerLevelGains.Damage = Range(1, 3);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(1, 2);
		OutPerLevelGains.Evasion = Range(2, 4);
		OutPerLevelGains.Luck = Range(1, 3);
		return true;
	}
	if (H == FName(TEXT("Hero_NorthKing")))
	{
		OutBaseStats = { 3, 2, 3, 4, 1, 2, 2 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 3);
		OutPerLevelGains.Armor = Range(2, 3);
		OutPerLevelGains.Evasion = Range(1, 2);
		OutPerLevelGains.Luck = Range(1, 2);
		return true;
	}
	if (H == FName(TEXT("Hero_Peakwarden")))
	{
		OutBaseStats = { 3, 2, 2, 5, 1, 2, 1 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(2, 4);
		OutPerLevelGains.Evasion = Range(1, 2);
		OutPerLevelGains.Luck = Range(1, 2);
		return true;
	}
	if (H == FName(TEXT("Hero_QuinnHex")))
	{
		OutBaseStats = { 2, 2, 3, 1, 2, 5, 2 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 3);
		OutPerLevelGains.Armor = Range(1, 2);
		OutPerLevelGains.Evasion = Range(1, 2);
		OutPerLevelGains.Luck = Range(2, 4);
		return true;
	}
	if (H == FName(TEXT("Hero_SandSultan")))
	{
		OutBaseStats = { 2, 2, 3, 2, 3, 4, 3 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 3);
		OutPerLevelGains.Armor = Range(1, 2);
		OutPerLevelGains.Evasion = Range(1, 3);
		OutPerLevelGains.Luck = Range(2, 4);
		return true;
	}
	if (H == FName(TEXT("Hero_CharNut")))
	{
		OutBaseStats = { 3, 2, 2, 4, 1, 2, 2 };
		OutPerLevelGains.Damage = Range(1, 2);
		OutPerLevelGains.AttackSpeed = Range(1, 2);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(2, 3);
		OutPerLevelGains.Evasion = Range(1, 2);
		OutPerLevelGains.Luck = Range(1, 2);
		return true;
	}
	if (H == FName(TEXT("Hero_Wraithveil")))
	{
		OutBaseStats = { 3, 3, 2, 1, 5, 2, 4 };
		OutPerLevelGains.Damage = Range(1, 3);
		OutPerLevelGains.AttackSpeed = Range(1, 3);
		OutPerLevelGains.AttackSize = Range(1, 2);
		OutPerLevelGains.Armor = Range(1, 2);
		OutPerLevelGains.Evasion = Range(2, 4);
		OutPerLevelGains.Luck = Range(1, 2);
		return true;
	}

	// Default tuning applies.
	return true;
}
