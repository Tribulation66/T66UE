# Challenges Reference Generation Notes - V2

- Chat number: Chat 3
- Status: accepted
- Source current screenshot: `C:\UE\T66\UI\screens\challenges\current\current_state_1920x1080.png`
- Accepted reference: `C:\UE\T66\UI\screens\challenges\reference\challenges_reference_1920x1080.png`
- Raw imagegen output: `C:\UE\T66\UI\screens\challenges\reference\challenges_raw_imagegen_1672x941_v2.png`
- Normalization: deterministic resample to 1920x1080 with `C:\UE\T66\Scripts\InvokeDeterministicResample.py`

## Style Sources

- Main-menu anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- 4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
- Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
- Imagegen chrome sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Layout And Rule Handling

- Current screenshot existed: yes
- Top bar: included as fixed shared chrome at the top.
- Back button handling: removed the current red `X` close/back affordance because this is a main-menu/top-bar child screen.
- Replaced back affordance: no
- Preserved components: title, `Official`, `Community`, `Create Challenge`, `Challenges`, `Mods`, status line, two challenge rows, selected challenge detail panel, two requirements, reward block, and `Confirm`.
- Explicitly not added: extra rows, weekly/expert/legacy labels, filters, sidebars, extra currencies, footer navigation, or replacement back controls.

## Runtime-Owned Regions To Preserve Later

- Challenge row data, selected challenge details, rule text, reward text, tags, and confirm action remain runtime-owned.
- Empty icon slots are placeholders for runtime challenge art or state.
- The shared top bar should remain shared chrome rather than duplicated per screen.
