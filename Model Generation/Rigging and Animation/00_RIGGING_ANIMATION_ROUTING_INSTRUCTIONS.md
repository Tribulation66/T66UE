# Rigging And Animation Routing

Use this folder when the task is about editable character rigs or animation clips for heroes, enemies, bosses, companions, or NPCs.

## Decision Tree

1. For tool installation, vendor setup, or local cache checks, use `01_TOOL_SETUP_INSTRUCTIONS.md`.
2. For a full humanoid hero or companion animation pass such as idle, walk, jump, roll, follow, alert, and attack, use `02_HERO_ANIMATION_PIPELINE_INSTRUCTIONS.md`.
3. For regular enemies, mobs, quadrupeds, flying silhouettes, blobs, swarms, or other non-humanoid runtime animation, use `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md`.
4. For Difficulty 1 / Easy mob animation work, also use `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md`.
5. For known tool behavior, current limitations, and package inventory, use `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`.
6. For Blender import/export inspection, use `Tools/inspect_animation_assets.py`.
7. For Unreal import, DataTable reload, runtime wiring, or standalone validation, also use `../Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`.
8. For monsters or bosses that are intentionally skeletal at runtime, document why they are exceptions before adapting the humanoid process.

## Scope Boundary

This folder owns rig and animation source work. It does not make an asset playable by itself. A character animation is playable only after Unreal import, data/runtime wiring, validation, and staged standalone verification when the change affects the playable build.
