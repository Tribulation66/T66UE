// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66GameInstance.h"
#include "Core/T66DirectEntry.h"
#include "Core/T66ReleaseVariantSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66CharacterVisualSubsystem.h"
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
#include "Misc/CommandLine.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
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
		&& ItemID != FName(TEXT("Item_BackroomsQuickRevive"))
		&& ItemID != CloseRangeItemID
		&& ItemID != LongRangeItemID
			&& ItemID != SpinWheelItemID
			&& ItemID != MovementSpeedItemID;
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

	static const TCHAR* FrontendLevelName = TEXT("/Game/Maps/FrontendLevel");
	static const TCHAR* GameplayLevelName = TEXT("/Game/Maps/GameplayLevel");

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
		SetCVarInt(TEXT("r.DynamicRes.OperationMode"), 0);
		// AA/upscaler CVars are owned by Config/DefaultEngine.ini [SystemSettings].
		// Writing them here at SetByGameSetting priority only creates ignored-CVar warnings.
	}
}

UT66GameInstance::UT66GameInstance()
{
	WeaponsDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/DT_Weapons.DT_Weapons")));
	BossesDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/DT_Bosses.DT_Bosses")));
	StagesDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/DT_Stages.DT_Stages")));
	EnemiesDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/DT_Enemies.DT_Enemies")));
	StatusEffectsDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/DT_StatusEffects.DT_StatusEffects")));
	BossEncountersDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/DT_BossEncounters.DT_BossEncounters")));
	BossEncounterMembersDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/DT_BossEncounterMembers.DT_BossEncounterMembers")));
	ArcadeInteractablesDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/DT_ArcadeInteractables.DT_ArcadeInteractables")));
	NPCsDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/DT_NPCs.DT_NPCs")));
	UniqueEnemiesDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/DT_UniqueEnemies.DT_UniqueEnemies")));

	// Default selections
	SelectedPartySize = ET66PartySize::Solo;
	SelectedHeroID = NAME_None;
	SelectedCompanionID = NAME_None;
	SelectedDifficulty = ET66Difficulty::Easy;
	SelectedRunMode = ET66RunMode::Regular;
	SelectedRunCategory = ET66RunCategory::Tower;
	SelectedRunModifierKind = ET66RunModifierKind::None;
	SelectedRunModifierID = NAME_None;
	MiniSelectedHeroID = NAME_None;
	MiniSelectedCompanionID = NAME_None;
	MiniSelectedDifficultyID = NAME_None;
	MiniSelectedIdolIDs.Reset();
	bMiniLoadFlow = false;
	bMiniIntermissionFlow = false;
	SelectedHeroBodyType = ET66BodyType::Chad;
	SelectedCompanionBodyType = ET66BodyType::Chad;
}

