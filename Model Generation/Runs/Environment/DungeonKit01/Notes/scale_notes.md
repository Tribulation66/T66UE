# DungeonKit01 Scale Notes

Initial generated meshes are ready for Blender review, not Unreal import acceptance.

Expected first wall convention:

- Unreal target length: `1800` cm
- Wall thickness fit target: current dungeon wall thickness, approximately `320` cm
- Pivot: bottom center of wall segment
- Local length axis: `Y`
- Local height axis: `Z`
- Visible face: local `+X`

Current Unreal-ready replacement-kit convention:

- Current generated tower layout sets placement cell size to `1300` cm and wall thickness to `280` cm.
- DungeonKit01 Unreal-ready exports use `1300` cm length, `1200` cm height, and `120` cm depth for wall / doorway modules.
- Floor export uses `1300 x 1300` cm footprint and `24` cm visual thickness.
- Wall / doorway origin is the back-bottom-center of the visual skin. Mesh bounds run from `X=0` to `X=120cm`, `Y=-650cm` to `Y=650cm`, and `Z=0` to `Z=1200cm`.
- Floor origin is the bottom center of the tile, with its top aligned to the original procedural floor surface at runtime.
- Generated wall and floor modules now replace old visible wall/floor cube geometry when `T66.Tower.UseGeneratedDungeonKit=1`; hidden `UBoxComponent` cube/slab proxies carry collision, and old visible cube geometry is fallback only if generated assets are unavailable.
- Wall visuals use a back-bottom-center pivot. Runtime placement must put that back pivot on the back side of the collision wall, not at the collision wall center.
- Generated wall collision must be derived per visual segment from the imported mesh bounds after runtime scale, pivot grounding, and actor rotation. Do not spawn a separate old full-wall box after segmenting the visual modules.
- The generated floor module also owns the ceiling role. Runtime spawns an inverted visual underside for the floor below, preserves carrier-floor drop holes, and only the top/start floor adds a matching invisible roof-cap collision proxy.

Current raw-normalized pass:

- `DungeonWall_Straight_A`: `76593` triangles, raw TRELLIS source normalized directly.
- `DungeonWall_Straight_Chains`: `78109` triangles, raw TRELLIS source normalized directly.
- `DungeonWall_Straight_BonesNiche`: `76641` triangles, raw TRELLIS source normalized directly.
- `DungeonWall_Doorway_Arch_Alpha`: `76497` triangles, raw TRELLIS source normalized directly.
- `DungeonFloor_BonesDrain_A`: `74540` triangles, raw TRELLIS source normalized directly.
- Ceiling remains excluded from runtime import for this pass.

RetopoFlow rule:

- Do not use Blender Decimate for accepted DungeonKit imports.
- RetopoFlow `4.1.5` is the only approved low-poly topology tool.
- For this static wall/floor pass, RetopoFlow was skipped because automated Decimate was the wrong tool and manual environment retopo is not required to validate the replacement-kit runtime path.

Notes before Unreal import:

- TRELLIS output is still normalized around roughly one Blender unit; final Unreal scale and pivot normalization is a separate acceptance step.
- The doorway arch alpha rerun is rotated `90` degrees in the Blender review scene so it presents face-on beside the other wall modules.
- The bones wall and ceiling have visible loose-fragment artifacts in the overview render and should be reviewed before accepting them as production kit pieces.
- `UnrealImport/*_UnrealReady.glb` files now contain raw-source baked scale and pivot normalization for the four walls and floor.
- `DungeonKit01_UnrealReady_RoomPreview.blend` reimports those normalized GLBs and arranges them as a one-tile room preview.
