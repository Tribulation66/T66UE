// Copyright Tribulation 66. All Rights Reserved.

#include "Save/T66MiniSaveSubsystem.h"

#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Save/T66MiniProfileSaveGame.h"
#include "Save/T66MiniRunSaveGame.h"

namespace
{
	constexpr int32 T66MiniSaveUserIndex = 0;

	float T66MiniResolveCompanionHealingMultiplier(const int32 UnityStagesCleared)
	{
		if (UnityStagesCleared >= UT66MiniSaveSubsystem::CompanionUnityTierHyperStages)
		{
			return 4.0f;
		}

		if (UnityStagesCleared >= UT66MiniSaveSubsystem::CompanionUnityTierMediumStages)
		{
			return 3.0f;
		}

		if (UnityStagesCleared >= UT66MiniSaveSubsystem::CompanionUnityTierGoodStages)
		{
			return 2.0f;
		}

		return 1.0f;
	}

	void T66MiniSeedDefaultProfileUnlocks(UT66MiniProfileSaveGame* ProfileSave, const UT66MiniDataSubsystem* DataSubsystem, bool& bOutChanged)
	{
		if (!ProfileSave || !DataSubsystem)
		{
			return;
		}

		for (const FT66MiniHeroDefinition& Hero : DataSubsystem->GetHeroes())
		{
			if (Hero.bUnlockedByDefault && !ProfileSave->UnlockedHeroIDs.Contains(Hero.HeroID))
			{
				ProfileSave->UnlockedHeroIDs.Add(Hero.HeroID);
				bOutChanged = true;
			}
		}

		for (const FT66MiniDifficultyDefinition& Difficulty : DataSubsystem->GetDifficulties())
		{
			if (!ProfileSave->UnlockedDifficultyIDs.Contains(Difficulty.DifficultyID))
			{
				ProfileSave->UnlockedDifficultyIDs.Add(Difficulty.DifficultyID);
				bOutChanged = true;
			}
		}

		for (const FT66MiniCompanionDefinition& Companion : DataSubsystem->GetCompanions())
		{
			if (Companion.bUnlockedByDefault && !ProfileSave->UnlockedCompanionIDs.Contains(Companion.CompanionID))
			{
				ProfileSave->UnlockedCompanionIDs.Add(Companion.CompanionID);
				bOutChanged = true;
			}
		}
	}
}

TArray<FT66MiniSaveSlotSummary> UT66MiniSaveSubsystem::BuildRunSlotSummaries(const UT66MiniDataSubsystem* DataSubsystem) const
{
	TArray<FT66MiniSaveSlotSummary> Summaries;
	Summaries.Reserve(MaxSlots);

	for (int32 SlotIndex = 0; SlotIndex < MaxSlots; ++SlotIndex)
	{
		FT66MiniSaveSlotSummary Summary;
		Summary.SlotIndex = SlotIndex;

		if (UT66MiniRunSaveGame* RunSave = LoadRunFromSlot(SlotIndex))
		{
			Summary.bOccupied = true;
			Summary.HeroID = RunSave->HeroID;
			Summary.DifficultyID = RunSave->DifficultyID;
			Summary.WaveIndex = FMath::Max(1, RunSave->WaveIndex);
			Summary.bHasMidWaveSnapshot = RunSave->bHasMidWaveSnapshot;
			Summary.LastUpdatedUtc = RunSave->LastUpdatedUtc;

			if (DataSubsystem)
			{
				if (const FT66MiniHeroDefinition* Hero = DataSubsystem->FindHero(RunSave->HeroID))
				{
					Summary.HeroDisplayName = Hero->DisplayName;
				}
				if (const FT66MiniDifficultyDefinition* Difficulty = DataSubsystem->FindDifficulty(RunSave->DifficultyID))
				{
					Summary.DifficultyDisplayName = Difficulty->DisplayName;
				}
			}
		}

		Summaries.Add(MoveTemp(Summary));
	}

	return Summaries;
}

UT66MiniRunSaveGame* UT66MiniSaveSubsystem::LoadRunFromSlot(const int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots)
	{
		return nullptr;
	}

	return Cast<UT66MiniRunSaveGame>(UGameplayStatics::LoadGameFromSlot(MakeRunSlotName(SlotIndex), T66MiniSaveUserIndex));
}

