# Inventory Layout List

Canvas: 1920x1080

Source current screenshot: `C:\UE\T66\UI\screens\inventory\current\current_state_1920x1080.png`
Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`

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
  - stage-number label panel directly below.
- Bottom-left player HUD cluster:
  - hearts/health row
  - `Idols` panel with four slots in a two-by-two grid
  - hero portrait/card with level badge
  - narrow vertical ability/icon strip to the right of the portrait.
- Large inventory panel anchored along the lower center/right:
  - header with `Inventory` title
  - small currency/resource icons and values beside the title
  - two-row item grid with ten columns visible
  - large empty lower interior below the grid.

## Runtime-Owned Regions

- Gameplay scene, character, crosshair, score/time values, minimap content, stage number, hearts, idol slots, hero portrait, level badge, ability icons, inventory title, resource values, and item slots are runtime-owned.
- The generated reference should stylize HUD frames, slot plates, and panel shells while leaving scene, portrait, icon, minimap, and item interiors as live apertures or socket wells.

## Explicit Bans

- Do not add modal title bars, powerup cards, challenge panels, party controls, tabs, filters, new buttons, or full-screen foreground panels.
- Do not obscure the live gameplay scene beyond the existing HUD overlays.
- Do not move the bottom inventory panel, bottom-left player HUD cluster, top-left score panel, or top-right minimap.
