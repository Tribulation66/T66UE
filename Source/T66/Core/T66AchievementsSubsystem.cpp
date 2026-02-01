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
	Profile->SaveVersion = FMath::Max(Profile->SaveVersion, 3);
	Profile->AchievementCoinsBalance = FMath::Max(0, Profile->AchievementCoinsBalance);
	Profile->LifetimeEnemiesKilled = FMath::Max(0, Profile->LifetimeEnemiesKilled);

	// Union defaults: clamp to sane ranges so corrupted saves can't break gameplay math.
	for (TPair<FName, int32>& Pair : Profile->CompanionUnionStagesClearedByID)
	{
		Pair.Value = FMath::Clamp(Pair.Value, 0, 2000000000);
	}
}

int32 UT66AchievementsSubsystem::GetAchievementCoinsBalance() const
{
	return Profile ? FMath::Max(0, Profile->AchievementCoinsBalance) : 0;
}

bool UT66AchievementsSubsystem::HasCompletedTutorial() const
{
	return Profile ? Profile->bHasCompletedTutorial : false;
}

void UT66AchievementsSubsystem::MarkTutorialCompleted()
{
	if (!Profile) return;
	if (Profile->bHasCompletedTutorial) return;
	Profile->bHasCompletedTutorial = true;
	bProfileDirty = true;
	SaveProfileIfNeeded(true);
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

		// Culture-based localization (gather/translate/compile .locres).
		// NOTE: Loc can be null extremely early; fall back to stable NSLOCTEXT keys.
		A.DisplayName = Loc
			? Loc->GetText_AchievementName(A.AchievementID)
			: NSLOCTEXT("T66.Achievements", "ACH_BLK_001_Name", "KILL 20 ENEMIES");
		A.Description = Loc
			? Loc->GetText_AchievementDescription(A.AchievementID)
			: NSLOCTEXT("T66.Achievements", "ACH_BLK_001_Desc", "Kill 20 enemies.");

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

	// Save immediately when crossing a tier boundary; otherwise throttle.
	const bool bTierCrossed =
		(Prev < UnionTier_GoodStages && Next >= UnionTier_GoodStages) ||
		(Prev < UnionTier_MediumStages && Next >= UnionTier_MediumStages) ||
		(Prev < UnionTier_HyperStages && Next >= UnionTier_HyperStages);
	MarkDirtyAndMaybeSave(bTierCrossed);
}

