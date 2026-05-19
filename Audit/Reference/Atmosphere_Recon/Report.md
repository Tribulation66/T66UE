# T66 Atmosphere Pass 00 - Reconnaissance Report

Working goal: produce a read-only, line-cited reconnaissance report for the atmosphere implementation pass, with no code changes, no asset changes, no builds, and no staged executable runs.

Method: inspected repo source, required docs, pending issue files, and prior visual reports. Asset parent chains, material parameters, and map actor inventory were gathered by read-only Unreal Python commandlet probes into `Saved/Atmosphere_Recon/asset_inventory.json` and `Saved/Atmosphere_Recon/material_params.json`.

Synthesis contradiction flags:

- The live tower generation path is not 6x6 cells at 6500 UU. `FLayout` still has header defaults of `GridColumns=6`, `GridRows=6`, and `GridCellSize=6500.0f`, but `BuildLayout()` overwrites the runtime values to `25x25` cells at `1300.0f` (`Source/T66/Gameplay/T66TowerMapTerrain.h:146-148`; `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5057`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5072`).
- The live procedural maze path is primarily the newer room/graph/corridor builder, not only the older biased random-walk branch builder. `T66BuildFloorMazeWalls()` tries `T66BuildFloorDungeonLoop()` first, falls back to `T66BuildFloorMazeWalls_GridGraph()`, then to the legacy path (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:3542-3577`).
- `Floor.MazeWallBoxes` is not a rich wall registry. It is `TArray<FBox2D>` with no stored orientation, owner cell, wall direction, or outer-shell flag (`Source/T66/Gameplay/T66TowerMapTerrain.h:112-114`).
- Dungeon is not forced for every runtime difficulty. `BuildLayout()` initially sets floor theme to Dungeon, but `Spawn()` resolves `StageTheme` from difficulty and clones every floor with that stage theme before resolving visuals (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:5005`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5149`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5760-5768`).
- The Dungeon kit contains a wall module named `DungeonWall_TorchSconce_A`, but there is no standalone torch-placement or sconce-location registry. The current generator treats it as one of four wall variants (`Source/T66/Gameplay/T66TowerThemeVisuals.cpp:299-313`).

## Section A - Procedural Maze Generation

### A1. Floor Build Flow

Top-level runtime flow:

1. `AT66GameMode::SpawnLevelContentAfterLandscapeReady()` calls `SpawnMainMapTerrain()` during bootstrap, then schedules visual cleanup (`Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:236-280`).
2. `AT66GameMode::SpawnMainMapTerrain()` chooses tower layout when the preset layout variant is tower. It calls `T66TowerMapTerrain::BuildLayout()`, caches tower anchors, then calls `T66TowerMapTerrain::Spawn()` (`Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:753-870`).
3. `T66TowerMapTerrain::BuildLayout()` fills global layout constants, creates floors, computes preliminary holes and arrival/exit anchors, seeds each floor, and calls `T66BuildFloorMazeWalls()` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:5057-5199`).
4. `T66BuildFloorMazeWalls()` resets metadata, skips non-gameplay/start/boss floors, tries `T66BuildFloorDungeonLoop()`, falls back to `T66BuildFloorMazeWalls_GridGraph()`, then to legacy wall generation (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:3542-3577`).
5. `T66TowerMapTerrain::Spawn()` resolves the runtime stage theme, resolves floor visuals, then calls shell wall, floor tile, maze wall, prop, underside/ceiling, and roof surface helpers in order (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:5739-5833`).

Data flow:

```text
GameMode bootstrap
  -> SpawnMainMapTerrain()
  -> T66TowerMapTerrain::BuildLayout()
     -> layout constants and floor records
     -> preliminary entry/hole anchors
     -> per-floor seed via T66BuildTowerFloorSeed()
     -> T66BuildFloorMazeWalls()
        -> room set / graph / corridor carve
        -> grid cell metadata
        -> MazeWallBoxes / DoorwayHeaderBoxes / TrapEligibleWallBoxes
  -> T66TowerMapTerrain::Spawn()
     -> ResolveGameplayLevelThemeForDifficulty()
     -> T66TowerThemeVisuals::ResolveFloorTheme()
     -> shell walls
     -> floor visuals
     -> maze wall visuals
     -> props
     -> underside / ceiling / roof visuals
```

### A2. Cell Grid Parameters

Header defaults still read as `6x6` and `6500.0f`, but they are not the live tower floor values (`Source/T66/Gameplay/T66TowerMapTerrain.h:146-148`). `BuildLayout()` overwrites runtime floor generation with generated-kit constants: wall depth `120.0f`, wall height `1200.0f`, grid columns/rows `25`, grid cell size `1300.0f`, and door width `1300.0f` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:40-44`; `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5057-5072`).

Configurability: these values are global source constants/current layout assignments inside `T66TowerMapTerrain.cpp`, not per-theme data and not per-floor authored data (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:5057-5072`).

### A3. Maze Carving Algorithm

Current primary algorithm:

```text
T66BuildFloorDungeonLoop(Layout, Floor, Rng):
  Build grid cells
  Clamp/project arrival cell
  BuildDungeonRoomSet(arrival cell)
    target rooms = random 15..20
    create 3x3/4x4 start room around arrival
    attempt random rooms with margin 1, then margin 0 fallback
  BuildDungeonTileMap()
  BuildDungeonRoomGraph()
    complete room graph weighted by Manhattan distance + random 0..0.25
    MST-style connect all rooms
    add extra loop edges with budget clamp(roomCount/4, 2..6)
    accept extra edge when random <= 0.38 and room degree < 4
  For each graph edge:
    CarveDungeonCorridor()
      A* over tile grid
      base move cost 1.0
      existing corridor cost 0.35
      start/goal room cost 0.60
      other-room crossing cost 42.0
      turn cost +0.15
      random noise +0..0.025
      heuristic 0.85 * Manhattan
      fallback to L-path when A* fails
  Pick farthest exit room, set Floor.HoleCenter and ExitPoint
  Mark room/corridor/door semantics, connection masks, templates
  Emit edge walls into MazeWallBoxes / TrapEligibleWallBoxes / DoorwayHeaderBoxes
```

Evidence: room target and attempts are in `T66BuildDungeonRoomSet()` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:2691-2732`), graph and loop edges are in `T66BuildDungeonRoomGraph()` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:2734-2834`), corridor A* is in `T66CarveDungeonCorridor()` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:2945-3078`), and wall emission is in `T66EmitDungeonTileEdgeWall()` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:3080-3149`).

Fallback biased random walk / looping branch algorithm:

```text
Build main path:
  scored neighbors = unvisited cardinal neighbors
  score = 3 if Manhattan distance to goal decreases, else 1
  score += random 0..0.75
  sort descending
  recurse/backtrack until goal reached

Add loops:
  for each main path index:
    if random <= 0.35:
      grow optional branch up to random 1..3 cells
      randomize directions
      reconnect branch to occupied non-start cell
      mark OptionalLoop and add optional cells
```