void UT66GameInstance::Init()
{
	Super::Init();

	ApplyCrispRenderingDefaults();
	ApplyConfiguredMainMapLayoutVariant();
	RestoreRememberedSelectionDefaults();

	FT66DirectEntryRequest DirectEntryRequest;
	FString DirectEntryError;
	if (T66DirectEntry::TryParseCommandLine(DirectEntryRequest, DirectEntryError))
	{
		T66DirectEntry::ApplyRequestToGameInstance(*this, DirectEntryRequest);
	}
	else if (!DirectEntryError.IsEmpty())
	{
		UE_LOG(LogT66GameInstance, Error, TEXT("Direct entry command line rejected: %s"), *DirectEntryError);
		FPlatformMisc::RequestExitWithStatus(false, 68, TEXT("T66DirectEntryInvalid"));
		return;
	}

	// Preload core DataTables early, asynchronously, so we avoid sync loads later.
	PrimeCoreDataTablesAsync();
	PrimeCorePresentationAssetsAsync();

	// Preload the main-menu textures so they are often ready before BuildSlateUI binds brushes.
	if (UT66UITexturePoolSubsystem* TexPool = GetSubsystem<UT66UITexturePoolSubsystem>())
	{
		auto RequestFrontendTexture = [TexPool, this](const TCHAR* PackagePath, const TCHAR* ObjectPath, const TCHAR* RequestKey)
		{
			if (!PackagePath || !ObjectPath || !RequestKey || !FPackageName::DoesPackageExist(PackagePath))
			{
				return;
			}

			TexPool->RequestTexture(
				TSoftObjectPtr<UTexture2D>(FSoftObjectPath(ObjectPath)),
				this,
				FName(RequestKey),
				[](UTexture2D*) {});
		};

		const TSoftObjectPtr<UTexture2D> LBGlobal(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_LB_Global.T_LB_Global")));
		const TSoftObjectPtr<UTexture2D> LBFriends(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_LB_Friends.T_LB_Friends")));
		const TSoftObjectPtr<UTexture2D> LBStreamers(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_LB_Streamers.T_LB_Streamers")));
		TexPool->RequestTexture(LBGlobal, this, FName(TEXT("PreloadLBGlobal")), [](UTexture2D*) {});
		TexPool->RequestTexture(LBFriends, this, FName(TEXT("PreloadLBFriends")), [](UTexture2D*) {});
		TexPool->RequestTexture(LBStreamers, this, FName(TEXT("PreloadLBStreamers")), [](UTexture2D*) {});

		const TCHAR* const PowerUpStatueNames[] = {
			TEXT("forbidden_chad_left_arm"),
			TEXT("forbidden_chad_right_arm"),
			TEXT("forbidden_chad_head"),
			TEXT("forbidden_chad_torso"),
			TEXT("forbidden_chad_left_leg"),
			TEXT("forbidden_chad_right_leg")
		};

		for (const TCHAR* StatueName : PowerUpStatueNames)
		{
			const FString PackagePath = FString::Printf(TEXT("/Game/UI/PowerUp/Statues/forbidden_chad/%s"), StatueName);
			const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, StatueName);
			const FString RequestKey = FString::Printf(TEXT("PreloadPowerUp_%s"), StatueName);
			RequestFrontendTexture(*PackagePath, *ObjectPath, *RequestKey);
		}

		const TCHAR* const PowerUpBuffSlugs[] = {
			TEXT("aoe-damage"),
			TEXT("bounce-damage"),
			TEXT("pierce-damage"),
			TEXT("dot-damage"),
			TEXT("crit-damage"),
			TEXT("aoe-speed"),
			TEXT("bounce-speed"),
			TEXT("pierce-speed"),
			TEXT("dot-speed"),
			TEXT("crit-chance"),
			TEXT("aoe-scale"),
			TEXT("bounce-scale"),
			TEXT("pierce-scale"),
			TEXT("dot-scale"),
			TEXT("range"),
			TEXT("taunt"),
			TEXT("damage-reduction"),
			TEXT("damage-reflection"),
			TEXT("hp-regen"),
			TEXT("crush"),
			TEXT("dodge"),
			TEXT("counter-attack"),
			TEXT("life-steal"),
			TEXT("invisibility"),
			TEXT("assassinate"),
			TEXT("treasure-chest"),
			TEXT("cheating"),
			TEXT("stealing"),
			TEXT("loot-crate"),
			TEXT("alchemy"),
			TEXT("accuracy")
		};

		for (const TCHAR* BuffSlug : PowerUpBuffSlugs)
		{
			const FString PackagePath = FString::Printf(TEXT("/Game/UI/PowerUp/SecondaryBuffs/%s"), BuffSlug);
			const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, BuffSlug);
			const FString RequestKey = FString::Printf(TEXT("PreloadPowerUp_%s"), BuffSlug);
			RequestFrontendTexture(*PackagePath, *ObjectPath, *RequestKey);
		}
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
	Paths.Reserve(15);

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
	AddDT(WeaponsDataTable);
	AddDT(BossesDataTable);
	AddDT(StagesDataTable);
	AddDT(EnemiesDataTable);
	AddDT(StatusEffectsDataTable);
	AddDT(BossEncountersDataTable);
	AddDT(BossEncounterMembersDataTable);
	AddDT(NPCsDataTable);
	AddDT(LoanSharkDataTable);
	AddDT(UniqueEnemiesDataTable);
	AddDT(CharacterVisualsDataTable);
	AddDT(ArcadeInteractablesDataTable);

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
	CoreDataTablesLoadHandle.Reset();

	// Cache any tables that successfully loaded.
	if (!CachedHeroDataTable) CachedHeroDataTable = HeroDataTable.Get();
	if (!CachedCompanionDataTable) CachedCompanionDataTable = CompanionDataTable.Get();
	if (!CachedItemsDataTable) CachedItemsDataTable = ItemsDataTable.Get();
	if (!CachedIdolsDataTable) CachedIdolsDataTable = IdolsDataTable.Get();
	if (!CachedWeaponsDataTable) CachedWeaponsDataTable = WeaponsDataTable.Get();
	if (!CachedBossesDataTable) CachedBossesDataTable = BossesDataTable.Get();
	if (!CachedStagesDataTable) CachedStagesDataTable = StagesDataTable.Get();
	if (!CachedEnemiesDataTable) CachedEnemiesDataTable = EnemiesDataTable.Get();
	if (!CachedStatusEffectsDataTable) CachedStatusEffectsDataTable = StatusEffectsDataTable.Get();
	if (!CachedBossEncountersDataTable) CachedBossEncountersDataTable = BossEncountersDataTable.Get();
	if (!CachedBossEncounterMembersDataTable) CachedBossEncounterMembersDataTable = BossEncounterMembersDataTable.Get();
	if (!CachedNPCsDataTable) CachedNPCsDataTable = NPCsDataTable.Get();
	if (!CachedLoanSharkDataTable) CachedLoanSharkDataTable = LoanSharkDataTable.Get();
	if (!CachedUniqueEnemiesDataTable) CachedUniqueEnemiesDataTable = UniqueEnemiesDataTable.Get();
	if (!CachedCharacterVisualsDataTable) CachedCharacterVisualsDataTable = CharacterVisualsDataTable.Get();
	if (!CachedArcadeInteractablesDataTable) CachedArcadeInteractablesDataTable = ArcadeInteractablesDataTable.Get();

	bCoreDataTablesLoaded = true;
	PrimeHeroSelectionAssetsAsync();

	if (bGameplayPreloadWaitingOnCoreTables)
	{
		bGameplayPreloadWaitingOnCoreTables = false;
		bGameplayAssetsPreloadInFlight = false;
		TFunction<void()> DeferredCallback = MoveTemp(GameplayAssetsPreloadCallback);
		PreloadGameplayAssets(MoveTemp(DeferredCallback));
		return;
	}
}

void UT66GameInstance::PrimeCorePresentationAssetsAsync()
{
	if (bCorePresentationAssetsLoaded || bCorePresentationAssetsLoadRequested)
	{
		return;
	}

	TArray<FSoftObjectPath> Paths;
	Paths.Reserve(3);
	Paths.AddUnique(FSoftObjectPath(TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle")));
	Paths.AddUnique(FSoftObjectPath(TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1")));

	Paths.RemoveAll([](const FSoftObjectPath& Path)
	{
		return Path.IsNull();
	});

	if (Paths.Num() <= 0)
	{
		bCorePresentationAssetsLoaded = true;
		return;
	}

	bCorePresentationAssetsLoadRequested = true;
	CorePresentationAssetsLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		Paths,
		FStreamableDelegate::CreateUObject(this, &UT66GameInstance::HandleCorePresentationAssetsLoaded));

	if (!CorePresentationAssetsLoadHandle.IsValid())
	{
		HandleCorePresentationAssetsLoaded();
		return;
	}

	UE_LOG(LogT66GameInstance, Log, TEXT("[LOAD] PrimeCorePresentationAssetsAsync queued %d startup combat-presentation assets."), Paths.Num());
}

void UT66GameInstance::HandleCorePresentationAssetsLoaded()
{
	CorePresentationAssetsLoadHandle.Reset();
	bCorePresentationAssetsLoaded = true;
	UE_LOG(LogT66GameInstance, Log, TEXT("[LOAD] Startup combat-presentation asset warmup completed."));
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
		AddPath(VisualRow->StaticMesh.ToSoftObjectPath());
		AddPath(VisualRow->PixelatedTextureAssetPath.ToSoftObjectPath());
		AddPath(VisualRow->WalkAnimation.ToSoftObjectPath());
		AddPath(VisualRow->IdleAnimation.ToSoftObjectPath());
		AddPath(VisualRow->JumpAnimation.ToSoftObjectPath());
		AddPath(VisualRow->RollAnimation.ToSoftObjectPath());
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
		AddPath(HeroRow->PortraitStacy.ToSoftObjectPath());
		AddPath(HeroRow->PortraitLow.ToSoftObjectPath());
		AddPath(HeroRow->PortraitFull.ToSoftObjectPath());
		AddPath(HeroRow->PortraitStacyLow.ToSoftObjectPath());
		AddPath(HeroRow->PortraitStacyFull.ToSoftObjectPath());
		AddPath(HeroRow->PortraitInvincible.ToSoftObjectPath());
		AddPath(HeroRow->PortraitStacyInvincible.ToSoftObjectPath());
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
		AddPath(VisualRow->StaticMesh.ToSoftObjectPath());
		AddPath(VisualRow->PixelatedTextureAssetPath.ToSoftObjectPath());
		AddPath(VisualRow->WalkAnimation.ToSoftObjectPath());
		AddPath(VisualRow->IdleAnimation.ToSoftObjectPath());
		AddPath(VisualRow->JumpAnimation.ToSoftObjectPath());
		AddPath(VisualRow->RollAnimation.ToSoftObjectPath());
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

UDataTable* UT66GameInstance::ResolveCachedDataTable(TObjectPtr<UDataTable>& Cached, const TSoftObjectPtr<UDataTable>& Soft)
{
	if (Cached || Soft.IsNull())
	{
		return Cached;
	}

	Cached = Soft.Get();
	if (Cached)
	{
		return Cached;
	}

	PrimeCoreDataTablesAsync();

	if (CoreDataTablesLoadHandle.IsValid() && !CoreDataTablesLoadHandle->HasLoadCompleted())
	{
		const double WaitStartSeconds = FPlatformTime::Seconds();
		CoreDataTablesLoadHandle->WaitUntilComplete();
		const double WaitMs = (FPlatformTime::Seconds() - WaitStartSeconds) * 1000.0;
		if (WaitMs > 5.0)
		{
			UE_LOG(
				LogT66GameInstance,
				Log,
				TEXT("[LOAD] ResolveCachedDataTable waited %.1fms for the existing core DataTable preload handle (%s)."),
				WaitMs,
				*Soft.ToString());
		}
	}

	Cached = Soft.Get();
	if (!Cached)
	{
		UE_LOG(
			LogT66GameInstance,
			Warning,
			TEXT("[LOAD] Core DataTable was requested before the async preload made it available: %s"),
			*Soft.ToString());
	}
	return Cached;
}

UDataTable* UT66GameInstance::GetHeroDataTable() { return ResolveCachedDataTable(CachedHeroDataTable, HeroDataTable); }
UDataTable* UT66GameInstance::GetCompanionDataTable() { return ResolveCachedDataTable(CachedCompanionDataTable, CompanionDataTable); }
UDataTable* UT66GameInstance::GetIdolsDataTable() { return ResolveCachedDataTable(CachedIdolsDataTable, IdolsDataTable); }
UDataTable* UT66GameInstance::GetWeaponsDataTable() { return ResolveCachedDataTable(CachedWeaponsDataTable, WeaponsDataTable); }

bool UT66GameInstance::GetHeroData(FName HeroID, FHeroData& OutHeroData)
{
	return FindDataRow(GetHeroDataTable(), HeroID, OutHeroData, TEXT("GetHeroData"));
}

bool UT66GameInstance::GetCompanionData(FName CompanionID, FCompanionData& OutCompanionData)
{
	return FindDataRow(GetCompanionDataTable(), CompanionID, OutCompanionData, TEXT("GetCompanionData"), /*bRequireValidID=*/false);
}

bool UT66GameInstance::GetArcadeInteractableData(FName ArcadeRowID, FT66ArcadeInteractableData& OutArcadeData)
{
	if (ArcadeRowID.IsNone())
	{
		return false;
	}

	UDataTable* DataTable = GetArcadeInteractablesDataTable();
	if (!DataTable)
	{
		return false;
	}

	if (const FT66ArcadeInteractableRow* FoundRow = DataTable->FindRow<FT66ArcadeInteractableRow>(ArcadeRowID, TEXT("GetArcadeInteractableData")))
	{
		OutArcadeData = FoundRow->ArcadeData;
		if (OutArcadeData.ArcadeID.IsNone())
		{
			OutArcadeData.ArcadeID = ArcadeRowID;
		}
		return true;
	}

	return false;
}

UDataTable* UT66GameInstance::GetItemsDataTable() { return ResolveCachedDataTable(CachedItemsDataTable, ItemsDataTable); }

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

UDataTable* UT66GameInstance::GetBossesDataTable() { return ResolveCachedDataTable(CachedBossesDataTable, BossesDataTable); }
UDataTable* UT66GameInstance::GetStagesDataTable() { return ResolveCachedDataTable(CachedStagesDataTable, StagesDataTable); }
UDataTable* UT66GameInstance::GetEnemiesDataTable() { return ResolveCachedDataTable(CachedEnemiesDataTable, EnemiesDataTable); }
UDataTable* UT66GameInstance::GetStatusEffectsDataTable() { return ResolveCachedDataTable(CachedStatusEffectsDataTable, StatusEffectsDataTable); }
UDataTable* UT66GameInstance::GetBossEncountersDataTable() { return ResolveCachedDataTable(CachedBossEncountersDataTable, BossEncountersDataTable); }
UDataTable* UT66GameInstance::GetBossEncounterMembersDataTable() { return ResolveCachedDataTable(CachedBossEncounterMembersDataTable, BossEncounterMembersDataTable); }
UDataTable* UT66GameInstance::GetNPCsDataTable() { return ResolveCachedDataTable(CachedNPCsDataTable, NPCsDataTable); }
UDataTable* UT66GameInstance::GetLoanSharkDataTable() { return ResolveCachedDataTable(CachedLoanSharkDataTable, LoanSharkDataTable); }
UDataTable* UT66GameInstance::GetUniqueEnemiesDataTable() { return ResolveCachedDataTable(CachedUniqueEnemiesDataTable, UniqueEnemiesDataTable); }
UDataTable* UT66GameInstance::GetCharacterVisualsDataTable() { return ResolveCachedDataTable(CachedCharacterVisualsDataTable, CharacterVisualsDataTable); }
UDataTable* UT66GameInstance::GetArcadeInteractablesDataTable() { return ResolveCachedDataTable(CachedArcadeInteractablesDataTable, ArcadeInteractablesDataTable); }

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
	return FindDataRow(GetIdolsDataTable(), IdolID, OutIdolData, TEXT("GetIdolData"));
}

bool UT66GameInstance::GetWeaponData(FName WeaponID, FWeaponData& OutWeaponData)
{
	return FindDataRow(GetWeaponsDataTable(), WeaponID, OutWeaponData, TEXT("GetWeaponData"));
}

bool UT66GameInstance::GetBossData(FName BossID, FBossData& OutBossData)
{
	return FindDataRow(GetBossesDataTable(), BossID, OutBossData, TEXT("GetBossData"), /*bRequireValidID=*/false);
}

bool UT66GameInstance::GetStageData(int32 StageNumber, FStageData& OutStageData)
{
	const FName RowName(*FString::Printf(TEXT("Stage_%02d"), StageNumber));
	return FindDataRow(GetStagesDataTable(), RowName, OutStageData, TEXT("GetStageData"), /*bRequireValidID=*/false);
}

bool UT66GameInstance::GetEnemyData(FName EnemyID, FT66EnemyData& OutEnemyData)
{
	return FindDataRow(GetEnemiesDataTable(), EnemyID, OutEnemyData, TEXT("GetEnemyData"));
}

bool UT66GameInstance::GetStatusEffectData(FName StatusEffectID, FT66StatusEffectData& OutStatusEffectData)
{
	return FindDataRow(GetStatusEffectsDataTable(), StatusEffectID, OutStatusEffectData, TEXT("GetStatusEffectData"));
}

bool UT66GameInstance::GetBossEncounterData(FName BossEncounterID, FT66BossEncounterData& OutEncounterData)
{
	return FindDataRow(GetBossEncountersDataTable(), BossEncounterID, OutEncounterData, TEXT("GetBossEncounterData"));
}

void UT66GameInstance::GetBossEncounterMemberData(FName BossEncounterID, TArray<FT66BossEncounterMemberData>& OutMembers)
{
	OutMembers.Reset();
	if (BossEncounterID.IsNone())
	{
		return;
	}

	UDataTable* MembersTable = GetBossEncounterMembersDataTable();
	if (!MembersTable)
	{
		return;
	}

	for (const FName RowName : MembersTable->GetRowNames())
	{
		const FT66BossEncounterMemberData* Row = MembersTable->FindRow<FT66BossEncounterMemberData>(RowName, TEXT("GetBossEncounterMemberData"));
		if (Row && Row->BossEncounterID == BossEncounterID && !Row->BossID.IsNone())
		{
			OutMembers.Add(*Row);
		}
	}

	OutMembers.Sort([](const FT66BossEncounterMemberData& A, const FT66BossEncounterMemberData& B)
	{
		return A.MemberIndex < B.MemberIndex;
	});
}

bool UT66GameInstance::GetNPCData(FName NPCID, FHouseNPCData& OutNPCData)
{
	return FindDataRow(GetNPCsDataTable(), NPCID, OutNPCData, TEXT("GetNPCData"));
}

bool UT66GameInstance::GetLoanSharkData(FName LoanSharkID, FLoanSharkData& OutData)
{
	return FindDataRow(GetLoanSharkDataTable(), LoanSharkID, OutData, TEXT("GetLoanSharkData"));
}

bool UT66GameInstance::GetUniqueEnemyData(FName UniqueEnemyID, FUniqueEnemyData& OutData)
{
	return FindDataRow(GetUniqueEnemiesDataTable(), UniqueEnemyID, OutData, TEXT("GetUniqueEnemyData"));
}

TArray<FName> UT66GameInstance::GetAllHeroIDs()
{
	UDataTable* DataTable = GetHeroDataTable();
	return DataTable ? DataTable->GetRowNames() : TArray<FName>();
}

TArray<FName> UT66GameInstance::GetPlayableHeroIDs()
{
	const TArray<FName> AllHeroIDs = GetAllHeroIDs();
	if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetSubsystem<UT66ReleaseVariantSubsystem>())
	{
		return ReleaseVariant->FilterHeroIDs(AllHeroIDs);
	}
	return AllHeroIDs;
}

bool UT66GameInstance::IsHeroPlayable(FName HeroID) const
{
	if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetSubsystem<UT66ReleaseVariantSubsystem>())
	{
		return ReleaseVariant->IsHeroAllowed(HeroID);
	}
	return true;
}

FName UT66GameInstance::ResolvePlayableHeroID(FName HeroID)
{
	if (!HeroID.IsNone() && IsHeroPlayable(HeroID))
	{
		return HeroID;
	}

	const TArray<FName> PlayableHeroIDs = GetPlayableHeroIDs();
	return PlayableHeroIDs.Num() > 0 ? PlayableHeroIDs[0] : NAME_None;
}

TArray<ET66Difficulty> UT66GameInstance::GetPlayableDifficulties() const
{
	if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetSubsystem<UT66ReleaseVariantSubsystem>())
	{
		return ReleaseVariant->GetPlayableDifficulties();
	}

	return {
		ET66Difficulty::Easy,
		ET66Difficulty::Medium,
		ET66Difficulty::Hard,
		ET66Difficulty::VeryHard,
		ET66Difficulty::Impossible
	};
}

TArray<ET66Difficulty> UT66GameInstance::GetVisibleDifficulties() const
{
	if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetSubsystem<UT66ReleaseVariantSubsystem>())
	{
		return ReleaseVariant->GetVisibleDifficulties();
	}

	return {
		ET66Difficulty::Easy,
		ET66Difficulty::Medium,
		ET66Difficulty::Hard,
		ET66Difficulty::VeryHard,
		ET66Difficulty::Impossible
	};
}

bool UT66GameInstance::IsDifficultyPlayable(ET66Difficulty Difficulty) const
{
	if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetSubsystem<UT66ReleaseVariantSubsystem>())
	{
		return ReleaseVariant->IsDifficultyAllowed(Difficulty);
	}
	return true;
}

