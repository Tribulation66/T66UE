# T66 Model Rigging System Report

Prepared for Claude handoff on 2026-05-17.

## Purpose

`C:\UE\T66\Model Rigging` is the new top-level home for the rigging system handoff. The currently implemented rigging and animation system still lives under:

```text
C:\UE\T66\Model Generation\Rigging and Animation
```

This report documents that current system before any physical migration. It is intentionally written as a handoff: a future agent should be able to understand what every current file or file family does, what evidence exists, and what process T66 is using for iterative rigging work.

No Blender scenes, vendor packages, Unreal assets, or generated run outputs were moved as part of this report. The existing paths remain authoritative until a dedicated migration pass moves them.

## Current Ownership

The rigging work is currently routed through `Model Generation` because it produces model-generation outputs before they become runtime assets.

Primary instruction routers:

| Path | Role |
| --- | --- |
| `AGENTS.md` | Root repo rules: goal translation, folder instruction discovery, pending issues, staged standalone shortcut verification, script lifecycle, and verification evidence. |
| `Model Generation/MODEL_GENERATION_AGENTS.md` | Owns model generation, Blender QA, rigging and retopo policy, generated model cleanup, and generated mesh import. Routes editable animation into the rigging folder. |
| `Model Generation/Instructions/README.md` | Canonical index for model-generation instructions. Points character animation work into the rigging folder. |
| `Model Generation/Instructions/00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md` | Model-generation decision tree. Says editable rigs, retargeting, Rigodotify, Quaternius, and authored animation sets route to `Rigging and Animation`. |
| `Model Generation/Instructions/04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md` | General Blender processing policy. Requires feet-origin scale consistency, explicit action preservation, and multi-angle validation for equipment and deformation quality. |
| `Model Generation/Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md` | Import and validation boundary. Generated assets are not runtime assets until imported, validated, cooked, staged, and shortcut-verified when gameplay-visible. |
| `Model Generation/Rigging and Animation/RIGGING_ANIMATION_AGENTS.md` | Most specific current router. Owns Blender source-of-truth rigs, Rigify/Rigodotify setup, Quaternius animation references, humanoid retargeting, editable Blender actions, preview QA, and export manifests. |

Documentation gap: the new top-level `Model Rigging` folder did not exist before this pass, so it currently has no `MODEL_RIGGING_AGENTS.md` router. If this folder becomes the canonical workflow root, add a folder router and update `Model Generation/MODEL_GENERATION_AGENTS.md` to point here.

## Folder Inventory Summary

Current source folder:

```text
Model Generation/Rigging and Animation/
```

Observed high-level contents:

| Folder | File count | Approx bytes | What it contains |
| --- | ---: | ---: | --- |
| `AssetInventory` | 5 | 261,775 | JSON probe reports plus a README. These are generated inventory outputs, not runtime assets. |
| `External` | 636 | 1,261,766,460 | Ignored local vendor cache for Rigodotify and Quaternius packages. |
| `Runs` | 1,569 | 725,110,503 | Ignored or disposable run outputs: Blender scenes, FBX exports, preview frames, contact sheets, and manifests. |
| `Tools` | 13 | 193,892 | Reusable Blender and Unreal Python/PowerShell tools plus Python cache files. |

Root files in `Model Generation/Rigging and Animation`:

| File | Role |
| --- | --- |
| `.gitignore` | Ignores `External/*`, `Runs/*`, and `AssetInventory/*.json` while allowing README files. This keeps vendor caches and generated outputs out of git by default. |
| `README.md` | Human overview of the rigging and animation system. It defines Blender as source of truth, Rigify plus Rigodotify as the rigging base, Quaternius as animation reference source, Arthur as the accepted humanoid proof, and Easy mobs as the first VAT batch. |
| `RIGGING_ANIMATION_AGENTS.md` | Folder router and hard rules. Requires source `.blend` scenes, vendor packages in `External`, multi-angle visual QA, no gameplay wiring before Blender QA plus Unreal validation, and findings recorded in `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`. |
| `00_RIGGING_ANIMATION_ROUTING_INSTRUCTIONS.md` | Decision tree for this folder. Routes tool setup to `01`, humanoid hero or companion animation to `02`, non-humanoid mobs to `04`, Easy mobs to `05`, known limitations to `03`, and Unreal work to model-generation import instructions. |
| `01_TOOL_SETUP_INSTRUCTIONS.md` | Tool setup process. Documents Blender 5.1 path, Rigodotify clone and correctly prefixed zip packaging, add-on enablement, Quaternius zip locations, extracted cache layout, and GLTF/GLB import guidance. |
| `02_HERO_ANIMATION_PIPELINE_INSTRUCTIONS.md` | Main humanoid hero and companion process. Defines acceptance bar, required Blender loop, rigging rules, visual QA, Unreal import expectations, Arthur proof case, live promotion rules, and current runtime slot boundary. |
| `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md` | Running findings log. Captures setup state, package inventory, Blender probe results, failed Arthur pilot path, accepted Royal Chad/Arthur QuadRetro UAL retarget, VAT exploration, Easy VAT implementation, failures, fixes, evidence paths, and caveats. |
| `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md` | Main non-humanoid regular-enemy pipeline. Defines VAT runtime contract, data seam, UE 5.7 AnimToTexture notes, Blender source process, required clips, QA evidence, import/runtime QA, performance acceptance, and prohibited shortcuts. |
| `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md` | Difficulty 1 batch plan and current output record. Lists the ten Easy enemies, source GLBs, live mesh paths, live scales, per-enemy animation direction, batch execution order, runtime caveats, evidence paths, frame layout, and promotion rule. |

## AssetInventory

Path:

```text
Model Generation/Rigging and Animation/AssetInventory/
```

This folder holds generated JSON inventory reports from `Tools/inspect_animation_assets.py`. The README says these reports are ignored by git by default, and stable findings should be promoted into `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`.

Files:

| File | Role |
| --- | --- |
| `README.md` | Explains that JSON inventory reports belong here and durable conclusions should move into the findings reference. |
| `arthur_skeleton_compare.json` | Compares three Arthur skeleton/source candidates, including archived FBX and GLB sources. It is used to understand bone names, root bones, and whether old Arthur sources had useful actions. |
| `current_inventory.json` | Blender 5.1.1 inspection of current Standard package assets. Records 5 inspected assets. Key findings include `UAL1_Standard.glb` with 45 actions, `UAL2_Standard.glb` with 43 actions, `Mannequin_F.blend` with 2 armatures and 128 meshes, and Standard base-character GLTFs with armatures but no actions. |
| `source_inventory.json` | Blender 5.1.1 inspection of source `.blend` files. Records 4 inspected assets. `UAL1.blend` has 127 actions, `UAL2.blend` has 135 actions, and source base-character `.blend` files have armatures and meshes but no stored animation actions. |
| `source_export_inventory.json` | Blender 5.1.1 inspection of source package FBX/GLB exports. Records 4 inspected assets. Confirms UAL1 and UAL2 exported FBX/GLB variants each contain the same 127 or 135 action counts as their source packages. |

Process rule: do not treat `AssetInventory/*.json` as the long-term source of truth. They are evidence snapshots. Promote important conclusions into the Markdown findings file.

## External Vendor Cache

Path:

```text
Model Generation/Rigging and Animation/External/
```

This folder is a local vendor cache. It is intentionally ignored by git except for `External/README.md`.

Files and folders:

| Entry | Role |
| --- | --- |
| `README.md` | Explains that this folder is a local third-party cache, not a direct playable-content source. Expected layout includes Rigodotify, Rigodotify zip, and Quaternius packages. |
| `Rigodotify.zip` | Blender-installable zip produced from the local Rigodotify clone with a required top-level `Rigodotify/` prefix. Used by the setup script. |
| `Rigodotify/` | Local clone of `https://github.com/catprisbrey/Rigodotify.git`. The installed commit recorded in findings is `4ee6e34b1580a0fac07e31c6cfed30addad182aa`. Contains `__init__.py`, `blender_manifest.toml`, `LICENSE`, `README.md`, and folders `.git`, `experimental`, `HowTo`, `metarigs`, and `rigs`. |
| `Quaternius/` | Extracted Quaternius animation and base character packages. Used as source/reference data for Blender retargeting and QA, not imported directly into playable content. |

Quaternius package inventory:

| Package | Files | What it provides |
| --- | ---: | --- |
| `Universal Animation Library Source` | 8 | Source `UAL1.blend`, Unity FBX, Unreal/Godot GLB, setup screenshots, and license. Preferred source for Arthur idle, walk, jump, and roll. |
| `Universal Animation Library 2 Source` | 12 | Source `UAL2.blend`, source/export variants, setup images, and license. Useful for later parkour/combat variants. |
| `Universal Base Characters Source` | 431 | Source base-character `.blend`, `.fbx`, `.gltf`, `.bin`, `.png`, and zip files. Useful for editable body/reference work. |
| `Universal Animation Library Standard` | 6 | Standard UAL1 package with GLB/FBX/reference images/license. Useful for import-path experiments and regression checks. |
| `Universal Animation Library 2 Standard` | 10 | Standard UAL2 package with GLB/FBX/reference images/license and mannequin reference. |
| `Universal Base Characters Standard` | 112 | Standard base-character package with GLTF/FBX/texture/reference assets. Useful as fallback references and import checks. |

External-cache rules:

- Do not commit extracted vendor files.
- Do not edit vendor `.blend` files destructively.
- Append/link/import from vendor sources into a run scene.
- Use source packages for editable work; use Standard packages for engine import checks and fallback comparison.

## Runs

Path:

```text
Model Generation/Rigging and Animation/Runs/
```

`Runs/README.md` says per-character rigging and animation work runs go here. Run folders should stay lightweight and disposable; durable lessons belong in the instruction docs.

Current run folders:

| Run folder | Files | Role |
| --- | ---: | --- |
| `Arthur_QuadRetro_UAL_Retarget_20260514` | 126 | First Royal Chad/Arthur QuadRetro UAL retarget run. Superseded by the roll-forward corrected 20260515 run, but still useful historical evidence. |
| `Arthur_QuadRetro_UAL_Retarget_RollForward_20260515` | 126 | Accepted Royal Chad/Arthur QuadRetro UAL retarget run with corrected roll direction and multi-view contact sheets. Current humanoid proof case. |
| `Easy_Mob_VAT_20260514` | 1,316 | Easy mob VAT batch run for ten Difficulty 1 enemies. Contains Blender source, FBX exports, preview frames, contact sheets, and manifest. |

### Arthur_QuadRetro_UAL_Retarget_20260514

Top-level files:

| File | Role |
| --- | --- |
| `Arthur_QuadRetro_UAL_Retarget.blend` | Blender source scene for the first QuadRetro UAL retarget. Contains live Arthur mesh source, UAL source armature, target armature, baked actions, and export setup. |
| `Arthur_QuadRetro_UAL_Retarget.blend1` | Blender backup file for the same source scene. |
| `arthur_quadretro_ual_retarget_manifest.json` | Manifest for the run. Records visual IDs, source GLB, source texture, UAL1 source `.blend`, baked live scale, exported skeletal mesh FBX, action FBXs, frame ranges, mesh bounds, weight counts, retarget map, and notes. |
| `Exports/` | FBX outputs: `SK_Hero_1_Chad_QuadRetroUALQA.fbx` plus `AM_Hero_1_Chad_QuadRetroUALQA_{Idle,Walk,Jump,Roll}.fbx`. |
| `PreviewFrames/` | Rendered action frames and contact sheets from the first pass. |

This run used `Roll` as the roll source in the manifest. The later accepted pass switched to `Roll_RM` plus correction.

### Arthur_QuadRetro_UAL_Retarget_RollForward_20260515

Top-level files:

| File | Role |
| --- | --- |
| `Arthur_QuadRetro_UAL_Retarget.blend` | Current accepted Blender source scene for Royal Chad/Arthur. This is the source of truth for the accepted UAL retarget. |
| `Arthur_QuadRetro_UAL_Retarget.blend1` | Blender backup for the accepted source scene. |
| `arthur_quadretro_ual_retarget_manifest.json` | Accepted manifest. Same structure as the first run, but roll action uses `Roll_RM` and records the local-X sagittal mirror correction so the in-place roll reads forward. |
| `preview_manifest.json` | Preview render manifest. Lists actions, sampled frames, and front/side/three-quarter/gameplay view image paths. |
| `Exports/` | Accepted FBX outputs: `SK_Hero_1_Chad_QuadRetroUALQA.fbx`, `AM_Hero_1_Chad_QuadRetroUALQA_Idle.fbx`, `Walk.fbx`, `Jump.fbx`, and `Roll.fbx`. |
| `AM_Hero_1_Chad_QuadRetroUALQA_<Action>_<View>_f####.png` | Individual preview frame renders for Idle, Walk, Jump, and Roll from front, side, three-quarter, and gameplay views. |
| `AM_Hero_1_Chad_QuadRetroUALQA_<Action>_<View>_contact_sheet.png` | Per-action, per-view contact sheets used for frame-by-frame QA. |
| `Arthur_All_Actions_front_Contact_Sheet.png` | All accepted Arthur actions in the front view. |
| `Arthur_All_Actions_side_Contact_Sheet.png` | All accepted Arthur actions in the side view. This was critical for roll-forward validation. |
| `Arthur_All_Actions_three_quarter_Contact_Sheet.png` | Three-quarter view acceptance sheet. |
| `Arthur_All_Actions_gameplay_Contact_Sheet.png` | Gameplay-camera acceptance sheet. |
| `Arthur_All_Actions_All_Views_Contact_Sheet.png` | Combined all-view proof sheet for quick review. |

Accepted Arthur runtime mapping:

- Source model: `Model Generation/Runs/Pixal3D/HeroArthur01/Post/QuadRetro/arthur_royal_chad/Models/arthur_royal_chad_QuadRetro.glb`
- Source texture: `Model Generation/Runs/Pixal3D/HeroArthur01/Post/QuadRetro/arthur_royal_chad/Textures/arthur_royal_chad_QuadRetro_Pixelated_512.png`
- UAL source: `External/Quaternius/Universal Animation Library Source/UAL1.blend`
- Actions: `Idle_Loop`, `Walk_Formal_Loop`, `Jump_Start` plus `Jump_Loop` plus `Jump_Land`, and `Roll_RM`
- Tool-only row: `Hero_1_Chad_QuadRetroUALQA`
- Live row after accepted promotion: `Hero_1_Chad`
- Correct promoted row yaw: `MeshRelativeRotation=(Pitch=0,Yaw=90.000000,Roll=0)`
- Correct skeletal texture: original GLB-layout `RoyalChad_QuadRetro_Pixelated_512`, not the normalized static-mesh atlas

### Easy_Mob_VAT_20260514

Top-level files:

| File | Role |
| --- | --- |
| `Easy_Mob_VAT_Source.blend` | Blender source scene for the Easy mob VAT batch. Contains imported source GLBs, behavior-specific authoring rigs/actions, render setup, and export setup. |
| `Easy_Mob_VAT_Source.blend1` | Blender backup for the source scene. |
| `easy_mob_vat_manifest.json` | Batch manifest. Records Blender version, run root, source `.blend`, preview manifest path, export root, clip lengths, view names, ten mob records, source GLB paths, live static mesh paths, live scale, FBX export paths, weight counts, and bounds. |
| `Exports/<EnemyID>/` | Per-enemy FBX exports. Each enemy has one bake-source mesh FBX and five action FBXs. |
| `PreviewFrames/preview_manifest.json` | Large preview manifest for all mobs, clips, views, and frame images. |
| `PreviewFrames/contact_sheet_manifest.json` | Manifest for generated contact sheets. |
| `PreviewFrames/Easy_Mobs_AllClips_AllViews_Index_Contact_Sheet.png` | Index sheet showing all ten mobs across all clips and views. |
| `PreviewFrames/<EnemyID>/` | Per-enemy preview frames and contact sheets. Each enemy folder has 125 files: 100 individual frame renders and 25 contact sheets. |

Per-enemy export pattern:

| Enemy | Files in `Exports/<EnemyID>/` |
| --- | --- |
| `Slime` | `SKM_EasyMobVAT_Slime.fbx` plus `AM_EasyMobVAT_Slime_{Idle,Move,AttackCue,HitReact,Death}.fbx` |
| `BoneWalker` | `SKM_EasyMobVAT_BoneWalker.fbx` plus `AM_EasyMobVAT_BoneWalker_{Idle,Move,AttackCue,HitReact,Death}.fbx` |
| `RatPack` | `SKM_EasyMobVAT_RatPack.fbx` plus `AM_EasyMobVAT_RatPack_{Idle,Move,AttackCue,HitReact,Death}.fbx` |
| `CaveBat` | `SKM_EasyMobVAT_CaveBat.fbx` plus `AM_EasyMobVAT_CaveBat_{Idle,Move,AttackCue,HitReact,Death}.fbx` |
| `HexSlinger` | `SKM_EasyMobVAT_HexSlinger.fbx` plus `AM_EasyMobVAT_HexSlinger_{Idle,Move,AttackCue,HitReact,Death}.fbx` |
| `TombSpider` | `SKM_EasyMobVAT_TombSpider.fbx` plus `AM_EasyMobVAT_TombSpider_{Idle,Move,AttackCue,HitReact,Death}.fbx` |
| `StoneSentinel` | `SKM_EasyMobVAT_StoneSentinel.fbx` plus `AM_EasyMobVAT_StoneSentinel_{Idle,Move,AttackCue,HitReact,Death}.fbx` |
| `MimicLure` | `SKM_EasyMobVAT_MimicLure.fbx` plus `AM_EasyMobVAT_MimicLure_{Idle,Move,AttackCue,HitReact,Death}.fbx` |
| `BoneConjurer` | `SKM_EasyMobVAT_BoneConjurer.fbx` plus `AM_EasyMobVAT_BoneConjurer_{Idle,Move,AttackCue,HitReact,Death}.fbx` |
| `CryptWraith` | `SKM_EasyMobVAT_CryptWraith.fbx` plus `AM_EasyMobVAT_CryptWraith_{Idle,Move,AttackCue,HitReact,Death}.fbx` |

Shared clip layout:

| Clip | Frames |
| --- | --- |
| `Idle` | `0-59` |
| `Move` | `60-99` |
| `AttackCue` | `100-129` |
| `HitReact` | `130-149` |
| `Death` | `150-194` |

The manifest reports clip lengths as `Idle=48`, `Move=32`, `AttackCue=24`, `HitReact=16`, and `Death=36` for authoring/export sampling. The runtime batch doc records the AnimToTexture frame ranges above at sample rate 30 and rows per frame 4.

## Tools

Path:

```text
Model Generation/Rigging and Animation/Tools/
```

Source tools:

| Tool | Environment | Role |
| --- | --- | --- |
| `setup_rigging_animation_infrastructure.ps1` | PowerShell | Creates `External` layout, clones or pulls Rigodotify, builds `Rigodotify.zip` with the required top-level folder prefix, extracts Quaternius packages from Downloads, and optionally installs/enables Rigify and Rigodotify in Blender. |
| `inspect_animation_assets.py` | Blender Python | Imports or opens `.blend`, `.glb`, `.gltf`, and `.fbx` files and writes a JSON report of armatures, actions, meshes, and material/image findings. Handles Blender 5.1 layered actions by counting nested fcurves. |
| `create_arthur_quadretro_ual_animation_source.py` | Blender Python | Builds the Arthur/Royal Chad QuadRetro UAL retarget source. Imports the exact live QuadRetro GLB, imports UAL1, creates the target armature, assigns weights, retargets UAL actions, corrects roll direction, exports skeletal/action FBXs, saves the `.blend`, and writes the manifest. |
| `render_arthur_action_previews.py` | Blender Python | Renders sampled preview frames for Arthur action candidates from front, side, three-quarter, and gameplay views. Writes `preview_manifest.json`. |
| `make_preview_contact_sheets.py` | Python with Pillow | Builds Arthur per-action, per-view, all-view, and combined contact sheets from preview frames. |
| `import_arthur_quadretro_animation_to_unreal.py` | Unreal Python | Imports accepted Arthur FBXs into `/Game/Characters/Heroes/Hero_1/Chad/QuadRetroUALQA`, creates/updates skeletal-safe unlit material, writes QA row, optionally promotes live `Hero_1_Chad`, reloads `DT_CharacterVisuals`, and writes `Saved/ArthurQuadRetroAnimationImportReport.json`. |
| `verify_arthur_quadretro_animation_in_unreal.py` | Unreal Python | Verifies the Arthur skeletal mesh, skeleton, material parent, material texture parameters, animation sequences, temporary row, and promoted live row when `T66_ARTHUR_QUADRETRO_EXPECT_LIVE_PROMOTED=1`. Writes `Saved/ArthurQuadRetroAnimationVerifyReport.json`. |
| `create_easy_mob_vat_sources.py` | Blender Python | Builds the Easy mob VAT source scene. Imports ten source GLBs, creates behavior-specific armatures/actions, assigns weights, exports mesh/action FBXs, renders preview frames, saves the source `.blend`, and writes `easy_mob_vat_manifest.json`. |
| `make_easy_mob_contact_sheets.py` | Python with Pillow | Builds per-clip, per-view, per-enemy, all-view, and index contact sheets from Easy mob preview output. |
| `import_easy_mob_vat_to_unreal.py` | Unreal Python | Imports Easy mob authoring FBXs, runs UE 5.7 AnimToTexture in vertex mode, creates VAT static meshes/textures/material instances, writes `Content/Data/MobVertexAnimations.csv`, reloads `/Game/Data/DT_MobVertexAnimations`, and writes `Saved/EasyMobVATImportReport.json`. |
| `verify_easy_mob_vat_in_unreal.py` | Unreal Python | Verifies all ten Easy VAT rows, asset paths, frame ranges, texture dimensions, material parameters, fallback `CharacterVisuals.csv` preservation, and data table row names. Writes `Saved/EasyMobVATVerifyReport.json`. |

