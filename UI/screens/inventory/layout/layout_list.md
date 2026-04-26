# Inventory Layout List - V2

Canvas: 1920x1080

Source current screenshot: `C:\UE\T66\UI\screens\inventory\current\current_state_1920x1080.png`
Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
Chrome helper sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Top-Bar And Back Handling

- This is a live gameplay HUD/inventory overlay. Do not force the global top bar into the composition.
- No standalone Back button is present; do not add one.

## Fixed Structure

- Full-screen live gameplay scene remains visible and dominant.
- Small score/time objective panel in the top-left corner:
  - score value
  - score-to-beat line
  - time value
  - time-to-beat line
- Small crosshair at the screen center.
- Top-right minimap panel:
  - square map aperture with small player marker
  - stage-number label panel directly below
- Bottom-left player HUD cluster:
  - hearts/health row
  - `Idols` panel with four slots in a two-by-two grid
  - hero portrait/card with level badge
  - narrow vertical ability/icon strip to the right of the portrait
- Large inventory panel anchored along the lower center/right:
  - header with `Inventory` title
  - small currency/resource icons and values beside the title
  - two-row item grid with ten columns visible
  - large empty lower interior below the grid

## Runtime-Owned Regions

- Gameplay scene, character, crosshair, score/time values, minimap content, stage number, hearts, idol slots, hero portrait, level badge, ability icons, inventory title, resource values, and item slots are runtime-owned.

## Explicit Bans

- Do not add modal title bars, powerup cards, challenge panels, party controls, tabs, filters, new buttons, full-screen foreground panels, extra inventory rows or columns, hotbars, currencies, meters, ability slots, idol slots, map controls, sidebars, or explanatory text.
- Do not obscure the live gameplay scene beyond the existing HUD overlays.
- Do not move the bottom inventory panel, bottom-left player HUD cluster, top-left score panel, top-right minimap, or crosshair.
- Do not add cracked stone, gemstones, crystal panels, screws, rivets, chipped edges, rubble, distressed frames, or noisy bevel-heavy borders.
