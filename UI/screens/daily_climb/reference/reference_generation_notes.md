# Daily Climb Reference Generation Notes - V2

- Chat number: Chat 3
- Status: accepted
- Source current screenshot: `C:\UE\T66\UI\screens\daily_climb\current\current_state_1920x1080.png`
- Accepted reference: `C:\UE\T66\UI\screens\daily_climb\reference\daily_climb_reference_1920x1080.png`
- Raw imagegen output: `C:\UE\T66\UI\screens\daily_climb\reference\daily_climb_raw_imagegen_1672x941_v2.png`
- Normalization: deterministic resample to 1920x1080 with `C:\UE\T66\Scripts\InvokeDeterministicResample.py`

## Style Sources

- Main-menu anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- 4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
- Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
- Imagegen chrome sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Layout And Rule Handling

- Current screenshot existed: yes
- Top bar: included as fixed shared chrome at the top.
- Back button handling: removed the standalone `Back` button from the current screenshot.
- Replaced back affordance: no
- Preserved components: `Daily Challenge` title, subtitle, `Today's Rules` panel, one rule text box, four stat cells, empty `Rule Stack`, hero monument scene, and `Start Challenge` button.
- Explicitly not added: extra challenge modes, history panels, leaderboards, weekly/expert/legacy labels, extra currencies, badges, or footer navigation.

## Runtime-Owned Regions To Preserve Later

- Hero, difficulty, seed quality, reward, rule stack content, and start action remain runtime-owned.
- The large scene/hero region is a style reference only; runtime implementation should preserve live or authored screen-specific ownership.
- The shared top bar should remain shared chrome rather than duplicated per screen.
