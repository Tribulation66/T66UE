// Copyright Tribulation 66. All Rights Reserved.

#include "Save/T66MiniSaveSubsystem.h"

#include "Core/T66PartySubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Gameplay/T66SessionPlayerState.h"
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

	void T66MiniCopySnapshotToLegacyFields(const FT66MiniPartyPlayerSnapshot& Snapshot, UT66MiniRunSaveGame& RunSave)
	{
		RunSave.HeroID = Snapshot.HeroID;
		RunSave.CompanionID = Snapshot.CompanionID;
		RunSave.EquippedIdolIDs = Snapshot.EquippedIdolIDs;
		RunSave.OwnedItemIDs = Snapshot.OwnedItemIDs;
		RunSave.HeroLevel = FMath::Max(1, Snapshot.HeroLevel);
		RunSave.CurrentHealth = Snapshot.CurrentHealth;
		RunSave.MaxHealth = Snapshot.MaxHealth;
		RunSave.Materials = Snapshot.Materials;
		RunSave.Gold = Snapshot.Gold;
		RunSave.Experience = Snapshot.Experience;
		RunSave.UltimateCooldownRemaining = Snapshot.UltimateCooldownRemaining;
		RunSave.bEnduranceCheatUsedThisWave = Snapshot.bEnduranceCheatUsedThisWave;
		RunSave.bQuickReviveReady = Snapshot.bQuickReviveReady;
		RunSave.bHasPlayerLocation = Snapshot.bHasPlayerLocation;
		RunSave.PlayerLocation = Snapshot.PlayerLocation;
	}

	void T66MiniCopyLegacyFieldsToSnapshot(const UT66MiniRunSaveGame& RunSave, FT66MiniPartyPlayerSnapshot& Snapshot)
	{
		Snapshot.HeroID = RunSave.HeroID;
		Snapshot.CompanionID = RunSave.CompanionID;
		Snapshot.EquippedIdolIDs = RunSave.EquippedIdolIDs;
		Snapshot.OwnedItemIDs = RunSave.OwnedItemIDs;
		Snapshot.HeroLevel = FMath::Max(1, RunSave.HeroLevel);
		Snapshot.CurrentHealth = RunSave.CurrentHealth;
		Snapshot.MaxHealth = RunSave.MaxHealth;
		Snapshot.Materials = RunSave.Materials;
		Snapshot.Gold = RunSave.Gold;
		Snapshot.Experience = RunSave.Experience;
		Snapshot.UltimateCooldownRemaining = RunSave.UltimateCooldownRemaining;
		Snapshot.bEnduranceCheatUsedThisWave = RunSave.bEnduranceCheatUsedThisWave;
		Snapshot.bQuickReviveReady = RunSave.bQuickReviveReady;
		Snapshot.bHasPlayerLocation = RunSave.bHasPlayerLocation;
		Snapshot.PlayerLocation = RunSave.PlayerLocation;
	}
}

void UT66MiniSaveSubsystem::Deinitialize()
{
	CachedProfileSave.Reset();
	bHasResolvedProfileSave = false;

	Super::Deinitialize();
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

	const FString SlotName = MakeRunSlotName(SlotIndex);
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, T66MiniSaveUserIndex))
	{
		return nullptr;
	}

	return Cast<UT66MiniRunSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, T66MiniSaveUserIndex));
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
	RunSave->UltimateCooldownRemaining = 0.f;
	RunSave->bEnduranceCheatUsedThisWave = false;
	RunSave->bQuickReviveReady = false;
	RunSave->TotalRunSeconds = 0.f;
	RunSave->ShopRerollCount = 0;
	RunSave->bHasMidWaveSnapshot = false;
	RunSave->bPendingShopIntermission = false;
	RunSave->bHasPlayerLocation = false;
	RunSave->PlayerLocation = FVector::ZeroVector;
	RunSave->WaveSecondsRemaining = 60.f;
	RunSave->bBossSpawnedForWave = false;
	RunSave->BossTelegraphRemaining = 0.f;
	RunSave->PendingBossID = NAME_None;
	RunSave->PendingBossSpawnLocation = FVector::ZeroVector;
	RunSave->EnemySpawnAccumulator = 0.f;
	RunSave->InteractableSpawnAccumulator = 0.f;
	RunSave->TrapSpawnAccumulator = 0.f;
	RunSave->PostBossDelayRemaining = 0.f;
	RunSave->EnemySnapshots.Reset();
	RunSave->PickupSnapshots.Reset();
	RunSave->InteractableSnapshots.Reset();
	RunSave->TrapSnapshots.Reset();
	RunSave->CircusDebt = 0;
	RunSave->CircusAnger01 = 0.f;
	RunSave->CircusBuybackItemIDs.Reset();
	RunSave->CircusVendorRerollCount = 0;
	RunSave->bOnlinePartyRun = false;
	RunSave->OnlineHostSteamId.Reset();
	RunSave->OnlineHostDisplayName.Reset();
	RunSave->OnlinePartyMemberIds.Reset();
	RunSave->OnlinePartyMemberDisplayNames.Reset();
	RunSave->PartyPlayerSnapshots.Reset();
	RunSave->LastUpdatedUtc = BuildUtcNowString();
	return RunSave;
}

