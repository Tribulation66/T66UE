# Hero 1 Active-Ragdoll Diagnosis Evidence

Generated: 2026-06-07
Project: C:\UE\T66 / UE5.7
Proof folder: C:\UE\T66\Reports\Proof\Physics\HeroRagdollDiagnosis_20260607_074119

Scope note: this file is evidence only. No proposed fixes are included.

## Part A - Verbatim Code Dump

### UT66HeroPhysicsComponent::SyncMeshComponentToCapsuleAuthority

```cpp
bool UT66HeroPhysicsComponent::SyncMeshComponentToCapsuleAuthority(const FName Reason)
{
	AT66HeroBase* Hero = ResolveHero();
	USkeletalMeshComponent* Mesh = ActiveMesh.Get();
	if (!Hero || !Mesh || !bHasActiveAuthorityMeshRelativeTransform)
	{
		return false;
	}

	USceneComponent* Root = Hero->GetRootComponent();
	if (!Root)
	{
		return false;
	}

	Root->UpdateComponentToWorld();
	Mesh->UpdateComponentToWorld();

	const FTransform DesiredWorldTransform = ActiveAuthorityMeshRelativeTransform * Root->GetComponentTransform();
	if (DesiredWorldTransform.ContainsNaN())
	{
		return false;
	}

	const bool bWasAttachedToRoot = Mesh->GetAttachParent() == Root;
	const FString PreviousParentName = GetNameSafe(Mesh->GetAttachParent());
	const FVector MeshLocationBefore = Mesh->GetComponentLocation();
	const FQuat MeshRotationBefore = Mesh->GetComponentQuat();
	const FVector MeshScaleBefore = Mesh->GetComponentScale();
	const float LocationDelta = FVector::Dist(MeshLocationBefore, DesiredWorldTransform.GetLocation());
	const float RotationDeltaDegrees = FMath::RadiansToDegrees(
		MeshRotationBefore.AngularDistance(DesiredWorldTransform.GetRotation()));
	const float ScaleDelta = FVector::Dist(MeshScaleBefore, DesiredWorldTransform.GetScale3D());
	const bool bNeedsSync = !bWasAttachedToRoot
		|| LocationDelta > 1.f
		|| RotationDeltaDegrees > 1.f
		|| ScaleDelta > 0.01f;
	if (!bNeedsSync)
	{
		return false;
	}

	if (!bWasAttachedToRoot)
	{
		Mesh->AttachToComponent(Root, FAttachmentTransformRules::KeepWorldTransform);
	}
	const FVector LocationOffset = DesiredWorldTransform.GetLocation() - MeshLocationBefore;
	if (!LocationOffset.IsNearlyZero(0.1f))
	{
		Mesh->AddWorldOffset(LocationOffset, false, nullptr, ETeleportType::TeleportPhysics);
	}
	if (RotationDeltaDegrees > 1.f)
	{
		Mesh->SetWorldRotation(DesiredWorldTransform.GetRotation(), false, nullptr, ETeleportType::TeleportPhysics);
	}
	if (ScaleDelta > 0.01f)
	{
		Mesh->SetWorldScale3D(DesiredWorldTransform.GetScale3D());
	}
	Mesh->UpdateComponentToWorld();
	Mesh->UpdateBounds();

	UWorld* World = GetWorld();
	const double Now = World ? World->GetTimeSeconds() : 0.0;
	const bool bDebugLog = CVarT66HeroPhysicsDebugLog.GetValueOnGameThread() != 0;
	const bool bLargeCorrection = LocationDelta > FMath::Max(24.f, Profile.MaxPelvisCapsuleDistance * 0.25f);
	if ((bDebugLog || bLargeCorrection || !bWasAttachedToRoot)
		&& Now - LastMeshAuthoritySyncLogTimeSeconds >= 0.25)
	{
		LastMeshAuthoritySyncLogTimeSeconds = Now;
		UE_LOG(
			LogT66HeroPhysics,
			Display,
			TEXT("[HeroActiveRagdoll] MeshCapsuleAuthoritySync Reason=%s WasAttached=%d PreviousParent=%s Parent=%s LocationDelta=%.1f RotationDeltaDeg=%.1f ScaleDelta=%.3f Hero=%s MeshBefore=%s MeshAfter=%s Desired=%s Anchor=%s"),
			*Reason.ToString(),
			bWasAttachedToRoot ? 1 : 0,
			*PreviousParentName,
			*GetNameSafe(Mesh->GetAttachParent()),
			LocationDelta,
			RotationDeltaDegrees,
			ScaleDelta,
			*Hero->GetActorLocation().ToCompactString(),
			*MeshLocationBefore.ToCompactString(),
			*Mesh->GetComponentLocation().ToCompactString(),
			*DesiredWorldTransform.GetLocation().ToCompactString(),
			*GetAnchorWorldLocation().ToCompactString());
	}

	return true;
}
```

### UT66HeroPhysicsComponent::ApplyPhysicsReaction

```cpp
bool UT66HeroPhysicsComponent::ApplyPhysicsReaction(
	const FVector& RequestedVelocityChange,
	const FVector& WorldHitLocation,
	const FName SourceTag)
{
	if (!bInitialized)
	{
		if (!TryInitializeActiveRagdoll())
		{
			return false;
		}
	}

	SyncMeshComponentToCapsuleAuthority(FName(TEXT("ReactionCapsuleAuthority")));

	UWorld* World = GetWorld();
	const double Now = World ? World->GetTimeSeconds() : 0.0;
	if (Now - LastReactionTimeSeconds < Profile.ReactionCooldownSeconds)
	{
		UE_LOG(
			LogT66HeroPhysics,
			Verbose,
			TEXT("[HeroActiveRagdoll] Reaction cooldown consumed Source=%s State=%s"),
			*SourceTag.ToString(),
			*T66HeroPhysicsStateName(RuntimeState));
		return true;
	}

	FVector VelocityChange = RequestedVelocityChange * FMath::Max(0.f, Profile.ReactionImpulseScale);
	if (Profile.MaxReactionVelocityChange > KINDA_SMALL_NUMBER)
	{
		VelocityChange = VelocityChange.GetClampedToMaxSize(Profile.MaxReactionVelocityChange);
	}

	if (!IsBodySimulating(PelvisBodyName))
	{
		UE_LOG(
			LogT66HeroPhysics,
			Warning,
			TEXT("[HeroActiveRagdoll] Reaction failed: pelvis body is not simulated. Source=%s Pelvis=%s RequestedVelocity=%s"),
			*SourceTag.ToString(),
			*PelvisBodyName.ToString(),
			*RequestedVelocityChange.ToCompactString());
		return false;
	}

	LastReactionTimeSeconds = Now;

	const bool bKnockdown = VelocityChange.Size() >= Profile.KnockdownSpeedThreshold;
	SetRuntimeState(
		bKnockdown ? ET66HeroPhysicsRuntimeState::KnockedDown : ET66HeroPhysicsRuntimeState::Staggered,
		SourceTag);

	ApplyMassScaledVelocityTargetToBody(VelocityChange, PelvisBodyName, 1.f, WorldHitLocation, true);
	ApplyMassScaledVelocityTargetBelow(VelocityChange, SimulationRootBodyName, Profile.BelowBodiesImpulseFraction, false);
	ApplyCapsuleReactionVelocity(VelocityChange);

	if (USkeletalMeshComponent* Mesh = ActiveMesh.Get())
	{
		Mesh->WakeAllRigidBodies();
	}

	UE_LOG(
		LogT66HeroPhysics,
		Display,
		TEXT("[HeroActiveRagdoll] Reaction Applied=1 Source=%s State=%s RequestedVelocity=%s AppliedVelocity=%s Hit=%s Pelvis=%s PoseMult=%.2f AnchorMult=%.2f"),
		*SourceTag.ToString(),
		*T66HeroPhysicsStateName(RuntimeState),
		*RequestedVelocityChange.ToCompactString(),
		*VelocityChange.ToCompactString(),
		*WorldHitLocation.ToCompactString(),
		*PelvisBodyName.ToString(),
		CurrentPoseMultiplier,
		CurrentAnchorMultiplier);

	return true;
}
```

### UT66HeroPhysicsComponent::ConfigureHipAnchorConstraint

```cpp
bool UT66HeroPhysicsComponent::ConfigureHipAnchorConstraint()
{
	AT66HeroBase* Hero = ResolveHero();
	AActor* Owner = GetOwner();
	USkeletalMeshComponent* Mesh = ResolveMesh();
	UCapsuleComponent* Capsule = Hero ? Hero->GetCapsuleComponent() : nullptr;
	if (!Owner || !Hero || !Mesh || !Capsule)
	{
		return false;
	}

	if (!HipAnchorComponent)
	{
		HipAnchorComponent = NewObject<USphereComponent>(Owner, TEXT("T66HeroPhysicsHipAnchor"));
		if (!HipAnchorComponent)
		{
			return false;
		}
		Owner->AddInstanceComponent(HipAnchorComponent);
		HipAnchorComponent->RegisterComponent();
		HipAnchorComponent->SetSphereRadius(12.f);
		HipAnchorComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HipAnchorComponent->SetHiddenInGame(true, true);
		HipAnchorComponent->SetVisibility(false, true);
		HipAnchorComponent->SetSimulatePhysics(false);
		HipAnchorComponent->SetEnableGravity(false);
		HipAnchorComponent->AttachToComponent(Capsule, FAttachmentTransformRules::KeepRelativeTransform);
	}
	HipAnchorComponent->SetRelativeLocation(Profile.AnchorRelativeLocation);

	if (!HipConstraintComponent)
	{
		HipConstraintComponent = NewObject<UPhysicsConstraintComponent>(Owner, TEXT("T66HeroPhysicsHipConstraint"));
		if (!HipConstraintComponent)
		{
			return false;
		}
		Owner->AddInstanceComponent(HipConstraintComponent);
		HipConstraintComponent->RegisterComponent();
		HipConstraintComponent->AttachToComponent(HipAnchorComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}

	HipConstraintComponent->SetRelativeLocation(FVector::ZeroVector);
	HipConstraintComponent->SetDisableCollision(true);
	HipConstraintComponent->SetLinearXLimit(LCM_Limited, FMath::Max(1.f, Profile.AnchorLinearLimit));
	HipConstraintComponent->SetLinearYLimit(LCM_Limited, FMath::Max(1.f, Profile.AnchorLinearLimit));
	HipConstraintComponent->SetLinearZLimit(LCM_Limited, FMath::Max(1.f, Profile.AnchorLinearLimit));
	HipConstraintComponent->SetAngularSwing1Limit(ACM_Limited, FMath::Clamp(Profile.AnchorSwingLimitDegrees, 0.f, 180.f));
	HipConstraintComponent->SetAngularSwing2Limit(ACM_Limited, FMath::Clamp(Profile.AnchorSwingLimitDegrees, 0.f, 180.f));
	HipConstraintComponent->SetAngularTwistLimit(ACM_Limited, FMath::Clamp(Profile.AnchorTwistLimitDegrees, 0.f, 180.f));
	HipConstraintComponent->SetLinearPositionDrive(true, true, true);
	HipConstraintComponent->SetLinearVelocityDrive(true, true, true);
	HipConstraintComponent->SetAngularDriveMode(EAngularDriveMode::SLERP);
	HipConstraintComponent->SetAngularOrientationDrive(true, true);
	HipConstraintComponent->SetAngularVelocityDrive(true, true);
	HipConstraintComponent->SetConstrainedComponents(Mesh, PelvisBodyName, HipAnchorComponent, NAME_None);
	ApplyDriveMultipliers(Profile.BalancedPoseStrengthMultiplier, 1.f);
	return true;
}
```

