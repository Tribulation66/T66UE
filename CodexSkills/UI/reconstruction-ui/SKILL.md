---
name: reconstruction-ui
description: Compatibility router for reconstructing UI screens 1:1 from reference images, locked mockups, or screenshots with minimal taste exploration. Use when Codex receives a broad or legacy UI reconstruction request and needs to route it to the newer T66 stage skills for reference prep, style-reference, layout-manifest, sprite-family generation, runtime reconstruction, and packaged review.
---

# Reconstruction UI

This skill now exists to preserve legacy triggers and explicit `$reconstruction-ui` invocations while routing work into the newer skill tree.

## Open These First

- `C:\UE\T66\Docs\UI\MASTER.md`
- `C:\UE\T66\Docs\UI\UI_CONTENT_OWNERSHIP_AUDIT.md`
- `C:\UE\T66\Docs\UI\UI_SKILL_ARCHITECTURE.md`
- `C:\UE\T66\Docs\UI\UI_RECONSTRUCTION_HANDOFF.md`

## Routing Rule

When this skill is invoked:

1. Determine whether the active blocker is reference usability.
2. Determine whether the target has the screen-specific generated reference at `C:\UE\T66\UI\screens\<screen_slug>\reference\canonical_reference_1920x1080.png`, created from the canonical main-menu anchor, the current target screenshot, and the layout list.
3. If that reference is missing, generic, or wrong-resolution, route only to `$ui-style-reference`; do not route to layout, sprites, runtime, or review.
4. Determine whether the screen is using the main menu golden calibration contract or another approved anchor.
5. Determine whether runtime-owned text, values, images, media, and states have been audited into `content_ownership.json`.
6. Determine whether a UI-free scene/background plate and foreground component families are required or already trusted.
7. If reference usability is the blocker, route to `$ui-reference-prep`.
8. Otherwise route to `$ui-reconstruction-orchestrator`.
9. Let the specialist skills do the real work.

Hard rule: do not route generated asset defects to manual pixel repair. Cleanup, masks, erase/fill, cover patches, clone/repaint fixes, and screenshot repair are not valid production paths. Route bad generated pixels back to regeneration; allow only deterministic slicing/cropping and runtime overlays after generation.

Resolution rule: `1672x941` and other non-canonical generated outputs are not production references, sprite sheets, scene plates, slices, or runtime assets for normal 16:9 work. Rebuild natively at `1920x1080`.

## Use `ui-reference-prep` When

- the source reference is too low-resolution to inspect comfortably
- deterministic `2x` or `4x` exports are needed
- helper-only AI upscales would make later prompting or inspection easier

Do not use it to hide wrong proportions or ownership problems.

## Use `ui-reconstruction-orchestrator` When

- the task is a broad reconstruction request
- the user has not already identified the exact stage
- multiple artifacts are missing and the next stage is not obvious

## Direct Stage Routing

If the user is already clearly asking for one specific stage, go directly to:

- `$ui-style-reference`
- `$ui-layout-manifest`
- `$ui-sprite-families`
- `$ui-runtime-reconstruction`
- `$ui-packaged-review`

## Guardrails

- Keep this skill thin.
- Do not duplicate the specialist skill instructions here.
- Do not treat this compatibility layer as the place where implementation happens.
- Do not skip the ownership audit just because the screen already has a reference image.
- Do not treat a full reference screenshot, buttonless master, or textless master as a production runtime background.
- Do not skip the strict baseline: canonical `1920x1080` packaged target for normal 16:9 screens, shell/control/live-content rect separation, asset validation, and packaged diff masks.
- Do not skip the layered baseline: UI-free scene/background plate plus separate foreground component families for topbar, side panels, leaderboard chrome, and center CTA stack.
