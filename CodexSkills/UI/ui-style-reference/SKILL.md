---
name: ui-style-reference
description: Create offline reference image sets for new T66 UI screens by extending an approved anchor screen's visual style without redesigning the layout contract. Use when Codex must produce a new screen comparison target, optional analysis variants, or a hi-res helper render that matches an existing approved screen such as the main menu.
---

# UI Style Reference

Create the approved offline reference frame for a new screen. This skill is for style continuation and clean reference production, not runtime implementation.

## Open These First

- `C:\UE\T66\Docs\UI\MASTER.md`
- `C:\UE\T66\Docs\UI\UI_CONTENT_OWNERSHIP_AUDIT.md`
- `C:\UE\T66\Docs\UI\UI_IMAGE_BACKEND_POLICY.md`
- `C:\UE\T66\Docs\UI\UI_SKILL_ARCHITECTURE.md`
- `C:\UE\T66\Docs\UI\UI_Screen_Reference_Pipeline.md`
- `C:\UE\T66\Docs\UI\UI_RECONSTRUCTION_HANDOFF.md` when the main menu is the anchor

## Core Job

Produce an offline reference set that keeps the approved T66 visual language while preserving the target screen structure.

Use the main menu as the golden style/calibration anchor when applicable. Preserve its calibrated contract: canonical `1920x1080` authoring/baseline packaged target, baked title wordmark allowed only as display art, and live/localizable tagline plus runtime labels, values, avatars, icons, media, and list content.

Hard blocking rule: every target screen, modal, HUD overlay, tab, and mini-game UI needs its own generated screen-specific full-screen reference before any sprite-family or runtime placement work begins. Do not proceed from a general description of the main-menu style.

The screen-specific reference must be generated from all three inputs:
- canonical main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- current screenshot of the exact target screen or modal
- layout list for the exact target screen, including regions, controls, panels, live-data wells, variants, and required states

Save the approved output at:

`C:\UE\T66\UI\screens\<screen_slug>\reference\canonical_reference_1920x1080.png`

The full reference screenshot is an offline target only. Do not promote `screen_master`, no-buttons, no-text, or no-dynamic variants as production runtime background layers.

Hard rule: generated style references must come correct from image generation. Do not manually pixel-edit, clean up, mask, erase/fill, cover-patch, clone, repaint, or screenshot-repair generated references. If the output is structurally wrong or contaminated, revise the prompt/request and regenerate. Deterministic export/crop helpers are allowed for inspection, downstream measurement, and target-canvas normalization.

Expected outputs:
- `screen_master.png` as the offline comparison target
- `screen_master_2x.png` as helper/reference art
- optional `screen_master_nobuttons.png`, `screen_master_notext.png`, and `screen_master_nodynamic.png` for analysis or prompting only
- scene/background plate requirements for `$ui-sprite-families`
- short note on what was inferred vs held fixed

Native Codex `image_gen` is the only allowed image-generation path for this workflow. Do not use legacy browser-automation generation, request manifests, token-driven local services, or removed external image-generation tooling. If native generation cannot produce the required reference, report a blocker instead of routing to a external-tool fallback.

This skill only clears the first gate. Do not report the overall screen task complete after reference generation alone; route immediately to `$ui-layout-manifest`, `$ui-sprite-families`, `$ui-runtime-reconstruction`, and `$ui-packaged-review` unless blocked.

## Workflow

### 1. Lock the contract

- Identify the approved anchor screen.
- Identify the target screen skeleton or component map.
- Create or verify `C:\UE\T66\UI\screens\<screen_slug>`.
- Capture the current runtime screenshot of the target screen.
- Write the target screen layout list before image generation.
- Lock the canvas size before generating anything.
- Use `1920x1080` for normal 16:9 offline targets and baseline packaged review unless the intake explicitly locks another size.
- For the active main menu pack, approve the canonical target only after it exists as a visually inspected `1920x1080` normalized output.
- Keep raw imagegen sources archived when normalization is used.
- Run the content ownership audit first.
- Mark dynamic, localizable, or live regions up front.

### 2. Separate fixed from inferred

Keep a short list of:
- what must stay structurally identical to the target layout
- what visual language must match the anchor
- what details are legitimately inferred because no direct reference exists
- which target regions are shell-only, socket-frame, empty-backplate, or open-aperture

### 3. Generate the reference set

- Generate one canonical offline target first from the main-menu anchor, current target screenshot, and layout list.
- If imagegen returns a landscape-safe non-1920 output, normalize a copy with `Scripts\InvokeDeterministicResample.py --target-width 1920 --target-height 1080`; reject it if the crop removes important structure.
- Derive the clean variants from the same composition.
- Keep localizable labels removable from runtime art.
- Keep taglines/subtitles removable unless explicitly approved as baked display art.
- Keep runtime-owned interiors removable from runtime art.
- Prefer a hi-res companion image for asset work, but do not replace the canonical offline target with it.
- If only a deterministic or helper hi-res companion is needed, use `$ui-reference-prep` instead of overloading this stage.
- Generate with native Codex `image_gen`. If the output does not preserve the full screen contract, revise the prompt and regenerate.

### 4. Review for structural fidelity

Check:
- silhouette and region breakdown
- panel and button hierarchy
- spacing rhythm
- whether the new screen still reads as the same product family as the anchor

## Guardrails

- Do not redesign layout hierarchy.
- Do not solve runtime slicing here.
- Do not solve Unreal layout or interaction here.
- Do not hand off any full-screen reference variant as a production runtime background.
- Do not ask image generation to invent the target screen structure from style alone.
- Do not skip the screen-specific generated reference and jump directly to sprites or Slate styling.
- Do not treat a hi-res repaint as the source of truth if it drifts from the canonical frame.
- Do not rescue badly framed or structurally wrong active main menu outputs with resize passes; regenerate them.
- Do not import scenic anchor motifs into utilitarian target screens unless the target layout explicitly calls for them.
- Do not paint portraits, icons, media, or preview content into regions that runtime owns.
- Do not bake runtime values, names, ranks, prices, timers, settings values, or localizable taglines into variants that may inform runtime component families.
- Do not use removed external generation tooling or request manifests as an alternate image path.
- Do not treat this stage as the end of a screen implementation pass.

## Handoff

Hand off:
- master image set
- canvas size
- dynamic-region notes
- `content_ownership.json`
- explicit note that full-screen masters are offline/helper references only
- list of inferred areas

Then move to `$ui-layout-manifest`.

