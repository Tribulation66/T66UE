# T66 Physics And Rigging Master Audit

Date: 2026-06-07

Purpose: give a new analysis agent the full current context for T66's Fall Guys-like hero physics effort, including current runtime infrastructure, Hero 1 Chad rigging and animation infrastructure, settings, proof artifacts, history, failed attempts, and rejected directions.

## 1. Honest Current Status

The current Stage 3 implementation is not accepted as working gameplay.

The user tested the latest build and reported:

- the hero still has several problems
- movement can still spazz or stretch
- the hero can look like it is being constantly pulled somewhere while moving
- obstacle impact produces only a small bump and does not feel like Fall Guys
- the existing proof artifacts show that code paths execute, but not that the gameplay feel is correct

The most important handoff point is this: do not treat the current Stage 3 proof as "the system works." Treat it as proof that a specific code path initializes, routes wipeout-arm hits through the active-ragdoll component, logs simulated pelvis state, and recovers through the state machine. The subjective gameplay result is still failing.

## 2. Current Target Feel

The target is a Fall Guys-like playable hero feel:

- always slightly unstable and bouncy, not a normal rigid character until hit
- capsule remains playable and controllable
- body visibly wobbles during ordinary movement
- obstacles physically perturb, tumble, shove, rebound, and knock the hero down
- recovery is fast and physical
- animations are pose targets and readability helpers, not the whole motion
- traps/obstacles should request physics reactions rather than each inventing bespoke launch math

The exact values must be T66-authored and tuned. The project is not copying Fall Guys art or proprietary values.

## 3. Canonical Live Files

Physics docs:

- `Gameplay/Physics/README.md`
- `Gameplay/Physics/PHYSICS_AGENTS.md`
- `Gameplay/Physics/MASTER_PHYSICS.md`
- `Gameplay/Physics/HeroPhysicsModel.md`
- `Gameplay/Physics/PhysicsReactionProfiles.md`
- `Gameplay/Physics/PhysicsAssetPipeline.md`
- `Source/T66/Gameplay/Physics/pending_issues_Physics.md`

Runtime physics:

- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h`
- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp`
- `Source/T66/Gameplay/T66KnockbackComponent.h`
- `Source/T66/Gameplay/T66KnockbackComponent.cpp`
- `Source/T66/Gameplay/T66HeroBase.h`
- `Source/T66/Gameplay/T66HeroBase.cpp`
- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`

Movement:

- `Gameplay/Movement/MASTER_MOVEMENT.md`
- `Source/T66/Gameplay/Movement/T66HeroMovementTypes.h`
- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.h`
- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp`
- `Config/DefaultInput.ini`

Traps and obstacle ownership:

- `Gameplay/Traps/MASTER_TRAPS.md`
- `Source/T66/Core/T66TrapSubsystem.h`
- `Source/T66/Core/T66TrapSubsystem.cpp`
- `Source/T66/Gameplay/Traps/`

Rigging and import:

- `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md`
- `Model Generation/Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`
- `Content/Data/CharacterVisuals.csv`
- `Reports/AgentReviews/FallGuysHeroRiggingStage2Implementation/physics_first_hero1_unreal_import_report.json`
- `Reports/AgentReviews/FallGuysHeroRiggingStage2Implementation/physics_first_hero1_physics_asset_report.json`

Stage 3 proof folder:

- `Reports/Proof/Physics/HeroRagdollStage3AuthorityRebuild/`

## 4. Current Stage 3 Authority Model

The current Stage 3 architecture is built around one gameplay authority:

- the capsule is gameplay/input/navigation/camera authority
- `UCharacterMovementComponent` still owns normal walking and falling
- the skeletal mesh component is intended to keep kinematic component-transform authority
- Chaos simulates the pelvis/body chain below the mesh component
- a hidden kinematic hip anchor follows the capsule
- a `UPhysicsConstraintComponent` connects the hip anchor to the simulated pelvis body
- `UPhysicalAnimationComponent` drives bodies toward the authored pose clips as muscle
- obstacle reactions apply simulated-body impulse plus only a bounded capsule shove
- normal runtime is not supposed to use actor-to-pelvis follow, `SimulationUpdatesComponentTransform`, or repeated pelvis/body teleport loops

Important nuance: Unreal can report the skeletal mesh parent as `None` during active body simulation. The current docs treat that as acceptable only if the component transform remains kinematic, the capsule remains gameplay authority, effective pelvis/capsule distance is bounded, and shutdown restores the pre-active mesh relative transform.

## 5. `UT66HeroPhysicsComponent` Runtime Infrastructure

Component:

- class: `UT66HeroPhysicsComponent`
- profile struct: `FT66HeroPhysicsProfile`
- runtime states: `Balanced`, `Staggered`, `KnockedDown`, `Recovering`
- active only when `t66.HeroPhysics.EnableActiveRagdoll=1`
- currently Hero 1 Chad only by default
- active mesh requirement checks for Hero body type Chad and mesh asset name containing both `Hero_1_Chad` and `PhysicsFirst`

Initialization path:

1. `AT66HeroBase::InitializeHero()` applies the visual from `CharacterVisuals.csv`.
2. `HeroPhysicsComponent->ShutdownActiveRagdoll()` is called before reinitializing after visual changes.
3. Movement animation assets are cached.
4. `HeroPhysicsComponent->InitializeForHero(this, GetMesh())` is called.
5. `TryInitializeActiveRagdoll()` checks CVar/profile/Hero 1 mesh.
6. The component caches `PreActiveMeshRelativeTransform`.
7. `ResolveRequiredBodies()` finds `SimulationRootBodyName` and `PelvisBodyName`, usually `pelvis`.
8. `ConfigureMeshPhysics()` sets mesh physics state, aligns pelvis to capsule anchor, enables collision, CCD, and simulation below pelvis.
9. `ConfigurePhysicalAnimation()` creates or reuses a `UPhysicalAnimationComponent`, sets local simulation, and applies drive settings below the simulation root.
10. `ConfigureHipAnchorConstraint()` creates a hidden `USphereComponent` anchor and a `UPhysicsConstraintComponent` constrained to mesh pelvis.
11. The component logs `Init OK ... PelvisSimulating=1 ... PACChildrenLocal=1 HipAnchorConstraint=1`.

Tick path:

- `SyncMeshComponentToCapsuleAuthority("TickCapsuleAuthority")`
- `UpdateAnchorTransform()`
- `UpdateStateMachine(DeltaTime)`
- `ApplyBalanceWobble(DeltaTime)`
- `EmitRuntimeSample(DeltaTime)` when debug logging is enabled

Reaction path:

- `ApplyPhysicsReaction(RequestedVelocityChange, WorldHitLocation, SourceTag)`
- synchronizes mesh to capsule authority before applying the reaction
- enforces reaction cooldown
- clamps the requested velocity change
- requires the pelvis body to be simulating
- classifies `Staggered` versus `KnockedDown`
- applies mass-scaled impulse to pelvis at hit location
- applies secondary impulse to bodies below the simulation root
- applies bounded capsule shove through `LaunchCharacter`
- wakes all rigid bodies
- logs `Reaction Applied=1`

Shutdown path:

- clears gameplay suppression
- resets physics blend and simulation
- disables mesh collision
- restores cached pre-active mesh relative transform through `EnsureMeshAttachedToHeroRoot`
- destroys hip constraint, hip anchor, and PAC component
- resets state to `Balanced`

## 6. Stage 3 Profile Settings

Current defaults from `FT66HeroPhysicsProfile`:

| Setting | Current value |
|---|---:|
| `bEnabled` | `true` |
| `bHero1ChadOnly` | `true` |
| `SimulationRootBodyName` | `pelvis` |
| `PelvisBodyName` | `pelvis` |
| `PoseOrientationStrength` | `520` |
| `PoseAngularVelocityStrength` | `70` |
| `PoseMaxAngularForce` | `26000` |
| `BalancedPoseStrengthMultiplier` | `0.74` |
| `AnchorRelativeLocation` | `(0, 0, 12)` |
| `AnchorLinearLimit` | `72` |
| `AnchorLinearStrength` | `8200` |
| `AnchorLinearVelocityStrength` | `640` |
| `AnchorLinearMaxForce` | `62000` |
| `AnchorSwingLimitDegrees` | `68` |
| `AnchorTwistLimitDegrees` | `58` |
| `AnchorAngularStrength` | `4200` |
| `AnchorAngularVelocityStrength` | `460` |
| `AnchorAngularMaxForce` | `32000` |
| `KnockdownSpeedThreshold` | `1800` |
| `ReactionImpulseScale` | `1.0` |
| `BelowBodiesImpulseFraction` | `0.72` |
| `MaxReactionVelocityChange` | `6500` |
| `CapsuleReactionVelocityFraction` | `0.38` |
| `MaxCapsuleReactionVelocityChange` | `2800` |
| `StaggerPoseStrengthMultiplier` | `0.34` |
| `StaggerAnchorStrengthMultiplier` | `0.46` |
| `KnockdownPoseStrengthMultiplier` | `0.10` |
| `KnockdownAnchorStrengthMultiplier` | `0.18` |
| `StaggerSeconds` | `0.42` |
| `KnockdownHoldSeconds` | `0.72` |
| `RecoverySeconds` | `0.72` |
| `ReactionCooldownSeconds` | `0.18` |
| `BalanceWobbleVelocityChange` | `16` |
| `BalanceWobbleIntervalSeconds` | `0.14` |
| `MaxPelvisCapsuleDistance` | `360` |

Console variables:

| CVar | Default | Purpose |
|---|---:|---|
| `t66.HeroPhysics.EnableActiveRagdoll` | `1` | Enables Stage 3 active-ragdoll path |
| `t66.HeroPhysics.DebugLog` | `0` | Enables periodic runtime samples |
| `t66.HeroPhysics.DebugLogInterval` | `0.25` | Debug sample interval |

## 7. Current Movement Infrastructure

The current movement stack is not a custom physics locomotion controller.

Live walking still comes from:

- `AT66PlayerController` input handlers
- `AddMovementInput()`
- `UCharacterMovementComponent`

`UT66HeroMovementComponent` owns:

- movement tuning
- cached move intent
- jump routing
- leap routing
- final `MaxWalkSpeed` refreshes

Movement settings from `FT66HeroMovementTuning` and `MASTER_MOVEMENT.md`:

| Setting | Current value |
|---|---:|
| `MaxAcceleration` | `9000` |
| `BrakingDecelerationWalking` | `12000` |
| `GroundFriction` | `8.0` |
| `bUseSeparateBrakingFriction` | `true` |
| `BrakingFriction` | `12.0` |
| `BrakingFrictionFactor` | `1.0` |
| `JumpZVelocity` | `1600` |
| `JumpMaxHoldTime` | `0.08` |
| `AirControl` | `0.40` |
| `GravityScale` | `4.5` |
| `FallingLateralFriction` | `0.35` |
| `BrakingDecelerationFalling` | `4096` |
| `LeapCooldownSeconds` | `0.7` |
| `LeapStrength` | `3200` |
| `LeapUpwardStrength` | `880` |
| `LeapSpeedMultiplierOverWalkSpeed` | `1.6` |
| `RotationRateYaw` | `1440` |

Walk speed:

- fallback base speed: `1800`
- runtime conversion: `RunState Speed * 840 UU/s`
- final clamp: `200` to `10000`

Leap:

- bound to `Leap`
- old roll wrappers remain deprecated compatibility aliases
- direction is hero actor forward vector, not movement input chord
- uses `LaunchCharacter(LeapVelocity, true, true)`
- horizontal strength is max of `3200` and `CurrentMaxWalkSpeed * 1.6`
- upward strength is max of `880` and `JumpZVelocity * 0.5`
- plays cached `LeapAnimation` once when available

Important implication: because normal locomotion remains standard CharacterMovement and active ragdoll tries to run constantly on top of it, movement feel is currently a mixed system. This is a major suspect area for the user-reported spazz/stretch and "being pulled" feeling.

## 8. TestRoom Wipeout Arm Infrastructure

File:

- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`

