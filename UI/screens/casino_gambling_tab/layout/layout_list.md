# Casino Gambling Tab Layout List

Reference gate target: canonical Casino overlay with Gambling tab active, embedding the Gambler game selection surface.

## Regions

- `casino_main_shell` - compact overlay outer frame; ownership: generated-shell.
- `header_summary_bar` - runtime score/time/gold/debt/anger summary; ownership: generated-shell plus runtime values.
- `tab_button_row` - Casino tabs with Gambling selected; ownership: real buttons/generator state plates.
- `gambler_content_panel` - embedded game selection page; ownership: generated-shell.
- `game_card_grid` - game entry cards for coin flip/RPS/blackjack/lottery/plinko/box opening; ownership: socket-frame with runtime icons/text.
- `banking_controls_panel` - borrow/pay/debt controls; ownership: generated-shell with live values and real controls.
- `inventory_sell_strip` - optional sell/buyback item sockets; ownership: socket-frame with runtime item icons.
- `status_message_footer` - live status/result line; ownership: runtime-text.

## Controls And States

- `casino_tab_button` - required states: normal, hover, pressed, selected, disabled.
- `game_card_button` - required states: normal, hover, pressed, disabled.
- `bet_borrow_pay_controls` - required states: normal, hover/focused, pressed, disabled.
- `inventory_sell_slot` - required states: normal, hover, selected, disabled/empty.

## Live Runtime Content

- Live text: game names, descriptions, status/result messages, button labels.
- Live values: wager, borrow/payback amount, gold, debt, anger, score, timer.
- Live image/icon/avatar wells: game icons, item icons, anger face icon.
- Live preview/media areas: none.

## Variants

- Gambling tab selected.
- Game card hovered.
- Banking state with debt.
- Game subpage entry state.
