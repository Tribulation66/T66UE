# Modular Dungeon Kit Process

This file is the authoritative `World Generation` process for generating modular dungeon and difficulty-themed environment pieces for `T66`.

Use `Model Generation` for the TRELLIS server, Blender QA scripts, RetopoFlow rule, and raw generation artifact storage. Use this file for world/room kit design, module contracts, and runtime integration.

Use this process when generating:

- wall meshes
- floor meshes
- ceiling / roof underside meshes
- trim, edge, arch, pillar, grate, chain, bone, rubble, and relief variants that attach to those surfaces

Do not use this process for full-room generation. The goal is a reusable modular kit that the runtime can assemble into many rooms.

## Goal

The current tower dungeon reads too flat because most room surfaces are procedural cuboids with materials applied to them. The new direction is:

1. generate detailed modular static meshes
2. keep procedural room layout stable
3. replace flat wall and floor cuboids with generated wall and floor meshes when the generated kit is available
4. build many room layouts from the same authored kit pieces

This is a better fit than whole-room generation because it preserves procedural variety. One good wall kit can support hundreds of room layouts.

## Runtime Context

Relevant current code:

- `Source/T66/Gameplay/T66TowerMapTerrain.cpp`
  - builds tower floors, shell walls, maze walls, floor caps, roof surfaces, and decoration props
- `Source/T66/Gameplay/T66TowerThemeVisuals.cpp`
  - resolves per-theme materials and wall visual families
  - current dungeon theme uses `EWallFamily::SplitCollisionVisual` with DungeonKit01 wall and floor meshes
  - several non-dungeon themes already use `EWallFamily::MeshCluster`
- `MASTER DOCS/T66_IMPORT_PIPELINE_GUIDELINES.md`
  - owns GLB import rules and unlit material conversion

First integration principle:

- keep the procedural room layout and spawn logic
- use generated DungeonKit01 wall and floor meshes as visual runtime geometry
- use hidden `UBoxComponent` cube/slab proxies for wall and floor collision
- use old visible cuboid wall/floor geometry only as fallback when the generated kit is unavailable or disabled

## Modular Kit Contract

### Shared Unit System

Use Unreal centimeters.

Current useful tower constants:

- tower placement cell: approximately `1800` units
- tower wall thickness: approximately `320` units
- tower floor thickness: approximately `320` units
- gameplay floor grid cells are larger layout cells, but surface art should be built from smaller repeatable modules

Primary module size targets:

| Module Type | Target Footprint | Notes |
|---|---:|---|
| floor tile | `1800 x 1800` | top surface must remain readable and low-profile |
| wall segment | `1800` length | depth should fit over current wall thickness |
| wall corner | `1800 x 1800` corner footprint | used where wall spans meet |
| doorway / arch | `1800` or `3600` width | visual only until gameplay collision is authored |
| ceiling tile | `1800 x 1800` | underside detail must not hide the player/camera |
| trim strip | `1800` length | chains, roots, bones, cracks, border pieces |

The first pass should not require perfect procedural replacement. It only needs clean snapping and predictable scale.

### Pivot And Orientation

Use these Blender conventions unless a later import test proves a better convention:

- floor module:
  - pivot at tile center
  - local `Z=0` is the walkable top plane
  - visible detail rises upward from `Z=0`
- wall module:
  - pivot at bottom center of the segment
  - length runs along local `Y`
  - height runs along local `Z`
  - visible face points toward local `+X`
  - back side sits near the collision wall plane
- ceiling module:
  - pivot at tile center
  - visible underside faces local `-Z`
  - avoid dangling detail unless it is authored as a separate hanging prop

Every exported GLB must include a `scale_notes.md` or run-note entry saying whether these conventions were preserved.

## Asset Families

### Dungeon Walls

Minimum first set:

- `DungeonWall_Straight_A`
- `DungeonWall_Straight_Cracked`
- `DungeonWall_Straight_Chains`
- `DungeonWall_Straight_BonesNiche`
- `DungeonWall_Corner_A`
- `DungeonWall_Doorway_Arch`
- `DungeonWall_Pillar_End`

Rules:

- the silhouette must stay rectangular enough to snap to procedural wall spans
- high-detail props such as chains should be attached to the wall mesh, not floating far forward
- no deep overhangs that catch the camera
- do not bake a full floor slab into a wall module

### Dungeon Floors

Minimum first set:

- `DungeonFloor_Stone_A`
- `DungeonFloor_Stone_Cracked`
- `DungeonFloor_Bones_A`
- `DungeonFloor_Drain_A`
- `DungeonFloor_RubbleEdge_A`
- `DungeonFloor_TrapClean_A`

Rules:

- floor detail must be low enough that hero movement feels smooth
- bones, cracks, and rubble are visual detail unless explicitly promoted to collision
- avoid tall props in the center of floor tiles; use separate props for large piles
- keep tile edges clean enough to repeat without visible gaps

### Dungeon Ceilings

Minimum first set:

- `DungeonCeiling_Stone_A`
- `DungeonCeiling_Cracked`
- `DungeonCeiling_Beam_A`
- `DungeonCeiling_ChainAnchor_A`
- `DungeonCeiling_Roots_A`

Rules:

- ceiling detail must not make camera collision noisy
- hanging chains should be separate optional props unless their bounds are tiny
- underside should read from gameplay camera angles, not only from a straight upward render

## Run Folder Contract

Use this folder shape:

```text
C:\UE\T66\Model Generation\Runs\Environment\DungeonKit01\
  Inputs\
    prompts\
    references\
    approved_seed_images\
  Raw\
    Trellis\
  Renders\
    raw\
    normalized\
    unreal_scale_preview\
  Retopo\
  UnrealImport\
  Notes\
    decision_log.md
    scale_notes.md
```

Do not mix wall, floor, and ceiling source files into the Arthur or enemy folders.

## Image Reference Rules

The image that feeds TRELLIS should describe one module, not a full room.

A coherent source sheet is allowed as an upstream imagegen artifact when the goal is a larger matched kit. In that case, the sheet itself is not a TRELLIS input. Split the sheet into individual wall or floor module crops first, then feed those crops to TRELLIS. Keep the original source sheet, crop script, manifest, and final crop PNGs in the run folder so every module can be traced back to the same art plate.

Good source image traits:

- isolated object on opaque green or white background
- straight-on orthographic-like view
- clear silhouette
- visible front/top/underside surface depending on module type
- no character, no hand, no UI, no text
- no dramatic perspective that warps the module dimensions
- enough thickness cues that TRELLIS understands it is a mesh, not a flat decal

Bad source image traits:

- full room compositions
- wall plus floor plus ceiling in one TRELLIS crop
- deep shadows hiding edges
- tiny repeated details with no primary shape
- floating chains or bones disconnected from the tile
- background rubble that can become a generated platform

### Coherent Source Sheet Variant

Use this variant when multiple modules need to read as the same world.

Rules:

- one source sheet per theme and surface type
- split walls and floors separately; do not mix them in one sheet
- use a 2x2 atlas when the target set is four modules
- wall sheet target: `3072 x 2048`, four `1536 x 1024` crops
- floor sheet target: `2048 x 2048`, four `1024 x 1024` crops
- keep all cells on the same flat chroma background
- keep camera, palette, material language, and lighting consistent across all cells
- design details as chunky low-poly geometry, not fine painted texture noise
- save source sheets under `Inputs/source_sheets`
- save split TRELLIS crops under `Inputs/approved_seed_images`

The active sheet-based batch plan is:

- [CoherentThemeKit01](C:/UE/T66/Model%20Generation/Runs/Environment/CoherentThemeKit01/Notes/batch_plan.md)

Current raw TRELLIS review target:

- [CoherentThemeKit01 Raw/Trellis](C:/UE/T66/Model%20Generation/Runs/Environment/CoherentThemeKit01/Raw/Trellis)
- 40 raw GLBs total: Dungeon/Easy, Forest/Medium, Ocean/Hard, Martian/VeryHard, and Hell/Impossible, each with four walls and four floors
- first gate is visual scale comparison in Blender beside the accepted Arthur model before normalization or Unreal import

## Prompt Templates

### Wall Segment

