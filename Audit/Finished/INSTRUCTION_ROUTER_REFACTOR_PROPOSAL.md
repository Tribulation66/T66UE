# Instruction Router Refactor Proposal

Created: 2026-05-13
Status: approved and implemented on 2026-05-13.
Scope: closed proposal and implementation record for the instruction-router refactor.

## Goal

Create a small folder-agent routing layer and clean the current instruction-style Markdown files so future agents can:

1. infer the owning folder from a task description,
2. read the folder router,
3. read all required instruction files,
4. avoid stale or contradictory workflow notes,
5. start work only after the relevant instructions are loaded.

This proposal intentionally separates routers, instructions, references, audits, reports, and generated verification artifacts.

## Current Markdown Surface

Current repo scan, excluding `Saved`, `Binaries`, `Intermediate`, `DerivedDataCache`, and `.git`:

- Total Markdown files: 233
- Root agent files: 1, `AGENTS.md`
- UI docs and generated verification artifacts: 134
- Gameplay docs: 29
- Audit docs: 23
- Model Generation docs: 17
- Content docs: 8
- Backend docs: 6
- SourceAssets docs: 6
- Release docs: 4
- Tools docs: 2
- Demo docs: 1
- RuntimeDependencies docs: 1
- Scripts docs: 1

The current problem is not missing documentation. The problem is that instruction-like files, reference files, historical files, and generated checklist files are mixed by name and role.

## Naming Taxonomy

Use these meanings going forward:

| Suffix | Meaning |
|---|---|
| `*_AGENTS.md` | Folder router. Tells agents when the folder owns a task and which instruction files to read next. |
| `*_INSTRUCTIONS.md` | Process or runbook. Tells agents how to execute a workflow. |
| `*_REFERENCE.md` | Durable facts, architecture notes, state snapshots, or source-of-truth maps. |
| `*_POLICY.md` | Durable policy with no step-by-step execution flow. Use sparingly; many policy docs can remain references. |
| `*_CHECKLIST.md` | Verification or implementation checklist. Not a router. |
| `*_AUDIT.md` | Investigation findings or review output. Not a live workflow unless promoted. |
| `*_REPORT.md` | Result of a completed run, experiment, or audit. |
| `README.md` | Folder index only. It may link to instructions, but it should not be the main workflow. |

Do not rename generated UI checklist or geometry artifacts just because they contain verification steps. Those are evidence/checklist inputs, not agent process files.

## Root AGENTS.md Proposal

Add this rule near the current `Goal Translation Rule`, before the UI-specific rule:

```md
## Folder Instruction Discovery Rule

- Before acting, infer which project folder owns the user's request. The user may describe the task by goal rather than by folder name.
- Use task wording, repo search, paths, READMEs, and existing docs to identify the responsible folder.
- Read that folder's `*_AGENTS.md` before editing files or running workflow commands.
- If the task crosses folders, read each relevant folder agent file and follow the most specific applicable instructions.
- Folder agent files are routers. They point to the required instruction files; they do not replace those files.
- If no folder agent exists, read the nearest `README.md` and relevant instruction docs, proceed conservatively, and report the missing router as a documentation gap.
```

Keep the existing root rules. The root file should stay short and should not become a project manual.

## Initial Router Set

Start with 10 router files. Add deeper routers later only after repeated work proves a folder needs its own rules.

| Proposed router | Why it deserves a router now |
|---|---|
| `UI/UI_AGENTS.md` | UI work has strict fidelity, capture, generated-art, and standalone rules. |
| `Model Generation/MODEL_GENERATION_AGENTS.md` | TRELLIS, RunPod, Quad Retro, Blender, and Unreal import have different evidence gates. |
| `Model Generation/Pixal3D/PIXAL3D_AGENTS.md` | Pixal3D is research-only and must not replace TRELLIS globally. |
| `Gameplay/GAMEPLAY_AGENTS.md` | Gameplay is the main source/router for combat, stats, movement, camera, traps, audio, world, and minigames. |
| `Gameplay/World/WORLD_AGENTS.md` | Tower map, modular kit generation, lighting, and HY-World research mix runtime and asset-generation rules. |
| `Gameplay/Minigames/MINIGAMES_AGENTS.md` | Mini, TD, Deck, Idle, and Versus need isolation boundaries. |
| `Backend/BACKEND_AGENTS.md` | Backend authority, anti-cheat, Steam auth, leaderboards, and Vercel work cross repos. |
| `Release/RELEASE_AGENTS.md` | Packaging, Steamworks upload, project guidelines, and staged/Steam validation have high operational risk. |
| `Demo/DEMO_AGENTS.md` | Demo AppID, demo content gates, and demo upload lane differ from full game release. |
| `Audit/AUDIT_AGENTS.md` | Audit files have status semantics: Pending, Finished, Reference. |

Do not add first-pass routers for `Source`, `Config`, `Content`, `SourceAssets`, `Scripts`, or `Tools` unless they become owning domains in a task. They are important work surfaces, but most tasks should route through UI, Gameplay, Model Generation, Backend, Release, or Demo first.

## Proposed Instruction Tree

Target shape after approved cleanup:

