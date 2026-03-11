// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66PropSubsystem.generated.h"

class UDataTable;

UCLASS()
class T66_API UT66PropSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Spawn all props for the current stage. Call after terrain is ready. */
	void SpawnPropsForStage(UWorld* World, int32 Seed);

	/** Destroy all spawned props (stage transition / map regeneration). */
	void ClearProps();

	int32 GetSpawnedPropCount() const { return SpawnedProps.Num(); }

private:
	UDataTable* GetPropsDataTable() const;

	UPROPERTY(Transient)
	mutable TObjectPtr<UDataTable> CachedPropsDataTable;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> SpawnedProps;
};
