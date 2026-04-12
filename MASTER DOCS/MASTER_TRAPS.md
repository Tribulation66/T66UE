# T66 Master Traps

**Last updated:** 2026-04-11  
**Scope:** Single-source handoff for environmental trap runtime, ownership, procedural spawning, damage routing, and the currently shipped trap families.  
**Companion docs:** `MASTER DOCS/T66_MASTER_GUIDELINES.md`, `MASTER DOCS/MASTER_MAP_DESIGN.md`, `MASTER DOCS/T66_PROJECT_CATALOGUE.md`  
**Maintenance rule:** Update this file after every material trap, trap-subsystem, trap-spawn, trap-damage, trap-VFX, or trap-map-integration change.

## 1. Executive Summary

- Runtime trap ownership now lives in `UT66TrapSubsystem`, a world subsystem.
- Trap actors derive from `AT66TrapBase` and auto-register with the subsystem on `BeginPlay`.
- The first shipped trap families are:
  - `WallArrow`
  - `FloorFlame`
- Tower gameplay floors currently procedurally spawn one wall-arrow trap and one floor-flame trap per gameplay floor.
- Trap damage routes through `UT66RunStateSubsystem::ApplyDamage()`, so safe-zone checks, invulnerability, floating feedback, and player-death flow stay on the normal runtime path.
- Wall-arrow visuals reuse the Arthur sword projectile asset.
- Floor-flame visuals use the existing fire VFX asset at `/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Fire.P_Fire`, with pixel VFX used for warning/burst support.

## 2. Runtime Ownership

- `Source/T66/Core/T66TrapSubsystem.h`
- `Source/T66/Core/T66TrapSubsystem.cpp`

`UT66TrapSubsystem` is the world-scoped owner for:

- trap registration
- cleanup of subsystem-managed trap actors
- procedural tower trap spawns

The current live spawn hook is in:

- `Source/T66/Gameplay/T66GameMode.cpp`
  - end of `AT66GameMode::SpawnWorldInteractablesForStage()`

That placement is intentional: trap spawning now happens after tower floor layout, interactables, NPC safe zones, and compact circus/utility placements exist, so trap placement can avoid those spaces.

## 3. Trap Base Contract

- `Source/T66/Gameplay/Traps/T66TrapBase.h`
- `Source/T66/Gameplay/Traps/T66TrapBase.cpp`

Shared base guarantees:

- `TrapTypeID`
- `bTrapEnabled`
- `TowerFloorNumber`
- subsystem registration/unregistration
- `IsHeroTargetable(...)` helper

`IsHeroTargetable(...)` currently enforces:

- valid hero actor
- trap enabled
- hero not inside a safe zone
- tower-floor match when the trap was spawned or authored for a specific floor

All new gameplay traps should build on this base instead of re-implementing safe-zone/floor filtering inside ad hoc actors.

## 4. Wall Arrow Trap

- `Source/T66/Gameplay/Traps/T66WallArrowTrap.h`
- `Source/T66/Gameplay/Traps/T66WallArrowTrap.cpp`
- `Source/T66/Gameplay/Traps/T66TrapArrowProjectile.h`
- `Source/T66/Gameplay/Traps/T66TrapArrowProjectile.cpp`

Current behavior:

- resolves the nearest valid hero in range
- performs a short windup
- fires a hostile projectile from the wall toward the hero
- reschedules itself on a fixed loop

Visual/runtime rules:

- projectile visual reuses `/Game/VFX/Projectiles/Hero1/Arthur_Sword.Arthur_Sword`
- projectile trail uses pixel particles
- projectile damage applies through `UT66RunStateSubsystem::ApplyDamage()`

Current first-pass tuning model:

- one trap per gameplay floor
- difficulty changes cadence and damage
- tower procedural spawns place the actor on shell walls and face it inward toward the playable space

## 5. Floor Flame Trap

- `Source/T66/Gameplay/Traps/T66FloorFlameTrap.h`
- `Source/T66/Gameplay/Traps/T66FloorFlameTrap.cpp`

Current behavior:

- warning ring phase
- active fire eruption phase
- repeated damage pulses while active
- cooldown loop before the next warning cycle

Visual/runtime rules:

- active flame VFX uses `/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Fire.P_Fire`
- warning and eruption support visuals use `UT66PixelVFXSubsystem`
- damage uses an overlap sphere plus `UT66RunStateSubsystem::ApplyDamage()`

Current first-pass tuning model:

- one trap per gameplay floor
- difficulty changes radius, damage, and cooldown cadence

## 6. Procedural Tower Spawn Rules

Current procedural trap pass is implemented only for tower gameplay floors.

Rules:

- start floor does not get traps
- boss floor does not get traps
- each gameplay floor gets:
  - `1` wall-arrow trap
  - `1` floor-flame trap
- placement avoids:
  - other subsystem-managed trap locations
  - NPC safe zones
  - circus safe zones
- wall-arrow traps use tower wall-spawn queries
- floor-flame traps use tower random-surface floor queries

This is intentionally a first playable pass, not the final authored density model.

## 7. Extension Rules

- New traps should derive from `AT66TrapBase`.
- Procedural trap spawning should go through `UT66TrapSubsystem`, not inline `AT66GameMode` logic.
- Trap damage should continue to route through `UT66RunStateSubsystem::ApplyDamage()` unless there is a very strong reason to bypass it.
- Trap actors that are floor-local in tower runs should set `TowerFloorNumber`.
- New trap visuals must use already cooked assets or land their cook/staging contract in the same change.

## 8. Current Open Next Steps

- data-driven trap counts/themes per map preset and difficulty
- authored trap placement support beyond the first procedural tower pass
- trap minimap and warning affordance work
- trap disable/destroy/interact mechanics
- additional trap families beyond wall arrows and floor flames
