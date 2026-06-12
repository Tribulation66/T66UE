# T66 Master Traps

**Last updated:** 2026-06-07
**Scope:** Single-source handoff for environmental trap runtime, ownership, procedural spawning, obstacle reactions, damage routing, trap-family identity, and shipped trap VFX/mesh rules.
**Companion docs:** `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md`, `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md`, `Gameplay/World/T66_Tower_Multi_Agent_Implementation_Plan.md`
**Maintenance rule:** Update this file after every material trap, trap-subsystem, trap-spawn, trap-damage, trap-VFX, or trap-map-integration change.

## 1. Executive Summary

- Runtime trap ownership lives in `UT66TrapSubsystem`, a world subsystem.
- Trap actors derive from `AT66TrapBase` and auto-register with the subsystem on `BeginPlay`.
- The currently shipped tower trap family for normal tower floors is now:
  - `Obstacle`
- The older damage trap families remain in source as legacy/support classes:
  - `WallProjectile`
  - `FloorBurst`
  - `AreaControl`
- The current concrete tower obstacle trap actors are:
  - `AT66SweeperArmTrap`
  - `AT66BumperTrap`
  - `AT66WallBumperTrap`
  - `AT66CeilingHammerTrap`
- The legacy concrete damage trap actors are:
  - `AT66WallArrowTrap`
  - `AT66FloorFlameTrap`
  - `AT66FloorSpikePatchTrap`
- Tower gameplay floors now use floor-driven obstacle trap pools instead of the original one-arrow/one-flame-only pass. Floor-to-pool eligibility lives in `Config/DefaultT66TowerTuning.ini`; trap counts and mechanics remain in `Config/DefaultT66TrapTuning.ini`.
- Trap damage still routes through `UT66RunStateSubsystem::ApplyDamage()`, so safe-zone checks, invulnerability, floating feedback, and player-death flow stay on the normal runtime path.
- New obstacle traps do not deal HP damage directly by default. They call `UT66HeroPhysicsComponent::ApplyPhysicsReaction()` so the hero enters the hit-triggered ragdoll/disabled state, where enemy damage can still land through the query-only ragdoll capsule hurtbox.
- The wipeout-arm active-ragdoll obstacle remains as TestRoom proof scaffolding in `T66GameMode_TestRoom.cpp`, but the production tower sweeper is now `AT66SweeperArmTrap` and is registered with `UT66TrapSubsystem`.

## 2. Runtime Ownership

- `Source/T66/Core/T66TrapSubsystem.h`
- `Source/T66/Core/T66TrapSubsystem.cpp`

`UT66TrapSubsystem` owns:

- trap registration
- cleanup of subsystem-managed trap actors
- trap progression scalar refresh
- floor-driven tower trap selection
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

### 4.0 Obstacle

- `Source/T66/Gameplay/Traps/T66ObstacleTrap.h`
- `Source/T66/Gameplay/Traps/T66ObstacleTrap.cpp`

Behavior:

- obstacle traps overlap the hero and request a physics reaction through `UT66HeroPhysicsComponent::ApplyPhysicsReaction()`
- obstacle traps default to `bDamagesHeroes=false` and `bDamagesEnemies=false`
- the current tower obstacle set is:
  - `ObstacleSweeperArm`
  - `ObstacleFloorBumper`
  - `ObstacleWallBumper`
  - `ObstacleCeilingHammer`
- obstacle launch strength, spawn counts, spacing, footprint, and size live in `Config/DefaultT66TrapTuning.ini`
- tower spawning uses `T66TowerMapTerrain::TryGetObstacleTrapSpawnLocation(...)` so obstacle footprint adds edge, wall, and hole clearance before placement
- wall-mounted obstacle traps use `T66TowerMapTerrain::TryGetMazeWallSpawnLocation(...)` so they face out from valid trap-eligible maze walls

Visual/runtime rules:

- obstacle trap visuals now prefer the InflatableTraps01 balloon kit
  (`/Game/World/Traps/Inflatable/SM_Inflatable_*` + pattern `MI_Inflatable_*`
  instances of `M_FriendSlop_FallGuys`): striped balloon sweeper arm on a scalloped
  hub, banded donut floor bumper, quilted chevron punch cushion for the wall bumper,
  and a starred two-lobe balloon mallet on a segmented tube cable; engine/basic shape
  meshes with flat colors remain the fallback when kit assets are missing
- kit meshes keep the basic-shape 100uu native envelope so the dimension-driven
  component scale knobs in `DefaultT66TrapTuning.ini` keep working unchanged; asset
  authoring lives in `Model Generation/Runs/Environment/InflatableTraps01/`
  (Blender lathe shapes + Codex imagegen pattern textures), and every loaded path is
  registered in `T66CodeReferencedAssets.cpp`
