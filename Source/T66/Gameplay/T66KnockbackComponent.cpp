// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66KnockbackComponent.h"

#include "CollisionQueryParams.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Knockback, Log, All);

namespace
{
	FName T66FirstExistingBone(const USkeletalMeshComponent* MeshComponent, const TArrayView<const FName> Candidates)
	{
		if (!MeshComponent)
		{
			return NAME_None;
		}

		for (const FName Candidate : Candidates)
		{
			if (MeshComponent->GetBoneIndex(Candidate) != INDEX_NONE)
			{
				return Candidate;
			}
		}

		return NAME_None;
	}

	FName T66FirstExistingPhysicsBody(const USkeletalMeshComponent* MeshComponent, const TArrayView<const FName> Candidates)
	{
		const UPhysicsAsset* PhysicsAsset = MeshComponent ? MeshComponent->GetPhysicsAsset() : nullptr;
		if (!MeshComponent || !PhysicsAsset)
		{
			return NAME_None;
		}

		for (const FName Candidate : Candidates)
		{
			if (MeshComponent->GetBoneIndex(Candidate) != INDEX_NONE
				&& PhysicsAsset->FindBodyIndex(Candidate) != INDEX_NONE)
			{
				return Candidate;
			}
		}

		if (PhysicsAsset->SkeletalBodySetups.Num() > 0 && PhysicsAsset->SkeletalBodySetups[0])
		{
			return PhysicsAsset->SkeletalBodySetups[0]->BoneName;
		}

		return NAME_None;
	}

	FVector T66ClampLaunchVelocity(const FVector& LaunchVelocity, const FT66KnockbackProfile& Profile)
	{
		FVector Result = LaunchVelocity * FMath::Max(0.f, Profile.LaunchVelocityScale);
		const float MaxSpeed = FMath::Max(0.f, Profile.MaxLaunchVelocity);
		if (MaxSpeed > KINDA_SMALL_NUMBER && Result.SizeSquared() > FMath::Square(MaxSpeed))
		{
			Result = Result.GetClampedToMaxSize(MaxSpeed);
		}
		return Result;
	}
}

UT66KnockbackComponent::UT66KnockbackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UT66KnockbackComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
}

void UT66KnockbackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Phase != ET66KnockbackPhase::Inactive)
	{
		RestoreFromKnockback();
	}

	if (PhysicalAnimationComponent)
	{
		PhysicalAnimationComponent->DestroyComponent();
		PhysicalAnimationComponent = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

ACharacter* UT66KnockbackComponent::ResolveCharacterOwner() const
{
	return Cast<ACharacter>(GetOwner());
}

USkeletalMeshComponent* UT66KnockbackComponent::ResolveSkeletalMesh() const
{
	if (const ACharacter* Character = ResolveCharacterOwner())
	{
		return Character->GetMesh();
	}

	return nullptr;
}

bool UT66KnockbackComponent::ApplyKnockbackLaunch(const FVector& LaunchVelocity, const FT66KnockbackProfile* ProfileOverride)
{
	ACharacter* Character = ResolveCharacterOwner();
	UWorld* World = GetWorld();
	if (!Character || !World || LaunchVelocity.IsNearlyZero())
	{
		return false;
	}

	if (Phase != ET66KnockbackPhase::Inactive)
	{
		return false;
	}

	ActiveProfile = ProfileOverride ? *ProfileOverride : DefaultProfile;
	ActiveProfile.MinIncapacitationSeconds = FMath::Clamp(ActiveProfile.MinIncapacitationSeconds, 0.01f, 20.f);
	ActiveProfile.MaxRagdollSeconds = FMath::Clamp(
		ActiveProfile.MaxRagdollSeconds,
		ActiveProfile.MinIncapacitationSeconds + 0.05f,
		30.f);
	ActiveProfile.SettleSpeed = FMath::Clamp(ActiveProfile.SettleSpeed, 1.f, 5000.f);
	ActiveProfile.SettleHoldSeconds = FMath::Clamp(ActiveProfile.SettleHoldSeconds, 0.f, 5.f);
	ActiveProfile.RecoveryBlendOutSeconds = FMath::Clamp(ActiveProfile.RecoveryBlendOutSeconds, 0.01f, 5.f);
	ActiveProfile.FloorPenetrationSkin = FMath::Clamp(ActiveProfile.FloorPenetrationSkin, 0.f, 100.f);
	ActiveProfile.FloorTraceUpDistance = FMath::Clamp(ActiveProfile.FloorTraceUpDistance, 100.f, 5000.f);
	ActiveProfile.FloorTraceDownDistance = FMath::Clamp(ActiveProfile.FloorTraceDownDistance, 100.f, 10000.f);
	ActiveProfile.MaxFloorCorrectionPerTick = FMath::Clamp(ActiveProfile.MaxFloorCorrectionPerTick, 0.f, 10000.f);
	ActiveProfile.RagdollLinearDampingOverride = FMath::Max(-1.f, ActiveProfile.RagdollLinearDampingOverride);
	ActiveProfile.RagdollAngularDampingOverride = FMath::Max(-1.f, ActiveProfile.RagdollAngularDampingOverride);
	ActiveProfile.RagdollFrictionOverride = FMath::Max(-1.f, ActiveProfile.RagdollFrictionOverride);
	ActiveProfile.RagdollRestitutionOverride = ActiveProfile.RagdollRestitutionOverride >= 0.f
		? FMath::Clamp(ActiveProfile.RagdollRestitutionOverride, 0.f, 1.f)
		: -1.f;
	const bool bPhysicalAnimationRequested =
		ActiveProfile.bEnablePhysicalAnimation
		&& ActiveProfile.PhysicalAnimationDriveMode != ET66KnockbackPhysicalAnimationDriveMode::Disabled;
	const bool bHeroKnockbackProfile =
		ActiveProfile.BudgetClass == ET66KnockbackBudgetClass::Hero
		|| Character->IsA<AT66HeroBase>();
	if (bPhysicalAnimationRequested && bHeroKnockbackProfile)
	{
		UE_LOG(
			LogT66Knockback,
			Display,
			TEXT("T66Knockback disabling physical animation for hero ragdoll: Owner=%s DriveMode=%d"),
			*GetNameSafe(Character),
			static_cast<int32>(ActiveProfile.PhysicalAnimationDriveMode));
		ActiveProfile.bEnablePhysicalAnimation = false;
		ActiveProfile.PhysicalAnimationDriveMode = ET66KnockbackPhysicalAnimationDriveMode::Disabled;
	}
	else if (bPhysicalAnimationRequested && ActiveProfile.bDetachMeshDuringRagdoll)
	{
		UE_LOG(
			LogT66Knockback,
			Display,
			TEXT("T66Knockback disabling physical animation for detached ragdoll: Owner=%s DriveMode=%d"),
			*GetNameSafe(Character),
			static_cast<int32>(ActiveProfile.PhysicalAnimationDriveMode));
		ActiveProfile.bEnablePhysicalAnimation = false;
		ActiveProfile.PhysicalAnimationDriveMode = ET66KnockbackPhysicalAnimationDriveMode::Disabled;
	}

	const FVector ResolvedLaunchVelocity = T66ClampLaunchVelocity(LaunchVelocity, ActiveProfile);
	const double Now = World->GetTimeSeconds();
	KnockbackStartedTimeSeconds = Now;
	ControlRestoreTimeSeconds = Now + ActiveProfile.MinIncapacitationSeconds;
	ForceRecoverTimeSeconds = Now + ActiveProfile.MaxRagdollSeconds;
	LowVelocityStartedTimeSeconds = -9999.0;
	RecoverStartedTimeSeconds = -9999.0;
	PhysicalAnimationActivationTimeSeconds = -9999.0;
	bPhysicalAnimationActivationPending = false;
	bDetachedMesh = false;
	bUsingSkeletalRagdoll = false;
	PhysicalAnimationDrivenBodyCount = 0;
	PreImpactBodyPhysicsSettings.Reset();
	ActiveRagdollPhysicalMaterial = nullptr;
	PreImpactFloorZ = Character->GetCapsuleComponent()
		? Character->GetActorLocation().Z - Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		: Character->GetActorLocation().Z;
	LastResolvedFloorZ = PreImpactFloorZ;
	LastFloorGuardLogTimeSeconds = -9999.0;
	LastActorFollowSampleLogTimeSeconds = -9999.0;
	LastFollowDivergenceLogTimeSeconds = -9999.0;
	bHasResolvedFloorZ = false;

	ApplyGameplaySuppression(true);

	USkeletalMeshComponent* MeshComponent = ResolveSkeletalMesh();
	const bool bCanUseSkeletalRagdoll = ActiveProfile.bEnableSkeletalRagdoll
		&& MeshComponent
		&& MeshComponent->GetSkeletalMeshAsset()
		&& MeshComponent->GetPhysicsAsset();

	if (bCanUseSkeletalRagdoll && TryBeginSkeletalRagdoll(Character, MeshComponent, ResolvedLaunchVelocity, ActiveProfile))
	{
		SetComponentTickEnabled(true);
		return true;
	}

	BeginFallbackLaunch(Character, ResolvedLaunchVelocity, ActiveProfile);
	SetComponentTickEnabled(true);
	return true;
}

void UT66KnockbackComponent::ApplyGameplaySuppression(const bool bSuppress)
{
	ACharacter* Character = ResolveCharacterOwner();
	if (!Character)
	{
		bGameplaySuppressed = bSuppress;
		return;
	}

	const bool bWasGameplaySuppressed = bGameplaySuppressed;
	bGameplaySuppressed = bSuppress;
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(Character); Hero && Hero->CombatComponent)
	{
		if (bSuppress)
		{
			if (!bWasGameplaySuppressed)
			{
				bPreImpactAutoAttackSuppressed = Hero->CombatComponent->IsAutoAttackSuppressed();
			}
			Hero->CombatComponent->SetAutoAttackSuppressed(true);
		}
		else if (bWasGameplaySuppressed)
		{
			Hero->CombatComponent->SetAutoAttackSuppressed(bPreImpactAutoAttackSuppressed);
			bPreImpactAutoAttackSuppressed = false;
		}
	}

	if (AController* Controller = Character->GetController())
	{
		if (bSuppress)
		{
			if (!bAppliedMoveInputSuppression)
			{
				Controller->SetIgnoreMoveInput(true);
				bAppliedMoveInputSuppression = true;
			}
		}
		else
		{
			if (bAppliedMoveInputSuppression)
			{
				Controller->SetIgnoreMoveInput(false);
				bAppliedMoveInputSuppression = false;
			}
		}
	}
}

