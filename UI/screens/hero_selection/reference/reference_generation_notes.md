# Reference Generation Notes

- Chat number: 2
- Screen slug: `hero_selection`
- Source current screenshot: `C:\UE\T66\UI\screens\hero_selection\current\current_state_1920x1080.png`
- Source screenshot state: existed before generation
- Accepted reference: `C:\UE\T66\UI\screens\hero_selection\reference\hero_selection_reference_1920x1080.png`
- Archived previous reference: `C:\UE\T66\UI\archive\reference_generation_v1_20260425\hero_selection\hero_selection_reference_1920x1080.png`
- Archived previous notes: `C:\UE\T66\UI\archive\reference_generation_v1_20260425\hero_selection\reference_generation_notes.md`
- Main-menu anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Main-menu 4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
- Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
- Global top-bar 4K helper: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_3840x280.png`
- Chrome helper sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`
- Chrome prompt source: `C:\UE\T66\Docs\UI\PromptPacks\MainMenuSpriteSheets\chrome_sheet_imagegen_prompt_20260425_v1.txt`
- Raw native image output: `C:\Users\DoPra\.codex\generated_images\019dc448-af5d-72b1-8abf-524ea1622a8b\ig_0de50fc64c2798b60169ecea0c72088198aa3b2296297f6d19.png`
- Normalization: deterministic resample from `1672x941` to `1920x1080`
- Status: accepted

## Required Handling

- Global top bar: applied as fixed shared chrome at the top.
- Standalone Back button: removed; no replacement back arrow or footer added.
- Deprecated/deferred target: no.
- Generated only a full-screen offline reference; no sprite sheets, runtime code, layout manifest, or packaged review were created.

## Runtime-Owned Regions To Preserve Later

- center hero preview/model stage
- top hero carousel thumbnails and selected/hover states
- hero name, skin rows, equipped/preview/buy state text, and currency value
- right gameplay video/screenshot well
- medal/stat values, ability icon sockets, difficulty dropdown, and enter button label/state
- bottom party slots, companion controls, player names, and all labels/values

## Integrator Notes

- Current screenshot remained authority for layout, hierarchy, rows, slots, controls, and spacing.
- Main-menu anchor, 4K helper, top-bar sprite, and chrome sheet were used only as style references.
