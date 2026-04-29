# Shared Model And World Asset Pipeline

This file holds rules that apply to both character/object model generation and world/environment generation.

## Source Of Truth Split

- `Model Generation` owns TRELLIS setup, RunPod operation, Blender QA, RetopoFlow rules, run history, and raw mesh artifacts.
- `World Generation` owns environment-kit design, room/world assembly policy, generated terrain/module runtime integration, and worldgen research.
- `MASTER DOCS/T66_IMPORT_PIPELINE_GUIDELINES.md` owns Unreal import rules, including unlit material conversion.

## Generation Contract

Use image generation only to create clean module or object references. The image should isolate one target asset on a simple opaque background, avoid UI/text/characters unless the character is the target, and avoid full-room compositions when the output should be a modular mesh.

For TRELLIS runs:

- keep the source image with the run artifacts
- generate raw GLB output into the matching run folder
- render raw QA views before cleanup
- reject outputs with unwanted background platforms, unreadable silhouettes, detached fragments, or extreme scale ambiguity
- normalize scale and pivot in Blender before Unreal import
- re-import the exported candidate into Blender and render again before accepting it

## Retopology Rule

RetopoFlow `4.1.5` is the only approved tool for intentional low-poly topology work. Do not promote Blender Decimate modifier output as an accepted low-poly model.

Use RetopoFlow for characters, enemies, animation-sensitive meshes, and environment details where a human retopo pass can improve the visible chunky low-poly shape. For static wall, floor, and ceiling modules, RetopoFlow is optional: if the work would only reduce triangle count mechanically, keep the normalized raw TRELLIS mesh instead of using Decimate.

## Unreal Import Contract

Accepted static meshes should enter Unreal through the project import workflow, not through ad hoc editor imports.

Required checks:

- imported materials use approved unlit parents
- imported normals/tangents and UV precision are preserved where needed
- scale and pivot match the documented module/object contract
- a fresh in-editor or standalone check proves the asset renders from the gameplay camera
- collision is explicit and authored for gameplay, not assumed from generated visual mesh complexity

For generated dungeon environment modules, the current rule is visual mesh plus hidden simple collision proxy. Do not use textured visible static mesh actors as collision proxies.

## Handoff Rule

Every accepted run needs enough notes for another agent to continue:

- source image path
- raw TRELLIS output path
- Blender normalization or RetopoFlow artifact path
- exported Unreal-ready file path
- target Unreal content path
- scale/pivot notes
- known visual or collision limitations
