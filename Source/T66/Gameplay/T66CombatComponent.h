// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "T66CombatComponent.generated.h"

class AT66EnemyBase;
class AT66BossBase;
class AT66GamblerBoss;
class AT66VendorBoss;
class UT66RunStateSubsystem;
class USoundBase;
class UNiagaraSystem;

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
	float AttackRange = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float FireIntervalSeconds = 1.f;

	/** Damage per shot (use high value for insta kill) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 DamagePerShot = 20;

	/** Base AoE splash radius for the slash attack (scales with ProjectileScaleMultiplier). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float SlashRadius = 300.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> CachedShotSfx = nullptr;

	bool bShotSfxWarnedMissing = false;

	/** Niagara system for slash VFX (particles in arc). Asset: /Game/VFX/VFX_Attack1 */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|VFX")
	TSoftObjectPtr<UNiagaraSystem> SlashVFXNiagara;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> CachedSlashVFXNiagara = nullptr;

	void PlayShotSfx();

	/** Apply damage to a single actor (enemy or boss), dispatching to the correct TakeDamage method. */
	void ApplyDamageToActor(AActor* Target, int32 DamageAmount);

	/** Spawn the slash VFX disc at the given world location. */
	void SpawnSlashVFX(const FVector& Location, float Radius, const FLinearColor& Color);

	float BaseAttackRange = 0.f;
	float BaseFireIntervalSeconds = 0.f;
	int32 BaseDamagePerShot = 0;

	float EffectiveFireIntervalSeconds = 1.f;
	int32 EffectiveDamagePerShot = 20;
	float ProjectileScaleMultiplier = 1.f;
};
