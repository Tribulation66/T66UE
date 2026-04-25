# UI Screen Reference Pipeline

## Goal

Standardize how future UI screens are prepared for reconstruction so layout authority, art generation, slicing, and runtime validation do not fight each other.

The key rule is simple:

- do not use one flattened screenshot as both the offline target and the production runtime background

Instead, every reconstructed screen should have distinct artifact layers:

1. `offline comparison target`
2. `hi-res helper/reference art`
3. `UI-free scene/background plate`
4. `foreground runtime component families`

And one required companion artifact:

5. `content ownership audit`

Hard asset rule: generated UI assets must come correct from image generation. After generation, do not manually pixel-edit, clean up, mask, erase/fill, cover-patch, clone, repaint, or repair screenshots/assets. The permitted post-generation operations are deterministic slicing/cropping from an accepted generated board, center-crop plus Lanczos resize for reference-canvas normalization, deterministic format/export steps, and runtime placement with live text/content overlays. If the generated pixels are wrong, regenerate the board, plate, or family.

## Non-Negotiable Per-Screen Reference Gate

Every target screen, modal, HUD overlay, tab, or mini-game UI must first receive a generated full-screen reference image for that exact target. This is required even when the code already has a styled first pass.

Generate the target reference by giving image generation all three inputs:

1. Canonical main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
2. Current screenshot of the exact target screen, modal, HUD overlay, tab, or mini-game UI.
3. Layout list for the exact target screen, including regions, controls, panels, live-data wells, tab/modal variants, and required states.

The current screenshot is authoritative for layout, content count, and arrangement. The main-menu reference is only the style anchor. Do not add ability slots, idol slots, extra buttons, invented panels, icons, meters, currencies, menu entries, tabs, explanatory labels, or any structure not present in the current screenshot and layout list.

Style direction for new references is sleek, modern, minimalist, clean planar surfaces, crisp borders, flat/satin metallic accents, restrained gold, and a clean red/black/charcoal scheme with the same font/layout/content. Negative guardrails: no grain, no cracked stone, no gemstone/crystal/beveled fantasy surface, no noisy distressed panels, no rubble texture, and no micro-detail borders.

Save the approved result to:

```text
C:\UE\T66\UI\screens\<screen_slug>\reference\<screen_slug>_reference_1920x1080.png
```

Nothing downstream may start without this file. Do not generate sprites, slice assets, patch Slate code, or run packaged diffing from a general style description alone. If the reference misses the layout or visual family, regenerate it.

Deprecated or stale targets are not valid generation tasks. Do not generate active references for `wheel_overlay`, `party_size_picker`, `casino_overlay`, `gambler_overlay`, `vendor_overlay`, standalone `leaderboard`, legacy `Lobby`, `LobbyReadyCheck`, `LobbyBackConfirm`, `HeroLore`, or `CompanionLore`. Use `powerup` instead of `shop`, and `minigames` instead of `unlocks`.

Generating this reference is not completion. It is only the first gate. A screen pass must continue through content ownership, element/state checklist, sprite-family generation, slicing/staging, runtime implementation, packaged capture, and packaged review unless a concrete blocker is recorded.

## Standard Artifact Model

The main menu is the calibration target for this whole model. It must prove the strict workflow first: canonical packaged capture, ownership audit, manifest rect separation, validated runtime assets, and masked packaged diff. New screens inherit the process only after they can express the same contract.

### 1. Offline comparison target

The offline comparison target is the canonical source of truth for measurement, placement, style matching, and packaged diffing.

- use one fixed canvas size per screen
- default to `1920x1080` as the 16:9 authoring and baseline review canvas
- if the screen uses a different aspect ratio, lock it once and keep it fixed everywhere
- for the active main menu pack, approve only a normalized `1920x1080` canonical frame
- never promote raw non-canonical generated output directly as a production reference, sprite sheet, scene plate, slice, or runtime asset
- archive raw imagegen outputs and record normalization metadata before promoting a normalized reference
- reject and regenerate wrong-composition or structurally cropped outputs instead of converting or salvaging them
- use this artifact to derive:
  - `reference_layout.json`
  - generated layout headers
  - strict screenshot diffs

This file is not a runtime asset source. The full reference screenshot must never be shipped as a runtime background layer.

### 2. Hi-res helper/reference art

The hi-res helper/reference art is the same screen composition at a larger scale for inspection, prompting, and family generation.

- preferred scale: `2x`
- recommended for 16:9: `3840x2160`
- optional helper scale for very paint-heavy screens: `7680x4320`

This file can be created by:

- exporting from the original design source
- a deterministic layout tool plus painted overlays
- a tightly controlled image-generation pass that preserves the exact composition of the approved offline target

If image generation is used here, the hi-res version is an art helper only. The offline comparison target remains the placement authority, and the hi-res helper does not become a runtime layer.

### 3. UI-free scene/background plate

The scene/background plate is the only full-screen production plate, and it must be UI-free.

It may contain:

- environment art
- atmospheric background
- non-interactive scenic framing that is not UI chrome

It must not contain:

- buttons, top bars, side panels, leaderboard rows, CTA stacks, labels, text, values, portraits, icons, avatars, media, or preview content
- buttonless or textless remnants of full-screen UI chrome that foreground runtime components are expected to own

Acceptance gate:

- plate is generated as a scene-only layer
- ownership audit confirms no foreground UI region is baked into the plate
- packaged review masks validate the plate only where it remains visible behind foreground components

### 4. Foreground runtime component families

Foreground runtime component families are isolated boards for reusable chrome and control states.

Typical families:

- `TopBar`
- `Center`
- `LeftPanel`
- `RightPanel`
- `LeaderboardChrome`
- `CTAStack`
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

Each screen intake must include an element checklist before image generation starts. The checklist names every required plate, shell, icon, control, aperture, and family board, plus the required `normal`, `hover`, `pressed`, `disabled`, and `selected` states for each stateful element. Asset generation continues until that checklist is complete and validated; placement work does not begin while required elements or states are still missing.

### 5. Content ownership audit

Every screen pack should also include:

- `content_ownership.json`

This file records which visible regions are owned by runtime text, images, icons, avatars, media, or preview stages so generation prompts and review masks do not treat them as fixed art.

Record all runtime text, images, and values, not just obviously dynamic art. Taglines, labels, names, ranks, scores, prices, keybinds, toggles, selected values, progress fills, and state text are runtime-owned unless the intake explicitly marks them as approved baked display art. For the main menu, the title wordmark may be baked art, but the tagline remains live/localizable text.

## Standard Tool Combination

Use a small pipeline, not one tool.

### Layout authority

Use a deterministic layout source such as:

- Figma
- a locked design file
- an approved handoff screenshot exported at a fixed canvas

This produces the offline comparison target.

### Art generation

Use native Codex `image_gen` only for:

- screen-specific full-screen references
- painted runtime family boards
- hi-res companion art when the original source is too low-resolution for clean inspection or prompting
- UI-free scene/background plates when runtime needs a full-screen environment layer
- isolated icon and chrome families

Do not use legacy browser-automation generation, request manifests, token-driven local services, or removed external image-generation tooling. If native generation cannot produce the needed artifact, mark the screen blocked and name the artifact instead of using a external-tool fallback.

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
- producing `no_buttons`, `no_text`, and `no_dynamic` export variants for offline analysis or helper prompts only

### Runtime validation

Use packaged-build screenshots for validation.

- compare the full packaged composition against the locked offline target
- capture at the canonical authoring target, normally `1920x1080`
- validate supported runtime aspect buckets after the baseline comparison: primary `16:9`, `16:10`, ultrawide `21:9`, and one smaller/windowed size where supported
- diff strict static regions
- verify the scene plate separately for contamination by foreground UI chrome
- validate dynamic/live regions separately
- save or describe masks for runtime-owned interiors

### Content ownership audit

Use a code-first pass to identify:

- runtime text
- runtime images
- runtime icons
- runtime avatars
- runtime media
- runtime 3D previews

Then record both:

- shell rects
- visible control rects
- live-content rects

before generating family boards or packaged-review masks.

## Production Rules

### Resolution rules

- one canonical authoring canvas per screen
- default `1920x1080` for normal 16:9 screen references
- supported runtime viewports are reached through anchors, safe zones, DPI scaling, stretch rules, and one documented transform path
- validate against aspect buckets instead of building bespoke art for every common resolution
- one hi-res companion at the same aspect ratio
- all manifest boxes recorded in canonical reference pixels
- runtime transforms convert `reference pixels -> widget pixels` through one transform path

### Asset rules

- real runtime controls must use real runtime plates
- the full reference screenshot, buttonless master, and textless master are not runtime backgrounds
- production full-screen background art must be a UI-free scene/background plate
- do not ship hotspot overlays as the real interaction model
- do not manually repair generated pixels; regenerate bad plates, boards, or families
- keep labels as `FText`
- do not bake localizable words into button art
- do not repair text-bearing screenshot crops as the long-term production method
- do not paint runtime-owned portraits, icons, media, or preview content into shell art
- validate asset dimensions against manifest slots before integration
- validate transparent padding and alpha edges before slicing/import
- validate state anchors so hover, pressed, disabled, and selected states do not jump
- record nine-slice margins for every stretchable plate

