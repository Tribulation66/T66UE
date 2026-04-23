---
name: ui-style-reference
description: Create canonical reference image sets for new T66 UI screens by extending an approved anchor screen's visual style without redesigning the layout contract. Use when Codex must produce a new screen master, buttonless or textless variants, or a hi-res companion render that matches an existing approved screen such as the main menu.
---

# UI Style Reference

Create the approved reference frame for a new screen. This skill is for style continuation and clean reference production, not runtime implementation.

## Open These First

- `C:\UE\T66\Docs\UI\MASTER.md`
- `C:\UE\T66\Docs\UI\UI_CONTENT_OWNERSHIP_AUDIT.md`
- `C:\UE\T66\Docs\UI\UI_IMAGE_BACKEND_POLICY.md`
- `C:\UE\T66\Docs\UI\UI_SKILL_ARCHITECTURE.md`
- `C:\UE\T66\Docs\UI\UI_Screen_Reference_Pipeline.md`
- `C:\UE\T66\Docs\UI\UI_RECONSTRUCTION_HANDOFF.md` when the main menu is the anchor

## Core Job

Produce a reference set that keeps the approved T66 visual language while preserving the target screen structure.

Expected outputs:
- `screen_master.png`
- `screen_master_2x.png`
- `screen_master_nobuttons.png`
- `screen_master_notext.png`
- optional `screen_master_nodynamic.png`
- short note on what was inferred vs held fixed

## Workflow

### 1. Lock the contract

- Identify the approved anchor screen.
- Identify the target screen skeleton or component map.
- Lock the canvas size before generating anything.
- Run the content ownership audit first.
- Mark dynamic, localizable, or live regions up front.

### 2. Separate fixed from inferred

Keep a short list of:
- what must stay structurally identical to the target layout
- what visual language must match the anchor
- what details are legitimately inferred because no direct reference exists
- which target regions are shell-only, socket-frame, empty-backplate, or open-aperture

### 3. Generate the reference set

- Generate one canonical master first.
- Derive the clean variants from the same composition.
- Keep localizable labels removable from runtime art.
- Keep runtime-owned interiors removable from runtime art.
- Prefer a hi-res companion image for asset work, but do not replace the canonical layout master with it.
- If only a deterministic or helper hi-res companion is needed, use `$ui-reference-prep` instead of overloading this stage.
- Current tested backend rule: for a canonical full-screen screen master built from multiple reference anchors, prefer the attachment-backed bridge path first. Use native Codex `image_gen` mainly for helper shell boards, supplements, or quick alternatives unless it clearly preserves the full screen contract.

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
- Do not ask image generation to invent the target screen structure from style alone.
- Do not treat a hi-res repaint as the source of truth if it drifts from the canonical frame.
- Do not import scenic anchor motifs into utilitarian target screens unless the target layout explicitly calls for them.
- Do not paint portraits, icons, media, or preview content into regions that runtime owns.

## Handoff

Hand off:
- master image set
- canvas size
- dynamic-region notes
- `content_ownership.json`
- list of inferred areas

Then move to `$ui-layout-manifest`.