void UT66MiniSaveSubsystem::ConfigureRunSaveForCurrentParty(UT66MiniRunSaveGame* RunSave) const
{
	if (!RunSave)
	{
		return;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UT66PartySubsystem* PartySubsystem = GameInstance ? GameInstance->GetSubsystem<UT66PartySubsystem>() : nullptr;
	const UT66SessionSubsystem* SessionSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	const bool bOnlinePartyRun = PartySubsystem && PartySubsystem->HasRemotePartyMembers();
	RunSave->bOnlinePartyRun = bOnlinePartyRun;
	RunSave->OnlinePartyMemberIds = PartySubsystem ? PartySubsystem->GetCurrentPartyMemberIds() : TArray<FString>();
	RunSave->OnlinePartyMemberDisplayNames = PartySubsystem ? PartySubsystem->GetCurrentPartyMemberDisplayNames() : TArray<FString>();

	FT66LobbyPlayerInfo HostLobbyInfo;
	if (bOnlinePartyRun && SessionSubsystem && SessionSubsystem->GetHostLobbyProfile(HostLobbyInfo))
	{
		RunSave->OnlineHostSteamId = HostLobbyInfo.SteamId;
		RunSave->OnlineHostDisplayName = HostLobbyInfo.DisplayName;
	}
	else
	{
		RunSave->OnlineHostSteamId = PartySubsystem ? PartySubsystem->GetLocalPlayerId() : FString();
		RunSave->OnlineHostDisplayName = PartySubsystem ? PartySubsystem->GetLocalDisplayName() : FString();
	}

	if (!bOnlinePartyRun)
	{
		RunSave->OnlinePartyMemberIds.Reset();
		RunSave->OnlinePartyMemberDisplayNames.Reset();
		RunSave->OnlineHostSteamId.Reset();
		RunSave->OnlineHostDisplayName.Reset();
		RunSave->PartyPlayerSnapshots.Reset();
	}
}

bool UT66MiniSaveSubsystem::SyncPartyPlayerSnapshotsFromLobby(UT66MiniRunSaveGame* RunSave, const UT66MiniFrontendStateSubsystem* FrontendState) const
{
	if (!RunSave)
	{
		return false;
	}

	ConfigureRunSaveForCurrentParty(RunSave);
	if (!RunSave->bOnlinePartyRun)
	{
		return false;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UT66SessionSubsystem* SessionSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	TMap<FString, FT66MiniPartyPlayerSnapshot> ExistingSnapshotsById;
	for (const FT66MiniPartyPlayerSnapshot& Snapshot : RunSave->PartyPlayerSnapshots)
	{
		if (!Snapshot.SteamId.IsEmpty())
		{
			ExistingSnapshotsById.Add(Snapshot.SteamId, Snapshot);
		}
	}

	TArray<FT66LobbyPlayerInfo> LobbyProfiles;
	if (SessionSubsystem)
	{
		SessionSubsystem->GetCurrentLobbyProfiles(LobbyProfiles);
	}

	FString LocalPlayerId;
	FString LocalDisplayName;
	ResolveLocalPartyIdentity(LocalPlayerId, LocalDisplayName);

	TArray<FT66MiniPartyPlayerSnapshot> RebuiltSnapshots;
	RebuiltSnapshots.Reserve(FMath::Max(LobbyProfiles.Num(), RunSave->OnlinePartyMemberIds.Num()));

	for (const FT66LobbyPlayerInfo& LobbyInfo : LobbyProfiles)
	{
		const FString SnapshotPlayerId = !LobbyInfo.SteamId.IsEmpty() ? LobbyInfo.SteamId : LocalPlayerId;
		if (SnapshotPlayerId.IsEmpty())
		{
			continue;
		}

		FT66MiniPartyPlayerSnapshot Snapshot;
		if (const FT66MiniPartyPlayerSnapshot* ExistingSnapshot = ExistingSnapshotsById.Find(SnapshotPlayerId))
		{
			Snapshot = *ExistingSnapshot;
		}

		Snapshot.SteamId = SnapshotPlayerId;
		Snapshot.DisplayName = !LobbyInfo.DisplayName.IsEmpty()
			? LobbyInfo.DisplayName
			: (Snapshot.DisplayName.IsEmpty() ? LocalDisplayName : Snapshot.DisplayName);
		Snapshot.HeroID = !LobbyInfo.MiniSelectedHeroID.IsNone()
			? LobbyInfo.MiniSelectedHeroID
			: ((SnapshotPlayerId == LocalPlayerId && FrontendState) ? FrontendState->GetSelectedHeroID() : Snapshot.HeroID);
		Snapshot.CompanionID = !LobbyInfo.MiniSelectedCompanionID.IsNone()
			? LobbyInfo.MiniSelectedCompanionID
			: ((SnapshotPlayerId == LocalPlayerId && FrontendState) ? FrontendState->GetSelectedCompanionID() : Snapshot.CompanionID);
		if (LobbyInfo.MiniSelectedIdolIDs.Num() > 0)
		{
			Snapshot.EquippedIdolIDs = LobbyInfo.MiniSelectedIdolIDs;
		}
		else if (SnapshotPlayerId == LocalPlayerId && FrontendState)
		{
			Snapshot.EquippedIdolIDs = FrontendState->GetSelectedIdolIDs();
		}

		Snapshot.HeroLevel = FMath::Max(1, Snapshot.HeroLevel);
		Snapshot.MaxHealth = FMath::Max(1.f, Snapshot.MaxHealth);
		Snapshot.CurrentHealth = FMath::Clamp(Snapshot.CurrentHealth, 0.f, Snapshot.MaxHealth);
		RebuiltSnapshots.Add(MoveTemp(Snapshot));
	}

	if (RebuiltSnapshots.Num() == 0 && !LocalPlayerId.IsEmpty())
	{
		FT66MiniPartyPlayerSnapshot Snapshot;
		Snapshot.SteamId = LocalPlayerId;
		Snapshot.DisplayName = LocalDisplayName;
		if (FrontendState)
		{
			Snapshot.HeroID = FrontendState->GetSelectedHeroID();
			Snapshot.CompanionID = FrontendState->GetSelectedCompanionID();
			Snapshot.EquippedIdolIDs = FrontendState->GetSelectedIdolIDs();
		}
		RebuiltSnapshots.Add(MoveTemp(Snapshot));
	}

	RunSave->PartyPlayerSnapshots = MoveTemp(RebuiltSnapshots);
	return RunSave->PartyPlayerSnapshots.Num() > 0;
}

bool UT66MiniSaveSubsystem::IsRunCompatibleWithCurrentParty(const UT66MiniRunSaveGame* RunSave, FString* OutFailureReason) const
{
	if (!RunSave)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = TEXT("No mini run save is available.");
		}
		return false;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UT66PartySubsystem* PartySubsystem = GameInstance ? GameInstance->GetSubsystem<UT66PartySubsystem>() : nullptr;
	if (!RunSave->bOnlinePartyRun && (!PartySubsystem || !PartySubsystem->HasRemotePartyMembers()))
	{
		return true;
	}

	if (RunSave->bOnlinePartyRun)
	{
		if (!PartySubsystem || !PartySubsystem->HasRemotePartyMembers())
		{
			if (OutFailureReason)
			{
				*OutFailureReason = TEXT("This mini party save can only be loaded after the original party is reformed.");
			}
			return false;
		}
	}

	if (!RunSave->bOnlinePartyRun)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = TEXT("This mini save was created for solo play, not a party run.");
		}
		return false;
	}

	const TArray<FString> CurrentPartyIds = PartySubsystem->GetCurrentPartyMemberIds();
	TArray<FString> SavedPartyIds = RunSave->OnlinePartyMemberIds;
	if (SavedPartyIds.Num() == 0)
	{
		for (const FT66MiniPartyPlayerSnapshot& Snapshot : RunSave->PartyPlayerSnapshots)
		{
			if (!Snapshot.SteamId.IsEmpty())
			{
				SavedPartyIds.AddUnique(Snapshot.SteamId);
			}
		}
	}

	if (CurrentPartyIds.Num() != SavedPartyIds.Num())
	{
		if (OutFailureReason)
		{
			*OutFailureReason = TEXT("This mini party save was recorded with a different party size.");
		}
		return false;
	}

	for (const FString& PlayerId : CurrentPartyIds)
	{
		if (!SavedPartyIds.Contains(PlayerId))
		{
			if (OutFailureReason)
			{
				*OutFailureReason = TEXT("This mini party save belongs to a different set of players.");
			}
			return false;
		}
	}

	return true;
}

