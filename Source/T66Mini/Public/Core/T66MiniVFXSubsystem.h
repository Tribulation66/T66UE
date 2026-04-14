// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66MiniVFXSubsystem.generated.h"

class AT66MiniFlipbookVFXActor;
class AT66MiniGroundTelegraphActor;
class USoundBase;
class UTexture;

UCLASS()
class T66MINI_API UT66MiniVFXSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	AT66MiniGroundTelegraphActor* SpawnGroundTelegraph(UWorld* World, const FVector& Location, float Radius, float LifetimeSeconds, const FLinearColor& Tint);
	AT66MiniFlipbookVFXActor* SpawnPulse(UWorld* World, const FVector& Location, const FVector& Scale, float LifetimeSeconds, const FLinearColor& Tint, float GrowthFactor = 0.65f);
	AT66MiniFlipbookVFXActor* SpawnSpritePulse(UWorld* World, const FVector& Location, const FVector& Scale, float LifetimeSeconds, const FLinearColor& Tint, UTexture* Texture, float GrowthFactor = 0.65f);

	void PlayShotSfx(UObject* WorldContext, float Volume = 0.18f, float Pitch = 1.0f);
	void PlayHitSfx(UObject* WorldContext, float Volume = 0.12f, float Pitch = 1.26f);
	void PlayPickupSfx(UObject* WorldContext, float Volume = 0.16f, float Pitch = 1.42f);
	void PlayBossAlertSfx(UObject* WorldContext, float Volume = 0.22f, float Pitch = 0.82f);
	void PlayBossSpawnSfx(UObject* WorldContext, float Volume = 0.24f, float Pitch = 0.72f);

private:
	USoundBase* LoadMiniSfx();
};
