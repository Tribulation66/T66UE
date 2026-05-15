# Model Generation Agents

## Owns

TRELLIS, RunPod model generation, source-image rules, Quad Retro, Blender QA, rigging/retopo policy, Rigging and Animation source workflows, Unreal mesh import, generated model cleanup, and model-generation scripts.

## Trigger Words

Trellis, RunPod, model generation, GLB, source image, Quad Retro, Blender QA, retopo, rigging, animation, Rigodotify, Quaternius, import meshes, generated meshes, environment kit, dungeon kit assets.

## Read First

- `Model Generation/Instructions/README.md`
- `Model Generation/Instructions/00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`
- `Model Generation/Rigging and Animation/RIGGING_ANIMATION_AGENTS.md` for editable character rigs or animations.
- Then the specific numbered instruction file for the task.

## Hard Rules

- Do not commit live secrets or pod-local access material.
- Do not keep raw generation output as a runtime dependency.
- Do not add one-off scripts when a manifest can drive an existing reusable script.
- If the generated asset affects the playable build, follow Unreal import and standalone validation instructions.

## Verification

Report pod health, HTTP generation evidence, nonzero artifact sizes, Blender import/QA evidence, Unreal import validation, and staged standalone evidence when applicable.
