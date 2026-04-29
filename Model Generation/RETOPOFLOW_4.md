# RetopoFlow 4 Rule

RetopoFlow `4.1.5` is the only approved tool for intentional low-poly topology work in the TRELLIS model-generation pipeline.

Do not use Blender's Decimate modifier as the normal way to lower poly count for accepted assets. Decimate can still be used only for throwaway diagnostics, quick triangle-budget experiments, or rejected prototypes, and those outputs must not be promoted as accepted `Retopo` assets.

## When To Use It

Use RetopoFlow for:

- characters
- enemies
- meshes that may be rigged, posed, or animated
- hands, faces, shoulders, elbows, knees, cloth edges, and other deformation-critical regions
- environment assets where the low-poly shape itself matters, such as chunky chains, bones, arches, trim, and silhouettes

For static environment walls, floors, and ceilings, RetopoFlow is optional. Use it only when a human can meaningfully redraw the visible large forms. If the job is just "reduce triangle count" on a static wall or floor, skip RetopoFlow and keep the raw TRELLIS mesh normalized for Unreal instead of using Decimate.

## Required RetopoFlow Artifact Trail

Every real RetopoFlow pass must keep:

- the raw TRELLIS source GLB
- a working `.blend` with the high-poly source and RetopoFlow target mesh
- the exported retopo GLB or FBX
- a fresh Blender re-import verification render from the exported file
- notes describing the intended topology target and final triangle count

## Environment Kit Note

Dungeon walls, floors, and ceilings are allowed to ship from raw normalized TRELLIS output during the first modular-kit prototype. The important constraints are:

- no Decimate modifier output in accepted imports
- correct Unreal scale and pivot normalization
- generated meshes are the actual spawned geometry when the kit is enabled
- generated wall and floor meshes are visual-only; hidden `UBoxComponent` proxies carry collision
- old procedural wall/floor cube geometry is only a fallback when generated assets are unavailable
