# T66 Model Rigging Stage 1 Information Dump

Generated: 2026-05-18 from workspace `C:\UE\T66`.

Scope: scratch reference for Claude/Pablo. This file intentionally does not change the existing rigging instruction docs. It consolidates current docs, environment state, runtime code seams, data-table reality, performance assumptions, and observed visual evidence.

## Working Goal

Create a complete current-state information dump for turning T66 rigging and animation into a documented, iteratively improvable system.

## Source Boundaries

- Repo root: `C:/UE/T66`.
- Current rigging docs are copied verbatim below.
- Planned final counts are reported only where repo data explicitly provides counts. No planning numbers were inferred.
- Direct visual assessment is based on the contact sheets opened from the paths listed in section G.
- The term `in-engine` below means the CSV/data-table paths and runtime C++ code point at Unreal assets. This pass did not load every referenced `.uasset` individually in the editor.

## A. Current Rigging Docs - Verbatim

### Model Generation/Rigging and Animation/RIGGING_ANIMATION_AGENTS.md
<pre><code># Rigging And Animation Agents

## Owns

Blender source-of-truth character rigs, Rigify/Rigodotify setup, Quaternius animation-library references, humanoid retargeting, editable Blender actions, animation QA previews, and export manifests before Unreal import.

## Trigger Words

Rigging, animation, Rigify, Rigodotify, Quaternius, retarget, roll animation, jump animation, walk animation, idle animation, Blender action, armature, skeletal mesh export.

## Read First

- `README.md`
- `00_RIGGING_ANIMATION_ROUTING_INSTRUCTIONS.md`
- Then the specific numbered instruction file for the task.
- For playable imports, also read `../Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`.

## Hard Rules

- Blender `.blend` scenes are the source of truth for editable rigs and actions.
- Keep third-party packages in `External/`; do not commit extracted vendor files.
- Do not treat a recognizable pose as an accepted animation. Contact, timing, silhouette, foot slide, mesh deformation, and recovery must pass visual QA.
- Do not wire new animation assets into playable content until Blender QA and Unreal import validation are complete.
- Record tool failures and workarounds in `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md` as they are discovered.

## Verification

Report the exact Blender version, add-on status, source files used, exported files, Blender preview evidence, Unreal import evidence, DataTable/runtime wiring evidence, and staged standalone evidence when gameplay-visible content changes.
</code></pre>

### Model Generation/Rigging and Animation/02_HERO_ANIMATION_PIPELINE_INSTRUCTIONS.md
<pre><code># Humanoid Hero And Companion Animation Pipeline

This process is for full humanoid action sets for heroes and companions. Use it for bipedal player-scale characters that can share a humanoid skeleton, humanoid retarget map, or Rigify/Rigodotify control rig.

Monster, boss, quadruped, flying, blob, and non-humanoid rigs need a separate pipeline. Do not force them through this humanoid process just because they need animation clips.

Typical hero scope:

- idle
- walk or run
- jump
- roll or dodge
- attack
- later ability-specific actions

Typical companion scope:

- idle
- walk or follow
- alert or interact
- attack or support action when gameplay requires it

## Acceptance Bar

Do not accept a humanoid action because it is merely recognizable. A production candidate must satisfy:

- readable silhouette from gameplay and preview camera angles
- stable proportions across the whole action
- no severe shoulder, elbow, wrist, hip, knee, or ankle collapse
- foot contact that does not slide unless intentionally stylized
- root or pelvis motion that matches the intended in-game behavior
- clean start and recovery poses for blending
- no mesh clipping that is visible from front, side, and three-quarter review angles

Roll-specific checks:

- clear crouch or dip before launch
- believable plant and tuck
- shoulder/back/side contact implied by body arc
- pelvis and head follow a coherent path
- feet recover under the body
- no full-body spin that reads like a generic tumble instead of an intentional combat roll
- for in-place gameplay rolls, side and gameplay-camera sheets must show the character tucking forward in the same direction the actor travels; a backward-opening backflip pose fails even if the runtime launch direction is correct

Jump-specific checks:

- anticipation before takeoff
- upward extension during launch
- airborne shape readable from the gameplay camera
- landing compression and recovery
- no looping airborne pose unless the runtime deliberately holds it

## Required Loop

Every humanoid animation pass must follow this order:

1. Identify the character, current source mesh, skeleton, visual row, and required gameplay states.
2. Create or update a Blender run folder and treat its `.blend` file as the source of truth.
3. Build or confirm the rig:
   - If the character already has a production skeletal mesh, preserve that mesh and skeleton unless there is a clear reason to rerig.
   - If the character is static or has an unusable skeleton, build a Rigify/Rigodotify-compatible humanoid rig in Blender.
   - Keep deformation bones and control bones understandable; name any generated retarget map in the run manifest.
4. Retarget or author the actions in Blender.
5. Render preview frames before export.
6. Review the contact sheets and write the QA result.
7. Make Blender-side corrections for any accepted blocker.
8. Rerender previews after corrections.
9. Export action clips only after the corrected Blender preview pass is acceptable.
10. Import into Unreal and verify asset type, skeleton, play length, and data wiring.
11. If gameplay-visible, refresh staged standalone and verify the shortcut target.

Skipping Step 5-8 is allowed only for a deliberate rough prototype, and the final report must say it is not production accepted.

## Blender Rigging Rules

- Keep one source `.blend` per character/run under `Runs/&lt;Character&gt;_&lt;Purpose&gt;_&lt;YYYYMMDD&gt;/`.
- Keep the target character armature, source/reference armature, retarget map, baked actions, preview camera setup, and export settings in that scene.
- Do not destructively edit vendor `.blend` files under `External/`; append/link or import into the run scene.
- Prefer action names that match Unreal output names, for example `AM_Hero_1_Chad_Walk`.
- For heroes, preserve the canonical game mesh scale and skeleton identity unless the task is explicitly to replace the rig.
- For companions, use the same humanoid checks, but action scope can be smaller if gameplay only needs idle/follow/alert.

## Visual QA

Required Blender preview evidence for production acceptance:

- front contact sheet
- side contact sheet
- three-quarter contact sheet
- gameplay-camera contact sheet when the runtime camera is different enough to hide contact or silhouette issues
- frame samples that include anticipation, contact, apex/impact, and recovery poses
- written QA notes listing either `PASS` or concrete issues per action

Review each action for:

- silhouette readability
- foot and hand contact
- pelvis path
- head path
- shoulder, elbow, wrist, hip, knee, and ankle collapse
- visible mesh clipping
- scale drift or mesh deformation
- start and recovery poses that can blend back to idle/walk

When a preview fails, fix the Blender action first. Do not compensate for a bad action by hiding it in Unreal, shortening the clip blindly, or declaring it done from a single still frame.

## Unreal Import And Runtime Wiring

Heroes currently use `CharacterVisuals.csv` / `DT_CharacterVisuals` animation slots:

- `LoopingAnimation`: walk/run loop
- `AlertAnimation`: idle/alert loop
- `RunAnimation`: current jump slot for the Arthur pilot
- `RollAnimation`: one-shot roll

Companions currently consume humanoid movement/alert slots through the character visual subsystem. Add companion-specific runtime state only when the companion gameplay requires an action that the current slots cannot represent.

Unreal validation must confirm:

- imported assets are `AnimSequence`
- skeleton path matches the character skeleton
- play length is nonzero and expected
- data row points to the imported assets
- runtime preload path includes new animation slots
- staged standalone is refreshed when playable behavior changed

## Arthur Pilot

Use Arthur as the first proof case before scaling to all heroes.

1. Select the canonical Arthur mesh for the pilot.
2. Fit or generate a Rigodotify/Rigify-compatible humanoid rig.
3. Retarget or author `Idle`, `Walk`, `Jump`, and `Roll`.
4. Save the `.blend` scene as the source of truth under a run folder.
5. Render front, side, and three-quarter action previews.
6. Export action clips only after Blender QA passes.
7. Import into Unreal through the existing skeletal import path.
8. Wire a test animation set before switching default Arthur visuals.

Deleted Arthur pilot run:

- `Hero_1_Chad_AnimPilot`, the old root-level `SK_Hero_1_Chad` skeletal mesh, and the pilot-specific tools/runs were deleted after the QuadRetro pass was validated.
- That pilot was useful only as a failed research path. It was not the selected Royal Chad/Arthur visual and previously caused the old headless/giant-model failure when wired live.
- Do not restore or rerun the deleted pilot path for production Arthur work.

Use forward slashes for Unreal Python `-script=` paths when invoking tools under this folder; backslashes before filenames such as `verify_...` can be parsed as escape characters by the commandlet path reader.

When invoking Rigging and Animation tools from Unreal, prefer the wrapper `Scripts/RunRiggingAnimationToolAndExit.py` with `T66_RIGGING_ANIMATION_TOOL_SCRIPT` set to the real tool path. Do not use 8.3 short paths such as `MODELG~1` / `RIGGIN~1` as a workaround for spaces; Unreal&#x27;s Python runner can treat those as Python text and fail with `SyntaxError`.

Accepted Royal Chad/Arthur QuadRetro UAL retarget run:

- Correct playable visual row: `Hero_1_Chad`.
- Correct source model: `../Runs/Pixal3D/HeroArthur01/Post/QuadRetro/arthur_royal_chad/Models/arthur_royal_chad_QuadRetro.glb`.
- Correct skeletal texture: `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/RoyalChad_QuadRetro/Textures/RoyalChad_QuadRetro_Pixelated_512.RoyalChad_QuadRetro_Pixelated_512`.
- Do not apply the normalized static-mesh atlas to the GLB-derived skeletal mesh. The animated mesh keeps the GLB UV layout, so using `RoyalChad_QuadRetro_Pixelated_512_Normalized` scrambles the runtime texture.
- Correct animation source: `External/Quaternius/Universal Animation Library Source/UAL1.blend`.
- UAL action mapping: `Idle_Loop` -&gt; idle, `Walk_Formal_Loop` -&gt; walk, `Jump_Start` + `Jump_Loop` + `Jump_Land` -&gt; jump, `Roll_RM` -&gt; roll with root motion stripped and the local-X sagittal mirror correction applied by `Tools/create_arthur_quadretro_ual_animation_source.py`.
- Blender source of truth: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_QuadRetro_UAL_Retarget.blend`.
- FBX exports: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Exports/SK_Hero_1_Chad_QuadRetroUALQA.fbx` and `AM_Hero_1_Chad_QuadRetroUALQA_{Idle,Walk,Jump,Roll}.fbx`.
- QA contact sheets: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_All_Actions_{front,side,three_quarter,gameplay}_Contact_Sheet.png`.
- Unreal import tool: `Tools/import_arthur_quadretro_animation_to_unreal.py`.
- Unreal verification tool: `Tools/verify_arthur_quadretro_animation_in_unreal.py`.
- Tool-only validation row: `Hero_1_Chad_QuadRetroUALQA`. This row exists for Arthur/UAL rigging pipeline validation tools and is not the gameplay-selected row.
- Promoted live row: `Hero_1_Chad`, after the temporary row was visually verified in gameplay.
- Correct promoted-row forward fix: `MeshRelativeRotation=(Pitch=0,Yaw=90.000000,Roll=0)`. The older static row used the opposite yaw; carrying that value onto the skeletal pass makes the hero walk backward.
- Correct roll-direction fix: the runtime roll burst remains actor-forward, but the in-place roll animation must use the corrected roll bake above. `Roll` and uncorrected `Roll_RM` both read as a backward flip after source root motion is stripped.
- Live promotion is opt-in: set `T66_ARTHUR_QUADRETRO_PROMOTE_LIVE=1` when running `Tools/import_arthur_quadretro_animation_to_unreal.py`. Without that flag, the importer should only refresh the temporary validation row.
- The verifier must check the skeletal material instance texture parameters (`EmissiveTexture`, `BaseColorTexture`, and `DiffuseColorMap`), not only the row&#x27;s `PixelatedTextureAssetPath`. The runtime skeletal material rebuild reads the material texture parameters.

Keep this sequence for future live hero replacements:

1. Trace the live row and source mesh from normal selection.
2. Rig the exact visible source mesh in Blender when no production skeleton exists.
3. Render and inspect front, side, three-quarter, and gameplay contact sheets for all required actions.
4. Import to a temporary row first.
5. Capture the temporary row in gameplay through the real hero pawn path.
6. Promote the live row only after the temporary row keeps the correct look, scale, and attachments.
7. Verify texture atlas compatibility, promoted-row yaw, and material texture parameters before staging.
8. Reload data tables, build, stage standalone, verify the shortcut target, and smoke boot the staged executable when playable content changes.

## Runtime Boundary

Current T66 runtime supports character visual rows with looping/walk, alert/idle, run/jump, and one-shot roll animation slots. A richer state machine still needs a separate animation-set table or AnimBP later, but the accepted QuadRetro Arthur pass now populates all four required gameplay clips through `CharacterVisuals.csv` / `DT_CharacterVisuals`.
</code></pre>

### Model Generation/Rigging and Animation/04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md
<pre><code># Mob Vertex Animation Pipeline Instructions

## Goal

Create professional mob animation sources that bake to vertex animation textures for runtime, so T66 can display many enemies at once without paying per-enemy skeletal animation cost.

This is not the humanoid hero pipeline. Use armatures, Rigify, Rigodotify, shape keys, lattices, constraints, or simulations inside Blender when they help author the motion, but the runtime deliverable for normal mobs is a static mesh driven by vertex baked animation data.

## When To Use This

Use this pipeline for:

- regular enemies
- difficulty batch mob animation passes
- blobs, insects, spiders, bats, quadrupeds, statues, swarms, wisps, and other non-humanoid silhouettes
- any mob that needs many on-screen copies

Do not use this pipeline as a shortcut for playable heroes or humanoid companions that need live skeletal gameplay slots. For those, use `02_HERO_ANIMATION_PIPELINE_INSTRUCTIONS.md`.

## Required Read Order

Before editing or generating mob animation assets, read:

1. `RIGGING_ANIMATION_AGENTS.md`
2. `00_RIGGING_ANIMATION_ROUTING_INSTRUCTIONS.md`
3. `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`
4. this file
5. the batch file for the requested difficulty, such as `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md`
6. `../Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md` before any Unreal import or playable wiring
7. the relevant Gameplay instruction files when runtime behavior or stage data is touched

Also read any `pending_issues_*.md` file in every folder you touch.

## Core Contract

The source-of-truth asset is an editable Blender scene. The runtime asset is a static-mesh visual with vertex baked animation, not a live skeletal mesh component.

The production contract is:

- one approved visible mob mesh/source per `EnemyID`
- one Blender source scene per mob or per tightly related batch
- authored animation clips that match that enemy&#x27;s intended behavior
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
- `UnrealEditor-Cmd.exe -run=pythonscript` can inspect assets, but FBX import through this path can fail with a Slate application assertion. Use full `UnrealEditor.exe -ExecutePythonScript=...` for FBX import/bake work.
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
- any touched runtime folder&#x27;s `pending_issues_*.md` when an out-of-scope issue is discovered

## Prohibited Shortcuts

- Do not wire bake-only skeletal assets into live mob rows as if they are final runtime visuals.
- Do not reuse hero animation clips on non-humanoid mobs unless the body plan and behavior actually match.
- Do not mark a mob done because it has any motion. The motion must fit the enemy&#x27;s gameplay read.
- Do not use an exploratory source asset for final content without documenting visual and runtime acceptance.
- Do not skip multi-angle Blender QA.
- Do not hardcode VAT asset paths or clip timings in C++.
</code></pre>

### Model Generation/Rigging and Animation/03_FINDINGS_AND_LIMITATIONS_REFERENCE.md
<pre><code># Findings And Limitations

## 2026-05-13 Setup

- Blender 5.1.1 is installed at `C:\Program Files\Blender Foundation\Blender 5.1\blender.exe`.
- Rigify enabled successfully in Blender 5.1.1.
- Rigodotify cloned from `https://github.com/catprisbrey/Rigodotify.git`.
- Rigodotify commit installed locally: `4ee6e34b1580a0fac07e31c6cfed30addad182aa`.
- Rigodotify install failed when packaged with `__init__.py` at zip root.
- Rigodotify install succeeded after repackaging with a top-level `Rigodotify/` prefix.
- Rigodotify enabled successfully in Blender 5.1.1.
- Blender reported a Rigodotify panel naming warning: `GodotMecanim_Panel` does not contain `_PT_` with prefix and suffix. This did not block enablement.
- `Tools/setup_rigging_animation_infrastructure.ps1 -SkipBlenderInstall` was run successfully after setup; it confirmed Rigodotify was already up to date and skipped the existing Quaternius extracts.

## Quaternius Packages

Installed from Downloads:

- `Universal Animation Library[Standard].zip`
- `Universal Animation Library 2[Standard].zip`
- `Universal Base Characters[Standard].zip`
- `Universal Animation Library[Source].zip`
- `Universal Animation Library 2[Source].zip`
- `Universal Base Characters[Source].zip`

Initial Standard extracted asset inventory:

- 1 `.blend`
- 3 `.glb`
- 18 `.gltf`
- 29 `.fbx`
- 18 `.bin`
- 55 `.png`
- 5 `.txt`

Source extracted asset inventory:

- 23 `.blend`
- 131 `.fbx`
- 92 `.gltf`
- 3 `.glb`
- 93 `.bin`
- 101 `.png`
- 5 `.txt`
- 3 `.zip`

Current source baseline:

- `Universal Animation Library Source\UAL1.blend` is the preferred first source for Arthur idle, walk, jump, and roll because it contains the simple base locomotion clips.
- `Universal Animation Library 2 Source\UAL2.blend` is useful for later parkour/combat variants.
- `Universal Base Characters Source\Base Characters\Superhero_Male_FullBody.blend` and related base-character scenes are available for editable body/reference work.
- Standard packages remain useful for engine import checks and fallback references, but Source `.blend` files are now the baseline for source-of-truth animation editing.

## Unreal GLTF Import Note

The Unreal setup screenshots and local `Unreal-Engine-README.txt` solve the Unreal import/retargeting side, not the Blender source-editing side.

Use this interpretation:

- For Unreal import, Quaternius recommends GLTF/GLB because rigged FBX exported from Blender can import at the wrong scale and break retargeting.
- Import animations enabled, 30 kHz bone-animation baking, and snap-to-closest-frame settings are appropriate for bringing the library into Unreal.
- Source `.blend` files are now available and should be used for original rig/action editing.
- The Standard GLB files still import into Blender with usable action lists, so they remain valid for import-path experiments and for learning the skeleton/action contract.

## Blender Inventory Probe

`Tools/inspect_animation_assets.py` was run through Blender 5.1.1 against the key Standard and Source package files.

Results:

- `UAL1_Standard.glb`: imports successfully, 1 armature, 45 actions.
- `UAL2_Standard.glb`: imports successfully, 1 armature, 43 actions.
- `Mannequin_F.blend`: opens successfully, 2 armatures, no stored actions in the inspected file.
- `Superhero_Male_FullBody.gltf`: imports successfully, 1 armature, no actions.
- `Superhero_Female_FullBody.gltf`: imports successfully, 1 armature, no actions.
- `UAL1.blend`: opens successfully, 1 armature, 127 actions.
- `UAL2.blend`: opens successfully, 1 armature, 135 actions.

Tooling notes:

- Blender 5.1 action data no longer exposes `action.fcurves` in the same way older scripts expect. The inspection helper now treats missing `fcurves` as zero instead of failing.
- Opening `Mannequin_F.blend` in background mode skipped its embedded `rig_ui.py` because scripts are disabled by default. This is expected for safe background inspection.
- Importing the Universal Base Characters GLTF files emitted missing-image warnings for `T_Hair_1_Normal_png.png` and `T_Eye_Normal_png.png`. The mesh and armature still imported. Treat this as a material-path issue to resolve before using the base characters for polished reference renders.

## Working Assumption

Use the Source packages for Arthur and future source-of-truth hero animation work. Keep the Standard packages for import-path comparison and regression checks.

## 2026-05-14 Arthur Pilot Visual QA

What was visually tested:

- The Arthur pilot `.blend` generated rendered frame previews for idle, walk, jump, and roll.
- `Tools/make_preview_contact_sheets.py` built `Runs/Arthur_Animation_Pilot_20260514/PreviewFrames/Arthur_All_Actions_Contact_Sheet.png`.
- The contact sheet was inspected before Unreal import.
- A separate object-delta retarget test was visually rejected because it caused severe target mesh deformation. That output should not be used as the Arthur baseline.

What changed based on visual findings:

- The rejected object-delta transfer path was abandoned.
- The accepted imported pass uses the local-basis retarget path because it preserved Arthur&#x27;s proportions and produced readable idle, walk, jump, and roll silhouettes.
- The roll source action was switched to `Roll_RM` for the final pilot export.

Current quality status:

- The roll reads as an intentional combat roll in the current preview camera.
- The jump and walk read as functional first-pass locomotion clips.
- The idle is usable as a prototype loop but still reads stylized and needs a polish pass before it becomes a hero-quality baseline.
- The Arthur pilot is playable and verified in Unreal, but it is not a final production-accepted animation set because it has not yet gone through the required front/side/three-quarter correction loop.

Process gap to close before scaling:

- `Tools/render_arthur_action_previews.py` currently renders one preview camera. Before treating a future humanoid set as production accepted, extend or replace it with a multi-view humanoid preview renderer that outputs front, side, three-quarter, and gameplay-camera contact sheets plus written QA notes.

## 2026-05-14 Arthur Runtime Row Rollback

Problem:

- The first Arthur pilot import wired the skeletal animation experiment directly into the live `Hero_1_Chad` visual row.
- `Hero_1_Chad` is the row selected by normal Royal Chad hero selection.
- The live row previously pointed to the QuadRetro static mesh `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/SM_Hero_1_Chad_QuadRetro.SM_Hero_1_Chad_QuadRetro` with scale `(X=1.011123,Y=1.011123,Z=1.011123)`.
- The pilot row pointed it instead at `/Game/Characters/Heroes/Hero_1/Chad/SK_Hero_1_Chad.SK_Hero_1_Chad` with scale `(X=1,Y=1,Z=1)`.
- That skeletal mesh was not proven to be the same playable hero-selection visual. In gameplay it produced an old headless small model plus an oversized model/head around the map.

Fix:

- Restore `Hero_1_Chad` to the QuadRetro static mesh row.
- Keep the skeletal experiment isolated until the correct QuadRetro replacement is validated.
- After the accepted QuadRetro pass was promoted, delete `Hero_1_Chad_AnimPilot`, the old root-level `SK_Hero_1_Chad` assets, and the pilot-specific tools/runs so the failed path cannot be selected or rerun by accident.

Rule:

- Never wire a skeletal pilot into a live hero-selection row until the exact selected runtime mesh, scale, attachments, camera behavior, animation response, and staged standalone output are visually verified in gameplay.

