# Model Generation Scripts

Scripts in this folder are grouped by durability.

## Layout

- `Core/Trellis/`: reusable RunPod/TRELLIS setup helpers.
- `Core/Blender/`: reusable headless Blender QA/render/export helpers.
- `Core/QuadRetro/`: reusable Quad Retro processing engine and wrapper.
- `Core/Environment/`: reusable environment-kit source splitting or preparation helpers.
- `Batches/<Domain>/<BatchName>/`: named batch drivers with hardcoded run roots, row orders, pod IDs, or asset maps.

## Lifecycle Rule

Reusable core scripts should stay tight and parameterized. Batch scripts are temporary. When a batch is imported, verified, or rejected, move any durable process improvement into a core script, manifest format, or instruction doc, then delete the batch driver and generated output.

## Current Master Scripts

- `Core/Trellis/bootstrap_trellis2_pod.sh`
- `Core/Trellis/Invoke-RunPodHfLogin.ps1`
- `Core/Blender/blender_glb_qa.py`
- `Core/QuadRetro/t66_quad_retro_character_pipeline.py`
- `Core/QuadRetro/RunQuadRetroCharacterPipeline.ps1`
- `Core/Environment/split_theme_module_sheet.py`

## Retired Scripts

Legacy TypeA, Mike, Arthur, and dungeon-kit prototype scripts were deleted after their durable lessons were captured in `Instructions/06_RUN_HISTORY_AND_KNOWN_ISSUES.md`.

## Future Consolidation

The remaining batch drivers should collapse into manifest-driven runners:

- one TRELLIS manifest runner
- one Quad Retro manifest runner
- one Unreal-ready static mesh exporter

Until then, keep batch folders named after the batch they serve and delete them when the batch is no longer active.
