// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66ProfileSaveGame.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66SaveMigration.h"
#include "Core/T66SkinSubsystem.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Achievements, Log, All);

const FString UT66AchievementsSubsystem::ProfileSaveSlotName(TEXT("T66_Profile"));

namespace
{
	FName MakeExtraAchievementID(const TCHAR* Prefix, int32 Index)
	{
		return FName(*FString::Printf(TEXT("%s%03d"), Prefix, Index + 1));
	}

	ET66AchievementTier GetGeneratedAchievementTier(int32 Index, int32 Total)
	{
		if (Total <= 1)
		{
			return ET66AchievementTier::Black;
		}

		const float Progress = static_cast<float>(Index) / static_cast<float>(Total - 1);
		if (Progress >= 0.90f)
		{
			return ET66AchievementTier::White;
		}
		if (Progress >= 0.70f)
		{
			return ET66AchievementTier::Yellow;
		}
		if (Progress >= 0.40f)
		{
			return ET66AchievementTier::Red;
		}
		return ET66AchievementTier::Black;
	}

	int32 GetGeneratedRewardCoins(ET66AchievementTier Tier)
	{
		switch (Tier)
		{
		case ET66AchievementTier::White:
			return 750;
		case ET66AchievementTier::Yellow:
			return 400;
		case ET66AchievementTier::Red:
			return 200;
		case ET66AchievementTier::Black:
		default:
			return 100;
		}
	}

	const TArray<int32>& GetExtraEnemyKillThresholds()
	{
		static const TArray<int32> Thresholds = { 5, 10, 50, 250, 500, 2500, 5000, 10000, 25000, 50000, 100000, 250000 };
		return Thresholds;
	}

	const TArray<int32>& GetExtraBossKillThresholds()
	{
		static const TArray<int32> Thresholds = { 2, 3, 5, 15, 25, 50, 75, 100 };
		return Thresholds;
	}

	const TArray<int32>& GetExtraStageClearThresholds()
	{
		static const TArray<int32> Thresholds = { 3, 5, 15, 20, 25, 30, 40, 50, 75, 100, 150, 200 };
		return Thresholds;
	}

	const TArray<int32>& GetExtraRunCompletionThresholds()
	{
		static const TArray<int32> Thresholds = { 3, 5, 10, 15, 20, 50, 75, 100 };
		return Thresholds;
	}

	const TArray<int32>& GetExtraVendorPurchaseThresholds()
	{
		static const TArray<int32> Thresholds = { 3, 5, 10, 15, 20, 25, 50, 100 };
		return Thresholds;
	}

	const TArray<int32>& GetExtraGamblerWinThresholds()
	{
		static const TArray<int32> Thresholds = { 3, 5, 10, 15, 20, 25, 50, 100 };
		return Thresholds;
	}

	const TArray<int32>& GetExtraLabItemThresholds()
	{
		static const TArray<int32> Thresholds = { 1, 3, 10, 15, 20, 30 };
		return Thresholds;
	}

	const TArray<int32>& GetExtraLabEnemyThresholds()
	{
		static const TArray<int32> Thresholds = { 1, 3, 10, 15, 20, 30 };
		return Thresholds;
	}

	const TArray<int32>& GetExtraUnionThresholds()
	{
		static const TArray<int32> Thresholds = { 3, 7, 10, 12, 15, 25, 30 };
		return Thresholds;
	}

	const TArray<int32>& GetExtraGamblerTokenThresholds()
	{
		static const TArray<int32> Thresholds = { 1, 2, 3, 4, 5 };
		return Thresholds;
	}

	void AddGeneratedAchievement(
		TArray<FAchievementData>& Out,
		FName Id,
		int32 Index,
		int32 Total,
		int32 RequirementCount,
		const FText& DisplayName,
		const FText& Description)
	{
		FAchievementData A;
		A.AchievementID = Id;
		A.RequirementCount = RequirementCount;
		A.DisplayName = DisplayName;
		A.Description = Description;
		A.Tier = GetGeneratedAchievementTier(Index, Total);
		A.RewardCoins = GetGeneratedRewardCoins(A.Tier);
		Out.Add(A);
	}
}

UT66LocalizationSubsystem* UT66AchievementsSubsystem::GetLocSubsystem() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

void UT66AchievementsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<UT66LocalizationSubsystem>();
	LoadOrCreateProfile();
	RebuildDefinitions();

	// Keep localized achievement text in sync with language changes.
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66AchievementsSubsystem::HandleLanguageChanged);
	}
}

void UT66AchievementsSubsystem::Deinitialize()
{
	SaveProfileIfNeeded(true);

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.RemoveDynamic(this, &UT66AchievementsSubsystem::HandleLanguageChanged);
	}

	Super::Deinitialize();
}

void UT66AchievementsSubsystem::HandleLanguageChanged(ET66Language NewLanguage)
{
	RebuildDefinitions();
	AchievementsStateChanged.Broadcast();
}

