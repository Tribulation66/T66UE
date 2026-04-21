# T66TD Master Implementation

Last updated: 2026-04-21

## 1. Purpose

This document is the implementation source of truth for Chadpocalypse TD.

Chadpocalypse TD is a second standalone minigame inside T66 with:

- a dedicated `T66TD` runtime module
- TD-specific frontend screens, data files, source assets, and docs
- no reuse-in-place of `T66Mini` runtime files
- the same Chadpocalypse hero roster and world themes, but translated into a tower-defense ruleset
- one launch mode only: `Regular`

## 2. Locked Product Decisions

- Prefix all TD runtime classes and files with `T66TD`.
- Keep TD files isolated under `Source/T66TD`, `Content/TD`, `SourceAssets/TD`, and `Docs/TD`.
- Chadpocalypse TD owns the second live slot on the Minigames page.
- The frontend title is `Chadpocalypse TD`.
- The TD main menu mirrors the overall shell format of Mini Chadpocalypse rather than inventing a new one-off layout.
- TD backgrounds must be authored as TD-owned assets rather than reusing Mini or main-menu art in place.
- Launch mode is `Regular` only.
- `Regular` contains 5 difficulties with 4 maps each for 20 total maps.
- Difficulty-to-theme mapping follows the tower progression already locked in T66:
  - `Easy -> Dungeon`
  - `Medium -> Forest`
  - `Hard -> Ocean`
  - `Very Hard -> Martian`
  - `Impossible -> Hell`

## 3. Isolation Contract

- Do not implement TD features by extending `T66Mini` files in place.
- Do not put TD data inside `Content/Mini/Data/`.
- Do not point TD source-asset paths at `SourceAssets/Mini/`.
- Shared frontend routing in `T66` may register TD screens, but the TD screens themselves live in `T66TD`.

## 4. Current Frontend Scope

Current implemented scope in this pass:

- `T66TD` runtime module registered in the project
- TD-specific data subsystem and frontend-state subsystem
- TD main menu screen
- TD difficulty and map browser screen
- Minigames page second slot wired to Chadpocalypse TD
- TD-owned CSV authoring inputs for heroes, difficulties, and maps
- TD-owned source-asset directories for map backgrounds

Deferred to later passes:

- battle runtime
- save/load flow
- hero placement, upgrades, enemy routing, and combat
- run summary and leaderboard flow
