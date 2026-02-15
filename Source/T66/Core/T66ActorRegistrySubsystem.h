// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "T66ActorRegistrySubsystem.generated.h"

class AT66EnemyBase;
class AT66HouseNPCBase;
class AT66StageGate;
class AT66MiasmaBoundary;

/**
 * [GOLD] Actor Registry: actors register on BeginPlay and unregister on EndPlay.
 * Replaces TActorIterator-based world scans with O(1) list lookups.
 * Used by: RefreshMapData (HUD minimap), safe-zone checks, spawn validation, etc.
 */
UCLASS()
class T66_API UT66ActorRegistrySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// --------------- Enemies ---------------
	void RegisterEnemy(AT66EnemyBase* Enemy);
	void UnregisterEnemy(AT66EnemyBase* Enemy);
	const TArray<TWeakObjectPtr<AT66EnemyBase>>& GetEnemies() const { return Enemies; }

	// --------------- House NPCs ---------------
	void RegisterNPC(AT66HouseNPCBase* NPC);
	void UnregisterNPC(AT66HouseNPCBase* NPC);
	const TArray<TWeakObjectPtr<AT66HouseNPCBase>>& GetNPCs() const { return NPCs; }

	// --------------- Stage Gates ---------------
	void RegisterStageGate(AT66StageGate* Gate);
	void UnregisterStageGate(AT66StageGate* Gate);
	const TArray<TWeakObjectPtr<AT66StageGate>>& GetStageGates() const { return StageGates; }

	// --------------- Miasma Boundaries ---------------
	void RegisterMiasmaBoundary(AT66MiasmaBoundary* Boundary);
	void UnregisterMiasmaBoundary(AT66MiasmaBoundary* Boundary);
	const TArray<TWeakObjectPtr<AT66MiasmaBoundary>>& GetMiasmaBoundaries() const { return MiasmaBoundaries; }

private:
	TArray<TWeakObjectPtr<AT66EnemyBase>> Enemies;
	TArray<TWeakObjectPtr<AT66HouseNPCBase>> NPCs;
	TArray<TWeakObjectPtr<AT66StageGate>> StageGates;
	TArray<TWeakObjectPtr<AT66MiasmaBoundary>> MiasmaBoundaries;
};
