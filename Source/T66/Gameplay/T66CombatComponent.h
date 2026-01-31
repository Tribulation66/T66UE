// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "T66CombatComponent.generated.h"

class AT66EnemyBase;
class AT66BossBase;
class AT66GamblerBoss;
class UT66RunStateSubsystem;
class USoundBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class T66_API UT66CombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66CombatComponent();

	/** Manual attack lock target (affects auto attacks only). */
	void SetLockedTarget(AActor* InTarget);
	void ClearLockedTarget();
	AActor* GetLockedTarget() const { return LockedTarget.Get(); }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackRange = 4000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float FireIntervalSeconds = 1.f;

	/** Damage per shot (use high value for insta kill) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 DamagePerShot = 20;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleInventoryChanged();

	void TryFire();
	void RecomputeFromRunState();

	FTimerHandle FireTimerHandle;

	TWeakObjectPtr<AActor> LockedTarget;

	UPROPERTY()
	TObjectPtr<UT66RunStateSubsystem> CachedRunState;

	// Auto-attack sound effect (optional).
	UPROPERTY()
	TSoftObjectPtr<USoundBase> ShotSfx;

	bool bShotSfxWarnedMissing = false;

	void PlayShotSfx();

	float BaseAttackRange = 0.f;
	float BaseFireIntervalSeconds = 0.f;
	int32 BaseDamagePerShot = 0;

	float EffectiveFireIntervalSeconds = 1.f;
	int32 EffectiveDamagePerShot = 20;
	float ProjectileScaleMultiplier = 1.f;
};