Evidence: main path recursion is `T66TryBuildMainPathRecursive()` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:1552-1618`), main path segment assembly is `T66BuildMainPathIndices()` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:1657-1698`), loop branch recursion and add logic are `T66TryBuildLoopBranchRecursive()` / `T66TryAddLoopBranch()` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:1700-1846`), and the branch chance is `DefaultDungeonBranchChance=0.35f` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:52-55`; `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3510`).

Seed source: `T66BuildTowerFloorSeed()` hashes the preset seed, floor number, gameplay level number, and theme, and the per-floor seed is consumed by `FRandomStream FloorMazeRng` before maze building (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:1374-1390`; `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5184-5199`).

### A4. Wall Spawn Mechanics

Walls are spawned as runtime actors containing `UHierarchicalInstancedStaticMeshComponent` components, not as individual `AStaticMeshActor` wall segments. `T66SpawnGeneratedDungeonInstancedMeshActor()` spawns a plain `AActor`, creates a root `USceneComponent`, then creates HISM components per mesh (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:454-548`).

Wall visuals are batched by mesh key and transformed into generated-kit module instances. `T66SpawnGeneratedDungeonWallVisualsForSide()` splits a `FBox2D` wall run into module segments, chooses a wall mesh variant, and stacks generated wall modules to fill the 1200 UU wall height (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:643-722`; `Source/T66/Gameplay/T66TowerMapTerrain.cpp:853-869`). Shell walls and maze walls are separate helper paths: `T66SpawnShellWallsForFloor()` emits the outer shell, and `T66SpawnMazeWalls()` iterates `Floor.MazeWallBoxes` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:4594-4688`; `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4690-4752`).

Dungeon has multiple wall variants, not a single wall mesh. The Dungeon wall family is `DungeonWall_TorchSconce_A`, `DungeonWall_StoneBlocks_A`, `DungeonWall_Chains_A`, and `DungeonWall_BonesNiche_A`; floor variants are `DungeonFloor_StoneSlabs_A`, `DungeonFloor_Drain_A`, `DungeonFloor_Cracked_A`, and `DungeonFloor_Bones_A` (`Source/T66/Gameplay/T66TowerThemeVisuals.cpp:299-313`).

Current generated-kit wall dimensions: wall depth `120.0f`, wall height `1200.0f`, floor thickness `24.0f`, and runtime cell/module footprint `1300.0f` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:40-44`; `Source/T66/Gameplay/World/MODULAR_DUNGEON_KIT_INSTRUCTIONS.md:13-16`).

### A5. Floor.MazeWallBoxes

Current struct definitions:

- `FGridCell` stores `Coord`, `Bounds`, `WorldCenter`, `ConnectionMask`, `Semantic`, `Template`, arrival/exit flags, main path/loop IDs, `EmittedWallBoxes`, and cached spawn slots (`Source/T66/Gameplay/T66TowerMapTerrain.h:73-87`).
- `FFloor` stores floor identity, role/theme, center/surface/hole/arrival/exit data, grid cells, path arrays, walkable boxes, `MazeWallBoxes`, `DoorwayHeaderBoxes`, `TrapEligibleWallBoxes`, cached spawn arrays, and floor tag (`Source/T66/Gameplay/T66TowerMapTerrain.h:89-120`).
- `FLayout` stores global layout settings, including the header defaults that runtime overwrites (`Source/T66/Gameplay/T66TowerMapTerrain.h:136-148`).

`Floor.MazeWallBoxes` is `TArray<FBox2D>`. Each entry contains only the box extents supplied by `FBox2D`; it does not carry position/orientation fields beyond the box, owning cell, direction, inner/outer classification, wall length semantic, or source mesh variant (`Source/T66/Gameplay/T66TowerMapTerrain.h:112-114`).

Canonical use: it is the current visual/collision wall run registry for maze walls and is iterated by `T66SpawnMazeWalls()` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:4690-4752`). It is also used as a fallback source by wall-spawn sampling when `TrapEligibleWallBoxes` is empty (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:5511-5597`). It is not a complete semantic registry.

### A6. Wall Classification

The code locally distinguishes outer edges while emitting grid side walls, but it does not persist that classification in `MazeWallBoxes`. `T66EmitGridCellSideWalls()` computes `bOuterEdge`, then emits only `FBox2D` boxes (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:1931-2004`; `Source/T66/Gameplay/T66TowerMapTerrain.h:112-114`).

Outer shell walls are generated by a separate path, `T66SpawnShellWallsForFloor()`, and are not stored in `Floor.MazeWallBoxes` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:4594-4688`). The reinforcement/wall-spawn path named `TryGetWallSpawnLocation()` samples around floor bounds with shell inset values rather than iterating `MazeWallBoxes` (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:5664-5736`).

The map design reference still describes reinforcement enemies spawning from the outer shell and chests/crates on gameplay levels, matching the separate shell sampling design (`Gameplay/World/T66_MAP_DESIGN_REFERENCE.md:13-14`).

### A7. Cell Midpoint / Corner / Junction Data

The generator tracks cell bounds, centers, connection masks, and a coarse `ET66TowerGridTemplate` classification such as straight, corner, T-junction, and cross (`Source/T66/Gameplay/T66TowerMapTerrain.h:54-85`). `T66ResolveGridCellTemplate()` derives the template from semantic plus connection count (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:1895-1929`).

There is no persisted list of torch-ready wall midpoints, cell corners, corridor intersections, or sconce anchors. Torch placement can reuse `GridCells` plus `ConnectionMask` / `Template`, or recompute candidates from `MazeWallBoxes` / `TrapEligibleWallBoxes`.

### A8. Entry/Exit Anchors

Entry anchors are tied to the previous floor's actual hole center. `BuildLayout()` computes initial hole centers and, for each floor, uses the previous floor hole as the next floor arrival anchor; after maze building, `T66BuildFloorDungeonLoop()` can overwrite `Floor.HoleCenter` and `ExitPoint` based on the selected exit room (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:5141-5157`; `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5184-5199`; `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3266-3278`).

This confirms the memory note: the next floor arrival can be anchored to the previous floor's generated exit hole rather than only a preliminary static hole.

### A9. Theme Resolution

`T66TowerThemeVisuals` defines `/Game/Materials/M_Environment_Unlit.M_Environment_Unlit` as the environment fallback master and `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01` as the generated-kit root (`Source/T66/Gameplay/T66TowerThemeVisuals.cpp:13-14`).

Dungeon surface entry: the Dungeon case supplies no explicit floor or wall material path, supplies roof material `/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof.MI_TowerDungeonRoof`, and no texture paths; missing floor/wall materials fall back to environment unlit (`Source/T66/Gameplay/T66TowerThemeVisuals.cpp:61-66`; `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:320-330`).

