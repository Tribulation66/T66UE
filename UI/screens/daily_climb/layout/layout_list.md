# Daily Climb Layout List - V2

Canvas: 1920x1080

Source current screenshot: `C:\UE\T66\UI\screens\daily_climb\current\current_state_1920x1080.png`
Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
Chrome helper sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Top-Bar And Back Handling

- Include the shared global top bar exactly as fixed chrome at the top of the full-screen reference.
- Remove the standalone `Back` button from the current screenshot.
- Reclaim the top-left Back-button space without adding any replacement back arrow, footer, or new control.
- Daily Climb content begins below the global top bar.

## Fixed Structure

- Scenic daily challenge background below the global top bar, preserving the same central statue/stair focal area, banners, and torch/fire accents from the current screenshot.
- Large centered page title across the upper content area.
- Subtitle centered below the title.
- Large left-side `Today's Rules` panel in the same lower-left position:
  - panel title
  - divider line
  - bordered description text box
  - four stat/info cards in a two-by-two grid: hero, difficulty, seed quality, reward
  - `Rule Stack` heading
  - long empty rule-stack box near the panel bottom
- Large bottom-right `Start Challenge` CTA button in the same position and size relationship.

## Runtime-Owned Regions

- Page title, subtitle, panel title, description, card labels, card values, rule-stack text, reward values, and CTA label are runtime-owned text/values.

## Explicit Bans

- Do not add sidebars, leaderboards, tabs, inventory slots, currency boxes, extra CTA buttons, or any replacement Back affordance.
- Do not add cracked stone, gemstones, crystal panels, screws, rivets, chipped edges, rubble, distressed frames, or noisy bevel-heavy borders.
