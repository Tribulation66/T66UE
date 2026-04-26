# Run Summary Layout List - V2

Canvas: 1920x1080

Source current screenshot: `C:\UE\T66\UI\screens\run_summary\current\current_state_1920x1080.png`
Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
Chrome helper sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Top-Bar And Back Handling

- This is a run summary/result screen. Do not force the global top bar into the composition.
- No standalone Back button is present; do not add one.

## Fixed Structure

- Full-screen framed summary surface with outer border and right-side vertical scrollbar.
- Top-left page title `Run Summary`.
- Summary stat line below the title containing stage reached, score, and time.
- Top-right `Event Log` button.
- Two wide horizontal rank panels in the upper half:
  - weekly rank panel
  - all-time rank panel
  - each panel has score and speed-run value regions
- Left lower column:
  - seed luck panel with value
  - integrity panel with status text
  - `Go Again!` button
  - `Main Menu` button
- Center column:
  - runtime 3D preview viewport
  - four selected-buff slots below the preview
  - bottom row of item/inventory/reward slots across the lower center
- Right column:
  - stats panel with a vertical list of player stat rows
  - damage-by-source panel with table header and ranked rows

## Runtime-Owned Regions

- All titles, stat names, values, rank values, time, score, stage, button labels, integrity text, table labels, and table rows are runtime-owned text/values.
- Center character/scene preview is a runtime 3D preview/open aperture.
- Buff/item slots are runtime-owned icon/image wells.

## Explicit Bans

- Do not add tabs, powerup cards, challenge controls, leaderboard filters, extra CTAs, extra result rows, extra metrics, currencies, sidebars, new buttons, new panels, or explanatory content.
- Do not remove the 3D preview aperture, lower item slots, or the two rank panels.
- Do not add cracked stone, gemstones, crystal panels, screws, rivets, chipped edges, rubble, distressed frames, or noisy bevel-heavy borders.
