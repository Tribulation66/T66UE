# World Agents

## Owns

Tower map design, lighting, world generation research boundaries, modular dungeon kit generation, and runtime integration of generated environment pieces.

## Trigger Words

World, tower, map, stage layout, lighting, HY-World, WorldMirror, modular dungeon kit, environment kit, wall mesh, floor mesh, ceiling mesh, generated kit.

## Read First

- `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md` for tower map/runtime design.
- `Gameplay/World/T66_LIGHTING_REFERENCE.md` for lighting.
- `Gameplay/World/MODULAR_DUNGEON_KIT_INSTRUCTIONS.md` for generated environment modules.
- `Gameplay/World/HY_WORLD_RESEARCH_REFERENCE.md` for HY-World research boundaries.
- `Model Generation/Instructions/00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md` and specific model-generation instructions when generating meshes.

## Hard Rules

- Do not treat HY-World as a production replacement for the current runtime terrain.
- Do not use full-room generation as the default modular-kit path.
- Keep gameplay collision authored through simple runtime proxies rather than deriving collision from high-poly generated visuals.
