# Temporary Buff Shop Layout List - V2

Canvas: 1920x1080

Source current screenshot: `C:\UE\T66\UI\screens\temporary_buff_shop\current\current_state_1920x1080.png`
Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
Chrome helper sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Top-Bar And Back Handling

- This is a run-flow shop surface, not a main-menu child screen. Do not force the global top bar into the composition.
- Preserve the existing `Back To Buffs` in-flow button because it is part of this shop screen, not a global standalone Back affordance.

## Fixed Structure

- Dark in-game background remains visible around the shop frame.
- One large centered shop panel, inset from the left and right screen edges, with a clean dark frame and crisp border.
- Top-left title `Temp Buff Shop`.
- Currency line beneath the title.
- One-line explanatory description below the currency line.
- Top-right `Back To Buffs` button.
- Main scrollable buff-card grid inside the shop panel:
  - five full columns visible
  - three full rows visible plus a clipped partial fourth row at the bottom
  - each card has a top icon socket, buff name, effect text, owned count, and bottom buy button
- Right-side vertical scrollbar inside the shop panel.

## Runtime-Owned Regions

- Title, currency value, description, card names, effect text, owned counts, prices, buy labels, and back-button label are runtime-owned text/values.
- Buff art/icons are runtime-owned image wells.

## Explicit Bans

- Do not add party controls, a difficulty dropdown, a `Done` button, or an `Enter Tribulation` CTA; those belong to temporary buff selection.
- Do not add tabs, filters, leaderboards, extra top navigation, extra offer columns, extra rows beyond the visible/partial count, extra currencies, sidebars, or explanatory panels.
- Do not add cracked stone, gemstones, crystal panels, screws, rivets, chipped edges, rubble, distressed frames, or noisy bevel-heavy borders.
