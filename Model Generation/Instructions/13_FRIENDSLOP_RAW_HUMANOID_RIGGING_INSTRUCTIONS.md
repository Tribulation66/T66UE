# Physics-First FriendSlop Hero Rigging Instructions

## Purpose

This is the canonical process for raw FriendSlop Pixal3D humanoid heroes when the target is the T66 physics-first playable hero standard.

The goal is not a normal humanoid animation bridge. The goal is a clean, physics-ready hero foundation for Fall Guys-like movement: simple pose-target clips, chunky body readability, reliable PhysicsAsset roles, and later active-ragdoll pose drive.

Use this process for Hero 1 Chad first. Future heroes should copy the method only after Hero 1 Chad passes proof.

## Current Direction

The old raw humanoid spike and Animated ToonStyle bridge are not the current foundation.

Do not build this process from:

- Archived AccuRig or Animated ToonStyle lineups.
- Quaternius-derived role clips.
- Existing spike FBXs under a prior `Blender/Rigging` folder.
- Existing `Idle/Walk/Jump/Roll` animation source exports.
- ToonStyle, Quad Retro, tint, inner-line, or outline sidecar processing.

Existing reports and proof renders may be read only as cautionary evidence. Do not copy the old rig, old skin weights, old FBX exports, or old animation clips into the new standard.

## Stage 2 Source Contract

Hero 1 Chad Stage 2 starts from the raw source GLB:

```text
Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb
```

Write fresh Stage 2 outputs to a new folder so the old spike stays readable:

```text
Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Blender/PhysicsFirstHero/
```

The folder must include:

- Blender working file.
- Skeletal FBX.
- Animation FBXs.
- QA JSON.
- Rig/animation report.
- Facing/rest/bend proof renders when Blender rendering is available.
- Unreal import report after import.

## Scope Boundary

This process may produce and wire the Stage 2 rig/animation foundation:

- Fresh Hero 1 Chad skeleton.
- Fresh skin weights.
- `Idle`, `Walk`, `Jump`, `Leap` clips.
- Optional recovery pose targets such as `RecoverStand`, `GetUp_Back`, or `GetUp_Front`.
- Unreal skeletal mesh and animation import.
- `CharacterVisuals.csv` and runtime code/data changes needed for Leap.

This process does not implement Stage 3 active-ragdoll runtime mechanics:

- No new `UPhysicalAnimationComponent` behavior.
- No always-simulated mesh policy.
- No hip/pelvis world constraint component.
- No reaction profile system.
- No broad all-hero migration.

## Rigging Philosophy

Animation clips are pose targets for later physics. They are not the full motion system.

The rig must therefore optimize for:

- stable pelvis/torso mass roles
- readable bean-like body silhouette
- simple, low-frequency pose targets
- clean shoulder/hip/neck/knee/ankle deformation
- predictable PhysicsAsset generation
- fast recovery pose targets
- preserving the raw FriendSlop texture identity

Do not optimize for realistic biped detail, retargeting large animation packs, fingers, twist bones, or complex locomotion layering in the MVP.

## Deform Skeleton Contract

Use one deform armature and one hierarchy.

Required hierarchy:

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

Hard rules:

- `pelvis` is the central follow/recovery body, not `root`.
- Export deform bones only.
- Do not export helpers, controls, duplicate roots, leaf bones, or mesh-parented bones.
- Do not add fingers or twist bones for the MVP unless a future acceptance gate requires them.
- Bone names should stay UE-friendly to reduce import and PhysicsAsset friction.

## Rest Pose

Preferred production rest pose is a soft A-pose:

- arms separated enough for shoulder weights and body collision
- feet grounded at world `Z=0`
- body centered
- approximate 180 cm height unless source scale requires otherwise
- no broad destructive transform of fused coat/costume panels

If the source mesh cannot survive a clean soft A-pose, document the compromise and keep the pass clearly marked as foundation/interim. Do not pretend a relaxed low-arm spike pose is the production standard.

## Skinning Requirements

Hard gates:

- Maximum four influences per vertex.
- All weights normalized.
- No unweighted vertices.
- No root-only or pelvis-only accidental blobs.
- No near-zero required bones.
- Bend proof at shoulders, elbows, hips, knees, ankles, and neck.

Production target:

- shoulder, hip, neck, knee, ankle, and costume-panel deformation should be clean enough for simple pose-target clips.
- retopo is not mandatory up front, but topology-driven tearing must be reported honestly.
- algorithmic weights are acceptable only as an explicitly labeled foundation/interim; final production acceptance requires visual bend proof.

## Clip Contract

Stage 2 clip set:

- `Idle`
- `Walk`
- `Jump`
- `Leap`
- optional `RecoverStand`, `GetUp_Back`, `GetUp_Front`

There is no `Run` clip in the MVP.

There is no `Roll` clip in the new standard. Leap replaces the old roll ability concept.

## Character Visual Data Contract

`Content/Data/CharacterVisuals.csv` remains the runtime seam for playable visual wiring.

Use the explicit animation fields:

- `IdleAnimation`: looping idle or preview animation.
- `WalkAnimation`: looping movement animation.
- `JumpAnimation`: one-shot jump animation.
- `LeapAnimation`: one-shot leap animation.

Do not reintroduce legacy animation column names such as `LoopingAnimation`, `AlertAnimation`, or `RunAnimation`, and do not hide Leap in a permanent `RollAnimation` slot. Legacy reports or exporters may still mention `Roll`, but the FriendSlop physics-first runtime contract is `LeapAnimation`.

All Stage 2 clips should be in-place/no-root-motion so gameplay movement and later active-ragdoll anchoring own world displacement.

Clip style:

- low-frequency and readable
- slightly bouncy
- strong center-of-mass cues
- simple limb motion
- no realistic high-frequency foot-plant complexity
- no foot IK requirement in MVP
- no animation-only fake wobble as a substitute for Stage 3 physics

## Leap Contract

Leap is a forward hop/dive-like burst, not a roll.

Expected pose beats:

1. short crouch/anticipation
2. forward/upward stretched launch pose
3. compact airborne tuck
4. short landing/recovery pose

The animation is a pose target. Runtime displacement belongs to the movement component and, later, the active-ragdoll anchor.

## Facing And Texture Import Gates

Prove the source's true visual front before writing `CharacterVisuals.csv` facing values or Unreal import transforms. Do not inherit legacy Animated ToonStyle yaw assumptions. If the source requires a yaw correction before weight transfer or runtime display, record the diagnostic evidence and the exact correction in the rig report.

For skeletal imports, preserve the raw FriendSlop texture identity explicitly. Do not rely on FBX source-material preservation alone; missing FBX material slots can import as `WorldGridMaterial` or another generic fallback and erase the character-specific color. The Unreal import report must identify the material or material instance assigned to the skeletal mesh and the raw base-color texture it uses.

## PhysicsAsset Intent

The skeleton must let Unreal create stable bodies for:

- pelvis
- spine chain
- head
- upper/lower arms and hands if stable
- thighs, calves, feet

Initial mass-role intent:

- pelvis/torso are the dominant body mass
- arms and legs are secondary stabilizers
- feet support contact/rebound
- ball bones are anchors, not necessarily separate physics bodies

Stage 2 only prepares this structure. Stage 3 owns always-on simulation, PAC pose drive, and hip/pelvis constraint behavior.

## Proofs To Produce

Every implementation pass must produce:

- source GLB path used
- output folder path
- Blender version
- final bounds and height
- true visual front
- exported bone hierarchy
- vertex influence QA
- rest-pose proof
- bend proof
- clip list and frame ranges
- exported FBX list
- pre-Unreal pose or clip proof showing idle stability and readable walk/jump/leap motion with no unintended flips
- re-import validation when available
- known compromises

Unreal wiring must produce:

- skeletal mesh import report
- animation import report
- `CharacterVisuals.csv` row check
- DataTable reload/import evidence
- focused compile evidence
- movement QA or runtime capture evidence when playable content changes

## Things To Avoid

- Do not restore deleted old assets.
- Do not use old mid-change assets as the new foundation.
- Do not reuse the old spike rig, old spike weights, or old spike animation FBXs.
- Do not hide Leap in a permanent `RollAnimation` slot.
- Do not touch all heroes just because Hero 1 Chad changes.
- Do not implement Stage 3 active ragdoll in this Stage 2 pass.
- Do not accept compile success as animation proof.
- Do not use desktop screenshots for gameplay proof; use Unreal-owned capture paths.
