# Mob Vertex Animation Pipeline Instructions

## Goal

Create professional mob animation sources that bake to vertex animation textures for runtime, so T66 can display many enemies at once without paying per-enemy skeletal animation cost.

Use armatures, shape keys, lattices, constraints, or simulations inside Blender when they help author the motion, but the runtime deliverable for normal mobs is a static mesh driven by vertex baked animation data.

## When To Use This

Use this pipeline for:

- regular enemies
- difficulty batch mob animation passes
- blobs, insects, spiders, bats, quadrupeds, statues, swarms, wisps, and other non-humanoid silhouettes
- any mob that needs many on-screen copies

Do not use this pipeline for playable heroes or humanoid companions. The user is handling those manually outside this automated mob/VAT path.

## Required Read Order

Before editing or generating mob animation assets, read:

1. `RIGGING_ANIMATION_AGENTS.md`
2. `00_RIGGING_ANIMATION_ROUTING_INSTRUCTIONS.md`
3. `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`
4. this file
5. `06_MOB_ANIMATION_GUIDELINES.md`
6. the batch file for the requested difficulty, such as `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md`
7. `../Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md` before any Unreal import or playable wiring
8. the relevant Gameplay instruction files when runtime behavior or stage data is touched

Also read any `pending_issues_*.md` file in every folder you touch.

## Core Contract

The source-of-truth asset is an editable Blender scene. The runtime asset is a static-mesh visual with vertex baked animation, not a live skeletal mesh component.

The production contract is:

- one approved visible mob mesh/source per `EnemyID`
- one Blender source scene per mob or per tightly related batch
- authored animation clips that match that enemy's intended behavior
- multi-angle Blender QA contact sheets before Unreal import
- Unreal VAT assets/materials imported only after Blender QA passes
- temporary test data/runtime wiring first
- live data promotion only after in-game visual, scale, behavior, and performance validation

## Current Runtime Baseline

Current regular mobs resolve through:

- `Content/Data/Enemies.csv`
- `Content/Data/CharacterVisuals.csv`
- `Source/T66/Gameplay/T66EnemyBase.h`
- `Source/T66/Gameplay/T66EnemyBase.cpp`
- `Source/T66/Gameplay/Enemies/T66EnemyFamilyResolver.cpp`

`AT66EnemyBase` currently owns both a skeletal `GetMesh()` component and a static `VisualMesh` component. Mob visuals are applied through `UT66CharacterVisualSubsystem::ApplyCharacterVisual(...)`.

The Easy-mob VAT implementation adds a dedicated runtime seam instead of overloading skeletal animation slots:

- `FT66MobVertexAnimationRow` in `Source/T66/Data/T66DataTypes.h`
- source CSV: `Content/Data/MobVertexAnimations.csv`
- Unreal data table: `/Game/Data/DT_MobVertexAnimations.DT_MobVertexAnimations`
- runtime loader/application: `UT66CharacterVisualSubsystem::TryGetMobVertexAnimationRow(...)` and `ApplyMobVertexAnimationVisual(...)`
- enemy playback owner: `AT66EnemyBase`, which selects `Idle`/`Move` by velocity and short one-shot overrides for `AttackCue`, `HitReact`, and `Death`

`CharacterVisuals.csv` remains the static fallback visual table. Do not overload `LoopingAnimation`, `AlertAnimation`, `RunAnimation`, or `RollAnimation` with VAT clip state.

## Required Runtime Data Seam

Before a VAT mob can be production-live, create or identify a dedicated runtime data seam for vertex animation. A safe first design is a mob visual animation table keyed by `VisualID` or `EnemyID`, with fields equivalent to:

- `EnemyID` or `VisualID`
- base static mesh
- VAT material parent or material instance
- position texture
- normal texture if used
- AnimToTexture data asset if the project keeps one
- idle clip frame range and rate
- move clip frame range and rate
- attack or cast clip frame range and rate
- hit react clip frame range and rate, if authored
- death clip frame range and rate, if authored
- local bounds expansion
- ground or hover mode
- per-instance phase/randomization support

The visual subsystem should apply VAT materials and parameters from data, not hardcoded C++ defaults. First implementation can prove the path with individual `UStaticMeshComponent` material instances. A later scaling pass can move the same data to instanced rendering when the enemy director/pooling path is ready.

