// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66WorldRuntimeProofTypes.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66BossAttackTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "T66ProjectileManagerSubsystem.generated.h"

class AActor;
class AT66HeroBase;
class UHierarchicalInstancedStaticMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class USceneComponent;
class UStaticMesh;

enum class ET66ManagedProjectileDelivery : uint8
{
	EnemyProjectile,
	BossProjectile
};

enum class ET66ManagedProjectileVisualBodyMode : uint8
{
	HISM,
	Niagara
};

struct FT66ManagedProjectileFireParams
{
	AActor* SourceActor = nullptr;
	FName SourceID = NAME_None;
	FName VisualProfileID = NAME_None;
	FVector Origin = FVector::ZeroVector;
	FVector Direction = FVector::ForwardVector;
	float Speed = 0.f;
	float Damage = 0.f;
	float Radius = 18.f;
	float Lifetime = 4.f;
	int32 ProjectileTypeIndex = 0;
	ET66AttackCategory AttackCategory = ET66AttackCategory::AOE;
	TSoftObjectPtr<UStaticMesh> ProjectileMesh;
	float ProjectileMeshScale = 1.f;
	ET66ManagedProjectileDelivery Delivery = ET66ManagedProjectileDelivery::EnemyProjectile;
	ET66BossAttackProfile BossAttackProfile = ET66BossAttackProfile::Balanced;
	FLinearColor BossPrimaryColor = FLinearColor(0.95f, 0.16f, 0.12f, 1.f);
	FLinearColor BossSecondaryColor = FLinearColor(1.f, 0.72f, 0.18f, 1.f);
	bool bUseBossSecondaryTint = false;
	float BossVisualScaleMultiplier = 1.f;
};

struct FT66ManagedProjectile
{
	bool bIsActive = false;
	FVector Position = FVector::ZeroVector;
	FVector PreviousPosition = FVector::ZeroVector;
	FVector Velocity = FVector::ZeroVector;
	float LifetimeRemaining = 0.f;
	float Radius = 18.f;
	int32 Damage = 20;
	TWeakObjectPtr<AActor> SourceMob;
	FName SourceMobID = NAME_None;
	bool bSourceWasLightweight = false;
	ET66ManagedProjectileDelivery Delivery = ET66ManagedProjectileDelivery::EnemyProjectile;
	ET66BossAttackProfile BossAttackProfile = ET66BossAttackProfile::Balanced;
	bool bUseBossSecondaryTint = false;
	float BossVisualScaleMultiplier = 1.f;
	ET66AttackCategory AttackCategory = ET66AttackCategory::AOE;
	FVector VisualScale = FVector(1.f);
	FQuat VisualRotationOffset = FQuat::Identity;
	FName VisualProfileID = NAME_None;
	ET66ManagedProjectileVisualBodyMode VisualBodyMode = ET66ManagedProjectileVisualBodyMode::HISM;
	FLinearColor BossPrimaryColor = FLinearColor::White;
	FLinearColor BossSecondaryColor = FLinearColor::White;
	int32 ProjectileTypeIndex = 0;
	int32 InstanceIndex = INDEX_NONE;
	float DistanceTraveled = 0.f;
	float SourceIgnoreDistance = 0.f;
	TWeakObjectPtr<UNiagaraComponent> BodyComponent;
	TWeakObjectPtr<UNiagaraComponent> TrailComponent;
	TWeakObjectPtr<UNiagaraSystem> TrailSystem;
	TWeakObjectPtr<UNiagaraSystem> ImpactSystem;
};

struct FT66ProjectileManagerDiagnostics
{
	int32 ResetCount = 0;
	int32 ProjectilesFired = 0;
	int32 ProjectilesActivePeak = 0;
	int32 ProjectilesHitHero = 0;
	int32 ProjectilesExpired = 0;
	int32 ProjectilesHitWorld = 0;
	int32 DroppedFires = 0;
	int32 DroppedInvalidSource = 0;
	int32 VisualBucketOverflowCount = 0;
	int32 ApplyDamageReturnedFalse = 0;
	int32 VisualProfilesResolved = 0;
	int32 VisualProfileFallbacks = 0;
	int32 NiagaraBodiesSpawned = 0;
	double ManagerTickMaxUs = 0.0;
	double ManagerTickTotalUs = 0.0;
	int32 ManagerTickSamples = 0;
	double HISMUpdateMaxUs = 0.0;
	double HISMUpdateTotalUs = 0.0;
	int32 HISMUpdateSamples = 0;
	FString LastResetReason;
	FString LastWorldImpactActor;

	void Reset(const FString& Reason);
	double GetManagerTickAvgUs() const;
	double GetHISMUpdateAvgUs() const;
};

struct FT66BossProjectileVisualKey
{
	ET66BossAttackProfile AttackProfile = ET66BossAttackProfile::Balanced;
	FColor Color = FColor::White;

	friend bool operator==(const FT66BossProjectileVisualKey& A, const FT66BossProjectileVisualKey& B)
	{
		return A.AttackProfile == B.AttackProfile && A.Color == B.Color;
	}
};

FORCEINLINE uint32 GetTypeHash(const FT66BossProjectileVisualKey& Key)
{
	const uint32 ColorHash =
		(static_cast<uint32>(Key.Color.R) << 24)
		| (static_cast<uint32>(Key.Color.G) << 16)
		| (static_cast<uint32>(Key.Color.B) << 8)
		| static_cast<uint32>(Key.Color.A);
	return HashCombine(GetTypeHash(static_cast<uint8>(Key.AttackProfile)), GetTypeHash(ColorHash));
}

