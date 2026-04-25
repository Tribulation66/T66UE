# Temporary Buff Shop Layout List

Canvas: 1920x1080

Source current screenshot: `C:\UE\T66\UI\screens\temporary_buff_shop\current\current_state_1920x1080.png`
Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`

## Fixed Structure

- Dark in-game background remains visible around the shop frame.
- One large centered shop panel, inset from the left and right screen edges, with a clean dark frame, crisp border, and restrained metallic accents.
- Top-left title `Temp Buff Shop`.
- Currency line beneath the title.
- One-line explanatory description below the currency line.
- Top-right `Back To Buffs` button.
- Main scrollable buff-card grid inside the shop panel:
  - five full columns visible.
  - three full rows visible plus a clipped partial fourth row at the bottom.
  - each card has a top icon socket, buff name, effect text, owned count, and bottom buy button.
- Right-side vertical scrollbar inside the shop panel.

## Runtime-Owned Regions

- Title, currency value, description, card names, effect text, owned counts, prices, buy labels, and back-button label are runtime-owned text/values.
- Buff art/icons are runtime-owned image wells.
- The generated reference should stylize frames, shells, cards, and button plates while keeping text/icon interiors replaceable later.

## Explicit Bans

- Do not add party controls, a difficulty dropdown, a `Done` button, or an `Enter Tribulation` CTA; those belong to temporary buff selection.
- Do not add tabs, filters, leaderboards, or extra top navigation.
