---
name: ui-packaged-review
description: Review packaged UI captures against approved references and classify the remaining deltas. Use when Codex needs to compare packaged screenshots, create diff overlays, rank blockers, and decide whether a mismatch is caused by layout, asset, ownership, text fit, or live-data behavior.
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
- per-region status
- root-cause classification for the largest deltas

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
- `text-fit`
- `live-data mismatch`

## Workflow

### 1. Review packaged output only

- Use packaged screenshots as the primary evidence.
- Do not let editor correctness hide a packaged regression.

### 2. Separate strict-diff from manual validation

Strict-diff:
- static regions that can be frozen into the same state as the reference

Manual validation:
- live regions
- dynamic or data-driven regions
- states that cannot be frozen exactly
- runtime-owned interiors called out in `content_ownership.json`

### 3. Rank blockers

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
- Do not strict-diff runtime-owned portraits, icons, media, avatars, or preview stages as though they were fixed shell art.

## Handoff

Hand off:
- blocker list
- diff artifact paths
- latest packaged screenshot path
- `content_ownership.json`
- recommended next stage

Usually the next stage is `$ui-runtime-reconstruction` or `$ui-sprite-families`.
