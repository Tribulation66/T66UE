# T66 Master UI Asset Library

This folder is the Part 0 output for `C:\UE\T66\Docs\UI\UI_RECONSTRUCTION_LOCKED_PROCESS.md`.

## Locked Font

- Font: Jersey 10
- Runtime path: `C:\UE\T66\RuntimeDependencies\T66\Fonts\Jersey10-Regular.ttf`
- License: `C:\UE\T66\RuntimeDependencies\T66\Fonts\Jersey10-OFL.txt`
- Rule: workers must not choose substitute fonts.

## Runtime Assets

- Individual runtime slices: `Slices\`
- Packed sprite sheets: `SpriteSheets\`
- Catalog and nine-slice metadata: `component_catalog.json`

The preview images under `Preview\` are human catalog previews only. Do not use preview images as runtime art.

## Top Bar Scope

Top-bar assets are valid only for:

- `main_menu`
- `account_status`
- `achievements`
- `settings`
- `language_select`
- `minigames`
- `powerup`

Do not place the global top bar on hero selection, save/load flow, loading screens, run screens, pause screens, casino screens, overlays, modals, HUD, mini screens, or TD screens.

## Asset Rules

- Do not create per-screen sprite sheets.
- Do not bake localizable labels into runtime chrome.
- Do not use a full-screen reference as a runtime background.
- If a needed component is missing, return to Part 0 and add it here. Do not invent screen-local chrome.
