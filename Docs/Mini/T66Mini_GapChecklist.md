# T66Mini Gap Checklist

Last updated: 2026-04-13

## 1. Purpose

This file tracks the gap between the target defined in `Docs/Mini/T66Mini_MasterImplementation.md` and the current in-repo `T66Mini` implementation.

Status key:

- `Done` = target behavior exists in a usable first-build form

## 2. Summary

- Core verdict: the mini-game now meets the first-build target described in the master implementation doc.
- Current practical state: launch flow, dedicated mini battle map, real combat HUD, shop/progression loop, pause/save flow, run summary flow, full mini data ownership, launch-scope enemy/boss coverage, and first-pass VFX/audio readability are now in place in standalone.
- Remaining work after this checklist is optional polish, not a blocker to the completed first-build target.

## 3. Checklist

## 3.1 Isolation and Module Ownership

- `Done` Dedicated `T66Mini` module exists.
- `Done` Mini-specific frontend, save, run-state, data, gameplay, UI, and VFX systems live in `T66Mini`.
- `Done` Base-game touchpoints remain narrow: launch entry, screen routing, and runtime bridge behavior.
- `Done` Mini runtime ownership is self-contained enough that the mode no longer reads like borrowed scaffolding.

## 3.2 Frontend Entry Flow

- `Done` Existing Minigames flow launches `Mini Chadpocalypse`.
- `Done` Mini shell exists with `New Game` and `Load Game`.
- `Done` Character select, difficulty select, idol select, and save-slot flow exist.
- `Done` Main mini back button returns to the full game main menu.
- `Done` Pause menu exists with `Resume` and `Back To Main Menu`, bound to `Esc`.
- `Done` Main menu run-intel panel is functional.
- `Done` Main menu local-ladder panel is functional.
- `Done` Run summary screen exists with replay and exit routing.

## 3.3 Battle Loop

- `Done` Entering battle from idol selection works.
- `Done` Mouse-follow movement works.
- `Done` Idol auto-fire works.
- `Done` XP/material pickup loop works.
- `Done` Boss appears and gates shop progression.
- `Done` Shop continues back into idol selection and the next wave.
- `Done` Five-wave block structure exists in current battle flow.
- `Done` Battle now boots on a dedicated mini battle map instead of the borrowed regular gameplay level path.

## 3.4 Player, Combat, and Runtime Systems

- `Done` `AT66MiniGameMode`, `AT66MiniGameState`, `AT66MiniPlayerController`, and `AT66MiniPlayerPawn` exist and are live.
- `Done` Projectile-driven idol attack families exist for pierce, bounce, aoe, and dot categories.
- `Done` First-build combat helpers are now split into mini-specific presentation, hit-flash, shadow, direction, pickup-magnet, and VFX systems instead of remaining a flat raw billboards-only slice.
- `Done` First-pass item stat application exists for the launch-scope mini item pool.
- `Done` Life steal is hooked into hit resolution.
- `Done` Combat readability now includes telegraphs, hit pulses, pickup bursts, sprite facing, shadows, and improved battlefield feedback.

## 3.5 Enemies, Bosses, Interactables, and Pickups

- `Done` Playable enemy spawning exists for the full regular enemy roster represented by the current game tables.
- `Done` Boss spawning exists and now reads from the full mini boss table.
- `Done` Required first-build interactables exist:
  - Treasure Chest
  - Fountain
  - Loot Crate
  - Quick Revive NPC
- `Done` Pickups for XP and materials exist.
- `Done` Launch-scope enemy identity behaviors are represented by active mini behavior profiles.
- `Done` Full regular boss roster coverage is represented in mini data for the launch flow.
- `Done` Pickup/interactable readability now has first-pass clarity support through shadows, pulses, hover motion, indicator meshes, and pickup magnet behavior.

## 3.6 Shop and Progression

