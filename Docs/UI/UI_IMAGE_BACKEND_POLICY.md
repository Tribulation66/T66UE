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
- format/export conversion
- alpha or dimension validation
- runtime placement
- runtime text/content overlay
- packaged-review masks for live content

If generated pixels are wrong, regenerate the reference, plate, board, icon, or state family.

## Resolution Rule

`1672x941` and other non-canonical wide outputs are helper-only unless the user explicitly approves a separate conversion stage. They must not become production references, sprite sheets, scene plates, slices, or runtime assets.

Normal 16:9 production UI work rebuilds natively at `1920x1080`.

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

