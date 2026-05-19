# Gameplay Agents

## Owns

Gameplay runtime systems and their documentation: combat, stats, movement, camera, traps, audio, world/tower, and minigames.

## Trigger Words

Combat, boss, enemy, stats, XP, movement, dash, camera, trap, audio, tower, world, map, stage, minigame, Mini, TD, Deck, Idle.

## Read First

- `Gameplay/README.md`
- Then the owning subfolder router if it exists.
- If no subfolder router exists, read the relevant `MASTER_*` or implementation reference file before editing source.

## Hard Rules

- Prefer data-authored tuning over hardcoded C++ defaults.
- Preserve minigame isolation boundaries.
- Runtime-facing gameplay changes need compile/build verification and staged standalone validation when they affect the playable standalone.
