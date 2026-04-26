# Cowardice Prompt Reference Generation Notes - V2

- Chat number: Chat 3
- Status: accepted with inference
- Source current screenshot: `C:\UE\T66\UI\screens\cowardice_prompt\current\current_state_1920x1080.png`
- Accepted reference: `C:\UE\T66\UI\screens\cowardice_prompt\reference\cowardice_prompt_reference_1920x1080.png`
- Raw imagegen output: none
- Current screenshot existed: no
- Top bar: not applicable
- Back button handling: no standalone Back button; YES/NO buttons are part of the modal confirmation/cancel flow.

## Style Sources

- Main-menu anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- 4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
- Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
- Imagegen chrome sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Runtime-Owned Regions To Preserve Later

- Gameplay/world underlay remains live runtime content.
- Prompt title, status text, YES/NO labels, enabled/hover/pressed state, and interaction behavior remain runtime-owned.
- The full PNG is an offline target only and must not be used as a runtime background shortcut.