bool UT66KnockbackComponent::TryBeginSkeletalRagdoll(
	ACharacter* Character,
	USkeletalMeshComponent* MeshComponent,
	const FVector& LaunchVelocity,
	const FT66KnockbackProfile& Profile)
{
	if (!Character || !MeshComponent)
	{
		return false;
	}

	SimulationRootBoneName = ResolveSimulationRootBone(MeshComponent, Profile);
	FollowBoneName = ResolveFollowBone(MeshComponent, Profile);
	VelocityBoneName = ResolveVelocityBone(MeshComponent, Profile);

	ActiveMesh = MeshComponent;
	Phase = ET66KnockbackPhase::Active;
	bUsingSkeletalRagdoll = true;
	PreImpactActorLocation = Character->GetActorLocation();
	PreImpactMeshRelativeLocation = MeshComponent->GetRelativeLocation();
	PreImpactMeshRelativeRotation = MeshComponent->GetRelativeRotation();
	PreImpactMeshRelativeScale = MeshComponent->GetRelativeScale3D();
	PreImpactAttachParent = MeshComponent->GetAttachParent();
	PreImpactAttachSocketName = MeshComponent->GetAttachSocketName();
	PreImpactMeshCollisionEnabled = MeshComponent->GetCollisionEnabled();
	PreImpactMeshCollisionProfileName = MeshComponent->GetCollisionProfileName();
	bPreImpactMeshHiddenInGame = MeshComponent->bHiddenInGame;
	bPreImpactMeshVisible = MeshComponent->IsVisible();
	PreImpactFloorZ = Character->GetCapsuleComponent()
		? Character->GetActorLocation().Z - Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		: Character->GetActorLocation().Z;
	LastResolvedFloorZ = PreImpactFloorZ;
	bHasResolvedFloorZ = false;
	float TracedPreImpactFloorZ = PreImpactFloorZ;
	if (TraceRagdollFloorZAtLocation(Character, Character->GetActorLocation(), TracedPreImpactFloorZ))
	{
		PreImpactFloorZ = TracedPreImpactFloorZ;
		LastResolvedFloorZ = TracedPreImpactFloorZ;
		bHasResolvedFloorZ = true;
	}

	const FVector FollowLocation = GetFollowLocation(MeshComponent);
	ActorToFollowBoneOffset = Profile.bUsePreImpactActorToFollowBoneOffset
		? Character->GetActorLocation() - FollowLocation
		: FVector::ZeroVector;

	if (Profile.bDetachMeshDuringRagdoll)
	{
		MeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		bDetachedMesh = true;
	}

	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		PreImpactMovementMode = Movement->MovementMode;
		PreImpactCustomMovementMode = Movement->CustomMovementMode;
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		PreImpactCapsuleCollisionEnabled = Capsule->GetCollisionEnabled();
		PreImpactCapsuleCollisionProfileName = Capsule->GetCollisionProfileName();
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetEnableGravity(true);
	MeshComponent->RecreatePhysicsState();
	SyncKinematicMeshPoseToPhysics(MeshComponent);
	MeshComponent->SetAllUseCCD(true);
	CacheBodyPhysicsSettings(MeshComponent);

	if (Profile.bSimulateAllPhysicsBodies)
	{
		MeshComponent->SetAllBodiesSimulatePhysics(true);
		MeshComponent->SetAllBodiesPhysicsBlendWeight(1.f);
	}
	else if (!SimulationRootBoneName.IsNone())
	{
		MeshComponent->SetAllBodiesBelowSimulatePhysics(SimulationRootBoneName, true, true);
		MeshComponent->SetAllBodiesBelowPhysicsBlendWeight(SimulationRootBoneName, 1.f, false, true);
	}
	else
	{
		MeshComponent->SetAllBodiesSimulatePhysics(true);
		MeshComponent->SetAllBodiesPhysicsBlendWeight(1.f);
	}
	ApplyRagdollPhysicsResponseProfile(MeshComponent, Profile);
	MeshComponent->WakeAllRigidBodies();

	FBox InitialBodyBounds(EForceInit::ForceInit);
	if (ComputeSimulatedBodyBounds(MeshComponent, InitialBodyBounds))
	{
		const float CapsuleHalfHeight = Character->GetCapsuleComponent()
			? Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
			: 100.f;
		FVector TargetBodyCenter = Character->GetActorLocation();
		TargetBodyCenter.Z = FMath::Max(TargetBodyCenter.Z, PreImpactFloorZ + CapsuleHalfHeight - 10.f);
		const FVector InitialBodyCenter = InitialBodyBounds.GetCenter();
		const bool bBodyCenterDetachedFromActor = FVector::DistSquared2D(InitialBodyCenter, TargetBodyCenter) > FMath::Square(250.f);
		const bool bBodiesStartedDeepBelowFloor = InitialBodyBounds.Min.Z < PreImpactFloorZ - 50.f;
		if (bBodyCenterDetachedFromActor || bBodiesStartedDeepBelowFloor)
		{
			const FVector BodyOffset = TargetBodyCenter - InitialBodyCenter;
			for (FBodyInstance* BodyInstance : MeshComponent->Bodies)
			{
				if (!BodyInstance || !BodyInstance->IsValidBodyInstance() || !BodyInstance->bSimulatePhysics)
				{
					continue;
				}

				FTransform BodyTransform = BodyInstance->GetUnrealWorldTransform();
				if (BodyTransform.ContainsNaN())
				{
					continue;
				}

				BodyTransform.AddToTranslation(BodyOffset);
				BodyInstance->SetBodyTransform(BodyTransform, ETeleportType::TeleportPhysics, true);
				BodyInstance->SetLinearVelocity(FVector::ZeroVector, false, true);
			}

			MeshComponent->UpdateBounds();
			UE_LOG(
				LogT66Knockback,
				Display,
				TEXT("T66Knockback initial body resync: Owner=%s Actor=%s BodyCenter=%s TargetCenter=%s Offset=%s BodyMinZ=%.1f FloorZ=%.1f"),
				*GetNameSafe(Character),
				*Character->GetActorLocation().ToCompactString(),
				*InitialBodyCenter.ToCompactString(),
				*TargetBodyCenter.ToCompactString(),
				*BodyOffset.ToCompactString(),
				InitialBodyBounds.Min.Z,
				PreImpactFloorZ);
		}
	}

	if (Profile.bEnablePhysicalAnimation
		&& Profile.PhysicalAnimationDriveMode != ET66KnockbackPhysicalAnimationDriveMode::Disabled)
	{
		bPhysicalAnimationActivationPending = true;
		PhysicalAnimationActivationTimeSeconds = (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0)
			+ FMath::Clamp(Profile.PhysicalAnimationActivationDelaySeconds, 0.0f, 2.0f);
	}

	ApplyLaunchImpulse(MeshComponent, LaunchVelocity, Profile);

	const UPhysicsAsset* PhysicsAsset = MeshComponent->GetPhysicsAsset();
	UE_LOG(
		LogT66Knockback,
		Display,
		TEXT("T66Knockback skeletal launch: Owner=%s Velocity=%s MinIncap=%.2fs MaxRagdoll=%.2fs SimulationRoot=%s FollowBone=%s VelocityBone=%s SimulateAllBodies=%d CenterActorOnFollow=%d SuppressLook=%d VelocityChange=%d BelowFraction=%.2f LinearDamping=%.2f AngularDamping=%.2f Friction=%.2f Restitution=%.2f PACPending=%d DriveMode=%d RuntimeBodies=%d PhysicsBodies=%d PhysicsConstraints=%d PhysicsAsset=%s"),
		*GetNameSafe(Character),
		*LaunchVelocity.ToCompactString(),
		Profile.MinIncapacitationSeconds,
		Profile.MaxRagdollSeconds,
		*SimulationRootBoneName.ToString(),
		*FollowBoneName.ToString(),
		*VelocityBoneName.ToString(),
		Profile.bSimulateAllPhysicsBodies ? 1 : 0,
		Profile.bUsePreImpactActorToFollowBoneOffset ? 0 : 1,
		Profile.bSuppressLookInput ? 1 : 0,
		Profile.bTreatLaunchVectorAsVelocityChange ? 1 : 0,
		Profile.BelowBodiesImpulseFraction,
		Profile.RagdollLinearDampingOverride,
		Profile.RagdollAngularDampingOverride,
		Profile.RagdollFrictionOverride,
		Profile.RagdollRestitutionOverride,
		bPhysicalAnimationActivationPending ? 1 : 0,
		static_cast<int32>(Profile.PhysicalAnimationDriveMode),
		MeshComponent->Bodies.Num(),
		PhysicsAsset ? PhysicsAsset->SkeletalBodySetups.Num() : 0,
		PhysicsAsset ? PhysicsAsset->ConstraintSetup.Num() : 0,
		PhysicsAsset ? *PhysicsAsset->GetPathName() : TEXT("(none)"));

	return true;
}

