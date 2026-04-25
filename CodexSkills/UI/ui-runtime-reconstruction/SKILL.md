---
name: ui-runtime-reconstruction
description: Rebuild a reference UI screen in Unreal using measured layout data and runtime art families. Use when Codex must edit screen or widget code, wire visible controls to real buttons, preserve live text or content, and match the approved reference in packaged runtime rather than only in the editor.
---

# UI Runtime Reconstruction

Rebuild the real screen in Unreal. This skill owns the runtime implementation step after the reference, manifest, and art families exist.

## Open These First

- `C:\UE\T66\Docs\UI\MASTER.md`
- `C:\UE\T66\Docs\UI\SCREEN_REVIEW_GATE.md`
- `C:\UE\T66\Docs\UI\UI_CONTENT_OWNERSHIP_AUDIT.md`
- `C:\UE\T66\Docs\UI\UI_SKILL_ARCHITECTURE.md`
- `C:\UE\T66\Docs\UI\UI_RECONSTRUCTION_HANDOFF.md` when working on the main menu
- the relevant runtime files under `C:\UE\T66\Source\T66\UI`

## Core Job

Implement the screen so the packaged build matches the reference with:
- a UI-free scene/background plate
- separate foreground component families for topbar, side panels, leaderboard chrome, and center CTA stack
- real visible controls
- real `FText` labels
- measured placement from the manifest
- live data only where appropriate

## Simplified Contract

Runtime reconstruction is the **placement phase**. Do not solve asset generation here.

Hard rule: runtime work may place accepted generated assets and overlay live text/content, but it must not compensate for bad generated pixels with manual cleanup, masks, erase/fill, cover patches, clone/repaint fixes, or screenshot repair. Send bad assets back for regeneration.

Use one fixed reference canvas per screen body as the authoring coordinate system, then transform it into runtime viewports by semantic anchors, safe zones, DPI scaling, and validated stretch rules:
- center stack, title, and tagline from the screen center
- left panels from left-side anchors
- right panels from right-side anchors
- component interiors in local coordinates inside the owning shell

Do not align widgets over a full-screen reference image. Do not reintroduce extra style/helper layers that partially own the same visible shell.

## Workflow

### 1. Confirm prerequisites

Before changing runtime code, make sure these exist:
- approved screen-specific offline comparison target at `C:\UE\T66\UI\screens\<screen_slug>\reference\canonical_reference_1920x1080.png`
- proof that the target reference was generated from the canonical main-menu anchor, the current target screenshot, and the target layout list
- UI-free scene/background plate when the screen has full-screen background art
- measured layout manifest
- `content_ownership.json`
- runtime art families for the regions being touched
- asset validation notes for dimensions, alpha, nine-slice margins, and state anchors

If they do not exist, stop and push the work back to the correct earlier stage.

If the screen was only styled from a general main-menu feeling, do not patch runtime code. Regenerate the target reference first.

### 2. Assign one owner per region

For each region, decide what owns the visible shell:
- UI-free scene/background plate
- runtime widget
- runtime shell asset

For mixed-ownership regions, decide what owns the interior:
- runtime text
- runtime values or state text
- runtime portrait or avatar
- runtime icon
- runtime media or 3D preview

Do not let multiple layers partially own the same visual shell.

Do not use the full reference screenshot, buttonless master, or textless master as the runtime background. Those are offline comparison or helper artifacts only.

For the main menu, the title wordmark may be baked display art, but the tagline, CTA labels, social rows, leaderboard labels, ranks, names, scores, and states stay live/localizable.

### 3. Build from large to small

Work in this order:
1. shell ownership and region composition
2. scene plate placement and contamination check
3. measured placement and sizing
4. visible control plates and text fit
5. hover, pressed, disabled, and selected states
6. live content framing and polish

### 4. Keep implementation safe

- Keep the visible control as the actual button.
- Keep labels as real `FText`.
- Do not ship hotspot overlays as the interaction model.
- Avoid transient Slate brush lifetime bugs.
- Preserve live Steam-driven content only where the brief requires it.
- Preserve shell rect, visible control rect, and live-content rect separation from the manifest.
- Preserve layered composition: scene plate behind, foreground components above, live/localizable content kept live.

### 5. Validate in packaged runtime

- Build the packaged target, not just the editor view.
- Capture at the canonical authoring baseline, normally `1920x1080`.
- Before final approval, also capture or validate supported aspect buckets: primary `16:9`, `16:10`, ultrawide `21:9`, and one smaller/windowed size where supported.
- Capture a packaged screenshot at the same screen and state as the reference.
- Save the capture under `C:\UE\T66\UI\screens\<screen_slug>\outputs\YYYY-MM-DD\`.
- Use that packaged result for the pass verdict.

## Guardrails

- Do not redesign layout or hierarchy.
- Do not add manual placement nudges if a manifest box already exists.
- Do not let fallback letters or placeholder fills ship as real UI.
- Do not ship a full-screen reference or buttonless/textless master as the background layer.
- Do not call a pass done because one region improved while another regressed.
- Do not integrate a stretchable plate without declared nine-slice margins.

## Handoff

Hand off:
- changed runtime files
- packaged screenshot path
- scene plate and foreground component ownership notes
- note on strict-diffed vs manually validated regions
- `content_ownership.json`
- mask notes for runtime-owned interiors

Then move to `$ui-packaged-review`.
