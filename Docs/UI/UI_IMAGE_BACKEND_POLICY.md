# UI Image Generation Policy

## Purpose

This file records the active image-generation rule for T66 UI reconstruction.

The policy is intentionally simple: use Codex-native `image_gen` for all UI references, scene plates, component boards, icons, and state families.

## Hard Rule

Do not use legacy browser-automation image generation for UI reconstruction.

Forbidden paths include:

- legacy request manifests
- browser-automation image generation
- legacy generation response JSONs as process artifacts
- any prompt that asks an agent to route image work through removed external generation tooling

If native Codex image generation cannot produce a required artifact, mark the screen blocked and name the missing capability or artifact. Do not route to removed external generation tooling as a fallback.

## Asset Rules

Backend choice does not permit post-generation pixel repair. Generated UI assets must come correct from image generation. Do not manually clean up, mask, erase/fill, cover-patch, clone, repaint, or screenshot-repair generated outputs.

Allowed deterministic operations:

- slicing or cropping from an accepted generated board
- center-crop to the target aspect and Lanczos resize for reference-canvas normalization
- format/export conversion
- alpha or dimension validation
- runtime placement
- runtime text/content overlay
- packaged-review masks for live content

If generated pixels are wrong, regenerate the reference, plate, board, icon, or state family.

## Resolution Rule

Raw non-canonical imagegen outputs are provenance artifacts, not production assets. Acceptable landscape outputs may be normalized into the canonical `1920x1080` authoring baseline with deterministic center-crop plus Lanczos resize:

```powershell
python C:\UE\T66\Scripts\InvokeDeterministicResample.py <raw_image.png> <normalized_output.png> --target-width 1920 --target-height 1080
```

The raw source should stay archived beside the normalized output or in the screen archive. If normalization crops important structure, title, controls, or content, reject the output and regenerate. Square, portrait, badly framed, or structurally wrong outputs remain helper-only or blocked.

Normal 16:9 production UI work uses `1920x1080` as the authoring and review baseline, then validates the runtime layout across supported aspect buckets.

## Stage Guidance

Use native Codex `image_gen` for:

- screen-specific full-screen references generated from the canonical main-menu anchor, current target screenshot, and layout list
- UI-free scene/background plates
- foreground component family boards
- button, panel, top-bar, toggle, dropdown, icon, and socket-frame families
- hover, pressed, disabled, selected, focused, and loading state variants
- helper boards for visual study

Do not generate from a general style description alone. Every target screen or modal must have its own reference gate before sprite work starts.

## Completion Rule

Image generation is not the end of the job. Reference generation unlocks sprite-family generation; sprite-family generation unlocks runtime implementation; runtime implementation unlocks packaged review.

A chat or agent may report completion only after packaged capture and review are done, or after a concrete blocker is recorded.

