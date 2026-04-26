# Reference Generation Notes

Chat number: 4

Source current screenshot path: `C:\UE\T66\UI\screens\gameplay_hud\current\current_state_1920x1080.png`

Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`

4K helper path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`

Global top-bar reference sprite path: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`

Chrome helper sheet path: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

Current screenshot status: existed.

Back button removed: no. This is an in-run HUD target, not a main-menu child screen.

Top bar handling: no global top bar added; top-bar sprite was style reference only.

Runtime-owned regions to preserve later:
- Gameplay scene, player, crosshair, minimap contents, stage label, score/time values, hearts, hero portrait, ability icon, passive icon, idol slots, inventory slots, item icons, keybinds, cooldowns, currencies, and all values.

Status: needs-regeneration.

Rejected native imagegen attempts:
- `C:\Users\DoPra\.codex\generated_images\019dc448-df2e-7fc3-aa90-0b2e5b00803e\ig_090ad53288e24d8d0169ecdf81165c819ab8dad1b791731881.png`
- `C:\Users\DoPra\.codex\generated_images\019dc448-df2e-7fc3-aa90-0b2e5b00803e\ig_090ad53288e24d8d0169ece2ddc384819a960dda0da6ee447a.png`

Rejection reason:
- Native imagegen changed gameplay composition and/or altered HUD counts by adding inventory contents, changing idol/heart counts, and inventing extra ability sockets. No output was promoted to `gameplay_hud_reference_1920x1080.png`.
