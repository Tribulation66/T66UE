# T66 Master Map Design

**Last updated:** 2026-05-28
**Scope:** Single-source handoff for the live tower-only Tribulation map runtime, stage-space flow, traversal structure, minimap behavior, miasma/blood pressure design, and the historical implementation plan that led to the tower layout.  
**Companion docs:** `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md`, `Gameplay/World/T66_LIGHTING_REFERENCE.md`  
**Maintenance rule:** Update this file after every material map-layout, terrain-generation, stage-space, minimap, traversal-gate, floor-transition, miasma/blood-pressure, or preset-selection change.

## 1. Executive Summary

- The live Tribulation runtime is now `Tower`-only.
- Runtime map-layout selection has been removed, including the old non-tower layout variants and config/console overrides.
- New runs and save-load restore now coerce the main gameplay map to the tower layout.
- The active gameplay map family is the tower generator and floor-based traversal path, not the old one-board terrain preset selection flow.
- Current normal tower pacing uses five floors:
  - `Floor 1 - Start`: weapon altar only; no enemies and no normal interactable scatter
  - `Floor 2` through `Floor 4`: gameplay floors with enemies and normal floor interactables
  - `Floor 5 - Boss`: boss flow only
- Boss-rush finale is a separate two-floor special case; it skips normal tower enemy and interactable population and must not be treated as the normal map-design floor-count contract.

### 1.1 Current tower implementation status

- The runtime now hard-locks Tribulation gameplay to the tower layout.
- Save/load compatibility still persists a layout field, but load paths sanitize back to `Tower`.
- The first playable tower pass currently does all of the following:
  - now uses a `400m x 400m` tower footprint so each gameplay floor has enough room for a real maze layout without the oversized first expansion pass
  - now uses a square tower shell with square floor footprints so the walls and floors meet directly without polygon-gap cleanup work
  - now clamps tower floor tiles all the way to the shell bounds so gameplay floors no longer leave a visible perimeter gap away from the surrounding wall
  - builds tower floors from tiled floor blocks instead of stretched slab bands, so the tower now reuses the regular flat-ground material look instead of the striped placeholder look
  - uses a lower tower floor spacing and smaller square descent holes so players cannot trivially double-jump back up through the previous floor opening
  - now snaps tower drop holes to the floor tile grid and removes exactly one square floor tile, so the descent opening reads as a true `1x1` square instead of an offset rectangle
  - randomizes each floor's square descent-hole position from the run seed instead of keeping the holes on a fixed shared pattern
  - adds full-height internal maze walls on gameplay floors and teaches tower surface placement to treat those wall volumes as blocked space
  - pushes maze-wall spans all the way into the outer shell so gameplay floors no longer keep an open perimeter ring around the tower walls
  - keeps `Floor 1 - Start` on the generic dungeon-tower scaffold again instead of the later enclosed-room pass, while still keeping tree props out of Stage 1
  - restores the original taller Stage 1 tower read by using the roof cap only at the top of the tower instead of adding the later low ceiling caps between floors
  - keeps `Floor 1 - Start` as a clean start floor with the weapon altar and no guaranteed start-area interactable scatter
  - keeps the stage-entry idol altar out of the current tower bootstrap path so the start floor remains weapon-altar-only
  - no longer spawns the old idol-area dummy enemy test targets
  - starts real stage combat/timer pressure once the player reaches the gameplay floors
  - builds blood-style miasma coverage across gameplay floors only
  - keeps tower-only combat rules isolated from the classic main-map rules:
    - no lava patch stage effects on tower floors
    - some enemies are pre-populated onto gameplay floors before the player reaches them
    - reinforcement enemies now emerge from tower wall surfaces instead of appearing from thin air
    - reinforcement enemies are now restricted to the outer tower shell walls, not interior maze walls
  - scopes minimap/full-map display to the current floor with floor-local reveal memory and polygon floor bounds
  - removes the old non-tower visual-navigation pillar from tower runs
  - removes obsolete traversal-pad interactables from stage population
  - allows compact casino/vendor utility opportunities to roll on gameplay floors only
  - can spawn a rare Backrooms entrance door on a tower wall when the run does not already own the Backrooms Quick Revive item; the Backrooms maze is spawned with the stage as a hidden pocket space, not loaded later
  - spawns exactly one saint on one gameplay floor per tower stage
  - spawns exactly one difficulty totem on every gameplay floor using floor-specific `T66_Tower_DifficultyTotem_##` tags, independent of the randomized saint floor selection
  - guarantees tower chest/crate rules by floor instead of using the old global stage scatter for those two interactables:
    - `Floor 1 - Start`: no chest, no crate
    - floors `2-4`: `1-3` chests and `1-3` crates per floor
    - `Floor 5 - Boss`: no chest, no crate
  - now rejects cross-floor tower placement traces and retries tower floor placement more aggressively, so gameplay-floor chest/crate/casino/utility placement stays on the intended floor instead of bleeding through floor gaps to another floor
  - snaps tower NPC/interactable spawns to an explicit requested floor and tags them with explicit tower-floor identity so floor-local placement and safety logic do not rely only on raw actor Z
  - re-snaps tower chest/crate/totem/wheel placements after rarity/configuration work so floor-local mesh swaps cannot pull those actors back onto the wrong floor
  - tower boss clears now spawn the next-stage gate at the dedicated tower boss exit location instead of trusting the exact boss death point
  - tower visuals are now stage-aware:
    - `Stage 1`: dungeon floor/wall/roof materials and no runtime rock/tree scatter props
    - `Stage 2`: forest ground/roof materials plus theme wall replacements for both inner maze walls and the outer perimeter shell
  - old tower-only collidable rock/deco scatter on gameplay floors has been disabled; keep floor dressing out of the map/runtime terrain path unless a new generated-prop pass is explicitly approved
  - uses tower-only dungeon material instances for floor, wall, and roof surfaces instead of the inherited green/brown legacy terrain look
  - uses dedicated tower descent-hole trigger actors for floor-to-floor progression while keeping the existing boss-kill `StageGate` portal for actual stage-to-stage travel
  - gates the `Floor 2`, `Floor 3`, and `Floor 4` descent holes behind fixed placed miniboss encounters: a scaled rich `Slime` spawns when the player enters each floor, and the descent hole cannot be opened until that miniboss is defeated
  - replaces the legacy boss-threshold gate on tower with final-hole boss entry, so dropping from `Floor 4` into `Floor 5 - Boss` pauses normal wave spawning and starts boss flow
  - keeps `Floor 5 - Boss` as the terminal floor with no further descent hole
  - uses tower-specific fall-rescue logic so normal hole descent does not snap players back onto upper floors
  - uses tighter floor-classification tolerance so falling between floors is not misread as standing on a gameplay floor
  - snaps tower bosses after boss initialization so tall boss meshes do not sink into the floor
  - resets run-state and damage-log state on pause-menu restart so tower restarts follow the same reset path as the run-summary restart
  - no longer relies on stale static raw mesh pointers for tower prop decoration, fixing the packaged tower reload/restart crash that could happen while rebuilding the tower after a gameplay map restart