```text
AGENTS.md

UI/
  UI_AGENTS.md
  README.md
  Instructions/
    UI_FIDELITY_LOOP_INSTRUCTIONS.md
    UI_GENERATION_INSTRUCTIONS.md
    UI_SCREEN_MODAL_INSTRUCTIONS.md
    UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md
    UI_IMPLEMENTATION_INSTRUCTIONS.md
    UI_SCREEN_WORKFLOW_INSTRUCTIONS.md
    UI_MAIN_MENU_VIDEO_BACKGROUND_INSTRUCTIONS.md
    UI_SPRITE_RETRO_PROCESS_INSTRUCTIONS.md
  Reference/
    UI_FLAT_REDESIGN_REFERENCE.md
    UI_STAGE2_FRESH_AGENT_HANDOFF_REFERENCE.md
    UI_STAGE2_CAPTURE_READINESS_REFERENCE.md
    UI_MASTER_REFERENCE_GENERATION_PROMPT_REFERENCE.md
  Checklists/
  Geometry/
  Screen References/

Model Generation/
  MODEL_GENERATION_AGENTS.md
  README.md
  Instructions/
    00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md
    01_TRELLIS_RUNPOD_SETUP_INSTRUCTIONS.md
    02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md
    03_QUAD_RETRO_PIPELINE_INSTRUCTIONS.md
    04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md
    05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md
    06_RUN_HISTORY_AND_KNOWN_ISSUES_REFERENCE.md
    07_PIXAL3D_RUNPOD_SETUP_INSTRUCTIONS.md
    08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md
  Pixal3D/
    PIXAL3D_AGENTS.md
    PIXAL3D_PIPELINE_REFERENCE.md
  Scripts/
    README.md
  Tools/
    BlenderLabMCP/
      BLENDER_LAB_MCP_INSTRUCTIONS.md

Gameplay/
  GAMEPLAY_AGENTS.md
  README.md
  World/
    WORLD_AGENTS.md
    T66_MAP_DESIGN_REFERENCE.md
    T66_LIGHTING_REFERENCE.md
    MODULAR_DUNGEON_KIT_INSTRUCTIONS.md
    HY_WORLD_RESEARCH_REFERENCE.md
    T66_TOWER_MULTI_AGENT_IMPLEMENTATION_PLAN.md
  Minigames/
    MINIGAMES_AGENTS.md
    MINIGAME_CHARACTER_ANIMATION_INSTRUCTIONS.md
    Mini/
      T66MINI_IMPLEMENTATION_REFERENCE.md
      T66MINI_WALKSHEET_PIPELINE_INSTRUCTIONS.md
      T66MINI_UI_MIRROR_INSTRUCTIONS.md
    TD/
      T66TD_IMPLEMENTATION_REFERENCE.md
    Deck/
      T66DECK_IMPLEMENTATION_REFERENCE.md
    Idle/
      T66IDLE_IMPLEMENTATION_REFERENCE.md
  Combat/
    T66_COMBAT_REFERENCE.md
    ENEMY_BOSS_ROSTER_DATA_CONTRACT_REFERENCE.md
  Stats/
    T66_STATS_REFERENCE.md
    T66_PLAYER_EXPERIENCE_REFERENCE.md
  Movement/
    T66_MOVEMENT_REFERENCE.md
  Camera/
    T66_CAMERA_REFERENCE.md
  Traps/
    T66_TRAPS_REFERENCE.md
  Audio/
    T66_AUDIO_INFRASTRUCTURE_INSTRUCTIONS.md

Backend/
  BACKEND_AGENTS.md
  BACKEND_SYSTEM_REFERENCE.md
  Anti Cheat/
    ANTI_CHEAT_POLICY_REFERENCE.md
    ANTI_CHEAT_IMPLEMENTATION_INSTRUCTIONS.md
  Community/
    COMMUNITY_MODS_AND_CHALLENGES_REFERENCE.md
    COMMUNITY_MODS_AND_CHALLENGES_IMPLEMENTATION_PLAN.md

Release/
  RELEASE_AGENTS.md
  PROJECT_GUIDELINES_INSTRUCTIONS.md
  Steam/
    STEAMWORKS_UPLOAD_AND_TESTING_INSTRUCTIONS.md
  QA/
    T66_CONSOLE_COMMANDS_REFERENCE.md

Demo/
  DEMO_AGENTS.md
  DEMO_RELEASE_INSTRUCTIONS.md

Audit/
  AUDIT_AGENTS.md
  README.md
  Pending/
  Finished/
  Reference/
  Inventory Cleanup/
```

This is the target naming shape, not an instruction to perform every rename in one commit.

## Classification Table

### Routers

| Current path | Current classification | Proposal |
|---|---|---|
| `AGENTS.md` | Root router and global agent rules | Keep. Add folder instruction discovery rule. |
| Proposed `UI/UI_AGENTS.md` | Missing router | Add in first implementation pass. |
| Proposed `Model Generation/MODEL_GENERATION_AGENTS.md` | Missing router | Add in first implementation pass. |
| Proposed `Model Generation/Pixal3D/PIXAL3D_AGENTS.md` | Missing router | Add in first implementation pass. |
| Proposed `Gameplay/GAMEPLAY_AGENTS.md` | Missing router | Add in first implementation pass. |
| Proposed `Gameplay/World/WORLD_AGENTS.md` | Missing router | Add in first implementation pass. |
| Proposed `Gameplay/Minigames/MINIGAMES_AGENTS.md` | Missing router | Add in first implementation pass. |
| Proposed `Backend/BACKEND_AGENTS.md` | Missing router | Add in first implementation pass. |
| Proposed `Release/RELEASE_AGENTS.md` | Missing router | Add in first implementation pass. |
| Proposed `Demo/DEMO_AGENTS.md` | Missing router | Add in first implementation pass. |
| Proposed `Audit/AUDIT_AGENTS.md` | Missing router | Add in first implementation pass. |

### UI

