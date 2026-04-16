// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MiniRunStateSubsystem.h"

#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66SessionSubsystem.h"
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

bool UT66MiniRunStateSubsystem::BootstrapOnlineRunFromFrontend(UT66MiniFrontendStateSubsystem* FrontendState, UT66MiniSaveSubsystem* SaveSubsystem)
{
	if (!FrontendState || !SaveSubsystem)
	{
		LastBootstrapStatus = TEXT("Mini online bootstrap failed: missing frontend or save subsystem.");
		return false;
	}

	UT66MiniRunSaveGame* RunSave = SaveSubsystem->CreateSeededRunSave(FrontendState);
	if (!RunSave)
	{
		LastBootstrapStatus = TEXT("Mini online bootstrap failed: could not build a party run state from frontend selections.");
		return false;
	}

	SaveSubsystem->ConfigureRunSaveForCurrentParty(RunSave);
	SaveSubsystem->SyncPartyPlayerSnapshotsFromLobby(RunSave, FrontendState);
	if (RunSave->bOnlinePartyRun)
	{
		SaveSubsystem->MirrorLocalPartyPlayerSnapshotToLegacyFields(RunSave);
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UT66PartySubsystem* PartySubsystem = GameInstance ? GameInstance->GetSubsystem<UT66PartySubsystem>() : nullptr;
	const UT66SessionSubsystem* SessionSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	const bool bPersistHostRun = RunSave->bOnlinePartyRun
		&& PartySubsystem
		&& PartySubsystem->HasRemotePartyMembers()
		&& SessionSubsystem
		&& SessionSubsystem->IsLocalPlayerPartyHost();

	if (bPersistHostRun)
	{
		const int32 TargetSlot = ResolveTargetSaveSlot(FrontendState, SaveSubsystem);
		if (TargetSlot == INDEX_NONE)
		{
			LastBootstrapStatus = TEXT("Mini online bootstrap failed: all mini save slots are occupied on the host.");
			return false;
		}

		if (!SaveSubsystem->SaveRunToSlot(TargetSlot, RunSave))
		{
			LastBootstrapStatus = TEXT("Mini online bootstrap failed: the host could not persist the party run state.");
			return false;
		}

		ActiveSaveSlot = TargetSlot;
		FrontendState->SetPendingSaveSlot(TargetSlot);
		LastBootstrapStatus = FString::Printf(TEXT("Mini online run prepared in host save slot %d."), TargetSlot + 1);
	}
	else
	{
		RunSave->SaveSlotIndex = INDEX_NONE;
		ActiveSaveSlot = INDEX_NONE;
		FrontendState->SetPendingSaveSlot(INDEX_NONE);
		LastBootstrapStatus = TEXT("Mini online run state prepared.");
	}

	ActiveRun = RunSave;
	ActiveRunChanged.Broadcast();
	return true;
}

bool UT66MiniRunStateSubsystem::BootstrapTransientRunFromFrontend(UT66MiniFrontendStateSubsystem* FrontendState, UT66MiniSaveSubsystem* SaveSubsystem)
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UT66PartySubsystem* PartySubsystem = GameInstance ? GameInstance->GetSubsystem<UT66PartySubsystem>() : nullptr;
	if (PartySubsystem && PartySubsystem->HasRemotePartyMembers())
	{
		return BootstrapOnlineRunFromFrontend(FrontendState, SaveSubsystem);
	}

	if (!FrontendState || !SaveSubsystem)
	{
		LastBootstrapStatus = TEXT("Mini bootstrap failed: missing frontend or save subsystem.");
		return false;
	}

	UT66MiniRunSaveGame* RunSave = SaveSubsystem->CreateSeededRunSave(FrontendState);
	if (!RunSave)
	{
		LastBootstrapStatus = TEXT("Mini bootstrap failed: could not build a transient run state from frontend selections.");
		return false;
	}

	RunSave->SaveSlotIndex = INDEX_NONE;
	ActiveRun = RunSave;
	ActiveSaveSlot = INDEX_NONE;
	FrontendState->SetPendingSaveSlot(INDEX_NONE);
	LastBootstrapStatus = TEXT("Mini online run state prepared.");
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

	if (InRunSave->bOnlinePartyRun)
	{
		if (UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr)
		{
			SaveSubsystem->MirrorLocalPartyPlayerSnapshotToLegacyFields(InRunSave);
		}
	}

	ActiveRun = InRunSave;
	ActiveSaveSlot = InRunSave->SaveSlotIndex;
	LastBootstrapStatus = ActiveSaveSlot != INDEX_NONE
		? FString::Printf(TEXT("Mini run loaded from save slot %d."), ActiveSaveSlot + 1)
		: TEXT("Mini run loaded from the active party state.");
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
