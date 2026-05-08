# Model Generation Master

This workspace owns model-generation process, not runtime game content. A model is not part of the playable build until it is imported through the Unreal scripts, validated, cooked, staged, and the standalone shortcut points at the staged executable when the change affects playable standalone output.

## Decision Tree

1. For a new source image or character direction, start with `02_SOURCE_IMAGE_RULES.md`.
2. For TRELLIS or RunPod setup, use `01_TRELLIS_RUNPOD_SETUP.md`.
3. For Quad Retro processing, use `03_QUAD_RETRO_PIPELINE.md`.
4. For Blender cleanup, rigging, or retopo policy, use `04_BLENDER_PROCESSING_AND_RIGGING.md`.
5. For Unreal import, DataTable reloads, material checks, or standalone verification, use `05_UNREAL_IMPORT_AND_VALIDATION.md`.
6. For known failure modes and historical lessons, use `06_RUN_HISTORY_AND_KNOWN_ISSUES.md`.

## Folder Rules

- `Scripts/Core`: reusable helpers only.
- `Scripts/Batches`: named batch drivers; delete after the batch is done.
- `Scripts/Legacy`: old prototype helpers; keep only while they document an unresolved workflow.
- `Tools`: server and Blender helper assets.
- Generated `Runs`, `Scenes`, `Archive`, `Reference`, and local access files are cleanup targets, not durable source.

## Script Lifecycle

Do not add a new one-off script when a manifest can drive an existing master. If a one-off is needed, delete it after the task succeeds or fails conclusively, and move the durable lesson into a core script, README, or instruction doc.