### UT66HeroPhysicsComponent::ConfigureMeshPhysics

```cpp
bool UT66HeroPhysicsComponent::ConfigureMeshPhysics()
{
	AT66HeroBase* Hero = ResolveHero();
	USkeletalMeshComponent* Mesh = ResolveMesh();
	if (!Mesh)
	{
		return false;
	}

	if (Hero)
	{
		if (USceneComponent* RootComponent = Hero->GetRootComponent())
		{
			RootComponent->UpdateComponentToWorld();
		}
	}
	Mesh->UpdateComponentToWorld();
	Mesh->PhysicsTransformUpdateMode = EPhysicsTransformUpdateMode::ComponentTransformIsKinematic;
	Mesh->KinematicBonesUpdateType = EKinematicBonesUpdateToPhysics::SkipSimulatingBones;
	Mesh->bBlendPhysics = true;
	Mesh->SetAllBodiesPhysicsBlendWeight(0.f);
	Mesh->SetAllBodiesSimulatePhysics(false);
	Mesh->ResetAllBodiesSimulatePhysics();
	Mesh->RecreatePhysicsState();
	Mesh->UpdateComponentToWorld();
	SyncKinematicMeshPoseToPhysics();
	AlignMeshPelvisToCapsuleAnchor(FName(TEXT("InitMeshPelvisAnchorAlign")));
	SyncKinematicMeshPoseToPhysics();
	ActiveAuthorityMeshRelativeTransform = Mesh->GetRelativeTransform();
	bHasActiveAuthorityMeshRelativeTransform = true;
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_PhysicsBody);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetEnableGravity(true);

	if (Mesh->Bodies.Num() <= 0)
	{
		Mesh->RecreatePhysicsState();
	}

	Mesh->SetAllUseCCD(true);
	Mesh->SetAllBodiesBelowSimulatePhysics(SimulationRootBodyName, true, true);
	Mesh->SetAllBodiesBelowPhysicsBlendWeight(SimulationRootBodyName, 1.f, false, true);
	Mesh->WakeAllRigidBodies();
	Mesh->UpdateBounds();
	if (Profile.MaxPelvisCapsuleDistance > 0.f
		&& ComputePelvisCapsuleDistance() > (Profile.MaxPelvisCapsuleDistance * 0.5f))
	{
		ResyncPelvisBodyToActor(FName(TEXT("InitEmergencyPelvisResync")));
	}
	return true;
}
```

### UT66HeroPhysicsComponent::ConfigurePhysicalAnimation

```cpp
bool UT66HeroPhysicsComponent::ConfigurePhysicalAnimation()
{
	AActor* Owner = GetOwner();
	USkeletalMeshComponent* Mesh = ResolveMesh();
	if (!Owner || !Mesh)
	{
		return false;
	}

	if (!PhysicalAnimationComponent)
	{
		PhysicalAnimationComponent = NewObject<UPhysicalAnimationComponent>(Owner, TEXT("T66HeroActivePhysicalAnimation"));
		if (!PhysicalAnimationComponent)
		{
			return false;
		}
		Owner->AddInstanceComponent(PhysicalAnimationComponent);
		PhysicalAnimationComponent->RegisterComponent();
	}

	FPhysicalAnimationData ChildDriveData;
	ChildDriveData.bIsLocalSimulation = true;
	ChildDriveData.OrientationStrength = FMath::Max(0.f, Profile.PoseOrientationStrength);
	ChildDriveData.AngularVelocityStrength = FMath::Max(0.f, Profile.PoseAngularVelocityStrength);
	ChildDriveData.PositionStrength = 0.f;
	ChildDriveData.VelocityStrength = 0.f;
	ChildDriveData.MaxLinearForce = 0.f;
	ChildDriveData.MaxAngularForce = FMath::Max(0.f, Profile.PoseMaxAngularForce);

	PhysicalAnimationComponent->SetSkeletalMeshComponent(Mesh);
	const int32 DrivenBodies = Mesh->ForEachBodyBelow(SimulationRootBodyName, true, false, [](FBodyInstance*) {});
	if (DrivenBodies <= 1)
	{
		return false;
	}

	// The hip anchor/constraint owns pelvis position. PAC is only a local muscle
	// drive for descendants, so it must not become a second gameplay root.
	PhysicalAnimationComponent->ApplyPhysicalAnimationSettingsBelow(SimulationRootBodyName, ChildDriveData, false);
	PhysicalAnimationComponent->SetStrengthMultiplyer(FMath::Clamp(Profile.BalancedPoseStrengthMultiplier, 0.f, 5.f));
	return true;
}
```

### UT66HeroPhysicsComponent::UpdateAnchorTransform

```cpp
void UT66HeroPhysicsComponent::UpdateAnchorTransform()
{
	if (HipAnchorComponent)
	{
		HipAnchorComponent->SetRelativeLocation(Profile.AnchorRelativeLocation);
	}
}
```

### UT66HeroPhysicsComponent::UpdateStateMachine

```cpp
void UT66HeroPhysicsComponent::UpdateStateMachine(const float DeltaTime)
{
	UWorld* World = GetWorld();
	const double Now = World ? World->GetTimeSeconds() : 0.0;
	const float StateAge = static_cast<float>(Now - StateStartTimeSeconds);

	switch (RuntimeState)
	{
	case ET66HeroPhysicsRuntimeState::Staggered:
		if (StateAge >= Profile.StaggerSeconds)
		{
			SetRuntimeState(ET66HeroPhysicsRuntimeState::Recovering, FName(TEXT("StaggerElapsed")));
		}
		break;
	case ET66HeroPhysicsRuntimeState::KnockedDown:
		if (StateAge >= Profile.KnockdownHoldSeconds)
		{
			SetRuntimeState(ET66HeroPhysicsRuntimeState::Recovering, FName(TEXT("KnockdownHoldElapsed")));
		}
		break;
	case ET66HeroPhysicsRuntimeState::Recovering:
		RecoveryElapsedSeconds += FMath::Max(0.f, DeltaTime);
		{
			const float Alpha = FMath::Clamp(RecoveryElapsedSeconds / FMath::Max(0.01f, Profile.RecoverySeconds), 0.f, 1.f);
			const float SmoothAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.0f);
			ApplyDriveMultipliers(
				FMath::Lerp(Profile.KnockdownPoseStrengthMultiplier, Profile.BalancedPoseStrengthMultiplier, SmoothAlpha),
				FMath::Lerp(Profile.KnockdownAnchorStrengthMultiplier, 1.f, SmoothAlpha));
			if (Alpha >= 1.f)
			{
				SetRuntimeState(ET66HeroPhysicsRuntimeState::Balanced, FName(TEXT("RecoveryComplete")));
			}
		}
		break;
	case ET66HeroPhysicsRuntimeState::Balanced:
	default:
		break;
	}

	if (Profile.MaxPelvisCapsuleDistance > 0.f)
	{
		const float Distance = ComputePelvisCapsuleDistance();
		if (Distance > Profile.MaxPelvisCapsuleDistance)
		{
			if (Now - LastPelvisDivergenceLogTimeSeconds >= 0.5)
			{
				LastPelvisDivergenceLogTimeSeconds = Now;
				UE_LOG(
					LogT66HeroPhysics,
					Warning,
					TEXT("[HeroActiveRagdoll] Pelvis divergence %.1f exceeds %.1f; anchor strengthened without runtime body teleport."),
					Distance,
					Profile.MaxPelvisCapsuleDistance);
			}
			ApplyDriveMultipliers(Profile.BalancedPoseStrengthMultiplier, 1.f);
			if (USkeletalMeshComponent* Mesh = ActiveMesh.Get())
			{
				Mesh->WakeAllRigidBodies();
			}
		}
		else
		{
			LastPelvisDivergenceLogTimeSeconds = -9999.0;
		}
	}
}
```

### TestRoom ApplyWipeoutArmHeroImpact

