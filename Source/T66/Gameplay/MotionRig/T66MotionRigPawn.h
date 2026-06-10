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
class UPhysicsControlComponent;
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

	UPROPERTY(VisibleAnywhere, Category = "MotionRig")
	TObjectPtr<UPhysicsConstraintComponent> PelvisConstraint;

	UPROPERTY(VisibleAnywhere, Category = "MotionRig")
	TObjectPtr<UPhysicsControlComponent> PhysicsControl;

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
};
