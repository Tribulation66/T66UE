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

## Commands (Hero 1 male + female reference run)

Source models (the runtime pawn picks the asset set from the hero-select body
type; captures force it with `-T66BodyType=`):

- Male / ET66BodyType::Chad: `HeroChadStacy_SourceAssets_20260609_0536/Outputs/Hero2Chad.glb`
  (hanging arms — build with `--pose hanging`, the default).
- Female / ET66BodyType::Stacy: `StacyTPoseVar1_20260611/Outputs/Hero1Stacy_TPose_Var1.glb`
  (T-POSE inflatable Variation 1, designed reference v5 in
  `Saved/Codex/ModelGeneration/StacyPhysiqueRef_20260611/` — build with
  `--pose tpose`).

PREFER T-POSE for new sources: the distance-based skinning is computed while
the arms are far from the torso/thighs (its best case), then the build rotates
the arms down 75° and applies that as the new rest pose, so clips/export/UE
keep the hanging-arm conventions automatically.

```powershell
# 1. Blender build (rig + weights + clips + albedo + QA + proofs), once per body
& "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --factory-startup `
    --python "C:\UE\T66\Scripts\MotionRig\BuildMotionRig.py" -- `
    --glb "...\HeroChadStacy_SourceAssets_20260609_0536\Outputs\Hero2Chad.glb" `
    --out "...\HeroChadStacy_SourceAssets_20260609_0536\Blender\MotionRig\Male" --name Hero1Male --front +y
& "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --factory-startup `
    --python "C:\UE\T66\Scripts\MotionRig\BuildMotionRig.py" -- `
    --glb "...\StacyTPoseVar1_20260611\Outputs\Hero1Stacy_TPose_Var1.glb" `
    --out "...\StacyTPoseVar1_20260611\Blender\MotionRig\Female" --name Hero1Female --front +y --pose tpose
# expect: MOTIONRIG_BUILD_RESULT=PASS (each). --front +y is REQUIRED for
# Pixal3D sources: chunky boots fool the toe-direction auto-detection.
# Source paths live in ImportMotionRig.py CHARACTERS — update them when a new
# run replaces a model.

# 2. UE import — BOTH characters in one run (must be -ExecutePythonScript;
#    -run=pythonscript crashes in AssetTools)
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" `
    -ExecutePythonScript="C:/UE/T66/Scripts/MotionRig/ImportMotionRig.py" -unattended -nop4 -nosplash -stdout
# expect: MOTIONRIG_IMPORT_RESULT=PASS (report JSON next to the Blender outputs)
# destinations: /Game/Characters/MotionRig/Hero_1_Male + Hero_1_Female
# (SK_MotionRig_Hero1Male/-Female, T_MotionRig_*_BaseColor, 6 clips each)

# 3. Physics assets — BOTH characters in one run (the commandlet owns PA generation)
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" `
    -run=T66MotionRigPhysicsAsset -unattended -nop4 -nosplash -stdout
# expect per character: MOTIONRIG_PA_RESULT=PASS, 18 bodies / 17 constraints,
# and the unit guard line MOTIONRIG_PA_REFPOSE pelvisLocalZ=98.10 ...
# calfLocal=45.90 rootScale=V(X=1.00, Y=1.00, Z=1.00) — anything else means a
# broken export

# 4. Verify in-game (one capture per body type; -BodyType maps to -T66BodyType=)
pwsh C:\UE\T66\Scripts\MotionRig\CaptureMotionRig.ps1 -Scenario walkcircle -Camera threequarter -BodyType chad -Label male_check
pwsh C:\UE\T66\Scripts\MotionRig\CaptureMotionRig.ps1 -Scenario walkcircle -Camera threequarter -BodyType stacy -Label female_check
# NOTE: commandlets may exit 1 on a benign startup LogPhysics error — judge by
# the RESULT line and on-disk assets, same policy as the audio importers.
```

## Split-generation assembly (separate head + body models)

`Scripts/MotionRig/AssembleHeadBody.py` joins a headless body GLB and a head
GLB before rigging. The knobs that define the look:

- **Head size**: `--head-frac` (default 0.42) = head+hair height as a
  fraction of body height. This is an EXPLICIT, documented decision — the
  neck-diameter auto-anchor was removed after hair strands repeatedly
  contaminated the measurement and shrank the head. Bigger = more toy-like.
- **Neck naturalness**: `--sink` (default 2.0) = how deep the head's neck
  stub buries into the body's stump (units of 0.10×head-height). Too low
  leaves a visible double-neck pole; the default seats the chin just above
  the stump. Reference images must give the BODY a short capped neck stump
  and the HEAD a cylindrical neck stub — the stub/stump pair is also the
  positioning anchor (their centers align).
- **Color match**: automatic, skin-weighted (head atlas corrected toward the
  body's mid-thigh skin sample; hair/eyes untouched). Logged as COLOR_MATCH.
- **Artifact cleanup**: flat-slab shards and far-from-body floating debris
  are dropped; tiny ON-SURFACE fragments are KEPT — deleting them punches
  visible holes (dark flecks in-game; verify with a BACKFACE-CULLED Blender
  render, the default workbench render hides holes).
- Verify every assembly with the culled front render before rigging.

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
5. **Units (doctrine v2 — raw binary FBX probe, 2026-06-10): the Blender FBX
   exporter converts NOTHING m→cm.** A meter-scene export writes meter
   numbers for rest bones, anim curves AND verts, then compensates with
   scale=100 on the armature/mesh OBJECT nodes. UE turns that into a
   scale-100 root bone: component-space looks right but the physics-asset
   generator and world-space bone writes use unscaled bone locals → bodies
   collapse to a point, zero-length anchors, centimeter-sized render. The
   only consistent form is REAL cm numbers with scale 1 everywhere:
   `convert_scene_to_centimeters()` bakes x100 at the DATA level
   (`Mesh.transform`/`Armature.transform` — object-level scale+apply on the
   parented pair double-scales the child mesh) plus x100 on location
   fcurves, and `global_scale=0.01` in FBX_COMMON cancels the exporter's
   invariant x100. Verify with `ProbeFbxRaw.py`/`ProbeFbxRaw2.py`: pelvis
   LclTranslation z=98.1, verts 0..180, all node scalings 1.0.
6. **UE import path**: `-run=pythonscript` asserts in AssetTools
   (`CurrentApplication.IsValid()` — no Slate). Use `-ExecutePythonScript=`.
   Also: UE 5.7 routes FBX through **Interchange by default, which IGNORES
   legacy FbxImportUI options** (measured: `use_t0_as_ref_pose` no-op).
   `ImportMotionRig.py` disables it via
   `Interchange.FeatureFlags.Import.FBX 0` to force the legacy importer.
7. **`create_physics_asset` on FbxImportUI does not run under Interchange**
   — but the LEGACY importer honors it, so the import script sets it False
   explicitly. The `T66MotionRigPhysicsAsset` commandlet owns PA generation
   and refuses collapsed (sub-50cm component-space pelvis) skeletons.
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