```text
Single modular dungeon wall segment, isolated on solid green background, straight-on orthographic front view, chunky carved stone blocks, rectangular slab silhouette, shallow relief depth, rusted chains attached to the stone, cracks and worn edges, game-ready low-poly stylized fantasy asset, no floor, no ceiling, no room, no character, no text.
```

### Floor Tile

```text
Single square modular dungeon floor tile, isolated on solid green background, top-down orthographic view, chunky stone slabs, low-profile bones embedded between cracks, worn edges, game-ready stylized fantasy asset, flat walkable top, no walls, no room, no character, no text.
```

### Ceiling Tile

```text
Single square modular dungeon ceiling underside tile, isolated on solid green background, straight underside view, heavy stone slabs, cracks, small chain anchors, shallow relief details, game-ready stylized fantasy asset, no floor, no walls, no room, no character, no text.
```

### Doorway / Arch

```text
Single modular dungeon doorway arch wall segment, isolated on solid green background, straight-on orthographic front view, rectangular module bounds, carved stone arch opening, worn blocks, metal studs, game-ready stylized fantasy asset, no full room, no floor, no character, no text.
```

## TRELLIS Run Rules

Use the existing TRELLIS.2 RunPod baseline in [MASTER_WORKFLOW.md](C:/UE/T66/Model%20Generation/MASTER_WORKFLOW.md), plus the shared world/model rules in [SHARED_ASSET_PIPELINE.md](C:/UE/T66/World%20Generation/SHARED_ASSET_PIPELINE.md).

Recommended first-pass settings:

- seed: `1337` for baseline comparison
- texture size: `2048`
- decimation: `80000` for raw environment modules
- `preprocess_image=True`
- opaque green or white backgrounds

For each serious module candidate:

1. save the source image under `Inputs/approved_seed_images`
2. generate raw GLB into `Raw/Trellis`
3. render raw front/top/oblique QA views
4. reject obvious background platforms immediately
5. normalize scale and pivot in Blender
6. use RetopoFlow `4.1.5` only if a human/manual retopo pass will materially improve the low-poly shape
7. if RetopoFlow is skipped, normalize the raw TRELLIS mesh directly; do not use Blender Decimate as a substitute
8. re-import the exported candidate in Blender and render again

Poly-reduction rule:

- RetopoFlow `4.1.5` is the only approved tool for accepted low-poly topology work.
- Do not use Blender's Decimate modifier for accepted wall, floor, ceiling, character, enemy, or prop imports.
- For static walls and floors, RetopoFlow can be skipped when the visible forms would not benefit from manual redraw. In that case the correct fallback is raw TRELLIS output plus scale/pivot normalization, not Decimate.

## Blender Normalization Rules

Every accepted module must be normalized before Unreal import.

TRELLIS can generate rectangular environment references as chunky blocks. Do not treat raw or manually retopo'd wall outputs as final modular walls. After the optional RetopoFlow pass, run the Unreal-ready normalization pass so the generated detail becomes predictable modular geometry.

Current DungeonKit01 normalization script:

- [normalize_dungeonkit_unreal_ready.py](C:/UE/T66/Model%20Generation/Scripts/normalize_dungeonkit_unreal_ready.py)

Current first runtime-test convention:

- wall / doorway length: `1300` cm
- wall / doorway visual depth: `120` cm
- wall / doorway visual height: `1200` cm
- floor footprint: `1300 x 1300` cm
- floor visual thickness: `24` cm
- wall / doorway pivot: back-bottom-center
- floor pivot: center, with visible detail above `Z=0`

This convention is now the runtime replacement size for generated dungeon-kit walls and floors. Old procedural cube geometry should exist only as a fallback when generated kit meshes are missing or disabled.

Required checks:

- pivot matches the module convention
- scale matches the target footprint
- back/bottom planes are clean enough to snap
- no unwanted floor platform
- no extreme triangle spikes or detached floating fragments
- material slots are understandable
- front/top/oblique renders show the intended detail

Triangle targets for first usable kit:

| Module Type | Target Triangles |
|---|---:|
| floor tile | `1k-6k` |
| wall segment | `2k-10k` |
| decorated wall segment | `5k-18k` |
| doorway / arch | `6k-20k` |
| ceiling tile | `1k-8k` |
| trim / prop overlay | `500-5k` |

