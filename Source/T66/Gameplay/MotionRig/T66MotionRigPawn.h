// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Gameplay/MotionRig/T66MotionRigInputReceiver.h"
#include "Gameplay/MotionRig/T66MotionRigTypes.h"
#include "T66MotionRigPawn.generated.h"

class UAnimSequence;
class UCameraComponent;
class UCapsuleComponent;
class UPhysicsConstraintComponent;
class UPoseableMeshComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class UT66MotionRigMotorSystem;
class UT66MotionRigScenario;

// MotionRig hero pawn: ONE physical reality. The root capsule ("bean") is a
// simulated rigid body that all movement forces act on; the skeletal mesh
// simulates every body and is pulled toward animation pose targets by
// PhysicsControl motors. States only change clips + motor gains + bean
// forces. See MOTION_RIG.md.
//
// Multiplayer seam: the bean (this actor's root) is the only thing that would
// ever replicate; the skeleton/motors are local cosmetics. All input arrives
// through IT66MotionRigInputReceiver.
UCLASS()
class T66_API AT66MotionRigPawn : public APawn, public IT66MotionRigInputReceiver
{
	GENERATED_BODY()

public:
	AT66MotionRigPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// IT66MotionRigInputReceiver
	virtual void MotionRigSetMoveAxes(float ForwardValue, float RightValue) override;
	virtual void MotionRigJumpPressed() override;
	virtual void MotionRigJumpReleased() override;
	virtual void MotionRigDivePressed() override;

	// Standardized impact entry (scenario harness + future gameplay routing).
	// LaunchVelocity is a velocity change applied coherently to the whole body.
	UFUNCTION(BlueprintCallable, Category = "MotionRig")
	void TriggerKnockdown(const FVector& LaunchVelocity);

	// Scenario input override: the player controller's axis bindings fire every
	// frame (usually zero) and would stomp scripted input. While active, the
	// bridge's MotionRigSetMoveAxes calls are ignored.
	void SetScenarioInputOverride(bool bActive);
	void ScenarioSetMoveAxes(float ForwardValue, float RightValue);

	// Telemetry surface for the scenario component.
	ET66MotionRigState GetMotionState() const { return MotionState; }
	bool IsBeanGrounded() const { return bGrounded; }
	FVector GetBeanVelocity() const;
	UCapsuleComponent* GetBean() const { return Bean; }
	USkeletalMeshComponent* GetRigMesh() const { return RigMesh; }
	UT66MotionRigMotorSystem* GetMotorSystem() const { return MotorSystem; }

private:
	// --- state machine ---
	void SetMotionState(ET66MotionRigState NewState);
	void TickStateMachine(float DeltaSeconds);
	void StartGetUp();
	void FinishKnockdownEnter();

	// --- bean physics ---
	void TickGroundSense();
	void TickBeanForces(float DeltaSeconds);
	void TickKnockdownFollow();
	void SetBeanPhysicsEnabled(bool bEnabled);
	void ReattachPelvisConstraint();
	void EnsureMeshSimulation();
	void TickVisualFromBodies();

	// --- visuals / clips ---
	void LoadAssets();
	void PlayStateClip(ET66MotionRigState State);
	void TickWalkCadence();
	UAnimSequence* ClipForState(ET66MotionRigState State) const;

	FVector GetCameraRelativeInputDirection() const;

	// --- components ---
	UPROPERTY(VisibleAnywhere, Category = "MotionRig")
	TObjectPtr<UCapsuleComponent> Bean;

	UPROPERTY(VisibleAnywhere, Category = "MotionRig")
	TObjectPtr<USkeletalMeshComponent> RigMesh;

	// Hidden kinematic clip player: the motor system reads per-tick bone
	// targets from here. The simulated RigMesh NEVER plays animation (playing
	// on it re-inits articulation and silently kills simulation).
	UPROPERTY(VisibleAnywhere, Category = "MotionRig")
	TObjectPtr<USkeletalMeshComponent> PoseSource;

	// The actual rendered character: bone transforms copied straight from the
	// simulated bodies every tick. The engine's physics→bone blend path never
	// produced bone updates on this setup (v14..v17: bodies standing/walking,
	// render frozen) — this copy CANNOT fail silently. RigMesh is invisible.
	UPROPERTY(VisibleAnywhere, Category = "MotionRig")
	TObjectPtr<UPoseableMeshComponent> Visual;

	UPROPERTY(VisibleAnywhere, Category = "MotionRig")
	TObjectPtr<UPhysicsConstraintComponent> PelvisConstraint;


	UPROPERTY(VisibleAnywhere, Category = "MotionRig")
	TObjectPtr<UT66MotionRigMotorSystem> MotorSystem;

	UPROPERTY(VisibleAnywhere, Category = "MotionRig")
	TObjectPtr<UT66MotionRigScenario> Scenario;

	UPROPERTY(VisibleAnywhere, Category = "MotionRig")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "MotionRig")
	TObjectPtr<UCameraComponent> FollowCamera;

	// --- clips (loaded by path; null-safe before Phase 2 assets land) ---
	UPROPERTY()
	TObjectPtr<UAnimSequence> ClipIdle;
	UPROPERTY()
	TObjectPtr<UAnimSequence> ClipWalk;
	UPROPERTY()
	TObjectPtr<UAnimSequence> ClipJump;
	UPROPERTY()
	TObjectPtr<UAnimSequence> ClipDive;
	UPROPERTY()
	TObjectPtr<UAnimSequence> ClipGetUpFront;
	UPROPERTY()
	TObjectPtr<UAnimSequence> ClipGetUpBack;

	// --- runtime state ---
	ET66MotionRigState MotionState = ET66MotionRigState::Idle;
	float MoveForwardValue = 0.f;
	float MoveRightValue = 0.f;
	bool bGrounded = false;
	float GroundDistance = 0.f;
	float AirTimeSeconds = 0.f;
	float StateTimeSeconds = 0.f;
	float KnockdownSettleSeconds = 0.f;
	float DiveSlideSeconds = 0.f;
	bool bGetUpFromFront = true;
	FVector PendingKnockdownLaunch = FVector::ZeroVector;
	bool bAssetsLoaded = false;
	// False until the deferred bring-up: the game mode spawns pawns at the map
	// origin and teleports them later; simulating full-size bodies before the
	// teleport detonates on depenetration.
	bool bPhysicsLive = false;
	bool bScenarioInputOverride = false;
};
