// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/Enemies/T66EnemyFamilyTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "T66MobManagerSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogT66RangedDiagnostics, Log, All);

class AActor;
class AT66HeroBase;
class AT66MobBase;
class UMaterialInstanceDynamic;
class UPrimitiveComponent;

enum class ET66RangedDiagnosticPath : uint8
{
	Unknown,
	Rich,
	Lightweight
};

enum class ET66RouteAttributionReason : uint8
{
	RoutedLightweight_BasicFamily,
	RoutedRich_SpecialOrMiniBoss,
	RoutedRich_MiniBossPromotion,
	RoutedRich_FamilyLookupFailed,
	RoutedRich_FallbackBranch,
	RoutedRich_NonDirectorPath
};

enum class ET66RouteAttributionChannel : uint8
{
	InitialPopulation,
	RuntimeTrickle,
	NonDirector
};

struct FT66RouteAttributionFamilyCounters
{
	int32 Total = 0;
	int32 RoutedLightweightBasic = 0;
	int32 RoutedRichSpecialOrMiniBoss = 0;
	int32 RoutedRichMiniBossPromotion = 0;
	int32 RoutedRichFamilyLookupFailed = 0;
	int32 RoutedRichFallbackBranch = 0;
	int32 RoutedRichNonDirectorPath = 0;
};

struct FT66RouteAttributionDiagnostics
{
	int32 ResetCount = 0;
	int32 TotalObservedSpawns = 0;
	int32 DirectorObservedSpawns = 0;
	int32 InitialPopulationSpawns = 0;
	int32 RuntimeTrickleSpawns = 0;
	int32 NonDirectorObservedSpawns = 0;
	int32 LightweightAcquireFailed = 0;
	int32 MiniBossPromotionSlots = 0;
	int32 SpecialSlots = 0;
	int32 BossOrGuardianObserved = 0;
	FString LastResetReason;
	FT66RouteAttributionFamilyCounters Melee;
	FT66RouteAttributionFamilyCounters Rush;
	FT66RouteAttributionFamilyCounters Flying;
	FT66RouteAttributionFamilyCounters Ranged;
	FT66RouteAttributionFamilyCounters SpecialUnknown;

	void Reset(const FString& Reason);
	int32 CalculateCounterMismatch() const;
};

struct FT66RangedPressureDiagnostics
{
	int32 ResetCount = 0;
	int32 RichRangedSpawns = 0;
	int32 LightweightRangedSpawns = 0;
	int32 RichFireAttempts = 0;
	int32 LightweightFireAttempts = 0;
	int32 RichSafeZoneSkips = 0;
	int32 LightweightSafeZoneSkips = 0;
	int32 RichStatusBlocked = 0;
	int32 LightweightStatusBlocked = 0;
	int32 RichOutOfRangeSkips = 0;
	int32 LightweightOutOfRangeSkips = 0;
	int32 RichCooldownBlocked = 0;
	int32 LightweightCooldownBlocked = 0;
	int32 RichDistancePass = 0;
	int32 LightweightDistancePass = 0;
	int32 RichLosBlocked = 0;
	int32 LightweightLosBlocked = 0;
	int32 RichLosBlockerWorldStatic = 0;
	int32 LightweightLosBlockerWorldStatic = 0;
	int32 RichLosBlockerWorldDynamic = 0;
	int32 LightweightLosBlockerWorldDynamic = 0;
	int32 RichLosBlockerRichEnemy = 0;
	int32 LightweightLosBlockerRichEnemy = 0;
	int32 RichLosBlockerLightweightMob = 0;
	int32 LightweightLosBlockerLightweightMob = 0;
	int32 RichLosBlockerOtherPawn = 0;
	int32 LightweightLosBlockerOtherPawn = 0;
	int32 RichLosBlockerUnknown = 0;
	int32 LightweightLosBlockerUnknown = 0;
	int32 RichLosPass = 0;
	int32 LightweightLosPass = 0;
	int32 RichDispatchReached = 0;
	int32 LightweightDispatchReached = 0;
	int32 RichProjectileSpawned = 0;
	int32 LightweightProjectileSpawned = 0;
	int32 RichProjectileSpawnFailed = 0;
	int32 LightweightProjectileSpawnFailed = 0;
	int32 RichZeroDirectionShots = 0;
	int32 LightweightZeroDirectionShots = 0;
	int32 ProjectileOwnerIgnored = 0;
	int32 ProjectileNonHeroImpacts = 0;
	int32 ProjectileHeroHurtboxRejects = 0;
	int32 ProjectileSafeZoneRejects = 0;
	int32 ProjectileHeroHits = 0;
	int32 RichProjectileHeroHits = 0;
	int32 LightweightProjectileHeroHits = 0;
	int32 UnknownProjectileHeroHits = 0;
	int32 ProjectileDamageHP = 0;
	float FirstHeroProjectileHitWorldTime = -1.f;
	float LastHeroProjectileHitWorldTime = -1.f;
	float LastHeroHP = -1.f;
	FName LastHeroHitMobID = NAME_None;
	FString LastHeroHitOwnerName;
	FString LastHeroHitOwnerClass;
	FString LastResetReason;

