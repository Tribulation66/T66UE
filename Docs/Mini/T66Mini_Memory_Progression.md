# T66Mini Memory and Progression

Last updated: 2026-04-13

## 1. Purpose

This file is the rolling implementation memory for Mini Chadpocalypse.

Update it whenever:

- architecture changes
- a milestone lands
- a blocker is found
- a content pipeline decision changes
- a feature is completed or intentionally deferred

## 2. Locked Decisions

- Mini-game prefix: `T66Mini`
- Isolation strategy: dedicated mini-specific module, folders, data, saves, UI, gameplay systems, and VFX
- Mini tables are separate from regular tables even when seeded from the same source values
- Idols replace the weapon system
- Shop has items only, not idols
- Idol selection remains a separate screen with rerolls and four slots
- Mouse-follow movement should match Brotato-style feel
- Visual target is Brotato-like chibi readability with mini chad faces and proportions
- First build ships without backend work
- First build ships without live multiplayer gameplay
- First build uses all 16 existing heroes as the launch roster
- First build supports true mid-wave resume
- First build includes Treasure Chest, Fountain, Loot Crate, and Quick Revive NPC as random interactables

## 3. Completed So Far

### 2026-04-13

- Defined the high-level product direction for Mini Chadpocalypse.
- Locked the isolation policy: mini systems do not go into regular game runtime files.
- Chose mini-specific data duplication for heroes, idols, items, enemies, bosses, and difficulties.
- Chose mini-specific duplication for UI panels and screens even where layouts initially match the regular game.
- Defined the intended implementation structure in `Docs/Mini/T66Mini_MasterImplementation.md`.
- Added the master-guideline policy that Mini Chadpocalypse must remain file-isolated from the regular game except for narrow launch/registration touchpoints.
- Prepared this checkpoint as the `v2.0` architecture baseline before implementation starts.
- Added the `T66Mini` runtime module to the project and created the first mini-specific UI classes.
- Replaced the Minigames screen's Snake button with a `Mini Chadpocalypse` launch path that navigates into the `T66Mini` shell.
- Built the first compiled mini-specific main menu shell with isolated left/right placeholder panels and center `New Game` / `Load Game` actions.
- Added `Tools/Mini/T66MiniSpriteBatch.py` and `Docs/Mini/T66Mini_SpriteBatchManifest.json` for bridge-driven mini art generation.
- Successfully generated four 2x2 hero source sheets covering all 16 launch heroes.
- Added `Tools/Mini/T66MiniSplitHeroSheets.py` and split those four sheets into 16 individual hero source sprites under `SourceAssets/Mini/Heroes/Singles/`.
- Duplicated the first mini-specific CSV authoring inputs under `Content/Mini/Data/` for heroes, idols, items, and bosses.
- Verified the current mini scaffolding with a successful `T66Editor Win64 Development` build.
- Added mini-specific frontend state, mini data loading, mini save slots, and mini run state subsystems inside `T66Mini`.
- Built the isolated mini flow screens for character selection, difficulty selection, idol selection, and mini-specific save slot loading.
- Split and normalized the first mini source asset sets beyond heroes:
  - enemy variants into `SourceAssets/Mini/Enemies/Variants/`
  - canonical enemy singles into `SourceAssets/Mini/Enemies/Singles/`
  - boss placeholders into `SourceAssets/Mini/Bosses/Singles/`
  - NPC singles into `SourceAssets/Mini/NPCs/Singles/`
  - interactable and loot singles into `SourceAssets/Mini/Interactables/Singles/`
- Started the isolated mini gameplay runtime stack with `T66MiniGameMode`, `T66MiniPlayerController`, `T66MiniPlayerPawn`, and `T66MiniGameState`.
- Added an isolated mini runtime launch subsystem that opens the existing gameplay map with the mini game mode override, so the mini mode can boot without touching the regular game mode path.
- Added the first mini battle autosave loop and a mini-specific HUD overlay that displays hero, difficulty, wave, timer, and active save slot.
- Wired the idol select and mini save slot screens to bootstrap or load a `T66Mini` run and launch the mini battle runtime instead of stopping at placeholder status text.
- Added the first playable combat slice inside `T66Mini`:
  - a mini arena spawned at an isolated location inside the borrowed gameplay level
  - a sprite-backed mini player pawn using the generated hero PNGs
  - sprite-backed enemy and boss actors using the generated enemy PNGs
  - idol-driven auto-fire projectiles for Pierce, Bounce, AOE, and DOT categories
  - loot pickup actors for materials and XP
  - random mini interactables for Treasure Chest, Fountain, Loot Crate, and Quick Revive Machine
  - automatic enemy spawning, boss spawning, wave progression, and frontend return on death or full 5-wave clear
- Extended mini save snapshots to persist player location, health, materials, XP, and level so mid-wave resume now restores more than just the timer shell.
- Added the first mini-specific intermission loop:
  - mini item definitions now load from `Content/Mini/Data/T66Mini_Items.csv`
  - mini run saves now track owned item IDs and a pending-shop-intermission state
  - boss clears now return to a dedicated `MiniShop` frontend screen instead of auto-advancing directly into the next wave
  - the mini shop supports buying items, locking offers, rerolling offers, and continuing into the next idol-selection screen
  - save/load now preserves whether the run should resume into battle or back into the mini shop intermission
