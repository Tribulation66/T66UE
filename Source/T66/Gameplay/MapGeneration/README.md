# T66 Map Generation

This folder owns procedural map-generation infrastructure for tower rooms and floors.
It is the staging area for moving room interiors away from hardcoded room archetypes and
toward data-driven composition.

The current gameplay goal is still bullet-heaven first: rooms must preserve open combat
space, enemy readability, reward access, and lava escape routes. Structures and hazards
decorate and pressure the room; they should not turn the room into a pure obstacle course.

## Current Ownership

- `T66TowerMapTerrain.*` still owns the floor layout, room bounds, grid cells, and the
  existing runtime output arrays used by spawning and rendering.
- `MapGeneration` owns the vocabulary and future composition layer that decides which
  reusable room ingredients should be emitted into those arrays.
- `Core/T66TowerTuningConfig.*` owns config-facing room budgets such as `HazardSlots`,
  `RewardContentSlots`, and `HazardPools`.
- `Core/T66TrapSubsystem.*` is still the legacy runtime actor/spawn implementation for
  offensive obstacles. Tower-generation code should describe those as hazards even while
  the underlying actor classes still use the older trap names.

## Folder Layout

```text
MapGeneration/
  README.md
  Composition/
    T66RoomComposer.h/.cpp
    T66RoomFeaturePlacer.h/.cpp
  Libraries/
    T66StructureLibrary.h/.cpp
    T66HazardLibrary.h/.cpp
  Types/
    T66RoomCompositionTypes.h
  Validation/
    T66LavaSurvivalGraph.h/.cpp
    T66RoomValidation.h/.cpp
```

Expected future layout:

```text
MapGeneration/
  Types/
    T66RoomCompositionTypes.h
    T66StructureTypes.h
    T66HazardTypes.h
  Libraries/
    T66StructureLibrary.h/.cpp
    T66HazardLibrary.h/.cpp
  Composition/
    T66RoomComposer.h/.cpp
    T66RoomFeaturePlacer.h/.cpp
  Validation/
    T66RoomValidation.h/.cpp
    T66LavaSurvivalGraph.h/.cpp
  Debug/
    T66RoomDebugReport.h/.cpp
```

Do not add empty placeholder source files just to match the future layout. Add each area
when code actually moves there.

## Vocabulary

Use these names in new map-generation code:

- **Structure**: neutral or helpful room geometry and movement toys. Examples: central
  mesa, ring mesa, stepping stones, bridge deck, scatter stones, ramps, lifts, bounce
  pads, low lava-safe islands.
- **Hazard**: offensive obstacle that threatens positioning or launches/damages the
  player. Examples: sweeper arm, ceiling hammer, bumper, crusher, launcher.
- **Reward slot**: a designed payoff point consumed by interactable/NPC/chest/crate
  population before random placement.
- **Composition profile**: budget and constraint profile for a room. It should describe
  how much structure, hazard, reward, openness, and lava survivability a room wants. It
  should not describe a named room theme.

Avoid adding new room names such as `TailTagCourtyard`, `BridgeCross`, or `RingRun`.
Those concepts should emerge from structures, hazard anchors, reward slots, and validation.

## Current Types

`Types/T66RoomCompositionTypes.h` currently defines:

- `FRoomCompositionProfile`
- `FStructureDefinition`
- `FHazardDefinition`
- reusable profile IDs:
  - `CombatPlayhouse`
  - `FlatCombat`
- initial structure IDs:
  - `Structure.CentralMesa`
  - `Structure.RingMesa`
  - `Structure.SteppingStones`
  - `Structure.BridgeDeck`
  - `Structure.ScatterStones`
- initial hazard IDs:
  - `ObstacleSweeperArm`
  - `ObstacleCeilingHammer`

These IDs are the first infrastructure bridge from the old hardcoded room-template path.
They are not final content breadth.

## Current Modules

`Libraries/T66StructureLibrary.*` owns the first hardcoded metadata library for:

- composition profiles
- structure definitions

