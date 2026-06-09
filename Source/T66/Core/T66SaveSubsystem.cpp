// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66SaveSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66SaveMigration.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Save, Log, All);

#if !UE_BUILD_SHIPPING
namespace
{
	static constexpr const TCHAR* T66SaveIntegrityOwnerId = TEXT("t66_save_integrity_harness");
	static constexpr const TCHAR* T66SaveIntegrityConfirmToken = TEXT("CONFIRM");

	static FString T66_BuildSaveIntegrityMapName(const FString& Marker)
	{
		return FString::Printf(TEXT("T66_SaveIntegrity_%s"), *Marker);
	}

	static UT66SaveSubsystem* T66_GetSaveSubsystemFromWorld(UWorld* World)
	{
		if (!World)
		{
			UE_LOG(LogT66Save, Warning, TEXT("[SaveIntegrity] Command failed: no world."));
			return nullptr;
		}

		UGameInstance* GameInstance = World->GetGameInstance();
		UT66SaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66SaveSubsystem>() : nullptr;
		if (!SaveSubsystem)
		{
			UE_LOG(LogT66Save, Warning, TEXT("[SaveIntegrity] Command failed: no save subsystem."));
			return nullptr;
		}

		return SaveSubsystem;
	}

	static bool T66_ParseSaveIntegrityArgs(
		const TArray<FString>& Args,
		int32& OutSlotIndex,
		FString& OutMarker,
		int32& OutExitCode)
	{
		if (Args.Num() < 3)
		{
			return false;
		}

		if (!LexTryParseString(OutSlotIndex, *Args[0]))
		{
			return false;
		}

		OutMarker = Args[1].TrimStartAndEnd();
		if (OutMarker.IsEmpty())
		{
			return false;
		}

		if (!Args[2].Equals(T66SaveIntegrityConfirmToken, ESearchCase::IgnoreCase))
		{
			return false;
		}

		OutExitCode = 0;
		if (Args.Num() > 3)
		{
			LexTryParseString(OutExitCode, *Args[3]);
		}

		return true;
	}

	static bool T66_RequestSaveIntegrityExit(UGameInstance* GameInstance, const int32 ExitCode, const TCHAR* ExitTag)
	{
		if (UT66ShutdownSubsystem* Shutdown = GameInstance ? GameInstance->GetSubsystem<UT66ShutdownSubsystem>() : nullptr)
		{
			Shutdown->RunShutdown(ET66ShutdownReason::TestHarness, true, ExitCode, ExitTag);
			return true;
		}

		UE_LOG(LogT66Save, Error, TEXT("[SaveIntegrity] Unable to request proof exit: missing shutdown subsystem."));
		return false;
	}

	static void T66_QueueIntegrityShutdownCommand(const TArray<FString>& Args, UWorld* World)
	{
		int32 SlotIndex = INDEX_NONE;
		FString Marker;
		int32 ExitCode = 0;
		if (!T66_ParseSaveIntegrityArgs(Args, SlotIndex, Marker, ExitCode))
		{
			UE_LOG(LogT66Save, Warning, TEXT("[SaveIntegrity] Usage: T66.Save.QueueIntegrityShutdown <slot 0-8> <marker> CONFIRM [exitCode]"));
			return;
		}

		if (UT66SaveSubsystem* SaveSubsystem = T66_GetSaveSubsystemFromWorld(World))
		{
			SaveSubsystem->RunQueuedSaveIntegrityShutdownHarness(SlotIndex, Marker, ExitCode);
		}
	}

	static void T66_VerifyIntegritySlotCommand(const TArray<FString>& Args, UWorld* World)
	{
		int32 SlotIndex = INDEX_NONE;
		FString Marker;
		int32 ExitCode = 0;
		if (!T66_ParseSaveIntegrityArgs(Args, SlotIndex, Marker, ExitCode))
		{
			UE_LOG(LogT66Save, Warning, TEXT("[SaveIntegrity] Usage: T66.Save.VerifyIntegritySlot <slot 0-8> <marker> CONFIRM [exitCode]"));
			return;
		}

		if (UT66SaveSubsystem* SaveSubsystem = T66_GetSaveSubsystemFromWorld(World))
		{
			SaveSubsystem->RunSaveIntegrityVerificationHarness(SlotIndex, Marker, ExitCode);
		}
	}

