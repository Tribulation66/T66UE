// Copyright Tribulation 66. All Rights Reserved.

#include "Save/T66TDSaveSubsystem.h"

#include "Core/T66TDDataSubsystem.h"
#include "Core/T66TDFrontendStateSubsystem.h"
#include "Data/T66TDDataTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Save/T66TDProfileSaveGame.h"
#include "Save/T66TDRunSaveGame.h"

namespace
{
	constexpr int32 T66TDSaveUserIndex = 0;
}

UT66TDRunSaveGame* UT66TDSaveSubsystem::LoadRun() const
{
	const FString SlotName = MakeRunSlotName();
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, T66TDSaveUserIndex))
	{
		return nullptr;
	}

	UT66TDRunSaveGame* RunSave = Cast<UT66TDRunSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, T66TDSaveUserIndex));
	CachedRunSave.Reset(RunSave);
	return RunSave;
}

bool UT66TDSaveSubsystem::SaveRun(UT66TDRunSaveGame* RunSave) const
{
	if (!RunSave)
	{
		return false;
	}

	RunSave->SaveSlotIndex = 0;
	RunSave->LastUpdatedUtc = BuildUtcNowString();
	CachedRunSave.Reset(RunSave);
	return UGameplayStatics::SaveGameToSlot(RunSave, MakeRunSlotName(), T66TDSaveUserIndex);
}

UT66TDRunSaveGame* UT66TDSaveSubsystem::CreateSeededRunSave(
	const UT66TDFrontendStateSubsystem* FrontendState,
	const UT66TDDataSubsystem* DataSubsystem) const
{
	if (!FrontendState || !DataSubsystem || !FrontendState->HasSelectedDifficulty() || !FrontendState->HasSelectedMap())
	{
		return nullptr;
	}

	const FT66TDStageDefinition* Stage = FrontendState->HasSelectedStage()
		? DataSubsystem->FindStage(FrontendState->GetSelectedStageID())
		: DataSubsystem->FindStageForMap(FrontendState->GetSelectedMapID());
	UT66TDRunSaveGame* RunSave = Cast<UT66TDRunSaveGame>(UGameplayStatics::CreateSaveGameObject(UT66TDRunSaveGame::StaticClass()));
	if (!RunSave)
	{
		return nullptr;
	}

	RunSave->DifficultyID = FrontendState->GetSelectedDifficultyID();
	RunSave->StageID = Stage ? Stage->StageID : NAME_None;
	RunSave->StageIndex = Stage ? Stage->StageIndex : 1;
	RunSave->MapID = FrontendState->GetSelectedMapID();
	RunSave->CurrentWave = 0;
	RunSave->Hearts = 0;
	RunSave->Gold = Stage ? Stage->StartingGold : 0;
	RunSave->Materials = Stage ? Stage->StartingMaterials : 0;
	RunSave->LastScore = 0;
	RunSave->bStageCleared = false;
	RunSave->bDailyRun = FrontendState->IsDailyRun();
	RunSave->DailyChallengeId = FrontendState->GetDailyChallengeId();
	RunSave->DailySeed = FrontendState->GetDailySeed();
	RunSave->LastUpdatedUtc = BuildUtcNowString();
	return RunSave;
}

UT66TDProfileSaveGame* UT66TDSaveSubsystem::LoadOrCreateProfileSave() const
{
	UT66TDProfileSaveGame* ProfileSave = CachedProfileSave.Get();
	if (!bHasResolvedProfileSave)
	{
		const FString SlotName = MakeProfileSlotName();
		if (UGameplayStatics::DoesSaveGameExist(SlotName, T66TDSaveUserIndex))
		{
			ProfileSave = Cast<UT66TDProfileSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, T66TDSaveUserIndex));
		}

		if (!ProfileSave)
		{
			ProfileSave = Cast<UT66TDProfileSaveGame>(UGameplayStatics::CreateSaveGameObject(UT66TDProfileSaveGame::StaticClass()));
		}

		CachedProfileSave.Reset(ProfileSave);
		bHasResolvedProfileSave = true;
	}

	return ProfileSave;
}

bool UT66TDSaveSubsystem::SaveProfile(UT66TDProfileSaveGame* ProfileSave) const
{
	if (!ProfileSave)
	{
		return false;
	}

	ProfileSave->CompletedStageCount = FMath::Max(0, ProfileSave->CompletedStageCount);
	ProfileSave->FailedBattleCount = FMath::Max(0, ProfileSave->FailedBattleCount);
	ProfileSave->BestScore = FMath::Max(0, ProfileSave->BestScore);
	ProfileSave->LifetimeGoldEarned = FMath::Max(0, ProfileSave->LifetimeGoldEarned);
	ProfileSave->LifetimeMaterialsEarned = FMath::Max(0, ProfileSave->LifetimeMaterialsEarned);
	for (TPair<FName, int32>& Pair : ProfileSave->BestStageIndexByDifficulty)
	{
		Pair.Value = FMath::Max(0, Pair.Value);
	}

	ProfileSave->LastUpdatedUtc = BuildUtcNowString();
	CachedProfileSave.Reset(ProfileSave);
	bHasResolvedProfileSave = true;
	return UGameplayStatics::SaveGameToSlot(ProfileSave, MakeProfileSlotName(), T66TDSaveUserIndex);
}

bool UT66TDSaveSubsystem::RecordStageResult(
	const FT66TDStageDefinition* Stage,
	const bool bVictory,
	const int32 Score,
	const int32 GoldEarned,
	const int32 MaterialsEarned) const
{
	UT66TDProfileSaveGame* ProfileSave = LoadOrCreateProfileSave();
	if (!ProfileSave)
	{
		return false;
	}

	if (bVictory)
	{
		++ProfileSave->CompletedStageCount;
		if (Stage)
		{
			int32& BestStage = ProfileSave->BestStageIndexByDifficulty.FindOrAdd(Stage->DifficultyID);
			BestStage = FMath::Max(BestStage, Stage->StageIndex);
			ProfileSave->LastStageID = Stage->StageID;
			ProfileSave->LastDifficultyID = Stage->DifficultyID;
		}
	}
	else
	{
		++ProfileSave->FailedBattleCount;
	}

	ProfileSave->BestScore = FMath::Max(ProfileSave->BestScore, Score);
	ProfileSave->LifetimeGoldEarned += FMath::Max(0, GoldEarned);
	ProfileSave->LifetimeMaterialsEarned += FMath::Max(0, MaterialsEarned);
	return SaveProfile(ProfileSave);
}

FString UT66TDSaveSubsystem::MakeRunSlotName()
{
	return TEXT("T66TD_Run");
}

FString UT66TDSaveSubsystem::MakeProfileSlotName()
{
	return TEXT("T66TD_Profile");
}

FString UT66TDSaveSubsystem::BuildUtcNowString()
{
	return FDateTime::UtcNow().ToIso8601();
}