The TestRoom wipeout arm is a prototype obstacle. It is not a production trap and does not route through `AT66TrapBase` or `UT66TrapSubsystem`.

Geometry and scheduling:

| Setting | Current value |
|---|---:|
| `WipeoutArmRadiusUU` | `92` |
| `WipeoutArmLengthUU` | `3600` |
| `WipeoutArmCenterZ` | `178` |
| `WipeoutArmAngularSpeedRadiansPerSecond` | `1.35` |
| `WipeoutArmImpactCooldownSeconds` | `2.75` |
| `WipeoutArmTimerIntervalSeconds` | `0.025` |

Runtime CVars:

| CVar | Default |
|---|---:|
| `t66.TestRoom.EnableWipeoutArmTrap` | `1` |
| `t66.TestRoom.WipeoutArmUseHeroActiveRagdoll` | `1` |
| `t66.TestRoom.WipeoutArmLaunchXY` | `10500` |
| `t66.TestRoom.WipeoutArmLaunchZ` | `750` |
| `t66.TestRoom.WipeoutArmIncapSeconds` | `0.15` |
| `t66.TestRoom.WipeoutArmRagdollMaxSeconds` | `3.10` |
| `t66.TestRoom.WipeoutArmRagdollSettleSpeed` | `165` |
| `t66.TestRoom.WipeoutArmRagdollSettleHoldSeconds` | `0.12` |
| `t66.TestRoom.WipeoutArmRagdollBlendOutSeconds` | `0.15` |
| `t66.TestRoom.WipeoutArmBelowBodiesImpulseFraction` | `1.0` |
| `t66.TestRoom.WipeoutArmLinearDamping` | `0.01` |
| `t66.TestRoom.WipeoutArmAngularDamping` | `0.02` |
| `t66.TestRoom.WipeoutArmFriction` | `0.04` |
| `t66.TestRoom.WipeoutArmRestitution` | `0.72` |
| `t66.TestRoom.WipeoutArmVelocityChangeImpulse` | `0` |
| `t66.TestRoom.WipeoutArmSimulateAllBodies` | `1` |
| `t66.TestRoom.WipeoutArmCenterActorOnRagdoll` | `1` |
| `t66.TestRoom.WipeoutArmSuppressLookInput` | `1` |
| `t66.TestRoom.WipeoutArmVerticalHitTolerance` | `18` |
| `t66.TestRoom.WipeoutArmPhysicalAnimationStrength` | `0.42` |
| `t66.TestRoom.WipeoutArmEnablePhysicalAnimation` | `0` |
| `t66.TestRoom.WipeoutArmPhysicalAnimationDriveMode` | `0` |
| `t66.TestRoom.WipeoutArmPhysicalAnimationActivationDelay` | `0.35` |

