# Chadpocalypse Deckbuilder Layout List

Canvas: `1920x1080`
Screen slug: `chadpocalypse_deckbuilder`
Status: simplified two-screen mockup pass generated on 2026-05-03. The earlier combat-table, reward-draft, map/shop, and component-sheet direction is superseded for now.

## Required Image-Generation Inputs

Use these inputs for future reference generation:

1. Style authority: `C:\UE\T66\Docs\UI\UI_GENERATION.md`
2. Screen workflow checklist: `C:\UE\T66\UI\SCREEN_WORKFLOW.md`
3. Main-menu visual anchor, if available: `C:\UE\T66\UI\screens\main_menu\reference\Reference 12.png`
4. This layout list: `C:\UE\T66\UI\screens\minigames\chadpocalypse_deckbuilder\layout\layout_list.md`
5. Current runtime capture: `C:\UE\T66\UI\screens\minigames\chadpocalypse_deckbuilder\current\2026-05-03\deck_main_menu_packaged_styled_1920x1080.png`
6. Main-menu mockup: `C:\UE\T66\UI\screens\minigames\chadpocalypse_deckbuilder\reference\chadpocalypse_deckbuilder_main_menu_mockup_1920x1080_imagegen_20260503_v2.png`
7. Gameplay mockup: `C:\UE\T66\UI\screens\minigames\chadpocalypse_deckbuilder\reference\chadpocalypse_deckbuilder_gameplay_mockup_1920x1080_imagegen_20260503_v2.png`

Do not generate component sheets in this pass. Use full-screen references only until the screen direction is approved.

## Product Concept

Chadpocalypse Deckbuilder is a simple dungeon-descent card minigame. The first UI pass should communicate only the entry screen and the first combat loop: start a run, see one enemy encounter, play cards from hand, and end the turn.

## Mockup Inventory

### Mockup 01: Main Menu

Canvas: `1920x1080`

Primary layout:
- Large game title area in the upper-left or upper-center.
- Dungeon descent visual, card spread, or relic pedestal as the primary scene anchor.
- Three large menu buttons: `Play`, `Collection`, `Options` or equivalent runtime labels.
- Optional small footer/back area for returning to Minigames.

What belongs here:
- Game identity.
- Simple entry actions.
- A visual promise of cards and descending into the dungeon.

What does not belong here:
- Combat state.
- Encounter map.
- Shop.
- Reward draft.
- Deck preview drawer.

Runtime-owned content:
- Button labels, title text, focus state, hover state, save-slot state, platform prompts, and localization.

### Mockup 02: Gameplay

Canvas: `1920x1080`

Primary layout:
- Enemy at the upper-right or upper-center with one health bar and one intent icon.
- Player status at the lower-left with portrait, health, armor, and energy.
- Five large cards across the bottom center with blank art/text wells.
- One large end-turn button at the bottom-right.
- Clear empty dungeon floor between player, cards, and enemy.

What belongs here:
- One enemy.
- One player status block.
- Five-card hand.
- End-turn affordance.

What does not belong here:
- Map paths.
- Shop inventory.
- Draft rewards.
- Relic walls.
- Large combat logs.
- Dense buff/debuff rows.

Runtime-owned content:
- Card names, card art, card rules, costs, enemy names, health values, intent values, energy, button labels, tooltips, and localization.

## Image And Asset Rules

- Raster pixel-art UI only.
- Keep references simple and readable at 1920x1080.
- Avoid baked card rules, values, prices, enemy names, long labels, or player-specific state.
- Do not create component sheets until the two screen references are approved.
- Old component-sheet candidates remain archived references and should not steer this simplified pass.