- The tower preset is still not complete:
  - broader tower-specific casino/vendor/NPC pacing beyond saint is still open

## 2. Current Map Runtime Spine

### 2.1 Preset selection and persistence

- `Source/T66/Gameplay/T66ProceduralLandscapeParams.h`
  - defines `ET66MainMapLayoutVariant`
  - current live value is:
    - `Tower`
- `Source/T66/Core/T66GameInstance.cpp`
  - no longer reads map-layout config
  - no longer supports a map-layout console override
  - applies the finalized tower layout directly to runtime run state
- Save/load already persists the chosen layout variant:
  - `Source/T66/Core/T66RunSaveGame.h`
  - `Source/T66/UI/Screens/T66PauseMenuScreen.cpp`
  - `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp`
- Important limitation:
  - saves still carry a layout field for compatibility
  - saves do **not** know a concept like `current floor inside a tower stage`

### 2.2 Current main-stage geometry model

- `Source/T66/Gameplay/T66MainMapTerrain.cpp`
  - is the active generator for the normal gameplay stage
  - builds a single square board with reserved regions:
    - `StartArea`
    - `StartPath`
    - `MainBoard`
    - `BossPath`
    - `BossArea`
- `Generate(...)`
  - expects one contiguous grid footprint
  - not stacked floors
  - not cylindrical walls
  - not floor-to-floor descent holes

### 2.3 Current stage flow

