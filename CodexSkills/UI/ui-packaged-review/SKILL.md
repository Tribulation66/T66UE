---
name: ui-packaged-review
description: Review packaged UI captures against approved offline references and classify the remaining deltas. Use when Codex needs to compare packaged screenshots, create diff overlays, rank blockers, and decide whether a mismatch is caused by layout, asset, scene-plate contamination, ownership, text fit, or live-data behavior.
---

# UI Packaged Review

Review the packaged build against the approved reference and classify what is still wrong. This skill exists to make the next pass unambiguous.

## Open These First

- `C:\UE\T66\Docs\UI\MASTER.md`
- `C:\UE\T66\Docs\UI\SCREEN_REVIEW_GATE.md`
- `C:\UE\T66\Docs\UI\UI_CONTENT_OWNERSHIP_AUDIT.md`
- `C:\UE\T66\Docs\UI\UI_SKILL_ARCHITECTURE.md`
- the approved reference image
- the latest packaged screenshot

## Core Job

Produce:
- a ranked blocker list
- diff overlays when possible
- strict-diff masks or mask notes for runtime-owned interiors
- scene plate contamination status
- per-region status
- root-cause classification for the largest deltas

Hard rule: review masks are validation artifacts only. Do not use packaged-review output to manually repair screenshots or generated assets with pixel edits, cleanup, erase/fill, cover patches, clone/repaint fixes, or masks. Asset defects route back to regeneration; layout defects route to runtime reconstruction.

Reference gate: the approved reference image must be the screen-specific generated target at `C:\UE\T66\UI\screens\<screen_slug>\reference\canonical_reference_1920x1080.png`, created from the canonical main-menu anchor, the current target screenshot, and the target layout list. If that reference does not exist, the review verdict is `blocked` and the next stage is `$ui-style-reference`.

## Review Order

Judge the screen in this order:
1. silhouette and framing
2. spacing and alignment
3. type fit and hierarchy
4. material and contrast match
5. state coverage

Do not spend time polishing later items if earlier items are still failing.

## Root-Cause Classes

Use these labels:
- `ownership`
- `layout`
- `asset`
- `scene-plate contamination`
- `text-fit`
- `live-data mismatch`

## Workflow

### 1. Review packaged output only

- Use packaged screenshots as the primary evidence.
- Do not let editor correctness hide a packaged regression.
- Confirm the capture matches the locked target size, normally `1920x1080`.
- Confirm the capture and review artifacts are saved under `C:\UE\T66\UI\screens\<screen_slug>\outputs\YYYY-MM-DD\` or `review\`.
- Confirm the packaged screen is a layered composition, not a full reference screenshot or buttonless/textless master used as the background.

### 2. Separate strict-diff from manual validation

Strict-diff:
- static regions that can be frozen into the same state as the reference
- shell regions whose ownership is generated art
- visible scene plate areas that are not covered by foreground components

Manual validation:
- live regions
- dynamic or data-driven regions
- states that cannot be frozen exactly
- runtime-owned interiors called out in `content_ownership.json`
- title art only when intentionally baked; taglines and runtime values are live unless the intake says otherwise

### 3. Rank blockers

- Fail the scene plate gate before polish if the background includes baked topbar, side panels, CTA stack, leaderboard rows, labels, values, avatars, icons, media, previews, or other foreground/live content.
- Put the most visible or structurally dangerous deltas first.
- Call out regressions explicitly.
- Keep the next-fix list short and concrete.

### 4. Keep the report blunt

Use one of:
- `exact enough`
- `close with blockers`
- `blocked`

## Guardrails

- Do not handwave obvious misses.
- Do not merge multiple root causes into one vague complaint.
- Do not prescribe a new fix until the likely cause is classified.
- Do not accept a pass where the runtime background is a full reference, buttonless, or textless master.
- Do not strict-diff runtime-owned portraits, icons, media, avatars, or preview stages as though they were fixed shell art.
- Do not strict-diff runtime text, values, ranks, scores, settings values, timers, prices, or localizable taglines without a fixture or mask.

## Handoff

Hand off:
- blocker list
- diff artifact paths
- mask artifact paths or mask notes
- latest packaged screenshot path
- `content_ownership.json`
- recommended next stage

Usually the next stage is `$ui-runtime-reconstruction` or `$ui-sprite-families`.