| Current path | Classification | Proposed action |
|---|---|---|
| `UI/README.md` | Folder index | Keep as README, update links after moves. |
| `UI/T66_UI_FIDELITY_LOOP.md` | Instruction | Rename to `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`. |
| `UI/Processes/UI_GENERATION.md` | Instruction | Rename to `UI/Instructions/UI_GENERATION_INSTRUCTIONS.md`. |
| `UI/Processes/SCREEN_MODAL_TASK.md` | Instruction | Rename to `UI/Instructions/UI_SCREEN_MODAL_INSTRUCTIONS.md`. |
| `UI/Processes/LAYOUT_AND_SIZING.md` | Instruction | Rename to `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`. |
| `UI/Processes/IMPLEMENTATION.md` | Instruction | Rename to `UI/Instructions/UI_IMPLEMENTATION_INSTRUCTIONS.md`. |
| `UI/SCREEN_WORKFLOW.md` | Instruction checklist | Rename to `UI/Instructions/UI_SCREEN_WORKFLOW_INSTRUCTIONS.md`. |
| `UI/Processes/Main Menu Video Background.md` | Instruction | Rename to `UI/Instructions/UI_MAIN_MENU_VIDEO_BACKGROUND_INSTRUCTIONS.md`. |
| `UI/Sprites/Sprite Retro Process.md` | Instruction | Rename to `UI/Instructions/UI_SPRITE_RETRO_PROCESS_INSTRUCTIONS.md`. |
| `UI/MASTER_REFERENCE_UI_GENERATION_PROMPT.md` | Prompt/reference | Keep initially, later rename to `UI/Reference/UI_MASTER_REFERENCE_GENERATION_PROMPT_REFERENCE.md` or keep as explicit prompt if the user prefers. |
| `UI/T66_UI_FLAT_REDESIGN_PLAN.md` | Mixed reference and instruction | Split later. First keep as `UI/Reference/UI_FLAT_REDESIGN_REFERENCE.md` and let `UI_AGENTS.md` route to the fidelity loop for active procedure. |
| `UI/T66_UI_STAGE2_FRESH_AGENT_HANDOFF.md` | Handoff/reference | Move or rename to `UI/Reference/UI_STAGE2_FRESH_AGENT_HANDOFF_REFERENCE.md`. |
| `UI/stage2_capture_readiness.md` | Reference | Rename to `UI/Reference/UI_STAGE2_CAPTURE_READINESS_REFERENCE.md`. |
| `UI/screen_name_resolver_audit.md` | Audit | Keep or move to `Audit/Reference` only after links are updated. |
| `UI/hero_selection_closeout_and_stage2_readiness.md` | Report/reference | Keep as reference or move to `Audit/Reference/UI`. |
| `UI/HeroSelection_Stage2_Content_Differences.md` | Report/reference | Keep as reference or move to `Audit/Reference/UI`. |
| `UI/hud_and_ingame_ui_inventory.md` | Inventory/reference | Keep as reference or move to `Audit/Reference/UI`. |
| `UI/bUseGlow_audit.md` | Audit | Keep as audit/reference. |
| `UI/Checklists/*.md` | Generated checklists, 62 files | Do not rename in first pass. |
| `UI/Geometry/*.md` | Generated geometry/reference, 48 files | Do not rename in first pass. |
| `UI/Screen References/*.md` | Comparison reports, 5 files | Keep as reference/evidence; do not make active instructions. |

### Model Generation

| Current path | Classification | Proposed action |
|---|---|---|
| `Model Generation/README.md` | Folder index | Keep as README, update links after moves. |
| `Model Generation/Instructions/README.md` | Instruction index | Keep or rename later to `MODEL_GENERATION_INSTRUCTIONS_INDEX.md`; not urgent. |
| `Model Generation/Instructions/00_MASTER.md` | Router/instruction | Rename to `00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`. |
| `Model Generation/Instructions/01_TRELLIS_RUNPOD_SETUP.md` | Instruction | Rename to `01_TRELLIS_RUNPOD_SETUP_INSTRUCTIONS.md`. |
| `Model Generation/Instructions/02_SOURCE_IMAGE_RULES.md` | Instruction | Rename to `02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md`. |
| `Model Generation/Instructions/03_QUAD_RETRO_PIPELINE.md` | Instruction | Rename to `03_QUAD_RETRO_PIPELINE_INSTRUCTIONS.md`. |
| `Model Generation/Instructions/04_BLENDER_PROCESSING_AND_RIGGING.md` | Instruction | Rename to `04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md`. |
| `Model Generation/Instructions/05_UNREAL_IMPORT_AND_VALIDATION.md` | Instruction | Rename to `05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`. |
| `Model Generation/Instructions/06_RUN_HISTORY_AND_KNOWN_ISSUES.md` | Reference | Rename to `06_RUN_HISTORY_AND_KNOWN_ISSUES_REFERENCE.md`. |
| `Model Generation/Instructions/07_PIXAL3D_RUNPOD_SETUP.md` | Instruction | Rename to `07_PIXAL3D_RUNPOD_SETUP_INSTRUCTIONS.md`. |
| `Model Generation/Instructions/08_PIXAL3D_TROUBLESHOOTING.md` | Instruction | Rename to `08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md`. |
| `Model Generation/Pixal3D/README.md` | Pixal3D reference and folder index | Rename content to `PIXAL3D_PIPELINE_REFERENCE.md`; keep a short README if needed. |
| `Model Generation/Scripts/README.md` | Script lifecycle instruction | Keep as README for now; reference from router. |
| `Model Generation/Tools/BlenderLabMCP/README.md` | Tool setup instruction | Rename to `BLENDER_LAB_MCP_INSTRUCTIONS.md`. |
| `Model Generation/Experiments/*/Report.md` | Experiment reports | Keep as reports while experiments are active; delete or archive after lessons are promoted. |

### Gameplay

