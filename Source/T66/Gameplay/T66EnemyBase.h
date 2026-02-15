// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "T66EnemyBase.generated.h"

class UWidgetComponent;
class UStaticMeshComponent;
class AT66EnemyDirector;
class AT66ItemPickup;

UCLASS(Blueprintable)
class T66_API AT66EnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AT66EnemyBase();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 MaxHP = 100;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 CurrentHP = 100;

	/** Touch damage to player in hearts (scaled by difficulty). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 TouchDamageHearts = 1;

	/** Enemy armor: damage reduction fraction (0.0 = none, 0.5 = 50% reduction). Reduced by Taunt procs. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	float Armor = 0.f;

	/** True if this enemy is currently confused/wandering (Invisibility proc). */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bIsConfused = false;

	/** Remaining seconds of confusion effect. */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float ConfusionSecondsRemaining = 0.f;

	/** Apply a confusion effect (from hero Invisibility proc). */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyConfusion(float DurationSeconds);

	/** Apply an armor debuff (from hero Taunt proc). Reduces armor temporarily. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyArmorDebuff(float ReductionAmount, float DurationSeconds);

	/** Point value for wave budget and Bounty score (Bible 2.9) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 PointValue = 10;

	/** XP granted to the hero on death. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression")
	int32 XPValue = 20;

	/** If false, this enemy will not spawn a loot bag on death (used by mimics/special cases). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	bool bDropsLoot = true;

	/** Visible mesh (cylinder) so enemy is seen */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	/** Visual mapping ID used by UT66CharacterVisualSubsystem (data-driven imported mesh). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	FName CharacterVisualID = FName(TEXT("RegularEnemy"));

	/** Health bar widget above head */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HealthBarWidget;

	/** Director that spawned this enemy (for death notification) */
	UPROPERTY(BlueprintReadWrite, Category = "AI")
	TObjectPtr<AT66EnemyDirector> OwningDirector;

	/** Apply damage from hero. Returns true if enemy died. DamageSourceID used for run damage log (default: AutoAttack). EventType for floating text (Crit, DoT, etc.; default none). */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual bool TakeDamageFromHero(int32 Damage, FName DamageSourceID = NAME_None, FName EventType = NAME_None);

	/** If true, this enemy prefers to flee from the hero (used by Leprechaun). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	bool bRunAwayFromPlayer = false;

	/** Refresh health bar display (call when HP changes) */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealthBar();

	/** Show/hide the lock indicator on this enemy's health bar. */
	void SetLockedIndicator(bool bLocked);

	/** Apply stage-based HP/Armor: stage 1 = 50 HP, 0.1 armor; each stage multiplies by 1.1. Call before ApplyDifficultyScalar. */
	void ApplyStageScaling(int32 Stage);

	/** Apply difficulty scaling using a scalar (e.g. 1.1, 1.2, ...). HP/Armor are skipped if ApplyStageScaling was used. */
	void ApplyDifficultyScalar(float Scalar);

	/** Apply difficulty tier (Tier 0 = 1.0x, Tier 1 = 1.1x, Tier 2 = 1.2x, ...). */
	void ApplyDifficultyTier(int32 Tier);

	/** Stage mob ID (data-driven via DT_Stages EnemyA/B/C). NAME_None means "not a stage mob". */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mob")
	FName MobID;

	/** True if this instance is a mini-boss version of a stage mob. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mob")
	bool bIsMiniBoss = false;

	/** Configure placeholder visuals for a stage mob (shape + color). */
	UFUNCTION(BlueprintCallable, Category = "Mob")
	void ConfigureAsMob(FName InMobID);

	/** Apply mini-boss multipliers (call after difficulty scaling). */
	UFUNCTION(BlueprintCallable, Category = "Mob")
	void ApplyMiniBossMultipliers(float HPScalar, float DamageScalar, float ScaleScalar);

	/** [GOLD] Reset this enemy for reuse from the object pool. */
	void ResetForReuse(const FVector& NewLocation, AT66EnemyDirector* NewDirector);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	/** Called when HP reaches 0: notify director, spawn pickup, return to pool */
	virtual void OnDeath();

	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Touch damage: last time we dealt damage to player (cooldown) */
	float LastTouchDamageTime = -9999.f;
	static constexpr float TouchDamageCooldown = 0.5f;

private:
	// Safety/perf: avoid per-enemy per-frame scans for safe zones.
	float SafeZoneCheckAccumSeconds = 0.f;
	/** Safe-zone check runs every this many seconds (perf: was 0.25, then 0.5; 1.0 reduces N×M cost). */
	float SafeZoneCheckIntervalSeconds = 1.0f;
	bool bCachedInsideSafeZone = false;
	FVector CachedSafeZoneEscapeDir = FVector::ZeroVector;

	bool bBaseTuningInitialized = false;
	bool bStageScalingApplied = false;
	int32 BaseMaxHP = 0;
	int32 BaseTouchDamageHearts = 0;
	int32 BasePointValue = 0;
	float BaseArmor = 0.f;

	// Persist mini-boss multipliers so difficulty changes can re-apply cleanly.
	float MiniBossHPScalarApplied = 1.0f;
	float MiniBossDamageScalarApplied = 1.0f;
	float MiniBossScaleScalarApplied = 1.0f;

	// Armor debuff tracking
	float ArmorDebuffAmount = 0.f;
	float ArmorDebuffSecondsRemaining = 0.f;

	/** Cached player pawn reference (avoid UGameplayStatics::GetPlayerPawn every tick per enemy). */
	TWeakObjectPtr<APawn> CachedPlayerPawn;

	/** Confusion: cached wander direction, refreshed once per second instead of every frame. */
	FVector CachedWanderDir = FVector::ZeroVector;
	float WanderDirRefreshAccum = 0.f;
	static constexpr float WanderDirRefreshInterval = 1.0f;

protected:
	/** True if an imported skeletal mesh was applied and placeholders should be hidden. */
	UPROPERTY(Transient)
	bool bUsingCharacterVisual = false;
};
