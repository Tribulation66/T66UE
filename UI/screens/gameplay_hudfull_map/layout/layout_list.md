# Gameplay HUD Full Map Layout List

Reference gate target: full in-run map overlay opened from the HUD. Preserve the current full-map scale, center placement, and live map data ownership.

## Regions

- `full_map_scrim` - dim gameplay backing layer; ownership: generated-shell or neutral overlay.
- `full_map_outer_frame` - large centered stone/dungeon frame; ownership: generated-shell.
- `map_canvas_live_area` - interactive/live world map render area; ownership: runtime-media/open-aperture.
- `world_map_marker_layer` - player arrow, POI, enemy, tower reveal, and marker data inside `ST66WorldMapWidget`; ownership: runtime-icon/runtime-state.
- `tower_reveal_and_floor_art` - dynamic reveal mask, tower themed floor art, bounds and hole data; ownership: runtime-media.
- `map_title_header` - title/header band if present; ownership: generated-shell plus runtime-text.
- `legend_or_hint_footer` - small control hints/legend if present; ownership: runtime-text on generated-shell.

## Controls And States

- `map_close_input` - real input state only; required states: available/unavailable.
- `map_marker` - runtime marker states: player, enemy, objective, tower reveal.
- `reveal_mask` - runtime states: disabled, enabled, partially revealed.
- `map_frame` - required states: normal only.

## Live Runtime Content

- Live text: map title, hints, labels if emitted by runtime.
- Live values: player position, floor bounds, reveal mask, markers.
- Live image/icon/avatar wells: marker icons and player arrow.
- Live preview/media areas: full world map canvas.

## Variants

- Full map open with known world bounds.
- Full map open with tower reveal mask.
- Full map open with sparse/empty marker set.