| Current path | Classification | Proposed action |
|---|---|---|
| `Gameplay/README.md` | Folder index | Keep as README. |
| `Gameplay/World/MASTER_MAP_DESIGN.md` | Reference | Rename later to `T66_MAP_DESIGN_REFERENCE.md`. |
| `Gameplay/World/MASTER_LIGHTING.md` | Reference | Rename later to `T66_LIGHTING_REFERENCE.md`. |
| `Gameplay/World/MODULAR_DUNGEON_KIT_PROCESS.md` | Instruction | Rename to `MODULAR_DUNGEON_KIT_INSTRUCTIONS.md`. Fix stale model-generation links first. |
| `Gameplay/World/HY_WORLD_RESEARCH.md` | Reference | Rename to `HY_WORLD_RESEARCH_REFERENCE.md`. Fix stale model-generation links first. |
| `Gameplay/World/T66_Tower_Multi_Agent_Implementation_Plan.md` | Task implementation plan | Keep as plan/reference. Do not make it a permanent instruction unless tower work resumes. |
| `Gameplay/Minigames/README.md` | Folder index | Keep as README. |
| `Gameplay/Minigames/T66Minigame_CharacterAnimationProcess.md` | Instruction | Rename to `MINIGAME_CHARACTER_ANIMATION_INSTRUCTIONS.md`. |
| `Gameplay/Minigames/Mini/T66Mini_MasterImplementation.md` | Implementation reference | Rename later to `T66MINI_IMPLEMENTATION_REFERENCE.md`. |
| `Gameplay/Minigames/Mini/T66Mini_WalksheetPipeline.md` | Instruction | Rename to `T66MINI_WALKSHEET_PIPELINE_INSTRUCTIONS.md`. |
| `Gameplay/Minigames/Mini/UI_Mirror_Process.md` | Instruction | Rename to `T66MINI_UI_MIRROR_INSTRUCTIONS.md`. |
| `Gameplay/Minigames/Mini/*Checklist.md` | Checklist | Keep as checklist. |
| `Gameplay/Minigames/Mini/*Memory_Progression.md` | Progress memory/reference | Keep as reference while active. |
| `Gameplay/Minigames/TD/T66TD_MasterImplementation.md` | Implementation reference | Rename later to `T66TD_IMPLEMENTATION_REFERENCE.md`. |
| `Gameplay/Minigames/TD/T66TD_Memory_Progression.md` | Progress memory/reference | Keep as reference while active. |
| `Gameplay/Minigames/Deck/T66Deck_MasterImplementation.md` | Implementation reference | Rename later to `T66DECK_IMPLEMENTATION_REFERENCE.md`. |
| `Gameplay/Minigames/Idle/T66Idle_MasterImplementation.md` | Implementation reference | Rename later to `T66IDLE_IMPLEMENTATION_REFERENCE.md`. |
| `Gameplay/Combat/MASTER_COMBAT.md` | Reference | Rename later to `T66_COMBAT_REFERENCE.md`. |
| `Gameplay/Combat/EnemyBossRoster_DataContract_2026-05-07.md` | Data contract reference | Rename later to `ENEMY_BOSS_ROSTER_DATA_CONTRACT_REFERENCE.md`. |
| `Gameplay/Stats/MASTER_STATS.md` | Reference | Rename later to `T66_STATS_REFERENCE.md`. |
| `Gameplay/Stats/MASTER_PLAYER_EXPERIENCE.md` | Reference | Rename later to `T66_PLAYER_EXPERIENCE_REFERENCE.md`. |
| `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md` | Audit | Keep as audit/reference. |
| `Gameplay/Movement/MASTER_MOVEMENT.md` | Reference | Rename later to `T66_MOVEMENT_REFERENCE.md`. |
| `Gameplay/Camera/MASTER_CAMERA.md` | Reference | Rename later to `T66_CAMERA_REFERENCE.md`. |
| `Gameplay/Traps/MASTER_TRAPS.md` | Reference with extension rules | Rename later to `T66_TRAPS_REFERENCE.md`. |
| `Gameplay/Audio/T66_Audio_Infrastructure.md` | Instruction/reference | Rename to `T66_AUDIO_INFRASTRUCTURE_INSTRUCTIONS.md` if it remains the audio setup runbook. |

### Backend, Release, Demo

| Current path | Classification | Proposed action |
|---|---|---|
| `Backend/README.md` | Folder index | Keep as README. |
| `Backend/MASTER_BACKEND.md` | Reference | Rename later to `BACKEND_SYSTEM_REFERENCE.md`. |
| `Backend/Anti Cheat/MASTER_ANTI_CHEAT.md` | Policy/reference | Rename later to `ANTI_CHEAT_POLICY_REFERENCE.md`. |
| `Backend/Anti Cheat/ANTI_CHEAT_IMPLEMENTATION_CHECKLIST.md` | Instruction/checklist | Rename later to `ANTI_CHEAT_IMPLEMENTATION_INSTRUCTIONS.md` or keep as checklist if execution is not active. |
| `Backend/Community/T66_Community_Mods_And_Challenges.md` | Product/architecture reference | Rename later to `COMMUNITY_MODS_AND_CHALLENGES_REFERENCE.md`. |
| `Backend/Community/T66_Community_Mods_And_Challenges_Implementation_Plan.md` | Implementation plan | Keep as plan until implementation starts. |
| `Release/README.md` | Folder index | Keep as README. |
| `Release/Project Guidelines.md` | Project policy/instruction | Rename later to `PROJECT_GUIDELINES_INSTRUCTIONS.md`. |
| `Release/Steam/MASTER_STEAMWORKS.md` | Steam upload/testing instruction/reference | Rename later to `STEAMWORKS_UPLOAD_AND_TESTING_INSTRUCTIONS.md`. |
| `Release/QA/T66_Console_Commands.md` | Reference | Keep or rename later to `T66_CONSOLE_COMMANDS_REFERENCE.md`. |
| `Demo/README.md` | Demo instruction/reference | Rename later to `DEMO_RELEASE_INSTRUCTIONS.md`, with a short README retained if needed. |
| `Tools/Release/Steam/README_DemoUpload.md` | Demo upload instruction | Merge into `Demo/DEMO_RELEASE_INSTRUCTIONS.md` or keep as tool-local instruction referenced by `DEMO_AGENTS.md`. |

### Scripts, Tools, Content, SourceAssets, RuntimeDependencies

