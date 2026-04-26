# Reference Generation Notes

- Chat number: 2
- Screen slug: `report_bug`
- Source current screenshot: `C:\UE\T66\UI\screens\report_bug\current\current_state_1920x1080.png`
- Source screenshot state: existed before generation
- Accepted reference: `C:\UE\T66\UI\screens\report_bug\reference\report_bug_reference_1920x1080.png`
- Archived previous reference: `C:\UE\T66\UI\archive\reference_generation_v1_20260425\report_bug\report_bug_reference_1920x1080.png`
- Archived previous notes: `C:\UE\T66\UI\archive\reference_generation_v1_20260425\report_bug\reference_generation_notes.md`
- Main-menu anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Main-menu 4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
- Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
- Global top-bar 4K helper: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_3840x280.png`
- Chrome helper sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`
- Chrome prompt source: `C:\UE\T66\Docs\UI\PromptPacks\MainMenuSpriteSheets\chrome_sheet_imagegen_prompt_20260425_v1.txt`
- Raw native image output: `C:\Users\DoPra\.codex\generated_images\019dc448-af5d-72b1-8abf-524ea1622a8b\ig_0f05a5b534baab010169ecee5480b8819a89ce1290edb80cbf.png`
- Normalization: deterministic resample from `1672x941` to `1920x1080`
- Status: accepted

## Required Handling

- Global top bar: preserved only as dimmed background chrome from the modal context; no new modal top bar added.
- Generic Back button: not added.
- Fields/buttons: preserved exactly as one multiline field, `SUBMIT`, and `CANCEL`.
- Deprecated/deferred target: no.
- Generated only a full-screen offline reference; no sprite sheets, runtime code, layout manifest, or packaged review were created.

## Runtime-Owned Regions To Preserve Later

- report text entered by player
- submit/cancel button states
- dimmed main-menu background live UI

## Integrator Notes

- Current screenshot remained authority for modal footprint and form-control count.
- No severity, attachment, email, checkbox, or extra form controls were added.
