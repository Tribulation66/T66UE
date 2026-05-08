# T66TD Memory and Progression

Last updated: 2026-04-21

## 1. Purpose

This file is the rolling implementation memory for Chadpocalypse TD.

## 2. Locked Decisions

- TD prefix: `T66TD`
- TD lives in its own runtime/module/data/source-asset/docs tree
- TD takes the second active slot on the Minigames page
- TD launch scope uses `Regular` only
- TD launch structure is `5 difficulties x 4 maps = 20 total maps`
- TD frontend shell mirrors the Mini Chadpocalypse shell format

## 3. Completed So Far

### 2026-04-21

- Added the `T66TD` runtime module to the project.
- Added TD-owned data contracts and the first TD data subsystem.
- Added TD-owned frontend state for difficulty/map selection.
- Added `T66TDMainMenuScreen` and `T66TDDifficultySelectScreen`.
- Registered TD screen types in the shared frontend routing path.
- Replaced the second Minigames-page placeholder slice with `Chadpocalypse TD`.
- Added TD-owned CSV authoring inputs for the 16-hero roster, 5 difficulties, and 20 launch maps.
- Added TD-owned docs and source-asset folder roots so future TD work can stay isolated from Mini Chadpocalypse.

## 4. Next Steps

- Generate and import the first-pass TD map background set.
- Add TD save/profile/runtime subsystems once gameplay flow starts.
- Build the first battle shell: map load, path visualization, tower-pad placement, and wave bootstrap.