ET66Difficulty UT66GameInstance::ResolvePlayableDifficulty(ET66Difficulty Difficulty) const
{
	if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetSubsystem<UT66ReleaseVariantSubsystem>())
	{
		return ReleaseVariant->ResolvePlayableDifficulty(Difficulty);
	}
	return Difficulty;
}

bool UT66GameInstance::IsRunCategoryPlayable(const ET66RunCategory RunCategory) const
{
	if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetSubsystem<UT66ReleaseVariantSubsystem>())
	{
		return ReleaseVariant->IsRunCategoryAllowed(RunCategory);
	}
	return true;
}

ET66RunCategory UT66GameInstance::ResolvePlayableRunCategory(const ET66RunCategory RunCategory) const
{
	if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetSubsystem<UT66ReleaseVariantSubsystem>())
	{
		return ReleaseVariant->ResolvePlayableRunCategory(RunCategory);
	}
	return RunCategory;
}

bool UT66GameInstance::IsCollectorPlayable() const
{
	if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetSubsystem<UT66ReleaseVariantSubsystem>())
	{
		return ReleaseVariant->IsCollectorAllowed();
	}
	return true;
}

TArray<FName> UT66GameInstance::GetAllCompanionIDs()
{
	UDataTable* DataTable = GetCompanionDataTable();
	return DataTable ? DataTable->GetRowNames() : TArray<FName>();
}

