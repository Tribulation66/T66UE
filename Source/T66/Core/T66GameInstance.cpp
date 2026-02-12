// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66GameInstance.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66LoadingScreenWidget.h"
#include "UI/Style/T66Style.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture2D.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"

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

	// Preload core DataTables early, asynchronously, so we avoid sync loads later.
	PrimeCoreDataTablesAsync();

	// Preload all main-menu + Party Picker textures so they are often ready before BuildSlateUI's
	// EnsureTexturesLoadedSync fallback fires. If these finish in time, the sync path becomes a no-op.
	if (UT66UITexturePoolSubsystem* TexPool = GetSubsystem<UT66UITexturePoolSubsystem>())
	{
		const TSoftObjectPtr<UTexture2D> MMDark(FSoftObjectPath(TEXT("/Game/UI/MainMenu/MMDark.MMDark")));
		const TSoftObjectPtr<UTexture2D> MMLight(FSoftObjectPath(TEXT("/Game/UI/MainMenu/MMLight.MMLight")));
		const TSoftObjectPtr<UTexture2D> LBGlobal(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_LB_Global.T_LB_Global")));
		const TSoftObjectPtr<UTexture2D> LBFriends(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_LB_Friends.T_LB_Friends")));
		const TSoftObjectPtr<UTexture2D> LBStreamers(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_LB_Streamers.T_LB_Streamers")));
		const TSoftObjectPtr<UTexture2D> SoloDark(FSoftObjectPath(TEXT("/Game/UI/PartyPicker/SoloDark.SoloDark")));
		const TSoftObjectPtr<UTexture2D> SoloLight(FSoftObjectPath(TEXT("/Game/UI/PartyPicker/SoloLight.SoloLight")));
		const TSoftObjectPtr<UTexture2D> CoopDark(FSoftObjectPath(TEXT("/Game/UI/PartyPicker/CoopDark.CoopDark")));
		const TSoftObjectPtr<UTexture2D> CoopLight(FSoftObjectPath(TEXT("/Game/UI/PartyPicker/CoopLight.CoopLight")));
		TexPool->RequestTexture(MMDark, this, FName(TEXT("PreloadMMDark")), [](UTexture2D*) {});
		TexPool->RequestTexture(MMLight, this, FName(TEXT("PreloadMMLight")), [](UTexture2D*) {});
		TexPool->RequestTexture(LBGlobal, this, FName(TEXT("PreloadLBGlobal")), [](UTexture2D*) {});
		TexPool->RequestTexture(LBFriends, this, FName(TEXT("PreloadLBFriends")), [](UTexture2D*) {});
		TexPool->RequestTexture(LBStreamers, this, FName(TEXT("PreloadLBStreamers")), [](UTexture2D*) {});
		TexPool->RequestTexture(SoloDark, this, FName(TEXT("PreloadSoloDark")), [](UTexture2D*) {});
		TexPool->RequestTexture(SoloLight, this, FName(TEXT("PreloadSoloLight")), [](UTexture2D*) {});
		TexPool->RequestTexture(CoopDark, this, FName(TEXT("PreloadCoopDark")), [](UTexture2D*) {});
		TexPool->RequestTexture(CoopLight, this, FName(TEXT("PreloadCoopLight")), [](UTexture2D*) {});
	}
}

void UT66GameInstance::PrimeCoreDataTablesAsync()
{
	if (bCoreDataTablesLoadRequested)
	{
		return;
	}
	bCoreDataTablesLoadRequested = true;

	TArray<FSoftObjectPath> Paths;
	Paths.Reserve(9);

	auto AddDT = [&](const TSoftObjectPtr<UDataTable>& DT)
	{
		if (!DT.IsNull())
		{
			Paths.AddUnique(DT.ToSoftObjectPath());
		}
	};

	AddDT(HeroDataTable);
	AddDT(CompanionDataTable);
	AddDT(ItemsDataTable);
	AddDT(IdolsDataTable);
	AddDT(BossesDataTable);
	AddDT(StagesDataTable);
	AddDT(HouseNPCsDataTable);
	AddDT(LoanSharkDataTable);
	AddDT(CharacterVisualsDataTable);

	if (Paths.Num() <= 0)
	{
		bCoreDataTablesLoaded = true;
		return;
	}

	CoreDataTablesLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		Paths,
		FStreamableDelegate::CreateUObject(this, &UT66GameInstance::HandleCoreDataTablesLoaded));

	// If the handle couldn't be created, fall back to on-demand loads via getters.
	if (!CoreDataTablesLoadHandle.IsValid())
	{
		bCoreDataTablesLoaded = true;
	}
}

void UT66GameInstance::HandleCoreDataTablesLoaded()
{
	// Cache any tables that successfully loaded.
	if (!CachedHeroDataTable) CachedHeroDataTable = HeroDataTable.Get();
	if (!CachedCompanionDataTable) CachedCompanionDataTable = CompanionDataTable.Get();
	if (!CachedItemsDataTable) CachedItemsDataTable = ItemsDataTable.Get();
	if (!CachedIdolsDataTable) CachedIdolsDataTable = IdolsDataTable.Get();
	if (!CachedBossesDataTable) CachedBossesDataTable = BossesDataTable.Get();
	if (!CachedStagesDataTable) CachedStagesDataTable = StagesDataTable.Get();
	if (!CachedHouseNPCsDataTable) CachedHouseNPCsDataTable = HouseNPCsDataTable.Get();
	if (!CachedLoanSharkDataTable) CachedLoanSharkDataTable = LoanSharkDataTable.Get();
	if (!CachedCharacterVisualsDataTable) CachedCharacterVisualsDataTable = CharacterVisualsDataTable.Get();

	bCoreDataTablesLoaded = true;
}

UDataTable* UT66GameInstance::GetHeroDataTable()
{
	if (!CachedHeroDataTable && !HeroDataTable.IsNull())
	{
		CachedHeroDataTable = HeroDataTable.Get();
		if (!CachedHeroDataTable)
		{
			// Kick off async preload if we haven't already, but keep a safe sync fallback.
			PrimeCoreDataTablesAsync();
			CachedHeroDataTable = HeroDataTable.LoadSynchronous();
		}
	}
	return CachedHeroDataTable;
}

UDataTable* UT66GameInstance::GetCompanionDataTable()
{
	if (!CachedCompanionDataTable && !CompanionDataTable.IsNull())
	{
		CachedCompanionDataTable = CompanionDataTable.Get();
		if (!CachedCompanionDataTable)
		{
			PrimeCoreDataTablesAsync();
			CachedCompanionDataTable = CompanionDataTable.LoadSynchronous();
		}
	}
	return CachedCompanionDataTable;
}

UDataTable* UT66GameInstance::GetIdolsDataTable()
{
	if (!CachedIdolsDataTable && !IdolsDataTable.IsNull())
	{
		CachedIdolsDataTable = IdolsDataTable.Get();
		if (!CachedIdolsDataTable)
		{
			PrimeCoreDataTablesAsync();
			CachedIdolsDataTable = IdolsDataTable.LoadSynchronous();
		}
	}
	return CachedIdolsDataTable;
}

bool UT66GameInstance::GetHeroData(FName HeroID, FHeroData& OutHeroData)
{
	if (HeroID.IsNone())
	{
		return false;
	}
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
		CachedItemsDataTable = ItemsDataTable.Get();
		if (!CachedItemsDataTable)
		{
			PrimeCoreDataTablesAsync();
			CachedItemsDataTable = ItemsDataTable.LoadSynchronous();
		}
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
		CachedItemIDs.Add(FName(TEXT("Item_AoeDamage")));
		CachedItemIDs.Add(FName(TEXT("Item_CritDamage")));
		CachedItemIDs.Add(FName(TEXT("Item_LifeSteal")));
		CachedItemIDs.Add(FName(TEXT("Item_MovementSpeed")));
	}
}

