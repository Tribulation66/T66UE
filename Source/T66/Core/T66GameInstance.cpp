// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66GameInstance.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66RetroFXSubsystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
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
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66GameInstance, Log, All);

namespace
{
	static const FName GamblersTokenItemID(TEXT("Item_GamblersToken"));
	static const FName AccuracyItemID(TEXT("Item_Accuracy"));
	static const FName CloseRangeItemID(TEXT("Item_CloseRangeDmg"));
	static const FName LongRangeItemID(TEXT("Item_LongRangeDmg"));
	static const FName SpinWheelItemID(TEXT("Item_SpinWheel"));
	static const FName MovementSpeedItemID(TEXT("Item_MovementSpeed"));
	static const TCHAR* KnightPreviewMovieName = TEXT("KnightClip.mp4");

	static FName NormalizeLegacyItemID(FName ItemID)
	{
		if (ItemID == FName(TEXT("Item_Goblin")))
		{
			return FName(TEXT("Item_LootCrate"));
		}
		return ItemID;
	}

	static bool IsRandomItemPoolEligible(FName ItemID)
	{
		return !ItemID.IsNone()
			&& ItemID != GamblersTokenItemID
			&& ItemID != CloseRangeItemID
			&& ItemID != LongRangeItemID
			&& ItemID != SpinWheelItemID
			&& ItemID != MovementSpeedItemID;
	}

	static FString ResolveKnightPreviewMoviePath()
	{
		const TArray<FString> CandidatePaths = {
			FPaths::ProjectContentDir() / TEXT("Movies") / KnightPreviewMovieName,
			FPaths::ProjectDir() / TEXT("SourceAssets/Movies") / KnightPreviewMovieName
		};

		for (const FString& CandidatePath : CandidatePaths)
		{
			if (FPaths::FileExists(CandidatePath))
			{
				return FPaths::ConvertRelativePathToFull(CandidatePath);
			}
		}

		return FString();
	}

	static FSoftObjectPath NormalizeHeroSelectionAssetPath(const FSoftObjectPath& Path)
	{
		if (Path.IsNull())
		{
			return Path;
		}

		const FString PathString = Path.ToString();
		const FString PackageName = Path.GetLongPackageName();
		if (!PackageName.IsEmpty() && FPackageName::DoesPackageExist(PackageName))
		{
			return Path;
		}

		int32 DotIndex = INDEX_NONE;
		if (!PathString.FindLastChar(TEXT('.'), DotIndex))
		{
			return Path;
		}

		const FString Base = PathString.Left(DotIndex);
		const FString ObjectName = PathString.Mid(DotIndex + 1);
		const bool bBaseHasAnimSuffix = Base.EndsWith(TEXT("_Anim"));
		const bool bObjectHasAnimSuffix = ObjectName.EndsWith(TEXT("_Anim"));
		const FString BaseStrip = bBaseHasAnimSuffix ? Base.LeftChop(5) : Base;
		const FString ObjectStrip = bObjectHasAnimSuffix ? ObjectName.LeftChop(5) : ObjectName;
		const TArray<FString> CandidatePaths = {
			BaseStrip + TEXT(".") + ObjectStrip,
			BaseStrip + TEXT(".") + ObjectName,
			Base + TEXT(".") + ObjectStrip
		};

		for (const FString& CandidatePath : CandidatePaths)
		{
			const FSoftObjectPath CandidateSoftPath(CandidatePath);
			const FString CandidatePackageName = CandidateSoftPath.GetLongPackageName();
			if (!CandidatePackageName.IsEmpty() && FPackageName::DoesPackageExist(CandidatePackageName))
			{
				return CandidateSoftPath;
			}
		}

		return Path;
	}

	static bool BuildSyntheticSpecialItemData(FName ItemID, FItemData& OutItemData)
	{
		if (ItemID == AccuracyItemID)
		{
			OutItemData = FItemData{};
			OutItemData.ItemID = AccuracyItemID;
			OutItemData.Icon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Items/Sprites/Item_Accuracy_black.Item_Accuracy_black")));
			OutItemData.BlackIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Items/Sprites/Item_Accuracy_black.Item_Accuracy_black")));
			OutItemData.RedIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Items/Sprites/Item_Accuracy_red.Item_Accuracy_red")));
			OutItemData.YellowIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Items/Sprites/Item_Accuracy_yellow.Item_Accuracy_yellow")));
			OutItemData.WhiteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Items/Sprites/Item_Accuracy_white.Item_Accuracy_white")));
			OutItemData.PrimaryStatType = ET66HeroStatType::Accuracy;
			OutItemData.SecondaryStatType = ET66SecondaryStatType::Accuracy;
			OutItemData.BaseBuyGold = 55;
			OutItemData.BaseSellGold = 27;
			return true;
		}

		if (ItemID != GamblersTokenItemID)
		{
			return false;
		}

		OutItemData = FItemData{};
		OutItemData.ItemID = GamblersTokenItemID;
		OutItemData.Icon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Items/Sprites/Item_GamblersToken_black.Item_GamblersToken_black")));
		OutItemData.BlackIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Items/Sprites/Item_GamblersToken_black.Item_GamblersToken_black")));
		OutItemData.RedIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Items/Sprites/Item_GamblersToken_red.Item_GamblersToken_red")));
		OutItemData.YellowIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Items/Sprites/Item_GamblersToken_yellow.Item_GamblersToken_yellow")));
		OutItemData.WhiteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Items/Sprites/Item_GamblersToken_white.Item_GamblersToken_white")));
		OutItemData.PrimaryStatType = ET66HeroStatType::Luck;
		OutItemData.SecondaryStatType = ET66SecondaryStatType::GamblerToken;
		OutItemData.BaseBuyGold = 100;
		OutItemData.BaseSellGold = 0;
		return true;
	}

