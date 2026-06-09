# Rigging And Animation Routing

Use this folder for regular enemy/mob animation work and routing to the physics-first raw FriendSlop hero process. The legacy Animated ToonStyle hero bridge is archived historical context, not active routing.

## Decision Tree

1. For Blender/VAT tool setup, use `01_TOOL_SETUP_INSTRUCTIONS.md`.
2. For the general regular-enemy VAT process, use `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md`.
3. For Easy / Stage 1 enemies, also use `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md`.
4. For the current creative and technical animation style guide, use `06_MOB_ANIMATION_GUIDELINES.md`.
5. For physics-first raw FriendSlop heroes, use `../Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md` and `../../Gameplay/Physics/MASTER_PHYSICS.md`.
6. For known tool behavior, runtime caveats, and proven fixes, use `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`.
7. For Unreal import, DataTable reload, runtime wiring, or standalone validation, also use `../Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`.
8. For performance, profiling, diagnostics, or "hundreds of enemies" validation, also use `../../PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`.

## Scope Boundary

This folder does not own broad automated hero or humanoid companion rigging. Hero 1 Chad physics-first rigging is an approved exception routed through the model-generation instruction set and Gameplay Physics contract.

The archived Animated ToonStyle bridge may be read as historical import/pipeline evidence only after explicit task scope allows it. It is not a renewed free-form humanoid rigging bakeoff and is not the active FriendSlop path.

The current Hero 1 Chad physics-first path is not the Animated ToonStyle bridge. It must not use Quaternius-derived `Roll` clips or old spike outputs as its foundation.

This folder does own mob animation source work. It does not make a mob production-ready by itself. A VAT mob is production-ready only after Blender visual QA, Unreal import, data/runtime wiring, crowd/performance validation, and staged standalone verification when playable content changes.