void UT66GameInstance::EnsureCachedItemIDsByRarity()
{
	// Items are now rarity-agnostic templates. All templates go into every pool.
	if (bCachedItemIDsByRarityInitialized)
	{
		return;
	}
	bCachedItemIDsByRarityInitialized = true;

	EnsureCachedItemIDs();

	// All templates are valid for any rarity.
	CachedItemIDs_Black = CachedItemIDs;
	CachedItemIDs_Red = CachedItemIDs;
	CachedItemIDs_Yellow = CachedItemIDs;
	CachedItemIDs_White = CachedItemIDs;
}

FName UT66GameInstance::GetRandomItemID()
{
	EnsureCachedItemIDs();
	if (CachedItemIDs.Num() <= 0)
	{
		return FName(TEXT("Item_AoeDamage"));
	}
	return CachedItemIDs[FMath::RandRange(0, CachedItemIDs.Num() - 1)];
}

FName UT66GameInstance::GetRandomItemIDForLootRarity(ET66Rarity LootRarity)
{
	// Items are now rarity-agnostic templates. Just return a random template.
	// The caller is responsible for assigning a rarity based on the loot context.
	return GetRandomItemID();
}

UDataTable* UT66GameInstance::GetBossesDataTable()
{
	if (!CachedBossesDataTable && !BossesDataTable.IsNull())
	{
		CachedBossesDataTable = BossesDataTable.Get();
		if (!CachedBossesDataTable)
		{
			PrimeCoreDataTablesAsync();
			CachedBossesDataTable = BossesDataTable.LoadSynchronous();
		}
	}
	return CachedBossesDataTable;
}

