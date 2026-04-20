# T66 Master Traps

**Last updated:** 2026-04-18
**Scope:** Single-source handoff for environmental trap runtime, ownership, procedural spawning, damage routing, trap-family identity, and shipped trap VFX/mesh rules.
**Companion docs:** `MASTER DOCS/T66_MASTER_GUIDELINES.md`, `MASTER DOCS/MASTER_MAP_DESIGN.md`, `Docs/Implementation Plan/T66_Tower_Multi_Agent_Implementation_Plan.md`
**Maintenance rule:** Update this file after every material trap, trap-subsystem, trap-spawn, trap-damage, trap-VFX, or trap-map-integration change.

## 1. Executive Summary

- Runtime trap ownership lives in `UT66TrapSubsystem`, a world subsystem.
- Trap actors derive from `AT66TrapBase` and auto-register with the subsystem on `BeginPlay`.
- The currently shipped tower trap families are:
  - `WallProjectile`
  - `FloorBurst`
  - `AreaControl`
- The current concrete tower trap actors are:
  - `AT66WallArrowTrap`
  - `AT66FloorFlameTrap`
  - `AT66FloorSpikePatchTrap`
- Tower gameplay floors now use level-driven trap pools instead of the original one-arrow/one-flame-only pass.
- Trap damage still routes through `UT66RunStateSubsystem::ApplyDamage()`, so safe-zone checks, invulnerability, floating feedback, and player-death flow stay on the normal runtime path.

## 2. Runtime Ownership

- `Source/T66/Core/T66TrapSubsystem.h`
- `Source/T66/Core/T66TrapSubsystem.cpp`

`UT66TrapSubsystem` owns:

- trap registration
- cleanup of subsystem-managed trap actors
- trap progression scalar refresh
- level-driven tower trap selection
- procedural tower trap spawns

The live spawn hook remains in:

- `Source/T66/Gameplay/T66GameMode.cpp`
  - end of `AT66GameMode::SpawnWorldInteractablesForStage()`

Trap spawning still happens after tower floor layout, interactables, NPC safe zones, and circus placements exist so runtime placement can reject bad locations.

## 3. Trap Base Contract

- `Source/T66/Gameplay/Traps/T66TrapBase.h`
- `Source/T66/Gameplay/Traps/T66TrapBase.cpp`

Shared base guarantees:

- `TrapTypeID`
- `TrapFamilyID`
- `bTrapEnabled`
- `TowerFloorNumber`
- subsystem registration and unregistration
- progression damage and speed scaling
- `IsHeroTargetable(...)` and `IsEnemyTargetable(...)`
- trigger-target filtering

All new gameplay traps should derive from `AT66TrapBase` instead of re-implementing floor or safe-zone checks ad hoc.

## 4. Trap Families

### 4.1 Wall Projectile

- `Source/T66/Gameplay/Traps/T66WallArrowTrap.h`
- `Source/T66/Gameplay/Traps/T66WallArrowTrap.cpp`
- `Source/T66/Gameplay/Traps/T66TrapArrowProjectile.h`
- `Source/T66/Gameplay/Traps/T66TrapArrowProjectile.cpp`

Behavior:

- resolves the nearest valid hero in range
- performs a short windup
- fires a hostile wall projectile toward the hero
- reschedules on a fixed loop

Visual/runtime rules:

- trap housing now renders as a dedicated wall-mounted body instead of a naked proxy mesh
- projectile and trap visuals prefer `/Game/Stylized_VFX_StPack/Meshes/SM_Arrows_PickUp.SM_Arrows_PickUp`
- Arthur sword remains a fallback only if the stylized arrow mesh fails to load
- primitive cone geometry remains a final fallback only
- projectile trail still uses `UT66PixelVFXSubsystem`

### 4.2 Floor Burst

- `Source/T66/Gameplay/Traps/T66FloorFlameTrap.h`
- `Source/T66/Gameplay/Traps/T66FloorFlameTrap.cpp`

Behavior:

- warning phase
- active eruption phase
- repeated damage pulses while active
- cooldown loop before the next cycle

Visual/runtime rules:

- floor burst traps now render as vent-style ground traps instead of only a flat puck marker
- active flame VFX uses `/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Fire.P_Fire`
- active fire now spawns as a clustered set of Niagara systems for fuller flame volume
- warning and eruption support visuals still use `UT66PixelVFXSubsystem`
- damage uses an overlap sphere plus `UT66RunStateSubsystem::ApplyDamage()`

### 4.3 Area Control

- `Source/T66/Gameplay/Traps/T66FloorSpikePatchTrap.h`
- `Source/T66/Gameplay/Traps/T66FloorSpikePatchTrap.cpp`
- `Source/T66/Gameplay/Traps/T66TrapPressurePlate.h`
- `Source/T66/Gameplay/Traps/T66TrapPressurePlate.cpp`

Behavior:

- warning phase
- spike rise phase
- raised damage phase
- retract phase
- cooldown or pressure-plate reset depending on activation mode

Visual/runtime rules:

- spike geometry now prefers `/Game/Stylized_VFX_StPack/Meshes/SM_Spike.SM_Spike`
- `/Game/Stylized_VFX_StPack/Meshes/SM_Spikes.SM_Spikes` is the secondary fallback
- primitive cones remain a final fallback only
- spike eruptions now spawn `/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Dirt_Spikes_02.P_Dirt_Spikes_02`
- warning and burst support visuals still use `UT66PixelVFXSubsystem`

## 5. Procedural Tower Spawn Rules

Current procedural trap spawning is implemented only for tower gameplay floors.

Rules:

- start floor does not get traps
- boss floor does not get traps
- gameplay floors 1 through 5 each use their own level trap pool
- placement avoids:
  - other subsystem-managed trap locations
  - NPC safe zones
  - circus safe zones
- wall projectile traps use tower maze-wall spawn queries
- floor burst and area-control traps use tile-center spawn queries
- area-control traps can auto-spawn linked pressure plates

Current level trap pools:

- Gameplay Level 1:
  - `DungeonWallArrow`
  - `DungeonFloorFlame`
  - `DungeonFloorSpikePatch`
- Gameplay Level 2:
  - `ForestThornVolley`
  - `ForestSporeBurst`
  - `ForestBramblePatch`
- Gameplay Level 3:
  - `OceanHarpoonVolley`
  - `OceanSteamBurst`
  - `OceanUrchinPatch`
- Gameplay Level 4:
  - `MartianShardVolley`
  - `MartianPlasmaBurst`
  - `MartianCrystalPatch`
- Gameplay Level 5:
  - `HellSoulBoltVolley`
  - `HellEmberBurst`
  - `HellBrimstonePatch`

Numeric spawn counts and cadence live in `Config/DefaultT66TrapTuning.ini`.

## 6. Extension Rules

- New traps should derive from `AT66TrapBase`.
- Procedural trap spawning should go through `UT66TrapSubsystem`, not inline `AT66GameMode` logic.
- Trap damage should continue to route through `UT66RunStateSubsystem::ApplyDamage()` unless there is a very strong reason to bypass it.
- Floor-local tower traps should set `TowerFloorNumber`.
- New trap visuals must use already cooked assets or land their cook or staging contract in the same change.
- If a higher-quality imported mesh exists, do not downgrade the trap back to primitive-only visuals.

## 7. Current Open Next Steps

- authored trap placement support beyond the current procedural tower pass
- trap-specific audio and impact sound design
- trap minimap and warning affordance work
- trap disable, destroy, or interact mechanics
- richer per-theme VFX overrides beyond the shared family-level visuals
