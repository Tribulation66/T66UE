# Casino Alchemy Tab Layout List

Reference gate target: canonical Casino overlay with Alchemy tab active.

## Regions

- `casino_main_shell` - compact overlay outer frame; ownership: generated-shell.
- `header_summary_bar` - runtime score/time/gold/debt/anger summary; ownership: generated-shell plus runtime values.
- `tab_button_row` - Casino tabs with Alchemy selected; ownership: real buttons/generator state plates.
- `alchemy_content_panel` - alchemy workspace panel; ownership: generated-shell.
- `target_item_socket` - primary item drop target; ownership: socket-frame with runtime icon/text.
- `sacrifice_item_socket` - secondary item drop target; ownership: socket-frame with runtime icon/text.
- `inventory_card_row` - draggable inventory item cards; ownership: socket-frame with runtime items.
- `alchemy_result_status` - live result/status area; ownership: runtime-text.

## Controls And States

- `casino_tab_button` - required states: normal, hover, pressed, selected, disabled.
- `alchemy_drop_socket` - required states: empty, hover/drop-target, occupied, disabled.
- `alchemy_inventory_card` - required states: normal, hover, dragging, selected, disabled.
- `transmute_button` - required states: normal, hover, pressed, disabled.
- `clear_slot_button` - required states: normal, hover, pressed, disabled.

## Live Runtime Content

- Live text: item names/details, result status, button labels.
- Live values: net worth, gold, debt, anger, score, timer, item counts.
- Live image/icon/avatar wells: item icons in sockets and cards.
- Live preview/media areas: none.

## Variants

- Empty alchemy slots.
- One slot occupied.
- Both slots occupied, transmute available.
- Invalid or disabled transmute state.
