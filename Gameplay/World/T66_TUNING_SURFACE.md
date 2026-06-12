# T66 Tuning Surface

**Scope:** Human-facing tuning index for gameplay balance and tower/world layout knobs. This file is a surface map, not a runtime mega-file. Runtime systems keep their typed owners.

## Runtime Owners

| Area | Runtime owner | Primary knobs |
|---|---|---|
| Tower floors and rooms | `Config/DefaultT66TowerTuning.ini`, `Source/T66/Core/T66TowerTuningConfig.*` | floor count/roles, boss-rush override, floor/cell/grid size, room count/size, room rules, tower chest/crate/fountain defaults, hazard pools |
| Hazard spawn counts and trap mechanics | `Config/DefaultT66TrapTuning.ini`, `Source/T66/Core/T66TrapTuningConfig.*` | keyed floor hazard counts, hazard spawn windows, obstacle sizes/reaction tuning, legacy trap-actor cadence |
| Player experience and difficulty pacing | `PlayerExperience` data/config owners | stage pressure, difficulty totems, progression counts, XP/level pacing |
| Smart loot and reroll bias | `Config/DefaultT66SmartLoot.ini`, `Source/T66/Core/T66SmartLootTuningConfig.*` | build-aware candidate weights, idol/item influence, reroll seen-item decay |
| Heroes/items/weapons/idols | `Content/Data/*.csv` and imported DataTables | base stats, level gains, item/stat families, weapon and idol sizing/damage |
| Stage progression and spawn pressure | stage progression data/config owners | difficulty bands, stage themes, enemy roster pressure |

## Tower Tuning Notes

`DefaultT66TowerTuning.ini` currently preserves the live code default: a 4-floor normal tower (`1` start, `2-3` mob, `4` boss). `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md` describes the future 5-floor contract. Treat changing `BossFloorNumber` / `LastMobFloorNumber` as a tuning instruction, not infrastructure cleanup.

Boss-rush finale stays an explicit override through `BossRushBossFloorNumber`.

## Room Rule Notes

Rooms are generated as durable layout records on `T66TowerMapTerrain::FFloor`. Room rules in `DefaultT66TowerTuning.ini` are data classes, not actor classes. The current combat-room consumers enforce:

- `HazardSlots`: room-level tower hazard spawning for floors with a configured hazard pool.
- `RewardContentSlots`: one room reward/content placement pass for tower mob floors.
- vendor uniqueness: one vendor per mob floor, currently floors `2` and `3`.

Room-size tuning still flows through `DungeonMinRoomTiles` / `DungeonMaxRoomTiles` for active combat-room generation. Keep the `RoomRules.WidthTiles` / `HeightTiles` ranges in sync so the human-facing rule class matches the generated-room defaults.

## Current Tower Defaults

| Knob | Current value |
|---|---:|
| `StartFloorNumber` | `1` |
| `FirstMobFloorNumber` | `2` |
| `LastMobFloorNumber` | `3` |
| `BossFloorNumber` | `4` |
| `BossRushBossFloorNumber` | `2` |
| `GridColumns` / `GridRows` | `70 / 70` |
| `PlacementCellSize` / `GridCellSize` / `GridDoorWidth` | `1000 / 1000 / 1000` |
| `DungeonMinRooms` / `DungeonMaxRooms` | `10 / 10` |
| `DungeonMinRoomTiles` / `DungeonMaxRoomTiles` | `10 / 20` |
| combat room `HazardSlots` | `1-2` |
| combat room `RewardContentSlots` | `1` |
| `TowerChestCountPerFloor` | `1-3` |
| `TowerCrateCountPerFloor` | `1-3` |
| `TowerFountainChancePerFloor` | `0.40` |
| hazard pools | floors `2` and `3` obstacle pool, floor `4` empty |
| `BounceCoursePlatforms` | `1` (on) |
| `PlatformTier1Height` / `PlatformTier2Height` | `200 / 400` |
| `ChainPlatformFootprint` / `PlatformChainMaxGap` | `700 / 350` |
| `RoomPlatformFootprintMin-Max` / `RoomPlatformDensityTiles` | `550-750 / 8` |
| `RoundPlatformChance` (non-square shape share: round/hex/tri per room theme) | `0.45` |
| `RingMesaChance` (>=5x5 mesa decks get a center drop hole) | `0.5` |
| `RampWidth` / `RampLength` | `600 / 520` |
| `TierLifts` / `LiftChance` | `1 (on) / 0.5` |
| `LiftFootprint` | `600` |
| `LiftTravelSeconds` / `LiftDwellSeconds` | `3.0 / 2.0` |
| `TowerLavaRise` | `1` (on) |
| `LavaGraceSeconds` / `LavaRiseSeconds` / `LavaMaxHeight` | `25 / 150 / 320` |
| `LavaDamagePerTick` | `20` |
| `DoorwayArches` / `ArchSegments` / `ArchTubeDiameter` | `1 / 10 / 110` |

Bounce-course and lava knobs are clamped in `UT66TowerTuningConfig::Sanitize()`
against hero jump reach: tier steps stay at or below `260`, the chain gap at or
below `420`, and `LavaMaxHeight` at least `60` below `PlatformTier2Height`, so a
tuning change cannot break the guaranteed dry arrival->exit chain. See
`T66_MAP_DESIGN_REFERENCE.md` section 1.2.

Lift knobs are clamped the same way: `LiftFootprint` fits its grid cell with mesa
clearance, `LiftTravelSeconds` is floored so the rise speed stays at or below
`~400uu/s` (rideable), and `LiftDwellSeconds` keeps at least `0.75s` of boarding
time. See `T66_MAP_DESIGN_REFERENCE.md` section 1.4.

## Ready For Later Tuning

The next tuning pass can safely request changes in this format:

```text
Tower:
- normal floors: start 1, gameplay 2-4, boss 5
- room count: 12-18
- room size: 3-6 tiles
- combat room rule: exactly 1 hazard and exactly 1 chest/crate/NPC/interactable
- fountain chance: 25%
- hazard pool: floors 2-4
```

Implementation rule: change data first, then verify generated layout summaries and staged runtime behavior before broad balance changes.
