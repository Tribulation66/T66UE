# T66Mini Master Implementation

Last updated: 2026-04-13

## 1. Purpose

This document is the implementation source of truth for Mini Chadpocalypse.

Mini Chadpocalypse is a Brotato-style 2D mini-game inside T66 with:

- Chadpocalypse heroes, enemies, bosses, NPCs, idols, and items
- mini-specific UI, save slots, gameplay systems, data tables, VFX, and runtime flow
- no backend work in the first build
- no playable multiplayer in the first build
- launch from the existing Minigames panel through a dedicated `Mini Chadpocalypse` entry

The primary engineering goal is isolation. Mini-game work must be able to evolve without contaminating the regular game codepath.

## 2. Locked Product Decisions

- Prefix all mini-game runtime classes/files with `T66Mini`.
- Use a dedicated `T66Mini` code module and mini-specific folders for content and source assets.
- Duplicate data tables into mini-specific tables even when values initially match the regular game.
- Duplicate UI widgets/panels into mini-specific classes even when layout initially matches the regular game.
- Idols replace Brotato weapons.
- Shop sells items and supports rerolls/locks. Idols are not sold in the shop.
- Idol selection is its own separate screen using mini-specific logic and data.
- Movement is Brotato-style mouse-follow with auto-fire.
- Presentation target is Brotato-like chibi readability, but with mini chad silhouettes rather than potato bodies.
- First build ships with all 16 heroes unlocked.
- First build supports true mid-wave resume using a mini-specific save/profile system.
- First build converts all current regular enemies and bosses into 2D mini variants.
- Mini leaderboard uses Steam leaderboards directly, not the Vercel backend.
- Mini leaderboard tracks score only, where score is the run's banked materials.
- Mini leaderboard supports all-time boards only.
- Mini leaderboard filters are global and friends only.
- Mini leaderboard does not ship speedrun, weekly, or streamers views.
- Mandatory first-build random interactables/NPCs:
  - Treasure Chest
  - Fountain
  - Loot Crate
  - Quick Revive NPC

## 3. Isolation Contract

### 3.1 Non-negotiable rule

Mini Chadpocalypse systems do not get implemented by extending the regular gameplay stack in-place.

That means:

- no mini-game combat logic inside regular idol classes
- no mini-game flow inside the regular `AT66GameMode`
- no mini-game save payloads inside the regular save classes
- no mini-game HUD logic inside the regular HUD widgets
- no mini-game shop logic inside the regular vendor/gambler/shop systems
- no mini-game wave, enemy, boss, or movement logic inside regular runtime actors unless explicitly designated as a tiny bridge point

### 3.2 Allowed shared touchpoints

The base T66 runtime is allowed to do only the following:

- expose a `Mini Chadpocalypse` button/bar in the existing Minigames screen
- register/open the mini-game entry screen or mini-game map
- provide generic shared engine-level helpers already meant to be shared across the whole project
- provide shared art/style helpers only when those helpers remain generic and contain no mini-specific branching

Everything else belongs to `T66Mini`.

### 3.3 Mini-specific ownership

The following must live only in mini-specific files/folders:

- menu flow
- hero select
- difficulty select
- idol select
- shop
- HUD
- save slots
- run resume
- combat
- projectiles
- enemy director
- enemy behavior
- boss behavior
- interactable spawning
- loot/material pickup logic
- 2D sprite presentation
- 2D VFX
- mini-game data tables

## 4. Proposed Project Layout

## 4.1 Code

Create a dedicated module:

```text
Source/T66Mini/
  T66Mini.Build.cs
  Public/
  Private/
    Core/
    Data/
    Save/
    Frontend/
    Gameplay/
    Gameplay/Actors/
    Gameplay/Components/
    Gameplay/Enemies/
    Gameplay/Bosses/
    Gameplay/Interactables/
    UI/
    UI/Screens/
    UI/Components/
    VFX/
    ArtPipeline/
```

Naming rule:

- Classes: `UT66Mini...`, `AT66Mini...`, `FT66Mini...`, `ET66Mini...`
- Files: `T66Mini...`
- Mini-only enums, structs, save payloads, and subsystems must also carry the `T66Mini` prefix

## 4.2 Content

```text
Content/Mini/
  Maps/
  Data/
  UI/
  UI/Icons/
  UI/Screens/
  Characters/Heroes/
  Characters/Enemies/
  Characters/Bosses/
  Characters/NPCs/
  Interactables/
  Projectiles/
  VFX/
  Materials/
  Audio/
```

## 4.3 Source Assets and Docs

```text
SourceAssets/Mini/
  Heroes/
  Enemies/
  Bosses/
  NPCs/
  Interactables/
  VFX/
  Sheets/

Docs/Mini/
  T66Mini_MasterImplementation.md
  T66Mini_Memory_Progression.md
```

## 5. High-Level Runtime Architecture

## 5.1 Frontend entry flow

Entry path:

1. Existing Minigames panel shows `Mini Chadpocalypse`.
2. Clicking it opens a dedicated mini full-screen shell.
3. Mini shell shows:
   - left: mini-specific friends panel
   - center: `New Game` and `Load Game`
   - right: mini-specific leaderboard panel
