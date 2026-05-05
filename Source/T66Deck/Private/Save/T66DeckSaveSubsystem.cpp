// Copyright Tribulation 66. All Rights Reserved.

#include "Save/T66DeckSaveSubsystem.h"

#include "Core/T66DeckFrontendStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Save/T66DeckRunSaveGame.h"

namespace
{
	constexpr int32 T66DeckSaveUserIndex = 0;
}

TArray<FT66DeckSaveSlotSummary> UT66DeckSaveSubsystem::BuildRunSlotSummaries() const
{
	TArray<FT66DeckSaveSlotSummary> Summaries;
	Summaries.Reserve(MaxSlots);

	for (int32 SlotIndex = 0; SlotIndex < MaxSlots; ++SlotIndex)
	{
		FT66DeckSaveSlotSummary Summary;
		Summary.SlotIndex = SlotIndex;

		if (const UT66DeckRunSaveGame* RunSave = LoadRunFromSlot(SlotIndex))
		{
			Summary.bOccupied = true;
			Summary.HeroID = RunSave->HeroID;
			Summary.DifficultyID = RunSave->DifficultyID;
			Summary.ActIndex = FMath::Max(1, RunSave->ActIndex);
			Summary.FloorIndex = FMath::Max(0, RunSave->FloorIndex);
			Summary.LastUpdatedUtc = RunSave->LastUpdatedUtc;
		}

		Summaries.Add(MoveTemp(Summary));
	}

	return Summaries;
}

UT66DeckRunSaveGame* UT66DeckSaveSubsystem::LoadRunFromSlot(const int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots)
	{
		return nullptr;
	}

	const FString SlotName = MakeRunSlotName(SlotIndex);
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, T66DeckSaveUserIndex))
	{
		return nullptr;
	}

	return Cast<UT66DeckRunSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, T66DeckSaveUserIndex));
}

bool UT66DeckSaveSubsystem::SaveRunToSlot(const int32 SlotIndex, UT66DeckRunSaveGame* RunSave) const
{
	if (!RunSave || SlotIndex < 0 || SlotIndex >= MaxSlots)
	{
		return false;
	}

	RunSave->SaveSlotIndex = SlotIndex;
	RunSave->LastUpdatedUtc = BuildUtcNowString();
	return UGameplayStatics::SaveGameToSlot(RunSave, MakeRunSlotName(SlotIndex), T66DeckSaveUserIndex);
}

bool UT66DeckSaveSubsystem::DeleteRunFromSlot(const int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots)
	{
		return false;
	}

	return UGameplayStatics::DeleteGameInSlot(MakeRunSlotName(SlotIndex), T66DeckSaveUserIndex);
}

UT66DeckRunSaveGame* UT66DeckSaveSubsystem::CreateSeededRunSave(const UT66DeckFrontendStateSubsystem* FrontendState) const
{
	if (!FrontendState || !FrontendState->HasSelectedHero() || !FrontendState->HasSelectedDifficulty())
	{
		return nullptr;
	}

	UT66DeckRunSaveGame* RunSave = Cast<UT66DeckRunSaveGame>(UGameplayStatics::CreateSaveGameObject(UT66DeckRunSaveGame::StaticClass()));
	if (!RunSave)
	{
		return nullptr;
	}

	RunSave->HeroID = FrontendState->GetSelectedHeroID();
	RunSave->DifficultyID = FrontendState->GetSelectedDifficultyID();
	RunSave->StartingDeckID = FrontendState->GetSelectedStartingDeckID();
	RunSave->CurrentNodeID = FrontendState->GetCurrentMapNodeID();
	RunSave->ActIndex = 1;
	RunSave->FloorIndex = 0;
	RunSave->LastUpdatedUtc = BuildUtcNowString();

	return RunSave;
}

UT66DeckRunSaveGame* UT66DeckSaveSubsystem::CreateBlankRunSave() const
{
	UT66DeckRunSaveGame* RunSave = Cast<UT66DeckRunSaveGame>(UGameplayStatics::CreateSaveGameObject(UT66DeckRunSaveGame::StaticClass()));
	if (RunSave)
	{
		RunSave->LastUpdatedUtc = BuildUtcNowString();
	}
	return RunSave;
}

FString UT66DeckSaveSubsystem::MakeRunSlotName(const int32 SlotIndex)
{
	return FString::Printf(TEXT("T66Deck_RunSlot_%d"), SlotIndex);
}

FString UT66DeckSaveSubsystem::BuildUtcNowString()
{
	return FDateTime::UtcNow().ToIso8601();
}
