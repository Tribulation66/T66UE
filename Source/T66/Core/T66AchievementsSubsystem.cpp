// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66ProfileSaveGame.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"

const FString UT66AchievementsSubsystem::ProfileSaveSlotName(TEXT("T66_Profile"));

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
	}

	// Enforce safe defaults.
	Profile->SaveVersion = FMath::Max(Profile->SaveVersion, 1);
	Profile->AchievementCoinsBalance = FMath::Max(0, Profile->AchievementCoinsBalance);
	Profile->LifetimeEnemiesKilled = FMath::Max(0, Profile->LifetimeEnemiesKilled);
}

int32 UT66AchievementsSubsystem::GetAchievementCoinsBalance() const
{
	return Profile ? FMath::Max(0, Profile->AchievementCoinsBalance) : 0;
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

void UT66AchievementsSubsystem::RebuildDefinitions()
{
	CachedDefinitions.Reset();

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	// v0: first real achievement (user request)
	{
		FAchievementData A;
		A.AchievementID = FName(TEXT("ACH_BLK_001"));
		A.Tier = ET66AchievementTier::Black;
		A.RequirementCount = 20;
		A.RewardCoins = 250;

		// NOTE: Localization hooks can be added later; keep safe fallbacks now.
		A.DisplayName = FText::FromName(A.AchievementID);
		A.Description = FText::FromString(TEXT("Kill 20 enemies."));

		CachedDefinitions.Add(A);
	}

	// Apply runtime state from profile into cached list.
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
		UGameplayStatics::SaveGameToSlot(Profile, ProfileSaveSlotName, ProfileSaveUserIndex);
		bProfileDirty = false;
		return;
	}

	const float Now = static_cast<float>(GetWorld()->GetTimeSeconds());
	if (bForce || (Now - LastProfileSaveWorldSeconds) >= SaveThrottleSeconds)
	{
		UGameplayStatics::SaveGameToSlot(Profile, ProfileSaveSlotName, ProfileSaveUserIndex);
		LastProfileSaveWorldSeconds = Now;
		bProfileDirty = false;
	}
}

void UT66AchievementsSubsystem::MarkDirtyAndMaybeSave(bool bForce)
{
	bProfileDirty = true;
	SaveProfileIfNeeded(bForce);
}

void UT66AchievementsSubsystem::NotifyEnemyKilled(int32 Count)
{
	if (!Profile) LoadOrCreateProfile();
	if (!Profile) return;

	const int32 Delta = FMath::Clamp(Count, 0, 1000000);
	if (Delta <= 0) return;

	Profile->LifetimeEnemiesKilled = FMath::Clamp(Profile->LifetimeEnemiesKilled + Delta, 0, 2000000000);

	// ACH_BLK_001: Kill 20 enemies (lifetime)
	const FName AchievementID(TEXT("ACH_BLK_001"));
	FT66AchievementState* S = FindOrAddState(AchievementID);
	if (S)
	{
		const int32 Target = 20;
		const int32 PrevProgress = S->CurrentProgress;
		const bool bPrevUnlocked = S->bIsUnlocked;

		S->CurrentProgress = FMath::Clamp(S->CurrentProgress + Delta, 0, Target);
		S->bIsUnlocked = (S->CurrentProgress >= Target);

		const bool bUnlockedNow = (!bPrevUnlocked && S->bIsUnlocked);
		const bool bProgressChanged = (S->CurrentProgress != PrevProgress) || (S->bIsUnlocked != bPrevUnlocked);

		if (bProgressChanged)
		{
			// Save immediately when the achievement unlocks; otherwise throttle.
			MarkDirtyAndMaybeSave(bUnlockedNow);
			AchievementsStateChanged.Broadcast();
		}
	}
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

	MarkDirtyAndMaybeSave(true);
	AchievementCoinsChanged.Broadcast();
	AchievementsStateChanged.Broadcast();

	RebuildDefinitions();
}