bool UT66MiniSaveSubsystem::ResolvePartyPlayerSnapshotIndex(const UT66MiniRunSaveGame* RunSave, const FString& PlayerId, int32& OutIndex) const
{
	OutIndex = INDEX_NONE;
	if (!RunSave || PlayerId.IsEmpty())
	{
		return false;
	}

	for (int32 SnapshotIndex = 0; SnapshotIndex < RunSave->PartyPlayerSnapshots.Num(); ++SnapshotIndex)
	{
		if (RunSave->PartyPlayerSnapshots[SnapshotIndex].SteamId == PlayerId)
		{
			OutIndex = SnapshotIndex;
			return true;
		}
	}

	return false;
}

bool UT66MiniSaveSubsystem::ResolveLocalPartyPlayerSnapshotIndex(const UT66MiniRunSaveGame* RunSave, int32& OutIndex) const
{
	FString LocalPlayerId;
	FString LocalDisplayName;
	ResolveLocalPartyIdentity(LocalPlayerId, LocalDisplayName);
	if (ResolvePartyPlayerSnapshotIndex(RunSave, LocalPlayerId, OutIndex))
	{
		return true;
	}

	if (!RunSave || RunSave->PartyPlayerSnapshots.Num() <= 0)
	{
		OutIndex = INDEX_NONE;
		return false;
	}

	OutIndex = 0;
	return true;
}