void UT66KnockbackComponent::BeginFallbackLaunch(ACharacter* Character, const FVector& LaunchVelocity, const FT66KnockbackProfile& Profile)
{
	if (!Character)
	{
		return;
	}

	Phase = ET66KnockbackPhase::Active;
	bUsingSkeletalRagdoll = false;
	PreImpactActorLocation = Character->GetActorLocation();
	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		PreImpactMovementMode = Movement->MovementMode;
		PreImpactCustomMovementMode = Movement->CustomMovementMode;
	}

	Character->LaunchCharacter(LaunchVelocity, true, true);
	UE_LOG(
		LogT66Knockback,
		Display,
		TEXT("T66Knockback fallback launch: Owner=%s Velocity=%s MinIncap=%.2fs SkeletalRagdoll=0"),
		*GetNameSafe(Character),
		*LaunchVelocity.ToCompactString(),
		Profile.MinIncapacitationSeconds);
}

void UT66KnockbackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Phase == ET66KnockbackPhase::Inactive)
	{
		SetComponentTickEnabled(false);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const double Now = World->GetTimeSeconds();
	if (bUsingSkeletalRagdoll)
	{
		UpdateActiveKnockback(Now);
		UpdateRecovery(Now);
	}
	else if (Now >= ControlRestoreTimeSeconds)
	{
		RestoreFromKnockback();
	}
}

