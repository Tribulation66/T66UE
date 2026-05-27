# Rigging And Animation Routing

Use this folder for regular enemy/mob animation work and the narrow animated ToonStyle hero import bridge.

## Decision Tree

1. For Blender/VAT tool setup, use `01_TOOL_SETUP_INSTRUCTIONS.md`.
2. For the general regular-enemy VAT process, use `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md`.
3. For Easy / Stage 1 enemies, also use `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md`.
4. For the current creative and technical animation style guide, use `06_MOB_ANIMATION_GUIDELINES.md`.
5. For animated ToonStyle demo heroes or companions, use `07_ANIMATED_TOONSTYLE_HERO_PIPELINE_INSTRUCTIONS.md`.
6. For known tool behavior, runtime caveats, and proven fixes, use `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`.
7. For Unreal import, DataTable reload, runtime wiring, or standalone validation, also use `../Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`.
8. For performance, profiling, diagnostics, or "hundreds of enemies" validation, also use `../../PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`.

## Scope Boundary

This folder does not own broad automated hero or humanoid companion rigging. The user is handling manual humanoid rigging with external tools.

The exception is the Animated ToonStyle bridge for approved demo hero and companion visual rows. That bridge is import/pipeline infrastructure, not a renewed free-form humanoid rigging bakeoff.

This folder does own mob animation source work. It does not make a mob production-ready by itself. A VAT mob is production-ready only after Blender visual QA, Unreal import, data/runtime wiring, crowd/performance validation, and staged standalone verification when playable content changes.
