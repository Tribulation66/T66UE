// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/MotionRig/T66MotionRigMotorSystem.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "HAL/IConsoleManager.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "ReferenceSkeleton.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66MotionRig, Log, All);

// Joint drive gains. Spring/damping are Chaos angular-drive units applied via
// SetAngularDriveParams. Damping low relative to spring is the wobble knob —
// rubric axis 3 (MOTION_RIG.md §4). All live; the scenario harness re-applies
// via RefreshBaseGains().
static TAutoConsoleVariable<float> CVarMRMotorLegSpring(
	TEXT("t66.MotionRig.Motor.LegSpring"), 2000000.f,
	TEXT("MotionRig leg joint drive spring."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorSpineSpring(
	TEXT("t66.MotionRig.Motor.SpineSpring"), 2600000.f,
	TEXT("MotionRig spine joint drive spring."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorArmSpring(
	TEXT("t66.MotionRig.Motor.ArmSpring"), 500000.f,
	TEXT("MotionRig arm joint drive spring (loose on purpose — secondary motion)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorHeadSpring(
	TEXT("t66.MotionRig.Motor.HeadSpring"), 700000.f,
	TEXT("MotionRig head joint drive spring."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorDampingRatio(
	TEXT("t66.MotionRig.Motor.DampingFraction"), 0.07f,
	TEXT("Joint drive damping as a fraction of spring (lower = wobblier)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorGlobalScale(
	TEXT("t66.MotionRig.Motor.GlobalScale"), 1.f,
	TEXT("Master multiplier over every MotionRig motor strength."), ECVF_Default);
// One-way pelvis coupling: virtual PD forces on the pelvis body toward the
// pose-source pelvis (which rides the bean). No constraint = no reaction on
// the bean = the body can NEVER push the bean around (a real tether buried
// the bean half a meter into the floor — walkcircle_v10).
static TAutoConsoleVariable<float> CVarMRPelvisLinearKp(
	TEXT("t66.MotionRig.Motor.PelvisLinearKp"), 480.f,
	TEXT("Pelvis follow proportional gain (accel per cm error, 1/s^2)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRPelvisLinearKd(
	TEXT("t66.MotionRig.Motor.PelvisLinearKd"), 28.f,
	TEXT("Pelvis follow damping gain (1/s)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRPelvisMaxAccel(
	TEXT("t66.MotionRig.Motor.PelvisMaxAccel"), 9000.f,
	TEXT("Clamp on pelvis follow acceleration (cm/s^2)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRPelvisAngularKp(
	TEXT("t66.MotionRig.Motor.PelvisAngularKp"), 400.f,
	TEXT("Pelvis orientation proportional gain (rad/s^2 per rad)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRPelvisAngularKd(
	TEXT("t66.MotionRig.Motor.PelvisAngularKd"), 22.f,
	TEXT("Pelvis orientation damping gain (1/s)."), ECVF_Default);

namespace
{
	bool IsArmBone(const FName Bone)
	{
		static const TSet<FName> Bones = {
			TEXT("clavicle_l"), TEXT("upperarm_l"), TEXT("lowerarm_l"), TEXT("hand_l"),
			TEXT("clavicle_r"), TEXT("upperarm_r"), TEXT("lowerarm_r"), TEXT("hand_r") };
		return Bones.Contains(Bone);
	}

	float SpringForBone(const FName Bone)
	{
		static const TSet<FName> Legs = {
			TEXT("thigh_l"), TEXT("calf_l"), TEXT("foot_l"),
			TEXT("thigh_r"), TEXT("calf_r"), TEXT("foot_r") };
		static const TSet<FName> Spine = { TEXT("spine_01"), TEXT("spine_02"), TEXT("pelvis") };
		if (Legs.Contains(Bone))
		{
			return CVarMRMotorLegSpring.GetValueOnGameThread();
		}
		if (Spine.Contains(Bone))
		{
			return CVarMRMotorSpineSpring.GetValueOnGameThread();
		}
		if (IsArmBone(Bone))
		{
			return CVarMRMotorArmSpring.GetValueOnGameThread();
		}
		if (Bone == TEXT("head"))
		{
			return CVarMRMotorHeadSpring.GetValueOnGameThread();
		}
		return CVarMRMotorArmSpring.GetValueOnGameThread();
	}
}

UT66MotionRigMotorSystem::UT66MotionRigMotorSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UT66MotionRigMotorSystem::InitializeMotors(USkeletalMeshComponent* InMesh, USkeletalMeshComponent* InPoseSource)
{
	Mesh = InMesh;
	PoseSource = InPoseSource;
	bMotorsInitialized = false;
	DriveJoints.Reset();

	if (!Mesh || !PoseSource)
	{
		return;
	}

	if (!Mesh->GetSkeletalMeshAsset() || !Mesh->GetPhysicsAsset())
	{
		if (!bLoggedMissingPhysicsAsset)
		{
			bLoggedMissingPhysicsAsset = true;
			UE_LOG(LogT66MotionRig, Warning,
				TEXT("MotionRig motors skipped: mesh/physics asset not available yet (bean-only mode)."));
		}
		return;
	}

	// Mass distribution per MOTION_RIG.md §3 (70 kg total, pelvis-heavy).
	// Auto-generated capsules default to volume-derived masses — measured
	// 485 kg on Hero 1 — which out-muscles every motor and drags the bean
	// down through the pelvis tether. Authored masses are part of the spec.
	{
		static const TPair<FName, float> BodyMassesKg[] = {
			{ TEXT("pelvis"), 14.0f }, { TEXT("spine_01"), 8.0f }, { TEXT("spine_02"), 8.5f },
			{ TEXT("head"), 5.6f },
			{ TEXT("clavicle_l"), 0.5f }, { TEXT("clavicle_r"), 0.5f },
			{ TEXT("upperarm_l"), 1.6f }, { TEXT("lowerarm_l"), 1.4f }, { TEXT("hand_l"), 1.2f },
			{ TEXT("upperarm_r"), 1.6f }, { TEXT("lowerarm_r"), 1.4f }, { TEXT("hand_r"), 1.2f },
			{ TEXT("thigh_l"), 5.5f }, { TEXT("calf_l"), 4.5f }, { TEXT("foot_l"), 2.25f },
			{ TEXT("thigh_r"), 5.5f }, { TEXT("calf_r"), 4.5f }, { TEXT("foot_r"), 2.25f } };
		float TotalMass = 0.f;
		for (const TPair<FName, float>& BoneMass : BodyMassesKg)
		{
			if (FBodyInstance* Body = Mesh->GetBodyInstance(BoneMass.Key))
			{
				Body->SetMassOverride(BoneMass.Value, true);
				Body->UpdateMassProperties();
				TotalMass += BoneMass.Value;
			}
		}
		UE_LOG(LogT66MotionRig, Display, TEXT("MotionRig body masses authored: %.1f kg total"), TotalMass);
	}

	// Configure SLERP angular drives on every runtime joint constraint and
	// cache what the per-tick target write needs. Reference-pose local
	// rotations let targets be expressed as deltas from the bind pose, which
	// matches how constraint reference frames are generated.
	const FReferenceSkeleton& RefSkeleton = Mesh->GetSkeletalMeshAsset()->GetRefSkeleton();
	const TArray<FTransform>& RefPose = RefSkeleton.GetRefBonePose();

	for (int32 ConstraintIndex = 0; ConstraintIndex < Mesh->Constraints.Num(); ++ConstraintIndex)
	{
		FConstraintInstance* Constraint = Mesh->Constraints[ConstraintIndex];
		if (!Constraint)
		{
			continue;
		}

		// ConstraintBone1 is the child body in generated assets.
		const FName ChildBone = Constraint->ConstraintBone1;
		const int32 ChildBoneIndex = RefSkeleton.FindBoneIndex(ChildBone);
		if (ChildBoneIndex == INDEX_NONE)
		{
			continue;
		}

		Constraint->SetAngularDriveMode(EAngularDriveMode::SLERP);
		Constraint->SetOrientationDriveSLERP(true);
		Constraint->SetAngularVelocityDriveSLERP(true);

		FDriveJoint Joint;
		Joint.ConstraintIndex = ConstraintIndex;
		Joint.ChildBoneIndex = ChildBoneIndex;
		Joint.RefLocalRotationInverse = RefPose[ChildBoneIndex].GetRotation().Inverse();
		Joint.SetStrengthScaleArm = IsArmBone(ChildBone) ? 1.f : 0.f;
		Joint.BaseStrength = SpringForBone(ChildBone);
		DriveJoints.Add(Joint);
	}

	bMotorsInitialized = DriveJoints.Num() > 0;
	UE_LOG(LogT66MotionRig, Display,
		TEXT("MotionRig motors initialized: %d joint drives (direct SLERP)."), DriveJoints.Num());

	ApplyStateProfile(ET66MotionRigState::Idle);
}

FT66MotionRigStateMotorScale UT66MotionRigMotorSystem::ProfileForState(const ET66MotionRigState State) const
{
	FT66MotionRigStateMotorScale Scale;
	switch (State)
	{
	case ET66MotionRigState::Idle:
		Scale.AllScale = 1.f; Scale.ArmScale = 1.f; Scale.PelvisWorldScale = 1.f; Scale.RampSeconds = 0.25f;
		break;
	case ET66MotionRigState::Walk:
		Scale.AllScale = 1.f; Scale.ArmScale = 1.f; Scale.PelvisWorldScale = 1.f; Scale.RampSeconds = 0.15f;
		break;
	case ET66MotionRigState::Jump:
		Scale.AllScale = 1.15f; Scale.ArmScale = 1.1f; Scale.PelvisWorldScale = 1.f; Scale.RampSeconds = 0.05f;
		break;
	case ET66MotionRigState::Dive:
		Scale.AllScale = 1.3f; Scale.ArmScale = 1.4f; Scale.PelvisWorldScale = 0.55f; Scale.RampSeconds = 0.05f;
		break;
	case ET66MotionRigState::Knockdown:
		Scale.AllScale = 0.03f; Scale.ArmScale = 0.02f; Scale.PelvisWorldScale = 0.f; Scale.RampSeconds = 0.f;
		break;
	case ET66MotionRigState::GetUp:
		Scale.AllScale = 1.f; Scale.ArmScale = 1.f; Scale.PelvisWorldScale = 0.8f; Scale.RampSeconds = 0.6f;
		break;
	}
	return Scale;
}

void UT66MotionRigMotorSystem::ApplyStateProfile(const ET66MotionRigState State)
{
	if (!bMotorsInitialized)
	{
		return;
	}

	CurrentState = State;
	const FT66MotionRigStateMotorScale Scale = ProfileForState(State);
	TargetAllScale = Scale.AllScale;
	TargetArmScale = Scale.ArmScale;
	TargetPelvisWorldScale = Scale.PelvisWorldScale;
	RampRatePerSecond = (Scale.RampSeconds > KINDA_SMALL_NUMBER) ? (1.f / Scale.RampSeconds) : 0.f;

	if (RampRatePerSecond <= 0.f)
	{
		CurrentAllScale = TargetAllScale;
		CurrentArmScale = TargetArmScale;
		CurrentPelvisWorldScale = TargetPelvisWorldScale;
		ApplyGainsAtScale(CurrentAllScale, CurrentArmScale, CurrentPelvisWorldScale);
	}
}

void UT66MotionRigMotorSystem::GoLimp()
{
	if (!bMotorsInitialized)
	{
		return;
	}
	TargetAllScale = CurrentAllScale = 0.03f;
	TargetArmScale = CurrentArmScale = 0.02f;
	TargetPelvisWorldScale = CurrentPelvisWorldScale = 0.f;
	RampRatePerSecond = 0.f;
	ApplyGainsAtScale(CurrentAllScale, CurrentArmScale, CurrentPelvisWorldScale);
}

void UT66MotionRigMotorSystem::RefreshBaseGains()
{
	if (bMotorsInitialized)
	{
		for (FDriveJoint& Joint : DriveJoints)
		{
			if (FConstraintInstance* Constraint = Mesh->Constraints.IsValidIndex(Joint.ConstraintIndex) ? Mesh->Constraints[Joint.ConstraintIndex] : nullptr)
			{
				Joint.BaseStrength = SpringForBone(Constraint->ConstraintBone1);
			}
		}
		ApplyGainsAtScale(CurrentAllScale, CurrentArmScale, CurrentPelvisWorldScale);
	}
}

void UT66MotionRigMotorSystem::TickComponent(
	const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bMotorsInitialized)
	{
		return;
	}

	if (RampRatePerSecond > 0.f)
	{
		CurrentAllScale = FMath::Clamp(FMath::FInterpConstantTo(CurrentAllScale, TargetAllScale, DeltaTime, RampRatePerSecond), 0.f, 4.f);
		CurrentArmScale = FMath::Clamp(FMath::FInterpConstantTo(CurrentArmScale, TargetArmScale, DeltaTime, RampRatePerSecond), 0.f, 4.f);
		CurrentPelvisWorldScale = FMath::Clamp(FMath::FInterpConstantTo(CurrentPelvisWorldScale, TargetPelvisWorldScale, DeltaTime, RampRatePerSecond), 0.f, 4.f);
	}

	const bool bDirty =
		!FMath::IsNearlyEqual(CurrentAllScale, LastAppliedAllScale, 0.01f) ||
		!FMath::IsNearlyEqual(CurrentArmScale, LastAppliedArmScale, 0.01f) ||
		!FMath::IsNearlyEqual(CurrentPelvisWorldScale, LastAppliedPelvisWorldScale, 0.01f);
	if (bDirty)
	{
		ApplyGainsAtScale(CurrentAllScale, CurrentArmScale, CurrentPelvisWorldScale);
	}

	TickDriveTargets();
}

void UT66MotionRigMotorSystem::TickDriveTargets()
{
	if (!Mesh || !PoseSource || !PoseSource->GetSkeletalMeshAsset())
	{
		return;
	}

	// Chaos puts settled islands to sleep, and a sleeping body ignores drive
	// target updates AND AddForce — the silent killer behind every "motors do
	// nothing" iteration (v5..v12). While motors are meaningfully on, the
	// skeleton must stay awake; during knockdown (scale ~0) it may sleep.
	if (CurrentAllScale > 0.15f)
	{
		Mesh->WakeAllRigidBodies();
	}
	++DiagTickCount;

	// Local-space animated pose straight from the hidden kinematic source.
	const TArray<FTransform>& LocalPose = PoseSource->GetBoneSpaceTransforms();
	if (LocalPose.Num() == 0)
	{
		return;
	}

	for (const FDriveJoint& Joint : DriveJoints)
	{
		if (!Mesh->Constraints.IsValidIndex(Joint.ConstraintIndex) || !LocalPose.IsValidIndex(Joint.ChildBoneIndex))
		{
			continue;
		}
		FConstraintInstance* Constraint = Mesh->Constraints[Joint.ConstraintIndex];
		if (!Constraint)
		{
			continue;
		}

		// Target = animated local rotation expressed as a delta from the bind
		// pose (constraint reference frames are generated at the bind pose).
		const FQuat AnimLocal = LocalPose[Joint.ChildBoneIndex].GetRotation();
		const FQuat TargetDelta = Joint.RefLocalRotationInverse * AnimLocal;
		Constraint->SetAngularOrientationTarget(TargetDelta);
	}

	TickPelvisFollow();
}

void UT66MotionRigMotorSystem::TickPelvisFollow()
{
	// Virtual PD on the pelvis body toward the pose-source pelvis world
	// transform. One-way by construction: forces act on the body only, so the
	// bean never feels the skeleton. Scaled by the state profile's pelvis
	// factor (0 while limp).
	FBodyInstance* Pelvis = Mesh ? Mesh->GetBodyInstance(TEXT("pelvis")) : nullptr;
	if (!Pelvis || !Pelvis->IsInstanceSimulatingPhysics() || CurrentPelvisWorldScale <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	FTransform TargetTransform = PoseSource->GetBoneTransform(TEXT("pelvis"));
	const FTransform PelvisTransform = Pelvis->GetUnrealWorldTransform();

	// Dive: the bean cannot pitch (locked axes), so the prone orientation is
	// authored here — pitch the pelvis target forward over the body's right
	// axis and drop it toward the ground for the belly-slide silhouette.
	if (CurrentState == ET66MotionRigState::Dive)
	{
		const FVector RightAxis = TargetTransform.GetRotation().GetRightVector();
		TargetTransform.SetRotation(FQuat(RightAxis, FMath::DegreesToRadians(-72.f)) * TargetTransform.GetRotation());
		TargetTransform.AddToTranslation(FVector(0.f, 0.f, -42.f));
	}

	// Linear PD in acceleration space (bAccelChange — mass-independent).
	const FVector PositionError = TargetTransform.GetLocation() - PelvisTransform.GetLocation();
	const FVector Velocity = Pelvis->GetUnrealWorldVelocity();
	FVector Accel = PositionError * CVarMRPelvisLinearKp.GetValueOnGameThread()
		- Velocity * CVarMRPelvisLinearKd.GetValueOnGameThread();
	const float MaxAccel = CVarMRPelvisMaxAccel.GetValueOnGameThread();
	if (Accel.SizeSquared() > MaxAccel * MaxAccel)
	{
		Accel = Accel.GetSafeNormal() * MaxAccel;
	}
	Pelvis->AddForce(Accel * CurrentPelvisWorldScale, false, /*bAccelChange*/ true);
	++DiagPelvisApplyCount;
	DiagLastPelvisAccel = Accel.Size() * CurrentPelvisWorldScale;

	// Angular PD toward the animated pelvis orientation (acceleration space).
	const FQuat DeltaQuat = TargetTransform.GetRotation() * PelvisTransform.GetRotation().Inverse();
	FVector Axis;
	float Angle;
	DeltaQuat.ToAxisAndAngle(Axis, Angle);
	if (Angle > PI)
	{
		Angle -= 2.f * PI;
	}
	const FVector AngularVelocity = Pelvis->GetUnrealWorldAngularVelocityInRadians();
	const FVector AngularAccel = Axis * Angle * CVarMRPelvisAngularKp.GetValueOnGameThread()
		- AngularVelocity * CVarMRPelvisAngularKd.GetValueOnGameThread();
	Pelvis->AddTorqueInRadians(AngularAccel * CurrentPelvisWorldScale, false, /*bAccelChange*/ true);
}

void UT66MotionRigMotorSystem::ApplyGainsAtScale(
	const float InAllScale, const float InArmScale, const float InPelvisWorldScale)
{
	if (!Mesh)
	{
		return;
	}

	const float Global = FMath::Max(0.f, CVarMRMotorGlobalScale.GetValueOnGameThread());
	const float All = InAllScale * Global;
	const float DampingFraction = FMath::Max(0.f, CVarMRMotorDampingRatio.GetValueOnGameThread());

	for (const FDriveJoint& Joint : DriveJoints)
	{
		if (!Mesh->Constraints.IsValidIndex(Joint.ConstraintIndex))
		{
			continue;
		}
		FConstraintInstance* Constraint = Mesh->Constraints[Joint.ConstraintIndex];
		if (!Constraint)
		{
			continue;
		}

		float Strength = Joint.BaseStrength * All;
		if (Joint.SetStrengthScaleArm > 0.f)
		{
			Strength *= InArmScale;
		}
		Constraint->SetAngularDriveParams(Strength, Strength * DampingFraction, 0.f);
	}

	LastAppliedAllScale = InAllScale;
	LastAppliedArmScale = InArmScale;
	LastAppliedPelvisWorldScale = InPelvisWorldScale;
}
