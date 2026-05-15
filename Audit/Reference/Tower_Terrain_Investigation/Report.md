# T66 Tower Terrain Investigation - Geometry Assembly & Proportions

Read-only audit. Scope was inspection only; no terrain code, material assets, mesh assets, or gameplay data were changed.

Supporting read-only probes generated during the audit:

- `Saved/TowerTerrainInvestigation/terrain_asset_probe.json` records current Unreal static-mesh bounds, material parents, texture parameter bindings, and hero mesh bounds.
- `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json` records source FBX dimensions and UV spans from `SourceAssets/Archive/Import/WorldKit/CoherentThemeKit01`.

## 1. Terrain assembly mechanism

The tower terrain is spawned at runtime by `AT66GameMode::SpawnMainMapTerrain()`. The game mode clears stale main-map terrain, builds a tower layout, then calls `T66TowerMapTerrain::Spawn()` with the selected difficulty/theme context (`Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:752`, `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:790`, `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:839`, `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:861`, `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:873`).

The current active path is the generated dungeon kit path. `T66.Tower.UseGeneratedDungeonKit` defaults on, generated floor visual tile size defaults to `T66TowerDungeonKitUnitSize * 2.0f`, and generated wall visual segment length defaults to the same value (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:73`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:79`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:85`). The configured values are clamped between one and six kit units (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:166`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:172`).

The generated kit separates visuals from collision. Visible floor, ceiling, and wall modules are instanced static meshes, while collision is built from hidden box proxies (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:257`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:285`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:466`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:787`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4103`). Visible wall/floor/ceiling instance transforms are grounded to the actor Z by subtracting the mesh bounds bottom (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:408`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:420`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:430`).

The layout baseline is:

| Item | Current value | Evidence |
|---|---:|---|
| Kit unit / grid cell size | `1300 UU` | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:39`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:48`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5070` |
| Generated wall target height | `1200 UU` | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:41`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5771` |
| Generated floor collision thickness | `24 UU` | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:42`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5057` |
| Generated floor spacing | `1224 UU` | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5057`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5058` |
| Wall collision depth | `120 UU` | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:40`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5065` |
| Shell radius | `20000 UU` | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5066` |
| Grid dimensions | `25 x 25` | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:46`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:47`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5068`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5069` |
| Door width | `1300 UU` | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:49`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5071` |

Gameplay floors are generated as a 25x25 grid of 1300 UU cells. Each grid cell receives exact world bounds, and row-contiguous walkable cells are merged into `WalkableFloorBoxes` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:3203`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3208`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3214`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3325`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3341`). Walls are emitted around solid neighbors and doorway edges (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:3383`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3409`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3411`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3416`).

Doorway wall geometry is side-segment based. A doorway edge emits two side wall boxes and records a header box (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:3101`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3140`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3142`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3159`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3161`), but the generated-kit path explicitly skips spawning doorway header cubes (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:4824`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4827`).

Theme selection is data-driven in code. All five themes resolve into the same generated-kit root, set `WallFamily=SplitCollisionVisual`, and load four wall meshes plus four floor meshes per theme (`Source/T66/Gameplay/T66TowerThemeVisuals.cpp:14`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:170`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:177`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:227`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:238`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:246`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:257`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:266`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:277`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:285`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:296`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:304`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:315`).

## 2. Piece inventory and dimensions

Generated-kit source FBXs are normalized to one UV repeat per module: source floor modules are `6 x 6 x 0.24` Blender units with `~1.0 x ~1.0` UV span, and source wall modules are `1.2 x 6 x 6` Blender units with `~1.0 x ~1.0` UV span (`Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:5`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:55`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:173`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:223`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:341`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:391`).

Unreal imported bounds vary because the meshes are stylized and include protruding geometry. Values below are current Unreal bounds in UU.

