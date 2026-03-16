// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66ActorRegistrySubsystem.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66StageGate.h"
#include "Gameplay/T66MiasmaBoundary.h"

namespace
{
	template <typename T>
	void AddUniqueWeak(TArray<TWeakObjectPtr<T>>& Entries, T* Actor)
	{
		if (!Actor)
		{
			return;
		}

		Entries.RemoveAllSwap([](const TWeakObjectPtr<T>& Weak) { return !Weak.IsValid(); }, EAllowShrinking::No);
		Entries.AddUnique(Actor);
	}

	template <typename T>
	void RemoveWeak(TArray<TWeakObjectPtr<T>>& Entries, T* Actor)
	{
		if (!Actor)
		{
			return;
		}

		Entries.RemoveAllSwap([Actor](const TWeakObjectPtr<T>& Weak)
		{
			return !Weak.IsValid() || Weak.Get() == Actor;
		}, EAllowShrinking::No);
	}
}

// --------------- Enemies ---------------

void UT66ActorRegistrySubsystem::RegisterEnemy(AT66EnemyBase* Enemy)
{
	if (!Enemy) return;
	AddUniqueWeak(Enemies, Enemy);
	UE_LOG(LogTemp, Verbose, TEXT("[GOLD] ActorRegistry: registered enemy %s (total: %d)"), *Enemy->GetName(), Enemies.Num());
}

void UT66ActorRegistrySubsystem::UnregisterEnemy(AT66EnemyBase* Enemy)
{
	if (!Enemy) return;
	RemoveWeak(Enemies, Enemy);
	UE_LOG(LogTemp, Verbose, TEXT("[GOLD] ActorRegistry: unregistered enemy %s (total: %d)"), *Enemy->GetName(), Enemies.Num());
}

// --------------- Bosses ---------------

void UT66ActorRegistrySubsystem::RegisterBoss(AT66BossBase* Boss)
{
	if (!Boss) return;
	AddUniqueWeak(Bosses, Boss);
	UE_LOG(LogTemp, Verbose, TEXT("[GOLD] ActorRegistry: registered boss %s (total: %d)"), *Boss->GetName(), Bosses.Num());
}

void UT66ActorRegistrySubsystem::UnregisterBoss(AT66BossBase* Boss)
{
	if (!Boss) return;
	RemoveWeak(Bosses, Boss);
	UE_LOG(LogTemp, Verbose, TEXT("[GOLD] ActorRegistry: unregistered boss %s (total: %d)"), *Boss->GetName(), Bosses.Num());
}

// --------------- House NPCs ---------------

void UT66ActorRegistrySubsystem::RegisterNPC(AT66HouseNPCBase* NPC)
{
	if (!NPC) return;
	AddUniqueWeak(NPCs, NPC);
	UE_LOG(LogTemp, Log, TEXT("[GOLD] ActorRegistry: registered NPC %s (total: %d)"), *NPC->GetName(), NPCs.Num());
}

void UT66ActorRegistrySubsystem::UnregisterNPC(AT66HouseNPCBase* NPC)
{
	if (!NPC) return;
	RemoveWeak(NPCs, NPC);
	UE_LOG(LogTemp, Log, TEXT("[GOLD] ActorRegistry: unregistered NPC %s (total: %d)"), *NPC->GetName(), NPCs.Num());
}

// --------------- Stage Gates ---------------

void UT66ActorRegistrySubsystem::RegisterStageGate(AT66StageGate* Gate)
{
	if (!Gate) return;
	AddUniqueWeak(StageGates, Gate);
	UE_LOG(LogTemp, Log, TEXT("[GOLD] ActorRegistry: registered StageGate %s (total: %d)"), *Gate->GetName(), StageGates.Num());
}

void UT66ActorRegistrySubsystem::UnregisterStageGate(AT66StageGate* Gate)
{
	if (!Gate) return;
	RemoveWeak(StageGates, Gate);
	UE_LOG(LogTemp, Log, TEXT("[GOLD] ActorRegistry: unregistered StageGate %s (total: %d)"), *Gate->GetName(), StageGates.Num());
}

// --------------- Miasma Boundaries ---------------

void UT66ActorRegistrySubsystem::RegisterMiasmaBoundary(AT66MiasmaBoundary* Boundary)
{
	if (!Boundary) return;
	AddUniqueWeak(MiasmaBoundaries, Boundary);
	UE_LOG(LogTemp, Log, TEXT("[GOLD] ActorRegistry: registered MiasmaBoundary %s (total: %d)"), *Boundary->GetName(), MiasmaBoundaries.Num());
}

void UT66ActorRegistrySubsystem::UnregisterMiasmaBoundary(AT66MiasmaBoundary* Boundary)
{
	if (!Boundary) return;
	RemoveWeak(MiasmaBoundaries, Boundary);
	UE_LOG(LogTemp, Log, TEXT("[GOLD] ActorRegistry: unregistered MiasmaBoundary %s (total: %d)"), *Boundary->GetName(), MiasmaBoundaries.Num());
}