```cpp
		void ApplyWipeoutArmHeroImpact(
			UWorld* World,
			const TSharedPtr<FWipeoutArmTrapState>& State,
			AT66HeroBase* Hero,
			const FVector& HubLocation,
			const FVector& MotionDirection)
		{
			if (!World || !State.IsValid() || !Hero)
			{
				return;
			}

			const double Now = World->GetTimeSeconds();
			if (Now - State->LastImpactTimeSeconds < WipeoutArmImpactCooldownSeconds || Hero->IsKnockbackActive())
			{
				return;
			}
			State->LastImpactTimeSeconds = Now;

			FVector RadialDirection = Hero->GetActorLocation() - HubLocation;
			RadialDirection.Z = 0.f;
			if (!RadialDirection.Normalize())
			{
				RadialDirection = MotionDirection.GetSafeNormal();
			}
			if (RadialDirection.IsNearlyZero())
			{
				RadialDirection = Hero->GetActorForwardVector();
				RadialDirection.Z = 0.f;
				RadialDirection.Normalize();
			}
			if (RadialDirection.IsNearlyZero())
			{
				RadialDirection = FVector::ForwardVector;
			}

			FVector TangentialDirection = MotionDirection.GetSafeNormal();
			TangentialDirection.Z = 0.f;
			if (!TangentialDirection.Normalize())
			{
				TangentialDirection = FVector::ZeroVector;
			}
			const FVector LaunchDir = (RadialDirection + (TangentialDirection * 0.22f)).GetSafeNormal();

			const float LaunchXY = FMath::Max(0.f, CVarT66TestRoomWipeoutArmLaunchXY.GetValueOnGameThread());
			const float LaunchZ = FMath::Max(0.f, CVarT66TestRoomWipeoutArmLaunchZ.GetValueOnGameThread());
			const FVector LaunchVelocity = LaunchDir * LaunchXY + FVector(0.f, 0.f, LaunchZ);
			const FVector ReactionHitLocation = Hero->GetActorLocation()
				+ LaunchDir * FMath::Max(40.f, Hero->GetCapsuleComponent() ? Hero->GetCapsuleComponent()->GetScaledCapsuleRadius() : 40.f)
				+ FVector(0.f, 0.f, 24.f);
			FT66KnockbackProfile Profile = MakeWipeoutArmKnockbackProfile(Hero);
			Profile.MaxLaunchVelocity = FMath::Max(Profile.MaxLaunchVelocity, LaunchVelocity.Size() * 1.05f);

			bool bAppliedActiveRagdoll = false;
			bool bTriedActiveRagdoll = false;
			if (CVarT66TestRoomWipeoutArmUseHeroActiveRagdoll.GetValueOnGameThread() != 0)
			{
				if (UT66HeroPhysicsComponent* HeroPhysicsComponent = Hero->GetHeroPhysicsComponent())
				{
					bTriedActiveRagdoll = true;
					bAppliedActiveRagdoll = HeroPhysicsComponent->ApplyPhysicsReaction(
						LaunchVelocity,
						ReactionHitLocation,
						FName(TEXT("TestRoomWipeoutArm")));
				}
			}

			bool bAppliedKnockback = false;
			if (!bAppliedActiveRagdoll)
			{
				UT66KnockbackComponent* KnockbackComponent = Hero->GetKnockbackComponent();
				bAppliedKnockback = KnockbackComponent
					? KnockbackComponent->ApplyKnockbackLaunch(LaunchVelocity, &Profile)
					: Hero->ApplyKnockbackLaunch(LaunchVelocity);
			}

			UE_LOG(LogTemp, Display, TEXT("TestRoom wipeout arm impact routed to hero physics: ActiveTried=%d ActiveApplied=%d LegacyApplied=%d Launch=%s Hit=%s Radial=%s Tangent=%s Incap=%.2fs MaxRagdoll=%.2fs VelocityChange=%d BelowFraction=%.2f LinearDamping=%.2f AngularDamping=%.2f Friction=%.2f Restitution=%.2f LegacyProfilePAC=%d LegacyDriveMode=%d"),
				bTriedActiveRagdoll ? 1 : 0,
				bAppliedActiveRagdoll ? 1 : 0,
				bAppliedKnockback ? 1 : 0,
				*LaunchVelocity.ToCompactString(),
				*ReactionHitLocation.ToCompactString(),
				*RadialDirection.ToCompactString(),
				*TangentialDirection.ToCompactString(),
				Profile.MinIncapacitationSeconds,
				Profile.MaxRagdollSeconds,
				Profile.bTreatLaunchVectorAsVelocityChange ? 1 : 0,
				Profile.BelowBodiesImpulseFraction,
				Profile.RagdollLinearDampingOverride,
				Profile.RagdollAngularDampingOverride,
				Profile.RagdollFrictionOverride,
				Profile.RagdollRestitutionOverride,
				Profile.bEnablePhysicalAnimation ? 1 : 0,
				static_cast<int32>(Profile.PhysicalAnimationDriveMode));
		}
```


## Part B - Static Facts

### B1 - ACTIVE LaunchCharacter flags and movement suppression search

ACTIVE path LaunchCharacter call:

```cpp
1202: 	FVector CapsuleVelocityChange = VelocityChange * FMath::Clamp(Profile.CapsuleReactionVelocityFraction, 0.f, 1.f);
1203: 	if (Profile.MaxCapsuleReactionVelocityChange > KINDA_SMALL_NUMBER)
1204: 	{
1205: 		CapsuleVelocityChange = CapsuleVelocityChange.GetClampedToMaxSize(Profile.MaxCapsuleReactionVelocityChange);
1208: 	if (CapsuleVelocityChange.IsNearlyZero())
1209: 	{
1210: 		return;
1211: 	}
1213: 	Hero->LaunchCharacter(CapsuleVelocityChange, false, false);
```

The ACTIVE call passes XYOverride=false and ZOverride=false.

The active physics component has no MovementMode, SetMovementMode, DisableMovement, GroundFriction, Braking, IgnoreBaseRotation, or IgnoreBaseRotation match. It does have input and combat suppression only:

```cpp
1153: void UT66HeroPhysicsComponent::ApplyGameplaySuppression(const bool bSuppress)
1154: {
1155: 	if (bGameplaySuppressed == bSuppress)
1156: 	{
1157: 		return;
1158: 	}
1159: 
1160: 	bGameplaySuppressed = bSuppress;
1161: 
1162: 	if (AT66HeroBase* Hero = ResolveHero())
1163: 	{
1164: 		if (APlayerController* PlayerController = Cast<APlayerController>(Hero->GetController()))
1165: 		{
1166: 			if (bSuppress)
1167: 			{
1168: 				Hero->DisableInput(PlayerController);
1169: 			}
1170: 			else
1171: 			{
1172: 				Hero->EnableInput(PlayerController);
1173: 			}
1174: 		}
1175: 
1176: 		if (Hero->CombatComponent)
1177: 		{
1178: 			Hero->CombatComponent->SetAutoAttackSuppressed(bSuppress);
1179: 		}
1180: 	}
```

Repo-wide search did find movement/friction changes outside UT66HeroPhysicsComponent, including HeroBase mount/stage-slide paths and legacy knockback, but not in the active physics component path.

### B2 - WipeoutArmLaunchXY and WipeoutArmLaunchZ usage

CVar defaults:

```cpp
111: static TAutoConsoleVariable<float> CVarT66TestRoomWipeoutArmLaunchXY(
112: 	TEXT("t66.TestRoom.WipeoutArmLaunchXY"),
113: 	10500.f,
114: 	TEXT("Horizontal launch speed applied when the TestRoom wipeout arm hits the hero."),
115: 	ECVF_Default);
116: 
117: static TAutoConsoleVariable<float> CVarT66TestRoomWipeoutArmLaunchZ(
118: 	TEXT("t66.TestRoom.WipeoutArmLaunchZ"),
119: 	750.f,
120: 	TEXT("Vertical launch speed applied when the TestRoom wipeout arm hits the hero."),
121: 	ECVF_Default);
```

Impact handler usage:

```cpp
559: 			const float LaunchXY = FMath::Max(0.f, CVarT66TestRoomWipeoutArmLaunchXY.GetValueOnGameThread());
560: 			const float LaunchZ = FMath::Max(0.f, CVarT66TestRoomWipeoutArmLaunchZ.GetValueOnGameThread());
561: 			const FVector LaunchVelocity = LaunchDir * LaunchXY + FVector(0.f, 0.f, LaunchZ);
562: 			const FVector ReactionHitLocation = Hero->GetActorLocation()
563: 				+ LaunchDir * FMath::Max(40.f, Hero->GetCapsuleComponent() ? Hero->GetCapsuleComponent()->GetScaledCapsuleRadius() : 40.f)
564: 				+ FVector(0.f, 0.f, 24.f);
565: 			FT66KnockbackProfile Profile = MakeWipeoutArmKnockbackProfile(Hero);
566: 			Profile.MaxLaunchVelocity = FMath::Max(Profile.MaxLaunchVelocity, LaunchVelocity.Size() * 1.05f);
567: 
568: 			bool bAppliedActiveRagdoll = false;
569: 			bool bTriedActiveRagdoll = false;
570: 			if (CVarT66TestRoomWipeoutArmUseHeroActiveRagdoll.GetValueOnGameThread() != 0)
571: 			{
572: 				if (UT66HeroPhysicsComponent* HeroPhysicsComponent = Hero->GetHeroPhysicsComponent())
573: 				{
574: 					bTriedActiveRagdoll = true;
575: 					bAppliedActiveRagdoll = HeroPhysicsComponent->ApplyPhysicsReaction(
576: 						LaunchVelocity,
577: 						ReactionHitLocation,
578: 						FName(TEXT("TestRoomWipeoutArm")));
```

Fallback legacy path after the active path:

```cpp
580: 			}
581: 
582: 			bool bAppliedKnockback = false;
583: 			if (!bAppliedActiveRagdoll)
584: 			{
585: 				UT66KnockbackComponent* KnockbackComponent = Hero->GetKnockbackComponent();
586: 				bAppliedKnockback = KnockbackComponent
587: 					? KnockbackComponent->ApplyKnockbackLaunch(LaunchVelocity, &Profile)
588: 					: Hero->ApplyKnockbackLaunch(LaunchVelocity);
589: 			}
590: 
591: 			UE_LOG(LogTemp, Display, TEXT("TestRoom wipeout arm impact routed to hero physics: ActiveTried=%d ActiveApplied=%d LegacyApplied=%d Launch=%s Hit=%s Radial=%s Tangent=%s Incap=%.2fs MaxRagdoll=%.2fs VelocityChange=%d BelowFraction=%.2f LinearDamping=%.2f AngularDamping=%.2f Friction=%.2f Restitution=%.2f LegacyProfilePAC=%d LegacyDriveMode=%d"),
592: 				bTriedActiveRagdoll ? 1 : 0,
593: 				bAppliedActiveRagdoll ? 1 : 0,
594: 				bAppliedKnockback ? 1 : 0,
595: 				*LaunchVelocity.ToCompactString(),
596: 				*ReactionHitLocation.ToCompactString(),
597: 				*RadialDirection.ToCompactString(),
598: 				*TangentialDirection.ToCompactString(),
599: 				Profile.MinIncapacitationSeconds,
```

Therefore the launch values build LaunchVelocity before routing. That same vector is passed to the ACTIVE ApplyPhysicsReaction path when available and to the LEGACY fallback path otherwise.

### B3 - Hip anchor constraint drive pushed to constraint instance