| Piece type | Mesh asset path(s) | X | Y | Z | Expected cell coverage | Actual runtime coverage | Gap/seam source |
|---|---|---:|---:|---:|---|---|---|
| Dungeon wall visuals | `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonWall_*_UnrealReady` | `146.8-161.1` | `610.7-647.0` | `613.0-654.1` | Collision wall depth `120`, height `1200`, segment span exact | Runtime scales local Y to segment length only; X/Z remain native | Visual top stops `545.9-587.0 UU` below ceiling; side joins are asset-edge joins |
| Forest wall visuals | `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/ForestWall_*_UnrealReady` | `125.8-176.4` | `603.2-612.9` | `610.1-653.7` | Collision wall depth `120`, height `1200`, segment span exact | Runtime scales local Y to segment length only; X/Z remain native | Visual top stops `546.3-589.9 UU` below ceiling; side joins are asset-edge joins |
| Ocean wall visuals | `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/OceanWall_*_UnrealReady` | `131.4-153.4` | `617.0-739.9` | `620.2-839.9` | Collision wall depth `120`, height `1200`, segment span exact | Runtime scales local Y to segment length only; X/Z remain native | Visual top stops `360.1-579.8 UU` below ceiling; side joins are asset-edge joins |
| Martian wall visuals | `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/MartianWall_*_UnrealReady` | `143.5-180.7` | `608.4-658.8` | `621.7-648.2` | Collision wall depth `120`, height `1200`, segment span exact | Runtime scales local Y to segment length only; X/Z remain native | Visual top stops `551.8-578.3 UU` below ceiling; side joins are asset-edge joins |
| Hell wall visuals | `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/HellWall_*_UnrealReady` | `134.8-171.1` | `603.8-633.7` | `618.2-727.8` | Collision wall depth `120`, height `1200`, segment span exact | Runtime scales local Y to segment length only; X/Z remain native | Visual top stops `472.2-581.8 UU` below ceiling; side joins are asset-edge joins |
| Dungeon floor visuals | `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonFloor_*_UnrealReady` | `600.8-614.4` | `601.5-621.0` | `31.2-45.0` | Source boxes tiled to exact coverage; default target tile `2600`, minimum one cell `1300` | Runtime scales X/Y to tile size; Z remains native | Tile-edge dark art/mesh borders can repeat at every visual tile or row-run boundary |
| Forest floor visuals | `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/ForestFloor_*_UnrealReady` | `600.5-606.5` | `600.3-608.3` | `25.7-34.4` | Source boxes tiled to exact coverage; default target tile `2600`, minimum one cell `1300` | Runtime scales X/Y to tile size; Z remains native | Tile-edge dark art/mesh borders can repeat at every visual tile or row-run boundary |
| Ocean floor visuals | `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/OceanFloor_*_UnrealReady` | `603.8-609.5` | `600.5-608.2` | `26.0-34.5` | Source boxes tiled to exact coverage; default target tile `2600`, minimum one cell `1300` | Runtime scales X/Y to tile size; Z remains native | Tile-edge dark art/mesh borders can repeat at every visual tile or row-run boundary |
| Martian floor visuals | `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/MartianFloor_*_UnrealReady` | `607.2-610.1` | `603.0-620.8` | `27.2-48.3` | Source boxes tiled to exact coverage; default target tile `2600`, minimum one cell `1300` | Runtime scales X/Y to tile size; Z remains native | Tile-edge dark art/mesh borders can repeat at every visual tile or row-run boundary |
| Hell floor visuals | `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/HellFloor_*_UnrealReady` | `606.0-608.0` | `604.3-610.0` | `33.1-40.4` | Source boxes tiled to exact coverage; default target tile `2600`, minimum one cell `1300` | Runtime scales X/Y to tile size; Z remains native | Tile-edge dark art/mesh borders can repeat at every visual tile or row-run boundary |
| Ceiling visuals | Same floor meshes as current theme | Same as floor module | Same as floor module | Same as floor module | Ceiling bottom is `Floor.SurfaceZ + 1200` | Runtime places grounded floor module at ceiling bottom | Bottom sits at 1200; it does not descend to meet current wall visuals |
| Wall collision | Hidden `UBoxComponent` proxy | Box-driven | Box-driven | `1200` | Match wall boxes exactly | Continuous proxy centered at `BaseZ + DesiredHeight * 0.5` | Collision does not explain visual gap |
| Floor collision | Hidden `UBoxComponent` proxy | Source box width | Source box depth | `24` | Match walkable boxes exactly | Top is `Floor.SurfaceZ` | Collision does not explain visual floor seams |
| Doorway sides | Same wall visual modules | Same as wall module | Runtime segment driven | Same as wall module | Doorway side gaps from `GridDoorWidth=1300` | Side wall boxes only in generated path | No generated doorway header is spawned |