bool UT66MiniSaveSubsystem::SaveRunToSlot(const int32 SlotIndex, UT66MiniRunSaveGame* RunSave) const
{
	if (!RunSave || SlotIndex < 0 || SlotIndex >= MaxSlots)
	{
		return false;
	}

	RunSave->SaveSlotIndex = SlotIndex;
	RunSave->LastUpdatedUtc = BuildUtcNowString();
	return UGameplayStatics::SaveGameToSlot(RunSave, MakeRunSlotName(SlotIndex), T66MiniSaveUserIndex);
}

bool UT66MiniSaveSubsystem::DeleteRunFromSlot(const int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots)
	{
		return false;
	}

	return UGameplayStatics::DeleteGameInSlot(MakeRunSlotName(SlotIndex), T66MiniSaveUserIndex);
}

UT66MiniRunSaveGame* UT66MiniSaveSubsystem::CreateSeededRunSave(const UT66MiniFrontendStateSubsystem* FrontendState) const
{
	if (!FrontendState || !FrontendState->HasSelectedHero() || !FrontendState->HasSelectedCompanion() || !FrontendState->HasSelectedDifficulty())
	{
		return nullptr;
	}

	UT66MiniRunSaveGame* RunSave = Cast<UT66MiniRunSaveGame>(UGameplayStatics::CreateSaveGameObject(UT66MiniRunSaveGame::StaticClass()));
	if (!RunSave)
	{
		return nullptr;
	}

	RunSave->HeroID = FrontendState->GetSelectedHeroID();
	RunSave->CompanionID = FrontendState->GetSelectedCompanionID();
	RunSave->DifficultyID = FrontendState->GetSelectedDifficultyID();
	RunSave->EquippedIdolIDs = FrontendState->GetSelectedIdolIDs();
	RunSave->OwnedItemIDs.Reset();
	RunSave->CurrentShopOfferIDs.Reset();
	RunSave->LockedShopOfferIDs.Reset();
	RunSave->WaveIndex = 1;
	RunSave->HeroLevel = 1;
	RunSave->CurrentHealth = 100.f;
	RunSave->MaxHealth = 100.f;
	RunSave->Materials = 0;
	RunSave->Gold = 0;
	RunSave->Experience = 0.f;
	RunSave->TotalRunSeconds = 0.f;
	RunSave->ShopRerollCount = 0;
	RunSave->bHasMidWaveSnapshot = false;
	RunSave->bPendingShopIntermission = false;
	RunSave->bHasPlayerLocation = false;
	RunSave->PlayerLocation = FVector::ZeroVector;
	RunSave->WaveSecondsRemaining = 180.f;
	RunSave->bBossSpawnedForWave = false;
	RunSave->BossTelegraphRemaining = 0.f;
	RunSave->PendingBossID = NAME_None;
	RunSave->PendingBossSpawnLocation = FVector::ZeroVector;
	RunSave->EnemySpawnAccumulator = 0.f;
	RunSave->InteractableSpawnAccumulator = 0.f;
	RunSave->PostBossDelayRemaining = 0.f;
	RunSave->EnemySnapshots.Reset();
	RunSave->PickupSnapshots.Reset();
	RunSave->InteractableSnapshots.Reset();
	RunSave->LastUpdatedUtc = BuildUtcNowString();
	return RunSave;
}

