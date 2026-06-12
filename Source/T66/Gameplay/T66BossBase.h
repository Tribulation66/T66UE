// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66BossAttackTypes.h"
#include "Gameplay/T66BossPartTypes.h"
#include "TimerManager.h"
#include "T66BossBase.generated.h"

class UStaticMeshComponent;
class AT66BossAttackTelegraph;
class AT66BossGroundAOE;
class AT66BossLaneBlockerHazard;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attacks")
	ET66BossAttackProfile AttackProfile = ET66BossAttackProfile::Balanced;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attacks")
	FLinearColor AttackPrimaryColor = FLinearColor(0.95f, 0.16f, 0.12f, 1.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attacks")
	FLinearColor AttackSecondaryColor = FLinearColor(1.f, 0.72f, 0.18f, 1.f);

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

	void ApplyEndgameBossMultipliers(float HealthScalar, float DamageScalar, float ScaleScalar);

	/** Called by hero projectile overlap; returns true if boss died. DamageSourceID used for run damage log (default: AutoAttack). EventType for floating text (Crit, DoT; default none). */
	bool TakeDamageFromHeroHit(int32 DamageAmount, FName DamageSourceID = NAME_None, FName EventType = NAME_None);
	bool TakeDamageFromHeroHitZone(int32 DamageAmount, const FT66CombatTargetHandle& TargetHandle, FName DamageSourceID = NAME_None, FName EventType = NAME_None);
	void SetZeroDamageUnkillable(bool bEnabled, FName Reason = NAME_None);
	bool IsZeroDamageUnkillable() const { return bZeroDamageUnkillable; }
	float GetEffectiveArmor() const;
	bool SupportsCombatHitZones() const;
	FT66CombatTargetHandle ResolveCombatTargetHandle(const UPrimitiveComponent* HitComponent = nullptr, ET66HitZoneType PreferredZone = ET66HitZoneType::Body) const;
	FVector GetAimPointForHitZone(ET66HitZoneType HitZoneType) const;

	bool IsAwakened() const { return bAwakened; }
	bool IsAlive() const { return !bDefeated && CurrentHP > 0; }
	bool IsCombatTargetable() const;

	/** Coliseum: start the fight immediately (bypasses proximity). */
	void ForceAwaken() { Awaken(); }
	void ForceSewerSlimeKingAttackForAutomation(FName AttackPartID);
	void RefreshRunStateBossState() const;

#if !UE_BUILD_SHIPPING
	void ResetBossAttackOwnershipAutomationCounters();
	int32 GetBossAttackOwnershipAutomationCounter(FName EventID, FName AttackID, FName PartID) const;
	bool HasVisibleBossBodyForAutomation() const;
	bool KillBossPartForAutomation(FName PartID);
	void ForceBossAttackForAutomation(FName AttackID, FName OwningPartID = NAME_None);
	void ResetBossMovementAutomationState();
	FName GetBossMovementAutomationMode() const;
#endif

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
	void AssignSewerSlimeKingPartDefinitions();
	void ConfigureAttackProfileFromBossPartProfile(ET66BossPartProfile InProfile);
	void EnsureDefaultBossPartDefinitions();
	void RefreshCombatHitZoneState();
	void DrawCombatDebug() const;
	void RebuildBossPartState(bool bPreserveCurrentPercent);
	void BuildBossPartSnapshots(TArray<FT66BossPartSnapshot>& OutBossParts) const;
	bool RestoreBossPartStateFromRunState();
	void PushBossPartStateToRunState() const;
	void LoadBossAttackOwnershipRows();
	void LoadBossMovementPatternRows();
	int32 ResolveBossPartIndex(const UPrimitiveComponent* HitComponent, ET66HitZoneType PreferredZone, FName PreferredPartID = NAME_None) const;
	int32 FindFallbackBossPartIndex() const;
	float GetBossPartDamageMultiplier(int32 PartIndex) const;
	float GetHealthPercent() const;
	int32 GetAttackPhaseIndex() const;
	FVector ResolveGroundLocation(const FVector& PreferredLocation) const;
	void SpawnGroundAOEAtLocation(const FVector& WorldLocation, float RadiusScale = 1.f, float WarningScale = 1.f, bool bUseSecondaryTint = false);
	void SpawnGroundAOEAtLocationForAttackRow(const FT66BossAttackOwnershipData& AttackRow, const FVector& WorldLocation, float RadiusScale = 1.f, float WarningScale = 1.f, bool bUseSecondaryTint = false);
	void SpawnProjectileInDirection(const FVector& Direction, float SpeedScale = 1.f, const FVector& SpawnOffset = FVector::ZeroVector, bool bUseSecondaryTint = false);
	void SpawnProjectileInDirectionForAttackRow(const FT66BossAttackOwnershipData& AttackRow, const FVector& Direction, float SpeedScale = 1.f, const FVector& SpawnOffset = FVector::ZeroVector, bool bUseSecondaryTint = false, FName VisualProfileID = NAME_None, float VisualScaleMultiplier = 1.f, ET66AttackCategory ProjectileCategory = ET66AttackCategory::AOE, TSoftObjectPtr<UStaticMesh> ProjectileMesh = TSoftObjectPtr<UStaticMesh>(), float ProjectileMeshScale = 1.f);
	void SpawnScaledProjectileInDirection(const FVector& Direction, float SpeedScale, const FVector& SpawnOffset, bool bUseSecondaryTint, float VisualScaleMultiplier);
	void QueueTimedAttackLambda(FTimerDelegate&& Delegate, float DelaySeconds);
	void QueueProjectileShotTowards(const FVector& TargetLocation, float DelaySeconds, float YawOffsetDegrees = 0.f, float SpeedScale = 1.f, const FVector& SpawnOffset = FVector::ZeroVector, bool bUseSecondaryTint = false);
	void QueueProjectileShotDirection(const FVector& Direction, float DelaySeconds, float SpeedScale = 1.f, const FVector& SpawnOffset = FVector::ZeroVector, bool bUseSecondaryTint = false);
	void QueueProjectileFanBurst(const FVector& TargetLocation, int32 ShotCount, float SpreadDegrees, float DelayStepSeconds, float SpeedScale = 1.f, float InitialDelaySeconds = 0.f, float SideOffsetDistance = 0.f, bool bUseSecondaryTint = false);
	void QueueRadialBurst(int32 ShotCount, float DelayStepSeconds, float StartAngleDegrees, float SpeedScale = 1.f, float InitialDelaySeconds = 0.f, bool bUseSecondaryTint = false);
	void QueueProjectileShotTowardsForAttackRow(const FT66BossAttackOwnershipData& AttackRow, const FVector& TargetLocation, float DelaySeconds, float YawOffsetDegrees = 0.f, float SpeedScale = 1.f, const FVector& SpawnOffset = FVector::ZeroVector, bool bUseSecondaryTint = false, FName VisualProfileID = NAME_None, float VisualScaleMultiplier = 1.f, ET66AttackCategory ProjectileCategory = ET66AttackCategory::AOE, TSoftObjectPtr<UStaticMesh> ProjectileMesh = TSoftObjectPtr<UStaticMesh>(), float ProjectileMeshScale = 1.f);
	void QueueProjectileShotDirectionForAttackRow(const FT66BossAttackOwnershipData& AttackRow, const FVector& Direction, float DelaySeconds, float SpeedScale = 1.f, const FVector& SpawnOffset = FVector::ZeroVector, bool bUseSecondaryTint = false, FName VisualProfileID = NAME_None, float VisualScaleMultiplier = 1.f, ET66AttackCategory ProjectileCategory = ET66AttackCategory::AOE, TSoftObjectPtr<UStaticMesh> ProjectileMesh = TSoftObjectPtr<UStaticMesh>(), float ProjectileMeshScale = 1.f);
	void QueueProjectileFanBurstForAttackRow(const FT66BossAttackOwnershipData& AttackRow, const FVector& TargetLocation, int32 ShotCount, float SpreadDegrees, float DelayStepSeconds, float SpeedScale = 1.f, float InitialDelaySeconds = 0.f, float SideOffsetDistance = 0.f, bool bUseSecondaryTint = false, FName VisualProfileID = NAME_None, float VisualScaleMultiplier = 1.f, ET66AttackCategory ProjectileCategory = ET66AttackCategory::AOE, TSoftObjectPtr<UStaticMesh> ProjectileMesh = TSoftObjectPtr<UStaticMesh>(), float ProjectileMeshScale = 1.f);
	void QueueRadialBurstForAttackRow(const FT66BossAttackOwnershipData& AttackRow, int32 ShotCount, float DelayStepSeconds, float StartAngleDegrees, float SpeedScale = 1.f, float InitialDelaySeconds = 0.f, bool bUseSecondaryTint = false, FName VisualProfileID = NAME_None, float VisualScaleMultiplier = 1.f, ET66AttackCategory ProjectileCategory = ET66AttackCategory::AOE, TSoftObjectPtr<UStaticMesh> ProjectileMesh = TSoftObjectPtr<UStaticMesh>(), float ProjectileMeshScale = 1.f);
	void TickSimpleChaseMovement(const FVector& MyLoc, const FVector& PlayerLoc, bool bRunAway);
	bool TickAuthoredBossMovementPattern(float DeltaSeconds, const FVector& MyLoc, const FVector& PlayerLoc);
	const FT66BossMovementPatternData* PickBossMovementPatternRow() const;
	bool DoesMovementPatternRequireAttackCoordination(const FT66BossMovementPatternData& PatternRow) const;
	bool IsMovementPatternAttackCoordinationActive(const FT66BossMovementPatternData& PatternRow) const;
	void NotifyBossMovementAttackCoordinationStarted(const FT66BossAttackOwnershipData& AttackRow);
	bool IsSewerSlimeKingBoss() const;
	bool IsBossPartAlive(FName PartID) const;
	FVector GetBossPartWorldLocation(FName PartID) const;
	bool AreBossAttackPartsAlive(const FT66BossAttackOwnershipData& AttackRow, FName& OutDeadPartID) const;
	bool CanSelectBossAttackRow(const FT66BossAttackOwnershipData& AttackRow, int32 Phase, FName& OutDeadPartID) const;
	const FT66BossAttackOwnershipData* PickBossAttackRowByPrefix(const TCHAR* AttackIDPrefix, bool& bOutHasMatchingRows, FName& OutSuppressedPartID) const;
	const FT66BossAttackOwnershipData* FindBossAttackRowByAttackID(FName AttackID, FName OwningPartID = NAME_None) const;
	bool FireAuthoredBossProjectileAttack(APawn* PlayerPawn);
	bool FireBossProjectileAttackDefinitionRows(APawn* PlayerPawn, const FT66BossAttackOwnershipData& AttackRow, const FVector& TargetLocation, int32 Phase, const FVector& PlanarToTarget, const FVector& Side);
	bool FireBossProjectileAttackRow(APawn* PlayerPawn, const FT66BossAttackOwnershipData& AttackRow);
	bool FireAuthoredBossGroundAOE(APawn* PlayerPawn);
	bool FireBossGroundAOEAttackRow(APawn* PlayerPawn, const FT66BossAttackOwnershipData& AttackRow);
	const FT66BossAttackOwnershipData* PickSewerSlimeKingAttackRow() const;
	const FT66BossAttackOwnershipData* FindSewerSlimeKingAttackRowForPart(FName AttackPartID) const;
	const FT66BossAttackOwnershipData* FindSewerSlimeKingMouthSidecarRow() const;
	void FireSewerSlimeKingAttack(APawn* PlayerPawn, FName ForcedAttackPartID = NAME_None);
	void FireSewerSlimeKingAttackRow(APawn* PlayerPawn, const FT66BossAttackOwnershipData& AttackRow);
	void QueueSewerSlimeKingLobeVolley(const FT66BossAttackOwnershipData& AttackRow, APawn* InitialPlayerPawn, bool bUseSecondaryTint);
	void SpawnSewerSlimeKingLaneBlocker(const FT66BossAttackOwnershipData& AttackRow, const FVector& TargetLocation);
	void SpawnSewerSlimeKingMouthProjectile(const FT66BossAttackOwnershipData& AttackRow, const FVector& TargetLocation);
	void SpawnSewerSlimeKingTelegraph(FName AttackPartID, const FVector& Location, float DurationSeconds, float ScaleMultiplier, bool bCylinder);
	void ClearPendingAttackTimers();
	void RecordBossAttackOwnershipEvent(FName EventID, const FT66BossAttackOwnershipData* AttackRow, FName PartID, const TCHAR* Context);

	bool bBaseTuningInitialized = false;
	int32 BaseMaxHP = 0;
	int32 BaseProjectileDamageHearts = 0;
	float BaseMoveSpeed = 350.f;
	bool bDefeated = false;
	bool bZeroDamageUnkillable = false;
	FName ZeroDamageUnkillableReason = NAME_None;

	float ArmorDebuffAmount = 0.f;
	float ArmorDebuffSecondsRemaining = 0.f;
	float ConfusionSecondsRemaining = 0.f;
	float MoveSlowMultiplier = 1.f;
	float MoveSlowSecondsRemaining = 0.f;
	float ForcedRunAwaySecondsRemaining = 0.f;
	float StunSecondsRemaining = 0.f;
	float RootSecondsRemaining = 0.f;
	float FreezeSecondsRemaining = 0.f;
	float BossMovementAttackCoordinationSecondsRemaining = 0.f;
	float BossMovementAttackCoordinationSecondsSinceStart = 0.f;
	FName ActiveBossMovementAttackID = NAME_None;
	FName ActiveBossMovementAttackPartID = NAME_None;
	FVector CachedWanderDir = FVector::ZeroVector;
	float WanderDirRefreshAccum = 0.f;
	TWeakObjectPtr<APawn> CachedPlayerPawn;

	UPROPERTY(Transient)
	TArray<FT66BossPartRuntimeState> BossPartStates;

	UPROPERTY(Transient)
	TArray<FTimerHandle> PendingAttackTimerHandles;

	UPROPERTY(Transient)
	TArray<FT66BossAttackOwnershipData> BossAttackOwnershipRows;

	UPROPERTY(Transient)
	TArray<FT66BossMovementPatternData> BossMovementPatternRows;

#if !UE_BUILD_SHIPPING
	TMap<FString, int32> BossAttackOwnershipAutomationCounters;
	FName LastBossMovementAutomationMode = NAME_None;
#endif

	FName LastSewerSlimeKingAttackPart = NAME_None;
	FName BossMovementProfileID = NAME_None;
};

