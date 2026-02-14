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

void UT66FloatingCombatTextSubsystem::ShowDamageNumber(AActor* Target, int32 Amount, FName EventType)
{
	if (!Target || Amount <= 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FVector TargetLoc = Target->GetActorLocation();
	// Offset to the side (alternate left/right based on hash so multiple hits don't stack in one spot).
	const uint32 Seed = static_cast<uint32>(FMath::Abs(FDateTime::Now().GetTicks()) + GetTypeHash(Target));
	const float Side = (Seed % 2u == 0) ? DamageNumberOffsetSide : -DamageNumberOffsetSide;
	const FVector SpawnLoc = TargetLoc + FVector(Side, 0.f, OffsetAboveHead);

	AT66FloatingCombatTextActor* Actor = World->SpawnActor<AT66FloatingCombatTextActor>(SpawnLoc, FRotator::ZeroRotator);
	if (Actor)
	{
		Actor->SetDamageNumber(Amount, EventType);
		Actor->SetLifeSpan(TextLifetimeSeconds);
	}

	if (!EventType.IsNone())
	{
		// Status label above head (slightly higher than damage number).
		const FVector StatusLoc = TargetLoc + FVector(0.f, 0.f, OffsetAboveHead + 40.f);
		AT66FloatingCombatTextActor* StatusActor = World->SpawnActor<AT66FloatingCombatTextActor>(StatusLoc, FRotator::ZeroRotator);
		if (StatusActor)
		{
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

	const FVector SpawnLoc = Target->GetActorLocation() + FVector(0.f, 0.f, OffsetAboveHead);
	AT66FloatingCombatTextActor* Actor = World->SpawnActor<AT66FloatingCombatTextActor>(SpawnLoc, FRotator::ZeroRotator);
	if (Actor)
	{
		Actor->SetStatusEvent(EventType);
		Actor->SetLifeSpan(TextLifetimeSeconds);
	}
}