| Current path | Classification | Proposed action |
|---|---|---|
| `Scripts/README.md` | Script lifecycle instruction/index | Keep. Root script lifecycle rule already points here conceptually. |
| `Tools/README.md` | Tool lifecycle instruction/index | Keep. |
| `Content/Mini/README.md` | Content ownership boundary | Keep. |
| `Content/Mini/Data/README.md` | Data ownership boundary | Keep. |
| `Content/TD/README.md` | Content ownership boundary | Keep. |
| `Content/TD/Data/README.md` | Data ownership boundary | Keep. |
| `Content/Deck/README.md` | Content ownership boundary | Keep. |
| `Content/Deck/Data/README.md` | Data ownership boundary | Keep. |
| `Content/Idle/Data/README.md` | Data ownership boundary | Keep. |
| `Content/UI/Leaderboard/README_Leaderboard_Filter_Assets.md` | Narrow setup instruction | Leave in place unless the leaderboard asset setup is revived. |
| `SourceAssets/Archive/*/README.md` | Archive/reference | Keep or delete only during source-asset cleanup. |
| `SourceAssets/Archive/TD/Maps/Backgrounds/TD_ImageGen_Prompts.md` | Prompt reference | Keep if TD image replacement is planned; otherwise archive. |
| `RuntimeDependencies/T66/UI/Reference/README.md` | Runtime asset boundary | Needs review: currently says `SourceAssets/UI/Reference/...` even though the path is under `RuntimeDependencies`. |

### Audit

| Current path | Classification | Proposed action |
|---|---|---|
| `Audit/README.md` | Audit index | Keep and update if routers are approved. |
| `Audit/Pending/*` | Active proposal/audit queue | Keep as pending while unresolved. |
| `Audit/Finished/*` | Closed audit/report | Do not rename unless stale references mislead agents. |
| `Audit/Reference/*` | Reference/audit history | Do not rename in first pass. |
| `Audit/Inventory Cleanup/T66_Project_Cleanup_Inventory.md` | Broad cleanup inventory | Keep as reference; do not treat older proposed moves inside it as current instructions without verifying live state. |

## Stale Or Contradictory Items Found

### Required Fixes

| File | Issue | Proposed fix |
|---|---|---|
| `Gameplay/World/HY_WORLD_RESEARCH.md` | References missing `Model Generation/Instructions/SHARED_ASSET_PIPELINE.md` and `Model Generation/Instructions/MASTER_WORKFLOW.md`. | Replace with current `Model Generation/Instructions/00_MASTER.md`, `01_TRELLIS_RUNPOD_SETUP.md`, and `05_UNREAL_IMPORT_AND_VALIDATION.md`. |
| `Gameplay/World/MODULAR_DUNGEON_KIT_PROCESS.md` | References missing `MASTER_WORKFLOW.md` and `SHARED_ASSET_PIPELINE.md`. | Replace with current model-generation instruction paths before or during the world cleanup pass. |
| `RuntimeDependencies/T66/UI/Reference/README.md` | It describes `SourceAssets/UI/Reference/...` paths even though the file lives in `RuntimeDependencies/T66/UI/Reference`. | Decide whether this README is stale copied text or whether runtime reference roots are intentionally mirrored. Update wording accordingly. |
| `SourceAssets/Archive/UI/Reference/README.md` | Same text as the runtime dependency README and appears archival. | Keep only as archive reference or delete during source-asset cleanup after confirming no active workflow points there. |
| `UI/T66_UI_FLAT_REDESIGN_PLAN.md` | Mixes locked design system, implementation plan, reference inventory, icon manifest, helper API notes, and quality gates. | Keep as reference in first pass. Later split the active quality gates into instructions if still current. |
| `UI/MASTER_REFERENCE_UI_GENERATION_PROMPT.md` | Self-contained prompt says it overrides older instructions if conflicts exist. | Keep as prompt/reference for now, but ensure `UI_AGENTS.md` makes `UI_FIDELITY_LOOP_INSTRUCTIONS.md` the gate for screen migration. |
| `Audit/Finished/T66_DOCS_CLEANUP_LEDGER.md` | Mentions old `MASTER DOCS` and `Docs` keep sets from an earlier cleanup state. | Keep as finished historical audit; do not treat it as current instruction. |
| `Audit/Inventory Cleanup/T66_Project_Cleanup_Inventory.md` | Contains old cleanup recommendations that are no longer current, including old folder moves and missing folders. | Keep as inventory/reference, but `AUDIT_AGENTS.md` should warn agents not to execute old recommendations without live verification. |

### Likely Non-Issues

| Pattern | Decision |
|---|---|
| Historical audit files mentioning `MASTER DOCS` or deleted `Docs` folders | Leave alone. They are historical evidence, not active instructions. |
| UI generated checklist and geometry files without `INSTRUCTIONS` suffix | Leave alone. They are target-specific verification artifacts. |
| `MASTER_*` gameplay files | Rename gradually only when touched. Most are references, not step-by-step instructions. |
| README files containing one or two operational notes | Leave until the folder router exists. Then make the README an index and move real procedure into an instruction file only if the workflow is active. |

## Router Drafts

These are draft contents only. They are intentionally short.

### `UI/UI_AGENTS.md`

```md
# UI Agents

## Owns

Frontend UI, Slate screens, screen/modal reference fidelity, generated UI chrome, captures, compare reports, layout sizing, top-bar screens, and UI runtime asset routing.

## Trigger Words

UI, screen, modal, Slate, frontend, reference image, capture, screenshot comparison, layout, top bar, hero selection, settings, Stage 2, fidelity, generated chrome, button plate, sprite sheet.

## Read First

- `UI/T66_UI_FIDELITY_LOOP.md` for reference-image UI migration.
- `UI/Processes/UI_GENERATION.md` for generated UI chrome.
- `UI/Processes/LAYOUT_AND_SIZING.md` for responsive layout rules.
- `UI/stage2_capture_readiness.md` for Stage 2 screen names and capture routing.
- `UI/MASTER_REFERENCE_UI_GENERATION_PROMPT.md` only when creating a fresh target prompt.

## Hard Rules

- Do not declare reference UI work complete without the fidelity loop.
- Do not run full UAT/package/stage for one screen unless the coordinating pass requires it.
- Do not bake live labels, player data, scores, or localized text into UI art.
- Do not use Pillow/PIL or local pixel repair for generated UI art.

## Verification

Use build, capture, compare, fidelity report, and resolution checks as the task requires.
```

