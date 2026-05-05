// Copyright Tribulation 66. All Rights Reserved.

#include "Save/T66IdleSaveSubsystem.h"

#include "Core/T66IdleDataSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"

namespace
{
	struct FT66IdleResolvedEnemy
	{
		FName StageID = NAME_None;
		FName EnemyID = NAME_None;
		bool bStageBoss = false;
		double MaxHealth = 1.0;
		double RewardGold = 1.0;
	};

	double T66IdleGoldRewardMultiplier(const UT66IdleProfileSaveGame* ProfileSave, const UT66IdleDataSubsystem* DataSubsystem)
	{
		if (!ProfileSave || !DataSubsystem)
		{
			return 1.0;
		}

		double Multiplier = 1.0;
		for (const FName ItemID : ProfileSave->OwnedItemIDs)
		{
			if (const FT66IdleItemDefinition* Item = DataSubsystem->FindItem(ItemID))
			{
				Multiplier *= FMath::Max(0.01, Item->GoldRewardMultiplier);
			}
		}

		for (const FName IdolID : ProfileSave->OwnedIdolIDs)
		{
			if (const FT66IdleIdolDefinition* Idol = DataSubsystem->FindIdol(IdolID))
			{
				Multiplier *= FMath::Max(0.01, Idol->GoldRewardMultiplier);
			}
		}

		return Multiplier;
	}

	FT66IdleResolvedEnemy T66IdleResolveEnemyForStage(
		const UT66IdleDataSubsystem* DataSubsystem,
		const int32 Stage,
		const FName PreferredEnemyID,
		const double RewardMultiplier)
	{
		FT66IdleResolvedEnemy Result;
		const FT66IdleStageDefinition* StageDefinition = DataSubsystem ? DataSubsystem->FindStageByIndex(Stage) : nullptr;
		const FT66IdleZoneDefinition* Zone = DataSubsystem ? DataSubsystem->FindZoneForStage(Stage) : nullptr;
		const FT66IdleEnemyDefinition* Enemy = DataSubsystem && PreferredEnemyID != NAME_None ? DataSubsystem->FindEnemy(PreferredEnemyID) : nullptr;
		if (!Enemy && DataSubsystem && StageDefinition)
		{
			const FName StageEnemyID = !StageDefinition->BossEnemyID.IsNone() ? StageDefinition->BossEnemyID : StageDefinition->EnemyID;
			Enemy = StageEnemyID != NAME_None ? DataSubsystem->FindEnemy(StageEnemyID) : nullptr;
		}
		if (!Enemy && DataSubsystem && Zone && Zone->EnemyIDs.Num() > 0)
		{
			const int32 EnemyIndex = FMath::Abs(Stage - Zone->FirstStage) % Zone->EnemyIDs.Num();
			Enemy = DataSubsystem->FindEnemy(Zone->EnemyIDs[EnemyIndex]);
		}
		if (!Enemy && DataSubsystem && DataSubsystem->GetEnemies().Num() > 0)
		{
			Enemy = &DataSubsystem->GetEnemies()[0];
		}

		const double BaseHealth = Enemy ? Enemy->BaseHealth : (Zone ? Zone->BaseEnemyHealth : 1.0);
		const double HealthPerStage = Enemy ? Enemy->HealthPerStage : 0.0;
		const double HealthScalar = Zone ? Zone->EnemyHealthScalar : 1.0;
		const double BaseReward = Enemy ? Enemy->BaseGoldReward : (Zone ? Zone->BaseGoldReward : 1.0);
		const double GoldPerStage = Enemy ? Enemy->GoldPerStage : 0.0;
		const double RewardScalar = Zone ? Zone->GoldRewardScalar : 1.0;
		const double StageReward = StageDefinition ? FMath::Max(0.0, StageDefinition->ClearGoldReward) : 0.0;

		Result.StageID = StageDefinition ? StageDefinition->StageID : NAME_None;
		Result.EnemyID = Enemy ? Enemy->EnemyID : NAME_None;
		Result.bStageBoss = StageDefinition && StageDefinition->bBossStage;
		Result.MaxHealth = FMath::Max(1.0, (BaseHealth + static_cast<double>(Stage - 1) * HealthPerStage) * HealthScalar);
		Result.RewardGold = FMath::Max(1.0, (BaseReward + static_cast<double>(Stage) * GoldPerStage) * RewardScalar * RewardMultiplier) + StageReward;
		return Result;
	}

	void T66IdleApplyEngineProgress(FT66IdleProfileSnapshot& Snapshot, const FT66IdleTuningDefinition& Tuning, const double PassiveDamagePerSecond, const double Seconds)
	{
		if (PassiveDamagePerSecond <= 0.0 || Seconds <= 0.0)
		{
			return;
		}

		Snapshot.EngineProgress += PassiveDamagePerSecond * Tuning.EngineProgressPerPassiveDamageSecond * Seconds;
		if (Snapshot.EngineProgress < 1.0)
		{
			return;
		}

		const double CompletedCycles = FMath::FloorToDouble(Snapshot.EngineProgress);
		const double EngineReward = FMath::Max(1.0, PassiveDamagePerSecond * Tuning.EngineGoldPayoutMultiplier);
		const double TotalReward = CompletedCycles * EngineReward;
		Snapshot.UncollectedProgress += TotalReward;
		Snapshot.LifetimeGold += TotalReward;
		Snapshot.EngineProgress = FMath::Fmod(Snapshot.EngineProgress, 1.0);
	}
}

void UT66IdleSaveSubsystem::Deinitialize()
{
	CachedProfileSave.Reset();
	bHasResolvedProfileSave = false;
	Super::Deinitialize();
}

