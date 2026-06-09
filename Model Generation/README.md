# Model Generation Workspace

This folder is the durable home for T66 model-generation process docs, reusable TRELLIS/Blender helpers, and batch scripts.

## Start Here

Read [Instructions/README.md](Instructions/README.md) first. It is the canonical instruction index for TRELLIS, Pixal3D, FriendSlop raw import, Blender processing, Unreal import, and cleanup policy.

## Workspace Shape

- `Instructions/`: current process docs only.
- `Scripts/`: reusable core helpers, named batch drivers, and clearly marked legacy scripts.
- `Tools/`: TRELLIS server files, Blender MCP helpers, and local tool launchers.
- `Pixal3D/`: production-cleared Pixal3D pipeline, server, bootstrap, smoke-test tooling, and FriendSlop raw import references.

Root `ART_DIRECTION.md` declares FriendSlop as the active 3D/world art direction. Historical ToonStyle and QuadRetro process docs are archived under `Archive/ToonStyle/` and `Archive/RetroFX/`.

Generated runs, Blender scenes, archives, local access files, and preview outputs do not belong here long-term. Once an asset is imported, verified, or rejected, keep only the durable rule or summary in `Instructions/` or `Scripts/README.md`; delete the generated output folder.

## Current FriendSlop Source Set

For current FriendSlop model work, use the raw Pixal3D FriendSlop runs, not the older AccuRig / Animated ToonStyle hero demo runs.

Active source runs:

- `Runs/Pixal3D/FriendSlopEasyBatch_20260604_1532`: current Easy-difficulty FriendSlop raw Pixal3D batch. Its manifest records 49 assets and its `Outputs/` folder contains the 49 source `.glb` files.
- `Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415`: current Hero 1 male FriendSlop probe. This is the source for `Hero_1_Chad` and `Hero_1_Chad_DemoSkin`.

Runtime import copies for the 49-asset Easy batch also live at `SourceAssets/Import/FriendSlop/Pixal3D/FriendSlopEasyBatch_20260604_1532`.

Do not use `HeroDemoLineup_20260522_AccuRig` or other archived AccuRig / ToonStyle hero demo runs as current model sources. Those are archived historical provenance only. A filename or report containing `AccuRig_Textured` inside a FriendSlop raw run is exporter-stage naming, not approval to use the archived AccuRig hero lineup.

Current known Hero 1 state:

- Hero 1 male uses the FriendSlop raw static mesh path in `Content/Data/CharacterVisuals.csv`.
- Hero 1 female does not currently have a generated FriendSlop 3D model in these runs. Its current runtime row still points at the older Stacy Animated ToonStyle / Pixal3D ToonStyle assets.

## Cleanup Policy

- Do not commit live secrets or pod-local access material.
- Do not keep raw TRELLIS/Blender output as a runtime dependency.
- Move reusable lessons from task scripts into a master script, manifest format, or instruction doc.
- Delete one-off scripts after the task is complete and the durable lesson has been captured.

## Model Cleanup And Organization

Use [Instructions/12_MODEL_CLEANUP_AND_ORGANIZATION_INSTRUCTIONS.md](Instructions/12_MODEL_CLEANUP_AND_ORGANIZATION_INSTRUCTIONS.md) after model replacement/import passes. The durable pattern is:

- Run broad character/world audits to find candidates.
- Build `Reports/Hygiene/<date>/model_cleanup_candidate_manifest.json`.
- Run exact package-path reference proof with `Scripts/AuditAssetReferencesAndExit.py`.
- Convert that proof into a deletion gate with `Scripts/EvaluateModelCleanupExactAudit.py`.
- Delete only approved runtime packages with `Scripts/DeleteModelCleanupAssetsAndExit.py`.
- Clean non-durable generated retry/smoke/guideline runs with `Scripts/CleanModelGenerationRuns.py`.

Skipped runtime candidates stay in place with their gate reasons. Generated folders marked `keep_review` stay until provenance is settled.
