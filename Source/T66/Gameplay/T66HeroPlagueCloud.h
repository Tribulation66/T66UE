// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66HeroPlagueCloud.generated.h"

class USphereComponent;
class UNiagaraSystem;

/**
 * Persistent poison cloud (Rogue ultimate). Spawned at hero location;
 * enemies inside take DOT every TickIntervalSeconds for DurationSeconds.
 * Visual: swirl of green pixel particles instead of a mesh cylinder.
 */
UCLASS(Blueprintable)
class T66_API AT66HeroPlagueCloud : public AActor
{
	GENERATED_BODY()

public:
	AT66HeroPlagueCloud();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlagueCloud")
	float Radius = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlagueCloud")
	float DurationSeconds = 5.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlagueCloud")
	float TickIntervalSeconds = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlagueCloud")
	int32 TotalDamagePerTarget = 200;

	void InitFromUltimate(int32 UltimateDamage);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> DamageZone;

	void ApplyTickDamage();
	void DestroySelf();

	FTimerHandle TickTimerHandle;
	FTimerHandle DestroyTimerHandle;
	float DamagePerTick = 0.f;
	int32 NumTicks = 0;

	float VFXAccum = 0.f;
	UNiagaraSystem* CachedPixelVFX = nullptr;
};
