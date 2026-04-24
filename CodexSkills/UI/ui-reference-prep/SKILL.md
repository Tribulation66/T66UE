---
name: ui-reference-prep
description: Prepare helper reference images before UI reconstruction by deciding between plain resample and AI upscale, producing 2x or 4x helper references, and classifying outputs as helper-only, runtime-safe, or do-not-slice. Use when Codex must make approved UI references easier to inspect, compare, crop, or derive assets from before layout manifesting or sprite-family generation.
---

# UI Reference Prep

Prepare better working references before reconstruction. This skill exists to make a locked screen easier to measure and inspect without confusing helper images with production-safe runtime art.

## Open These First

- `C:\UE\T66\Docs\UI\MASTER.md`
- `C:\UE\T66\Docs\UI\UI_CONTENT_OWNERSHIP_AUDIT.md`
- `C:\UE\T66\Docs\UI\UI_SKILL_ARCHITECTURE.md`
- `C:\UE\T66\Docs\UI\UI_Screen_Reference_Pipeline.md`
- `C:\UE\T66\Docs\UI\UI_REFERENCE_PREP_WORKFLOW.md`
- `C:\UE\T66\Scripts\SetupFreeUIUpscaleStack.ps1`
- `C:\UE\T66\Scripts\InvokeFreeUpscale.ps1`
- `C:\UE\T66\Scripts\InvokeDeterministicResample.py`
- `C:\UE\T66\Docs\UI\UI_RECONSTRUCTION_HANDOFF.md` when the main menu is the active screen

## Core Job

Produce higher-utility reference images for downstream UI work.

Reference prep does not change the acceptance target. Normal 16:9 screens still measure and validate against the canonical packaged frame, normally `1920x1080`. The main menu remains the golden calibration screen; helper images must not blur the title/tagline ownership contract or any shell/control/live-content separation.

Prepared full-screen references, including no-buttons or no-text variants, remain offline/helper artifacts. They do not become runtime background plates unless a later `$ui-sprite-families` pass explicitly validates them as UI-free scene/background plates.

Hard rule: reference prep must not manually pixel-edit, clean up, mask, erase/fill, cover-patch, clone, repaint, or screenshot-repair generated assets. It may create deterministic resamples, helper upscales, crop guides, and classification notes. If pixels need correction, route to generation and regenerate.

Typical outputs:
- `*_2x.png`
- `*_4x.png`
- optional AI-upscaled helper variants
- a short classification note for each output:
  - `helper-only`
  - `runtime-safe`
  - `do-not-slice`

## Decision Rule

Choose the least aggressive operation that solves the problem.

### Use plain resample when:

- exact shape and placement fidelity matter more than added detail
- the image is already structurally correct and only needs to be larger for inspection
- the output is mainly for layout work, diffing, or box measurement
- a deterministic canvas export is needed from Figma or a local image tool

Plain resample is the default safe option.

### Use AI upscale when:

- the team needs a cleaner helper view of ornament, edge breakup, painted trim, or material detail
- the output is only meant to guide manual reconstruction or family-board prompting
- the source is structurally correct but too low-resolution to inspect comfortably

AI upscale is for helper references, not for automatically trusted runtime slices.

### Do not upscale. Regenerate or re-author instead when:

- the source crop contains baked localizable text
- the control plate has the wrong aspect ratio
- the source is contaminated by neighboring art or background
- the source is a buttonless/textless full-screen composite that still contains foreground UI chrome
- the active main menu asset was generated at a non-canonical canvas
- the target needs true stateful runtime art such as normal, hover, or pressed buttons
- the issue is ownership or composition, not pixel density

If the plate or shell is fundamentally wrong, move to `$ui-style-reference` or `$ui-sprite-families` instead of trying to rescue it with upscale.
For the active main menu pack, delete and rebuild wrong-resolution generated assets at `1920x1080`; do not resample them into compliance.

## Workflow

### 1. Lock the intent of the helper image

State which of these is needed:
- layout helper
- visual inspection helper
- family-board prompting helper
- possible runtime-safe source

If the purpose is unclear, assume `layout helper` and use plain resample.

### 2. Inspect the source before touching it

Check for:
- baked text
- mixed ownership between background and controls
- contamination from nearby panels or scenery
- shape distortion already present in the source

Record the risk briefly. A larger bad source is still a bad source.

### 3. Choose the prep method

#### Plain resample path

- Prefer this first for approved full-screen references.
- Use deterministic scaling such as `2x` or `4x`.
- Keep the original aspect ratio exactly.
- Use Figma only as a controlled export or canvas tool, not as a true detail-restoration tool.

#### AI upscale path

- Use only when extra visual readability is needed.
- Keep the composition identical to the approved reference.
- Produce separate files from the original so the source of truth remains untouched.
- Treat outputs as provisional until classified.
- Preserve transparent regions and alpha edges when preparing already-isolated assets.

If a free local stack exists, prefer it before paid tools. Typical no-cost choices are Real-ESRGAN or Upscayl. If only simple resizing is available, fall back to plain resample and be explicit about the limitation.

### 4. Produce standard helper sizes

Default targets:
- `2x` for routine inspection and prompting
- `4x` for close crop analysis or ornament study

Do not invent non-uniform scaling. Keep the same aspect ratio as the source.

### 5. Classify every output

Every prepared image must be labeled with one of these classes:

#### `helper-only`

Use when the image is useful for:
- inspection
- prompting
- visual comparison
- manual crop guidance

But it is not safe to slice directly into runtime assets.

This is the expected class for most AI-upscaled composites.

#### `runtime-safe`

Use only when:
- the image contains no baked localizable text
- the crop is clean and isolated
- the shape is already correct
- alpha edges and transparent padding are clean
- nine-slice margins and state anchors remain trustworthy when applicable
- no neighboring contamination is present
- the asset is suitable for direct slicing or import without interpretive cleanup

This class should be rare for full-screen composites and more common for already-clean family boards.

#### `do-not-slice`

Use when the image is actively unsafe for runtime extraction, including:
- text baked into art
- incorrect proportions
- obvious AI drift
- polluted borders
- merged background/control ownership

When in doubt between `helper-only` and `runtime-safe`, choose `helper-only`.

### 6. Hand off clearly

Hand off:
- original source path
- prepared output paths
- scale used
- classification for each output
- short note on why that classification was chosen

Then route to the next skill:
- `$ui-layout-manifest` for box measurement from a stable layout image
- `$ui-sprite-families` for clean runtime asset generation
- `$ui-style-reference` if the source itself needs a cleaner or more faithful master image

## Guardrails

- Do not replace the approved reference as the layout authority unless the task explicitly changes the source of truth.
- Do not treat Figma export scaling as AI super-resolution.
- Do not mark an image `runtime-safe` just because it is larger.
- Do not try to fix bad proportions with upscale.
- Do not use helper images as silent production inputs; classify them first.
- Do not mark full-screen reference variants as runtime backgrounds unless they pass the UI-free scene plate gate.
- Do not crop or clean pixels by hand; regenerate a clean family board when pixels are wrong.
- Do not use upscale to rescue mixed ownership problems that should be handled by the content ownership audit.
- Do not mark prepared assets `runtime-safe` without checking dimensions, alpha, nine-slice requirements, and state anchors.

## Success Bar

The prepared references should make downstream work easier while reducing confusion about what is safe to measure, prompt from, or slice into runtime assets.