	static const bool bAlwaysRouteTribulationToTutorial = false;
	static const TCHAR* FrontendLevelName = TEXT("/Game/Maps/FrontendLevel");
	static const TCHAR* LabLevelName = TEXT("/Game/Maps/LabLevel");
	static const TCHAR* GameplayLevelName = TEXT("/Game/Maps/GameplayLevel");
	static const TCHAR* ColiseumLevelName = TEXT("/Game/Maps/Gameplay_Coliseum");
	static const TCHAR* TutorialLevelName = TEXT("/Game/Maps/Gameplay_Tutorial");

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
	MiniSelectedHeroID = NAME_None;
	MiniSelectedCompanionID = NAME_None;
	MiniSelectedDifficultyID = NAME_None;
	MiniSelectedIdolIDs.Reset();
	bMiniLoadFlow = false;
	bMiniIntermissionFlow = false;
	MiniIntermissionStateRevision = 0;
	MiniIntermissionStateJson.Reset();
	MiniIntermissionRequestRevision = 0;
	MiniIntermissionRequestJson.Reset();
	SelectedHeroBodyType = ET66BodyType::TypeA;
	SelectedCompanionBodyType = ET66BodyType::TypeA;
}

void UT66GameInstance::Init()
{
	Super::Init();

	ApplyCrispRenderingDefaults();
	ApplyConfiguredMainMapLayoutVariant();
	RestoreRememberedSelectionDefaults();

	// Preload core DataTables early, asynchronously, so we avoid sync loads later.
	PrimeCoreDataTablesAsync();

	// Preload the main-menu textures so they are often ready before BuildSlateUI's
	// EnsureTexturesLoadedSync fallback fires. If these finish in time, the sync path becomes a no-op.
	if (UT66UITexturePoolSubsystem* TexPool = GetSubsystem<UT66UITexturePoolSubsystem>())
	{
		const TSoftObjectPtr<UTexture2D> MMRed(FSoftObjectPath(TEXT("/Game/UI/MainMenu/MMRed.MMRed")));
		const TSoftObjectPtr<UTexture2D> SkyBg(FSoftObjectPath(TEXT("/Game/UI/MainMenu/sky_bg.sky_bg")));
		const TSoftObjectPtr<UTexture2D> FireMoon(FSoftObjectPath(TEXT("/Game/UI/MainMenu/fire_moon.fire_moon")));
		const TSoftObjectPtr<UTexture2D> PyramidChad(FSoftObjectPath(TEXT("/Game/UI/MainMenu/pyramid_chad.pyramid_chad")));
		const TSoftObjectPtr<UTexture2D> LBGlobal(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_LB_Global.T_LB_Global")));
		const TSoftObjectPtr<UTexture2D> LBFriends(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_LB_Friends.T_LB_Friends")));
		const TSoftObjectPtr<UTexture2D> LBStreamers(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_LB_Streamers.T_LB_Streamers")));
		TexPool->RequestTexture(MMRed, this, FName(TEXT("PreloadMMRed")), [](UTexture2D*) {});
		if (FPackageName::DoesPackageExist(TEXT("/Game/UI/MainMenu/sky_bg")))
		{
			TexPool->RequestTexture(SkyBg, this, FName(TEXT("PreloadSkyBg")), [](UTexture2D*) {});
		}
		if (FPackageName::DoesPackageExist(TEXT("/Game/UI/MainMenu/fire_moon")))
		{
			TexPool->RequestTexture(FireMoon, this, FName(TEXT("PreloadFireMoon")), [](UTexture2D*) {});
		}
		if (FPackageName::DoesPackageExist(TEXT("/Game/UI/MainMenu/pyramid_chad")))
		{
			TexPool->RequestTexture(PyramidChad, this, FName(TEXT("PreloadPyramidChad")), [](UTexture2D*) {});
		}
		TexPool->RequestTexture(LBGlobal, this, FName(TEXT("PreloadLBGlobal")), [](UTexture2D*) {});
		TexPool->RequestTexture(LBFriends, this, FName(TEXT("PreloadLBFriends")), [](UTexture2D*) {});
		TexPool->RequestTexture(LBStreamers, this, FName(TEXT("PreloadLBStreamers")), [](UTexture2D*) {});
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
	PrimeHeroSelectionAssetsAsync();
}

void UT66GameInstance::PrimeHeroSelectionAssetsAsync()
{
	if (bHeroSelectionAssetsLoadRequested)
	{
		return;
	}

	if (!CachedHeroDataTable && !HeroDataTable.IsNull())
	{
		CachedHeroDataTable = HeroDataTable.Get();
	}
	if (!CachedCompanionDataTable && !CompanionDataTable.IsNull())
	{
		CachedCompanionDataTable = CompanionDataTable.Get();
	}
	if (!CachedCharacterVisualsDataTable && !CharacterVisualsDataTable.IsNull())
	{
		CachedCharacterVisualsDataTable = CharacterVisualsDataTable.Get();
	}

	if (!CachedHeroDataTable || !CachedCompanionDataTable || !CachedCharacterVisualsDataTable)
	{
		PrimeCoreDataTablesAsync();
		return;
	}

	TArray<FSoftObjectPath> Paths;
	Paths.Reserve(192);

	auto AddPath = [&](const FSoftObjectPath& Path)
	{
		if (!Path.IsNull())
		{
			Paths.AddUnique(NormalizeHeroSelectionAssetPath(Path));
		}
	};

	auto AddVisualAssets = [&](const FName VisualID)
	{
		if (VisualID.IsNone())
		{
			return;
		}

		const FT66CharacterVisualRow* VisualRow = CachedCharacterVisualsDataTable->FindRow<FT66CharacterVisualRow>(VisualID, TEXT("PrimeHeroSelectionAssetsAsync"));
		if (!VisualRow)
		{
			return;
		}

		AddPath(VisualRow->SkeletalMesh.ToSoftObjectPath());
		AddPath(VisualRow->LoopingAnimation.ToSoftObjectPath());
		AddPath(VisualRow->AlertAnimation.ToSoftObjectPath());
		AddPath(VisualRow->RunAnimation.ToSoftObjectPath());
	};

	TArray<FHeroData*> HeroRows;
	CachedHeroDataTable->GetAllRows(TEXT("PrimeHeroSelectionAssetsAsync"), HeroRows);
	for (const FHeroData* HeroRow : HeroRows)
	{
		if (!HeroRow)
		{
			continue;
		}

		AddPath(HeroRow->Portrait.ToSoftObjectPath());
		AddPath(HeroRow->PortraitTypeB.ToSoftObjectPath());
		AddPath(HeroRow->PortraitLow.ToSoftObjectPath());
		AddPath(HeroRow->PortraitFull.ToSoftObjectPath());
		AddPath(HeroRow->PortraitTypeBLow.ToSoftObjectPath());
		AddPath(HeroRow->PortraitTypeBFull.ToSoftObjectPath());
		AddPath(HeroRow->PortraitInvincible.ToSoftObjectPath());
		AddPath(HeroRow->PortraitTypeBInvincible.ToSoftObjectPath());
		if (HeroRow->HeroID == FName(TEXT("Hero_1")))
		{
			AddPath(FSoftObjectPath(TEXT("/Game/UI/Sprites/Heroes/Hero_1/T_Hero_1_TypeA_Invincible.T_Hero_1_TypeA_Invincible")));
			AddPath(FSoftObjectPath(TEXT("/Game/UI/Sprites/Heroes/Hero_1/T_Hero_1_TypeB_Invincible.T_Hero_1_TypeB_Invincible")));
		}
	}

	TArray<FCompanionData*> CompanionRows;
	CachedCompanionDataTable->GetAllRows(TEXT("PrimeHeroSelectionAssetsAsync"), CompanionRows);
	for (const FCompanionData* CompanionRow : CompanionRows)
	{
		if (!CompanionRow)
		{
			continue;
		}

		AddPath(CompanionRow->Portrait.ToSoftObjectPath());
		AddPath(CompanionRow->SelectionPortrait.ToSoftObjectPath());
	}

	FName InitialHeroID = SelectedHeroID;
	if (InitialHeroID.IsNone() && HeroRows.Num() > 0 && HeroRows[0])
	{
		InitialHeroID = HeroRows[0]->HeroID;
	}
	if (!InitialHeroID.IsNone())
	{
		const FName InitialHeroSkinID = SelectedHeroSkinID.IsNone() ? FName(TEXT("Default")) : SelectedHeroSkinID;
		AddVisualAssets(UT66CharacterVisualSubsystem::GetHeroVisualID(InitialHeroID, SelectedHeroBodyType, InitialHeroSkinID));
	}

	if (!SelectedCompanionID.IsNone())
	{
		AddVisualAssets(UT66CharacterVisualSubsystem::GetCompanionVisualID(SelectedCompanionID, FName(TEXT("Default"))));
	}

	if (!ResolveKnightPreviewMoviePath().IsEmpty())
	{
		AddPath(FSoftObjectPath(TEXT("/Game/Characters/Heroes/Knight/KnightClip.KnightClip")));
	}

	bHeroSelectionAssetsLoadRequested = true;
	if (Paths.Num() <= 0)
	{
		bHeroSelectionAssetsLoaded = true;
		return;
	}

	UE_LOG(LogT66GameInstance, Log, TEXT("[LOAD] PrimeHeroSelectionAssetsAsync queued %d startup hero-selection assets."), Paths.Num());

	HeroSelectionAssetsLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		Paths,
		FStreamableDelegate::CreateUObject(this, &UT66GameInstance::HandleHeroSelectionAssetsLoaded));

	if (!HeroSelectionAssetsLoadHandle.IsValid())
	{
		HandleHeroSelectionAssetsLoaded();
	}
}

void UT66GameInstance::HandleHeroSelectionAssetsLoaded()
{
	bHeroSelectionAssetsLoaded = true;
	HeroSelectionAssetsLoadHandle.Reset();
	UE_LOG(LogT66GameInstance, Log, TEXT("[LOAD] Startup hero-selection asset warmup completed."));
}

void UT66GameInstance::PrimeHeroSelectionPreviewVisualsAsync()
{
	if (bHeroSelectionPreviewVisualsLoadRequested)
	{
		return;
	}

	if (!CachedCharacterVisualsDataTable && !CharacterVisualsDataTable.IsNull())
	{
		CachedCharacterVisualsDataTable = CharacterVisualsDataTable.Get();
	}

	if (!CachedCharacterVisualsDataTable)
	{
		PrimeCoreDataTablesAsync();
		return;
	}

	TArray<FSoftObjectPath> Paths;
	Paths.Reserve(256);

	auto AddPath = [&](const FSoftObjectPath& Path)
	{
		if (!Path.IsNull())
		{
			Paths.AddUnique(NormalizeHeroSelectionAssetPath(Path));
		}
	};

	for (const FName& RowName : CachedCharacterVisualsDataTable->GetRowNames())
	{
		const FString RowNameString = RowName.ToString();
		if (!RowNameString.StartsWith(TEXT("Hero_")) && !RowNameString.StartsWith(TEXT("Companion_")))
		{
			continue;
		}

		const FT66CharacterVisualRow* VisualRow = CachedCharacterVisualsDataTable->FindRow<FT66CharacterVisualRow>(RowName, TEXT("PrimeHeroSelectionPreviewVisualsAsync"));
		if (!VisualRow)
		{
			continue;
		}

		AddPath(VisualRow->SkeletalMesh.ToSoftObjectPath());
		AddPath(VisualRow->LoopingAnimation.ToSoftObjectPath());
		AddPath(VisualRow->AlertAnimation.ToSoftObjectPath());
		AddPath(VisualRow->RunAnimation.ToSoftObjectPath());
	}

	bHeroSelectionPreviewVisualsLoadRequested = true;
	if (Paths.Num() <= 0)
	{
		bHeroSelectionPreviewVisualsLoaded = true;
		return;
	}

	UE_LOG(LogT66GameInstance, Log, TEXT("[LOAD] PrimeHeroSelectionPreviewVisualsAsync queued %d deferred preview visual assets."), Paths.Num());

	HeroSelectionPreviewVisualsLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		Paths,
		FStreamableDelegate::CreateUObject(this, &UT66GameInstance::HandleHeroSelectionPreviewVisualsLoaded));

	if (!HeroSelectionPreviewVisualsLoadHandle.IsValid())
	{
		HandleHeroSelectionPreviewVisualsLoaded();
	}
}

void UT66GameInstance::HandleHeroSelectionPreviewVisualsLoaded()
{
	bHeroSelectionPreviewVisualsLoaded = true;
	HeroSelectionPreviewVisualsLoadHandle.Reset();
	UE_LOG(LogT66GameInstance, Log, TEXT("[LOAD] Deferred hero-selection preview visual warmup completed."));
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
		for (const FName ItemID : ItemsDT->GetRowNames())
		{
			FItemData ItemData;
			if (!GetItemData(ItemID, ItemData))
			{
				continue;
			}

			if (IsRandomItemPoolEligible(ItemID) && T66IsLiveSecondaryStatType(ItemData.SecondaryStatType))
			{
				CachedItemIDs.Add(ItemID);
			}
		}
	}

	FItemData AccuracyItemData;
	if (GetItemData(AccuracyItemID, AccuracyItemData)
		&& IsRandomItemPoolEligible(AccuracyItemID)
		&& T66IsLiveSecondaryStatType(AccuracyItemData.SecondaryStatType)
		&& !CachedItemIDs.Contains(AccuracyItemID))
	{
		CachedItemIDs.Add(AccuracyItemID);
	}

	// Fallback (keeps game functional even if DT isn't wired yet).
	if (CachedItemIDs.Num() == 0)
	{
		CachedItemIDs.Add(FName(TEXT("Item_AoeDamage")));
		CachedItemIDs.Add(FName(TEXT("Item_CritDamage")));
		CachedItemIDs.Add(AccuracyItemID);
		CachedItemIDs.Add(FName(TEXT("Item_DamageReduction")));
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
	if (UT66RngSubsystem* RngSub = GetSubsystem<UT66RngSubsystem>())
	{
		return GetRandomItemIDFromStream(RngSub->GetRunStream());
	}

	EnsureCachedItemIDs();
	if (CachedItemIDs.Num() <= 0)
	{
		return FName(TEXT("Item_AoeDamage"));
	}
	return CachedItemIDs[FMath::RandRange(0, CachedItemIDs.Num() - 1)];
}

FName UT66GameInstance::GetRandomItemIDFromStream(FRandomStream& Stream)
{
	EnsureCachedItemIDs();
	if (CachedItemIDs.Num() <= 0)
	{
		return FName(TEXT("Item_AoeDamage"));
	}

	if (UT66RngSubsystem* RngSub = GetSubsystem<UT66RngSubsystem>())
	{
		if (RngSub->UsesRunStream(Stream))
		{
			return CachedItemIDs[RngSub->RunRandRange(0, CachedItemIDs.Num() - 1)];
		}
	}

	return CachedItemIDs[Stream.RandRange(0, CachedItemIDs.Num() - 1)];
}

FName UT66GameInstance::GetRandomItemIDForLootRarity(ET66Rarity LootRarity)
{
	if (UT66RngSubsystem* RngSub = GetSubsystem<UT66RngSubsystem>())
	{
		return GetRandomItemIDForLootRarityFromStream(LootRarity, RngSub->GetRunStream());
	}

	return GetRandomItemID();
}

FName UT66GameInstance::GetRandomItemIDForLootRarityFromStream(ET66Rarity LootRarity, FRandomStream& Stream)
{
	// Items are now rarity-agnostic templates. Just return a random template.
	// The caller is responsible for assigning a rarity based on the loot context.
	return GetRandomItemIDFromStream(Stream);
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
	const FName NormalizedItemID = NormalizeLegacyItemID(ItemID);
	if (UDataTable* DataTable = GetItemsDataTable())
	{
		if (FItemData* FoundRow = DataTable->FindRow<FItemData>(NormalizedItemID, TEXT("GetItemData")))
		{
			OutItemData = *FoundRow;
			OutItemData.PrimaryStatType = T66ResolveEffectivePrimaryStatType(OutItemData.PrimaryStatType, OutItemData.SecondaryStatType);
			return true;
		}
	}

	if (BuildSyntheticSpecialItemData(NormalizedItemID, OutItemData))
	{
		OutItemData.PrimaryStatType = T66ResolveEffectivePrimaryStatType(OutItemData.PrimaryStatType, OutItemData.SecondaryStatType);
		return true;
	}

	return false;
}

bool UT66GameInstance::GetIdolData(FName IdolID, FIdolData& OutIdolData)
{
	if (IdolID.IsNone())
	{
		return false;
	}

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

TSoftObjectPtr<UTexture2D> UT66GameInstance::ResolveHeroPortrait(FName HeroID, ET66BodyType BodyType, ET66HeroPortraitVariant Variant) const
{
	FHeroData HeroData;
	if (!const_cast<UT66GameInstance*>(this)->GetHeroData(HeroID, HeroData))
	{
		return TSoftObjectPtr<UTexture2D>();
	}
	return ResolveHeroPortrait(HeroData, BodyType, Variant);
}

TSoftObjectPtr<UTexture2D> UT66GameInstance::ResolveHeroPortrait(const FHeroData& HeroData, ET66BodyType BodyType, ET66HeroPortraitVariant Variant) const
{
	const bool bUseTypeB = (BodyType == ET66BodyType::TypeB);

	const TSoftObjectPtr<UTexture2D>& Half = bUseTypeB && !HeroData.PortraitTypeB.IsNull()
		? HeroData.PortraitTypeB
		: HeroData.Portrait;

	const TSoftObjectPtr<UTexture2D>& Low = bUseTypeB
		? HeroData.PortraitTypeBLow
		: HeroData.PortraitLow;

	const TSoftObjectPtr<UTexture2D>& Full = bUseTypeB
		? HeroData.PortraitTypeBFull
		: HeroData.PortraitFull;

	const TSoftObjectPtr<UTexture2D>& Invincible = bUseTypeB
		? HeroData.PortraitTypeBInvincible
		: HeroData.PortraitInvincible;

	switch (Variant)
	{
	case ET66HeroPortraitVariant::Low:
		if (!Low.IsNull()) return Low;
		if (!Half.IsNull()) return Half;
		return Full;

	case ET66HeroPortraitVariant::Full:
		if (!Full.IsNull()) return Full;
		if (!Half.IsNull()) return Half;
		return Low;

	case ET66HeroPortraitVariant::Invincible:
		if (!Invincible.IsNull()) return Invincible;
		if (HeroData.HeroID == FName(TEXT("Hero_1")))
		{
			return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(
				bUseTypeB
					? TEXT("/Game/UI/Sprites/Heroes/Hero_1/T_Hero_1_TypeB_Invincible.T_Hero_1_TypeB_Invincible")
					: TEXT("/Game/UI/Sprites/Heroes/Hero_1/T_Hero_1_TypeA_Invincible.T_Hero_1_TypeA_Invincible")));
		}
		if (!Full.IsNull()) return Full;
		if (!Half.IsNull()) return Half;
		return Low;

	case ET66HeroPortraitVariant::Half:
	default:
		if (!Half.IsNull()) return Half;
		if (!Full.IsNull()) return Full;
		return Low;
	}
}

void UT66GameInstance::ClearSelections()
{
	SelectedPartySize = ET66PartySize::Solo;
	SelectedHeroID = NAME_None;
	SelectedCompanionID = NAME_None;
	SelectedDifficulty = ET66Difficulty::Easy;
	MiniSelectedHeroID = NAME_None;
	MiniSelectedCompanionID = NAME_None;
	MiniSelectedDifficultyID = NAME_None;
	MiniSelectedIdolIDs.Reset();
	bMiniLoadFlow = false;
	bMiniIntermissionFlow = false;
	MiniIntermissionStateRevision = 0;
	MiniIntermissionStateJson.Reset();
	MiniIntermissionRequestRevision = 0;
	MiniIntermissionRequestJson.Reset();
	SelectedHeroBodyType = ET66BodyType::TypeA;
	SelectedCompanionBodyType = ET66BodyType::TypeA;
	ApplyConfiguredMainMapLayoutVariant();
	RestoreRememberedSelectionDefaults();
}

void UT66GameInstance::PersistRememberedSelectionDefaults()
{
	if (UT66AchievementsSubsystem* Achievements = GetSubsystem<UT66AchievementsSubsystem>())
	{
		Achievements->RememberLastSelectedLoadout(SelectedHeroID, SelectedCompanionID);
	}
}

void UT66GameInstance::RestoreRememberedSelectionDefaults()
{
	if (UT66AchievementsSubsystem* Achievements = GetSubsystem<UT66AchievementsSubsystem>())
	{
		const FName RememberedHeroID = Achievements->GetLastSelectedHeroID();
		if (!RememberedHeroID.IsNone())
		{
			FHeroData HeroData;
			if (GetHeroData(RememberedHeroID, HeroData))
			{
				SelectedHeroID = RememberedHeroID;
			}
		}

		const FName RememberedCompanionID = Achievements->GetLastSelectedCompanionID();
		if (RememberedCompanionID.IsNone())
		{
			SelectedCompanionID = NAME_None;
		}
		else
		{
			FCompanionData CompanionData;
			SelectedCompanionID = GetCompanionData(RememberedCompanionID, CompanionData)
				? RememberedCompanionID
				: NAME_None;
		}
	}
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
	OutBaseStats.Accuracy = 2;
	OutBaseStats.Armor = 2;
	OutBaseStats.Evasion = 2;
	OutBaseStats.Luck = 2;
	OutBaseStats.Speed = 2;

	OutPerLevelGains = FT66HeroPerLevelStatGains{};
	OutPerLevelGains.Damage = Range(1, 2);
	OutPerLevelGains.AttackSpeed = Range(1, 2);
	OutPerLevelGains.AttackScale = Range(1, 2);
	OutPerLevelGains.Accuracy = Range(1, 2);
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
		OutBaseStats.Accuracy    = FMath::Max(1, HD.BaseAccuracyStat);
		OutBaseStats.Armor       = FMath::Max(1, HD.BaseArmor);
		OutBaseStats.Evasion     = FMath::Max(1, HD.BaseEvasion);
		OutBaseStats.Luck        = FMath::Max(1, HD.BaseLuck);
		OutBaseStats.Speed       = FMath::Max(1, HD.BaseSpeed);

		OutPerLevelGains.Damage      = Range(HD.LvlDmgMin, HD.LvlDmgMax);
		OutPerLevelGains.AttackSpeed = Range(HD.LvlAtkSpdMin, HD.LvlAtkSpdMax);
		OutPerLevelGains.AttackScale = Range(HD.LvlAtkScaleMin, HD.LvlAtkScaleMax);
		OutPerLevelGains.Accuracy    = Range(HD.LvlAccuracyMin, HD.LvlAccuracyMax);
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
	Paths.Reserve(48);
	GameplayPreloadVisualIDs.Reset();

	auto AddPath = [&Paths](const FSoftObjectPath& Path)
	{
		if (!Path.IsNull())
		{
			Paths.AddUnique(Path);
		}
	};

	auto AddDifficultyThemeTextures = [&AddPath](const TCHAR* FolderName, const TCHAR* Suffix)
	{
		const FString BlockAssetName = FString::Printf(TEXT("T_MegabonkBlock_%s"), Suffix);
		const FString SlopeAssetName = FString::Printf(TEXT("T_MegabonkSlope_%s"), Suffix);
		AddPath(FSoftObjectPath(FString::Printf(
			TEXT("/Game/World/Terrain/MegabonkThemes/%s/%s.%s"),
			FolderName,
			*BlockAssetName,
			*BlockAssetName)));
		AddPath(FSoftObjectPath(FString::Printf(
			TEXT("/Game/World/Terrain/MegabonkThemes/%s/%s.%s"),
			FolderName,
			*SlopeAssetName,
			*SlopeAssetName)));
	};

	auto AddVisualAssets = [this, &AddPath](FName VisualID)
	{
		if (VisualID.IsNone())
		{
			return;
		}

		UDataTable* VisualsDT = GetCharacterVisualsDataTable();
		const FT66CharacterVisualRow* VisualRow = VisualsDT ? VisualsDT->FindRow<FT66CharacterVisualRow>(VisualID, TEXT("PreloadGameplayAssets")) : nullptr;
		if (!VisualRow)
		{
			return;
		}

		GameplayPreloadVisualIDs.AddUnique(VisualID);
		AddPath(VisualRow->SkeletalMesh.ToSoftObjectPath());
		AddPath(VisualRow->LoopingAnimation.ToSoftObjectPath());
		AddPath(VisualRow->AlertAnimation.ToSoftObjectPath());
		AddPath(VisualRow->RunAnimation.ToSoftObjectPath());
	};

	auto AddCurrentDifficultyThemeTextures = [this, &AddDifficultyThemeTextures]()
	{
		switch (SelectedDifficulty)
		{
		case ET66Difficulty::Medium:
			AddDifficultyThemeTextures(TEXT("VeryHardGraveyard"), TEXT("VeryHardGraveyard"));
			break;
		case ET66Difficulty::Hard:
			AddDifficultyThemeTextures(TEXT("ImpossibleNorthPole"), TEXT("ImpossibleNorthPole"));
			break;
		case ET66Difficulty::VeryHard:
			AddDifficultyThemeTextures(TEXT("PerditionMars"), TEXT("PerditionMars"));
			break;
		case ET66Difficulty::Impossible:
			AddDifficultyThemeTextures(TEXT("FinalHell"), TEXT("FinalHell"));
			break;
		case ET66Difficulty::Easy:
		default:
			break;
		}
	};

	// Engine cube mesh (used ~6 times in GameMode for walls/floors/arenas).
	AddPath(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube")));
	AddPath(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));

	// Main gameplay uses a dedicated terrain asset set. Preload the full terrain/prop contract
	// before opening the gameplay level so the first entry does not depend on cold material state.
	AddPath(FSoftObjectPath(TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/Megabonk/SM_MegabonkBlock.SM_MegabonkBlock")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/Megabonk/SM_MegabonkSlope.SM_MegabonkSlope")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkSlope.MI_MegabonkSlope")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkWall.MI_MegabonkWall")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/Megabonk/T_MegabonkBlock.T_MegabonkBlock")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/Megabonk/T_MegabonkSlope.T_MegabonkSlope")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/Megabonk/T_MegabonkWall.T_MegabonkWall")));
	AddCurrentDifficultyThemeTextures();
	AddPath(FSoftObjectPath(TEXT("/Engine/BasicShapes/Plane.Plane")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Grass.Grass")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Log.Log")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Grass/Materials/Material_0_014.Material_0_014")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Tree.Tree")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Tree2.Tree2")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Tree3.Tree3")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Rocks.Rocks")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Barn.Barn")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Boulder.Boulder")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Fence.Fence")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Fence2.Fence2")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Fence3.Fence3")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Haybell.Haybell")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Scarecrow.Scarecrow")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Silo.Silo")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Stump.Stump")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Troth.Troth")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Windmill.Windmill")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Tractor.Tractor")));

	AddVisualAssets(UT66CharacterVisualSubsystem::GetHeroVisualID(
		SelectedHeroID,
		SelectedHeroBodyType,
		SelectedHeroSkinID.IsNone() ? FName(TEXT("Default")) : SelectedHeroSkinID));

	AddVisualAssets(UT66CharacterVisualSubsystem::GetCompanionVisualID(SelectedCompanionID, FName(TEXT("Default"))));

	if (Paths.Num() <= 0)
	{
		if (OnComplete) OnComplete();
		return;
	}

	bGameplayAssetsPreloadInFlight = true;
	GameplayAssetsPreloadCallback = MoveTemp(OnComplete);

	UE_LOG(LogT66GameInstance, Log, TEXT("[LOAD] PreloadGameplayAssets queued %d transition assets."), Paths.Num());

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
	UE_LOG(LogT66GameInstance, Log, TEXT("[LOAD] Gameplay transition asset preload completed. Pre-resolving %d visuals."), GameplayPreloadVisualIDs.Num());
	if (UT66CharacterVisualSubsystem* Visuals = GetSubsystem<UT66CharacterVisualSubsystem>())
	{
		for (const FName VisualID : GameplayPreloadVisualIDs)
		{
			Visuals->PreloadCharacterVisual(VisualID);
		}
	}
	GameplayPreloadVisualIDs.Reset();
	if (GameplayAssetsPreloadCallback)
	{
		TFunction<void()> Cb = MoveTemp(GameplayAssetsPreloadCallback);
		Cb();
	}
}