Runtime theme caveat: `Spawn()` resolves `StageTheme = ResolveGameplayLevelThemeForDifficulty(Difficulty)` and applies that stage theme to every spawned floor. Difficulty gameplay-level themes map to Dungeon, Forest, Ocean, Martian, and Hell (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:5005`; `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5760-5768`).

## Section B - Material Inventory

### B1. M_Environment_Unlit Descendants

Probe result: `M_Environment_Unlit` is a Surface material using `MSM_UNLIT`, root path `/Game/Materials/M_Environment_Unlit.M_Environment_Unlit` (`Saved/Atmosphere_Recon/asset_inventory.json:258-271`). The full descendant list is in `Saved/Atmosphere_Recon/asset_inventory.json:52-136`.

Grouped by usage:

- NPC-authored asset materials: `/Game/Characters/NPCs/Gambler/GamblerDemonStand/Materials/M_GamblerDemonStand.M_GamblerDemonStand`, `/Game/Characters/NPCs/Gambler/QuadRetro/Materials/M_SM_Gambler_QuadRetro.M_SM_Gambler_QuadRetro`, `/Game/Characters/NPCs/Ouroboros/QuadRetro/Materials/M_SM_Ouroboros_QuadRetro.M_SM_Ouroboros_QuadRetro`, `/Game/Characters/NPCs/Saint/QuadRetro/Materials/M_SM_Saint_QuadRetro.M_SM_Saint_QuadRetro` (`Saved/Atmosphere_Recon/asset_inventory.json:52-56`).
- Terrain and cliff materials: `/Game/World/Cliffs/MI_HillTile1.MI_HillTile1`, `/Game/World/Cliffs/MI_HillTile2.MI_HillTile2`, `/Game/World/Cliffs/MI_HillTile3.MI_HillTile3`, `/Game/World/Cliffs/MI_HillTile4.MI_HillTile4`, `/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof.MI_TowerDungeonRoof`, `/Game/World/Terrain/TowerForest/MI_TowerForestGround.MI_TowerForestGround`, `/Game/World/Terrain/TowerForest/MI_TowerForestRoof.MI_TowerForestRoof` (`Saved/Atmosphere_Recon/asset_inventory.json:57-60`; `Saved/Atmosphere_Recon/asset_inventory.json:134-136`).
- Generated terrain kit materials: all Dungeon/Forest/Hell/Martian/Ocean generated wall/floor MIs under `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/`, including Dungeon floor/wall variants and the `DungeonWall_TorchSconce_A` material (`Saved/Atmosphere_Recon/asset_inventory.json:93-133`).
- Interactables and arcade materials: arcade module MIs, vehicle, arcade amplifier variants, legacy arcade machine, chest, crate, difficulty totem, fountain, idol altar, shroom, quick-revive vending (`Saved/Atmosphere_Recon/asset_inventory.json:61-89`).
- Loot bag materials: `/Game/World/LootBags/Black/Materials/M_SM_LootBag_Black_QuadRetro.M_SM_LootBag_Black_QuadRetro`, `/Game/World/LootBags/Red/Materials/M_SM_LootBag_Red_QuadRetro.M_SM_LootBag_Red_QuadRetro`, `/Game/World/LootBags/White/Materials/M_SM_LootBag_White_QuadRetro.M_SM_LootBag_White_QuadRetro`, `/Game/World/LootBags/Yellow/Materials/M_SM_LootBag_Yellow_QuadRetro.M_SM_LootBag_Yellow_QuadRetro` (`Saved/Atmosphere_Recon/asset_inventory.json:89-92`).

### B2. M_GLB_Unlit Descendants

Probe result: `M_GLB_Unlit` is a Surface material using `MSM_UNLIT`, root path `/Game/Materials/M_GLB_Unlit.M_GLB_Unlit` (`Saved/Atmosphere_Recon/asset_inventory.json:272-285`). Full descendant list is `Saved/Atmosphere_Recon/asset_inventory.json:137-160`.

Current descendants:

- Shared character path: `/Game/Materials/MI_GLB_Unlit_Character_Shared.MI_GLB_Unlit_Character_Shared` (`Saved/Atmosphere_Recon/asset_inventory.json:137`).
- Weapon projectile MIs: `/Game/Weapons/Projectiles/Materials/MI_SM_BaldChad_Hatchet.MI_SM_BaldChad_Hatchet`, `/Game/Weapons/Projectiles/Materials/MI_SM_BillyChad_Bullet.MI_SM_BillyChad_Bullet`, `/Game/Weapons/Projectiles/Materials/MI_SM_BoxerChad_Glove.MI_SM_BoxerChad_Glove`, `/Game/Weapons/Projectiles/Materials/MI_SM_CSChad_TacticalKnife.MI_SM_CSChad_TacticalKnife`, `/Game/Weapons/Projectiles/Materials/MI_SM_ChineseChad_Guandao.MI_SM_ChineseChad_Guandao`, `/Game/Weapons/Projectiles/Materials/MI_SM_FoundingChad_Rapier.MI_SM_FoundingChad_Rapier`, `/Game/Weapons/Projectiles/Materials/MI_SM_GoblinoChad_Cleaver.MI_SM_GoblinoChad_Cleaver`, `/Game/Weapons/Projectiles/Materials/MI_SM_MonotoneChad_InkShard.MI_SM_MonotoneChad_InkShard`, `/Game/Weapons/Projectiles/Materials/MI_SM_RabbitChad_Carrot.MI_SM_RabbitChad_Carrot`, `/Game/Weapons/Projectiles/Materials/MI_SM_RoachChad_RustyCrown.MI_SM_RoachChad_RustyCrown`, `/Game/Weapons/Projectiles/Materials/MI_SM_RoboChad_GearBlade.MI_SM_RoboChad_GearBlade`, `/Game/Weapons/Projectiles/Materials/MI_SM_RoyalChad_Sword.MI_SM_RoyalChad_Sword` (`Saved/Atmosphere_Recon/asset_inventory.json:138-149`).
- Legacy interactable/loot bag materials under GLB: altar, crate, fountain, shroom, totem, vending, and four loot bag `Material_0*` assets (`Saved/Atmosphere_Recon/asset_inventory.json:150-160`). These are not the active QuadRetro imported interactable MIs sampled for crates/chests/loot bags.

### B3. M_Character_Unlit Descendants

Probe result: `M_Character_Unlit` is a Surface material using `MSM_UNLIT`, root path `/Game/Materials/M_Character_Unlit.M_Character_Unlit` (`Saved/Atmosphere_Recon/asset_inventory.json:244-257`). Full descendant list is `Saved/Atmosphere_Recon/asset_inventory.json:3-51`.

Current descendants are companion idle/walk authored materials and hero authored MIs/materials. Heroes are mostly individually authored MIs/material assets, including `/Game/Characters/Heroes/Hero_1/Chad/QuadRetroUALQA/MI_Hero_1_Chad_QuadRetroUALQA_Unlit.MI_Hero_1_Chad_QuadRetroUALQA_Unlit`, plus many `Material_0` / `Material_0_001` beachgoer or Chad materials for heroes 1-12 (`Saved/Atmosphere_Recon/asset_inventory.json:3-51`).

Runtime hero visual assignment flows through `UT66CharacterVisualSubsystem::ApplyCharacterVisual()` to the hero skeletal/static/placeholder meshes rather than through a single shared hero MI in `AT66HeroBase` (`Source/T66/Gameplay/T66HeroBase.cpp:1031-1046`).

### B4. Master Parameter Surfaces

- `M_Environment_Unlit`: scalar `Brightness=1.0`; texture `DiffuseColorMap=/Engine/EngineResources/DefaultTexture.DefaultTexture`; vector `Tint=(1,1,1,1)` (`Saved/Atmosphere_Recon/material_params.json:26-55`).
- `M_GLB_Unlit`: scalar `Brightness=1.0`; texture `BaseColorTexture=/Engine/EngineResources/DefaultTexture.DefaultTexture`; vector `Tint=(1,1,1,1)` (`Saved/Atmosphere_Recon/material_params.json:60-88`).
- `M_Character_Unlit`: scalar `Brightness=1.0`; texture `DiffuseColorMap=/Engine/EngineResources/DefaultTexture.DefaultTexture`; no vector parameters in the probe (`Saved/Atmosphere_Recon/material_params.json:2-24`).

Runtime utility caveat: `FT66VisualUtil` writes `DiffuseColorMap`, `BaseColorTexture`, `Color`, `BaseColor`, and `Tint` when applying color/material fallbacks, even though the master surfaces do not all expose the same names (`Source/T66/Gameplay/T66VisualUtil.cpp:23-104`).

### B5. Interactable Materials - Exact Mapping

- `AT66CrateInteractable`: constructor hardcodes `/Game/World/Interactables/Crate/Crate_QuadRetro.Crate_QuadRetro`, then returns after `TryApplyImportedMesh()` succeeds. Runtime material comes from the mesh slot, sampled as `/Game/World/Interactables/Crate/Materials/M_Crate_QuadRetro.M_Crate_QuadRetro`, parent/root `M_Environment_Unlit`, brightness `1.0` (`Source/T66/Gameplay/T66CrateInteractable.cpp:12-21`; `Saved/Atmosphere_Recon/asset_inventory.json:10758-10784`).
- `AT66ChestInteractable`: constructor maps all rarities to `/Game/World/Interactables/Chests/ChestModel/Chest_QuadRetro.Chest_QuadRetro`, then returns after `TryApplyImportedMesh()` succeeds. Runtime material comes from the mesh slot, sampled as `/Game/World/Interactables/Chests/ChestModel/Materials/M_Chest_QuadRetro.M_Chest_QuadRetro`, parent/root `M_Environment_Unlit`, brightness `1.0` (`Source/T66/Gameplay/T66ChestInteractable.cpp:19-35`; `Saved/Atmosphere_Recon/asset_inventory.json:10726-10752`).
- `AT66LootBagPickup`: constructor hardcodes four rarity meshes under `/Game/World/LootBags/*/SM_LootBag_*_QuadRetro`; `UpdateVisualsFromRarity()` chooses the mesh and then calls `FT66VisualUtil::ApplyT66Color()` (`Source/T66/Gameplay/T66LootBagPickup.cpp:94-97`; `Source/T66/Gameplay/T66LootBagPickup.cpp:310-377`). Sampled mesh MIs are parent/root `M_Environment_Unlit`; brightness values are Black `1.1`, Red `1.65`, White `1.85`, Yellow `1.35` (`Saved/Atmosphere_Recon/asset_inventory.json:10797-10912`).
- `AT66ArcadeInteractableBase`: uses `GetArcadeData().DisplayMesh`, calls `TryApplyImportedMesh()`, and otherwise falls back to data tint. Data rows point to QuadRetro arcade meshes under `/Game/World/Interactables/Arcade/...` (`Source/T66/Gameplay/T66ArcadeInteractableBase.cpp:366-381`; `Content/Data/ArcadeInteractables.json:68`, `Content/Data/ArcadeInteractables.json:157`, `Content/Data/ArcadeInteractables.json:199`, `Content/Data/ArcadeInteractables.json:244`, `Content/Data/ArcadeInteractables.json:276`, `Content/Data/ArcadeInteractables.json:308`, `Content/Data/ArcadeInteractables.json:340`, `Content/Data/ArcadeInteractables.json:372`, `Content/Data/ArcadeInteractables.json:404`, `Content/Data/ArcadeInteractables.json:436`, `Content/Data/ArcadeInteractables.json:468`, `Content/Data/ArcadeInteractables.json:500`, `Content/Data/ArcadeInteractables.json:532`, `Content/Data/ArcadeInteractables.json:564`). All sampled arcade QuadRetro MIs descend from `M_Environment_Unlit` (`Saved/Atmosphere_Recon/asset_inventory.json:61-75`).
- `AT66ArcadeTruckInteractable`: uses `GetArcadeData().DisplayMesh`, calls `TryApplyImportedMesh()`, then calls `FT66VisualUtil::ApplyT66Color()` with the data tint. Vehicle data uses `/Game/World/Interactables/Arcade/Vehicle/Vehicle_QuadRetro.Vehicle_QuadRetro`, whose sampled material descends from `M_Environment_Unlit` (`Source/T66/Gameplay/T66ArcadeTruckInteractable.cpp:115-132`; `Content/Data/ArcadeInteractables.json:13`; `Saved/Atmosphere_Recon/asset_inventory.json:75`).
- `AT66WorldInteractableBase`: imported mesh path clears override materials and sets the mesh, so mesh-authored material slots remain active; fallback path uses `FT66VisualUtil::ApplyT66Color()` and `M_Environment_Unlit` (`Source/T66/Gameplay/T66WorldInteractableBase.cpp:176-179`; `Source/T66/Gameplay/T66WorldInteractableBase.cpp:255-290`; `Source/T66/Gameplay/T66VisualUtil.cpp:70-104`).

Conclusion: the active crate/chest/loot bag/arcade QuadRetro runtime assets currently parent directly or transitively to `M_Environment_Unlit`.

### B6. Lava and Miasma

`T66LavaShared` defines base material path `/Game/Materials/M_Environment_Unlit.M_Environment_Unlit` (`Source/T66/Gameplay/T66LavaShared.h:5-8`).

`AT66LavaPatch` creates a DMI from `T66LavaShared::BaseMaterialPath`, writes `DiffuseColorMap`, `BaseColorTexture`, `Tint`, `BaseColor`, and `Brightness`; runtime brightness is the patch `Brightness` value when frames exist, otherwise `1.0` (`Source/T66/Gameplay/T66LavaPatch.cpp:288-325`; `Source/T66/Gameplay/T66LavaPatch.cpp:459-472`). Legacy stage lava patch spawning is currently disabled with `T66EnableLegacyLavaPatches=false`; if re-enabled, per-patch brightness randomizes from `2.10` to `2.90` (`Source/T66/Gameplay/T66MiasmaManager.cpp:32`; `Source/T66/Gameplay/T66MiasmaManager.cpp:625-656`).

`AT66MiasmaBoundary` creates a boundary lava DMI from `M_Environment_Unlit`, sets `Tint=(1.00,0.34,0.05,1)`, `BaseColor` to the same tint, and `Brightness=2.6` (`Source/T66/Gameplay/T66MiasmaBoundary.cpp:59-82`; `Source/T66/Gameplay/T66MiasmaBoundary.cpp:209-211`).

`AT66MiasmaManager` also creates a lava/miasma material from `T66LavaShared::BaseMaterialPath`, writes texture parameters, and applies brightness; tower blood look forces brightness `1.45` (`Source/T66/Gameplay/T66MiasmaManager.cpp:826-867`; `Source/T66/Gameplay/T66MiasmaManager.cpp:1036-1049`).

Recommendation: keep lava/miasma on the environment path for the first lit-environment pass unless design wants them to behave as bright gameplay pickups. They are scenery/hazard surfaces whose visual contribution should react with the lit environment, but preserve their authored brightness boosts on the lit environment master. If gameplay readability demands bright hazard signaling, make a dedicated hazard-lit/unlit decision later rather than mixing them into the shared character MI.

### B7. Stock Default Lit Master

No project-local `M_Environment_Lit` was found in source/config/content references. The project retains `/Game/Materials/M_GLB_ViewSpaceLit_Character` as a parked Track 2 character master, but that pending issue explicitly says production mobs currently use `MI_GLB_Unlit_Character_Shared` and the Track 2 master still needs a visual-lock decision (`Content/Materials/pending_issues_Materials.md:3-8`).

Recommendation: author a new `/Game/Materials/M_Environment_Lit` Default Lit surface master that mirrors the active unlit parameter surface: `DiffuseColorMap`, `BaseColorTexture` alias support if practical, `Tint`, and `Brightness`. Existing environment/interactable MIs carry overrides for those names, and directly repointing to a stock engine material would not preserve the parameter contract (`Saved/Atmosphere_Recon/material_params.json:26-55`; `Source/T66/Gameplay/T66VisualUtil.cpp:52-68`).

### B8. Mob VAT Materials

`Content/Data/MobVertexAnimations.csv` maps production VAT mobs to `/Game/Characters/MobsVAT/*/MI_EasyMobVAT_*` material instances (`Content/Data/MobVertexAnimations.csv:2-11`). The asset probe shows these MIs parent/root to `/Game/Materials/M_EasyMobVAT_Unlit_UV2` with `Brightness=0.800000011920929` (`Saved/Atmosphere_Recon/asset_inventory.json:3073-3518`).

Runtime VAT material assignment sets the VAT material dynamically and applies `Brightness=T66_CharacterVisualBrightness`, currently `0.8`, in `UT66CharacterVisualSubsystem` (`Source/T66/Core/T66CharacterVisualSubsystem.cpp:940-975`). These VAT MIs are not descendants of `M_Environment_Unlit`, `M_GLB_Unlit`, or `M_Character_Unlit`; they are not affected by an environment-to-lit parent swap.

## Section C - Lighting & World Setup State

### C1. T66WorldVisualSetup.cpp Method Map

- `T66DestroyActorsOfType<T>()`: iterates actors of a type and destroys them (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:20-44`).
- `T66DestroyActorsWithTag()`: destroys actors matching a tag (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:46-72`).
- `T66DestroyActorsWithClassName()`: destroys actors by class name string (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:74-107`).
- `T66FindOrCreateUnboundPostProcessVolume()`: returns an existing unbound PP volume, converts the first existing PP volume to unbound, or spawns one at origin (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:109-147`).
- `T66ApplyNeutralPostProcess()`: marks volume unbound and applies neutral exposure/bloom/AO/saturation overrides (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:149-175`).
- `FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld()`: strips atmosphere/light/fog/legacy sky actors, then applies neutral PP (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:178-220`).
- `FT66WorldVisualSetup::FindOrCreateRuntimePostProcessVolume()`: public wrapper returning the unbound PP volume (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:222-225`).

