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
- real visible controls
- real `FText` labels
- measured placement from the manifest
- live data only where appropriate

## Workflow

### 1. Confirm prerequisites

Before changing runtime code, make sure these exist:
- approved reference
- measured layout manifest
- `content_ownership.json`
- runtime art families for the regions being touched

If they do not exist, stop and push the work back to the correct earlier stage.

### 2. Assign one owner per region

For each region, decide what owns the visible shell:
- backdrop art
- runtime widget
- runtime shell asset

For mixed-ownership regions, decide what owns the interior:
- runtime text
- runtime portrait or avatar
- runtime icon
- runtime media or 3D preview

Do not let multiple layers partially own the same visual shell.

### 3. Build from large to small

Work in this order:
1. shell ownership and region composition
2. measured placement and sizing
3. visible control plates and text fit
4. hover, pressed, disabled, and selected states
5. live content framing and polish

### 4. Keep implementation safe

- Keep the visible control as the actual button.
- Keep labels as real `FText`.
- Do not ship hotspot overlays as the interaction model.
- Avoid transient Slate brush lifetime bugs.
- Preserve live Steam-driven content only where the brief requires it.

### 5. Validate in packaged runtime

- Build the packaged target, not just the editor view.
- Capture a packaged screenshot at the same screen and state as the reference.
- Use that packaged result for the pass verdict.

## Guardrails

- Do not redesign layout or hierarchy.
- Do not add manual placement nudges if a manifest box already exists.
- Do not let fallback letters or placeholder fills ship as real UI.
- Do not call a pass done because one region improved while another regressed.

## Handoff

Hand off:
- changed runtime files
- packaged screenshot path
- note on strict-diffed vs manually validated regions
- `content_ownership.json`

Then move to `$ui-packaged-review`.