4. `New Game` enters:
   - hero selection
   - difficulty selection
   - pre-wave idol selection
   - battle
5. `Load Game` opens the mini-specific run/profile slot flow and resumes a mini run.

Recommended core screens:

- `UT66MiniEntryScreen`
- `UT66MiniMainMenuScreen`
- `UT66MiniSaveSlotsScreen`
- `UT66MiniCharacterSelectScreen`
- `UT66MiniDifficultySelectScreen`
- `UT66MiniIdolSelectScreen`
- `UT66MiniShopScreen`
- `UT66MiniPauseScreen`
- `UT66MiniRunSummaryScreen`

## 5.2 Gameplay framework

Core runtime classes:

- `AT66MiniGameMode`
- `AT66MiniGameState`
- `AT66MiniPlayerController`
- `AT66MiniPlayerPawn`
- `UT66MiniRunStateSubsystem`
- `UT66MiniSaveSubsystem`
- `UT66MiniDataSubsystem`
- `UT66MiniSpawnSubsystem`
- `UT66MiniVFXSubsystem`

Rules:

- `AT66MiniGameMode` owns only mini gameplay flow.
- The regular `AT66GameMode` remains untouched except for any minimal launch bridge that is unavoidable.
- Mini runtime state should be serializable without consulting the main game runtime classes.

## 5.3 Battle loop

Per-wave loop for the first build:

1. Enter from mini idol selection.
2. Spawn into the battle map with selected hero, difficulty, idol loadout, and carried run state.
3. Mouse-follow movement and idol auto-fire start immediately.
4. Gain XP/materials during the wave.
5. Random mini interactables can appear temporarily.
6. Boss appears at the end of the wave.
7. On boss defeat, open mini shop.
8. Shop `Continue` opens mini idol selection.
9. Next wave starts.
10. Five waves complete one difficulty block.

Wave duration, spawn pacing, and exact boss timings are intentionally tunable after the first playable version.

## 5.4 Combat model

Mini combat is its own stack:

- `UT66MiniCombatComponent`
- `UT66MiniIdolComponent`
- `UT66MiniAutoFireComponent`
- `UT66MiniProjectileComponent`
- `UT66MiniDamageSystem`
- `UT66MiniPickupMagnetComponent`

Behavior rules:

- Idols are the primary attack system.
- Existing idol roster is copied into mini-specific data tables.
- Each idol category keeps its own attack behavior family:
  - pierce
  - bounce
  - aoe
  - dot
- Shop items modify mini stats only.
- No dependency on the regular game idol implementation for runtime behavior.

## 5.5 Hero model

Use the 16 existing heroes as the launch roster, but copy their values into mini-specific hero tables.

Mini hero stack:

- `FT66MiniHeroDefinition`
- `UT66MiniHeroRosterSubsystem`
- `AT66MiniHeroPreviewActor`
- `UT66MiniHeroStatResolver`

Principles:

- preserve each hero's Chadpocalypse identity
- rebalance for Brotato-like wave gameplay inside the mini tables only
- no direct runtime reads from the main `Heroes.csv` once mini tables exist

## 5.6 Enemies and bosses

Mini enemy stack:

- `AT66MiniEnemyBase`
- `AT66MiniBossBase`
- `UT66MiniEnemyDirector`
- `UT66MiniWaveDefinitionSubsystem`
- `UT66MiniStatusEffectComponent`

Use 2D mini variants of the regular game enemies and bosses.

All enemy/boss tuning is copied into mini-specific data.

## 5.7 Interactables and pickups

Required first-build interactables:

- `AT66MiniTreasureChest`
- `AT66MiniFountain`
- `AT66MiniLootCrate`
- `AT66MiniQuickReviveNPC`

Pickup/runtime objects:

- `AT66MiniMaterialPickup`
- `AT66MiniXPOrb`
- `AT66MiniHealPickup`
- `AT66MiniTemporaryInteractableSpawner`

These do not reuse the regular world interactable runtime classes except through generic helper utilities if absolutely necessary.

## 5.8 Shop and progression

Mini shop stack:

- `UT66MiniShopScreen`
- `UT66MiniShopSubsystem`
- `FT66MiniShopOffer`
- `FT66MiniLockedOfferState`

Required shop behavior:

- item offers only
- rerolls
- locks
- continue to idol selection

Explicitly excluded from the first build:

- stat-choice level-up interruptions during battle
- companions
- backend wiring
- live multiplayer gameplay

## 5.9 Save/profile architecture

Mini save system must be separate from the regular game.

Required save classes:

- `UT66MiniProfileSaveGame`
- `UT66MiniRunSaveGame`
- `UT66MiniSettingsSaveGame`

Required save concerns:

- separate mini save slots
- separate mini profile metadata
- mid-wave resume
- per-run RNG state
- shop lock/reroll state
- idol loadout and cooldown state
- hero state
- boss/enemy/interactable runtime state
- pending despawn timers
- pickups and materials on the ground
- wave timer and wave index