Representative Unreal bounds evidence: Dungeon wall/floor blocks are enumerated in `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:93`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:369`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:461`, and `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:737`; Forest in `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:829`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:1473`; Ocean in `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:1565`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:2209`; Martian in `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:2301`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:2945`; Hell in `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:3037`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:3681`.

## 3. Gap diagnosis

### Wall-to-wall horizontal seams

Root cause: not a placement-space arithmetic gap. The code divides each wall run into `SegmentCount`, derives `WallUnitLength = SpanLength / SegmentCount`, places each segment at `SegmentStart + WallUnitLength * (SegmentIndex + 0.5)`, and scales local Y by `WallUnitLength / meshYSize` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:704`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:710`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:734`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:739`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:741`). This makes adjacent segment bounds mathematically contiguous along the wall run.

The visible wall-to-wall lines are asset/material join artifacts between butt-joined, randomized visual wall modules. A different wall mesh can be chosen for each segment by `Seed + SideIndex * 977 + SegmentIndex * 37` modulo the theme mesh count (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:619`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:630`). The source wall FBXs are normalized modules with one UV span per module (`Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:341`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:391`), and the current runtime has no overlap, trim cap, or seam-hiding piece at the join; it simply butts the scaled module bounds together (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:753`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:758`).

Conclusion: this is primarily an asset-edge/material-boundary issue at visual wall segment joins, amplified by per-segment module variation. The collision wall behind it is continuous and does not create the visual seam (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:783`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:787`).

### Wall-to-ceiling vertical gap

Root cause: the generated wall visual meshes are too short for the generated wall/ceiling height, and wall visuals are not scaled in Z.

The generated path sets `ModuleWallHeight` to `T66TowerGeneratedDungeonKitWallHeight`, which is `1200 UU` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:41`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5771`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5772`). Ceiling/underside visuals are placed at `Floor.SurfaceZ + ModuleWallHeight`, so the ceiling bottom is at `SurfaceZ + 1200` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:5790`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5795`). The underside code then uses `CeilingVisualZ = CeilingBottomZ` and grounds each floor mesh at that Z (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:4207`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4265`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4267`).

Wall visuals, however, are placed at `BaseZ` and scaled as `(1.0, WallUnitLength / meshY, 1.0)`, leaving native wall mesh height unchanged (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:738`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:739`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:742`). The hidden collision proxy does use `DesiredHeight * 0.5` and therefore reaches 1200 UU, so collision and visuals diverge (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:783`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:785`).

Measured current wall visual heights:

| Theme | Wall visual Z range | Ceiling bottom | Resulting visual gap |
|---|---:|---:|---:|
| Dungeon | `613.0-654.1 UU` | `1200 UU` | `545.9-587.0 UU` |
| Forest | `610.1-653.7 UU` | `1200 UU` | `546.3-589.9 UU` |
| Ocean | `620.2-839.9 UU` | `1200 UU` | `360.1-579.8 UU` |
| Martian | `621.7-648.2 UU` | `1200 UU` | `551.8-578.3 UU` |
| Hell | `618.2-727.8 UU` | `1200 UU` | `472.2-581.8 UU` |

Conclusion: this is a certain mechanism-level mismatch. The runtime expects a 1200 UU wall/ceiling span, but generated wall visuals are currently native-height modules roughly 610-840 UU tall and receive no Z scale.

### Floor-to-floor seams

Root cause: visual tile boundaries, not a placement gap in the floor boxes.

The gameplay floor builder merges contiguous walkable cells only within each grid row into `WalkableFloorBoxes` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:3325`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3339`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3341`). Each walkable source box is then subdivided into generated floor visual tiles. For each source box, tile count is `ceil(BoxSize / PlannedTileSize)`, and each tile size is exactly `BoxSize / Count`, so the tiles exactly cover the box with no intended gap (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:4031`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4037`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4044`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4069`).