TArray<FName> UT66GameInstance::GetPlayableCompanionIDs()
{
	const TArray<FName> AllCompanionIDs = GetAllCompanionIDs();
	if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetSubsystem<UT66ReleaseVariantSubsystem>())
	{
		return ReleaseVariant->FilterCompanionIDs(AllCompanionIDs);
	}
	return AllCompanionIDs;
}

bool UT66GameInstance::IsCompanionPlayable(FName CompanionID) const
{
	if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetSubsystem<UT66ReleaseVariantSubsystem>())
	{
		return ReleaseVariant->IsCompanionAllowed(CompanionID);
	}
	return true;
}

FName UT66GameInstance::ResolvePlayableCompanionID(FName CompanionID) const
{
	return (CompanionID.IsNone() || IsCompanionPlayable(CompanionID))
		? CompanionID
		: NAME_None;
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
	const bool bUseStacyPortrait = T66BodyTypeAliases::IsStacy(BodyType);

	const TSoftObjectPtr<UTexture2D>& Half = bUseStacyPortrait && !HeroData.PortraitStacy.IsNull()
		? HeroData.PortraitStacy
		: HeroData.Portrait;

	const TSoftObjectPtr<UTexture2D>& Low = bUseStacyPortrait
		? HeroData.PortraitStacyLow
		: HeroData.PortraitLow;

	const TSoftObjectPtr<UTexture2D>& Full = bUseStacyPortrait
		? HeroData.PortraitStacyFull
		: HeroData.PortraitFull;

	const TSoftObjectPtr<UTexture2D>& Invincible = bUseStacyPortrait
		? HeroData.PortraitStacyInvincible
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
	SelectedHeroBodyType = ET66BodyType::Chad;
	SelectedCompanionBodyType = ET66BodyType::Chad;
	ClearActiveDailyClimbRun();
	ApplyConfiguredMainMapLayoutVariant();
	RestoreRememberedSelectionDefaults();
}