UDataTable* UT66GameInstance::GetStagesDataTable()
{
	if (!CachedStagesDataTable && !StagesDataTable.IsNull())
	{
		CachedStagesDataTable = StagesDataTable.Get();
		if (!CachedStagesDataTable)
		{
			PrimeCoreDataTablesAsync();
			CachedStagesDataTable = StagesDataTable.LoadSynchronous();
		}
	}
	return CachedStagesDataTable;
}

UDataTable* UT66GameInstance::GetHouseNPCsDataTable()
{
	if (!CachedHouseNPCsDataTable && !HouseNPCsDataTable.IsNull())
	{
		CachedHouseNPCsDataTable = HouseNPCsDataTable.Get();
		if (!CachedHouseNPCsDataTable)
		{
			PrimeCoreDataTablesAsync();
			CachedHouseNPCsDataTable = HouseNPCsDataTable.LoadSynchronous();
		}
	}
	return CachedHouseNPCsDataTable;
}

UDataTable* UT66GameInstance::GetLoanSharkDataTable()
{
	if (!CachedLoanSharkDataTable && !LoanSharkDataTable.IsNull())
	{
		CachedLoanSharkDataTable = LoanSharkDataTable.Get();
		if (!CachedLoanSharkDataTable)
		{
			PrimeCoreDataTablesAsync();
			CachedLoanSharkDataTable = LoanSharkDataTable.LoadSynchronous();
		}
	}
	return CachedLoanSharkDataTable;
}

