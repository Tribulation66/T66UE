# Model Generation Workspace

This folder is the durable home for T66 model-generation process docs, reusable TRELLIS/Blender helpers, and batch scripts.

## Start Here

Read [Instructions/README.md](Instructions/README.md) first. It is the canonical instruction index for TRELLIS, Quad Retro, Blender processing, Unreal import, and cleanup policy.

## Workspace Shape

- `Instructions/`: current process docs only.
- `Scripts/`: reusable core helpers, named batch drivers, and clearly marked legacy scripts.
- `Tools/`: TRELLIS server files, Blender MCP helpers, and local tool launchers.

Generated runs, Blender scenes, archives, local access files, and preview outputs do not belong here long-term. Once an asset is imported, verified, or rejected, keep only the durable rule or summary in `Instructions/` or `Scripts/README.md`; delete the generated output folder.

## Cleanup Policy

- Do not commit live secrets or pod-local access material.
- Do not keep raw TRELLIS/Blender output as a runtime dependency.
- Move reusable lessons from task scripts into a master script, manifest format, or instruction doc.
- Delete one-off scripts after the task is complete and the durable lesson has been captured.