	static FAutoConsoleCommandWithWorldAndArgs T66SaveQueueIntegrityShutdownCommand(
		TEXT("T66.Save.QueueIntegrityShutdown"),
		TEXT("Development automation: queue a run save/index save, exercise durable-state shutdown, verify immediate load, then exit. Args: <slot 0-8> <marker> CONFIRM [exitCode]."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&T66_QueueIntegrityShutdownCommand));

	static FAutoConsoleCommandWithWorldAndArgs T66SaveVerifyIntegritySlotCommand(
		TEXT("T66.Save.VerifyIntegritySlot"),
		TEXT("Development automation: verify a save integrity harness slot/index marker can be loaded in a fresh process. Args: <slot 0-8> <marker> CONFIRM [exitCode]."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&T66_VerifyIntegritySlotCommand));
}
#endif

const FString UT66SaveSubsystem::SaveIndexSlotName(TEXT("T66_SaveIndex"));
const FString UT66SaveSubsystem::SlotNamePrefix(TEXT("T66_Slot_"));

void UT66SaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UT66ShutdownSubsystem::StaticClass());
	Super::Initialize(Collection);
	DeleteOutdatedRunSaves();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66ShutdownSubsystem* Shutdown = GI->GetSubsystem<UT66ShutdownSubsystem>())
		{
			ShutdownParticipantHandle = Shutdown->RegisterParticipant(
				this,
				FName(TEXT("Save.RunSlotAndIndex")),
				ET66ShutdownPhase::DurableState,
				10,
				1.0,
				true,
				FT66ShutdownParticipantDelegate::CreateUObject(this, &UT66SaveSubsystem::HandleShutdown));
		}
	}
}

void UT66SaveSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66ShutdownSubsystem* Shutdown = GI->GetSubsystem<UT66ShutdownSubsystem>())
		{
			Shutdown->UnregisterParticipant(ShutdownParticipantHandle);
		}
	}
	ShutdownParticipantHandle.Reset();
	FlushPendingDurableState(TEXT("Deinitialize"));
	Super::Deinitialize();
}

FString UT66SaveSubsystem::GetSlotName(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots) return FString();
	return FString::Printf(TEXT("%s%02d"), *SlotNamePrefix, SlotIndex);
}

int32 UT66SaveSubsystem::FindFirstEmptySlot() const
{
	UT66SaveIndex* Index = LoadOrCreateIndex();
	if (!Index) return INDEX_NONE;

	for (int32 i = 0; i < MaxSlots; ++i)
	{
		if (Index->SlotMeta.Num() <= i || !Index->SlotMeta[i].bOccupied)
		{
			// Also verify no file exists for this slot (in case index was lost)
			if (!UGameplayStatics::DoesSaveGameExist(GetSlotName(i), 0))
			{
				return i;
			}
		}
	}
	return INDEX_NONE;
}

int32 UT66SaveSubsystem::FindOldestOccupiedSlot() const
{
	UT66SaveIndex* Index = LoadOrCreateIndex();
	if (!Index) return INDEX_NONE;

	int32 BestSlot = INDEX_NONE;
	FDateTime BestTime = FDateTime::MaxValue();

	for (int32 i = 0; i < MaxSlots; ++i)
	{
		if (!UGameplayStatics::DoesSaveGameExist(GetSlotName(i), 0))
		{
			continue;
		}

		FDateTime Parsed = FDateTime::MinValue();
		if (Index->SlotMeta.Num() > i)
		{
			const FString& Utc = Index->SlotMeta[i].LastPlayedUtc;
			if (!Utc.IsEmpty())
			{
				FDateTime::ParseIso8601(*Utc, Parsed);
			}
		}

		if (BestSlot == INDEX_NONE || Parsed < BestTime)
		{
			BestSlot = i;
			BestTime = Parsed;
		}
	}

	// If index metadata was empty, just fall back to slot 0 if it exists.
	if (BestSlot == INDEX_NONE)
	{
		for (int32 i = 0; i < MaxSlots; ++i)
		{
			if (UGameplayStatics::DoesSaveGameExist(GetSlotName(i), 0))
			{
				return i;
			}
		}
	}
	return BestSlot;
}

