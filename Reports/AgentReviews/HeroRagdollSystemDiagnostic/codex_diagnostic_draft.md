# Codex Diagnostic Draft

Read-only diagnostic; no implementation was performed.

## Core Conclusion

The current Hero 1 active-ragdoll path is not failing because of one bad PhysicsAsset number. It is failing because the runtime has multiple competing motion authorities:

- CharacterMovement/capsule locomotion remains authoritative for walking/falling.
- Simple `PlayAnimation()` asset playback is still switching pose clips.
- Physical Animation drives child bodies toward animation, but currently angular-only.
- The hip anchor constraint is attached to a pelvis body that is excluded from simulation.
- Runtime resync/follow code teleports either the physics bodies or actor.
- TestRoom obstacle impacts are not physical contacts; the arm has no collision and the mesh ignores WorldDynamic.
- The authored 10500 UU/s TestRoom launch is clamped to 1700 before application.

The user-reported symptom, "stretched/spazzing while moving as if being pulled somewhere," is consistent with this authority conflict: the capsule/actor moves, the pelvis/root is kinematic, child bodies simulate, PAC tries to preserve pose, and the code also teleports the actor toward the pelvis in non-balanced states.

## Likely Causes

1. Kinematic pelvis vs simulated-child setup does not match the documented world-anchor model. `SetAllBodiesBelowSimulatePhysics(..., false)` excludes pelvis while the hip constraint targets pelvis.
2. `PhysicsTransformUpdateMode=ComponentTransformIsKinematic` plus `KinematicBonesUpdateType=SkipAllBones` makes the component/capsule path dominate transforms.
3. `FollowActorToPelvisBody()` moves the actor toward physics bodies with `TeleportPhysics` while preserving body snapshots, creating a feedback loop during stagger/knockdown/recovery.
4. Physical Animation uses local simulation but zero linear drive (`PositionStrength=0`, `VelocityStrength=0`, `MaxLinearForce=0`), so limbs get angular correction but weak positional coherence.
5. TestRoom wipeout arm is a procedural/timer hit with `NoCollision`; the skeletal mesh blocks only WorldStatic and ignores WorldDynamic/Pawn.
6. `MaxReactionVelocityChange=1700` clamps the arm's `10500` XY launch heavily, so the hit feels like a bump.
7. `ApplyPhysicsReaction()` also writes `CharacterMovement->Velocity`, so body physics and CharacterMovement both own the impact.
8. Stock movement tuning is very high authority: walk speed scales as Speed * 840, MaxAcceleration 9000, BrakingDecelerationWalking 12000, GroundFriction 8, BrakingFriction 12, JumpZ 1600, GravityScale 4.5.
9. `AT66HeroBase::Tick()` still switches clips with `GetMesh()->PlayAnimation(...)`; that is not a stable anim blueprint/pose target pipeline for active physical animation.
10. The previous proof solved only a narrow flattening/divergence case, not walk + obstacle contact + hit + recovery.

## PAC Recommendation

Use Physical Animation Component, but not as a complete controller and not on top of the current authority conflict. PAC should provide the muscle/pose target for a coherent rigid-body graph. The locomotion/anchor/collision contract must be decided first.

The next architecture should be:

- capsule remains clean gameplay mover only for input/navigation;
- pelvis/central physics body is simulated when active-ragdoll is enabled;
- a kinematic hip target rides the capsule;
- a real physics constraint with linear/angular drives connects hip target to simulated pelvis;
- PAC drives spine/limbs toward simple in-place pose clips;
- impacts loosen pose/anchor profiles and apply impulse at true contact/body location;
- obstacle colliders physically block/impulse the relevant physics bodies, or at minimum the geometry hit must be treated as a physics-profile impulse without clamp swallowing the result.

## Unreal Reference Patterns

Unreal's own physics-driven animation docs center the setup on PhysicsAssets, SetAllBodiesBelowSimulatePhysics, SetAllBodiesBelowPhysicsBlendWeight, hit location/bone, and blending physics weight back down. The PhysicalAnimation profile docs also show per-body profile values and nonzero drive values rather than one global angular-only drive.

Unreal collision docs confirm the current TestRoom setup cannot physically collide: both objects must block the relevant channels, and NoCollision / Ignore makes the collision effectively absent.

Community Unreal ragdoll recovery patterns also separate capsule tracking from ragdoll authority and call out pelvis/capsule offset/tick order as a frequent failure point.

## Recommended Next Diagnostic Gates

Before implementing, run/prepare four isolated proofs:

1. No-hit movement proof with active ragdoll on, logging actor/capsule, pelvis, mesh, anchor, PAC target deltas.
2. Obstacle contact proof with real collision enabled in a sandbox, not timer launch.
3. PAC-only proof: simulated pelvis + anchor off/on, linear drive off/on, verify whether stretching disappears.
4. Impact energy proof: requested impulse, clamped impulse, body velocity, CharacterMovement velocity, and recovery state over multiple frames.

Expected answer to the user: do not tune another one-off. Rebuild Stage 3 around one motion authority plus real collision/profile-driven impacts.