- Added first-pass item stat application inside the mini player pawn so purchased items affect damage, attack speed, armor, range, crits, DOT power, regen, evade chance, and several other run stats on the next battle load.
- Expanded true mid-wave resume beyond player-only state:
  - mini run saves now capture per-wave transient state such as boss-spawned status, spawn accumulators, and post-boss delay
  - mini run saves now serialize active enemies, loot pickups, and random interactables
  - the mini game mode now restores those actors and controller fields on battle load instead of regenerating the wave shell
- Added visual-ID persistence for mini pickups and interactables so restored snapshots can reload the same loose PNG sprites they were using before the save.
- Hooked first-pass life steal into projectile hit resolution so item-driven life steal now actually heals the mini player during combat.
- Added run-end cleanup so completed or failed runs delete their active mini save slot and clear mini frontend/run state instead of leaving stale resumable saves behind.
- Added first-pass enemy identity behaviors for the current four mini archetypes:
  - `Roost` presses harder at range
  - `Cow` closes more steadily and hits harder
  - `Goat` performs periodic charge bursts
  - `Pig` enrages under half health
- Re-verified the entire `T66Mini` module with a successful `T66Editor Win64 Development` build after the snapshot and cleanup changes.
- Imported the generated mini hero, enemy, boss, NPC, and interactable PNGs into cooked Unreal assets under `/Game/Mini/Sprites/...`.
- Added `/Game/Mini` to the always-cook directories and kept the loose-file staging path as a runtime fallback for mini CSVs and source sprites.
- Updated the mini visual subsystem so runtime texture loads prefer cooked `/Game/Mini/Sprites/...` assets and only fall back to `SourceAssets/Mini/...` if needed.
- Hardened mini idol loading so `T66Mini_Idols.csv` falling over no longer leaves the selection screen empty; the data subsystem now falls back to built-in idol definitions.
- Hardened the idol selection screen so it forces a mini data reload and regenerates offers if the screen reaches render time with zero idol offers.
- Restaged the standalone build after the mini asset import and idol-offer fixes so the taskbar shortcut points at the refreshed packaged runtime.
- Fixed the mini main-menu back action so it resets mini frontend state and returns to the full game `MainMenu` instead of dead-ending in mini navigation history.
- Added cooked-or-loose mini background loading and applied it to the spawned battle arena floor so the mini battle no longer opens over an all-black field.
- Explicitly hid the persistent gameplay transition curtain when mini gameplay begins so the battle scene can render in the standalone runtime.
- Verified the taskbar-pinned standalone target path, rebuilt the `T66 Win64 Development` game target, and re-synced the staged standalone build after mini-runtime fixes so the packaged executable matches the current code.
- Fixed mini runtime sprite visibility by explicitly enabling all `UBillboardComponent`-based mini actor sprites in game, which restored hero, enemy, pickup, interactable, NPC, and projectile rendering in standalone.
- Added the first mini pause menu with `Resume` and `Back To Main Menu`, bound it to `Esc`, and wired it so pausing no longer traps the player in mini gameplay.
- Added the current gap tracker document as `Docs/Mini/T66Mini_GapChecklist.md` so implementation status can be checked against the master target without re-deriving the delta from code or chat history.
- Added a dedicated cooked mini battle map at `/Game/Mini/Maps/T66MiniBattleMap` and switched mini runtime level launch over to that path.
- Expanded mini-owned CSV data coverage to include difficulties, enemies, interactables, waves, and a corrected full boss-table span for the launch mini runtime.
- Replaced the previous temporary battle overlay with a real mini combat HUD showing hero card, timer/materials, combat stats, XP, idol loadout, and boss-state UI.
- Added a run-summary screen and routed completed/failed mini runs into that summary instead of dropping straight back into generic menu flow.
- Upgraded the mini main-menu side panels from placeholder copy into live local run-intel and ladder/profile summaries.
- Added shop item selling and improved shop progression flow with owned-item visibility.
- Hardened explicit save-on-pause and save-on-quit behavior so `Esc` pause and pause-exit preserve resumable mini runs.
- Added first-pass mini VFX and presentation systems:
  - dedicated `UT66MiniVFXSubsystem`
  - mini pulse/telegraph actors
  - sprite presentation, shadow, hit-flash, direction, and pickup-magnet components
  - boss telegraphs, hit pulses, pickup bursts, spawn pulses, and improved battlefield readability
- Added a first-pass mini audio pass beyond music through mini shot, hit, pickup, and boss alert/spawn SFX routing.
- Repackaged the standalone runtime with the dedicated mini map, updated mini data tables, HUD, VFX/audio layer, and staged output sync so the pinned `T66 Standalone` shortcut now points at the refreshed packaged build.
- Updated the gap checklist so it now reflects a completed first-build target rather than a pre-finish delta snapshot.

## 4. Optional Next Steps

- Replace shared placeholder boss/enemy visual reuse with more bespoke per-ID mini art over time.
- Broaden the mini-only audio library beyond the current first-pass SFX routing.
- Extend the current pulse/telegraph VFX layer into more authored flipbook-sheet effects as new art lands.
- Continue replacing the first imported texture pass with more polished final sprite sets over time.

## 5. Current Risks

- Boss and enemy runtime coverage is complete for the first build, but some IDs still intentionally reuse shared mini visuals until bespoke replacement art is generated.
- The first-pass mini audio/VFX layer is now functional, but continued polish will benefit from a broader dedicated asset library.
- The art pipeline now supports the implemented launch scope; future content growth still depends on keeping the sprite workflow automated.

## 6. Rules for Future Updates

- Log what was completed, not just what changed.
- Record why a decision changed when a previous rule is replaced.
- Keep this file current enough that another agent can resume work without reconstructing the entire mini-game history from git or chat context.
