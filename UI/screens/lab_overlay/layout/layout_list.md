# Lab Overlay Layout List

Reference gate target: Lab overlay where it shares the in-run overlay family.

## Regions

- `lab_outer_shell` - full or partial lab overlay frame; ownership: generated-shell.
- `lab_header` - title, status, and close/exit controls; ownership: generated-shell with runtime-text.
- `item_grant_panel` - item grant list/cards; ownership: socket-frame with runtime item text/icons.
- `enemy_spawn_panel` - enemy tabs/list/cards; ownership: generated-shell with runtime text.
- `reset_controls` - reset/grant/spawn buttons; ownership: generated control plates with live labels.
- `status_footer` - action result text; ownership: runtime-text.

## Controls And States

- `lab_section_tab` - required states: normal, hover, pressed, selected, disabled.
- `grant_item_button` - required states: normal, hover, pressed, disabled.
- `spawn_enemy_button` - required states: normal, hover, pressed, disabled.
- `reset_button` - required states: normal, hover, pressed, disabled.

## Live Runtime Content

- Live text: item/enemy names, labels, statuses, button text.
- Live values: counts, selected tab, spawn/reset state.
- Live image/icon/avatar wells: item icons if present.
- Live preview/media areas: none.

## Variants

- Item grant tab.
- Enemy spawn tab.
- Reset/empty state.
- Collapsed/expanded lab panel if applicable.
