Working task:
Operator: Codex
Validator: Claude
Scope: read-only solution analysis for three fundamental FriendSlop test-room issues: walk animation, camera follow during ragdoll throw, and controlled/limited ragdoll behavior.
Stop condition: give a repo-grounded recommendation for each issue, including what should change, where, and what Claude agrees/disagrees with; no code changes.

## Codex Draft

I understand the ragdoll issue clearly: this is not just "turn ragdoll down." The target is a controlled active ragdoll where major body parts flop and collide, but the body is still made of solid-feeling segments. Arms can go over the body, legs can split, the character can bounce/drag, but the torso and limbs should not twist into spaghetti or visibly deform through the costume.

### 1. Walk animation

Repo evidence:
- `Content/Data/CharacterVisuals.csv` has empty `SkeletalMesh`, `WalkAnimation`, `IdleAnimation`, `JumpAnimation`, and `RollAnimation` for `Hero_1_Chad`; it points at the static FriendSlop raw mesh.
- The test room explicitly overrides Hero 1 Chad to `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/SK_Hero_1_Chad_Male_FriendSlop` in `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp:315-318` and sets single-node animation mode at `:471-475`, but it does not assign a compatible walk animation.
- Runtime hero movement animation is already simple C++ `PlayAnimation()`: `AT66HeroBase` caches visual row animations at `Source/T66/Gameplay/T66HeroBase.cpp:1059-1068` and switches Idle/Jump/Roll/Walk at `:694-740`.
- The current rigging docs say the old Animated ToonStyle bridge is legacy for FriendSlop raw, but it identifies the source/action the user remembers: accepted Hero 1 Rigify/Quaternius template, `Walk_Fwd_Loop_LegsTorsoOnly`, with explicit `WalkAnimation`/`IdleAnimation`/`JumpAnimation`/`RollAnimation` fields.
- The FriendSlop rig report says the current skeletal mesh is a 23-bone deterministic spike rig, good for PhysicsAsset/PAC structure, not yet a polished production animation rig.

Recommendation:
- Do not just point the FriendSlop skeletal mesh at the old `AM_Hero_1_Chad_Walk` asset. That animation was created for the older Animated ToonStyle skeleton/path, and the current FriendSlop skeleton is a different imported asset.
- For the test-room pass, create/import a FriendSlop-skeleton-compatible `Idle` and `Walk` at minimum, derived from the accepted Quaternius/template walk action. The runtime can stay pure C++ and single-node `PlayAnimation()`.
- Implement this as a narrow FriendSlop raw animation import/retarget step, not by reviving the full old ToonStyle bridge. The old bridge is useful provenance for source actions and import options; the target assets should live beside the FriendSlop skeletal mesh, e.g. `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/AM_Hero_1_Chad_Male_FriendSlop_Walk`.
- For the test room, either set those assets directly in the skeletal override state or create a dedicated test visual row. I prefer a dedicated test visual row or a tiny override cache because it avoids changing production Hero 1 Chad, which still intentionally points at the static mesh.

### 2. Camera follow when thrown

Repo evidence:
- The hero camera boom is attached to the hero root/capsule in `Source/T66/Gameplay/T66HeroBase.cpp:76-90`.
- The player controller sets the view target to the controlled pawn in `Source/T66/Gameplay/T66PlayerController.cpp:969-1016`.
- During ragdoll, the test-room code calculates a follow bone and actor offset, but only moves the hero actor to that follow bone when physical animation is not driving: `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp:966-983`.
- Therefore, when PAC is active, the mesh can move away from the actor root while the camera keeps following the actor/capsule location.

Recommendation:
- The smallest fix is to keep the actor/capsule root synchronized to the ragdoll follow bone during the active ragdoll phase even when physical animation is enabled. The code already has the right "move actor, then restore mesh world transform" pattern at `:970-981`; it should not be gated out by `bPhysicalAnimationDriving`.
- If that root sync destabilizes Chaos/PAC, the fallback is a dedicated camera follow target updated from pelvis/COM during ragdoll, with the controller temporarily viewing or blending toward that target and restoring to the pawn on recovery. I would start with root follow for the test room because it uses the existing camera stack.
- Recovery should still snap/blend back from the ragdoll follow bone, not from the original impact point, otherwise the camera and pawn can disagree again after the throw.