bool UT66SaveSubsystem::DoesSlotExist(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots) return false;
	return UGameplayStatics::DoesSaveGameExist(GetSlotName(SlotIndex), 0);
}

bool UT66SaveSubsystem::SaveToSlot(int32 SlotIndex, UT66RunSaveGame* SaveGameObject)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots || !SaveGameObject) return false;
	SaveGameObject->SaveVersion = T66CurrentRunSaveVersion;

	FString SlotName = GetSlotName(SlotIndex);
	const int64 SaveSequence = ++RunSaveAsyncSequence;
	PendingRunSaveSequence = SaveSequence;
	PendingRunSaveSlotName = SlotName;
	PendingRunSaveObject = SaveGameObject;

	// [GOLD] Async save: writes to disk on a background thread so the game thread doesn't hitch.
	UE_LOG(LogT66Save, Log, TEXT("[GOLD] AsyncSave: queuing async save for slot %s"), *SlotName);
	TWeakObjectPtr<UT66SaveSubsystem> WeakThis(this);
	UGameplayStatics::AsyncSaveGameToSlot(SaveGameObject, SlotName, 0,
		FAsyncSaveGameToSlotDelegate::CreateLambda([WeakThis, SlotName, SaveSequence](const FString& /*InSlotName*/, const int32 /*UserIndex*/, bool bSuccess)
		{
			if (bSuccess)
			{
				UE_LOG(LogT66Save, Verbose, TEXT("[GOLD] AsyncSave: slot %s saved successfully"), *SlotName);
				if (UT66SaveSubsystem* SaveSubsystem = WeakThis.Get())
				{
					if (SaveSubsystem->PendingRunSaveSequence == SaveSequence)
					{
						SaveSubsystem->PendingRunSaveSlotName.Reset();
						SaveSubsystem->PendingRunSaveObject = nullptr;
					}
				}
			}
			else
			{
				UE_LOG(LogT66Save, Warning, TEXT("[GOLD] AsyncSave: FAILED for slot %s"), *SlotName);
			}
		}));

	// Update the index in memory immediately (metadata is tiny; write async too).
	UpdateIndexOnSave(SlotIndex, SaveGameObject->HeroID.ToString(), SaveGameObject->MapName, SaveGameObject->LastPlayedUtc);
	return true;
}

