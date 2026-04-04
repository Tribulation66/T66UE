// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Core/T66FloatingCombatTextPoolSubsystem.h"
#include "Gameplay/T66FloatingCombatTextActor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

const FName UT66FloatingCombatTextSubsystem::EventType_Crit(TEXT("Crit"));
const FName UT66FloatingCombatTextSubsystem::EventType_DoT(TEXT("DoT"));
const FName UT66FloatingCombatTextSubsystem::EventType_Burn(TEXT("Burn"));
const FName UT66FloatingCombatTextSubsystem::EventType_Chill(TEXT("Chill"));
const FName UT66FloatingCombatTextSubsystem::EventType_Curse(TEXT("Curse"));
const FName UT66FloatingCombatTextSubsystem::EventType_LevelUp(TEXT("LevelUp"));
const FName UT66FloatingCombatTextSubsystem::EventType_Taunt(TEXT("Taunt"));
const FName UT66FloatingCombatTextSubsystem::EventType_Confusion(TEXT("Confusion"));
const FName UT66FloatingCombatTextSubsystem::EventType_Invisibility(TEXT("Invisibility"));
const FName UT66FloatingCombatTextSubsystem::EventType_Dodge(TEXT("Dodge"));
const FName UT66FloatingCombatTextSubsystem::EventType_LifeSteal(TEXT("LifeSteal"));
const FName UT66FloatingCombatTextSubsystem::EventType_Reflect(TEXT("Reflect"));
const FName UT66FloatingCombatTextSubsystem::EventType_Crush(TEXT("Crush"));
const FName UT66FloatingCombatTextSubsystem::EventType_CounterAttack(TEXT("CounterAttack"));
const FName UT66FloatingCombatTextSubsystem::EventType_CloseRange(TEXT("CloseRange"));
const FName UT66FloatingCombatTextSubsystem::EventType_LongRange(TEXT("LongRange"));
const FName UT66FloatingCombatTextSubsystem::EventType_DamageTaken(TEXT("DamageTaken"));

void UT66FloatingCombatTextSubsystem::ShowDamageNumber(AActor* Target, int32 Amount, FName EventType)
{
	if (!Target || Amount <= 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UT66FloatingCombatTextPoolSubsystem* Pool = World->GetSubsystem<UT66FloatingCombatTextPoolSubsystem>();
	if (!Pool) return;

	++DamageNumberSequence;
	const float Side = (((GetTypeHash(Target) + DamageNumberSequence) & 1u) == 0u) ? DamageNumberOffsetSide : -DamageNumberOffsetSide;
	AT66FloatingCombatTextActor* Actor = Pool->AcquireActor(Target, FVector(Side, 0.f, OffsetAboveHead), TextLifetimeSeconds);
	if (Actor)
	{
		Actor->SetDamageNumber(Amount, EventType);
	}
}

void UT66FloatingCombatTextSubsystem::ShowDamageTaken(AActor* Target, int32 Amount)
{
	if (!Target || Amount <= 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UT66FloatingCombatTextPoolSubsystem* Pool = World->GetSubsystem<UT66FloatingCombatTextPoolSubsystem>();
	if (!Pool) return;

	++DamageNumberSequence;
	const float Side = (((GetTypeHash(Target) + DamageNumberSequence) & 1u) == 0u) ? DamageNumberOffsetSide : -DamageNumberOffsetSide;
	AT66FloatingCombatTextActor* Actor = Pool->AcquireActor(Target, FVector(Side, 0.f, OffsetAboveHead), TextLifetimeSeconds);
	if (Actor)
	{
		Actor->SetDamageNumber(Amount, EventType_DamageTaken);
	}
}

void UT66FloatingCombatTextSubsystem::ShowStatusEvent(AActor* Target, FName EventType)
{
	// Per current UX direction, only numeric damage text should be visible.
	// Keep callers intact, but suppress standalone status-word popups globally.
	(void)Target;
	(void)EventType;
}
