# Model Generation Agents

## Owns

TRELLIS, production-cleared Pixal3D model replacement, FriendSlop raw Pixal3D import, RunPod model generation, source-image rules, Blender QA, rigging/retopo policy, Rigging and Animation source workflows, Unreal mesh import, generated model cleanup, and model-generation scripts.

## Trigger Words

Trellis, Pixal3D, FriendSlop, RunPod, model generation, GLB, source image, Blender QA, retopo, rigging, animation, Rigodotify, Quaternius, import meshes, generated meshes, environment kit, dungeon kit assets.

## Read First

- `Model Generation/Instructions/README.md`
- `Model Generation/Instructions/00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`
- `ART_DIRECTION.md` for current FriendSlop canonical direction and archived art-direction boundaries.
- `Model Generation/Rigging and Animation/RIGGING_ANIMATION_AGENTS.md` for editable character rigs or animations.
- Then the specific numbered instruction file for the task.

## Hard Rules

- Do not commit live secrets or pod-local access material.
- Do not keep raw generation output as a runtime dependency.
- Do not add one-off scripts when a manifest can drive an existing reusable script.
- For active production Pixal3D replacements, use FriendSlop raw import docs: `Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md` for static meshes and `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md` for physics-first humanoid rigs. Do not route active work through archived ToonStyle or QuadRetro docs unless the user explicitly revives that historical path.
- If the generated asset affects the playable build, follow Unreal import and standalone validation instructions.

## Verification

Report pod health, HTTP generation evidence, nonzero artifact sizes, Blender import/QA evidence, Unreal import validation, and staged standalone evidence when applicable.