Active path:

- arm mesh blocks `ECC_PhysicsBody`
- arm mesh ignores `ECC_Pawn`
- geometric hit detection calculates radial plus tangential direction
- launch direction is `RadialDirection + TangentialDirection * 0.22`
- active-ragdoll path calls `UT66HeroPhysicsComponent::ApplyPhysicsReaction`
- legacy knockback path is fallback only when active ragdoll fails or is disabled

Log marker:

```text
TestRoom wipeout arm impact routed to hero physics:
ActiveTried=1 ActiveApplied=1 LegacyApplied=0
```

## 9. Legacy Knockback Infrastructure

The old `UT66KnockbackComponent` still exists and remains fallback/prototype support.

It is not the desired final Hero 1 feel.

Legacy profile defaults:

| Setting | Default |
|---|---:|
| `bEnableSkeletalRagdoll` | `true` |
| `bSimulateAllPhysicsBodies` | `true` |
| `MinIncapacitationSeconds` | `0.15` |
| `MaxRagdollSeconds` | `0.4` |
| `SettleSpeed` | `165` |
| `SettleHoldSeconds` | `0.25` |
| `RecoveryBlendOutSeconds` | `0.1` |
| `bDetachMeshDuringRagdoll` | `true` |
| `bFollowActorToRagdoll` | `true` |
| `bUseSimulatedBodyCenterForActorFollow` | `true` |
| `bUsePreImpactActorToFollowBoneOffset` | `false` |
| `bEnableFloorPenetrationGuard` | `true` |
| `FloorPenetrationSkin` | `4` |
| `FloorTraceUpDistance` | `600` |
| `FloorTraceDownDistance` | `1800` |
| `MaxFloorCorrectionPerTick` | `2000` |
| `LaunchVelocityScale` | `1.0` |
| `MaxLaunchVelocity` | `4200` |
| `bTreatLaunchVectorAsVelocityChange` | `false` |
| `MainBodyImpulseScale` | `1.0` |
| `BelowBodiesImpulseFraction` | `0.65` |
| `bIncludeSimulationRootInBelowBodyImpulse` | `true` |
| `RagdollLinearDampingOverride` | `-1` |
| `RagdollAngularDampingOverride` | `-1` |
| `RagdollFrictionOverride` | `-1` |
| `RagdollRestitutionOverride` | `-1` |
| `bEnablePhysicalAnimation` | `false` |
| `PhysicalAnimationStrength` | `0.42` |
| `PhysicalAnimationActivationDelaySeconds` | `0.35` |
| `PhysicalAnimationOrientationStrength` | `240` |
| `PhysicalAnimationAngularVelocityStrength` | `32` |
| `PhysicalAnimationMaxLinearForce` | `3000` |
| `PhysicalAnimationMaxAngularForce` | `9000` |

Legacy behavior includes:

- optional full skeletal ragdoll
- optional mesh detach
- optional actor-follow-to-ragdoll
- floor penetration guard
- fallback `LaunchCharacter`
- optional physical animation, but hero ragdoll currently disables requested PAC in several cases

Rejected as final direction:

- treating this component as the final Hero 1 active-ragdoll feel
- actor-follows-ragdoll as steady-state authority
- detached mesh plus follow loop as the normal movement model

## 10. Hero 1 Rigging And Animation Infrastructure

Current canonical rigging process:

- `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md`

Current source model:

- `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb`

Current output root:

- `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Blender/PhysicsFirstHero/`

Produced files:

- `Hero_1_Chad_PhysicsFirst.blend`
- `Hero_1_Chad_PhysicsFirst_Skeletal.fbx`
- `Hero_1_Chad_PhysicsFirst_QA.json`
- `Hero_1_Chad_PhysicsFirst_Rig_Report.md`
- `physics_first_hero1_chad_manifest.json`
- proof renders under `ProofRenders/`
- animation FBXs under `AnimationSources/`

