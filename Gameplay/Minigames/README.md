# T66 Minigames Docs

This folder contains shared and mode-owned process docs for isolated T66 minigame modes.

## Shared

- [MINIGAME_CHARACTER_ANIMATION_INSTRUCTIONS.md](MINIGAME_CHARACTER_ANIMATION_INSTRUCTIONS.md): shared sprite/atlas process for Mini, TD, Deck, and Idle.

## Modes

- [Mini](Mini): Mini Chadpocalypse implementation, ROTMG-style pixel runtime pass, multiplayer checklist, sprite manifests, walksheets, and UI mirror notes.
- [TD](TD): Chadpocalypse TD implementation and progression memory.
- [Deck](Deck): T66Deck implementation and progression memory.
- [Idle](Idle): T66Idle implementation and progression memory.

## Closed-Loop Runtime Contract

Minigames tab buttons open each mode's main menu first. Gameplay starts from the mode-owned Start/New Game/Enter action inside that main menu.

The first closed-loop scope is intentionally small and data-driven:

- Mini: 10 stages per difficulty, stage 10 is the only boss stage, and completion routes to the Mini run summary.
- TD: 10 waves per selected map/stage, wave 10 is the boss wave, and the battle board shows victory/defeat stats.
- Deck: one 10-floor descent, floor 10 is the boss encounter, and the deck screen shows a run summary.
- Idle: 10 enemies/stages, stage 10 is the boss, and the idle screen shows a run summary.

Keep mode-specific source, data, and loose runtime art in that mode's own roots.
