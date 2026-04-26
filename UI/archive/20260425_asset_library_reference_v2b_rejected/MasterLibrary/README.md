# T66 Master UI Asset Library V2B

This is the reference-derived Part 0 output for `C:\UE\T66\Docs\UI\UI_RECONSTRUCTION_LOCKED_PROCESS.md`.

The first procedural library was archived to `C:\UE\T66\UI\archive\20260425_asset_library_procedural_v1`.

## Font

- Locked font: Jersey 10
- Runtime path: `C:\UE\T66\RuntimeDependencies\T66\Fonts\Jersey10-Regular.ttf`
- Workers must not choose substitute fonts.

## Runtime Chrome

- Transparent slices: `Slices\`
- Packed atlases: `SpriteSheets\`
- 9-slice metadata: `component_catalog.json`
- Proof sheets: `Preview\reference_derived_library_proof.png` and `Preview\reference_derived_library_preview.png`

Chrome assets are derived from `UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`. They are not procedural placeholders. Interior areas are repainted so runtime text/data remains live.

## Icons

- Imagegen review atlas: `IconGeneration\icon_sprite_sheet_imagegen_20260425_v1.png`
- Transparent icon slices: `Slices\IconsGenerated\`

## Hard Rules

- Do not bake text into runtime chrome.
- Do not use full-screen references as runtime backgrounds.
- Use these slices through Slate/UMG box brushes with the catalogued margins.
- If a component is missing, add it to this library instead of making screen-local chrome.