Explicitly not used:

- old spike rig
- old spike weights
- old animation FBXs
- Quaternius clips
- Animated ToonStyle outputs
- AccuRig lineup
- old Roll clip

Skeleton contract:

```text
root
  pelvis
    spine_01
      spine_02
        spine_03
          neck_01
            head
          clavicle_l
            upperarm_l
              lowerarm_l
                hand_l
          clavicle_r
            upperarm_r
              lowerarm_r
                hand_r
    thigh_l
      calf_l
        foot_l
          ball_l
    thigh_r
      calf_r
        foot_r
          ball_r
```

QA facts from `Hero_1_Chad_PhysicsFirst_QA.json`:

- source: raw Hero 1 Chad GLB
- vertex count: `163496`
- required bone count: `23`
- mesh object count: `1`
- unweighted vertices: `0`
- max influences per vertex: `1`
- weight model: single nearest required deform bone, physics-first MVP weights
- zero-vertex deform bones: none
- normalized target height: `180 cm`
- Blender front axis: `-Y`
- expected Unreal forward after FBX conversion: `+X`
- result: `PASS`

Important limitation: max influences per vertex is `1`. That is simple and stable for a physics-first MVP, but it may produce harsh deformation. A future agent should not assume the current skinning is production-quality just because QA passed.

## 11. Animation Clip Infrastructure

Imported Stage 2 clips:

| Clip | Source FBX frame range | Unreal asset |
|---|---|---|
| `Idle` | `1-60` | `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/AM_Hero_1_Chad_PhysicsFirst_Idle` |
| `Walk` | `1-30` | `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/AM_Hero_1_Chad_PhysicsFirst_Walk` |
| `Jump` | `1-34` | `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/AM_Hero_1_Chad_PhysicsFirst_Jump` |
| `Leap` | `1-42` | `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/AM_Hero_1_Chad_PhysicsFirst_Leap` |
| `RecoverStand` | `1-45` | `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/AM_Hero_1_Chad_PhysicsFirst_RecoverStand` |
| `GetUp_Back` | `1-50` | `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/AM_Hero_1_Chad_PhysicsFirst_GetUp_Back` |
| `GetUp_Front` | `1-50` | `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/AM_Hero_1_Chad_PhysicsFirst_GetUp_Front` |

Runtime uses only these explicit `CharacterVisuals.csv` columns:

- `IdleAnimation`
- `WalkAnimation`
- `JumpAnimation`
- `LeapAnimation`

`RecoverStand`, `GetUp_Back`, and `GetUp_Front` were imported but are not yet fully wired as a physical recovery system in Stage 3.

No `Run` clip is part of the MVP. `Leap` replaces the old roll concept.

## 12. Unreal Import And Runtime Wiring

Unreal import report:

- `Reports/AgentReviews/FallGuysHeroRiggingStage2Implementation/physics_first_hero1_unreal_import_report.json`

Imported assets:

- skeletal mesh: `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst`
- skeleton: `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst_Skeleton`
- material: `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/Materials/MI_SK_Hero_1_Chad_PhysicsFirst`
- texture: `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/Textures/T_Hero_1_Chad_PhysicsFirst_BaseColor`
- bounds size: approximately `90.2 x 52.4 x 180 cm`
- import uniform scale: `0.01`
- material assignment: OK, one material slot

Current `Content/Data/CharacterVisuals.csv` rows:

```csv
Hero_1_Chad,/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst.SK_Hero_1_Chad_PhysicsFirst,,,,/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/AM_Hero_1_Chad_PhysicsFirst_Walk.AM_Hero_1_Chad_PhysicsFirst_Walk,/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/AM_Hero_1_Chad_PhysicsFirst_Idle.AM_Hero_1_Chad_PhysicsFirst_Idle,/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/AM_Hero_1_Chad_PhysicsFirst_Jump.AM_Hero_1_Chad_PhysicsFirst_Jump,/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/AM_Hero_1_Chad_PhysicsFirst_Leap.AM_Hero_1_Chad_PhysicsFirst_Leap,"(X=0,Y=0,Z=0)","(Pitch=0,Yaw=0.000000,Roll=0)","(X=1,Y=1,Z=1)",true,true
Hero_1_Chad_DemoSkin,,/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/SM_Hero_1_Chad_Male.SM_Hero_1_Chad_Male,,,,,,,"(X=0,Y=0,Z=0)","(Pitch=0,Yaw=90.000000,Roll=0)","(X=1,Y=1,Z=1)",false,true
```

Interpretation:

