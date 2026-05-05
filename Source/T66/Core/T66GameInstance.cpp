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
	WeaponsDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/DT_Weapons.DT_Weapons")));
	ArcadeInteractablesDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/DT_ArcadeInteractables.DT_ArcadeInteractables")));

	// Default selections
	SelectedPartySize = ET66PartySize::Solo;
	SelectedHeroID = NAME_None;
	SelectedCompanionID = NAME_None;
	SelectedDifficulty = ET66Difficulty::Easy;
	bPendingWeaponUpgradeOffer = false;
	PendingWeaponUpgradeRarity = ET66WeaponRarity::Black;
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

	// Preload core DataTables early, asynchronously, so we avoid sync loads later.
	PrimeCoreDataTablesAsync();
	PrimeCorePresentationAssetsAsync();

	// Preload the main-menu textures so they are often ready before BuildSlateUI's
	// EnsureTexturesLoadedSync fallback fires. If these finish in time, the sync path becomes a no-op.
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
	Paths.Reserve(11);

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
	AddDT(HouseNPCsDataTable);
	AddDT(LoanSharkDataTable);
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
	if (!CachedHouseNPCsDataTable) CachedHouseNPCsDataTable = HouseNPCsDataTable.Get();
	if (!CachedLoanSharkDataTable) CachedLoanSharkDataTable = LoanSharkDataTable.Get();
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
		AddPath(HeroRow->PortraitStacy.ToSoftObjectPath());
		AddPath(HeroRow->PortraitLow.ToSoftObjectPath());
		AddPath(HeroRow->PortraitFull.ToSoftObjectPath());
		AddPath(HeroRow->PortraitStacyLow.ToSoftObjectPath());
		AddPath(HeroRow->PortraitStacyFull.ToSoftObjectPath());
		AddPath(HeroRow->PortraitInvincible.ToSoftObjectPath());
		AddPath(HeroRow->PortraitStacyInvincible.ToSoftObjectPath());
		if (HeroRow->HeroID == FName(TEXT("Hero_1")))
		{
			AddPath(FSoftObjectPath(TEXT("/Game/UI/Sprites/Heroes/Hero_1/T_Hero_1_Chad_Invincible.T_Hero_1_Chad_Invincible")));
			AddPath(FSoftObjectPath(TEXT("/Game/UI/Sprites/Heroes/Hero_1/T_Hero_1_Stacy_Invincible.T_Hero_1_Stacy_Invincible")));
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

UDataTable* UT66GameInstance::ResolveCachedDataTable(TObjectPtr<UDataTable>& Cached, const TSoftObjectPtr<UDataTable>& Soft)
{
	if (!Cached && !Soft.IsNull())
	{
		Cached = Soft.Get();
		if (!Cached)
		{
			// Kick off async preload if we haven't already, but keep a safe sync fallback.
			PrimeCoreDataTablesAsync();
			Cached = Soft.LoadSynchronous();
		}
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
UDataTable* UT66GameInstance::GetHouseNPCsDataTable() { return ResolveCachedDataTable(CachedHouseNPCsDataTable, HouseNPCsDataTable); }
UDataTable* UT66GameInstance::GetLoanSharkDataTable() { return ResolveCachedDataTable(CachedLoanSharkDataTable, LoanSharkDataTable); }
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

bool UT66GameInstance::GetHouseNPCData(FName NPCID, FHouseNPCData& OutNPCData)
{
	return FindDataRow(GetHouseNPCsDataTable(), NPCID, OutNPCData, TEXT("GetHouseNPCData"));
}

bool UT66GameInstance::GetLoanSharkData(FName LoanSharkID, FLoanSharkData& OutData)
{
	return FindDataRow(GetLoanSharkDataTable(), LoanSharkID, OutData, TEXT("GetLoanSharkData"));
}

TArray<FName> UT66GameInstance::GetAllHeroIDs()
{
	UDataTable* DataTable = GetHeroDataTable();
	return DataTable ? DataTable->GetRowNames() : TArray<FName>();
}

TArray<FName> UT66GameInstance::GetAllCompanionIDs()
{
	UDataTable* DataTable = GetCompanionDataTable();
	return DataTable ? DataTable->GetRowNames() : TArray<FName>();
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
		if (HeroData.HeroID == FName(TEXT("Hero_1")))
		{
			return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(
				bUseStacyPortrait
					? TEXT("/Game/UI/Sprites/Heroes/Hero_1/T_Hero_1_Stacy_Invincible.T_Hero_1_Stacy_Invincible")
					: TEXT("/Game/UI/Sprites/Heroes/Hero_1/T_Hero_1_Chad_Invincible.T_Hero_1_Chad_Invincible")));
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
	bPendingWeaponUpgradeOffer = false;
	PendingWeaponUpgradeRarity = ET66WeaponRarity::Black;
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
	bIsDailyClimbRunActive = Challenge.IsValid();
	if (!bIsDailyClimbRunActive)
	{
		return;
	}

	SelectedPartySize = ET66PartySize::Solo;
	SelectedHeroID = Challenge.HeroID;
	SelectedCompanionID = NAME_None;
	SelectedDifficulty = Challenge.Difficulty;
	bPendingWeaponUpgradeOffer = false;
	PendingWeaponUpgradeRarity = ET66WeaponRarity::Black;
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
	bIsDailyClimbRunActive = false;
	ActiveDailyClimbChallenge = FT66DailyClimbChallengeData{};
}

int32 UT66GameInstance::GetDailyClimbIntRuleValue(const ET66DailyClimbRuleType RuleType, const int32 DefaultValue) const
{
	return IsDailyClimbRunActive()
		? ActiveDailyClimbChallenge.FindIntRuleValue(RuleType, DefaultValue)
		: DefaultValue;
}

float UT66GameInstance::GetDailyClimbFloatRuleValue(const ET66DailyClimbRuleType RuleType, const float DefaultValue) const
{
	return IsDailyClimbRunActive()
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
		AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Branch.Branch")));
		AddPath(FSoftObjectPath(TEXT("/Game/World/Props/Rock.Rock")));
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

	// Main gameplay uses a dedicated terrain asset set. Preload the full terrain/prop contract
	// before opening the gameplay level so the first entry does not depend on cold material state.
	AddPath(CharacterVisualsDataTable.ToSoftObjectPath());
	AddPath(FSoftObjectPath(TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof.MI_TowerDungeonRoof")));
	AddPath(FSoftObjectPath(TEXT("/Game/World/Terrain/TowerDungeon/T_TowerDungeonRoof.T_TowerDungeonRoof")));
	AddCoherentThemeKitAssets();
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
	AddAllTowerThemeAssets();
	AddAllCombatEffectAssets();

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