### `Model Generation/MODEL_GENERATION_AGENTS.md`

```md
# Model Generation Agents

## Owns

TRELLIS, RunPod model generation, source-image rules, Quad Retro, Blender QA, rigging/retopo policy, Unreal mesh import, generated model cleanup, and model-generation scripts.

## Trigger Words

Trellis, RunPod, model generation, GLB, source image, Quad Retro, Blender QA, retopo, rigging, import meshes, generated meshes, environment kit, dungeon kit assets.

## Read First

- `Model Generation/Instructions/README.md`
- `Model Generation/Instructions/00_MASTER.md`
- Then the specific numbered instruction file for the task.

## Hard Rules

- Do not commit live secrets or pod-local access material.
- Do not keep raw generation output as a runtime dependency.
- Do not add one-off scripts when a manifest can drive an existing reusable script.
- If the generated asset affects the playable build, follow Unreal import and standalone validation instructions.

## Verification

Report pod health, HTTP generation evidence, nonzero artifact sizes, Blender import/QA evidence, Unreal import validation, and staged standalone evidence when applicable.
```

### `Model Generation/Pixal3D/PIXAL3D_AGENTS.md`

```md
# Pixal3D Agents

## Owns

The separate Pixal3D research pipeline only.

## Trigger Words

Pixal3D, TencentARC, CuMesh, Pixal3D RunPod, Pixal3D smoke, `run_pixal3d_batch.py`, `run_pixal3d_smoke.py`.

## Read First

- `Model Generation/Pixal3D/README.md`
- `Model Generation/Instructions/07_PIXAL3D_RUNPOD_SETUP.md`
- `Model Generation/Instructions/08_PIXAL3D_TROUBLESHOOTING.md`

## Hard Rules

- Pixal3D is research-only until licensing changes or legal approval is explicit.
- Do not replace TRELLIS globally.
- Do not import Pixal3D assets into shipped Unreal content unless explicitly approved.
- Do not debug Pixal3D failures by editing TRELLIS scripts.
- For multi-model runs, prefer the detached batch runner over one long foreground SSH loop.

## Verification

Verify pod reachability first, then service health, nonzero GLBs, JSONL/DONE logs, export headers/settings, and Blender import counts.
```

### `Gameplay/GAMEPLAY_AGENTS.md`

```md
# Gameplay Agents

## Owns

Gameplay runtime systems and their documentation: combat, stats, movement, camera, traps, audio, world/tower, and minigames.

## Trigger Words

Combat, boss, enemy, stats, XP, movement, dash, camera, trap, audio, tower, world, map, stage, minigame, Mini, TD, Deck, Idle, Versus.

## Read First

- `Gameplay/README.md`
- Then the owning subfolder router if it exists.
- If no subfolder router exists, read the relevant `MASTER_*` or implementation reference file before editing source.

## Hard Rules

- Prefer data-authored tuning over hardcoded C++ defaults.
- Preserve minigame isolation boundaries.
- Runtime-facing gameplay changes need compile/build verification and staged standalone validation when they affect the playable standalone.
```

### `Gameplay/World/WORLD_AGENTS.md`

```md
# World Agents

## Owns

Tower map design, lighting, world generation research boundaries, modular dungeon kit generation, and runtime integration of generated environment pieces.

## Trigger Words

World, tower, map, stage layout, lighting, HY-World, WorldMirror, modular dungeon kit, environment kit, wall mesh, floor mesh, ceiling mesh, generated kit.

## Read First

- `Gameplay/World/MASTER_MAP_DESIGN.md` for tower map/runtime design.
- `Gameplay/World/MASTER_LIGHTING.md` for lighting.
- `Gameplay/World/MODULAR_DUNGEON_KIT_PROCESS.md` for generated environment modules.
- `Gameplay/World/HY_WORLD_RESEARCH.md` for HY-World research boundaries.
- `Model Generation/Instructions/00_MASTER.md` and specific model-generation instructions when generating meshes.

## Hard Rules

- Do not treat HY-World as a production replacement for the current runtime terrain.
- Do not use full-room generation as the default modular-kit path.
- Keep gameplay collision authored through simple runtime proxies rather than deriving collision from high-poly generated visuals.
```

### `Gameplay/Minigames/MINIGAMES_AGENTS.md`

```md
# Minigames Agents

## Owns

Mini, TD, Deck, Idle, Versus, shared minigame process docs, minigame source/content/data isolation, and minigame animation workflows.

## Trigger Words

Minigame, Mini, Chadpocalypse TD, TD, Deck, Idle, Versus, bob loop, walksheet, animation atlas, tower-defense, deckbuilder, idle mode.

## Read First

- `Gameplay/Minigames/README.md`
- `Gameplay/Minigames/T66Minigame_CharacterAnimationProcess.md` for actor animation assets.
- The owning mode's implementation file before touching mode-specific runtime code.

## Hard Rules

- Keep modes isolated in their own source, content, source-asset, and documentation roots.
- Do not implement TD by extending Mini files in place.
- Do not put mode-specific data into another mode's content/data folder.
- Use the smallest useful animation scope first, such as `bob-only`, when that satisfies the mode.
```

### `Backend/BACKEND_AGENTS.md`