void UT66GameInstance::BeginDailyClimbRun(const FT66DailyClimbChallengeData& Challenge)
{
	CachedDailyClimbChallenge = Challenge;
	ActiveDailyClimbChallenge = Challenge;
	SelectedRunMode = Challenge.IsValid() ? ET66RunMode::DailyClimb : ET66RunMode::Regular;
	SelectedRunCategory = ET66RunCategory::Tower;
	if (!Challenge.IsValid())
	{
		return;
	}

	SelectedPartySize = ET66PartySize::Solo;
	SelectedHeroID = ResolvePlayableHeroID(Challenge.HeroID);
	SelectedCompanionID = NAME_None;
	SelectedDifficulty = ResolvePlayableDifficulty(Challenge.Difficulty);
	SelectedRunModifierKind = ET66RunModifierKind::None;
	SelectedRunModifierID = NAME_None;
	SelectedHeroBodyType = ET66BodyType::Chad;
	SelectedCompanionBodyType = ET66BodyType::Chad;
	bIsNewGameFlow = true;
	bIsStageTransition = false;
	bRunIneligibleForLeaderboard = false;
	RunSeed = Challenge.RunSeed;
}

void UT66GameInstance::ClearActiveDailyClimbRun()
{
	ActiveDailyClimbChallenge = FT66DailyClimbChallengeData{};
	if (SelectedRunMode == ET66RunMode::DailyClimb)
	{
		SelectedRunMode = ET66RunMode::Regular;
	}
}

int32 UT66GameInstance::GetDailyClimbIntRuleValue(const ET66DailyClimbRuleType RuleType, const int32 DefaultValue) const
{
	return IsDailyClimbRun() && ActiveDailyClimbChallenge.IsValid()
		? ActiveDailyClimbChallenge.FindIntRuleValue(RuleType, DefaultValue)
		: DefaultValue;
}

