# Model Generation Instructions

This is the canonical instruction set for T66 model generation. The old loose root files, handoff prompts, archived runs, scenes, and reference outputs have been collapsed into these current process docs.

Read in order:

1. [00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md](00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md)
2. [01_TRELLIS_RUNPOD_SETUP_INSTRUCTIONS.md](01_TRELLIS_RUNPOD_SETUP_INSTRUCTIONS.md)
3. [02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md](02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md)
   - For heroes, companions, and other manually rigged characters, continue with [10_HUMANOID_IMAGE_GENERATION_GUIDELINES.md](10_HUMANOID_IMAGE_GENERATION_GUIDELINES.md).
4. [04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md](04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md)
   - For editable character animation sets, use [../Rigging and Animation/README.md](../Rigging%20and%20Animation/README.md).
5. [05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md](05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md)
6. [06_RUN_HISTORY_AND_KNOWN_ISSUES_REFERENCE.md](06_RUN_HISTORY_AND_KNOWN_ISSUES_REFERENCE.md)
7. [11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md](11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md) for active FriendSlop raw Pixal3D assets that must preserve the generated GLB texture.
8. [13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md](13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md) for physics-first raw FriendSlop hero rigs and pose-target animation sets.
9. [12_MODEL_CLEANUP_AND_ORGANIZATION_INSTRUCTIONS.md](12_MODEL_CLEANUP_AND_ORGANIZATION_INSTRUCTIONS.md) after replacement/import passes, to remove unused runtime packages and non-durable generated runs through manifest-gated cleanup.

Candidate pipelines:

- [../Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md](../Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md)
- [07_PIXAL3D_RUNPOD_SETUP_INSTRUCTIONS.md](07_PIXAL3D_RUNPOD_SETUP_INSTRUCTIONS.md)
- [08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md](08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md)

Archived historical pipelines:

- `../../Archive/ToonStyle/` contains retired ToonStyle and Animated ToonStyle docs.
- `../../Archive/RetroFX/` contains retired QuadRetro / RetroFX 3D art-direction docs.

## Current FriendSlop Model Sources

Current FriendSlop raw Pixal3D model work starts from:

- `../Runs/Pixal3D/FriendSlopEasyBatch_20260604_1532`
- `../Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415`

Older AccuRig / Animated ToonStyle hero demo runs are archived provenance and must not be treated as current FriendSlop source assets. Use [11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md](11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md) for FriendSlop raw imports.

## Retention Rule

Keep reusable decisions here. Do not keep full generated runs, Blender scenes, logs, screenshots, local access files, or one-off handoff prompts after the task is complete.
