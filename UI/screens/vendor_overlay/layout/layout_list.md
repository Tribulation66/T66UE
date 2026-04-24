# Vendor Overlay Layout List

Reference gate target: standalone in-run Vendor overlay where it remains outside the Casino shell.

## Regions

- `vendor_outer_shell` - large vendor overlay frame; ownership: generated-shell.
- `vendor_header` - title/economy summary; ownership: generated-shell with runtime text/values.
- `offer_grid` - vendor item offer cards; ownership: socket-frame with runtime item content.
- `inventory_sell_panel` - player item slots/sell details; ownership: socket-frame with runtime item content.
- `action_button_row` - buy/steal/reroll/lock/back controls; ownership: generated control plates with live labels.
- `status_message_footer` - vendor feedback and errors; ownership: runtime-text.

## Controls And States

- `offer_card` - required states: normal, hover, selected, locked, disabled/sold.
- `buy_button` - required states: normal, hover, pressed, disabled.
- `steal_button` - required states: normal, hover, pressed, disabled.
- `reroll_button` - required states: normal, hover, pressed, disabled.
- `inventory_slot` - required states: normal, hover, selected, disabled/empty.

## Live Runtime Content

- Live text: item names/descriptions, action labels, status text.
- Live values: prices, gold, debt, reroll costs, item counts.
- Live image/icon/avatar wells: item icons and rarity visuals.
- Live preview/media areas: none.

## Variants

- Buy-enabled vendor.
- Steal-enabled vendor.
- Locked offers.
- Buyback/sell state.
