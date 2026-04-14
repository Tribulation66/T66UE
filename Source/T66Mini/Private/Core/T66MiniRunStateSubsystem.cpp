// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MiniRunStateSubsystem.h"

#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Data/T66MiniDataTypes.h"
#include "Save/T66MiniRunSaveGame.h"
#include "Save/T66MiniSaveSubsystem.h"

bool UT66MiniRunStateSubsystem::BootstrapRunFromFrontend(UT66MiniFrontendStateSubsystem* FrontendState, UT66MiniSaveSubsystem* SaveSubsystem)
{
	if (!FrontendState || !SaveSubsystem)
	{
		LastBootstrapStatus = TEXT("Mini bootstrap failed: missing frontend or save subsystem.");
		return false;
	}

	const int32 TargetSlot = ResolveTargetSaveSlot(FrontendState, SaveSubsystem);
	if (TargetSlot == INDEX_NONE)
	{
		LastBootstrapStatus = TEXT("Mini bootstrap failed: all mini save slots are occupied. Free a slot or load an existing run.");
		return false;
	}

	UT66MiniRunSaveGame* RunSave = SaveSubsystem->CreateSeededRunSave(FrontendState);
	if (!RunSave)
	{
		LastBootstrapStatus = TEXT("Mini bootstrap failed: could not build a run save from frontend state.");
		return false;
	}

	if (!SaveSubsystem->SaveRunToSlot(TargetSlot, RunSave))
	{
		LastBootstrapStatus = TEXT("Mini bootstrap failed: could not persist the seeded run save.");
		return false;
	}

	ActiveRun = RunSave;
	ActiveSaveSlot = TargetSlot;
	FrontendState->SetPendingSaveSlot(TargetSlot);
	LastBootstrapStatus = FString::Printf(TEXT("Mini run bootstrapped in save slot %d."), TargetSlot + 1);
	ActiveRunChanged.Broadcast();
	return true;
}

bool UT66MiniRunStateSubsystem::LoadRunFromSave(UT66MiniRunSaveGame* InRunSave)
{
	if (!InRunSave)
	{
		LastBootstrapStatus = TEXT("Mini load failed: no save object was provided.");
		return false;
	}

	ActiveRun = InRunSave;
	ActiveSaveSlot = InRunSave->SaveSlotIndex;
	LastBootstrapStatus = FString::Printf(TEXT("Mini run loaded from save slot %d."), ActiveSaveSlot + 1);
	ActiveRunChanged.Broadcast();
	return true;
}

void UT66MiniRunStateSubsystem::ResetActiveRun()
{
	ActiveRun = nullptr;
	ActiveSaveSlot = INDEX_NONE;
	LastBootstrapStatus = TEXT("Mini run state cleared.");
	ActiveRunChanged.Broadcast();
}

int32 UT66MiniRunStateSubsystem::ResolveTargetSaveSlot(const UT66MiniFrontendStateSubsystem* FrontendState, UT66MiniSaveSubsystem* SaveSubsystem) const
{
	if (!FrontendState || !SaveSubsystem)
	{
		return INDEX_NONE;
	}

	if (const int32 PendingSlot = FrontendState->GetPendingSaveSlot(); PendingSlot >= 0 && PendingSlot < UT66MiniSaveSubsystem::MaxSlots)
	{
		return PendingSlot;
	}

	const TArray<FT66MiniSaveSlotSummary> Summaries = SaveSubsystem->BuildRunSlotSummaries(nullptr);
	for (const FT66MiniSaveSlotSummary& Summary : Summaries)
	{
		if (!Summary.bOccupied)
		{
			return Summary.SlotIndex;
		}
	}

	return INDEX_NONE;
}
