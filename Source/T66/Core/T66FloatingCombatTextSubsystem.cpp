// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66FloatingCombatTextSubsystem.h"
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

void UT66FloatingCombatTextSubsystem::ShowDamageNumber(AActor* Target, int32 Amount, FName EventType)
{
	if (!Target || Amount <= 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// Spawn at origin; attach to target so text follows them.
	AT66FloatingCombatTextActor* Actor = World->SpawnActor<AT66FloatingCombatTextActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	if (Actor)
	{
		Actor->AttachToActor(Target, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		const uint32 Seed = static_cast<uint32>(FMath::Abs(FDateTime::Now().GetTicks()) + GetTypeHash(Target));
		const float Side = (Seed % 2u == 0) ? DamageNumberOffsetSide : -DamageNumberOffsetSide;
		Actor->SetActorRelativeLocation(FVector(Side, 0.f, OffsetAboveHead));
		Actor->SetDamageNumber(Amount, EventType);
		Actor->SetLifeSpan(TextLifetimeSeconds);
	}

	if (!EventType.IsNone())
	{
		AT66FloatingCombatTextActor* StatusActor = World->SpawnActor<AT66FloatingCombatTextActor>(FVector::ZeroVector, FRotator::ZeroRotator);
		if (StatusActor)
		{
			StatusActor->AttachToActor(Target, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			StatusActor->SetActorRelativeLocation(FVector(0.f, 0.f, OffsetAboveHead + 40.f));
			StatusActor->SetStatusEvent(EventType);
			StatusActor->SetLifeSpan(TextLifetimeSeconds);
		}
	}
}

void UT66FloatingCombatTextSubsystem::ShowStatusEvent(AActor* Target, FName EventType)
{
	if (!Target || EventType.IsNone()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	AT66FloatingCombatTextActor* Actor = World->SpawnActor<AT66FloatingCombatTextActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	if (Actor)
	{
		Actor->AttachToActor(Target, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Actor->SetActorRelativeLocation(FVector(0.f, 0.f, OffsetAboveHead));
		Actor->SetStatusEvent(EventType);
		Actor->SetLifeSpan(TextLifetimeSeconds);
	}
}
