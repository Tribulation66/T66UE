---
name: ui-sprite-families
description: Generate clean runtime-ready UI art families from an approved reference and measured layout. Use when Codex must produce button, panel, top-bar, toggle, or icon family boards and slices with real normal, hover, pressed, or disabled states and no baked localizable text.
---

# UI Sprite Families

Generate reusable runtime art families from the approved reference. This skill is for clean asset production, not layout measurement or Unreal implementation.

## Open These First

- `C:\UE\T66\Docs\UI\MASTER.md`
- `C:\UE\T66\Docs\UI\UI_CONTENT_OWNERSHIP_AUDIT.md`
- `C:\UE\T66\Docs\UI\UI_IMAGE_BACKEND_POLICY.md`
- `C:\UE\T66\Docs\UI\UI_SKILL_ARCHITECTURE.md`
- `C:\UE\T66\Docs\UI\UI_Reconstruction_Sprite_Sheet_Workflow.md`
- `C:\UE\T66\Docs\UI\UI_RECONSTRUCTION_HANDOFF.md` when working on the main menu
- the relevant `reference_layout.json`
- the relevant `content_ownership.json`

## Core Job

Produce family boards and slices for recurring chrome:
- top bar plates
- CTA plates
- panel shells
- toggles
- icons

Expected outputs:
- family boards
- sliced runtime assets
- saved prompt packs
- state list per family

## Workflow

### 1. Choose one family at a time

Do not ask one generation pass to solve unrelated widget classes. Work family by family:
- `topbar-family`
- `cta-family`
- `panel-shell-family`
- `toggle-family`
- `icon-family`

Current tested backend rule:
- prefer native Codex `image_gen` first for family boards
- use the bridge as fallback when native output drifts or attachment-heavy prompting is mandatory

### 2. Use measured proportions

- Pull the actual target sizes from the layout manifest.
- Match aspect ratios before generating anything.
- Generate native-proportion art instead of stretching a fixed plate later.
- Helper-prepped images may guide prompting or inspection, but they remain `helper-only` unless explicitly reviewed as `runtime-safe`.
- If runtime owns the interior, generate the shell around the live well instead of faking the inserted content.

### 3. Keep runtime art clean

- Do not bake localizable labels into final control art.
- Generate empty plates and shells when runtime text must stay live.
- Generate socket frames, empty backplates, or open apertures when runtime owns portraits, icons, avatars, media, or previews.
- Record the required states:
  - `normal`
  - `hover`
  - `pressed`
  - `disabled`
  - `selected` when needed

### 4. Slice and name deterministically

- Preserve family-specific naming.
- Keep sliced assets grouped by family.
- Save the prompt that produced a successful family so later screens can reuse it.

### 5. Reject weak methods

Do not rely on:
- generic cleanup passes over contaminated screenshots
- a single board trying to cover every UI element
- non-uniform stretching of fixed plate art

## Guardrails

- Match the approved reference, not older UI families.
- Preserve shell logic and border structure.
- Keep interaction-state ownership in mind when generating plates.
- If a button or panel cannot stretch cleanly, regenerate it at the correct proportion instead of forcing it.
- Do not bake runtime-owned portraits, icons, media, or preview content into a family board.

## Handoff

Hand off:
- final family boards
- final slices
- prompt pack used
- `content_ownership.json`
- state matrix

Then move to `$ui-runtime-reconstruction`.
