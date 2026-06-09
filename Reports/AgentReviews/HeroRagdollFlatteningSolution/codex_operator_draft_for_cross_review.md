# Codex Operator Draft - Ragdoll Flattening Solution

## Short Answer

There is no single guaranteed runtime parameter that prevents flattening. The guaranteed solution is a validated PhysicsAsset/rig acceptance pass: the ragdoll must be built from non-degenerate rigid bodies that fill the character, connected by correctly aligned constraints whose linear and angular freedoms prevent inversion and volume collapse, with adequate solver/projection/inertia settings for the expected obstacle impulses.

## Why This Is Confirmed

Unreal defines a PhysicsAsset as the rigid bodies and constraints that make the ragdoll. The Physical Animation Component applies simulation on top of animation; it does not replace the body/constraint structure. If the body/constraint structure lets the character fold flat, stronger pose drive can create spikes or jitter, but it cannot make the asset structurally valid.

The T66 Stage 3 runtime path is already doing the correct high-level runtime pieces: simulated bodies below pelvis, PhysicalAnimationComponent drive, hip/pelvis anchor, active-first obstacle routing, and recovery state transitions. The remaining symptom is therefore in the PhysicsAsset/rig validation layer.

The Stage 2 PhysicsAsset seed report proves only:

- 18 bodies
- 17 constraints
- pelvis-rooted body/constraint graph
- `min_bone_size = 4.0`

It does not prove:

- capsule/sphere/box radii and extents
- angular limits are Limited/Locked instead of Free
- constraint frames are aligned to joints
- directly connected body collision is disabled while non-adjacent collision is still usable
- solver iteration overrides/projection/inertia conditioning
- mass distribution and center-of-mass stability

## Required Fix Order

1. Rebuild or tune the Hero 1 PhysicsFirst PhysicsAsset bodies so pelvis, torso, head, arms, legs, and feet have real capsule/sphere/box volume matching the mesh. No sliver bodies, no near-flat primitives, no tiny limb capsules.
2. Tune constraints per joint. Linear motion should be locked or tightly limited; swing/twist must be anatomically limited where the body should not invert. Constraint frames must sit at the joint pivots and be aligned so the yellow limit arc/cone contains intended motion.
3. Set self-collision intentionally. Disable collision between immediate constrained neighbors to avoid jitter, but do not let the entire ragdoll self-overlap freely into a stack.
4. Tune mass/inertia/solver stability after bodies and limits are correct. Pelvis/torso should dominate, limbs should not yank the core flat, and solver/projection/inertia settings need to withstand the TestRoom arm impulse.
5. Only then tune PhysicalAnimation/PAC strength and hip-anchor drive for feel.

## Practical Guarantee

The guarantee is not "set X to Y." The guarantee is this invariant: when every simulated bone is represented by a real collision body, those bodies are connected by non-free constraints that block impossible folding, and the solver has enough iterations/projection/inertia to enforce those constraints under the expected impulse, the skeletal mesh cannot physically pancake into a flat plane while still passing the validation gate.

## Recommended Next Step

Run a dedicated Hero 1 PhysicsAsset tuning pass:

- inspect and record every body primitive radius/extent
- inspect and record every constraint linear/swing/twist mode and limit angle
- fix shape volumes first
- fix joint limits second
- fix self-collision and mass/inertia third
- rerun `heroactiveragdollproof`
- accept only when logs stop repeated body resyncs and multi-frame proof shows a coherent body through impact and recovery

## Claude Alignment

Claude independently reached the same conclusion: the fix is structural PhysicsAsset/rig validation, not drive-strength tuning. Claude also called body volume and non-free joint angular limits the first requirements to verify.