```md
# Backend Agents

## Owns

Backend authority, Vercel backend state, Steam auth, leaderboards, anti-cheat, account restrictions, multiplayer backend handoff, and community mods/challenges backend plans.

## Trigger Words

Backend, Vercel, leaderboard, anti-cheat, ranked, Steam auth, Web API ticket, submit-run, account status, community content, admin, multiplayer diagnostics.

## Read First

- `Backend/README.md`
- `Backend/MASTER_BACKEND.md`
- `Backend/Anti Cheat/MASTER_ANTI_CHEAT.md` for ranked/integrity/anti-cheat work.
- `Release/Steam/MASTER_STEAMWORKS.md` for Steam build/upload/private-test context.

## Hard Rules

- Backend-authoritative systems must not be replaced by client-local authority.
- Do not treat Steam trusted writes as anti-cheat.
- Update backend and Steam docs together when online/Steam operational state changes.
```

### `Release/RELEASE_AGENTS.md`

```md
# Release Agents

## Owns

Project policy, packaging, staged standalone, Steamworks upload, private testing, version naming, release validation, and console command references.

## Trigger Words

Release, package, stage, standalone, taskbar shortcut, Steam, Steamworks, SteamPipe, upload, private test, branch, build ID, version, tag, release.

## Read First

- `Release/README.md`
- `Release/Project Guidelines.md`
- `Release/Steam/MASTER_STEAMWORKS.md` for Steam operations.
- `Demo/README.md` for demo-specific release work.

## Hard Rules

- Packaged Development standalone is the runtime source of truth for runtime-facing changes.
- Refresh staged standalone and verify the taskbar shortcut when the change affects the playable standalone.
- Do not upload the inner `Saved/StagedBuilds/Windows/T66` folder to Steam. Upload the root staged Windows folder.
```

### `Demo/DEMO_AGENTS.md`

```md
# Demo Agents

## Owns

The Steam demo build lane, demo AppID/depot, demo content gates, demo staging, demo upload, and demo-specific validation.

## Trigger Words

Demo, Steam demo, Next Fest, demo AppID, demo depot, demo upload, demo content gate, `T66 Demo Standalone.lnk`.

## Read First

- `Demo/README.md`
- `Tools/Release/Steam/README_DemoUpload.md`
- `Release/Steam/MASTER_STEAMWORKS.md`

## Hard Rules

- Do not fork the Unreal project for the demo.
- Do not delete full-game rows to make the demo.
- Use the central release/content gate for demo availability.
- Verify the demo AppID path separately from the full-game AppID path.
```

### `Audit/AUDIT_AGENTS.md`

```md
# Audit Agents

## Owns

Audit organization, pending/finished/reference classification, cleanup inventories, and review packets.

## Trigger Words

Audit, cleanup inventory, pending, finished, reference, classify docs, review packet, stale docs, cleanup ledger.

## Read First

- `Audit/README.md`
- Active files under `Audit/Pending/` for in-progress work.
- Finished/reference files only as evidence, not as current instructions unless live repo checks confirm them.

## Hard Rules

- Do not execute old cleanup recommendations without verifying current repo state.
- Preserve user/current in-progress files unless explicitly asked to change them.
- Keep new review proposals in `Audit/Pending/` until approved or closed.
```

## Rename Map

This is the proposed rename map for active instruction-like docs. Apply in phases and update all links in the same phase.

### Phase 1: Routers And Stale Links

No existing files renamed. Add routers and update stale links only:

```text
add UI/UI_AGENTS.md
add Model Generation/MODEL_GENERATION_AGENTS.md
add Model Generation/Pixal3D/PIXAL3D_AGENTS.md
add Gameplay/GAMEPLAY_AGENTS.md
add Gameplay/World/WORLD_AGENTS.md
add Gameplay/Minigames/MINIGAMES_AGENTS.md
add Backend/BACKEND_AGENTS.md
add Release/RELEASE_AGENTS.md
add Demo/DEMO_AGENTS.md
add Audit/AUDIT_AGENTS.md

Gameplay/World/HY_WORLD_RESEARCH.md
  update missing Model Generation instruction links

Gameplay/World/MODULAR_DUNGEON_KIT_PROCESS.md
  update missing Model Generation instruction links

AGENTS.md
  add Folder Instruction Discovery Rule
```

### Phase 2: Model Generation And Pixal3D

```text
Model Generation/Instructions/00_MASTER.md
-> Model Generation/Instructions/00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md

Model Generation/Instructions/01_TRELLIS_RUNPOD_SETUP.md
-> Model Generation/Instructions/01_TRELLIS_RUNPOD_SETUP_INSTRUCTIONS.md

Model Generation/Instructions/02_SOURCE_IMAGE_RULES.md
-> Model Generation/Instructions/02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md

Model Generation/Instructions/03_QUAD_RETRO_PIPELINE.md
-> Model Generation/Instructions/03_QUAD_RETRO_PIPELINE_INSTRUCTIONS.md

Model Generation/Instructions/04_BLENDER_PROCESSING_AND_RIGGING.md
-> Model Generation/Instructions/04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md

Model Generation/Instructions/05_UNREAL_IMPORT_AND_VALIDATION.md
-> Model Generation/Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md

Model Generation/Instructions/06_RUN_HISTORY_AND_KNOWN_ISSUES.md
-> Model Generation/Instructions/06_RUN_HISTORY_AND_KNOWN_ISSUES_REFERENCE.md

Model Generation/Instructions/07_PIXAL3D_RUNPOD_SETUP.md
-> Model Generation/Instructions/07_PIXAL3D_RUNPOD_SETUP_INSTRUCTIONS.md

Model Generation/Instructions/08_PIXAL3D_TROUBLESHOOTING.md
-> Model Generation/Instructions/08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md

Model Generation/Pixal3D/README.md
-> Model Generation/Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md
```

### Phase 3: UI