void UT66AchievementsSubsystem::LoadOrCreateProfile()
{
	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(ProfileSaveSlotName, ProfileSaveUserIndex);
	Profile = Cast<UT66ProfileSaveGame>(Loaded);
	if (!Profile)
	{
		Profile = NewObject<UT66ProfileSaveGame>(this);
		Profile->AchievementCoinsBalance = 10000; // Starting grant for new players only
		UE_LOG(LogT66Achievements, Log, TEXT("[SKIN] LoadOrCreateProfile: Created FRESH profile (no save file found, starting AC=%d)"), Profile->AchievementCoinsBalance);
	}
	else
	{
		UE_LOG(LogT66Achievements, Log, TEXT("[SKIN] LoadOrCreateProfile: Loaded EXISTING profile from save file"));
	}

	const int32 LoadedSaveVersion = Profile->SaveVersion;

	// Enforce safe defaults.
	Profile->SaveVersion = FMath::Max(Profile->SaveVersion, 0);
	Profile->LifetimeEnemiesKilled = FMath::Max(0, Profile->LifetimeEnemiesKilled);
	Profile->LifetimeBossesKilled = FMath::Max(0, Profile->LifetimeBossesKilled);
	Profile->LifetimeStagesCleared = FMath::Max(0, Profile->LifetimeStagesCleared);
	Profile->LifetimeRunsCompleted = FMath::Max(0, Profile->LifetimeRunsCompleted);
	Profile->LifetimeVendorPurchases = FMath::Max(0, Profile->LifetimeVendorPurchases);
	Profile->LifetimeGamblerWins = FMath::Max(0, Profile->LifetimeGamblerWins);
	Profile->GamblersTokenUnlockedLevel = FMath::Clamp(Profile->GamblersTokenUnlockedLevel, 0, 6);

	// Migration: Hero renumbering (Hero_1 removed, Hero_2..5 -> Hero_1..4). SaveVersion 9.
	if (LoadedSaveVersion < 9)
	{
		TMap<FName, FT66OwnedSkinsList> NewOwned;
		for (const auto& Pair : Profile->OwnedHeroSkinsByHero)
		{
			FName NewKey = T66MigrateHeroIDFromSave(Pair.Key);
			if (NewOwned.Contains(NewKey))
			{
				for (const FName& SkinID : Pair.Value.SkinIDs)
				{
					if (!NewOwned.Find(NewKey)->SkinIDs.Contains(SkinID))
					{
						NewOwned.Find(NewKey)->SkinIDs.Add(SkinID);
					}
				}
			}
			else
			{
				NewOwned.Add(NewKey, Pair.Value);
			}
		}
		Profile->OwnedHeroSkinsByHero = MoveTemp(NewOwned);
		TMap<FName, FName> NewEquipped;
		for (const auto& Pair : Profile->EquippedHeroSkinIDByHero)
		{
			FName NewKey = T66MigrateHeroIDFromSave(Pair.Key);
			NewEquipped.Add(NewKey, Pair.Value);
		}
		Profile->EquippedHeroSkinIDByHero = MoveTemp(NewEquipped);
		Profile->SaveVersion = 9;
		bProfileDirty = true;
		UE_LOG(LogT66Achievements, Log, TEXT("[SKIN] LoadOrCreateProfile: Applied HeroID renumber migration (SaveVersion 9)"));
	}

	// Hero skins: log current state (no more auto-reset; purchases persist).
	const FName DefaultSkin(TEXT("Default"));
	
	UE_LOG(LogT66Achievements, Log, TEXT("[SKIN] LoadOrCreateProfile: OwnedHeroSkinsByHero.Num=%d, AC Balance=%d"),
		Profile->OwnedHeroSkinsByHero.Num(), Profile->AchievementCoinsBalance);
	
	// Log current owned skins
	for (const auto& Pair : Profile->OwnedHeroSkinsByHero)
	{
		FString SkinList;
		for (const FName& S : Pair.Value.SkinIDs)
		{
			if (!SkinList.IsEmpty()) SkinList += TEXT(", ");
			SkinList += S.ToString();
		}
		UE_LOG(LogT66Achievements, Log, TEXT("[SKIN] LoadOrCreateProfile: %s owns [%s]"), *Pair.Key.ToString(), *SkinList);
	}

	// Union defaults: clamp to sane ranges so corrupted saves can't break gameplay math.
	for (TPair<FName, int32>& Pair : Profile->CompanionUnionStagesClearedByID)
	{
		Pair.Value = FMath::Clamp(Pair.Value, 0, 2000000000);
	}

	Profile->SaveVersion = FMath::Max(Profile->SaveVersion, 10);
}

int32 UT66AchievementsSubsystem::GetAchievementCoinsBalance() const
{
	return Profile ? FMath::Max(0, Profile->AchievementCoinsBalance) : 0;
}

bool UT66AchievementsSubsystem::SpendAchievementCoins(int32 Amount)
{
	if (!Profile)
	{
		LoadOrCreateProfile();
	}
	if (!Profile || Amount <= 0 || Profile->AchievementCoinsBalance < Amount) return false;
	Profile->AchievementCoinsBalance -= Amount;
	bProfileDirty = true;
	SaveProfileIfNeeded(true);
	AchievementCoinsChanged.Broadcast();
	return true;
}

void UT66AchievementsSubsystem::AddAchievementCoins(int32 Amount)
{
	if (!Profile)
	{
		LoadOrCreateProfile();
	}
	if (!Profile || Amount <= 0)
	{
		return;
	}

	Profile->AchievementCoinsBalance = FMath::Clamp(Profile->AchievementCoinsBalance + Amount, 0, 2000000000);
	bProfileDirty = true;
	SaveProfileIfNeeded(true);
	AchievementCoinsChanged.Broadcast();
}

bool UT66AchievementsSubsystem::IsHeroSkinOwned(FName HeroID, FName SkinID) const
{
	if (UT66SkinSubsystem* Skin = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SkinSubsystem>() : nullptr)
		return Skin->IsHeroSkinOwned(HeroID, SkinID);
	return false;
}

bool UT66AchievementsSubsystem::PurchaseHeroSkin(FName HeroID, FName SkinID, int32 CostAC)
{
	if (UT66SkinSubsystem* Skin = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SkinSubsystem>() : nullptr)
		return Skin->PurchaseHeroSkin(HeroID, SkinID, CostAC);
	return false;
}

FName UT66AchievementsSubsystem::GetEquippedHeroSkinID(FName HeroID) const
{
	if (UT66SkinSubsystem* Skin = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SkinSubsystem>() : nullptr)
		return Skin->GetEquippedHeroSkinID(HeroID);
	return FName(TEXT("Default"));
}

