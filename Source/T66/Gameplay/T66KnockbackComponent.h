// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "T66KnockbackComponent.generated.h"

class ACharacter;
class AT66HeroBase;
class UPhysicalAnimationComponent;
class UPhysicalMaterial;
class USkeletalMeshComponent;
class USceneComponent;

UENUM(BlueprintType)
enum class ET66KnockbackBudgetClass : uint8
{
	Hero UMETA(DisplayName = "Hero"),
	Boss UMETA(DisplayName = "Boss"),
	Elite UMETA(DisplayName = "Elite"),
	HordeFallback UMETA(DisplayName = "Horde Fallback")
};

UENUM(BlueprintType)
enum class ET66KnockbackPhysicalAnimationDriveMode : uint8
{
	Disabled UMETA(DisplayName = "Disabled"),
	PelvisOnly UMETA(DisplayName = "Pelvis Only"),
	CoreChain UMETA(DisplayName = "Core Chain"),
	AllBodiesBelowRoot UMETA(DisplayName = "All Bodies Below Root")
};

UENUM(BlueprintType)
enum class ET66KnockbackPhase : uint8
{
	Inactive UMETA(DisplayName = "Inactive"),
	Active UMETA(DisplayName = "Active"),
	Recovering UMETA(DisplayName = "Recovering")
};

/**
 * Data-authored knockback/ragdoll tuning. It is intentionally component-owned for
 * the first hero-only pass, but shaped like a future DataTable row/profile.
 */