bool UT66MiniSaveSubsystem::MirrorPartyPlayerSnapshotToLegacyFields(UT66MiniRunSaveGame* RunSave, const FString& PlayerId) const
{
	int32 SnapshotIndex = INDEX_NONE;
	if (!RunSave || !ResolvePartyPlayerSnapshotIndex(RunSave, PlayerId, SnapshotIndex))
	{
		return false;
	}

	T66MiniCopySnapshotToLegacyFields(RunSave->PartyPlayerSnapshots[SnapshotIndex], *RunSave);
	return true;
}

bool UT66MiniSaveSubsystem::MirrorLocalPartyPlayerSnapshotToLegacyFields(UT66MiniRunSaveGame* RunSave) const
{
	int32 SnapshotIndex = INDEX_NONE;
	if (!RunSave || !ResolveLocalPartyPlayerSnapshotIndex(RunSave, SnapshotIndex))
	{
		return false;
	}

	T66MiniCopySnapshotToLegacyFields(RunSave->PartyPlayerSnapshots[SnapshotIndex], *RunSave);
	return true;
}

bool UT66MiniSaveSubsystem::CapturePartyPlayerSnapshotFromLegacyFields(UT66MiniRunSaveGame* RunSave, const FString& PlayerId) const
{
	int32 SnapshotIndex = INDEX_NONE;
	if (!RunSave || !ResolvePartyPlayerSnapshotIndex(RunSave, PlayerId, SnapshotIndex))
	{
		return false;
	}

	T66MiniCopyLegacyFieldsToSnapshot(*RunSave, RunSave->PartyPlayerSnapshots[SnapshotIndex]);
	return true;
}

bool UT66MiniSaveSubsystem::CaptureLocalPartyPlayerSnapshotFromLegacyFields(UT66MiniRunSaveGame* RunSave) const
{
	int32 SnapshotIndex = INDEX_NONE;
	if (!RunSave || !ResolveLocalPartyPlayerSnapshotIndex(RunSave, SnapshotIndex))
	{
		return false;
	}

	T66MiniCopyLegacyFieldsToSnapshot(*RunSave, RunSave->PartyPlayerSnapshots[SnapshotIndex]);
	return true;
}

