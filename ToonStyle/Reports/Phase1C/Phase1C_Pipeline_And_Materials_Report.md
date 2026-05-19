# Phase 1C Pipeline And Materials Report

Date: 2026-05-17

## Scope

This report covers Phase 1C Part 2 after the Pixal3D pause point: Blender pipeline extensions, UE import, material fixes, TestRoom updates, build/stage/smoke verification, and the remaining caveats.

## Pipeline Changes

- `ToonStyle/BlenderScripts/run_toon_pipeline.py`
  - Runs texture flattening after Pixal3D texture extraction.
  - Duplicates an outline mesh before humanoid face-normal transfer.
  - Reverses outline face winding while preserving outward normals.
  - Writes `retained_from_phase1a`, flattening metadata, normal delta evidence, and outline winding evidence into each manifest.
- `ToonStyle/BlenderScripts/flatten_diffuse_texture.py`
  - Numpy-only deterministic clustering because Blender Python did not have scikit-learn.
- `ToonStyle/BlenderScripts/verify_winding_reversal.py`
  - Disposable cube pre-flight for winding reversal.
- `ToonStyle/Source/ImportPixal3DAsset_Phase1C.py`
  - Phase 1C import wrapper used after the original importer hit mapped-section/source lock friction.

## Gates

| Gate | Result | Evidence |
| --- | --- | --- |
| G4 | Passed | `SourceAssets/ToonStyle/Pixal3D/Phase1C/LuBu/Working/lubu_validation/lubu_validation_manifest.json`; Lu Bu colors reduced `244782/622 -> 8/7`. |
| G4.5 | Passed | `Saved/Codex/ToonStyle/Phase1C/verify_winding_reversal.json`; cube face order reversed and sampled vertex normals remained outward. |
| G5 | Passed | `SourceAssets/ToonStyle/Pixal3D/Phase1C/LuBu/Working/lubu_validation/lubu_validation_ue_verify.json`; Lu Bu shading and outline meshes imported at ~180 UU and outline winding evidence is true. |
| G5.5 | Failed in cooked SM6 | The throwaway graph-distance material and the CameraOffset fallback compiled in editor but both produced cooked `M_Toon_Character_Outline` invalid ShaderMap/default-material warnings in staged smoke. |
| G6 | Passed with fallback | Production outline master now uses the cooked-valid fallback: opaque one-sided reversed-winding material with normal-extrusion WPO only. Staged smoke shows no outline ShaderMap/default-material warnings. Distance modulation remains deferred. |
| G7 | Deferred | Vertex color A is authored into the meshes but not consumed by the cooked-valid fallback. A-mask consumption remains tied to the same cooked SM6 outline-material failure family. |
| G8 | Passed | `StageStandaloneBuild.ps1 -ClientConfig Development` succeeded; staged direct-entry smoke reached TestRoom. |
| G9 | Technically passed with caveats | Screenshot `Saved/Codex/ToonStyle/Phase1C/Phase1C_TestRoom_MinimalOutline.png` exists; log `Saved/StandaloneLogs/Phase1C_TestRoomScreenshot_MinimalOutline.log` shows 28 toon registrations, manual exposure path, no outline ShaderMap errors, and TestRoom BeginPlay. |

## Bulk Asset Outcome

All eleven lineup slots imported through the Part 2 pipeline with both shading and outline meshes, flattened textures, material instances, and ~180 UU bounds.

| Asset | Retained | Source note | UE shading mesh | UE outline mesh |
| --- | --- | --- | --- | --- |
| Lu Bu | no | standard | `/Game/ToonStyle/TestAssets/Validation/SM_lubu_validation` | `/Game/ToonStyle/TestAssets/Validation/SM_lubu_validation_Outline` |
| ARIA | no | standard | `/Game/ToonStyle/TestAssets/Lineup/SM_aria` | `/Game/ToonStyle/TestAssets/Lineup/SM_aria_Outline` |
| Gambler | no | standard | `/Game/ToonStyle/TestAssets/Lineup/SM_gambler` | `/Game/ToonStyle/TestAssets/Lineup/SM_gambler_Outline` |
| Slime | yes | retained Phase 1A raw | `/Game/ToonStyle/TestAssets/Lineup/SM_slime` | `/Game/ToonStyle/TestAssets/Lineup/SM_slime_Outline` |
| TombSpider | no | standard | `/Game/ToonStyle/TestAssets/Lineup/SM_tombspider` | `/Game/ToonStyle/TestAssets/Lineup/SM_tombspider_Outline` |
| CaveBat | no | standard | `/Game/ToonStyle/TestAssets/Lineup/SM_cavebat` | `/Game/ToonStyle/TestAssets/Lineup/SM_cavebat_Outline` |
| Idol Altar | no | standard | `/Game/ToonStyle/TestAssets/Lineup/SM_idolaltar` | `/Game/ToonStyle/TestAssets/Lineup/SM_idolaltar_Outline` |
| Arcade Machine | no | standard | `/Game/ToonStyle/TestAssets/Lineup/SM_arcademachine` | `/Game/ToonStyle/TestAssets/Lineup/SM_arcademachine_Outline` |
| Loot Chest | no | Pixal3D no-remesh export fallback | `/Game/ToonStyle/TestAssets/Lineup/SM_lootchest` | `/Game/ToonStyle/TestAssets/Lineup/SM_lootchest_Outline` |
| Loot Bag Yellow | no | R1024 Pixal3D exception | `/Game/ToonStyle/TestAssets/Lineup/SM_lootbag_yellow` | `/Game/ToonStyle/TestAssets/Lineup/SM_lootbag_yellow_Outline` |
| Loot Crate | no | Pixal3D no-remesh export fallback | `/Game/ToonStyle/TestAssets/Lineup/SM_lootcrate` | `/Game/ToonStyle/TestAssets/Lineup/SM_lootcrate_Outline` |

