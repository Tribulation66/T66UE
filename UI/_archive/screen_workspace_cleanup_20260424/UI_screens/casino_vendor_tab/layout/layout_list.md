# Casino Vendor Tab Layout List

Reference gate target: canonical Casino overlay with Vendor tab active. This is the canonical in-run vendor/gambling/alchemy shell.

## Regions

- `casino_overlay_scrim` - gameplay backdrop dim/context; ownership: neutral generated-shell.
- `casino_main_shell` - compact casino overlay outer frame; ownership: generated-shell.
- `header_summary_bar` - score/time/gold/debt/anger summary; ownership: generated-shell with runtime values.
- `tab_button_row` - Vendor/Gambling/Alchemy tabs; ownership: real buttons with generated state plates and live labels.
- `vendor_content_panel` - embedded vendor page surface; ownership: generated-shell.
- `vendor_offer_grid` - item offer card sockets; ownership: socket-frame with runtime item icons/prices/text.
- `inventory_buyback_strip` - player inventory/buyback sockets if visible; ownership: socket-frame with runtime items.
- `status_message_footer` - live vendor/casino status line; ownership: runtime-text.

## Controls And States

- `casino_tab_button` - required states: normal, hover, pressed, selected, disabled.
- `vendor_offer_card` - required states: normal, hover, selected, locked, disabled/sold.
- `buy_button` - required states: normal, hover, pressed, disabled.
- `steal_reroll_lock_buttons` - required states: normal, hover, pressed, disabled, selected/locked.

## Live Runtime Content

- Live text: tab labels, offer names/descriptions, button labels, status messages.
- Live values: prices, gold, debt, anger, score, timer, reroll count/cost.
- Live image/icon/avatar wells: item icons and rarity icons.
- Live preview/media areas: none.

## Variants

- Vendor tab selected.
- Offer hovered/selected.
- Locked offer.
- Insufficient gold/debt/anger state.