Profile defaults:

```cpp
53: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor")
54: 	FVector AnchorRelativeLocation = FVector(0.f, 0.f, 12.f);
55: 
56: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor", meta = (ClampMin = "1.0"))
57: 	float AnchorLinearLimit = 72.f;
58: 
59: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor", meta = (ClampMin = "0.0"))
60: 	float AnchorLinearStrength = 8200.f;
61: 
62: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor", meta = (ClampMin = "0.0"))
63: 	float AnchorLinearVelocityStrength = 640.f;
64: 
65: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor", meta = (ClampMin = "0.0"))
66: 	float AnchorLinearMaxForce = 62000.f;
67: 
68: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor", meta = (ClampMin = "0.0", ClampMax = "180.0"))
69: 	float AnchorSwingLimitDegrees = 68.f;
70: 
71: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor", meta = (ClampMin = "0.0", ClampMax = "180.0"))
72: 	float AnchorTwistLimitDegrees = 58.f;
73: 
74: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor", meta = (ClampMin = "0.0"))
75: 	float AnchorAngularStrength = 4200.f;
76: 
77: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor", meta = (ClampMin = "0.0"))
78: 	float AnchorAngularVelocityStrength = 460.f;
79: 
80: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor", meta = (ClampMin = "0.0"))
81: 	float AnchorAngularMaxForce = 32000.f;
```

Constraint limit and drive setup:

```cpp
540: 	HipConstraintComponent->SetRelativeLocation(FVector::ZeroVector);
541: 	HipConstraintComponent->SetDisableCollision(true);
542: 	HipConstraintComponent->SetLinearXLimit(LCM_Limited, FMath::Max(1.f, Profile.AnchorLinearLimit));
543: 	HipConstraintComponent->SetLinearYLimit(LCM_Limited, FMath::Max(1.f, Profile.AnchorLinearLimit));
544: 	HipConstraintComponent->SetLinearZLimit(LCM_Limited, FMath::Max(1.f, Profile.AnchorLinearLimit));
545: 	HipConstraintComponent->SetAngularSwing1Limit(ACM_Limited, FMath::Clamp(Profile.AnchorSwingLimitDegrees, 0.f, 180.f));
546: 	HipConstraintComponent->SetAngularSwing2Limit(ACM_Limited, FMath::Clamp(Profile.AnchorSwingLimitDegrees, 0.f, 180.f));
547: 	HipConstraintComponent->SetAngularTwistLimit(ACM_Limited, FMath::Clamp(Profile.AnchorTwistLimitDegrees, 0.f, 180.f));
548: 	HipConstraintComponent->SetLinearPositionDrive(true, true, true);
549: 	HipConstraintComponent->SetLinearVelocityDrive(true, true, true);
550: 	HipConstraintComponent->SetAngularDriveMode(EAngularDriveMode::SLERP);
551: 	HipConstraintComponent->SetAngularOrientationDrive(true, true);
552: 	HipConstraintComponent->SetAngularVelocityDrive(true, true);
553: 	HipConstraintComponent->SetConstrainedComponents(Mesh, PelvisBodyName, HipAnchorComponent, NAME_None);
554: 	ApplyDriveMultipliers(Profile.BalancedPoseStrengthMultiplier, 1.f);
```

Drive params pushed by current multipliers:

```cpp
1103: 	{
1104: 		PhysicalAnimationComponent->SetStrengthMultiplyer(CurrentPoseMultiplier);
1105: 	}
1106: 
1107: 	if (HipConstraintComponent)
1108: 	{
1109: 		HipConstraintComponent->SetLinearDriveParams(
1110: 			Profile.AnchorLinearStrength * CurrentAnchorMultiplier,
1111: 			Profile.AnchorLinearVelocityStrength * CurrentAnchorMultiplier,
1112: 			Profile.AnchorLinearMaxForce * CurrentAnchorMultiplier);
1113: 		HipConstraintComponent->SetAngularDriveParams(
1114: 			Profile.AnchorAngularStrength * CurrentAnchorMultiplier,
1115: 			Profile.AnchorAngularVelocityStrength * CurrentAnchorMultiplier,
1116: 			Profile.AnchorAngularMaxForce * CurrentAnchorMultiplier);
1117: 	}
1118: }
```

Runtime state changes and recovering tick reapplication:

```cpp
590: 
591: 	switch (RuntimeState)
592: 	{
593: 	case ET66HeroPhysicsRuntimeState::Balanced:
594: 		ApplyGameplaySuppression(false);
595: 		ApplyDriveMultipliers(Profile.BalancedPoseStrengthMultiplier, 1.f);
596: 		break;
597: 	case ET66HeroPhysicsRuntimeState::Staggered:
598: 		ApplyGameplaySuppression(false);
599: 		ApplyDriveMultipliers(Profile.StaggerPoseStrengthMultiplier, Profile.StaggerAnchorStrengthMultiplier);
600: 		break;
601: 	case ET66HeroPhysicsRuntimeState::KnockedDown:
602: 		ApplyGameplaySuppression(true);
603: 		ApplyDriveMultipliers(Profile.KnockdownPoseStrengthMultiplier, Profile.KnockdownAnchorStrengthMultiplier);
604: 		break;
605: 	case ET66HeroPhysicsRuntimeState::Recovering:
606: 		ApplyGameplaySuppression(false);
607: 		ApplyDriveMultipliers(Profile.KnockdownPoseStrengthMultiplier, Profile.KnockdownAnchorStrengthMultiplier);
608: 		break;
609: 	default:
610: 		break;
611: 	}
624: void UT66HeroPhysicsComponent::UpdateStateMachine(const float DeltaTime)
625: {
626: 	UWorld* World = GetWorld();
627: 	const double Now = World ? World->GetTimeSeconds() : 0.0;
628: 	const float StateAge = static_cast<float>(Now - StateStartTimeSeconds);
629: 
630: 	switch (RuntimeState)
631: 	{
632: 	case ET66HeroPhysicsRuntimeState::Staggered:
633: 		if (StateAge >= Profile.StaggerSeconds)
634: 		{
635: 			SetRuntimeState(ET66HeroPhysicsRuntimeState::Recovering, FName(TEXT("StaggerElapsed")));
636: 		}
637: 		break;
638: 	case ET66HeroPhysicsRuntimeState::KnockedDown:
639: 		if (StateAge >= Profile.KnockdownHoldSeconds)
640: 		{
641: 			SetRuntimeState(ET66HeroPhysicsRuntimeState::Recovering, FName(TEXT("KnockdownHoldElapsed")));
642: 		}
643: 		break;
644: 	case ET66HeroPhysicsRuntimeState::Recovering:
645: 		RecoveryElapsedSeconds += FMath::Max(0.f, DeltaTime);
646: 		{
647: 			const float Alpha = FMath::Clamp(RecoveryElapsedSeconds / FMath::Max(0.01f, Profile.RecoverySeconds), 0.f, 1.f);
648: 			const float SmoothAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.0f);
649: 			ApplyDriveMultipliers(
650: 				FMath::Lerp(Profile.KnockdownPoseStrengthMultiplier, Profile.BalancedPoseStrengthMultiplier, SmoothAlpha),
651: 				FMath::Lerp(Profile.KnockdownAnchorStrengthMultiplier, 1.f, SmoothAlpha));
652: 			if (Alpha >= 1.f)
653: 			{
654: 				SetRuntimeState(ET66HeroPhysicsRuntimeState::Balanced, FName(TEXT("RecoveryComplete")));
655: 			}
656: 		}
657: 		break;
658: 	case ET66HeroPhysicsRuntimeState::Balanced:
659: 	default:
660: 		break;
661: 	}
662: 
663: 	if (Profile.MaxPelvisCapsuleDistance > 0.f)
664: 	{
665: 		const float Distance = ComputePelvisCapsuleDistance();
666: 		if (Distance > Profile.MaxPelvisCapsuleDistance)
667: 		{
668: 			if (Now - LastPelvisDivergenceLogTimeSeconds >= 0.5)
669: 			{
670: 				LastPelvisDivergenceLogTimeSeconds = Now;
671: 				UE_LOG(
672: 					LogT66HeroPhysics,
673: 					Warning,
674: 					TEXT("[HeroActiveRagdoll] Pelvis divergence %.1f exceeds %.1f; anchor strengthened without runtime body teleport."),
675: 					Distance,
676: 					Profile.MaxPelvisCapsuleDistance);
677: 			}
678: 			ApplyDriveMultipliers(Profile.BalancedPoseStrengthMultiplier, 1.f);
679: 			if (USkeletalMeshComponent* Mesh = ActiveMesh.Get())
680: 			{
681: 				Mesh->WakeAllRigidBodies();
682: 			}
683: 		}
684: 		else
685: 		{
686: 			LastPelvisDivergenceLogTimeSeconds = -9999.0;
687: 		}
688: 	}
689: }
```

Static facts:
- Linear limit: limited X/Y/Z, Profile.AnchorLinearLimit = 72.f.
- Linear drive type: both position drive and velocity drive are enabled for X/Y/Z.
- Balanced linear drive params: strength 8200, velocity strength 640, max force 62000 through CurrentAnchorMultiplier = 1.00.
- KnockedDown linear drive params during the logged hit: strength 1476, velocity strength 115.2, max force 11160 through CurrentAnchorMultiplier = 0.18.
- Angular limit: swing1/swing2 limited 68 degrees, twist limited 58 degrees.
- Angular drive type: SLERP, orientation drive enabled, velocity drive enabled.
- Balanced angular drive params: strength 4200, velocity strength 460, max force 32000 through CurrentAnchorMultiplier = 1.00.
- KnockedDown angular drive params during the logged hit: strength 756, velocity strength 82.8, max force 5760 through CurrentAnchorMultiplier = 0.18.
- UpdateStateMachine re-applies drive multipliers every tick only while RuntimeState == Recovering; otherwise reapplication occurs on SetRuntimeState, and additionally inside the pelvis-divergence branch if Distance > Profile.MaxPelvisCapsuleDistance.

Logged first hit state lines:

```text
[2026.06.07-10.43.55:225][ 36]LogT66HeroPhysics: Display: [HeroActiveRagdoll] State Balanced -> KnockedDown Reason=TestRoomWipeoutArm PoseMult=0.10 AnchorMult=0.18
[2026.06.07-10.43.55:945][109]LogT66HeroPhysics: Display: [HeroActiveRagdoll] State KnockedDown -> Recovering Reason=KnockdownHoldElapsed PoseMult=0.10 AnchorMult=0.18
[2026.06.07-10.43.56:668][168]LogT66HeroPhysics: Display: [HeroActiveRagdoll] State Recovering -> Balanced Reason=RecoveryComplete PoseMult=0.74 AnchorMult=1.00
```

### B4 - PAC state during a TestRoom wipeout hit

Active component PAC configuration:

```cpp
455: bool UT66HeroPhysicsComponent::ConfigurePhysicalAnimation()
456: {
457: 	AActor* Owner = GetOwner();
458: 	USkeletalMeshComponent* Mesh = ResolveMesh();
459: 	if (!Owner || !Mesh)
460: 	{
461: 		return false;
462: 	}
463: 
464: 	if (!PhysicalAnimationComponent)
465: 	{
466: 		PhysicalAnimationComponent = NewObject<UPhysicalAnimationComponent>(Owner, TEXT("T66HeroActivePhysicalAnimation"));
467: 		if (!PhysicalAnimationComponent)
468: 		{
469: 			return false;
470: 		}
471: 		Owner->AddInstanceComponent(PhysicalAnimationComponent);
472: 		PhysicalAnimationComponent->RegisterComponent();
473: 	}
474: 
475: 	FPhysicalAnimationData ChildDriveData;
476: 	ChildDriveData.bIsLocalSimulation = true;
477: 	ChildDriveData.OrientationStrength = FMath::Max(0.f, Profile.PoseOrientationStrength);
478: 	ChildDriveData.AngularVelocityStrength = FMath::Max(0.f, Profile.PoseAngularVelocityStrength);
479: 	ChildDriveData.PositionStrength = 0.f;
480: 	ChildDriveData.VelocityStrength = 0.f;
481: 	ChildDriveData.MaxLinearForce = 0.f;
482: 	ChildDriveData.MaxAngularForce = FMath::Max(0.f, Profile.PoseMaxAngularForce);
483: 
484: 	PhysicalAnimationComponent->SetSkeletalMeshComponent(Mesh);
485: 	const int32 DrivenBodies = Mesh->ForEachBodyBelow(SimulationRootBodyName, true, false, [](FBodyInstance*) {});
486: 	if (DrivenBodies <= 1)
487: 	{
488: 		return false;
489: 	}
490: 
491: 	// The hip anchor/constraint owns pelvis position. PAC is only a local muscle
492: 	// drive for descendants, so it must not become a second gameplay root.
493: 	PhysicalAnimationComponent->ApplyPhysicalAnimationSettingsBelow(SimulationRootBodyName, ChildDriveData, false);
```

Legacy TestRoom PAC toggle and profile assignment:

```cpp
213: static TAutoConsoleVariable<float> CVarT66TestRoomWipeoutArmPhysicalAnimationStrength(
214: 	TEXT("t66.TestRoom.WipeoutArmPhysicalAnimationStrength"),
215: 	0.42f,
216: 	TEXT("Legacy fallback knockback PAC strength. Stage 3 active ragdoll owns PAC through UT66HeroPhysicsComponent."),
217: 	ECVF_Default);
218: 
219: static TAutoConsoleVariable<int32> CVarT66TestRoomWipeoutArmEnablePhysicalAnimation(
220: 	TEXT("t66.TestRoom.WipeoutArmEnablePhysicalAnimation"),
221: 	0,
222: 	TEXT("Legacy fallback knockback PAC experiment toggle. Stage 3 hero active ragdoll uses UT66HeroPhysicsComponent PAC/hip-anchor authority instead."),
223: 	ECVF_Default);
497: 			Profile.bSuppressLookInput = CVarT66TestRoomWipeoutArmSuppressLookInput.GetValueOnGameThread() != 0;
498: 			Profile.bEnablePhysicalAnimation = CVarT66TestRoomWipeoutArmEnablePhysicalAnimation.GetValueOnGameThread() != 0;
499: 			Profile.PhysicalAnimationDriveMode = ResolveWipeoutArmPhysicalAnimationDriveMode();
500: 			Profile.bDetachMeshDuringRagdoll = true;
501: 			if (Profile.bEnablePhysicalAnimation
502: 				&& Profile.PhysicalAnimationDriveMode != ET66KnockbackPhysicalAnimationDriveMode::Disabled)
503: 			{
504: 				Profile.bEnablePhysicalAnimation = false;
505: 				Profile.PhysicalAnimationDriveMode = ET66KnockbackPhysicalAnimationDriveMode::Disabled;
506: 			}
507: 			Profile.PhysicalAnimationStrength = FMath::Clamp(CVarT66TestRoomWipeoutArmPhysicalAnimationStrength.GetValueOnGameThread(), 0.f, 2.f);
```

Active path call does not pass the legacy FT66KnockbackProfile to ApplyPhysicsReaction:

```cpp
565: 			FT66KnockbackProfile Profile = MakeWipeoutArmKnockbackProfile(Hero);
566: 			Profile.MaxLaunchVelocity = FMath::Max(Profile.MaxLaunchVelocity, LaunchVelocity.Size() * 1.05f);
567: 
568: 			bool bAppliedActiveRagdoll = false;
569: 			bool bTriedActiveRagdoll = false;
570: 			if (CVarT66TestRoomWipeoutArmUseHeroActiveRagdoll.GetValueOnGameThread() != 0)
571: 			{
572: 				if (UT66HeroPhysicsComponent* HeroPhysicsComponent = Hero->GetHeroPhysicsComponent())
573: 				{
574: 					bTriedActiveRagdoll = true;
575: 					bAppliedActiveRagdoll = HeroPhysicsComponent->ApplyPhysicsReaction(
576: 						LaunchVelocity,
577: 						ReactionHitLocation,
578: 						FName(TEXT("TestRoomWipeoutArm")));
```

Logged routed first hit:

```text
[2026.06.07-10.43.55:226][ 36]LogT66HeroPhysics: Display: [HeroActiveRagdoll] Reaction Applied=1 Source=TestRoomWipeoutArm State=KnockedDown RequestedVelocity=V(X=10277.89, Y=2148.24, Z=750.00) AppliedVelocity=V(X=6346.34, Y=1326.48, Z=463.11) Hit=V(X=889.15, Y=8.18, Z=126.15) Pelvis=pelvis PoseMult=0.10 AnchorMult=0.18
[2026.06.07-10.43.55:226][ 36]LogTemp: Display: TestRoom wipeout arm impact routed to hero physics: ActiveTried=1 ActiveApplied=1 LegacyApplied=0 Launch=V(X=10277.89, Y=2148.24, Z=750.00) Hit=V(X=889.15, Y=8.18, Z=126.15) Radial=V(X=1.00) Tangent=V(X=0.17, Y=0.99) Incap=0.15s MaxRagdoll=3.10s VelocityChange=0 BelowFraction=1.00 LinearDamping=0.01 AngularDamping=0.02 Friction=0.04 Restitution=0.72 LegacyProfilePAC=0 LegacyDriveMode=0
```

Static facts: WipeoutArmEnablePhysicalAnimation=0 controls the legacy fallback knockback profile. During the logged active TestRoom hit, LegacyProfilePAC=0 is logged, while the active component logs PoseMult=0.10 AnchorMult=0.18; active PAC/hip-anchor authority is from UT66HeroPhysicsComponent.

### B5 - Skeletal mesh simulation space and PAC local/world setting

Mesh physics flags:

```cpp
413: 	Mesh->UpdateComponentToWorld();
414: 	Mesh->PhysicsTransformUpdateMode = EPhysicsTransformUpdateMode::ComponentTransformIsKinematic;
415: 	Mesh->KinematicBonesUpdateType = EKinematicBonesUpdateToPhysics::SkipSimulatingBones;
416: 	Mesh->bBlendPhysics = true;
417: 	Mesh->SetAllBodiesPhysicsBlendWeight(0.f);
418: 	Mesh->SetAllBodiesSimulatePhysics(false);
419: 	Mesh->ResetAllBodiesSimulatePhysics();
420: 	Mesh->RecreatePhysicsState();
421: 	Mesh->UpdateComponentToWorld();
422: 	SyncKinematicMeshPoseToPhysics();
423: 	AlignMeshPelvisToCapsuleAnchor(FName(TEXT("InitMeshPelvisAnchorAlign")));
424: 	SyncKinematicMeshPoseToPhysics();
425: 	ActiveAuthorityMeshRelativeTransform = Mesh->GetRelativeTransform();
426: 	bHasActiveAuthorityMeshRelativeTransform = true;
427: 	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
428: 	Mesh->SetCollisionObjectType(ECC_PhysicsBody);
429: 	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
430: 	Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
431: 	Mesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
432: 	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
433: 	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
434: 	Mesh->SetGenerateOverlapEvents(false);
435: 	Mesh->SetEnableGravity(true);
436: 
437: 	if (Mesh->Bodies.Num() <= 0)
438: 	{
439: 		Mesh->RecreatePhysicsState();
440: 	}
441: 
442: 	Mesh->SetAllUseCCD(true);
443: 	Mesh->SetAllBodiesBelowSimulatePhysics(SimulationRootBodyName, true, true);
444: 	Mesh->SetAllBodiesBelowPhysicsBlendWeight(SimulationRootBodyName, 1.f, false, true);
```

PAC local setting:

```cpp
475: 	FPhysicalAnimationData ChildDriveData;
476: 	ChildDriveData.bIsLocalSimulation = true;
477: 	ChildDriveData.OrientationStrength = FMath::Max(0.f, Profile.PoseOrientationStrength);
478: 	ChildDriveData.AngularVelocityStrength = FMath::Max(0.f, Profile.PoseAngularVelocityStrength);
479: 	ChildDriveData.PositionStrength = 0.f;
480: 	ChildDriveData.VelocityStrength = 0.f;
481: 	ChildDriveData.MaxLinearForce = 0.f;
482: 	ChildDriveData.MaxAngularForce = FMath::Max(0.f, Profile.PoseMaxAngularForce);
483: 
484: 	PhysicalAnimationComponent->SetSkeletalMeshComponent(Mesh);
485: 	const int32 DrivenBodies = Mesh->ForEachBodyBelow(SimulationRootBodyName, true, false, [](FBodyInstance*) {});
486: 	if (DrivenBodies <= 1)
487: 	{
488: 		return false;
489: 	}
490: 
491: 	// The hip anchor/constraint owns pelvis position. PAC is only a local muscle
492: 	// drive for descendants, so it must not become a second gameplay root.
493: 	PhysicalAnimationComponent->ApplyPhysicalAnimationSettingsBelow(SimulationRootBodyName, ChildDriveData, false);
```

