// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66BossGroundAOE.generated.h"

class USphereComponent;
class UNiagaraSystem;

/**
 * Ground AOE attack spawned by a boss.
 * Phase 1 (warning): pulsing ring of pixel particles for WarningDurationSeconds.
 * Phase 2 (activation): damage check + tall column burst of pixel particles for PillarLingerSeconds.
 * Then self-destructs.
 */
UCLASS(Blueprintable)
class T66_API AT66BossGroundAOE : public AActor
{
	GENERATED_BODY()

public:
	AT66BossGroundAOE();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOE")
	float Radius = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOE")
	float WarningDurationSeconds = 1.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOE")
	float PillarLingerSeconds = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	int32 DamageHP = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	bool bDamageEnemies = false;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> DamageZone;

	void ActivateDamage();
	void DestroySelf();

	FTimerHandle ActivateTimerHandle;
	FTimerHandle DestroyTimerHandle;
	bool bDamageActivated = false;

	float WarningElapsed = 0.f;
	float VFXAccum = 0.f;
	UNiagaraSystem* CachedPixelVFX = nullptr;
};