Recommended snapshot payloads:

- `FT66MiniRunSnapshot`
- `FT66MiniPlayerSnapshot`
- `FT66MiniEnemySnapshot`
- `FT66MiniBossSnapshot`
- `FT66MiniInteractableSnapshot`
- `FT66MiniProjectileSnapshot`
- `FT66MiniPickupSnapshot`
- `FT66MiniWaveSnapshot`

Autosave strategy:

- save on wave start
- save on shop entry
- save on idol selection commit
- rolling timed autosave during battle
- explicit save on pause/quit

## 5.10 Data ownership

Mini data tables live under `Content/Mini/Data/`.

Planned tables:

- `DT_T66MiniHeroes`
- `DT_T66MiniIdols`
- `DT_T66MiniItems`
- `DT_T66MiniEnemies`
- `DT_T66MiniBosses`
- `DT_T66MiniInteractables`
- `DT_T66MiniDifficulties`
- `DT_T66MiniWaves`
- `DT_T66MiniShops`

Data rules:

- mini tables may initially mirror regular game values
- mini tables are still their own source of truth
- regular tables are reference input only during authoring/migration
- future balance passes happen in mini tables, not the regular tables

## 6. Rendering and Visual Direction

## 6.1 Camera and presentation

Target look:

- Brotato-like top-down readability
- large-headed chibi mini chads
- fast, readable sprite silhouettes
- clean attack telegraphs
- bright pickup readability

Camera rules:

- fixed mini-specific battle camera
- tuned to Brotato-like presentation
- not shared with regular game cameras

## 6.2 Sprite strategy

All first-pass runtime characters are 2D sprite actors or billboard-driven actors.

Use mini-specific presentation classes:

- `UT66MiniSpritePresentationComponent`
- `UT66MiniShadowComponent`
- `UT66MiniHitFlashComponent`
- `UT66MiniDirectionResolver`

## 6.3 VFX strategy

Primary VFX stack should stay in Unreal, not Blender.

Use:

- sprite-sheet projectiles
- sprite-sheet muzzle flashes
- sprite-sheet hit sparks
- sprite-sheet explosions
- sprite-sheet status ticks
- mini-specific Niagara only as a 2D particle/sprite carrier when useful
- mini-specific materials for tint, additive glow, hit flash, spawn/despawn dissolve, and telegraphs

Blender is optional only for exceptional pre-rendered sheets, not the default pipeline.

Recommended runtime classes:

- `UT66MiniVFXSubsystem`
- `AT66MiniProjectileActor`
- `AT66MiniGroundTelegraphActor`
- `AT66MiniFlipbookVFXActor`

## 7. Art Production Pipeline

## 7.1 Generation workflow

Use the documented ChatGPT Chrome bridge workflow for mini-game sprite generation.

Generation rule:

- prompt for four related subjects per source image whenever practical
- no background
- consistent chibi scale
- consistent outline thickness
- consistent shadow language
- consistent lighting direction
- Brotato-like readability, but mini chad anatomy and faces

Batch families:

- heroes
- enemies
- bosses
- NPCs
- interactables
- VFX sheets

## 7.2 Post-processing workflow

Author or add mini-specific helper scripts to:

- crop each subject from the shared sheet
- normalize canvas size
- center pivot consistently
- export individual sprite PNGs
- build flipbook sheets where needed
- generate metadata manifests

Proposed script area:

```text
Tools/ChatGPTBridge/
Tools/Mini/
```

## 8. Integration Rules with the Base Game

The regular T66 codebase may only do the minimum needed to expose and launch the mini-game.

Examples of acceptable base-game changes:

- add `Mini Chadpocalypse` entry to the Minigames screen
- register mini screen types or map launch helpers
- add build/module registration for `T66Mini`

Examples of forbidden leakage:

- putting mini idol attack code into `T66CombatComponent`
- putting mini save fields into `T66RunSaveGame`
- putting mini enemy spawning into `T66EnemyDirector`
- putting mini HUD sections into `T66GameplayHUDWidget`
- putting mini shop logic into regular vendor/gambler screens

## 9. First Implementation Milestones

### Milestone A: Baseline structure

- create `T66Mini` module
- create `Content/Mini/` roots
- create mini docs
- wire the minigames entry to the mini shell

### Milestone B: Frontend shell

- mini main menu
- mini leaderboard/friends panels
- mini save slots
- mini hero select
- mini difficulty select
- mini idol select

### Milestone C: First playable loop

- mouse-follow pawn
- idol auto-fire
- wave director
- one battle map
- pickups
- boss end gate
- shop
- continue into next wave

### Milestone D: Resume and content expansion

- true mid-wave save/load
- full enemy roster conversion
- full boss roster conversion
- interactable set
- VFX polish
- hero balance pass

## 10. Current Open Engineering Notes

- Because true mid-wave resume is required, battle state serialization should be designed before gameplay implementation hardens.
- Because mini data will diverge from the main game, all gameplay tuning must read from mini tables from the start.
- Because art volume is high, the asset pipeline must be scripted early instead of relying on manual crop/export work.