- `Source/T66/Gameplay/T66GameMode.cpp`
  - boots one stage as one play space
  - current normal-stage sequence is:
    1. spawn terrain
    2. place tower start-floor content
    3. player crosses start threshold
    4. stage timer starts
    5. miasma + enemy director activate
    6. player reaches boss threshold
    7. boss awakens
    8. boss dies
    9. stage gate spawns
    10. stage transition reloads the gameplay map for the next stage
- Current special flow actors:
  - `AT66StartGate`
  - `AT66BossGate`
  - `AT66StageGate`
  - `AT66CowardiceGate`
  - `AT66IdolAltar`

### 2.4 Current timing and pressure model

- `Source/T66/Core/T66RunStateSubsystem.h`
  - stage timer duration is currently fixed:
    - `420s`
    - `7:00`
- `Source/T66/Gameplay/T66MiasmaManager.cpp`
  - normal layouts still build coverage over the stage footprint
  - tower currently builds blood-style coverage over gameplay floors only
  - tower hazard timing is floor-aware, but altar-driven blood escalation is still not fully authored

### 2.5 Current minimap model

- `Source/T66/UI/T66GameplayHUDWidget.cpp`
  - normal-layout minimap is still a generic XY snapshot over the current stage
  - tower minimap/full-map are now active-floor views
  - tower reveal memory is floor-local and resets visually when moving to another floor
  - tower map rendering now uses the current floor polygon instead of the old square placeholder
  - full map uses broad stage areas:
    - `START`
    - `MAIN`
    - `BOSS`
  - markers come from actor positions, not explored floor tiles
- Current limitation:
  - normal layouts do not have per-floor reveal memory because they are still single-board stages
  - tower reveal is marker- and floor-memory-based; it no longer depends on a lighting or fog visibility mask

### 2.6 Current NPC / interactable placement

- `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp`
  - `SpawnWorldInteractablesForStage()`
    - scatters world interactables across the stage, excluding reserved start/boss zones
  - `SpawnSupportVendorAtStartIfNeeded()`
    - optional start-area vendor
  - `SpawnCircusInteractableIfNeeded()`
    - current casino shell
- Important current reality:
  - the "casino" is not a separate Gambler NPC in stage placement
  - it is `AT66CircusInteractable`
  - current circus/casino spawns at most once per stage
  - current circus safe zone radius is large (`1100`)
  - current circus mesh is `/Game/World/Interactables/Casino/Casino.Casino`
- Backrooms entrance doors are tower-wall interactables placed by the tower GameMode bootstrap. The spawned pocket uses `/Game/World/Backrooms/Textures/T_Backrooms_Wall`, `T_Backrooms_Floor`, and `T_Backrooms_Door`, with a closed entrance door behind the player and a separate exit door inside the maze.
- Entering the Backrooms pauses stage pressure, timers, enemies, traps, projectiles, miasma, and save-and-return. The hero and Backrooms chaser remain active; inventory and weapon state are temporarily hidden and restored on exit or death resolution.
- A successful Backrooms exit grants the reward-only `Item_BackroomsQuickRevive` item. Owning that item makes later Backrooms door rolls ineligible.
- Non-shipping QA uses `T66.Backrooms.ForceSpawn 1` plus `-T66BackroomsAutoQA=Exit|Death|Consume`. `-T66BackroomsAutoQAScreenshot=<path>` is requested by `AT66GameMode` only after the entry interaction has made `bBackroomsChallengeActive` true, so the proof image captures the active hidden pocket rather than the pre-entry tower wall.
- `SpawnIdolAltarForPlayer()` / `SpawnIdolAltarAtLocation(...)`
  - currently supports:
    - stage-start altar
    - post-boss altar near gate spawn

### 2.7 Current enemy spawn model

- `Source/T66/Gameplay/T66EnemyDirector.cpp`
  - normal layouts still spawn around player world positions using min/max distance rings and safe-zone exclusion
  - tower layout now filters by the active floor and spawns from the shell perimeter instead of below-ground rise locations
- `Source/T66/Gameplay/T66GameMode.cpp`
  - legacy visual-navigation and traversal-pad runtime paths have been removed
  - tower terrain safety rescue now uses tower-local traces and a below-bottom-floor threshold so falling through holes is not treated as an out-of-bounds recovery
- `AT66BossGate`
  - currently pauses the enemy director when the player enters the boss threshold

