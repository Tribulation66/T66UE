# Temporary Buff Selection Layout List - V2

Canvas: 1920x1080

Source current screenshot: `C:\UE\T66\UI\screens\temporary_buff_selection\current\current_state_1920x1080.png`
Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
Chrome helper sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Top-Bar And Back Handling

- This is a run-flow selection surface, not a main-menu child screen. Do not force the global top bar into the composition.
- No standalone Back button is present; do not add one.

## Fixed Structure

- Full-screen dark framed selection surface with a clean outer border.
- Top-left title `Select Temp Buffs`.
- Instruction text beneath the title.
- Four selected-buff slots along the upper-left area, each with a centered plus marker.
- Top-right currency panel showing Chad Coupons / current CC.
- Main scrollable buff-card grid fills the center:
  - visible card columns across the screen, with a partial clipped column at the far right
  - two full rows visible plus lower partial content
  - each card has an icon socket near the top, buff name, short description, owned/slotted counts, and a bottom buy button
- Right-side vertical scrollbar for the grid.
- Bottom command strip:
  - party panel at bottom left
  - four party member selection buttons inside the party panel
  - `Done` button near bottom center
  - difficulty dropdown near bottom center-right
  - large `Enter Tribulation` CTA button at bottom right

## Runtime-Owned Regions

- Title, instruction copy, currency value, party labels, party slot values, dropdown value, CTA labels, card names, descriptions, owned/slotted counts, prices, and buy labels are runtime-owned text/values.
- Buff icons are runtime-owned icon/image wells.
- Selection slots and party slots are runtime-owned state wells.

## Explicit Bans

- Do not remove the bottom party/done/dropdown/enter strip.
- Do not add a `Back To Buffs` button here; that belongs to the temporary buff shop screen.
- Do not add top bar, tabs, filters, leaderboards, side navigation, extra rows, extra slots, extra currencies, or explanatory panels.
- Do not turn the grid into a centered modal or change the visible card density.
- Do not add cracked stone, gemstones, crystal panels, screws, rivets, chipped edges, rubble, distressed frames, or noisy bevel-heavy borders.