- `Done` Mini shop screen exists.
- `Done` Shop offers items only.
- `Done` Rerolls exist.
- `Done` Locks exist.
- `Done` Selling exists.
- `Done` Continue-to-idol-selection flow exists.
- `Done` Launch-scope stat coverage and progression depth are in place for the current mini item table.
- `Done` Battle level-up interruptions remain excluded, matching the first-build scope.

## 3.7 Save and Resume

- `Done` Mini save system is separate from the regular save system.
- `Done` Mini save slots exist.
- `Done` Mid-wave resume exists.
- `Done` Run state restores player state, enemy state, pickup state, interactable state, and wave shell state.
- `Done` Save/load supports shop intermission return.
- `Done` Timed battle autosave exists.
- `Done` Save on wave start exists.
- `Done` Save on shop entry exists.
- `Done` Save on idol selection commit exists.
- `Done` Explicit save-on-pause/quit behavior now exists.
- `Done` Save architecture holds up against the current launch-scope content systems.

## 3.8 Data Ownership

- `Done` Mini CSV data authoring roots exist under `Content/Mini/Data/`.
- `Done` Heroes, idols, items, enemies, bosses, interactables, difficulties, and waves now have mini-owned data inputs.
- `Done` Runtime prefers cooked `/Game/Mini` sprite assets and can fall back to loose source assets when needed.
- `Done` Difficulties are mini-owned table data instead of hardcoded bootstrap values.
- `Done` Enemy/interactable/wave data ownership is now table-driven for the first-build runtime.
- `Done` Current tuning reads from mini-owned tables as the runtime source of truth.

## 3.9 Rendering, HUD, and Visual Direction

- `Done` Hero, enemy, boss, NPC, interactable, pickup, and projectile sprites render in standalone.
- `Done` Arena background/floor rendering exists instead of the previous all-black battle presentation.
- `Done` Mini battle camera is functional and tuned for the current survivor layout.
- `Done` Brotato-style combat HUD exists in a real gameplay form rather than a debug text overlay.
- `Done` Arena presentation now has a dedicated mini map path plus first-pass scene treatment.
- `Done` Shadows, hit pulses, direction resolution, boss telegraphs, and clearer pickup/combat feedback are now present in the runtime.
- `Done` Bright pickup readability and cleaner attack/boss telegraphs now meet the first-build baseline.

## 3.10 VFX and Audio

- `Done` Mini-specific VFX runtime now exists through the mini VFX subsystem and dedicated mini VFX actors.
- `Done` Ground telegraphs, impact pulses, projectile hit feedback, explosions, pickup bursts, and boss spawn readability are present as a cohesive first-pass system.
- `Done` Mini-specific audio pass now exists at the first-build level through battle music plus mini combat/pickup/boss SFX routing.

## 3.11 Art Pipeline

- `Done` Sprite batch generation scripts exist.
- `Done` Hero sheet splitting and normalization workflow exists.
- `Done` Launch-scope hero, enemy, boss, NPC, and interactable sprite sets were generated and imported.
- `Done` Runtime fallback staging for source sprites remains in place as a safety net for standalone builds.
- `Done` Current first-build art pipeline supports the implemented launch-scope content set.

## 4. Optional Polish After Checklist Completion

These are no longer checklist blockers for the first-build target:

- Replace shared boss/enemy placeholder visual reuse with more bespoke per-ID mini art over time.
- Add a broader mini-only audio library instead of reusing the current first-pass SFX source.
- Expand the VFX layer from clean pulse/telegraph readability into richer authored flipbook sheets.
- Continue art-pipeline automation for higher-volume replacement passes as the mini roster grows.

## 5. Definition of "First Build Complete"

The first-build completion definition is now satisfied:

- Mini flow from launch through five-wave completion feels self-contained.
- Battle runs on a dedicated mini map with a real mini HUD.
- Launch-scope enemy and boss coverage is represented in mini runtime data.
- Required interactables, shop loop, and save/resume behavior hold together in standalone.
- Presentation reaches a clean, readable survivor-game baseline with visible telegraphs, pickups, and combat feedback.
