// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66SkinSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66ProfileSaveGame.h"
#include "Kismet/GameplayStatics.h"

const FName UT66SkinSubsystem::DefaultSkinID(TEXT("Default"));
const FName UT66SkinSubsystem::BeachgoerSkinID(TEXT("Beachgoer"));

void UT66SkinSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

TArray<FName> UT66SkinSubsystem::GetAllSkinIDs()
{
	return { DefaultSkinID, BeachgoerSkinID };
}

UT66ProfileSaveGame* UT66SkinSubsystem::GetProfile() const
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return nullptr;
	UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>();
	return Ach ? Ach->GetProfile() : nullptr;
}

void UT66SkinSubsystem::MarkProfileDirtyAndSave(bool bBroadcastCoinsChanged)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Ach->MarkProfileDirtyAndSave(bBroadcastCoinsChanged);
		}
	}
}

bool UT66SkinSubsystem::IsSkinOwned(ET66SkinEntityType EntityType, FName EntityID, FName SkinID) const
{
	if (EntityID.IsNone()) return false;
	if (SkinID == DefaultSkinID) return true;
	UT66ProfileSaveGame* Profile = GetProfile();
	if (!Profile)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (const UT66AchievementsSubsystem* Achievements = GI->GetSubsystem<UT66AchievementsSubsystem>())
			{
				const ET66AchievementRewardEntityType RewardEntityType = (EntityType == ET66SkinEntityType::Hero)
					? ET66AchievementRewardEntityType::Hero
					: ET66AchievementRewardEntityType::Companion;
				return Achievements->DoesClaimedAchievementGrantSkin(RewardEntityType, EntityID, SkinID);
			}
		}
		return false;
	}

	bool bOwnedByProfile = false;
	if (EntityType == ET66SkinEntityType::Hero)
	{
		const FT66OwnedSkinsList* Entry = Profile->OwnedHeroSkinsByHero.Find(EntityID);
		bOwnedByProfile = Entry && Entry->SkinIDs.Contains(SkinID);
	}
	else
	{
		const FT66OwnedSkinsList* Entry = Profile->OwnedCompanionSkinsByCompanion.Find(EntityID);
		bOwnedByProfile = Entry && Entry->SkinIDs.Contains(SkinID);
	}

	if (bOwnedByProfile)
	{
		return true;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (const UT66AchievementsSubsystem* Achievements = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			const ET66AchievementRewardEntityType RewardEntityType = (EntityType == ET66SkinEntityType::Hero)
				? ET66AchievementRewardEntityType::Hero
				: ET66AchievementRewardEntityType::Companion;
			return Achievements->DoesClaimedAchievementGrantSkin(RewardEntityType, EntityID, SkinID);
		}
	}

	return false;
}

bool UT66SkinSubsystem::PurchaseSkin(ET66SkinEntityType EntityType, FName EntityID, FName SkinID, int32 CostAC)
{
	if (EntityID.IsNone() || SkinID.IsNone() || CostAC <= 0) return false;
	if (SkinID == DefaultSkinID) return false;
	if (IsSkinOwned(EntityType, EntityID, SkinID)) return false;

	UT66ProfileSaveGame* Profile = GetProfile();
	if (!Profile || Profile->ChadCouponsBalance < CostAC) return false;

	Profile->ChadCouponsBalance -= CostAC;

	if (EntityType == ET66SkinEntityType::Hero)
	{
		FT66OwnedSkinsList& Entry = Profile->OwnedHeroSkinsByHero.FindOrAdd(EntityID);
		Entry.SkinIDs.AddUnique(SkinID);
		Profile->EquippedHeroSkinIDByHero.FindOrAdd(EntityID) = SkinID;
	}
	else
	{
		FT66OwnedSkinsList& Entry = Profile->OwnedCompanionSkinsByCompanion.FindOrAdd(EntityID);
		Entry.SkinIDs.AddUnique(SkinID);
		Profile->EquippedCompanionSkinIDByCompanion.FindOrAdd(EntityID) = SkinID;
	}

	MarkProfileDirtyAndSave(true);
	OnSkinStateChanged.Broadcast();
	return true;
}

