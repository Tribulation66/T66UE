// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MiniVFXSubsystem.h"

#include "Core/T66AudioSubsystem.h"
#include "Engine/Texture.h"
#include "VFX/T66MiniFlipbookVFXActor.h"
#include "VFX/T66MiniGroundTelegraphActor.h"

AT66MiniGroundTelegraphActor* UT66MiniVFXSubsystem::AcquireGroundTelegraph(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	for (AT66MiniGroundTelegraphActor* Telegraph : GroundTelegraphPool)
	{
		if (Telegraph && Telegraph->IsAvailableForReuse() && Telegraph->GetWorld() == World)
		{
			return Telegraph;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66MiniGroundTelegraphActor* Telegraph = World->SpawnActor<AT66MiniGroundTelegraphActor>(AT66MiniGroundTelegraphActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams))
	{
		GroundTelegraphPool.Add(Telegraph);
		return Telegraph;
	}

	return nullptr;
}

AT66MiniFlipbookVFXActor* UT66MiniVFXSubsystem::AcquirePulseActor(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	for (AT66MiniFlipbookVFXActor* Pulse : PulseActorPool)
	{
		if (Pulse && Pulse->IsAvailableForReuse() && Pulse->GetWorld() == World)
		{
			return Pulse;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66MiniFlipbookVFXActor* Pulse = World->SpawnActor<AT66MiniFlipbookVFXActor>(AT66MiniFlipbookVFXActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams))
	{
		PulseActorPool.Add(Pulse);
		return Pulse;
	}

	return nullptr;
}

AT66MiniGroundTelegraphActor* UT66MiniVFXSubsystem::SpawnGroundTelegraph(
	UWorld* World,
	const FVector& Location,
	const float Radius,
	const float LifetimeSeconds,
	const FLinearColor& Tint)
{
	if (!World)
	{
		return nullptr;
	}

	if (AT66MiniGroundTelegraphActor* Telegraph = AcquireGroundTelegraph(World))
	{
		Telegraph->InitializeTelegraph(Location, Radius, LifetimeSeconds, Tint);
		return Telegraph;
	}

	return nullptr;
}

AT66MiniFlipbookVFXActor* UT66MiniVFXSubsystem::SpawnPulse(
	UWorld* World,
	const FVector& Location,
	const FVector& Scale,
	const float LifetimeSeconds,
	const FLinearColor& Tint,
	const float GrowthFactor)
{
	if (!World)
	{
		return nullptr;
	}

	if (AT66MiniFlipbookVFXActor* Pulse = AcquirePulseActor(World))
	{
		Pulse->InitializeVfx(Location, Scale, LifetimeSeconds, Tint, nullptr, GrowthFactor);
		return Pulse;
	}

	return nullptr;
}

AT66MiniFlipbookVFXActor* UT66MiniVFXSubsystem::SpawnSpritePulse(
	UWorld* World,
	const FVector& Location,
	const FVector& Scale,
	const float LifetimeSeconds,
	const FLinearColor& Tint,
	UTexture* Texture,
	const float GrowthFactor)
{
	if (!World)
	{
		return nullptr;
	}

	if (AT66MiniFlipbookVFXActor* Pulse = AcquirePulseActor(World))
	{
		Pulse->InitializeVfx(Location, Scale, LifetimeSeconds, Tint, Texture, GrowthFactor);
		return Pulse;
	}

	return nullptr;
}

void UT66MiniVFXSubsystem::PlayShotSfx(UObject* WorldContext, const float Volume, const float Pitch)
{
	(void)Volume;
	(void)Pitch;
	UT66AudioSubsystem::PlayEventFromWorldContext(WorldContext, FName(TEXT("Hero.Attack.Generic")));
}

void UT66MiniVFXSubsystem::PlayHitSfx(UObject* WorldContext, const float Volume, const float Pitch)
{
	(void)Volume;
	(void)Pitch;
	UT66AudioSubsystem::PlayEventFromWorldContext(WorldContext, FName(TEXT("Combat.Hit.Enemy")));
}

void UT66MiniVFXSubsystem::PlayPickupSfx(UObject* WorldContext, const float Volume, const float Pitch)
{
	(void)Volume;
	(void)Pitch;
	UT66AudioSubsystem::PlayEventFromWorldContext(WorldContext, FName(TEXT("Pickup.Item")));
}

void UT66MiniVFXSubsystem::PlayBossAlertSfx(UObject* WorldContext, const float Volume, const float Pitch)
{
	(void)Volume;
	(void)Pitch;
	UT66AudioSubsystem::PlayEventFromWorldContext(WorldContext, FName(TEXT("Boss.AOE.Warning")));
}

void UT66MiniVFXSubsystem::PlayBossSpawnSfx(UObject* WorldContext, const float Volume, const float Pitch)
{
	(void)Volume;
	(void)Pitch;
	UT66AudioSubsystem::PlayEventFromWorldContext(WorldContext, FName(TEXT("Combat.Ultimate.Cast")));
}
