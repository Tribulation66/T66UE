// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66BossPartTypes.h"
#include "T66BossBase.generated.h"

class UStaticMeshComponent;
class AT66BossGroundAOE;
class UT66CombatHitZoneComponent;
class UPrimitiveComponent;

USTRUCT()
struct FT66BossPartRuntimeState
{
	GENERATED_BODY()

	UPROPERTY()
	FName PartID = NAME_None;

	UPROPERTY()
	ET66HitZoneType HitZoneType = ET66HitZoneType::Body;

	UPROPERTY()
	float DamageMultiplier = 1.f;

	UPROPERTY()
	int32 MaxHP = 0;

	UPROPERTY()
	int32 CurrentHP = 0;

	UPROPERTY(Transient)
	TObjectPtr<UT66CombatHitZoneComponent> ZoneComponent = nullptr;
};

/** Boss: dormant until player proximity, then awakens, chases, and fires projectiles. */
UCLASS(Blueprintable)
class T66_API AT66BossBase : public ACharacter
{
	GENERATED_BODY()

public:
	AT66BossBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	FName BossID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	/** True once player gets close enough. */
	UPROPERTY(BlueprintReadOnly, Category = "Boss")
	bool bAwakened = false;

	/** Current HP (v0: starts at MaxHP once awakened). */
	UPROPERTY(BlueprintReadOnly, Category = "Boss")
	int32 CurrentHP = 0;

	/** Max HP (v0: 100). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	int32 MaxHP = 1000;

	/** Distance required to awaken. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	float AwakenDistance = 900.f;

	/** Fire interval once awakened. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	float FireIntervalSeconds = 2.0f;

	/** Projectile speed once awakened. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	float ProjectileSpeed = 900.f;

	/** Damage per boss projectile in hearts. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	int32 ProjectileDamageHearts = 1;

	/** Score awarded for defeating this boss (before difficulty scalar). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	int32 PointValue = 0;

	/** Armor (0–1): fraction of damage reduced. Can be debuffed below 0 for bonus damage. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	float Armor = 0.f;

	/** Interval between ground AOE attacks (0 = disabled). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|AOE")
	float GroundAOEIntervalSeconds = 5.f;

	/** Radius of each ground AOE zone. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|AOE")
	float GroundAOERadius = 300.f;

	/** Warning telegraph duration before AOE fires. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|AOE")
	float GroundAOEWarningSeconds = 1.2f;

	/** Base HP damage of each ground AOE hit (stage-scaled at spawn time). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|AOE")
	int32 GroundAOEBaseDamageHP = 25;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Parts")
	bool bUsesBossPartHitZones = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Parts")
	TArray<FT66BossPartDefinition> BossPartDefinitions;

	/** Apply an armor debuff (from hero Taunt). Reduces armor temporarily. */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void ApplyArmorDebuff(float ReductionAmount, float DurationSeconds);

	void ApplyConfusion(float DurationSeconds);
	void ApplyMoveSlow(float SpeedMultiplier, float DurationSeconds);
	void ApplyForcedRunAway(float DurationSeconds);
	void ApplyStun(float DurationSeconds);
	void ApplyRoot(float DurationSeconds);
	void ApplyFreeze(float DurationSeconds);
	void ApplyPullTowards(const FVector& PullOrigin, float Distance);
	void ApplyPushAwayFrom(const FVector& PushOrigin, float Distance);

	/** Initialize boss from data table (optional). */
	void InitializeBoss(const FBossData& BossData);

	/** Apply difficulty scaling using a scalar (e.g. 1.1, 1.2, ...). */
	void ApplyDifficultyScalar(float Scalar);

	/** Called by hero projectile overlap; returns true if boss died. DamageSourceID used for run damage log (default: AutoAttack). EventType for floating text (Crit, DoT; default none). */
	bool TakeDamageFromHeroHit(int32 DamageAmount, FName DamageSourceID = NAME_None, FName EventType = NAME_None);
	bool TakeDamageFromHeroHitZone(int32 DamageAmount, const FT66CombatTargetHandle& TargetHandle, FName DamageSourceID = NAME_None, FName EventType = NAME_None);
	float GetEffectiveArmor() const;
	bool SupportsCombatHitZones() const;
	FT66CombatTargetHandle ResolveCombatTargetHandle(const UPrimitiveComponent* HitComponent = nullptr, ET66HitZoneType PreferredZone = ET66HitZoneType::Body) const;
	FVector GetAimPointForHitZone(ET66HitZoneType HitZoneType) const;

	bool IsAwakened() const { return bAwakened; }
	bool IsAlive() const { return CurrentHP > 0; }

	/** Coliseum: start the fight immediately (bypasses proximity). */
	void ForceAwaken() { Awaken(); }

	int32 GetPointValue() const { return PointValue; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void Awaken();
	void FireAtPlayer();
	void SpawnGroundAOE();
	virtual void Die();

	FTimerHandle FireTimerHandle;
	FTimerHandle AOETimerHandle;

private:
	APawn* ResolvePlayerPawn();
	void AssignBossPartDefinitionsForProfile(ET66BossPartProfile InProfile);
	void EnsureDefaultBossPartDefinitions();
	void RefreshCombatHitZoneState();
	void RebuildBossPartState(bool bPreserveCurrentPercent);
	void BuildBossPartSnapshots(TArray<FT66BossPartSnapshot>& OutBossParts) const;
	bool RestoreBossPartStateFromRunState();
	void PushBossPartStateToRunState() const;
	int32 ResolveBossPartIndex(const UPrimitiveComponent* HitComponent, ET66HitZoneType PreferredZone, FName PreferredPartID = NAME_None) const;
	int32 FindFallbackBossPartIndex() const;
	float GetBossPartDamageMultiplier(int32 PartIndex) const;

	bool bBaseTuningInitialized = false;
	int32 BaseMaxHP = 0;
	int32 BaseProjectileDamageHearts = 0;
	float BaseMoveSpeed = 350.f;

	float ArmorDebuffAmount = 0.f;
	float ArmorDebuffSecondsRemaining = 0.f;
	float ConfusionSecondsRemaining = 0.f;
	float MoveSlowMultiplier = 1.f;
	float MoveSlowSecondsRemaining = 0.f;
	float ForcedRunAwaySecondsRemaining = 0.f;
	float StunSecondsRemaining = 0.f;
	float RootSecondsRemaining = 0.f;
	float FreezeSecondsRemaining = 0.f;
	FVector CachedWanderDir = FVector::ZeroVector;
	float WanderDirRefreshAccum = 0.f;
	TWeakObjectPtr<APawn> CachedPlayerPawn;

	UPROPERTY(Transient)
	TArray<FT66BossPartRuntimeState> BossPartStates;
};