Timing:

- Bootstrap schedules cleanup next tick, then again after `0.35s`, then again after `0.65s`; each pass calls `EnsureNeutralVisualSetupForWorld()` and `ApplyStageProgressionVisuals()` (`Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:317-348`).
- Main-map setup calls `EnsureNeutralVisualSetupForWorld()` in non-main-map setup and after standard setup flow (`Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:434-452`).
- Settings changes call `EnsureNeutralVisualSetupForWorld()` again (`Source/T66/Gameplay/T66GameMode.cpp:1468`).

### C2. Strip Pass Detail

Exact strip actor types:

```cpp
const int32 RemovedAtmospheres = T66DestroyActorsOfType<ASkyAtmosphere>(World);
const int32 RemovedDirectionalLights = T66DestroyActorsOfType<ADirectionalLight>(World);
const int32 RemovedSkyLights = T66DestroyActorsOfType<ASkyLight>(World);
const int32 RemovedFogActors = T66DestroyActorsOfType<AExponentialHeightFog>(World);
```

Source: `Source/T66/Gameplay/T66WorldVisualSetup.cpp:185-188`.

Additional legacy strips: tagged `T66_LegacyQuakeSky`, class name `T66QuakeSkyActor`, and class name `T66EclipseActor` (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:189-191`).

Cleanest seam for the atmosphere pass: add explicit allow/spare logic in `EnsureNeutralVisualSetupForWorld()` before the type-wide destruction. A type-level boolean is too blunt if future maps contain editor lights to strip; a stable tag/name allowlist for atmosphere-owned `ASkyLight` and `AExponentialHeightFog` instances is the smallest mechanism that preserves current strip behavior for all other actors while allowing the atmosphere pass to add owned actors.

### C3. Neutral PP Volume Creation

`T66FindOrCreateUnboundPostProcessVolume()` has no persistent parent actor; it either reuses/converts an existing `APostProcessVolume` or spawns a new `APostProcessVolume` in the world at zero transform (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:109-147`).