void UT66AchievementsSubsystem::SetEquippedHeroSkinID(FName HeroID, FName SkinID)
{
	if (UT66SkinSubsystem* Skin = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SkinSubsystem>() : nullptr)
		Skin->SetEquippedHeroSkinID(HeroID, SkinID);
}

void UT66AchievementsSubsystem::ResetAllHeroSkinOwnership()
{
	if (UT66SkinSubsystem* Skin = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SkinSubsystem>() : nullptr)
		Skin->ResetAllHeroSkinOwnership();
}

bool UT66AchievementsSubsystem::IsCompanionSkinOwned(FName CompanionID, FName SkinID) const
{
	if (UT66SkinSubsystem* Skin = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SkinSubsystem>() : nullptr)
		return Skin->IsCompanionSkinOwned(CompanionID, SkinID);
	return false;
}

bool UT66AchievementsSubsystem::PurchaseCompanionSkin(FName CompanionID, FName SkinID, int32 CostAC)
{
	if (UT66SkinSubsystem* Skin = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SkinSubsystem>() : nullptr)
		return Skin->PurchaseCompanionSkin(CompanionID, SkinID, CostAC);
	return false;
}

FName UT66AchievementsSubsystem::GetEquippedCompanionSkinID(FName CompanionID) const
{
	if (UT66SkinSubsystem* Skin = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SkinSubsystem>() : nullptr)
		return Skin->GetEquippedCompanionSkinID(CompanionID);
	return FName(TEXT("Default"));
}

void UT66AchievementsSubsystem::SetEquippedCompanionSkinID(FName CompanionID, FName SkinID)
{
	if (UT66SkinSubsystem* Skin = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SkinSubsystem>() : nullptr)
		Skin->SetEquippedCompanionSkinID(CompanionID, SkinID);
}

bool UT66AchievementsSubsystem::HasCompletedTutorial() const
{
	return Profile ? Profile->bHasCompletedTutorial : false;
}

void UT66AchievementsSubsystem::MarkTutorialCompleted()
{
	if (!Profile) LoadOrCreateProfile();
	if (!Profile || Profile->bHasCompletedTutorial) return;
	Profile->bHasCompletedTutorial = true;
	TArray<FName> NewlyUnlocked;
	FT66AchievementState* S = FindOrAddState(FName(TEXT("ACH_BLK_005")));
	if (S && !S->bIsUnlocked) { S->CurrentProgress = 1; S->bIsUnlocked = true; NewlyUnlocked.Add(FName(TEXT("ACH_BLK_005"))); }
	bProfileDirty = true;
	SaveProfileIfNeeded(true);
	AchievementsStateChanged.Broadcast();
	if (NewlyUnlocked.Num() > 0) AchievementsUnlocked.Broadcast(NewlyUnlocked);
}

const FT66AchievementState* UT66AchievementsSubsystem::FindState(FName AchievementID) const
{
	if (!Profile || AchievementID.IsNone()) return nullptr;
	return Profile->AchievementStateByID.Find(AchievementID);
}

FT66AchievementState* UT66AchievementsSubsystem::FindOrAddState(FName AchievementID)
{
	if (!Profile || AchievementID.IsNone()) return nullptr;
	return &Profile->AchievementStateByID.FindOrAdd(AchievementID);
}

static void AddAchievement(TArray<FAchievementData>& Out, UT66LocalizationSubsystem* Loc,
	FName Id, ET66AchievementTier Tier, int32 RequirementCount, int32 RewardCoins)
{
	FAchievementData A;
	A.AchievementID = Id;
	A.Tier = Tier;
	A.RequirementCount = RequirementCount;
	A.RewardCoins = RewardCoins;
	A.DisplayName = Loc ? Loc->GetText_AchievementName(Id) : FText::FromString(Id.ToString());
	A.Description = Loc ? Loc->GetText_AchievementDescription(Id) : FText::FromString(Id.ToString());
	Out.Add(A);
}