UT66MiniProfileSaveGame* UT66MiniSaveSubsystem::LoadOrCreateProfileSave(const UT66MiniDataSubsystem* DataSubsystem) const
{
	if (UT66MiniProfileSaveGame* ExistingProfile = Cast<UT66MiniProfileSaveGame>(UGameplayStatics::LoadGameFromSlot(MakeProfileSlotName(), T66MiniSaveUserIndex)))
	{
		bool bChanged = false;
		T66MiniSeedDefaultProfileUnlocks(ExistingProfile, DataSubsystem, bChanged);

		ExistingProfile->CompletedRunCount = FMath::Max(0, ExistingProfile->CompletedRunCount);
		ExistingProfile->FailedRunCount = FMath::Max(0, ExistingProfile->FailedRunCount);
		ExistingProfile->BestWaveReached = FMath::Max(1, ExistingProfile->BestWaveReached);
		ExistingProfile->LifetimeMaterialsCollected = FMath::Max(0, ExistingProfile->LifetimeMaterialsCollected);
		ExistingProfile->TotalStagesCleared = FMath::Max(0, ExistingProfile->TotalStagesCleared);
		for (TPair<FName, int32>& Pair : ExistingProfile->CompanionUnityStagesClearedByID)
		{
			const int32 ClampedValue = FMath::Max(0, Pair.Value);
			if (Pair.Value != ClampedValue)
			{
				Pair.Value = ClampedValue;
				bChanged = true;
			}
		}

		if (bChanged)
		{
			SaveProfile(ExistingProfile);
		}

		return ExistingProfile;
	}

	UT66MiniProfileSaveGame* ProfileSave = Cast<UT66MiniProfileSaveGame>(UGameplayStatics::CreateSaveGameObject(UT66MiniProfileSaveGame::StaticClass()));
	if (!ProfileSave)
	{
		return nullptr;
	}

	if (DataSubsystem)
	{
		bool bUnusedChanged = false;
		T66MiniSeedDefaultProfileUnlocks(ProfileSave, DataSubsystem, bUnusedChanged);
	}

	ProfileSave->BestWaveReached = 1;
	ProfileSave->TotalStagesCleared = 0;
	ProfileSave->LastUpdatedUtc = BuildUtcNowString();
	return ProfileSave;
}

bool UT66MiniSaveSubsystem::SaveProfile(UT66MiniProfileSaveGame* ProfileSave) const
{
	if (!ProfileSave)
	{
		return false;
	}

	ProfileSave->LastUpdatedUtc = BuildUtcNowString();
	return UGameplayStatics::SaveGameToSlot(ProfileSave, MakeProfileSlotName(), T66MiniSaveUserIndex);
}

bool UT66MiniSaveSubsystem::RecordRunSummary(const FT66MiniRunSummary& Summary, const UT66MiniDataSubsystem* DataSubsystem) const
{
	UT66MiniProfileSaveGame* ProfileSave = LoadOrCreateProfileSave(DataSubsystem);
	if (!ProfileSave)
	{
		return false;
	}

	ProfileSave->LastRunSummary = Summary;
	ProfileSave->LifetimeMaterialsCollected += FMath::Max(0, Summary.MaterialsCollected);
	ProfileSave->BestWaveReached = FMath::Max(ProfileSave->BestWaveReached, Summary.WaveReached);
	ProfileSave->FavoriteHeroID = Summary.HeroID;

	if (Summary.bWasVictory)
	{
		++ProfileSave->CompletedRunCount;
		if (Summary.RunSeconds > 0.f && (ProfileSave->BestClearSeconds <= 0.f || Summary.RunSeconds < ProfileSave->BestClearSeconds))
		{
			ProfileSave->BestClearSeconds = Summary.RunSeconds;
		}
	}
	else
	{
		++ProfileSave->FailedRunCount;
	}

	return SaveProfile(ProfileSave);
}

bool UT66MiniSaveSubsystem::IsCompanionUnlocked(const FName CompanionID, const UT66MiniDataSubsystem* DataSubsystem) const
{
	if (CompanionID.IsNone())
	{
		return true;
	}

	const UT66MiniProfileSaveGame* ProfileSave = LoadOrCreateProfileSave(DataSubsystem);
	if (!ProfileSave)
	{
		return false;
	}

	if (ProfileSave->UnlockedCompanionIDs.Contains(CompanionID))
	{
		return true;
	}

	if (const FT66MiniCompanionDefinition* CompanionDefinition = DataSubsystem ? DataSubsystem->FindCompanion(CompanionID) : nullptr)
	{
		return CompanionDefinition->bUnlockedByDefault;
	}

	return false;
}

FName UT66MiniSaveSubsystem::GetFirstUnlockedCompanionID(const UT66MiniDataSubsystem* DataSubsystem) const
{
	if (!DataSubsystem)
	{
		return NAME_None;
	}

	for (const FT66MiniCompanionDefinition& Companion : DataSubsystem->GetCompanions())
	{
		if (IsCompanionUnlocked(Companion.CompanionID, DataSubsystem))
		{
			return Companion.CompanionID;
		}
	}

	return NAME_None;
}

