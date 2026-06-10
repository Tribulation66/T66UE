// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/MotionRig/T66MotionRigTypes.h"
#include "T66MotionRigMotorSystem.generated.h"

class UPhysicsControlComponent;
class USkeletalMeshComponent;

// Owns the PhysicsControl motors that pull the always-simulated skeleton
// toward the animation pose. States are expressed purely as motor gain
// profiles applied to named control sets — never as simulate/kinematic
// switches. See MOTION_RIG.md section 2.
UCLASS()
class T66_API UT66MotionRigMotorSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66MotionRigMotorSystem();

	// Creates body modifiers (everything simulated) and the control sets.
	// Safe to call when the mesh has no physics asset yet — logs and no-ops,
	// so the pawn still works as a bare bean before Phase 2 assets land.
	void InitializeMotors(USkeletalMeshComponent* InMesh, UPhysicsControlComponent* InControl);

	bool AreMotorsInitialized() const { return bMotorsInitialized; }

	// Applies a state's motor profile. Ramping is handled internally over
	// Scale.RampSeconds (0 = instant).
	void ApplyStateProfile(ET66MotionRigState State);

	// Drops every motor to zero immediately (limp). Knockdown entry path —
	// kept separate from ApplyStateProfile so impact code can be explicit.
	void GoLimp();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Re-reads base gains from CVars and reapplies at current scale. The
	// scenario harness calls this between tuning iterations.
	void RefreshBaseGains();

private:
	void ApplyGainsAtScale(float InAllScale, float InArmScale, float InPelvisWorldScale);
	FT66MotionRigStateMotorScale ProfileForState(ET66MotionRigState State) const;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY()
	TObjectPtr<UPhysicsControlComponent> Control;

	bool bMotorsInitialized = false;
	bool bLoggedMissingPhysicsAsset = false;

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
