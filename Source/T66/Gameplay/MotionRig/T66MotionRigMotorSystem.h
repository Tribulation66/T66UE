// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/MotionRig/T66MotionRigTypes.h"
#include "T66MotionRigMotorSystem.generated.h"

class USkeletalMeshComponent;

// Owns the motors that pull the always-simulated skeleton toward the
// animation pose. States are expressed purely as motor gain profiles applied
// to named body sets — never as simulate/kinematic switches. See
// MOTION_RIG.md section 2.
//
// Implementation: direct SLERP angular drives on the simulated skeleton's own
// joint constraints, with per-tick orientation targets read from a hidden
// kinematic POSE SOURCE mesh that plays the clips. Fully self-owned — both
// the PhysicsControl plugin and UPhysicalAnimationComponent silently produced
// zero force on this setup in 5.7 (walkcircle_v5..v9 evidence).
UCLASS()
class T66_API UT66MotionRigMotorSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66MotionRigMotorSystem();

	// Authored per-bone masses (70 kg total) + joint drive setup. Safe to call
	// when the mesh has no physics asset yet — logs and no-ops, so the pawn
	// still works as a bare bean before Phase 2 assets land.
	void InitializeMotors(USkeletalMeshComponent* InMesh, USkeletalMeshComponent* InPoseSource);

	bool AreMotorsInitialized() const { return bMotorsInitialized; }

	// Applies a state's motor profile. Ramping is handled internally over
	// Scale.RampSeconds (0 = instant).
	void ApplyStateProfile(ET66MotionRigState State);

	// Drops every motor to zero immediately (limp). Knockdown entry path.
	void GoLimp();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Re-reads base gains from CVars and reapplies at current scale. The
	// scenario harness calls this between tuning iterations.
	void RefreshBaseGains();

private:
	void ApplyGainsAtScale(float InAllScale, float InArmScale, float InPelvisWorldScale);
	void TickDriveTargets();
	void TickPelvisFollow();
	FT66MotionRigStateMotorScale ProfileForState(ET66MotionRigState State) const;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> PoseSource;

	// Per-runtime-constraint cached data for fast per-tick target writes.
	struct FDriveJoint
	{
		int32 ConstraintIndex = INDEX_NONE;
		int32 ChildBoneIndex = INDEX_NONE;
		FQuat RefLocalRotationInverse = FQuat::Identity;
		float SetStrengthScaleArm = 0.f; // 1 if this joint belongs to the arm set
		float BaseStrength = 0.f;        // resolved from set CVars at gain apply
	};
	TArray<FDriveJoint> DriveJoints;

	bool bMotorsInitialized = false;
	bool bLoggedMissingPhysicsAsset = false;
	ET66MotionRigState CurrentState = ET66MotionRigState::Idle;

public:
	// Diagnostic counters (read by the pawn's MR_DIAG snapshot).
	int32 DiagTickCount = 0;
	int32 DiagPelvisApplyCount = 0;
	float DiagLastPelvisAccel = 0.f;

private:

	// Ramp state
	float CurrentAllScale = 1.f;
	float CurrentArmScale = 1.f;
	float CurrentPelvisWorldScale = 1.f;
	float TargetAllScale = 1.f;
	float TargetArmScale = 1.f;
	float TargetPelvisWorldScale = 1.f;
	float RampRatePerSecond = 0.f; // 0 = snap
	float LastAppliedAllScale = -1.f;
	float LastAppliedArmScale = -1.f;
	float LastAppliedPelvisWorldScale = -1.f;
};
