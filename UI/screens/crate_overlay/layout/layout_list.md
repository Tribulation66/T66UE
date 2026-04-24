# Crate Overlay Layout List

Reference gate target: CS-style crate opening overlay presentation.

## Regions

- `crate_overlay_shell` - centered presentation frame; ownership: generated-shell.
- `strip_container_frame` - item scrolling strip frame; ownership: generated-shell.
- `item_tile_sockets` - repeated item tiles inside strip; ownership: socket-frame with runtime item icons/rarity.
- `winner_marker` - center pointer/selection marker; ownership: generated-shell/runtime-state.
- `skip_hint_text` - skip instruction/status; ownership: runtime-text.
- `result_status_area` - resolved item/reward message if visible; ownership: runtime-text/runtime-value.

## Controls And States

- `crate_overlay_frame` - required states: opening, scrolling, resolved.
- `item_tile` - required states: normal, highlighted/winner, rarity variants.
- `skip_control` - required states: available, disabled, hidden.

## Live Runtime Content

- Live text: skip hint, resolved item name/status.
- Live values: winner rarity, scroll offset, draw result.
- Live image/icon/avatar wells: runtime item icons and rarity colors.
- Live preview/media areas: scrolling strip animation.

## Variants

- Pre-scroll opening.
- Active scrolling.
- Resolved item state.