bool UT66SkinSubsystem::RefundSkin(ET66SkinEntityType EntityType, FName EntityID, FName SkinID, int32 RefundAC)
{
	if (EntityID.IsNone() || SkinID.IsNone() || RefundAC <= 0) return false;
	if (SkinID == DefaultSkinID) return false;
	if (!IsSkinOwned(EntityType, EntityID, SkinID)) return false;

	UT66ProfileSaveGame* Profile = GetProfile();
	if (!Profile) return false;

	if (EntityType == ET66SkinEntityType::Hero)
	{
		if (FT66OwnedSkinsList* Entry = Profile->OwnedHeroSkinsByHero.Find(EntityID))
		{
			Entry->SkinIDs.Remove(SkinID);
			if (Entry->SkinIDs.Num() <= 0)
			{
				Profile->OwnedHeroSkinsByHero.Remove(EntityID);
			}
		}

		FName& EquippedSkinID = Profile->EquippedHeroSkinIDByHero.FindOrAdd(EntityID);
		if (EquippedSkinID == SkinID)
		{
			EquippedSkinID = DefaultSkinID;
		}
	}
	else
	{
		if (FT66OwnedSkinsList* Entry = Profile->OwnedCompanionSkinsByCompanion.Find(EntityID))
		{
			Entry->SkinIDs.Remove(SkinID);
			if (Entry->SkinIDs.Num() <= 0)
			{
				Profile->OwnedCompanionSkinsByCompanion.Remove(EntityID);
			}
		}

		FName& EquippedSkinID = Profile->EquippedCompanionSkinIDByCompanion.FindOrAdd(EntityID);
		if (EquippedSkinID == SkinID)
		{
			EquippedSkinID = DefaultSkinID;
		}
	}

	Profile->ChadCouponsBalance += RefundAC;
	MarkProfileDirtyAndSave(true);
	OnSkinStateChanged.Broadcast();
	return true;
}

FName UT66SkinSubsystem::GetEquippedSkin(ET66SkinEntityType EntityType, FName EntityID) const
{
	if (EntityID.IsNone()) return DefaultSkinID;
	UT66ProfileSaveGame* Profile = GetProfile();
	if (!Profile) return DefaultSkinID;

	FName Equipped;
	if (EntityType == ET66SkinEntityType::Hero)
	{
		const FName* P = Profile->EquippedHeroSkinIDByHero.Find(EntityID);
		Equipped = (P && !P->IsNone()) ? *P : DefaultSkinID;
	}
	else
	{
		const FName* P = Profile->EquippedCompanionSkinIDByCompanion.Find(EntityID);
		Equipped = (P && !P->IsNone()) ? *P : DefaultSkinID;
	}
	if (Equipped == DefaultSkinID) return Equipped;
	if (!IsSkinOwned(EntityType, EntityID, Equipped)) return DefaultSkinID;
	return Equipped;
}

void UT66SkinSubsystem::SetEquippedSkin(ET66SkinEntityType EntityType, FName EntityID, FName SkinID)
{
	if (!GetProfile() || EntityID.IsNone() || SkinID.IsNone()) return;
	if (SkinID != DefaultSkinID && !IsSkinOwned(EntityType, EntityID, SkinID)) return;

	UT66ProfileSaveGame* Profile = GetProfile();
	if (EntityType == ET66SkinEntityType::Hero)
		Profile->EquippedHeroSkinIDByHero.FindOrAdd(EntityID) = SkinID;
	else
		Profile->EquippedCompanionSkinIDByCompanion.FindOrAdd(EntityID) = SkinID;

	MarkProfileDirtyAndSave(false);
	OnSkinStateChanged.Broadcast();
}

TArray<FSkinData> UT66SkinSubsystem::GetSkinsForEntity(ET66SkinEntityType EntityType, FName EntityID) const
{
	TArray<FSkinData> Out;
	FName EquippedID = GetEquippedSkin(EntityType, EntityID);
	if (EquippedID.IsNone()) EquippedID = DefaultSkinID;

	for (FName SkinID : GetAllSkinIDs())
	{
		FSkinData Skin;
		Skin.SkinID = SkinID;
		Skin.OwnerID = EntityID;
		Skin.bIsDefault = (SkinID == DefaultSkinID);
		Skin.bIsOwned = Skin.bIsDefault || IsSkinOwned(EntityType, EntityID, SkinID);
		Skin.bIsEquipped = (SkinID == EquippedID);
		Skin.CoinCost = Skin.bIsDefault ? 0 : DefaultSkinPriceAC;
		Skin.Portrait = GetSkinPortrait(EntityType, EntityID, SkinID, false);
		Skin.SelectionPortrait = GetSkinPortrait(EntityType, EntityID, SkinID, true);
		Out.Add(Skin);
	}
	return Out;
}