## 2026-05-14 Royal Chad QuadRetro UAL Retarget Pass

What was proven before replacing live Arthur:

- Normal Royal Chad/Arthur selection resolves to `Hero_1_Chad`.
- The correct live visual source was the QuadRetro static mesh `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/SM_Hero_1_Chad_QuadRetro.SM_Hero_1_Chad_QuadRetro` and normalized Royal Chad texture.
- The correct source model was found at `Model Generation/Runs/Pixal3D/HeroArthur01/Post/QuadRetro/arthur_royal_chad/Models/arthur_royal_chad_QuadRetro.glb`.
- That source is static-only: one mesh, no armature, no actions, and no usable vertex groups.
- The old root-level `SK_Hero_1_Chad` pilot was not the selected Royal Chad visual and previously caused the old headless/giant-model failure when wired live.

What worked:

- `Tools/create_arthur_quadretro_ual_animation_source.py` imports the exact QuadRetro GLB, imports `External/Quaternius/Universal Animation Library Source/UAL1.blend`, builds a Rigodotify-style deform skeleton around the visible mesh, retargets UAL source motion, bakes the former live row scale `1.011123`, and exports the skeletal mesh plus action FBXs.
- The accepted UAL mapping is `Idle_Loop` -&gt; idle, `Walk_Formal_Loop` -&gt; walk, `Jump_Start` + `Jump_Loop` + `Jump_Land` -&gt; jump, and `Roll_RM` -&gt; roll.
- The accepted roll bake strips source root motion because T66 supplies actor-forward travel at runtime, then mirrors the roll target&#x27;s local-X sagittal rotation component so the in-place clip tumbles forward instead of reading as a backflip.
- Root XY is kept in-place because T66 movement and roll direction are actor-driven, including the existing one-button roll path.
- `Tools/render_arthur_action_previews.py` supports front, side, three-quarter, and gameplay-camera views with an action-prefix filter and target armature selector.
- `Tools/make_preview_contact_sheets.py` builds per-view and all-view contact sheets from the multi-view manifest.
- `Tools/import_arthur_quadretro_animation_to_unreal.py` imports into `/Game/Characters/Heroes/Hero_1/Chad/QuadRetroUALQA`, creates a skeletal-safe unlit material instance, writes a temporary `Hero_1_Chad_QuadRetroUALQA` row, and reloads `DT_CharacterVisuals`.
- `Tools/verify_arthur_quadretro_animation_in_unreal.py` validates the skeletal mesh, skeleton, material parent, texture, four AnimSequences, temporary row, and promoted live row when `T66_ARTHUR_QUADRETRO_EXPECT_LIVE_PROMOTED=1`.
- `Scripts/RunRiggingAnimationToolAndExit.py` is the stable Unreal Python wrapper for tools under paths with spaces.

What did not work:

- The manual/procedural `QuadRetroAnimQA` pass was readable but not professional. It did not follow the intended UAL/Rigodotify-quality path and was deleted after the UAL retarget passed.
- Running the import through `UnrealEditor-Cmd.exe -run=pythonscript` hit a Slate application assertion in this path. The working import path is full editor `UnrealEditor.exe -ExecutePythonScript=...` with `T66_ARTHUR_QUADRETRO_QUIT_EDITOR=1`.
- Passing tool paths through 8.3 short names such as `MODELG~1` / `RIGGIN~1` can make Unreal&#x27;s Python runner treat the path as Python text and fail with `SyntaxError`. Use the wrapper instead.
- Blender 5.1 layered action data can make old scripts report zero `action.fcurves` even when the action is valid. The inspection helper now counts nested/layered fcurves too.
- Direct source first-frame retargeting caused source-pose offsets. The accepted script neutralizes each source action against its first frame.
- Early weighting caused prop/cape/robe deformation artifacts. The accepted script uses spatial weighting, action-specific damping, and hidden source-armature handling to keep attachments stable.
- Initial front and three-quarter preview cameras were back-facing. They were corrected before accepting the contact sheets.
- A later reimport accidentally assigned the normalized static-mesh texture atlas to the GLB-derived skeletal material. That made the in-game hero texture appear scrambled even though the row still named a Royal Chad texture. The fixed path uses the original GLB-layout texture `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/RoyalChad_QuadRetro/Textures/RoyalChad_QuadRetro_Pixelated_512.RoyalChad_QuadRetro_Pixelated_512` for the row and material instance.
- The same reimport carried over the old static row yaw and made Arthur walk backward. The promoted skeletal row now uses `MeshRelativeRotation=(Pitch=0,Yaw=90.000000,Roll=0)`.
- Switching only from UAL `Roll` to `Roll_RM` did not fix the roll direction once root motion was stripped; the baked local pose still opened backward from the side view. The accepted fix is the target-roll local-X sagittal mirror in `Tools/create_arthur_quadretro_ual_animation_source.py`, verified against side and gameplay-camera contact sheets before Unreal export.
- The importer module docstring must stay raw (`r&quot;&quot;&quot;...&quot;&quot;&quot;`) because the usage examples contain Windows paths such as `C:\UE\...`; a normal docstring can fail with a Python `unicodeescape` syntax error before the import runs.
- One full-editor reimport wrote the Unreal import report and saved the assets, then the editor exited with `-1073741819` during shutdown. Treat the report and the commandlet verifier as the source of truth before repeating the same full-editor import; do not keep rerunning imports just to chase a shutdown-only exit code.

Accepted evidence paths:

- Blender source: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_QuadRetro_UAL_Retarget.blend`
- Blender manifest: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/arthur_quadretro_ual_retarget_manifest.json`
- Contact sheets: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_All_Actions_{front,side,three_quarter,gameplay}_Contact_Sheet.png`
- Roll-specific proof sheets: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/AM_Hero_1_Chad_QuadRetroUALQA_Roll_side_contact_sheet.png` and `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/AM_Hero_1_Chad_QuadRetroUALQA_Roll_gameplay_contact_sheet.png`
- All-view sheet: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_All_Actions_All_Views_Contact_Sheet.png`
- Unreal import report: `Saved/ArthurQuadRetroAnimationImportReport.json`
- Unreal verification report: `Saved/ArthurQuadRetroAnimationVerifyReport.json`
- Old-pass cleanup report: `Saved/ArthurQuadRetroOldAnimQACleanupReport.json`
- Temporary in-game screenshot: `Saved/StandaloneLogs/ArthurQuadRetroUALTempHeroQA.png`
- Live staged in-game screenshot: `Saved/StandaloneLogs/ArthurQuadRetroUALLiveHeroQA.png`
- Live staged widget dump: `Saved/StandaloneLogs/ArthurQuadRetroUALLiveHeroQA.json`
- Live staged log evidence: `Saved/StagedBuilds/Windows/T66/Saved/Logs/T66.log` lines showing `VisualID=Hero_1_Chad`, `ResolvedRow=Hero_1_Chad`, `SK_Hero_1_Chad_QuadRetroUALQA`, and `AM_Hero_1_Chad_QuadRetroUALQA_Idle`.

Texture/forward reimport fix evidence:

- Unreal verification report: `Saved/ArthurQuadRetroAnimationVerifyReport.json` with `ok=true`, zero errors, live `Hero_1_Chad` row pointing at `SK_Hero_1_Chad_QuadRetroUALQA`, `PixelatedTextureAssetPath` set to `RoyalChad_QuadRetro_Pixelated_512`, `MeshRelativeRotation=(Pitch=0,Yaw=90.000000,Roll=0)`, and material texture params `EmissiveTexture`, `BaseColorTexture`, and `DiffuseColorMap` all set to the same original GLB-layout texture.
- Staged smoke screenshot: `Saved/StandaloneLogs/ArthurQuadRetroTextureForwardFix_HeroQA.png`.
- Staged smoke log: `Saved/StandaloneLogs/ArthurQuadRetroTextureForwardFix_HeroQA.log` with `HeroBase::InitializeHero ... VisualID=Hero_1_Chad` and no fatal, material compile, or default-material fallback lines.
- Staged standalone refresh: `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development` exited successfully and updated both `T66 Standalone.lnk` shortcuts to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

Roll forward fix evidence:

- Unreal import report: `Saved/ArthurQuadRetroAnimationImportReport.json` with `source_dir` set to `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Exports` and `promoted_live_row=true`.
- Unreal verification report: `Saved/ArthurQuadRetroAnimationVerifyReport.json` with `ok=true`, zero errors, live `Hero_1_Chad` row pointing at `AM_Hero_1_Chad_QuadRetroUALQA_Roll`, and roll play length `1.4666666984558105`.
- Staged smoke screenshots/logs: `Saved/StandaloneLogs/ArthurRollForwardFix_HeroRollQA.png`, `Saved/StandaloneLogs/ArthurRollForwardFix_HeroRollQA.log`, `Saved/StandaloneLogs/ArthurRollForwardFix_HeroRollQA_MidRoll.png`, and `Saved/StandaloneLogs/ArthurRollForwardFix_HeroRollQA_MidRoll.log`.

Runtime note:

- The current row uses `LoopingAnimation` for walk, `AlertAnimation` for idle, `RunAnimation` for jump, and `RollAnimation` for the one-shot roll. The existing one-button roll behavior remains owned by gameplay movement and is still bound to Left Shift and Gamepad Face Button Right.

Cleanup note:

- Deleted `Hero_1_Chad_AnimPilot` from `Content/Data/CharacterVisuals.csv`.
- Deleted root-level old pilot assets under `/Game/Characters/Heroes/Hero_1/Chad`: `SK_Hero_1_Chad`, `SK_Hero_1_Chad_Skeleton`, `AM_Hero_1_Chad_{Idle,Walk,Jump,Roll}`, `AM_Hero_1_Chad_RigIdleV2`, and the leftover root `Image_0*` / `Material_0*` import artifacts.
- Deleted obsolete pilot tools: `create_arthur_animation_pilot.py`, `import_arthur_animation_pilot_to_unreal.py`, and `verify_arthur_animation_pilot_in_unreal.py`.
- Deleted obsolete pilot run folders: `Runs/Arthur_Animation_Pilot_20260514` and `Runs/Arthur_Animation_Pilot_ObjectDelta_20260514`.
- Deleted the rejected manual/procedural `QuadRetroAnimQA` Unreal assets, run folder, and generator tool after the UAL retarget replaced it.

## 2026-05-14 Enemy VAT Pipeline Exploration

What was inspected:

- `Content/Data/Enemies.csv`
- `Content/Data/Stages.csv`
- `Content/Data/CharacterVisuals.csv`
- `Source/T66/Gameplay/T66EnemyBase.h`
- `Source/T66/Gameplay/T66EnemyBase.cpp`
- `Source/T66/Gameplay/Enemies/T66EnemyFamilyResolver.cpp`
- `Source/T66/Gameplay/Enemies/T66FlyingEnemy.cpp`
- `Source/T66/Gameplay/Enemies/T66RushEnemy.cpp`
- `Source/T66/Data/T66DataTypes.h`
- `Source/T66/Gameplay/Enemies/pending_issues_Enemies.md`
- `Model Generation/Production/Roster_v1/AgentA/Report.md`
- `Model Generation/Production/Roster_v1/AgentB/Report.md`
- UE 5.7 local `AnimToTexture` plugin headers and source references

Current Difficulty 1 / Easy mob set:

- `Slime`
- `BoneWalker`
- `RatPack`
- `CaveBat`
- `HexSlinger`
- `TombSpider`
- `StoneSentinel`
- `MimicLure`
- `BoneConjurer`
- `CryptWraith`

Runtime visual finding:

- Easy mob `CharacterVisuals.csv` rows are static-only today.
- Their live static meshes resolve under `/Game/Characters/Mobs/&lt;EnemyID&gt;/SM_&lt;EnemyID&gt;`.
- Their skeletal mesh and animation slots are empty.
- `AT66EnemyBase::ConfigureAsMob(...)` applies the row through `UT66CharacterVisualSubsystem::ApplyCharacterVisual(...)`.
- `FT66CharacterVisualRow` has skeletal animation slots, but no dedicated vertex animation texture fields.

Source finding:

- Source GLBs for all ten Easy mobs exist under `Model Generation/Production/Roster_v1`.
- Agent A owns `Slime`, `RatPack`, `HexSlinger`, `StoneSentinel`, and `BoneConjurer`.
- Agent B owns `BoneWalker`, `CaveBat`, `TombSpider`, `MimicLure`, and `CryptWraith`.
- Promote Pixal3D GLB outputs only after visual and runtime acceptance.

Runtime behavior finding:

- The implemented enemy families currently resolve to `Melee`, `Rush`, `Ranged`, and `Flying`.
- Data archetypes such as `Exploder`, `Turret`, `Necromancer`, and `Stutterer` exist in the roster data, but matching runtime subclasses are currently missing.
- Do not claim that VAT animation work implements those behaviors. It can provide visual cues only until gameplay classes are added.

VAT tooling finding:

- UE 5.7 has an Experimental `AnimToTexture` plugin at `C:\Program Files\Epic Games\UE_5.7\Engine\Plugins\Experimental\AnimToTexture`.
- The plugin exposes `UAnimToTextureBPLibrary::AnimationToTexture(UAnimToTextureDataAsset*)`.
- The local headers expose vertex and bone texture modes plus 8-bit and 16-bit precision choices.
- This is the likely Unreal-side bake path for regular mob VAT work, but T66 still needs a project-owned runtime data seam and material/application path before production promotion.

Process result:

- Added `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md` as the general regular-enemy vertex baked animation process.
- Added `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md` as the first Difficulty 1 batch plan with source paths, live visual paths, scale values, behavior notes, and runtime caveats.

Open implementation gap:

- T66 needs a dedicated mob VAT data table/subsystem/material application path before any Easy mob VAT asset should be wired live.
- Do not reuse `CharacterVisuals.csv` skeletal animation slots for VAT clip state.
- Do not promote bake-only skeletal sources as live mob visuals.

## 2026-05-14 Easy Mob VAT Implementation

What was built:

- `Tools/create_easy_mob_vat_sources.py` builds the Easy batch Blender scene, behavior-specific rigs/actions, FBX exports, preview frames, and manifest.
- `Tools/make_easy_mob_contact_sheets.py` builds per-mob and all-mob contact sheets.
- `Tools/import_easy_mob_vat_to_unreal.py` imports bake-only skeletal sources, runs AnimToTexture, creates VAT static meshes, textures, material instances, writes `Content/Data/MobVertexAnimations.csv`, and reloads `/Game/Data/DT_MobVertexAnimations`.
- `Tools/verify_easy_mob_vat_in_unreal.py` verifies all ten rows, assets, frame ranges, material parameters, texture dimensions, fallback `CharacterVisuals.csv` preservation, and data table row names.
- Runtime support was added through `FT66MobVertexAnimationRow`, `UT66CharacterVisualSubsystem`, and `AT66EnemyBase`.

Output evidence paths:

- Source inspection: `Saved/EasyMobSourceInspection_20260514.json`
- Blender source: `Runs/Easy_Mob_VAT_20260514/Easy_Mob_VAT_Source.blend`
- Blender manifest: `Runs/Easy_Mob_VAT_20260514/easy_mob_vat_manifest.json`
- Contact sheets: `Runs/Easy_Mob_VAT_20260514/PreviewFrames/*_AllClips_AllViews_Contact_Sheet.png`
- Contact sheet index: `Runs/Easy_Mob_VAT_20260514/PreviewFrames/Easy_Mobs_AllClips_AllViews_Index_Contact_Sheet.png`
- Unreal import report: `Saved/EasyMobVATImportReport.json`
- Unreal verification report: `Saved/EasyMobVATVerifyReport.json`
- Staged gameplay smoke log: `Saved/StandaloneLogs/EasyMobVAT_GameplaySmoke_WithAudio.log`
- Staged gameplay smoke screenshot: `Saved/StandaloneLogs/EasyMobVAT_GameplaySmoke_WithAudio.png`

What worked:

- All ten Easy source GLBs import into Blender as single-mesh static sources.
- Procedural Blender rigs can produce first production-direction clips for blobs, flying mobs, humanoid enemies, swarms, spiders, casters, sentinels, mimics, conjurers, and wraiths while preserving vertex topology for VAT.
- AnimToTexture vertex mode produced nonzero position/normal textures for all ten mobs.
- Material instance bounds written by `UpdateMaterialInstanceFromDataAsset` are valid and can be copied into the runtime CSV.
- `DT_MobVertexAnimations` loads with exactly the ten Easy mob rows, and each row preserves the original fallback texture and scale from `CharacterVisuals.csv`.

What failed or needed correction:

- `UnrealEditor-Cmd.exe -run=pythonscript` hit a Slate application assertion during automated FBX import. Full `UnrealEditor.exe -ExecutePythonScript=...` with `Scripts/RunRiggingAnimationToolAndExit.py` is the working import path.
- Windows paths inside Python docstrings caused `unicodeescape` failures when `\U` appeared. Use raw docstrings or forward slashes in usage blocks.
- Backslash script paths can be mangled when passed to Unreal&#x27;s Python runner. Use forward-slash paths.
- The first VAT bake used UV channel `1`, which conflicted with lightmap UVs. The accepted path uses UV channel `2` and a UV2 material master.
- Direct Python reads of AnimToTexture `FVector3f` data-asset bounds returned zeros even when material parameters and textures were valid. The importer and verifier now treat material `MinBBox`/`SizeBBox` as authoritative after `UpdateMaterialInstanceFromDataAsset`.
- A temporary material deletion probe triggered a `ForceDeleteObject` ensure when deleting a loaded material in the same commandlet. Avoid same-process deletion of loaded assets.
- Initial bat and spider preview motion was too subtle from gameplay camera. The generator was adjusted to strengthen bat wing and spider leg deformation before export.
- The first generated VAT material custom node called `TransformLocalVectorToWorld(local_delta)`. That produced cooked material failures and default-material fallback in staged standalone. The fixed material calls `TransformLocalVectorToWorld(Parameters, local_delta)`, and the importer now recreates the generated master material when custom HLSL changes.
- A smoke run before the final full stage exposed missing OGG/Vorbis runtime DLL handling. The final staged standalone launched without `-nosound`; audio initialized, `Lib vorbis DLL was dynamically loaded`, and `OGG` registered.
- A code-only restage is not enough after VAT material/content fixes. The accepted path was full cook/stage after material repair, then code-only restage only for the later QA-spawn-position adjustment.

Runtime and staged evidence:

- `Saved/EasyMobVATVerifyReport.json` has `data_table_row_count=10`, ten mob rows, zero errors, and only the accepted Python-bound warnings documented in the batch file.
- The staged QA capture spawned all ten Easy mobs with representative clip overrides: idle, move, attack cue, hit react, and death all appeared in one gameplay scene.
- The final staged log had no matches for VAT material compile failures, invalid shader maps, default-material fallback, fatal errors, critical errors, assertion failures, `libogg`, or `libvorbis`.
- Both `C:\UE\T66\T66 Standalone.lnk` and the pinned taskbar `T66 Standalone.lnk` target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

Remaining caveats:

- The Easy VAT rows are live-runtime QA assets, not final release art until final visual acceptance.
- Gameplay archetypes beyond `Melee`, `Rush`, `Ranged`, and `Flying` remain visual-only until the missing behavior classes are implemented.
- Current playback uses one dynamic material instance per enemy. This is suitable for first live QA, but crowd-scale optimization should move toward instanced playback once the enemy rendering path is ready.
</code></pre>

## Additional Rigging Folder Inventory
| Path | Role |
| --- | --- |
| Model Generation/Rigging and Animation/00_RIGGING_ANIMATION_ROUTING_INSTRUCTIONS.md | rigging/animation process or reference doc |
| Model Generation/Rigging and Animation/01_TOOL_SETUP_INSTRUCTIONS.md | rigging/animation process or reference doc |
| Model Generation/Rigging and Animation/02_HERO_ANIMATION_PIPELINE_INSTRUCTIONS.md | rigging/animation process or reference doc |
| Model Generation/Rigging and Animation/03_FINDINGS_AND_LIMITATIONS_REFERENCE.md | rigging/animation process or reference doc |
| Model Generation/Rigging and Animation/04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md | rigging/animation process or reference doc |
| Model Generation/Rigging and Animation/05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md | rigging/animation process or reference doc |
| Model Generation/Rigging and Animation/External/ | third-party local sources, including Quaternius packages |
| Model Generation/Rigging and Animation/README.md | rigging/animation process or reference doc |
| Model Generation/Rigging and Animation/RIGGING_ANIMATION_AGENTS.md | rigging/animation process or reference doc |
| Model Generation/Rigging and Animation/Runs/ | generated run outputs and QA evidence; not runtime dependencies |
| Model Generation/Rigging and Animation/Tools/ | reusable Python/PowerShell/Blender/Unreal automation for rigging, import, verification, and contact sheets |
| Model Generation/Rigging and Animation/Tools/create_arthur_quadretro_ual_animation_source.py | tooling file in current rigging/animation workflow |
| Model Generation/Rigging and Animation/Tools/create_easy_mob_vat_sources.py | tooling file in current rigging/animation workflow |
| Model Generation/Rigging and Animation/Tools/import_arthur_quadretro_animation_to_unreal.py | tooling file in current rigging/animation workflow |
| Model Generation/Rigging and Animation/Tools/import_easy_mob_vat_to_unreal.py | tooling file in current rigging/animation workflow |
| Model Generation/Rigging and Animation/Tools/inspect_animation_assets.py | tooling file in current rigging/animation workflow |
| Model Generation/Rigging and Animation/Tools/make_easy_mob_contact_sheets.py | tooling file in current rigging/animation workflow |
| Model Generation/Rigging and Animation/Tools/make_preview_contact_sheets.py | tooling file in current rigging/animation workflow |
| Model Generation/Rigging and Animation/Tools/render_arthur_action_previews.py | tooling file in current rigging/animation workflow |
| Model Generation/Rigging and Animation/Tools/setup_rigging_animation_infrastructure.ps1 | tooling file in current rigging/animation workflow |
| Model Generation/Rigging and Animation/Tools/verify_arthur_quadretro_animation_in_unreal.py | tooling file in current rigging/animation workflow |
| Model Generation/Rigging and Animation/Tools/verify_easy_mob_vat_in_unreal.py | tooling file in current rigging/animation workflow |

## B. Environment Specs

### Unreal Engine / Project
- `T66.uproject` EngineAssociation: `5.7`.
- Unreal commandlet metadata probe: `5.7.1-48512491+++UE5+Release-5.7` / release `5.7.1`.
- Current project version in `Config/DefaultGame.ini`: `alpha 0.6`.
- Enabled plugins relevant to this topic in `T66.uproject`: `PythonScriptPlugin`, `EditorScriptingUtilities`, `AnimToTexture`, `ModelingToolsEditorMode`; Steam/Electra/procedural plugins are enabled but not rigging-specific.
- Primary packaged target in repo evidence: Windows standalone (`Saved/StagedBuilds/Windows/...`) plus Windows target settings in `Config/DefaultEngine.ini`.
- Runtime Steam Deck profile exists in `Config/DefaultGame.ini` and `UT66RuntimePlatformSubsystem`; repo setting `SteamDeckDefaultFrameRateLimit=60.000000`.
- Target platform modules present in the UE probe included Android, iOS, Linux, Mac, TVOS, and Windows, but this report treats Windows/Steam Deck runtime profile as the repo-confirmed target surface. No current packaged Linux/Steam Deck build artifact was verified in this pass.

### FPS / Frame Pacing Targets Found In Repo
- `Config/DefaultGame.ini`: `SteamDeckDefaultFrameRateLimit=60.000000`.
- `Source/T66/Core/T66RuntimePlatformSubsystem.h/.cpp`: exposes `SteamDeckDefaultFrameRateLimit` and `GetDefaultFrameRateLimit()`.
- `Source/T66/Core/T66PlayerSettingsSubsystem.cpp`: uses `GUS->SetFrameRateLimit(60.0f)` as a conservative fallback cap.
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_Graphics.cpp`: user-facing FPS cap UI reads/writes `UGameUserSettings::GetFrameRateLimit()` / `SetFrameRateLimit()`.
- `Config/DefaultEngine.ini`: desktop/scalable graphics target and RHI settings; no global `t.MaxFPS` or smooth-frame-rate cap found in repo search.
- No repo-confirmed current FPS target beyond the 60 FPS Steam Deck/default cap was found.

### Blender
- Executable: `C:/Program Files/Blender Foundation/Blender 5.1/blender.exe`.
- Version: `5.1.1` (hash `b70da489d7f4`, built 2026-04-14 01:37:22).
- Blender Python: `3.13.9 [MSC v.1944 64 bit (AMD64)]`.
- Blender Pillow: not importable (`No module named 'PIL'`).
- Add-on probe warning: Rigodotify emits the known Blender warning about `GodotMecanim_Panel` lacking `_PT_`.
| State | Module | Name | Version |
| --- | --- | --- | --- |
| enabled | io_anim_bvh | BioVision Motion Capture (BVH) format | 1.0.1 |
| enabled | bl_pkg | Blender Extensions | 0.0.1 |
| enabled | cycles | Cycles Render Engine | version unavailable |
| enabled | io_scene_fbx | FBX format | 5.15.0 |
| enabled | io_scene_gltf2 | glTF 2.0 format | 5.1.19 |
| enabled | bl_ext.user_default.mcp | MCP | 1.0.0 |
| enabled | pose_library | Pose Library | 2.0 |
| enabled | rigify | Rigify | 0.6.10 |
| enabled | Rigodotify | Rigodotify | 2.3.0 |
| enabled | io_curve_svg | Scalable Vector Graphics (SVG) format | version unavailable |
| enabled | io_mesh_uv_layout | UV Layout | 1.2.0 |
| installed disabled | hydra_storm | Hydra Storm render engine | 1.0.0 |
| installed disabled | ui_translate | Manage UI translations | 2.1.0 |
| installed disabled | node_wrangler | Node Wrangler | 4.1.0 |
| installed disabled | quad_remesher_1_4 | Quad Remesher 1.4 Bridge | 1.4.1 |
| installed disabled | retopoflow | Retopoflow 4 | 4.1.5 |
| installed disabled | viewport_vr_preview | VR Scene Inspection | 0.11.2 |

### Python / Pillow Contexts
| Context | Python | Pillow |
| --- | --- | --- |
| system | 3.13.6 (C:\Python313\python.exe) | 12.1.0 |
| workspace bundled runtime | Python 3.12.13 (C:\Users\DoPra\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe) | 12.2.0 |
| Unreal embedded Python | 3.11.8 via `UnrealEditor-Cmd.exe` | not importable: `No module named 'PIL'` |
| Blender embedded Python | 3.13.9 via Blender 5.1.1 | not importable: `No module named 'PIL'` |

### Standalone AI / Model Tooling Integrated Beyond Quaternius
- TRELLIS.2 RunPod tooling exists under `Model Generation/Instructions/01_TRELLIS_RUNPOD_SETUP_INSTRUCTIONS.md`, `Model Generation/Scripts/Core/Trellis/`, and `Model Generation/Tools/Trellis2/trellis_server.py`. It is a mesh-generation path, not a direct rigging/animation runtime path.
- Pixal3D RunPod tooling exists under `Model Generation/Pixal3D/` and `Model Generation/Instructions/07_PIXAL3D_RUNPOD_SETUP_INSTRUCTIONS.md`. It directly connects to rigging inputs for current evidence: Arthur's source GLB path in the hero pipeline is under `Model Generation/Runs/Pixal3D/...`, and Easy mob source GLBs are under `Model Generation/Production/Roster_v1/...` with Pixal3D logs.
- Quad Retro/Quad Remesher post-processing is part of the mesh preparation path before rigging. Quad Remesher is installed as a disabled Blender add-on and the Quad Retro docs/scripts expect it where needed.
- No OpenAI/Image API runtime integration was found in the rigging playback path. Image/model generation tooling is repo-local process tooling, not runtime animation code.

## C. Runtime Animation Code

### Summary
- No hero Animation Blueprint path was found in `Source/T66`; hero animation playback uses `USkeletalMeshComponent::PlayAnimation()` directly from cached `UAnimationAsset` pointers.
- Hero/companion/boss skeletal and static visual assignment goes through `UT66CharacterVisualSubsystem` and `CharacterVisuals.csv` / `DT_CharacterVisuals`.
- Easy mobs use `AT66EnemyBase` VAT playback first, driven by `DT_MobVertexAnimations`. If no enabled VAT row exists, enemies fall back to the static `CharacterVisuals` row.
- Bosses have a visual application path through `AT66BossBase` into `ApplyCharacterVisual`, but current boss rows are static-only, so no boss-specific animation path is active from data.
- Requested TODO/HACK/FIXME search: no TODO/HACK/FIXME comments were found in `T66EnemyBase`, `T66CharacterVisualSubsystem`, `T66BossBase`, or the data structs. `T66HeroBase.cpp` has one unrelated TODO about future barefoot cosmetics; `T66PlayerController_Input.cpp` has an unrelated hitbox overlay TODO.

### Source/T66/Gameplay/T66HeroBase.h
```cpp
  230: 	bool bIsPreviewMode = false;
  231: 
  232: 	int32 SafeZoneOverlapCount = 0;
  233: 
  234: 	UPROPERTY()
  235: 	TObjectPtr<UT66RunStateSubsystem> CachedRunState;
  236: 
  237: 	/** Cached idle/walk/jump/roll anims for the current hero visual. */
  238: 	UPROPERTY(Transient)
  239: 	TObjectPtr<UAnimationAsset> CachedIdleAnim = nullptr;
  240: 
  241: 	UPROPERTY(Transient)
  242: 	TObjectPtr<UAnimationAsset> CachedWalkAnim = nullptr;
  243: 
  244: 	UPROPERTY(Transient)
  245: 	TObjectPtr<UAnimationAsset> CachedJumpAnim = nullptr;
  246: 
  247: 	UPROPERTY(Transient)
  248: 	TObjectPtr<UAnimationAsset> CachedRollAnim = nullptr;
  249: 
  250: 	/** Last animation state so we only call PlayAnimation on change. */
  251: 	enum class EMovementAnimState : uint8 { Idle, Walk, Jump, Roll };
  252: 	EMovementAnimState LastMovementAnimState = EMovementAnimState::Idle;
  253: 	float RollAnimLockEndTimeSeconds = -1.f;
  254: 	FVector LastAnimSampleLocation = FVector::ZeroVector;
  255: 	bool bHasLastAnimSampleLocation = false;
  256: 	bool bLobbyDrivenVisualsApplied = false;
  257: 
  258: 	void PlayRollAnimation();
  259: 
  260: 	bool bVehicleMounted = false;
