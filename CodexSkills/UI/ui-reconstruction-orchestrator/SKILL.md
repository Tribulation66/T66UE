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

Identify which of these already exist:
- approved reference image set
- usable helper reference exports when source readability is the blocker
- trusted `content_ownership.json`
- measured layout manifest
- runtime sprite families
- runtime implementation
- packaged screenshot review

Then route to the earliest missing stage, unless the real blocker is reference usability.

## Routing

### If there is no approved reference for the target screen

Run:
1. code-first content ownership audit
2. `$ui-style-reference`
3. `$ui-layout-manifest`
4. `$ui-sprite-families`
5. `$ui-runtime-reconstruction`
6. `$ui-packaged-review`

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
- Do not use `ui-reference-prep` to paper over wrong proportions or ownership problems.
- Do not route into style-reference, sprite families, or packaged diffing until runtime-owned regions are classified.
- Do not declare success without a packaged review.

## Handoff

When routing, summarize:
- which artifacts exist
- which artifact is missing or untrusted
- which stage skill should run next
- what concrete files that stage should produce
