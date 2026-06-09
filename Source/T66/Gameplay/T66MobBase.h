// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Enemies/T66EnemyFamilyTypes.h"
#include "Gameplay/T66CombatTargetTypes.h"
#include "T66MobBase.generated.h"

class UCapsuleComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;
class UT66CombatHitZoneComponent;
class UWidgetComponent;
class UMaterialInstanceDynamic;
class AT66EnemyDirector;
class AT66EnemyProjectileBase;
class AT66HeroBase;

UENUM(BlueprintType)
enum class ET66MobLifecycleState : uint8
{
	Active UMETA(DisplayName = "Active"),
	Dying UMETA(DisplayName = "Dying"),
	Pooled UMETA(DisplayName = "Pooled"),
};

/**
 * Additive lightweight basic-mob foundation.
 *
 * This class intentionally does not spawn in Pass B.1. It mirrors the minimal
 * targeting/status surface that future passes will route through without
 * changing AT66EnemyBase or the current ACharacter enemy path.
 */
UCLASS(Blueprintable)
class T66_API AT66MobBase : public AActor
{
	GENERATED_BODY()

public:
	AT66MobBase();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|HitZones")
	TObjectPtr<UT66CombatHitZoneComponent> BodyHitZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|HitZones")
	TObjectPtr<UT66CombatHitZoneComponent> HeadHitZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> LockIndicatorWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mob")
	ET66EnemyFamily EnemyFamily = ET66EnemyFamily::Melee;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mob")
	FName MobID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mob")
	FName CharacterVisualID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mob")
	FName Archetype = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mob")
	bool bIsMiniBoss = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float CurrentHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float MaxHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 XPValue = 20;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float ChaseSpeed = 175.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector StoredVelocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Rush")
	bool bIsRushing = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Rush")
	FVector RushDirection = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Rush")
	float RushSecondsRemaining = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Rush")
	float RushCooldownRemaining = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Rush")
	float RushIntervalSeconds = 2.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Rush")
	float RushDurationSeconds = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Rush")
	float RushSpeedMultiplier = 3.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Rush")
	float RushTriggerDistance = 1700.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Rush")
	float InitialRushCooldownSeconds = 0.8f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Flying")
	float HoverAnchorZ = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Flying")
	float HoverBobTime = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Flying")
	float HoverHeight = 180.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Flying")
	float HoverBobFrequency = 2.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Flying")
	float HoverBobAmplitude = 35.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Ranged")
	float DesiredMinRange = 800.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Ranged")
	float DesiredMaxRange = 1400.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Ranged")
	float FireRangeGrace = 150.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Ranged")
	float ProjectileSpawnHeight = 80.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Ranged")
	float FireCooldownRemaining = 0.6f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Ranged")
	float FireCooldownDuration = 1.6f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Ranged")
	TSubclassOf<AT66EnemyProjectileBase> ProjectileClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float StunSecondsRemaining = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float StunDurationSeconds = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float RootSecondsRemaining = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float RootDurationSeconds = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float FreezeSecondsRemaining = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float FreezeDurationSeconds = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float SlowSecondsRemaining = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float SlowDurationSeconds = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float SlowMultiplier = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float SlowStrength = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float KnockbackSecondsRemaining = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float KnockbackDurationSeconds = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	FVector KnockbackVelocity = FVector::ZeroVector;

	// --- Physical knockback (ballistic 3D launch) ---
	// Mobs are AActor (not ACharacter) and the existing ApplyMobKnockback in
	// T66MobManagerSubsystem strips Z, so the legacy knockback can't lift them off the
	// ground. The physical-launch test (Hero_1 Slash, Idol_Fire_Pierce) writes a full
	// 3D velocity here; the mob manager ticks it as a simple ballistic (constant
	// gravity) until the mob lands back at PhysicalLaunchRestZ. While bPhysicalLaunchActive
	// is true the legacy KnockbackVelocity path is skipped so the two never fight.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	bool bPhysicalLaunchActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	FVector PhysicalLaunchVelocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float PhysicalLaunchSecondsRemaining = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float PhysicalLaunchRestZ = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	bool bIsLockedOn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lifecycle")
	ET66MobLifecycleState LifecycleState = ET66MobLifecycleState::Pooled;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|HitZones")
	float BodyDamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|HitZones")
	float HeadDamageMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 TouchDamageHearts = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float TouchDamageCooldownSeconds = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsTouchingHero = false;

