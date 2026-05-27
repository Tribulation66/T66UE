# Model Generation Master

This workspace owns model-generation process, not runtime game content. A model is not part of the playable build until it is imported through the Unreal scripts, validated, cooked, staged, and the standalone shortcut points at the staged executable when the change affects playable standalone output.

## Decision Tree

1. For a new source image or character direction, start with `02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md`.
   - For humanoid heroes, companions, or manually rigged characters, continue with `10_HUMANOID_IMAGE_GENERATION_GUIDELINES.md` before writing the live prompt.
2. For TRELLIS RunPod setup, use `01_TRELLIS_RUNPOD_SETUP_INSTRUCTIONS.md`.
3. For Quad Retro processing, use `03_QUAD_RETRO_PIPELINE_INSTRUCTIONS.md`.
4. For Blender cleanup, rigging, or retopo policy, use `04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md`.
   - For editable character rigs, retargeting, Rigodotify, Quaternius, or authored hero/enemy animation sets, route into `../Rigging and Animation/RIGGING_ANIMATION_AGENTS.md`.
5. For Unreal import, DataTable reloads, material checks, or standalone verification, use `05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`.
6. For known failure modes and historical lessons, use `06_RUN_HISTORY_AND_KNOWN_ISSUES_REFERENCE.md`.
7. For Pixal3D production replacement assets, read `09_PIXAL3D_TOONSTYLE_PRODUCTION_IMPORT_INSTRUCTIONS.md` first, then `../Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md`, `07_PIXAL3D_RUNPOD_SETUP_INSTRUCTIONS.md`, and `08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md` for CuMesh/export/remesh failures.

## Folder Rules

- `Scripts/Core`: reusable helpers only.
- `Scripts/Batches`: named batch drivers; delete after the batch is done.
- `Scripts/Legacy`: old prototype helpers; keep only while they document an unresolved workflow.
- `Tools`: server and Blender helper assets.
- Generated `Runs`, `Scenes`, `Archive`, `Reference`, and local access files are cleanup targets, not durable source.

## Script Lifecycle

Do not add a new one-off script when a manifest can drive an existing master. If a one-off is needed, delete it after the task succeeds or fails conclusively, and move the durable lesson into a core script, README, or instruction doc.
