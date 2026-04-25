# UI Reference Prep Workflow

## Purpose

This document defines the helper workflow for preparing high-resolution UI references before reconstruction.

This is not a replacement for measured layout or native-proportion runtime art generation. It is a support lane used to make reference work easier and cleaner.

Full-screen helper outputs, including buttonless or textless variants, remain offline artifacts. They are not production runtime backgrounds unless a later sprite-family pass explicitly validates them as UI-free scene/background plates.

If the work needs a newly generated reference frame rather than a larger helper image, route to native Codex `image_gen` through `ui-style-reference`. Do not use legacy browser-automation generation as a fallback.

Hard asset rule: reference prep may produce deterministic resamples, helper upscales, crop guides, reference-canvas normalization, and classification notes. It may not manually pixel-edit, clean up, mask, erase/fill, cover-patch, clone, repaint, or repair generated assets or screenshots. If a runtime candidate needs corrected pixels, regenerate it through `ui-sprite-families` or `ui-style-reference`.

## What This Stage Is For

Use reference prep when you need to:

- create `2x` or `4x` helper references from an approved screen
- inspect painterly detail, trims, bevels, and ornament more clearly
- generate a higher-resolution companion image for style continuation work
- prepare helper boards before family reconstruction

Do not use it as the primary fix for:

- wrong runtime plate proportions
- badly framed or structurally wrong generated assets
- contaminated screenshot crops
- baked localizable text
- duplicated shell ownership

For the active main menu pack, raw imagegen outputs that are landscape-safe may be archived and normalized into the canonical `1920x1080` authoring baseline. If the crop would remove important UI/title/content or the composition is wrong, regenerate instead. Do not use upscale as a rescue path for bad composition or ownership.

## Output Labels

Every prepared image should be treated as one of these:

- `layout-export`
  - deterministic enlarged export for measurement or presentation
- `helper-only`
  - good for visual study, not approved for direct runtime slicing
- `runtime-safe`
  - explicitly reviewed and acceptable to slice or ship
- `do-not-slice`
  - useful for inspection only; not safe for production asset extraction

Default assumption:
- plain resamples are `layout-export`
- AI upscales are `helper-only`
- complex composites with baked text are `do-not-slice`
- buttonless/textless full-screen composites are `helper-only` until they pass the UI-free scene plate gate

## No-Cost Stack

The current free stack is:

- Real-ESRGAN NCNN Vulkan
- Upscayl
- chaiNNer

Installation and wrapper scripts:

- `C:\UE\T66\Scripts\SetupFreeUIUpscaleStack.ps1`
- `C:\UE\T66\Scripts\InvokeFreeUpscale.ps1`
- `C:\UE\T66\Scripts\InvokeDeterministicResample.py`

Default local install root:

- `C:\UE\T66\Tools\Temp\UIUpscaleStack`

## Tool Roles

### Figma

Use for:

- deterministic `1x`, `2x`, and `4x` exports
- canonical canvas locking
- exact composition carry-through

Do not treat Figma export scaling as true detail reconstruction.

### Deterministic resample

Use for:

- safe `2x/4x` helper exports from the approved master
- measurement, diffing, and close inspection where exact composition matters more than invented detail
- normalizing acceptable landscape imagegen outputs into the `1920x1080` authoring baseline through center-crop plus Lanczos resize

Use:

- `C:\UE\T66\Scripts\InvokeDeterministicResample.py`

Reference-canvas normalization example:

```powershell
python C:\UE\T66\Scripts\InvokeDeterministicResample.py <raw_image.png> <normalized_output.png> --target-width 1920 --target-height 1080
```

### Real-ESRGAN

Use for:

- local scriptable helper upscales
- batch generation of `2x` or `4x` helper references
- conservative sharpening and detail recovery

### Upscayl

Use for:

- quick manual artist review
- comparing upscale models visually
- one-off helper references

### chaiNNer

Use for:

- reusable image-processing graphs
- repeatable pipelines that combine resample, upscale, classification, and export

### Native Codex image generation

Use for:

- new painted helper companions
- style-continuation helper references
- family-board prompting support when prompt-only generation or thread-attached references are enough

This is the preferred first generation path.

## Decision Rule

### If you only need a larger deterministic copy

Use:

- Figma export scaling
- or high-quality resampling

### If you need a cleaner high-res helper for visual study

Use:

- Real-ESRGAN
- or Upscayl

Mark the result `helper-only` unless specifically reviewed.

### If a runtime control plate is proportionally wrong

Do not upscale it as the main fix.

Instead:

- regenerate that family at the correct measured aspect ratio

## Recommended Practice

1. Keep one canonical `1x` master.
2. Produce a deterministic `2x` export when needed.
3. Use AI upscale only for helper inspection or style continuation support.
4. Use native Codex `image_gen` when a new helper render is actually needed.
5. Never assume an AI-upscaled composite is safe to slice into runtime controls.
6. Never use a full reference, buttonless master, or textless master as the runtime background.
7. Promote an output to `runtime-safe` only after a deliberate review.