	UPROPERTY(BlueprintReadWrite, Category = "AI")
	TObjectPtr<AT66EnemyDirector> OwningDirector;

	float GetCurrentHP() const { return CurrentHP; }
	float GetMaxHP() const { return MaxHP; }
	ET66EnemyFamily GetEnemyFamily() const { return EnemyFamily; }
	FName GetMobID() const { return MobID; }
	UCapsuleComponent* GetCollisionCapsule() const { return CapsuleComponent.Get(); }
	bool IsAliveAndActive() const { return CurrentHP > 0.f && LifecycleState == ET66MobLifecycleState::Active; }
	bool SupportsCombatHitZones() const { return BodyHitZone && HeadHitZone; }
	float GetHitZoneDamageMultiplier(ET66HitZoneType HitZoneType) const;
	FT66CombatTargetHandle ResolveCombatTargetHandle(const UPrimitiveComponent* HitComponent = nullptr, ET66HitZoneType PreferredZone = ET66HitZoneType::Body) const;

	void ConfigureAsMob(
		FName InMobID,
		ET66EnemyFamily InFamily = ET66EnemyFamily::Melee,
		FName InArchetype = NAME_None,
		int32 StageNum = 1,
		float DifficultyScalar = 1.f,
		float EnemyProgressionScalar = 1.f,
		float FinaleScalar = 1.f,
		bool bInIsMiniBoss = false);

	bool TakeDamageFromHeroHitZone(int32 Damage, const FT66CombatTargetHandle& TargetHandle, FName DamageSourceID = NAME_None, FName EventType = NAME_None);

	void ApplyStun(float DurationSeconds);
	void ApplyRoot(float DurationSeconds);
	void ApplyFreeze(float DurationSeconds);
	void ApplySlow(float SpeedMultiplier, float DurationSeconds);
	void ApplyMoveSlow(float SpeedMultiplier, float DurationSeconds);
	void ApplyAutoAttackKnockback(const FVector& HitOrigin, float StrengthScale = 1.f);

	/**
	 * Physical launch knockback (3D vector with Z, ballistic). Used by the physical-knockback
	 * test (Hero_1 Slash, Idol_Fire_Pierce). The mob manager integrates the velocity with
	 * constant gravity and lands the mob back at its current Z. While the launch is active,
	 * the legacy planar KnockbackVelocity path is skipped. Magnitude is clamped to
	 * PhysicalKnockbackMaxLaunchSpeed.
	 */
	void ApplyPhysicalKnockback(const FVector& LaunchVelocity);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0"))
	float PhysicalKnockbackMaxLaunchSpeed = 2500.f;

	/** Hard cap on airborne time so a launched mob can't hang indefinitely if it overshoots ground. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.1"))
	float PhysicalKnockbackMaxAirborneSeconds = 1.5f;

	void ApplyPullTowards(const FVector& PullOrigin, float Distance);
	void ApplyPushAwayFrom(const FVector& PushOrigin, float Distance);

	void ShowLockIndicator();
	void HideLockIndicator();

	void FireProjectile();
	bool TryFireProjectileAtHero(const AT66HeroBase* Hero);

	void ResetForReuse();

#if !UE_BUILD_SHIPPING
	void ForceMobVertexAnimationClipForAutomation(FName ClipName, float OverrideSeconds = 30.f);
#endif

private:
	void ApplyConfiguredVisual();
	bool TryApplyMobVertexAnimationVisual();
	void SetMobVertexAnimationClip(FName ClipName, float OverrideSeconds = 0.f);
	bool IsUsingMobVertexAnimation() const;
	bool HasProjectileLineOfSightToHero(const AT66HeroBase* Hero, const FVector& Start, const FVector& End, FString& OutBlockerName, const AActor*& OutBlockerActor, const UPrimitiveComponent*& OutBlockerComponent) const;
	void NotifyOwningDirectorOfDeath();
};