void UT66AchievementsSubsystem::RebuildDefinitions()
{
	CachedDefinitions.Reset();
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_BLK_001")), ET66AchievementTier::Black, 20, 250);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_BLK_002")), ET66AchievementTier::Black, 1, 25);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_BLK_003")), ET66AchievementTier::Black, 100, 100);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_RED_001")), ET66AchievementTier::Red, 1000, 500);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_BLK_004")), ET66AchievementTier::Black, 1, 50);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_RED_002")), ET66AchievementTier::Red, 10, 300);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_BLK_005")), ET66AchievementTier::Black, 1, 50);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_BLK_006")), ET66AchievementTier::Black, 1, 75);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_RED_003")), ET66AchievementTier::Red, 10, 400);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_BLK_007")), ET66AchievementTier::Black, 1, 100);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_YEL_001")), ET66AchievementTier::Yellow, 25, 750);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_BLK_008")), ET66AchievementTier::Black, 1, 50);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_RED_004")), ET66AchievementTier::Red, 5, 200);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_YEL_002")), ET66AchievementTier::Yellow, 20, 500);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_BLK_009")), ET66AchievementTier::Black, 1, 25);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_BLK_010")), ET66AchievementTier::Black, 1, 50);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_RED_005")), ET66AchievementTier::Red, 1, 150);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_RED_006")), ET66AchievementTier::Red, 1, 200);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_BLK_011")), ET66AchievementTier::Black, 5, 100);
	AddAchievement(CachedDefinitions, Loc, FName(TEXT("ACH_RED_007")), ET66AchievementTier::Red, 5, 250);

	const TArray<int32>& ExtraEnemyThresholds = GetExtraEnemyKillThresholds();
	for (int32 Index = 0; Index < ExtraEnemyThresholds.Num(); ++Index)
	{
		const int32 Requirement = ExtraEnemyThresholds[Index];
		AddGeneratedAchievement(
			CachedDefinitions,
			MakeExtraAchievementID(TEXT("ACH_EXT_ENEMY_"), Index),
			Index,
			ExtraEnemyThresholds.Num(),
			Requirement,
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraEnemyTitle", "Enemy Hunter {0}"), FText::AsNumber(Index + 1)),
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraEnemyDesc", "Defeat {0} enemies."), FText::AsNumber(Requirement)));
	}

	const TArray<int32>& ExtraBossThresholds = GetExtraBossKillThresholds();
	for (int32 Index = 0; Index < ExtraBossThresholds.Num(); ++Index)
	{
		const int32 Requirement = ExtraBossThresholds[Index];
		AddGeneratedAchievement(
			CachedDefinitions,
			MakeExtraAchievementID(TEXT("ACH_EXT_BOSS_"), Index),
			Index,
			ExtraBossThresholds.Num(),
			Requirement,
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraBossTitle", "Boss Hunter {0}"), FText::AsNumber(Index + 1)),
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraBossDesc", "Defeat {0} bosses."), FText::AsNumber(Requirement)));
	}

	const TArray<int32>& ExtraStageThresholds = GetExtraStageClearThresholds();
	for (int32 Index = 0; Index < ExtraStageThresholds.Num(); ++Index)
	{
		const int32 Requirement = ExtraStageThresholds[Index];
		AddGeneratedAchievement(
			CachedDefinitions,
			MakeExtraAchievementID(TEXT("ACH_EXT_STAGE_"), Index),
			Index,
			ExtraStageThresholds.Num(),
			Requirement,
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraStageTitle", "Pilgrim {0}"), FText::AsNumber(Index + 1)),
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraStageDesc", "Clear {0} stages."), FText::AsNumber(Requirement)));
	}

	const TArray<int32>& ExtraRunThresholds = GetExtraRunCompletionThresholds();
	for (int32 Index = 0; Index < ExtraRunThresholds.Num(); ++Index)
	{
		const int32 Requirement = ExtraRunThresholds[Index];
		AddGeneratedAchievement(
			CachedDefinitions,
			MakeExtraAchievementID(TEXT("ACH_EXT_RUN_"), Index),
			Index,
			ExtraRunThresholds.Num(),
			Requirement,
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraRunTitle", "Veteran {0}"), FText::AsNumber(Index + 1)),
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraRunDesc", "Finish {0} runs."), FText::AsNumber(Requirement)));
	}

	const TArray<int32>& ExtraVendorThresholds = GetExtraVendorPurchaseThresholds();
	for (int32 Index = 0; Index < ExtraVendorThresholds.Num(); ++Index)
	{
		const int32 Requirement = ExtraVendorThresholds[Index];
		AddGeneratedAchievement(
			CachedDefinitions,
			MakeExtraAchievementID(TEXT("ACH_EXT_VENDOR_"), Index),
			Index,
			ExtraVendorThresholds.Num(),
			Requirement,
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraVendorTitle", "Customer {0}"), FText::AsNumber(Index + 1)),
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraVendorDesc", "Buy {0} items from the vendor."), FText::AsNumber(Requirement)));
	}

	const TArray<int32>& ExtraGamblerThresholds = GetExtraGamblerWinThresholds();
	for (int32 Index = 0; Index < ExtraGamblerThresholds.Num(); ++Index)
	{
		const int32 Requirement = ExtraGamblerThresholds[Index];
		AddGeneratedAchievement(
			CachedDefinitions,
			MakeExtraAchievementID(TEXT("ACH_EXT_GAMBLER_"), Index),
			Index,
			ExtraGamblerThresholds.Num(),
			Requirement,
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraGamblerTitle", "High Roller {0}"), FText::AsNumber(Index + 1)),
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraGamblerDesc", "Win {0} gambles."), FText::AsNumber(Requirement)));
	}

	const TArray<int32>& ExtraLabItemThresholds = GetExtraLabItemThresholds();
	for (int32 Index = 0; Index < ExtraLabItemThresholds.Num(); ++Index)
	{
		const int32 Requirement = ExtraLabItemThresholds[Index];
		AddGeneratedAchievement(
			CachedDefinitions,
			MakeExtraAchievementID(TEXT("ACH_EXT_ITEM_"), Index),
			Index,
			ExtraLabItemThresholds.Num(),
			Requirement,
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraItemTitle", "Collector {0}"), FText::AsNumber(Index + 1)),
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraItemDesc", "Discover {0} items."), FText::AsNumber(Requirement)));
	}

	const TArray<int32>& ExtraLabEnemyThresholds = GetExtraLabEnemyThresholds();
	for (int32 Index = 0; Index < ExtraLabEnemyThresholds.Num(); ++Index)
	{
		const int32 Requirement = ExtraLabEnemyThresholds[Index];
		AddGeneratedAchievement(
			CachedDefinitions,
			MakeExtraAchievementID(TEXT("ACH_EXT_LABENEMY_"), Index),
			Index,
			ExtraLabEnemyThresholds.Num(),
			Requirement,
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraLabEnemyTitle", "Field Notes {0}"), FText::AsNumber(Index + 1)),
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraLabEnemyDesc", "Discover {0} enemies."), FText::AsNumber(Requirement)));
	}

	const TArray<int32>& ExtraUnionThresholds = GetExtraUnionThresholds();
	for (int32 Index = 0; Index < ExtraUnionThresholds.Num(); ++Index)
	{
		const int32 Requirement = ExtraUnionThresholds[Index];
		AddGeneratedAchievement(
			CachedDefinitions,
			MakeExtraAchievementID(TEXT("ACH_EXT_UNION_"), Index),
			Index,
			ExtraUnionThresholds.Num(),
			Requirement,
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraUnionTitle", "Companion Bond {0}"), FText::AsNumber(Index + 1)),
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraUnionDesc", "Clear {0} stages with the same companion."), FText::AsNumber(Requirement)));
	}

	const TArray<int32>& ExtraTokenThresholds = GetExtraGamblerTokenThresholds();
	for (int32 Index = 0; Index < ExtraTokenThresholds.Num(); ++Index)
	{
		const int32 Requirement = ExtraTokenThresholds[Index];
		AddGeneratedAchievement(
			CachedDefinitions,
			MakeExtraAchievementID(TEXT("ACH_EXT_TOKEN_"), Index),
			Index,
			ExtraTokenThresholds.Num(),
			Requirement,
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraTokenTitle", "Token Rank {0}"), FText::AsNumber(Requirement)),
			FText::Format(NSLOCTEXT("T66.Achievements", "ExtraTokenDesc", "Unlock Gambler's Token level {0}."), FText::AsNumber(Requirement)));
	}

	ApplyRuntimeStateToCachedDefinitions(CachedDefinitions);
}

void UT66AchievementsSubsystem::ApplyRuntimeStateToCachedDefinitions(TArray<FAchievementData>& InOut) const
{
	for (FAchievementData& A : InOut)
	{
		const FT66AchievementState* S = FindState(A.AchievementID);
		if (!S)
		{
			A.CurrentProgress = 0;
			A.bIsUnlocked = false;
		}
		else
		{
			A.CurrentProgress = FMath::Max(0, S->CurrentProgress);
			A.bIsUnlocked = S->bIsUnlocked;
		}
	}
}

TArray<FAchievementData> UT66AchievementsSubsystem::GetAllAchievements() const
{
	TArray<FAchievementData> Out = CachedDefinitions;
	ApplyRuntimeStateToCachedDefinitions(Out);
	return Out;
}

TArray<FAchievementData> UT66AchievementsSubsystem::GetAchievementsForTier(ET66AchievementTier Tier) const
{
	TArray<FAchievementData> Out;
	for (const FAchievementData& A : CachedDefinitions)
	{
		if (A.Tier == Tier)
		{
			Out.Add(A);
		}
	}
	ApplyRuntimeStateToCachedDefinitions(Out);
	return Out;
}

bool UT66AchievementsSubsystem::IsAchievementClaimed(FName AchievementID) const
{
	const FT66AchievementState* S = FindState(AchievementID);
	return S ? S->bIsClaimed : false;
}

void UT66AchievementsSubsystem::SaveProfileIfNeeded(bool bForce)
{
	if (!Profile) return;
	if (!bProfileDirty && !bForce) return;

	if (bForce || !GetWorld())
	{
		// [GOLD] Async save: avoid blocking the game thread during profile writes.
		UE_LOG(LogT66Achievements, Verbose, TEXT("[GOLD] AsyncSave: queuing async achievement profile save (forced=%d)"), bForce ? 1 : 0);
		UGameplayStatics::AsyncSaveGameToSlot(Profile, ProfileSaveSlotName, ProfileSaveUserIndex);
		bProfileDirty = false;
		return;
	}

	const float Now = static_cast<float>(GetWorld()->GetTimeSeconds());
	if (bForce || (Now - LastProfileSaveWorldSeconds) >= SaveThrottleSeconds)
	{
		UE_LOG(LogT66Achievements, Verbose, TEXT("[GOLD] AsyncSave: queuing async achievement profile save (throttled)"));
		UGameplayStatics::AsyncSaveGameToSlot(Profile, ProfileSaveSlotName, ProfileSaveUserIndex);
		LastProfileSaveWorldSeconds = Now;
		bProfileDirty = false;
	}
}

void UT66AchievementsSubsystem::MarkProfileDirtyAndSave(bool bBroadcastCoinsChanged)
{
	bProfileDirty = true;
	SaveProfileIfNeeded(true);
	if (bBroadcastCoinsChanged)
	{
		AchievementCoinsChanged.Broadcast();
	}
}

void UT66AchievementsSubsystem::MarkDirtyAndMaybeSave(bool bForce)
{
	bProfileDirty = true;
	SaveProfileIfNeeded(bForce);
}

bool UT66AchievementsSubsystem::AddLabUnlockedItem(FName ItemID)
{
	if (ItemID.IsNone()) return false;
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return false;
	if (Profile->LabUnlockedItemIDs.Contains(ItemID)) return false;
	Profile->LabUnlockedItemIDs.Add(ItemID);
	const int32 NumItems = Profile->LabUnlockedItemIDs.Num();
	TArray<FName> NewlyUnlocked;
	bool bAnyChanged = UpdateCountAchievement(FName(TEXT("ACH_BLK_011")), NumItems, 5, &NewlyUnlocked);
	bAnyChanged |= UpdateMilestoneAchievements(TEXT("ACH_EXT_ITEM_"), GetExtraLabItemThresholds(), NumItems, &NewlyUnlocked);
	MarkDirtyAndMaybeSave(true);
	if (bAnyChanged) { AchievementsStateChanged.Broadcast(); if (NewlyUnlocked.Num() > 0) AchievementsUnlocked.Broadcast(NewlyUnlocked); }
	return true;
}

bool UT66AchievementsSubsystem::AddLabUnlockedEnemy(FName EnemyOrBossID)
{
	if (EnemyOrBossID.IsNone()) return false;
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return false;
	if (Profile->LabUnlockedEnemyIDs.Contains(EnemyOrBossID)) return false;
	Profile->LabUnlockedEnemyIDs.Add(EnemyOrBossID);
	const int32 NumEnemies = Profile->LabUnlockedEnemyIDs.Num();
	TArray<FName> NewlyUnlocked;
	bool bAnyChanged = UpdateCountAchievement(FName(TEXT("ACH_RED_007")), NumEnemies, 5, &NewlyUnlocked);
	bAnyChanged |= UpdateMilestoneAchievements(TEXT("ACH_EXT_LABENEMY_"), GetExtraLabEnemyThresholds(), NumEnemies, &NewlyUnlocked);
	MarkDirtyAndMaybeSave(true);
	if (bAnyChanged) { AchievementsStateChanged.Broadcast(); if (NewlyUnlocked.Num() > 0) AchievementsUnlocked.Broadcast(NewlyUnlocked); }
	return true;
}

bool UT66AchievementsSubsystem::IsLabUnlockedItem(FName ItemID) const
{
	return Profile && Profile->LabUnlockedItemIDs.Contains(ItemID);
}

bool UT66AchievementsSubsystem::IsLabUnlockedEnemy(FName EnemyOrBossID) const
{
	return Profile && Profile->LabUnlockedEnemyIDs.Contains(EnemyOrBossID);
}

// Helper: set progress from a source value (e.g. LifetimeEnemiesKilled), unlock if >= Target. Returns true if state changed.
bool UT66AchievementsSubsystem::UpdateCountAchievement(FName AchievementID, int32 SourceValue, int32 Target, TArray<FName>* OutNewlyUnlocked)
{
	FT66AchievementState* S = FindOrAddState(AchievementID);
	if (!S) return false;
	const int32 PrevProgress = S->CurrentProgress;
	const bool bPrevUnlocked = S->bIsUnlocked;
	S->CurrentProgress = FMath::Min(SourceValue, Target);
	S->bIsUnlocked = (S->CurrentProgress >= Target);
	if (OutNewlyUnlocked && !bPrevUnlocked && S->bIsUnlocked)
		OutNewlyUnlocked->Add(AchievementID);
	const bool bProgressChanged = (S->CurrentProgress != PrevProgress) || (S->bIsUnlocked != bPrevUnlocked);
	return bProgressChanged;
}

bool UT66AchievementsSubsystem::UpdateMilestoneAchievements(const TCHAR* Prefix, const TArray<int32>& Thresholds, int32 SourceValue, TArray<FName>* OutNewlyUnlocked)
{
	bool bAnyChanged = false;
	for (int32 Index = 0; Index < Thresholds.Num(); ++Index)
	{
		bAnyChanged |= UpdateCountAchievement(MakeExtraAchievementID(Prefix, Index), SourceValue, Thresholds[Index], OutNewlyUnlocked);
	}
	return bAnyChanged;
}

void UT66AchievementsSubsystem::NotifyEnemyKilled(int32 Count)
{
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;

	const int32 Delta = FMath::Clamp(Count, 0, 1000000);
	if (Delta <= 0) return;

	Profile->LifetimeEnemiesKilled = FMath::Clamp(Profile->LifetimeEnemiesKilled + Delta, 0, 2000000000);
	const int32 Total = Profile->LifetimeEnemiesKilled;

	TArray<FName> NewlyUnlocked;
	bool bAnyChanged = false;
	bAnyChanged |= UpdateCountAchievement(FName(TEXT("ACH_BLK_001")), Total, 20, &NewlyUnlocked);
	bAnyChanged |= UpdateCountAchievement(FName(TEXT("ACH_BLK_002")), Total, 1, &NewlyUnlocked);
	bAnyChanged |= UpdateCountAchievement(FName(TEXT("ACH_BLK_003")), Total, 100, &NewlyUnlocked);
	bAnyChanged |= UpdateCountAchievement(FName(TEXT("ACH_RED_001")), Total, 1000, &NewlyUnlocked);
	bAnyChanged |= UpdateMilestoneAchievements(TEXT("ACH_EXT_ENEMY_"), GetExtraEnemyKillThresholds(), Total, &NewlyUnlocked);
	if (bAnyChanged)
	{
		MarkDirtyAndMaybeSave(true);
		AchievementsStateChanged.Broadcast();
		if (NewlyUnlocked.Num() > 0) AchievementsUnlocked.Broadcast(NewlyUnlocked);
	}
}

void UT66AchievementsSubsystem::NotifyBossKilled(int32 Count)
{
	if (!Profile) LoadOrCreateProfile();
	if (!Profile || Count <= 0) return;
	const int32 Delta = FMath::Clamp(Count, 0, 1000);
	Profile->LifetimeBossesKilled = FMath::Clamp(Profile->LifetimeBossesKilled + Delta, 0, 2000000000);
	const int32 Total = Profile->LifetimeBossesKilled;
	TArray<FName> NewlyUnlocked;
	bool bAnyChanged = false;
	bAnyChanged |= UpdateCountAchievement(FName(TEXT("ACH_BLK_004")), Total, 1, &NewlyUnlocked);
	bAnyChanged |= UpdateCountAchievement(FName(TEXT("ACH_RED_002")), Total, 10, &NewlyUnlocked);
	bAnyChanged |= UpdateMilestoneAchievements(TEXT("ACH_EXT_BOSS_"), GetExtraBossKillThresholds(), Total, &NewlyUnlocked);
	if (bAnyChanged) { MarkDirtyAndMaybeSave(true); AchievementsStateChanged.Broadcast(); if (NewlyUnlocked.Num() > 0) AchievementsUnlocked.Broadcast(NewlyUnlocked); }
}

void UT66AchievementsSubsystem::NotifyStageCleared(int32 Count)
{
	if (!Profile) LoadOrCreateProfile();
	if (!Profile || Count <= 0) return;
	const int32 Delta = FMath::Clamp(Count, 0, 1000);
	Profile->LifetimeStagesCleared = FMath::Clamp(Profile->LifetimeStagesCleared + Delta, 0, 2000000000);
	const int32 Total = Profile->LifetimeStagesCleared;
	TArray<FName> NewlyUnlocked;
	bool bAnyChanged = false;
	bAnyChanged |= UpdateCountAchievement(FName(TEXT("ACH_BLK_006")), Total, 1, &NewlyUnlocked);
	bAnyChanged |= UpdateCountAchievement(FName(TEXT("ACH_RED_003")), Total, 10, &NewlyUnlocked);
	bAnyChanged |= UpdateMilestoneAchievements(TEXT("ACH_EXT_STAGE_"), GetExtraStageClearThresholds(), Total, &NewlyUnlocked);
	if (bAnyChanged) { MarkDirtyAndMaybeSave(true); AchievementsStateChanged.Broadcast(); if (NewlyUnlocked.Num() > 0) AchievementsUnlocked.Broadcast(NewlyUnlocked); }
}

void UT66AchievementsSubsystem::NotifyRunCompleted(UT66RunStateSubsystem* RunState)
{
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;

	const int32 Delta = 1;
	Profile->LifetimeRunsCompleted = FMath::Clamp(Profile->LifetimeRunsCompleted + Delta, 0, 2000000000);
	const int32 TotalRuns = Profile->LifetimeRunsCompleted;
	TArray<FName> NewlyUnlocked;
	bool bAnyChanged = false;
	bAnyChanged |= UpdateCountAchievement(FName(TEXT("ACH_BLK_007")), TotalRuns, 1, &NewlyUnlocked);
	bAnyChanged |= UpdateCountAchievement(FName(TEXT("ACH_YEL_001")), TotalRuns, 25, &NewlyUnlocked);
	bAnyChanged |= UpdateMilestoneAchievements(TEXT("ACH_EXT_RUN_"), GetExtraRunCompletionThresholds(), TotalRuns, &NewlyUnlocked);

	if (RunState)
	{
		const int32 Gold = RunState->GetCurrentGold();
		const int32 Debt = RunState->GetCurrentDebt();
		FT66AchievementState* SGold = FindOrAddState(FName(TEXT("ACH_RED_005")));
		if (SGold && !SGold->bIsUnlocked && Gold >= 500) { SGold->CurrentProgress = 1; SGold->bIsUnlocked = true; bAnyChanged = true; NewlyUnlocked.Add(FName(TEXT("ACH_RED_005"))); }
		FT66AchievementState* SDebt = FindOrAddState(FName(TEXT("ACH_RED_006")));
		if (SDebt && !SDebt->bIsUnlocked && Debt == 0) { SDebt->CurrentProgress = 1; SDebt->bIsUnlocked = true; bAnyChanged = true; NewlyUnlocked.Add(FName(TEXT("ACH_RED_006"))); }
	}
	if (bAnyChanged) { MarkDirtyAndMaybeSave(true); AchievementsStateChanged.Broadcast(); if (NewlyUnlocked.Num() > 0) AchievementsUnlocked.Broadcast(NewlyUnlocked); }
}

void UT66AchievementsSubsystem::NotifyVendorPurchase()
{
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;
	Profile->LifetimeVendorPurchases = FMath::Clamp(Profile->LifetimeVendorPurchases + 1, 0, 2000000000);
	const int32 Total = Profile->LifetimeVendorPurchases;
	TArray<FName> NewlyUnlocked;
	bool bAnyChanged = UpdateCountAchievement(FName(TEXT("ACH_BLK_009")), Total, 1, &NewlyUnlocked);
	bAnyChanged |= UpdateMilestoneAchievements(TEXT("ACH_EXT_VENDOR_"), GetExtraVendorPurchaseThresholds(), Total, &NewlyUnlocked);
	if (bAnyChanged) { MarkDirtyAndMaybeSave(true); AchievementsStateChanged.Broadcast(); if (NewlyUnlocked.Num() > 0) AchievementsUnlocked.Broadcast(NewlyUnlocked); }
}

void UT66AchievementsSubsystem::NotifyGamblerWin()
{
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;
	Profile->LifetimeGamblerWins = FMath::Clamp(Profile->LifetimeGamblerWins + 1, 0, 2000000000);
	const int32 Total = Profile->LifetimeGamblerWins;
	TArray<FName> NewlyUnlocked;
	bool bAnyChanged = UpdateCountAchievement(FName(TEXT("ACH_BLK_010")), Total, 1, &NewlyUnlocked);
	bAnyChanged |= UpdateMilestoneAchievements(TEXT("ACH_EXT_GAMBLER_"), GetExtraGamblerWinThresholds(), Total, &NewlyUnlocked);
	if (bAnyChanged) { MarkDirtyAndMaybeSave(true); AchievementsStateChanged.Broadcast(); if (NewlyUnlocked.Num() > 0) AchievementsUnlocked.Broadcast(NewlyUnlocked); }
}

int32 UT66AchievementsSubsystem::GetGamblersTokenUnlockedLevel() const
{
	return Profile ? FMath::Clamp(Profile->GamblersTokenUnlockedLevel, 0, 6) : 0;
}

int32 UT66AchievementsSubsystem::GetGamblersTokenDifficultyFloor(ET66Difficulty Difficulty)
{
	const int32 DifficultyIndex = static_cast<int32>(Difficulty);
	return FMath::Clamp(DifficultyIndex + 1, 1, 6);
}

int32 UT66AchievementsSubsystem::UpgradeGamblersTokenForDifficulty(ET66Difficulty Difficulty)
{
	if (!Profile) LoadOrCreateProfile();
	if (!Profile)
	{
		return 0;
	}

	const int32 CurrentLevel = FMath::Clamp(Profile->GamblersTokenUnlockedLevel, 0, 6);
	const int32 NextSequentialLevel = FMath::Clamp(CurrentLevel + 1, 1, 6);
	const int32 DifficultyFloor = GetGamblersTokenDifficultyFloor(Difficulty);
	const int32 NewLevel = FMath::Clamp(FMath::Max(NextSequentialLevel, DifficultyFloor), 1, 6);

	if (NewLevel == CurrentLevel)
	{
		return CurrentLevel;
	}

	Profile->GamblersTokenUnlockedLevel = NewLevel;
	TArray<FName> NewlyUnlocked;
	const bool bExtraAchievementsChanged = UpdateMilestoneAchievements(
		TEXT("ACH_EXT_TOKEN_"),
		GetExtraGamblerTokenThresholds(),
		NewLevel,
		&NewlyUnlocked);
	MarkDirtyAndMaybeSave(true);
	AchievementsStateChanged.Broadcast();
	if (bExtraAchievementsChanged && NewlyUnlocked.Num() > 0)
	{
		AchievementsUnlocked.Broadcast(NewlyUnlocked);
	}
	return NewLevel;
}

bool UT66AchievementsSubsystem::TryClaimAchievement(FName AchievementID)
{
	if (!Profile) LoadOrCreateProfile();
	if (!Profile || AchievementID.IsNone()) return false;

	// Find definition (reward amount, tier, etc.)
	const FAchievementData* Def = CachedDefinitions.FindByPredicate([&](const FAchievementData& A) { return A.AchievementID == AchievementID; });
	if (!Def) return false;

	FT66AchievementState* S = FindOrAddState(AchievementID);
	if (!S) return false;
	if (!S->bIsUnlocked || S->bIsClaimed) return false;

	S->bIsClaimed = true;
	Profile->AchievementCoinsBalance = FMath::Clamp(Profile->AchievementCoinsBalance + FMath::Max(0, Def->RewardCoins), 0, 2000000000);

	MarkDirtyAndMaybeSave(true);
	AchievementCoinsChanged.Broadcast();
	AchievementsStateChanged.Broadcast();
	return true;
}

void UT66AchievementsSubsystem::ResetProfileProgress()
{
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;

	Profile->AchievementCoinsBalance = 0;
	Profile->AchievementStateByID.Reset();
	Profile->LifetimeEnemiesKilled = 0;
	Profile->LifetimeBossesKilled = 0;
	Profile->LifetimeStagesCleared = 0;
	Profile->LifetimeRunsCompleted = 0;
	Profile->LifetimeVendorPurchases = 0;
	Profile->LifetimeGamblerWins = 0;
	Profile->CompanionUnionStagesClearedByID.Reset();

	MarkDirtyAndMaybeSave(true);
	AchievementCoinsChanged.Broadcast();
	AchievementsStateChanged.Broadcast();

	RebuildDefinitions();
}

int32 UT66AchievementsSubsystem::GetCompanionUnionStagesCleared(FName CompanionID) const
{
	if (!Profile || CompanionID.IsNone()) return 0;
	const int32* Found = Profile->CompanionUnionStagesClearedByID.Find(CompanionID);
	return Found ? FMath::Max(0, *Found) : 0;
}

float UT66AchievementsSubsystem::GetCompanionUnionProgress01(FName CompanionID) const
{
	if (CompanionID.IsNone()) return 0.f;
	const int32 Stages = GetCompanionUnionStagesCleared(CompanionID);
	return (UnionTier_HyperStages <= 0)
		? 0.f
		: FMath::Clamp(static_cast<float>(Stages) / static_cast<float>(UnionTier_HyperStages), 0.f, 1.f);
}

float UT66AchievementsSubsystem::GetCompanionUnionHealingIntervalSeconds(FName CompanionID) const
{
	const int32 Stages = GetCompanionUnionStagesCleared(CompanionID);
	if (Stages >= UnionTier_HyperStages) return UnionHealInterval_HyperSeconds;
	if (Stages >= UnionTier_MediumStages) return UnionHealInterval_MediumSeconds;
	if (Stages >= UnionTier_GoodStages) return UnionHealInterval_GoodSeconds;
	return UnionHealInterval_BasicSeconds;
}

void UT66AchievementsSubsystem::AddCompanionUnionStagesCleared(FName CompanionID, int32 DeltaStagesCleared)
{
	if (CompanionID.IsNone()) return;
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;

	const int32 Delta = FMath::Clamp(DeltaStagesCleared, 0, 1000000);
	if (Delta <= 0) return;

	const int32 Prev = GetCompanionUnionStagesCleared(CompanionID);
	const int32 Next = FMath::Clamp(Prev + Delta, 0, 2000000000);
	Profile->CompanionUnionStagesClearedByID.FindOrAdd(CompanionID) = Next;

	// Union achievements: max stages cleared with any single companion
	int32 MaxStages = 0;
	for (const TPair<FName, int32>& Pair : Profile->CompanionUnionStagesClearedByID)
		MaxStages = FMath::Max(MaxStages, Pair.Value);
	TArray<FName> NewlyUnlocked;
	bool bAnyChanged = false;
	bAnyChanged |= UpdateCountAchievement(FName(TEXT("ACH_BLK_008")), MaxStages, 1, &NewlyUnlocked);
	bAnyChanged |= UpdateCountAchievement(FName(TEXT("ACH_RED_004")), MaxStages, 5, &NewlyUnlocked);
	bAnyChanged |= UpdateCountAchievement(FName(TEXT("ACH_YEL_002")), MaxStages, 20, &NewlyUnlocked);
	bAnyChanged |= UpdateMilestoneAchievements(TEXT("ACH_EXT_UNION_"), GetExtraUnionThresholds(), MaxStages, &NewlyUnlocked);

	const bool bTierCrossed =
		(Prev < UnionTier_GoodStages && Next >= UnionTier_GoodStages) ||
		(Prev < UnionTier_MediumStages && Next >= UnionTier_MediumStages) ||
		(Prev < UnionTier_HyperStages && Next >= UnionTier_HyperStages);
	if (bTierCrossed || bAnyChanged) { MarkDirtyAndMaybeSave(true); AchievementsStateChanged.Broadcast(); if (NewlyUnlocked.Num() > 0) AchievementsUnlocked.Broadcast(NewlyUnlocked); }
	else MarkDirtyAndMaybeSave(false);
}

