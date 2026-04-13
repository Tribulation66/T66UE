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

## 4. Next Planned Steps

- Create the `T66Mini` module and mini-specific folder roots.
- Add the mini-game launch entry to the Minigames screen.
- Build the mini main menu shell with mini-specific leaderboard/friends clones.
- Create mini-specific save/profile classes and slot flow.
- Create mini-specific data assets and initial migration path from the regular data tables.
- Build the first playable battle loop with mouse-follow movement and idol auto-fire.
- Stand up the Chrome-bridge-driven sprite workflow for heroes, enemies, bosses, NPCs, and interactables.

## 5. Current Risks

- Mid-wave resume adds serialization scope early; if deferred too long, retrofit cost will spike.
- Sprite volume is high enough that the crop/export pipeline must be automated.
- If mini screens quietly depend on regular screen classes, isolation will erode quickly and future divergence will become expensive.

## 6. Rules for Future Updates

- Log what was completed, not just what changed.
- Record why a decision changed when a previous rule is replaced.
- Keep this file current enough that another agent can resume work without reconstructing the entire mini-game history from git or chat context.
