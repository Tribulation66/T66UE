# Scripts

`Scripts` contains project automation that is called by Unreal, import workflows, data-table setup, staging, capture, and validation. Keep live callable scripts in this root until their source/docs callers are updated to a new path.

## Lifecycle Rule

Master scripts are reusable project tools. One-off task scripts should be deleted after the task is proven complete, with any durable lesson folded into an existing master script, a manifest format, a new reusable tool, or a canonical doc.

## Current Master Areas

- Build and package helpers: `StageStandaloneBuild.ps1`, `GuardT66RuntimeAssetContract.ps1`.
- UI capture/import helpers: `CaptureT66UIScreen.ps1`, UI texture import and repair scripts.
- Data-table setup scripts: `Setup*DataTable.py`, roster/data reload helpers.
- Import core: `ImportStaticMeshes.py`, `ImportSkeletalMeshes.py`, `VerifyImportBatch.py`, material flatten/unlit helpers.
- Active batch wrappers: current Quad Retro, combat roster, weapon projectile, coherent theme kit, arcade replacement, and world NPC/interactable imports.
- Maintenance: focused audit, repair, and verification scripts that are still used by current docs or source-owned tooling.

## Cleanup Boundary

Do not add new task-specific scripts here by default. Prefer a manifest plus an existing master script. If a custom script is unavoidable, delete it after the task is complete unless it has been promoted into a reusable master.