void UT66KnockbackComponent::UpdateActiveKnockback(const double Now)
{
	if (Phase == ET66KnockbackPhase::Inactive)
	{
		return;
	}

	ACharacter* Character = ResolveCharacterOwner();
	USkeletalMeshComponent* MeshComponent = ActiveMesh.Get();
	if (!Character || !MeshComponent)
	{
		RestoreFromKnockback();
		return;
	}

	TryActivatePhysicalAnimation(Now);
	EnforceFloorPenetrationGuard(Character, MeshComponent);

	if (ActiveProfile.bFollowActorToRagdoll)
	{
		const FVector RawFollowLocation = GetFollowLocation(MeshComponent);
		const FVector ActorFollowLocation = ResolveActorFollowLocation(MeshComponent);
		FVector TargetActorLocation = ActorFollowLocation + ActorToFollowBoneOffset;
		TargetActorLocation.Z = ResolveActorFloorAnchorZ(Character, MeshComponent);
		if (ActiveProfile.MaxActorFollowHeightAboveStart > KINDA_SMALL_NUMBER)
		{
			TargetActorLocation.Z = FMath::Min(TargetActorLocation.Z, PreImpactActorLocation.Z + ActiveProfile.MaxActorFollowHeightAboveStart);
		}

		if (!TargetActorLocation.ContainsNaN())
		{
			const FTransform MeshWorldTransformBeforeActorFollow = MeshComponent->GetComponentTransform();
			Character->SetActorLocation(TargetActorLocation, false, nullptr, ETeleportType::TeleportPhysics);
			if (!bDetachedMesh)
			{
				MeshComponent->SetWorldLocationAndRotation(
					MeshWorldTransformBeforeActorFollow.GetLocation(),
					MeshWorldTransformBeforeActorFollow.GetRotation(),
					false,
					nullptr,
					ETeleportType::TeleportPhysics);
			}

			const double LogTime = GetWorld() ? GetWorld()->GetTimeSeconds() : Now;
			if (LogTime - LastActorFollowSampleLogTimeSeconds >= 0.35)
			{
				LastActorFollowSampleLogTimeSeconds = LogTime;
				const float ActorToFollowXY = FVector::Dist2D(Character->GetActorLocation(), ActorFollowLocation + ActorToFollowBoneOffset);
				UE_LOG(
					LogT66Knockback,
					Display,
					TEXT("T66Knockback actor follow sample: Owner=%s Actor=%s Target=%s BodyCenter=%s BoneFollow=%s ActorToTargetXY=%.1f"),
					*GetNameSafe(Character),
					*Character->GetActorLocation().ToCompactString(),
					*TargetActorLocation.ToCompactString(),
					*ActorFollowLocation.ToCompactString(),
					*RawFollowLocation.ToCompactString(),
					ActorToFollowXY);
			}

			const float FollowDivergenceSq = FVector::DistSquared2D(RawFollowLocation, ActorFollowLocation);
			if (FollowDivergenceSq > FMath::Square(450.f))
			{
				if (LogTime - LastFollowDivergenceLogTimeSeconds >= 0.35)
				{
					LastFollowDivergenceLogTimeSeconds = LogTime;
					UE_LOG(
						LogT66Knockback,
						Display,
						TEXT("T66Knockback actor follow using simulated body center: Owner=%s Actor=%s BoneFollow=%s BodyCenter=%s Divergence2D=%.1f"),
						*GetNameSafe(Character),
						*Character->GetActorLocation().ToCompactString(),
						*RawFollowLocation.ToCompactString(),
						*ActorFollowLocation.ToCompactString(),
						FMath::Sqrt(FollowDivergenceSq));
				}
			}
		}
	}

	if (Phase != ET66KnockbackPhase::Active)
	{
		return;
	}

	const FVector PhysicsVelocity = MeshComponent->GetPhysicsLinearVelocity(VelocityBoneName);
	const float PhysicsSpeed = PhysicsVelocity.Size();
	const bool bPastMinIncap = Now >= ControlRestoreTimeSeconds;
	const bool bUnderSettleSpeed = PhysicsSpeed <= ActiveProfile.SettleSpeed;
	if (bPastMinIncap && bUnderSettleSpeed)
	{
		if (LowVelocityStartedTimeSeconds < 0.0)
		{
			LowVelocityStartedTimeSeconds = Now;
		}
	}
	else
	{
		LowVelocityStartedTimeSeconds = -9999.0;
	}

	const bool bSettled = LowVelocityStartedTimeSeconds >= 0.0
		&& (Now - LowVelocityStartedTimeSeconds) >= ActiveProfile.SettleHoldSeconds;
	const bool bForcedRecovery = Now >= ForceRecoverTimeSeconds;
	if (bPastMinIncap && (bSettled || bForcedRecovery))
	{
		Phase = ET66KnockbackPhase::Recovering;
		RecoverStartedTimeSeconds = Now;
		UE_LOG(
			LogT66Knockback,
			Display,
			TEXT("T66Knockback recovery started: Owner=%s Speed=%.1f Settled=%d Forced=%d FollowBone=%s Actor=%s"),
			*GetNameSafe(Character),
			PhysicsSpeed,
			bSettled ? 1 : 0,
			bForcedRecovery ? 1 : 0,
			*FollowBoneName.ToString(),
			*Character->GetActorLocation().ToCompactString());
	}
}

void UT66KnockbackComponent::UpdateRecovery(const double Now)
{
	if (Phase != ET66KnockbackPhase::Recovering)
	{
		return;
	}

	USkeletalMeshComponent* MeshComponent = ActiveMesh.Get();
	if (!MeshComponent)
	{
		RestoreFromKnockback();
		return;
	}

	const float RecoverAlpha = FMath::Clamp(
		static_cast<float>((Now - RecoverStartedTimeSeconds) / FMath::Max(0.01f, ActiveProfile.RecoveryBlendOutSeconds)),
		0.f,
		1.f);
	const float PhysicsBlend = 1.f - RecoverAlpha;
	MeshComponent->SetAllBodiesPhysicsBlendWeight(PhysicsBlend);
	if (PhysicalAnimationComponent)
	{
		PhysicalAnimationComponent->SetStrengthMultiplyer(FMath::Max(0.f, ActiveProfile.PhysicalAnimationStrength) * PhysicsBlend);
	}

	if (RecoverAlpha >= 1.f)
	{
		RestoreFromKnockback();
	}
}

void UT66KnockbackComponent::RestoreFromKnockback()
{
	ACharacter* Character = ResolveCharacterOwner();
	USkeletalMeshComponent* MeshComponent = ActiveMesh.Get();
	FVector RestoreCarryVelocity = FVector::ZeroVector;
	bool bHasRestoreCarryVelocity = false;
	if (MeshComponent && bUsingSkeletalRagdoll && !VelocityBoneName.IsNone())
	{
		RestoreCarryVelocity = MeshComponent->GetPhysicsLinearVelocity(VelocityBoneName);
		bHasRestoreCarryVelocity = !RestoreCarryVelocity.ContainsNaN() && !RestoreCarryVelocity.IsNearlyZero();
		if (bHasRestoreCarryVelocity)
		{
			const float MaxCarrySpeed = FMath::Max(0.f, ActiveProfile.MaxLaunchVelocity);
			if (MaxCarrySpeed > KINDA_SMALL_NUMBER)
			{
				RestoreCarryVelocity = RestoreCarryVelocity.GetClampedToMaxSize(MaxCarrySpeed);
			}
		}
	}

	ApplyGameplaySuppression(false);

	if (PhysicalAnimationComponent)
	{
		PhysicalAnimationComponent->SetStrengthMultiplyer(0.f);
	}

	if (Character && MeshComponent && bUsingSkeletalRagdoll)
	{
		if (!FollowBoneName.IsNone())
		{
			EnforceFloorPenetrationGuard(Character, MeshComponent);
			FVector TargetActorLocation = ResolveActorFollowLocation(MeshComponent) + ActorToFollowBoneOffset;
			TargetActorLocation.Z = ResolveActorFloorAnchorZ(Character, MeshComponent);
			if (!TargetActorLocation.ContainsNaN())
			{
				const FTransform MeshWorldTransformBeforeActorFollow = MeshComponent->GetComponentTransform();
				Character->SetActorLocation(TargetActorLocation, false, nullptr, ETeleportType::None);
				if (!bDetachedMesh)
				{
					MeshComponent->SetWorldLocationAndRotation(
						MeshWorldTransformBeforeActorFollow.GetLocation(),
						MeshWorldTransformBeforeActorFollow.GetRotation(),
						false,
						nullptr,
						ETeleportType::None);
				}
			}
		}

		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			const bool bRestoreFalling = bHasRestoreCarryVelocity && RestoreCarryVelocity.Z > 180.f;
			Movement->StopMovementImmediately();
			Movement->SetMovementMode(
				bRestoreFalling ? MOVE_Falling : (PreImpactMovementMode != MOVE_None ? PreImpactMovementMode : MOVE_Walking),
				bRestoreFalling ? 0 : PreImpactCustomMovementMode);
			if (bHasRestoreCarryVelocity)
			{
				if (!bRestoreFalling && RestoreCarryVelocity.Z < 0.f)
				{
					RestoreCarryVelocity.Z = 0.f;
				}
				Movement->Velocity = RestoreCarryVelocity;
			}
		}

		if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
		{
			Capsule->SetCollisionProfileName(PreImpactCapsuleCollisionProfileName);
			Capsule->SetCollisionEnabled(PreImpactCapsuleCollisionEnabled);
		}
	}

	if (MeshComponent && bUsingSkeletalRagdoll)
	{
		RestoreBodyPhysicsSettings(MeshComponent);
		if (bDetachedMesh)
		{
			USceneComponent* AttachParent = PreImpactAttachParent.Get();
			if (!AttachParent && Character)
			{
				AttachParent = Character->GetCapsuleComponent();
			}
			if (AttachParent)
			{
				MeshComponent->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepWorldTransform, PreImpactAttachSocketName);
			}
		}

		MeshComponent->SetAllUseCCD(false);
		MeshComponent->SetAllBodiesPhysicsBlendWeight(0.f);
		MeshComponent->SetAllBodiesSimulatePhysics(false);
		MeshComponent->ResetAllBodiesSimulatePhysics();
		MeshComponent->SetCollisionProfileName(PreImpactMeshCollisionProfileName);
		MeshComponent->SetCollisionEnabled(PreImpactMeshCollisionEnabled);
		MeshComponent->SetGenerateOverlapEvents(false);
		MeshComponent->SetRelativeLocationAndRotation(
			PreImpactMeshRelativeLocation,
			PreImpactMeshRelativeRotation,
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
		MeshComponent->SetRelativeScale3D(PreImpactMeshRelativeScale);
		SyncKinematicMeshPoseToPhysics(MeshComponent);
		MeshComponent->SetHiddenInGame(bPreImpactMeshHiddenInGame, true);
		MeshComponent->SetVisibility(bPreImpactMeshVisible, true);
	}

	if (Character)
	{
		UE_LOG(LogT66Knockback, Display, TEXT("T66Knockback restored: Owner=%s Actor=%s CarryVelocity=%s"),
			*GetNameSafe(Character),
			*Character->GetActorLocation().ToCompactString(),
			*RestoreCarryVelocity.ToCompactString());
	}

	ResetRuntimeState();
}