float UT66GameInstance::GetDailyClimbFloatRuleValue(const ET66DailyClimbRuleType RuleType, const float DefaultValue) const
{
	return IsDailyClimbRun() && ActiveDailyClimbChallenge.IsValid()
		? ActiveDailyClimbChallenge.FindFloatRuleValue(RuleType, DefaultValue)
		: DefaultValue;
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
			if (IsHeroPlayable(RememberedHeroID) && GetHeroData(RememberedHeroID, HeroData))
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
			SelectedCompanionID = IsCompanionPlayable(RememberedCompanionID) && GetCompanionData(RememberedCompanionID, CompanionData)
				? RememberedCompanionID
				: NAME_None;
		}
	}
}

bool UT66GameInstance::GetHeroStatTuning(FName HeroID, FT66HeroStatBlock& OutBaseStats, FT66HeroPerLevelStatGains& OutPerLevelGains) const
{
	auto Range = [](const float Min, const float Max) -> FT66HeroStatGainRange
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
	OutPerLevelGains.Damage = Range(0.5f, 1.0f);
	OutPerLevelGains.AttackSpeed = Range(0.2f, 0.4f);
	OutPerLevelGains.AttackScale = Range(0.2f, 0.4f);
	OutPerLevelGains.Accuracy = Range(0.2f, 0.4f);
	OutPerLevelGains.Armor = Range(0.2f, 0.4f);
	OutPerLevelGains.Evasion = Range(0.2f, 0.4f);
	OutPerLevelGains.Luck = Range(0.2f, 0.4f);
	OutPerLevelGains.Speed = Range(0.2f, 0.4f);

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
		OutPerLevelGains.Speed       = Range(HD.LvlSpeedMin, HD.LvlSpeedMax);
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

	if (!bCoreDataTablesLoaded)
	{
		PrimeCoreDataTablesAsync();
		if (!bCoreDataTablesLoaded && CoreDataTablesLoadHandle.IsValid())
		{
			bGameplayAssetsPreloadInFlight = true;
			bGameplayPreloadWaitingOnCoreTables = true;
			GameplayAssetsPreloadCallback = MoveTemp(OnComplete);
			UE_LOG(LogT66GameInstance, Log, TEXT("[LOAD] Gameplay transition preload is waiting on core DataTables before gathering gameplay assets."));
			return;
		}
	}

	TArray<FSoftObjectPath> Paths;
	Paths.Reserve(128);
	GameplayPreloadVisualIDs.Reset();
	bGameplayVisualAssetsPhaseQueued = false;

	auto AddPath = [&Paths](const FSoftObjectPath& Path)
	{
		if (!Path.IsNull())
		{
			Paths.AddUnique(Path);
		}
	};

	auto AddAllCombatEffectAssets = [&AddPath]()
	{
		static const TCHAR* CombatEffectPaths[] = {
			TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1"),
			TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Portal.P_Cosmic_Portal"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Fire.P_Fire"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Poison_02.P_Poison_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Liquid_Hit_03.P_Liquid_Hit_03"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/P_Electric_Projectile_02.P_Electric_Projectile_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Ice_Projectile_02.P_Ice_Projectile_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Projectile_02.P_Cosmic_Projectile_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Projectile_03.P_Cosmic_Projectile_03"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Dirt_Spikes_02.P_Dirt_Spikes_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/P_Splash_02.P_Splash_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/P_Laser_02.P_Laser_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_4/P_Weapon_01.P_Weapon_01"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Web_Projectile_01.P_Web_Projectile_01"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_4/P_Weapon_02.P_Weapon_02"),
			TEXT("/Game/Stylized_VFX_StPack/Blueprints/BP_Storm.BP_Storm_C")
		};

		for (const TCHAR* AssetPath : CombatEffectPaths)
		{
			AddPath(FSoftObjectPath(AssetPath));
		}
	};

	auto AddCoherentThemeKitAssets = [&AddPath]()
	{
		static const TCHAR* ModuleIds[] = {
			TEXT("DungeonWall_TorchSconce_A"),
			TEXT("DungeonWall_StoneBlocks_A"),
			TEXT("DungeonWall_Chains_A"),
			TEXT("DungeonWall_BonesNiche_A"),
			TEXT("DungeonFloor_StoneSlabs_A"),
			TEXT("DungeonFloor_Drain_A"),
			TEXT("DungeonFloor_Cracked_A"),
			TEXT("DungeonFloor_Bones_A"),
			TEXT("ForestWall_VineTotem_A"),
			TEXT("ForestWall_TrunkWeave_A"),
			TEXT("ForestWall_RootBraid_A"),
			TEXT("ForestWall_MushroomBark_A"),
			TEXT("ForestFloor_RootMat_A"),
			TEXT("ForestFloor_MossStone_A"),
			TEXT("ForestFloor_LeafCrack_A"),
			TEXT("ForestFloor_BrambleEdge_A"),
			TEXT("OceanWall_CoralReef_A"),
			TEXT("OceanWall_ShellLimestone_A"),
			TEXT("OceanWall_KelpCoral_A"),
			TEXT("OceanWall_ReefRuin_A"),
			TEXT("OceanFloor_ReefStone_A"),
			TEXT("OceanFloor_ShellSand_A"),
			TEXT("OceanFloor_CoralCrack_A"),
			TEXT("OceanFloor_TidePool_A"),
			TEXT("MartianWall_RuinPanel_A"),
			TEXT("MartianWall_RedRock_A"),
			TEXT("MartianWall_MeteorScar_A"),
			TEXT("MartianWall_CrystalVein_A"),
			TEXT("MartianFloor_RuinTile_A"),
			TEXT("MartianFloor_RegolithPlates_A"),
			TEXT("MartianFloor_CrystalDust_A"),
			TEXT("MartianFloor_CraterCracks_A"),
			TEXT("HellWall_SpikeBasalt_A"),
			TEXT("HellWall_LavaCrack_A"),
			TEXT("HellWall_ChainsSkulls_A"),
			TEXT("HellWall_Brimstone_A"),
			TEXT("HellFloor_RunePlate_A"),
			TEXT("HellFloor_Obsidian_A"),
			TEXT("HellFloor_EmberFissure_A"),
			TEXT("HellFloor_BoneAsh_A"),
		};

		for (const TCHAR* ModuleId : ModuleIds)
		{
			const FString MeshPath = FString::Printf(
				TEXT("/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/%s_UnrealReady.%s_UnrealReady"),
				ModuleId,
				ModuleId);
			AddPath(FSoftObjectPath(MeshPath));
		}
	};

	auto AddAllTowerThemeAssets = [&AddPath, &AddCoherentThemeKitAssets]()
	{
		AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/TowerForest/MI_TowerForestGround.MI_TowerForestGround")));
		AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/TowerForest/MI_TowerForestRoof.MI_TowerForestRoof")));
		AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/TowerForest/T_TowerForestGround.T_TowerForestGround")));
		AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/TowerForest/T_TowerForestRoof.T_TowerForestRoof")));
		AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof.MI_TowerDungeonRoof")));
		AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/TowerDungeon/T_TowerDungeonRoof.T_TowerDungeonRoof")));
		AddCoherentThemeKitAssets();
		AddPath(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile1.MI_HillTile1")));
		AddPath(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile2.MI_HillTile2")));
		AddPath(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile3.MI_HillTile3")));
		AddPath(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile4.MI_HillTile4")));
	};

	auto AddVisualAssets = [this, &AddPath](FName VisualID)
	{
		if (VisualID.IsNone())
		{
			return;
		}

		GameplayPreloadVisualIDs.AddUnique(VisualID);

		UDataTable* VisualsDT = CachedCharacterVisualsDataTable.Get();
		if (!VisualsDT)
		{
			VisualsDT = CharacterVisualsDataTable.Get();
		}
		if (!VisualsDT && bCoreDataTablesLoaded)
		{
			VisualsDT = GetCharacterVisualsDataTable();
		}
		FName ResolvedVisualID = VisualID;
		const FT66CharacterVisualRow* VisualRow = VisualsDT ? VisualsDT->FindRow<FT66CharacterVisualRow>(ResolvedVisualID, TEXT("PreloadGameplayAssets")) : nullptr;
		if (!VisualRow)
		{
			const FName FallbackVisualID = UT66CharacterVisualSubsystem::GetFallbackVisualID(VisualID);
			if (!FallbackVisualID.IsNone())
			{
				ResolvedVisualID = FallbackVisualID;
				VisualRow = VisualsDT ? VisualsDT->FindRow<FT66CharacterVisualRow>(ResolvedVisualID, TEXT("PreloadGameplayAssetsFallback")) : nullptr;
			}
		}

		if (!VisualRow)
		{
			return;
		}

		TArray<FSoftObjectPath> VisualPreloadPaths;
		UT66CharacterVisualSubsystem::AppendCharacterVisualPreloadPaths(*VisualRow, VisualPreloadPaths);
		for (const FSoftObjectPath& VisualPath : VisualPreloadPaths)
		{
			AddPath(VisualPath);
		}
	};

	// Engine cube mesh (used ~6 times in GameMode for walls/floors/arenas).
	AddPath(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube")));
	AddPath(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));

	// Main gameplay uses a dedicated terrain asset set. Preload the full terrain contract
	// before opening the gameplay level so the first entry does not depend on cold material state.
	AddPath(CharacterVisualsDataTable.ToSoftObjectPath());
	AddPath(FSoftObjectPath(TEXT("/Game/Materials/M_Environment_Lit.M_Environment_Lit")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof.MI_TowerDungeonRoof")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/TowerDungeon/T_TowerDungeonRoof.T_TowerDungeonRoof")));
	AddCoherentThemeKitAssets();
	AddPath(FSoftObjectPath(TEXT("/Engine/BasicShapes/Plane.Plane")));
	AddAllTowerThemeAssets();
	AddAllCombatEffectAssets();

	AddVisualAssets(UT66CharacterVisualSubsystem::GetHeroVisualID(
		SelectedHeroID,
		SelectedHeroBodyType,
		SelectedHeroSkinID.IsNone() ? FName(TEXT("Default")) : SelectedHeroSkinID));

	AddVisualAssets(UT66CharacterVisualSubsystem::GetCompanionVisualID(SelectedCompanionID, FName(TEXT("Default"))));

	if (UT66RunStateSubsystem* RunState = GetSubsystem<UT66RunStateSubsystem>())
	{
		FStageData StageData;
		if (GetStageData(RunState->GetCurrentStage(), StageData))
		{
			AddVisualAssets(StageData.EnemyA);
			AddVisualAssets(StageData.EnemyB);
			AddVisualAssets(StageData.EnemyC);
			AddVisualAssets(StageData.EnemyD);
			AddVisualAssets(StageData.EnemyE);
			AddVisualAssets(StageData.EnemyF);
			AddVisualAssets(StageData.EnemyG);
			AddVisualAssets(StageData.EnemyH);
			AddVisualAssets(StageData.EnemyI);
			AddVisualAssets(StageData.EnemyJ);
			AddVisualAssets(StageData.BossID);

			TArray<FT66BossEncounterMemberData> EncounterMembers;
			if (!StageData.BossEncounterID.IsNone())
			{
				GetBossEncounterMemberData(StageData.BossEncounterID, EncounterMembers);
				for (const FT66BossEncounterMemberData& Member : EncounterMembers)
				{
					AddVisualAssets(Member.BossID);
				}
			}
		}
	}

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

bool UT66GameInstance::QueueGameplayVisualAssetPreload()
{
	if (GameplayPreloadVisualIDs.Num() <= 0)
	{
		return false;
	}

	UDataTable* VisualsDT = CachedCharacterVisualsDataTable.Get();
	if (!VisualsDT)
	{
		VisualsDT = CharacterVisualsDataTable.Get();
	}
	if (!VisualsDT)
	{
		UE_LOG(LogT66GameInstance, Warning, TEXT("[LOAD] Gameplay preload skipped visual-asset phase because the character visuals table is not loaded yet."));
		return false;
	}

	TArray<FSoftObjectPath> VisualPaths;
	VisualPaths.Reserve(GameplayPreloadVisualIDs.Num() * 8);

	for (const FName VisualID : GameplayPreloadVisualIDs)
	{
		FName ResolvedVisualID = VisualID;
		const FT66CharacterVisualRow* VisualRow = VisualsDT->FindRow<FT66CharacterVisualRow>(ResolvedVisualID, TEXT("QueueGameplayVisualAssetPreload"));
		if (!VisualRow)
		{
			const FName FallbackVisualID = UT66CharacterVisualSubsystem::GetFallbackVisualID(VisualID);
			if (!FallbackVisualID.IsNone())
			{
				ResolvedVisualID = FallbackVisualID;
				VisualRow = VisualsDT->FindRow<FT66CharacterVisualRow>(ResolvedVisualID, TEXT("QueueGameplayVisualAssetPreloadFallback"));
			}
		}

		if (!VisualRow)
		{
			continue;
		}

		UT66CharacterVisualSubsystem::AppendCharacterVisualPreloadPaths(*VisualRow, VisualPaths);
	}

	for (int32 Index = VisualPaths.Num() - 1; Index >= 0; --Index)
	{
		if (VisualPaths[Index].ResolveObject())
		{
			VisualPaths.RemoveAtSwap(Index, 1, EAllowShrinking::No);
		}
	}

	if (VisualPaths.Num() <= 0)
	{
		return false;
	}

	bGameplayVisualAssetsPhaseQueued = true;
	UE_LOG(LogT66GameInstance, Log, TEXT("[LOAD] Gameplay preload queued %d visual assets for the second-stage warmup."), VisualPaths.Num());

	GameplayAssetsPreloadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		VisualPaths,
		FStreamableDelegate::CreateUObject(this, &UT66GameInstance::HandleGameplayAssetsPreloaded));
	if (!GameplayAssetsPreloadHandle.IsValid())
	{
		bGameplayVisualAssetsPhaseQueued = false;
		return false;
	}

	return true;
}

void UT66GameInstance::HandleGameplayAssetsPreloaded()
{
	GameplayAssetsPreloadHandle.Reset();
	if (!CachedCharacterVisualsDataTable && !CharacterVisualsDataTable.IsNull())
	{
		CachedCharacterVisualsDataTable = CharacterVisualsDataTable.Get();
	}
	if (!bGameplayVisualAssetsPhaseQueued && QueueGameplayVisualAssetPreload())
	{
		return;
	}

	UE_LOG(LogT66GameInstance, Log, TEXT("[LOAD] Gameplay transition asset preload completed. Pre-resolving %d visuals."), GameplayPreloadVisualIDs.Num());
	bool bWaitingOnVisualResolves = false;
	if (UT66CharacterVisualSubsystem* Visuals = GetSubsystem<UT66CharacterVisualSubsystem>())
	{
		for (const FName VisualID : GameplayPreloadVisualIDs)
		{
			Visuals->PreloadCharacterVisual(VisualID);
			if (!Visuals->IsCharacterVisualReady(VisualID))
			{
				bWaitingOnVisualResolves = true;
			}
		}
	}

	if (bWaitingOnVisualResolves)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(GameplayVisualPreloadPollTimerHandle);
			GameplayVisualPreloadPollRetriesRemaining = 100;
			World->GetTimerManager().SetTimer(
				GameplayVisualPreloadPollTimerHandle,
				this,
				&UT66GameInstance::PollGameplayVisualPreloadCompletion,
				0.02f,
				true);
			return;
		}
	}

	FinalizeGameplayAssetsPreload();
}

void UT66GameInstance::PollGameplayVisualPreloadCompletion()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		FinalizeGameplayAssetsPreload();
		return;
	}

	UT66CharacterVisualSubsystem* Visuals = GetSubsystem<UT66CharacterVisualSubsystem>();
	bool bAllReady = true;
	if (Visuals)
	{
		for (const FName VisualID : GameplayPreloadVisualIDs)
		{
			if (!Visuals->IsCharacterVisualReady(VisualID))
			{
				bAllReady = false;
				break;
			}
		}
	}

	if (bAllReady || --GameplayVisualPreloadPollRetriesRemaining <= 0)
	{
		if (!bAllReady)
		{
			UE_LOG(LogT66GameInstance, Warning, TEXT("[LOAD] Gameplay transition visual preload polling timed out; continuing with remaining fallback-safe startup behavior."));
		}
		World->GetTimerManager().ClearTimer(GameplayVisualPreloadPollTimerHandle);
		FinalizeGameplayAssetsPreload();
	}
}