	void Reset(const FString& Reason);
};

USTRUCT()
struct FT66MobVertexAnimationRuntimeState
{
	GENERATED_BODY()

	TWeakObjectPtr<AT66MobBase> Mob;

	FT66MobVertexAnimationRow Row;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> Material = nullptr;

	UPROPERTY(Transient)
	FName Clip = NAME_None;

	UPROPERTY(Transient)
	float ClipTime = 0.f;

	UPROPERTY(Transient)
	float PlayRate = 1.f;

	UPROPERTY(Transient)
	float OverrideSecondsRemaining = 0.f;

	UPROPERTY(Transient)
	bool bUsingVertexAnimation = false;

	// B.13 consumes this flat layout as HISM per-instance custom data.
	UPROPERTY(Transient)
	float CustomDataFrame = 0.f;

	UPROPERTY(Transient)
	float CustomDataStartFrame = 0.f;

	UPROPERTY(Transient)
	float CustomDataEndFrame = 0.f;

	UPROPERTY(Transient)
	float CustomDataClipIndex = 0.f;

	UPROPERTY(Transient)
	float CustomDataPlayRate = 1.f;

	UPROPERTY(Transient)
	float CustomDataFlags = 0.f;

	void Reset();
};

UCLASS()
class T66_API UT66MobManagerSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxInactiveMobs = 128;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	static bool IsRangedDiagnosticLoggingEnabled();

	AT66MobBase* AcquireMob(UClass* MobClass, const FTransform& SpawnTransform, bool* bOutRequiresFinishSpawning = nullptr);
	void ReleaseMob(AT66MobBase* Mob);
	void RegisterMob(AT66MobBase* Mob);
	void UnregisterMob(AT66MobBase* Mob);
	void NotifyMobDying(AT66MobBase* Mob);
	void ConfigureMobVertexAnimationState(AT66MobBase* Mob, const FT66MobVertexAnimationRow& Row, UMaterialInstanceDynamic* Material);
	void ClearMobVertexAnimationState(AT66MobBase* Mob);
	void SetMobVertexAnimationClip(AT66MobBase* Mob, FName ClipName, float OverrideSeconds = 0.f);
	void TickMobVertexAnimationState(AT66MobBase* Mob, float DeltaSeconds);
	bool IsMobUsingVertexAnimation(const AT66MobBase* Mob) const;
	const TArray<TWeakObjectPtr<AT66MobBase>>& GetActiveMobs() const { return ActiveMobs; }
	int32 GetInactiveMobCount() const { return InactiveMobs.Num(); }
	int32 GetPoolFreshSpawnCount() const { return PoolFreshSpawnCount; }
	int32 GetPoolReuseAcquireCount() const { return PoolReuseAcquireCount; }
	int32 GetPoolReleaseCount() const { return PoolReleaseCount; }
	int32 GetPoolOverflowDestroyCount() const { return PoolOverflowDestroyCount; }
	int32 GetPeakInactiveMobCount() const { return PeakInactiveMobCount; }
	void SetBackroomsGameplayPaused(bool bPaused);
	bool IsBackroomsGameplayPaused() const { return bBackroomsGameplayPaused; }
	void ResetRangedPressureDiagnostics(const TCHAR* Reason);
	void RecordRangedMobSpawn(bool bLightweight, FName MobID);
	void RecordRangedFireAttempt(bool bLightweight, FName MobID, float Dist2D);
	void RecordRangedStatusBlocked(bool bLightweight, FName MobID, float Dist2D);
	void RecordRangedCooldownBlocked(bool bLightweight, FName MobID, float Dist2D, float CooldownRemaining);
	void RecordRangedDistancePassed(bool bLightweight, FName MobID, float Dist2D, float DesiredMinRange, float DesiredMaxRange, float MaxFireRange);
	void RecordRangedFireSkippedSafeZone(bool bLightweight, FName MobID, float Dist2D);
	void RecordRangedFireSkippedOutOfRange(bool bLightweight, FName MobID, float Dist2D, float MaxFireRange);
	void RecordRangedLosBlocked(bool bLightweight, FName MobID, float Dist2D, const AActor* BlockerActor, const UPrimitiveComponent* BlockerComponent);
	void RecordRangedLosPassed(bool bLightweight, FName MobID, float Dist2D);
	void RecordRangedDispatchReached(bool bLightweight, FName MobID, float Dist2D);
	void RecordRangedProjectileSpawned(bool bLightweight, FName MobID);
	void RecordRangedProjectileSpawnFailed(bool bLightweight, FName MobID);
	void RecordRangedZeroDirectionShot(bool bLightweight, FName MobID);
	void RecordEnemyProjectileOwnerIgnored(const AActor* Projectile, const AActor* Owner);
	void RecordEnemyProjectileNonHeroImpact(const AActor* Projectile, const AActor* Owner, const AActor* OtherActor);
	void RecordEnemyProjectileHeroHurtboxReject(const AActor* Projectile, const AActor* Owner, const AT66HeroBase* Hero);
	void RecordEnemyProjectileSafeZoneReject(const AActor* Projectile, const AActor* Owner, const AT66HeroBase* Hero);
	void RecordEnemyProjectileHeroHit(const AActor* Projectile, const AActor* Owner, const AT66HeroBase* Hero, int32 DamageHP);
	void RecordManagedEnemyProjectileOwnerIgnored(const AActor* Owner, FName OwnerMobID, bool bLightweight);
	void RecordManagedEnemyProjectileWorldImpact(const AActor* Owner, FName OwnerMobID, bool bLightweight, const AActor* HitActor);
	void RecordManagedEnemyProjectileSafeZoneReject(const AActor* Owner, FName OwnerMobID, bool bLightweight, const AT66HeroBase* Hero);
	void RecordManagedEnemyProjectileHeroHit(const AActor* Owner, FName OwnerMobID, bool bLightweight, const AT66HeroBase* Hero, int32 DamageHP);
	void RecordRouteAttribution(ET66EnemyFamily Family, ET66RouteAttributionReason Reason, ET66RouteAttributionChannel Channel);
	void RecordLightweightAcquireFailed(ET66EnemyFamily Family, ET66RouteAttributionChannel Channel);
	void RecordBossOrGuardianRouteAttribution(ET66EnemyFamily Family);
	void EmitRangedPressureSummary(const TCHAR* Reason, bool bTerminal);
	const FT66RangedPressureDiagnostics& GetRangedPressureDiagnostics() const { return RangedDiagnostics; }
	const FT66RouteAttributionDiagnostics& GetRouteAttributionDiagnostics() const { return RouteAttributionDiagnostics; }

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override;
	virtual bool IsTickableInEditor() const override;