UT66RunSaveGame* UT66SaveSubsystem::LoadFromSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots) return nullptr;

	// Load remains synchronous because callers expect the data immediately.
	// If needed, callers can use AsyncLoadGameFromSlot with a callback instead.
	FString SlotName = GetSlotName(SlotIndex);
	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
	UT66RunSaveGame* RunSave = Cast<UT66RunSaveGame>(Loaded);
	if (RunSave)
	{
		if (RunSave->SaveVersion < T66PercentDamageRunSaveVersion)
		{
			UE_LOG(LogT66Save, Warning, TEXT("[SaveMigration] Deleting outdated run save Slot=%s Version=%d Required=%d"),
				*SlotName,
				RunSave->SaveVersion,
				T66PercentDamageRunSaveVersion);
			UGameplayStatics::DeleteGameInSlot(SlotName, 0);
			if (UT66SaveIndex* Index = LoadOrCreateIndex())
			{
				if (Index->SlotMeta.IsValidIndex(SlotIndex))
				{
					Index->SlotMeta[SlotIndex] = FT66SaveSlotMeta{};
					SaveIndex(Index);
				}
			}
			return nullptr;
		}
		if (RunSave->SaveVersion < T66SparseActiveHeroIdRunSaveVersion)
		{
			RunSave->HeroID = T66MigrateSparseActiveHeroID(RunSave->HeroID);
			RunSave->SaveVersion = T66SparseActiveHeroIdRunSaveVersion;
		}
		if (RunSave->SaveVersion < T66RunModeCategoryRunSaveVersion)
		{
			RunSave->RunMode = ET66RunMode::Regular;
			RunSave->RunCategory = ET66RunCategory::Tower;
			RunSave->SaveVersion = T66RunModeCategoryRunSaveVersion;
		}
		if (RunSave->SaveVersion < T66CollectedMobLootRunSaveVersion)
		{
			RunSave->RunSnapshot.CollectedMobLootStack = 0;
			RunSave->SaveVersion = T66CollectedMobLootRunSaveVersion;
		}
		if (RunSave->SaveVersion < T66MobLootRunSummaryCountersRunSaveVersion)
		{
			RunSave->RunSnapshot.MobLootDropsCollectedThisRun = 0;
			RunSave->RunSnapshot.MobLootQuantityCollectedThisRun = 0;
			RunSave->RunSnapshot.MobLootGoldValueCollectedThisRun = 0;
			RunSave->RunSnapshot.MobLootQuantityCollectedByPlayerThisRun = 0;
			RunSave->RunSnapshot.MobLootQuantityCollectedByPetThisRun = 0;
			RunSave->RunSnapshot.MobLootDropsCollectedByPetThisRun = 0;
			RunSave->RunSnapshot.MobLootQuantitySoldThisRun = 0;
			RunSave->RunSnapshot.MobLootSaleGoldThisRun = 0;
			RunSave->SaveVersion = T66MobLootRunSummaryCountersRunSaveVersion;
		}
		T66NormalizeEquippedIdolSaveArrays(RunSave->EquippedIdols, RunSave->EquippedIdolTiers);
		T66NormalizeEquippedIdolSaveArrays(RunSave->RunSnapshot.EquippedIdols, RunSave->RunSnapshot.EquippedIdolTiers);

		FString LocalPlayerId = TEXT("local_player");
		FString LocalDisplayName = TEXT("You");
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
			{
				if (!PartySubsystem->GetLocalPlayerId().IsEmpty())
				{
					LocalPlayerId = PartySubsystem->GetLocalPlayerId();
				}
				if (!PartySubsystem->GetLocalDisplayName().IsEmpty())
				{
					LocalDisplayName = PartySubsystem->GetLocalDisplayName();
				}
			}
		}

		if (RunSave->OwnerPlayerId.IsEmpty())
		{
			RunSave->OwnerPlayerId = LocalPlayerId;
		}
		if (RunSave->OwnerDisplayName.IsEmpty())
		{
			RunSave->OwnerDisplayName = LocalDisplayName;
		}
		if (!RunSave->OwnerPlayerId.IsEmpty() && !RunSave->PartyMemberIds.Contains(RunSave->OwnerPlayerId))
		{
			RunSave->PartyMemberIds.Insert(RunSave->OwnerPlayerId, 0);
		}
		if (!RunSave->OwnerDisplayName.IsEmpty() && !RunSave->PartyMemberDisplayNames.Contains(RunSave->OwnerDisplayName))
		{
			RunSave->PartyMemberDisplayNames.Insert(RunSave->OwnerDisplayName, 0);
		}

		if (RunSave->SavedPartyPlayers.Num() == 0)
		{
			FT66SavedPartyPlayerState& LegacyHost = RunSave->SavedPartyPlayers.AddDefaulted_GetRef();
			LegacyHost.PlayerId = RunSave->OwnerPlayerId;
			LegacyHost.DisplayName = RunSave->OwnerDisplayName;
			LegacyHost.HeroID = RunSave->HeroID;
			LegacyHost.HeroBodyType = RunSave->HeroBodyType;
			LegacyHost.HeroSkinID = FName(TEXT("Default"));
			LegacyHost.CompanionID = RunSave->CompanionID;
			LegacyHost.PlayerTransform = RunSave->PlayerTransform;
			LegacyHost.bIsPartyHost = true;
		}

		for (const FT66SavedPartyPlayerState& SavedPlayer : RunSave->SavedPartyPlayers)
		{
			if (!SavedPlayer.PlayerId.IsEmpty() && !RunSave->PartyMemberIds.Contains(SavedPlayer.PlayerId))
			{
				RunSave->PartyMemberIds.Add(SavedPlayer.PlayerId);
			}

			if (!SavedPlayer.DisplayName.IsEmpty() && !RunSave->PartyMemberDisplayNames.Contains(SavedPlayer.DisplayName))
			{
				RunSave->PartyMemberDisplayNames.Add(SavedPlayer.DisplayName);
			}
		}
	}
	return RunSave;
}