UT66IdleProfileSaveGame* UT66IdleSaveSubsystem::LoadOrCreateProfileSave() const
{
	if (bHasResolvedProfileSave)
	{
		return CachedProfileSave.Get();
	}

	bHasResolvedProfileSave = true;

	if (USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(MakeProfileSlotName(), 0))
	{
		if (UT66IdleProfileSaveGame* IdleProfile = Cast<UT66IdleProfileSaveGame>(LoadedSave))
		{
			CachedProfileSave.Reset(IdleProfile);
			return CachedProfileSave.Get();
		}
	}

	UT66IdleProfileSaveGame* NewProfile = Cast<UT66IdleProfileSaveGame>(UGameplayStatics::CreateSaveGameObject(UT66IdleProfileSaveGame::StaticClass()));
	if (NewProfile)
	{
		NewProfile->Snapshot.LastSavedUtc = BuildUtcNowString();
		CachedProfileSave.Reset(NewProfile);
	}

	return CachedProfileSave.Get();
}

bool UT66IdleSaveSubsystem::SaveProfile(UT66IdleProfileSaveGame* ProfileSave) const
{
	if (!ProfileSave)
	{
		return false;
	}

	ProfileSave->Snapshot.LastSavedUtc = BuildUtcNowString();
	CachedProfileSave.Reset(ProfileSave);
	bHasResolvedProfileSave = true;
	return UGameplayStatics::SaveGameToSlot(ProfileSave, MakeProfileSlotName(), 0);
}

bool UT66IdleSaveSubsystem::ApplyOfflineProgress(UT66IdleProfileSaveGame* ProfileSave, const UT66IdleDataSubsystem* DataSubsystem, const FTimespan MaxOfflineTime) const
{
	if (!ProfileSave || !DataSubsystem || ProfileSave->Snapshot.LastSavedUtc.IsEmpty())
	{
		return false;
	}

	FDateTime LastSavedUtc;
	if (!FDateTime::ParseIso8601(*ProfileSave->Snapshot.LastSavedUtc, LastSavedUtc))
	{
		return false;
	}

	const FDateTime NowUtc = FDateTime::UtcNow();
	const FTimespan Elapsed = NowUtc - LastSavedUtc;
	if (Elapsed <= FTimespan::Zero())
	{
		return false;
	}

	double RemainingSeconds = (Elapsed < MaxOfflineTime ? Elapsed : MaxOfflineTime).GetTotalSeconds();
	const double PassiveDamagePerSecond = FMath::Max(0.0, ProfileSave->Snapshot.PassiveDamagePerSecond);
	if (RemainingSeconds <= 0.0 || PassiveDamagePerSecond <= 0.0)
	{
		return false;
	}

	FT66IdleProfileSnapshot& Snapshot = ProfileSave->Snapshot;
	const FT66IdleTuningDefinition& Tuning = DataSubsystem->GetTuning();
	const double RewardMultiplier = T66IdleGoldRewardMultiplier(ProfileSave, DataSubsystem);
	int32 Stage = FMath::Max(Tuning.StartingStage, Snapshot.CurrentStage > 0 ? Snapshot.CurrentStage : Snapshot.HighestStageReached);
	FT66IdleResolvedEnemy Enemy = T66IdleResolveEnemyForStage(DataSubsystem, Stage, Snapshot.CurrentEnemyID, RewardMultiplier);
	double CurrentEnemyHealth = Snapshot.EnemyHealth > 0.0 && Snapshot.CurrentEnemyID == Enemy.EnemyID
		? FMath::Min(Snapshot.EnemyHealth, Enemy.MaxHealth)
		: Enemy.MaxHealth;

	bool bAdvanced = false;
	int32 SafetyKills = 0;
	while (RemainingSeconds > 0.0 && SafetyKills < 10000)
	{
		const double TimeToKill = CurrentEnemyHealth / PassiveDamagePerSecond;
		if (TimeToKill > RemainingSeconds)
		{
			CurrentEnemyHealth -= PassiveDamagePerSecond * RemainingSeconds;
			T66IdleApplyEngineProgress(Snapshot, Tuning, PassiveDamagePerSecond, RemainingSeconds);
			RemainingSeconds = 0.0;
			break;
		}

		T66IdleApplyEngineProgress(Snapshot, Tuning, PassiveDamagePerSecond, TimeToKill);
		RemainingSeconds -= TimeToKill;
		Snapshot.UncollectedProgress += Enemy.RewardGold;
		Snapshot.LifetimeGold += Enemy.RewardGold;
		if (Enemy.bStageBoss)
		{
			++Snapshot.BossStagesCleared;
		}
		++Stage;
		++SafetyKills;
		bAdvanced = true;

		Enemy = T66IdleResolveEnemyForStage(DataSubsystem, Stage, NAME_None, RewardMultiplier);
		CurrentEnemyHealth = Enemy.MaxHealth;
	}

	Snapshot.CurrentStage = Stage;
	Snapshot.HighestStageReached = FMath::Max(Snapshot.HighestStageReached, Stage);
	Snapshot.CurrentStageID = Enemy.StageID;
	Snapshot.CurrentEnemyID = Enemy.EnemyID;
	Snapshot.bCurrentEnemyIsStageBoss = Enemy.bStageBoss;
	Snapshot.EnemyMaxHealth = Enemy.MaxHealth;
	Snapshot.EnemyHealth = FMath::Clamp(CurrentEnemyHealth, 0.0, Enemy.MaxHealth);
	return bAdvanced || Snapshot.EngineProgress > 0.0;
}

FString UT66IdleSaveSubsystem::MakeProfileSlotName()
{
	return TEXT("T66Idle_Profile");
}

FString UT66IdleSaveSubsystem::BuildUtcNowString()
{
	return FDateTime::UtcNow().ToIso8601();
}