## Unreal VAT Tooling

UE 5.7 includes an Experimental `AnimToTexture` plugin at:

```text
C:\Program Files\Epic Games\UE_5.7\Engine\Plugins\Experimental\AnimToTexture
```

The plugin exposes `UAnimToTextureBPLibrary::AnimationToTexture(UAnimToTextureDataAsset*)` and supports vertex and bone texture modes. For normal mobs, prefer vertex mode unless a specific mesh needs bone mode for memory or deformation reasons.

Use the plugin as the Unreal bake/import path only after:

- the plugin is enabled in `T66.uproject`
- the mob has a skeletal or deforming bake source imported into Unreal as a temporary bake-only asset
- the bake output static mesh/material/textures are isolated under a QA path
- the runtime material can drive frame playback from data parameters

Keep bake-only skeletal assets out of live `CharacterVisuals.csv` rows unless a specific QA row explicitly says it is a temporary bake source.

## UE 5.7 AnimToTexture Notes

Current repo-proven notes:

- Enable `AnimToTexture` in `T66.uproject` before running the import tooling.
- `UnrealEditor-Cmd.exe -run=pythonscript` can inspect many assets, but FBX import through this path can fail with a Slate application assertion and generated static mesh UV-channel reads can report zero channels. Use full `UnrealEditor.exe -ExecutePythonScript=...` for FBX import/bake work and UV2-sensitive VAT verification.
- Use `Scripts/RunRiggingAnimationToolAndExit.py` with `T66_RIGGING_ANIMATION_TOOL_SCRIPT` and `T66_RIGGING_ANIMATION_TOOL_QUIT_EDITOR=1` for full-editor automation under paths with spaces.
- Pass Unreal Python script paths with forward slashes. Backslash script paths can be mangled by command-line escaping, and Python docstrings containing Windows paths must be raw strings or use escaped backslashes.
- Scan `/AnimToTexture`, `/AnimToTexture/Characters`, and `/AnimToTexture/Materials` in the asset registry before loading plugin sample assets.
- Use VAT UV channel `2`. Channel `1` conflicts with generated lightmap UVs on the converted static mesh.
- First-time bakes may warn that UV channel `2` is out of range before the plugin writes the channel. Treat the warning as non-fatal only if the output textures/material parameters and runtime playback verification pass.
- In UE 5.7 Python, direct reads of `UAnimToTextureDataAsset` `FVector3f` bounds can return zero even when the bake is valid. After `UpdateMaterialInstanceFromDataAsset`, read `MinBBox` and `SizeBBox` from the material instance and write those values to `MobVertexAnimations.csv`.
- Generated VAT material custom nodes that call `TransformLocalVectorToWorld` must pass the material `Parameters` argument, for example `TransformLocalVectorToWorld(Parameters, local_delta)`. The one-argument form can appear to work in editor automation but fails cooked material compilation and falls back in staged builds.
- After changing generated custom-node HLSL, force-recreate or explicitly rebuild the generated material master before reparenting material instances. A stale expression graph can keep invalid shader-map state through cook.
- Avoid ad-hoc deletion of arbitrary loaded assets inside the same commandlet that created them; it can trigger `ForceDeleteObject` ensures. If the pipeline owns a generated material and must recreate it, do that intentionally in the importer, save all dependents, and prove the result with a full cook/stage smoke.
- Full cook/stage is required after generated material or content-bake changes. `-SkipCook` restage is only acceptable for a later code-only verification pass after content cookability has already passed.
- Staged smoke must fail the pass if the log contains VAT material compile failures, invalid shader maps, uncooked shader-map IDs, default-material fallback, fatal errors, or assertion failures.

## Blender Source Process

For each mob:

1. Resolve the live `EnemyID`, `CharacterVisuals.csv` row, static mesh path, texture, and mesh scale.
2. Locate the approved source mesh. Prefer the production source GLB or Blender file that generated the live static mesh.
3. Confirm promotion status before using generated GLBs as final source. If the source report says exploratory, document the remaining visual or runtime gate before export.
4. Import the source mesh into Blender and verify mesh count, material count, texture assignment, forward axis, ground contact, and scale.
5. Build the animation authoring rig appropriate to the silhouette:
   - simple bones for humanoids and spiders
   - lattice, bend bones, and shape keys for blobs
   - wing bones and body hover controls for flying enemies
   - local object controls for swarms or multi-part fused mobs
   - recoil/aim controls for statues or turret-like enemies