Runtime sample world-transform reads:

```cpp
1335: 	const FVector PelvisBoneLocation = Mesh && !PelvisBodyName.IsNone() && Mesh->GetBoneIndex(PelvisBodyName) != INDEX_NONE
1336: 		? Mesh->GetBoneLocation(PelvisBodyName, EBoneSpaces::WorldSpace)
1337: 		: FVector::ZeroVector;
1338: 	FVector PelvisBodyRawLocation = FVector::ZeroVector;
1339: 	if (const FBodyInstance* PelvisBody = Mesh && !PelvisBodyName.IsNone() ? Mesh->GetBodyInstance(PelvisBodyName) : nullptr)
1340: 	{
1341: 		if (PelvisBody->IsValidBodyInstance())
1342: 		{
1343: 			const FTransform PelvisBodyTransform = PelvisBody->GetUnrealWorldTransform();
1344: 			if (!PelvisBodyTransform.ContainsNaN())
1345: 			{
1346: 				PelvisBodyRawLocation = PelvisBodyTransform.GetLocation();
1347: 			}
1348: 		}
1349: 	}
1350: 	UE_LOG(
1351: 		LogT66HeroPhysics,
1352: 		Display,
1353: 		TEXT("[HeroActiveRagdoll] Sample State=%s PoseMult=%.2f AnchorMult=%.2f PelvisSimulating=%d PelvisCapsuleDist=%.1f RawPelvisDist=%.1f ComponentSpacePelvisDist=%.1f ComponentBonePelvisDist=%.1f Anchor=%s EffectivePelvis=%s BodyRaw=%s BoneRaw=%s MeshWorld=%s Parent=%s Bodies=%d Mesh=%s Delta=%.3f"),
1354: 		*T66HeroPhysicsStateName(RuntimeState),
1355: 		CurrentPoseMultiplier,
1356: 		CurrentAnchorMultiplier,
1357: 		bPelvisSimulating ? 1 : 0,
1358: 		ComputePelvisCapsuleDistance(),
1359: 		RawPelvisDistance,
1360: 		ComponentSpacePelvisDistance,
1361: 		ComponentBonePelvisDistance,
1362: 		*AnchorLocation.ToCompactString(),
1363: 		*EffectivePelvisLocation.ToCompactString(),
1364: 		*PelvisBodyRawLocation.ToCompactString(),
1365: 		*PelvisBoneLocation.ToCompactString(),
1366: 		Mesh ? *Mesh->GetComponentLocation().ToCompactString() : TEXT("(none)"),
1367: 		Mesh ? *GetNameSafe(Mesh->GetAttachParent()) : TEXT("(none)"),
1368: 		Mesh ? Mesh->Bodies.Num() : 0,
1369: 		Mesh && Mesh->GetSkeletalMeshAsset() ? *Mesh->GetSkeletalMeshAsset()->GetName() : TEXT("(none)"),
1370: 		DeltaTime);
```

Static facts: the mesh uses PhysicsTransformUpdateMode = ComponentTransformIsKinematic, KinematicBonesUpdateType = SkipSimulatingBones, and BlendPhysics = true; bodies below the simulation root are simulated with blend weight 1.f. PAC is configured with ChildDriveData.bIsLocalSimulation = true. Runtime sampling reads the pelvis body through PelvisBody->GetUnrealWorldTransform() and the pelvis bone through Mesh->GetBoneLocation(..., EBoneSpaces::WorldSpace).

## Part C - Mass Readback

No existing debug CVar path found that logs GetMass()/GetBodyMass() for every simulated body or a summed simulated mass. Gameplay source was not modified to add body-mass logging.

Impulse code uses GetBodyMass() but does not emit it:

```cpp
1216: void UT66HeroPhysicsComponent::ApplyMassScaledVelocityTargetToBody(
1217: 	const FVector& VelocityChange,
1218: 	const FName BodyName,
1219: 	const float Scale,
1220: 	const FVector& WorldHitLocation,
1221: 	const bool bUseHitLocation)
1222: {
1223: 	USkeletalMeshComponent* Mesh = ActiveMesh.Get();
1224: 	if (!Mesh || BodyName.IsNone() || Scale <= 0.f)
1225: 	{
1226: 		return;
1227: 	}
1228: 
1229: 	FBodyInstance* BodyInstance = Mesh->GetBodyInstance(BodyName);
1230: 	if (!BodyInstance)
1231: 	{
1232: 		return;
1233: 	}
1234: 
1235: 	if (!BodyInstance->IsValidBodyInstance() || !BodyInstance->IsInstanceSimulatingPhysics())
1236: 	{
1237: 		UE_LOG(
1238: 			LogT66HeroPhysics,
1239: 			Verbose,
1240: 			TEXT("[HeroActiveRagdoll] Skipped impulse on non-simulated body Body=%s Velocity=%s"),
1241: 			*BodyName.ToString(),
1242: 			*VelocityChange.ToCompactString());
1243: 		return;
1244: 	}
1245: 
1246: 	const FVector Impulse = VelocityChange * Scale * FMath::Max(1.f, BodyInstance->GetBodyMass());
1247: 	if (bUseHitLocation && !WorldHitLocation.ContainsNaN())
1248: 	{
1249: 		BodyInstance->AddImpulseAtPosition(Impulse, WorldHitLocation);
1250: 	}
1251: 	else
1252: 	{
1253: 		BodyInstance->AddImpulse(Impulse, false);
1254: 	}
1257: void UT66HeroPhysicsComponent::ApplyMassScaledVelocityTargetBelow(
1258: 	const FVector& VelocityChange,
1259: 	const FName BodyName,
1260: 	const float Scale,
1261: 	const bool bIncludeSelf)
1262: {
1263: 	USkeletalMeshComponent* Mesh = ActiveMesh.Get();
1264: 	if (!Mesh || BodyName.IsNone() || Scale <= 0.f)
1265: 	{
1266: 		return;
1267: 	}
1268: 
1269: 	Mesh->ForEachBodyBelow(BodyName, bIncludeSelf, false, [&VelocityChange, Scale](FBodyInstance* BodyInstance)
1270: 	{
1271: 		if (!BodyInstance || !BodyInstance->IsValidBodyInstance() || !BodyInstance->IsInstanceSimulatingPhysics())
1272: 		{
1273: 			return;
1274: 		}
1275: 
1276: 		BodyInstance->AddImpulse(VelocityChange * Scale * FMath::Max(1.f, BodyInstance->GetBodyMass()), false);
1277: 	});
```

The Part E runtime log emits Bodies=18, but not individual body masses. Pelvis mass: UNAVAILABLE_existing_runtime_log_does_not_emit. Total simulated mass: UNAVAILABLE_existing_runtime_log_does_not_emit.

Existing asset report with body mass_scale values, not runtime mass values: C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\physicsfirst_asset_report.json.

```json
"bone_name": "pelvis",
"mass_scale": 2.600,
"bone_name": "spine_01",
"mass_scale": 1.800,
"bone_name": "spine_02",
"mass_scale": 1.800,
"bone_name": "spine_03",
"mass_scale": 1.800,
"bone_name": "neck_01",
"mass_scale": 0.900,
"bone_name": "head",
"mass_scale": 1.100,
"bone_name": "upperarm_l",
"mass_scale": 0.700,
"bone_name": "lowerarm_l",
"mass_scale": 0.550,
"bone_name": "hand_l",
"mass_scale": 0.350,
"bone_name": "upperarm_r",
"mass_scale": 0.700,
"bone_name": "lowerarm_r",
"mass_scale": 0.550,
"bone_name": "hand_r",
"mass_scale": 0.350,
"bone_name": "thigh_l",
"mass_scale": 0.950,
"bone_name": "calf_l",
"mass_scale": 0.750,
"bone_name": "foot_l",
"mass_scale": 0.650,
"bone_name": "thigh_r",
"mass_scale": 0.950,
"bone_name": "calf_r",
"mass_scale": 0.750,
"bone_name": "foot_r",
"mass_scale": 0.650,
```

## Part D - Floor And Collision Facts

TestRoom floor surfaces are spawned through SpawnRectSurface, which calls SpawnCubeSurface:

```cpp
815: 			if (UStaticMeshComponent* MeshComponent = Surface->GetStaticMeshComponent())
816: 			{
817: 				MeshComponent->SetMobility(EComponentMobility::Movable);
818: 				MeshComponent->SetStaticMesh(CubeMesh);
819: 				if (Material)
820: 				{
821: 					MeshComponent->SetMaterial(0, Material);
822: 				}
823: 				MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
824: 				MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
825: 				Surface->SetActorScale3D(Scale);
826: 				FT66WorldVisualSetup::RegisterToonMaterial(MeshComponent, ET66ToonMaterialKind::Environment);
827: 				const bool bCeilingSurface = Label && FCString::Stristr(Label, TEXT("Ceiling")) != nullptr;
828: 				if (bCeilingSurface && CVarT66TestRoomShowCeiling.GetValueOnGameThread() == 0)
829: 				{
830: 					MeshComponent->SetHiddenInGame(true);
831: 					MeshComponent->SetVisibility(false, true);
832: 				}
833: 				MeshComponent->SetMobility(EComponentMobility::Static);
837: 		void SpawnRectSurface(
838: 			UWorld* World,
839: 			UStaticMesh* CubeMesh,
840: 			UMaterialInterface* Material,
841: 			const FString& Label,
842: 			const FBox2D& Rect,
843: 			const float Z,
844: 			const float Thickness)
845: 		{
846: 			const FVector Location(
847: 				(Rect.Min.X + Rect.Max.X) * 0.5f,
848: 				(Rect.Min.Y + Rect.Max.Y) * 0.5f,
849: 				Z);
850: 			const FVector Scale(
851: 				(Rect.Max.X - Rect.Min.X) / TestRoomCubeSize,
852: 				(Rect.Max.Y - Rect.Min.Y) / TestRoomCubeSize,
853: 				Thickness / TestRoomCubeSize);
854: 			SpawnCubeSurface(World, CubeMesh, Material, *Label, Location, Scale);
855: 		}
1759: 		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_CenterFloor"), CenterBox, FloorZ, TestRoomWallThickness);
1760: 		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_MobRoomFloor"), NorthRoomBox, FloorZ, TestRoomWallThickness);
1761: 		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_BossRoomFloor"), EastRoomBox, FloorZ, TestRoomWallThickness);
1762: 		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_SouthEmptyRoomFloor"), SouthRoomBox, FloorZ, TestRoomWallThickness);
1763: 		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_WestEmptyRoomFloor"), WestRoomBox, FloorZ, TestRoomWallThickness);
1764: 		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_NorthCorridorFloor"), NorthCorridorBox, FloorZ, TestRoomWallThickness);
1765: 		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_SouthCorridorFloor"), SouthCorridorBox, FloorZ, TestRoomWallThickness);
1766: 		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_EastCorridorFloor"), EastCorridorBox, FloorZ, TestRoomWallThickness);
1767: 		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_WestCorridorFloor"), WestCorridorBox, FloorZ, TestRoomWallThickness);
```