struct FT66ProjectileVisualBucket
{
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Component = nullptr;
	FName VisualProfileID = NAME_None;
	ET66BossAttackProfile AttackProfile = ET66BossAttackProfile::Balanced;
	FColor Color = FColor::White;
	FVector VisualScale = FVector(1.f);
	FQuat RotationOffset = FQuat::Identity;
	ET66ManagedProjectileVisualBodyMode BodyMode = ET66ManagedProjectileVisualBodyMode::HISM;
	TWeakObjectPtr<UStaticMesh> BodyMesh;
	TWeakObjectPtr<UNiagaraSystem> BodySystem;
	TWeakObjectPtr<UNiagaraSystem> TrailSystem;
	TWeakObjectPtr<UNiagaraSystem> ImpactSystem;
	bool bUseFlatColorMaterial = true;
	bool bOverflowBucket = false;
};

UCLASS()
class T66_API UT66ProjectileManagerSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxProjectiles = 512;
	static constexpr int32 EnemySpitProjectileTypeIndex = 0;
	static constexpr int32 MaxExactBossVisualBuckets = 32;

	static FName DefaultEnemySpitVisualProfileID();
	static FName EnemyWebVisualProfileID();
	static FName EnemyWebNiagaraVisualProfileID();
	static FName BossWebNeedleVisualProfileID();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool FireProjectile(
		AActor* SourceMob,
		FName SourceMobID,
		FVector Origin,
		FVector Direction,
		float Speed,
		float Damage,
		float Radius,
		float Lifetime,
		int32 ProjectileTypeIndex);
	bool FireManagedProjectile(const FT66ManagedProjectileFireParams& Params);
	bool FireBossProjectile(const FT66ManagedProjectileFireParams& Params);

	int32 GetActiveProjectileCount() const { return ActiveProjectileCount; }
	const FT66ProjectileManagerDiagnostics& GetDiagnostics() const { return Diagnostics; }
	void ResetProjectileDiagnostics(const TCHAR* Reason);
	void EmitProjectileManagerSummary(const TCHAR* Reason, bool bTerminal);
#if !UE_BUILD_SHIPPING
	bool RunBossProjectileKillMidFlightProof();
	bool RunManagedProjectileVisualProfileProof();
	bool RunBossProjectileVisualProfileProof();
	FT66WorldRuntimeDebugSnapshot GetWorldRuntimeDebugSnapshot() const;
#endif

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override;
	virtual bool IsTickableInEditor() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;

private:
	UPROPERTY(Transient)
	TObjectPtr<AActor> RenderHost = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> RenderRoot = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> ProjectileComponents;

	TArray<FT66ManagedProjectile> Projectiles;
	TArray<FT66ProjectileVisualBucket> ProjectileVisualBuckets;
	TMap<FName, int32> ManagedVisualBucketByProfileID;
	TMap<FT66BossProjectileVisualKey, int32> BossVisualBucketByKey;
	TMap<ET66BossAttackProfile, int32> BossOverflowBucketByProfile;
	TMap<FString, int32> AuthoredMeshBucketByKey;
	TSet<FString> VisualBucketWarningsEmitted;
	int32 ActiveProjectileCount = 0;
	bool bInitialized = false;
	bool bShuttingDown = false;
	bool bTerminalSummaryEmitted = false;
	bool bProjectileInstancesDirty = false;
	FT66ProjectileManagerDiagnostics Diagnostics;

	bool EnsureRenderResources(const FVector& AnchorLocation);
	UHierarchicalInstancedStaticMeshComponent* GetProjectileComponent(int32 ProjectileTypeIndex);
	int32 ResolveManagedProjectileVisualTypeIndex(const FT66ManagedProjectileFireParams& Params);
	int32 ResolveBossProjectileTypeIndex(const FT66ManagedProjectileFireParams& Params);
	int32 ResolveAuthoredProjectileMeshTypeIndex(const FT66ManagedProjectileFireParams& Params);
	UHierarchicalInstancedStaticMeshComponent* CreateProjectileComponent(int32 ProjectileTypeIndex, const FT66ProjectileVisualBucket& Bucket);
	void PreallocateInstances(UHierarchicalInstancedStaticMeshComponent* Component);
	int32 FindFreeProjectileSlot() const;
	bool AllocateProjectile(FT66ManagedProjectileFireParams Params);
	void DeactivateProjectile(int32 SlotIndex);
	void CleanupProjectileTrail(FT66ManagedProjectile& Projectile);
	void HideProjectileInstance(const FT66ManagedProjectile& Projectile);
	void UpdateProjectileInstance(const FT66ManagedProjectile& Projectile);
	void SpawnProjectileBody(FT66ManagedProjectile& Projectile);
	void SpawnBossTrail(FT66ManagedProjectile& Projectile);
	void SpawnBossImpact(const FT66ManagedProjectile& Projectile);
	void TickProjectile(int32 SlotIndex, float DeltaTime, AT66HeroBase* Hero);
	bool ResolveHeroCapsule(const AT66HeroBase* Hero, FVector& OutSegmentA, FVector& OutSegmentB, float& OutRadius) const;
	bool FindHeroHitFraction(const FVector& SegmentStart, const FVector& SegmentEnd, const AT66HeroBase* Hero, float ProjectileRadius, float& OutFraction) const;
	bool FindNonHeroImpact(const FT66ManagedProjectile& Projectile, const FVector& SegmentStart, const FVector& SegmentEnd, const AT66HeroBase* Hero, FHitResult& OutHit) const;
	float ResolveSourceIgnoreDistance(const AActor* SourceMob, float ProjectileRadius) const;
	FTransform MakeLocalTransform(const FT66ManagedProjectile& Projectile) const;
	FVector GetRenderHostLocation() const;
	void FlushProjectileInstanceUpdates();
	void AccumulateManagerTick(double ElapsedUs);
	void AccumulateHISMUpdate(uint64 StartCycles);
};
