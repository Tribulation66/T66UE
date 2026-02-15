// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66ActorRegistrySubsystem.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66StageGate.h"
#include "Gameplay/T66MiasmaBoundary.h"

// --------------- Enemies ---------------

void UT66ActorRegistrySubsystem::RegisterEnemy(AT66EnemyBase* Enemy)
{
	if (!Enemy) return;
	Enemies.Add(Enemy);
	UE_LOG(LogTemp, Verbose, TEXT("[GOLD] ActorRegistry: registered enemy %s (total: %d)"), *Enemy->GetName(), Enemies.Num());
}

void UT66ActorRegistrySubsystem::UnregisterEnemy(AT66EnemyBase* Enemy)
{
	if (!Enemy) return;
	Enemies.RemoveAllSwap([Enemy](const TWeakObjectPtr<AT66EnemyBase>& W) { return W.Get() == Enemy; }, EAllowShrinking::No);
	UE_LOG(LogTemp, Verbose, TEXT("[GOLD] ActorRegistry: unregistered enemy %s (total: %d)"), *Enemy->GetName(), Enemies.Num());
}

// --------------- House NPCs ---------------

void UT66ActorRegistrySubsystem::RegisterNPC(AT66HouseNPCBase* NPC)
{
	if (!NPC) return;
	NPCs.Add(NPC);
	UE_LOG(LogTemp, Log, TEXT("[GOLD] ActorRegistry: registered NPC %s (total: %d)"), *NPC->GetName(), NPCs.Num());
}

void UT66ActorRegistrySubsystem::UnregisterNPC(AT66HouseNPCBase* NPC)
{
	if (!NPC) return;
	NPCs.RemoveAllSwap([NPC](const TWeakObjectPtr<AT66HouseNPCBase>& W) { return W.Get() == NPC; }, EAllowShrinking::No);
	UE_LOG(LogTemp, Log, TEXT("[GOLD] ActorRegistry: unregistered NPC %s (total: %d)"), *NPC->GetName(), NPCs.Num());
}

// --------------- Stage Gates ---------------

void UT66ActorRegistrySubsystem::RegisterStageGate(AT66StageGate* Gate)
{
	if (!Gate) return;
	StageGates.Add(Gate);
	UE_LOG(LogTemp, Log, TEXT("[GOLD] ActorRegistry: registered StageGate %s (total: %d)"), *Gate->GetName(), StageGates.Num());
}

void UT66ActorRegistrySubsystem::UnregisterStageGate(AT66StageGate* Gate)
{
	if (!Gate) return;
	StageGates.RemoveAllSwap([Gate](const TWeakObjectPtr<AT66StageGate>& W) { return W.Get() == Gate; }, EAllowShrinking::No);
	UE_LOG(LogTemp, Log, TEXT("[GOLD] ActorRegistry: unregistered StageGate %s (total: %d)"), *Gate->GetName(), StageGates.Num());
}

// --------------- Miasma Boundaries ---------------

void UT66ActorRegistrySubsystem::RegisterMiasmaBoundary(AT66MiasmaBoundary* Boundary)
{
	if (!Boundary) return;
	MiasmaBoundaries.Add(Boundary);
	UE_LOG(LogTemp, Log, TEXT("[GOLD] ActorRegistry: registered MiasmaBoundary %s (total: %d)"), *Boundary->GetName(), MiasmaBoundaries.Num());
}

void UT66ActorRegistrySubsystem::UnregisterMiasmaBoundary(AT66MiasmaBoundary* Boundary)
{
	if (!Boundary) return;
	MiasmaBoundaries.RemoveAllSwap([Boundary](const TWeakObjectPtr<AT66MiasmaBoundary>& W) { return W.Get() == Boundary; }, EAllowShrinking::No);
	UE_LOG(LogTemp, Log, TEXT("[GOLD] ActorRegistry: unregistered MiasmaBoundary %s (total: %d)"), *Boundary->GetName(), MiasmaBoundaries.Num());
}
