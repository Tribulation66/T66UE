# Gameplay HUD Inventory Inspect Layout List

Reference gate target: in-run HUD with inventory inspect mode open. Preserve the existing inspect layout and slot hit targets.

## Regions

- `hud_background_context` - gameplay scene remains visible behind HUD; ownership: generated-scene-plate/runtime scene, not UI art.
- `inventory_inspect_panel_shell` - expanded inspect surface anchored to inventory region; ownership: generated-shell.
- `inventory_panel_scaled_state` - existing bottom-right inventory panel enlarged/interactive in inspect mode; ownership: generated-shell/runtime-state.
- `inventory_slot_strip` - existing item slots, not a separate detail grid; ownership: socket-frame with runtime item icons and counts.
- `rarity_accent_frames` - per-slot rarity border/highlight; ownership: generated-shell/runtime-state.
- `economy_readouts` - gold/debt/score/timer text still live around the inspect surface; ownership: runtime-text/runtime-value.
- `close_or_exit_affordance` - implicit real control via input/cursor mode; ownership: runtime control, no baked text.

## Controls And States

- `inspect_inventory_slot` - required states: normal, hover, selected, disabled/empty, rarity variants.
- `inventory_panel_scaled_state` - required states: closed, inspect-open.

## Live Runtime Content

- Live text: keybind/inspect prompt if visible.
- Live values: item stack counts, rarity state, selection index, economy/timer values.
- Live image/icon/avatar wells: item icons and rarity icons.
- Live preview/media areas: none.

## Variants

- Empty inventory inspect.
- Populated inventory inspect.
- Hovered slot.
- Selected/hovered slot, with any native tooltip if runtime shows one.