```text
UI/T66_UI_FIDELITY_LOOP.md
-> UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md

UI/Processes/UI_GENERATION.md
-> UI/Instructions/UI_GENERATION_INSTRUCTIONS.md

UI/Processes/SCREEN_MODAL_TASK.md
-> UI/Instructions/UI_SCREEN_MODAL_INSTRUCTIONS.md

UI/Processes/LAYOUT_AND_SIZING.md
-> UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md

UI/Processes/IMPLEMENTATION.md
-> UI/Instructions/UI_IMPLEMENTATION_INSTRUCTIONS.md

UI/SCREEN_WORKFLOW.md
-> UI/Instructions/UI_SCREEN_WORKFLOW_INSTRUCTIONS.md

UI/Processes/Main Menu Video Background.md
-> UI/Instructions/UI_MAIN_MENU_VIDEO_BACKGROUND_INSTRUCTIONS.md

UI/Sprites/Sprite Retro Process.md
-> UI/Instructions/UI_SPRITE_RETRO_PROCESS_INSTRUCTIONS.md

UI/T66_UI_FLAT_REDESIGN_PLAN.md
-> UI/Reference/UI_FLAT_REDESIGN_REFERENCE.md

UI/T66_UI_STAGE2_FRESH_AGENT_HANDOFF.md
-> UI/Reference/UI_STAGE2_FRESH_AGENT_HANDOFF_REFERENCE.md

UI/stage2_capture_readiness.md
-> UI/Reference/UI_STAGE2_CAPTURE_READINESS_REFERENCE.md
```

### Phase 4: Gameplay World And Minigames

```text
Gameplay/World/MODULAR_DUNGEON_KIT_PROCESS.md
-> Gameplay/World/MODULAR_DUNGEON_KIT_INSTRUCTIONS.md

Gameplay/World/HY_WORLD_RESEARCH.md
-> Gameplay/World/HY_WORLD_RESEARCH_REFERENCE.md

Gameplay/World/MASTER_MAP_DESIGN.md
-> Gameplay/World/T66_MAP_DESIGN_REFERENCE.md

Gameplay/World/MASTER_LIGHTING.md
-> Gameplay/World/T66_LIGHTING_REFERENCE.md

Gameplay/Minigames/T66Minigame_CharacterAnimationProcess.md
-> Gameplay/Minigames/MINIGAME_CHARACTER_ANIMATION_INSTRUCTIONS.md

Gameplay/Minigames/Mini/T66Mini_WalksheetPipeline.md
-> Gameplay/Minigames/Mini/T66MINI_WALKSHEET_PIPELINE_INSTRUCTIONS.md

Gameplay/Minigames/Mini/UI_Mirror_Process.md
-> Gameplay/Minigames/Mini/T66MINI_UI_MIRROR_INSTRUCTIONS.md
```

### Phase 5: Backend, Release, Demo

```text
Backend/MASTER_BACKEND.md
-> Backend/BACKEND_SYSTEM_REFERENCE.md

Backend/Anti Cheat/MASTER_ANTI_CHEAT.md
-> Backend/Anti Cheat/ANTI_CHEAT_POLICY_REFERENCE.md

Backend/Anti Cheat/ANTI_CHEAT_IMPLEMENTATION_CHECKLIST.md
-> Backend/Anti Cheat/ANTI_CHEAT_IMPLEMENTATION_INSTRUCTIONS.md

Release/Project Guidelines.md
-> Release/PROJECT_GUIDELINES_INSTRUCTIONS.md

Release/Steam/MASTER_STEAMWORKS.md
-> Release/Steam/STEAMWORKS_UPLOAD_AND_TESTING_INSTRUCTIONS.md

Demo/README.md
-> Demo/DEMO_RELEASE_INSTRUCTIONS.md
```

For `Demo/README.md`, either keep a new short `README.md` that points to `DEMO_RELEASE_INSTRUCTIONS.md`, or leave the existing README in place if the user prefers fewer file moves.

## Execution Order

Recommended approval and implementation order:

1. Add root folder-discovery rule and first 10 router files.
2. Fix live stale links in `Gameplay/World/HY_WORLD_RESEARCH.md` and `Gameplay/World/MODULAR_DUNGEON_KIT_PROCESS.md`.
3. Clean Model Generation and Pixal3D names.
4. Clean UI instruction names and update all UI links.
5. Clean Gameplay/World and Minigames names.
6. Clean Backend, Release, and Demo names.
7. Re-run stale-link search and validate all router targets exist.
8. Leave historical audits, generated UI checklists, geometry files, and finished reports alone unless they actively mislead current work.

## Verification Plan For The Actual Refactor

For each phase:

```powershell
rg -n "old/path/or/filename" -g "*.md"
rg --files -g "*.md" | rg "expected new name"
git diff --check
```

For the whole pass:

```powershell
rg -n "MASTER_WORKFLOW|SHARED_ASSET_PIPELINE|MASTER DOCS|Docs/|World Generation/" -g "*.md" -g "!Audit/Reference/**" -g "!Audit/Finished/**"
rg -n "\\*_AGENTS.md|_INSTRUCTIONS.md|_REFERENCE.md" AGENTS.md UI Model\ Generation Gameplay Backend Release Demo Audit
git status --short
```

Docs-only changes do not require Unreal build, cook, stage, or standalone verification unless a runtime file or build script changes.

## Approval Questions

Approve or modify these before implementation:

1. Should folder routers use `FOLDERNAME_AGENTS.md` exactly, or should nested folders use conventional plain `AGENTS.md`?
2. Should `README.md` files that currently contain real workflows be renamed, or should we keep README names where they are already short and folder-local?
3. Should `MASTER_*` files be renamed to `*_REFERENCE.md` in this pass, or should only active instruction files move first?
4. Should UI prompt files keep `PROMPT` in the filename, or become `*_REFERENCE.md` files under `UI/Reference`?
5. Should historical audit files be excluded from stale-link enforcement, as proposed here?

## Recommendation

Approve Phase 1 first. It gives agents the new discovery behavior and fixes confirmed stale world/model-generation references without risking a broad rename.

After Phase 1 works, proceed with Model Generation/Pixal3D because that folder already has the cleanest instruction structure. UI should come next because it has the highest agent workflow risk and the most active instruction files.
