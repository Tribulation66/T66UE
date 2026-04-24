# Collector Overlay Layout List

Reference gate target: in-run/lab Collector overlay where it shares the in-run overlay family.

## Regions

- `collector_outer_shell` - main overlay frame; ownership: generated-shell.
- `collector_header` - title and exit affordance area; ownership: generated-shell with runtime-text.
- `item_spawn_panel` - unlocked item list/cards; ownership: socket-frame with runtime item text/icons.
- `npc_spawn_panel` - NPC/interactable cards; ownership: generated-shell with runtime text.
- `enemy_spawn_panel` - enemy/boss cards; ownership: generated-shell with runtime text.
- `status_footer` - live action status line; ownership: runtime-text.

## Controls And States

- `spawn_card_button` - required states: normal, hover, pressed, disabled.
- `tab_or_section_button` - required states: normal, hover, pressed, selected, disabled if present.
- `exit_button` - required states: normal, hover, pressed.

## Live Runtime Content

- Live text: item/enemy/NPC names, card labels, status messages.
- Live values: counts/unlock availability if displayed.
- Live image/icon/avatar wells: item icons if present.
- Live preview/media areas: none.

## Variants

- Items section populated.
- Enemies section populated.
- NPC/interactable section populated.
- Empty/unavailable data state.