void UT66SaveSubsystem::UpdateIndexOnSave(int32 SlotIndex, const FString& HeroDisplayName, const FString& MapName, const FString& LastPlayedUtc)
{
	UT66SaveIndex* Index = LoadOrCreateIndex();
	if (!Index) return;

	while (Index->SlotMeta.Num() <= SlotIndex)
	{
		FT66SaveSlotMeta Meta;
		Index->SlotMeta.Add(Meta);
	}

	Index->SlotMeta[SlotIndex].bOccupied = true;
	Index->SlotMeta[SlotIndex].LastPlayedUtc = LastPlayedUtc;
	Index->SlotMeta[SlotIndex].HeroDisplayName = HeroDisplayName;
	Index->SlotMeta[SlotIndex].MapName = MapName;

	// Async index save (metadata is small, no need to block the game thread).
	SaveIndex(Index);
}

bool UT66SaveSubsystem::GetSlotMeta(int32 SlotIndex, bool& bOutOccupied, FString& OutLastPlayedUtc, FString& OutHeroDisplayName, FString& OutMapName) const
{
	bOutOccupied = false;
	OutLastPlayedUtc = OutHeroDisplayName = OutMapName = FString();
	if (SlotIndex < 0 || SlotIndex >= MaxSlots) return false;

	UT66SaveIndex* Index = LoadOrCreateIndex();
	if (!Index || Index->SlotMeta.Num() <= SlotIndex) return true;

	const FT66SaveSlotMeta& Meta = Index->SlotMeta[SlotIndex];
	bOutOccupied = Meta.bOccupied && UGameplayStatics::DoesSaveGameExist(GetSlotName(SlotIndex), 0);
	OutLastPlayedUtc = Meta.LastPlayedUtc;
	OutHeroDisplayName = Meta.HeroDisplayName;
	OutMapName = Meta.MapName;
	return true;
}

UT66SaveIndex* UT66SaveSubsystem::LoadOrCreateIndex()
{
	if (CachedSaveIndex)
	{
		return CachedSaveIndex;
	}

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SaveIndexSlotName, 0);
	CachedSaveIndex = Cast<UT66SaveIndex>(Loaded);
	if (!CachedSaveIndex)
	{
		CachedSaveIndex = NewObject<UT66SaveIndex>(this);
		if (CachedSaveIndex)
		{
			CachedSaveIndex->SlotMeta.SetNum(MaxSlots);
		}
	}

	return CachedSaveIndex;
}

UT66SaveIndex* UT66SaveSubsystem::LoadOrCreateIndex() const
{
	return const_cast<UT66SaveSubsystem*>(this)->LoadOrCreateIndex();
}

bool UT66SaveSubsystem::SaveIndex(UT66SaveIndex* Index)
{
	if (!Index) return false;
	const int64 SaveSequence = ++SaveIndexAsyncSequence;
	PendingSaveIndexSequence = SaveSequence;
	bSaveIndexFlushNeeded = true;

	// [GOLD] Async index save to avoid blocking the game thread.
	UE_LOG(LogT66Save, Verbose, TEXT("[GOLD] AsyncSave: queuing async save for index"));
	TWeakObjectPtr<UT66SaveSubsystem> WeakThis(this);
	UGameplayStatics::AsyncSaveGameToSlot(Index, SaveIndexSlotName, 0,
		FAsyncSaveGameToSlotDelegate::CreateLambda([WeakThis, SaveSequence](const FString& /*InSlotName*/, const int32 /*UserIndex*/, bool bSuccess)
		{
			if (UT66SaveSubsystem* SaveSubsystem = WeakThis.Get())
			{
				if (bSuccess && SaveSubsystem->PendingSaveIndexSequence == SaveSequence)
				{
					SaveSubsystem->bSaveIndexFlushNeeded = false;
				}
			}

			if (!bSuccess)
			{
				UE_LOG(LogT66Save, Warning, TEXT("[GOLD] AsyncSave: FAILED for save index"));
			}
		}));
	return true;
}

