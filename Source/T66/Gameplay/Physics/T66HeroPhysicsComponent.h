#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/T66DataTypes.h"
#include "T66HeroPhysicsComponent.generated.h"

class AT66HeroBase;
class UAnimationAsset;
class UCapsuleComponent;
class UCharacterMovementComponent;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class ET66HeroPhysicsRuntimeState : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Ragdoll UMETA(DisplayName = "Ragdoll"),
	GettingUp UMETA(DisplayName = "Getting Up")
};

USTRUCT(BlueprintType)
struct FT66HeroPhysicsProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero Physics")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero Physics")
	bool bHero1ChadOnly = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero Physics")
	FName SimulationRootBodyName = TEXT("pelvis");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero Physics")
	FName PelvisBodyName = TEXT("pelvis");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero Physics", meta = (ClampMin = "0.0"))
	float ReactionCooldownSeconds = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero Physics", meta = (ClampMin = "0.0"))
	float RagdollLaunchSpeedMax = 3500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero Physics", meta = (ClampMin = "0.0"))
	float RagdollLaunchUpSpeed = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero Physics", meta = (ClampMin = "0.0"))
	float RagdollSettleSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero Physics", meta = (ClampMin = "0.0"))
	float RagdollSettleHoldSeconds = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero Physics", meta = (ClampMin = "0.0"))
	float RagdollMaxSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero Physics", meta = (ClampMin = "0.0"))
	float RagdollBlendOutSeconds = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero Physics", meta = (ClampMin = "0.0"))
	float FloorTraceUpDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hero Physics", meta = (ClampMin = "0.0"))
	float FloorTraceDownDistance = 5000.0f;
};

USTRUCT(BlueprintType)
struct FT66HeroRagdollRecoveryUIState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hero Physics|Recovery")
	bool bVisible = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hero Physics|Recovery")
	float Progress01 = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hero Physics|Recovery")
	float ElapsedSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hero Physics|Recovery")
	float RemainingSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hero Physics|Recovery")
	float EffectiveMaxSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hero Physics|Recovery")
	float UncreditedMaxSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hero Physics|Recovery")
	float CreditSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hero Physics|Recovery")
	int32 AcceptedJumpPresses = 0;
};

/**
 * Hero 1 Chad hit-triggered ragdoll authority.
 *
 * Normal play is owned by the capsule and CharacterMovementComponent. The mesh stays animated,
 * non-simulating, collisionless, and attached to the capsule. A qualifying hit temporarily
 * transfers authority to a detached full skeletal ragdoll, then recovers by placing the capsule
 * under the settled pelvis and playing a get-up animation.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class T66_API UT66HeroPhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66HeroPhysicsComponent();

	void InitializeForHero(AT66HeroBase* InHero, USkeletalMeshComponent* InMesh = nullptr);
	void ShutdownActiveRagdoll();

	bool ApplyPhysicsReaction(const FVector& RequestedVelocityChange, const FVector& WorldHitLocation, FName SourceTag, float LaunchScale = 1.0f, float DurationDamagePercent = -1.0f, float DurationScaleStartPercent = 0.0f, float DurationScaleFullPercent = -1.0f);
	bool NotifyJumpRecoveryInput();

	bool IsActiveRagdollInitialized() const { return bInitialized; }
	bool IsRagdollActive() const { return RuntimeState != ET66HeroPhysicsRuntimeState::Normal; }
	bool IsIncapacitated() const { return bGameplaySuppressed || IsRagdollActive(); }
	ET66HeroPhysicsRuntimeState GetRuntimeState() const { return RuntimeState; }
	bool GetRagdollRecoveryUIState(FT66HeroRagdollRecoveryUIState& OutState) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool TryInitializeActiveRagdoll();
	bool ResolveRequiredBodies();

	void ConfigureNormalMesh();
	void ConfigureRagdollMesh();

	bool EnterRagdoll(const FVector& RequestedVelocityChange, const FVector& WorldHitLocation, FName SourceTag, float LaunchScale, float DurationDamagePercent, float DurationScaleStartPercent, float DurationScaleFullPercent);
	void UpdateRagdoll(float DeltaTime);
	void EnterGettingUp(FName Reason);
	void UpdateGettingUp(float DeltaTime);
	void FinishGetUp();

	void ApplyGameplaySuppression(bool bSuppress);
	void SetRuntimeState(ET66HeroPhysicsRuntimeState NewState, FName Reason);
	FVector BuildLaunchVelocity(const FVector& RequestedVelocityChange, float LaunchScale) const;
	float GetCurrentHeroDamagePercent() const;
	float ComputeDamagePercentDurationScale() const;
	float ComputeHealthScaledRagdollMaxSeconds() const;
	float ComputeEffectiveRagdollMaxSeconds() const;
	float ComputeEffectiveRagdollSettleHoldSeconds() const;

	bool ResolvePelvisWorld(FVector& OutLocation, FVector* OutVelocity = nullptr, FTransform* OutTransform = nullptr) const;
	float ComputePelvisCapsuleDistance(const FVector& PelvisWorldLocation) const;
	void MoveActorXYToPelvis(const FVector& PelvisWorldLocation);
	void PlaceCapsuleForGetUp(const FVector& PelvisWorldLocation);
	bool IsFaceDown(const FTransform& PelvisWorldTransform) const;
	UAnimationAsset* ResolveGetUpAnimation(bool bFaceDown) const;

	void ApplyLaunchToBodies(const FVector& LaunchVelocity, const FVector& WorldHitLocation);
	void EmitRuntimeSample(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = "Hero Physics")
	FT66HeroPhysicsProfile Profile;

	UPROPERTY(Transient)
	TObjectPtr<AT66HeroBase> CachedHero;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> ActiveMesh;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CurrentGetUpAnimation;

	ET66HeroPhysicsRuntimeState RuntimeState = ET66HeroPhysicsRuntimeState::Normal;
	FTransform DefaultMeshRelativeTransform = FTransform::Identity;
	FName ResolvedPelvisBodyName = NAME_None;
	FName ResolvedSimulationRootBodyName = NAME_None;
	bool bHasDefaultMeshRelativeTransform = false;
	bool bInitialized = false;
	bool bGameplaySuppressed = false;
	bool bPreImpactAutoAttackSuppressed = false;
	bool bAppliedMoveInputSuppression = false;

	float LastReactionTimeSeconds = -1000.0f;
	float StateStartWorldTimeSeconds = 0.0f;
	float LastDebugLogTimeSeconds = -1000.0f;
	float RagdollElapsedSeconds = 0.0f;
	float RagdollSettleHeldSeconds = 0.0f;
	float RagdollJumpRecoveryCreditSeconds = 0.0f;
	float GettingUpElapsedSeconds = 0.0f;
	float GetUpDurationSeconds = 0.6f;
	float LastAppliedImpulseMagnitude = 0.0f;
	float LastJumpRecoveryInputTimeSeconds = -1000.0f;
	float RagdollDurationDamagePercent = -1.0f;
	float RagdollDurationScaleStartPercent = 0.0f;
	float RagdollDurationScaleFullPercent = -1.0f;
	int32 RagdollJumpRecoveryAcceptedPresses = 0;
};