TSoftObjectPtr<UTexture2D> UT66SkinSubsystem::GetSkinPortrait(
	const ET66SkinEntityType EntityType,
	const FName EntityID,
	const FName SkinID,
	const bool bSelectionPortrait) const
{
	if (EntityID.IsNone())
	{
		return TSoftObjectPtr<UTexture2D>();
	}

	const FName EffectiveSkinID = SkinID.IsNone() ? DefaultSkinID : SkinID;

	if (EntityType == ET66SkinEntityType::Companion)
	{
		if (const TSoftObjectPtr<UTexture2D> OverridePortrait = GetCompanionSkinPortraitOverride(EntityID, EffectiveSkinID, bSelectionPortrait);
			!OverridePortrait.IsNull())
		{
			return OverridePortrait;
		}

		if (UT66GameInstance* T66GameInstance = Cast<UT66GameInstance>(GetGameInstance()))
		{
			FCompanionData CompanionData;
			if (T66GameInstance->GetCompanionData(EntityID, CompanionData))
			{
				if (bSelectionPortrait)
				{
					return !CompanionData.SelectionPortrait.IsNull()
						? CompanionData.SelectionPortrait
						: CompanionData.Portrait;
				}

				return CompanionData.Portrait;
			}
		}
	}

	return TSoftObjectPtr<UTexture2D>();
}

int32 UT66SkinSubsystem::GetAchievementCoinsBalance() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
			return Ach->GetAchievementCoinsBalance();
	}
	return 0;
}

bool UT66SkinSubsystem::IsHeroSkinOwned(FName HeroID, FName SkinID) const
{
	return IsSkinOwned(ET66SkinEntityType::Hero, HeroID, SkinID);
}

bool UT66SkinSubsystem::IsCompanionSkinOwned(FName CompanionID, FName SkinID) const
{
	return IsSkinOwned(ET66SkinEntityType::Companion, CompanionID, SkinID);
}

bool UT66SkinSubsystem::PurchaseHeroSkin(FName HeroID, FName SkinID, int32 CostAC)
{
	return PurchaseSkin(ET66SkinEntityType::Hero, HeroID, SkinID, CostAC);
}

bool UT66SkinSubsystem::PurchaseCompanionSkin(FName CompanionID, FName SkinID, int32 CostAC)
{
	return PurchaseSkin(ET66SkinEntityType::Companion, CompanionID, SkinID, CostAC);
}

bool UT66SkinSubsystem::RefundHeroSkin(FName HeroID, FName SkinID, int32 RefundAC)
{
	return RefundSkin(ET66SkinEntityType::Hero, HeroID, SkinID, RefundAC);
}

bool UT66SkinSubsystem::RefundCompanionSkin(FName CompanionID, FName SkinID, int32 RefundAC)
{
	return RefundSkin(ET66SkinEntityType::Companion, CompanionID, SkinID, RefundAC);
}

FName UT66SkinSubsystem::GetEquippedHeroSkinID(FName HeroID) const
{
	return GetEquippedSkin(ET66SkinEntityType::Hero, HeroID);
}

FName UT66SkinSubsystem::GetEquippedCompanionSkinID(FName CompanionID) const
{
	return GetEquippedSkin(ET66SkinEntityType::Companion, CompanionID);
}

void UT66SkinSubsystem::SetEquippedHeroSkinID(FName HeroID, FName SkinID)
{
	SetEquippedSkin(ET66SkinEntityType::Hero, HeroID, SkinID);
}

void UT66SkinSubsystem::SetEquippedCompanionSkinID(FName CompanionID, FName SkinID)
{
	SetEquippedSkin(ET66SkinEntityType::Companion, CompanionID, SkinID);
}

void UT66SkinSubsystem::ResetAllHeroSkinOwnership()
{
	UT66ProfileSaveGame* Profile = GetProfile();
	if (!Profile) return;
	Profile->OwnedHeroSkinsByHero.Reset();
	Profile->EquippedHeroSkinIDByHero.Reset();
	Profile->OwnedHeroSkinIDs.Reset();
	Profile->EquippedHeroSkinID = DefaultSkinID;
	MarkProfileDirtyAndSave(false);
	OnSkinStateChanged.Broadcast();
}

TSoftObjectPtr<UTexture2D> UT66SkinSubsystem::GetCompanionSkinPortraitOverride(
	const FName CompanionID,
	const FName SkinID,
	const bool bSelectionPortrait) const
{
	(void)CompanionID;
	(void)SkinID;
	(void)bSelectionPortrait;

	// Future hook for per-skin companion preview art. Add explicit overrides here when
	// a skin gets its own companion portrait or selection/info portrait asset.
	return TSoftObjectPtr<UTexture2D>();
}