This is intentionally C++ data for now. Move it to config or data assets only after the
composer/validator loop is stable.

`Libraries/T66HazardLibrary.*` owns the first hardcoded metadata library for hazard
definitions. It intentionally maps to the existing obstacle registry IDs for now, because
the runtime actor classes still use legacy trap names.

`Composition/T66RoomComposer.*` owns structure selection. It receives a room, layout
tuning, and RNG, then writes:

- `CompositionProfileID`
- `StructureIDs`

Selection is profile-budget driven. `CombatPlayhouse` rooms can now combine a primary
movement structure with sparse scatter-stone support. This is intentionally limited to
stepping-stone and bridge-deck primaries so the first multi-structure pass keeps combat
space and lava escape readable.

`Composition/T66RoomFeaturePlacer.*` owns generated room feature placement:

- floor-level hazard anchor reset
- reward-slot writes
- hazard-anchor writes
- scatter-stone placement
- stepping-stone structure placement
- bridge-deck structure placement
- shared room placement context and occupancy callbacks

Mesa and ring-mesa tier construction still lives in `T66TowerMapTerrain.cpp` because it
owns the cell-tier arrays, tier ramp structs, and lift structs. The feature placer owns
the shared structure identity helpers, and mesa geometry should move after those terrain
helpers become portable.

`Validation/T66RoomValidation.*` owns profile/budget validation for generated rooms:

- structure budget
- reward budget
- hazard budget
- estimated combat openness
- structure density
- lava-survival support

`Validation/T66LavaSurvivalGraph.*` owns the first lava-survival support query. It is a
lightweight bridge for now and should grow into the dedicated hop graph when multi-structure
generation starts.

## Data Flow

Current data flow:

```text
DefaultT66TowerTuning.ini
  -> UT66TowerTuningConfig
  -> T66TowerMapTerrain::BuildLayout
  -> FFloor / FRoom output arrays
  -> world spawning, hazard spawning, reward/content placement
```

Composition output should continue to emit the existing runtime arrays until there is a
larger terrain-output refactor:

- `FRoom::CompositionProfileID`
- `FRoom::StructureIDs`
- `FRoom::RewardSlots`
- `FFloor::BouncePlatforms`
- `FFloor::BounceRamps`
- `FFloor::TierMesas`
- `FFloor::TierRamps`
- `FFloor::TierLifts`
- `FFloor::BouncePadSpots`
- `FFloor::HazardAnchors`
- `FFloor::HazardAnchorTypes`

## Implementation Rules

1. Reserve combat space first.
2. Select a composition profile by room role, floor, size, and difficulty.
3. Select structures from `T66StructureLibrary`, not from named room templates.
4. Place reward slots at readable payoff points.
5. Place a small number of hazards through hazard anchors or room hazard budgets.
6. Validate reachability, lava survival, overlap, combat openness, and density through
   `T66RoomValidation`.
7. Downgrade or retry invalid compositions rather than forcing bad geometry.

## What Belongs Here

Put code here when it decides or validates procedural room/floor composition:

- structure definitions and selection rules
- hazard definitions and room-level hazard placement intent
- room composition profiles
- room feature placement
- lava survival graphs
- map-generation validation
- debug summaries for generated rooms

## What Does Not Belong Here

Do not move these systems here:

- runtime hazard actor behavior, damage, VFX, or animation
- UI widgets, HUD/minimap rendering, or reward-card presentation
- enemy spawning behavior
- save data, economy, item, surgery, or drug systems
- high-poly mesh generation or imported visual asset authoring

Those systems may consume map-generation output, but they should keep their current owners.

## Current Migration Boundary

The room generator no longer stores a single named room template such as `CenterKeep`,
`RingRun`, `SteppingCourt`, `BridgeCross`, or `OpenArena`. It stores a composition profile
and reusable structure IDs.

The underlying offensive-obstacle actors still have legacy `Trap` class and file names.
That is a separate Unreal asset-safe migration. New tower-room generation code should use
`Hazard` terminology even when calling the legacy runtime subsystem.
