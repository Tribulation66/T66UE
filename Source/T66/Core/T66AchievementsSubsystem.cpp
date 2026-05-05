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
			return 50;
		case ET66AchievementTier::Yellow:
			return 30;
		case ET66AchievementTier::Red:
			return 15;
		case ET66AchievementTier::Black:
		default:
			return 5;
		}
	}

	int32 NormalizeLegacyRewardCoins(const int32 LegacyRewardCoins)
	{
		if (LegacyRewardCoins <= 0)
		{
			return 0;
		}

		return FMath::Clamp(FMath::RoundToInt(static_cast<float>(LegacyRewardCoins) / 15.f), 5, 50);
	}

	void MigrateHeroOwnedSkinsMap(TMap<FName, FT66OwnedSkinsList>& HeroSkinMap, FName (*MigrateHeroID)(FName))
	{
		TMap<FName, FT66OwnedSkinsList> Migrated;
		for (const auto& Pair : HeroSkinMap)
		{
			const FName NewKey = MigrateHeroID(Pair.Key);
			FT66OwnedSkinsList& TargetList = Migrated.FindOrAdd(NewKey);
			for (const FName& SkinID : Pair.Value.SkinIDs)
			{
				TargetList.SkinIDs.AddUnique(SkinID);
			}
		}
		HeroSkinMap = MoveTemp(Migrated);
	}

	template <typename TValue>
	void MigrateHeroKeyMapReplace(TMap<FName, TValue>& HeroMap, FName (*MigrateHeroID)(FName))
	{
		TMap<FName, TValue> Migrated;
		for (const auto& Pair : HeroMap)
		{
			Migrated.Add(MigrateHeroID(Pair.Key), Pair.Value);
		}
		HeroMap = MoveTemp(Migrated);
	}

	void MigrateHeroIntMapAdditive(TMap<FName, int32>& HeroMap, FName (*MigrateHeroID)(FName))
	{
		TMap<FName, int32> Migrated;
		for (const auto& Pair : HeroMap)
		{
			Migrated.FindOrAdd(MigrateHeroID(Pair.Key)) += Pair.Value;
		}
		HeroMap = MoveTemp(Migrated);
	}

	void MigrateHeroMedalMap(TMap<FName, ET66AccountMedalTier>& HeroMap, FName (*MigrateHeroID)(FName))
	{
		TMap<FName, ET66AccountMedalTier> Migrated;
		for (const auto& Pair : HeroMap)
		{
			ET66AccountMedalTier& Target = Migrated.FindOrAdd(MigrateHeroID(Pair.Key));
			if (static_cast<uint8>(Pair.Value) > static_cast<uint8>(Target))
			{
				Target = Pair.Value;
			}
		}
		HeroMap = MoveTemp(Migrated);
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

	constexpr int32 SpecialCompanionSkinRequirement = UT66AchievementsSubsystem::UnionTier_MediumStages;

	FName MakeSpecialCompanionSkinAchievementID(const FName CompanionID)
	{
		return CompanionID.IsNone()
			? NAME_None
			: FName(*FString::Printf(TEXT("ACH_SPC_COMP_%s"), *CompanionID.ToString()));
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
		A.Category = ET66AchievementCategory::Standard;
		A.RewardType = ET66AchievementRewardType::ChadCoupons;
		A.Tier = GetGeneratedAchievementTier(Index, Total);
		A.RewardCoins = GetGeneratedRewardCoins(A.Tier);
		Out.Add(A);
	}

	void AddSpecialSkinAchievement(
		TArray<FAchievementData>& Out,
		FName Id,
		ET66AchievementTier Tier,
		int32 RequirementCount,
		ET66AchievementRewardEntityType RewardEntityType,
		FName RewardEntityID,
		FName RewardSkinID,
		const FText& DisplayName,
		const FText& Description)
	{
		FAchievementData A;
		A.AchievementID = Id;
		A.Tier = Tier;
		A.Category = ET66AchievementCategory::Special;
		A.RewardType = ET66AchievementRewardType::SkinUnlock;
		A.RewardCoins = 0;
		A.RewardEntityType = RewardEntityType;
		A.RewardEntityID = RewardEntityID;
		A.RewardSkinID = RewardSkinID;
		A.RequirementCount = RequirementCount;
		A.DisplayName = DisplayName;
		A.Description = Description;
		Out.Add(A);
	}

	ET66AccountMedalTier ClampMedalTier(const ET66AccountMedalTier Tier)
	{
		const int32 Value = FMath::Clamp(static_cast<int32>(Tier), static_cast<int32>(ET66AccountMedalTier::None), static_cast<int32>(ET66AccountMedalTier::Diamond));
		return static_cast<ET66AccountMedalTier>(Value);
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
		Profile->ChadCouponsBalance = 10;
		Profile->AchievementCoinsBalance = 0;
		UE_LOG(LogT66Achievements, Log, TEXT("[Progression] LoadOrCreateProfile: Created fresh profile (starting CC=%d)"), Profile->ChadCouponsBalance);
	}
	else
	{
		UE_LOG(LogT66Achievements, Log, TEXT("[Progression] LoadOrCreateProfile: Loaded existing profile from save file"));
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
	Profile->GamblersTokenUnlockedLevel = FMath::Clamp(Profile->GamblersTokenUnlockedLevel, 0, 5);

	// Historical migration: early hero renumbering (Hero_1 removed, Hero_2..5 -> Hero_1..4). SaveVersion 9.
	if (LoadedSaveVersion < 9)
	{
		MigrateHeroOwnedSkinsMap(Profile->OwnedHeroSkinsByHero, T66MigratePreSaveVersion9HeroID);
		MigrateHeroKeyMapReplace(Profile->EquippedHeroSkinIDByHero, T66MigratePreSaveVersion9HeroID);
		Profile->SaveVersion = 9;
		bProfileDirty = true;
		UE_LOG(LogT66Achievements, Log, TEXT("[SKIN] LoadOrCreateProfile: Applied HeroID renumber migration (SaveVersion 9)"));
	}

	if (LoadedSaveVersion < 12)
	{
		Profile->ChadCouponsBalance = 10;
		Profile->AchievementCoinsBalance = 0;
		Profile->SaveVersion = 12;
		bProfileDirty = true;
		UE_LOG(LogT66Achievements, Log, TEXT("[Progression] LoadOrCreateProfile: Reset legacy AC/PC economy to Chad Coupons (SaveVersion 12)."));
	}

	if (LoadedSaveVersion < 13)
	{
		Profile->SaveVersion = 13;
		bProfileDirty = true;
		UE_LOG(LogT66Achievements, Log, TEXT("[Account] LoadOrCreateProfile: Added cumulative score tracking (SaveVersion 13)."));
	}

	if (LoadedSaveVersion < 14)
	{
		Profile->SaveVersion = 14;
		bProfileDirty = true;
		UE_LOG(LogT66Achievements, Log, TEXT("[Account] LoadOrCreateProfile: Added companion total healing tracking (SaveVersion 14)."));
	}

	if (LoadedSaveVersion < 15)
	{
		Profile->SaveVersion = 15;
		bProfileDirty = true;
		UE_LOG(LogT66Achievements, Log, TEXT("[Account] LoadOrCreateProfile: Added remembered hero/companion defaults (SaveVersion 15)."));
	}

	if (LoadedSaveVersion >= 9 && LoadedSaveVersion < T66SparseActiveHeroIdProfileSaveVersion)
	{
		MigrateHeroOwnedSkinsMap(Profile->OwnedHeroSkinsByHero, T66MigrateSparseActiveHeroID);
		MigrateHeroKeyMapReplace(Profile->EquippedHeroSkinIDByHero, T66MigrateSparseActiveHeroID);
		MigrateHeroIntMapAdditive(Profile->HeroUnityStagesClearedByID, T66MigrateSparseActiveHeroID);
		MigrateHeroIntMapAdditive(Profile->HeroGamesPlayedByID, T66MigrateSparseActiveHeroID);
		MigrateHeroMedalMap(Profile->HeroHighestMedalByID, T66MigrateSparseActiveHeroID);
		MigrateHeroIntMapAdditive(Profile->HeroCumulativeScoreByID, T66MigrateSparseActiveHeroID);
		Profile->LastSelectedHeroID = T66MigrateSparseActiveHeroID(Profile->LastSelectedHeroID);
		Profile->SaveVersion = T66SparseActiveHeroIdProfileSaveVersion;
		bProfileDirty = true;
		UE_LOG(LogT66Achievements, Log, TEXT("[Account] LoadOrCreateProfile: Applied sparse active HeroID migration (SaveVersion %d)."), T66SparseActiveHeroIdProfileSaveVersion);
	}

	// Hero skins: log current state (no more auto-reset; purchases persist).
	const FName DefaultSkin(TEXT("Default"));
	
	UE_LOG(LogT66Achievements, Log, TEXT("[SKIN] LoadOrCreateProfile: OwnedHeroSkinsByHero.Num=%d, CC Balance=%d"),
		Profile->OwnedHeroSkinsByHero.Num(), Profile->ChadCouponsBalance);
	
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

	for (TPair<FName, int32>& Pair : Profile->HeroUnityStagesClearedByID)
	{
		Pair.Value = FMath::Clamp(Pair.Value, 0, 2000000000);
	}

	for (TPair<FName, int32>& Pair : Profile->HeroGamesPlayedByID)
	{
		Pair.Value = FMath::Clamp(Pair.Value, 0, 2000000000);
	}

	for (TPair<FName, int32>& Pair : Profile->CompanionGamesPlayedByID)
	{
		Pair.Value = FMath::Clamp(Pair.Value, 0, 2000000000);
	}

	for (TPair<FName, ET66AccountMedalTier>& Pair : Profile->HeroHighestMedalByID)
	{
		Pair.Value = ClampMedalTier(Pair.Value);
	}

	for (TPair<FName, ET66AccountMedalTier>& Pair : Profile->CompanionHighestMedalByID)
	{
		Pair.Value = ClampMedalTier(Pair.Value);
	}

	for (TPair<FName, int32>& Pair : Profile->HeroCumulativeScoreByID)
	{
		Pair.Value = FMath::Clamp(Pair.Value, 0, 2000000000);
	}

	for (TPair<FName, int32>& Pair : Profile->CompanionCumulativeScoreByID)
	{
		Pair.Value = FMath::Clamp(Pair.Value, 0, 2000000000);
	}

	for (TPair<FName, int32>& Pair : Profile->CompanionTotalHealingByID)
	{
		Pair.Value = FMath::Clamp(Pair.Value, 0, 2000000000);
	}

	Profile->SaveVersion = FMath::Max(Profile->SaveVersion, T66SparseActiveHeroIdProfileSaveVersion);
}

int32 UT66AchievementsSubsystem::GetChadCouponBalance() const
{
	return Profile ? FMath::Max(0, Profile->ChadCouponsBalance) : 0;
}

int32 UT66AchievementsSubsystem::GetAccountLevel() const
{
	if (!Profile)
	{
		return 1;
	}

	constexpr int32 MaxAccountExperience = (AccountMaxLevel - 1) * AccountExperiencePerLevel;
	const int32 TotalExperience = FMath::Clamp(Profile->LifetimeRunsCompleted * AccountExperiencePerCompletedRun, 0, MaxAccountExperience);
	const int32 ClampedExperience = FMath::Clamp(TotalExperience, 0, MaxAccountExperience);
	return FMath::Clamp(1 + (ClampedExperience / AccountExperiencePerLevel), 1, AccountMaxLevel);
}

int32 UT66AchievementsSubsystem::GetAccountMaxLevel() const
{
	return AccountMaxLevel;
}

int32 UT66AchievementsSubsystem::GetAccountNextLevel() const
{
	return FMath::Clamp(GetAccountLevel() + 1, 1, AccountMaxLevel);
}

int32 UT66AchievementsSubsystem::GetAccountExperienceIntoLevel() const
{
	if (!Profile)
	{
		return 0;
	}

	if (GetAccountLevel() >= AccountMaxLevel)
	{
		return AccountExperiencePerLevel;
	}

	constexpr int32 MaxAccountExperience = (AccountMaxLevel - 1) * AccountExperiencePerLevel;
	const int32 TotalExperience = FMath::Clamp(Profile->LifetimeRunsCompleted * AccountExperiencePerCompletedRun, 0, MaxAccountExperience);
	return TotalExperience % AccountExperiencePerLevel;
}

int32 UT66AchievementsSubsystem::GetAccountExperienceToNextLevel() const
{
	return AccountExperiencePerLevel;
}

float UT66AchievementsSubsystem::GetAccountLevelProgress01() const
{
	if (GetAccountLevel() >= AccountMaxLevel)
	{
		return 1.f;
	}

	return FMath::Clamp(
		static_cast<float>(GetAccountExperienceIntoLevel()) / static_cast<float>(FMath::Max(1, AccountExperiencePerLevel)),
		0.f,
		1.f);
}

bool UT66AchievementsSubsystem::SpendChadCoupons(int32 Amount)
{
	if (!Profile)
	{
		LoadOrCreateProfile();
	}
	if (!Profile || Amount <= 0 || Profile->ChadCouponsBalance < Amount) return false;
	Profile->ChadCouponsBalance -= Amount;
	bProfileDirty = true;
	SaveProfileIfNeeded(true);
	AchievementCoinsChanged.Broadcast();
	return true;
}

void UT66AchievementsSubsystem::AddChadCoupons(int32 Amount)
{
	if (!Profile)
	{
		LoadOrCreateProfile();
	}
	if (!Profile || Amount <= 0)
	{
		return;
	}

	Profile->ChadCouponsBalance = FMath::Clamp(Profile->ChadCouponsBalance + Amount, 0, 2000000000);
	bProfileDirty = true;
	SaveProfileIfNeeded(true);
	AchievementCoinsChanged.Broadcast();
}

int32 UT66AchievementsSubsystem::GetAchievementCoinsBalance() const
{
	return GetChadCouponBalance();
}

bool UT66AchievementsSubsystem::SpendAchievementCoins(int32 Amount)
{
	return SpendChadCoupons(Amount);
}

void UT66AchievementsSubsystem::AddAchievementCoins(int32 Amount)
{
	AddChadCoupons(Amount);
}

FName UT66AchievementsSubsystem::GetLastSelectedHeroID() const
{
	return Profile ? Profile->LastSelectedHeroID : NAME_None;
}

FName UT66AchievementsSubsystem::GetLastSelectedCompanionID() const
{
	return Profile ? Profile->LastSelectedCompanionID : NAME_None;
}

void UT66AchievementsSubsystem::RememberLastSelectedLoadout(FName HeroID, FName CompanionID)
{
	if (!Profile)
	{
		LoadOrCreateProfile();
	}
	if (!Profile)
	{
		return;
	}

	bool bChanged = false;
	if (!HeroID.IsNone() && Profile->LastSelectedHeroID != HeroID)
	{
		Profile->LastSelectedHeroID = HeroID;
		bChanged = true;
	}
	if (Profile->LastSelectedCompanionID != CompanionID)
	{
		Profile->LastSelectedCompanionID = CompanionID;
		bChanged = true;
	}

	if (bChanged)
	{
		MarkProfileDirtyAndSave(false);
	}
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
	A.Category = ET66AchievementCategory::Standard;
	A.RewardType = ET66AchievementRewardType::ChadCoupons;
	A.RequirementCount = RequirementCount;
	A.RewardCoins = NormalizeLegacyRewardCoins(RewardCoins);
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

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		const FText BeachgoerSkinName = Loc
			? Loc->GetText_SkinName(UT66SkinSubsystem::BeachgoerSkinID)
			: FText::FromName(UT66SkinSubsystem::BeachgoerSkinID);
		for (const FName CompanionID : GI->GetAllCompanionIDs())
		{
			if (CompanionID.IsNone())
			{
				continue;
			}

			FCompanionData CompanionData;
			const bool bHasCompanionData = GI->GetCompanionData(CompanionID, CompanionData);
			const FText CompanionName = bHasCompanionData
				? (Loc ? Loc->GetCompanionDisplayName(CompanionData) : (!CompanionData.DisplayName.IsEmpty() ? CompanionData.DisplayName : FText::FromName(CompanionID)))
				: FText::FromName(CompanionID);

			AddSpecialSkinAchievement(
				CachedDefinitions,
				MakeSpecialCompanionSkinAchievementID(CompanionID),
				ET66AchievementTier::White,
				SpecialCompanionSkinRequirement,
				ET66AchievementRewardEntityType::Companion,
				CompanionID,
				UT66SkinSubsystem::BeachgoerSkinID,
				FText::Format(
					NSLOCTEXT("T66.Achievements", "SpecialCompanionSkinTitle", "{0} Devotion"),
					CompanionName),
				FText::Format(
					NSLOCTEXT("T66.Achievements", "SpecialCompanionSkinDesc", "Clear {0} stages with {1} to unlock the {2} outfit."),
					FText::AsNumber(SpecialCompanionSkinRequirement),
					CompanionName,
					BeachgoerSkinName));
		}
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

TArray<FAchievementData> UT66AchievementsSubsystem::GetAchievementsForCategory(ET66AchievementCategory Category) const
{
	TArray<FAchievementData> Out;
	for (const FAchievementData& A : CachedDefinitions)
	{
		if (A.Category == Category)
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

bool UT66AchievementsSubsystem::DoesClaimedAchievementGrantSkin(
	const ET66AchievementRewardEntityType EntityType,
	const FName EntityID,
	const FName SkinID) const
{
	if (EntityID.IsNone() || SkinID.IsNone())
	{
		return false;
	}

	for (const FAchievementData& Achievement : CachedDefinitions)
	{
		if (Achievement.RewardType != ET66AchievementRewardType::SkinUnlock)
		{
			continue;
		}
		if (Achievement.RewardEntityType != EntityType
			|| Achievement.RewardEntityID != EntityID
			|| Achievement.RewardSkinID != SkinID)
		{
			continue;
		}

		const FT66AchievementState* State = FindState(Achievement.AchievementID);
		if (State && State->bIsClaimed)
		{
			return true;
		}
	}

	return false;
}

void UT66AchievementsSubsystem::SaveProfileIfNeeded(bool bForce)
{
	if (!Profile) return;
	if (!bProfileDirty && !bForce) return;

	if (bForce || !GetWorld())
	{
		// Critical profile transactions must survive immediate level travel, so forced saves are synchronous.
		const bool bSaved = UGameplayStatics::SaveGameToSlot(Profile, ProfileSaveSlotName, ProfileSaveUserIndex);
		UE_LOG(LogT66Achievements, Verbose, TEXT("[GOLD] SaveProfile: blocking achievement profile save complete=%d (forced=%d)"), bSaved ? 1 : 0, bForce ? 1 : 0);
		if (bSaved)
		{
			bProfileDirty = false;
			if (GetWorld())
			{
				LastProfileSaveWorldSeconds = static_cast<float>(GetWorld()->GetTimeSeconds());
			}
		}
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

	const int32 OldAccountLevel = GetAccountLevel();
	const int32 Delta = 1;
	Profile->LifetimeRunsCompleted = FMath::Clamp(Profile->LifetimeRunsCompleted + Delta, 0, 2000000000);
	const int32 TotalRuns = Profile->LifetimeRunsCompleted;
	TArray<FName> NewlyUnlocked;
	bool bAnyChanged = true;
	const int32 NewAccountLevel = GetAccountLevel();
	const int32 AccountLevelsGained = FMath::Max(0, NewAccountLevel - OldAccountLevel);
	if (AccountLevelsGained > 0 && ChadCouponsPerAccountLevel > 0)
	{
		Profile->ChadCouponsBalance = FMath::Clamp(
			Profile->ChadCouponsBalance + (AccountLevelsGained * ChadCouponsPerAccountLevel),
			0,
			2000000000);
		AchievementCoinsChanged.Broadcast();
	}

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
	return Profile ? FMath::Clamp(Profile->GamblersTokenUnlockedLevel, 0, 5) : 0;
}

int32 UT66AchievementsSubsystem::GetGamblersTokenDifficultyFloor(ET66Difficulty Difficulty)
{
	const int32 DifficultyIndex = static_cast<int32>(Difficulty);
	return FMath::Clamp(DifficultyIndex + 1, 1, 5);
}

int32 UT66AchievementsSubsystem::UpgradeGamblersTokenForDifficulty(ET66Difficulty Difficulty)
{
	if (!Profile) LoadOrCreateProfile();
	if (!Profile)
	{
		return 0;
	}

	const int32 CurrentLevel = FMath::Clamp(Profile->GamblersTokenUnlockedLevel, 0, 5);
	const int32 NextSequentialLevel = FMath::Clamp(CurrentLevel + 1, 1, 5);
	const int32 DifficultyFloor = GetGamblersTokenDifficultyFloor(Difficulty);
	const int32 NewLevel = FMath::Clamp(FMath::Max(NextSequentialLevel, DifficultyFloor), 1, 5);

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
	const bool bRewardsCoins = Def->RewardType == ET66AchievementRewardType::ChadCoupons;
	const bool bRewardsSkin = Def->RewardType == ET66AchievementRewardType::SkinUnlock;
	if (bRewardsCoins)
	{
		Profile->ChadCouponsBalance = FMath::Clamp(Profile->ChadCouponsBalance + FMath::Max(0, Def->RewardCoins), 0, 2000000000);
	}

	MarkDirtyAndMaybeSave(true);
	if (bRewardsCoins)
	{
		AchievementCoinsChanged.Broadcast();
	}
	AchievementsStateChanged.Broadcast();
	if (bRewardsSkin)
	{
		if (UT66SkinSubsystem* SkinSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SkinSubsystem>() : nullptr)
		{
			SkinSubsystem->OnSkinStateChanged.Broadcast();
		}
	}
	return true;
}

void UT66AchievementsSubsystem::ResetProfileProgress()
{
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;

	Profile->ChadCouponsBalance = 0;
	Profile->AchievementCoinsBalance = 0;
	Profile->AchievementStateByID.Reset();
	Profile->LifetimeEnemiesKilled = 0;
	Profile->LifetimeBossesKilled = 0;
	Profile->LifetimeStagesCleared = 0;
	Profile->LifetimeRunsCompleted = 0;
	Profile->LifetimeVendorPurchases = 0;
	Profile->LifetimeGamblerWins = 0;
	Profile->CompanionUnionStagesClearedByID.Reset();
	Profile->HeroUnityStagesClearedByID.Reset();
	Profile->HeroGamesPlayedByID.Reset();
	Profile->HeroHighestMedalByID.Reset();
	Profile->HeroCumulativeScoreByID.Reset();
	Profile->CompanionGamesPlayedByID.Reset();
	Profile->CompanionHighestMedalByID.Reset();
	Profile->CompanionCumulativeScoreByID.Reset();
	Profile->CompanionTotalHealingByID.Reset();

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

int32 UT66AchievementsSubsystem::GetHeroUnityStagesCleared(FName HeroID) const
{
	if (!Profile || HeroID.IsNone()) return 0;
	const int32* Found = Profile->HeroUnityStagesClearedByID.Find(HeroID);
	return Found ? FMath::Max(0, *Found) : 0;
}

int32 UT66AchievementsSubsystem::GetLifetimeStagesCleared() const
{
	return Profile ? FMath::Max(0, Profile->LifetimeStagesCleared) : 0;
}

float UT66AchievementsSubsystem::GetCompanionUnionProgress01(FName CompanionID) const
{
	if (CompanionID.IsNone()) return 0.f;
	const int32 Stages = GetCompanionUnionStagesCleared(CompanionID);
	return (UnionTier_HyperStages <= 0)
		? 0.f
		: FMath::Clamp(static_cast<float>(Stages) / static_cast<float>(UnionTier_HyperStages), 0.f, 1.f);
}

float UT66AchievementsSubsystem::GetHeroUnityProgress01(FName HeroID) const
{
	if (HeroID.IsNone()) return 0.f;
	const int32 Stages = GetHeroUnityStagesCleared(HeroID);
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
	bAnyChanged |= UpdateCountAchievement(
		MakeSpecialCompanionSkinAchievementID(CompanionID),
		Next,
		SpecialCompanionSkinRequirement,
		&NewlyUnlocked);

	const bool bTierCrossed =
		(Prev < UnionTier_GoodStages && Next >= UnionTier_GoodStages) ||
		(Prev < UnionTier_MediumStages && Next >= UnionTier_MediumStages) ||
		(Prev < UnionTier_HyperStages && Next >= UnionTier_HyperStages);
	if (bTierCrossed || bAnyChanged) { MarkDirtyAndMaybeSave(true); AchievementsStateChanged.Broadcast(); if (NewlyUnlocked.Num() > 0) AchievementsUnlocked.Broadcast(NewlyUnlocked); }
	else MarkDirtyAndMaybeSave(false);
}

void UT66AchievementsSubsystem::AddHeroUnityStagesCleared(FName HeroID, int32 DeltaStagesCleared)
{
	if (HeroID.IsNone()) return;
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;

	const int32 Delta = FMath::Clamp(DeltaStagesCleared, 0, 1000000);
	if (Delta <= 0) return;

	const int32 Prev = GetHeroUnityStagesCleared(HeroID);
	const int32 Next = FMath::Clamp(Prev + Delta, 0, 2000000000);
	Profile->HeroUnityStagesClearedByID.FindOrAdd(HeroID) = Next;

	const bool bTierCrossed =
		(Prev < UnionTier_GoodStages && Next >= UnionTier_GoodStages) ||
		(Prev < UnionTier_MediumStages && Next >= UnionTier_MediumStages) ||
		(Prev < UnionTier_HyperStages && Next >= UnionTier_HyperStages);

	MarkDirtyAndMaybeSave(bTierCrossed);
	if (bTierCrossed)
	{
		AchievementsStateChanged.Broadcast();
	}
}

int32 UT66AchievementsSubsystem::GetHeroGamesPlayed(FName HeroID) const
{
	if (!Profile || HeroID.IsNone()) return 0;
	const int32* Found = Profile->HeroGamesPlayedByID.Find(HeroID);
	return Found ? FMath::Max(0, *Found) : 0;
}

int32 UT66AchievementsSubsystem::GetCompanionGamesPlayed(FName CompanionID) const
{
	if (!Profile || CompanionID.IsNone()) return 0;
	const int32* Found = Profile->CompanionGamesPlayedByID.Find(CompanionID);
	return Found ? FMath::Max(0, *Found) : 0;
}

int32 UT66AchievementsSubsystem::GetHeroCumulativeScore(FName HeroID) const
{
	if (!Profile || HeroID.IsNone()) return 0;
	const int32* Found = Profile->HeroCumulativeScoreByID.Find(HeroID);
	return Found ? FMath::Max(0, *Found) : 0;
}

int32 UT66AchievementsSubsystem::GetCompanionCumulativeScore(FName CompanionID) const
{
	if (!Profile || CompanionID.IsNone()) return 0;
	const int32* Found = Profile->CompanionCumulativeScoreByID.Find(CompanionID);
	return Found ? FMath::Max(0, *Found) : 0;
}

int32 UT66AchievementsSubsystem::GetCompanionTotalHealing(FName CompanionID) const
{
	if (!Profile || CompanionID.IsNone()) return 0;
	const int32* Found = Profile->CompanionTotalHealingByID.Find(CompanionID);
	return Found ? FMath::Max(0, *Found) : 0;
}

ET66AccountMedalTier UT66AchievementsSubsystem::GetHeroHighestMedal(FName HeroID) const
{
	if (!Profile || HeroID.IsNone()) return ET66AccountMedalTier::None;
	const ET66AccountMedalTier* Found = Profile->HeroHighestMedalByID.Find(HeroID);
	return Found ? ClampMedalTier(*Found) : ET66AccountMedalTier::None;
}

ET66AccountMedalTier UT66AchievementsSubsystem::GetCompanionHighestMedal(FName CompanionID) const
{
	if (!Profile || CompanionID.IsNone()) return ET66AccountMedalTier::None;
	const ET66AccountMedalTier* Found = Profile->CompanionHighestMedalByID.Find(CompanionID);
	return Found ? ClampMedalTier(*Found) : ET66AccountMedalTier::None;
}

void UT66AchievementsSubsystem::AddHeroGamesPlayed(FName HeroID, int32 DeltaGamesPlayed)
{
	if (HeroID.IsNone()) return;
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;

	const int32 Delta = FMath::Clamp(DeltaGamesPlayed, 0, 1000000);
	if (Delta <= 0) return;

	const int32 Prev = GetHeroGamesPlayed(HeroID);
	Profile->HeroGamesPlayedByID.FindOrAdd(HeroID) = FMath::Clamp(Prev + Delta, 0, 2000000000);
	MarkDirtyAndMaybeSave(false);
}

void UT66AchievementsSubsystem::AddCompanionGamesPlayed(FName CompanionID, int32 DeltaGamesPlayed)
{
	if (CompanionID.IsNone()) return;
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;

	const int32 Delta = FMath::Clamp(DeltaGamesPlayed, 0, 1000000);
	if (Delta <= 0) return;

	const int32 Prev = GetCompanionGamesPlayed(CompanionID);
	Profile->CompanionGamesPlayedByID.FindOrAdd(CompanionID) = FMath::Clamp(Prev + Delta, 0, 2000000000);
	MarkDirtyAndMaybeSave(false);
}

void UT66AchievementsSubsystem::AddHeroCumulativeScore(FName HeroID, int32 DeltaScore)
{
	if (HeroID.IsNone()) return;
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;

	const int32 Delta = FMath::Clamp(DeltaScore, 0, 2000000000);
	if (Delta <= 0) return;

	const int32 Prev = GetHeroCumulativeScore(HeroID);
	Profile->HeroCumulativeScoreByID.FindOrAdd(HeroID) = FMath::Clamp(Prev + Delta, 0, 2000000000);
	MarkDirtyAndMaybeSave(false);
}

void UT66AchievementsSubsystem::AddCompanionCumulativeScore(FName CompanionID, int32 DeltaScore)
{
	if (CompanionID.IsNone()) return;
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;

	const int32 Delta = FMath::Clamp(DeltaScore, 0, 2000000000);
	if (Delta <= 0) return;

	const int32 Prev = GetCompanionCumulativeScore(CompanionID);
	Profile->CompanionCumulativeScoreByID.FindOrAdd(CompanionID) = FMath::Clamp(Prev + Delta, 0, 2000000000);
	MarkDirtyAndMaybeSave(false);
}

void UT66AchievementsSubsystem::AddCompanionTotalHealing(FName CompanionID, int32 DeltaHealing)
{
	if (CompanionID.IsNone()) return;
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;

	const int32 Delta = FMath::Clamp(DeltaHealing, 0, 1000000000);
	if (Delta <= 0) return;

	const int32 Prev = GetCompanionTotalHealing(CompanionID);
	Profile->CompanionTotalHealingByID.FindOrAdd(CompanionID) = FMath::Clamp(Prev + Delta, 0, 2000000000);
	MarkDirtyAndMaybeSave(false);
}

ET66AccountMedalTier UT66AchievementsSubsystem::MedalTierForDifficulty(ET66Difficulty Difficulty)
{
	switch (Difficulty)
	{
	case ET66Difficulty::Easy:
		return ET66AccountMedalTier::Bronze;
	case ET66Difficulty::Medium:
		return ET66AccountMedalTier::Silver;
	case ET66Difficulty::Hard:
		return ET66AccountMedalTier::Gold;
	case ET66Difficulty::VeryHard:
		return ET66AccountMedalTier::Platinum;
	case ET66Difficulty::Impossible:
		return ET66AccountMedalTier::Diamond;
	default:
		return ET66AccountMedalTier::None;
	}
}

void UT66AchievementsSubsystem::RecordHeroDifficultyClear(FName HeroID, ET66Difficulty Difficulty)
{
	if (HeroID.IsNone()) return;
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;

	const ET66AccountMedalTier NewTier = MedalTierForDifficulty(Difficulty);
	const ET66AccountMedalTier PrevTier = GetHeroHighestMedal(HeroID);
	if (static_cast<int32>(NewTier) <= static_cast<int32>(PrevTier))
	{
		return;
	}

	Profile->HeroHighestMedalByID.FindOrAdd(HeroID) = NewTier;
	MarkDirtyAndMaybeSave(true);
	AchievementsStateChanged.Broadcast();
}

void UT66AchievementsSubsystem::RecordCompanionDifficultyClear(FName CompanionID, ET66Difficulty Difficulty)
{
	if (CompanionID.IsNone()) return;
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;

	const ET66AccountMedalTier NewTier = MedalTierForDifficulty(Difficulty);
	const ET66AccountMedalTier PrevTier = GetCompanionHighestMedal(CompanionID);
	if (static_cast<int32>(NewTier) <= static_cast<int32>(PrevTier))
	{
		return;
	}

	Profile->CompanionHighestMedalByID.FindOrAdd(CompanionID) = NewTier;
	MarkDirtyAndMaybeSave(true);
	AchievementsStateChanged.Broadcast();
}