## 3. Current Design Constraints That Matter For The Tower Preset

### 3.1 The current system assumes one continuous stage board

- Start flow, combat activation, boss activation, miasma coverage, minimap, and interactable scatter all assume one board for one stage.

### 3.2 Save/load needs new floor-aware state

- Layout variant is already saved.
- Floor index, current-floor hazard state, and per-floor reveal state are not.

### 3.3 The current minimap is too stage-global

- Tower requires the minimap to become a current-floor system.

### 3.4 The current miasma system is too footprint-global

- Tower blood pressure needs to care about the active floor only.

### 3.5 The current casino implementation is already a shell/interactable, not a gambler house NPC

- Recommended path:
  - keep the casino as a variant of `AT66CircusInteractable`
  - shrink it for the tower preset
  - do **not** revive a separate Gambler NPC unless that is a deliberate broader design change

## 4. Target Tower Preset Design

### 4.1 Core fantasy

- One gameplay stage remains one runtime stage and one timer.
- Inside that stage, the player descends through multiple vertically stacked floors.
- Each floor is part of the same stage, not a separate stage.
- The geometry theme is:
  - connected polygon shell with straight walls for the first practical implementation
  - walkable floor footprint that can evolve toward a rounder silhouette later if needed
  - side-wall enemy emergence
  - one descent hole leading to the next floor
  - final floor is the boss floor

### 4.2 Recommended first playable structure

- Current normal tower structure:
  - `Floor 1 - Start`: start floor only, modeled after the current start area, with the weapon altar and no normal enemy/wave gameplay
  - `Floor 2`: gameplay floor with monsters, world interactables, and side-wall enemy spawning
  - `Floor 3`: gameplay floor with monsters, world interactables, and side-wall enemy spawning
  - `Floor 4`: gameplay floor with monsters, world interactables, and side-wall enemy spawning
  - `Floor 5 - Boss`: boss floor
- Boss-rush finale is an explicit two-floor exception and skips the normal enemy/interactable population rules.
- Reason for this recommendation:
  - `7:00` total stage time is already established in runtime/UI/save logic
  - `3:00` boss budget leaves `4:00` for non-boss gameplay
  - separating the top floor as a start-only space preserves the current game feel
  - `5` total floors keeps the descent readable without the old seven-floor confusion

### 4.3 Traversal rules

- `Floor 1 - Start` should own:
  - the stage-start weapon altar
  - current start-area feel and breathing room
  - no standard monster spawning
  - no normal world-interactable scatter pass
  - the first descent hole into real gameplay
- Each gameplay floor (`Floor 2` through `Floor 4`) should own:
  - one local exploration space
  - one descent hole
  - side-wall enemy spawn anchors
  - optional small casino / vendor opportunities
- `Floor 5 - Boss` should own:
  - boss entry point
  - boss combat space
  - no further descent hole
- Live implementation note:
  - between floors in the same stage, traversal is handled by the floor hole
  - after the boss dies, progression to the next stage still uses the existing `AT66StageGate` portal

### 4.3.1 Placed miniboss descent gates

- Normal tower stages use deliberate placed miniboss encounters instead of random per-wave miniboss promotion.
- The eligible floor exits are:
  - `Floor 2 -> Floor 3`
  - `Floor 3 -> Floor 4`
  - `Floor 4 -> Floor 5 - Boss`
- `Floor 1 -> Floor 2` has no miniboss gate.
- Boss-rush finale stages are a two-floor exception and have no placed miniboss gates.
- The placeholder miniboss is the basic `Slime` mob row, spawned as a rich `AT66EnemyBase` and scaled with the current miniboss multipliers (`3.0x` HP, `2.0x` damage, `1.75x` actor scale).
- The same slime placeholder appears across all stages until unique miniboss creatures are authored.
- The placed miniboss spawns when the player enters its floor, is assigned to that floor's `AT66TowerDescentHole`, and must be killed before the hole can open.
- The existing tower guardian death behavior still runs on defeat, including the idol altar side effect; changing miniboss rewards is a separate design pass.
- Random miniboss promotion in normal trickle waves is disabled, which keeps basic-mob performance captures free of random rich miniboss routes.

### 4.4 Hazard rules

