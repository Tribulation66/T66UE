# Operator Draft - FriendSlop Ragdoll Reassessment

## Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: research and reassess why the current FriendSlop ragdoll stretches, then recommend the correct UE/Chaos process before further implementation.
Stop condition: repo-grounded diagnosis plus external-source-informed options, tradeoffs, and a revised process; no gameplay/code/content implementation.

## Current Finding

The screenshot is not a tuning problem. The current ragdoll pipeline is structurally out of order:

1. We are trying to tune Physical Animation / active-ragdoll feel before the passive PhysicsAsset has passed a no-stretch impulse test.
2. The runtime applies a mass-independent velocity change to one body and then to every body below the simulation root:
   - `T66GameMode_TestRoom.cpp:1127` calls `MeshComponent->AddImpulse(LaunchVelocity, ImpulseBoneName, true)`.
   - `T66GameMode_TestRoom.cpp:1130` calls `AddImpulseToAllBodiesBelow(..., true, true)`.
   - UE docs and local engine source confirm `bVelChange=true` means mass has no effect; Chaos writes velocity directly instead of mass-scaled impulse.
3. The controlled PhysicsAsset was pruned to 18 bodies / 15 constraints after an earlier 24-body / 23-constraint passive proof. The smaller body set can be fine, but it requires validated body shapes and constraint frames. We have only a body/constraint name report, not shape/constraint-frame proof.
4. The current commandlet sets angular limits, angular drive, projection, damping, and mass scale, but does not explicitly reassert linear locks. Local UE 5.7 source shows default `FLinearConstraint` X/Y/Z motion is locked, so missing explicit linear locks is a hardening/validation gap rather than a proven sole cause.
5. PAC is scheduled on top of the launch path. `ApplyPhysicalAnimationSettingsBelow` is a legitimate method class, but if it activates while the body chain is over-impulsed or the PhysicsAsset is misfit, it can add a second solver fight instead of improving control.
6. The actor/camera follow path moves the hero actor every tick to the ragdoll follow bone via `SetActorLocation(..., TeleportPhysics)` while the mesh is simulating. Because the mesh is detached this should not directly stretch it, but it complicates recovery/camera ownership and should be separated from physics validation.
7. Blender rig QA is acceptable for the spike but explicitly not production weighting: dense Pixal3D mesh, procedural coordinate-region weights, relaxed low-arm pose, fused costume. This is secondary until a no-stretch PhysicsAsset test proves weights are the remaining cause.

## External/Reference Grounding

- Epic PhysicsAsset docs define a ragdoll as rigid bodies plus constraints for a skeletal mesh; the editor exists to create, visualize, and test those bodies/constraints.
- Epic constraint docs distinguish linear X/Y/Z motion from angular swing/twist limits. Linear locked means fully constrained; angular limits only stop rotation.
- Epic Add Impulse docs say `Vel Change` ignores mass.
- Epic Physical Animation docs apply physical-animation settings/profile below a body, not as a substitute for a valid PhysicsAsset.
- The active-ragdoll transcript path also starts with skeletal mesh + Physical Animation Component + apply settings below a pelvis/root body, then uses collision setup; it does not solve bad constraints or over-impulse.
- The fall/recover transcript handles the capsule/mesh problem by tracking pelvis/capsule relationship and blending/get-up later, after ragdoll state works.

## Recommended Replacement Process

Phase 0: freeze PAC and recovery polish.
- Disable physical-animation drive for the next proof.
- Disable multi-body velocity-change launch.
- Stop moving the actor every tick during the physics-only proof; camera can follow a proxy/pelvis target for observation, but the hero actor should not be the mechanism under test.

Phase 1: build a validated passive ragdoll PhysicsAsset.
- Use a simplified 12-20 body set: pelvis, 2-3 spine, neck/head, upper/lower arms, thighs/calves/feet; include clavicles if shoulders need readable motion.
- Fit capsules/boxes in PhAT or via a script that records shape sizes/centers/orientations; prove no adjacent body overlap.
- Explicitly set linear X/Y/Z locked on every joint even if UE defaults are locked, because we need deterministic generated assets and auditability.
- Set angular limits by joint role, no angular drives at first.
- Validate with PhAT/Unreal-owned drop and single-impulse captures before touching gameplay feel.

Phase 2: fix runtime launch.
- Apply a single bounded mass-scaled impulse to pelvis or hit body with `bVelChange=false`.
- Add separate bounded angular impulse/torque for tumble if needed.
- Avoid `AddImpulseToAllBodiesBelow` as the default. Use it only for special effects after no-stretch proof, and never mass-independent across the entire chain.
- Clamp max linear/angular velocities so the wipeout arm cannot overfeed Chaos.

Phase 3: camera/capsule separation.
- During ragdoll, disable movement/attacks/capsule collision, but do not use the character actor as the physical object.
- Follow the mesh pelvis with a camera target/proxy.
- At recovery, move the capsule/actor once to the final pelvis-derived location, then blend back.

Phase 4: add active feel only after passive proof.
- Re-enable Physical Animation using profiles/settings below pelvis/core.
- Start with local simulation and orientation/angular velocity strengths only; avoid high position strength unless needed.
- Ramp PAC in after the initial launch impulse and keep limbs looser than torso.
- Add get-up/pose snapshot or C++ equivalent later; this is a separate recovery polish phase.

## Verification Gates Before Feel Tuning

1. No-stretch PhysicsAsset gate: drop/poke/impulse test shows no body separation smear and logs max bone-pair distance ratios against rest pose.
2. Runtime passive impact gate: wipeout arm hit launches the body, it slides/bounces into surfaces, and the mesh remains body-shaped at rest.
3. PAC A/B gate: PAC-off is stable; PAC-on changes posture only, not stability.
4. Camera/capsule gate: camera tracks the ragdoll without moving the actor every physics tick; actor/capsule snaps or blends only during recovery.
5. Rig escalation gate: only if the no-stretch PhysicsAsset/impulse gates pass but visual mesh still smears, return to Blender for hand-weight cleanup, segmentation, or retopo.

## Draft Final Recommendation

Do not tune the current cvars. The next implementation phase should be a narrow passive-ragdoll rebuild/validator: make the PhysicsAsset auditably constrained, prove it with a mass-scaled single-impulse test, and only then restore physical animation. The "Fall Guys" feel should come from a stable passive ragdoll plus delayed/ramped active drive, not from adding stronger PAC to an unstable launch chain.