void UT66KnockbackComponent::ResetRuntimeState()
{
	ActiveMesh = nullptr;
	PreImpactAttachParent = nullptr;
	ActiveRagdollPhysicalMaterial = nullptr;
	ActiveProfile = DefaultProfile;
	PreImpactBodyPhysicsSettings.Reset();
	Phase = ET66KnockbackPhase::Inactive;
	KnockbackStartedTimeSeconds = -9999.0;
	ControlRestoreTimeSeconds = -9999.0;
	ForceRecoverTimeSeconds = -9999.0;
	LowVelocityStartedTimeSeconds = -9999.0;
	RecoverStartedTimeSeconds = -9999.0;
	PhysicalAnimationActivationTimeSeconds = -9999.0;
	SimulationRootBoneName = NAME_None;
	FollowBoneName = NAME_None;
	VelocityBoneName = NAME_None;
	PreImpactAttachSocketName = NAME_None;
	ActorToFollowBoneOffset = FVector::ZeroVector;
	PreImpactActorLocation = FVector::ZeroVector;
	PreImpactFloorZ = 0.f;
	LastResolvedFloorZ = 0.f;
	LastFloorGuardLogTimeSeconds = -9999.0;
	LastActorFollowSampleLogTimeSeconds = -9999.0;
	LastFollowDivergenceLogTimeSeconds = -9999.0;
	bHasResolvedFloorZ = false;
	PreImpactMeshRelativeLocation = FVector::ZeroVector;
	PreImpactMeshRelativeRotation = FRotator::ZeroRotator;
	PreImpactMeshRelativeScale = FVector::OneVector;
	PreImpactMovementMode = MOVE_Walking;
	PreImpactCustomMovementMode = 0;
	PreImpactCapsuleCollisionEnabled = ECollisionEnabled::QueryAndPhysics;
	PreImpactCapsuleCollisionProfileName = FName(TEXT("Pawn"));
	PreImpactMeshCollisionEnabled = ECollisionEnabled::NoCollision;
	PreImpactMeshCollisionProfileName = NAME_None;
	bPreImpactMeshHiddenInGame = false;
	bPreImpactMeshVisible = true;
	bPreImpactAutoAttackSuppressed = false;
	bAppliedMoveInputSuppression = false;
	bPhysicalAnimationActivationPending = false;
	bDetachedMesh = false;
	bUsingSkeletalRagdoll = false;
	PhysicalAnimationDrivenBodyCount = 0;
	SetComponentTickEnabled(false);
}

FName UT66KnockbackComponent::ResolveSimulationRootBone(const USkeletalMeshComponent* MeshComponent, const FT66KnockbackProfile& Profile) const
{
	const UPhysicsAsset* PhysicsAsset = MeshComponent ? MeshComponent->GetPhysicsAsset() : nullptr;
	if (!MeshComponent || !PhysicsAsset)
	{
		return NAME_None;
	}

	if (!Profile.SimulationRootBoneName.IsNone()
		&& MeshComponent->GetBoneIndex(Profile.SimulationRootBoneName) != INDEX_NONE
		&& PhysicsAsset->FindBodyIndex(Profile.SimulationRootBoneName) != INDEX_NONE)
	{
		return Profile.SimulationRootBoneName;
	}

	static const FName CandidateBodies[] =
	{
		FName(TEXT("pelvis")),
		FName(TEXT("spine_01")),
		FName(TEXT("spine_02")),
		FName(TEXT("spine_03")),
		FName(TEXT("DEF-spine")),
		FName(TEXT("DEF-spine.001")),
		FName(TEXT("DEF-spine.002")),
		FName(TEXT("root"))
	};

	return T66FirstExistingPhysicsBody(MeshComponent, MakeArrayView(CandidateBodies));
}

FName UT66KnockbackComponent::ResolveFollowBone(const USkeletalMeshComponent* MeshComponent, const FT66KnockbackProfile& Profile) const
{
	if (!MeshComponent)
	{
		return NAME_None;
	}

	if (!Profile.FollowBoneName.IsNone() && MeshComponent->GetBoneIndex(Profile.FollowBoneName) != INDEX_NONE)
	{
		return Profile.FollowBoneName;
	}

	static const FName CandidateBones[] =
	{
		FName(TEXT("pelvis")),
		FName(TEXT("spine_02")),
		FName(TEXT("spine_01")),
		FName(TEXT("spine_03")),
		FName(TEXT("DEF-spine.002")),
		FName(TEXT("DEF-spine.001")),
		FName(TEXT("DEF-spine")),
		FName(TEXT("ORG-spine")),
		FName(TEXT("root")),
		FName(TEXT("DEF-pelvis.L")),
		FName(TEXT("DEF-pelvis.R"))
	};

	return T66FirstExistingBone(MeshComponent, MakeArrayView(CandidateBones));
}

FName UT66KnockbackComponent::ResolveVelocityBone(const USkeletalMeshComponent* MeshComponent, const FT66KnockbackProfile& Profile) const
{
	const UPhysicsAsset* PhysicsAsset = MeshComponent ? MeshComponent->GetPhysicsAsset() : nullptr;
	if (!MeshComponent || !PhysicsAsset)
	{
		return NAME_None;
	}

	if (!Profile.VelocityBoneName.IsNone()
		&& MeshComponent->GetBoneIndex(Profile.VelocityBoneName) != INDEX_NONE
		&& PhysicsAsset->FindBodyIndex(Profile.VelocityBoneName) != INDEX_NONE)
	{
		return Profile.VelocityBoneName;
	}

	static const FName CandidateBodies[] =
	{
		FName(TEXT("pelvis")),
		FName(TEXT("spine_02")),
		FName(TEXT("spine_01")),
		FName(TEXT("spine_03")),
		FName(TEXT("thigh_l")),
		FName(TEXT("thigh_r")),
		FName(TEXT("DEF-spine.002")),
		FName(TEXT("DEF-spine.001")),
		FName(TEXT("DEF-spine")),
		FName(TEXT("ORG-spine")),
		FName(TEXT("root")),
		FName(TEXT("DEF-thigh.L")),
		FName(TEXT("DEF-thigh.R"))
	};

	return T66FirstExistingPhysicsBody(MeshComponent, MakeArrayView(CandidateBodies));
}

FVector UT66KnockbackComponent::GetFollowLocation(const USkeletalMeshComponent* MeshComponent) const
{
	if (!MeshComponent)
	{
		return FVector::ZeroVector;
	}

	if (!FollowBoneName.IsNone() && MeshComponent->GetBoneIndex(FollowBoneName) != INDEX_NONE)
	{
		return MeshComponent->GetSocketLocation(FollowBoneName);
	}

	return MeshComponent->GetComponentLocation();
}