Static fact: floors use SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics) and SetCollisionResponseToAllChannels(ECR_Block). No later floor-specific ECC_PhysicsBody override was found in T66GameMode_TestRoom.cpp; therefore the quoted setup blocks ECC_PhysicsBody via all-channel block.

Hero capsule setup/restoration:

```cpp
70: AT66HeroBase::AT66HeroBase()
71: {
72: 	PrimaryActorTick.bCanEverTick = true;
73: 
74: 	// ========== Capsule Setup ==========
75: 	GetCapsuleComponent()->SetCapsuleHalfHeight(T66HeroHeightChadUU * 0.5f);
76: 	GetCapsuleComponent()->SetCapsuleRadius(T66HeroCapsuleRadiusUU);
77: 
532: 	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
533: 	{
534: 		if (bMounted)
535: 		{
536: 			Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
537: 		}
538: 		else
539: 		{
540: 			Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
541: 			Capsule->SetCollisionProfileName(TEXT("Pawn"));
542: 		}
```

Project config search found no project override for Profiles=(Name="Pawn" and no DefaultChannelResponses entry in Config. The quoted hero setup sets collision enabled to QueryAndPhysics and collision profile name to Pawn; this report does not contain a runtime component-query readback of the Pawn profile's ECC_PhysicsBody response.

## Part E - Hit-Moment Telemetry Trace

Capture command line from the run log:

```text
LogInit: Command Line: /Game/Maps/GameplayLevel -game -windowed -ResX=1280 -ResY=720 -T66AutomationResX=1280 -T66AutomationResY=720 -T66AutomationWindowed -T66GameplayAutoCapture=heroactiveragdollproof -T66GameplayAutoScreenshotSequenceDir=C:\UE\T66\Reports\Proof\Physics\HeroRagdollDiagnosis_20260607_074119\frames_hero1chad -T66GameplayAutoScreenshotSequencePrefix=frame -T66GameplayAutoScreenshotSequenceCount=64 -T66GameplayAutoScreenshotSequenceInterval=0.0625 -T66GameplayAutoScreenshotDelay=3 -T66GameplayAutoPostCaptureScreenshotDelay=0.1 -unattended -nop4 -nosplash -ExecCmds="t66.HeroPhysics.DebugLog 1,t66.HeroPhysics.DebugLogInterval 0.01" -T66HeroVisualOverride=Hero_1_Chad -T66TestRoomRagdollProofCameraDistance=850 -T66TestRoomRagdollProofCameraSideOffset=-260 -T66TestRoomRagdollProofFocusForwardOffset=500 -T66TestRoomRagdollProofCameraFOV=90 -T66TestRoomRagdollProofCameraHeight=850 -T66TestRoomRagdollProofMaxCameraZ=1400 -T66TestRoomRagdollProofFocusZ=130 -T66TestRoomRagdollProofFocusHeightOffset=40 -T66AutomationTestRoom -T66AutoCaptureHeroHPOverride=20000
```

Debug CVar echo from the run log:

```text
[2026.06.07-10.43.54:567][  0]t66.HeroPhysics.DebugLog = "1"
[2026.06.07-10.43.54:567][  0]t66.HeroPhysics.DebugLogInterval = "0.01"
```

Source clamp for the debug interval:

```cpp
1307: void UT66HeroPhysicsComponent::EmitRuntimeSample(const float DeltaTime)
1308: {
1309: 	if (CVarT66HeroPhysicsDebugLog.GetValueOnGameThread() == 0)
1310: 	{
1311: 		return;
1312: 	}
1313: 
1314: 	UWorld* World = GetWorld();
1315: 	const double Now = World ? World->GetTimeSeconds() : 0.0;
1316: 	const float Interval = FMath::Clamp(CVarT66HeroPhysicsDebugLogInterval.GetValueOnGameThread(), 0.05f, 5.f);
1317: 	if (Now - LastRuntimeLogTimeSeconds < Interval)
```

First-hit reaction line used as the CSV impact point:

```text
[2026.06.07-10.43.55:226][ 36]LogT66HeroPhysics: Display: [HeroActiveRagdoll] Reaction Applied=1 Source=TestRoomWipeoutArm State=KnockedDown RequestedVelocity=V(X=10277.89, Y=2148.24, Z=750.00) AppliedVelocity=V(X=6346.34, Y=1326.48, Z=463.11) Hit=V(X=889.15, Y=8.18, Z=126.15) Pelvis=pelvis PoseMult=0.10 AnchorMult=0.18
[2026.06.07-10.43.55:226][ 36]LogTemp: Display: TestRoom wipeout arm impact routed to hero physics: ActiveTried=1 ActiveApplied=1 LegacyApplied=0 Launch=V(X=10277.89, Y=2148.24, Z=750.00) Hit=V(X=889.15, Y=8.18, Z=126.15) Radial=V(X=1.00) Tangent=V(X=0.17, Y=0.99) Incap=0.15s MaxRagdoll=3.10s VelocityChange=0 BelowFraction=1.00 LinearDamping=0.01 AngularDamping=0.02 Friction=0.04 Restitution=0.72 LegacyProfilePAC=0 LegacyDriveMode=0
```

Full run reaction rows: $(C:\UE\T66\Reports\Proof\Physics\HeroRagdollDiagnosis_20260607_074119\T66_heroactiveragdoll_debug.log:880:[2026.06.07-10.43.55:226][ 36]LogT66HeroPhysics: Display: [HeroActiveRagdoll] Reaction Applied=1 Source=TestRoomWipeoutArm State=KnockedDown RequestedVelocity=V(X=10277.89, Y=2148.24, Z=750.00) AppliedVelocity=V(X=6346.34, Y=1326.48, Z=463.11) Hit=V(X=889.15, Y=8.18, Z=126.15) Pelvis=pelvis PoseMult=0.10 AnchorMult=0.18 C:\UE\T66\Reports\Proof\Physics\HeroRagdollDiagnosis_20260607_074119\T66_heroactiveragdoll_debug.log:960:[2026.06.07-10.44.00:246][238]LogT66HeroPhysics: Display: [HeroActiveRagdoll] Reaction Applied=1 Source=TestRoomWipeoutArm State=KnockedDown RequestedVelocity=V(X=10262.69, Y=2219.74, Z=750.00) AppliedVelocity=V(X=6336.95, Y=1370.63, Z=463.11) Hit=V(X=889.10, Y=8.46, Z=126.15) Pelvis=pelvis PoseMult=0.10 AnchorMult=0.18 C:\UE\T66\Reports\Proof\Physics\HeroRagdollDiagnosis_20260607_074119\T66_heroactiveragdoll_debug.log:1023:[2026.06.07-10.44.04:948][267]LogT66HeroPhysics: Display: [HeroActiveRagdoll] Reaction Applied=1 Source=TestRoomWipeoutArm State=KnockedDown RequestedVelocity=V(X=10195.31, Y=2511.10, Z=750.00) AppliedVelocity=V(X=6295.34, Y=1550.54, Z=463.11) Hit=V(X=1006.54, Y=35.02, Z=126.15) Pelvis=pelvis PoseMult=0.10 AnchorMult=0.18 C:\UE\T66\Reports\Proof\Physics\HeroRagdollDiagnosis_20260607_074119\T66_heroactiveragdoll_debug.log:1091:[2026.06.07-10.44.09:532][421]LogT66HeroPhysics: Display: [HeroActiveRagdoll] Reaction Applied=1 Source=TestRoomWipeoutArm State=KnockedDown RequestedVelocity=V(X=10153.04, Y=2676.88, Z=750.00) AppliedVelocity=V(X=6269.25, Y=1652.90, Z=463.11) Hit=V(X=1123.16, Y=64.42, Z=126.15) Pelvis=pelvis PoseMult=0.10 AnchorMult=0.18.Count). The CSV extracts only the first-hit window from $(@{timestamp=2026-06-07 10:43:54.824; relative_seconds_to_impact=-0.402; source_log_frame=4; state=Balanced; cmc_movement_mode=UNAVAILABLE_existing_debug_log_does_not_emit; capsule_world_location=V(X=850.00, Z=102.15); capsule_velocity=UNAVAILABLE_existing_debug_log_does_not_emit; pelvis_body_world_location=V(X=-33.79, Y=-7.97, Z=-54.77); pelvis_body_linear_velocity=UNAVAILABLE_existing_debug_log_does_not_emit; mesh_component_world_location=V(X=850.00, Y=1.00, Z=30.15); effective_pelvis_capsule_distance=143.0; impulse_magnitude_applied_at_impact=UNAVAILABLE_body_mass_not_emitted; applied_velocity_change_magnitude_from_reaction_log=; anchor_constraint_current_linear_force=UNAVAILABLE_existing_debug_log_does_not_emit; anchor_constraint_violation_distance=143.0; anchor_world_location=V(X=850.00, Z=114.15); effective_pelvis_world_location=V(X=816.21, Y=-6.97, Z=-24.62); source_sample_line=[2026.06.07-10.43.54:824][  4]LogT66HeroPhysics: Display: [HeroActiveRagdoll] Sample State=Balanced PoseMult=0.74 AnchorMult=1.00 PelvisSimulating=1 PelvisCapsuleDist=143.0 RawPelvisDist=899.8 ComponentSpacePelvisDist=143.0 ComponentBonePelvisDist=899.8 Anchor=V(X=850.00, Z=114.15) EffectivePelvis=V(X=816.21, Y=-6.97, Z=-24.62) BodyRaw=V(X=-33.79, Y=-7.97, Z=-54.77) BoneRaw=V(X=-33.79, Y=-7.97, Z=-54.77) MeshWorld=V(X=850.00, Y=1.00, Z=30.15) Parent=None Bodies=18 Mesh=SK_Hero_1_Chad_PhysicsFirst Delta=0.171; nearest_sync_line=[2026.06.07-10.43.54:824][  4]LogT66HeroPhysics: Display: [HeroActiveRagdoll] MeshCapsuleAuthoritySync Reason=TickCapsuleAuthority WasAttached=0 PreviousParent=None Parent=None LocationDelta=103.8 RotationDeltaDeg=0.0 ScaleDelta=0.000 Hero=V(X=850.00, Z=102.15) MeshBefore=V(X=850.00, Y=1.00, Z=133.92) MeshAfter=V(X=850.00, Y=1.00, Z=30.15) Desired=V(X=850.00, Y=1.00, Z=30.15) Anchor=V(X=850.00, Z=114.15)}.timestamp) through $(@{timestamp=2026-06-07 10:43:56.688; relative_seconds_to_impact=1.462; source_log_frame=170; state=Balanced; cmc_movement_mode=UNAVAILABLE_existing_debug_log_does_not_emit; capsule_world_location=V(X=964.00, Y=23.83, Z=102.15); capsule_velocity=UNAVAILABLE_existing_debug_log_does_not_emit; pelvis_body_world_location=V(X=0.10, Y=3.03, Z=-71.92); pelvis_body_linear_velocity=UNAVAILABLE_existing_debug_log_does_not_emit; mesh_component_world_location=V(X=964.00, Y=24.83, Z=30.15); effective_pelvis_capsule_distance=156.0; impulse_magnitude_applied_at_impact=UNAVAILABLE_body_mass_not_emitted; applied_velocity_change_magnitude_from_reaction_log=; anchor_constraint_current_linear_force=UNAVAILABLE_existing_debug_log_does_not_emit; anchor_constraint_violation_distance=156.0; anchor_world_location=V(X=964.00, Y=23.83, Z=114.15); effective_pelvis_world_location=V(X=964.10, Y=27.86, Z=-41.77); source_sample_line=[2026.06.07-10.43.56:688][170]LogT66HeroPhysics: Display: [HeroActiveRagdoll] Sample State=Balanced PoseMult=0.74 AnchorMult=1.00 PelvisSimulating=1 PelvisCapsuleDist=156.0 RawPelvisDist=981.9 ComponentSpacePelvisDist=156.0 ComponentBonePelvisDist=982.0 Anchor=V(X=964.00, Y=23.83, Z=114.15) EffectivePelvis=V(X=964.10, Y=27.86, Z=-41.77) BodyRaw=V(X=0.10, Y=3.03, Z=-71.92) BoneRaw=V(X=0.02, Y=3.95, Z=-71.87) MeshWorld=V(X=964.00, Y=24.83, Z=30.15) Parent=None Bodies=18 Mesh=SK_Hero_1_Chad_PhysicsFirst Delta=0.012; nearest_sync_line=[2026.06.07-10.43.56:688][170]LogT66HeroPhysics: Display: [HeroActiveRagdoll] MeshCapsuleAuthoritySync Reason=TickCapsuleAuthority WasAttached=0 PreviousParent=None Parent=None LocationDelta=0.0 RotationDeltaDeg=0.0 ScaleDelta=0.000 Hero=V(X=964.00, Y=23.83, Z=102.15) MeshBefore=V(X=964.00, Y=24.83, Z=30.15) MeshAfter=V(X=964.00, Y=24.83, Z=30.15) Desired=V(X=964.00, Y=24.83, Z=30.15) Anchor=V(X=964.00, Y=23.83, Z=114.15)}.timestamp), with $rows sample rows, covering $(@{timestamp=2026-06-07 10:43:54.824; relative_seconds_to_impact=-0.402; source_log_frame=4; state=Balanced; cmc_movement_mode=UNAVAILABLE_existing_debug_log_does_not_emit; capsule_world_location=V(X=850.00, Z=102.15); capsule_velocity=UNAVAILABLE_existing_debug_log_does_not_emit; pelvis_body_world_location=V(X=-33.79, Y=-7.97, Z=-54.77); pelvis_body_linear_velocity=UNAVAILABLE_existing_debug_log_does_not_emit; mesh_component_world_location=V(X=850.00, Y=1.00, Z=30.15); effective_pelvis_capsule_distance=143.0; impulse_magnitude_applied_at_impact=UNAVAILABLE_body_mass_not_emitted; applied_velocity_change_magnitude_from_reaction_log=; anchor_constraint_current_linear_force=UNAVAILABLE_existing_debug_log_does_not_emit; anchor_constraint_violation_distance=143.0; anchor_world_location=V(X=850.00, Z=114.15); effective_pelvis_world_location=V(X=816.21, Y=-6.97, Z=-24.62); source_sample_line=[2026.06.07-10.43.54:824][  4]LogT66HeroPhysics: Display: [HeroActiveRagdoll] Sample State=Balanced PoseMult=0.74 AnchorMult=1.00 PelvisSimulating=1 PelvisCapsuleDist=143.0 RawPelvisDist=899.8 ComponentSpacePelvisDist=143.0 ComponentBonePelvisDist=899.8 Anchor=V(X=850.00, Z=114.15) EffectivePelvis=V(X=816.21, Y=-6.97, Z=-24.62) BodyRaw=V(X=-33.79, Y=-7.97, Z=-54.77) BoneRaw=V(X=-33.79, Y=-7.97, Z=-54.77) MeshWorld=V(X=850.00, Y=1.00, Z=30.15) Parent=None Bodies=18 Mesh=SK_Hero_1_Chad_PhysicsFirst Delta=0.171; nearest_sync_line=[2026.06.07-10.43.54:824][  4]LogT66HeroPhysics: Display: [HeroActiveRagdoll] MeshCapsuleAuthoritySync Reason=TickCapsuleAuthority WasAttached=0 PreviousParent=None Parent=None LocationDelta=103.8 RotationDeltaDeg=0.0 ScaleDelta=0.000 Hero=V(X=850.00, Z=102.15) MeshBefore=V(X=850.00, Y=1.00, Z=133.92) MeshAfter=V(X=850.00, Y=1.00, Z=30.15) Desired=V(X=850.00, Y=1.00, Z=30.15) Anchor=V(X=850.00, Z=114.15)}.relative_seconds_to_impact)s to $(@{timestamp=2026-06-07 10:43:56.688; relative_seconds_to_impact=1.462; source_log_frame=170; state=Balanced; cmc_movement_mode=UNAVAILABLE_existing_debug_log_does_not_emit; capsule_world_location=V(X=964.00, Y=23.83, Z=102.15); capsule_velocity=UNAVAILABLE_existing_debug_log_does_not_emit; pelvis_body_world_location=V(X=0.10, Y=3.03, Z=-71.92); pelvis_body_linear_velocity=UNAVAILABLE_existing_debug_log_does_not_emit; mesh_component_world_location=V(X=964.00, Y=24.83, Z=30.15); effective_pelvis_capsule_distance=156.0; impulse_magnitude_applied_at_impact=UNAVAILABLE_body_mass_not_emitted; applied_velocity_change_magnitude_from_reaction_log=; anchor_constraint_current_linear_force=UNAVAILABLE_existing_debug_log_does_not_emit; anchor_constraint_violation_distance=156.0; anchor_world_location=V(X=964.00, Y=23.83, Z=114.15); effective_pelvis_world_location=V(X=964.10, Y=27.86, Z=-41.77); source_sample_line=[2026.06.07-10.43.56:688][170]LogT66HeroPhysics: Display: [HeroActiveRagdoll] Sample State=Balanced PoseMult=0.74 AnchorMult=1.00 PelvisSimulating=1 PelvisCapsuleDist=156.0 RawPelvisDist=981.9 ComponentSpacePelvisDist=156.0 ComponentBonePelvisDist=982.0 Anchor=V(X=964.00, Y=23.83, Z=114.15) EffectivePelvis=V(X=964.10, Y=27.86, Z=-41.77) BodyRaw=V(X=0.10, Y=3.03, Z=-71.92) BoneRaw=V(X=0.02, Y=3.95, Z=-71.87) MeshWorld=V(X=964.00, Y=24.83, Z=30.15) Parent=None Bodies=18 Mesh=SK_Hero_1_Chad_PhysicsFirst Delta=0.012; nearest_sync_line=[2026.06.07-10.43.56:688][170]LogT66HeroPhysics: Display: [HeroActiveRagdoll] MeshCapsuleAuthoritySync Reason=TickCapsuleAuthority WasAttached=0 PreviousParent=None Parent=None LocationDelta=0.0 RotationDeltaDeg=0.0 ScaleDelta=0.000 Hero=V(X=964.00, Y=23.83, Z=102.15) MeshBefore=V(X=964.00, Y=24.83, Z=30.15) MeshAfter=V(X=964.00, Y=24.83, Z=30.15) Desired=V(X=964.00, Y=24.83, Z=30.15) Anchor=V(X=964.00, Y=23.83, Z=114.15)}.relative_seconds_to_impact)s relative to the first impact. The next Reaction Applied line in the full log occurs outside this CSV window.

CSV: C:\UE\T66\Reports\Proof\Physics\HeroRagdollDiagnosis_20260607_074119\hit_moment_first_wipeout_window.csv
Log: C:\UE\T66\Reports\Proof\Physics\HeroRagdollDiagnosis_20260607_074119\T66_heroactiveragdoll_debug.log
Video: C:\UE\T66\Reports\Proof\Physics\HeroRagdollDiagnosis_20260607_074119\heroactiveragdoll_debug_hero1chad.mp4
Frames: C:\UE\T66\Reports\Proof\Physics\HeroRagdollDiagnosis_20260607_074119\frames_hero1chad\

Existing debug log fields did not include CMC movement mode, capsule velocity, pelvis-body linear velocity, body mass, impulse magnitude, or constraint current linear force. Those CSV columns are marked UNAVAILABLE_*. The CSV includes pplied_velocity_change_magnitude_from_reaction_log derived from the logged AppliedVelocity vector for the impact row.