Generated tool cache:

| Entry | Role |
| --- | --- |
| `__pycache__/import_arthur_quadretro_animation_to_unreal.cpython-*.pyc` | Python bytecode cache generated by Unreal/Python execution. Not source. |
| `__pycache__/verify_arthur_quadretro_animation_in_unreal.cpython-*.pyc` | Python bytecode cache generated by Unreal/Python execution. Not source. |

## Runtime Seams Used By Rigging Work

The rigging folder does not own runtime gameplay by itself. It produces source assets and import artifacts that cross into T66 runtime through existing seams:

| Runtime area | Role |
| --- | --- |
| `Content/Data/CharacterVisuals.csv` | Hero and character visual rows. Arthur skeletal animation slots currently use `LoopingAnimation`, `AlertAnimation`, `RunAnimation`, and `RollAnimation`. |
| `/Game/Data/DT_CharacterVisuals` | Unreal data table reloaded from `CharacterVisuals.csv`. |
| `Content/Data/MobVertexAnimations.csv` | Dedicated CSV for Easy mob VAT rows. Do not overload skeletal animation fields for VAT clip state. |
| `/Game/Data/DT_MobVertexAnimations` | Runtime data table for mob VAT playback. |
| `Source/T66/Data/T66DataTypes.h` | Contains runtime row structures including `FT66MobVertexAnimationRow`. |
| `Source/T66/Gameplay/T66EnemyBase.*` | Enemy actor path that chooses VAT visual rows first, applies VAT playback, and keeps movement/collision/damage gameplay-owned. |
| `Source/T66/Gameplay/Enemies/T66EnemyFamilyResolver.cpp` | Defines currently implemented enemy family behavior mapping. Animation does not add missing behavior classes. |
| `UT66CharacterVisualSubsystem` | Applies character visual rows and mob VAT rows. |
| `Scripts/RunRiggingAnimationToolAndExit.py` | Wrapper used by Unreal Python automation when scripts live under folders with spaces. |
| `Scripts/StageStandaloneBuild.ps1` | Required when rigging changes become playable standalone content. |

## Known Current State

### Humanoid Arthur/Royal Chad

Current accepted proof:

