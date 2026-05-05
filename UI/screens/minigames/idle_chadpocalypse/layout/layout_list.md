# Idle Chadpocalypse Layout List

Canvas: `1920x1080`
Screen slug: `idle_chadpocalypse`
Status: simplified two-screen mockup pass generated on 2026-05-03. The earlier dashboard, upgrade-lab, offline-results, and component-sheet direction is superseded for now.

## Required Image-Generation Inputs

Use these inputs for future reference generation:

1. Style authority: `C:\UE\T66\Docs\UI\UI_GENERATION.md`
2. Screen workflow checklist: `C:\UE\T66\UI\SCREEN_WORKFLOW.md`
3. Main-menu visual anchor, if available: `C:\UE\T66\UI\screens\main_menu\reference\Reference 12.png`
4. This layout list: `C:\UE\T66\UI\screens\minigames\idle_chadpocalypse\layout\layout_list.md`
5. Current runtime capture: `C:\UE\T66\UI\screens\minigames\idle_chadpocalypse\current\2026-05-03\idle_main_menu_packaged_1920x1080.png`
6. Main-menu mockup: `C:\UE\T66\UI\screens\minigames\idle_chadpocalypse\reference\idle_chadpocalypse_main_menu_mockup_1920x1080_imagegen_20260503_v2.png`
7. Gameplay mockup: `C:\UE\T66\UI\screens\minigames\idle_chadpocalypse\reference\idle_chadpocalypse_gameplay_mockup_1920x1080_imagegen_20260503_v2.png`

Do not generate component sheets in this pass. Use full-screen references only until the screen direction is approved.

## Product Concept

Idle Chadpocalypse is a simple offline-progress idle minigame. The first UI pass should communicate only the entry screen and the first playable loop: start the idle run, watch a hero fight automatically, collect progress, and buy a small set of upgrades.

## Mockup Inventory

### Mockup 01: Main Menu

Canvas: `1920x1080`

Primary layout:
- Large game title area in the upper-left or upper-center.
- One hero or idle engine visual as the primary scene anchor.
- Three large menu buttons: `Play`, `Upgrades`, `Options` or equivalent runtime labels.
- Optional small footer/back area for returning to Minigames.

What belongs here:
- Game identity.
- Simple entry actions.
- A readable dungeon/idle-engine mood.

What does not belong here:
- Resource dashboards.
- Hero rosters.
- Task lists.
- Offline reward modals.
- Upgrade grids.

Runtime-owned content:
- Button labels, title text, focus state, hover state, save-slot state, platform prompts, and localization.

### Mockup 02: Gameplay

Canvas: `1920x1080`

Primary layout:
- Thin top resource bar with three broad resource wells.
- Center gameplay scene: one hero, one enemy, and one idle engine or progress object.
- Simple health/progress bars for hero, enemy, and idle progress.
- Bottom action band with three large upgrade/action buttons.
- Small offline/progress meter in the bottom-right or bottom band.

What belongs here:
- One visible idle combat loop.
- One clear collect/progress meter.
- A tiny number of upgrade/action affordances.

What does not belong here:
- Multiple management tabs.
- Dense sortable lists.
- Hero assignment tables.
- Prestige systems.
- Pop-up reward screens.

Runtime-owned content:
- Resource values, health values, timers, progress fills, button labels, enemy names, hero names, upgrade names, tooltips, and localization.

## Image And Asset Rules

- Raster pixel-art UI only.
- Keep references simple and readable at 1920x1080.
- Avoid baked live data, player-specific values, prices, timers, or long labels.
- Do not create component sheets until the two screen references are approved.
- Old component-sheet candidates remain archived references and should not steer this simplified pass.
