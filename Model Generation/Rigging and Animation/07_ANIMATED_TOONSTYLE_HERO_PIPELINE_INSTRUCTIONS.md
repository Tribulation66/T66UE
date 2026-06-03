# Animated ToonStyle Hero Pipeline

## Purpose

This is the skeletal-mesh counterpart to the static Pixal3D ToonStyle production import. Use it for humanoid heroes and companions that need runtime animations while preserving the existing ToonStyle material treatment used by the production static meshes.

The current demo scope is 28 animated visual rows:

- 10 default hero rows.
- 10 demo-skin hero rows.
- 8 companion rows: default and demo skin for `Companion_01` through `Companion_04`.

The default hero rows are:

- `Hero_1_Chad`
- `Hero_1_Stacy`
- `Hero_2_Chad`
- `Hero_2_Stacy`
- `Hero_3_Chad`
- `Hero_3_Stacy`
- `Hero_4_Chad`
- `Hero_4_Stacy`
- `Hero_5_Chad`
- `Hero_5_Stacy`

## Data Contract

`Content/Data/CharacterVisuals.csv` uses explicit animation fields:

- `WalkAnimation`: looping movement animation.
- `IdleAnimation`: looping idle/preview animation.
- `JumpAnimation`: one-shot jump animation.
- `RollAnimation`: one-shot roll animation.

Do not reintroduce the old `LoopingAnimation`, `AlertAnimation`, or `RunAnimation` column names.

## Pipeline

1. Start from the Pixal3D GLBs under:
   `Model Generation/Runs/Pixal3D/HeroDemoLineup_20260522_AccuRig/Outputs/<HeroID>.glb`.
   Do not use the AccuRig-textured FBX/OBJ outputs unless the GLB is unavailable; the GLB path preserves the UV/material source expected by the ToonStyle texture binding.
2. For demo skins and companions, start from the Pixal3D GLBs under:
   `Model Generation/Runs/Pixal3D/HumanoidGuidelineTest_20260522_100k/Outputs/<SourceID>.glb`.
3. Before a batch export, run the all-row facing diagnostic:
   `Model Generation/Rigging and Animation/Tools/render_animated_toonstyle_facing_diagnostic.py`.
   The accepted 2026-05-26 diagnostic showed all 28 raw GLBs back-facing at yaw 0 and front-facing at yaw 180, while the accepted Rigify/Quaternius template is front-facing at yaw 0. The exporter therefore applies `source_yaw_degrees=180` to imported Pixal3D GLBs before weight transfer. Do not remove or change that correction without a new all-28 diagnostic proving the source orientation changed.
4. Run the Blender source exporter:
   `Model Generation/Rigging and Animation/Tools/create_animated_toonstyle_hero_sources.py`.
5. The exporter uses the accepted Hero 1 Rigify/Quaternius template blend as the deformation and animation source. The reviewed template covers the gameplay movement clips; idle is regenerated as a neutral standing loop because the template idle is not an accepted gameplay idle. Do not rebuild or replace the reviewed movement actions during batch export:
   - `Idle`: regenerated neutral `Idle_No_Loop`
   - `Walk`: `Walk_Fwd_Loop_LegsTorsoOnly`
   - `Jump`: `DoubleJump_LegsTorsoOnly`
   - `Roll`: `Roll_LegsTorsoOnly`
   If one of the movement clips regresses, fix the reviewed template blend first, then rerun the exporter. Do not generate ad hoc replacement walk/jump/roll curves in the batch exporter.
6. The exporter writes:
   - `<HeroID>_Skeletal.fbx`
   - `<HeroID>_Idle.fbx`
   - `<HeroID>_Walk.fbx`
   - `<HeroID>_Jump.fbx`
   - `<HeroID>_Roll.fbx`
7. Rebuild the C++ editor target after changing animation row fields.
8. Run the Unreal import script:
   `Model Generation/Rigging and Animation/Tools/import_animated_toonstyle_heroes_to_unreal.py`.
9. The importer writes skeletal assets to:
   `/Game/Characters/Heroes/Hero_<N>/<Body>/AnimatedToonStyle/`
   or the companion `target_dir` recorded in the manifest.
10. The importer uses the GLB-derived ToonStyle working textures recorded in the
   Blender manifest to build and assign `/AnimatedToonStyle/Materials/MI_*`
   material instances for the skeletal meshes. Do not rely on FBX source-material
   preservation for the batch; missing FBX material slots can import as
   `WorldGridMaterial` and erase the hero-specific color treatment.
11. The importer updates `CharacterVisuals.csv` for the manifest rows, sets the
   animated runtime facing rotation to `Yaw=-90`, and reloads `DT_CharacterVisuals`.

## ToonStyle Notes

The skeletal path keeps the visible ToonStyle surface by importing the
GLB-derived BaseColor/Tint/InnerLine textures and assigning a per-hero animated
material instance. Runtime code can then rebuild safe ToonStyle dynamic materials
from those material texture parameters. Do not blindly replace animated hero
materials with the shared unlit fallback material, the static-mesh material
instances, or FBX-imported `WorldGridMaterial`; those paths can erase the
hero-specific color treatment and produce a dark single-color character.

The static inverted-hull outline sidecar mesh does not deform with a skeleton, so it remains a static-mesh fallback only. If animated outlines are required later, add a separate skeletal outline pass that imports a duplicate weighted outline skeletal mesh or moves the outline entirely into a skeletal-compatible material path.

## Validation

For each batch:

1. Confirm the Blender manifest exists:
   `Model Generation/Rigging and Animation/Runs/AnimatedToonStyleHeroes_20260522/animated_toonstyle_hero_sources_manifest.json`.
2. Confirm the Unreal import report exists:
   `Saved/AnimatedToonStyleHeroImportReport.json`.
3. Confirm `Content/Data/CharacterVisuals.csv` has `SkeletalMesh`, `WalkAnimation`, `IdleAnimation`, `JumpAnimation`, and `RollAnimation` set for all 28 demo-scope rows.
4. Before importing to Unreal, generate or inspect a Blender pose sheet/video from the exported FBXs. Idle must stand still; walk and jump must not contain backflip frames.
5. Compile the editor target.
6. If the change affects playable standalone content, refresh the staged standalone build and verify the taskbar shortcut target.