USTRUCT(BlueprintType)
struct T66_API FT66KnockbackProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	ET66KnockbackBudgetClass BudgetClass = ET66KnockbackBudgetClass::Hero;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
	bool bEnableSkeletalRagdoll = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
	FName SimulationRootBoneName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
	FName FollowBoneName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
	FName VelocityBoneName = NAME_None;

	/** Simulates every body in the PhysicsAsset instead of leaving parent bodies kinematic. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
	bool bSimulateAllPhysicsBodies = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "0.01", ClampMax = "20.0"))
	float MinIncapacitationSeconds = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "0.05", ClampMax = "30.0"))
	float MaxRagdollSeconds = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "1.0", ClampMax = "5000.0"))
	float SettleSpeed = 165.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float SettleHoldSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "0.01", ClampMax = "5.0"))
	float RecoveryBlendOutSeconds = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
	bool bDetachMeshDuringRagdoll = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
	bool bFollowActorToRagdoll = true;

	/** Uses the simulated physics-body bounds center as the actor/camera follow target, falling back to FollowBoneName. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
	bool bUseSimulatedBodyCenterForActorFollow = true;

	/** False centers the camera/actor on the follow bone while ragdolled. True preserves the pre-impact capsule-to-bone offset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
	bool bUsePreImpactActorToFollowBoneOffset = false;

	/** 0 means unbounded above the impact location. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "0.0"))
	float MaxActorFollowHeightAboveStart = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
	bool bEnableFloorPenetrationGuard = true;

	/** Minimum clearance kept between the lowest simulated physics-body bounds and the resolved floor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float FloorPenetrationSkin = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float FloorTraceUpDistance = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float FloorTraceDownDistance = 1800.f;

	/** Large by default: this is a safety gate, not a soft visual correction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
	float MaxFloorCorrectionPerTick = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bSuppressLookInput = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse", meta = (ClampMin = "0.0"))
	float LaunchVelocityScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse", meta = (ClampMin = "0.0"))
	float MaxLaunchVelocity = 4200.f;

	/** Debug compatibility path. Default false keeps mass in the equation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse")
	bool bTreatLaunchVectorAsVelocityChange = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float MainBodyImpulseScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float BelowBodiesImpulseFraction = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse")
	bool bIncludeSimulationRootInBelowBodyImpulse = true;

	/** Optional runtime override for simulated ragdoll body linear damping. Negative keeps the PhysicsAsset/body default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Response", meta = (ClampMin = "-1.0"))
	float RagdollLinearDampingOverride = -1.f;

	/** Optional runtime override for simulated ragdoll body angular damping. Negative keeps the PhysicsAsset/body default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Response", meta = (ClampMin = "-1.0"))
	float RagdollAngularDampingOverride = -1.f;

	/** Optional runtime friction override for simulated ragdoll bodies. Negative keeps the existing physical material. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Response", meta = (ClampMin = "-1.0"))
	float RagdollFrictionOverride = -1.f;

	/** Optional runtime restitution override for simulated ragdoll bodies. Negative keeps the existing physical material. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Response", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float RagdollRestitutionOverride = -1.f;

	/** Legacy knockback ragdoll defaults to pure Chaos. Hero 1 active ragdoll is owned by UT66HeroPhysicsComponent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Animation")
	bool bEnablePhysicalAnimation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Animation")
	ET66KnockbackPhysicalAnimationDriveMode PhysicalAnimationDriveMode = ET66KnockbackPhysicalAnimationDriveMode::Disabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Animation", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float PhysicalAnimationStrength = 0.42f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Animation", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float PhysicalAnimationActivationDelaySeconds = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Animation", meta = (ClampMin = "0.0"))
	float PhysicalAnimationOrientationStrength = 240.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Animation", meta = (ClampMin = "0.0"))
	float PhysicalAnimationAngularVelocityStrength = 32.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Animation", meta = (ClampMin = "0.0"))
	float PhysicalAnimationMaxLinearForce = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Animation", meta = (ClampMin = "0.0"))
	float PhysicalAnimationMaxAngularForce = 9000.f;
};

UCLASS(ClassGroup=(Gameplay), BlueprintType, meta=(BlueprintSpawnableComponent))
class T66_API UT66KnockbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66KnockbackComponent();

	bool ApplyKnockbackLaunch(const FVector& LaunchVelocity, const FT66KnockbackProfile* ProfileOverride = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|Knockback")
	bool IsKnockbackActive() const { return Phase != ET66KnockbackPhase::Inactive; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|Knockback")
	bool IsIncapacitated() const { return Phase != ET66KnockbackPhase::Inactive; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|Knockback")
	ET66KnockbackPhase GetKnockbackPhase() const { return Phase; }

	const FT66KnockbackProfile& GetDefaultProfile() const { return DefaultProfile; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Knockback")
	FT66KnockbackProfile DefaultProfile;

private:
	ACharacter* ResolveCharacterOwner() const;
	USkeletalMeshComponent* ResolveSkeletalMesh() const;
	void ApplyGameplaySuppression(bool bSuppress);
	bool TryBeginSkeletalRagdoll(ACharacter* Character, USkeletalMeshComponent* MeshComponent, const FVector& LaunchVelocity, const FT66KnockbackProfile& Profile);
	void BeginFallbackLaunch(ACharacter* Character, const FVector& LaunchVelocity, const FT66KnockbackProfile& Profile);
	void UpdateActiveKnockback(double Now);
	void UpdateRecovery(double Now);
	void RestoreFromKnockback();
	void ResetRuntimeState();

	FName ResolveSimulationRootBone(const USkeletalMeshComponent* MeshComponent, const FT66KnockbackProfile& Profile) const;
	FName ResolveFollowBone(const USkeletalMeshComponent* MeshComponent, const FT66KnockbackProfile& Profile) const;
	FName ResolveVelocityBone(const USkeletalMeshComponent* MeshComponent, const FT66KnockbackProfile& Profile) const;
	FVector GetFollowLocation(const USkeletalMeshComponent* MeshComponent) const;
	bool HasPhysicalAnimationPoseBuffers(USkeletalMeshComponent* MeshComponent) const;
	void SyncKinematicMeshPoseToPhysics(USkeletalMeshComponent* MeshComponent);
	int32 ApplyPhysicalAnimationDrive(UPhysicalAnimationComponent* PhysicalAnimation, USkeletalMeshComponent* MeshComponent, const FT66KnockbackProfile& Profile) const;
	UPhysicalAnimationComponent* GetOrCreatePhysicalAnimationComponent(USkeletalMeshComponent* MeshComponent, const FT66KnockbackProfile& Profile, int32& OutDrivenBodyCount);
	void TryActivatePhysicalAnimation(double Now);
	void CacheBodyPhysicsSettings(USkeletalMeshComponent* MeshComponent);
	void ApplyRagdollPhysicsResponseProfile(USkeletalMeshComponent* MeshComponent, const FT66KnockbackProfile& Profile);
	void RestoreBodyPhysicsSettings(USkeletalMeshComponent* MeshComponent);
	void ApplyLaunchImpulse(USkeletalMeshComponent* MeshComponent, const FVector& LaunchVelocity, const FT66KnockbackProfile& Profile);
	void ApplyMassScaledImpulseToBody(USkeletalMeshComponent* MeshComponent, FName BodyName, const FVector& DesiredVelocityChange, float Scale, bool bVelocityChange);
	void ApplyMassScaledImpulseToBodiesBelow(USkeletalMeshComponent* MeshComponent, FName RootBoneName, const FVector& DesiredVelocityChange, float Scale, bool bVelocityChange, bool bIncludeSelf);
	bool ComputeSimulatedBodyBounds(const USkeletalMeshComponent* MeshComponent, FBox& OutBounds) const;
	FVector ResolveActorFollowLocation(const USkeletalMeshComponent* MeshComponent) const;
	bool TraceRagdollFloorZAtLocation(const ACharacter* Character, const FVector& ProbeLocation, float& OutFloorZ) const;
	bool ResolveRagdollFloorZ(const ACharacter* Character, const USkeletalMeshComponent* MeshComponent, float& OutFloorZ);
	float ResolveActorFloorAnchorZ(ACharacter* Character, USkeletalMeshComponent* MeshComponent);
	float EnforceFloorPenetrationGuard(ACharacter* Character, USkeletalMeshComponent* MeshComponent);

	UPROPERTY(Transient)
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> ActiveMesh = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> PreImpactAttachParent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UPhysicalMaterial> ActiveRagdollPhysicalMaterial = nullptr;

	struct FPreImpactBodyPhysicsSettings
	{
		int32 BodyIndex = INDEX_NONE;
		float LinearDamping = 0.f;
		float AngularDamping = 0.f;
		TWeakObjectPtr<UPhysicalMaterial> PhysMaterialOverride;
	};

	FT66KnockbackProfile ActiveProfile;
	TArray<FPreImpactBodyPhysicsSettings> PreImpactBodyPhysicsSettings;
	ET66KnockbackPhase Phase = ET66KnockbackPhase::Inactive;
	double KnockbackStartedTimeSeconds = -9999.0;
	double ControlRestoreTimeSeconds = -9999.0;
	double ForceRecoverTimeSeconds = -9999.0;
	double LowVelocityStartedTimeSeconds = -9999.0;
	double RecoverStartedTimeSeconds = -9999.0;
	double PhysicalAnimationActivationTimeSeconds = -9999.0;
	FName SimulationRootBoneName = NAME_None;
	FName FollowBoneName = NAME_None;
	FName VelocityBoneName = NAME_None;
	FName PreImpactAttachSocketName = NAME_None;
	FVector ActorToFollowBoneOffset = FVector::ZeroVector;
	FVector PreImpactActorLocation = FVector::ZeroVector;
	float PreImpactFloorZ = 0.f;
	float LastResolvedFloorZ = 0.f;
	double LastFloorGuardLogTimeSeconds = -9999.0;
	double LastActorFollowSampleLogTimeSeconds = -9999.0;
	double LastFollowDivergenceLogTimeSeconds = -9999.0;
	bool bHasResolvedFloorZ = false;
	FVector PreImpactMeshRelativeLocation = FVector::ZeroVector;
	FRotator PreImpactMeshRelativeRotation = FRotator::ZeroRotator;
	FVector PreImpactMeshRelativeScale = FVector::OneVector;
	EMovementMode PreImpactMovementMode = MOVE_Walking;
	uint8 PreImpactCustomMovementMode = 0;
	ECollisionEnabled::Type PreImpactCapsuleCollisionEnabled = ECollisionEnabled::QueryAndPhysics;
	FName PreImpactCapsuleCollisionProfileName = FName(TEXT("Pawn"));
	ECollisionEnabled::Type PreImpactMeshCollisionEnabled = ECollisionEnabled::NoCollision;
	FName PreImpactMeshCollisionProfileName = NAME_None;
	bool bPreImpactMeshHiddenInGame = false;
	bool bPreImpactMeshVisible = true;
	bool bPreImpactAutoAttackSuppressed = false;
	bool bGameplaySuppressed = false;
	bool bAppliedMoveInputSuppression = false;
	bool bPhysicalAnimationActivationPending = false;
	bool bDetachedMesh = false;
	bool bUsingSkeletalRagdoll = false;
	int32 PhysicalAnimationDrivenBodyCount = 0;
};