Loot Chest and Loot Crate required Pixal3D's no-remesh export fallback in Part 1. They did not require any extra Blender cleanup in Part 2: no weld, decimate, or nonstandard mesh operation was added. The standard join, normalize, vertex color, outline duplicate, winding reversal, and FBX export path handled both.

## Material Outcome

- Toon material setup script: `ToonStyle/Source/SetupPhase1CToonMaterials.py`
- Verification:
  - `Saved/Codex/ToonStyle/Phase1C/phase1c_material_masters_verify.json`
  - `Saved/Codex/ToonStyle/Phase1C/phase1c_material_instances_verify.json`
  - `Saved/Codex/ToonStyle/Phase1C/phase1c_toon_material_setup_verify.json`

Production material state:

- `M_Toon_Character`: unchanged cel character master, unlit emissive.
- `M_Toon_Environment`: non-uniform UV tiling via `UVTileU` / `UVTileV`.
- `M_Toon_Character_Outline`: reversed-winding one-sided opaque outline material with normal-extrusion WPO only.

Rarity outline colors were applied to the eleven outline instances:

- Black: Lu Bu, Slime, Arcade Machine
- Red: ARIA, TombSpider, Loot Chest
- Yellow: Gambler, CaveBat, Loot Bag Yellow
- White: Idol Altar, Loot Crate

## TestRoom Outcome

- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`
  - Adds `t66.TestRoom.UseManualExposure` defaulting to `1`.
  - Manual path uses `AEM_Manual` and `AutoExposureBias=+0.7`.
  - Rollback path preserves the Phase 1B fixed clamp.
  - Spawns all eleven lineup slots with shading and outline actors.
  - Registers 28 toon components with `T66WorldVisualSetup` for theme parameter delivery.
  - Uses wall/ceiling `UVTileU=20`, `UVTileV=2`; floor `UVTileU=10`, `UVTileV=10`.

The staged screenshot shows the environment textures tiled and the lineup present. The camera framing does not show every asset equally clearly, so Pablo should still do the manual walkaround visual review.

## Build And Smoke Evidence

- Build/stage: `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development`
- Staged executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Shortcut rule verified by the stage script:
  - `C:\UE\T66\T66 Standalone.lnk`
  - `%APPDATA%\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk`
- Shortcut targets were also checked via `WScript.Shell`: both point to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe` and the target exists.
- Direct-entry smoke log: `Saved/StandaloneLogs/Phase1C_TestRoomSmoke_MinimalOutline_Wait.log`
- Screenshot smoke log: `Saved/StandaloneLogs/Phase1C_TestRoomScreenshot_MinimalOutline.log`
- Screenshot: `Saved/Codex/ToonStyle/Phase1C/Phase1C_TestRoom_MinimalOutline.png`

Important log markers:

- `Direct entry configured gameplay run Category=3 Hero=Hero_2`
- `T66GameMode BeginPlay - TestRoom`
- `Applied cel atmosphere theme=0/4/0 to 28 registered toon material(s)`
- `ToonStyle TestRoom G6 parameter probe applied Dungeon=28 Hell=28 RestoredDungeon=28`
- `ToonStyle TestRoom using Phase 1C manual exposure path: AEM_Manual, Bias=+0.7`

No `M_Toon_Character_Outline` invalid ShaderMap, uncooked shader map, failed material compile, or default-material fallback appears in the final smoke log.

## Remaining Caveats

1. Distance-modulated outline thickness remains deferred. Both graph-distance and CameraOffset material-function variants compiled in editor but failed cooked SM6 ShaderMap validation.
2. Vertex color A masking remains deferred. The meshes carry the authored A channel, but the cooked-valid outline fallback uses a constant mask of 1.0.
3. The final screenshot is useful for technical smoke but not sufficient for aesthetic approval. Pablo should walk the room, inspect each row, and judge the color/outline readability in motion.

The shader/material caveat is tracked in `ToonStyle/Source/pending_issues_Source.md`.