These are not final budgets. They are first-pass targets so we can test the runtime idea quickly.

## Unreal Import Rules

Accepted GLBs should be staged here:

```text
C:\UE\T66\SourceAssets\Import\WorldKit\DungeonKit01\
```

Suggested Unreal content target:

```text
/Game/World/Terrain/TowerDungeon/GeneratedKit/DungeonKit01/
```

Import path:

1. add the GLBs to the static mesh import manifest
2. run `Scripts/ImportStaticMeshes.py` through the project import workflow
3. verify imported materials use approved unlit material parents
4. verify full-precision UVs and imported normals/tangents are preserved
5. run import verification where applicable

For the active generated-kit runtime test, generated wall and floor meshes are visual-only. Do not import DungeonKit01 static meshes as complex-as-simple collision; traversal collision must come from hidden `UBoxComponent` cube/slab proxies. Do not use a textured `AStaticMeshActor` as a collision proxy.

## Runtime Integration Plan

### Phase 1: Replacement Kit

Use generated wall and floor modules as the actual runtime geometry for DungeonKit01.

- spawn wall modules along existing shell and maze wall spans
- spawn floor tile modules over current walkable floor boxes
- enable wall and floor collision on simple hidden proxies, not generated meshes
- derive generated wall collision per visual segment from the imported mesh bounds after scale, pivot grounding, and rotation; do not reuse one old full-wall collision box after segmenting the visuals
- skip old procedural wall/floor cubes when generated modules spawn successfully
- use the generated floor module as the ceiling surface: the floor above supplies the underside for the floor below, and the top/start floor gets a generated floor-module roof cap
- do not spawn old visible roof/ceiling cubes for the generated dungeon kit

This proves visual quality and traversal together while retaining legacy cube fallback when generated assets are unavailable.

### Phase 2: Data-Driven Kit Registry

Create a data-authored registry before broad use.

Recommended row fields:

- `ModuleID`
- `SurfaceType` (`Wall`, `Floor`, `Ceiling`, `Trim`, `Doorway`)
- `MeshPath`
- `Theme`
- `VariantWeight`
- `FootprintUnits`
- `HeightUnits`
- `DepthUnits`
- `bCanRepeat`
- `bCanMirror`
- `bAllowsGameplayCollision`
- `CollisionProxyPath`
- `SocketTags`

Do not scatter hard-coded asset paths through the terrain generator once the prototype works.

### Phase 3: Procedural Replacement

After Phase 1 proves stable:

- replace some dungeon wall spans with modular wall meshes
- replace floor cap visuals with floor tiles
- add doorway / arch modules where path graph openings exist
- keep generated meshes visual-only and keep simple collision proxies authoritative
- add ceiling modules only where camera readability remains good

## Acceptance Gate

A generated module is accepted only if:

- it snaps to the target footprint without visible scale hacks
- it looks richer than the current flat material cuboid
- it does not include unwanted background floor/platform geometry
- it has clean preview renders from relevant angles
- it survives Blender re-import after export
- it can be imported through the existing Unreal GLB workflow
- it works as a repeated tile or span without obvious seams
- it does not break movement, camera, or combat readability

The kit is accepted only if:

- at least three wall variants, three floor variants, and two ceiling variants are imported
- a test room can be assembled from the modules
- collision remains stable
- the same procedural layout can choose different module variants
- a packaged Development standalone test still renders the modules correctly

## First Recommended Batch

The older first DungeonKit01 batch remains useful as the prototype reference:

1. `DungeonWall_Straight_A`
2. `DungeonFloor_Stone_A`
3. `DungeonWall_Straight_Chains`
4. `DungeonFloor_Bones_A`
5. `DungeonCeiling_Stone_A`
6. `DungeonWall_Corner_A`
7. `DungeonWall_Doorway_Arch`
8. `DungeonCeiling_ChainAnchor_A`

For the current CoherentThemeKit01 pass, do not generate more source sheets or TRELLIS outputs until the 40 raw wall/floor modules have been reviewed beside Arthur and obvious rejects have been recorded.