- Blender source of truth: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_QuadRetro_UAL_Retarget.blend`
- Source mesh: exact live QuadRetro GLB for Royal Chad/Arthur
- Source animation library: `External/Quaternius/Universal Animation Library Source/UAL1.blend`
- Actions: Idle, Walk, Jump, Roll
- QA evidence: front, side, three-quarter, gameplay, and all-view contact sheets
- Unreal import tool: `Tools/import_arthur_quadretro_animation_to_unreal.py`
- Unreal verifier: `Tools/verify_arthur_quadretro_animation_in_unreal.py`
- Live row after promotion: `Hero_1_Chad`

Important guardrails:

- Do not reuse the deleted old `Hero_1_Chad_AnimPilot` path.
- Do not assign the normalized static texture atlas to the skeletal mesh.
- Do not carry the old static row yaw into the skeletal promotion.
- Do not switch to uncorrected `Roll` or uncorrected `Roll_RM`; the accepted roll uses source root motion stripped plus local-X sagittal mirror correction.
- Import to temporary QA row first, then promote live row only after Blender contact sheets and in-game QA pass.

### Easy Mob VAT Batch

Current accepted runtime QA path:

- Blender source of truth: `Runs/Easy_Mob_VAT_20260514/Easy_Mob_VAT_Source.blend`
- Ten mobs: `Slime`, `BoneWalker`, `RatPack`, `CaveBat`, `HexSlinger`, `TombSpider`, `StoneSentinel`, `MimicLure`, `BoneConjurer`, `CryptWraith`
- Runtime target: static mesh driven by vertex animation textures
- Dedicated data table: `DT_MobVertexAnimations`
- Runtime fallback: original `CharacterVisuals.csv` static rows remain preserved
- QA evidence: per-mob contact sheets, all-mob index sheet, Unreal import report, Unreal verification report, staged gameplay smoke screenshot/log

Important guardrails:

- Do not wire bake-only skeletal FBXs as live mob visuals.
- Do not use hero skeletal slots for VAT clip state.
- Do not claim animation implements missing archetype behavior. `Exploder`, `Turret`, `Necromancer`, and `Stutterer` still need gameplay classes if they are to behave differently.
- Use VAT UV channel 2. UV channel 1 conflicted with lightmap UVs.
- For generated material custom nodes, `TransformLocalVectorToWorld` must include `Parameters`.
- Full cook/stage is required after VAT material/content changes.

## Findings That Matter For Iteration

- Blender 5.1.1 is the current local Blender version.
- Rigify and Rigodotify are enabled locally.
- Rigodotify must be zipped with a top-level `Rigodotify/` prefix.
- Blender 5.1 action data may be layered; old scripts that only inspect `action.fcurves` can report zero actions incorrectly.
- Quaternius Source packages are the preferred editable animation sources. Standard packages are fallback/import references.
- Unreal commandlet FBX import can hit a Slate assertion in the current path. Full editor `UnrealEditor.exe -ExecutePythonScript=...` plus the rigging wrapper is the working import path.
- Backslash script paths and Windows paths in non-raw Python docstrings can break Unreal Python invocation. Prefer forward-slash script paths and raw docstrings for usage blocks.
- Do not chase shutdown-only Unreal editor exit codes if reports and verifiers already prove the import; inspect the report first.
- Multi-angle Blender contact sheets are the actual quality gate. A recognizable action from one camera is not enough.

## Current Process And Procedure

This is the current T66 model-rigging procedure that future agents should follow and improve.

### 1. Route The Task

Start with the repo goal and decide whether the target is:

- tool setup or vendor cache work
- humanoid hero/companion skeletal animation
- regular enemy or mob VAT animation
- Unreal import, data table, or runtime wiring
- standalone verification of gameplay-visible changes

Read the most specific instruction file before editing. For current rigging work, that usually means:

```text
Model Generation/Rigging and Animation/RIGGING_ANIMATION_AGENTS.md
Model Generation/Rigging and Animation/00_RIGGING_ANIMATION_ROUTING_INSTRUCTIONS.md
Model Generation/Rigging and Animation/03_FINDINGS_AND_LIMITATIONS_REFERENCE.md
```

Then read the specific pipeline file:

- Humanoid: `02_HERO_ANIMATION_PIPELINE_INSTRUCTIONS.md`
- Regular mobs/VAT: `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md`
- Easy mob batch: `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md`
- Unreal import: `Model Generation/Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`

### 2. Confirm Source Assets

For any character, identify the live runtime visual first:

- live row ID
- live static or skeletal mesh path
- live texture path
- live scale and rotation
- current gameplay selection path
- source GLB or source `.blend`
- accepted or exploratory status

Do not rig a convenient mesh unless it is proven to be the actual selected visual or an explicit replacement target.

### 3. Work In Blender Source Scenes

Use a `.blend` scene under a named run folder as the source of truth:

```text
Runs/<CharacterOrBatch>_<Purpose>_<YYYYMMDD>/
```

The scene should contain the target mesh, source/reference armature, target armature or VAT authoring rig, actions, cameras, export settings, and enough manifest data to reproduce the result.

Do not destructively edit vendor package scenes. Import, append, or link from `External`.

### 4. Author Or Retarget Motion

For humanoids:

- preserve the correct visible mesh and proportions
- prefer Rigify/Rigodotify-compatible humanoid naming and retarget maps
- retarget from Quaternius source actions when appropriate
- keep root/actor movement separated from in-place animation when gameplay owns movement
- bake clean action clips with readable start and recovery poses

For normal mobs:

- use rigs, shape keys, lattices, or controls only as authoring tools
- preserve stable vertex count and order for VAT
- use behavior-specific motion, not generic biped cycles
- keep runtime movement actor-driven
- export bake-only skeletal/action sources for AnimToTexture

### 5. Render Multi-Angle QA

Before Unreal import, render contact sheets:

- front
- side
- three-quarter
- gameplay camera when runtime camera changes the read

Review:

- silhouette readability
- foot or body contact
- timing and anticipation
- pelvis/head path
- shoulder, elbow, wrist, hip, knee, ankle collapse
- clipping and deformation
- prop, cape, robe, crown, weapon, or attachment stability
- start and recovery poses

If it fails, fix the Blender source and rerender. Do not hide a bad action in Unreal or accept a single still frame.

### 6. Export With A Manifest

Each accepted run should produce:

- source `.blend`
- manifest JSON
- exported mesh FBX or bake-source FBX
- exported action FBXs
- preview frames
- contact sheets
- notes on source paths, frame ranges, scale, rotation, texture, row IDs, and caveats

Durable lessons go into Markdown process docs, not only into run manifests.

### 7. Import Through The Domain Tool

Use the existing domain-specific import tool:

- Arthur/humanoid proof: `import_arthur_quadretro_animation_to_unreal.py`
- Easy mob VAT: `import_easy_mob_vat_to_unreal.py`

Use `Scripts/RunRiggingAnimationToolAndExit.py` for Unreal Python tools under paths with spaces. Prefer full editor for import/bake work when commandlet import hits known Slate assertions.

Import to temporary QA rows or QA asset paths first. Promote live rows only after visual QA and runtime proof.

### 8. Verify In Unreal

Run the matching verifier:

- Arthur: `verify_arthur_quadretro_animation_in_unreal.py`
- Easy VAT: `verify_easy_mob_vat_in_unreal.py`

Verification should prove:

- expected assets exist
- skeleton or VAT data is correct
- animation play lengths or frame ranges are nonzero and expected
- material parent and texture parameters are correct
- data rows point at imported assets
- data tables reload
- fallback rows are preserved where required
- runtime caveats are explicit

### 9. Prove Playable Changes In Standalone

If the rigging change affects the playable standalone build:

1. Refresh staged standalone with the repo staging script.
2. Verify `T66 Standalone.lnk` targets:

```text
C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

3. Smoke boot or capture runtime evidence relevant to the change.
4. Report the exact logs, screenshots, or verification reports used as evidence.

Docs-only changes, such as this report, do not require staged standalone refresh.

### 10. Keep The System Iterative

After each pass:

- update the specific pipeline doc if the process changed
- update `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md` with failures, fixes, and evidence
- delete one-off scripts after durable lessons are moved into reusable tools or docs
- keep vendor caches ignored
- keep run folders disposable unless they are the current source of truth
- document out-of-scope issues in `pending_issues_<foldername>.md` in the affected folder

Recommended next elevation step:

1. Add `Model Rigging/MODEL_RIGGING_AGENTS.md` as the new folder router.
2. Move or mirror the stable instruction docs from `Model Generation/Rigging and Animation` into `Model Rigging`.
3. Keep heavy `External` and `Runs` folders where they are until migration rules are decided.
4. Update `Model Generation/MODEL_GENERATION_AGENTS.md` and `Model Generation/Instructions/00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md` to route future editable rigging work to `Model Rigging`.
5. Only then migrate source-of-truth `.blend` runs or tools, with path updates and verification.
