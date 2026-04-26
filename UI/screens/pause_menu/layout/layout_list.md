# Pause Menu Layout List - V2

Canvas: 1920x1080

Source current screenshot: `C:\UE\T66\UI\screens\pause_menu\current\current_state_1920x1080.png`
Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
Chrome helper sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Top-Bar And Back Handling

- This is a modal overlay; do not force a global top bar into the modal.
- The dimmed background may imply the underlying frontend, but the pause modal owns only the centered modal and action stack.
- No standalone Back button is present; do not add one.

## Fixed Structure

- Full-screen dimmed main-menu/game frontend remains visible as the background only.
- One centered vertical pause modal with a clean dark frame and crisp border.
- Large title `Paused` centered near the top of the modal.
- Exactly six stacked full-width menu buttons inside the modal:
  - Resume Game, highlighted/selected green
  - Save And Quit
  - Restart, destructive/red state
  - Settings
  - Achievements
  - Leaderboard

## Runtime-Owned Regions

- Modal title and all button labels are runtime-owned text.
- Button state colors and plates are generated shell/control art, but labels stay live later.

## Explicit Bans

- Do not add close buttons, tabs, sidebars, extra menu options, icons, inventory slots, currency, confirmation panels, or any seventh button.
- Do not change the centered single-column modal hierarchy.
- Do not add cracked stone, gemstones, crystal panels, screws, rivets, chipped edges, rubble, distressed frames, or noisy bevel-heavy borders.