bool UT66KnockbackComponent::ComputeSimulatedBodyBounds(const USkeletalMeshComponent* MeshComponent, FBox& OutBounds) const
{
	OutBounds = FBox(EForceInit::ForceInit);
	if (!MeshComponent)
	{
		return false;
	}

	int32 ValidBodyCount = 0;
	for (const FBodyInstance* BodyInstance : MeshComponent->Bodies)
	{
		if (!BodyInstance || !BodyInstance->IsValidBodyInstance() || !BodyInstance->bSimulatePhysics)
		{
			continue;
		}

		const FBox BodyBounds = BodyInstance->GetBodyBounds();
		if (!BodyBounds.IsValid || BodyBounds.ContainsNaN())
		{
			continue;
		}

		OutBounds += BodyBounds;
		++ValidBodyCount;
	}

	return ValidBodyCount > 0 && OutBounds.IsValid && !OutBounds.ContainsNaN();
}

FVector UT66KnockbackComponent::ResolveActorFollowLocation(const USkeletalMeshComponent* MeshComponent) const
{
	if (ActiveProfile.bUseSimulatedBodyCenterForActorFollow)
	{
		FBox BodyBounds(EForceInit::ForceInit);
		if (ComputeSimulatedBodyBounds(MeshComponent, BodyBounds))
		{
			return BodyBounds.GetCenter();
		}
	}

	return GetFollowLocation(MeshComponent);
}

bool UT66KnockbackComponent::TraceRagdollFloorZAtLocation(
	const ACharacter* Character,
	const FVector& ProbeLocation,
	float& OutFloorZ) const
{
	UWorld* World = GetWorld();
	if (!World || !Character)
	{
		return false;
	}

	const float TraceUp = FMath::Max(100.f, ActiveProfile.FloorTraceUpDistance);
	const float TraceDown = FMath::Max(100.f, ActiveProfile.FloorTraceDownDistance);
	const float StartZ = FMath::Min(
		FMath::Max(ProbeLocation.Z + 120.f, PreImpactFloorZ + 120.f),
		PreImpactFloorZ + FMath::Min(TraceUp, 240.f));
	const FVector TraceStart(ProbeLocation.X, ProbeLocation.Y, StartZ);
	const FVector TraceEnd(ProbeLocation.X, ProbeLocation.Y, PreImpactFloorZ - TraceDown);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66KnockbackFloorGuard), false, Character);
	Params.bFindInitialOverlaps = false;

	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params)
		&& Hit.ImpactNormal.Z > 0.35f
		&& Hit.ImpactPoint.Z <= PreImpactFloorZ + 180.f)
	{
		OutFloorZ = Hit.ImpactPoint.Z;
		return true;
	}

	return false;
}

bool UT66KnockbackComponent::ResolveRagdollFloorZ(
	const ACharacter* Character,
	const USkeletalMeshComponent* MeshComponent,
	float& OutFloorZ)
{
	FVector ProbeLocation = Character ? Character->GetActorLocation() : FVector::ZeroVector;
	FBox BodyBounds(EForceInit::ForceInit);
	if (MeshComponent && ComputeSimulatedBodyBounds(MeshComponent, BodyBounds))
	{
		ProbeLocation.X = BodyBounds.GetCenter().X;
		ProbeLocation.Y = BodyBounds.GetCenter().Y;
		ProbeLocation.Z = BodyBounds.GetCenter().Z;
	}

	if (Character && TraceRagdollFloorZAtLocation(Character, ProbeLocation, OutFloorZ))
	{
		LastResolvedFloorZ = OutFloorZ;
		bHasResolvedFloorZ = true;
		return true;
	}

	if (bHasResolvedFloorZ)
	{
		OutFloorZ = LastResolvedFloorZ;
		return true;
	}

	OutFloorZ = PreImpactFloorZ;
	return true;
}

float UT66KnockbackComponent::ResolveActorFloorAnchorZ(ACharacter* Character, USkeletalMeshComponent* MeshComponent)
{
	const float CapsuleHalfHeight = Character && Character->GetCapsuleComponent()
		? Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		: 100.f;

	float FloorZ = PreImpactFloorZ;
	if (!ResolveRagdollFloorZ(Character, MeshComponent, FloorZ))
	{
		FloorZ = PreImpactFloorZ;
	}

	return FloorZ + CapsuleHalfHeight;
}

float UT66KnockbackComponent::EnforceFloorPenetrationGuard(ACharacter* Character, USkeletalMeshComponent* MeshComponent)
{
	if (!ActiveProfile.bEnableFloorPenetrationGuard || !Character || !MeshComponent)
	{
		return 0.f;
	}

	FBox BodyBounds(EForceInit::ForceInit);
	if (!ComputeSimulatedBodyBounds(MeshComponent, BodyBounds))
	{
		return 0.f;
	}

	float FloorZ = PreImpactFloorZ;
	if (!ResolveRagdollFloorZ(Character, MeshComponent, FloorZ))
	{
		return 0.f;
	}

	const float AllowedMinZ = FloorZ + FMath::Max(0.f, ActiveProfile.FloorPenetrationSkin);
	const float Penetration = AllowedMinZ - BodyBounds.Min.Z;
	if (Penetration <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}

	const float MaxCorrection = ActiveProfile.MaxFloorCorrectionPerTick > KINDA_SMALL_NUMBER
		? ActiveProfile.MaxFloorCorrectionPerTick
		: Penetration;
	const float CorrectionZ = FMath::Min(Penetration, MaxCorrection);
	const FVector Correction(0.f, 0.f, CorrectionZ);

	for (FBodyInstance* BodyInstance : MeshComponent->Bodies)
	{
		if (!BodyInstance || !BodyInstance->IsValidBodyInstance() || !BodyInstance->bSimulatePhysics)
		{
			continue;
		}

		FTransform BodyTransform = BodyInstance->GetUnrealWorldTransform();
		if (BodyTransform.ContainsNaN())
		{
			continue;
		}

		BodyTransform.AddToTranslation(Correction);
		BodyInstance->SetBodyTransform(BodyTransform, ETeleportType::TeleportPhysics, true);

		FVector Velocity = BodyInstance->GetUnrealWorldVelocity();
		if (!Velocity.ContainsNaN() && Velocity.Z < 0.f)
		{
			Velocity.Z = 0.f;
			BodyInstance->SetLinearVelocity(Velocity, false, true);
		}
	}

	MeshComponent->WakeAllRigidBodies();
	MeshComponent->UpdateBounds();

	const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	if (Now - LastFloorGuardLogTimeSeconds >= 0.35)
	{
		LastFloorGuardLogTimeSeconds = Now;
		UE_LOG(
			LogT66Knockback,
			Display,
			TEXT("T66Knockback floor guard lift: Owner=%s FloorZ=%.1f BodyMinZ=%.1f AllowedMinZ=%.1f Lift=%.1f BoundsOrigin=%s BoundsExtent=%s"),
			*GetNameSafe(Character),
			FloorZ,
			BodyBounds.Min.Z,
			AllowedMinZ,
			CorrectionZ,
			*BodyBounds.GetCenter().ToCompactString(),
			*BodyBounds.GetExtent().ToCompactString());
	}

	return CorrectionZ;
}

bool UT66KnockbackComponent::HasPhysicalAnimationPoseBuffers(USkeletalMeshComponent* MeshComponent) const
{
	if (!MeshComponent || !MeshComponent->GetSkeletalMeshAsset())
	{
		return false;
	}

	const int32 BoneCount = MeshComponent->GetSkeletalMeshAsset()->GetRefSkeleton().GetNum();
	if (BoneCount <= 0)
	{
		return false;
	}

	if (MeshComponent->GetNumComponentSpaceTransforms() < BoneCount)
	{
		MeshComponent->AllocateTransformData();
		MeshComponent->TickAnimation(0.f, false);
		MeshComponent->RefreshBoneTransforms();
	}

	if (MeshComponent->GetBoneSpaceTransforms().Num() < BoneCount)
	{
		MeshComponent->AllocateTransformData();
	}

	return MeshComponent->GetNumComponentSpaceTransforms() >= BoneCount
		&& MeshComponent->GetBoneSpaceTransforms().Num() >= BoneCount;
}

