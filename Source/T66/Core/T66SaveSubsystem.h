// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Shutdown/T66ShutdownSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66SaveIndex.h"
#include "T66SaveSubsystem.generated.h"

class UT66RunSaveGame;

UCLASS()
class T66_API UT66SaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static const FString SaveIndexSlotName;
	static const FString SlotNamePrefix;
	static const int32 MaxSlots = 9;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Save")
	int32 FindFirstEmptySlot() const;

	/** If all slots are full, pick the oldest occupied slot (by LastPlayedUtc). */
	UFUNCTION(BlueprintCallable, Category = "Save")
	int32 FindOldestOccupiedSlot() const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool DoesSlotExist(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveToSlot(int32 SlotIndex, UT66RunSaveGame* SaveGameObject);

	UFUNCTION(BlueprintCallable, Category = "Save")
	UT66RunSaveGame* LoadFromSlot(int32 SlotIndex);

	void UpdateIndexOnSave(int32 SlotIndex, const FString& HeroDisplayName, const FString& MapName, const FString& LastPlayedUtc);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool GetSlotMeta(int32 SlotIndex, bool& bOutOccupied, FString& OutLastPlayedUtc, FString& OutHeroDisplayName, FString& OutMapName) const;

#if !UE_BUILD_SHIPPING
	bool RunQueuedSaveIntegrityShutdownHarness(int32 SlotIndex, const FString& Marker, int32 ExitCode);
	bool RunSaveIntegrityVerificationHarness(int32 SlotIndex, const FString& Marker, int32 ExitCode);
#endif

private:
	FString GetSlotName(int32 SlotIndex) const;
	UT66SaveIndex* LoadOrCreateIndex();
	UT66SaveIndex* LoadOrCreateIndex() const;
	bool SaveIndex(UT66SaveIndex* Index);
	void DeleteOutdatedRunSaves();
	bool HandleShutdown(const FT66ShutdownContext& Context);
	bool FlushPendingDurableState(const TCHAR* Reason);

	UPROPERTY(Transient)
	TObjectPtr<UT66SaveIndex> CachedSaveIndex;

	UPROPERTY(Transient)
	TObjectPtr<UT66RunSaveGame> PendingRunSaveObject;

	FString PendingRunSaveSlotName;
	int64 RunSaveAsyncSequence = 0;
	int64 PendingRunSaveSequence = 0;
	bool bSaveIndexFlushNeeded = false;
	int64 SaveIndexAsyncSequence = 0;
	int64 PendingSaveIndexSequence = 0;
	FT66ShutdownParticipantHandle ShutdownParticipantHandle;
};