- Replace the current lava presentation with blood / miasma for the tower preset.
- Recommended first-pass behavior:
  - interacting with the floor altar arms that floor's blood pressure
  - blood spreads only on the current floor
  - the HUD timer turns red once current-floor blood is active
  - when the player descends, the previous floor can be discarded from active hazard simulation

### 4.5 Minimap rules

- Minimap should show only the active floor.
- Floor starts mostly dark.
- The visible region is revealed through exploration.
- When the player descends:
  - old floor map is no longer the active display
  - new floor starts dark again
- Recommended first-pass rule:
  - no stacked tower overview yet
  - current-floor-only map is enough

### 4.6 NPC / interactable rules for tower preset

- Casino:
  - use a smaller circus/casino version
  - chance to spawn on each gameplay floor
- Saint:
  - exactly once per stage
  - only on one gameplay floor
- Baby gate:
  - placed beside the descent hole before the boss floor

### 4.7 Visual and asset rules for tower preset

- The first tower prototype should reuse existing assets only.
- Terrain materials:
  - reuse the same texture/material family already used by the current main map
  - do not block the preset on new texture work
- Props:
  - do not reuse old tree, dirt, or rock clutter from the legacy map/prop pipeline
  - use only current approved generated visual props or a future explicitly reviewed generated-prop pass
- Practical implication:
  - the tower preset should be shippable with current tower terrain textures and latest approved generated visual props
  - no new environment-art dependency is required for the first playable version

## 5. Recommended Technical Approach

### 5.1 Keep preset selection shared, but split geometry implementation

- Keep one shared live preset enum value:
  - `Tower`
- Do **not** force tower geometry into a long chain of `if (LayoutVariant == Tower)` inside `T66MainMapTerrain::Generate(...)`.
- Recommended implementation:
  - keep tower geometry in the tower runtime generator

### 5.2 Use one runtime world, stacked vertically

- Recommended first implementation:
  - all floors exist inside the same gameplay world
  - floors are separated by Z distance
  - descending through a hole moves the player to the next floor in the same map
- Reason:
  - preserves current stage model
  - avoids per-floor streaming as the first implementation burden
  - keeps one stage timer and one stage save context

### 5.3 Add floor-aware runtime state

- Add state for:
  - current tower floor index
  - total floors in this stage
  - current-floor blood active state
  - current-floor reveal data
- This should be available to:
  - `GameMode`
  - minimap/HUD
  - save/load
  - enemy spawning

### 5.4 Convert spawning from area-random to floor-anchor-driven

- Current `EnemyDirector` chooses random world positions around players.
- Tower preset needs authored/runtime-generated wall anchors.
- Recommended first-pass architecture:
  - tower generator emits wall spawn anchors per floor
  - enemy director selects from anchors on the active floor
  - anchors must respect min distance and visibility fairness

### 5.5 Make hazard coverage provider-driven

- Current `AT66MiasmaManager` derives coverage from the whole main board.
- Tower preset needs:
  - current floor footprint
  - blood visual profile
  - floor-local damage only
- Recommended path:
  - keep the manager actor
  - add coverage/profile modes rather than writing a second unrelated hazard actor

### 5.6 Keep the casino as the circus shell

- The project already routes casino behavior through `AT66CircusInteractable`.
- Recommended tower approach:
  - create a smaller tower-specific circus/casino spawn mode
  - reduce safe-zone radius
  - reduce visual footprint
  - preserve the existing circus overlay flow

## 6. Required File Surface

### 6.1 Preset plumbing

- `Source/T66/Gameplay/T66ProceduralLandscapeParams.h`
  - add `Tower`
- `Source/T66/Core/T66GameInstance.cpp`
  - parse `Tower`
  - extend console override handling beyond `0/1`
- `Config/DefaultGame.ini`
  - update valid-values comment

### 6.2 New tower runtime generator

- Recommended new files:
  - `Source/T66/Gameplay/T66TowerMapTerrain.h`
  - `Source/T66/Gameplay/T66TowerMapTerrain.cpp`
- Responsibilities:
  - build stacked floor descriptors
  - emit traversal/boss floor anchors
  - emit side-wall spawn anchors
  - emit hole locations
  - emit minimap bounds per floor

### 6.3 Game flow / floor transitions

- `Source/T66/Gameplay/T66GameMode.h/.cpp`
  - route to tower preparation when `LayoutVariant == Tower`
  - replace start/boss-board assumptions for that preset
  - manage active floor transitions