#if !UE_BUILD_SHIPPING
	bool RunMobTickVatRuntimeProof();
	bool AutomationApplyMobTouchDamageForTest(AT66MobBase* Mob, AT66HeroBase* Hero, float DeltaTime);
#endif

private:
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AT66MobBase>> ActiveMobs;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AT66MobBase>> InactiveMobs;

	UPROPERTY(Transient)
	TArray<FT66MobVertexAnimationRuntimeState> ActiveMobVertexAnimationStates;

	int32 PoolFreshSpawnCount = 0;
	int32 PoolReuseAcquireCount = 0;
	int32 PoolReleaseCount = 0;
	int32 PoolOverflowDestroyCount = 0;
	int32 PeakInactiveMobCount = 0;
	bool bBackroomsGameplayPaused = false;
	uint64 TickFrameCounter = 0;
	bool bRangedDiagnosticTerminalSummaryEmitted = false;
	bool bMobTickVatRuntimeProofEmitted = false;
	FT66RangedPressureDiagnostics RangedDiagnostics;
	FT66RouteAttributionDiagnostics RouteAttributionDiagnostics;

	bool ShouldTrackRangedDiagnostics(const TCHAR* FunctionName) const;
	int32 FindMobVertexAnimationStateIndex(const AT66MobBase* Mob) const;
	FT66MobVertexAnimationRuntimeState* FindMobVertexAnimationState(AT66MobBase* Mob);
	const FT66MobVertexAnimationRuntimeState* FindMobVertexAnimationState(const AT66MobBase* Mob) const;
	bool GetMobVertexAnimationClipRange(const FT66MobVertexAnimationRuntimeState& State, FName ClipName, int32& OutStartFrame, int32& OutEndFrame, float& OutPlayRate) const;
	float ResolveMobVertexAnimationClipIndex(FName ClipName) const;
	void UpdateMobVertexAnimationCustomData(FT66MobVertexAnimationRuntimeState& State, int32 Frame, int32 StartFrame, int32 EndFrame, float PlayRate) const;
};
