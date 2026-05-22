# Rigging And Animation Routing

Use this folder for regular enemy and mob animation work where the intended runtime result is vertex animation texture playback.

## Decision Tree

1. For Blender/VAT tool setup, use `01_TOOL_SETUP_INSTRUCTIONS.md`.
2. For the general regular-enemy VAT process, use `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md`.
3. For Easy / Stage 1 enemies, also use `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md`.
4. For the current creative and technical animation style guide, use `06_MOB_ANIMATION_GUIDELINES.md`.
5. For known tool behavior, runtime caveats, and proven fixes, use `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`.
6. For Unreal import, DataTable reload, runtime wiring, or standalone validation, also use `../Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`.
7. For performance, profiling, diagnostics, or "hundreds of enemies" validation, also use `../../PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`.

## Scope Boundary

This folder no longer owns automated hero or humanoid companion rigging. The user is handling those manually with external humanoid-rigging tools.

This folder does own mob animation source work. It does not make a mob production-ready by itself. A VAT mob is production-ready only after Blender visual QA, Unreal import, data/runtime wiring, crowd/performance validation, and staged standalone verification when playable content changes.