- normal Hero 1 Chad now uses the PhysicsFirst skeletal mesh and movement clips
- DemoSkin remains a raw static mesh with yaw 90
- Stage 2 import report says `character_visuals_csv_touched=false`, but the live CSV now has the PhysicsFirst row wired. That means the report is stale on that one field because the CSV was edited after the import report.

## 13. PhysicsAsset Infrastructure

PhysicsAsset seed report:

- `Reports/AgentReviews/FallGuysHeroRiggingStage2Implementation/physics_first_hero1_physics_asset_report.json`

Current PhysicsAsset:

- `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/PA_Hero_1_Chad_PhysicsFirst_Stage2Seed`

Report summary:

- `ok: true`
- `body_count: 18`
- `constraint_count: 17`
- `min_bone_size: 4.000`
- `body_for_all: true`
- body bones:
  - pelvis
  - spine_01, spine_02, spine_03
  - neck_01, head
  - upperarm_l, lowerarm_l, hand_l
  - upperarm_r, lowerarm_r, hand_r
  - thigh_l, calf_l, foot_l
  - thigh_r, calf_r, foot_r
- constraint graph:
  - spine chain anchored to pelvis
  - neck/head to spine
  - arms to spine_03
  - thighs to pelvis
  - calves to thighs
  - feet to calves

Separate report-only evidence from the flattening investigation later recorded:

- body count: `18`
- constraint count: `17`
- disabled collision pairs: `153`
- bodies marked `PhysicsOnly`
- limited angular constraints
- authored pelvis/spine capsule/sphyl volumes

Important limitation: this PhysicsAsset is still a Stage 2 seed. It is not accepted as production active-ragdoll tuning. The current user report suggests asset body shapes, mass, inertia, constraints, drives, or their interaction with runtime authority may still be wrong.

## 14. Proof Artifacts And What They Actually Prove

Stage 3 proof summary:

- `Reports/Proof/Physics/HeroRagdollStage3AuthorityRebuild/stage3_authority_rebuild_proof_summary.md`

Primary infrastructure proof:

- `Reports/Proof/Physics/HeroRagdollStage3AuthorityRebuild/heroactiveragdollproof_reaction_testroom_high_no_runtime_resync.mp4`
- `Reports/Proof/Physics/HeroRagdollStage3AuthorityRebuild/T66_heroactiveragdollproof_high_no_runtime_resync.log`

Supplemental movement telemetry:

- `Reports/Proof/Physics/HeroRagdollStage3AuthorityRebuild/hero_movementqa_active_ragdoll_no_runtime_resync_60f.mp4`
- `Reports/Proof/Physics/HeroRagdollStage3AuthorityRebuild/T66_hero_movementqa_active_ragdoll_no_runtime_resync_60f.log`

What the high-angle proof log proves:

- active-ragdoll component initialized
- `PelvisSimulating=1`
- PhysicsAsset had 18 bodies and 17 constraints
- TestRoom wipeout arm routed through active ragdoll
- `ActiveTried=1 ActiveApplied=1 LegacyApplied=0`
- `Reaction Applied=1 Source=TestRoomWipeoutArm`
- state advanced through `Balanced -> KnockedDown -> Recovering -> Balanced`
- no `EmergencyPelvisResync` marker in the accepted scan

What the proof does not prove:

- it does not prove Fall Guys-like feel
- it does not prove normal movement is visually stable
- it does not prove the obstacle hit is strong enough
- it does not prove the recovery looks good
- it does not prove the proof camera makes contact readable

The proof summary itself says the MP4 is infrastructure proof, not final subjective feel proof. It also says normal-movement stretch/spazz was telemetry-supported but not visually accepted.

## 15. Debug Iterations Already Produced

The proof folder contains many Stage 3 iteration videos. The filenames document the journey:

- `heroactiveragdollproof_after_align.mp4`
- `heroactiveragdollproof_after_restore.mp4`
- `heroactiveragdollproof_attached_kinematic_component.mp4`
- `heroactiveragdollproof_body_distance.mp4`
- `heroactiveragdollproof_detached_authority.mp4`
- `heroactiveragdollproof_effective_pelvis.mp4`
- `heroactiveragdollproof_init_reset_first.mp4`
- `heroactiveragdollproof_kinematic_component.mp4`
- `heroactiveragdollproof_reaction_hpoverride.mp4`
- `heroactiveragdollproof_reaction_testroom.mp4`
- `heroactiveragdollproof_reaction_testroom_final.mp4`
- `heroactiveragdollproof_reaction_testroom_final_framed.mp4`
- `heroactiveragdollproof_reaction_testroom_no_runtime_resync.mp4`
- `heroactiveragdollproof_reaction_testroom_wide_no_runtime_resync.mp4`
- `heroactiveragdollproof_reaction_testroom_high_no_runtime_resync.mp4`
- `heroactiveragdollproof_reattach.mp4`
- `heroactiveragdollproof_sim_root.mp4`
- `heroactiveragdollproof_visual_distance.mp4`
- `hero_movementqa_active_ragdoll_authorityoffset.mp4`
- `hero_movementqa_active_ragdoll_final.mp4`
- `hero_movementqa_active_ragdoll_meshsync.mp4`
- `hero_movementqa_active_ragdoll_meshsync_resolved_hp.mp4`
- `hero_movementqa_active_ragdoll_no_runtime_resync_60f.mp4`