The visible floor lines therefore come from butt-joined visual modules: every generated tile is an independent floor mesh, scaled in X/Y but not Z (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:4071`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4073`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4075`). Source floor modules carry one full UV span each (`Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:173`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:223`), so any dark border/detail near the texture or mesh edge repeats at every tile boundary.

Floor collision is continuous per walkable source box and reaches the floor surface: the hidden collision slab is centered at `Floor.SurfaceZ - 12` with half height `12`, so its top is `Floor.SurfaceZ` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:4018`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4103`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4105`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4107`). Collision does not explain the dark visual lines.

One secondary proportions issue can contribute: floor visuals are placed with bottom at `Floor.SurfaceZ - 24` but native floor mesh heights are `25.7-48.3 UU`, so visual tops can protrude `1.7-24.3 UU` above the collision surface depending on module (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:4071`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4075`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:461`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:3681`).

## 4. Floor texture density

The runtime environment material is not applying an additional UV tiling multiplier. Coherent kit materials are material instances parented to `/Game/Materials/M_Environment_Unlit`, and the import script assigns only `DiffuseColorMap` / `BaseColorTexture`, `Tint=white`, and `Brightness=1.0` (`Scripts/RunImportCoherentThemeKit01AndExit.py:23`, `Scripts/RunImportCoherentThemeKit01AndExit.py:99`, `Scripts/RunImportCoherentThemeKit01AndExit.py:107`, `Scripts/RunImportCoherentThemeKit01AndExit.py:111`). The current material probe found no active `TextureScale`, `UVScale`, `Tiling`, or `TileScale` override on a generated floor material; those values are zero/unset while the texture parameters point at the generated-kit base color asset (`Saved/TowerTerrainInvestigation/terrain_asset_probe.json:410`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:414`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:416`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:436`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:448`).

Texture filtering was already unified in Iteration 02: the report records 43 targeted terrain floor/wall base-color textures changed from `TF_DEFAULT` to `TF_NEAREST`, and specifically cites the ground/mob mismatch as a contained texture-filtering mismatch that was fixed (`Audit/Reference/Visual_Lock/Iteration_02_Report.md:39`, `Audit/Reference/Visual_Lock/Iteration_02_Report.md:61`, `Audit/Reference/Visual_Lock/Iteration_02_Report.md:70`, `Audit/Reference/Visual_Lock/Iteration_02_Report.md:71`).

Current texture-repeat control is therefore mesh UVs plus runtime tile scale:

| Measurement | Current value | Evidence |
|---|---:|---|
| Source floor mesh UV span | about `1.0 x 1.0` per module | `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:173`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:223`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:453`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:503` |
| Native imported floor footprint | about `600 x 600 UU` | `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:461`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:737`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:1197`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:1473` |
| Runtime tile target | default `2600 UU`, clamped to source-box size | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:79`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:166`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4017`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4031` |
| Single-cell floor tile | `1300 UU` when a source box is one grid cell | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:39`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4031`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4037`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4044` |
| Larger row-run tile | up to about `2600 UU` before subdivision | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:79`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4031`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4044` |

Conclusion: the floor is not currently repeating too often in runtime UV space. One full floor texture repeat spans roughly one generated tile, usually `1300-2600 UU`, not one player width. If the floor reads as microscopic, the driver is the source texture's internal detail scale and high-frequency art inside a single repeat, plus nearest filtering/low-res rendering, not a material tiling multiplier.

## 5. Proportions snapshot

| Proportion | Current value | Evidence |
|---|---:|---|
| Player capsule radius | `34 UU` | `Source/T66/Gameplay/T66HeroBase.cpp:41`, `Source/T66/Gameplay/T66HeroBase.cpp:69`, `Source/T66/Gameplay/T66HeroBase.cpp:232` |
| Player capsule diameter | `68 UU` | `Source/T66/Gameplay/T66HeroBase.cpp:41`, `Source/T66/Gameplay/T66HeroBase.cpp:232` |
| Player capsule half-height | `100 UU` | `Source/T66/Gameplay/T66HeroBase.cpp:42`, `Source/T66/Gameplay/T66HeroBase.cpp:68`, `Source/T66/Gameplay/T66HeroBase.cpp:232` |
| Player capsule height | `200 UU` | `Source/T66/Gameplay/T66HeroBase.cpp:42`, `Source/T66/Gameplay/T66HeroBase.cpp:197`, `Source/T66/Gameplay/T66HeroBase.cpp:199` |
| Chad visual mesh bounds | `146.5 x 90.4 x 201.4 UU` | `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:3849`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:3863`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:3866` |
| Wall collision height | `1200 UU` | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:41`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5771`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5772` |
| Wall visual height | `610.1-839.9 UU` depending theme/module | `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:829`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:1565`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:3037` |
| Ceiling bottom relative to floor | `+1200 UU` | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5790`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5795` |
| Floor visual tile native size | about `600 x 600 UU` | `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:461`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:1473`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:2209`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:3681` |
| Floor visual tile runtime repeat size | usually `1300-2600 UU` per texture repeat | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4031`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4037`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4044` |
| Grid cell size | `1300 UU` | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:39`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5070` |
| Doorway width | `1300 UU`, `650 UU` half-width before wall-thickness clamp | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:49`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3101` |