void UT66SaveSubsystem::DeleteOutdatedRunSaves()
{
	UT66SaveIndex* Index = LoadOrCreateIndex();
	bool bIndexChanged = false;

	for (int32 SlotIndex = 0; SlotIndex < MaxSlots; ++SlotIndex)
	{
		const FString SlotName = GetSlotName(SlotIndex);
		if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
		{
			continue;
		}

		UT66RunSaveGame* RunSave = Cast<UT66RunSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		if (!RunSave || RunSave->SaveVersion >= T66PercentDamageRunSaveVersion)
		{
			continue;
		}

		UE_LOG(LogT66Save, Warning, TEXT("[SaveMigration] Startup deleting outdated run save Slot=%s Version=%d Required=%d"),
			*SlotName,
			RunSave->SaveVersion,
			T66PercentDamageRunSaveVersion);
		UGameplayStatics::DeleteGameInSlot(SlotName, 0);
		if (Index)
		{
			while (Index->SlotMeta.Num() <= SlotIndex)
			{
				Index->SlotMeta.AddDefaulted();
			}
			Index->SlotMeta[SlotIndex] = FT66SaveSlotMeta{};
			bIndexChanged = true;
		}
	}

	if (bIndexChanged)
	{
		SaveIndex(Index);
	}
}

bool UT66SaveSubsystem::HandleShutdown(const FT66ShutdownContext& /*Context*/)
{
	return FlushPendingDurableState(TEXT("ShutdownSystem"));
}

bool UT66SaveSubsystem::FlushPendingDurableState(const TCHAR* Reason)
{
	const bool bHadRunSavePending = !PendingRunSaveSlotName.IsEmpty() && PendingRunSaveObject;
	const bool bHadIndexPending = bSaveIndexFlushNeeded;
	bool bOk = true;

	if (bHadRunSavePending)
	{
		const bool bSaved = UGameplayStatics::SaveGameToSlot(PendingRunSaveObject, PendingRunSaveSlotName, 0);
		UE_LOG(LogT66Save, Log, TEXT("[Shutdown] Durable flush run save Slot=%s Saved=%d Reason=%s"),
			*PendingRunSaveSlotName,
			bSaved ? 1 : 0,
			Reason ? Reason : TEXT("Unknown"));
		if (bSaved)
		{
			PendingRunSaveSlotName.Reset();
			PendingRunSaveObject = nullptr;
		}
		bOk &= bSaved;
	}

	if (bHadIndexPending)
	{
		UT66SaveIndex* Index = LoadOrCreateIndex();
		const bool bSaved = Index && UGameplayStatics::SaveGameToSlot(Index, SaveIndexSlotName, 0);
		UE_LOG(LogT66Save, Log, TEXT("[Shutdown] Durable flush save index Saved=%d Reason=%s"),
			bSaved ? 1 : 0,
			Reason ? Reason : TEXT("Unknown"));
		if (bSaved)
		{
			bSaveIndexFlushNeeded = false;
		}
		bOk &= bSaved;
	}

	UE_LOG(LogT66Save, Log, TEXT("[Shutdown] Durable flush complete RunSavePending=%d IndexPending=%d Success=%d Reason=%s"),
		bHadRunSavePending ? 1 : 0,
		bHadIndexPending ? 1 : 0,
		bOk ? 1 : 0,
		Reason ? Reason : TEXT("Unknown"));
	return bOk;
}