6. Keep world translation actor-driven. Local motion should sell gait, drag, hover, flap, recoil, squash, or impact without moving the mesh across the level.
7. Bake deformations to a stable mesh with unchanged topology. VAT requires the same vertex order and vertex count for every frame in a clip.
8. Render QA sheets from front, side, three-quarter, and gameplay-camera views.
9. Fix silhouette, clipping, contact, timing, bounds, and scale issues in Blender before Unreal export.
10. Export the bake source and manifest only after the multi-view QA pass is acceptable.

## Required Clips

Minimum first production set for regular mobs:

- `Idle`: readable ambient motion while actor is stationary or waiting
- `Move`: behavior-specific locomotion or hover motion while gameplay moves the actor
- `AttackCue`: wind-up, cast, bite, recoil, or lunge cue that can later be event-driven
- `HitReact`: short readable damage response if the mob survives hits
- `Death`: optional for the first proof if runtime death visuals are not wired, but required before final production sign-off

Do not force every mob into a biped walk cycle. The move loop must match the visual concept:

- slimes drag, squash, and stretch at ground contact
- bats hover and flap
- spiders use alternating leg groups
- swarms jitter and scuttle as a mass
- statues idle heavily and recoil or aim
- caster mobs should pulse, chant, or cast instead of sprinting like melee mobs

## Blender QA Evidence

Every mob VAT run must produce:

- a source `.blend`
- a manifest with source mesh path, live row, scale, frame ranges, export paths, and known caveats
- per-clip contact sheets from front, side, three-quarter, and gameplay-camera angles
- an all-view contact sheet for quick review
- a written QA note describing accepted issues and fixed issues

Reject the pass if:

- only one still frame exists
- only one camera angle exists
- motion is recognizable but not behavior-correct
- the source mesh is not proven to match the live visual
- topology changes across frames
- ground mobs slide without a visual reason
- flying mobs read as walking or falling
- scale or origin differs from the live row without documented runtime compensation

## Unreal Import And Runtime QA

After Blender QA passes:

1. Import the bake-only source under a QA folder, not a live production folder.
2. Run the AnimToTexture bake into an isolated VAT QA asset path.
3. Create a temporary mob visual animation row or temporary test map/row.
4. Validate the material plays each clip at the expected rate.
5. Validate bounds. VAT displacement can move vertices outside the static bounds if the mesh bounds are not expanded.
6. Validate many instances with randomized phase. A crowd of identical enemies should not pulse in exact sync unless the design asks for that.
7. Validate gameplay behavior still owns movement, attacks, collision, hit reactions, and death state.
8. Verify the CSV and `/Game/Data/DT_MobVertexAnimations` with `Tools/verify_easy_mob_vat_in_unreal.py` or the matching batch verifier before live promotion.
9. Promote live data only after temporary in-game QA proves scale, look, animation, and performance.
10. If playable standalone is affected, refresh the staged standalone build and verify `T66 Standalone.lnk` targets `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

## Performance Acceptance

A mob VAT pass is not production-accepted until runtime evidence shows:

- the live enemy does not tick a skeletal animation graph for normal playback
- many copies can animate simultaneously without obvious CPU animation spikes
- animation phases can be desynchronized
- material parameter updates are data-driven and bounded
- culling bounds are correct
- collision and damage capsules remain gameplay-owned and stable

## Documentation Output

Every completed mob pass must update:

- this pipeline if the process changed
- the relevant difficulty batch instruction file
- `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md` with what worked and what failed
- any touched runtime folder's `pending_issues_*.md` when an out-of-scope issue is discovered

## Prohibited Shortcuts

- Do not wire bake-only skeletal assets into live mob rows as if they are final runtime visuals.
- Do not reuse humanoid animation clips on non-humanoid mobs unless the body plan and behavior actually match.
- Do not mark a mob done because it has any motion. The motion must fit the enemy's gameplay read.
- Do not use an exploratory source asset for final content without documenting visual and runtime acceptance.
- Do not skip multi-angle Blender QA.
- Do not hardcode VAT asset paths or clip timings in C++.
