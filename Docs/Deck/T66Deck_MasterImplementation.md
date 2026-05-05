# T66Deck Master Implementation

## Scope

T66Deck is the isolated runtime module for a Chadpocalypse deckbuilder mode. It follows the existing `T66Mini` and `T66TD` module pattern: module-owned source under `Source/T66Deck`, loose data under `Content/Deck/Data`, source assets under `SourceAssets/Deck`, and docs under `Docs/Deck`.

This first pass is infrastructure only. It does not register shared screens, edit `T66.uproject`, add gameplay combat, generate art, or claim backend support.

## Runtime Shape

- `UT66DeckDataSubsystem` owns deckbuilder definitions loaded for the active game instance. The current skeleton exposes card, relic, and encounter registries and leaves data ingestion empty until authored data files are introduced.
- `UT66DeckFrontendStateSubsystem` stores transient frontend choices for a new run: hero, difficulty, starting deck, and current map node.
- `UT66DeckRunSaveGame` stores local run state for dungeon descent: act, floor, current node, deck list, relic list, map nodes, health, gold, and timestamps.
- `UT66DeckSaveSubsystem` provides local slot operations and seeded run-save creation from frontend state.
- `UT66DeckMainMenuScreen` is a minimal Slate screen for later central routing. It intentionally uses `ET66ScreenType::None` until the parent integration adds shared enum/routing entries.

## Intended Game Loop

1. Pick a hero/difficulty/starting deck.
2. Generate an act map with branching node choices.
3. Resolve one node at a time: combat, elite, boss, rest, shop, event, or treasure.
4. Award cards, relics, gold, upgrades, removals, or route choices.
5. Descend through acts until the run wins, dies, or is abandoned.

## Deferred Work

- Data file schema and import path for cards, relics, encounters, enemies, and map generation rules.
- Central frontend routing and `ET66ScreenType` registration.
- Battle runtime, card execution, enemy AI, reward screens, map screen, shop/rest/event screens.
- Backend-authoritative progression, leaderboards, or anti-cheat validation. Any backend integration must be designed separately before implementation.
