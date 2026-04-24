---
name: ui-layout-manifest
description: Convert an approved UI reference image into measured runtime layout data. Use when Codex needs to derive `reference_layout.json`, generated layout headers, or labeled debug overlays from a locked screen reference so all runtime placement uses one coordinate system.
---

# UI Layout Manifest

Turn a locked offline reference into measured geometry. This skill exists to remove guesswork from placement.

## Open These First

- `C:\UE\T66\Docs\UI\MASTER.md`
- `C:\UE\T66\Docs\UI\UI_CONTENT_OWNERSHIP_AUDIT.md`
- `C:\UE\T66\Docs\UI\UI_SKILL_ARCHITECTURE.md`
- `C:\UE\T66\Docs\UI\UI_RECONSTRUCTION_HANDOFF.md` when working on the main menu
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\reference_layout.json` if a prior manifest already exists

## Core Job

Produce one placement source of truth:
- `reference_layout.json`
- generated layout header
- optional labeled debug overlay

## Workflow

### 1. Lock the reference canvas

- Work from the approved screen-specific offline comparison target at `C:\UE\T66\UI\screens\<screen_slug>\reference\canonical_reference_1920x1080.png`.
- Confirm it was generated from the canonical main-menu anchor, the current target screenshot, and the target layout list. If this proof is missing, stop and route back to `ui-style-reference`.
- Confirm this frame is not intended to become a runtime background.
- A prepared `layout-export` is acceptable, but keep the original canonical pixel size and scale factor explicit.
- Confirm the exact pixel size before measuring boxes.
- Normal 16:9 screens use `1920x1080` as the canonical measurement and packaged acceptance target unless the intake locks another size.
- Reject `1672x941` and other non-canonical generated outputs as production measurement sources for normal 16:9 screens.
- If multiple reference variants exist, keep one primary comparison frame.
- Load `content_ownership.json` before measuring mixed-ownership regions.

### 2. Partition the screen

Create stable named regions such as:
- UI-free scene/background plate
- top bar
- center stage
- CTA stack
- left panel
- right panel
- leaderboard chrome and live content

### 3. Separate rect types

For every important element, distinguish:
- scene plate visible rect
- visual crop rect
- runtime control rect
- live-content rect when the shell and live content are different boxes
- shell rect versus live-content rect when runtime owns the interior

This distinction matters. Many reconstruction failures come from using a crop box as the real interaction or content box.

For controls, record state-anchor expectations when they matter: the visual plate, hit rect, and hover/pressed/disabled artwork must share the same anchor unless the manifest explicitly states otherwise.

### 4. Generate manifest artifacts

- Prefer repo scripts when they already own the format.
- Keep naming stable and explicit.
- Generate the paired header when the runtime expects it.

### 5. Validate the overlay

- Render or inspect a labeled overlay against the reference.
- Reject boxes that require "looks right" nudges outside the manifest.

## Guardrails

- Do not generate new art here.
- Do not treat a full-screen reference, buttonless master, or textless master as a runtime background.
- Do not manually offset runtime elements if a manifest box already exists.
- Do not mix multiple coordinate systems.
- Do not hide uncertainty behind broad crop regions.
- Do not collapse shell, control, and live-content ownership into one generic rectangle.

## Handoff

Hand off:
- `reference_layout.json`
- generated layout header
- labeled overlay or equivalent proof
- `content_ownership.json`
- canonical canvas and packaged target size
- shell/control/live-content rect notes
- scene plate and foreground component ownership notes

Then move to `$ui-sprite-families` or `$ui-runtime-reconstruction`, depending on what already exists.
