# Minigames Agents

## Owns

Mini, TD, Deck, Idle, Versus, shared minigame process docs, minigame source/content/data isolation, and minigame animation workflows.

## Trigger Words

Minigame, Mini, Chadpocalypse TD, TD, Deck, Idle, Versus, bob loop, walksheet, animation atlas, tower-defense, deckbuilder, idle mode.

## Read First

- `Gameplay/Minigames/README.md`
- `Gameplay/Minigames/MINIGAME_CHARACTER_ANIMATION_INSTRUCTIONS.md` for actor animation assets.
- The owning mode's implementation file before touching mode-specific runtime code.

## Hard Rules

- Keep modes isolated in their own source, content, source-asset, and documentation roots.
- Do not implement TD by extending Mini files in place.
- Do not put mode-specific data into another mode's content/data folder.
- Use the smallest useful animation scope first, such as `bob-only`, when that satisfies the mode.