```

### Source/T66/Gameplay/T66HeroBase.cpp
```cpp
  545: 	if (USkeletalMeshComponent* Skel = GetMesh())
  546: 	{
  547: 		Skel->SetRelativeLocation(DefaultSkeletalMeshRelativeLocation + (bMounted ? VisualOffset : FVector::ZeroVector));
  548: 		Skel->SetRelativeRotation(DefaultSkeletalMeshRelativeRotation + (bMounted ? VisualRotation : FRotator::ZeroRotator));
  549: 		Skel->GlobalAnimRateScale = bMounted ? 0.f : 1.f;
  550: 		if (bMounted && CachedIdleAnim)
  551: 		{
  552: 			Skel->PlayAnimation(CachedIdleAnim, true);
  553: 		}

// ...

  720: 		const bool bHasMovementInput = HeroMovementComponent
  721: 			? HeroMovementComponent->HasMovementInput()
  722: 			: (GetLastMovementInputVector().SizeSquared() > 0.01f);
  723: 		const FVector ReplicatedVelocity = GetReplicatedMovement().LinearVelocity;
  724: 		const bool bHasReplicatedHorizontalVelocity =
  725: 			GetVelocity().SizeSquared2D() > FMath::Square(8.f)
  726: 			|| ReplicatedVelocity.SizeSquared2D() > FMath::Square(8.f);
  727: 		bool bHasRemoteLocationDelta = false;
  728: 		if (bHasLastAnimSampleLocation)
  729: 		{
  730: 			const float AnimTravelDeltaSq = FVector::DistSquared2D(GetActorLocation(), LastAnimSampleLocation);
  731: 			bHasRemoteLocationDelta = AnimTravelDeltaSq > FMath::Square(5.f);
  732: 		}
  733: 		LastAnimSampleLocation = GetActorLocation();
  734: 		bHasLastAnimSampleLocation = true;
  735: 		const bool bRollAnimationActive = CachedRollAnim
  736: 			&& GetWorld()
  737: 			&& static_cast<float>(GetWorld()->GetTimeSeconds()) < RollAnimLockEndTimeSeconds;
  738: 		if (GetMesh() && GetMesh()->IsVisible() && (CachedIdleAnim || CachedJumpAnim || CachedWalkAnim || CachedRollAnim))
  739: 		{
  740: 			EMovementAnimState NewState = EMovementAnimState::Idle;
  741: 			if (bRollAnimationActive)
  742: 			{
  743: 				NewState = EMovementAnimState::Roll;
  744: 			}
  745: 			else if (UCharacterMovementComponent* Movement = GetCharacterMovement(); Movement && Movement->IsFalling())
  746: 			{
  747: 				NewState = EMovementAnimState::Jump;
  748: 			}
  749: 			else if (bHasMovementInput || bHasReplicatedHorizontalVelocity || (!IsLocallyControlled() && bHasRemoteLocationDelta))
  750: 			{
  751: 				NewState = EMovementAnimState::Walk;
  752: 			}
  753: 
  754: 			if (LastMovementAnimState != NewState)
  755: 			{
  756: 				LastMovementAnimState = NewState;
  757: 				UAnimationAsset* ToPlay = nullptr;
  758: 				bool bLoopAnimation = true;
  759: 				switch (NewState)
  760: 				{
  761: 				case EMovementAnimState::Idle:
  762: 					ToPlay = CachedIdleAnim ? CachedIdleAnim : CachedWalkAnim;
  763: 					break;
  764: 				case EMovementAnimState::Jump:
  765: 					ToPlay = CachedJumpAnim ? CachedJumpAnim : CachedWalkAnim;
  766: 					break;
  767: 				case EMovementAnimState::Roll:
  768: 					ToPlay = CachedRollAnim;
  769: 					bLoopAnimation = false;
  770: 					break;
  771: 				case EMovementAnimState::Walk:
  772: 				default:
  773: 					ToPlay = CachedWalkAnim ? CachedWalkAnim : CachedIdleAnim;
  774: 					break;
  775: 				}
  776: 				if (ToPlay)
  777: 				{
  778: 					GetMesh()->PlayAnimation(ToPlay, bLoopAnimation);
  779: 				}
  780: 			}

// ...

 1068: 			if (bApplied && !bPreviewMode && GetMesh() && GetMesh()->GetSkeletalMeshAsset())
 1069: 			{
 1070: 				// Cache idle/walk/jump/roll anims and init hero speed params.
 1071: 				UAnimationAsset* WalkRaw = nullptr;
 1072: 				UAnimationAsset* JumpRaw = nullptr;
 1073: 				UAnimationAsset* IdleRaw = nullptr;
 1074: 				UAnimationAsset* RollRaw = nullptr;
 1075: 				Visuals->GetMovementAnimsForVisual(VisualID, WalkRaw, JumpRaw, IdleRaw, RollRaw);
 1076: 				CachedWalkAnim = WalkRaw;
 1077: 				CachedJumpAnim = JumpRaw;
 1078: 				CachedIdleAnim = IdleRaw;
 1079: 				CachedRollAnim = RollRaw;
 1080: 				RollAnimLockEndTimeSeconds = -1.f;
 1081: 				// Force first Tick to play idle (speed 0); if we left Idle we wouldn't call PlayAnimation.
 1082: 				LastMovementAnimState = EMovementAnimState::Walk;
 1083: 				if (HeroMovementComponent)

// ...

 1180: bool AT66HeroBase::RollForward()
 1181: {
 1182: 	if (HeroMovementComponent && HeroMovementComponent->TryRollForward())
 1183: 	{
 1184: 		PlayRollAnimation();
 1185: 		return true;
 1186: 	}
 1187: 
 1188: 	return false;
 1189: }
 1190: 
 1191: void AT66HeroBase::DashForward()
 1192: {
 1193: 	RollForward();
 1194: }
 1195: 
 1196: void AT66HeroBase::PlayRollAnimation()
 1197: {
 1198: 	if (!CachedRollAnim || !GetWorld() || !GetMesh() || !GetMesh()->IsVisible())
 1199: 	{
 1200: 		return;
 1201: 	}
 1202: 
 1203: 	const float DurationSeconds = FMath::Max(CachedRollAnim->GetPlayLength(), 0.1f);
 1204: 	RollAnimLockEndTimeSeconds = static_cast<float>(GetWorld()->GetTimeSeconds()) + DurationSeconds;
 1205: 	LastMovementAnimState = EMovementAnimState::Roll;
 1206: 	GetMesh()->PlayAnimation(CachedRollAnim, false);
 1207: }
```

### Source/T66/Core/T66CharacterVisualSubsystem.h
```cpp
    1: // Copyright Tribulation 66. All Rights Reserved.
    2: 
    3: #pragma once
    4: 
    5: #include "CoreMinimal.h"
    6: #include "Subsystems/GameInstanceSubsystem.h"
    7: #include "Data/T66DataTypes.h"
    8: #include "T66CharacterVisualSubsystem.generated.h"
    9: 
   10: class UDataTable;
   11: class USkeletalMeshComponent;
   12: class USkeletalMesh;
   13: class UStaticMesh;
   14: class UStaticMeshComponent;
   15: class UAnimationAsset;
   16: class USceneComponent;
   17: class USkeleton;
   18: class UMaterialInstanceDynamic;
   19: struct FStreamableHandle;
   20: 
   21: USTRUCT()
   22: struct FT66ResolvedCharacterVisual
   23: {
   24: 	GENERATED_BODY()
   25: 
   26: 	UPROPERTY()
   27: 	TObjectPtr<USkeletalMesh> Mesh = nullptr;
   28: 
   29: 	UPROPERTY()
   30: 	TObjectPtr<UStaticMesh> StaticMesh = nullptr;
   31: 
   32: 	UPROPERTY()
   33: 	TObjectPtr<UAnimationAsset> LoopingAnim = nullptr;
   34: 
   35: 	/** Optional alert/stand animation (preview). */
   36: 	UPROPERTY()
   37: 	TObjectPtr<UAnimationAsset> AlertAnim = nullptr;
   38: 
   39: 	/** Optional run animation (gameplay; when moving fast). */
   40: 	UPROPERTY()
   41: 	TObjectPtr<UAnimationAsset> RunAnim = nullptr;
   42: 
   43: 	/** Optional one-shot roll animation. */
   44: 	UPROPERTY()
   45: 	TObjectPtr<UAnimationAsset> RollAnim = nullptr;
   46: 
   47: 	UPROPERTY()
   48: 	FT66CharacterVisualRow Row;
   49: 
   50: 	UPROPERTY()
   51: 	bool bHasRow = false;
   52: };
   53: 
   54: /**
   55:  * Centralized character visuals resolver + applier.
   56:  *
   57:  * Goals:
   58:  * - data-driven mapping: ID -> skeletal mesh + optional looping animation + per-character transform
   59:  * - avoid repeated sync loads: load once per ID and cache
   60:  */
   61: UCLASS()
   62: class T66_API UT66CharacterVisualSubsystem : public UGameInstanceSubsystem
   63: {
   64: 	GENERATED_BODY()
   65: 
   66: public:
   67: 	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
   68: 
   69: 	/** Apply visual mapping to a SkeletalMeshComponent or optional StaticMeshComponent. Returns true if a mapping existed and was applied. */
   70: 	UFUNCTION(BlueprintCallable, Category = "T66|Visuals")
   71: 	bool ApplyCharacterVisual(
   72: 		FName VisualID,
   73: 		USkeletalMeshComponent* TargetMesh,
   74: 		USceneComponent* PlaceholderToHide = nullptr,
   75: 		bool bEnableSingleNodeAnimation = true,
   76: 		bool bUseAlertAnimation = false,
   77: 		bool bIsPreviewContext = false,
   78: 		UStaticMeshComponent* TargetStaticMesh = nullptr);
   79: 
   80: 	/** Compute the legacy hero visual row ID from HeroID + body style + SkinID (for example Chad -> Hero_1_Chad). */
   81: 	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|Visuals")
   82: 	static FName GetHeroVisualID(FName HeroID, ET66BodyType BodyType, FName SkinID);
   83: 
   84: 	/** Compute companion VisualID from CompanionID + SkinID (e.g. Companion_01 + Beachgoer -> Companion_01_Beachgoer). */
   85: 	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|Visuals")
   86: 	static FName GetCompanionVisualID(FName CompanionID, FName SkinID);
   87: 
   88: 	/** Resolve the canonical fallback visual row ID (for example Hero_1_Chad_Skin -> Hero_1_Chad). */
   89: 	static FName GetFallbackVisualID(FName VisualID);
   90: 
   91: 	/** Append all known preload candidates for a visual row, including animation fallback variants. */
   92: 	static void AppendCharacterVisualPreloadPaths(const FT66CharacterVisualRow& Row, TArray<FSoftObjectPath>& OutPaths);
   93: 
   94: 	/** Preload a visual mapping asynchronously and cache it once ready. */
   95: 	UFUNCTION(BlueprintCallable, Category = "T66|Visuals")
   96: 	void PreloadCharacterVisual(FName VisualID);
   97: 
   98: 	/** Returns true when a visual no longer has pending preload work for the given ID. */
   99: 	UFUNCTION(BlueprintCallable, Category = "T66|Visuals")
  100: 	bool IsCharacterVisualReady(FName VisualID) const;
  101: 
  102: 	/** Get walk, run/jump, alert/idle, and roll animations for a visual. OutRun/OutAlert/OutRoll may be null. */
  103: 	UFUNCTION(BlueprintCallable, Category = "T66|Visuals")
  104: 	void GetMovementAnimsForVisual(FName VisualID, UAnimationAsset*& OutWalk, UAnimationAsset*& OutRun, UAnimationAsset*& OutAlert, UAnimationAsset*& OutRoll);
  105: 
  106: 	/** Returns true when a visual row exists for this ID or its fallback ID. */
  107: 	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|Visuals")
  108: 	bool HasCharacterVisual(FName VisualID) const;
  109: 
  110: 	bool TryGetMobVertexAnimationRow(FName VisualID, FT66MobVertexAnimationRow& OutRow) const;
  111: 	bool ApplyMobVertexAnimationVisual(FName VisualID, UStaticMeshComponent* TargetStaticMesh, UMaterialInstanceDynamic*& OutMID, FT66MobVertexAnimationRow& OutRow);
  112: 
  113: private:
  114: 	FT66ResolvedCharacterVisual ResolveVisual(FName VisualID);
  115: 	UDataTable* GetVisualsDataTable() const;
  116: 	UDataTable* GetMobVertexAnimationsDataTable() const;
  117: 	UAnimationAsset* FindFallbackLoopingAnim(USkeleton* Skeleton) const;
  118: 	const FT66CharacterVisualRow* FindVisualRow(FName VisualID, FName* OutResolvedVisualID = nullptr) const;
  119: 	void HandleCharacterVisualPreloadCompleted(FName VisualID);
  120: 
  121: 	UPROPERTY(Transient)
  122: 	mutable TObjectPtr<UDataTable> CachedVisualsDataTable;
  123: 
  124: 	UPROPERTY(Transient)
  125: 	mutable TObjectPtr<UDataTable> CachedMobVertexAnimationsDataTable;
  126: 
  127: 	UPROPERTY(Transient)
  128: 	mutable TMap<FName, FT66ResolvedCharacterVisual> ResolvedCache;
  129: 
  130: 	TMap<FName, TSharedPtr<FStreamableHandle>> PendingPreloadHandles;
  131: 
  132: 	/** Cache: Skeleton asset path -> chosen looping animation */
  133: 	UPROPERTY(Transient)
  134: 	mutable TMap<FName, TObjectPtr<UAnimationAsset>> SkeletonAnimCache;
  135: };
  136:
```

### Source/T66/Core/T66CharacterVisualSubsystem.cpp
```cpp
   26: static const TCHAR* T66_DefaultCharacterVisualsDTPath = TEXT("/Game/Data/DT_CharacterVisuals.DT_CharacterVisuals");
   27: static const TCHAR* T66_DefaultMobVertexAnimationsDTPath = TEXT("/Game/Data/DT_MobVertexAnimations.DT_MobVertexAnimations");
   28: static const FName T66_AnimSkeletonTag(TEXT("Skeleton"));
   29: static const FName T66_CharactersRootPath(TEXT("/Game/Characters"));
   30: static const TCHAR* T66_CharacterBaseMaterialPath = TEXT("/Game/Materials/M_Character_Unlit.M_Character_Unlit");
   31: static const TCHAR* T66_FbxBaseMaterialPath = TEXT("/Game/Materials/M_FBX_Unlit.M_FBX_Unlit");
   32: static const TCHAR* T66_QuadRetroSharedMaterialPath = TEXT("/Game/Materials/MI_GLB_Unlit_Character_Shared.MI_GLB_Unlit_Character_Shared");
   33: static constexpr float T66_CharacterVisualBrightness = 0.8f;

// ...

  713: void UT66CharacterVisualSubsystem::AppendCharacterVisualPreloadPaths(const FT66CharacterVisualRow& Row, TArray<FSoftObjectPath>& OutPaths)
  714: {
  715: 	T66AppendCharacterVisualAssetPaths(Row, OutPaths);
  716: }
  717: 
  718: UAnimationAsset* UT66CharacterVisualSubsystem::FindFallbackLoopingAnim(USkeleton* Skeleton) const
  719: {
  720: 	if (!Skeleton)
  721: 	{
  722: 		return nullptr;
  723: 	}
  724: 
  725: 	const FName SkelKey(*Skeleton->GetPathName());
  726: 	if (const TObjectPtr<UAnimationAsset>* Cached = SkeletonAnimCache.Find(SkelKey))
  727: 	{
  728: 		return Cached ? Cached->Get() : nullptr;
  729: 	}
  730: 
  731: 	UAnimationAsset* Chosen = nullptr;
  732: 	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
  733: 	IAssetRegistry& Registry = ARM.Get();
  734: 
  735: 	FARFilter Filter;
  736: 	Filter.bRecursivePaths = true;
  737: 	Filter.PackagePaths.Add(T66_CharactersRootPath);
  738: 	Filter.ClassPaths.Add(UAnimSequence::StaticClass()->GetClassPathName());
  739: 
  740: 	TArray<FAssetData> Assets;
  741: 	Registry.GetAssets(Filter, Assets);
  742: 
  743: 	const FString SkeletonPath = Skeleton->GetPathName();
  744: 
  745: 	// Prefer an Idle if present, otherwise Walk/Run, otherwise first match.
  746: 	int32 BestScore = -1;
  747: 	FAssetData BestAsset;
  748: 
  749: 	for (const FAssetData& A : Assets)
  750: 	{
  751: 		FString Tag;
  752: 		if (!A.GetTagValue(T66_AnimSkeletonTag, Tag))
  753: 		{
  754: 			continue;
  755: 		}
  756: 		// Tag formats can vary; be permissive.
  757: 		if (!Tag.Contains(SkeletonPath))
  758: 		{
  759: 			continue;
  760: 		}
  761: 
  762: 		const FString Name = A.AssetName.ToString().ToLower();
  763: 		int32 Score = 0;
  764: 		if (Name.Contains("idle")) Score += 100;
  765: 		if (Name.Contains("walk")) Score += 50;
  766: 		if (Name.Contains("run")) Score += 40;
  767: 		if (Name.Contains("loop")) Score += 10;
  768: 
  769: 		if (Score > BestScore)
  770: 		{
  771: 			BestScore = Score;
  772: 			BestAsset = A;
  773: 		}
  774: 	}
  775: 
  776: 	if (BestScore >= 0)
  777: 	{
  778: 		UObject* Obj = BestAsset.GetAsset(); // loads
  779: 		Chosen = Cast<UAnimationAsset>(Obj);
  780: 	}
  781: 
  782: 	SkeletonAnimCache.Add(SkelKey, Chosen);
  783: 	return Chosen;
  784: }
  785: 
  786: UDataTable* UT66CharacterVisualSubsystem::GetVisualsDataTable() const
  787: {
  788: 	if (CachedVisualsDataTable)
  789: 	{
  790: 		return CachedVisualsDataTable;
  791: 	}
  792: 
  793: 	if (const UGameInstance* GI = GetGameInstance())
  794: 	{
  795: 		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(const_cast<UGameInstance*>(GI)))
  796: 		{
  797: 			if (UDataTable* LoadedDataTable = T66GI->GetCharacterVisualsDataTable())
  798: 			{
  799: 				CachedVisualsDataTable = LoadedDataTable;
  800: 				return CachedVisualsDataTable;
  801: 			}
  802: 
  803: 			// Optional: wire this in BP_T66GameInstance later; keep a safe hardcoded fallback for now.
  804: 			if (!T66GI->CharacterVisualsDataTable.IsNull())
  805: 			{
  806: 				CachedVisualsDataTable = T66GI->CharacterVisualsDataTable.Get();
  807: 				if (CachedVisualsDataTable)
  808: 				{
  809: 					return CachedVisualsDataTable;
  810: 				}
  811: 			}
  812: 		}
  813: 	}
  814: 
  815: 	CachedVisualsDataTable = FindObject<UDataTable>(nullptr, T66_DefaultCharacterVisualsDTPath);
  816: 	if (!CachedVisualsDataTable && GEngine)
  817: 	{
  818: 		static bool bLoggedMissingVisualsTable = false;
  819: 		if (!bLoggedMissingVisualsTable)
  820: 		{
  821: 			bLoggedMissingVisualsTable = true;
  822: 			UE_LOG(
  823: 				LogT66CharacterVisuals,
  824: 				Warning,
  825: 				TEXT("[MESH] DT_CharacterVisuals was not preloaded. Character visual resolution will wait for GameInstance async core tables instead of sync-loading %s."),
  826: 				T66_DefaultCharacterVisualsDTPath);
  827: 		}
  828: 	}
  829: 	return CachedVisualsDataTable;
  830: }
  831: 
  832: UDataTable* UT66CharacterVisualSubsystem::GetMobVertexAnimationsDataTable() const
  833: {
  834: 	if (CachedMobVertexAnimationsDataTable)
  835: 	{
  836: 		return CachedMobVertexAnimationsDataTable;
  837: 	}
  838: 
  839: 	CachedMobVertexAnimationsDataTable = FindObject<UDataTable>(nullptr, T66_DefaultMobVertexAnimationsDTPath);
  840: 	if (!CachedMobVertexAnimationsDataTable)
  841: 	{
  842: 		CachedMobVertexAnimationsDataTable = LoadObject<UDataTable>(nullptr, T66_DefaultMobVertexAnimationsDTPath);
  843: 	}
  844: 
  845: 	if (!CachedMobVertexAnimationsDataTable)
  846: 	{
  847: 		static bool bLoggedMissingMobVertexTable = false;
  848: 		if (!bLoggedMissingMobVertexTable)
  849: 		{
  850: 			bLoggedMissingMobVertexTable = true;
  851: 			UE_LOG(LogT66CharacterVisuals, Warning, TEXT("[MOB_VAT] Missing DT_MobVertexAnimations at %s. Mob visuals will use CharacterVisuals fallback."), T66_DefaultMobVertexAnimationsDTPath);
  852: 		}
  853: 	}
  854: 
  855: 	return CachedMobVertexAnimationsDataTable;
  856: }
  857: 
  858: bool UT66CharacterVisualSubsystem::TryGetMobVertexAnimationRow(FName VisualID, FT66MobVertexAnimationRow& OutRow) const
  859: {
  860: 	UDataTable* DT = GetMobVertexAnimationsDataTable();
  861: 	if (!DT || VisualID.IsNone())
  862: 	{
  863: 		return false;
  864: 	}
  865: 
  866: 	const FT66MobVertexAnimationRow* Row = DT->FindRow<FT66MobVertexAnimationRow>(VisualID, TEXT("TryGetMobVertexAnimationRow"), false);
  867: 	if (!Row || !Row->bEnabled)
  868: 	{
  869: 		return false;
  870: 	}
  871: 
  872: 	OutRow = *Row;
  873: 	return true;
  874: }
  875: 
  876: bool UT66CharacterVisualSubsystem::ApplyMobVertexAnimationVisual(

// ...

  876: bool UT66CharacterVisualSubsystem::ApplyMobVertexAnimationVisual(
  877: 	FName VisualID,
  878: 	UStaticMeshComponent* TargetStaticMesh,
  879: 	UMaterialInstanceDynamic*& OutMID,
  880: 	FT66MobVertexAnimationRow& OutRow)
  881: {
  882: 	OutMID = nullptr;
  883: 	if (!TargetStaticMesh || !TryGetMobVertexAnimationRow(VisualID, OutRow))
  884: 	{
  885: 		return false;
  886: 	}
  887: 
  888: 	UStaticMesh* StaticMesh = OutRow.StaticMesh.Get();
  889: 	if (!StaticMesh && !OutRow.StaticMesh.IsNull())
  890: 	{
  891: 		StaticMesh = OutRow.StaticMesh.LoadSynchronous();
  892: 	}
  893: 
  894: 	UMaterialInterface* Material = OutRow.Material.Get();
  895: 	if (!Material && !OutRow.Material.IsNull())
  896: 	{
  897: 		Material = OutRow.Material.LoadSynchronous();
  898: 	}
  899: 
  900: 	UTexture2D* PixelatedTexture = OutRow.PixelatedTextureAssetPath.Get();
  901: 	if (!PixelatedTexture && !OutRow.PixelatedTextureAssetPath.IsNull())
  902: 	{
  903: 		PixelatedTexture = OutRow.PixelatedTextureAssetPath.LoadSynchronous();
  904: 	}
  905: 
  906: 	UTexture2D* PositionTexture = OutRow.PositionTexture.Get();
  907: 	if (!PositionTexture && !OutRow.PositionTexture.IsNull())
  908: 	{
  909: 		PositionTexture = OutRow.PositionTexture.LoadSynchronous();
  910: 	}
  911: 
  912: 	UTexture2D* NormalTexture = OutRow.NormalTexture.Get();
  913: 	if (!NormalTexture && !OutRow.NormalTexture.IsNull())
  914: 	{
  915: 		NormalTexture = OutRow.NormalTexture.LoadSynchronous();
  916: 	}
  917: 
  918: 	if (!StaticMesh || !Material || !PixelatedTexture || !PositionTexture)
  919: 	{
  920: 		UE_LOG(LogT66CharacterVisuals, Warning, TEXT("[MOB_VAT] Could not apply VisualID=%s. Mesh=%s Material=%s PixelTexture=%s PositionTexture=%s"),
  921: 			*VisualID.ToString(),
  922: 			StaticMesh ? *StaticMesh->GetPathName() : TEXT("(null)"),
  923: 			Material ? *Material->GetPathName() : TEXT("(null)"),
  924: 			PixelatedTexture ? *PixelatedTexture->GetPathName() : TEXT("(null)"),
  925: 			PositionTexture ? *PositionTexture->GetPathName() : TEXT("(null)"));
  926: 		return false;
  927: 	}
  928: 
  929: 	TargetStaticMesh->EmptyOverrideMaterials();
  930: 	TargetStaticMesh->SetStaticMesh(StaticMesh);
  931: 	TargetStaticMesh->SetRelativeRotation(OutRow.MeshRelativeRotation);
  932: 
  933: 	const FVector Scale = OutRow.MeshRelativeScale.IsNearlyZero() ? FVector::OneVector : OutRow.MeshRelativeScale;
  934: 	TargetStaticMesh->SetRelativeScale3D(Scale);
  935: 
  936: 	FVector RelLoc = OutRow.MeshRelativeLocation;
  937: 	if (const ACharacter* OwnerChar = Cast<ACharacter>(TargetStaticMesh->GetOwner()))
  938: 	{
  939: 		if (const UCapsuleComponent* Cap = OwnerChar->GetCapsuleComponent())
  940: 		{
  941: 			const FBoxSphereBounds Bounds = StaticMesh->GetBounds();
  942: 			const float BottomZ = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale.Z;
  943: 			RelLoc.Z += -Cap->GetScaledCapsuleHalfHeight() - BottomZ;
  944: 		}
  945: 	}
  946: 	TargetStaticMesh->SetRelativeLocation(RelLoc);
  947: 	TargetStaticMesh->SetHiddenInGame(false, true);
  948: 	TargetStaticMesh->SetVisibility(true, true);
  949: 
  950: 	const int32 NumMaterials = FMath::Max(1, TargetStaticMesh->GetNumMaterials());
  951: 	for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; ++MaterialIndex)
  952: 	{
  953: 		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, TargetStaticMesh);
  954: 		if (!DynamicMaterial)
  955: 		{
  956: 			continue;
  957: 		}
  958: 
  959: 		DynamicMaterial->SetTextureParameterValue(TEXT("EmissiveTexture"), PixelatedTexture);
  960: 		DynamicMaterial->SetTextureParameterValue(TEXT("BaseColorTexture"), PixelatedTexture);
  961: 		DynamicMaterial->SetTextureParameterValue(TEXT("DiffuseColorMap"), PixelatedTexture);
  962: 		DynamicMaterial->SetTextureParameterValue(TEXT("PositionTexture"), PositionTexture);
  963: 		if (NormalTexture)
  964: 		{
  965: 			DynamicMaterial->SetTextureParameterValue(TEXT("NormalTexture"), NormalTexture);
  966: 		}
  967: 		DynamicMaterial->SetScalarParameterValue(TEXT("Brightness"), T66_CharacterVisualBrightness);
  968: 		DynamicMaterial->SetScalarParameterValue(TEXT("Frame"), static_cast<float>(OutRow.IdleStartFrame));
  969: 		DynamicMaterial->SetScalarParameterValue(TEXT("SampleRate"), OutRow.SampleRate);
  970: 		DynamicMaterial->SetScalarParameterValue(TEXT("NumFrames"), static_cast<float>(OutRow.NumFrames));
  971: 		DynamicMaterial->SetScalarParameterValue(TEXT("RowsPerFrame"), static_cast<float>(OutRow.RowsPerFrame));
  972: 		DynamicMaterial->SetVectorParameterValue(TEXT("MinBBox"), FLinearColor(OutRow.MinBBox.X, OutRow.MinBBox.Y, OutRow.MinBBox.Z, 0.f));
  973: 		DynamicMaterial->SetVectorParameterValue(TEXT("SizeBBox"), FLinearColor(OutRow.SizeBBox.X, OutRow.SizeBBox.Y, OutRow.SizeBBox.Z, 0.f));
  974: 		DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveFactor"), FLinearColor::White);
  975: 		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColorFactor"), FLinearColor::Black);
  976: 		DynamicMaterial->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
  977: 		TargetStaticMesh->SetMaterial(MaterialIndex, DynamicMaterial);
  978: 		if (!OutMID)
  979: 		{
  980: 			OutMID = DynamicMaterial;
  981: 		}
  982: 	}
  983: 
  984: 	if (!OutMID)
  985: 	{
  986: 		return false;
  987: 	}
  988: 
  989: 	UE_LOG(LogT66CharacterVisuals, Verbose, TEXT("[MOB_VAT] Applied VisualID=%s Mesh=%s PositionTexture=%s RowsPerFrame=%d NumFrames=%d"),
  990: 		*VisualID.ToString(),
  991: 		*StaticMesh->GetPathName(),
  992: 		*PositionTexture->GetPathName(),
  993: 		OutRow.RowsPerFrame,
  994: 		OutRow.NumFrames);
  995: 	return true;
  996: }
  997: 

// ...

 1325: void UT66CharacterVisualSubsystem::GetMovementAnimsForVisual(FName VisualID, UAnimationAsset*& OutWalk, UAnimationAsset*& OutRun, UAnimationAsset*& OutAlert, UAnimationAsset*& OutRoll)
 1326: {
 1327: 	OutWalk = nullptr;
 1328: 	OutRun = nullptr;
 1329: 	OutAlert = nullptr;
 1330: 	OutRoll = nullptr;
 1331: 	const FT66ResolvedCharacterVisual Res = ResolveVisual(VisualID);
 1332: 	if (!Res.bHasRow) return;
 1333: 	OutWalk = Res.LoopingAnim;
 1334: 	OutRun = Res.RunAnim;
 1335: 	OutAlert = Res.AlertAnim;
 1336: 	OutRoll = Res.RollAnim;
 1337: }
 1338: 
 1339: bool UT66CharacterVisualSubsystem::HasCharacterVisual(FName VisualID) const
 1340: {
 1341: 	return FindVisualRow(VisualID) != nullptr;
 1342: }
 1343: 
 1344: bool UT66CharacterVisualSubsystem::ApplyCharacterVisual(
 1345: 	FName VisualID,
 1346: 	USkeletalMeshComponent* TargetMesh,
 1347: 	USceneComponent* PlaceholderToHide,
 1348: 	bool bEnableSingleNodeAnimation,
 1349: 	bool bUseAlertAnimation,
 1350: 	bool bIsPreviewContext,
 1351: 	UStaticMeshComponent* TargetStaticMesh)
 1352: {
 1353: 	if (VisualID.IsNone() || (!TargetMesh && !TargetStaticMesh))
 1354: 	{
 1355: 		return false;
 1356: 	}
 1357: 
 1358: 	const FT66ResolvedCharacterVisual Res = ResolveVisual(VisualID);
 1359: 	if (!Res.bHasRow || (!Res.Mesh && !Res.StaticMesh))
 1360: 	{
 1361: 		UE_LOG(LogT66CharacterVisuals, Warning, TEXT("[MESH] ApplyCharacterVisual FAILED for VisualID=%s: bHasRow=%d, SkeletalMesh=%s StaticMesh=%s"),
 1362: 			*VisualID.ToString(),
 1363: 			Res.bHasRow ? 1 : 0,
 1364: 			Res.Mesh ? *Res.Mesh->GetName() : TEXT("(null)"),
 1365: 			Res.StaticMesh ? *Res.StaticMesh->GetName() : TEXT("(null)"));
 1366: 		return false;
 1367: 	}
 1368: 
 1369: 	if (Res.StaticMesh && (!Res.Mesh || !TargetMesh) && TargetStaticMesh)
 1370: 	{
 1371: 		TargetStaticMesh->EmptyOverrideMaterials();
 1372: 		TargetStaticMesh->SetStaticMesh(Res.StaticMesh);
 1373: 		TargetStaticMesh->SetRelativeRotation(Res.Row.MeshRelativeRotation);
 1374: 
 1375: 		const FVector Scale = Res.Row.MeshRelativeScale.IsNearlyZero() ? FVector::OneVector : Res.Row.MeshRelativeScale;
 1376: 		TargetStaticMesh->SetRelativeScale3D(Scale);
 1377: 
 1378: 		FVector RelLoc = Res.Row.MeshRelativeLocation;
 1379: 		const bool bIsCharacterOwner = Cast<ACharacter>(TargetStaticMesh->GetOwner()) != nullptr;
 1380: 		const FBoxSphereBounds B = Res.StaticMesh->GetBounds();
 1381: 		const float BottomZ = (B.Origin.Z - B.BoxExtent.Z) * Scale.Z;
 1382: 		if (bIsCharacterOwner)
 1383: 		{
 1384: 			if (const ACharacter* OwnerChar = Cast<ACharacter>(TargetStaticMesh->GetOwner()))
 1385: 			{
 1386: 				if (const UCapsuleComponent* Cap = OwnerChar->GetCapsuleComponent())
 1387: 				{
 1388: 					RelLoc.Z += -Cap->GetScaledCapsuleHalfHeight() - BottomZ;
 1389: 				}
 1390: 			}
 1391: 		}
 1392: 		else if (Res.Row.bAutoGroundToActorOrigin)
 1393: 		{
 1394: 			RelLoc.Z -= BottomZ;
 1395: 		}
 1396: 
 1397: 		TargetStaticMesh->SetRelativeLocation(RelLoc);
 1398: 		TargetStaticMesh->SetHiddenInGame(false, true);
 1399: 		TargetStaticMesh->SetVisibility(true, true);
 1400: 		if (T66IsQuadRetroStaticVisual(Res))
 1401: 		{
 1402: 			UMaterialInterface* SharedMaterial = T66LoadQuadRetroSharedMaterial();
 1403: 			UTexture2D* PixelatedTexture = nullptr;
 1404: 			if (!Res.Row.PixelatedTextureAssetPath.IsNull())
 1405: 			{
 1406: 				PixelatedTexture = Res.Row.PixelatedTextureAssetPath.Get();
 1407: 				if (!PixelatedTexture)
 1408: 				{
 1409: 					PixelatedTexture = Res.Row.PixelatedTextureAssetPath.LoadSynchronous();
 1410: 				}
 1411: 			}
 1412: 
 1413: 			if (SharedMaterial && PixelatedTexture)
 1414: 			{
 1415: 				T66ApplyQuadRetroStaticMaterialOverrides(TargetStaticMesh, SharedMaterial, PixelatedTexture, VisualID);
 1416: 			}
 1417: 			else
 1418: 			{
 1419: 				UE_LOG(
 1420: 					LogT66CharacterVisuals,
 1421: 					Warning,
 1422: 					TEXT("[MATERIAL] QuadRetro static visual %s missing shared material or pixelated texture. Shared=%s Texture=%s"),
 1423: 					*VisualID.ToString(),
 1424: 					SharedMaterial ? *SharedMaterial->GetPathName() : TEXT("(null)"),
 1425: 					PixelatedTexture ? *PixelatedTexture->GetPathName() : TEXT("(null)"));
 1426: 			}
 1427: 		}
 1428: 
 1429: 		if (TargetMesh)
 1430: 		{
 1431: 			TargetMesh->SetHiddenInGame(true, true);
 1432: 			TargetMesh->SetVisibility(false, true);
 1433: 		}
 1434: 		if (PlaceholderToHide && PlaceholderToHide != TargetStaticMesh)
 1435: 		{
 1436: 			PlaceholderToHide->SetVisibility(false, true);
 1437: 			PlaceholderToHide->SetHiddenInGame(true, true);
 1438: 		}
 1439: 		return true;
 1440: 	}
 1441: 
 1442: 	if (!TargetMesh || !Res.Mesh)
 1443: 	{
 1444: 		return false;
 1445: 	}
 1446:
```

### Source/T66/Gameplay/T66EnemyBase.h
```cpp
    1: // Copyright Tribulation 66. All Rights Reserved.
    2: 
    3: #pragma once
    4: 
    5: #include "CoreMinimal.h"
    6: #include "Data/T66DataTypes.h"
    7: #include "GameFramework/Character.h"
    8: #include "Gameplay/Enemies/T66EnemyFamilyTypes.h"
    9: #include "Gameplay/T66CombatTargetTypes.h"
   10: #include "T66EnemyBase.generated.h"
   11: 
   12: class UWidgetComponent;
   13: class UStaticMeshComponent;
   14: class AT66EnemyDirector;
   15: class UT66CombatHitZoneComponent;
   16: class UPrimitiveComponent;
   17: class UMaterialInstanceDynamic;
   18: 
   19: UCLASS(Blueprintable)
   20: class T66_API AT66EnemyBase : public ACharacter
   21: {
   22: 	GENERATED_BODY()
   23: 
   24: public:
   25: 	AT66EnemyBase();
   26: 
   27: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
   28: 	int32 MaxHP = 100;
   29: 
   30: 	UPROPERTY(BlueprintReadOnly, Category = "Combat")
   31: 	int32 CurrentHP = 100;
   32: 
   33: 	/** Touch damage to player in hearts (scaled by difficulty). */
   34: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
   35: 	int32 TouchDamageHearts = 1;
   36: 
   37: 	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
   38: 	ET66EnemyFamily EnemyFamily = ET66EnemyFamily::Melee;
   39: 
   40: 	/** Enemy armor: damage reduction fraction (0.0 = none, 0.5 = 50% reduction). Reduced by Taunt procs. */
   41: 	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
   42: 	float Armor = 0.f;
   43: 
   44: 	/** True if this enemy is currently confused/wandering (Invisibility proc). */
   45: 	UPROPERTY(BlueprintReadOnly, Category = "AI")
   46: 	bool bIsConfused = false;
   47: 
   48: 	/** Remaining seconds of confusion effect. */
   49: 	UPROPERTY(BlueprintReadOnly, Category = "AI")
   50: 	float ConfusionSecondsRemaining = 0.f;
   51: 
   52: 	/** Apply a confusion effect (from hero Invisibility proc). */
   53: 	UFUNCTION(BlueprintCallable, Category = "Combat")
   54: 	void ApplyConfusion(float DurationSeconds);
   55: 
   56: 	/** Apply an armor debuff (from hero Taunt proc). Reduces armor temporarily. */
   57: 	UFUNCTION(BlueprintCallable, Category = "Combat")
   58: 	void ApplyArmorDebuff(float ReductionAmount, float DurationSeconds);
   59: 
   60: 	/** Apply a move speed slow (from Frostbite passive). Multiplier is applied for DurationSeconds. */
   61: 	void ApplyMoveSlow(float SpeedMultiplier, float DurationSeconds);
   62: 
   63: 	/** Force this enemy to flee from the player for a short duration without changing its default AI tuning. */
   64: 	void ApplyForcedRunAway(float DurationSeconds);
   65: 
   66: 	/** Hard crowd control that fully interrupts enemy movement for a short duration. */
   67: 	void ApplyStun(float DurationSeconds);
   68: 
   69: 	/** Bind/root effect: enemy cannot move but may still remain active. */
   70: 	void ApplyRoot(float DurationSeconds);
   71: 
   72: 	/** Freeze is a stronger immobilize that also visually reads as a full stop. */
   73: 	void ApplyFreeze(float DurationSeconds);
   74: 
   75: 	/** Pull the enemy toward a point by a short swept displacement. */
   76: 	void ApplyPullTowards(const FVector& PullOrigin, float Distance);
   77: 
   78: 	/** Push the enemy away from a point by a short swept displacement. */
   79: 	void ApplyPushAwayFrom(const FVector& PushOrigin, float Distance);
   80: 
   81: 	/** Point value for wave budget and score (Bible 2.9) */
   82: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
   83: 	int32 PointValue = 10;
   84: 
   85: 	/** XP granted to the hero on death. */
   86: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression")
   87: 	int32 XPValue = 20;
   88: 
   89: 	/** If false, this enemy will not spawn a loot bag on death (used by mimics/special cases). */
   90: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
   91: 	bool bDropsLoot = true;
   92: 
   93: 	/** Visible mesh (cylinder) so enemy is seen */
   94: 	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
   95: 	TObjectPtr<UStaticMeshComponent> VisualMesh;
   96: 
   97: 	/** Visual mapping ID used by UT66CharacterVisualSubsystem (data-driven imported mesh). */
   98: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
   99: 	FName CharacterVisualID = FName(TEXT("RegularEnemy"));
  100: 
  101: 	/** Dedicated bullseye widget shown when this enemy is manually locked. */
  102: 	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
  103: 	TObjectPtr<UWidgetComponent> LockIndicatorWidget;
  104: 
  105: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|HitZones")
  106: 	bool bUsesCombatHitZones = true;
  107: 
  108: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|HitZones", meta = (ClampMin = "0.1"))
  109: 	float BodyDamageMultiplier = 1.f;
  110: 
  111: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|HitZones", meta = (ClampMin = "0.1"))
  112: 	float HeadDamageMultiplier = 1.5f;
  113: 
  114: 	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|HitZones")
  115: 	TObjectPtr<UT66CombatHitZoneComponent> BodyHitZone;
  116: 
  117: 	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|HitZones")
  118: 	TObjectPtr<UT66CombatHitZoneComponent> HeadHitZone;
  119: 
  120: 	/** Director that spawned this enemy (for death notification) */
  121: 	UPROPERTY(BlueprintReadWrite, Category = "AI")
  122: 	TObjectPtr<AT66EnemyDirector> OwningDirector;
  123: 
  124: 	/** Apply damage from hero. Returns true if enemy died. DamageSourceID used for run damage log (default: AutoAttack). EventType for floating text (Crit, DoT, etc.; default none). */
  125: 	UFUNCTION(BlueprintCallable, Category = "Combat")
  126: 	virtual bool TakeDamageFromHero(int32 Damage, FName DamageSourceID = NAME_None, FName EventType = NAME_None);
  127: 
  128: 	virtual bool TakeDamageFromHeroHitZone(int32 Damage, const FT66CombatTargetHandle& TargetHandle, FName DamageSourceID = NAME_None, FName EventType = NAME_None);
  129: 	virtual bool TakeDamageFromEnvironment(int32 Damage, AActor* DamageCauser = nullptr, FName EventType = NAME_None);
  130: 
  131: 	/** Briefly shove the enemy back when hit by a hero auto attack. */
  132: 	void ApplyAutoAttackKnockback(const FVector& HitOrigin, float StrengthScale = 1.f);
  133: 
  134: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0"))
  135: 	float AutoAttackKnockbackSpeed = 260.f;
  136: 
  137: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0"))
  138: 	float AutoAttackKnockbackStutterSeconds = 0.12f;
  139: 
  140: 	float GetEffectiveArmor() const;
  141: 
  142: 	/** If true, this enemy prefers to flee from the hero instead of closing distance. */
  143: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
  144: 	bool bRunAwayFromPlayer = false;
  145: 
  146: 	/** If distance to player exceeds this (uu), enemy gains leash speed instead of teleporting. 0 = disabled. */
  147: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0"))
  148: 	float LeashMaxDistance = 3000.f;
  149: 
  150: 	/** Legacy leash interval kept for backwards compatibility with existing defaults/assets. */
  151: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.5"))
  152: 	float LeashCheckIntervalSeconds = 2.f;
  153: 
  154: 	/** Maximum speed multiplier applied when the enemy falls far behind the player. */
  155: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "1.0"))
  156: 	float FarChaseSpeedMultiplier = 2.0f;
  157: 
  158: 	/** Distance beyond LeashMaxDistance over which the far-chase speed ramps up. */
  159: 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "1.0"))
  160: 	float FarChaseRampDistance = 2000.f;
  161: 
  162: 	/** Show/hide the lock indicator on this enemy's health bar. */
  163: 	void SetLockedIndicator(bool bLocked);
  164: 
  165: 	bool SupportsCombatHitZones() const;
  166: 	FT66CombatTargetHandle ResolveCombatTargetHandle(const UPrimitiveComponent* HitComponent = nullptr, ET66HitZoneType PreferredZone = ET66HitZoneType::Body) const;
  167: 	FVector GetAimPointForHitZone(ET66HitZoneType HitZoneType) const;
  168: 
  169: 	/** Apply stage-based HP/Armor: stage 1 = 50 HP, 0.1 armor; each stage multiplies by 1.1. Call before ApplyDifficultyScalar. */
  170: 	void ApplyStageScaling(int32 Stage);
  171: 
  172: 	/** Apply difficulty scaling using a scalar (e.g. 1.1, 1.2, ...). HP/Armor are skipped if ApplyStageScaling was used. */
  173: 	void ApplyDifficultyScalar(float Scalar);
  174: 
  175: 	/** Apply stage-within-difficulty progression on top of the base stage + difficulty tuning. */
  176: 	void ApplyProgressionEnemyScalar(float Scalar);
  177: 
  178: 	/** Extra end-of-difficulty survival scaling layered on top of the normal stage + difficulty tuning. */
  179: 	void ApplyFinaleScaling(float Scalar);
  180: 
  181: 	/** Freeze the score award using the difficulty scalar active when this enemy spawned. */
  182: 	void FreezeScoreAwardAtSpawn(float DifficultyScalar);
  183: 
  184: 	int32 GetResolvedScoreAward() const { return ResolvedScoreAward; }
  185: 
  186: 	/** Apply difficulty tier (Tier 0 = 1.0x, Tier 1 = 1.1x, Tier 2 = 1.2x, ...). */
  187: 	void ApplyDifficultyTier(int32 Tier);
  188: 
  189: 	/** Stage mob ID (data-driven via DT_Stages EnemyA..EnemyJ). NAME_None means "not a stage mob". */
  190: 	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mob")
  191: 	FName MobID;
  192: 
  193: 	/** True if this instance is a mini-boss version of a stage mob. */
  194: 	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mob")
  195: 	bool bIsMiniBoss = false;
  196: 
  197: 	/** Configure placeholder visuals for a stage mob (shape + color). */
  198: 	UFUNCTION(BlueprintCallable, Category = "Mob")
  199: 	void ConfigureAsMob(FName InMobID);
  200: 
  201: #if !UE_BUILD_SHIPPING
  202: 	/** Automation-only hook for staged visual smoke tests. */
  203: 	void ForceMobVertexAnimationClipForAutomation(FName ClipName, float OverrideSeconds = 30.f);
  204: #endif
  205: 
  206: 	/** Apply mini-boss multipliers (call after difficulty scaling). */
  207: 	UFUNCTION(BlueprintCallable, Category = "Mob")
  208: 	void ApplyMiniBossMultipliers(float HPScalar, float DamageScalar, float ScaleScalar);
  209: 
  210: 	/** [GOLD] Reset this enemy for reuse from the object pool. */
  211: 	void ResetForReuse(const FVector& NewLocation, AT66EnemyDirector* NewDirector);
  212: 
  213: 	/** Start the rise-from-ground animation. Enemy is buried below TargetGroundZ and lerps up over RiseDuration. */
  214: 	void StartRiseFromGround(float TargetGroundZ);
  215: 
  216: 	/** Start a short emergence from a nearby wall surface into the arena. */
  217: 	void StartEmergeFromWall(const FVector& TargetLocation, const FVector& WallNormal);
  218: 
  219: protected:
  220: 	virtual void BeginPlay() override;
  221: 	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  222: 	virtual void Tick(float DeltaSeconds) override;
  223: 	virtual void ResetFamilyState();
  224: 	virtual void TickFamilyBehavior(APawn* PlayerPawn, float DeltaSeconds, float Dist2DToPlayer, bool bShouldRunAwayFromPlayer);
  225: 	virtual EMovementMode GetDefaultMovementMode() const { return MOVE_Walking; }
  226: 	float GetBaseWalkSpeed() const { return BaseMaxWalkSpeed; }
  227: 
  228: 	/** Called when HP reaches 0: notify director, spawn pickup, return to pool */
  229: 	virtual void OnDeath();
  230: 
  231: 	UFUNCTION()
  232: 	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
  233: 		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
  234: 
  235: 	/** Touch damage: last time we dealt damage to player (cooldown) */
  236: 	float LastTouchDamageTime = -9999.f;
  237: 	static constexpr float TouchDamageCooldown = 0.5f;
  238: 
  239: private:
  240: 	bool ApplyResolvedDamage(int32 Damage, bool bCreditHeroKill, FName DamageSourceID, FName EventType);
  241: 	APawn* ResolveCachedPlayerPawn(float DeltaSeconds);
  242: 	void RebuildScaledCombatStats(bool bResetCurrentHPToMax);
  243: 	void RefreshCombatHitZoneState();
  244: 	bool TryApplyMobVertexAnimationVisual();
  245: 	void SetMobVertexAnimationClip(FName ClipName, float OverrideSeconds = 0.f);
  246: 	void TickMobVertexAnimationState(float DeltaSeconds);
  247: 	bool GetMobVertexAnimationClipRange(FName ClipName, int32& OutStartFrame, int32& OutEndFrame, float& OutPlayRate) const;
  248: 	ET66HitZoneType ResolveHitZoneType(const UPrimitiveComponent* HitComponent, ET66HitZoneType PreferredZone) const;
  249: 	float GetHitZoneDamageMultiplier(ET66HitZoneType HitZoneType) const;
  250: 
  251: 	// Safety/perf: avoid per-enemy per-frame scans for safe zones.
  252: 	float SafeZoneCheckAccumSeconds = 0.f;
  253: 	/** Safe-zone check runs every this many seconds (perf: was 0.25, then 0.5; 1.0 reduces N×M cost). */
  254: 	float SafeZoneCheckIntervalSeconds = 1.0f;
  255: 	bool bCachedInsideSafeZone = false;
  256: 	FVector CachedSafeZoneEscapeDir = FVector::ZeroVector;
  257: 	FVector CachedSafeZoneCenter = FVector::ZeroVector;
  258: 	float CachedSafeZoneRadius = 0.f;
  259: 	FVector CachedSafeZoneLoiterDir = FVector::ZeroVector;
  260: 	float SafeZoneLoiterDirRefreshAccum = 0.f;
  261: 	static constexpr float SafeZoneLoiterDirRefreshInterval = 0.85f;
  262: 	static constexpr float SafeZoneLoiterMoveScale = 0.35f;
  263: 
  264: 	FT66MobVertexAnimationRow ActiveMobVertexAnimationRow;
  265: 	UPROPERTY(Transient)
  266: 	TObjectPtr<UMaterialInstanceDynamic> ActiveMobVertexAnimationMID;
  267: 	FName ActiveMobVertexAnimationClip = NAME_None;
  268: 	float MobVertexAnimationClipTime = 0.f;
  269: 	float MobVertexAnimationOverrideSecondsRemaining = 0.f;
  270: 	bool bUsingMobVertexAnimation = false;
  271: 
  272: 	bool bBaseTuningInitialized = false;
  273: 	bool bStageScalingApplied = false;
  274: 	int32 BaseMaxHP = 0;
  275: 	int32 BaseTouchDamageHearts = 0;
  276: 	int32 BasePointValue = 0;
  277: 	int32 ResolvedScoreAward = 0;
  278: 	float BaseArmor = 0.f;
  279: 	float DifficultyScalarApplied = 1.0f;
  280: 	float ProgressionEnemyScalarApplied = 1.0f;
  281: 	float FinaleScalarApplied = 1.0f;
  282: 
  283: 	// Persist mini-boss multipliers so difficulty changes can re-apply cleanly.
  284: 	float MiniBossHPScalarApplied = 1.0f;
  285: 	float MiniBossDamageScalarApplied = 1.0f;
  286: 	float MiniBossScaleScalarApplied = 1.0f;
  287: 
  288: 	// Armor debuff tracking
  289: 	float ArmorDebuffAmount = 0.f;
  290: 	float ArmorDebuffSecondsRemaining = 0.f;
