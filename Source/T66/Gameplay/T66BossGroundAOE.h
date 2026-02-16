// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66BossGroundAOE.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * Ground AOE attack spawned by a boss.
 * Phase 1 (warning): flat red disc on the ground for WarningDurationSeconds.
 * Phase 2 (activation): damage check + tall red pillar visual for PillarLingerSeconds.
 * Then self-destructs.
 */
UCLASS(Blueprintable)
class T66_API AT66BossGroundAOE : public AActor
{
	GENERATED_BODY()

public:
	AT66BossGroundAOE();

	/** Radius of the damage zone (world units). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOE")
	float Radius = 300.f;

	/** How long the warning disc is shown before damage fires. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOE")
	float WarningDurationSeconds = 1.2f;

	/** How long the impact pillar stays visible after damage fires. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOE")
	float PillarLingerSeconds = 0.8f;

	/** Flat HP damage dealt to the hero (stage-scaled by the boss before spawning). When bDamageEnemies is true, this is damage to enemies instead. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	int32 DamageHP = 25;

	/** If true, overlap and damage enemies/bosses; if false, damage the hero (boss AOE behavior). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	bool bDamageEnemies = false;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> DamageZone;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WarningDisc;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ImpactPillar;

	void ActivateDamage();
	void DestroySelf();

	FTimerHandle ActivateTimerHandle;
	FTimerHandle DestroyTimerHandle;
	bool bDamageActivated = false;

	float WarningElapsed = 0.f;
};