This list matters because it shows the current system is the product of several reactive fixes around alignment, restore, detachment, authority, effective pelvis distance, mesh sync, HP override, and runtime resync removal. A fresh agent should not assume the current design was cleanly derived from first principles.

## 16. History Of The Physics Journey

Stage 0 or old direction:

- The first concept considered a hybrid or hit-triggered ragdoll: character moves normally, then becomes ragdoll after impact.
- The user rejected this as the final direction because the desired game feel is always bouncy and unstable, not a normal controller with a hit reaction.
- The older `UT66KnockbackComponent` remains as evidence and fallback.

Stage 1:

- A dedicated `Gameplay/Physics` ownership layer was created.
- Physics was defined as whole-game feel ownership, not obstacle-only ownership.
- The docs were updated to supersede older pure-Chaos/PAC-off and ToonStyle assumptions.

Stage 2:

- Hero 1 Chad became the MVP.
- The project stopped using old mid-change/legacy rigging work as foundation.
- A fresh PhysicsFirst Hero 1 rig was produced from the raw FriendSlop GLB.
- New clips were produced: `Idle`, `Walk`, `Jump`, `Leap`, plus recovery/get-up clips.
- `Run` was deliberately omitted.
- `Roll` was deprecated and replaced by `Leap`.
- The PhysicsFirst skeletal mesh and animation assets were imported and wired into `CharacterVisuals.csv`.
- A Stage 2 PhysicsAsset seed was created and reported as 18 bodies and 17 constraints.

Stage 3 first proof attempts:

- Work focused on getting active ragdoll initialized and reacting to the TestRoom wipeout arm.
- The system tried to solve flattening/spiking/origin-divergence through PhysicsAsset hardening and runtime ownership changes.
- Earlier evidence suggested the asset had enough bodies and constraints, so the problem was not only "missing PhysicsAsset bodies."
- Runtime ownership became the main focus.

Flattening fix attempt:

- A previous closeout concluded the long-line flattening defect was addressed by hardening the asset and changing runtime ownership.
- Report-only asset dump confirmed an inspectable PhysicsAsset.
- The important lesson was that asset geometry and runtime authority are separate defect classes.
- The closeout warned not to claim the fix was purely PhysicsAsset tuning.

Stage 3 authority rebuild:

- The current system was rebuilt around capsule authority plus mesh component-frame sync plus simulated pelvis anchored by hip constraint.
- Runtime emergency pelvis/body resync was removed from the normal tick path.
- Effective pelvis distance logging was added because raw Chaos body readback can appear in different spaces.
- TestRoom active-first routing was added: active ragdoll first, legacy fallback only if active fails.
- Staged standalone was refreshed and shortcuts were verified.

Current user test after Stage 3:

- The user says it still does not really work.
- Movement still has spazz/stretch/pulled behavior.
- Wipeout arm hit barely moves the character and does not feel like Fall Guys.
- Therefore the next agent should reopen diagnosis, not tune around the assumption that the architecture is correct.

## 17. Explicitly Rejected Or Superseded Directions

Do not revive these without explicit user approval:

- hit-only ragdoll as the final feel
- pure `LaunchCharacter` knockback as the whole obstacle response
- old pure-Chaos/PAC-off hero ragdoll as the standing target
- legacy `UT66KnockbackComponent` as the final Hero 1 feel
- actor-to-pelvis follow as steady-state authority
- `SimulationUpdatesComponentTransform` on the pelvis-rooted rig as the root solution
- repeated normal-runtime pelvis/body teleport or emergency resync loops
- old spike rig
- old spike animation FBXs
- Animated ToonStyle bridge
- AccuRig lineup
- Quaternius retargeted clips
- a `Run` clip for the Hero 1 MVP
- keeping the old roll ability concept as the player-facing action
- animation-only fake wobble with no active simulated body chain
- treating desktop screenshots as physics proof

## 18. Current Known Failure Surfaces

The following are likely diagnosis surfaces, not proven root causes:

1. Mixed locomotion authority
   - Normal walking is still CharacterMovement/AddMovementInput.
   - Active ragdoll is trying to run on top of it every tick.
   - The mesh/capsule sync and hip constraint may be fighting normal movement.