Floor texture repeat ratio:

| Reference width | Repeats across one player width at 1300 UU tile | Repeats across one player width at 2600 UU tile | Target note |
|---|---:|---:|---|
| Capsule diameter `68 UU` | `0.052` repeats | `0.026` repeats | Target of `~1 repeat/player-width` would be much denser than current runtime UV tiling |
| Chad visual X width `146.5 UU` | `0.113` repeats | `0.056` repeats | Current repeat is about `8.9x-17.7x` larger than the visual player width |

The important baseline: one runtime floor texture repeat spans far more than one player width. A fix pass aimed at "one texture repeat per player-width" would be changing current world repeat size from about `1300-2600 UU` down toward `68-146 UU`, depending whether capsule or visual width is the chosen reference.

## 6. Theme consistency

Pablo's observation that the gaps appear across all five themes is consistent with the implementation. All five themes use distinct generated wall/floor assets, but they share the same generated-kit assembly mechanism, the same 1200 UU generated wall/ceiling span, and the same no-Z-scale visual wall placement (`Source/T66/Gameplay/T66TowerThemeVisuals.cpp:227`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:246`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:266`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:285`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:304`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5771`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5795`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:739`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:742`).

Source FBX dimensions are consistent across themes: floors are `6 x 6 x 0.24`, walls are `1.2 x 6 x 6`, and UV spans are approximately one repeat per module for all 40 source FBXs (`Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:5`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:223`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:453`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:901`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:1349`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:2189`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:2239`).

The wall/ceiling gap is mechanism-level with asset-level variation. The mechanism-level cause is that ceiling bottom is fixed at `SurfaceZ + 1200` while visual wall Z scale remains `1.0`; the asset-level variation is the actual native wall height range per theme (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:5795`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:739`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:742`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:829`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:1657`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:3037`).

The floor seams are also shared-mechanism plus asset-edge. All themes use the same exact tile coverage math and same one-UV-repeat source module pattern, but each theme's generated floor art can carry its own edge darkness or high-frequency details (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:4037`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4044`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:453`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:901`, `Saved/TowerTerrainInvestigation/coherent_kit_uv_probe.json:1797`).

## 7. Other proportions observations

The visual wall height is only about `3.1x-4.2x` Chad's current 201.4 UU mesh height, while collision/ceiling height is about `6x` the hero height (`Saved/TowerTerrainInvestigation/terrain_asset_probe.json:3849`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:3866`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:41`). This mismatch is likely why the wall feels short and the ceiling gap reads as a black band instead of a deliberate upper wall zone.

Generated doorway headers are currently absent. Doorway header boxes are recorded during grid wall emission, but generated-kit themes return before spawning header cubes (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:3122`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4824`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4827`). That can make door tops/openings read differently from the non-generated fallback path.

Floor visual height is not normalized to the `24 UU` collision slab. Since generated floor visuals keep native Z scale and are grounded at `SurfaceZ - 24`, taller visual modules can protrude above the collision surface (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:4018`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4071`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4075`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:2669`, `Saved/TowerTerrainInvestigation/terrain_asset_probe.json:2945`).

## 8. Pending issues created

None. The observed terrain problems are the direct subject of this audit and should be handled in the follow-up fix pass rather than filed as separate out-of-scope pending issues.