void UT66KnockbackComponent::SyncKinematicMeshPoseToPhysics(USkeletalMeshComponent* MeshComponent)
{
	if (!MeshComponent || !MeshComponent->GetSkeletalMeshAsset())
	{
		return;
	}

	MeshComponent->SetWorldLocationAndRotation(
		MeshComponent->GetComponentLocation(),
		MeshComponent->GetComponentQuat(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
	MeshComponent->AllocateTransformData();
	MeshComponent->TickAnimation(0.f, false);
	MeshComponent->RefreshBoneTransforms();
	MeshComponent->UpdateComponentToWorld();
	if (MeshComponent->Bodies.Num() > 0)
	{
		MeshComponent->UpdateKinematicBonesToAnim(
			MeshComponent->GetComponentSpaceTransforms(),
			ETeleportType::TeleportPhysics,
			false);
	}
	MeshComponent->UpdateBounds();
}

int32 UT66KnockbackComponent::ApplyPhysicalAnimationDrive(
	UPhysicalAnimationComponent* PhysicalAnimation,
	USkeletalMeshComponent* MeshComponent,
	const FT66KnockbackProfile& Profile) const
{
	if (!PhysicalAnimation || !MeshComponent || !MeshComponent->GetPhysicsAsset())
	{
		return 0;
	}

	if (Profile.PhysicalAnimationDriveMode == ET66KnockbackPhysicalAnimationDriveMode::Disabled)
	{
		return 0;
	}

	FPhysicalAnimationData DriveData;
	DriveData.bIsLocalSimulation = false;
	DriveData.OrientationStrength = FMath::Max(0.f, Profile.PhysicalAnimationOrientationStrength);
	DriveData.AngularVelocityStrength = FMath::Max(0.f, Profile.PhysicalAnimationAngularVelocityStrength);
	DriveData.PositionStrength = 0.f;
	DriveData.VelocityStrength = 0.f;
	DriveData.MaxLinearForce = FMath::Max(0.f, Profile.PhysicalAnimationMaxLinearForce);
	DriveData.MaxAngularForce = FMath::Max(0.f, Profile.PhysicalAnimationMaxAngularForce);

	if (Profile.PhysicalAnimationDriveMode == ET66KnockbackPhysicalAnimationDriveMode::AllBodiesBelowRoot)
	{
		const int32 DrivenBodyCount = MeshComponent->ForEachBodyBelow(SimulationRootBoneName, true, false, [](FBodyInstance*) {});
		if (DrivenBodyCount > 0)
		{
			PhysicalAnimation->ApplyPhysicalAnimationSettingsBelow(SimulationRootBoneName, DriveData, true);
		}
		return DrivenBodyCount;
	}

	static const FName CoreDriveBodies[] =
	{
		FName(TEXT("pelvis")),
		FName(TEXT("spine_01")),
		FName(TEXT("spine_02")),
		FName(TEXT("spine_03")),
		FName(TEXT("neck_01")),
		FName(TEXT("head"))
	};

	UPhysicsAsset* PhysicsAsset = MeshComponent->GetPhysicsAsset();
	int32 DrivenBodyCount = 0;
	for (const FName BodyName : CoreDriveBodies)
	{
		if (MeshComponent->GetBoneIndex(BodyName) == INDEX_NONE
			|| !PhysicsAsset
			|| PhysicsAsset->FindBodyIndex(BodyName) == INDEX_NONE)
		{
			continue;
		}

		if (Profile.PhysicalAnimationDriveMode == ET66KnockbackPhysicalAnimationDriveMode::PelvisOnly
			&& BodyName != FName(TEXT("pelvis")))
		{
			continue;
		}

		PhysicalAnimation->ApplyPhysicalAnimationSettings(BodyName, DriveData);
		++DrivenBodyCount;
	}

	return DrivenBodyCount;
}

UPhysicalAnimationComponent* UT66KnockbackComponent::GetOrCreatePhysicalAnimationComponent(
	USkeletalMeshComponent* MeshComponent,
	const FT66KnockbackProfile& Profile,
	int32& OutDrivenBodyCount)
{
	OutDrivenBodyCount = 0;
	if (!Profile.bEnablePhysicalAnimation
		|| Profile.PhysicalAnimationDriveMode == ET66KnockbackPhysicalAnimationDriveMode::Disabled
		|| !MeshComponent
		|| !HasPhysicalAnimationPoseBuffers(MeshComponent))
	{
		return nullptr;
	}

	AActor* Owner = GetOwner();
	if (!PhysicalAnimationComponent && Owner)
	{
		PhysicalAnimationComponent = NewObject<UPhysicalAnimationComponent>(Owner, TEXT("T66KnockbackPhysicalAnimation"));
		if (PhysicalAnimationComponent)
		{
			Owner->AddInstanceComponent(PhysicalAnimationComponent);
			PhysicalAnimationComponent->RegisterComponent();
		}
	}

	if (PhysicalAnimationComponent)
	{
		if (MeshComponent->Bodies.Num() <= 0)
		{
			MeshComponent->RecreatePhysicsState();
		}
		MeshComponent->AllocateTransformData();
		PhysicalAnimationComponent->SetSkeletalMeshComponent(MeshComponent);
		OutDrivenBodyCount = ApplyPhysicalAnimationDrive(PhysicalAnimationComponent, MeshComponent, Profile);
		if (OutDrivenBodyCount > 0)
		{
			PhysicalAnimationComponent->SetStrengthMultiplyer(FMath::Clamp(Profile.PhysicalAnimationStrength, 0.f, 5.f));
		}
	}

	return OutDrivenBodyCount > 0 ? PhysicalAnimationComponent : nullptr;
}

void UT66KnockbackComponent::TryActivatePhysicalAnimation(const double Now)
{
	if (!bPhysicalAnimationActivationPending || Now < PhysicalAnimationActivationTimeSeconds)
	{
		return;
	}

	USkeletalMeshComponent* MeshComponent = ActiveMesh.Get();
	if (!MeshComponent)
	{
		bPhysicalAnimationActivationPending = false;
		return;
	}

	int32 DrivenBodyCount = 0;
	UPhysicalAnimationComponent* ActivatedPhysicalAnimation = GetOrCreatePhysicalAnimationComponent(
		MeshComponent,
		ActiveProfile,
		DrivenBodyCount);

	if (ActivatedPhysicalAnimation || ActiveProfile.PhysicalAnimationDriveMode == ET66KnockbackPhysicalAnimationDriveMode::Disabled)
	{
		bPhysicalAnimationActivationPending = false;
		PhysicalAnimationDrivenBodyCount = DrivenBodyCount;
		UE_LOG(
			LogT66Knockback,
			Display,
			TEXT("T66Knockback physical animation activation: Owner=%s PhysicalAnimation=%d DriveMode=%d DrivenBodies=%d RuntimeBodies=%d ComponentTransforms=%d BoneTransforms=%d"),
			*GetNameSafe(GetOwner()),
			ActivatedPhysicalAnimation ? 1 : 0,
			static_cast<int32>(ActiveProfile.PhysicalAnimationDriveMode),
			DrivenBodyCount,
			MeshComponent->Bodies.Num(),
			MeshComponent->GetNumComponentSpaceTransforms(),
			MeshComponent->GetBoneSpaceTransforms().Num());
	}
}

void UT66KnockbackComponent::CacheBodyPhysicsSettings(USkeletalMeshComponent* MeshComponent)
{
	PreImpactBodyPhysicsSettings.Reset();
	if (!MeshComponent)
	{
		return;
	}

	PreImpactBodyPhysicsSettings.Reserve(MeshComponent->Bodies.Num());
	for (int32 BodyIndex = 0; BodyIndex < MeshComponent->Bodies.Num(); ++BodyIndex)
	{
		FBodyInstance* BodyInstance = MeshComponent->Bodies[BodyIndex];
		if (!BodyInstance || !BodyInstance->IsValidBodyInstance())
		{
			continue;
		}

		FPreImpactBodyPhysicsSettings Settings;
		Settings.BodyIndex = BodyIndex;
		Settings.LinearDamping = BodyInstance->LinearDamping;
		Settings.AngularDamping = BodyInstance->AngularDamping;
		Settings.PhysMaterialOverride = BodyInstance->GetPhysMaterialOverride();
		PreImpactBodyPhysicsSettings.Add(Settings);
	}
}

void UT66KnockbackComponent::ApplyRagdollPhysicsResponseProfile(
	USkeletalMeshComponent* MeshComponent,
	const FT66KnockbackProfile& Profile)
{
	if (!MeshComponent)
	{
		return;
	}

	UPhysicalMaterial* PhysicsMaterialOverride = nullptr;
	if (Profile.RagdollFrictionOverride >= 0.f || Profile.RagdollRestitutionOverride >= 0.f)
	{
		ActiveRagdollPhysicalMaterial = NewObject<UPhysicalMaterial>(this);
		if (ActiveRagdollPhysicalMaterial)
		{
			const float Friction = Profile.RagdollFrictionOverride >= 0.f
				? FMath::Max(0.f, Profile.RagdollFrictionOverride)
				: ActiveRagdollPhysicalMaterial->Friction;
			const float Restitution = Profile.RagdollRestitutionOverride >= 0.f
				? FMath::Clamp(Profile.RagdollRestitutionOverride, 0.f, 1.f)
				: ActiveRagdollPhysicalMaterial->Restitution;
			ActiveRagdollPhysicalMaterial->Friction = Friction;
			ActiveRagdollPhysicalMaterial->StaticFriction = Friction;
			ActiveRagdollPhysicalMaterial->Restitution = Restitution;
			ActiveRagdollPhysicalMaterial->bOverrideFrictionCombineMode = true;
			ActiveRagdollPhysicalMaterial->FrictionCombineMode = EFrictionCombineMode::Min;
			ActiveRagdollPhysicalMaterial->bOverrideRestitutionCombineMode = true;
			ActiveRagdollPhysicalMaterial->RestitutionCombineMode = EFrictionCombineMode::Max;
			PhysicsMaterialOverride = ActiveRagdollPhysicalMaterial;
		}
	}

	for (FBodyInstance* BodyInstance : MeshComponent->Bodies)
	{
		if (!BodyInstance || !BodyInstance->IsValidBodyInstance())
		{
			continue;
		}

		bool bUpdatedDamping = false;
		if (Profile.RagdollLinearDampingOverride >= 0.f)
		{
			BodyInstance->LinearDamping = Profile.RagdollLinearDampingOverride;
			bUpdatedDamping = true;
		}
		if (Profile.RagdollAngularDampingOverride >= 0.f)
		{
			BodyInstance->AngularDamping = Profile.RagdollAngularDampingOverride;
			bUpdatedDamping = true;
		}
		if (bUpdatedDamping)
		{
			BodyInstance->UpdateDampingProperties();
		}

		if (PhysicsMaterialOverride)
		{
			BodyInstance->SetPhysMaterialOverride(PhysicsMaterialOverride);
		}
	}
}

void UT66KnockbackComponent::RestoreBodyPhysicsSettings(USkeletalMeshComponent* MeshComponent)
{
	if (!MeshComponent)
	{
		PreImpactBodyPhysicsSettings.Reset();
		ActiveRagdollPhysicalMaterial = nullptr;
		return;
	}

	for (const FPreImpactBodyPhysicsSettings& Settings : PreImpactBodyPhysicsSettings)
	{
		if (!MeshComponent->Bodies.IsValidIndex(Settings.BodyIndex))
		{
			continue;
		}

		FBodyInstance* BodyInstance = MeshComponent->Bodies[Settings.BodyIndex];
		if (!BodyInstance || !BodyInstance->IsValidBodyInstance())
		{
			continue;
		}

		BodyInstance->LinearDamping = Settings.LinearDamping;
		BodyInstance->AngularDamping = Settings.AngularDamping;
		BodyInstance->UpdateDampingProperties();
		BodyInstance->SetPhysMaterialOverride(Settings.PhysMaterialOverride.Get());
	}

	PreImpactBodyPhysicsSettings.Reset();
	ActiveRagdollPhysicalMaterial = nullptr;
}

void UT66KnockbackComponent::ApplyLaunchImpulse(USkeletalMeshComponent* MeshComponent, const FVector& LaunchVelocity, const FT66KnockbackProfile& Profile)
{
	if (!MeshComponent)
	{
		return;
	}

	const FName MainBodyName = !VelocityBoneName.IsNone() ? VelocityBoneName : SimulationRootBoneName;
	ApplyMassScaledImpulseToBody(
		MeshComponent,
		MainBodyName,
		LaunchVelocity,
		FMath::Max(0.f, Profile.MainBodyImpulseScale),
		Profile.bTreatLaunchVectorAsVelocityChange);

	if (!SimulationRootBoneName.IsNone() && Profile.BelowBodiesImpulseFraction > 0.f)
	{
		ApplyMassScaledImpulseToBodiesBelow(
			MeshComponent,
			SimulationRootBoneName,
			LaunchVelocity,
			FMath::Clamp(Profile.BelowBodiesImpulseFraction, 0.f, 2.f),
			Profile.bTreatLaunchVectorAsVelocityChange,
			Profile.bIncludeSimulationRootInBelowBodyImpulse);
	}
}

void UT66KnockbackComponent::ApplyMassScaledImpulseToBody(
	USkeletalMeshComponent* MeshComponent,
	const FName BodyName,
	const FVector& DesiredVelocityChange,
	const float Scale,
	const bool bVelocityChange)
{
	if (!MeshComponent || BodyName.IsNone() || Scale <= 0.f)
	{
		return;
	}

	if (bVelocityChange)
	{
		MeshComponent->AddImpulse(DesiredVelocityChange * Scale, BodyName, true);
		return;
	}

	FBodyInstance* BodyInstance = MeshComponent->GetBodyInstance(BodyName);
	if (!BodyInstance || !BodyInstance->IsValidBodyInstance() || !BodyInstance->bSimulatePhysics)
	{
		return;
	}

	const float BodyMass = FMath::Max(1.f, BodyInstance->GetBodyMass());
	BodyInstance->AddImpulse(DesiredVelocityChange * BodyMass * Scale, false);
}

void UT66KnockbackComponent::ApplyMassScaledImpulseToBodiesBelow(
	USkeletalMeshComponent* MeshComponent,
	const FName RootBoneName,
	const FVector& DesiredVelocityChange,
	const float Scale,
	const bool bVelocityChange,
	const bool bIncludeSelf)
{
	if (!MeshComponent || RootBoneName.IsNone() || Scale <= 0.f)
	{
		return;
	}

	if (bVelocityChange)
	{
		MeshComponent->AddImpulseToAllBodiesBelow(DesiredVelocityChange * Scale, RootBoneName, true, bIncludeSelf);
		return;
	}

	MeshComponent->ForEachBodyBelow(
		RootBoneName,
		bIncludeSelf,
		false,
		[&DesiredVelocityChange, Scale](FBodyInstance* BodyInstance)
		{
			if (!BodyInstance || !BodyInstance->IsValidBodyInstance() || !BodyInstance->bSimulatePhysics)
			{
				return;
			}

			const float BodyMass = FMath::Max(1.f, BodyInstance->GetBodyMass());
			BodyInstance->AddImpulse(DesiredVelocityChange * BodyMass * Scale, false);
		});
}