#if !UE_BUILD_SHIPPING
bool UT66SaveSubsystem::RunQueuedSaveIntegrityShutdownHarness(const int32 SlotIndex, const FString& Marker, const int32 ExitCode)
{
	UGameInstance* GameInstance = GetGameInstance();
	UT66ShutdownSubsystem* Shutdown = GameInstance ? GameInstance->GetSubsystem<UT66ShutdownSubsystem>() : nullptr;
	if (!Shutdown)
	{
		UE_LOG(LogT66Save, Error, TEXT("[SaveIntegrity] FAIL Slot=%d Marker=%s Reason=MissingShutdownSubsystem"),
			SlotIndex,
			*Marker);
		T66_RequestSaveIntegrityExit(GameInstance, 66, TEXT("T66SaveIntegrityHarness"));
		return false;
	}

	if (SlotIndex < 0 || SlotIndex >= MaxSlots || Marker.IsEmpty())
	{
		UE_LOG(LogT66Save, Error, TEXT("[SaveIntegrity] FAIL Slot=%d Marker=%s Reason=InvalidArguments"),
			SlotIndex,
			*Marker);
		T66_RequestSaveIntegrityExit(GameInstance, 66, TEXT("T66SaveIntegrityHarness"));
		return false;
	}

	bool bWasOccupied = false;
	FString PreviousUtc;
	FString PreviousHero;
	FString PreviousMap;
	GetSlotMeta(SlotIndex, bWasOccupied, PreviousUtc, PreviousHero, PreviousMap);

	UT66RunSaveGame* SaveGameObject = NewObject<UT66RunSaveGame>(this);
	if (!SaveGameObject)
	{
		UE_LOG(LogT66Save, Error, TEXT("[SaveIntegrity] FAIL Slot=%d Marker=%s Reason=CreateSaveGameFailed"),
			SlotIndex,
			*Marker);
		T66_RequestSaveIntegrityExit(GameInstance, 66, TEXT("T66SaveIntegrityHarness"));
		return false;
	}

	const FString ExpectedMapName = T66_BuildSaveIntegrityMapName(Marker);
	const FString LastPlayedUtc = FDateTime::UtcNow().ToIso8601();
	SaveGameObject->SaveVersion = T66CurrentRunSaveVersion;
	SaveGameObject->HeroID = FName(TEXT("Hero_1"));
	SaveGameObject->MapName = ExpectedMapName;
	SaveGameObject->LastPlayedUtc = LastPlayedUtc;
	SaveGameObject->OwnerPlayerId = T66SaveIntegrityOwnerId;
	SaveGameObject->OwnerDisplayName = FString::Printf(TEXT("SaveIntegrity:%s"), *Marker);
	SaveGameObject->StageReached = 66;
	SaveGameObject->RunSeed = 660066;
	SaveGameObject->RunSnapshot.bValid = true;
	SaveGameObject->RunSnapshot.CurrentStage = 66;
	SaveGameObject->RunSnapshot.CurrentHP = 66.f;
	SaveGameObject->RunSnapshot.MaxHP = 66.f;
	SaveGameObject->RunSnapshot.EventLog.Add(FString::Printf(TEXT("SaveIntegrityMarker=%s"), *Marker));

	FT66SavedPartyPlayerState& SavedPlayer = SaveGameObject->SavedPartyPlayers.AddDefaulted_GetRef();
	SavedPlayer.PlayerId = SaveGameObject->OwnerPlayerId;
	SavedPlayer.DisplayName = SaveGameObject->OwnerDisplayName;
	SavedPlayer.HeroID = SaveGameObject->HeroID;
	SavedPlayer.bIsPartyHost = true;
	SaveGameObject->PartyMemberIds.Add(SaveGameObject->OwnerPlayerId);
	SaveGameObject->PartyMemberDisplayNames.Add(SaveGameObject->OwnerDisplayName);

	UE_LOG(LogT66Save, Log, TEXT("[SaveIntegrity] Queue Slot=%d Marker=%s WasOccupied=%d PreviousMap=%s LastPlayedUtc=%s"),
		SlotIndex,
		*Marker,
		bWasOccupied ? 1 : 0,
		*PreviousMap,
		*LastPlayedUtc);

	if (!SaveToSlot(SlotIndex, SaveGameObject))
	{
		UE_LOG(LogT66Save, Error, TEXT("[SaveIntegrity] FAIL Slot=%d Marker=%s Reason=SaveToSlotFailed"),
			SlotIndex,
			*Marker);
		T66_RequestSaveIntegrityExit(GameInstance, 66, TEXT("T66SaveIntegrityHarness"));
		return false;
	}

	const bool bShutdownOk = Shutdown->RunShutdown(ET66ShutdownReason::TestHarness, false, 0, TEXT("T66SaveIntegrityHarness"));
	const bool bPendingCleared = PendingRunSaveSlotName.IsEmpty() && !PendingRunSaveObject && !bSaveIndexFlushNeeded;

	UT66RunSaveGame* LoadedSave = LoadFromSlot(SlotIndex);
	bool bMetaOccupied = false;
	FString MetaUtc;
	FString MetaHero;
	FString MetaMap;
	GetSlotMeta(SlotIndex, bMetaOccupied, MetaUtc, MetaHero, MetaMap);

	const bool bLoadedOk = LoadedSave
		&& LoadedSave->MapName == ExpectedMapName
		&& LoadedSave->OwnerPlayerId == T66SaveIntegrityOwnerId
		&& LoadedSave->OwnerDisplayName.Contains(Marker);
	const bool bMetaOk = bMetaOccupied && MetaMap == ExpectedMapName && MetaUtc == LastPlayedUtc;
	const bool bPass = bShutdownOk && bPendingCleared && bLoadedOk && bMetaOk;

	UE_LOG(LogT66Save, Log, TEXT("[SaveIntegrity] %s Slot=%d Marker=%s ShutdownOk=%d PendingCleared=%d LoadedOk=%d MetaOk=%d ExpectedMap=%s LoadedMap=%s MetaMap=%s MetaUtc=%s"),
		bPass ? TEXT("PASS") : TEXT("FAIL"),
		SlotIndex,
		*Marker,
		bShutdownOk ? 1 : 0,
		bPendingCleared ? 1 : 0,
		bLoadedOk ? 1 : 0,
		bMetaOk ? 1 : 0,
		*ExpectedMapName,
		LoadedSave ? *LoadedSave->MapName : TEXT("<null>"),
		*MetaMap,
		*MetaUtc);

	T66_RequestSaveIntegrityExit(GameInstance, bPass ? ExitCode : 66, TEXT("T66SaveIntegrityHarness"));
	return bPass;
}

