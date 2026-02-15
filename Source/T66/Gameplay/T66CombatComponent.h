// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
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

	/** Cooldown progress 0..1 (0 = just fired, 1 = ready). For UI cooldown bar below hero. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	float GetAutoAttackCooldownProgress() const;

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

	// ---------------------------------------------------------------------------
	// Overlap-based target detection: a sphere component tracks enemies in range
	// so TryFire never needs to iterate all world actors.
	// ---------------------------------------------------------------------------

	/** Invisible sphere attached to the hero; generates overlap events for enemies entering/leaving attack range. */
	UPROPERTY(Transient)
	TObjectPtr<USphereComponent> RangeSphere;

	/** Live set of enemies/bosses currently overlapping the range sphere. Maintained by overlap callbacks. */
	TArray<TWeakObjectPtr<AActor>> EnemiesInRange;

	UFUNCTION()
	void OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Helper: find closest valid target in EnemiesInRange from a given location within MaxRangeSq.
	 *  Optionally exclude actors already in ExcludeSet (for bounce chains). */
	AActor* FindClosestEnemyInRange(const FVector& FromLocation, float MaxRangeSq,
		const TSet<AActor*>* ExcludeSet = nullptr) const;

	/** Returns true if an actor is a valid auto-attack target (alive enemy/boss). */
	static bool IsValidAutoTarget(AActor* A);

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

	/** Apply damage to a single actor. EventType for floating text. SourceID for damage log (NAME_None = AutoAttack, or IdolID for idol/DOT). */
	void ApplyDamageToActor(AActor* Target, int32 DamageAmount, FName EventType = NAME_None, FName SourceID = NAME_None);

	/** Spawn slash VFX (arc) at the given location. */
	void SpawnSlashVFX(const FVector& Location, float Radius, const FLinearColor& Color);

	/** Spawn pierce VFX (line) from Start to End. */
	void SpawnPierceVFX(const FVector& Start, const FVector& End, const FLinearColor& Color);

	/** Spawn bounce VFX (chain of segments between positions). */
	void SpawnBounceVFX(const TArray<FVector>& ChainPositions, const FLinearColor& Color);

	/** Spawn DOT VFX (persistent effect at target location; set lifespan to Duration). */
	void SpawnDOTVFX(const FVector& Location, float Duration, float Radius, const FLinearColor& Color);

	float BaseAttackRange = 0.f;
	float BaseFireIntervalSeconds = 0.f;
	int32 BaseDamagePerShot = 0;

	float EffectiveFireIntervalSeconds = 1.f;
	int32 EffectiveDamagePerShot = 20;
	float ProjectileScaleMultiplier = 1.f;

public:
	/** Current fire interval (seconds) after all multipliers. For UI cooldown timer text. */
	float GetEffectiveFireInterval() const { return EffectiveFireIntervalSeconds; }
protected:

	/** Time of last fire (for cooldown bar). */
	float LastFireTime = -9999.f;
};