Neutral overrides:

- `bUnbound=true` (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:156`).
- `AutoExposureMinBrightness=1.0`, override true (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:163-164`).
- `AutoExposureMaxBrightness=1.0`, override true (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:165-166`).
- `AmbientOcclusionIntensity=0.0`, override true (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:167-168`).
- `BloomIntensity=0.0`, override true (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:169-170`).
- `BloomThreshold=10.0`, override true (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:171-172`).
- `ColorSaturation=(0.95,0.95,0.95,1.0)`, override true (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:173-174`).

No LUT, vignette, chromatic aberration, or film grain overrides are set by this method.

### C4. Retro FX PP Volume

`UT66RetroFXSubsystem` defines `RetroPostProcessPriority=5000.0f` and spawns a transient unbound `APostProcessVolume` labeled `DEV_RetroFX_PostProcessVolume` when Retro FX is applied (`Source/T66/Core/T66RetroFXSubsystem.cpp:55`; `Source/T66/Core/T66RetroFXSubsystem.cpp:795-823`). It resets its weighted blendables on reuse (`Source/T66/Core/T66RetroFXSubsystem.cpp:1809-1845`).

Coexistence: the neutral PP volume owns core `FPostProcessSettings` overrides at default priority; the Retro FX volume has priority 5000 and owns material blendables/weights. With all feature gates zero, blendable weights remain zero for PS1, fog, outline, N64, and chromatic features (`Source/T66/Core/T66RetroFXSubsystem.cpp:854-886`).

### C5. Other PP Volumes

Other source paths that can touch PP:

- `FT66StageProgressionVisuals::ApplyToWorld()` reuses the world visual setup PP volume and writes only `ColorSaturation` (`Source/T66/Gameplay/T66StageProgressionVisuals.cpp:11-20`).
- `UT66PixelationSubsystem` can reuse an existing unbound PP volume, reuse an existing PP volume, or spawn a PP volume only when pixelation levels are nonzero; zero levels return early (`Source/T66/Core/T66PixelationSubsystem.cpp:80-99`; `Source/T66/Core/T66PixelationSubsystem.cpp:120-188`).

Map probe: `GameplayLevel` has no placed post-process volume, fog, sky, or light actor. It contains three plain `Actor` entries labeled `T66 Procedural Foliage`, each with a `HierarchicalInstancedStaticMeshComponent` (`Saved/Atmosphere_Recon/asset_inventory.json:170-238`).

### C6. Existing Dynamic Light Spawning

Only one runtime light-creation pattern was found in source: `T66GameMode_BossFlow.cpp` creates a `UPointLightComponent` on a beacon actor, attaches it to the root, sets relative Z, intensity, color, attenuation radius, disables shadows, disables inverse-squared falloff, sets falloff exponent, and registers the component (`Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:1238-1261`).

No runtime `APointLight`, `USpotLightComponent`, `URectLightComponent`, `ASpotLight`, or `ARectLight` creation was found in `Source/T66` / `Source/T66Mini` beyond that pattern (`Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:1238-1249`).

### C7. T66HeroBase Structure

Hero root is the inherited `ACharacter` capsule. The constructor sets capsule radius `34.0f` and half-height `100.0f` for a desired 200 UU hero height (`Source/T66/Gameplay/T66HeroBase.cpp:41-43`; `Source/T66/Gameplay/T66HeroBase.cpp:67-69`). `CameraBoom`, placeholder mesh, static visual mesh, and range ring ISMs attach to `RootComponent` (`Source/T66/Gameplay/T66HeroBase.cpp:74-85`; `Source/T66/Gameplay/T66HeroBase.cpp:97-116`; `Source/T66/Gameplay/T66HeroBase.cpp:149-163`).

Natural carry-light attachment: attach a `UPointLightComponent` to `RootComponent` at a fixed vertical offset near upper body height. Mesh sockets are less reliable because the runtime can hide skeletal mesh, use placeholder mesh, or use static visual mesh (`Source/T66/Gameplay/T66HeroBase.cpp:1031-1046`). Height basis: capsule half-height is about 100 UU and camera boom relative Z is +60 UU (`Source/T66/Gameplay/T66HeroBase.cpp:67-77`; `Source/T66/Gameplay/T66HeroBase.cpp:226-232`).

### C8. T66StageProgressionVisuals Confirmation

`FT66StageProgressionVisuals::ApplyToWorld()` finds/creates the runtime PP volume, marks it unbound, and writes `PPS.ColorSaturation = Snapshot.ColorSaturation` (`Source/T66/Gameplay/T66StageProgressionVisuals.cpp:11-20`).

Config values:

- Stage1 `ColorSaturation=(0.95,0.95,0.95,1.0)` (`Config/DefaultT66StageProgression.ini:6`).
- Stage2 `ColorSaturation=(1.0,0.9,0.88,1.0)` (`Config/DefaultT66StageProgression.ini:7`).
- Stage3 `ColorSaturation=(1.05,0.86,0.82,1.0)` (`Config/DefaultT66StageProgression.ini:8`).
- Stage4 `ColorSaturation=(1.1,0.81,0.76,1.0)` (`Config/DefaultT66StageProgression.ini:9`).

The handoff explicitly says this should be removed/disabled in the atmosphere pass because it fights theme color grading (`C:/Users/DoPra/Desktop/T66_Atmosphere_Pass_Handoff.md:146`; `C:/Users/DoPra/Desktop/T66_Atmosphere_Pass_Handoff.md:259-260`).

### C9. Player Settings Fog Wiring

Fog settings exist in the save game: `bFogEnabled=true` and `FogIntensityPercent=55.0f` (`Source/T66/Core/T66PlayerSettingsSaveGame.h:214-218`).

The settings subsystem exposes a multicast delegate `FOnT66PlayerSettingsChanged OnSettingsChanged`, saves broadcast it, and fog setters call `Save()` (`Source/T66/Core/T66PlayerSettingsSubsystem.h:17`; `Source/T66/Core/T66PlayerSettingsSubsystem.h:252-279`; `Source/T66/Core/T66PlayerSettingsSubsystem.cpp:310-318`; `Source/T66/Core/T66PlayerSettingsSubsystem.cpp:928-955`).

Cleanest future hook: an atmosphere/fog actor or subsystem can subscribe to `OnSettingsChanged` and read `GetFogEnabled()` / `GetFogIntensityPercent()`; no new delegate is strictly required.

## Section D - Map Structure & Game Mode Wiring

### D1. GameplayLevel

Current `GameplayLevel` probe found exactly three actors, all plain `Actor` instances labeled `T66 Procedural Foliage`, each with a `HierarchicalInstancedStaticMeshComponent` named `FoliageHISM`. No light, sky, fog, PP, or terrain actor is placed in the map asset (`Saved/Atmosphere_Recon/asset_inventory.json:170-238`).

### D2. FrontendLevel

`FrontendLevel` is the default map and editor startup map (`Config/DefaultEngine.ini:16-17`). The game instance defines `/Game/Maps/FrontendLevel` and `/Game/Maps/GameplayLevel` as the frontend/gameplay map names (`Source/T66/Core/T66GameInstance.cpp:138-139`). `FrontendLevel` is a UI/bootstrap shell; it is not directly relevant to Dungeon atmosphere except that frontend game mode also calls neutral visual setup for its world.

### D3. T66TowerMapTerrain vs T66MainMapTerrain

`AT66GameMode::SpawnMainMapTerrain()` is the shared entry point. When the preset layout variant is tower, it calls `T66TowerMapTerrain::BuildLayout()` and `T66TowerMapTerrain::Spawn()` (`Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:753-870`). When the preset is not tower, it uses `T66MainMapTerrain::Generate()` and `T66MainMapTerrain::Spawn()` (`Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:882-961`).

Bootstrap calls `SpawnMainMapTerrain()` during `SpawnLevelContentAfterLandscapeReady()` before structures/interactables and visual cleanup (`Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:236-280`).

### D4. T66WorldVisualSetup Invocation

Call sites and timing:

- `AT66GameMode::ScheduleGameplayVisualCleanup()` next tick, then 0.35s delay, then 0.65s delay; each calls `EnsureNeutralVisualSetupForWorld()` and `ApplyStageProgressionVisuals()` (`Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:317-348`).
- Bootstrap invokes `ScheduleGameplayVisualCleanup()` after terrain and stage structures/interactables are spawned (`Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:257-280`).
- `T66GameMode_MainMap` calls `EnsureNeutralVisualSetupForWorld()` in non-main-map/main-map setup paths (`Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:434-452`).
- Settings change handling calls `EnsureNeutralVisualSetupForWorld()` again (`Source/T66/Gameplay/T66GameMode.cpp:1468`).

Atmosphere actors added before the strip must be allowlisted/spared or added after the final delayed strip. Since the strip also reruns on settings changes, durable atmosphere ownership should modify the strip behavior rather than only rely on spawn timing.

## Section E - Conventions & Pending Issues

### E1. AGENTS.md Relevant Excerpts

- Goal rule: derive the current working goal before acting and use it to choose inspection scope, changes, and verification (`AGENTS.md:15-22`).
- Folder instruction discovery: infer the owning folder, read folder `*_AGENTS.md`, and follow specific instructions (`AGENTS.md:24-31`).
- Pending issues protocol: out-of-scope encountered problems must be documented in a sibling `pending_issues_<foldername>.md` file using the required issue format (`AGENTS.md:52-63`).
- Verification evidence: report exact verification performed or why skipped (`AGENTS.md:41-49`).
- Script lifecycle: reusable scripts stay documented; task-specific scripts should be deleted after use (`AGENTS.md:11-13`).

Folder-specific instructions:

- `Gameplay/README.md` routes world work to `Gameplay/World` for tower map design, lighting, modular dungeon kit generation, and world-generation boundaries (`Gameplay/README.md:14`).
- `Gameplay/World/WORLD_AGENTS.md` says to read `T66_MAP_DESIGN_REFERENCE.md`, `T66_LIGHTING_REFERENCE.md`, and `MODULAR_DUNGEON_KIT_INSTRUCTIONS.md` for tower/lighting/generated environment work (`Gameplay/World/WORLD_AGENTS.md:9-16`).
- `Audit/README.md` classifies `Reference/` as technical background/inventories and historical drafts useful to read but not active fix queues; root audit drafts should be classified as Pending, Finished, or Reference (`Audit/README.md:7-32`).
- `Audit/AUDIT_AGENTS.md` says finished/reference files are evidence, not current instructions unless live repo checks confirm them (`Audit/AUDIT_AGENTS.md:14-15`).

### E2. Pending Issues Survey

- `Content/Audio/pending_issues_Audio.md`: missing audio SoundClass/theme packages logged by staged smoke; not atmosphere-adjacent except audio load noise (`Content/Audio/pending_issues_Audio.md:3-8`).
- `Content/Data/pending_issues_Data.md`: production mobs do not use status effects yet; not atmosphere-adjacent (`Content/Data/pending_issues_Data.md:3-8`).
- `Content/Materials/pending_issues_Materials.md`: parked `/Game/Materials/M_GLB_ViewSpaceLit_Character` master needs a visual-lock decision while production mobs use `MI_GLB_Unlit_Character_Shared`; atmosphere-adjacent material decision (`Content/Materials/pending_issues_Materials.md:3-8`).
- `Scripts/pending_issues_Scripts.md`: shared GLB import helpers still active and headless Interchange can crash after saving; relevant only if the implementation pass imports/authors material or mesh assets via scripts (`Scripts/pending_issues_Scripts.md:3-15`).
- `Source/T66/Core/pending_issues_Core.md`: legacy lab unlock IDs and skeletal hero rows ignoring `MeshRelativeScale`; not directly atmosphere-adjacent (`Source/T66/Core/pending_issues_Core.md:3-15`).
- `Source/T66/Data/pending_issues_Data.md`: Enemy Family/Role/Archetype redundancy; not atmosphere-adjacent (`Source/T66/Data/pending_issues_Data.md:3-8`).
- `Source/T66/Gameplay/pending_issues_Gameplay.md`: several gameplay items. Atmosphere-adjacent items are generated wall stack mid-height visual join, inter-walkable-box floor seams, and doorway header metadata lacking wall-run/source metadata; these directly affect Dungeon wall/floor visual polish and any wall-anchored torch placement metadata (`Source/T66/Gameplay/pending_issues_Gameplay.md:31-50`).
- `Source/T66/Gameplay/Enemies/pending_issues_Enemies.md`: missing production archetype classes; not atmosphere-adjacent (`Source/T66/Gameplay/Enemies/pending_issues_Enemies.md:3-8`).
- `Source/T66Mini/pending_issues_T66Mini.md`: T66Mini build script references a missing UI components dir; not atmosphere-adjacent (`Source/T66Mini/pending_issues_T66Mini.md:3-8`).

No new pending issue was added in this pass. The missing rich torch/wall metadata is a design gap for the upcoming atmosphere work, but the closely related doorway/wall-run metadata limitation is already tracked in `Source/T66/Gameplay/pending_issues_Gameplay.md:45-50`.

### E3. Atmosphere-Adjacent Prior Reports

- `Audit/Reference/Atmosphere_Current_State/Report.md`: current verified materials, lighting, fog, Retro FX, outline, and rendering-settings baseline for atmosphere work.
- `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md`: confirms Retro FX feature gates/default-zero state, runtime low-res ownership cleanup, and parked Track 2 master (`Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:42-51`; `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:173-179`).
- `Audit/Reference/Visual_Lock/Iteration_01_Report.md`: earlier visual lock baseline for Retro FX settings and outline defaults.
- `Audit/Reference/Visual_Lock/Iteration_02_Report.md`: final low-res visual baseline, renderer CVars, and mob brightness target `0.8` (`Audit/Reference/Visual_Lock/Iteration_02_Report.md:34-55`).
- `Audit/Reference/Visual_Systems_Audit/Report.md`: broader material/post-process/outline audit; some defaults are older than the cleanup/current-state reports, so use current source/probes for drift-prone values (`Audit/Reference/Visual_Systems_Audit/Report.md:27-29`; `Audit/Reference/Visual_Systems_Audit/Report.md:327`).
- `Audit/Reference/Gameplay_Visual_Cleanup_Investigation/Report.md`: inventory of active/legacy visual assets, including active `M_Environment_Unlit` paths and Track 2 material status (`Audit/Reference/Gameplay_Visual_Cleanup_Investigation/Report.md:105-113`).
- `Audit/Reference/Track2_Visibility/Track2_Report.md`: view-space lit character master history and parameter contract (`Audit/Reference/Track2_Visibility/Track2_Report.md:17-24`).
- `Audit/Reference/Visibility_Readability_Investigation.md` and `Audit/Reference/Visibility_Investigation_Wave2.md`: older visibility/material/custom-depth investigations, useful for historical material parameter and stencil context (`Audit/Reference/Visibility_Readability_Investigation.md:16`; `Audit/Reference/Visibility_Readability_Investigation.md:77`; `Audit/Reference/Visibility_Investigation_Wave2.md:101-118`).
- `Audit/Reference/Tower_Terrain_Investigation/Report.md`: generated kit material and tiling evidence for tower terrain (`Audit/Reference/Tower_Terrain_Investigation/Report.md:106`).
- `Audit/Reference/Track3_Performance_Pass_Report.md`: broad performance pass with material/root counts and rendering-performance context (`Audit/Reference/Track3_Performance_Pass_Report.md:216`).

## Section F - Recommendations

### F1. Torch Density Rule

Recommendation: start with spacing-based placement from eligible interior wall boxes, not one torch per cell. With the live grid at 1300 UU and planned torch radius 800-1000 UU, one torch per cell would overlight the dungeon and destroy the intended dark gaps. A better first pass is one torch candidate every 2400-3000 UU along `TrapEligibleWallBoxes` / `MazeWallBoxes`, capped per floor, skipping doorway headers and outer shell walls. Use a `TryGetMazeWallSpawnLocation()`-style normal validation so each torch sits on a wall with a walkable side (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:5511-5597`).

