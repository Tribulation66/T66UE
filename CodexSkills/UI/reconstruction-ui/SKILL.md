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
2. Determine whether runtime-owned regions have been audited into `content_ownership.json`.
3. If reference usability is the blocker, route to `$ui-reference-prep`.
4. Otherwise route to `$ui-reconstruction-orchestrator`.
5. Let the specialist skills do the real work.

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
