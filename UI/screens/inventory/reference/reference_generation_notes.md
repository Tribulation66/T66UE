# Inventory Reference Generation Notes - V2

- Chat number: Chat 3
- Status: accepted
- Source current screenshot: `C:\UE\T66\UI\screens\inventory\current\current_state_1920x1080.png`
- Accepted reference: `C:\UE\T66\UI\screens\inventory\reference\inventory_reference_1920x1080.png`
- Raw imagegen output: `C:\UE\T66\UI\screens\inventory\reference\inventory_raw_imagegen_1672x941_v2.png`
- Normalization: deterministic resample to 1920x1080 with `C:\UE\T66\Scripts\InvokeDeterministicResample.py`

## Style Sources

- Main-menu anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- 4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
- Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
- Imagegen chrome sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Layout And Rule Handling

- Current screenshot existed: yes
- Top bar: not forced; this is an in-game HUD inventory overlay.
- Back button handling: no standalone back button was generated.
- Replaced back affordance: no
- Preserved components: live 3D scene, top-left score/time panel, center crosshair, top-right minimap and stage label, bottom-left idols/health/portrait/ability cluster, and bottom inventory panel with two rows of ten slots.
- Explicitly not added: menu tabs, filters, search, sort buttons, category chips, extra currencies, extra rows, extra columns, top navigation, back button, footer navigation, or explanatory content.

## Runtime-Owned Regions To Preserve Later

- Gameplay scene, player character, crosshair, score/time values, minimap, stage label, hearts, idols, portrait, ability buttons, currencies, and inventory slots remain runtime-owned.
- Inventory should remain an overlay and must not cover the full gameplay scene with a menu panel.