UT66MiniProfileSaveGame* UT66MiniSaveSubsystem::LoadOrCreateProfileSave(const UT66MiniDataSubsystem* DataSubsystem) const
{
	UT66MiniProfileSaveGame* ProfileSave = CachedProfileSave.Get();
	bool bCreatedFreshProfile = false;

	if (!bHasResolvedProfileSave)
	{
		const FString ProfileSlotName = MakeProfileSlotName();
		if (UGameplayStatics::DoesSaveGameExist(ProfileSlotName, T66MiniSaveUserIndex))
		{
			ProfileSave = Cast<UT66MiniProfileSaveGame>(UGameplayStatics::LoadGameFromSlot(ProfileSlotName, T66MiniSaveUserIndex));
		}

		if (!ProfileSave)
		{
			ProfileSave = Cast<UT66MiniProfileSaveGame>(UGameplayStatics::CreateSaveGameObject(UT66MiniProfileSaveGame::StaticClass()));
			if (!ProfileSave)
			{
				bHasResolvedProfileSave = true;
				return nullptr;
			}

			ProfileSave->BestWaveReached = 1;
			ProfileSave->TotalStagesCleared = 0;
			bCreatedFreshProfile = true;
		}

		CachedProfileSave.Reset(ProfileSave);
		bHasResolvedProfileSave = true;
	}

	if (!ProfileSave)
	{
		return nullptr;
	}

	bool bChanged = false;
	T66MiniSeedDefaultProfileUnlocks(ProfileSave, DataSubsystem, bChanged);

	const int32 ClampedCompletedRunCount = FMath::Max(0, ProfileSave->CompletedRunCount);
	if (ProfileSave->CompletedRunCount != ClampedCompletedRunCount)
	{
		ProfileSave->CompletedRunCount = ClampedCompletedRunCount;
		bChanged = true;
	}

	const int32 ClampedFailedRunCount = FMath::Max(0, ProfileSave->FailedRunCount);
	if (ProfileSave->FailedRunCount != ClampedFailedRunCount)
	{
		ProfileSave->FailedRunCount = ClampedFailedRunCount;
		bChanged = true;
	}

	const int32 ClampedBestWaveReached = FMath::Max(1, ProfileSave->BestWaveReached);
	if (ProfileSave->BestWaveReached != ClampedBestWaveReached)
	{
		ProfileSave->BestWaveReached = ClampedBestWaveReached;
		bChanged = true;
	}

	const int32 ClampedLifetimeMaterialsCollected = FMath::Max(0, ProfileSave->LifetimeMaterialsCollected);
	if (ProfileSave->LifetimeMaterialsCollected != ClampedLifetimeMaterialsCollected)
	{
		ProfileSave->LifetimeMaterialsCollected = ClampedLifetimeMaterialsCollected;
		bChanged = true;
	}

	const int32 ClampedTotalStagesCleared = FMath::Max(0, ProfileSave->TotalStagesCleared);
	if (ProfileSave->TotalStagesCleared != ClampedTotalStagesCleared)
	{
		ProfileSave->TotalStagesCleared = ClampedTotalStagesCleared;
		bChanged = true;
	}

	for (TPair<FName, int32>& Pair : ProfileSave->CompanionUnityStagesClearedByID)
	{
		const int32 ClampedValue = FMath::Max(0, Pair.Value);
		if (Pair.Value != ClampedValue)
		{
			Pair.Value = ClampedValue;
			bChanged = true;
		}
	}

	if (bCreatedFreshProfile || bChanged)
	{
		SaveProfile(ProfileSave);
	}

	return ProfileSave;
}

bool UT66MiniSaveSubsystem::SaveProfile(UT66MiniProfileSaveGame* ProfileSave) const
{
	if (!ProfileSave)
	{
		return false;
	}

	CachedProfileSave.Reset(ProfileSave);
	bHasResolvedProfileSave = true;
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

bool UT66MiniSaveSubsystem::ResolveLocalPartyIdentity(FString& OutPlayerId, FString& OutDisplayName) const
{
	OutPlayerId.Reset();
	OutDisplayName.Reset();

	const UGameInstance* GameInstance = GetGameInstance();
	const UT66PartySubsystem* PartySubsystem = GameInstance ? GameInstance->GetSubsystem<UT66PartySubsystem>() : nullptr;
	if (PartySubsystem)
	{
		OutPlayerId = PartySubsystem->GetLocalPlayerId();
		OutDisplayName = PartySubsystem->GetLocalDisplayName();
	}

	if (!OutPlayerId.IsEmpty() || !OutDisplayName.IsEmpty())
	{
		return true;
	}

	const UT66SessionSubsystem* SessionSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (SessionSubsystem)
	{
		TArray<FT66LobbyPlayerInfo> LobbyProfiles;
		SessionSubsystem->GetCurrentLobbyProfiles(LobbyProfiles);
		for (const FT66LobbyPlayerInfo& LobbyInfo : LobbyProfiles)
		{
			if (LobbyInfo.bPartyHost || LobbyInfo.DisplayName == TEXT("You"))
			{
				OutPlayerId = LobbyInfo.SteamId;
				OutDisplayName = LobbyInfo.DisplayName;
				break;
			}
		}
	}

	return !OutPlayerId.IsEmpty() || !OutDisplayName.IsEmpty();
}
