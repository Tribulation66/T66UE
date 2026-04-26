# Pause Menu Reference Generation Notes - V2

- Chat number: Chat 3
- Status: accepted
- Source current screenshot: `C:\UE\T66\UI\screens\pause_menu\current\current_state_1920x1080.png`
- Accepted reference: `C:\UE\T66\UI\screens\pause_menu\reference\pause_menu_reference_1920x1080.png`
- Raw imagegen output: `C:\UE\T66\UI\screens\pause_menu\reference\pause_menu_raw_imagegen_1672x941_v2.png`
- Normalization: deterministic resample to 1920x1080 with `C:\UE\T66\Scripts\InvokeDeterministicResample.py`

## Style Sources

- Main-menu anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- 4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
- Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
- Imagegen chrome sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Layout And Rule Handling

- Current screenshot existed: yes
- Top bar: not forced; active content is the centered pause modal over a dimmed runtime background.
- Back button handling: no standalone back button was generated.
- Replaced back affordance: no
- Preserved components: dimmed background, centered modal, `Paused` title, and exactly six buttons: `Resume Game`, `Save And Quit`, `Restart`, `Settings`, `Achievements`, `Leaderboard`.
- Explicitly not added: close icon, back button, seventh button, tabs, sidebars, currencies, footer navigation, or explanatory content.

## Runtime-Owned Regions To Preserve Later

- Dimmed background is runtime-owned and should reflect the current underlying game/menu state.
- All six button actions remain runtime-owned.
