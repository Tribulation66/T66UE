# Reference Generation Notes

- Chat number: Chat 1
- Source current screenshot path: `C:\UE\T66\UI\screens\language_select\current\current_state_1920x1080.png`
- Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- 4K helper path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
- Global top-bar reference sprite path: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
- Chrome helper sheet path: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`
- Current screenshot existed: yes
- Back button removed: yes
- Raw generated source: none
- Canonical reference: `C:\UE\T66\UI\screens\language_select\reference\language_select_reference_1920x1080.png`
- Runtime-owned regions to preserve later: title, language names, selected language state, confirm label, top navigation labels, currency amount, and scroll state.
- Reference status: accepted
- Notes for integrator: this reference keeps the current centered language-list structure, removes the standalone Back button, uses the shared global top bar, and restyles the body shell/buttons in the accepted V2 purple/charcoal chrome. The full PNG is an offline target only; runtime implementation must use real widgets and live FText.
