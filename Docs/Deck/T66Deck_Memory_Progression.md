# T66Deck Memory And Progression

## Current State

The Deck module now has local run-save scaffolding only. Saves are intentionally mode-specific and use `T66Deck_RunSlot_%d` slot names through `UT66DeckSaveSubsystem`.

## Run Memory

`UT66DeckRunSaveGame` is the first-pass run memory container:

- Selected identity: hero, difficulty, starting deck.
- Descent position: act, floor, current node.
- Player resources: health, max health, gold.
- Build state: draw pile card IDs and relic IDs.
- Route state: generated run map nodes.
- Maintenance: save slot index and UTC update timestamp.

## Progression Direction

Longer-term progression should be split from active run saves:

- Run save: resumable in-progress dungeon descent.
- Profile save: unlocks, discovered cards/relics, ascension-like difficulty state, stats, and tutorial flags.
- Summary telemetry: completed run result, deaths, wins, boss kills, floor reached, deck composition, and reward history.

## Backend Boundary

There is no backend-authoritative Deck progression in this pass. Backend or Steam integration should be treated as deferred architecture work and should not be inferred from the local save skeleton.
