# T66 Rigging And Animation

This folder is the repo-native process for Codex-controlled character rigging and animation work.

The goal is to make requests like "give Arthur all his animations" executable through a repeatable Blender pipeline, not through one-off manual sessions.

## Current Direction

- Blender is the source of truth for editable rigs and actions.
- Rigify is the base rigging system.
- Rigodotify is installed on top of Rigify to create game-engine-friendly skeletons for Unreal-compatible retargeting.
- Quaternius animation packs are local reference/action sources.
- T66 humanoid hero and companion animations should be authored or retargeted in Blender, QA-rendered from multiple angles, corrected from the findings, exported, imported into Unreal, and only then wired into gameplay data/runtime.
- Regular enemies and mobs use the vertex baked animation pipeline in `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md`, not the humanoid skeletal runtime pipeline.
- Difficulty 1 / Easy mobs are the first mob VAT batch and are tracked in `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md`.

## Local Tool Cache

The local vendor cache is ignored by git:

```text
Model Generation/Rigging and Animation/External/
```

Refresh the cache and Blender add-ons with:

```powershell
.\Model Generation\Rigging and Animation\Tools\setup_rigging_animation_infrastructure.ps1
```

Current expected packages:

- `Rigodotify` cloned from GitHub.
- `Universal Animation Library[Standard].zip` extracted from Downloads.
- `Universal Animation Library 2[Standard].zip` extracted from Downloads.
- `Universal Base Characters[Standard].zip` extracted from Downloads.
- `Universal Animation Library[Source].zip` extracted from Downloads when available.
- `Universal Animation Library 2[Source].zip` extracted from Downloads when available.
- `Universal Base Characters[Source].zip` extracted from Downloads when available.

## Current Asset Baseline

The Source packages are preferred for full hero animation work because they include editable `.blend` files such as `UAL1.blend`, `UAL2.blend`, and base-character source scenes. The Standard packages remain useful as import/reference checks and as a fallback when a source package is unavailable.

For Royal Chad/Arthur, the accepted playable source is not the old skeletal pilot. The live visual was traced to `Hero_1_Chad` in `Content/Data/CharacterVisuals.csv`, which originally resolved to the QuadRetro static mesh and normalized Royal Chad texture. The correct source mesh for the accepted animated pass is:

- Source GLB: `../Runs/Pixal3D/HeroArthur01/Post/QuadRetro/arthur_royal_chad/Models/arthur_royal_chad_QuadRetro.glb`
- Source texture: `../Runs/Pixal3D/HeroArthur01/Post/QuadRetro/arthur_royal_chad/Textures/arthur_royal_chad_QuadRetro_Pixelated_512.png`
- Unreal skeletal texture: `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/RoyalChad_QuadRetro/Textures/RoyalChad_QuadRetro_Pixelated_512.RoyalChad_QuadRetro_Pixelated_512`
- UAL source: `External/Quaternius/Universal Animation Library Source/UAL1.blend`
- UAL action sources: `Idle_Loop`, `Walk_Formal_Loop`, `Jump_Start` + `Jump_Loop` + `Jump_Land`, and `Roll_RM`
- Blender source of truth: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_QuadRetro_UAL_Retarget.blend`
- Unreal skeletal mesh: `/Game/Characters/Heroes/Hero_1/Chad/QuadRetroUALQA/SK_Hero_1_Chad_QuadRetroUALQA.SK_Hero_1_Chad_QuadRetroUALQA`
- Unreal actions: `/Game/Characters/Heroes/Hero_1/Chad/QuadRetroUALQA/AM_Hero_1_Chad_QuadRetroUALQA_{Idle,Walk,Jump,Roll}`

Important Arthur reimport guard: the GLB-derived skeletal mesh uses the original GLB texture atlas. Do not assign `RoyalChad_QuadRetro_Pixelated_512_Normalized` to the skeletal material or live skeletal row; that normalized atlas is for the old static mesh layout and will scramble the animated hero texture. The promoted skeletal row also needs `MeshRelativeRotation=(Pitch=0,Yaw=90.000000,Roll=0)` so gameplay movement and the visible forward direction match. The accepted roll action uses UAL `Roll_RM` with root motion stripped plus the script's local-X sagittal mirror correction; using `Roll` or `Roll_RM` without that correction reads as a backward flip after in-place baking.

The legacy `Hero_1_Chad_AnimPilot` experiment, its old root-level Arthur skeletal assets, and the rejected manual/procedural `QuadRetroAnimQA` pass were deleted after the UAL retarget was validated. Do not recreate those paths unless there is a new, explicit research need.

For Easy mobs, the original fallback rows remain static-only `CharacterVisuals.csv` rows under `/Game/Characters/Mobs/<EnemyID>/SM_<EnemyID>`. The current VAT QA/runtime path is separate:

- Blender source of truth: `Runs/Easy_Mob_VAT_20260514/Easy_Mob_VAT_Source.blend`
- Contact sheet index: `Runs/Easy_Mob_VAT_20260514/PreviewFrames/Easy_Mobs_AllClips_AllViews_Index_Contact_Sheet.png`
- Unreal VAT root: `/Game/Characters/MobsVAT/<EnemyID>/`
- Runtime CSV: `Content/Data/MobVertexAnimations.csv`
- Runtime data table: `/Game/Data/DT_MobVertexAnimations.DT_MobVertexAnimations`
- Verification report: `Saved/EasyMobVATVerifyReport.json`
- Staged smoke screenshot: `Saved/StandaloneLogs/EasyMobVAT_GameplaySmoke_WithAudio.png`
- Staged smoke log: `Saved/StandaloneLogs/EasyMobVAT_GameplaySmoke_WithAudio.log`

Source GLBs for the first batch exist under `Model Generation/Production/Roster_v1/AgentA` and `Model Generation/Production/Roster_v1/AgentB`. Use those sources for process exploration and in-game QA; promote them to final VAT source only after visual and runtime acceptance.

## Pipeline Contract

The core humanoid pipeline is `02_HERO_ANIMATION_PIPELINE_INSTRUCTIONS.md`. Despite the historical filename, it is now the active process for heroes and humanoid companions.

The core regular-enemy pipeline is `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md`. Its runtime target is a static mesh driven by vertex animation textures/material data, with Blender rigs used only as editable source and bake inputs.

The important gate is visual QA in Blender:

- render front, side, three-quarter, and gameplay-camera contact sheets
- inspect the frames for silhouette, contact, timing, deformation, clipping, and recovery
- make Blender-side corrections based on the findings
- rerender after corrections
- only then export and import to Unreal

If a run only has a single-angle preview or no correction pass, report it as a playable prototype or first pass, not a production-accepted animation set.

Current Royal Chad/Arthur QA evidence:

- `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_All_Actions_front_Contact_Sheet.png`
- `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_All_Actions_side_Contact_Sheet.png`
- `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_All_Actions_three_quarter_Contact_Sheet.png`
- `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_All_Actions_gameplay_Contact_Sheet.png`
- `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_All_Actions_All_Views_Contact_Sheet.png`

These sheets were used before Unreal import to catch static-action, camera, source-pose, and deformation mistakes. The accepted pass uses UAL timing, preserves the crown, head, armor, weapon, proportions, and former live scale, and is promoted through the live `Hero_1_Chad` row only after temporary-row gameplay QA.

## Related T66 Docs

- `../Instructions/04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md`
- `../Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`
- `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md`
- `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md`
- `../../Gameplay/Movement/MASTER_MOVEMENT.md`