bool UT66SaveSubsystem::RunSaveIntegrityVerificationHarness(const int32 SlotIndex, const FString& Marker, const int32 ExitCode)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (SlotIndex < 0 || SlotIndex >= MaxSlots || Marker.IsEmpty())
	{
		UE_LOG(LogT66Save, Error, TEXT("[SaveIntegrityReload] FAIL Slot=%d Marker=%s Reason=InvalidArguments"),
			SlotIndex,
			*Marker);
		T66_RequestSaveIntegrityExit(GameInstance, 66, TEXT("T66SaveIntegrityVerifier"));
		return false;
	}

	const FString ExpectedMapName = T66_BuildSaveIntegrityMapName(Marker);
	UT66RunSaveGame* LoadedSave = LoadFromSlot(SlotIndex);

	bool bMetaOccupied = false;
	FString MetaUtc;
	FString MetaHero;
	FString MetaMap;
	GetSlotMeta(SlotIndex, bMetaOccupied, MetaUtc, MetaHero, MetaMap);

	const bool bSlotExists = DoesSlotExist(SlotIndex);
	const bool bLoadedOk = LoadedSave
		&& LoadedSave->MapName == ExpectedMapName
		&& LoadedSave->OwnerPlayerId == T66SaveIntegrityOwnerId
		&& LoadedSave->OwnerDisplayName.Contains(Marker);
	const bool bMetaOk = bMetaOccupied && MetaMap == ExpectedMapName;
	const bool bPass = bSlotExists && bLoadedOk && bMetaOk;

	UE_LOG(LogT66Save, Log, TEXT("[SaveIntegrityReload] %s Slot=%d Marker=%s SlotExists=%d LoadedOk=%d MetaOk=%d ExpectedMap=%s LoadedMap=%s MetaMap=%s MetaUtc=%s"),
		bPass ? TEXT("PASS") : TEXT("FAIL"),
		SlotIndex,
		*Marker,
		bSlotExists ? 1 : 0,
		bLoadedOk ? 1 : 0,
		bMetaOk ? 1 : 0,
		*ExpectedMapName,
		LoadedSave ? *LoadedSave->MapName : TEXT("<null>"),
		*MetaMap,
		*MetaUtc);

	T66_RequestSaveIntegrityExit(GameInstance, bPass ? ExitCode : 66, TEXT("T66SaveIntegrityVerifier"));
	return bPass;
}
#endif
