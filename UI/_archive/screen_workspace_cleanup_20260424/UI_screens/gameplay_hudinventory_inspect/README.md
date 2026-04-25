# GameplayHUDInventoryInspect Reference Pack

This folder stores the canonical UI reconstruction artifacts used to reconstruct `GameplayHUDInventoryInspect`.

## Canonical Canvas

- generated screen-specific reference target: `1920x1080`
- optional hi-res helper target: `3840x2160`

## Expected Files

- `reference/canonical_reference_1920x1080.png`
- `current/YYYY-MM-DD/`
- `layout/layout_list.md`
- `layout/reference_layout.json`
- `assets/content_ownership.json`
- `assets/asset_manifest.json`
- `assets/SpriteSheets/`
- `outputs/YYYY-MM-DD/`
- `review/`
- sliced family outputs:
- `assets/SheetSlices/TopBar/`
- `assets/SheetSlices/Center/`
- `assets/SheetSlices/LeftPanel/`
- `assets/SheetSlices/RightPanel/`
- `assets/SheetSlices/Decor/`
- promoted runtime assets:
- `assets/TopBar/`
- `assets/Center/`
- `assets/LeftPanel/`
- `assets/RightPanel/`
- `assets/Decor/`

Do not start sprite generation or runtime placement until the screen-specific reference has been generated from the canonical main-menu anchor, the current target screenshot, and the layout list.
