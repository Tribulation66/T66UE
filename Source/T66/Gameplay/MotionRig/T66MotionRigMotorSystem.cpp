// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/MotionRig/T66MotionRigMotorSystem.h"

#include "Components/SkeletalMeshComponent.h"
#include "HAL/IConsoleManager.h"
#include "PhysicsControlComponent.h"
#include "PhysicsControlData.h"
#include "PhysicsEngine/PhysicsAsset.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66MotionRig, Log, All);

// Base motor gains. DampingRatio < 1 is the wobble knob: the body overshoots
// its pose target and settles — tune against rubric axis 3 (MOTION_RIG.md §4).
// All live-tunable; the scenario harness re-applies via RefreshBaseGains().
static TAutoConsoleVariable<float> CVarMRMotorLegAngular(
	TEXT("t66.MotionRig.Motor.LegAngularStrength"), 2200.f,
	TEXT("MotionRig parent-space angular strength for leg motors."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorLegDamping(
	TEXT("t66.MotionRig.Motor.LegAngularDamping"), 0.8f,
	TEXT("MotionRig leg motor damping ratio (1 = critical, <1 = wobbly)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorSpineAngular(
	TEXT("t66.MotionRig.Motor.SpineAngularStrength"), 2600.f,
	TEXT("MotionRig parent-space angular strength for spine motors."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorSpineDamping(
	TEXT("t66.MotionRig.Motor.SpineAngularDamping"), 0.7f,
	TEXT("MotionRig spine motor damping ratio."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorArmAngular(
	TEXT("t66.MotionRig.Motor.ArmAngularStrength"), 900.f,
	TEXT("MotionRig parent-space angular strength for arm motors (loose on purpose)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorArmDamping(
	TEXT("t66.MotionRig.Motor.ArmAngularDamping"), 0.45f,
	TEXT("MotionRig arm motor damping ratio (low = floppy secondary motion)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorHeadAngular(
	TEXT("t66.MotionRig.Motor.HeadAngularStrength"), 700.f,
	TEXT("MotionRig parent-space angular strength for the head motor."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorHeadDamping(
	TEXT("t66.MotionRig.Motor.HeadAngularDamping"), 0.55f,
	TEXT("MotionRig head motor damping ratio."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorPelvisLinear(
	TEXT("t66.MotionRig.Motor.PelvisWorldLinearStrength"), 1200.f,
	TEXT("MotionRig world-space linear strength holding the pelvis to the animated pose."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorPelvisAngular(
	TEXT("t66.MotionRig.Motor.PelvisWorldAngularStrength"), 2400.f,
	TEXT("MotionRig world-space angular strength orienting the pelvis to the animated pose."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorPelvisDamping(
	TEXT("t66.MotionRig.Motor.PelvisWorldDamping"), 0.9f,
	TEXT("MotionRig pelvis world-space damping ratio (linear and angular)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRMotorGlobalScale(
	TEXT("t66.MotionRig.Motor.GlobalScale"), 1.f,
	TEXT("Master multiplier over every MotionRig motor strength."), ECVF_Default);

namespace T66MotionRigSets
{
	static const FName ParentLegs(TEXT("MR_Parent_Legs"));
	static const FName ParentSpine(TEXT("MR_Parent_Spine"));
	static const FName ParentArms(TEXT("MR_Parent_Arms"));
	static const FName ParentHead(TEXT("MR_Parent_Head"));
	static const FName WorldPelvis(TEXT("MR_World_Pelvis"));
	static const FName Bodies(TEXT("MR_Bodies"));
}

namespace
{
	FPhysicsControlData MakeAngularControlData(const float AngularStrength, const float AngularDamping)
	{
		FPhysicsControlData Data;
		Data.bEnabled = true;
		Data.LinearStrength = 0.f;
		Data.LinearDampingRatio = 1.f;
		Data.AngularStrength = AngularStrength;
		Data.AngularDampingRatio = AngularDamping;
		Data.bUseSkeletalAnimation = true;
		Data.bDisableCollision = false;
		Data.bOnlyControlChildObject = false;
		return Data;
	}
}

UT66MotionRigMotorSystem::UT66MotionRigMotorSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UT66MotionRigMotorSystem::InitializeMotors(USkeletalMeshComponent* InMesh, UPhysicsControlComponent* InControl)
{
	Mesh = InMesh;
	Control = InControl;
	bMotorsInitialized = false;

	if (!Mesh || !Control)
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

	// Everything simulates, full blend, normal gravity. There is deliberately
	// no kinematic fallback anywhere in this lane.
	{
		FPhysicsControlModifierData BodyData;
		BodyData.MovementType = EPhysicsMovementType::Simulated;
		BodyData.CollisionType = ECollisionEnabled::QueryAndPhysics;
		BodyData.GravityMultiplier = 1.f;
		BodyData.PhysicsBlendWeight = 1.f;
		Control->CreateBodyModifiersFromSkeletalMeshBelow(
			Mesh, TEXT("pelvis"), /*bIncludeSelf*/ true, T66MotionRigSets::Bodies, BodyData);
	}

	// Parent-space joint motors, grouped so states can treat limbs differently.
	const TArray<FName> LegBones = {
		TEXT("thigh_l"), TEXT("calf_l"), TEXT("foot_l"),
		TEXT("thigh_r"), TEXT("calf_r"), TEXT("foot_r") };
	const TArray<FName> SpineBones = { TEXT("spine_01"), TEXT("spine_02") };
	const TArray<FName> ArmBones = {
		TEXT("upperarm_l"), TEXT("lowerarm_l"), TEXT("hand_l"),
		TEXT("upperarm_r"), TEXT("lowerarm_r"), TEXT("hand_r") };
	const TArray<FName> HeadBones = { TEXT("head") };
	const TArray<FName> PelvisBones = { TEXT("pelvis") };

	const int32 NumLegs = Control->CreateControlsFromSkeletalMesh(
		Mesh, LegBones, EPhysicsControlType::ParentSpace,
		MakeAngularControlData(CVarMRMotorLegAngular.GetValueOnGameThread(), CVarMRMotorLegDamping.GetValueOnGameThread()),
		T66MotionRigSets::ParentLegs).Num();
	const int32 NumSpine = Control->CreateControlsFromSkeletalMesh(
		Mesh, SpineBones, EPhysicsControlType::ParentSpace,
		MakeAngularControlData(CVarMRMotorSpineAngular.GetValueOnGameThread(), CVarMRMotorSpineDamping.GetValueOnGameThread()),
		T66MotionRigSets::ParentSpine).Num();
	const int32 NumArms = Control->CreateControlsFromSkeletalMesh(
		Mesh, ArmBones, EPhysicsControlType::ParentSpace,
		MakeAngularControlData(CVarMRMotorArmAngular.GetValueOnGameThread(), CVarMRMotorArmDamping.GetValueOnGameThread()),
		T66MotionRigSets::ParentArms).Num();
	const int32 NumHead = Control->CreateControlsFromSkeletalMesh(
		Mesh, HeadBones, EPhysicsControlType::ParentSpace,
		MakeAngularControlData(CVarMRMotorHeadAngular.GetValueOnGameThread(), CVarMRMotorHeadDamping.GetValueOnGameThread()),
		T66MotionRigSets::ParentHead).Num();

	// World-space pelvis assist: holds the torso onto the animated pose (which
	// lives in component space and therefore follows the bean). This is what
	// keeps the body assembled while parent-space motors do the limbs.
	FPhysicsControlData PelvisData = MakeAngularControlData(
		CVarMRMotorPelvisAngular.GetValueOnGameThread(), CVarMRMotorPelvisDamping.GetValueOnGameThread());
	PelvisData.LinearStrength = CVarMRMotorPelvisLinear.GetValueOnGameThread();
	PelvisData.LinearDampingRatio = CVarMRMotorPelvisDamping.GetValueOnGameThread();
	const int32 NumPelvis = Control->CreateControlsFromSkeletalMesh(
		Mesh, PelvisBones, EPhysicsControlType::WorldSpace, PelvisData, T66MotionRigSets::WorldPelvis).Num();

	bMotorsInitialized = (NumLegs + NumSpine + NumArms + NumHead + NumPelvis) > 0;
	UE_LOG(LogT66MotionRig, Display,
		TEXT("MotionRig motors initialized: legs=%d spine=%d arms=%d head=%d pelvisWorld=%d"),
		NumLegs, NumSpine, NumArms, NumHead, NumPelvis);

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
		// Slightly stiffer in the air so the tuck pose reads.
		Scale.AllScale = 1.15f; Scale.ArmScale = 1.1f; Scale.PelvisWorldScale = 1.f; Scale.RampSeconds = 0.05f;
		break;
	case ET66MotionRigState::Dive:
		// Strong pose hold for the airborne superman; pelvis world assist
		// eases so the bean's pitch-over decides body orientation.
		Scale.AllScale = 1.3f; Scale.ArmScale = 1.4f; Scale.PelvisWorldScale = 0.55f; Scale.RampSeconds = 0.05f;
		break;
	case ET66MotionRigState::Knockdown:
		Scale.AllScale = 0.04f; Scale.ArmScale = 0.02f; Scale.PelvisWorldScale = 0.f; Scale.RampSeconds = 0.f;
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
	TargetAllScale = CurrentAllScale = 0.04f;
	TargetArmScale = CurrentArmScale = 0.02f;
	TargetPelvisWorldScale = CurrentPelvisWorldScale = 0.f;
	RampRatePerSecond = 0.f;
	ApplyGainsAtScale(CurrentAllScale, CurrentArmScale, CurrentPelvisWorldScale);
}

void UT66MotionRigMotorSystem::RefreshBaseGains()
{
	if (bMotorsInitialized)
	{
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
}

void UT66MotionRigMotorSystem::ApplyGainsAtScale(
	const float InAllScale, const float InArmScale, const float InPelvisWorldScale)
{
	if (!Control)
	{
		return;
	}

	const float Global = FMath::Max(0.f, CVarMRMotorGlobalScale.GetValueOnGameThread());
	const float All = InAllScale * Global;

	Control->SetControlDatasInSet(
		T66MotionRigSets::ParentLegs,
		MakeAngularControlData(
			CVarMRMotorLegAngular.GetValueOnGameThread() * All,
			CVarMRMotorLegDamping.GetValueOnGameThread()));
	Control->SetControlDatasInSet(
		T66MotionRigSets::ParentSpine,
		MakeAngularControlData(
			CVarMRMotorSpineAngular.GetValueOnGameThread() * All,
			CVarMRMotorSpineDamping.GetValueOnGameThread()));
	Control->SetControlDatasInSet(
		T66MotionRigSets::ParentHead,
		MakeAngularControlData(
			CVarMRMotorHeadAngular.GetValueOnGameThread() * All,
			CVarMRMotorHeadDamping.GetValueOnGameThread()));
	Control->SetControlDatasInSet(
		T66MotionRigSets::ParentArms,
		MakeAngularControlData(
			CVarMRMotorArmAngular.GetValueOnGameThread() * All * InArmScale,
			CVarMRMotorArmDamping.GetValueOnGameThread()));

	FPhysicsControlData PelvisData = MakeAngularControlData(
		CVarMRMotorPelvisAngular.GetValueOnGameThread() * All * InPelvisWorldScale,
		CVarMRMotorPelvisDamping.GetValueOnGameThread());
	PelvisData.LinearStrength = CVarMRMotorPelvisLinear.GetValueOnGameThread() * All * InPelvisWorldScale;
	PelvisData.LinearDampingRatio = CVarMRMotorPelvisDamping.GetValueOnGameThread();
	Control->SetControlDatasInSet(T66MotionRigSets::WorldPelvis, PelvisData);

	LastAppliedAllScale = InAllScale;
	LastAppliedArmScale = InArmScale;
	LastAppliedPelvisWorldScale = InPelvisWorldScale;
}
