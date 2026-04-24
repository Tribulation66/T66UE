// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "T66ActorRegistrySubsystem.generated.h"

class AT66EnemyBase;
class AT66BossBase;
class AT66HouseNPCBase;
class AT66CasinoInteractable;
class AT66StageGate;
class AT66MiasmaBoundary;
class AT66WorldInteractableBase;
class AT66LootBagPickup;

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

	// --------------- Bosses ---------------
	void RegisterBoss(AT66BossBase* Boss);
	void UnregisterBoss(AT66BossBase* Boss);
	const TArray<TWeakObjectPtr<AT66BossBase>>& GetBosses() const { return Bosses; }

	// --------------- House NPCs ---------------
	void RegisterNPC(AT66HouseNPCBase* NPC);
	void UnregisterNPC(AT66HouseNPCBase* NPC);
	const TArray<TWeakObjectPtr<AT66HouseNPCBase>>& GetNPCs() const { return NPCs; }

	// --------------- Casino Safe Zones ---------------
	void RegisterCasino(AT66CasinoInteractable* Casino);
	void UnregisterCasino(AT66CasinoInteractable* Casino);
	const TArray<TWeakObjectPtr<AT66CasinoInteractable>>& GetCasinos() const { return Casinos; }

	// --------------- Stage Gates ---------------
	void RegisterStageGate(AT66StageGate* Gate);
	void UnregisterStageGate(AT66StageGate* Gate);
	const TArray<TWeakObjectPtr<AT66StageGate>>& GetStageGates() const { return StageGates; }

	// --------------- Miasma Boundaries ---------------
	void RegisterMiasmaBoundary(AT66MiasmaBoundary* Boundary);
	void UnregisterMiasmaBoundary(AT66MiasmaBoundary* Boundary);
	const TArray<TWeakObjectPtr<AT66MiasmaBoundary>>& GetMiasmaBoundaries() const { return MiasmaBoundaries; }

	// --------------- World Interactables ---------------
	void RegisterWorldInteractable(AT66WorldInteractableBase* Interactable);
	void UnregisterWorldInteractable(AT66WorldInteractableBase* Interactable);
	const TArray<TWeakObjectPtr<AT66WorldInteractableBase>>& GetWorldInteractables() const { return WorldInteractables; }

	// --------------- Loot Bags ---------------
	void RegisterLootBag(AT66LootBagPickup* LootBag);
	void UnregisterLootBag(AT66LootBagPickup* LootBag);
	const TArray<TWeakObjectPtr<AT66LootBagPickup>>& GetLootBags() const { return LootBags; }

private:
	TArray<TWeakObjectPtr<AT66EnemyBase>> Enemies;
	TArray<TWeakObjectPtr<AT66BossBase>> Bosses;
	TArray<TWeakObjectPtr<AT66HouseNPCBase>> NPCs;
	TArray<TWeakObjectPtr<AT66CasinoInteractable>> Casinos;
	TArray<TWeakObjectPtr<AT66StageGate>> StageGates;
	TArray<TWeakObjectPtr<AT66MiasmaBoundary>> MiasmaBoundaries;
	TArray<TWeakObjectPtr<AT66WorldInteractableBase>> WorldInteractables;
	TArray<TWeakObjectPtr<AT66LootBagPickup>> LootBags;
};