void UT66GameInstance::FinalizeGameplayAssetsPreload()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GameplayVisualPreloadPollTimerHandle);
	}

	GameplayVisualPreloadPollRetriesRemaining = 0;
	bGameplayAssetsPreloadInFlight = false;
	bGameplayPreloadWaitingOnCoreTables = false;
	bGameplayVisualAssetsPhaseQueued = false;
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

FName UT66GameInstance::GetGameplayLevelName()
{
	return FName(GameplayLevelName);
}

FName UT66GameInstance::GetTribulationEntryLevelName()
{
	return GetGameplayLevelName();
}

void UT66GameInstance::TransitionToGameplayLevel()
{
	UWorld* World = GetWorld();
	if (!World) return;

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
			const FName LevelToOpen = GetGameplayLevelName();
			UE_LOG(LogT66GameInstance, Log, TEXT("[LOAD] TransitionToGameplayLevel opening %s."), *LevelToOpen.ToString());
			UGameplayStatics::OpenLevel(this, LevelToOpen);
		});
	}), 0.05f, false); // Small delay so the loading widget paints first.
}

void UT66GameInstance::MarkPendingDirectGameplayEntry(const FString& Source)
{
	bPendingDirectGameplayEntry = true;
	PendingDirectGameplayEntrySource = Source;
}

bool UT66GameInstance::ConsumePendingDirectGameplayEntry(FString& OutSource)
{
	if (!bPendingDirectGameplayEntry)
	{
		return false;
	}

	OutSource = PendingDirectGameplayEntrySource;
	ClearPendingDirectGameplayEntry();
	return true;
}

void UT66GameInstance::ClearPendingDirectGameplayEntry()
{
	bPendingDirectGameplayEntry = false;
	PendingDirectGameplayEntrySource.Reset();
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
