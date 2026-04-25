# Gambler Overlay Layout List

Reference gate target: standalone Gambler overlay/casino page. Preserve existing game pages and live economy behavior.

## Regions

- `gambler_outer_shell` - full-screen or large overlay panel; ownership: generated-shell.
- `economy_header` - net worth/gold/debt/anger/status bar; ownership: generated-shell with runtime values.
- `dialogue_or_casino_switcher` - page switcher content area; ownership: generated-shell.
- `game_selection_grid` - game cards; ownership: socket-frame with runtime icons/text.
- `game_play_panel` - active minigame page frame; ownership: generated-shell with runtime game elements.
- `coin_flip_live_well` - coin image/status/wager result area; ownership: runtime-image/runtime-state/runtime-text.
- `blackjack_card_well` - dealer/player card images and value text; ownership: runtime-image/runtime-value.
- `lottery_number_row` - selectable numbers and drawn results; ownership: real controls/runtime-value.
- `plinko_board_well` - plinko board, pegs, ball/result; ownership: runtime-state/open-aperture.
- `box_opening_strip` - scrolling/opening strip; ownership: runtime-image/runtime-state.
- `cheat_prompt_overlay` - prompt/status overlay; ownership: generated-shell with runtime text and real controls.
- `inventory_sell_panel` - item sell/buyback strip; ownership: socket-frame with runtime icons/text.
- `banking_controls` - wager/borrow/payback controls; ownership: real controls and live values.

## Controls And States

- `game_card_button` - required states: normal, hover, pressed, disabled.
- `casino_mode_toggle` - required states: normal, hover, pressed, selected.
- `wager_controls` - required states: normal, focused, disabled.
- `minigame_action_button` - required states: normal, hover, pressed, disabled.
- `lottery_number_button` - required states: normal, hover, pressed, selected, disabled.
- `blackjack_card_slot` - required states: face-down, face-up, highlighted.
- `box_opening_tile` - required states: normal, highlighted/winner, rarity variants.
- `inventory_slot` - required states: normal, hover, selected, disabled/empty.

## Live Runtime Content

- Live text: dialogue lines, game names/descriptions, result/status messages, labels.
- Live values: wager, payouts, gold, debt, anger, net worth, item prices.
- Live image/icon/avatar wells: anger face, game icons, item icons, card/hand/coin images.
- Live preview/media areas: animated game visuals are runtime-owned.

## Variants

- Dialogue page.
- Casino game selection page.
- Coin flip/RPS/blackjack/lottery/plinko/box opening pages.
- Cheat prompt visible.
