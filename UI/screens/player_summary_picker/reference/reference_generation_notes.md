# Reference Generation Notes

- Chat number: 2
- Screen slug: `player_summary_picker`
- Source current screenshot: `C:\UE\T66\UI\screens\player_summary_picker\current\current_state_1920x1080.png`
- Source screenshot state: existed before generation
- Accepted reference: `C:\UE\T66\UI\screens\player_summary_picker\reference\player_summary_picker_reference_1920x1080.png`
- Archived previous reference: `C:\UE\T66\UI\archive\reference_generation_v1_20260425\player_summary_picker\player_summary_picker_reference_1920x1080.png`
- Archived previous notes: `C:\UE\T66\UI\archive\reference_generation_v1_20260425\player_summary_picker\reference_generation_notes.md`
- Main-menu anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Main-menu 4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
- Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
- Global top-bar 4K helper: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_3840x280.png`
- Chrome helper sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`
- Chrome prompt source: `C:\UE\T66\Docs\UI\PromptPacks\MainMenuSpriteSheets\chrome_sheet_imagegen_prompt_20260425_v1.txt`
- Raw native image output: `C:\Users\DoPra\.codex\generated_images\019dc448-af5d-72b1-8abf-524ea1622a8b\ig_0f05a5b534baab010169ecedb85f78819a84d1b0761781823a.png`
- Normalization: deterministic resample from `1672x941` to `1920x1080`
- Status: accepted

## Required Handling

- Global top bar: preserved only as dimmed background chrome from the modal context; no new modal top bar added.
- Generic Back button: not added.
- Extra controls: not added; modal remains title plus empty-state text only.
- Deprecated/deferred target: no.
- Generated only a full-screen offline reference; no sprite sheets, runtime code, layout manifest, or packaged review were created.

## Runtime-Owned Regions To Preserve Later

- player-slot list content when present
- runtime-owned player portraits and names when present
- empty-state text
- dimmed main-menu background live UI

## Integrator Notes

- Current screenshot remained authority for the compact picker modal footprint and absence of visible rows/buttons.
- Player data remains runtime-owned.
