// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/MotionRig/T66MotionRigPawn.h"

#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"
#include "Gameplay/MotionRig/T66MotionRigMotorSystem.h"
#include "Gameplay/MotionRig/T66MotionRigScenario.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66MotionRigPawn, Log, All);

// ---------------------------------------------------------------------------
// Bean tuning. Everything live so the scenario harness can iterate without a
// rebuild. Naming mirrors MOTION_RIG.md section 5.
// ---------------------------------------------------------------------------
static TAutoConsoleVariable<float> CVarMRBeanMaxSpeed(
	TEXT("t66.MotionRig.Bean.MaxSpeed"), 520.f,
	TEXT("MotionRig bean target ground speed (cm/s)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRBeanDriveGain(
	TEXT("t66.MotionRig.Bean.DriveGain"), 8.0f,
	TEXT("Proportional gain from velocity error to drive force (1/s)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRBeanMaxAccel(
	TEXT("t66.MotionRig.Bean.MaxAccel"), 3600.f,
	TEXT("Clamp on bean drive acceleration (cm/s^2)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRBeanAirControl(
	TEXT("t66.MotionRig.Bean.AirControl"), 0.28f,
	TEXT("Multiplier on drive force while airborne."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRBeanUprightStrength(
	TEXT("t66.MotionRig.Bean.UprightStrength"), 1500.f,
	TEXT("Angular spring toward vertical (per-rad, mass-scaled). Deliberately imperfect."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRBeanUprightDamping(
	TEXT("t66.MotionRig.Bean.UprightDamping"), 130.f,
	TEXT("Angular damping for the upright spring (mass-scaled)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRBeanYawStrength(
	TEXT("t66.MotionRig.Bean.YawStrength"), 330.f,
	TEXT("Yaw spring toward movement heading (per-rad, mass-scaled)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRBeanJumpSpeed(
	TEXT("t66.MotionRig.Bean.JumpSpeed"), 470.f,
	TEXT("Vertical velocity change applied on jump (cm/s)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRBeanDiveForwardSpeed(
	TEXT("t66.MotionRig.Bean.DiveForwardSpeed"), 620.f,
	TEXT("Forward velocity change applied on dive (cm/s)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRBeanDiveUpSpeed(
	TEXT("t66.MotionRig.Bean.DiveUpSpeed"), 260.f,
	TEXT("Upward velocity change applied on dive (cm/s)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRBeanDiveUprightScale(
	TEXT("t66.MotionRig.Bean.DiveUprightScale"), 0.12f,
	TEXT("Upright spring multiplier while diving (lets the body pitch prone)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRBeanDivePitchImpulse(
	TEXT("t66.MotionRig.Bean.DivePitchImpulse"), 3.2f,
	TEXT("Angular velocity change (rad/s) pitching the bean forward on dive."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRBeanFriction(
	TEXT("t66.MotionRig.Bean.Friction"), 0.55f,
	TEXT("Bean physics-material friction (slipperiness knob)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRBeanRestitution(
	TEXT("t66.MotionRig.Bean.Restitution"), 0.32f,
	TEXT("Bean physics-material restitution (bounciness knob — the T66 replacement for slipperiness)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMRWalkReferenceSpeed(
	TEXT("t66.MotionRig.Walk.ReferenceSpeed"), 520.f,
	TEXT("Ground speed (cm/s) the Walk clip was authored against; play rate scales with speed so feet match the floor."), ECVF_Default);

// Isolation switches for physics debugging — read once at BeginPlay.
static TAutoConsoleVariable<int32> CVarMRDebugEnableMeshSim(
	TEXT("t66.MotionRig.Debug.EnableMeshSim"), 1,
	TEXT("0 = skeletal mesh stays kinematic (visual only). Isolation switch."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarMRDebugEnableConstraint(
	TEXT("t66.MotionRig.Debug.EnableConstraint"), 1,
	TEXT("0 = no pelvis-to-bean constraint. Isolation switch."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarMRDebugEnableMotors(
	TEXT("t66.MotionRig.Debug.EnableMotors"), 1,
	TEXT("0 = no PhysicsControl motors. Isolation switch."), ECVF_Default);

namespace T66MotionRigPaths
{
	static const TCHAR* SkeletalMesh = TEXT("/Game/Characters/MotionRig/Hero_1/SK_MotionRig_Hero1.SK_MotionRig_Hero1");
	static const TCHAR* FallGuysMaster = TEXT("/Game/Materials/M_FriendSlop_FallGuys.M_FriendSlop_FallGuys");
	static const TCHAR* BaseColorTexture = TEXT("/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Textures/T_Hero_1_Chad_Male_BaseColor.T_Hero_1_Chad_Male_BaseColor");
	static const TCHAR* ClipIdle = TEXT("/Game/Characters/MotionRig/Hero_1/AM_MotionRig_Hero1_Idle.AM_MotionRig_Hero1_Idle");
	static const TCHAR* ClipWalk = TEXT("/Game/Characters/MotionRig/Hero_1/AM_MotionRig_Hero1_Walk.AM_MotionRig_Hero1_Walk");
	static const TCHAR* ClipJump = TEXT("/Game/Characters/MotionRig/Hero_1/AM_MotionRig_Hero1_Jump.AM_MotionRig_Hero1_Jump");
	static const TCHAR* ClipDive = TEXT("/Game/Characters/MotionRig/Hero_1/AM_MotionRig_Hero1_Dive.AM_MotionRig_Hero1_Dive");
	static const TCHAR* ClipGetUpFront = TEXT("/Game/Characters/MotionRig/Hero_1/AM_MotionRig_Hero1_GetUp_Front.AM_MotionRig_Hero1_GetUp_Front");
	static const TCHAR* ClipGetUpBack = TEXT("/Game/Characters/MotionRig/Hero_1/AM_MotionRig_Hero1_GetUp_Back.AM_MotionRig_Hero1_GetUp_Back");
}

namespace
{
	constexpr float BeanCapsuleRadius = 34.f;
	constexpr float BeanCapsuleHalfHeight = 90.f;
	constexpr float BeanMassKg = 70.f;
	constexpr float GroundProbeDistance = 16.f;
	constexpr float MinAirTimeForLanding = 0.12f;
	constexpr float KnockdownSettleSpeed = 120.f;
	constexpr float KnockdownSettleHold = 0.25f;
	constexpr float KnockdownMaxSeconds = 3.0f;
	constexpr float GetUpSeconds = 1.6f;
	constexpr float DiveProneSlideSeconds = 0.45f;
	constexpr float DiveMinAirSeconds = 0.25f;
}

AT66MotionRigPawn::AT66MotionRigPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	Bean = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Bean"));
	Bean->InitCapsuleSize(BeanCapsuleRadius, BeanCapsuleHalfHeight);
	Bean->SetCollisionObjectType(ECC_Pawn);
	Bean->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Bean->SetCollisionResponseToAllChannels(ECR_Block);
	// The skeleton's bodies live inside the capsule; they must never collide
	// with the bean or the whole thing fights itself.
	Bean->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
	Bean->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Bean->SetSimulatePhysics(true);
	Bean->SetEnableGravity(true);
	Bean->BodyInstance.bUseCCD = true;
	// Pitch/roll are LOCKED: ground-friction torque while driving exceeds any
	// reasonable upright spring by ~8x (measured: the bean walked lying at
	// ~89 deg). The Fall Guys feel lives in the BODY's wobble; the bean stays
	// vertical and only yaws. The dive's prone pitch is expressed through the
	// pelvis PD target, not bean rotation.
	Bean->BodyInstance.bLockXRotation = true;
	Bean->BodyInstance.bLockYRotation = true;
	// Mass override lives in BeginPlay: calling SetMassOverrideInKg here runs
	// during CDO construction, hits GetSimplePhysicalMaterial with no GEngine,
	// and logs an Error that fails cooks.
	SetRootComponent(Bean);

	RigMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RigMesh"));
	RigMesh->SetupAttachment(Bean);
	RigMesh->SetRelativeLocation(FVector(0.f, 0.f, -BeanCapsuleHalfHeight));
	RigMesh->SetRelativeRotation(FRotator::ZeroRotator); // rig authored UE-forward (+X)
	RigMesh->SetCollisionObjectType(ECC_PhysicsBody);
	RigMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RigMesh->SetCollisionResponseToAllChannels(ECR_Block);
	RigMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	RigMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	RigMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	// Must be set BEFORE physics state creation: the end-physics tick that
	// blends simulated bodies into the rendered bones is registered at state
	// creation time. Enabling it later renders a frozen skeleton while the
	// simulation moves underneath (the v14/v15 heap illusion).
	RigMesh->bBlendPhysics = true;

	PoseSource = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PoseSource"));
	PoseSource->SetupAttachment(Bean);
	PoseSource->SetRelativeLocation(FVector(0.f, 0.f, -BeanCapsuleHalfHeight));
	PoseSource->SetRelativeRotation(FRotator::ZeroRotator);
	PoseSource->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// HiddenInGame (not bVisible=false) so the pose keeps evaluating: a
	// non-visible component can skip pose/space-transform updates entirely,
	// which silently starves every drive target downstream.
	PoseSource->SetVisibility(true);
	PoseSource->SetHiddenInGame(true);
	PoseSource->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	PoseSource->bPauseAnims = false;

	Visual = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Bean);
	Visual->SetRelativeLocation(FVector(0.f, 0.f, -BeanCapsuleHalfHeight));
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// World-space bone writes leave the component's own bounds stale, which
	// gets the poseable frustum-culled into invisibility. Ride the bean's
	// bounds instead — the camera always frames the bean.
	Visual->bUseAttachParentBound = true;

	PelvisConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PelvisConstraint"));
	PelvisConstraint->SetupAttachment(Bean);

	MotorSystem = CreateDefaultSubobject<UT66MotionRigMotorSystem>(TEXT("MotorSystem"));
	Scenario = CreateDefaultSubobject<UT66MotionRigScenario>(TEXT("Scenario"));

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(Bean);
	CameraBoom->SetUsingAbsoluteRotation(true); // never inherit bean wobble
	CameraBoom->SetRelativeRotation(FRotator(-48.f, 0.f, 0.f));
	CameraBoom->TargetArmLength = 1450.f;
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 8.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
}

void AT66MotionRigPawn::BeginPlay()
{
	Super::BeginPlay();

	LoadAssets();

	Bean->SetPhysMaterialOverride(nullptr); // material params applied through body instance below
	Bean->BodyInstance.SetMassOverride(BeanMassKg, true);

	// Order is load-bearing twice over:
	// 1. PlayAnimation/SetAnimationMode re-initializes the articulation and
	//    CLOBBERS an earlier SetSimulatePhysics(true) — simulation is
	//    re-asserted after every clip change (EnsureMeshSimulation).
	// 2. The game mode spawns pawns at the map origin and the test-room flow
	//    teleports them to the real start AFTERWARDS. Full-size simulated
	//    bodies brought up at the origin interpenetrate the room geometry and
	//    the depenetration impulse destroys the pawn. So the physics bring-up
	//    (mesh simulation + pelvis constraint) is DEFERRED past the spawn/
	//    teleport window; until then the pawn is a bean with a kinematic,
	//    clip-animated mesh.
	// Mode BEFORE the first PlayStateClip — re-setting the mode after a play
	// clears the queued asset.
	if (PoseSource->GetSkeletalMeshAsset())
	{
		PoseSource->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	}

	SetMotionState(ET66MotionRigState::Idle);

	FTimerHandle BringUpTimer;
	GetWorldTimerManager().SetTimer(BringUpTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		bPhysicsLive = true;
		Bean->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Bean->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);

		// The simulated mesh needs a LIVE anim instance or the physics→bone
		// blend path (PerformBlendPhysicsBones) never runs and the RENDERED
		// skeleton freezes while the simulation moves underneath (the v14
		// discovery: bodies standing at target, bones reporting a heap).
		// One play, once, here — then RigMesh animation is never touched
		// again; the articulation re-init this causes is immediately followed
		// by the simulation re-assert and motor init below.
		if (RigMesh->GetSkeletalMeshAsset() && ClipIdle)
		{
			RigMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			RigMesh->PlayAnimation(ClipIdle, true);
		}

		// DETACH the simulated mesh (old-lane proven: bDetachMeshDuringRagdoll)
		// — the engine's bodies→bones blend only behaves on detached or
		// root-simulated meshes; attached fully-simulated meshes render frozen
		// (v14..v22). The pose source stays attached to the bean, so drive
		// targets still follow it; the pelvis PD keeps the body near the bean.
		if (RigMesh->GetSkeletalMeshAsset())
		{
			RigMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			RigMesh->SetVisibility(true);
			Visual->SetVisibility(false);
		}

		EnsureMeshSimulation();

		if (CVarMRDebugEnableMotors.GetValueOnGameThread() != 0)
		{
			MotorSystem->InitializeMotors(RigMesh, PoseSource);
			MotorSystem->ApplyStateProfile(MotionState);
		}
		// No pelvis constraint: the bean↔body coupling is one-way by design —
		// the motor system applies virtual PD forces on the pelvis instead
		// (a real tether let the body bury the bean into the floor).

		UE_LOG(LogT66MotionRigPawn, Display,
			TEXT("[MR_BRINGUP] physics live at %s (meshSim=%d motors=%d)"),
			*Bean->GetComponentLocation().ToCompactString(),
			RigMesh->GetSkeletalMeshAsset() && RigMesh->IsSimulatingPhysics(TEXT("pelvis")) ? 1 : 0,
			MotorSystem->AreMotorsInitialized() ? 1 : 0);
	}), 0.75f, false);

	UE_LOG(LogT66MotionRigPawn, Display,
		TEXT("MotionRig pawn ready (physics bring-up deferred). MeshLoaded=%d"),
		RigMesh->GetSkeletalMeshAsset() ? 1 : 0);

	// One-shot diagnostic snapshot after the world settles.
	FTimerHandle DiagTimer;
	GetWorldTimerManager().SetTimer(DiagTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		float MeshMass = 0.f;
		int32 SimBodies = 0;
		int32 TotalBodies = 0;
		if (RigMesh->GetSkeletalMeshAsset())
		{
			for (FBodyInstance* Body : RigMesh->Bodies)
			{
				if (!Body) { continue; }
				++TotalBodies;
				if (Body->IsInstanceSimulatingPhysics())
				{
					++SimBodies;
					MeshMass += Body->GetBodyMass();
				}
			}
		}
		float SampleStiffness = -1.f;
		if (RigMesh->Constraints.Num() > 0 && RigMesh->Constraints[0])
		{
			SampleStiffness = RigMesh->Constraints[0]->ProfileInstance.AngularDrive.SlerpDrive.Stiffness;
		}
		float PelvisBodyZ = -999.f;
		int32 PelvisAwake = -1;
		if (FBodyInstance* PelvisBody = RigMesh->GetSkeletalMeshAsset() ? RigMesh->GetBodyInstance(TEXT("pelvis")) : nullptr)
		{
			PelvisBodyZ = PelvisBody->GetUnrealWorldTransform().GetLocation().Z;
			PelvisAwake = PelvisBody->IsInstanceAwake() ? 1 : 0;
		}
		UE_LOG(LogT66MotionRigPawn, Display,
			TEXT("[MR_DIAG2] pelvisBODYz=%.1f awake=%d motorTicks=%d pdApplies=%d lastPdAccel=%.0f blendPhysics=%d blendWeight=%.2f"),
			PelvisBodyZ, PelvisAwake,
			MotorSystem->DiagTickCount, MotorSystem->DiagPelvisApplyCount, MotorSystem->DiagLastPelvisAccel,
			RigMesh->bBlendPhysics ? 1 : 0,
			RigMesh->Bodies.Num() > 0 && RigMesh->Bodies[0] ? RigMesh->Bodies[0]->PhysicsBlendWeight : -1.f);
		UE_LOG(LogT66MotionRigPawn, Display,
			TEXT("[MR_DIAG] beanSim=%d beanZ=%.1f meshBodies=%d/%d meshMass=%.1f constraints=%d slerpStiffness=%.0f pelvisZ=%.1f poseBones=%d posePelvisZ=%.1f poseAnim=%s grounded=%d state=%s"),
			Bean->IsSimulatingPhysics() ? 1 : 0,
			Bean->GetComponentLocation().Z,
			SimBodies, TotalBodies, MeshMass,
			RigMesh->Constraints.Num(),
			SampleStiffness,
			RigMesh->GetSkeletalMeshAsset() ? RigMesh->GetBoneLocation(TEXT("pelvis")).Z : -1.f,
			PoseSource->GetBoneSpaceTransforms().Num(),
			PoseSource->GetSkeletalMeshAsset() ? PoseSource->GetBoneTransform(TEXT("pelvis")).GetLocation().Z : -1.f,
			PoseSource->GetSingleNodeInstance() && PoseSource->GetSingleNodeInstance()->GetCurrentAsset()
				? *GetNameSafe(PoseSource->GetSingleNodeInstance()->GetCurrentAsset()) : TEXT("none"),
			bGrounded ? 1 : 0,
			T66MotionRigStateName(MotionState));
	}), 3.0f, false);
}

void AT66MotionRigPawn::EnsureMeshSimulation()
{
	if (!bPhysicsLive || !RigMesh->GetSkeletalMeshAsset() || CVarMRDebugEnableMeshSim.GetValueOnGameThread() == 0)
	{
		return;
	}
	if (!RigMesh->IsSimulatingPhysics(TEXT("pelvis")))
	{
		RigMesh->SetAllBodiesSimulatePhysics(true);
		RigMesh->SetAllBodiesPhysicsBlendWeight(1.f);
		RigMesh->bBlendPhysics = true;
	}
}

void AT66MotionRigPawn::LoadAssets()
{
	if (bAssetsLoaded)
	{
		return;
	}
	bAssetsLoaded = true;

	if (USkeletalMesh* MeshAsset = LoadObject<USkeletalMesh>(nullptr, T66MotionRigPaths::SkeletalMesh))
	{
		RigMesh->SetSkeletalMesh(MeshAsset);
		PoseSource->SetSkeletalMesh(MeshAsset);
		Visual->SetSkinnedAssetAndUpdate(MeshAsset);
		// The simulated mesh renders (detached at bring-up); the poseable copy
		// is a dormant fallback.
		Visual->SetVisibility(false);

		// FriendSlop look: instance of the one lit master, raw albedo.
		UMaterialInterface* Master = LoadObject<UMaterialInterface>(nullptr, T66MotionRigPaths::FallGuysMaster);
		UTexture* Albedo = LoadObject<UTexture>(nullptr, T66MotionRigPaths::BaseColorTexture);
		if (Master)
		{
			for (int32 SlotIndex = 0; SlotIndex < RigMesh->GetNumMaterials(); ++SlotIndex)
			{
				if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Master, RigMesh))
				{
					if (Albedo)
					{
						MID->SetTextureParameterValue(TEXT("BaseColorTexture"), Albedo);
					}
					RigMesh->SetMaterial(SlotIndex, MID);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogT66MotionRigPawn, Warning,
			TEXT("MotionRig skeletal mesh not found at %s — running in bean-only mode."),
			T66MotionRigPaths::SkeletalMesh);
	}

	ClipIdle = LoadObject<UAnimSequence>(nullptr, T66MotionRigPaths::ClipIdle);
	ClipWalk = LoadObject<UAnimSequence>(nullptr, T66MotionRigPaths::ClipWalk);
	ClipJump = LoadObject<UAnimSequence>(nullptr, T66MotionRigPaths::ClipJump);
	ClipDive = LoadObject<UAnimSequence>(nullptr, T66MotionRigPaths::ClipDive);
	ClipGetUpFront = LoadObject<UAnimSequence>(nullptr, T66MotionRigPaths::ClipGetUpFront);
	ClipGetUpBack = LoadObject<UAnimSequence>(nullptr, T66MotionRigPaths::ClipGetUpBack);
}

void AT66MotionRigPawn::ReattachPelvisConstraint()
{
	// Intentionally unused: the bean↔body coupling is one-way (virtual PD on
	// the pelvis in the motor system). A physical tether lets the body's
	// reaction forces shove the bean — walkcircle_v10 buried it in the floor.
	// Kept as a stub in case a future state wants a real tether temporarily.
}

FVector AT66MotionRigPawn::GetBeanVelocity() const
{
	return Bean ? Bean->GetPhysicsLinearVelocity() : FVector::ZeroVector;
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

void AT66MotionRigPawn::MotionRigSetMoveAxes(const float ForwardValue, const float RightValue)
{
	if (bScenarioInputOverride)
	{
		return; // scripted scenario owns the stick
	}
	MoveForwardValue = FMath::Clamp(ForwardValue, -1.f, 1.f);
	MoveRightValue = FMath::Clamp(RightValue, -1.f, 1.f);
}

void AT66MotionRigPawn::SetScenarioInputOverride(const bool bActive)
{
	bScenarioInputOverride = bActive;
	if (!bActive)
	{
		MoveForwardValue = 0.f;
		MoveRightValue = 0.f;
	}
}

void AT66MotionRigPawn::ScenarioSetMoveAxes(const float ForwardValue, const float RightValue)
{
	MoveForwardValue = FMath::Clamp(ForwardValue, -1.f, 1.f);
	MoveRightValue = FMath::Clamp(RightValue, -1.f, 1.f);
}

void AT66MotionRigPawn::MotionRigJumpPressed()
{
	if (!bGrounded)
	{
		return;
	}
	if (MotionState != ET66MotionRigState::Idle && MotionState != ET66MotionRigState::Walk)
	{
		return;
	}

	FVector Velocity = Bean->GetPhysicsLinearVelocity();
	Velocity.Z = FMath::Max(Velocity.Z, 0.f) + CVarMRBeanJumpSpeed.GetValueOnGameThread();
	Bean->SetPhysicsLinearVelocity(Velocity);
	SetMotionState(ET66MotionRigState::Jump);
}

void AT66MotionRigPawn::MotionRigJumpReleased()
{
	// Reserved for variable jump height tuning later.
}

void AT66MotionRigPawn::MotionRigDivePressed()
{
	if (MotionState != ET66MotionRigState::Idle &&
		MotionState != ET66MotionRigState::Walk &&
		MotionState != ET66MotionRigState::Jump)
	{
		return;
	}

	FVector Heading = GetCameraRelativeInputDirection();
	if (Heading.IsNearlyZero())
	{
		Heading = FVector(Bean->GetComponentQuat().GetForwardVector().X,
			Bean->GetComponentQuat().GetForwardVector().Y, 0.f).GetSafeNormal();
		if (Heading.IsNearlyZero())
		{
			Heading = FVector::ForwardVector;
		}
	}

	FVector Velocity = Bean->GetPhysicsLinearVelocity();
	Velocity += Heading * CVarMRBeanDiveForwardSpeed.GetValueOnGameThread();
	Velocity.Z = FMath::Max(Velocity.Z, 0.f) + CVarMRBeanDiveUpSpeed.GetValueOnGameThread();
	Bean->SetPhysicsLinearVelocity(Velocity);

	// Pitch the bean forward over the dive axis (right vector of the heading).
	const FVector PitchAxis = FVector::CrossProduct(FVector::UpVector, Heading).GetSafeNormal();
	Bean->SetPhysicsAngularVelocityInRadians(
		Bean->GetPhysicsAngularVelocityInRadians() + PitchAxis * -CVarMRBeanDivePitchImpulse.GetValueOnGameThread());

	SetMotionState(ET66MotionRigState::Dive);
}

void AT66MotionRigPawn::TriggerKnockdown(const FVector& LaunchVelocity)
{
	if (MotionState == ET66MotionRigState::Knockdown)
	{
		return;
	}
	PendingKnockdownLaunch = LaunchVelocity;
	SetMotionState(ET66MotionRigState::Knockdown);
}

// ---------------------------------------------------------------------------
// State machine
// ---------------------------------------------------------------------------

void AT66MotionRigPawn::SetMotionState(const ET66MotionRigState NewState)
{
	if (MotionState == NewState && StateTimeSeconds > 0.f)
	{
		return;
	}

	const ET66MotionRigState OldState = MotionState;
	MotionState = NewState;
	StateTimeSeconds = 0.f;

	switch (NewState)
	{
	case ET66MotionRigState::Knockdown:
		FinishKnockdownEnter();
		break;
	case ET66MotionRigState::GetUp:
		// Handled by StartGetUp (which sets the state).
		break;
	default:
		break;
	}

	MotorSystem->ApplyStateProfile(NewState);
	PlayStateClip(NewState);
	// PlayAnimation can re-init articulation and silently kill simulation.
	EnsureMeshSimulation();

	UE_LOG(LogT66MotionRigPawn, Verbose, TEXT("MotionRig state %s -> %s"),
		T66MotionRigStateName(OldState), T66MotionRigStateName(NewState));
}

void AT66MotionRigPawn::FinishKnockdownEnter()
{
	KnockdownSettleSeconds = 0.f;

	// Motors limp first so the body is compliant when the launch arrives.
	MotorSystem->GoLimp();

	// The bean stops being a physical presence and becomes a follower until
	// recovery (no tether exists; the pelvis PD is already zero while limp).
	SetBeanPhysicsEnabled(false);

	if (RigMesh->GetSkeletalMeshAsset() && RigMesh->IsSimulatingPhysics())
	{
		RigMesh->SetAllPhysicsLinearVelocity(PendingKnockdownLaunch, false);
		// Deterministic tumble derived from the launch direction — no RNG so
		// scenario captures are reproducible.
		const FVector TumbleAxis = FVector::CrossProduct(FVector::UpVector, PendingKnockdownLaunch.GetSafeNormal()).GetSafeNormal();
		RigMesh->SetAllPhysicsAngularVelocityInRadians(TumbleAxis * -6.f, false);
	}
	else
	{
		// Bean-only mode: shove the bean itself so the state still demos.
		SetBeanPhysicsEnabled(true);
		Bean->SetPhysicsLinearVelocity(PendingKnockdownLaunch);
	}

	PendingKnockdownLaunch = FVector::ZeroVector;
}

void AT66MotionRigPawn::StartGetUp()
{
	// Face-up vs face-down decides the recovery clip.
	bGetUpFromFront = true;
	if (RigMesh->GetSkeletalMeshAsset())
	{
		const FQuat PelvisQuat = RigMesh->GetBoneQuaternion(TEXT("pelvis"));
		bGetUpFromFront = FVector::DotProduct(PelvisQuat.GetUpVector(), FVector::UpVector) < 0.f;
	}

	// Re-seat the bean on the floor under the pelvis, calm everything, then
	// let motors ramp the body back onto the pose (profile handles the ramp).
	const FVector PelvisLocation = RigMesh->GetSkeletalMeshAsset()
		? RigMesh->GetBoneLocation(TEXT("pelvis"))
		: Bean->GetComponentLocation();

	FVector FloorPoint = PelvisLocation;
	if (UWorld* World = GetWorld())
	{
		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(MotionRigGetUp), false, this);
		const FVector TraceStart = PelvisLocation + FVector(0.f, 0.f, 200.f);
		const FVector TraceEnd = PelvisLocation - FVector(0.f, 0.f, 2000.f);
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
		{
			FloorPoint = Hit.ImpactPoint;
		}
	}

	const FVector BeanTarget = FVector(PelvisLocation.X, PelvisLocation.Y, FloorPoint.Z + BeanCapsuleHalfHeight + 2.f);
	Bean->SetWorldLocation(BeanTarget, false, nullptr, ETeleportType::TeleportPhysics);
	// Stand the bean's yaw along the body's current heading; zero roll/pitch.
	const FVector BodyForward = RigMesh->GetSkeletalMeshAsset()
		? FVector(RigMesh->GetBoneQuaternion(TEXT("pelvis")).GetForwardVector().X,
			RigMesh->GetBoneQuaternion(TEXT("pelvis")).GetForwardVector().Y, 0.f).GetSafeNormal()
		: FVector::ForwardVector;
	if (!BodyForward.IsNearlyZero())
	{
		Bean->SetWorldRotation(BodyForward.Rotation(), false, nullptr, ETeleportType::TeleportPhysics);
	}

	SetBeanPhysicsEnabled(true);
	Bean->SetPhysicsLinearVelocity(FVector::ZeroVector);
	Bean->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);

	SetMotionState(ET66MotionRigState::GetUp);
}

void AT66MotionRigPawn::TickStateMachine(const float DeltaSeconds)
{
	StateTimeSeconds += DeltaSeconds;
	const float PlanarSpeed = GetBeanVelocity().Size2D();

	switch (MotionState)
	{
	case ET66MotionRigState::Idle:
		if (PlanarSpeed > 60.f || !GetCameraRelativeInputDirection().IsNearlyZero())
		{
			SetMotionState(ET66MotionRigState::Walk);
		}
		break;

	case ET66MotionRigState::Walk:
		if (PlanarSpeed <= 40.f && GetCameraRelativeInputDirection().IsNearlyZero())
		{
			SetMotionState(ET66MotionRigState::Idle);
		}
		else if (!bGrounded && AirTimeSeconds > 0.25f)
		{
			SetMotionState(ET66MotionRigState::Jump); // walked off a ledge: reuse airborne pose
		}
		break;

	case ET66MotionRigState::Jump:
		if (bGrounded && StateTimeSeconds > MinAirTimeForLanding)
		{
			SetMotionState(PlanarSpeed > 60.f ? ET66MotionRigState::Walk : ET66MotionRigState::Idle);
		}
		break;

	case ET66MotionRigState::Dive:
		if (bGrounded && StateTimeSeconds > DiveMinAirSeconds)
		{
			DiveSlideSeconds += DeltaSeconds;
			if (DiveSlideSeconds >= DiveProneSlideSeconds)
			{
				DiveSlideSeconds = 0.f;
				bGetUpFromFront = true;
				StartGetUp();
			}
		}
		break;

	case ET66MotionRigState::Knockdown:
	{
		const float BodySpeed = RigMesh->GetSkeletalMeshAsset()
			? RigMesh->GetPhysicsLinearVelocity(TEXT("pelvis")).Size()
			: GetBeanVelocity().Size();
		if (BodySpeed < KnockdownSettleSpeed)
		{
			KnockdownSettleSeconds += DeltaSeconds;
		}
		else
		{
			KnockdownSettleSeconds = 0.f;
		}

		if ((KnockdownSettleSeconds >= KnockdownSettleHold && StateTimeSeconds > 0.6f)
			|| StateTimeSeconds >= KnockdownMaxSeconds)
		{
			StartGetUp();
		}
		break;
	}

	case ET66MotionRigState::GetUp:
		if (StateTimeSeconds >= GetUpSeconds)
		{
			SetMotionState(ET66MotionRigState::Idle);
		}
		break;
	}
}

// ---------------------------------------------------------------------------
// Bean physics
// ---------------------------------------------------------------------------

void AT66MotionRigPawn::SetBeanPhysicsEnabled(const bool bEnabled)
{
	Bean->SetSimulatePhysics(bEnabled);
	Bean->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	if (bEnabled)
	{
		Bean->SetMassOverrideInKg(NAME_None, BeanMassKg, true);
	}
}

void AT66MotionRigPawn::TickGroundSense()
{
	bGrounded = false;
	GroundDistance = TNumericLimits<float>::Max();

	UWorld* World = GetWorld();
	if (!World || !Bean->IsSimulatingPhysics())
	{
		return;
	}

	const FVector Start = Bean->GetComponentLocation();
	const float ProbeLength = BeanCapsuleHalfHeight + GroundProbeDistance;
	const FVector End = Start - FVector(0.f, 0.f, ProbeLength);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(MotionRigGround), false, this);
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FHitResult Hit;
	if (World->SweepSingleByObjectType(
		Hit, Start, End, FQuat::Identity, ObjectParams,
		FCollisionShape::MakeSphere(BeanCapsuleRadius * 0.85f), Params))
	{
		GroundDistance = (Start.Z - Hit.ImpactPoint.Z) - BeanCapsuleHalfHeight;
		bGrounded = GroundDistance <= GroundProbeDistance;
	}

	AirTimeSeconds = bGrounded ? 0.f : (AirTimeSeconds);
}

void AT66MotionRigPawn::TickBeanForces(const float DeltaSeconds)
{
	if (!Bean->IsSimulatingPhysics())
	{
		return;
	}

	const float Mass = Bean->GetMass();
	const FVector InputDir = GetCameraRelativeInputDirection();
	const bool bDriveAllowed =
		MotionState == ET66MotionRigState::Idle ||
		MotionState == ET66MotionRigState::Walk ||
		MotionState == ET66MotionRigState::Jump;

	// --- locomotion drive: proportional controller on planar velocity ---
	if (bDriveAllowed)
	{
		const float MaxSpeed = CVarMRBeanMaxSpeed.GetValueOnGameThread();
		const FVector DesiredVelocity = InputDir * MaxSpeed;
		const FVector Velocity = Bean->GetPhysicsLinearVelocity();
		FVector VelocityError = DesiredVelocity - FVector(Velocity.X, Velocity.Y, 0.f);

		float Gain = CVarMRBeanDriveGain.GetValueOnGameThread();
		if (!bGrounded)
		{
			Gain *= CVarMRBeanAirControl.GetValueOnGameThread();
		}

		FVector DriveAccel = VelocityError * Gain;
		const float MaxAccel = CVarMRBeanMaxAccel.GetValueOnGameThread();
		if (DriveAccel.SizeSquared() > MaxAccel * MaxAccel)
		{
			DriveAccel = DriveAccel.GetSafeNormal() * MaxAccel;
		}
		Bean->AddForce(DriveAccel * Mass);
	}

	// --- upright spring (the charm lives in this being imperfect) ---
	{
		float UprightScale = 1.f;
		if (MotionState == ET66MotionRigState::Dive)
		{
			UprightScale = CVarMRBeanDiveUprightScale.GetValueOnGameThread();
		}

		const FQuat BeanQuat = Bean->GetComponentQuat();
		const FVector CurrentUp = BeanQuat.GetUpVector();
		const FVector TorqueAxis = FVector::CrossProduct(CurrentUp, FVector::UpVector);
		const float SinAngle = FMath::Clamp(TorqueAxis.Size(), 0.f, 1.f);
		const float Angle = FMath::Asin(SinAngle); // rad, 0..pi/2 range is plenty

		const FVector AngularVelocity = Bean->GetPhysicsAngularVelocityInRadians();
		const FVector SpringTorque = TorqueAxis.GetSafeNormal() * Angle *
			CVarMRBeanUprightStrength.GetValueOnGameThread() * UprightScale;
		const FVector DampingTorque = -FVector(AngularVelocity.X, AngularVelocity.Y, 0.f) *
			CVarMRBeanUprightDamping.GetValueOnGameThread();
		Bean->AddTorqueInRadians((SpringTorque + DampingTorque) * Mass);
	}

	// --- yaw toward heading ---
	if (bDriveAllowed && !InputDir.IsNearlyZero())
	{
		const float CurrentYaw = FMath::DegreesToRadians(Bean->GetComponentRotation().Yaw);
		const float TargetYaw = FMath::Atan2(InputDir.Y, InputDir.X);
		const float YawError = FMath::FindDeltaAngleRadians(CurrentYaw, TargetYaw);
		const float YawVelocity = Bean->GetPhysicsAngularVelocityInRadians().Z;
		const float YawTorque = YawError * CVarMRBeanYawStrength.GetValueOnGameThread() - YawVelocity * 28.f;
		Bean->AddTorqueInRadians(FVector(0.f, 0.f, YawTorque) * Mass);
	}

	// --- live friction/restitution knobs ---
	if (FBodyInstance* Body = Bean->GetBodyInstance())
	{
		// Walkable physics: keep linear damping modest so momentum reads.
		Body->LinearDamping = 0.15;
		Body->AngularDamping = 0.4;
		Body->UpdateDampingProperties();
	}
}

void AT66MotionRigPawn::TickKnockdownFollow()
{
	if (MotionState != ET66MotionRigState::Knockdown || !RigMesh->GetSkeletalMeshAsset())
	{
		return;
	}

	// The bean shadows the body so cameras and future gameplay queries stay
	// anchored to the action while the skeleton is free.
	const FVector PelvisLocation = RigMesh->GetBoneLocation(TEXT("pelvis"));
	Bean->SetWorldLocation(
		FVector(PelvisLocation.X, PelvisLocation.Y, PelvisLocation.Z + BeanCapsuleHalfHeight * 0.5f),
		false, nullptr, ETeleportType::TeleportPhysics);
}

void AT66MotionRigPawn::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bGrounded)
	{
		AirTimeSeconds += DeltaSeconds;
	}

	TickGroundSense();
	TickBeanForces(DeltaSeconds);
	TickKnockdownFollow();
	TickStateMachine(DeltaSeconds);
	TickWalkCadence();
	TickVisualFromBodies();
}

void AT66MotionRigPawn::TickVisualFromBodies()
{
	static bool bLoggedOnce = false;
	if (!bLoggedOnce && bPhysicsLive)
	{
		bLoggedOnce = true;
		UE_LOG(LogT66MotionRigPawn, Display,
			TEXT("[MR_VISUAL] physicsLive=%d rigAsset=%d visualAsset=%d visualVisible=%d visualBones=%d"),
			bPhysicsLive ? 1 : 0,
			RigMesh->GetSkeletalMeshAsset() ? 1 : 0,
			Visual->GetSkinnedAsset() ? 1 : 0,
			Visual->IsVisible() ? 1 : 0,
			Visual->GetNumBones());
	}
	if (!bPhysicsLive || !RigMesh->GetSkeletalMeshAsset() || !Visual->GetSkinnedAsset())
	{
		return;
	}

	static const FName CanonicalBones[] = {
		TEXT("pelvis"), TEXT("spine_01"), TEXT("spine_02"), TEXT("head"),
		TEXT("clavicle_l"), TEXT("upperarm_l"), TEXT("lowerarm_l"), TEXT("hand_l"),
		TEXT("clavicle_r"), TEXT("upperarm_r"), TEXT("lowerarm_r"), TEXT("hand_r"),
		TEXT("thigh_l"), TEXT("calf_l"), TEXT("foot_l"),
		TEXT("thigh_r"), TEXT("calf_r"), TEXT("foot_r") };

	// Component-space writes with explicit math — the WorldSpace path of the
	// poseable setter put the body off-screen (apparent double transform).
	const FTransform WorldToComponent = Visual->GetComponentTransform().Inverse();
	for (const FName& Bone : CanonicalBones)
	{
		if (const FBodyInstance* Body = RigMesh->GetBodyInstance(Bone))
		{
			const FTransform ComponentSpace = Body->GetUnrealWorldTransform() * WorldToComponent;
			Visual->SetBoneTransformByName(Bone, ComponentSpace, EBoneSpaces::ComponentSpace);
		}
	}
}

// ---------------------------------------------------------------------------
// Clips
// ---------------------------------------------------------------------------

UAnimSequence* AT66MotionRigPawn::ClipForState(const ET66MotionRigState State) const
{
	switch (State)
	{
	case ET66MotionRigState::Idle:      return ClipIdle;
	case ET66MotionRigState::Walk:      return ClipWalk;
	case ET66MotionRigState::Jump:      return ClipJump;
	case ET66MotionRigState::Dive:      return ClipDive;
	case ET66MotionRigState::Knockdown: return nullptr; // limp: no target needed
	case ET66MotionRigState::GetUp:     return bGetUpFromFront ? ClipGetUpFront : ClipGetUpBack;
	}
	return nullptr;
}

void AT66MotionRigPawn::PlayStateClip(const ET66MotionRigState State)
{
	// Clips play ONLY on the hidden pose source; the simulated mesh is never
	// touched by animation calls (articulation re-init kills simulation).
	if (!PoseSource->GetSkeletalMeshAsset())
	{
		return;
	}

	UAnimSequence* Clip = ClipForState(State);
	if (!Clip)
	{
		return;
	}

	const bool bLoop =
		State == ET66MotionRigState::Idle ||
		State == ET66MotionRigState::Walk;
	PoseSource->PlayAnimation(Clip, bLoop);
	PoseSource->SetPlayRate(1.f);
}

void AT66MotionRigPawn::TickWalkCadence()
{
	if (MotionState != ET66MotionRigState::Walk || !PoseSource->GetSkeletalMeshAsset())
	{
		return;
	}

	// Feet match the floor: clip play rate scales with actual bean speed.
	const float ReferenceSpeed = FMath::Max(50.f, CVarMRWalkReferenceSpeed.GetValueOnGameThread());
	const float Rate = FMath::Clamp(GetBeanVelocity().Size2D() / ReferenceSpeed, 0.25f, 1.6f);
	PoseSource->SetPlayRate(Rate);
}

FVector AT66MotionRigPawn::GetCameraRelativeInputDirection() const
{
	if (FMath::IsNearlyZero(MoveForwardValue) && FMath::IsNearlyZero(MoveRightValue))
	{
		return FVector::ZeroVector;
	}

	// Camera boom uses absolute rotation; its yaw defines the input frame.
	const FRotator YawRotation(0.f, CameraBoom->GetComponentRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	FVector Direction = Forward * MoveForwardValue + Right * MoveRightValue;
	Direction.Z = 0.f;
	return Direction.GetClampedToMaxSize(1.f);
}