2. Mesh component-frame sync while simulating
   - `SyncMeshComponentToCapsuleAuthority` may move the mesh component using teleport-style physics correction.
   - The user sees stretching/pulling during movement, which could mean component sync, body simulation, and constraints are fighting.

3. Hip anchor and pose-drive stiffness balance
   - Anchor values may be too strong to let obstacles visibly move the body.
   - Or the anchor may be strong in the wrong way and create pulled/stretch artifacts.

4. PhysicsAsset mass/inertia/body shape
   - The PhysicsAsset exists and is inspectable, but it is not tuned as final.
   - Body volumes, mass distribution, inertia conditioning, joint limits, and collision disable pairs may still be wrong for always-on active ragdoll.

5. Reaction impulse path versus playable displacement
   - The log shows a large requested/applied reaction velocity.
   - The user sees only a small bump.
   - That implies the visible body impulse, anchor loosen, capsule shove fraction, obstacle geometry, or camera/readability path is not producing the intended displacement.

6. Proof camera and test path
   - The current proof camera can hide the character behind the arm or miss the best contact frames.
   - Weak proof may have caused false confidence.

7. Animation and skinning quality
   - Single-influence weights can be stable but harsh.
   - The current clips are simple pose targets, not polished gameplay animation.
   - Recovery/get-up clips are imported but not deeply integrated as a physical recovery solution.

8. TestRoom-only obstacle limitations
   - The wipeout arm is not a production trap.
   - It uses geometric hit checks, a simple cylinder, and prototype CVars.
   - It may not represent the final obstacle design or contact setup.

## 19. What The Next Agent Should Not Assume

Do not assume:

- `PelvisSimulating=1` means the feel works
- `ActiveApplied=1` means the hit is satisfying
- no `EmergencyPelvisResync` means normal movement is solved
- 18 bodies and 17 constraints means the PhysicsAsset is production-ready
- the current authority model is definitely the final model
- the proof MP4 is visually accepted
- the latest docs fully reflect the user's lived test result unless this audit is read with them

Do assume:

- the user wants the system to be reconsidered comprehensively
- runtime authority, asset physics, movement, rigging, and obstacle design are coupled
- solving one log marker is not enough
- the next solution needs better proof of ordinary movement, obstacle impact, rebound/tumble, and recovery

## 20. Suggested Analysis Questions For The Next Agent

These are not implementation instructions. They are the questions the next analysis pass should answer.

1. Should this still be an always-on simulated skeletal mesh on the playable `ACharacter`, or should T66 switch to a different active-ragdoll architecture such as a physics proxy body plus visual mesh follower?
2. Is the current mesh component-frame sync fundamentally causing stretch/pull during walking?
3. Should the pelvis be continuously simulated, or should only selected body groups simulate while a more explicit balance controller handles center-of-mass?
4. Are the anchor/pose drives too stiff, too weak, or applied in the wrong frame?
5. Are the current PhysicsAsset bodies and masses appropriate for a bean-like character, or should the asset be redesigned around fewer, larger, more stable core bodies?
6. Should obstacle response loosen/disable the anchor more aggressively for a short window?
7. Is `LaunchCharacter` capsule shove with `false,false` flags too weak or too easily canceled by CharacterMovement?
8. Does CharacterMovement need a custom movement mode or movement suppression during active-ragdoll reaction windows?
9. Should normal movement wobble be driven by physics, animation pose noise, or a hybrid proxy, and what should be the acceptance gate?
10. What exact Unreal-owned capture proves ordinary movement stability before testing obstacle hits again?

## 21. Verification History

Previous Stage 3 verification that passed structurally:

- focused `T66Editor Win64 Development` build
- `Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode heroactiveragdollproof`
- primary TestRoom active-ragdoll proof log with active route markers
- supplemental MovementQA telemetry
- staged standalone build
- standalone shortcut target verification
- Claude cross-review `Result: OK`

Current audit verification:

- live docs and source files were read
- current rigging QA/report/import artifacts were read
- current `CharacterVisuals.csv` rows were checked
- current proof summary and proof folder were checked
- Claude validator gave an independent read-only outline and warned against overstating success

This audit did not run gameplay, build, import, Blender, or packaged verification because the user requested a descriptive handoff file, not another implementation or proof pass.

## 22. Claude Validator Artifact

Claude independent answer:

- `Reports/AgentReviews/PhysicsMasterAuditHandoff/Claude/20260607T071920-IndependentAnswer-pass1/claude_review_pass1.md`
- result: `OK`
- key correction: lead with the fact that Stage 3 still fails in play; classify existing videos/logs as infrastructure evidence only
