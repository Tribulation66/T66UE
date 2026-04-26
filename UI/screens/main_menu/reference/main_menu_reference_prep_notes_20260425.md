# Main Menu Reference Prep Notes - 2026-04-25

## Source

- Accepted raw imagegen source: `C:\UE\T66\UI\archive\main_menu_reference_pre_final_20260425\main_menu_final_imagegen_raw_1672x941_20260425.png`
- Previous canonical reference archived at: `C:\UE\T66\UI\archive\main_menu_reference_pre_final_20260425\main_menu_reference_1920x1080_previous.png`

## Active Outputs

- Canonical layout/reference anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
  - Size: `1920x1080`
  - Method: deterministic center crop to 16:9 plus Lanczos normalization.
  - Classification: canonical offline reference.
- AI-upscaled helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
  - Size: `3840x2160`
  - Method: crop accepted raw source to exact 16:9, Real-ESRGAN NCNN Vulkan with `realesrgan-x4plus-anime` at native `4x`, then Lanczos downsample to exact 4K.
  - Classification: helper-only for inspection, prompting, and chrome/style continuation.

## Sprite Sheet Helpers

- Global top-bar reference sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
- Global top-bar 4K helper sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_3840x280.png`
- Imagegen chrome sprite sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`
- Imagegen source copy: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1.png`
- Imagegen prompt: `C:\UE\T66\Docs\UI\PromptPacks\MainMenuSpriteSheets\chrome_sheet_imagegen_prompt_20260425_v1.txt`
- Legacy deterministic crop sheet archive: `C:\UE\T66\UI\archive\main_menu_reference_pre_final_20260425\debug_chrome_crop_sheet\mainmenu_chrome_reference_sheet_20260425_4k.png`
- Legacy deterministic crop manifest archive: `C:\UE\T66\UI\archive\main_menu_reference_pre_final_20260425\debug_chrome_crop_sheet\mainmenu_chrome_reference_sheet_20260425_4k_manifest.json`
- Classification: helper-only until a later slicing pass validates dimensions, alpha, nine-slice margins, and state anchors.

Use the imagegen chrome sprite sheet as the preferred visual reference for future sprite-family and screen-reference prompting. The legacy deterministic crop sheet is retained only as a coordinate/debug artifact and should not be used as the style sheet for worker chats.

Use the global top-bar sprite as the fixed reference for main-menu child screen mockups. Future screens should treat the top bar as shared global chrome; screen-specific composition begins below it and should not redesign or own that bar.

The imagegen chrome sprite sheet is cleaner than the deterministic crop sheet, but it is still a helper board. Do not treat generated components as final runtime slices until a dedicated slicing pass validates them. Runtime text, values, avatars, and localizable labels still need live widget ownership.

## Rules For Downstream Chats

- Use `main_menu_reference_1920x1080.png` as the layout/style anchor.
- Use the 4K helper and chrome sheet for sharp detail inspection and prompting support.
- Use `topbar_global_reference_sprite_1920x140.png` as the reusable top-bar reference for screen mockups.
- Do not pull from `C:\UE\T66\UI\archive` unless the integrator explicitly asks for provenance.
- Do not use the full reference or chrome helper sheet as a runtime background.
- Do not slice helper crops that contain baked text, values, avatars, player names, scores, icons, or live data.
