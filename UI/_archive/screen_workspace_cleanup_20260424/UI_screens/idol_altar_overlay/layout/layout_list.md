# Idol Altar Overlay Layout List

Reference gate target: in-run Idol Altar selection overlay. Preserve existing card count, action locations, and source altar behavior.

## Regions

- `modal_scrim` - centered overlay context over gameplay; ownership: neutral generated-shell.
- `altar_panel_shell` - main stone/dungeon modal frame; ownership: generated-shell.
- `title_header` - "IDOL ALTAR" title area; ownership: runtime-text on generated-shell.
- `idol_offer_card_row` - offer card shells for current stock; ownership: socket-frame with runtime idol icons/text.
- `idol_icon_socket` - icon well inside each offer card; ownership: runtime-icon.
- `idol_description_well` - live name/description/level text; ownership: runtime-text/runtime-value.
- `status_footer` - status/error/success message area; ownership: runtime-text.
- `button_row` - back/reroll/take/return controls; ownership: generated button shells with live labels.

## Controls And States

- `idol_offer_card` - required states: normal, hover, selected, disabled/unavailable.
- `take_return_button` - required states: normal, hover, pressed, disabled.
- `reroll_button` - required states: normal, hover, pressed, disabled.
- `back_button` - required states: normal, hover, pressed.

## Live Runtime Content

- Live text: title, idol names, descriptions, upgrade/selection status, button labels.
- Live values: remaining selections, idol level, reroll availability/cost if visible.
- Live image/icon/avatar wells: idol icons.
- Live preview/media areas: none.

## Variants

- Normal multi-offer altar.
- Tutorial single-offer altar.
- Offer selected.
- No selections remaining.