void UT66GameInstance::ApplyConfiguredMainMapLayoutVariant()
{
	CurrentMainMapLayoutVariant = ET66MainMapLayoutVariant::Tower;
}

FName UT66GameInstance::GetFrontendLevelName()
{
	return FName(FrontendLevelName);
}

FName UT66GameInstance::GetLabLevelName()
{
	return FName(LabLevelName);
}

FName UT66GameInstance::GetGameplayLevelName()
{
	return FName(GameplayLevelName);
}

FName UT66GameInstance::GetTribulationEntryLevelName()
{
	if (bAlwaysRouteTribulationToTutorial)
	{
		return GetTutorialLevelName();
	}

	return GetGameplayLevelName();
}

FName UT66GameInstance::GetColiseumLevelName()
{
	return FName(ColiseumLevelName);
}

FName UT66GameInstance::GetTutorialLevelName()
{
	return FName(TutorialLevelName);
}

void UT66GameInstance::TransitionToGameplayLevel()
{
	UWorld* World = GetWorld();
	if (!World) return;

	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RetroFXSubsystem* RetroFX = GI->GetSubsystem<UT66RetroFXSubsystem>())
			{
				FT66RetroFXSettings GameplayRetroSettings;
				if (UT66PlayerSettingsSubsystem* PlayerSettings = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
				{
					GameplayRetroSettings = PlayerSettings->GetRetroFXSettings();
				}

				GameplayRetroSettings.bEnableWorldGeometry = false;
				GameplayRetroSettings.WorldVertexSnapPercent = 0.0f;
				GameplayRetroSettings.WorldVertexSnapResolutionPercent = 0.0f;
				GameplayRetroSettings.WorldVertexNoisePercent = 0.0f;
				GameplayRetroSettings.WorldAffineBlendPercent = 0.0f;
				GameplayRetroSettings.WorldAffineDistance1Percent = 0.0f;
				GameplayRetroSettings.WorldAffineDistance2Percent = 0.0f;
				GameplayRetroSettings.WorldAffineDistance3Percent = 0.0f;
				RetroFX->ApplySettings(GameplayRetroSettings, World);
			}
		}
	}

	ShowPersistentGameplayTransitionCurtain();

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
		UE_LOG(LogT66GameInstance, Log, TEXT("[LOAD] TransitionToGameplayLevel started pre-open asset preload."));
		PreloadGameplayAssets([this]()
		{
			const FName LevelToOpen = GetTribulationEntryLevelName();
			UE_LOG(LogT66GameInstance, Log, TEXT("[LOAD] TransitionToGameplayLevel opening %s."), *LevelToOpen.ToString());
			UGameplayStatics::OpenLevel(this, LevelToOpen);
		});
	}), 0.05f, false); // Small delay so the loading widget paints first.
}

void UT66GameInstance::ShowPersistentGameplayTransitionCurtain()
{
	if (PersistentGameplayTransitionCurtain.IsValid())
	{
		return;
	}

	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}

	SAssignNew(PersistentGameplayTransitionCurtain, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black);

	GEngine->GameViewport->AddViewportWidgetContent(
		PersistentGameplayTransitionCurtain.ToSharedRef(),
		9500);
}

void UT66GameInstance::HidePersistentGameplayTransitionCurtain()
{
	if (GEngine && GEngine->GameViewport && PersistentGameplayTransitionCurtain.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(PersistentGameplayTransitionCurtain.ToSharedRef());
	}

	PersistentGameplayTransitionCurtain.Reset();
}