### 3. Controlled / limited ragdoll

Repo evidence:
- The current impact path enables simulation on every body and full physics blend: `SetAllBodiesSimulatePhysics(true)` and `SetAllBodiesPhysicsBlendWeight(1.f)` at `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp:1161-1162`.
- It applies impulse to all bodies below `NAME_None` at `:1179`, meaning essentially the whole body receives direct impulse.
- The PAC drive currently targets only core body names in mode 1/2 (`pelvis`, spines, neck, head) at `:691-722`, which is a good direction, but it is layered over a generated physics asset.
- The test physics asset commandlet clears and regenerates a PhysicsAsset using `FPhysicsAssetUtils::CreateFromSkeletalMesh` with `bCreateConstraints=true` at `Source/T66Editor/T66CreateTestRoomPhysicsAssetCommandlet.cpp:192-213`. There is no per-joint swing/twist profile in that code.
- The rigging instruction says the mesh should expose pelvis, spine, head, arms, thighs, calves, and feet for PhysicsAsset/PAC, and explicitly warns the fused costume/rest-pose compromise may need segmentation, retopo, or hand cleanup for production animation.

Recommendation:
- This needs a custom PhysicsAsset/constraint profile, not cvar tuning on the generated default asset.
- Use major body bodies only: pelvis, 2-3 torso bodies, head, upper/lower arms, hands optional, thighs, calves, feet. Do not make accessory/coattail/tiny bones active ragdoll bodies.
- Set explicit angular limits per joint: tight twist on spine/hips/shoulders, moderate swing on shoulders/hips, limited elbow/knee hinge, limited ankles/wrists. Enable projection and damping to avoid constraint drift.
- Drive the core with Physical Animation, not every limb equally. The torso/pelvis should try to preserve a readable target pose; limbs can be looser but constrained.
- Change the impulse model from "impulse all bodies" to "impulse pelvis/core/impact body, with optional torque or secondary limb impulse." Let constraints propagate the hit.
- If the mesh still visibly deforms after constraint limits, that becomes Blender work: rigid-ish torso/coat weighting, possibly segmenting fused coat panels or retopo. The current rig report already says the relaxed low-arm pose is acceptable for the spike, not production animation-ready.

### What Claude and Codex agree on

- The walk issue is not a gameplay animation system problem; the C++ single-node path already exists. The missing piece is a compatible FriendSlop skeleton animation asset and test-room/runtime wiring.
- The camera issue is actor-root vs ragdoll-mesh decoupling. The controller is still looking at the pawn; the pawn root is not following the PAC-driven mesh.
- The ragdoll issue is primarily PhysicsAsset constraints and impulse strategy. The current generated asset has bodies/constraints, but not authored angular limits or a Fall Guys-style controlled body profile.

### Where I would be more precise than Claude

- Claude suggested restoring deleted old `AM_Hero_1_Chad_Walk` assets. That may be useful historical reference, but I would not bind them directly to the current FriendSlop skeletal mesh unless Unreal confirms skeleton compatibility. The safer route is reimport/retarget onto `SK_Hero_1_Chad_Male_FriendSlop_Skeleton`.
- Claude focused on angular limits, which is correct, but the visible "torso deformation" can also come from Blender weights/topology. Unreal constraints can stop physical twisting; they cannot fully fix a fused costume mesh whose vertices are skinned across twisting bones.

### Proposed next implementation phase

1. Import or retarget FriendSlop-compatible `Idle` and `Walk` assets, wire them only into the test-room skeletal override, and capture a walk proof.
2. Fix ragdoll camera following by synchronizing actor root to the follow bone during PAC, with a camera-target fallback if needed.
3. Replace the generated-default PhysicsAsset behavior with an authored limited profile: fewer active bodies, explicit constraints, damping/projection, core PAC drive, and core-only launch impulse.
4. Test the throw before any "feel tuning." If the body still twists visually, send the rig back to Blender for rigid torso/coat weight cleanup or segmentation.
5. Only after Hero 1 feels right in the test room should this become a reusable hero incapacitation state and trap interface.

No code changes were made in this pass.
