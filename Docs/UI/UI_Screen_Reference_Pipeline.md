# UI Screen Reference Pipeline

## Goal

Standardize how future UI screens are prepared for reconstruction so layout authority, art generation, slicing, and runtime validation do not fight each other.

The key rule is simple:

- do not use one flattened screenshot as both the layout spec and the production asset source

Instead, every reconstructed screen should have three distinct artifact layers:

1. `layout master`
2. `hi-res art master`
3. `runtime family sheets`

## Standard Artifact Model

### 1. Layout master

The layout master is the canonical source of truth for measurement, placement, and diffing.

- use one fixed canvas size per screen
- default to `1920x1080` for future 16:9 screens
- if the screen uses a different aspect ratio, lock it once and keep it fixed everywhere
- use this artifact to derive:
  - `reference_layout.json`
  - generated layout headers
  - strict screenshot diffs

This file is not the primary runtime asset source.

### 2. Hi-res art master

The hi-res art master is the same screen composition at a larger scale for asset extraction and family generation.

- preferred scale: `2x`
- recommended for 16:9: `3840x2160`
- optional helper scale for very paint-heavy screens: `7680x4320`

This file can be created by:

- exporting from the original design source
- a deterministic layout tool plus painted overlays
- a tightly controlled image-generation pass that preserves the exact composition of the approved layout master

If image generation is used here, the hi-res version is an art helper only. The layout master remains the placement authority.

### 3. Runtime family sheets

Runtime family sheets are isolated boards for reusable chrome and control states.

Typical families:

- `TopBar`
- `Center`
- `LeftPanel`
- `RightPanel`
- `Decor`

Common requirements:

- no baked localizable text
- no baked numerals for live values
- state coverage where needed:
  - `normal`
  - `hover`
  - `pressed`
  - `disabled`
  - `selected`

## Standard Tool Combination

Use a small pipeline, not one tool.

### Layout authority

Use a deterministic layout source such as:

- Figma
- a locked design file
- an approved handoff screenshot exported at a fixed canvas

This produces the `layout master`.

### Art generation

Use `image_gen` for:

- painted runtime family boards
- hi-res companion art when the original source is too low-resolution for clean extraction
- isolated icon and chrome families

Do not ask one image-generation board to solve multiple unrelated size classes. Generate families at the measured target proportions from the layout manifest.

### Reference prep / upscale

Use `ui-reference-prep` and the local helper stack when the blocker is reference usability rather than structure.

See:

- `C:\UE\T66\Docs\UI\UI_REFERENCE_PREP_WORKFLOW.md`
- `C:\UE\T66\Scripts\SetupFreeUIUpscaleStack.ps1`
- `C:\UE\T66\Scripts\InvokeFreeUpscale.ps1`

Use this lane for deterministic `2x/4x` exports and helper-only AI upscales. Do not treat it as a replacement for measured layout or native-proportion family generation.

### Repo scripts

Use local scripts for:

- stamping out the folder structure
- generating `reference_layout.json`
- generating `T66...ReferenceLayout.generated.h`
- slicing sprite sheets
- producing `no_buttons`, `no_text`, and `no_dynamic` export variants

### Runtime validation

Use packaged-build screenshots for validation.

- compare against the locked layout master
- diff strict static regions
- validate dynamic/live regions separately

## Production Rules

### Resolution rules

- one canonical canvas per screen
- one hi-res companion at the same aspect ratio
- all manifest boxes recorded in canonical reference pixels
- runtime transforms convert `reference pixels -> widget pixels` through one transform path

### Asset rules

- real runtime controls must use real runtime plates
- do not ship hotspot overlays as the real interaction model
- keep labels as `FText`
- do not bake localizable words into button art
- do not repair text-bearing screenshot crops as the long-term production method

### Stretching rules

Every runtime plate must be one of these:

- exact-size plate
- uniformly downscaled plate
- a true nine-slice plate authored to stretch

Never independently distort `X` and `Y` on a painted plate just to fit a measured box.

## Standard Folder Layout

For a screen token like `SettingsMenu`, use:

```text
SourceAssets/UI/SettingsMenuReference/
  asset_manifest.json
  reference_layout.json
  README.md
  debug/
  SpriteSheets/
  SheetSlices/
    TopBar/
    Center/
    LeftPanel/
    RightPanel/
    Decor/
  TopBar/
  Center/
  LeftPanel/
  RightPanel/
  Decor/

Docs/UI/PromptPacks/SettingsMenuSpriteSheets/
  screen_intake.md
  master_frame_prompt.txt
  topbar_sheet_prompt.txt
  center_sheet_prompt.txt
  left_panel_sheet_prompt.txt
  right_panel_sheet_prompt.txt
  decor_sheet_prompt.txt

Source/T66/UI/Style/T66SettingsMenuReferenceLayout.generated.h
```

## Required Screen Exports

Every screen reference pack should aim to contain:

- `screen_master.png`
- `screen_master_no_buttons.png`
- `screen_master_no_text.png`
- `screen_master_no_dynamic.png`
- `reference_layout.json`
- `asset_manifest.json`
- family boards for all major chrome families

If a variant cannot be produced immediately, track it in the screen intake file instead of silently skipping it.

## Recommended Workflow

1. Scaffold the screen pack and prompt pack.
2. Lock the canonical canvas.
3. Export or create the `layout master`.
4. Optionally run reference prep for deterministic `2x/4x` exports or helper-only AI upscales.
5. Export or create the hi-res companion at the same aspect ratio.
6. Partition the screen into families and live regions.
7. Generate family boards at measured target proportions.
8. Slice and stage runtime assets.
9. Rebuild with real widgets.
10. Validate in packaged build.

## Scaffold Script

Use:

```powershell
python Scripts/ScaffoldUIScreenReference.py "Settings Menu" --canvas-width 1920 --canvas-height 1080
```

This creates:

- the reference source-asset folder tree
- the prompt-pack folder tree
- a starter `reference_layout.json`
- a starter generated-layout header
- a screen intake document
- family-specific prompt files

Use `--dry-run` to inspect the planned output before writing files.
