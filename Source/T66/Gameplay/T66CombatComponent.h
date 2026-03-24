// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "Data/T66DataTypes.h"
#include "T66CombatComponent.generated.h"

class AT66EnemyBase;
class AT66BossBase;
class UT66RunStateSubsystem;
class UT66FloatingCombatTextSubsystem;
class UT66IdolManagerSubsystem;
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
	void SetAutoAttackSuppressed(bool bSuppressed) { bSuppressAutoAttack = bSuppressed; }
	bool IsAutoAttackSuppressed() const { return bSuppressAutoAttack; }
	void PerformScopedPiercingShot(const FVector& Start, const FVector& End);

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

	/** Spawn death VFX: burst of red pixel particles at the given location. */
	void SpawnDeathVFX(const FVector& Location);

	/** Static: spawn a death burst at a world location (for enemies/bosses that don't own a CombatComponent). */
	static void SpawnDeathBurstAtLocation(UWorld* World, const FVector& Location, int32 NumParticles = 20, float BurstRadius = 80.f);

	void PerformUltimateSpearStorm(int32 UltimateDamage);
	void PerformUltimateChainLightning(int32 UltimateDamage);
	void PerformUltimatePrecisionStrike(int32 UltimateDamage);
	void PerformUltimateFanTheHammer(int32 UltimateDamage);
	void PerformUltimateDeadeye(int32 UltimateDamage);
	void PerformUltimateDischarge(int32 UltimateDamage);
	void PerformUltimateJuiced();
	void PerformUltimateDeathSpiral(int32 UltimateDamage);
	void PerformUltimateShockwave(int32 UltimateDamage);
	void PerformUltimateTidalWave(int32 UltimateDamage);
	void PerformUltimateGoldRush(int32 UltimateDamage);
	void PerformUltimateMiasmaBomb(int32 UltimateDamage);
	void PerformUltimateRabidFrenzy();
	void PerformUltimateBlizzard(int32 UltimateDamage);

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

	UPROPERTY(Transient)
	TObjectPtr<UT66FloatingCombatTextSubsystem> CachedFloatingCombatText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UT66IdolManagerSubsystem> CachedIdolManager = nullptr;

	bool bHasCachedHeroData = false;
	FHeroData CachedHeroData;

	struct FCachedIdolSlot
	{
		FName IdolID = NAME_None;
		FIdolData IdolData;
		ET66ItemRarity Rarity = ET66ItemRarity::Black;
		bool bValid = false;
	};

	TArray<FCachedIdolSlot> CachedIdolSlots;

	// Auto-attack sound effect (optional).
	UPROPERTY()
	TSoftObjectPtr<USoundBase> ShotSfx;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> CachedShotSfx = nullptr;

	bool bShotSfxWarnedMissing = false;

	/** Legacy Niagara system (round ball). Asset: /Game/VFX/VFX_Attack1 */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|VFX")
	TSoftObjectPtr<UNiagaraSystem> SlashVFXNiagara;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> CachedSlashVFXNiagara = nullptr;

	/** Retro pixel Niagara system (square particle). Asset: /Game/VFX/NS_PixelParticle */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|VFX")
	TSoftObjectPtr<UNiagaraSystem> PixelVFXNiagara;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> CachedPixelVFXNiagara = nullptr;

	/** Returns the active VFX system: pixel if available, otherwise legacy. */
	UNiagaraSystem* GetActiveVFXSystem() const;

	void PlayShotSfx();

	/** Apply damage to a single actor. EventType for floating text. SourceID for damage log (NAME_None = AutoAttack, or IdolID for idol/DOT). */
	void ApplyDamageToActor(AActor* Target, int32 DamageAmount, FName EventType = NAME_None, FName SourceID = NAME_None, FName RangeEventForHero = NAME_None);

	void SpawnSlashVFX(const FVector& Location, float Radius, const FLinearColor& Color);
	void SpawnPierceVFX(const FVector& Start, const FVector& End, const FLinearColor& Color);
	void SpawnHeroOnePierceVFX(const FVector& Start, const FVector& End, const FVector& ImpactLocation);
	void SpawnBounceVFX(const TArray<FVector>& ChainPositions, const FLinearColor& Color);
	void SpawnDOTVFX(const FVector& Location, float Duration, float Radius, const FLinearColor& Color);
	void SpawnIdolPierceVFX(const FName& IdolID, ET66ItemRarity Rarity, const FVector& Start, const FVector& End, const FVector& ImpactLocation, float StartDelaySeconds);
	void SpawnIdolAOEVFX(const FName& IdolID, ET66ItemRarity Rarity, const FVector& Location, float Radius, float StartDelaySeconds);
	void SpawnIdolBounceVFX(const FName& IdolID, ET66ItemRarity Rarity, const TArray<FVector>& ChainPositions, float StartDelaySeconds);
	void SpawnIdolDOTVFX(const FName& IdolID, ET66ItemRarity Rarity, AActor* FollowTarget, const FVector& Location, float Duration, float Radius, float StartDelaySeconds);

	/** Hero-specific VFX variants: spawn unique pixel patterns based on HeroID. */
	void SpawnHeroSlashVFX(const FVector& Location, float Radius, const FLinearColor& Color, const FName& HeroID);
	void SpawnHeroPierceVFX(const FVector& Start, const FVector& End, const FLinearColor& Color, const FName& HeroID);
	void SpawnHeroBounceVFX(const TArray<FVector>& ChainPositions, const FLinearColor& Color, const FName& HeroID);
	void SpawnHeroDOTVFX(const FVector& Location, float Duration, float Radius, const FLinearColor& Color, const FName& HeroID);

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

	/** Marksman's Focus: consecutive hits on same target stack +8% damage (max 5 stacks). */
	TWeakObjectPtr<AActor> LastMarksmanTarget;
	int32 MarksmanStacks = 0;

	// Timed ultimate buffs
	float DeadeyeEndTime = -1.f;
	float JuicedEndTime = -1.f;
	int32 JuicedBonusBounce = 0;
	float RabidFrenzyEndTime = -1.f;
	bool bSuppressAutoAttack = false;
};
