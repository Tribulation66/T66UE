// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66DamageLogSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FDamageLogEntry
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FName SourceID = NAME_None;
	UPROPERTY(BlueprintReadOnly) int32 TotalDamage = 0;
};

/**
 * Run-scoped damage log: tracks how much damage was dealt by which source during the run.
 * Used by Run Summary to display "Damage by source" (e.g. Auto Attack, Ultimate).
 * Reset when a new run starts (same time as RunState).
 */
UCLASS()
class T66_API UT66DamageLogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Canonical source IDs for damage (use these when recording). */
	static const FName SourceID_AutoAttack;
	static const FName SourceID_Ultimate;

	/** Record damage dealt from a given source this run. */
	UFUNCTION(BlueprintCallable, Category = "DamageLog")
	void RecordDamageDealt(FName SourceID, int32 Amount);

	/** Get all sources and their total damage, sorted by total descending (highest first). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DamageLog")
	TArray<FDamageLogEntry> GetDamageBySourceSorted() const;

	/** Clear log for a new run. Call wherever RunState->ResetForNewRun() is called. */
	UFUNCTION(BlueprintCallable, Category = "DamageLog")
	void ResetForNewRun();

private:
	TMap<FName, int32> DamageBySource;
};