```

### Source/T66/Gameplay/T66EnemyBase.cpp
```cpp
   45: 	const FName T66MobVATClip_Idle(TEXT("Idle"));
   46: 	const FName T66MobVATClip_Move(TEXT("Move"));
   47: 	const FName T66MobVATClip_AttackCue(TEXT("AttackCue"));
   48: 	const FName T66MobVATClip_HitReact(TEXT("HitReact"));
   49: 	const FName T66MobVATClip_Death(TEXT("Death"));

// ...

  389: void AT66EnemyBase::ConfigureAsMob(FName InMobID)
  390: {
  391: 	MobID = InMobID;
  392: 	CharacterVisualID = MobID;
  393: 
  394: 	if (!VisualMesh) return;
  395: 
  396: 	// Stable per-mob visuals: mesh shape + HSV color from MobID hash.
  397: 	const uint32 H = GetTypeHash(MobID);
  398: 	const int32 Shape = static_cast<int32>(H % 4u);
  399: 
  400: 	UStaticMesh* StaticShapeMesh = nullptr;
  401: 	switch (Shape)
  402: 	{
  403: 		case 0: StaticShapeMesh = FT66VisualUtil::GetBasicShapeSphere(); break;
  404: 		case 1: StaticShapeMesh = FT66VisualUtil::GetBasicShapeCube(); break;
  405: 		case 2: StaticShapeMesh = FT66VisualUtil::GetBasicShapeCylinder(); break;
  406: 		default: StaticShapeMesh = FT66VisualUtil::GetBasicShapeCone(); break;
  407: 	}
  408: 	if (StaticShapeMesh)
  409: 	{
  410: 		VisualMesh->SetStaticMesh(StaticShapeMesh);
  411: 	}
  412: 
  413: 	const float Hue01 = static_cast<float>((H / 7u) % 360u) / 360.f;
  414: 	const FLinearColor C = FLinearColor::MakeFromHSV8(
  415: 		static_cast<uint8>(Hue01 * 255.f),
  416: 		210,
  417: 		240
  418: 	);
  419: 	FT66VisualUtil::ApplyT66Color(VisualMesh, this, C);
  420: 
  421: 	switch (Shape)
  422: 	{
  423: 		case 1: VisualMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f)); break;
  424: 		case 2: VisualMesh->SetRelativeScale3D(FVector(0.70f, 0.70f, 0.95f)); break;
  425: 		case 3: VisualMesh->SetRelativeScale3D(FVector(0.80f, 0.80f, 0.95f)); break;
  426: 		default: VisualMesh->SetRelativeScale3D(FVector(0.85f, 0.85f, 0.85f)); break;
  427: 	}
  428: 
  429: 	// Re-apply character visual for pooled (reused) actors whose BeginPlay already ran.
  430: 	if (HasActorBegunPlay() && !CharacterVisualID.IsNone() && CharacterVisualID != FName(TEXT("RegularEnemy")))
  431: 	{
  432: 		if (TryApplyMobVertexAnimationVisual())
  433: 		{
  434: 			return;
  435: 		}
  436: 
  437: 		if (UWorld* World = GetWorld())
  438: 		{
  439: 			if (UGameInstance* GI = World->GetGameInstance())
  440: 			{
  441: 				if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
  442: 				{
  443: 					bUsingCharacterVisual = Visuals->HasCharacterVisual(CharacterVisualID)
  444: 						&& Visuals->ApplyCharacterVisual(CharacterVisualID, GetMesh(), VisualMesh, true, false, false, VisualMesh);
  445: 					if (!bUsingCharacterVisual)
  446: 					{
  447: 						if (USkeletalMeshComponent* Skel = GetMesh())
  448: 						{
  449: 							Skel->SetHiddenInGame(true, true);
  450: 							Skel->SetVisibility(false, true);
  451: 						}
  452: 						if (VisualMesh)
  453: 						{
  454: 							VisualMesh->SetHiddenInGame(false, true);
  455: 							VisualMesh->SetVisibility(true, true);
  456: 						}
  457: 					}
  458: 				}
  459: 			}
  460: 		}
  461: 	}
  462: }
  463: 
  464: bool AT66EnemyBase::TryApplyMobVertexAnimationVisual()
  465: {
  466: 	bUsingMobVertexAnimation = false;
  467: 	ActiveMobVertexAnimationMID = nullptr;
  468: 	ActiveMobVertexAnimationClip = NAME_None;
  469: 	MobVertexAnimationClipTime = 0.f;
  470: 	MobVertexAnimationOverrideSecondsRemaining = 0.f;
  471: 
  472: 	if (CharacterVisualID.IsNone() || CharacterVisualID == FName(TEXT("RegularEnemy")) || !VisualMesh)
  473: 	{
  474: 		return false;
  475: 	}
  476: 
  477: 	if (UWorld* World = GetWorld())
  478: 	{
  479: 		if (UGameInstance* GI = World->GetGameInstance())
  480: 		{
  481: 			if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
  482: 			{
  483: 				UMaterialInstanceDynamic* DynamicMaterial = nullptr;
  484: 				FT66MobVertexAnimationRow Row;
  485: 				if (Visuals->ApplyMobVertexAnimationVisual(CharacterVisualID, VisualMesh, DynamicMaterial, Row) && DynamicMaterial)
  486: 				{
  487: 					ActiveMobVertexAnimationRow = Row;
  488: 					ActiveMobVertexAnimationMID = DynamicMaterial;
  489: 					bUsingMobVertexAnimation = true;
  490: 					bUsingCharacterVisual = false;
  491: 					if (USkeletalMeshComponent* Skel = GetMesh())
  492: 					{
  493: 						Skel->SetHiddenInGame(true, true);
  494: 						Skel->SetVisibility(false, true);
  495: 					}
  496: 					SetMobVertexAnimationClip(T66MobVATClip_Idle);
  497: 					return true;
  498: 				}
  499: 			}
  500: 		}
  501: 	}
  502: 
  503: 	return false;
  504: }
  505: 
  506: bool AT66EnemyBase::GetMobVertexAnimationClipRange(FName ClipName, int32& OutStartFrame, int32& OutEndFrame, float& OutPlayRate) const
  507: {
  508: 	if (ClipName == T66MobVATClip_Move)
  509: 	{
  510: 		OutStartFrame = ActiveMobVertexAnimationRow.MoveStartFrame;
  511: 		OutEndFrame = ActiveMobVertexAnimationRow.MoveEndFrame;
  512: 		OutPlayRate = ActiveMobVertexAnimationRow.MovePlayRate;
  513: 	}
  514: 	else if (ClipName == T66MobVATClip_AttackCue)
  515: 	{
  516: 		OutStartFrame = ActiveMobVertexAnimationRow.AttackCueStartFrame;
  517: 		OutEndFrame = ActiveMobVertexAnimationRow.AttackCueEndFrame;
  518: 		OutPlayRate = ActiveMobVertexAnimationRow.AttackCuePlayRate;
  519: 	}
  520: 	else if (ClipName == T66MobVATClip_HitReact)
  521: 	{
  522: 		OutStartFrame = ActiveMobVertexAnimationRow.HitReactStartFrame;
  523: 		OutEndFrame = ActiveMobVertexAnimationRow.HitReactEndFrame;
  524: 		OutPlayRate = ActiveMobVertexAnimationRow.HitReactPlayRate;
  525: 	}
  526: 	else if (ClipName == T66MobVATClip_Death)
  527: 	{
  528: 		OutStartFrame = ActiveMobVertexAnimationRow.DeathStartFrame;
  529: 		OutEndFrame = ActiveMobVertexAnimationRow.DeathEndFrame;
  530: 		OutPlayRate = ActiveMobVertexAnimationRow.DeathPlayRate;
  531: 	}
  532: 	else
  533: 	{
  534: 		OutStartFrame = ActiveMobVertexAnimationRow.IdleStartFrame;
  535: 		OutEndFrame = ActiveMobVertexAnimationRow.IdleEndFrame;
  536: 		OutPlayRate = ActiveMobVertexAnimationRow.IdlePlayRate;
  537: 	}
  538: 
  539: 	OutStartFrame = FMath::Max(0, OutStartFrame);
  540: 	OutEndFrame = FMath::Max(OutStartFrame, OutEndFrame);
  541: 	OutPlayRate = FMath::Max(0.01f, OutPlayRate);
  542: 	return ActiveMobVertexAnimationRow.SampleRate > 0.f
  543: 		&& ActiveMobVertexAnimationRow.RowsPerFrame > 0
  544: 		&& OutEndFrame >= OutStartFrame;
  545: }
  546: 
  547: void AT66EnemyBase::SetMobVertexAnimationClip(FName ClipName, float OverrideSeconds)
  548: {
  549: 	if (!bUsingMobVertexAnimation || !ActiveMobVertexAnimationMID)
  550: 	{
  551: 		return;
  552: 	}
  553: 
  554: 	int32 StartFrame = 0;
  555: 	int32 EndFrame = 0;
  556: 	float PlayRate = 1.f;
  557: 	if (!GetMobVertexAnimationClipRange(ClipName, StartFrame, EndFrame, PlayRate))
  558: 	{
  559: 		return;
  560: 	}
  561: 
  562: 	if (ActiveMobVertexAnimationClip != ClipName)
  563: 	{
  564: 		ActiveMobVertexAnimationClip = ClipName;
  565: 		MobVertexAnimationClipTime = 0.f;
  566: 		ActiveMobVertexAnimationMID->SetScalarParameterValue(TEXT("StartFrame"), static_cast<float>(StartFrame));
  567: 		ActiveMobVertexAnimationMID->SetScalarParameterValue(TEXT("EndFrame"), static_cast<float>(EndFrame));
  568: 	}
  569: 
  570: 	MobVertexAnimationOverrideSecondsRemaining = FMath::Max(MobVertexAnimationOverrideSecondsRemaining, OverrideSeconds);
  571: }
  572: 
  573: #if !UE_BUILD_SHIPPING
  574: void AT66EnemyBase::ForceMobVertexAnimationClipForAutomation(FName ClipName, float OverrideSeconds)
  575: {
  576: 	SetMobVertexAnimationClip(ClipName, OverrideSeconds);
  577: }
  578: #endif
  579: 
  580: void AT66EnemyBase::TickMobVertexAnimationState(float DeltaSeconds)
  581: {
  582: 	if (!bUsingMobVertexAnimation || !ActiveMobVertexAnimationMID)
  583: 	{
  584: 		return;
  585: 	}
  586: 
  587: 	if (MobVertexAnimationOverrideSecondsRemaining > 0.f)
  588: 	{
  589: 		MobVertexAnimationOverrideSecondsRemaining = FMath::Max(0.f, MobVertexAnimationOverrideSecondsRemaining - DeltaSeconds);
  590: 	}
  591: 	else
  592: 	{
  593: 		const FVector Velocity = GetVelocity();
  594: 		const FName DesiredClip = Velocity.SizeSquared2D() > FMath::Square(10.f) ? T66MobVATClip_Move : T66MobVATClip_Idle;
  595: 		if (ActiveMobVertexAnimationClip != DesiredClip)
  596: 		{
  597: 			SetMobVertexAnimationClip(DesiredClip);
  598: 		}
  599: 	}
  600: 
  601: 	int32 StartFrame = 0;
  602: 	int32 EndFrame = 0;
  603: 	float PlayRate = 1.f;
  604: 	if (!GetMobVertexAnimationClipRange(ActiveMobVertexAnimationClip, StartFrame, EndFrame, PlayRate))
  605: 	{
  606: 		return;
  607: 	}
  608: 
  609: 	MobVertexAnimationClipTime += DeltaSeconds;
  610: 	const int32 FrameCount = FMath::Max(1, EndFrame - StartFrame + 1);
  611: 	const int32 ClipFrameOffset = FMath::FloorToInt(MobVertexAnimationClipTime * ActiveMobVertexAnimationRow.SampleRate * PlayRate) % FrameCount;
  612: 	const int32 CurrentFrame = StartFrame + ClipFrameOffset;
  613: 	ActiveMobVertexAnimationMID->SetScalarParameterValue(TEXT("Frame"), static_cast<float>(CurrentFrame));
  614: }
  615: 