Tradeoff: this is less semantically elegant than T-junction-only placement, but the current code does not persist a junction or sconce registry. It is also safer than room-per-cell placement because current rooms and corridors are 1300 UU tiles, not 6500 UU macro cells.

### F2. Torch Placement Architecture

Recommendation: attach `UPointLightComponent` instances to a single terrain-owned lighting actor/component, rather than spawning many `APointLight` actors. The only existing dynamic-light pattern uses `UPointLightComponent` on an owning actor and disables shadows (`Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:1238-1261`). A single owner lowers actor overhead, makes teardown easier when tower terrain is rebuilt, and keeps all generated torch lights tied to the generated floor lifecycle.

For inspection, name components predictably and optionally add tiny marker mesh components in development builds or behind a debug flag.

### F3. Placeholder Torch Mesh

Recommendation: use a small basic shape marker for the first pass, not no mesh. The generated Dungeon kit has a wall variant named `DungeonWall_TorchSconce_A`, but no separate torch anchor registry exists (`Source/T66/Gameplay/T66TowerThemeVisuals.cpp:299-313`). A small cube/cylinder marker using an unlit warm material is enough to see placement during iteration without committing to final sconce art.

### F4. Procedural Torch Placement Code Home

Recommendation: create a dedicated helper/module such as `T66TowerLighting` called from `T66TowerMapTerrain::Spawn()` after maze walls are spawned. `T66TowerMapTerrain` owns the necessary `FFloor`, `GridCells`, `MazeWallBoxes`, and `TrapEligibleWallBoxes` data, while `T66TowerThemeVisuals` is currently a theme asset resolver and should not grow procedural spawn behavior (`Source/T66/Gameplay/T66TowerMapTerrain.h:73-120`; `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:193-333`; `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5739-5833`).