int32 UT66MiniSaveSubsystem::GetUnlockedCompanionCount(const UT66MiniDataSubsystem* DataSubsystem) const
{
	if (!DataSubsystem)
	{
		return 0;
	}

	int32 UnlockedCount = 0;
	for (const FT66MiniCompanionDefinition& Companion : DataSubsystem->GetCompanions())
	{
		if (IsCompanionUnlocked(Companion.CompanionID, DataSubsystem))
		{
			++UnlockedCount;
		}
	}

	return UnlockedCount;
}

int32 UT66MiniSaveSubsystem::GetTotalStagesCleared(const UT66MiniDataSubsystem* DataSubsystem) const
{
	const UT66MiniProfileSaveGame* ProfileSave = LoadOrCreateProfileSave(DataSubsystem);
	return ProfileSave ? FMath::Max(0, ProfileSave->TotalStagesCleared) : 0;
}

int32 UT66MiniSaveSubsystem::GetCompanionUnityStagesCleared(const FName CompanionID, const UT66MiniDataSubsystem* DataSubsystem) const
{
	if (CompanionID.IsNone())
	{
		return 0;
	}

	const UT66MiniProfileSaveGame* ProfileSave = LoadOrCreateProfileSave(DataSubsystem);
	if (!ProfileSave)
	{
		return 0;
	}

	if (const int32* Found = ProfileSave->CompanionUnityStagesClearedByID.Find(CompanionID))
	{
		return FMath::Max(0, *Found);
	}

	return 0;
}

float UT66MiniSaveSubsystem::GetCompanionUnityProgress01(const FName CompanionID, const UT66MiniDataSubsystem* DataSubsystem) const
{
	return FMath::Clamp(
		static_cast<float>(GetCompanionUnityStagesCleared(CompanionID, DataSubsystem)) / static_cast<float>(CompanionUnityTierHyperStages),
		0.0f,
		1.0f);
}

float UT66MiniSaveSubsystem::GetCompanionHealingPerSecond(const FName CompanionID, const UT66MiniDataSubsystem* DataSubsystem) const
{
	if (CompanionID.IsNone() || !DataSubsystem)
	{
		return 0.0f;
	}

	const FT66MiniCompanionDefinition* CompanionDefinition = DataSubsystem->FindCompanion(CompanionID);
	if (!CompanionDefinition)
	{
		return 0.0f;
	}

	return CompanionDefinition->BaseHealingPerSecond
		* T66MiniResolveCompanionHealingMultiplier(GetCompanionUnityStagesCleared(CompanionID, DataSubsystem));
}

bool UT66MiniSaveSubsystem::RecordClearedMiniStage(
	const FName CompanionID,
	const UT66MiniDataSubsystem* DataSubsystem,
	TArray<FName>* OutNewlyUnlocked) const
{
	UT66MiniProfileSaveGame* ProfileSave = LoadOrCreateProfileSave(DataSubsystem);
	if (!ProfileSave || !DataSubsystem)
	{
		return false;
	}

	++ProfileSave->TotalStagesCleared;
	if (!CompanionID.IsNone())
	{
		int32& UnityStages = ProfileSave->CompanionUnityStagesClearedByID.FindOrAdd(CompanionID);
		UnityStages = FMath::Max(0, UnityStages) + 1;
	}

	bool bChanged = true;
	for (const FT66MiniCompanionDefinition& Companion : DataSubsystem->GetCompanions())
	{
		const bool bShouldBeUnlocked = Companion.bUnlockedByDefault
			|| ProfileSave->TotalStagesCleared >= FMath::Max(0, Companion.UnlockStageRequirement);
		if (!bShouldBeUnlocked || ProfileSave->UnlockedCompanionIDs.Contains(Companion.CompanionID))
		{
			continue;
		}

		ProfileSave->UnlockedCompanionIDs.Add(Companion.CompanionID);
		if (OutNewlyUnlocked)
		{
			OutNewlyUnlocked->Add(Companion.CompanionID);
		}
	}

	return bChanged && SaveProfile(ProfileSave);
}

FString UT66MiniSaveSubsystem::MakeRunSlotName(const int32 SlotIndex)
{
	return FString::Printf(TEXT("T66Mini_RunSlot_%02d"), SlotIndex);
}

FString UT66MiniSaveSubsystem::MakeProfileSlotName()
{
	return TEXT("T66Mini_Profile");
}

FString UT66MiniSaveSubsystem::BuildUtcNowString()
{
	return FDateTime::UtcNow().ToIso8601();
}
