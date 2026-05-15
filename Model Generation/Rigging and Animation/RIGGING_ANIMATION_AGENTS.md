# Rigging And Animation Agents

## Owns

Blender source-of-truth character rigs, Rigify/Rigodotify setup, Quaternius animation-library references, humanoid retargeting, editable Blender actions, animation QA previews, and export manifests before Unreal import.

## Trigger Words

Rigging, animation, Rigify, Rigodotify, Quaternius, retarget, roll animation, jump animation, walk animation, idle animation, Blender action, armature, skeletal mesh export.

## Read First

- `README.md`
- `00_RIGGING_ANIMATION_ROUTING_INSTRUCTIONS.md`
- Then the specific numbered instruction file for the task.
- For playable imports, also read `../Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`.

## Hard Rules

- Blender `.blend` scenes are the source of truth for editable rigs and actions.
- Keep third-party packages in `External/`; do not commit extracted vendor files.
- Do not treat a recognizable pose as an accepted animation. Contact, timing, silhouette, foot slide, mesh deformation, and recovery must pass visual QA.
- Do not wire new animation assets into playable content until Blender QA and Unreal import validation are complete.
- Record tool failures and workarounds in `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md` as they are discovered.

## Verification

Report the exact Blender version, add-on status, source files used, exported files, Blender preview evidence, Unreal import evidence, DataTable/runtime wiring evidence, and staged standalone evidence when gameplay-visible content changes.
