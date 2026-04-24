# Gameplay HUD Layout List

Reference gate target: normal in-run gameplay HUD at 1920x1080. Preserve the current HUD layout and gameplay readability; only the generated reference may reinterpret the shells in the canonical main-menu stone/dungeon style.

## Regions

- `left_status_stack_shell` - bottom-left grouped HUD shell for player/status/abilities/idols; ownership: generated-shell with live wells.
- `portrait_status_frame` - player portrait/name/status socket; ownership: socket-frame, runtime portrait/status text live.
- `health_bar_socket` - HP presentation well and backing frame; ownership: generated-shell plus runtime-value/fill.
- `ability_socket_row` - basic ability/ultimate/passive sockets; ownership: socket-frame, runtime icons/cooldowns live.
- `bottom_right_inventory_panel` - bottom-right inventory/economy surface; ownership: generated-shell with runtime values and item sockets.
- `inventory_slot_strip` - bottom-right inventory item sockets with rarity/accent borders; ownership: socket-frame, runtime icons/counts live.
- `minimap_frame` - top/right minimap frame and title accents; ownership: generated-shell, map pixels and markers live.
- `boss_bar_region` - boss health/name overlay when active; ownership: generated-shell with runtime text/value/fill.
- `loot_prompt_region` - nearby loot/tutorial/interaction prompts; ownership: generated-shell with runtime localized text.
- `pickup_chest_reward_cards` - transient reward cards sharing HUD surface; ownership: generated-shell plus runtime text/icons/values.
- `timer_score_resource_text` - live run values, gold/debt/score/timer labels; ownership: runtime-text/runtime-value.

## Controls And States

- `inventory_slot` - required states: normal, hover, selected, disabled/empty, rarity accents.
- `ability_socket` - required states: ready, cooldown, disabled/locked, active.
- `idol_socket` - required states: empty, occupied, hover, selected.
- `minimap_frame` - required states: normal only; live map remains separate.
- `reward_card` - required states: hidden, visible, hover if interactive.
- `boss_bar` - required states: hidden, visible, damaged/low HP.
- `loot_prompt` - required states: hidden, visible, accepted/rejected if applicable.

## Live Runtime Content

- Live text: hero/status labels, timers, cooldown numbers, reward descriptions, keybind prompts.
- Live values: HP, gold, debt, score, stage timer, cooldowns, item counts, damage/reward values.
- Live image/icon/avatar wells: portrait, ability icons, ultimate/passive icons, idol icons, inventory item icons, reward item icons.
- Live preview/media areas: minimap render data and map markers.

## Variants

- Normal gameplay HUD.
- Combat update state with changing HP/cooldowns.
- Pickup/chest/reward panel visible.
- Boss bar visible.
- Loot/tutorial prompt visible.
- Empty versus populated inventory/idol rows.