UDataTable* UT66GameInstance::GetCharacterVisualsDataTable()
{
	if (!CachedCharacterVisualsDataTable && !CharacterVisualsDataTable.IsNull())
	{
		CachedCharacterVisualsDataTable = CharacterVisualsDataTable.Get();
		if (!CachedCharacterVisualsDataTable)
		{
			PrimeCoreDataTablesAsync();
			CachedCharacterVisualsDataTable = CharacterVisualsDataTable.LoadSynchronous();
		}
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

bool UT66GameInstance::GetIdolData(FName IdolID, FIdolData& OutIdolData)
{
	UDataTable* DataTable = GetIdolsDataTable();
	if (!DataTable)
	{
		return false;
	}
	FIdolData* FoundRow = DataTable->FindRow<FIdolData>(IdolID, TEXT("GetIdolData"));
	if (FoundRow)
	{
		OutIdolData = *FoundRow;
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

	// Safe defaults (used if DataTable lookup fails).
	OutBaseStats = FT66HeroStatBlock{};
	OutBaseStats.Damage = 2;
	OutBaseStats.AttackSpeed = 2;
	OutBaseStats.AttackScale = 2;
	OutBaseStats.Armor = 2;
	OutBaseStats.Evasion = 2;
	OutBaseStats.Luck = 2;
	OutBaseStats.Speed = 2;

	OutPerLevelGains = FT66HeroPerLevelStatGains{};
	OutPerLevelGains.Damage = Range(1, 2);
	OutPerLevelGains.AttackSpeed = Range(1, 2);
	OutPerLevelGains.AttackScale = Range(1, 2);
	OutPerLevelGains.Armor = Range(1, 2);
	OutPerLevelGains.Evasion = Range(1, 2);
	OutPerLevelGains.Luck = Range(1, 2);

	if (HeroID.IsNone()) return false;

	// Data-driven: read base stats and per-level gains from the Heroes DataTable.
	FHeroData HD;
	if (const_cast<UT66GameInstance*>(this)->GetHeroData(HeroID, HD))
	{
		OutBaseStats.Damage      = FMath::Max(1, HD.BaseDamage);
		OutBaseStats.AttackSpeed = FMath::Max(1, HD.BaseAttackSpeed);
		OutBaseStats.AttackScale = FMath::Max(1, HD.BaseAttackScale);
		OutBaseStats.Armor       = FMath::Max(1, HD.BaseArmor);
		OutBaseStats.Evasion     = FMath::Max(1, HD.BaseEvasion);
		OutBaseStats.Luck        = FMath::Max(1, HD.BaseLuck);
		OutBaseStats.Speed       = FMath::Max(1, HD.BaseSpeed);

		OutPerLevelGains.Damage      = Range(HD.LvlDmgMin, HD.LvlDmgMax);
		OutPerLevelGains.AttackSpeed = Range(HD.LvlAtkSpdMin, HD.LvlAtkSpdMax);
		OutPerLevelGains.AttackScale = Range(HD.LvlAtkScaleMin, HD.LvlAtkScaleMax);
		OutPerLevelGains.Armor       = Range(HD.LvlArmorMin, HD.LvlArmorMax);
		OutPerLevelGains.Evasion     = Range(HD.LvlEvasionMin, HD.LvlEvasionMax);
		OutPerLevelGains.Luck        = Range(HD.LvlLuckMin, HD.LvlLuckMax);
		return true;
	}

	// DataTable row not found: defaults apply.
	return true;
}

void UT66GameInstance::PreloadGameplayAssets(TFunction<void()> OnComplete)
{
	if (bGameplayAssetsPreloadInFlight)
	{
		// Already in flight; replace callback so the latest caller gets notified.
		GameplayAssetsPreloadCallback = MoveTemp(OnComplete);
		return;
	}

	TArray<FSoftObjectPath> Paths;
	Paths.Reserve(8);

	// Engine cube mesh (used ~6 times in GameMode for walls/floors/arenas).
	Paths.Add(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube")));

	// Ground floor materials (4 rotation variants).
	Paths.Add(FSoftObjectPath(TEXT("/Game/World/Ground/M_GroundAtlas_2x2_R0.M_GroundAtlas_2x2_R0")));
	Paths.Add(FSoftObjectPath(TEXT("/Game/World/Ground/M_GroundAtlas_2x2_R90.M_GroundAtlas_2x2_R90")));
	Paths.Add(FSoftObjectPath(TEXT("/Game/World/Ground/M_GroundAtlas_2x2_R180.M_GroundAtlas_2x2_R180")));
	Paths.Add(FSoftObjectPath(TEXT("/Game/World/Ground/M_GroundAtlas_2x2_R270.M_GroundAtlas_2x2_R270")));

	// Placeholder color material used during level setup.
	Paths.Add(FSoftObjectPath(TEXT("/Game/Materials/M_PlaceholderColor.M_PlaceholderColor")));

	if (Paths.Num() <= 0)
	{
		if (OnComplete) OnComplete();
		return;
	}

	bGameplayAssetsPreloadInFlight = true;
	GameplayAssetsPreloadCallback = MoveTemp(OnComplete);

	GameplayAssetsPreloadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		Paths,
		FStreamableDelegate::CreateUObject(this, &UT66GameInstance::HandleGameplayAssetsPreloaded));

	if (!GameplayAssetsPreloadHandle.IsValid())
	{
		// Failed to start; fire callback immediately.
		HandleGameplayAssetsPreloaded();
	}
}

void UT66GameInstance::HandleGameplayAssetsPreloaded()
{
	bGameplayAssetsPreloadInFlight = false;
	GameplayAssetsPreloadHandle.Reset();
	if (GameplayAssetsPreloadCallback)
	{
		TFunction<void()> Cb = MoveTemp(GameplayAssetsPreloadCallback);
		Cb();
	}
}

void UT66GameInstance::TransitionToGameplayLevel()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Show loading screen immediately so the player sees feedback instead of a frozen frame.
	UT66LoadingScreenWidget* LoadingWidget = CreateWidget<UT66LoadingScreenWidget>(World, UT66LoadingScreenWidget::StaticClass());
	if (LoadingWidget)
	{
		LoadingWidget->AddToViewport(9999); // On top of everything.
	}

	// Flush one frame so the loading screen renders before we start async work.
	// Then pre-load heavy gameplay assets; once done, open the level.
	FTimerHandle PreloadTimerHandle;
	World->GetTimerManager().SetTimer(PreloadTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		PreloadGameplayAssets([this]()
		{
			UGameplayStatics::OpenLevel(this, FName(TEXT("GameplayLevel")));
		});
	}), 0.05f, false); // Small delay so the loading widget paints first.
}
