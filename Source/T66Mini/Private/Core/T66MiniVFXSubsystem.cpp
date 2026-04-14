// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MiniVFXSubsystem.h"

#include "Core/T66PlayerSettingsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/Texture.h"
#include "VFX/T66MiniFlipbookVFXActor.h"
#include "VFX/T66MiniGroundTelegraphActor.h"

namespace
{
	float T66MiniResolveEffectiveSfxVolume(UObject* WorldContext, const float BaseVolume)
	{
		const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
		const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		const UT66PlayerSettingsSubsystem* PlayerSettings = GameInstance ? GameInstance->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
		const float MasterVolume = PlayerSettings ? FMath::Clamp(PlayerSettings->GetMasterVolume(), 0.f, 1.f) : 1.f;
		const float SfxVolume = PlayerSettings ? FMath::Clamp(PlayerSettings->GetSfxVolume(), 0.f, 1.f) : 1.f;
		return BaseVolume * MasterVolume * SfxVolume;
	}
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

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66MiniGroundTelegraphActor* Telegraph = World->SpawnActor<AT66MiniGroundTelegraphActor>(AT66MiniGroundTelegraphActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams))
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

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66MiniFlipbookVFXActor* Pulse = World->SpawnActor<AT66MiniFlipbookVFXActor>(AT66MiniFlipbookVFXActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams))
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

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66MiniFlipbookVFXActor* Pulse = World->SpawnActor<AT66MiniFlipbookVFXActor>(AT66MiniFlipbookVFXActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams))
	{
		Pulse->InitializeVfx(Location, Scale, LifetimeSeconds, Tint, Texture, GrowthFactor);
		return Pulse;
	}

	return nullptr;
}

void UT66MiniVFXSubsystem::PlayShotSfx(UObject* WorldContext, const float Volume, const float Pitch)
{
	const float EffectiveVolume = T66MiniResolveEffectiveSfxVolume(WorldContext, Volume);
	if (EffectiveVolume <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	if (USoundBase* Sfx = LoadMiniSfx())
	{
		UGameplayStatics::PlaySound2D(WorldContext, Sfx, EffectiveVolume, Pitch);
	}
}

void UT66MiniVFXSubsystem::PlayHitSfx(UObject* WorldContext, const float Volume, const float Pitch)
{
	const float EffectiveVolume = T66MiniResolveEffectiveSfxVolume(WorldContext, Volume);
	if (EffectiveVolume <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	if (USoundBase* Sfx = LoadMiniSfx())
	{
		UGameplayStatics::PlaySound2D(WorldContext, Sfx, EffectiveVolume, Pitch);
	}
}

void UT66MiniVFXSubsystem::PlayPickupSfx(UObject* WorldContext, const float Volume, const float Pitch)
{
	const float EffectiveVolume = T66MiniResolveEffectiveSfxVolume(WorldContext, Volume);
	if (EffectiveVolume <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	if (USoundBase* Sfx = LoadMiniSfx())
	{
		UGameplayStatics::PlaySound2D(WorldContext, Sfx, EffectiveVolume, Pitch);
	}
}

void UT66MiniVFXSubsystem::PlayBossAlertSfx(UObject* WorldContext, const float Volume, const float Pitch)
{
	const float EffectiveVolume = T66MiniResolveEffectiveSfxVolume(WorldContext, Volume);
	if (EffectiveVolume <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	if (USoundBase* Sfx = LoadMiniSfx())
	{
		UGameplayStatics::PlaySound2D(WorldContext, Sfx, EffectiveVolume, Pitch);
	}
}

void UT66MiniVFXSubsystem::PlayBossSpawnSfx(UObject* WorldContext, const float Volume, const float Pitch)
{
	const float EffectiveVolume = T66MiniResolveEffectiveSfxVolume(WorldContext, Volume);
	if (EffectiveVolume <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	if (USoundBase* Sfx = LoadMiniSfx())
	{
		UGameplayStatics::PlaySound2D(WorldContext, Sfx, EffectiveVolume, Pitch);
	}
}

USoundBase* UT66MiniVFXSubsystem::LoadMiniSfx()
{
	static TWeakObjectPtr<USoundBase> Cached;
	if (!Cached.IsValid())
	{
		Cached = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/SFX/Shot.Shot"));
	}

	return Cached.Get();
}
