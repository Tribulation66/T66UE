# Reference Generation Notes

Chat number: 4

Source current screenshot path: `C:\UE\T66\UI\screens\gameplay_hudfull_map\current\current_state_1920x1080.png`

Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`

4K helper path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`

Global top-bar reference sprite path: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`

Chrome helper sheet path: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

Current screenshot status: existed.

Back button removed: no. This is an in-run map overlay target, not a main-menu child screen.

Top bar handling: no global top bar added; top-bar sprite was style reference only.

Runtime-owned regions to preserve later:
- Dimmed gameplay HUD behind the overlay, map contents, fog/route data, player marker, close keybind/label, minimap content, stage label, and all underlying HUD values.

Status: needs-regeneration.

Rejected native imagegen attempts:
- `C:\Users\DoPra\.codex\generated_images\019dc448-df2e-7fc3-aa90-0b2e5b00803e\ig_090ad53288e24d8d0169ece6084dc0819ab3cb7170f156bcd2.png`
- `C:\Users\DoPra\.codex\generated_images\019dc448-df2e-7fc3-aa90-0b2e5b00803e\ig_090ad53288e24d8d0169ece7b53138819a9c5e70b8dfe878d4.png`

Rejection reason:
- Native imagegen preserved the rough map concept but shrank and shifted the modal instead of keeping the current screenshot's large full-screen map bounds. No output was promoted to `gameplay_hudfull_map_reference_1920x1080.png`.
