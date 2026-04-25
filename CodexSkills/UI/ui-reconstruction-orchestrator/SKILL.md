---
name: ui-reconstruction-orchestrator
description: Coordinate the T66 UI reconstruction pipeline across the optional reference-prep helper and the style-reference, layout-manifest, sprite-families, runtime-reconstruction, and packaged-review stages. Use when Codex needs to decide which reconstruction stage to run next, whether reference usability is the real blocker, which artifacts are missing, or which specialist UI skill should own the current work.
---

# UI Reconstruction Orchestrator

Use this skill to route work to the correct stage skill. Keep it thin. Its job is to decide what comes next, not to absorb all implementation detail.

## Open These First

- `C:\UE\T66\Docs\UI\MASTER.md`
- `C:\UE\T66\Docs\UI\UI_CONTENT_OWNERSHIP_AUDIT.md`
- `C:\UE\T66\Docs\UI\UI_SKILL_ARCHITECTURE.md`
- `C:\UE\T66\Docs\UI\UI_RECONSTRUCTION_HANDOFF.md` when the main menu is involved

## Decision Rule

First reduce the request to the two production phases:
- **asset phase:** make or fix the scene plate and foreground component families
- **placement phase:** assemble those assets in Unreal with one coordinate system, semantic anchors, real controls, and live text/data

Route only the missing or failing phase. Do not invent a third phase where runtime widgets are aligned against a full reference screenshot.

For multi-screen or multi-modal assignments, deploy agents at the start for independent capture/audit, reference preparation, asset checklist validation, and packaged review work. Give every agent explicit file or folder ownership, the canonical main-menu anchor path, the target `UI\screens\<screen_slug>` folder, and the same hard reference gate. Do not assign two agents to edit the same runtime file or asset folder.

Every agent also gets the completion gate: reference generation is not completion. Agents must continue through sprite/component generation, runtime implementation, packaged capture, and packaged review unless blocked. If blocked, they must report `blocked` with the exact missing artifact or failing command.

Use native Codex `image_gen` only. Do not route UI image generation through legacy browser-automation generation, request manifests, token-driven local services, or removed external image-generation tooling.

Identify which of these already exist:
- approved reference image set
- screen-specific generated reference created from the canonical main-menu anchor, the current target screenshot, and the target layout list
- UI-free scene/background plate when the screen needs full-screen background art
- usable helper reference exports when source readability is the blocker
- trusted `content_ownership.json`
- measured layout manifest
- runtime sprite families
- asset validation notes for dimensions, alpha, nine-slice margins, and state anchors
- runtime implementation
- packaged screenshot review

Then route to the earliest missing stage, unless the real blocker is reference usability.

The main menu is the golden calibration screen. Before routing another screen as production-ready, confirm the main menu contract is understood: canonical `1920x1080` authoring/baseline packaged target, full reference screenshots are offline targets only, baked title wordmark allowed, live/localizable tagline and runtime labels/values, UI-free scene plate plus foreground component families, separated shell/control/live-content rects, validated assets, ownership-aware packaged masks, and responsive/aspect validation before final approval.

For any non-main-menu target, if `C:\UE\T66\UI\screens\<screen_slug>\reference\canonical_reference_1920x1080.png` does not exist or was not generated from all three required inputs, route immediately to `$ui-style-reference`. Do not allow component generation, runtime styling, or packaged review to continue from a generic style interpretation.

For the active main menu pack, acceptable landscape imagegen outputs may be archived and normalized to `1920x1080` with `Scripts\InvokeDeterministicResample.py --target-width 1920 --target-height 1080`. Route badly framed, portrait/square, structurally cropped, or composition-wrong outputs back to regeneration.

Hard rule: never route a bad generated asset to manual pixel repair. Manual cleanup, masks, erase/fill, cover patches, clone/repaint fixes, and screenshot repair are not valid stages. Route bad pixels back to `ui-sprite-families` or `ui-style-reference` for regeneration; allow only deterministic slicing/cropping and runtime overlays after generation.

## Routing

### If there is no approved reference for the target screen

Run:
1. create the screen pack under `C:\UE\T66\UI\screens\<screen_slug>`
2. capture the current target screenshot
3. write the target layout list
4. code-first content ownership audit
5. `$ui-style-reference`
6. `$ui-layout-manifest`
7. `$ui-sprite-families`
8. `$ui-runtime-reconstruction`
9. `$ui-packaged-review`

### If the approved reference already exists

Run:
1. code-first content ownership audit
2. `$ui-layout-manifest`
3. `$ui-sprite-families`
4. `$ui-runtime-reconstruction`
5. `$ui-packaged-review`

This is the current main-menu path.

### If the reference exists but is too weak to inspect comfortably

Run `$ui-reference-prep` first.

Use it only for:
- deterministic `2x/4x` exports
- helper-only AI upscales
- better prompt/reference support for later stages

Then continue into the normal production stages.

### If the manifest exists but runtime assets do not

Run `$ui-sprite-families`.

### If the assets exist but the packaged screen still diverges

Run `$ui-runtime-reconstruction`, then `$ui-packaged-review`.

### If the screen is not yet in production mode and the team is still exploring style directions

Do not use this stack yet. Use the lighter discovery workflow first, then re-enter this pipeline once one direction is approved.

## Guardrails

- Do not let this skill become a second copy of the specialist skills.
- Do not skip a missing earlier-stage artifact just because a later-stage workaround seems faster.
- Do not skip the per-screen generated reference gate for any screen, modal, HUD overlay, tab, or mini-game UI.
- Do not use `ui-reference-prep` to paper over wrong proportions or ownership problems.
- Do not preserve bad active main menu generated assets by converting them to the canonical canvas; normalization is allowed only for acceptable landscape outputs that survive visual inspection.
- Do not route a buttonless or textless full-screen master into runtime as a background layer.
- Do not route into style-reference, sprite families, or packaged diffing until runtime-owned regions are classified.
- Do not treat a packaged capture as acceptable unless the baseline capture matches the locked target size, normally `1920x1080`, and final approval also checks supported aspect buckets.
- Do not declare success without a packaged review.
- Do not declare success after reference generation alone.
- Do not use removed external generation tooling as an image-generation fallback.

## Handoff

When routing, summarize:
- which artifacts exist
- which artifact is missing or untrusted
- which stage skill should run next
- what concrete files that stage should produce
- whether the next stage must preserve scene plate/foreground component ownership, shell/control/live-content rect separation, or packaged diff masks