- every concrete obstacle trap now has authored motion: sweeper rotates, floor bumper rises/falls, wall bumper extends/retracts, and ceiling hammer swings
- the primary gameplay effect is the ragdoll/disabled state, not direct trap HP damage
- floors `2` and `3` are the only normal tower floors that spawn these traps in the current parity tuning

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
- projectile damage now uses an oriented box component named `DamageBox`, not the older small sphere, so the combat debug view matches the long arrow/harpoon body more closely
- combat debug shows the projectile visual mesh/trail and its separate damage box container together
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
- combat debug draws the floor-burst damage sphere even while dormant, labeled as a container until warning or active damage begins

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
- combat debug draws the area-control damage sphere even while dormant, labeled as a container until warning or raised damage begins
- pressure plates are non-damage triggers, but combat debug draws their trigger box as `Trap Trigger Zone` so the trap activation container is visible separately from the plate mesh

## 5. Procedural Tower Spawn Rules

Current procedural obstacle spawning is implemented only for tower gameplay floors.

Rules:

- start floor does not get traps
- boss floor does not get traps
- normal tower gameplay floors `Floor 2` and `Floor 3` use the obstacle trap pool
- `Floor 4` is explicitly excluded for this obstacle pass
- placement avoids:
  - other subsystem-managed trap locations
  - NPC safe zones
  - circus safe zones
- obstacle traps use tower obstacle spawn queries with footprint-derived edge, wall, and hole clearance

Current normal tower floor trap pool:

- Floor 2:
  - `ObstacleSweeperArm`
  - `ObstacleFloorBumper`
  - `ObstacleWallBumper`
  - `ObstacleCeilingHammer`
- Floor 3:
  - `ObstacleSweeperArm`
  - `ObstacleFloorBumper`
  - `ObstacleWallBumper`
  - `ObstacleCeilingHammer`

Floor trap pools live in `Config/DefaultT66TowerTuning.ini`. Numeric spawn counts and cadence live in `Config/DefaultT66TrapTuning.ini`.

## 6. Extension Rules

- New traps should derive from `AT66TrapBase`.
- Procedural trap spawning should go through `UT66TrapSubsystem`, not inline `AT66GameMode` logic.
- Legacy trap damage should continue to route through `UT66RunStateSubsystem::ApplyDamage()` unless there is a very strong reason to bypass it.
- Obstacle traps should route hero impact through `UT66HeroPhysicsComponent::ApplyPhysicsReaction()` and leave direct HP damage off unless a future product pass explicitly opts a specific obstacle into damage.
- Floor-local tower traps should set `TowerFloorNumber`.
- New trap visuals must use already cooked assets or land their cook or staging contract in the same change.
- If a higher-quality imported mesh exists, do not downgrade the trap back to primitive-only visuals.

### 6.1 TestRoom wipeout-arm prototype

- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp` owns the current center-pivot wipeout-arm active-ragdoll obstacle.
- It is spawned after TestRoom room cleanup so it survives `SpawnRoom()` actor teardown.
- Runtime CVars:
  - `t66.TestRoom.EnableWipeoutArmTrap`
  - `t66.TestRoom.WipeoutArmUseHeroActiveRagdoll`
  - `t66.TestRoom.WipeoutArmLaunchXY`
  - `t66.TestRoom.WipeoutArmLaunchZ`
  - `t66.TestRoom.WipeoutArmIncapSeconds`
- The active-ragdoll proof uses the current selected Hero 1 physics-first visual path. Do not reintroduce the retired AnimatedToonStyle TestRoom override as the Stage 3 proof target.
- Current Stage 3 Hero 1 proof routing prefers `UT66HeroPhysicsComponent`: the arm is a moving `WorldDynamic` physics-body blocker, the geometric hit trigger calls the active-ragdoll reaction profile, and the response is simulated-body impulse plus bounded capsule shove.
- The accepted proof capture mode is `Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode heroactiveragdollproof`; the helper adds `-T66AutomationTestRoom` so this route proves the TestRoom obstacle instead of the regular tower layout.
- Legacy `UT66KnockbackComponent` launch/ragdoll remains fallback only when active ragdoll is unavailable. This path is not the target Hero 1 feel.
- This path still does not route through `AT66TrapBase`, progression tuning, `UT66TrapSubsystem`, or production trap damage rules.

## 7. Current Open Next Steps

- authored trap placement support beyond the current procedural tower pass
- trap-specific audio and impact sound design
- trap minimap and warning affordance work
- trap disable, destroy, or interact mechanics
- richer per-theme VFX overrides beyond the shared family-level visuals