// ...

  625: void AT66EnemyBase::BeginPlay()
  626: {
  627: 	Super::BeginPlay();
  628: 
  629: 	// [GOLD] Register with the central actor registry (replaces TActorIterator world scans).
  630: 	if (UWorld* W = GetWorld())
  631: 	{
  632: 		if (UT66ActorRegistrySubsystem* Registry = W->GetSubsystem<UT66ActorRegistrySubsystem>())
  633: 		{
  634: 			Registry->RegisterEnemy(this);
  635: 		}
  636: 	}
  637: 
  638: 	if (UCharacterMovementComponent* Move = GetCharacterMovement())
  639: 	{
  640: 		Move->SetMovementMode(GetDefaultMovementMode());
  641: 		BaseMaxWalkSpeed = Move->MaxWalkSpeed;
  642: 	}
  643: 
  644: 	if (!bBaseTuningInitialized)
  645: 	{
  646: 		BaseMaxHP = MaxHP;
  647: 		BaseTouchDamageHearts = TouchDamageHearts;
  648: 		BasePointValue = PointValue;
  649: 		BaseArmor = Armor;
  650: 		bBaseTuningInitialized = true;
  651: 	}
  652: 	// Ensure HP is valid on spawn (in case difficulty scaled before BeginPlay).
  653: 	CurrentHP = FMath::Clamp(CurrentHP, 1, MaxHP);
  654: 	ResetFamilyState();
  655: 
  656: 	if (LockIndicatorWidget)
  657: 	{
  658: 		LockIndicatorWidget->InitWidget();
  659: 		LockIndicatorWidget->SetHiddenInGame(true, true);
  660: 		LockIndicatorWidget->SetVisibility(false, true);
  661: 	}
  662: 
  663: 	UCapsuleComponent* Capsule = GetCapsuleComponent();
  664: 	if (Capsule)
  665: 	{
  666: 		Capsule->OnComponentBeginOverlap.AddDynamic(this, &AT66EnemyBase::OnCapsuleBeginOverlap);
  667: 	}
  668: 
  669: 	RefreshCombatHitZoneState();
  670: 
  671: 	// Fail-safe: always make sure the placeholder is visible unless we successfully apply a skeletal mesh.
  672: 	if (VisualMesh)
  673: 	{
  674: 		VisualMesh->SetHiddenInGame(false, true);
  675: 		VisualMesh->SetVisibility(true, true);
  676: 	}
  677: 
  678: 	// Per request: RegularEnemy + Unique enemy use simple placeholder visuals (no FBX).
  679: 	const bool bForcePlaceholder = CharacterVisualID.IsNone() || (CharacterVisualID == FName(TEXT("RegularEnemy")));
  680: 	if (bForcePlaceholder)
  681: 	{
  682: 		if (USkeletalMeshComponent* Skel = GetMesh())
  683: 		{
  684: 			Skel->SetHiddenInGame(true, true);
  685: 			Skel->SetVisibility(false, true);
  686: 		}
  687: #if !UE_BUILD_SHIPPING
  688: 		static int32 LoggedEnemies = 0;
  689: 		if (LoggedEnemies < 12)
  690: 		{
  691: 			++LoggedEnemies;
  692: 			UE_LOG(LogT66Enemy, Verbose, TEXT("EnemyVisuals: %s VisualID=%s UsingPlaceholder=1"), *GetName(), *CharacterVisualID.ToString());
  693: 		}
  694: #endif
  695: 		return;
  696: 	}
  697: 
  698: 	if (TryApplyMobVertexAnimationVisual())
  699: 	{
  700: #if !UE_BUILD_SHIPPING
  701: 		static int32 LoggedVATEnemies = 0;
  702: 		if (LoggedVATEnemies < 12)
  703: 		{
  704: 			++LoggedVATEnemies;
  705: 			UE_LOG(LogT66Enemy, Verbose, TEXT("EnemyVisuals: %s VisualID=%s UsingMobVAT=1"), *GetName(), *CharacterVisualID.ToString());
  706: 		}
  707: #endif

// ...

 1058: void AT66EnemyBase::Tick(float DeltaSeconds)
 1059: {
 1060: 	Super::Tick(DeltaSeconds);
 1061: 	TickMobVertexAnimationState(DeltaSeconds);
 1062: 	if (CurrentHP <= 0) return;

// ...

 1310: void AT66EnemyBase::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
 1311: 	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
 1312: {
 1313: 	if (!OtherActor) return;
 1314: 	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
 1315: 	if (!Hero) return;
 1316: 	if (Hero->IsVehicleMounted()) return;
 1317: 	if (Hero->IsInSafeZone()) return;
 1318: 
 1319: 	UWorld* World = GetWorld();
 1320: 	if (!World) return;
 1321: 	UGameInstance* GI = World->GetGameInstance();
 1322: 	if (!GI) return;
 1323: 	UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
 1324: 	if (!RunState) return;
 1325: 
 1326: 	const float Now = static_cast<float>(World->GetTimeSeconds());
 1327: 	if (Now - LastTouchDamageTime < TouchDamageCooldown) return;
 1328: 
 1329: 	LastTouchDamageTime = Now;
 1330: 	SetMobVertexAnimationClip(T66MobVATClip_AttackCue, 0.25f);
 1331: 	const int32 DamageHP = 20;
 1332: 	RunState->ApplyDamage(DamageHP, this);
 1333: }
 1334: 
 1335: bool AT66EnemyBase::ApplyResolvedDamage(int32 Damage, const bool bCreditHeroKill, FName DamageSourceID, FName EventType)
 1336: {
 1337: 	if (Damage <= 0 || CurrentHP <= 0)
 1338: 	{
 1339: 		return false;
 1340: 	}
 1341: 
 1342: 	const float EffectiveArmor = GetEffectiveArmor();
 1343: 	const int32 ReducedDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(Damage) * (1.f - EffectiveArmor)));
 1344: 	const FName SourceID = DamageSourceID.IsNone() ? UT66DamageLogSubsystem::SourceID_AutoAttack : DamageSourceID;
 1345: 
 1346: 	if (UWorld* World = GetWorld())
 1347: 	{
 1348: 		if (UGameInstance* GI = World->GetGameInstance())
 1349: 		{
 1350: 			if (bCreditHeroKill)
 1351: 			{
 1352: 				if (UT66DamageLogSubsystem* DamageLog = GI->GetSubsystem<UT66DamageLogSubsystem>())
 1353: 				{
 1354: 					DamageLog->RecordDamageDealt(SourceID, ReducedDamage);
 1355: 				}
 1356: 			}
 1357: 
 1358: 			if (UT66FloatingCombatTextSubsystem* FloatingText = GI->GetSubsystem<UT66FloatingCombatTextSubsystem>())
 1359: 			{
 1360: 				FloatingText->ShowDamageNumber(this, ReducedDamage, EventType);
 1361: 			}
 1362: 		}
 1363: 	}
 1364: 
 1365: 	CurrentHP = FMath::Max(0, CurrentHP - ReducedDamage);
 1366: 	if (CurrentHP <= 0)
 1367: 	{
 1368: 		if (UWorld* World = GetWorld())
 1369: 		{
 1370: 			if (bCreditHeroKill)
 1371: 			{
 1372: 				if (UGameInstance* GI = World->GetGameInstance())
 1373: 				{
 1374: 					if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
 1375: 					{
 1376: 						RunState->NotifyEnemyKilledByHero();
 1377: 					}
 1378: 				}
 1379: 			}
 1380: 
 1381: 			UT66CombatComponent::SpawnDeathBurstAtLocation(World, GetActorLocation(), 16, 60.f);
 1382: 		}
 1383: 
 1384: 		T66PlayEnemyAudioEvent(this, TEXT("Combat.Enemy.Death"), FName(TEXT("Combat.Enemy.Death")));
 1385: 		SetMobVertexAnimationClip(T66MobVATClip_Death, 0.45f);
 1386: 		OnDeath();
 1387: 		return true;
 1388: 	}
 1389: 
 1390: 	T66PlayEnemyAudioEvent(this, TEXT("Combat.Hit.Enemy"), FName(TEXT("Combat.Hit.Enemy")));
 1391: 	SetMobVertexAnimationClip(T66MobVATClip_HitReact, 0.16f);
 1392: 	return false;

// ...

 1575: void AT66EnemyBase::OnDeath()
 1576: {
 1577: 	SetLockedIndicator(false);
 1578: 
 1579: 	UWorld* World = GetWorld();
 1580: 	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
 1581: 	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
 1582: 	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
 1583: 	if (RunState)
 1584: 	{
 1585: 		const int32 AwardPoints = FMath::Max(0, ResolvedScoreAward);
 1586: 		int32 AwardXP = XPValue;
 1587: 		if (AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr)
 1588: 		{
 1589: 			if (GameMode->IsUsingTowerMainMapLayout())
 1590: 			{
 1591: 				AwardXP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(XPValue) * 0.5f));
 1592: 			}
 1593: 		}
 1594: 		RunState->AddEnemyKillScore(AwardPoints);
 1595: 		RunState->AddHeroXP(AwardXP);
 1596: 		RunState->AddStructuredEvent(ET66RunEventType::EnemyKilled, FString::Printf(TEXT("Score=%d,XP=%d"), AwardPoints, AwardXP));
 1597: 	}
 1598: 	if (Achievements)
 1599: 	{
 1600: 		Achievements->NotifyEnemyKilled(1);
 1601: 		// Lab unlock: mark this enemy type as unlocked for The Lab.
 1602: 		if (!CharacterVisualID.IsNone())
 1603: 		{
 1604: 			Achievements->AddLabUnlockedEnemy(CharacterVisualID);
 1605: 		}
 1606: 	}
 1607: 
 1608: 	if (OwningDirector)
 1609: 	{
 1610: 		OwningDirector->NotifyEnemyDied(this);
```

### Source/T66/Data/T66DataTypes.h
```cpp
 1720:  * Character visual mapping row.
 1721:  * Maps a stable gameplay ID (HeroID / CompanionID / NPCID / BossID / EnemyVisualID) to imported character assets.
 1722:  *
 1723:  * - SkeletalMesh: the mesh to assign to a USkeletalMeshComponent
 1724:  * - StaticMesh: optional unrigged mesh to assign to a UStaticMeshComponent when no rig is available
 1725:  * - LoopingAnimation: walk animation (used when moving slowly)
 1726:  * - AlertAnimation: alert/stand animation (e.g. hero/companion selection preview)
 1727:  * - RunAnimation: run animation (used when moving fast); if unset, walk is used for all movement
 1728:  * - RollAnimation: one-shot forward roll animation
 1729:  * - MeshRelative*: applied directly to the target component
 1730:  */
 1731: USTRUCT(BlueprintType)
 1732: struct T66_API FT66CharacterVisualRow : public FTableRowBase
 1733: {
 1734: 	GENERATED_BODY()
 1735: 
 1736: 	/** Primary mesh to display. */
 1737: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1738: 	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
 1739: 
 1740: 	/** Optional unrigged/static mesh to display when no skeletal mesh is available yet. */
 1741: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1742: 	TSoftObjectPtr<UStaticMesh> StaticMesh;
 1743: 
 1744: 	/** Pixelated texture used by QuadRetro static meshes at runtime. */
 1745: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1746: 	TSoftObjectPtr<UTexture2D> PixelatedTextureAssetPath;
 1747: 
 1748: 	/** Walk animation (looping). Used when moving below run threshold. */
 1749: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1750: 	TSoftObjectPtr<UAnimationAsset> LoopingAnimation;
 1751: 
 1752: 	/** Alert/stand animation (e.g. hero/companion selection preview). If set, used in preview instead of walk. */
 1753: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1754: 	TSoftObjectPtr<UAnimationAsset> AlertAnimation;
 1755: 
 1756: 	/** Run animation (looping). Used when moving above run threshold. If unset, walk is used for all movement. */
 1757: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1758: 	TSoftObjectPtr<UAnimationAsset> RunAnimation;
 1759: 
 1760: 	/** Roll animation (one-shot). Used when the player triggers forward roll. */
 1761: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1762: 	TSoftObjectPtr<UAnimationAsset> RollAnimation;
 1763: 
 1764: 	/** Relative location applied to the target mesh component. */
 1765: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1766: 	FVector MeshRelativeLocation = FVector(0.f, 0.f, -88.f);
 1767: 
 1768: 	/** Relative rotation applied to the target mesh component. */
 1769: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1770: 	FRotator MeshRelativeRotation = FRotator(0.f, -90.f, 0.f);
 1771: 
 1772: 	/** Relative scale applied to the target mesh component. */
 1773: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1774: 	FVector MeshRelativeScale = FVector(1.f, 1.f, 1.f);
 1775: 
 1776: 	/** If true and LoopingAnimation is set, play it in a loop. */
 1777: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1778: 	bool bLoopAnimation = true;
 1779: 
 1780: 	/**
 1781: 	 * If true, the system will auto-adjust the target component's Z so the mesh bounds bottom sits at the actor origin.
 1782: 	 * Intended for non-capsule actors (companions, house NPCs) where the actor origin is treated as "ground contact".
 1783: 	 */
 1784: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1785: 	bool bAutoGroundToActorOrigin = false;
 1786: 
 1787: 	FT66CharacterVisualRow() = default;
 1788: };
 1789: 
 1790: /**
 1791:  * Static-mesh vertex-animation mapping for mob visuals.
 1792:  *
 1793:  * These rows intentionally live outside CharacterVisuals so the existing
 1794:  * unanimated static rows remain the fallback while VAT assets are validated.
 1795:  */
 1796: USTRUCT(BlueprintType)
 1797: struct T66_API FT66MobVertexAnimationRow : public FTableRowBase
 1798: {
 1799: 	GENERATED_BODY()
 1800: 
 1801: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
 1802: 	FName EnemyID;
 1803: 
 1804: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1805: 	TSoftObjectPtr<UStaticMesh> StaticMesh;
 1806: 
 1807: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1808: 	TSoftObjectPtr<UMaterialInterface> Material;
 1809: 
 1810: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1811: 	TSoftObjectPtr<UTexture2D> PixelatedTextureAssetPath;
 1812: 
 1813: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT")
 1814: 	TSoftObjectPtr<UTexture2D> PositionTexture;
 1815: 
 1816: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT")
 1817: 	TSoftObjectPtr<UTexture2D> NormalTexture;
 1818: 
 1819: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1820: 	FVector MeshRelativeLocation = FVector(0.f, 0.f, -88.f);
 1821: 
 1822: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1823: 	FRotator MeshRelativeRotation = FRotator(0.f, -90.f, 0.f);
 1824: 
 1825: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
 1826: 	FVector MeshRelativeScale = FVector(1.f, 1.f, 1.f);
 1827: 
 1828: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT")
 1829: 	bool bEnabled = false;
 1830: 
 1831: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT")
 1832: 	float SampleRate = 30.f;
 1833: 
 1834: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT")
 1835: 	int32 NumFrames = 0;
 1836: 
 1837: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT")
 1838: 	int32 RowsPerFrame = 0;
 1839: 
 1840: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT")
 1841: 	FVector MinBBox = FVector::ZeroVector;
 1842: 
 1843: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT")
 1844: 	FVector SizeBBox = FVector::OneVector;
 1845: 
 1846: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1847: 	int32 IdleStartFrame = 0;
 1848: 
 1849: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1850: 	int32 IdleEndFrame = 0;
 1851: 
 1852: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1853: 	float IdlePlayRate = 1.f;
 1854: 
 1855: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1856: 	int32 MoveStartFrame = 0;
 1857: 
 1858: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1859: 	int32 MoveEndFrame = 0;
 1860: 
 1861: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1862: 	float MovePlayRate = 1.f;
 1863: 
 1864: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1865: 	int32 AttackCueStartFrame = 0;
 1866: 
 1867: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1868: 	int32 AttackCueEndFrame = 0;
 1869: 
 1870: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1871: 	float AttackCuePlayRate = 1.f;
 1872: 
 1873: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1874: 	int32 HitReactStartFrame = 0;
 1875: 
 1876: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1877: 	int32 HitReactEndFrame = 0;
 1878: 
 1879: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1880: 	float HitReactPlayRate = 1.f;
 1881: 
 1882: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1883: 	int32 DeathStartFrame = 0;
 1884: 
 1885: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1886: 	int32 DeathEndFrame = 0;
 1887: 
 1888: 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT|Clips")
 1889: 	float DeathPlayRate = 1.f;
 1890: 
 1891: 	FT66MobVertexAnimationRow() = default;
 1892: };
 1893:
```

### Source/T66/Gameplay/T66CompanionBase.cpp
```cpp
  218: 			const bool bUseAlertAnimation = bIsPreviewMode;
  219: 			const bool bIsPreviewContext = bIsPreviewMode;
  220: 			bUsingCharacterVisual = Visuals->ApplyCharacterVisual(VisualID, SkeletalMesh, PlaceholderMesh, true, bUseAlertAnimation, bIsPreviewContext);
  221: 			if (!bUsingCharacterVisual && SkeletalMesh)
  222: 			{
  223: 				SkeletalMesh->SetVisibility(false, true);
  224: 			}
  225: 			else if (!bIsPreviewMode)
  226: 			{
  227: 				UAnimationAsset* WalkRaw = nullptr;
  228: 				UAnimationAsset* RunRaw = nullptr;
  229: 				UAnimationAsset* AlertRaw = nullptr;
  230: 				UAnimationAsset* UnusedRollRaw = nullptr;
  231: 				Visuals->GetMovementAnimsForVisual(VisualID, WalkRaw, RunRaw, AlertRaw, UnusedRollRaw);
  232: 				CachedWalkAnim = WalkRaw;
  233: 				CachedRunAnim = RunRaw;
  234: 				CachedAlertAnim = AlertRaw;
  235: 				// Match hero: ApplyCharacterVisual just played LoopingAnim (walk), so store Walk (1).
  236: 				// Then when hero is Idle (0) we see a state change and play Alert.

// ...

  313: 	if (bUsingCharacterVisual && SkeletalMesh && SkeletalMesh->IsVisible() && (CachedAlertAnim || CachedRunAnim || CachedWalkAnim))
  314: 	{
  315: 		if (CachedHeroSpeedSubsystem)
  316: 		{
  317: 			const int32 NewState = CachedHeroSpeedSubsystem->GetMovementAnimState(); // 0=Idle, 2=Run
  318: 			if (NewState != LastMovementAnimState)
  319: 			{
  320: 				LastMovementAnimState = static_cast<uint8>(NewState);
  321: 				UAnimationAsset* ToPlay = nullptr;
  322: 				if (NewState == 0)
  323: 					ToPlay = CachedAlertAnim;
  324: 				else
  325: 					ToPlay = CachedRunAnim ? CachedRunAnim : CachedWalkAnim;
  326: 				if (ToPlay)
  327: 					SkeletalMesh->PlayAnimation(ToPlay, true);
  328: 			}
```

### Source/T66/Gameplay/T66TutorialGuideCompanion.cpp
```cpp
   89: void AT66TutorialGuideCompanion::UpdateGuideAnimation(const bool bIsMoving)
   90: {
   91: 	if (!bUsingCharacterVisual || !SkeletalMesh || !SkeletalMesh->IsVisible())
   92: 	{
   93: 		return;
   94: 	}
   95: 
   96: 	const uint8 DesiredState = bIsMoving ? 2 : 0;
   97: 	if (DesiredState == LastMovementAnimState)
   98: 	{
   99: 		return;
  100: 	}
  101: 
  102: 	LastMovementAnimState = DesiredState;
  103: 	UAnimationAsset* DesiredAnimation = bIsMoving
  104: 		? (CachedRunAnim ? CachedRunAnim : CachedWalkAnim)
  105: 		: CachedAlertAnim;
  106: 	if (DesiredAnimation)
  107: 	{
  108: 		SkeletalMesh->PlayAnimation(DesiredAnimation, true);
  109: 	}
```

### Source/T66/Gameplay/T66BossBase.cpp
```cpp
  236: 	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
  237: 	VisualMesh->SetupAttachment(RootComponent);
  238: 	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  239: 	// Align primitive mesh to ground when capsule is grounded:
  240: 	// capsule half-height~88, sphere half-height=50*6=300 => relative Z = 300 - 88 = 212.
  241: 	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 212.f));
  242: 
  243: 	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
  244: 	{
  245: 		VisualMesh->SetStaticMesh(Sphere);
  246: 		VisualMesh->SetRelativeScale3D(FVector(6.f, 6.f, 6.f)); // very large sphere
  247: 	}
  248: 	if (UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
  249: 	{
  250: 		Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.9f, 0.05f, 0.05f, 1.f));
  251: 	}
  252: 
  253: 	AttackPrimaryColor = FLinearColor(0.95f, 0.16f, 0.12f, 1.f);
  254: 	AttackSecondaryColor = T66MakeAttackSecondaryColor(AttackPrimaryColor);
  255: 
  256: 	// Prepare built-in SkeletalMeshComponent for imported models.
  257: 	if (USkeletalMeshComponent* Skel = GetMesh())
  258: 	{
  259: 		Skel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  260: 		Skel->SetVisibility(false, true); // shown only when a character visual mapping exists
  261: 	}
  262: 
  263: 	EnsureDefaultBossPartDefinitions();
  264: }
  265: 
  266: void AT66BossBase::AssignBossPartDefinitionsForProfile(const ET66BossPartProfile InProfile)
  267: {

// ...

  720: 		Move->MaxWalkSpeed = BossData.MoveSpeed;
  721: 		BaseMoveSpeed = BossData.MoveSpeed;
  722: 	}
  723: 	if (VisualMesh)
  724: 	{
  725: 		if (UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
  726: 		{
  727: 			Mat->SetVectorParameterValue(TEXT("BaseColor"), BossData.PlaceholderColor);
  728: 		}
  729: 	}
  730: 
  731: 	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
  732: 	{
  733: 		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
  734: 		{
  735: 			const FName BossVisualID = BossID.IsNone() ? FName(TEXT("Boss")) : BossID;
  736: 			bool bApplied = Visuals->ApplyCharacterVisual(BossVisualID, GetMesh(), nullptr, true, false, false, VisualMesh);
  737: 			if (!bApplied && BossVisualID != FName(TEXT("Boss")))
  738: 			{
  739: 				bApplied = Visuals->ApplyCharacterVisual(FName(TEXT("Boss")), GetMesh(), VisualMesh, true);
  740: 			}
  741: 			if (USkeletalMeshComponent* SkelMesh = GetMesh())
  742: 			{
  743: 				if (bApplied && SkelMesh->IsVisible())
  744: 				{
  745: 					SkelMesh->SetRelativeScale3D(SkelMesh->GetRelativeScale3D() * 3.0f);
  746: 				}
  747: 				else
  748: 				{
  749: 					SkelMesh->SetVisibility(false, true);
  750: 				}
  751: 			}
  752: 		}
  753: 	}
  754: 
  755: 	// Apply current run difficulty (boss is usually dormant until awaken).
  756: 	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
  757: 	{
  758: 		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
  759: 		{
  760: 			ApplyDifficultyScalar(RunState->GetDifficultyScalar());
  761: 		}
  762: 	}
  763: 	else
  764: 	{
  765: 		RebuildBossPartState(false);
  766: 	}
  767: }
```

### Source/T66/Gameplay/T66PlayerController_Overlays.cpp
```cpp
  600: 			};
  601: 
  602: 			const FVector Origin = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
  603: 			FActorSpawnParameters SpawnParams;
  604: 			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  605: 
  606: 			for (int32 Index = 0; Index < EasyMobIDs.Num(); ++Index)
  607: 			{
  608: 				const int32 Row = Index / 5;
  609: 				const int32 Col = Index % 5;
  610: 				const FVector QAEnemyLocation = Origin + FVector(480.f + static_cast<float>(Row) * 300.f, (static_cast<float>(Col) - 4.f) * 220.f, 0.f);
  611: 				FRotator QAEnemyRotation = (Origin - QAEnemyLocation).Rotation();
  612: 				QAEnemyRotation.Pitch = 0.f;
  613: 				QAEnemyRotation.Roll = 0.f;
  614: 				SpawnParams.Name = FName(*FString::Printf(TEXT("T66MobVATQA_%s"), *EasyMobIDs[Index].ToString()));
  615: 
  616: 				AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(
  617: 					AT66EnemyBase::StaticClass(),
  618: 					QAEnemyLocation,
  619: 					QAEnemyRotation,
  620: 					SpawnParams);
  621: 				if (!Enemy)
  622: 				{
  623: 					UE_LOG(LogTemp, Error, TEXT("Mob VAT QA failed to spawn %s"), *EasyMobIDs[Index].ToString());
  624: 					continue;
  625: 				}
  626: 
  627: 				Enemy->Tags.AddUnique(FName(TEXT("T66Automation_EasyMobVATQA")));
  628: 				Enemy->SetActorEnableCollision(false);
  629: 				Enemy->MaxHP = 20000;
  630: 				Enemy->CurrentHP = 20000;
  631: 				Enemy->TouchDamageHearts = 0;
  632: 				Enemy->PointValue = 0;
  633: 				Enemy->XPValue = 0;
  634: 				Enemy->bDropsLoot = false;
  635: 				Enemy->OwningDirector = nullptr;
  636: 				Enemy->ConfigureAsMob(EasyMobIDs[Index]);
  637: 				if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
  638: 				{
  639: 					Movement->StopMovementImmediately();
  640: 					Movement->DisableMovement();
  641: 				}
  642: 
  643: 				const FName ClipName = ClipNames[Index % ClipNames.Num()];
  644: 				Enemy->ForceMobVertexAnimationClipForAutomation(ClipName, 30.f);
  645: 				UE_LOG(LogTemp, Display, TEXT("Mob VAT QA spawned %s clip=%s location=%s"),
  646: 					*EasyMobIDs[Index].ToString(),
  647: 					*ClipName.ToString(),
```

### Model Generation/Rigging and Animation/Tools/import_easy_mob_vat_to_unreal.py
```python
   37: DEST_ROOT = os.environ.get("T66_EASY_MOB_VAT_DEST_ROOT", "/Game/Characters/MobsVAT")
   38: MASTER_MATERIAL_DIR = "/Game/Materials"
   39: MASTER_MATERIAL_NAME = "M_EasyMobVAT_Unlit_UV2"
   40: MASTER_MATERIAL_PATH = f"{MASTER_MATERIAL_DIR}/{MASTER_MATERIAL_NAME}"
   41: 
   42: SAMPLE_DATA_ASSET = "/AnimToTexture/Characters/Mannequin/Data/DA_VertexAnimation.DA_VertexAnimation"
   43: SAMPLE_POSITION_TEXTURE = "/AnimToTexture/Characters/Mannequin/Textures/VertexAnimation/TX_VertexPosition.TX_VertexPosition"
   44: SAMPLE_NORMAL_TEXTURE = "/AnimToTexture/Characters/Mannequin/Textures/VertexAnimation/TX_VertexNormal.TX_VertexNormal"

// ...

  330: def ensure_master_material():
  331:     existing = unreal.EditorAssetLibrary.load_asset(MASTER_MATERIAL_PATH)
  332:     if existing and isinstance(existing, unreal.Material):
  333:         unreal.EditorAssetLibrary.delete_asset(MASTER_MATERIAL_PATH)
  334: 
  335:     ensure_directory(MASTER_MATERIAL_DIR)
  336:     material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
  337:         MASTER_MATERIAL_NAME,
  338:         MASTER_MATERIAL_DIR,
  339:         unreal.Material,
  340:         unreal.MaterialFactoryNew(),
  341:     )
  342:     if not material:
  343:         raise RuntimeError(f"Could not create {MASTER_MATERIAL_PATH}")
  344: 
  345:     mel = unreal.MaterialEditingLibrary
  346:     safe_set(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
  347:     safe_set(material, "blend_mode", unreal.BlendMode.BLEND_OPAQUE)
  348:     safe_set(material, "two_sided", True)
  349:     safe_set(material, "used_with_instanced_static_meshes", True)
  350:     safe_set(material, "used_with_nanite", True)
  351: 
  352:     base_tex = mel.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -760, -180)
  353:     tint = mel.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -760, 20)
  354:     brightness = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -760, 220)
  355:     tint_mul = mel.create_material_expression(material, unreal.MaterialExpressionMultiply, -470, -80)
  356:     bright_mul = mel.create_material_expression(material, unreal.MaterialExpressionMultiply, -180, -80)
  357: 
  358:     uv1 = mel.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate, -760, 520)
  359:     position_tex = mel.create_material_expression(material, unreal.MaterialExpressionTextureObjectParameter, -760, 700)
  360:     frame = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -760, 880)
  361:     rows = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -760, 1040)
  362:     min_bbox = mel.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -760, 1200)
  363:     size_bbox = mel.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -760, 1360)
  364:     custom = mel.create_material_expression(material, unreal.MaterialExpressionCustom, -260, 860)
  365: 
  366:     base_tex.set_editor_property("parameter_name", "BaseColorTexture")
  367:     try:
  368:         base_tex.set_editor_property("texture", load_asset("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"))
  369:     except Exception:
  370:         pass
  371:     tint.set_editor_property("parameter_name", "Tint")
  372:     tint.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
  373:     brightness.set_editor_property("parameter_name", "Brightness")
  374:     brightness.set_editor_property("default_value", 0.8)
  375: 
  376:     uv1.set_editor_property("coordinate_index", 2)
  377:     position_tex.set_editor_property("parameter_name", "PositionTexture")
  378:     position_tex.set_editor_property("texture", load_asset(SAMPLE_POSITION_TEXTURE))
  379:     frame.set_editor_property("parameter_name", "Frame")
  380:     frame.set_editor_property("default_value", 0.0)
  381:     rows.set_editor_property("parameter_name", "RowsPerFrame")
  382:     rows.set_editor_property("default_value", 1.0)
  383:     min_bbox.set_editor_property("parameter_name", "MinBBox")
  384:     min_bbox.set_editor_property("default_value", unreal.LinearColor(0.0, 0.0, 0.0, 0.0))
  385:     size_bbox.set_editor_property("parameter_name", "SizeBBox")
  386:     size_bbox.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 0.0))
  387: 
  388:     custom.set_editor_property("description", "EasyMobVAT_WPO")
  389:     custom.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
  390:     custom.set_editor_property("code", "\n".join([
  391:         "uint tex_width;",
  392:         "uint tex_height;",
  393:         "PositionTexture.GetDimensions(tex_width, tex_height);",
  394:         "float frame_index = floor(Frame + 0.0001);",
  395:         "float2 sample_uv = UV1;",
  396:         "sample_uv.y += (frame_index * RowsPerFrame) / max(1.0, (float)tex_height);",
  397:         "float3 packed_delta = Texture2DSample(PositionTexture, PositionTextureSampler, sample_uv).rgb;",
  398:         "float3 local_delta = packed_delta * SizeBBox.rgb + MinBBox.rgb;",
  399:         "return TransformLocalVectorToWorld(Parameters, local_delta);",
  400:     ]))
  401:     inputs = []
  402:     for input_name in ("UV1", "PositionTexture", "Frame", "RowsPerFrame", "MinBBox", "SizeBBox"):
  403:         custom_input = unreal.CustomInput()
  404:         custom_input.set_editor_property("input_name", input_name)
  405:         inputs.append(custom_input)
  406:     custom.set_editor_property("inputs", inputs)
  407: 
  408:     mel.connect_material_expressions(base_tex, "RGB", tint_mul, "A")
  409:     mel.connect_material_expressions(tint, "", tint_mul, "B")
  410:     mel.connect_material_expressions(tint_mul, "", bright_mul, "A")
  411:     mel.connect_material_expressions(brightness, "", bright_mul, "B")
  412:     mel.connect_material_property(bright_mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
  413:     try:
  414:         mel.connect_material_property(bright_mul, "", unreal.MaterialProperty.MP_BASE_COLOR)
  415:     except Exception:
  416:         pass
  417: 
  418:     mel.connect_material_expressions(uv1, "", custom, "UV1")
  419:     mel.connect_material_expressions(position_tex, "", custom, "PositionTexture")
  420:     mel.connect_material_expressions(frame, "", custom, "Frame")
  421:     mel.connect_material_expressions(rows, "", custom, "RowsPerFrame")
  422:     mel.connect_material_expressions(min_bbox, "", custom, "MinBBox")
  423:     mel.connect_material_expressions(size_bbox, "", custom, "SizeBBox")
  424:     mel.connect_material_property(custom, "", unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)
  425: 
  426:     try:
  427:         mel.layout_material_expressions(material)
  428:     except Exception:
  429:         pass
  430:     mel.recompile_material(material)
  431:     unreal.EditorAssetLibrary.save_asset(MASTER_MATERIAL_PATH)
  432:     return material
  433: 
  434: 

// ...

  476: def configure_material_instance(material, base_texture, data_asset, position_texture, normal_texture):
  477:     mel = unreal.MaterialEditingLibrary
  478:     for param_name in ("BaseColorTexture", "EmissiveTexture", "DiffuseColorMap"):
  479:         try:
  480:             mel.set_material_instance_texture_parameter_value(material, param_name, base_texture)
  481:         except Exception:
  482:             pass
  483:     for param_name, texture in (("PositionTexture", position_texture), ("NormalTexture", normal_texture)):
  484:         try:
  485:             mel.set_material_instance_texture_parameter_value(material, param_name, texture)
  486:         except Exception:
  487:             pass
  488:     try:
  489:         unreal.AnimToTextureBPLibrary.update_material_instance_from_data_asset(data_asset, material)
  490:     except Exception as exc:
  491:         warn(f"AnimToTexture material update skipped for {material.get_name()}: {exc}")
  492:     try:
  493:         mel.set_material_instance_scalar_parameter_value(material, "Brightness", 0.8)
  494:         mel.set_material_instance_vector_parameter_value(material, "Tint", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
  495:         mel.update_material_instance(material)
  496:     except Exception:
  497:         pass
  498:     unreal.EditorAssetLibrary.save_loaded_asset(material)
  499: 
  500: 

// ...

  536:     data_asset_path = f"{dest_dir}/DA_EasyMobVAT_{enemy_id}"
  537:     data_asset = create_data_asset_duplicate(data_asset_path)
  538:     data_asset.set_editor_property("mode", unreal.AnimToTextureMode.VERTEX)
  539:     data_asset.set_editor_property("precision", unreal.AnimToTexturePrecision.SIXTEEN_BITS)
  540:     data_asset.set_editor_property("skeletal_mesh", skeletal_mesh)
  541:     data_asset.set_editor_property("skeletal_lod_index", 0)
  542:     data_asset.set_editor_property("static_mesh", static_mesh)
  543:     data_asset.set_editor_property("static_lod_index", 0)
  544:     data_asset.set_editor_property("uv_channel", 2)
  545:     data_asset.set_editor_property("max_width", 4096)
  546:     data_asset.set_editor_property("max_height", 4096)
  547:     data_asset.set_editor_property("enforce_power_of_two", False)
  548:     data_asset.set_editor_property("sample_rate", 30.0)
  549:     data_asset.set_editor_property("auto_play", False)
  550:     data_asset.set_editor_property("frame", 0)
  551:     data_asset.set_editor_property("vertex_position_texture", position_texture)
  552:     data_asset.set_editor_property("vertex_normal_texture", normal_texture)
  553:     data_asset.set_editor_property("anim_sequences", make_anim_sequence_infos(anim_assets))
  554:     unreal.EditorAssetLibrary.save_loaded_asset(data_asset)
  555: 
  556:     if not unreal.AnimToTextureBPLibrary.animation_to_texture(data_asset):
  557:         raise RuntimeError(f"AnimToTexture bake failed for {enemy_id}")
  558: 
  559:     material_path = f"{dest_dir}/MI_EasyMobVAT_{enemy_id}"
  560:     material = create_or_load_material_instance(material_path, master_material)
```

## D. Asset Reality Vs. Plan

### Repo Counts
| Data Source | Current Rows |
| --- | --- |
| Heroes.csv | 12 |
| Companions.csv | 24 |
| Enemies.csv | 50 |
| Bosses.csv | 23 |
| CharacterVisuals.csv | 164 |
| MobVertexAnimations.csv | 10 |
Planned final counts: not found in repo. Current counts above are repo-confirmed CSV counts only.

### Heroes
- Hero data rows: 12.
- CharacterVisuals hero variant rows: 37.
- Hero visual status counts: skeletal + animation rows: 11; skeletal mesh, no animations listed: 3; static-only: 23.
- Current main Royal Chad row (`Hero_1_Chad`) is skeletal + animated with walk/idle/jump/roll animation assets.
- Most base Chad/Stacy rows remain static-only; Beachgoer Chad rows 1-9 have skeletal animation entries; Beachgoer Chad rows 10-12 have skeletal meshes with no animation fields listed.
| HeroID | Display | Visual Variant Rows | Status Summary | Variants |
| --- | --- | --- | --- | --- |
| Hero_1 | Royal Chad | 4 | skeletal + animation rows: 3; static-only: 1 | Hero_1_Chad, Hero_1_Chad_QuadRetroUALQA, Hero_1_Chad_Beachgoer, Hero_1_Stacy |
| Hero_2 | Chinese Chad | 3 | skeletal + animation rows: 1; static-only: 2 | Hero_2_Chad, Hero_2_Chad_Beachgoer, Hero_2_Stacy |
| Hero_3 | Boxer Chad | 3 | skeletal + animation rows: 1; static-only: 2 | Hero_3_Chad, Hero_3_Chad_Beachgoer, Hero_3_Stacy |
| Hero_4 | Founding Chad | 3 | skeletal + animation rows: 1; static-only: 2 | Hero_4_Chad, Hero_4_Chad_Beachgoer, Hero_4_Stacy |
| Hero_5 | Robo Chad | 3 | skeletal + animation rows: 1; static-only: 2 | Hero_5_Chad, Hero_5_Chad_Beachgoer, Hero_5_Stacy |
| Hero_6 | Billy Chad | 3 | skeletal + animation rows: 1; static-only: 2 | Hero_6_Chad, Hero_6_Chad_Beachgoer, Hero_6_Stacy |
| Hero_7 | Rabbit Chad | 3 | skeletal + animation rows: 1; static-only: 2 | Hero_7_Chad, Hero_7_Chad_Beachgoer, Hero_7_Stacy |
| Hero_8 | CS Chad | 3 | skeletal + animation rows: 1; static-only: 2 | Hero_8_Chad, Hero_8_Chad_Beachgoer, Hero_8_Stacy |
| Hero_9 | Goblino Chad | 3 | skeletal + animation rows: 1; static-only: 2 | Hero_9_Chad, Hero_9_Chad_Beachgoer, Hero_9_Stacy |
| Hero_10 | Monotone Chad | 3 | skeletal mesh, no animations listed: 1; static-only: 2 | Hero_10_Chad, Hero_10_Chad_Beachgoer, Hero_10_Stacy |
| Hero_11 | Bald Chad | 3 | skeletal mesh, no animations listed: 1; static-only: 2 | Hero_11_Chad, Hero_11_Chad_Beachgoer, Hero_11_Stacy |
| Hero_12 | Roach Chad | 3 | skeletal mesh, no animations listed: 1; static-only: 2 | Hero_12_Chad, Hero_12_Chad_Beachgoer, Hero_12_Stacy |

### Companions
- Companion data rows: 24.
- CharacterVisuals companion variant rows: 48.
- Companion visual status counts: skeletal + animation rows: 48.
- All companion visual rows currently have skeletal meshes and animation fields listed, but many rows reuse a smaller set of companion meshes/animations rather than unique rigs per companion.
| CompanionID | Display | Visual Variant Rows | Status Summary | Variants |
| --- | --- | --- | --- | --- |
| Companion_01 | Aria | 2 | skeletal + animation rows: 2 | Companion_01, Companion_01_Beachgoer |
| Companion_02 | Blaze | 2 | skeletal + animation rows: 2 | Companion_02, Companion_02_Beachgoer |
| Companion_03 | Bone | 2 | skeletal + animation rows: 2 | Companion_03, Companion_03_Beachgoer |
| Companion_04 | Cinder | 2 | skeletal + animation rows: 2 | Companion_04, Companion_04_Beachgoer |
| Companion_05 | Coral | 2 | skeletal + animation rows: 2 | Companion_05, Companion_05_Beachgoer |
| Companion_06 | Dawn | 2 | skeletal + animation rows: 2 | Companion_06, Companion_06_Beachgoer |
| Companion_07 | Echo | 2 | skeletal + animation rows: 2 | Companion_07, Companion_07_Beachgoer |
| Companion_08 | Ember | 2 | skeletal + animation rows: 2 | Companion_08, Companion_08_Beachgoer |
| Companion_09 | Fang | 2 | skeletal + animation rows: 2 | Companion_09, Companion_09_Beachgoer |
| Companion_10 | Frost | 2 | skeletal + animation rows: 2 | Companion_10, Companion_10_Beachgoer |
| Companion_11 | Hex | 2 | skeletal + animation rows: 2 | Companion_11, Companion_11_Beachgoer |
| Companion_12 | Ivy | 2 | skeletal + animation rows: 2 | Companion_12, Companion_12_Beachgoer |
| Companion_13 | Jinx | 2 | skeletal + animation rows: 2 | Companion_13, Companion_13_Beachgoer |
| Companion_14 | Luna | 2 | skeletal + animation rows: 2 | Companion_14, Companion_14_Beachgoer |
| Companion_15 | Mercy | 2 | skeletal + animation rows: 2 | Companion_15, Companion_15_Beachgoer |
| Companion_16 | Nova | 2 | skeletal + animation rows: 2 | Companion_16, Companion_16_Beachgoer |
| Companion_17 | Nyx | 2 | skeletal + animation rows: 2 | Companion_17, Companion_17_Beachgoer |
| Companion_18 | Petra | 2 | skeletal + animation rows: 2 | Companion_18, Companion_18_Beachgoer |
| Companion_19 | Pixel | 2 | skeletal + animation rows: 2 | Companion_19, Companion_19_Beachgoer |
| Companion_20 | Raven | 2 | skeletal + animation rows: 2 | Companion_20, Companion_20_Beachgoer |
| Companion_21 | Sage | 2 | skeletal + animation rows: 2 | Companion_21, Companion_21_Beachgoer |
| Companion_22 | Sera | 2 | skeletal + animation rows: 2 | Companion_22, Companion_22_Beachgoer |
| Companion_23 | Storm | 2 | skeletal + animation rows: 2 | Companion_23, Companion_23_Beachgoer |
| Companion_24 | Vex | 2 | skeletal + animation rows: 2 | Companion_24, Companion_24_Beachgoer |

### Mobs / Enemies
- Enemy data rows: 50 total, 10 per difficulty bucket.
- Easy/Dungeon has 10 VAT-enabled rows in `MobVertexAnimations.csv`.
- Difficulty 2+ mobs are present in `Enemies.csv` and `CharacterVisuals.csv` as static-only rows; no Medium/Hard/VeryHard/Impossible VAT rows exist in `MobVertexAnimations.csv`.
- Easy: VAT enabled + static fallback row: 10.
- Medium: static-only: 10.
- Hard: static-only: 10.
- VeryHard: static-only: 10.
- Impossible: static-only: 10.
| EnemyID | Display | Difficulty | Theme | Family | Archetype | Visual/Animation State | VAT Row |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Slime | Slime | Easy | Dungeon | Melee | Melee | VAT enabled + static fallback row | yes |
| BoneWalker | Bone Walker | Easy | Dungeon | Melee | Melee | VAT enabled + static fallback row | yes |
| RatPack | Rat Pack | Easy | Dungeon | Rush | Rush | VAT enabled + static fallback row | yes |
| CaveBat | Cave Bat | Easy | Dungeon | Flying | Flying | VAT enabled + static fallback row | yes |
| HexSlinger | Hex Slinger | Easy | Dungeon | Ranged | Ranged | VAT enabled + static fallback row | yes |
| TombSpider | Tomb Spider | Easy | Dungeon | Melee | Melee | VAT enabled + static fallback row | yes |
| StoneSentinel | Stone Sentinel | Easy | Dungeon | Ranged | Turret | VAT enabled + static fallback row | yes |
| MimicLure | Mimic Lure | Easy | Dungeon | Rush | Exploder | VAT enabled + static fallback row | yes |
| BoneConjurer | Bone Conjurer | Easy | Dungeon | Ranged | Necromancer | VAT enabled + static fallback row | yes |
| CryptWraith | Crypt Wraith | Easy | Dungeon | Melee | Stutterer | VAT enabled + static fallback row | yes |
| MushroomBrute | Mushroom Brute | Medium | Forest | Melee | Melee | static-only | no |
| TreantSapling | Treant Sapling | Medium | Forest | Melee | Melee | static-only | no |
| ThornImp | Thorn Imp | Medium | Forest | Ranged | Ranged | static-only | no |
| TuskerBoar | Tusker Boar | Medium | Forest | Rush | Rush | static-only | no |
| HiveWasp | Hive Wasp | Medium | Forest | Flying | Flying | static-only | no |
| TreantAncient | Treant Ancient | Medium | Forest | Melee | Melee | static-only | no |
| ForestWraith | Forest Wraith | Medium | Forest | Ranged | Strafer | static-only | no |
| SporeBomb | Spore Bomb | Medium | Forest | Rush | Exploder | static-only | no |
| VineStrangler | Vine Strangler | Medium | Forest | Melee | Burrower | static-only | no |
| MyconidDruid | Myconid Druid | Medium | Forest | Ranged | Necromancer | static-only | no |
| CrabGuard | Crab Guard | Hard | Ocean | Melee | Melee | static-only | no |
| DrownedSailor | Drowned Sailor | Hard | Ocean | Melee | Melee | static-only | no |
| JellyHover | Jelly Hover | Hard | Ocean | Ranged | Ranged | static-only | no |
| ReefShark | Reef Shark | Hard | Ocean | Rush | Rush | static-only | no |
| GhostRay | Ghost Ray | Hard | Ocean | Flying | Flying | static-only | no |
| AnglerfishStalker | Anglerfish Stalker | Hard | Ocean | Melee | Stutterer | static-only | no |
| CoralMortar | Coral Mortar | Hard | Ocean | Ranged | Turret | static-only | no |
| SeaMine | Sea Mine | Hard | Ocean | Rush | Exploder | static-only | no |
| BrineStrafer | Brine Strafer | Hard | Ocean | Ranged | Strafer | static-only | no |
| DrownedPriestess | Drowned Priestess | Hard | Ocean | Ranged | Necromancer | static-only | no |
| DroneGrunt | Drone Grunt | VeryHard | Martian | Ranged | Ranged | static-only | no |
| CrystalCrawler | Crystal Crawler | VeryHard | Martian | Melee | Melee | static-only | no |
| PlasmaSpitter | Plasma Spitter | VeryHard | Martian | Ranged | Ranged | static-only | no |
| RocketLeaper | Rocket Leaper | VeryHard | Martian | Rush | Rush | static-only | no |
| SaucerDrone | Saucer Drone | VeryHard | Martian | Flying | Flying | static-only | no |
| PlasmaSentinel | Plasma Sentinel | VeryHard | Martian | Ranged | Turret | static-only | no |
| MindSlug | Mind Slug | VeryHard | Martian | Melee | Stutterer | static-only | no |
| CrystalBomber | Crystal Bomber | VeryHard | Martian | Rush | Exploder | static-only | no |
| SandTunneler | Sand Tunneler | VeryHard | Martian | Melee | Burrower | static-only | no |
| CyberLich | Cyber Lich | VeryHard | Martian | Ranged | Necromancer | static-only | no |
| PitImp | Pit Imp | Impossible | Hell | Rush | Rush | static-only | no |
| BoneKnight | Bone Knight | Impossible | Hell | Melee | Melee | static-only | no |
| FireSkull | Fire Skull | Impossible | Hell | Flying | Flying | static-only | no |
| Hellhound | Hellhound | Impossible | Hell | Rush | Rush | static-only | no |
| Gargoyle | Gargoyle | Impossible | Hell | Flying | Flying | static-only | no |
| DemonSentinel | Demon Sentinel | Impossible | Hell | Melee | Stutterer | static-only | no |
| BrimstoneMortar | Brimstone Mortar | Impossible | Hell | Ranged | Turret | static-only | no |
| SinEater | Sin Eater | Impossible | Hell | Rush | Exploder | static-only | no |
| PlagueCultist | Plague Cultist | Impossible | Hell | Ranged | Necromancer | static-only | no |
| HellWyrm | Hell Wyrm | Impossible | Hell | Melee | Burrower | static-only | no |

### Mob Vertex Animation Rows
| EnemyID | Enabled | SampleRate | Frames | Clip Ranges | StaticMesh | Material |
| --- | --- | --- | --- | --- | --- | --- |
| Slime | true | 30.000000 | 195 | Idle 0-59; Move 60-99; Attack 100-129; Hit 130-149; Death 150-194 | /Game/Characters/MobsVAT/Slime/SM_EasyMobVAT_Slime | /Game/Characters/MobsVAT/Slime/MI_EasyMobVAT_Slime |
| CaveBat | true | 30.000000 | 195 | Idle 0-59; Move 60-99; Attack 100-129; Hit 130-149; Death 150-194 | /Game/Characters/MobsVAT/CaveBat/SM_EasyMobVAT_CaveBat | /Game/Characters/MobsVAT/CaveBat/MI_EasyMobVAT_CaveBat |
| BoneWalker | true | 30.000000 | 195 | Idle 0-59; Move 60-99; Attack 100-129; Hit 130-149; Death 150-194 | /Game/Characters/MobsVAT/BoneWalker/SM_EasyMobVAT_BoneWalker | /Game/Characters/MobsVAT/BoneWalker/MI_EasyMobVAT_BoneWalker |
| RatPack | true | 30.000000 | 195 | Idle 0-59; Move 60-99; Attack 100-129; Hit 130-149; Death 150-194 | /Game/Characters/MobsVAT/RatPack/SM_EasyMobVAT_RatPack | /Game/Characters/MobsVAT/RatPack/MI_EasyMobVAT_RatPack |
| TombSpider | true | 30.000000 | 195 | Idle 0-59; Move 60-99; Attack 100-129; Hit 130-149; Death 150-194 | /Game/Characters/MobsVAT/TombSpider/SM_EasyMobVAT_TombSpider | /Game/Characters/MobsVAT/TombSpider/MI_EasyMobVAT_TombSpider |
| HexSlinger | true | 30.000000 | 195 | Idle 0-59; Move 60-99; Attack 100-129; Hit 130-149; Death 150-194 | /Game/Characters/MobsVAT/HexSlinger/SM_EasyMobVAT_HexSlinger | /Game/Characters/MobsVAT/HexSlinger/MI_EasyMobVAT_HexSlinger |
| StoneSentinel | true | 30.000000 | 195 | Idle 0-59; Move 60-99; Attack 100-129; Hit 130-149; Death 150-194 | /Game/Characters/MobsVAT/StoneSentinel/SM_EasyMobVAT_StoneSentinel | /Game/Characters/MobsVAT/StoneSentinel/MI_EasyMobVAT_StoneSentinel |
| MimicLure | true | 30.000000 | 195 | Idle 0-59; Move 60-99; Attack 100-129; Hit 130-149; Death 150-194 | /Game/Characters/MobsVAT/MimicLure/SM_EasyMobVAT_MimicLure | /Game/Characters/MobsVAT/MimicLure/MI_EasyMobVAT_MimicLure |
| BoneConjurer | true | 30.000000 | 195 | Idle 0-59; Move 60-99; Attack 100-129; Hit 130-149; Death 150-194 | /Game/Characters/MobsVAT/BoneConjurer/SM_EasyMobVAT_BoneConjurer | /Game/Characters/MobsVAT/BoneConjurer/MI_EasyMobVAT_BoneConjurer |
| CryptWraith | true | 30.000000 | 195 | Idle 0-59; Move 60-99; Attack 100-129; Hit 130-149; Death 150-194 | /Game/Characters/MobsVAT/CryptWraith/SM_EasyMobVAT_CryptWraith | /Game/Characters/MobsVAT/CryptWraith/MI_EasyMobVAT_CryptWraith |

### Bosses
- Boss data rows: 23.
- Boss visual status counts: static-only: 23.
- All boss rows currently resolve to static-only visual rows. No current boss row uses VAT or a skeletal animation row.
- `T66GamblerBoss` exists in source, but no `Bosses.csv` row currently references a non-empty `BossClass` value.
| BossID | Display | Difficulty | Theme | Local Stage | BossClass | Visual/Animation State | Static Mesh |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Dungeon_SewerSlimeKing | The Sewer Slime King | Easy | Dungeon | 1 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Dungeon_SewerSlimeKing/QuadRetro/SM_Dungeon_SewerSlimeKing_QuadRetro |
| Dungeon_WebMatriarch | The Web Matriarch | Easy | Dungeon | 2 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Dungeon_WebMatriarch/QuadRetro/SM_Dungeon_WebMatriarch_QuadRetro |
| Dungeon_BoneJailer | The Bone Jailer | Easy | Dungeon | 3 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Dungeon_BoneJailer/QuadRetro/SM_Dungeon_BoneJailer_QuadRetro |
| Dungeon_BaelFallenChad | Bael Fallen Chad | Easy | Dungeon | 4 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Dungeon_BaelFallenChad/QuadRetro/SM_Dungeon_BaelFallenChad_QuadRetro |
| Forest_BrambleTreant | The Bramble Treant | Medium | Forest | 1 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Forest_BrambleTreant/QuadRetro/SM_Forest_BrambleTreant_QuadRetro |
| Forest_MyconidQueen | The Myconid Queen | Medium | Forest | 2 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Forest_MyconidQueen/QuadRetro/SM_Forest_MyconidQueen_QuadRetro |
| Forest_ThornHive | The Thorn Hive | Medium | Forest | 3 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Forest_ThornHive/QuadRetro/SM_Forest_ThornHive_QuadRetro |
| Forest_BuerVerdantChad | Buer Verdant Chad | Medium | Forest | 4 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Forest_BuerVerdantChad/QuadRetro/SM_Forest_BuerVerdantChad_QuadRetro |
| Ocean_ReefCrabColossus | The Reef Crab Colossus | Hard | Ocean | 1 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Ocean_ReefCrabColossus/QuadRetro/SM_Ocean_ReefCrabColossus_QuadRetro |
| Ocean_AbyssalJellyfish | The Abyssal Jellyfish | Hard | Ocean | 2 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Ocean_AbyssalJellyfish/QuadRetro/SM_Ocean_AbyssalJellyfish_QuadRetro |
| Ocean_DrownedCaptain | The Drowned Captain | Hard | Ocean | 3 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Ocean_DrownedCaptain/QuadRetro/SM_Ocean_DrownedCaptain_QuadRetro |
| Ocean_FocalorDrownedChad | Focalor Drowned Chad | Hard | Ocean | 4 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Ocean_FocalorDrownedChad/QuadRetro/SM_Ocean_FocalorDrownedChad_QuadRetro |
| Martian_RedSandBehemoth | The Red Sand Behemoth | VeryHard | Martian | 1 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Martian_RedSandBehemoth/QuadRetro/SM_Martian_RedSandBehemoth_QuadRetro |
| Martian_CrystalMantis | The Crystal Mantis | VeryHard | Martian | 2 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Martian_CrystalMantis/QuadRetro/SM_Martian_CrystalMantis_QuadRetro |
| Martian_PlasmaSaucerPrime | The Plasma Saucer Prime | VeryHard | Martian | 3 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Martian_PlasmaSaucerPrime/QuadRetro/SM_Martian_PlasmaSaucerPrime_QuadRetro |
| Martian_StolasAstralChad | Stolas Astral Chad | VeryHard | Martian | 4 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Martian_StolasAstralChad/QuadRetro/SM_Martian_StolasAstralChad_QuadRetro |
| Hell_Horseman_Conquest | Conquest | Impossible | Hell | 1 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Hell_Horseman_Conquest/QuadRetro/SM_Hell_Horseman_Conquest_QuadRetro |
| Hell_Horseman_War | War | Impossible | Hell | 1 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Hell_Horseman_War/QuadRetro/SM_Hell_Horseman_War_QuadRetro |
| Hell_Horseman_Famine | Famine | Impossible | Hell | 1 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Hell_Horseman_Famine/QuadRetro/SM_Hell_Horseman_Famine_QuadRetro |
| Hell_Horseman_Death | Death | Impossible | Hell | 1 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Hell_Horseman_Death/QuadRetro/SM_Hell_Horseman_Death_QuadRetro |
| Hell_FalseProphet | The False Prophet | Impossible | Hell | 2 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Hell_FalseProphet/QuadRetro/SM_Hell_FalseProphet_QuadRetro |
| Hell_Antichrist | The Antichrist | Impossible | Hell | 3 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Hell_Antichrist/QuadRetro/SM_Hell_Antichrist_QuadRetro |
| Hell_GreatDragon | The Great Dragon | Impossible | Hell | 4 | AT66BossBase default/fallback | static-only | /Game/Characters/Enemies/Bosses/Hell_GreatDragon/QuadRetro/SM_Hell_GreatDragon_QuadRetro |

## E. Performance Reality
- Configured tower baseline in `Content/Data/PlayerExperience.json`: `RuntimeEnemiesPerWave=10`, `RuntimeMaxAliveEnemies=30`, `RuntimeSpawnIntervalSeconds=5`, `RuntimeWaveStaggerDurationSeconds=5`, `RuntimeMaxSpawnsPerStaggeredBatch=1`, plus 4 initial enemies per gameplay floor and 3 gameplay floors per stage.
- Stage-local progression scalars in `Config/DefaultT66StageProgression.ini` raise runtime count scalar from 1.0 on local stage 1 to 1.4 on local stage 4.
- Difficulty scalar is skull-driven in `UT66RunStateSubsystem::GetDifficultyScalar()`: `1.0 + 0.1 * DifficultySkulls`, clamped to 99.
- Current normal tower path therefore starts at 30 max alive enemies. With local-stage-4 runtime count scalar and no skulls, the code computes 42 max alive. With four skulls, it computes about 59. The absolute tower build-budget clamp is 1024.
- Non-tower fallback code in `AT66EnemyDirector::SpawnRuntimeTrickleWave` can clamp max alive as high as 1500, but that appears to be fallback/non-tower behavior rather than the current tower budget.
- No CPU/GPU profiling capture or benchmark table for animation cost was found in the rigging docs or runtime code search.
- VAT rationale in docs: VAT was chosen for mobs to avoid per-enemy skeletal animation graph cost and support many enemies. Current evidence shows a functional validation/spawn QA path, not a measured VAT-vs-skeletal benchmark.
- `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md` explicitly requires future performance acceptance: no skeletal animation graph for normal playback, many copies without CPU spikes, desynced phases, data-driven material updates, and correct culling bounds.

## F. Assessment

### What Existing Docs / Findings Claim
- Arthur/Royal Chad: the current accepted path uses the QuadRetro Pixal3D GLB, Rigodotify/Rigify-compatible humanoid rigging, UAL retargeting, and Unreal import verification. The docs call the roll-forward pass the current accepted run and note prior scale/retarget/import issues.
- Easy mobs: the docs say all ten Easy/Dungeon mobs have enabled VAT rows, nonzero position/normal textures, static mesh + material instance paths, and staged QA spawn coverage. They also state the batch is for in-game QA and future release review, not necessarily final animation art.
- Known limitations captured in `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`: Quaternius FBX scale risk, Rigodotify Blender warning, material custom-node cooked failure fixed by adding `Parameters`, and the fact that Pixal3D GLBs should be promoted only after visual/runtime acceptance.

### Direct Observation From Contact Sheets
- Arthur looks recognizable and coherent, but motion is still generic library retargeting rather than authored character performance. The roll reads as a forward roll, but side/gameplay views show a stiff full-body tumble with the large weapon passing through or visually dominating the pose. It is functional, not polished.
- Arthur walk and jump poses preserve the character silhouette, but the gait is restrained and heavy; foot contact and weight transfer are not convincing enough to call final. The idle is mostly stable, with limited personality.
- Easy mob VAT contact sheet shows all ten mobs present across clips/views. Slime, CaveBat, TombSpider, and StoneSentinel read best at sheet scale because their silhouettes tolerate broad deformation. Humanoid/robed rows such as BoneWalker, HexSlinger, BoneConjurer, and CryptWraith read more like generic sways/turns than creature-specific acting.
- Some motion is too subtle at gameplay scale in the index sheet. That is a release-risk because VAT can be technically valid while still visually under-communicating movement, attack, hit, or death beats.
- I did not see evidence in this pass of an AnimBP/state-machine layer, foot IK, additive hit reactions, or bespoke boss animation. The current runtime path is deliberately simple and data-driven.

### Likely Root Causes
- Current hero animation is retargeted UAL library motion onto a generated/retopoed character, not authored around Arthur's armor, oversized shoulders, skirt, and weapon silhouette.
- The mob VAT batch is procedural/automation-first. It prioritizes complete runtime data rows and clip ranges over creature-specific animation direction.
- Static fallback data remains the majority of non-Easy mobs and all bosses, so the system is still early-stage outside the first hero and Easy mob VAT batch.
- There is no benchmark-driven performance envelope yet; VAT was selected for a plausible scaling reason, but not proven against skeletal alternatives in this repo evidence.

### Tried And Failed But Not In Findings
No additional tried-and-failed rigging/animation approaches were found in this pass beyond what is already captured in `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md` and the related Easy mob batch docs.

## G. Visual Evidence Pointers
Do not move or copy these; they are current evidence paths for Pablo/Claude to inspect.
| Evidence | Path |
| --- | --- |
| Arthur all actions/all views contact sheet | C:\UE\T66\Model Generation\Rigging and Animation\Runs\Arthur_QuadRetro_UAL_Retarget_RollForward_20260515\Arthur_All_Actions_All_Views_Contact_Sheet.png |
| Arthur RollForward side contact sheet | C:\UE\T66\Model Generation\Rigging and Animation\Runs\Arthur_QuadRetro_UAL_Retarget_RollForward_20260515\AM_Hero_1_Chad_QuadRetroUALQA_Roll_side_contact_sheet.png |
| Arthur RollForward gameplay contact sheet | C:\UE\T66\Model Generation\Rigging and Animation\Runs\Arthur_QuadRetro_UAL_Retarget_RollForward_20260515\AM_Hero_1_Chad_QuadRetroUALQA_Roll_gameplay_contact_sheet.png |
| Easy mob VAT all-clips/all-views index sheet | C:\UE\T66\Model Generation\Rigging and Animation\Runs\Easy_Mob_VAT_20260514\PreviewFrames\Easy_Mobs_AllClips_AllViews_Index_Contact_Sheet.png |

## Current Process And Procedure As Implemented Today
1. Source model generation happens outside runtime through Pixal3D/TRELLIS/Quad Retro style mesh preparation. For the current accepted hero and Easy mob evidence, Pixal3D/production GLBs feed the rigging/VAT process; TRELLIS remains a broader mesh-generation path unless a specific run routes into rigging.
2. Hero/companion-style skeletal work imports a generated GLB into Blender, prepares a humanoid Rigodotify/Rigify-compatible rig, retargets Quaternius UAL actions, bakes/exports skeletal mesh and animation FBXs, imports them to Unreal, and wires `CharacterVisuals.csv` / `DT_CharacterVisuals`.
3. Runtime hero playback does not use an AnimBP. `AT66HeroBase` caches the four movement animation assets from `UT66CharacterVisualSubsystem` and calls `PlayAnimation()` for idle, walk, jump, and one-shot roll states.
4. Easy mob animation uses an authored/baked VAT path: Blender creates source actions, Unreal AnimToTexture bakes position/normal textures and a static mesh, `MobVertexAnimations.csv` stores clip frame ranges, and `AT66EnemyBase` updates material scalar parameters per enemy.
5. For mobs, runtime chooses VAT first when `DT_MobVertexAnimations` has an enabled row. If the VAT row is missing/disabled, the enemy uses its static `CharacterVisuals.csv` row.
6. Bosses currently use the same visual subsystem but all boss data rows are static-only, so boss animation is a future process area rather than an active current code path.
7. Current verification evidence is contact sheets, Unreal import/verification scripts, CSV/data-table rows, and staged QA spawn hooks. There is no repo-confirmed animation performance benchmark yet.