### Stretching rules

Every runtime plate must be one of these:

- exact-size plate
- uniformly downscaled plate
- a true nine-slice plate authored to stretch

Never independently distort `X` and `Y` on a painted plate just to fit a measured box.

Nine-slice plates must declare their stable border margins in the asset manifest or screen intake. If the corners, bevels, or state anchors distort under the intended runtime size, the asset is rejected and regenerated at the correct proportion.

## Standard Folder Layout

For a screen slug like `settings`, use the canonical UI workspace:

```text
UI/screens/settings/
  reference/
    <screen_slug>_reference_1920x1080.png
    optional_helper_2x.png
  current/
    YYYY-MM-DD/
  layout/
    layout_list.md
    reference_layout.json
  assets/
    asset_manifest.json
    content_ownership.json
    SpriteSheets/
    SheetSlices/
    scene_plate.png
  outputs/
    YYYY-MM-DD/
      packaged_capture.png
      diff_notes.md
  review/
    review_notes.md

SourceAssets/UI/SettingsMenuReference/
  final runtime-imported assets only when code/import tooling expects them there
  asset_manifest.json
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
  scene_plate_prompt.txt
  topbar_sheet_prompt.txt
  center_sheet_prompt.txt
  left_panel_sheet_prompt.txt
  right_panel_sheet_prompt.txt
  leaderboard_chrome_prompt.txt
  decor_sheet_prompt.txt

Source/T66/UI/Style/T66SettingsMenuReferenceLayout.generated.h
```

## Required Screen Exports

Every screen reference pack should aim to contain:

- `screen_master.png` as the offline comparison target
- raw imagegen source for the accepted reference, archived or stored beside the normalized output
- normalization metadata when the accepted reference came from a non-1920 raw source: source size, crop rect, target size, resampling method
- optional `screen_master_no_buttons.png`, `screen_master_no_text.png`, and `screen_master_no_dynamic.png` for analysis or prompting only
- `scene_plate.png` or equivalent UI-free background plate
- `content_ownership.json`
- `reference_layout.json`
- `asset_manifest.json`
- family boards for all major foreground chrome families
- packaged capture at the canonical target size
- responsive/aspect validation captures or notes
- diff masks or mask notes for runtime-owned regions

If a variant cannot be produced immediately, track it in the screen intake file instead of silently skipping it.

## Recommended Workflow

1. Scaffold the screen pack under `C:\UE\T66\UI\screens\<screen_slug>`.
2. Lock the canonical canvas.
3. Capture the current runtime target screenshot.
4. Write the target layout list.
5. Generate the offline comparison target from the canonical main-menu anchor, current target screenshot, and layout list.
6. If the raw imagegen output is landscape-safe but not `1920x1080`, archive the raw source and normalize a copy with `InvokeDeterministicResample.py --target-width 1920 --target-height 1080`.
7. Reject and regenerate any output whose normalization crops important layout, title, UI, character, or content.
8. Optionally run reference prep for deterministic `2x/4x` exports or helper-only AI upscales.
9. Export or create the hi-res companion at the same aspect ratio.
10. Audit content ownership from code and record `content_ownership.json`.
11. Fill the element checklist with every required plate, shell, icon, control, aperture, family board, and required state.
12. Partition the screen into the UI-free scene plate, foreground component families, and live regions.
13. Generate the scene plate and verify it contains no foreground UI.
14. Generate family boards at measured target proportions until the checklist is complete.
15. Slice and stage runtime assets.
16. Validate dimensions, alpha, nine-slice margins, state anchors, and ownership masks.
17. Rebuild with real widgets and layered foreground components only after the checklist is complete.
18. Validate in packaged build with strict diffs, ownership masks, and supported aspect-bucket captures.
19. Only then report the screen pass complete. If blocked earlier, report `blocked` with the exact missing artifact or failing command.

## Scaffold Script

Use:

```powershell
python Scripts/ScaffoldUIScreenReference.py "Settings Menu" --canvas-width 1920 --canvas-height 1080
```

This creates:

- the canonical `UI\screens\<screen_slug>` folder tree
- `reference`, `current`, `layout`, `assets`, `outputs`, `review`, and `prompts` folders
- starter `layout\layout_list.md` and `layout\reference_layout.json`
- starter `assets\asset_manifest.json` and `assets\content_ownership.json`
- a starter generated-layout header
- a screen intake document
- family-specific prompt files

Use `--dry-run` to inspect the planned output before writing files.

