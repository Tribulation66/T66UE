# MotionRig Rigging Pipeline Instructions

Status: ACTIVE — this is the master rigging/animation pipeline for MotionRig
characters (MOTION_RIG.md is the authority doc for the whole lane). It will
eventually replace the PhysicsFirst process in
`13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md` once the user approves
migration. Hero 1 Chad male is the proof character; future heroes, enemies,
and bosses reuse this process with different proportions.

## What this pipeline produces

From a raw FriendSlop Pixal3D GLB, one deterministic headless Blender run
produces: a fresh 18-bone skeletal FBX with smooth distance-based weights,
six pose-target clip FBXs (Idle, Walk, Jump, Dive, GetUp_Front, GetUp_Back),
QA JSON, and proof renders (skeleton overlay + posed deformation checks).
A UE import script plus a physics-asset commandlet turn those into game
assets under `Content/Characters/MotionRig/<CharacterName>/`.

Clips are POSE TARGETS for the PhysicsControl motors, not final motion. They
should be simple, big, readable. The physics softens and sells them.

## Commands (Hero 1 Chad reference run)

```powershell
# 1. Blender build (rig + weights + clips + QA + proofs)
& "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --factory-startup `
    --python "C:\UE\T66\Scripts\MotionRig\BuildMotionRig.py" -- `
    --glb "C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb" `
    --out "C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\MotionRig"
# expect: MOTIONRIG_BUILD_RESULT=PASS

# 2. UE import (must be -ExecutePythonScript; -run=pythonscript crashes in AssetTools)
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" `
    -ExecutePythonScript="C:/UE/T66/Scripts/MotionRig/ImportMotionRig.py" -unattended -nop4 -nosplash -stdout
# expect: MOTIONRIG_IMPORT_RESULT=PASS (report JSON next to the Blender outputs)

# 3. Physics asset (FBX auto-create does not run in automated imports)
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" `
    -run=T66MotionRigPhysicsAsset -unattended -nop4 -nosplash -stdout
# expect: MOTIONRIG_PA_RESULT=PASS, 18 bodies / 17 constraints
# NOTE: commandlets may exit 1 on a benign startup LogPhysics error — judge by
# the RESULT line and on-disk assets, same policy as the audio importers.
```

## The rig spec (master contract)

- 18 bones: pelvis; spine_01; spine_02; head; clavicle_l/r; upperarm_l/r;
  lowerarm_l/r; hand_l/r; thigh_l/r; calf_l/r; foot_l/r. No fingers, no twist
  bones, no IK bones.
- 180 cm normalized height; Blender front = -Y; UE forward = +X; root at
  origin between the feet.
- Weights: distance-based nearest-segment falloff, max 4 influences/vertex,
  min weight 0.04, falloff power 3.5. HARD (1-influence) SKINNING IS BANNED —
  it reads as rigid plastic; smooth blends read as rubber.
- Physics asset: capsule per bone, generated constraints, adjacent-body
  collisions disabled; the FBX armature-root body is culled.

## Hard-won pitfalls (do not rediscover these)

1. **Bone-heat auto weights fail silently** on generated multi-shell meshes
   (zero vertex groups, no error). That failure is what pushed the old lane
   into rigid 1-influence skinning. Always use the distance-based weighting in
   `BuildMotionRig.py`.
2. **glTF imports arrive in QUATERNION rotation mode.** Assigning
   `rotation_euler` does nothing until `rotation_mode = "XYZ"` is set first —
   the facing flip becomes a silent no-op.
3. **Facing must be auto-detected** (toe direction at the foot band). Raw
   Pixal3D GLBs have shipped facing +Y; never trust a fixed convention.
4. **Landmark measurements must dodge clothing**: hip width measured at hip
   height reads the coat flare (legs land outside the body). Measure leg
   columns at the KNEE band; find hands by walking DOWN the x-extreme column
   (cuffs flare wider than fists).
5. **Scale: bake x100 into mesh+armature before FBX export** (Blender meters →
   UE centimeters). Importer unit magic produced a 1.8 cm character. Clips
   keyframe rotations only, so they survive the scale bake.
6. **UE import path**: `-run=pythonscript` asserts in AssetTools
   (`CurrentApplication.IsValid()` — no Slate). Use `-ExecutePythonScript=`.
7. **`create_physics_asset` on FbxImportUI does not run** in automated
   imports. The `T66MotionRigPhysicsAsset` commandlet owns PA generation.
8. **Runtime: `PlayAnimation`/`SetAnimationMode` re-initializes articulation
   and clobbers `SetSimulatePhysics(true)`** — the mesh silently goes
   kinematic and the bean hangs from its own pelvis constraint (the
   frozen-floating-pawn signature: velocity reads nonzero, position barely
   moves). The pawn re-asserts simulation after every clip change
   (`EnsureMeshSimulation`).

## Verification

- Blender QA JSON: `pass: true`, 0 unweighted vertices, max 4 influences,
  18 bones, `facing_flip_applied` as detected.
- Proof renders: `proof_skeleton_front_with_mesh.png` (bones inside the body),
  `proof_walk_contact.png` (clean stride deformation).
- In-game: `pwsh Scripts/MotionRig/CaptureMotionRig.ps1 -Scenario walkcircle`
  then `python Scripts/MotionRig/AnalyzeTelemetry.py <telemetry.csv>` —
  invariants must PASS (no explosion, no floor penetration, no jitter).