This keeps direct access to generation data without further bloating the already large terrain file.

### F5. Lit Environment Master Strategy

Recommendation: author a new `M_Environment_Lit` Default Lit master with the same parameter surface as the active unlit master, then repoint environment MIs to it selectively. Required surface: `DiffuseColorMap`, `BaseColorTexture` alias if practical, `Tint`, `Brightness`, and reasonable Default Lit controls such as roughness/specular defaults. Current generated kit, terrain, and interactable MIs depend on these parameter names (`Saved/Atmosphere_Recon/material_params.json:26-55`; `Source/T66/Gameplay/T66VisualUtil.cpp:52-68`).

Do not directly point environment MIs at a stock engine material. The existing overrides would not map cleanly, and the project has no current environment lit master found in source/config/content references.

### F6. Interactable Consolidation

Recommendation: moving interactables to `MI_GLB_Unlit_Character_Shared` is feasible only if parameter migration is handled. Current interactable materials and runtime helpers use `DiffuseColorMap`, `BaseColorTexture`, `Tint`, `Brightness`, and sometimes write `Color` / `BaseColor`; `M_GLB_Unlit` exposes `BaseColorTexture`, `Tint`, and `Brightness`, but not `DiffuseColorMap` in the probed master surface (`Saved/Atmosphere_Recon/material_params.json:60-88`; `Source/T66/Gameplay/T66VisualUtil.cpp:52-68`).

Migration work to preserve visuals:

- Preserve crate/chest/arcade texture bindings currently stored on their QuadRetro mesh MIs (`Saved/Atmosphere_Recon/asset_inventory.json:10726-10784`).
- Preserve loot bag rarity brightness/tint values: Black `1.1`, Red `1.65`, White `1.85`, Yellow `1.35` (`Saved/Atmosphere_Recon/asset_inventory.json:10797-10912`).
- Either add `DiffuseColorMap` alias support to the target unlit master or migrate all texture overrides to `BaseColorTexture`.
- Avoid assigning every textured interactable to the exact same shared MI asset unless per-instance Dynamic MIDs set the texture/tint/brightness, because each interactable needs its own texture data.

## Verification Performed

- Read required instruction, handoff, pending issue, and prior visual report files.
- Searched source/config/content/audit files for terrain generation, material references, world visual setup, PP volumes, light creation, fog settings, outline/custom-depth paths, map names, and rendering settings.
- Ran read-only Unreal Python commandlet probes to inspect asset parent chains, material parameter surfaces, and map actor inventory.
- Did not run builds, staged executable runs, or gameplay smoke tests, per mission constraints.
