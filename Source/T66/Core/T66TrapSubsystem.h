// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66TrapTuningConfig.h"
#include "Subsystems/WorldSubsystem.h"
#include "Gameplay/T66TowerMapTerrain.h"
#include "T66TrapSubsystem.generated.h"

class AActor;
class AT66TrapBase;
enum class ET66Difficulty : uint8;

/**
 * World-scoped owner for environmental trap registration and procedural trap spawns.
 * Trap actors register themselves here so stage bootstrap and future trap systems
 * can reason about them without adding more GameMode-side actor scans.
 */
UCLASS()
class T66_API UT66TrapSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UT66TrapSubsystem* Get(UWorld* World);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	void RegisterTrap(AT66TrapBase* Trap);
	void UnregisterTrap(AT66TrapBase* Trap);

	const TArray<TWeakObjectPtr<AT66TrapBase>>& GetRegisteredTraps() const { return RegisteredTraps; }

	void ClearManagedTrapActors();
	bool AreTowerTrapSpawnsEnabled() const;
	void SetActiveTowerFloor(int32 ActiveFloorNumber);
	void SpawnTowerStageTraps(const T66TowerMapTerrain::FLayout& Layout, int32 StageNum, ET66Difficulty Difficulty, int32 RunSeed);
	void RefreshRegisteredTrapProgression();

private:
	void ApplyCurrentProgressionToTrap(AT66TrapBase* Trap) const;
	void PruneTrackedActors();

	UT66TrapTuningConfig CachedTuning;
	TArray<TWeakObjectPtr<AT66TrapBase>> RegisteredTraps;
	TArray<TWeakObjectPtr<AActor>> ManagedTrapActors;
	int32 ActiveTowerFloorNumber = INDEX_NONE;
};