- Recommended new actor(s):
  - `AT66TowerDescentHole`
  - `AT66TowerBabyGate`

### 6.4 Enemy spawning

- `Source/T66/Gameplay/T66EnemyDirector.h/.cpp`
  - either:
    - add spawn-provider support
    - or split tower spawning into a dedicated tower director

### 6.5 Hazard / timer warning

- `Source/T66/Gameplay/T66MiasmaManager.h/.cpp`
  - add tower floor coverage mode
  - add blood visual profile
- `Source/T66/UI/T66GameplayHUDWidget.cpp`
  - timer warning turns red when current-floor blood is active

### 6.6 Minimap

- `Source/T66/UI/T66GameplayHUDWidget.h/.cpp`
  - active-floor-only map mode
  - per-floor reveal state
  - tower minimap bounds instead of current stage-global board framing

### 6.7 Save/load

- `Source/T66/Core/T66RunStateSubsystem.h/.cpp`
  - export/import tower floor state
- `Source/T66/Core/T66RunSaveGame.h`
  - persist current tower floor data
- `Source/T66/UI/Screens/T66PauseMenuScreen.cpp`
- `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp`
  - save/load new tower state

## 7. Recommended Implementation Order

### Phase 1. Lock the preset and floor model

- Keep the live layout locked to `Tower`.
- Lock initial stage shape:
  - `1` start floor
  - `4` gameplay floors
  - `1` boss floor
  - existing textures/materials only
  - current terrain/material assets only; no old tree, dirt, or rock clutter props

### Phase 2. Get tower geometry playable without full feature parity

- Build the stacked tower floors.
- Add descent holes.
- Add active floor tracking.
- Let the player move from floor to floor.
- Keep `Floor 1 - Start` as a start-only, weapon-altar-only floor.

### Phase 3. Move combat spawning to tower rules

- Add side-wall spawn anchors.
- Make enemy spawns floor-aware.
- Restrict spawns to the active floor.
- Keep normal monster spawning disabled on `Floor 1 - Start`.

### Phase 4. Move stage systems to tower rules

- Replace boss-threshold assumptions with boss-floor entry behavior.
- Move baby gate to final traversal floor.

### Phase 5. Move hazard pressure to tower rules

- Convert tower preset from lava presentation to blood presentation.
- Make hazard floor-local.
- Arm it from altar interaction.
- Turn timer red when active.

### Phase 6. Move the minimap to floor-local reveal

- Current floor only
- dark-until-explored
- reset visibility state on descent

### Phase 7. Save/load and standalone validation

- Save current floor index
- save active floor hazard state
- save map reveal state if needed
- validate in packaged Development standalone, not just editor

## 8. Recommended Defaults For First Pass

- Preset enum label:
  - `Tower`
- Start floor count:
  - `1`
- Gameplay floor count:
  - `3` (`Floor 2` through `Floor 4`)
- Boss floor count:
  - `1`
- Circus/casino:
  - smaller tower-specific variant
  - chance per gameplay floor
- Saint:
  - exactly once per stage
- Baby gate:
  - not part of the current normal five-floor tower contract
- Art source:
  - reuse current terrain textures/materials
  - do not reuse old tree, dirt, or rock clutter props

## 9. Open Questions Still To Lock

1. Do we want the preset name to stay plain `Tower`, or should it be something more thematic like `Descent` while keeping the same design?
2. Does altar interaction immediately arm blood pressure, or should blood begin after a short grace delay on that floor?
3. Should the timer remain frozen on `Floor 1 - Start` and only begin once the player descends into `Floor 2`, matching the current start-area philosophy?
4. Should the casino be allowed on `Floor 4`, or should that floor stay focused on pre-boss setup?
5. Do we want one altar on every gameplay floor, or only on selected floors after pacing tests?

## 10. Bottom Line

- The current project already has the right place to add a third preset.
- The current project does **not** have a tower-ready runtime model yet.
- The major shift is not just terrain shape; it is a change from:
  - one continuous stage board
  - one global minimap
  - one global hazard footprint
  - area-random enemy spawning
- The safest path is:
  - add `Tower` as a third preset
  - implement tower geometry as a separate map family
  - then migrate minimap, hazard, spawn, and NPC rules onto a floor-aware runtime model in phases
